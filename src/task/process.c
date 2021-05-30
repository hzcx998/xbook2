#include <xbook/process.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>
#include <xbook/elf.h>
#include <string.h>
#include <math.h>
#include <xbook/memspace.h>
#include <xbook/vmm.h>
#include <string.h>
#include <xbook/schedule.h>
#include <arch/interrupt.h>
#include <arch/task.h>
#include <arch/page.h>
#include <xbook/safety.h>
#include <xbook/fd.h>
#include <xbook/fs.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#ifdef CONFIG_PTHREAD
#include <xbook/pthread.h>
#include <sys/pthread.h>
#endif

#define DEBUG_PROCESS 0

static int proc_load_segment(int fd, unsigned long offset, unsigned long file_sz,
    unsigned long mem_sz, unsigned long vaddr)
{
    unsigned long vaddr_page = vaddr & PAGE_MASK;
    unsigned long size_in_first_page = PAGE_SIZE - (vaddr & PAGE_LIMIT);
    unsigned long occupy_pages = 0;
    if (mem_sz > size_in_first_page) {
        unsigned long left_size = mem_sz - size_in_first_page;
        occupy_pages = DIV_ROUND_UP(left_size, PAGE_SIZE) + 1;
    } else {
        occupy_pages = 1;
    }
    // dbgprintln("[proc] memmap vaddr=%p pages=%d", vaddr_page, occupy_pages);
    void *retaddr = mem_space_mmap(vaddr_page, 0, occupy_pages * PAGE_SIZE, 
            PROT_EXEC | PROT_WRITE | PROT_READ, MEM_SPACE_MAP_FIXED);
    if (retaddr == ((void *)-1)) {
        keprint(PRINT_ERR "proc_load_segment: mem_space_mmap failed!\n");
        return -1;
    }
    kfile_lseek(fd, offset, SEEK_SET);
    
    /* read file */
    memset((void *)vaddr_page, 0, occupy_pages * PAGE_SIZE);

    if (kfile_read(fd, (void *)vaddr, file_sz) != file_sz) {
        keprint(PRINT_ERR "proc_load_segment: read file failed!\n");
        return -1;
    }
    return 0;
}

// Load a program segment into pagetable at virtual address va.
// va must be page-aligned
// and the pages from va to va+sz must already be mapped.
// Returns 0 on success, -1 on failure.
static int
loadseg(void *pgdir, uint64_t va, int fd, unsigned long offset, size_t sz)
{
    uint32_t i, n;
    uint64_t *pa;

    uint64_t _va = va & PAGE_MASK;
    if((_va % PAGE_SIZE) != 0)
        panic("loadseg: va %p must be page aligned", _va);

    for(i = 0; i < sz; i += PAGE_SIZE){
        pa = user_walk_addr((pgdir_t)pgdir, _va + i);
        if(pa == NULL)
        panic("loadseg: address should exist");
        if(sz - i < PAGE_SIZE)
            n = sz - i;
        else
            n = PAGE_SIZE;
        
        kfile_lseek(fd, offset+i, SEEK_SET);
        if (kfile_read(fd, (void *)pa, n) != n) {
            keprint(PRINT_ERR "loadseg: read file failed!\n");
            return -1;
        }
    }

  return 0;
}


static int proc_load_segment_ext(vmm_t *vmm, int fd, unsigned long offset, unsigned long file_sz,
    unsigned long mem_sz, unsigned long vaddr)
{
    unsigned long vaddr_page = vaddr & PAGE_MASK;
    unsigned long size_in_first_page = PAGE_SIZE - (vaddr & PAGE_LIMIT);
    unsigned long occupy_pages = 0;
    if (mem_sz > size_in_first_page) {
        unsigned long left_size = mem_sz - size_in_first_page;
        occupy_pages = DIV_ROUND_UP(left_size, PAGE_SIZE) + 1;
    } else {
        occupy_pages = 1;
    }
    // dbgprintln("[proc] memmap vaddr=%p pages=%d", vaddr_page, occupy_pages);
    void *retaddr = mem_space_mmap2(vmm, vaddr_page, 0, occupy_pages * PAGE_SIZE, 
            PROT_USER | PROT_EXEC | PROT_WRITE | PROT_READ, MEM_SPACE_MAP_FIXED);
    if (retaddr == ((void *)-1)) {
        keprint(PRINT_ERR "proc_load_segment: mem_space_mmap failed!\n");
        return -1;
    }
    
    if (loadseg(vmm->page_storage, vaddr, fd, offset, file_sz) < 0) {
        keprint(PRINT_ERR "proc_load_segment: read file failed!\n");
        return -1;
    }
    return 0;
}

int proc_load_image32(vmm_t *vmm, Elf32_Ehdr *elf_header, int fd)
{
    Elf32_Phdr prog_header;
    Elf32_Off prog_header_off = elf_header->e_phoff;
    Elf32_Half prog_header_size = elf_header->e_phentsize;
    Elf32_Off prog_end;
    unsigned long grog_idx = 0;
    while (grog_idx < elf_header->e_phnum) {
        memset(&prog_header, 0, prog_header_size);
        kfile_lseek(fd, prog_header_off, SEEK_SET);
        if (kfile_read(fd, (void *)&prog_header, prog_header_size) != prog_header_size) {
            return -1;
        }
        if (prog_header.p_type == PT_LOAD) {
            #if DEBUG_PROCESS == 1
            keprint("elf segment: paddr:%x vaddr:%x file size:%x mem size: %x\n", 
                prog_header.p_paddr, prog_header.p_vaddr, prog_header.p_filesz, prog_header.p_memsz);
            #endif
            if (proc_load_segment(fd, prog_header.p_offset, 
                    prog_header.p_filesz, prog_header.p_memsz, prog_header.p_vaddr)) {
                return -1;
            }
            /* 如果内存大小比文件大小大，就要清0 */
            if (prog_header.p_memsz > prog_header.p_filesz) {
                memset((void *)(unsigned long)(prog_header.p_vaddr + prog_header.p_filesz), 0,
                    prog_header.p_memsz - prog_header.p_filesz);    
            }
            prog_end = prog_header.p_vaddr + prog_header.p_memsz;
            
            if (prog_header.p_flags == PHDR_CODE) {
                vmm->code_start = prog_header.p_vaddr;
                vmm->code_end = PAGE_ALIGN(prog_end);
                vmm->heap_start = vmm->code_end + PAGE_SIZE;
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            } else if (prog_header.p_flags == PHDR_DATA) {
                vmm->data_start = prog_header.p_vaddr;
                vmm->data_end = PAGE_ALIGN(prog_end);
                vmm->heap_start = vmm->data_end + PAGE_SIZE;                
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            } else if (prog_header.p_flags == PHDR_CODE_DATA) {
                vmm->code_start = prog_header.p_vaddr;
                vmm->code_end = PAGE_ALIGN(prog_end);
                vmm->data_start = prog_header.p_vaddr;
                vmm->data_end = PAGE_ALIGN(prog_end);
                vmm->heap_start = vmm->code_end + PAGE_SIZE;
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            }
            if (!vmm->heap_start && !vmm->heap_end) {
                vmm->heap_start = prog_end + PAGE_SIZE;
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            }
        }
        prog_header_off += prog_header_size;
        grog_idx++;
    }
    return 0;
}

int proc_load_image64(vmm_t *vmm, Elf64_Ehdr *elf_header, int fd)
{
    Elf64_Phdr prog_header;
    Elf64_Off prog_header_off = elf_header->e_phoff;
    Elf64_Half prog_header_size = elf_header->e_phentsize;
    Elf64_Off prog_end;
    unsigned long grog_idx = 0;
    while (grog_idx < elf_header->e_phnum) {
        memset(&prog_header, 0, prog_header_size);
        kfile_lseek(fd, prog_header_off, SEEK_SET);
        if (kfile_read(fd, (void *)&prog_header, prog_header_size) != prog_header_size) {
            return -1;
        }
        if (prog_header.p_type == PT_LOAD) {
            #if DEBUG_PROCESS == 1
            keprint("elf segment: paddr:%lx vaddr:%lx file size:%lx mem size: %lx\n", 
                prog_header.p_paddr, prog_header.p_vaddr, prog_header.p_filesz, prog_header.p_memsz);
            #endif
            if (proc_load_segment(fd, prog_header.p_offset, 
                    prog_header.p_filesz, prog_header.p_memsz, prog_header.p_vaddr)) {
                return -1;
            }
            /* 如果内存大小比文件大小大，就要清0 */
            if (prog_header.p_memsz > prog_header.p_filesz) {
                memset((void *)(unsigned long)(prog_header.p_vaddr + prog_header.p_filesz), 0,
                    prog_header.p_memsz - prog_header.p_filesz);    
            }
            prog_end = prog_header.p_vaddr + prog_header.p_memsz;
            
            if (prog_header.p_flags == PHDR_CODE) {
                vmm->code_start = prog_header.p_vaddr;
                vmm->code_end = PAGE_ALIGN(prog_end);
                vmm->heap_start = vmm->code_end + PAGE_SIZE;
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            } else if (prog_header.p_flags == PHDR_DATA) {
                vmm->data_start = prog_header.p_vaddr;
                vmm->data_end = PAGE_ALIGN(prog_end);
                vmm->heap_start = vmm->data_end + PAGE_SIZE;                
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            } else if (prog_header.p_flags == PHDR_CODE_DATA) {
                vmm->code_start = prog_header.p_vaddr;
                vmm->code_end = PAGE_ALIGN(prog_end);
                vmm->data_start = prog_header.p_vaddr;
                vmm->data_end = PAGE_ALIGN(prog_end);
                vmm->heap_start = vmm->code_end + PAGE_SIZE;
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            }
            if (!vmm->heap_start && !vmm->heap_end) {
                vmm->heap_start = prog_end + PAGE_SIZE;
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            }
        }
        prog_header_off += prog_header_size;
        grog_idx++;
    }
    return 0;
}


int proc_load_image64_ext(vmm_t *vmm, Elf64_Ehdr *elf_header, int fd)
{
    Elf64_Phdr prog_header;
    Elf64_Off prog_header_off = elf_header->e_phoff;
    Elf64_Half prog_header_size = elf_header->e_phentsize;
    Elf64_Off prog_end;
    unsigned long grog_idx = 0;
    while (grog_idx < elf_header->e_phnum) {
        memset(&prog_header, 0, prog_header_size);
        kfile_lseek(fd, prog_header_off, SEEK_SET);
        if (kfile_read(fd, (void *)&prog_header, prog_header_size) != prog_header_size) {
            return -1;
        }
        if (prog_header.p_type == PT_LOAD) {
            #if DEBUG_PROCESS == 1
            keprint("elf segment: paddr:%lx vaddr:%lx file size:%lx mem size: %lx\n", 
                prog_header.p_paddr, prog_header.p_vaddr, prog_header.p_filesz, prog_header.p_memsz);
            #endif
            if (proc_load_segment_ext(vmm, fd, prog_header.p_offset, 
                    prog_header.p_filesz, prog_header.p_memsz, prog_header.p_vaddr)) {
                return -1;
            }
            /* 如果内存大小比文件大小大，就要清0 */
            if (prog_header.p_memsz > prog_header.p_filesz) {
                /* TODO: 将bss段置0 */
                #if 0
                memset((void *)(unsigned long)(prog_header.p_vaddr + prog_header.p_filesz), 0,
                    prog_header.p_memsz - prog_header.p_filesz);
                #endif
            }
            prog_end = prog_header.p_vaddr + prog_header.p_memsz;
            
            if (prog_header.p_flags == PHDR_CODE) {
                vmm->code_start = prog_header.p_vaddr;
                vmm->code_end = PAGE_ALIGN(prog_end);
                vmm->heap_start = vmm->code_end + PAGE_SIZE;
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            } else if (prog_header.p_flags == PHDR_DATA) {
                vmm->data_start = prog_header.p_vaddr;
                vmm->data_end = PAGE_ALIGN(prog_end);
                vmm->heap_start = vmm->data_end + PAGE_SIZE;                
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            } else if (prog_header.p_flags == PHDR_CODE_DATA) {
                vmm->code_start = prog_header.p_vaddr;
                vmm->code_end = PAGE_ALIGN(prog_end);
                vmm->data_start = prog_header.p_vaddr;
                vmm->data_end = PAGE_ALIGN(prog_end);
                vmm->heap_start = vmm->code_end + PAGE_SIZE;
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            }
            if (!vmm->heap_start && !vmm->heap_end) {
                vmm->heap_start = prog_end + PAGE_SIZE;
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            }
        }
        prog_header_off += prog_header_size;
        grog_idx++;
    }
    return 0;
}


int proc_build_arg(unsigned long arg_top, unsigned long *arg_bottom, char *argv[], char **dest_argv[])
{
    int argc = 0;
    unsigned long arg_pos = arg_top;
    if (argv != NULL) {
        while (argv[argc]) {
            argc++;
        }
        if (argc != 0) {
            int i;
            for (i = 0; i < argc; i++) {
                arg_pos -= (strlen(argv[i]) + 1);
            }        
            arg_pos -= arg_pos % sizeof(unsigned long);
            arg_pos -= (argc + 1) * sizeof(char*);
            arg_pos -= sizeof(char**);
            arg_pos -= sizeof(unsigned long);
            if (arg_bottom) {
                *arg_bottom = (unsigned long) (arg_pos - sizeof(unsigned long));
            }
            unsigned long top = arg_pos;            
            *(unsigned long *)top = argc;
            top += sizeof(unsigned long);
            *(unsigned long *)top = top + sizeof(char **);
            *dest_argv = (char **)(top + sizeof(char **));
            top += sizeof(char **);
            char** _argv = (char **) top;
            char* p = (char *) top + sizeof(char *) * (argc + 1);
            for (i = 0; i < argc; i++) {
                _argv[i] = p;
                strcpy(p, argv[i]);
                p += (strlen(p) + 1);
            }
            _argv[i] = NULL;
            return argc;
        }
    }
    arg_pos -= 1 * sizeof(char*);
    arg_pos -= sizeof(char**);
    arg_pos -= sizeof(unsigned long);
    if (arg_bottom) {
        *arg_bottom = (unsigned long) (arg_pos - sizeof(unsigned long));
    }
    unsigned long top = arg_pos;
    *(unsigned long *)top = 0;
    top += sizeof(unsigned long);
    *(unsigned long *)top = top + sizeof(char **);
    *dest_argv = (char **)(top + sizeof(char **));
    top += sizeof(char **);
    char** _argv = (char **) top;
    _argv[0] = NULL;
    return argc;
}

void proc_map_space_init(task_t *task)
{
    task->vmm->map_start = (unsigned long) MEM_SPACE_MAP_ADDR_START;    
    task->vmm->map_end = task->vmm->map_start + MAX_MEM_SPACE_MAP_SIZE;
}

int proc_vmm_init(task_t *task)
{
    task->vmm = (vmm_t *)mem_alloc(sizeof(vmm_t));
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

int proc_vmm_exit_when_forking(task_t *child, task_t *parent)
{
    if (child->vmm == NULL)
        return -1;
    vmm_exit_when_fork_failed(child->vmm, parent->vmm);
    return 0;
}

int proc_pthread_init(task_t *task)
{
    #ifdef CONFIG_PTHREAD
    task->pthread = mem_alloc(sizeof(pthread_desc_t));
    if (task->pthread == NULL)
        return -1;
    pthread_desc_init(task->pthread);
    #endif
    return 0;
}

int proc_pthread_exit(task_t *task)
{
    #ifdef CONFIG_PTHREAD
    if (!task->pthread)
        return -1; 
    pthread_desc_exit(task->pthread);
    task->pthread = NULL;
    #endif
    return 0;
}

int proc_release(task_t *task)
{
    proc_vmm_exit(task);
    fs_fd_exit(task);
    exception_manager_exit(&task->exception_manager);
    #ifdef CONFIG_PTHREAD
    proc_pthread_exit(task);
    #endif
    sys_port_comm_unbind(-1);
    task_do_cancel(task);
    return 0;
}

void proc_exec_init(task_t *task)
{
    proc_map_space_init(task);
    fs_fd_reinit(task);
    exception_manager_exit(&task->exception_manager);
    exception_manager_init(&task->exception_manager);
    #ifdef CONFIG_PTHREAD
    pthread_desc_init(task->pthread);
    #endif
    sys_port_comm_unbind(-1);
    task_do_cancel(task);
    
    fpu_init(&task->fpu, 1); /* 需要初始化fpu */
}

int proc_destroy(task_t *task, int thread)
{
    if (!thread) {
        if (task->vmm == NULL)
            return -1;
        vmm_free(task->vmm);
        task->vmm = NULL;    
    }
    task_free(task);
    return 0;
}

void proc_trap_frame_init(task_t *task)
{
    #ifdef TASK_TRAPFRAME_ON_KSTACK
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)task + TASK_KERN_STACK_SIZE - sizeof(trap_frame_t));
    #else
    trap_frame_t *frame = task->trapframe;
    assert(frame != NULL);
    #endif
    user_frame_init(frame);
}

int proc_deal_zombie_child(task_t *parent)
{
    int zombies = 0;
    int zombie = -1;
    task_t *child, *next;
    list_for_each_owner_safe (child, next, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid) {
            if (child->state == TASK_ZOMBIE) {
                if (zombie == -1) {
                    zombie = child->pid;
                }
                if (TASK_IS_SINGAL_THREAD(child)) {
                    proc_destroy(child, 0);
                } else {
                    proc_destroy(child, 1);
                }
                zombies++;
            }
        }
    }
    return zombie; /* 如果没有僵尸进程就返回-1，有则返回第一个僵尸进程的pid */
}

void proc_close_one_thread(task_t *thread)
{
    if (thread->state == TASK_READY) {
        list_del_init(&thread->list);
    }
    if (thread->state != TASK_HANGING && thread->state != TASK_ZOMBIE) {
        task_do_cancel(thread);
    }
    proc_destroy(thread, 1);
}

void proc_close_other_threads(task_t *thread)
{
    task_t *borther, *next;
    list_for_each_owner_safe (borther, next, &task_global_list, global_list) {
        if (TASK_IN_SAME_THREAD_GROUP(thread, borther)) {
            if (thread->pid != borther->pid) {
                proc_close_one_thread(borther);
            }
        }
    }
    #ifdef CONFIG_PTHREAD
    if (thread->pthread) {
        atomic_set(&thread->pthread->thread_count, 0);
    }
    #endif
}

void proc_entry(void* arg)
{
    const char *pathname = (const char *) arg;
    task_t *cur = task_current;
    
    proc_execve(pathname, (const char **)cur->vmm->argv, (const char **)cur->vmm->envp);
    
    /* rease proc resource */
    proc_release(cur);
    /* thread exit. */
    kern_thread_exit(-1);
    panic("proc: start INIT process %s failed!\n", pathname);
}

/**
 * 创建一个进程
 * argv: 参数，argv[0]必须指定为文件路径
 * envp: 环境变量数组
 * flags: 进程标志，PROC_CREATE_STOPPED，表示创建后进程不立即执行，需要通过process_resume唤醒
 */
task_t *process_create(char **argv, char **envp, uint32_t flags)
{
    if (!argv || !argv[0])
        return NULL;
    task_t *task = (task_t *) mem_alloc(TASK_KERN_STACK_SIZE);
    if (!task)
        return NULL;
    task_t *parent = task_current;
    task_init(task, argv[0], TASK_PRIO_LEVEL_NORMAL);

    task->parent_pid = parent->pid;
    if (parent)
        task->pgid = parent->pgid;
    /* 进程执行前必须初始化文件描述符，内存管理，参数缓冲区 */
    if (fs_fd_init(task) < 0) {
        mem_free(task);
        return NULL;
    }
    /* 需要继承父进程的部分文件描述符 */
    fs_fd_copy_only(parent, task);
    
    if (proc_vmm_init(task)) {
        fs_fd_exit(task);
        mem_free(task);
        return NULL;
    }

    if (vmm_build_argbuf(task->vmm, argv, envp) < 0) {
        keprint(PRINT_ERR "process_create: pathname %s build arg buf failed !\n", argv[0]);
        proc_vmm_exit(task);
        fs_fd_exit(task);
        mem_free(task);
        return NULL;
    }
    /* argbuf[0-255] is path name */
    task_stack_build(task, proc_entry, task->vmm->argbuf);
    memcpy(task->vmm->argbuf, argv[0], min(MAX_PATH, strlen(argv[0])));

    /* 不是init就释放参数，这些参数来自sys_process_create */
    if (!(flags & PROC_CREATE_INKERN)) {
        proc_free_arg((char **)argv);
        proc_free_arg((char **)envp);
    }

    unsigned long irqflags;
    interrupt_save_and_disable(irqflags);
    task_add_to_global_list(task);
    
    if (flags & PROC_CREATE_STOP) {    /* 阻塞，需要等待唤醒 */
        task->state = TASK_STOPPED;
    } else {    /* 进入就绪队列执行 */
        sched_queue_add_tail(sched_get_cur_unit(), task);
    }
    interrupt_restore_state(irqflags);  
    return task;
}

int sys_create_process(char **argv, char **envp, uint32_t flags)
{
    if (!argv)
        return -EINVAL;
    /* 从用户态复制参数到内核 */
    char *_argv[MAX_TASK_STACK_ARG_NR], *_envp[MAX_TASK_STACK_ARG_NR];
    memset(_argv, 0, sizeof(_argv));
    memset(_envp, 0, sizeof(_envp));
    if (proc_copy_arg_from_user((char **)_argv, (char **)argv) < 0) {
        errprintln("[proc] proc_copy_arg_from_user for argv failed!");
        return -ENOMEM;
    }
    if (proc_copy_arg_from_user((char **)_envp, (char **)envp) < 0) {
        errprintln("[proc] proc_copy_arg_from_user for envp failed!");
        proc_free_arg(_argv);
        return -ENOMEM;
    }
    keprintln("create process");
    task_t *task = process_create(_argv, _envp, flags & ~PROC_CREATE_INKERN);
    if (task == NULL) {
        errprintln("[proc] create process failed!");
        return -EPERM;
    }
    return task->pid;
}

int sys_resume_process(pid_t pid)
{
    task_t *child = task_find_by_pid(pid);
    if (!child)
        return -EPERM;
    task_t *cur = task_current;
    if (child->parent_pid != cur->pid) {
        errprint("run process: task %d not the parent of task %d, no permission do this operation!\n",
            cur->pid, child->pid);
        return -EPERM;
    }
    if (child->state != TASK_STOPPED) {
        errprint("resume process: process %d not stopped!\n", child->pid);
        return -EBUSY;
    }
    task_wakeup(child);   
    return 0;
}


void proc_free_arg(char *arg[])
{
    if (!arg)
        return;
    int i;
    for (i = 0; i < MAX_TASK_STACK_ARG_NR; i++) {
        if (arg[i]) {
            mem_free(arg[i]);
        }
    }
}

void proc_dump_arg(char *arg[])
{
    if (!arg)
        return;
    int i;
    for (i = 0; i < MAX_TASK_STACK_ARG_NR; i++) {
        if (arg[i]) {
            dbgprintln("[proc] arg[%d]=%s", i, arg[i]);
        } else {
            break;
        }
    }
}

/**
 * 从用户复制参数，内核才能访问
 * 每一个参数都通过内存分配了地址，使用完后需要释放
 */
int proc_copy_arg_from_user(char *dst[], char *src[])
{
    if (!dst)
        return -1;
    uint64_t uargv = (uint64_t ) src;
    uint64_t *uarg;
    
    int i = 0;
    /* 获取argv */
    if (src) {
        while (1) {
            if (i >= MAX_TASK_STACK_ARG_NR) {
                goto err;
            }
            if (mem_copy_from_user((void *)&uarg, (void *)(uargv + sizeof(uint64_t)*i), sizeof(uarg)) < 0) {
                goto err;
            }
            // 没有参数了
            if(uarg == NULL){
                dst[i] = 0;
                break;
            }
            dst[i] = mem_alloc(PAGE_SIZE);
            if(dst[i] == NULL)
                goto err;
            if(mem_copy_from_user_str((char *)dst[i], (char *)uarg, PAGE_SIZE) < 0)
                goto err;
            i++;
        }
    } else {
        dst[0] = 0;
    }
    return i;
err:
    proc_free_arg(dst);
    return -1;
}
