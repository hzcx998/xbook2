#include <xbook/process.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>
#include <xbook/rawblock.h>
#include <xbook/elf32.h>
#include <xbook/memops.h>
#include <arch/interrupt.h>

/**
 * load_data_from - 从文件读取数据
 * @fd: 文件描述符
 * @buffer: 读取到的buffer
 * @offset: 偏移
 * @size: 要读取的数据数量
 */
static int load_data_from(raw_block_t *rb, void *buffer, unsigned long offset, unsigned long size)
{
    raw_block_seek(rb, offset, RB_SEEK_SET);
    if (!raw_block_read(rb, buffer, size)) {
        printk(KERN_ERR "load_data_from: read %d failed!\n", size);
        return -1;
    }
    return 0;
}

void process_execute(int argc, char **argv)
{
    /* 没有参数或者参数错误 */
    if (argc < 1 || argv == NULL)
        return;
    /* 对文件信息进行检测 */
    char *name = argv[0];
    raw_block_t *rb = raw_block_get_by_name(name);
    if (rb == NULL) {
        printk("get raw block failed!\n");
        return;
    }
    /* 读取文件头 */
    struct Elf32_Ehdr elfHeader;
    memset(&elfHeader, 0, sizeof(struct Elf32_Ehdr));
    
    if (load_data_from(rb, &elfHeader, 0, sizeof(struct Elf32_Ehdr))) {
        printk(KERN_ERR "process_execute: read elf header failed!\n");
        return;
    }
    unsigned char *p = (unsigned char *)&elfHeader;
    /*int i;
    for (i = 0; i < sizeof(struct Elf32_Ehdr); i++) {
        printk("%x ", p[i]);
    }
    */

    /* 检验elf头，看它是否为一个elf格式的程序 */
    if (memcmp(elfHeader.e_ident, "\177ELF\1\1\1", 7) || \
        elfHeader.e_type != 2 || \
        elfHeader.e_machine != 3 || \
        elfHeader.e_version != 1 || \
        elfHeader.e_phnum > 1024 || \
        elfHeader.e_phentsize != sizeof(struct Elf32_Phdr)) {
        
        /* 头文件检测出错 */
        printk(KERN_ERR "process_execute: it is not a elf format file!\n", name);
        return;
    }
    
    /* 加载镜像 */

    /* 构建中断栈框 */

    /* 初始化用户栈 */

    /* 初始化用户堆 */

    /* 修改进程运行寄存器 */

    /* 修改进程相关的task成员 */

    /* 进行善后处理 */    

    /* 切换到进程执行 */

}

/**
 * process_start - 进程开始执行的地方
 * @arg: 进程的参数
 * 
 * 当进程开始运行的时候，就会先执行该函数，然后在该函数里面
 * 该函数运行在[内核态]，是进入用户态的一个重要通道
 */
void process_start(void *arg)
{
    printk("process start!\n");
    
    /* 参数转换 */
    char **argv = (char **)arg;
    
    int i = 0;
    char *p;

    while (argv[i]) {
        p = argv[i];

        printk("%s ", p);
        
        i++;
    }
    /* 加载镜像到进程空间，构建堆栈，然后跳转到程序入口执行进程空间代码 */
    process_execute(i, argv);
    
    while (1)
    {
        /* code */
    }
    
    /* 如果执行失败，那么就退出线程运行 */
    kthread_exit(current_task);
}

/**
 * process_create - 创建一个进程
 * @name: 进程名字
 * @arg: 线程参数
 */
task_t *process_create(char *name, char **argv)
{
    // 创建一个新的线程结构体
    task_t *task = (task_t *) kmalloc(TASK_KSTACK_SIZE);
    
    if (!task)
        return NULL;
    
    // 初始化线程
    task_init(task, name, TASK_PRIO_USER);
    
    /* 进程才需要虚拟内存单元 */
    task->vmm = vmm_alloc();
    if (task->vmm == NULL) {
        kfree(task);
        return NULL;
    }
    vmm_init(task->vmm);
    
    /* 把参数进行额外的保存 */

    // 创建一个线程
    make_task_stack(task, process_start, (void *)argv);

    /* 操作链表时关闭中断，结束后恢复之前状态 */
    unsigned long flags;
    save_intr(flags);

    task_global_list_add(task);
    task_priority_queue_add_tail(task);
    
    restore_intr(flags);
    printk("create done!\n");
    return task;
}
