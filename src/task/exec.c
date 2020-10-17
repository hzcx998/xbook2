#include <xbook/process.h>
#include <string.h>
#include <xbook/pthread.h>
#include <xbook/srvcall.h>
#include <xbook/vmspace.h>
#include <xbook/debug.h>
#include <xbook/elf32.h>
#include <xbook/schedule.h>
#include <arch/interrupt.h>
#include <arch/task.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fsal/dir.h>
#include <fsal/fsal.h>
#include <gui/message.h>

/**
 * exec使用新的镜像以及堆栈替换原有的内容。
 * 注意，加载代码和数据的过程中，不会释放掉已经映射的内容，
 * 只是把虚拟内存空间结构释放掉。
 * 内存映射也是安全映射，即如果虚拟地址没有映射才映射，已经映射就不映射。
 * 只有在退出进程的时候才释放所有资源。这样也在一定程度上提高了效率，
 * 但是占用的内存变大，是空间换取时间的做法。
 * 
 * 如果在线程中执行exec，那么线程会全部关闭，并把当前进程用新进程镜像替换。
 * 
 */

// #define DEBUG_EXEC

/**
 * do_execute - 替换当前进程的运行镜像
 * 
 * @pathname: 文件路径名字
 * @name: 进程的名字
 * @argv: 参数的指针
 * @envp: 环境的指针
 * 
 * 通过给出的文件，解析加载镜像。
 * 
 * @return: 失败返回-1，成功不返回
 */
static int do_execute(const char *pathname, char *name, const char *argv[], const char *envp[])
{
#ifdef DEBUG_EXEC
    printk(KERN_DEBUG "%s: enter.\n", __func__);
#endif
    /* 没有参数或者参数错误 */
    task_t *cur = current_task;
    unsigned long flags;
    save_intr(flags);

    /* 执行新进程的时候，需要关闭旧进程的子线程。 */
    close_other_threads(cur);
    
    restore_intr(flags);

    /* 根据文件信息创建临时原始块 */
    int fd = sys_open(pathname, O_RDONLY);
    if (fd < 0) {   /* file not exist! */
        return -1;
    }
    struct stat sbuf;
    if (sys_stat(pathname, &sbuf) < 0) {
        pr_err("[exec]: %s: file stat failed!\n", __func__);
        goto free_tmp_fd;
    }
    if (!sbuf.st_size) {
        pr_err("[exec]: %s: file size is zero!\n", __func__);
        goto free_tmp_fd;
    }
#ifdef CONFIG_32BIT     /* 32位 elf 头解析 */
    /* 读取文件头 */
    struct Elf32_Ehdr elf_header;
    memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));
    sys_lseek(fd, 0, SEEK_SET);
    if (sys_read(fd, &elf_header, sizeof(struct Elf32_Ehdr)) != sizeof(struct Elf32_Ehdr)) {
        printk(KERN_ERR "sys_exec_file: read elf header failed!\n");
        goto free_tmp_fd;
    }

    /* 检验elf头，看它是否为一个elf格式的程序 */
    if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) || \
        elf_header.e_type != 2 || \
        elf_header.e_machine != 3 || \
        elf_header.e_version != 1 || \
        elf_header.e_phnum > 1024 || \
        elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) {
        printk(KERN_DEBUG "sys_exec_file: ident=%s type=%d machine=%d version=%d phnum=%d\n",
            elf_header.e_ident, elf_header.e_type, elf_header.e_machine,
            elf_header.e_version, elf_header.e_phnum, elf_header.e_phentsize);
        
        /* 头文件检测出错 */
        printk(KERN_ERR "sys_exec_file: it is not a elf format file!\n", name);
        goto free_tmp_fd;
    }
#else   /* CONFIG_64BIT 64位 elf 头解析 */

#endif
#ifdef DEBUG_EXEC
    int argc = 0;
    printk(KERN_DEBUG "%s: dump args:\n", __func__);
    if (argv) {   
        while (argv[argc]) {
            printk(KERN_DEBUG "arg: %s\n", argv[argc]);
            argc++;
        }    
    }
    int envc = 0;
    printk(KERN_DEBUG "%s: dump envp:\n", __func__);
    if (envp) {  
        while (envp[envc]) {
            printk(KERN_DEBUG "env: %s\n", envp[envc]);
            envc++;
        }
    }
#endif
    /* 由于需要重载镜像数据，而传递的参数是在用户数据中，因此需要先保存
    参数，然后再重载镜像 */
    char *tmp_arg = kmalloc(PAGE_SIZE);
    if (tmp_arg == NULL) {
        printk(KERN_ERR "sys_exec_file: it is not a elf format file!\n", name);
        goto free_tmp_fd;
    }
    /* 备份参数和环境 */
    char **new_envp = NULL;
    unsigned long arg_bottom;
    proc_build_arg((unsigned long) tmp_arg + PAGE_SIZE, &arg_bottom, (char **) envp, &new_envp);
    char **new_argv = NULL;
    proc_build_arg(arg_bottom, NULL, (char **) argv, &new_argv);
    
    /* 备份进程名 */
    char tmp_name[MAX_TASK_NAMELEN] = {0};
    strcpy(tmp_name, name);

    /* 取消空间映射 */
    // vmm_unmap_space(cur->vmm);
    vmm_unmap_space_maparea(cur->vmm);
    /* 释放虚拟空间地址管理，后面映射时重新加载镜像 */
    vmm_release_space(cur->vmm);


    /* 加载镜像 */
    if (proc_load_image(cur->vmm, &elf_header, fd) < 0) {
        printk(KERN_ERR "sys_exec_file: load_image failed!\n");
        goto free_tmp_arg;
    }

    /* 构建中断栈框 */
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)cur + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    
    proc_make_trap_frame(cur);
    
    /* 初始化用户栈 */
    if(proc_stack_init(cur, frame, new_argv, new_envp) < 0){
        /* !!!需要取消已经加载镜像虚拟地址映射 */
        goto free_loaded_image;
    }
    kfree(tmp_arg); /* 不需要了，释放掉 */
    sys_close(fd);  /* 正常关闭文件 */

    /* 初始化用户堆 */
    proc_heap_init(cur);
    /* 初始化用户映射区域 */
    proc_map_space_init(cur);
    
    /* 初始化触发器 */
    trigger_init(cur->triggers);
    
    /* 如果是从一个多线程执行的，那么就会有线程结构体，由于在close_other_threads
    的时候，把thread_count置0，因此，在此需要重新初始化线程描述（将thread_count置1） */
    pthread_desc_init(cur->pthread);

    fs_fd_reinit(cur);

    /* 替换新镜像后前，都需要退出原来的图形消息池，“新”进程需要重新初始化 */
    gui_msgpool_exit(cur);

    /* 解除服务调用绑定 */
    sys_srvcall_unbind(-1);
#ifdef DEBUG_EXEC
    if (cur->pthread) {
        printk(KERN_DEBUG "%s: thread count %d\n", __func__, atomic_get(&cur->pthread->thread_count));
    }
#endif

    /* 执行程序的时候需要继承原有进程的资源，因此不在这里初始化资源 */
    /* 设置执行入口 */
    user_entry_point(frame, (unsigned long)elf_header.e_entry);

    /* 设置进程名 */
    memset(cur->name, 0, MAX_TASK_NAMELEN);
    strcpy(cur->name, tmp_name);
    
    // dump_trap_frame(frame);
    /* 切换到进程执行 */
    switch_to_user(frame);
    
    /* 不会继续往后执行 */
free_loaded_image:
    /* 释放已经加载的镜像，不过由于已经替换了新的镜像，回不去了，就直接exit吧。 */
    sys_exit(-1);
free_tmp_arg:
    kfree(tmp_arg);
free_tmp_fd:
    sys_close(fd);
    return -1;   
}


int sys_execve(const char *pathname, const char *argv[], const char *envp[])
{
    /* 检查参数 */
    if (pathname == NULL)
        return -1;
    
    /* 先查看pathname是否在根目录下 */
    char *p = (char *) pathname;
    char newpath[MAX_PATH];
    memset(newpath, 0, MAX_PATH);
    /* 是在根目录下面，就查看这个目录 */
    if (*p == '/') { 
        wash_path(p, newpath);
        if (!sys_access((const char *) newpath, F_OK)) {   /* file exist */
            /* 名字指向文件名 */
            char *name = strrchr(newpath, '/');
            if (name) {
                name++; /* 跳过'/' */
            } else {    /* 找到'/'，那就和路径一样 */
                name = (char *) newpath;
            }
            #ifdef DEBUG_EXEC
            printk(KERN_DEBUG "execute: full path: %s -> %s\n", newpath, name);
            #endif
            /* 尝试替换镜像 */
            if (do_execute((const char *) p, name, argv, envp) < 0) {
                /* 替换失败 */
                printk(KERN_ERR "%s: path %s not executable!", __func__, newpath);
                return -1;
            }
        }
    } else if ((*p == '.' && *(p+1) == '/') || (*p == '.' && *(p+1) == '.' && *(p+2) == '/')) {    /* 当前目录 */
        /* 构建路径 */
        build_path(p, newpath);
        #ifdef DEBUG_EXEC
        printk(KERN_DEBUG "execute: merged path: %s from %s\n", newpath, p);
        #endif

        if (!sys_access(newpath, F_OK)) {   /* file exist */
            char *pname = strrchr(newpath, '/');
            if (pname)
                pname++;
            else 
                pname =  newpath;
            /* 替换镜像 */
            if (do_execute((const char* )newpath, (char *)pname, argv, envp)) {
                /* 替换失败 */
                printk(KERN_ERR "%s: path %s not executable!", __func__, newpath);
                return -1;
            }
        }
    } else {    /* 不是在根目录下面，只有文件名，需要进行和环境变量组合 */
        if (envp) { /* 有环境才进行组合 */
            
            char **env = (char **) envp;
            char *q;

            while (*env) {
                
                q = *env;
                /* 构建路径 */
                strcpy(newpath, q);
                if (newpath[strlen(newpath) - 1] != '/') {
                    strcat(newpath, "/");
                }
                strcat(newpath, p);
                
                char finalpath[MAX_PATH] = {0};
                wash_path(newpath, finalpath);

                #ifdef DEBUG_EXEC
                printk(KERN_DEBUG "execute: merged path: %s from %s\n", finalpath, p);
                #endif
                /* 尝试执行 */
                
                if (!sys_access(finalpath, F_OK)) {   /* file exist */
                    char *pname = strrchr(finalpath, '/');
                    if (pname)
                        pname++;
                    else 
                        pname =  finalpath;
                    /* 替换镜像 */
                    do_execute((const char* )finalpath, (char *)pname, argv, envp);
                }
                env++;
                memset(newpath, 0, MAX_PATH);
            }
        }
    }
    printk(KERN_ERR "%s: path %s not exist or not executable!\n", __func__, pathname);
    return -1;  /* 失败 */
}
