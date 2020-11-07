#include <xbook/process.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>
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
    int ret = (int)mem_space_mmap(vaddr_page, 0, occupy_pages * PAGE_SIZE, 
            PROT_USER | PROT_WRITE, MEM_SPACE_MAP_FIXED);
    if (ret < 0) {
        printk(KERN_ERR "proc_load_segment: mem_space_mmap failed!\n");
        return -1;
    }
    sys_lseek(fd, offset, SEEK_SET);
    if (sys_read(fd, (void *)vaddr, file_sz) != file_sz) {
        return -1;
    }
    return 0;
}

int proc_load_image(vmm_t *vmm, struct Elf32_Ehdr *elf_header, int fd)
{
    struct Elf32_Phdr prog_header;
    Elf32_Off prog_header_off = elf_header->e_phoff;
    Elf32_Half prog_header_size = elf_header->e_phentsize;
    Elf32_Off prog_end;
    unsigned long grog_idx = 0;
    while (grog_idx < elf_header->e_phnum) {
        memset(&prog_header, 0, prog_header_size);
        sys_lseek(fd, prog_header_off, SEEK_SET);
        if (sys_read(fd, (void *)&prog_header, prog_header_size) != prog_header_size) {
            return -1;
        }
        if (prog_header.p_type == PT_LOAD) {
            if (proc_load_segment(fd, prog_header.p_offset, 
                    prog_header.p_filesz, prog_header.p_memsz, prog_header.p_vaddr)) {
                return -1;
            }
            /* 如果内存大小比文件大小大，就要清0 */
            if (prog_header.p_memsz > prog_header.p_filesz) {
                memset((void *)(prog_header.p_vaddr + prog_header.p_filesz), 0,
                    prog_header.p_memsz - prog_header.p_filesz);    
            }
            prog_end = prog_header.p_vaddr + prog_header.p_memsz;
            if (prog_header.p_flags == ELF32_PHDR_CODE) {
                vmm->code_start = prog_header.p_vaddr;
                vmm->code_end = prog_end;
                vmm->heap_start = vmm->code_end + PAGE_SIZE;
                vmm->heap_start = PAGE_ALIGN(vmm->heap_start);
                vmm->heap_end = vmm->heap_start;
            } else if (prog_header.p_flags == ELF32_PHDR_DATA) {
                vmm->data_start = prog_header.p_vaddr;
                vmm->data_end = prog_end;
                vmm->heap_start = vmm->data_end + PAGE_SIZE;                
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
    proc_vmm_exit(task);
    proc_trigger_exit(task);
    fs_fd_exit(task);
    gui_user_exit(task);
    proc_pthread_exit(task);
    task_do_cancel(task);
    return 0;
}

int proc_destroy(task_t *task, int thread)
{
    if (task->vmm == NULL)
        return -1;
    if (!thread) {
        page_free_one(kern_vir_addr2phy_addr(task->vmm->page_storage));
        vmm_free(task->vmm);
        task->vmm = NULL;    
    }
    task_free(task);
    return 0;
}

void proc_trap_frame_init(task_t *task)
{
    trap_frame_t *frame = (trap_frame_t *)\
        ((unsigned long)task + TASK_KERN_STACK_SIZE - sizeof(trap_frame_t));
    user_frame_init(frame);
}

void proc_entry(void* arg)
{
    const char **argv = (const char **) arg;
    sys_execve(argv[0], argv, NULL);
    panic("proc: start first process %s failed!\n", argv[0]);
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
    if (thread->pthread) {
        atomic_set(&thread->pthread->thread_count, 0);
    }
}

task_t *user_process_start(char *name, char **argv)
{
    task_t *task = (task_t *) mem_alloc(TASK_KERN_STACK_SIZE);
    if (!task)
        return NULL;
    task_init(task, name, TASK_PRIO_USER);
    task->pid = USER_INIT_PROC_ID;
    task->tgid = task->pid;
    if (proc_vmm_init(task)) {
        mem_free(task);
        return NULL;
    }
    if (proc_trigger_init(task)) {
        proc_vmm_exit(task);
        mem_free(task);
        return NULL;
    }
    if (fs_fd_init(task) < 0) {
        proc_trigger_exit(task);
        proc_vmm_exit(task);
        mem_free(task);
        return NULL;
    }
    task_stack_build(task, proc_entry, argv);
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task_add_to_global_list(task);
    sched_queue_add_tail(sched_get_cur_unit(), task);
    interrupt_restore_state(flags);    
    return task;
}
