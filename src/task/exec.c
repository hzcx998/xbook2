#include <xbook/process.h>
#include <string.h>
#include <xbook/pthread.h>
#include <xbook/memspace.h>
#include <xbook/debug.h>
#include <xbook/elf.h>
#include <xbook/schedule.h>
#include <arch/interrupt.h>
#include <arch/task.h>
#include <unistd.h>
#include <sys/stat.h>
#include <xbook/dir.h>
#include <xbook/fsal.h>
#include <xbook/fd.h>
#include <xbook/fs.h>
#include <xbook/process.h>
#include <xbook/safety.h>
#include <errno.h>

/**
 * exec使用新的镜像以及堆栈替换原有的内容。
 * 注意，加载代码和数据的过程中，不会释放掉已经映射的内容，
 * 只是把虚拟内存空间结构释放掉。
 * 内存映射也是安全映射，即如果虚拟地址没有映射才映射，已经映射就不映射。
 * 只有在退出进程的时候才释放所有资源。这样也在一定程度上提高了效率，
 * 但是占用的内存变大，是空间换取时间的做法。
 * 
 * 如果在线程中执行exec，那么线程会全部关闭，并把当前进程用新进程镜像替换。
 */
static int do_execute(const char *pathname, char *name, const char *argv[], const char *envp[])
{
    task_t *cur = task_current;
    //errprint("[exec]: %s: path %s\n", __func__, pathname);
    /*
    unsigned long flags;
    interrupt_save_and_disable(flags);*/
    proc_close_other_threads(cur);
    //interrupt_restore_state(flags);
    vmm_t *old_vmm = cur->vmm;

    int fd = kfile_open(pathname, O_RDONLY);
    if (fd < 0) {
        errprint("[exec]: %s: file %s not exist!\n", __func__, pathname);
        goto free_task_arg;
    }
    struct stat sbuf;
    if (kfile_stat(pathname, &sbuf) < 0) {
        errprint("[exec]: %s: file stat failed!\n", __func__);
        goto free_tmp_fd;
    }
    if (!sbuf.st_size) {
        errprint("[exec]: %s: file size is zero!\n", __func__);
        goto free_tmp_fd;
    }
    #ifdef CONFIG_32BIT     /* 32位 elf 头解析 */
    Elf32_Ehdr elf_header;
    memset(&elf_header, 0, sizeof(Elf32_Ehdr));
    kfile_lseek(fd, 0, SEEK_SET);
    if (kfile_read(fd, &elf_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        keprint(PRINT_ERR "sys_exec_file: read elf header failed!\n");
        goto free_tmp_fd;
    }
    if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) || \
        elf_header.e_type != 2 || \
        elf_header.e_machine != 3 || \
        elf_header.e_version != 1 || \
        elf_header.e_phnum > 1024 || \
        elf_header.e_phentsize != sizeof(Elf32_Phdr)) {
        keprint(PRINT_DEBUG "sys_exec_file: ident=%s type=%d machine=%d version=%d phnum=%d\n",
            elf_header.e_ident, elf_header.e_type, elf_header.e_machine,
            elf_header.e_version, elf_header.e_phnum, elf_header.e_phentsize);
        keprint(PRINT_ERR "sys_exec_file: it is not a elf 32 format file!\n", name);
        goto free_tmp_fd;
    }
    #else   /* CONFIG_64BIT 64位 elf 头解析 */
    Elf64_Ehdr elf_header;
    memset(&elf_header, 0, sizeof(Elf64_Ehdr));
    kfile_lseek(fd, 0, SEEK_SET);
    if (kfile_read(fd, &elf_header, sizeof(Elf64_Ehdr)) != sizeof(Elf64_Ehdr)) {
        keprint(PRINT_ERR "sys_exec_file: read elf header failed!\n");
        goto free_tmp_fd;
    }
    if (elf_header.magic != ELF_MAGIC) {
        keprint(PRINT_DEBUG "sys_exec_file: ident=%s type=%d machine=%d version=%d phnum=%d\n",
            elf_header.e_ident, elf_header.e_type, elf_header.e_machine,
            elf_header.e_version, elf_header.e_phnum, elf_header.e_phentsize);
        keprint(PRINT_ERR "sys_exec_file: it is not a elf 64 format file!\n", name);
        goto free_tmp_fd;
    }
    #endif
    
    vmm_t *new_vmm = mem_alloc(sizeof(vmm_t));
    assert(new_vmm != NULL);
    vmm_init(new_vmm);

    char **new_envp = NULL;
    char **new_argv = NULL;
    char *tmp_arg = NULL;
    /* 如果任务中带有参数，就不用重新构建新参数 */
    if (old_vmm->argbuf) {  /* 来自create_process */
        new_envp = old_vmm->envp;
        new_argv = old_vmm->argv;
        /* 不需要释放进程参数，因为在create_process中已经进行了释放 */
    } else {    /* 来自proc_execute */
        tmp_arg = mem_alloc(PAGE_SIZE);
        if (tmp_arg == NULL) {
            keprint(PRINT_ERR "sys_exec_file: task %s malloc for tmp arg failed!\n", name);
            goto free_tmp_fd;
        }
        unsigned long arg_bottom;
        proc_build_arg((unsigned long) tmp_arg + PAGE_SIZE, &arg_bottom, (char **) envp, &new_envp);
        proc_build_arg(arg_bottom, NULL, (char **) argv, &new_argv); 

        /* 完成参数构建后需要释放进程参数，因为proc_exec执行前，在内核中复制了用户参数 */
        proc_free_arg((char **)argv);
        proc_free_arg((char **)envp);
        argv = NULL;
        envp = NULL;
    }

    char tmp_name[MAX_TASK_NAMELEN] = {0};
    strcpy(tmp_name, name);
    vmm_unmap_the_mapping_space(old_vmm);
    vmm_release_space(old_vmm);
    #ifdef CONFIG_32BIT     /* 32位 elf 头解析 */
    if (proc_load_image32(cur->vmm, &elf_header, fd) < 0) {
        keprint(PRINT_ERR "sys_exec_file: load_image failed!\n");
        goto free_tmp_arg;
    }
    #else
    if (proc_load_image64_ext(new_vmm, &elf_header, fd) < 0) {
        keprint(PRINT_ERR "sys_exec_file: load_image failed!\n");
        goto free_tmp_arg;
    }
    #endif
    #ifdef TASK_TRAPFRAME_ON_KSTACK
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)cur + TASK_KERN_STACK_SIZE - sizeof(trap_frame_t));
    #else
    trap_frame_t *frame = cur->trapframe;
    assert(frame != NULL);
    #endif
    proc_trap_frame_init(cur);
    if(process_frame_init(cur, new_vmm, frame, new_argv, new_envp) < 0){
        goto free_loaded_image;
    }
    if (old_vmm->argbuf) {
        vmm_debuild_argbuf(old_vmm);
    } else {
        if (tmp_arg)
            mem_free(tmp_arg);
    }
    vmm_free(old_vmm);
    cur->vmm = new_vmm;
    kfile_close(fd);

    /* proc exec init */
    proc_exec_init(cur);
    user_set_entry_point(frame, (unsigned long)elf_header.e_entry);
    memset(cur->name, 0, MAX_TASK_NAMELEN);
    strcpy(cur->name, tmp_name);
    
    kernel_switch_to_user(frame);
free_loaded_image:
    sys_exit(-1);
free_tmp_arg:
    if (tmp_arg) {
        mem_free(tmp_arg);
        tmp_arg = NULL;
    }
free_tmp_fd:
    kfile_close(fd);
free_task_arg:
    if (old_vmm->argbuf) {
        vmm_debuild_argbuf(cur->vmm);
    } else {
        if (argv) {
            proc_free_arg((char **)argv);
        }
        if (envp) {
            proc_free_arg((char **)envp);
        }
    }
    return -1;   
}

int proc_execve(const char *pathname, const char *argv[], const char *envp[])
{
    if (pathname == NULL)
        return -1;
    char *p = (char *) pathname;
    char newpath[MAX_PATH];
    memset(newpath, 0, MAX_PATH);
    if (*p == '/') { 
        wash_path(p, newpath);
        if (!kfile_access((const char *) newpath, F_OK)) {
            char *name = strrchr(newpath, '/');
            if (name) {
                name++;
            } else {
                name = (char *) newpath;
            }
            if (do_execute((const char *) p, name, argv, envp) < 0) {
                keprint(PRINT_ERR "%s: path %s not executable!", __func__, newpath);
                return -1;
            }
        }
    } else if ((*p == '.' && *(p+1) == '/') || (*p == '.' && *(p+1) == '.' && *(p+2) == '/')) {    /* 当前目录 */
        build_path(p, newpath);
        //keprint("build path: %s -> %s\n", p, newpath);
        if (!kfile_access(newpath, F_OK)) {
            char *pname = strrchr(newpath, '/');
            if (pname)
                pname++;
            else 
                pname =  newpath;
            if (do_execute((const char* )newpath, (char *)pname, argv, envp)) {
                keprint(PRINT_ERR "%s: path %s not executable!", __func__, newpath);
                return -1;
            }
        }
    } else {
        if (envp) {
            char **env = (char **) envp;
            char *q;
            while (*env) {
                q = *env;
                strcpy(newpath, q);
                if (newpath[strlen(newpath) - 1] != '/') {
                    strcat(newpath, "/");
                }
                strcat(newpath, p);
                char finalpath[MAX_PATH] = {0};
                /* 清洗路径时第一个字符必须是'/' */
                wash_path(strchr(newpath, '/'), finalpath);
                if (!kfile_access(finalpath, F_OK)) {
                    char *pname = strrchr(finalpath, '/');
                    if (pname)
                        pname++;
                    else 
                        pname =  finalpath;
                    if (do_execute((const char* )finalpath, (char *)pname, argv, envp) < 0)
                        keprint(PRINT_ERR "%s: path %s not executable!\n", __func__, pathname);                
                }
                env++;
                memset(newpath, 0, MAX_PATH);
            }
        } else {
            if (do_execute((const char* )pathname, (char *)pathname, argv, envp) < 0) {
                keprint(PRINT_ERR "%s: path %s not executable!", __func__, newpath);
                return -1;
            }
        }
    }
    keprint(PRINT_ERR "%s: path %s not exist or not executable!\n", __func__, pathname);
    return -1;
}

int sys_execve(const char *pathname, const char *argv[], const char *envp[])
{
    if (!pathname)
        return -EINVAL;
    /* 从用户态复制参数到内核 */
    char path[32] = {0};
    if (mem_copy_from_user_str(path, (void *)pathname, MAX_PATH) < 0) {
        return -EINVAL;
    }
    char *_argv[MAX_TASK_STACK_ARG_NR], *_envp[MAX_TASK_STACK_ARG_NR];
    memset(_argv, 0, sizeof(_argv));
    memset(_envp, 0, sizeof(_envp));
    if (proc_copy_arg_from_user((char **)_argv, (char **)argv) < 0) {
        errprintln("[exec] proc_copy_arg_from_user for argv failed!");
        return -ENOMEM;
    }
    if (proc_copy_arg_from_user((char **)_envp, (char **)envp) < 0) {
        errprintln("[exec] proc_copy_arg_from_user for envp failed!");
        proc_free_arg(_argv);
        return -ENOMEM;
    }
    return proc_execve((const char *)path, (const char **)_argv, (const char **)_envp);
}