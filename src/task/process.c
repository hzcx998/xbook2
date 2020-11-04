#include <xbook/process.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>
#include <xbook/rawblock.h>
#include <xbook/elf32.h>
#include <string.h>
#include <math.h>
#include <xbook/memspace.h>
#include <string.h>
#include <xbook/resource.h>
#include <xbook/pthread.h>
#include <xbook/gui.h>
#include <xbook/schedule.h>
#include <arch/interrupt.h>
#include <arch/task.h>
#include <sys/pthread.h>
#include <fsal/fsal.h>
#include <gui/message.h>
#include <unistd.h>

// #define DEBUG_PROC

/**
 * load_segment - 加载段
 * @fd: 文件描述符
 * @offset: 
 * @file_sz: 段大小
 * @vaddr: 虚拟地址
 * 
 * 加载一个段到内存
 */
static int load_segment(int fd, unsigned long offset, unsigned long file_sz,
    unsigned long mem_sz, unsigned long vaddr)
{
    /*
    printk(PART_TIP "load_segment:fd %d off %x size %x mem %x vaddr %x\n",
        fd, offset, file_sz, mem_sz, vaddr);
    */
    /* 获取虚拟地址的页对齐地址 */
    unsigned long vaddr_page = vaddr & PAGE_MASK;

    /* 获取在第一个页中的大小 */
    unsigned long size_in_first_page = PAGE_SIZE - (vaddr & PAGE_LIMIT);

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
    int ret = (int)mem_space_mmap(vaddr_page, 0, occupy_pages * PAGE_SIZE, 
            PROT_USER | PROT_WRITE, MEM_SPACE_MAP_FIXED);
    if (ret < 0) {
        printk(KERN_ERR "load_segment: mem_space_mmap failed!\n");
        return -1;
    }
    // printk(KERN_DEBUG "task %d map space: addr %x end %x\n", current_task->pid, vaddr_page, vaddr_page + occupy_pages * PAGE_SIZE);
    /* 清空内存 */
    //memset((void *) vaddr_page, 0, occupy_pages * PAGE_SIZE);

    //printk(KERN_DEBUG "task %s space: addr %x page %d\n",(current_task)->name, vaddr_page, occupy_pages);

    // printk(KERN_DEBUG "[proc]: read file off %x size %x to vaddr %x\n", offset, file_sz, vaddr);

    /* 读取数据到内存中 */
    sys_lseek(fd, offset, SEEK_SET);
    if (sys_read(fd, (void *)vaddr, file_sz) != file_sz) {
        return -1;
    }

    return 0;
}

/**
 * proc_load_image - 加载文件镜像
 */
int proc_load_image(vmm_t *vmm, struct Elf32_Ehdr *elf_header, int fd)
{
    struct Elf32_Phdr prog_header;
    /* 获取程序头起始偏移 */
    Elf32_Off prog_header_off = elf_header->e_phoff;
    Elf32_Half prog_header_size = elf_header->e_phentsize;
    #ifdef DEBUG_PROC
    printk(KERN_DEBUG "prog offset %x size %d\n", prog_header_off, prog_header_size);
    #endif
    Elf32_Off prog_end;
    /* 遍历所有程序头 */
    unsigned long grog_idx = 0;
    while (grog_idx < elf_header->e_phnum) {
        memset(&prog_header, 0, prog_header_size);
        
        /* 读取程序头 */
        sys_lseek(fd, prog_header_off, SEEK_SET);
        if (sys_read(fd, (void *)&prog_header, prog_header_size) != prog_header_size) {
            return -1;
        }
#ifdef DEBUG_PROC
        printk(KERN_DEBUG "proc_load_image: read prog header off %x vaddr %x filesz %x memsz %x\n", 
            prog_header.p_offset, prog_header.p_vaddr, prog_header.p_filesz, prog_header.p_memsz);
#endif        
        /* 如果是可加载的段就加载到内存中 */
        if (prog_header.p_type == PT_LOAD) {
            

            /* 由于bss段不占用文件大小，但是要占用内存，
            所以这个地方我们加载的时候就加载成memsz，
            运行的时候访问未初始化的全局变量时才可以正确
            filesz用于读取磁盘上的数据，而memsz用于内存中的扩展，
            因此filesz<=memsz
             */
            if (load_segment(fd, prog_header.p_offset, 
                    prog_header.p_filesz, prog_header.p_memsz, prog_header.p_vaddr)) {
                return -1;
            }
            
            /* 如果内存占用大于文件占用，就要把内存中多余的部分置0 */
            if (prog_header.p_memsz > prog_header.p_filesz) {
                #ifdef DEBUG_PROC
                printk("[proc] clean bss at %x size(%x)\n", 
                    prog_header.p_vaddr + prog_header.p_filesz, prog_header.p_memsz - prog_header.p_filesz);
                printk("[proc] memsz(%x) > filesz(%x)\n", 
                    prog_header.p_memsz, prog_header.p_filesz);
                #endif
                memset((void *)(prog_header.p_vaddr + prog_header.p_filesz), 0,
                    prog_header.p_memsz - prog_header.p_filesz);
                
            }
            prog_end = prog_header.p_vaddr + prog_header.p_memsz;
#ifdef DEBUG_PROC            
            printk(KERN_DEBUG "seg start:%x end:%x\n", prog_header.p_vaddr, prog_end);
#endif        

            
            /* 设置段的起始和结束 */
            if (prog_header.p_flags == ELF32_PHDR_CODE) {
                vmm->code_start = prog_header.p_vaddr;
                vmm->code_end = prog_end;
#ifdef DEBUG_PROC  
                printk(KERN_DEBUG "code start:%x end:%x\n", vmm->code_start, vmm->code_end);
#endif
                /*堆默认在代码的后面的一个页后面 */
                vmm->heap_start = vmm->code_end + PAGE_SIZE;
                
                /* 页对齐 */
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            } else if (prog_header.p_flags == ELF32_PHDR_DATA) {
                vmm->data_start = prog_header.p_vaddr;
                vmm->data_end = prog_end;
#ifdef DEBUG_PROC                  
                printk(KERN_DEBUG "data start:%x end:%x\n", vmm->data_start, vmm->data_end);
#endif
                /*堆默认在数据的后面的一个页后面 */
                vmm->heap_start = vmm->data_end + PAGE_SIZE;
                
                /* 页对齐 */
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            }

            /* 没有获取到数据段和代码段，就直接放到加载的最后 */
            if (!vmm->heap_start && !vmm->heap_end) {
                /*堆默认在数据的后面的一个页后面 */
                vmm->heap_start = prog_end + PAGE_SIZE;
                
                /* 页对齐 */
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            }
        }

        /* 指向下一个程序头 */
        prog_header_off += prog_header_size;
        grog_idx++;
    }
#ifdef DEBUG_PROC 
    printk(KERN_DEBUG "heap start:%x\n", vmm->heap_start);
#endif
    return 0;
}
/**
 * 构建一个参数栈，把参数放到一个固定的区域中，可以通过argv来索引到。
 * @arg_top: 要存放到的参数栈顶位置
 * @arg_bottom: 参数栈已经移动到的最低位置
 * @argv: 要解析的参数
 * @dest_argv: 存放到的参数位置
 * 
 * 返回参数个数
 */
int proc_build_arg(unsigned long arg_top, unsigned long *arg_bottom, char *argv[], char **dest_argv[])
{
    int argc = 0;
    /* 填写参数到参数区域 */
    unsigned long arg_pos = arg_top;
    
    if (argv != NULL) {
        while (argv[argc]) {
            argc++;
        }
        /* 构建参数，用栈的方式传递参数，不用寄存器保存参数，再传递给main */
        if (argc != 0) {
            int i;
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
            
            if (arg_bottom) { /* 传回参数栈最底部位置 */
                /* 存放参数栈底部再下降unsigned long 大小个位置，避免覆盖 */
                *arg_bottom = (unsigned long) (arg_pos - sizeof(unsigned long));
            }

            /* 在下面把参数写入到栈中，可以直接在main函数中使用 */
            unsigned long top = arg_pos;
            /* 从下往上填写 */
            // argc
            *(unsigned long *)top = argc;
            top += sizeof(unsigned long);

            // argv
            *(unsigned long *)top = top + sizeof(char **);
            *dest_argv = (char **)(top + sizeof(char **));
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
            return argc;
        }
    }

    // 预留argv[]，只留一个，拿来储存指向参数0（NULL）的指针
    arg_pos -= 1 * sizeof(char*);
    // 预留argv
    arg_pos -= sizeof(char**);
    // 预留argc
    arg_pos -= sizeof(unsigned long);

    if (arg_bottom) {   /* 传回参数栈最底部位置 */
        /* 存放参数栈底部再下降unsigned long 大小个位置，避免覆盖 */
        *arg_bottom = (unsigned long) (arg_pos - sizeof(unsigned long));
    }

    /* 在下面把参数写入到栈中，可以直接在main函数中使用 */
    
    unsigned long top = arg_pos;
    
    // argc
    *(unsigned long *)top = 0;
    top += sizeof(unsigned long);

    // argv
    *(unsigned long *)top = top + sizeof(char **);
    *dest_argv = (char **)(top + sizeof(char **));
    top += sizeof(char **);

    // 指向指针数组
    // argv[]
    char** _argv = (char **) top;
    _argv[0] = NULL; /* 第一个参数就是空 */
    return argc;
}

/**
 * proc_heap_init - 用户堆的初始化
 * 默认是没有堆空间的，只有堆的信息记录，需要在使用中拓展
 */
void proc_heap_init(task_t *task)
{
    return;
#ifdef DEBUG_PROC
    printk(KERN_DEBUG "task=%d dump memspace.\n", task->pid);
    mem_space_dump(task->vmm);

    printk(KERN_DEBUG "data segment end=%x end2=%d.\n", 
        task->vmm->data_end, PAGE_ALIGN(task->vmm->data_end));
#endif    
    /* heap默认在数据的后面的一个页后面 */
    task->vmm->heap_start = task->vmm->data_end + PAGE_SIZE;
    
    /* 页对齐 */
    task->vmm->heap_start = PAGE_ALIGN(task->vmm->heap_start);
    task->vmm->heap_end = task->vmm->heap_start;
}

/**
 * proc_map_space_init - 用户映射空间的初始化
 * 
 * 默认没有任何映射，不过得先给映射一个起始地址
 */
void proc_map_space_init(task_t *task)
{
    /* map默认在堆的末尾+10个页的位置 */
    //task->vmm->map_start = task->vmm->heap_start + MAX_MEM_SPACE_HEAP_SIZE + PAGE_SIZE * 10;
    task->vmm->map_start = (unsigned long) MEM_SPACE_MAP_ADDR_START;
    
    task->vmm->map_end = task->vmm->map_start + MAX_MEM_SPACE_MAP_SIZE;
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

int proc_vmm_exit(task_t *task)
{
    if (task->vmm == NULL)
        return -1;
    vmm_exit(task->vmm);
    return 0;
}

int proc_trigger_init(task_t *task)
{
    task->triggers = mem_alloc(sizeof(triggers_t));
    if (task->triggers == NULL)
        return -1;
    trigger_init(task->triggers);
    return 0;
}

int proc_trigger_exit(task_t *task)
{
    if (task->triggers == NULL)
        return -1;
    mem_free(task->triggers);
    task->triggers = NULL;
    return 0;
}

int proc_pthread_init(task_t *task)
{
    task->pthread = mem_alloc(sizeof(pthread_desc_t));
    if (task->pthread == NULL)
        return -1;
    pthread_desc_init(task->pthread);
    return 0;
}

int proc_pthread_exit(task_t *task)
{
    pthread_desc_exit(task->pthread);
    task->pthread = NULL;
    return 0;
}

int proc_release(task_t *task)
{
    if (proc_vmm_exit(task))
        return -1;
    if (proc_trigger_exit(task))
        return -1;
    if (fs_fd_exit(task))
        return -1;
    if (gui_user_exit(task))
        return -1;
    if (proc_pthread_exit(task))
        return -1;
    
    if (thread_release_resource(task))
        return -1;
    printk(KERN_DEBUG "[proc]: release don.\n");
    return 0;
}

/**
 * proc_destroy - 进程销毁
 * @task: 任务
 * @thread: 是否为线程：1是，0不是。
 * 
 * 不是线程才会释放页目录表，vmm等资源。
 */
int proc_destroy(task_t *task, int thread)
{
    if (task->vmm == NULL)
        return -1;
    if (!thread) {
        page_free_one(kern_vir_addr2phy_addr(task->vmm->page_storage));
        vmm_free(task->vmm);
        task->vmm = NULL;    
    }
    task_free(task);    /* 子线程只释放结构体 */
    return 0;
}

/**
 * proc_make_trap_frame - 创建一个线程
 * @task: 线程结构体
 * @function: 要去执行的函数
 * @arg: 参数
 */
void proc_make_trap_frame(task_t *task)
{
    /* 预留中断栈 */
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)task + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    /* 默认内核线程使用内核段 */
    user_frame_init(frame);
}

/* 构建用户进程初始上下文信息 */
void proc_entry(void* arg)
{
    const char **argv = (const char **) arg;
    sys_execve(argv[0], argv, NULL);
    panic("[task]: start process %s failed!\n", argv[0]);
}

/**
 * 
 * start_process - 创建一个进程
 * @name: 进程名字
 * @arg: 线程参数
 */
task_t *start_process(char *name, char **argv)
{
    // 创建一个新的线程结构体
    task_t *task = (task_t *) mem_alloc(TASK_KSTACK_SIZE);
    
    if (!task)
        return NULL;
    // 初始化线程
    task_init(task, name, TASK_PRIO_USER);
    task->pid = INIT_PROC_PID;  /* 修改pid为INIT进程pid */
    task->tgid = task->pid;     /* 主线程，和pid一样 */

    if (proc_vmm_init(task)) {
        mem_free(task);
        return NULL;
    }
    
    if (proc_trigger_init(task)) {
        proc_vmm_exit(task);
        mem_free(task);
        return NULL;
    }

    /* 创建文件描述表 */
    if (fs_fd_init(task) < 0) {
        proc_trigger_exit(task);
        proc_vmm_exit(task);
        mem_free(task);
        return NULL;
    }

    /* 创建进程栈 */
    task_stack_build(task, proc_entry, argv);
    //current_task = task;    /* 指向当前任务 */
    /*
    int argc = 0;
    char *p;
    while (argv[argc]) {
        p = argv[argc];
        printk("%s ", p);
        argc++;
    }*/
    //process_setup(task, name, argv);
    unsigned long flags;
    interrupt_save_state(flags);
    task_global_list_add(task);
    task_priority_queue_add_tail(sched_get_unit(), task);
    interrupt_restore_state(flags);
    
    return task;
}
