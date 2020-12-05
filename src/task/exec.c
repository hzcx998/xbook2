#include <xbook/process.h>
#include <string.h>
#include <xbook/pthread.h>
#include <xbook/memspace.h>
#include <xbook/debug.h>
#include <xbook/elf32.h>
#include <xbook/schedule.h>
#include <arch/interrupt.h>
#include <arch/task.h>
#include <unistd.h>
#include <sys/stat.h>
#include <xbook/dir.h>
#include <xbook/fsal.h>
#include <xbook/fd.h>

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
    unsigned long flags;
    interrupt_save_and_disable(flags);
    proc_close_other_threads(cur);
    interrupt_restore_state(flags);
    int fd = kfile_open(pathname, O_RDONLY);
    if (fd < 0) {
        goto free_task_arg;
    }
    struct stat sbuf;
    if (kfile_stat(pathname, &sbuf) < 0) {
        pr_err("[exec]: %s: file stat failed!\n", __func__);
        goto free_tmp_fd;
    }
    if (!sbuf.st_size) {
        pr_err("[exec]: %s: file size is zero!\n", __func__);
        goto free_tmp_fd;
    }
    #ifdef CONFIG_32BIT     /* 32位 elf 头解析 */
    struct Elf32_Ehdr elf_header;
    memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));
    kfile_lseek(fd, 0, SEEK_SET);
    if (kfile_read(fd, &elf_header, sizeof(struct Elf32_Ehdr)) != sizeof(struct Elf32_Ehdr)) {
        printk(KERN_ERR "sys_exec_file: read elf header failed!\n");
        goto free_tmp_fd;
    }
    if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) || \
        elf_header.e_type != 2 || \
        elf_header.e_machine != 3 || \
        elf_header.e_version != 1 || \
        elf_header.e_phnum > 1024 || \
        elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) {
        printk(KERN_DEBUG "sys_exec_file: ident=%s type=%d machine=%d version=%d phnum=%d\n",
            elf_header.e_ident, elf_header.e_type, elf_header.e_machine,
            elf_header.e_version, elf_header.e_phnum, elf_header.e_phentsize);
        printk(KERN_ERR "sys_exec_file: it is not a elf format file!\n", name);
        goto free_tmp_fd;
    }
    #else   /* CONFIG_64BIT 64位 elf 头解析 */
    #endif

    char **new_envp = NULL;
    char **new_argv = NULL;
    char *tmp_arg = NULL;
    /* 如果任务中带有参数，就不用重新构建新参数 */
    if (cur->vmm->argbuf) {
        new_envp = cur->vmm->envp;
        new_argv = cur->vmm->argv;
    } else {
        tmp_arg = mem_alloc(PAGE_SIZE);
        if (tmp_arg == NULL) {
            printk(KERN_ERR "sys_exec_file: task %s malloc for tmp arg failed!\n", name);
            goto free_tmp_fd;
        }
        unsigned long arg_bottom;
        proc_build_arg((unsigned long) tmp_arg + PAGE_SIZE, &arg_bottom, (char **) envp, &new_envp);
        proc_build_arg(arg_bottom, NULL, (char **) argv, &new_argv); 
    }

    char tmp_name[MAX_TASK_NAMELEN] = {0};
    strcpy(tmp_name, name);
    vmm_unmap_the_mapping_space(cur->vmm);
    vmm_release_space(cur->vmm);
    if (proc_load_image(cur->vmm, &elf_header, fd) < 0) {
        printk(KERN_ERR "sys_exec_file: load_image failed!\n");
        goto free_tmp_arg;
    }
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)cur + TASK_KERN_STACK_SIZE - sizeof(trap_frame_t));
    proc_trap_frame_init(cur);
    if(process_frame_init(cur, frame, new_argv, new_envp) < 0){
        goto free_loaded_image;
    }
    if (cur->vmm->argbuf) {
        vmm_debuild_argbug(cur->vmm);
    } else {
        if (tmp_arg)
            mem_free(tmp_arg);
    }
    kfile_close(fd);
    proc_map_space_init(cur);
    pthread_desc_init(cur->pthread);
    fs_fd_reinit(cur);
    exception_manager_exit(&cur->exception_manager);
    exception_manager_init(&cur->exception_manager);
    user_set_entry_point(frame, (unsigned long)elf_header.e_entry);
    memset(cur->name, 0, MAX_TASK_NAMELEN);
    strcpy(cur->name, tmp_name);
    kernel_switch_to_user(frame);
free_loaded_image:
    sys_exit(-1);
free_tmp_arg:
    if (tmp_arg)
        mem_free(tmp_arg);
free_tmp_fd:
    kfile_close(fd);
free_task_arg:
    vmm_debuild_argbug(cur->vmm);
    return -1;   
}

int sys_execve(const char *pathname, const char *argv[], const char *envp[])
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
                printk(KERN_ERR "%s: path %s not executable!", __func__, newpath);
                return -1;
            }
        }
    } else if ((*p == '.' && *(p+1) == '/') || (*p == '.' && *(p+1) == '.' && *(p+2) == '/')) {    /* 当前目录 */
        build_path(p, newpath);
        if (!kfile_access(newpath, F_OK)) {
            char *pname = strrchr(newpath, '/');
            if (pname)
                pname++;
            else 
                pname =  newpath;
            if (do_execute((const char* )newpath, (char *)pname, argv, envp)) {
                printk(KERN_ERR "%s: path %s not executable!", __func__, newpath);
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
                wash_path(newpath, finalpath);
                if (!kfile_access(finalpath, F_OK)) {
                    char *pname = strrchr(finalpath, '/');
                    if (pname)
                        pname++;
                    else 
                        pname =  finalpath;
                    do_execute((const char* )finalpath, (char *)pname, argv, envp);
                }
                env++;
                memset(newpath, 0, MAX_PATH);
            }
        }
    }
    printk(KERN_ERR "%s: path %s not exist or not executable!\n", __func__, pathname);
    return -1;
}
