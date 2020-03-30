#include <xbook/process.h>

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
        printk(KERN_ERR "process_execute: read elf header failed!\n");
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
        printk(KERN_ERR "process_execute: it is not a elf format file!\n", filename);
        return -1;
    }
    int argc = 0;
    /*while (argv[argc]) {
        printk("arg: %s", argv[argc]);
        argc++;
    }*/
    /* 先退出当前的虚拟内存空间 */
    vmm_release_space(cur->vmm);
    printk("ready load image\n");
    /* 加载镜像 */
    if (proc_load_image(cur->vmm, &elf_header, rb)) {
        printk(KERN_ERR "process_execute: load_image failed!\n");
        return -1;
    }
    printk("load image done!\n");
    
    /* 构建中断栈框 */
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)cur + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    
    /* 初始化用户栈 */
    if(proc_stack_init(cur, frame, argc, argv)){
        /* !!!需要取消已经加载镜像虚拟地址映射 */
        return -1;
    }

    /* 初始化用户堆 */
    proc_heap_init(cur);
    
    /* 设置执行入口 */
    user_entry_point(frame, (unsigned long)elf_header.e_entry);
    
    dump_trap_frame(frame);

    /* 切换到进程执行 */
    switch_to_user(frame);
    return 0;   /* 不会执行到这儿 */
}
