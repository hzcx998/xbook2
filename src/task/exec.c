#include <xbook/process.h>

/**
 * 使用新的镜像以及堆栈替换原有的内容。
 * 注意，加载代码和数据的过程中，不会释放掉已经映射的内容，
 * 只是把虚拟内存空间结构释放掉。
 * 内存映射也是安全映射，即如果虚拟地址没有映射才映射，已经映射就不映射。
 * 只有在退出进程的时候才释放所有资源。这样也在一定程度上提高了效率，
 * 但是占用的内存变大，是空间换取时间的做法。
 */
int proc_exec(char *name, char **argv)
{
    /* 没有参数或者参数错误 */
    if (argv == NULL)
        return -1;
    task_t *cur = current_task;

    /* 对原始块名进行检测 */
    char *filename = name;
    raw_block_t *rb = raw_block_get_by_name(filename);
    if (rb == NULL) {
        printk(KERN_ERR "get raw block failed!\n");
        return -1;
    }
    /* 读取文件头 */
    struct Elf32_Ehdr elf_header;
    memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));
    
    if (raw_block_read_off(rb, &elf_header, 0, sizeof(struct Elf32_Ehdr))) {
        printk(KERN_ERR "proc_exec: read elf header failed!\n");
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
        printk(KERN_ERR "proc_exec: it is not a elf format file!\n", filename);
        return -1;
    }
    /*int argc = 0;
    while (argv[argc]) {
        printk("arg: %s", argv[argc]);
        argc++;
    }*/
    /* 由于需要重载镜像数据，而传递的参数是在用户数据中，因此需要先保存
    参数，然后再重载镜像 */
    char *tmp_arg = kmalloc(PAGE_SIZE);
    if (tmp_arg == NULL) {
        printk(KERN_ERR "proc_exec: it is not a elf format file!\n", filename);
        return -1;
    }
    char **new_argv = NULL;
    proc_build_arg((unsigned long) tmp_arg + PAGE_SIZE, argv, &new_argv);

    /* 释放虚拟空间地址管理，后面映射时重新加载镜像 */
    vmm_release_space(cur->vmm);
    /* 加载镜像 */
    if (proc_load_image(cur->vmm, &elf_header, rb)) {
        printk(KERN_ERR "proc_exec: load_image failed!\n");
        return -1;
    }
    /* 构建中断栈框 */
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)cur + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    
    /* 初始化用户栈 */
    if(proc_stack_init(cur, frame, new_argv)){
        /* !!!需要取消已经加载镜像虚拟地址映射 */
        return -1;
    }

    kfree(tmp_arg); /* 不需要了，释放掉 */

    /* 初始化用户堆 */
    proc_heap_init(cur);
    
    /* 设置执行入口 */
    user_entry_point(frame, (unsigned long)elf_header.e_entry);
    
    //dump_trap_frame(frame);

    printk("ready switch!\n");
    /* 切换到进程执行 */
    switch_to_user(frame);
    return 0;   /* 不会执行到这儿 */
}
