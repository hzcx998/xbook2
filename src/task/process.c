#include <xbook/process.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>
#include <xbook/rawblock.h>
#include <xbook/elf32.h>
#include <xbook/memops.h>
#include <xbook/math.h>
#include <xbook/vmspace.h>
#include <xbook/string.h>
#include <arch/interrupt.h>

/**
 * load_data_from - 从文件读取数据
 * @rb: 文件描述符
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

/**
 * load_segment - 加载段
 * @rb: 文件描述符
 * @offset: 
 * @file_sz: 段大小
 * @vaddr: 虚拟地址
 * 
 * 加载一个段到内存
 */
static int load_segment(raw_block_t *rb, unsigned long offset, unsigned long file_sz,
    unsigned long mem_sz, unsigned long vaddr)
{
    /*
    printk(PART_TIP "load_segment:rb %d off %x size %x mem %x vaddr %x\n",
        rb, offset, file_sz, mem_sz, vaddr);
    */
    /* 获取虚拟地址的页对齐地址 */
    unsigned long vaddr_page = vaddr & PAGE_ADDR_MASK;

    /* 获取在第一个页中的大小 */
    unsigned long size_in_first_page = PAGE_SIZE - (vaddr & PAGE_INSIDE);

    /*  段要占用多少个页 */
    unsigned long occupy_pages = 0;

    /* 计算最终占用多少个页 */
    if (mem_sz > size_in_first_page) {
        /* 计算文件大小比第一个页中的大小多多少，也就是还剩下多少字节 */
        unsigned long left_size = mem_sz - size_in_first_page;

        /* 整除后向上取商，然后加上1（1是first page） */
        occupy_pages = DIV_ROUND_UP(left_size, PAGE_SIZE) + 1;
    } else {
        occupy_pages = 1;
    }
    
    /* 映射虚拟空间 */  
    int ret = (int)vmspace_mmap(vaddr_page, occupy_pages * PAGE_SIZE, 
            PROT_USER | PROT_WRITE, VMS_MAP_FIXED);
    if (ret < 0) {
        printk(KERN_ERR "load_segment: SysMmap failed!\n");
        return -1;
    }
    printk(KERN_DEBUG "task %s space: addr %x page %d\n",(current_task)->name, vaddr_page, occupy_pages);
    
    /* 读取数据到内存中 */
    if (load_data_from(rb, (void *)vaddr, offset, file_sz)) {
        return -1;
    }
    return 0;
}

/**
 * load_image - 加载文件镜像
 */
static int load_image(vmm_t *vmm, struct Elf32_Ehdr *elf_header, raw_block_t *rb)
{
    struct Elf32_Phdr prog_header;
    /* 获取程序头起始偏移 */
    Elf32_Off prog_header_off = elf_header->e_phoff;
    Elf32_Half prog_header_size = elf_header->e_phentsize;
    
    //printk(KERN_DEBUG "prog offset %x size %d\n", prog_header_off, prog_header_size);
    
    /* 遍历所有程序头 */
    unsigned long grog_idx = 0;
    while (grog_idx < elf_header->e_phnum) {
        memset(&prog_header, 0, prog_header_size);

        /* 读取程序头 */
        if (load_data_from(rb, (void *)&prog_header, prog_header_off, prog_header_size)) {
            return -1;
        }/*
        printk(KERN_DEBUG "read prog header off %x vaddr %x filesz %x memsz %x\n", 
            prog_header.p_offset, prog_header.p_vaddr, prog_header.p_filesz, prog_header.p_memsz);
        */
        /* 如果是可加载的段就加载到内存中 */
        if (prog_header.p_type == PT_LOAD) {

            /* 由于bss段不占用文件大小，但是要占用内存，
            所以这个地方我们加载的时候就加载成memsz，
            运行的时候访问未初始化的全局变量时才可以正确
            filesz用于读取磁盘上的数据，而memsz用于内存中的扩展，
            因此filesz<=memsz
             */
            if (load_segment(rb, prog_header.p_offset, 
                    prog_header.p_filesz, prog_header.p_memsz, prog_header.p_vaddr)) {
                return -1;
            }
            
            /* 如果内存占用大于文件占用，就要把内存中多余的部分置0 */
            if (prog_header.p_memsz > prog_header.p_filesz) {
                memset((void *)(prog_header.p_vaddr + prog_header.p_filesz), 0,
                    prog_header.p_memsz - prog_header.p_filesz);
                /*printk("memsz(%x) > filesz(%x)\n", 
                    prog_header.p_memsz, prog_header.p_filesz);
                */
            }
            
            /* 设置段的起始和结束 */
            if (prog_header.p_flags == ELF32_PHDR_CODE) {
                vmm->code_start = prog_header.p_vaddr;
                vmm->code_end = prog_header.p_vaddr + prog_header.p_memsz;
                //printk(KERN_DEBUG "code start:%x end:%x\n", vmm->code_start, vmm->code_end);

            } else if (prog_header.p_flags == ELF32_PHDR_DATA) {
                vmm->data_start = prog_header.p_vaddr;
                vmm->data_end = prog_header.p_vaddr + prog_header.p_memsz;
                //printk(KERN_DEBUG "data start:%x end:%x\n", vmm->data_start, vmm->data_end);
            }
            //printk(KERN_DEBUG "seg start:%x end:%x\n", prog_header.p_vaddr, prog_header.p_vaddr + prog_header.p_memsz);
        }

        /* 指向下一个程序头 */
        prog_header_off += prog_header_size;
        grog_idx++;
    }
    return 0;
}

/**
 * user_stack_arg_init - 初始化用户栈和参数
 * 
 */
static int user_stack_arg_init(task_t *task, trap_frame_t *frame, 
        int argc, char **argv)
{
    
    vmm_t *vmm = task->vmm;
    /* 记录栈和参数信息到vmm里面 */
    vmm->arg_end = USER_STACK_TOP; /* 栈占用1个页 */
    vmm->arg_start = vmm->arg_end - PAGE_SIZE;
    
    vmm->stack_end = vmm->arg_start;
    vmm->stack_start = vmm->stack_end - PAGE_SIZE * 1; /* 参数占用4个页 */

    /* 固定位置 */
    if (vmspace_mmap(vmm->arg_start, vmm->arg_end - vmm->arg_start, PROT_USER | PROT_WRITE,
        VMS_MAP_FIXED) == ((void *)-1)) {
        return -1;
    }

    /* 固定位置，初始化时需要一个固定位置，向下拓展时才动态。 */
    if (vmspace_mmap(vmm->stack_start, vmm->stack_end - vmm->stack_start , PROT_USER | PROT_WRITE,
        VMS_MAP_FIXED | VMS_MAP_STACK) == ((void *)-1)) {
        return -1;
    }

    unsigned long arg_start; 

    /* 填写参数到参数区域种 */
    unsigned long arg_pos = vmm->arg_end;
    int i;
    /* 构建参数，用栈的方式传递参数，不用寄存器保存参数，再传递给main */
    if (argc != 0) {
        // 预留字符串的空间
        for (i = 0; i < argc; i++) {
            arg_pos -= (strlen(argv[i]) + 1);
        }        
        // 对齐
        arg_pos -= arg_pos % sizeof(unsigned long);

        // 预留argv[]，多留一个，拿来储存指向参数0（NULL）的指针
        arg_pos -= (argc + 1) * sizeof(char*);
        // 预留argv
        arg_pos -= sizeof(char**);
        // 预留argc
        arg_pos -= sizeof(unsigned long);
        /* 开始地址为参数存放的地址 */
        arg_start = arg_pos;

        /* 在下面把参数写入到栈中，可以直接在main函数中使用 */
        unsigned long top = arg_pos;
        /* 从下往上填写 */
        // argc
        *(unsigned long *)top = argc;
        top += sizeof(unsigned long);

        // argv
        *(unsigned long *)top = top + sizeof(char **);
        top += sizeof(char **);

        // 指向指针数组

        // argv[]
        char** _argv = (char **) top;

        // 指向参数值
        char* p = (char *) top + sizeof(char *) * (argc + 1);

        /* 把每一个参数值的首地址给指针数组 */
        for (i = 0; i < argc; i++) {
            /* 首地址 */
            _argv[i] = p;
            /* 复制参数值字符串 */
            strcpy(p, argv[i]);
            /* 指向下一个字符串 */
            p += (strlen(p) + 1);
        }
        _argv[i] = NULL;    /*  在最后加一个空参数 */
    } else {
        /* 把参数设置为空 */

        // 预留argv[]，只留一个，拿来储存指向参数0（NULL）的指针
        arg_pos -= 1 * sizeof(char*);
        // 预留argv
        arg_pos -= sizeof(char**);
        // 预留argc
        arg_pos -= sizeof(unsigned long);

        arg_start = arg_pos;
        
        /* 在下面把参数写入到栈中，可以直接在main函数中使用 */
        
        unsigned long top = arg_pos;
        
        // argc
        *(unsigned long *)top = 0;
        top += sizeof(unsigned long);

        // argv
        *(unsigned long *)top = top + sizeof(char **);
        top += sizeof(char **);

        // 指向指针数组
        // argv[]
        char** _argv = (char **) top;
        _argv[0] = NULL; /* 第一个参数就是空 */
    }
    /* 记录栈顶 */
    frame->esp = (unsigned long)vmm->stack_end;
    /* 记录参数个数 */
    frame->ecx = *(unsigned long *)arg_start;
    /* 记录参数个数 */
    frame->ebx = *(unsigned long *)(arg_start + sizeof(unsigned long));
    
    printk(KERN_DEBUG "task %s arg space: start %x end %x\n",
        (current_task)->name, vmm->arg_start, vmm->arg_end);
    printk(KERN_DEBUG "task %s stack space: start %x end %x\n",
        (current_task)->name, vmm->stack_start, vmm->stack_end);
    
    printk(KERN_DEBUG "stack %x argc %d argv %x\n", frame->esp, frame->ecx, frame->ebx);
#if 0 /* print args */
    char **new_argv = (char **)frame->ebx;
    i = 0;
    while (new_argv[i]) {
        printk("[%x %x]", new_argv[i], new_argv[i][0]);
        
        printk("%s ", new_argv[i]);
        i++;
    }
#endif
    return 0;
}

/**
 * user_heap_init - 用户堆的初始化
 * 默认是没有堆空间的，只有堆的信息记录，需要在使用中拓展
 */
static void user_heap_init(task_t *cur)
{
    /* brk默认在数据的后面 */
    cur->vmm->heap_start = cur->vmm->data_end;
    
    /* 页对齐 */
    cur->vmm->heap_start = PAGE_ALIGN(cur->vmm->heap_start);
    cur->vmm->heap_end = cur->vmm->heap_start;
}

void process_execute(task_t *cur, int argc, char **argv)
{
    /* 没有参数或者参数错误 */
    if (argc < 1 || argv == NULL)
        return;
    /* 对文件信息进行检测 */
    char *name = argv[0];
    raw_block_t *rb = raw_block_get_by_name(name);
    if (rb == NULL) {
        printk(KERN_ERR "get raw block failed!\n");
        return;
    }
    /* 读取文件头 */
    struct Elf32_Ehdr elf_header;
    memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));
    
    if (load_data_from(rb, &elf_header, 0, sizeof(struct Elf32_Ehdr))) {
        printk(KERN_ERR "process_execute: read elf header failed!\n");
        return;
    }

    /* 检验elf头，看它是否为一个elf格式的程序 */
    if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) || \
        elf_header.e_type != 2 || \
        elf_header.e_machine != 3 || \
        elf_header.e_version != 1 || \
        elf_header.e_phnum > 1024 || \
        elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) {
        
        /* 头文件检测出错 */
        printk(KERN_ERR "process_execute: it is not a elf format file!\n", name);
        return;
    }
    
    /* 切换到进程页空间 */
    write_cr3(v2p(cur->vmm->page_storage));

    /* 加载镜像 */
    if (load_image(cur->vmm, &elf_header, rb)) {
        printk(KERN_ERR "process_execute: load_image failed!\n");
        return;
    }

    /* 构建中断栈框 */
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)cur + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    
    /* 初始化用户栈 */
    if(user_stack_arg_init(cur, frame, argc, argv)){
        /* !!!需要取消已经加载镜像虚拟地址映射 */
        return;
    }
    /* 切换回内核页空间 */
    write_cr3(PAGE_DIR_PHY_ADDR);

    /* 初始化用户堆 */
    user_heap_init(cur);
    
    /* 设置执行入口 */
    user_entry_point(frame, (unsigned long)elf_header.e_entry);
    
    //printk(KERN_DEBUG "switch to user!\n");
    /* 切换到进程执行 */
    dump_trap_frame(frame);
}

int proc_vmm_init(task_t *task)
{
    /* 进程才需要虚拟内存单元 */
    task->vmm = vmm_alloc();
    if (task->vmm == NULL) {
        return -1;
    }
    vmm_init(task->vmm);
    return 0;
}


/**
 * make_proc_stack - 创建一个线程
 * @task: 线程结构体
 * @function: 要去执行的函数
 * @arg: 参数
 */
void make_proc_stack(task_t *task)
{
    /* 预留中断栈 */
    task->kstack -= sizeof(trap_frame_t);
    trap_frame_t *frame = (trap_frame_t *) task->kstack;
    /* 默认内核线程使用内核段 */
    user_trap_frame_init(frame);
}

/**
 * 
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
    printk(KERN_DEBUG "new task at %x\n", task);
    // 初始化线程
    task_init(task, name, TASK_PRIO_USER);
    
    if (proc_vmm_init(task)) {
        kfree(task);
        return NULL;
    }
    /* 创建进程栈 */
    make_proc_stack(task);
    current_task = task;    /* 指向当前任务 */

    int argc = 0;
    char *p;
    while (argv[argc]) {
        p = argv[argc];
        printk("%s ", p);
        argc++;
    }
    process_execute(task, argc, argv);

    task_global_list_add(task);
    task_priority_queue_add_tail(task);

    return task;
}
