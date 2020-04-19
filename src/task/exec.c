#include <xbook/process.h>
#include <xbook/string.h>
#include <sys/xfile.h>
#include <arch/interrupt.h>

/**
 * exec使用新的镜像以及堆栈替换原有的内容。
 * 注意，加载代码和数据的过程中，不会释放掉已经映射的内容，
 * 只是把虚拟内存空间结构释放掉。
 * 内存映射也是安全映射，即如果虚拟地址没有映射才映射，已经映射就不映射。
 * 只有在退出进程的时候才释放所有资源。这样也在一定程度上提高了效率，
 * 但是占用的内存变大，是空间换取时间的做法。
 */


/**
 * sys_exec_raw - 执行原始进程
 * 
 * @name: 名字
 * @argv: 参数
 * 
 * @return: 失败返回-1，成功不返回
 */
int sys_exec_raw(char *name, char **argv)
{
    /* 没有参数或者参数错误 */
    /*if (argv == NULL)
        return -1;*/
    task_t *cur = current_task;

    /* 对原始块名进行检测 */
    raw_block_t *rb = raw_block_get_by_name(name);
    if (rb == NULL) {
        printk(KERN_ERR "sys_exec_raw: get raw block failed!\n");
        return -1;
    }
    /* 读取文件头 */
    struct Elf32_Ehdr elf_header;
    memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));
    
    if (raw_block_read_off(rb, &elf_header, 0, sizeof(struct Elf32_Ehdr))) {
        printk(KERN_ERR "sys_exec_raw: read elf header failed!\n");
        return -1;
    }

    /* 检验elf头，看它是否为一个elf格式的程序 */
    if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) || \
        elf_header.e_type != 2 || \
        elf_header.e_machine != 3 || \
        elf_header.e_version != 1 || \
        elf_header.e_phnum > 1024 || \
        elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) {
        
        /* 头文件检测出错 */
        printk(KERN_ERR "sys_exec_raw: it is not a elf format file!\n", name);
        return -1;
    }
    /*
    int argc = 0;
    while (argv[argc]) {
        printk("arg: %s", argv[argc]);
        argc++;
    }*/
    /* 由于需要重载镜像数据，而传递的参数是在用户数据中，因此需要先保存
    参数，然后再重载镜像 */
    char *tmp_arg = kmalloc(PAGE_SIZE);
    if (tmp_arg == NULL) {
        printk(KERN_ERR "sys_exec_raw: it is not a elf format file!\n", name);
        return -1;
    }
    char **new_argv = NULL;
    proc_build_arg((unsigned long) tmp_arg + PAGE_SIZE, argv, &new_argv);

    /* 备份进程名 */
    char tmp_name[MAX_TASK_NAMELEN] = {0};
    strcpy(tmp_name, name);

    /* 释放虚拟空间地址管理，后面映射时重新加载镜像 */
    vmm_release_space(cur->vmm);
    /* 加载镜像 */
    if (proc_load_image(cur->vmm, &elf_header, rb)) {
        printk(KERN_ERR "sys_exec_raw: load_image failed!\n");
        goto free_tmp_arg;
    }
    /* 构建中断栈框 */
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)cur + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    
    proc_make_trap_frame(cur);

    /* 初始化用户栈 */
    if(proc_stack_init(cur, frame, new_argv)){
        /* !!!需要取消已经加载镜像虚拟地址映射 */
        goto free_loaded_image;
    }

    kfree(tmp_arg); /* 不需要了，释放掉 */

    /* 初始化用户堆 */
    proc_heap_init(cur);

    /* 初始化用户映射区域 */
    proc_map_space_init(cur);
    
    /* 初始化触发器 */
    trigger_init(cur->triggers);
    
    /* 执行程序的时候需要继承原有进程的资源，因此不在这里初始化资源 */

    /* 设置执行入口 */
    user_entry_point(frame, (unsigned long)elf_header.e_entry);
    
    /* 设置进程名 */
    memset(cur->name, 0, MAX_TASK_NAMELEN);
    strcpy(cur->name, tmp_name);

    dump_trap_frame(frame);
    /* 切换到进程执行 */
    switch_to_user(frame);
    /* 不会继续往后执行 */
free_loaded_image:
    /* 释放已经加载的镜像，不过由于已经替换了新的镜像，回不去了，就直接exit吧。 */
    sys_exit(-1);
free_tmp_arg:
    kfree(tmp_arg);
    return -1;
}

/**
 * sys_exec_file - 通过文件信息执行进程
 * 
 * @name: 进程的名字
 * @file: 进程的文件镜像信息
 * @argv: 参数
 * 
 * 通过给出的文件，解析加载镜像。
 * 
 * @return: 失败返回-1，成功不返回
 */
int sys_exec_file(char *name, xfile_t *file, char **argv)
{
    /* 没有参数或者参数错误 */
    task_t *cur = current_task;
    
    unsigned long flags;
    save_intr(flags);
    /* 根据文件信息创建临时原始块 */
    raw_block_t rb;
    if (raw_block_tmp_add(&rb, file->file, file->size)) {
        printk(KERN_ERR "sys_exec_file: raw_block_tmp_add failed!\n");
        restore_intr(flags);
        return -1;
    }
    restore_intr(flags);
    
    /* 读取文件头 */
    struct Elf32_Ehdr elf_header;
    memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));
    
    if (raw_block_read_off(&rb, &elf_header, 0, sizeof(struct Elf32_Ehdr))) {
        printk(KERN_ERR "sys_exec_file: read elf header failed!\n");
        goto free_tmp_rb;
    }

    /* 检验elf头，看它是否为一个elf格式的程序 */
    if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) || \
        elf_header.e_type != 2 || \
        elf_header.e_machine != 3 || \
        elf_header.e_version != 1 || \
        elf_header.e_phnum > 1024 || \
        elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) {
        
        /* 头文件检测出错 */
        printk(KERN_ERR "sys_exec_file: it is not a elf format file!\n", name);
        goto free_tmp_rb;
    }
    /*
    int argc = 0;
    while (argv[argc]) {
        printk("arg: %s", argv[argc]);
        argc++;
    }*/
    
    /* 由于需要重载镜像数据，而传递的参数是在用户数据中，因此需要先保存
    参数，然后再重载镜像 */
    char *tmp_arg = kmalloc(PAGE_SIZE);
    if (tmp_arg == NULL) {
        printk(KERN_ERR "sys_exec_file: it is not a elf format file!\n", name);
        goto free_tmp_rb;
    }
    char **new_argv = NULL;
    proc_build_arg((unsigned long) tmp_arg + PAGE_SIZE, argv, &new_argv);

    /* 备份进程名 */
    char tmp_name[MAX_TASK_NAMELEN] = {0};
    strcpy(tmp_name, name);
    
    /* 释放虚拟空间地址管理，后面映射时重新加载镜像 */
    vmm_release_space(cur->vmm);

    /* 加载镜像 */
    if (proc_load_image(cur->vmm, &elf_header, &rb)) {
        printk(KERN_ERR "sys_exec_file: load_image failed!\n");
        goto free_tmp_arg;
    }

    /* 构建中断栈框 */
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)cur + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    
    proc_make_trap_frame(cur);
    
    /* 初始化用户栈 */
    if(proc_stack_init(cur, frame, new_argv)){
        /* !!!需要取消已经加载镜像虚拟地址映射 */
        goto free_loaded_image;
    }
    kfree(tmp_arg); /* 不需要了，释放掉 */

    /* 初始化用户堆 */
    proc_heap_init(cur);
    /* 初始化用户映射区域 */
    proc_map_space_init(cur);
    
    /* 初始化触发器 */
    trigger_init(cur->triggers);
    
    /* 执行程序的时候需要继承原有进程的资源，因此不在这里初始化资源 */

    /* 设置执行入口 */
    user_entry_point(frame, (unsigned long)elf_header.e_entry);
    
    /* 设置进程名 */
    memset(cur->name, 0, MAX_TASK_NAMELEN);
    strcpy(cur->name, tmp_name);
    
    raw_block_tmp_del(&rb);  /* 删除临时原始块 */
    
    //dump_trap_frame(frame);
    /* 切换到进程执行 */
    switch_to_user(frame);
    
    /* 不会继续往后执行 */
free_loaded_image:
    /* 释放已经加载的镜像，不过由于已经替换了新的镜像，回不去了，就直接exit吧。 */
    sys_exit(-1);
free_tmp_arg:
    kfree(tmp_arg);
free_tmp_rb:
    raw_block_tmp_del(&rb);
    return -1;   
}
