#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/memspace.h>
#include <xbook/binformat.h>
#include <arch/phymem.h>
#include <arch/interrupt.h>
#include <arch/riscv.h>
#include <arch/page.h>

extern char trampoline[], uservec[], userret[];

extern void task_register_init();

static void kernel_thread_entry()
{
    task_t *cur = task_current;
    interrupt_enable();  /* 在启动前需要打开中断，避免启动后不能产生时钟中断调度 */
    task_register_init();
    cur->kthread_entry(cur->kthread_arg);
    // 如果函数返回了，那么就需要调用线程退出
    kern_thread_exit(0);
}

void task_stack_build(task_t *task, task_func_t *function, void *arg)
{
    // Set up new context to start executing at forkret,
    // which returns to user space.
    memset(&task->context, 0, sizeof(task->context));
    task->context.ra = (uint64_t)kernel_thread_entry;      // 函数执行入口
    task->context.sp = (uint64_t)(task->kstack + PAGE_SIZE); 
    task->kthread_entry = function;
    task->kthread_arg = arg;
}


/**
 * 将参数构建到栈中
 */
static int build_arg_stack(vmm_t *vmm, unsigned long stackbase, unsigned long *_sp, char **argv, int argv_argc, int total_argc)
{
    unsigned long sp = *_sp;
    dbgprint("sp=%p\n", sp);
    uint64_t argc = 6;
    uint64_t ustack[MAX_TASK_STACK_ARG_NR + 1] = {0};
    
    // push the array of argv[] pointers.
    sp -= (argc+1) * sizeof(uint64_t);
    dbgprint("sp=%p\n", sp);
    sp -= sp % 16;
    dbgprint("sp=%p\n", sp);
    if(sp < stackbase)
        return -1;
    
    // save argc
    sp -= sizeof(long);
    if(page_copy_out(vmm->page_storage, sp, (char *)ustack, (argc+1)*sizeof(uint64_t)) < 0)
        return -1;
    argv_argc = 0;

    dbgprint("sp=%p\n", sp);
    sp -= sp % 16;
    if(page_copy_out(vmm->page_storage, sp, (char *)&argv_argc, sizeof(uint64_t)) < 0)
        return -1;
    // save sp as new value
    dbgprint("final sp=%p\n", sp);
    *_sp = sp;
    return 0;
} 
#if 0
/**
 * 将参数构建到栈中
 */
static int build_arg_stack2(vmm_t *vmm, unsigned long stackbase, unsigned long *_sp, char **argv, int argv_argc, int total_argc)
{
    unsigned long sp = *_sp;
    uint64_t argc;
    uint64_t ustack[MAX_TASK_STACK_ARG_NR + 1];
    int _total_argc = total_argc;
    // Push argument strings, prepare rest of stack in ustack.
    for(argc = 0; _total_argc > 0; argc++) {
        if(argc >= MAX_TASK_STACK_ARG_NR)
            return -1;
        if (argv[argc]) {
            int arg_len = strlen(argv[argc]); 
            dbgprint("build_arg_stack: len %d=>%d\n", argc, arg_len);
            dbgprint("build_arg_stack: sp=%x\n", sp);            
            sp -= arg_len + 1;        
            dbgprint("build_arg_stack: sp=%x\n", sp);            
            sp -= sp % 16; // riscv sp must be 16-byte aligned
            dbgprint("build_arg_stack: sp=%x\n", sp);
            if(sp < stackbase)
                return -1;
            if(page_copy_out(vmm->page_storage, sp, argv[argc], arg_len + 1) < 0)
                return -1;
            dbgprint("build_arg_stack: %d=>%p\n", argc, sp);
            ustack[argc] = sp;
        } else {    // 0参数
            ustack[argc] = 0;
        }
        --_total_argc;
    }
    ustack[argc] = 0;
    
    dbgprint("ustack: 1\n");

    // push the array of argv[] pointers.
    sp -= (argc+1) * sizeof(uint64_t);
    sp -= sp % 16;
    if(sp < stackbase)
        return -1;
    dbgprint("ustack: 2\n");
    if(page_copy_out(vmm->page_storage, sp, (char *)ustack, (argc+1)*sizeof(uint64_t)) < 0)
        return -1;
    dbgprint("ustack: 3, argc=%d, total_argc=%d\n", argc, total_argc);

    /* print ustack */
    int i;
    for (i = 0; i < total_argc; i++) {
        if (ustack[i]) {
            dbgprint("ustack: %d->%p\n", i, ustack[i]);
        } else {
            dbgprint("ustack: %d->null\n", i);
        }
    }
    // save argc
    sp -= sizeof(uint64_t);
    if(page_copy_out(vmm->page_storage, sp, (char *)&argv_argc, sizeof(uint64_t)) < 0)
        return -1;
    // save sp as new value
    *_sp = sp;
    return 0;
} 
#endif
// #define DEBUG_ARGS
#define ELF_HWCAP	(0)

static void get_random_bytes(char *buf, size_t len)
{
    clock_t t = sys_get_ticks();
    int i;
    for (i = 0; i < len; i++) {
        buf[i] = ((t >> (i % 16)) & 0xff) + 1 * i;
    }
}
	

#if 1
static uint64_t create_user_stack(bin_program_t *bin_program, Elf64_Ehdr *elf, Elf64_Phdr *elf_phdr)
{
    int index = bin_program->argc + bin_program->envc + 2;
    
    uint64_t filename = bin_program_copy_string(bin_program, "./name");
    if ((long)filename == -1)
        return -1;

    char k_rand_bytes[16];
	get_random_bytes(k_rand_bytes, sizeof(k_rand_bytes));
    uint64_t u_rand_bytes = bin_program_copy_nbytes(bin_program, k_rand_bytes, sizeof(k_rand_bytes));
    if ((long)u_rand_bytes == -1)
        return -1;

    uint64_t u_elf_phdr = bin_program_copy_nbytes(bin_program, (char *)elf_phdr, elf->e_phentsize * elf->e_phnum);
    if ((long)u_elf_phdr == -1)
        return -1;

#define NEW_AUX_ENT(id, val)                                                   \
do {                                                                         \
    bin_program->ustack[index++] = (id);                                         \
    bin_program->ustack[index++] = (uint64_t)(val);                                        \
} while (0)

    /*
    dbgprint("AT_PHDR: %p, AT_PHENT: %d, AT_PHNUM: %d, AT_PAGESZ: %x, AT_ENTRY: %p\n",
        elf->e_phoff, sizeof(Elf64_Phdr), elf->e_phnum, 0x1000, elf->e_entry);
    */

    NEW_AUX_ENT(AT_HWCAP, ELF_HWCAP);
	NEW_AUX_ENT(AT_PAGESZ, PAGE_SIZE);
	NEW_AUX_ENT(AT_CLKTCK, HZ);
	NEW_AUX_ENT(AT_PHDR, u_elf_phdr);
	NEW_AUX_ENT(AT_PHENT, sizeof(Elf64_Phdr));
	NEW_AUX_ENT(AT_PHNUM, elf->e_phnum);
	NEW_AUX_ENT(AT_BASE, 0);
	
    NEW_AUX_ENT(AT_FLAGS, 0);
	NEW_AUX_ENT(AT_ENTRY, elf->e_entry);
	NEW_AUX_ENT(AT_UID, 0);
	NEW_AUX_ENT(AT_EUID, 0);
	NEW_AUX_ENT(AT_GID, 0);
	NEW_AUX_ENT(AT_EGID, 0);
	NEW_AUX_ENT(AT_SECURE, 0);
	NEW_AUX_ENT(AT_RANDOM, (unsigned long)u_rand_bytes);

    NEW_AUX_ENT(AT_EXECFN, filename);
    NEW_AUX_ENT(AT_PLATFORM, NULL);

    NEW_AUX_ENT(AT_BASE_PLATFORM, NULL);
    NEW_AUX_ENT(AT_EXECFD, 3);  /* 3 is free */
    NEW_AUX_ENT(AT_NULL, NULL);

    #undef NEW_AUX_ENT

    dbgprint("index: %d\n", index);

    bin_program->sp -= sizeof(uint64_t) * index;
    if (page_copy_out(bin_program->pagetable, bin_program->sp,
                (char *)bin_program->ustack, sizeof(uint64_t) * index)) {
        return -1;
    }

    uint64_t argc = bin_program->argc;
    bin_program->sp -= sizeof(uint64_t);
    page_copy_out(bin_program->pagetable, bin_program->sp, (char *)&argc,
            sizeof(uint64_t));
    return 0;
}

#else
static uint64_t create_user_stack(bin_program_t *bin_program, Elf64_Ehdr *elf)
{
    int index = bin_program->argc + bin_program->envc + 2;

    uint64_t filename = bin_program_copy_string(bin_program, "./name");
#define NEW_AUX_ENT(id, val)                                                   \
do {                                                                         \
    bin_program->ustack[index++] = id;                                         \
    bin_program->ustack[index++] = val;                                        \
} while (0)

    // 1
    // 2
    NEW_AUX_ENT(0x28, 0);
    NEW_AUX_ENT(0x29, 0);
    NEW_AUX_ENT(0x2a, 0);
    NEW_AUX_ENT(0x2b, 0);
    NEW_AUX_ENT(0x2c, 0);
    NEW_AUX_ENT(0x2d, 0);
#if 1

    dbgprint("AT_PHDR: %p, AT_PHENT: %d, AT_PHNUM: %d, AT_PAGESZ: %x, AT_ENTRY: %p\n",
        elf->e_phoff, sizeof(Elf64_Phdr), elf->e_phnum, 0x1000, elf->e_entry);
    NEW_AUX_ENT(AT_PHDR, elf->e_phoff);               // 3
    //NEW_AUX_ENT(AT_PHENT, sizeof(Elf64_Phdr));  // 4
    NEW_AUX_ENT(AT_PHENT, 0);  // 4
    //NEW_AUX_ENT(AT_PHNUM, elf->e_phnum);              // 5
    NEW_AUX_ENT(AT_PHNUM, 0);              // 5
    NEW_AUX_ENT(AT_PAGESZ, 0x1000);                 // 6
    NEW_AUX_ENT(AT_BASE, 0);                        // 7
    NEW_AUX_ENT(AT_FLAGS, 0);                       // 8
    NEW_AUX_ENT(AT_ENTRY, elf->e_entry);              // 9
    NEW_AUX_ENT(AT_UID, 0);                         // 11
    NEW_AUX_ENT(AT_EUID, 0);                        // 12
    NEW_AUX_ENT(AT_GID, 0);                         // 13
    NEW_AUX_ENT(AT_EGID, 0);                        // 14
    NEW_AUX_ENT(AT_HWCAP, 0x112d);                  // 16
    NEW_AUX_ENT(AT_CLKTCK, 64);                     // 17
    NEW_AUX_ENT(AT_EXECFN, filename);               // 31
    NEW_AUX_ENT(0, 0);
#else
    NEW_AUX_ENT(AT_PHDR, 0);               // 3
    NEW_AUX_ENT(AT_PHENT, 0);  // 4
    NEW_AUX_ENT(AT_PHNUM, 0);              // 5
    NEW_AUX_ENT(AT_PAGESZ, 0x1000);                 // 6
    NEW_AUX_ENT(AT_BASE, 0);                        // 7
    NEW_AUX_ENT(AT_FLAGS, 0);                       // 8
    NEW_AUX_ENT(AT_ENTRY, 0);              // 9
    NEW_AUX_ENT(AT_UID, 0);                         // 11
    NEW_AUX_ENT(AT_EUID, 0);                        // 12
    NEW_AUX_ENT(AT_GID, 0);                         // 13
    NEW_AUX_ENT(AT_EGID, 0);                        // 14
    NEW_AUX_ENT(AT_HWCAP, 0);                  // 16
    NEW_AUX_ENT(AT_CLKTCK, 100);                     // 17
    NEW_AUX_ENT(AT_EXECFN, filename);               // 31
    NEW_AUX_ENT(0, 0);
#endif
    #undef NEW_AUX_ENT
    bin_program->sp -= sizeof(uint64_t) * index;
    if (page_copy_out(bin_program->pagetable, bin_program->sp,
                (char *)bin_program->ustack, sizeof(uint64_t) * index)) {
        return -1;
    }
    uint64_t argc = bin_program->argc;
    bin_program->sp -= sizeof(uint64_t);
    page_copy_out(bin_program->pagetable, bin_program->sp, (char *)&argc,
            sizeof(uint64_t));
    return 0;
}
#endif

static int merge_args(char *args[], char *argv[], char *envp[])
{
    int i, j = 0;
    for (i = 0; i < MAX_TASK_STACK_ARG_NR; i++) {
        if (argv[i] != NULL) {
#ifdef DEBUG_ARGS
            dbgprint("[proc] argv[%d]=%s\n", i, argv[i]);
#endif
            args[j++] = argv[i];
        } else {
            if (i == 0) {   /* 没有任何参数，不过还是需要至少保留一个参数 */
                dbgprint("[proc] no argv at %d\n", j);
                args[j++] = NULL;
            }
            break;
        }
    }
#ifdef DEBUG_ARGS
    dbgprint("[proc] argv end at %d\n", j);
#endif
    args[j++] = NULL; // 中间预留一个0，因为环境和参数中间要间隔一个0
    for (i = 0; i < MAX_TASK_STACK_ARG_NR; i++) {
        if (envp[i] != NULL) {
#ifdef DEBUG_ARGS
            dbgprint("[proc] envp[%d]=%p\n", i, envp[i]);
#endif
            args[j++] = envp[i];

        } else {
            if (i == 0) {   /* 没有任何参数，不过还是需要至少保留一个参数 */
                dbgprint("[proc] no envp at %d\n", j);
                args[j++] = NULL;
            }
            break;
        }
    }
    args[j++] = NULL;
    return j;
}

int process_frame_init(task_t *task, vmm_t *vmm, trap_frame_t *frame, char **argv, char **envp, 
    Elf64_Ehdr *elf_header, Elf64_Phdr *elf_phdr)
{
    vmm->stack_end = USER_STACK_TOP;
    vmm->stack_start = vmm->stack_end - MEM_SPACE_STACK_SIZE_DEFAULT;

    if (mem_space_mmap2(vmm, vmm->stack_start, 0, vmm->stack_end - vmm->stack_start , PROT_USER | PROT_WRITE | PROT_READ,
        MEM_SPACE_MAP_FIXED | MEM_SPACE_MAP_STACK) == ((void *)-1)) {
        return -1;
    }

    /**
     * 参数布局：
     * --------*
     * aux arg *
     * --------*
     * env arg *
     * --------*
     * arg     *
     * --------*
     * argc    * <- sp
     * --------*
     */

    uint64_t ustack[MAX_TASK_STACK_ARG_NR + 1];  // 最后一项为0，用于标记结束
    uint64_t sp = vmm->stack_end;
    sp -= sizeof(uint64_t);

    bin_program_t bin_prog;
    bin_program_init(&bin_prog);
        
    bin_prog.stack_top = 0;
    bin_prog.sp = sp;
    bin_prog.stackbase = vmm->stack_start;
    bin_prog.pagetable = vmm->page_storage;
    bin_prog.ustack = ustack;
    #ifdef DEBUG_ARGS
    int i;
    for (i = 0; argv[i]; i++) {
        dbgprint("argv %d=>%p:%s\n", i, argv[i], argv[i] == NULL ? "null": argv[i]);
    }
    for (i = 0; envp[i]; i++) {
        dbgprint("envp %d=>%p:%s\n", i, envp[i], envp[i] == NULL ? "null": envp[i]);
    }
    dbgprint("argv %p envp %p\n", argv, envp);
    #endif
    uint64_t *mergestack[MAX_TASK_STACK_ARG_NR + 1];
    merge_args((char **)mergestack, argv, envp);
    
    bin_prog.argc = bin_program_copy_string2stack(&bin_prog, (char **)mergestack);
    if (bin_prog.argc < 0)
        return -1;
    
    bin_prog.envc = bin_program_copy_string2stack(&bin_prog, (char **)mergestack);
    if (bin_prog.envc < 0)
        return -1;

    if (create_user_stack(&bin_prog, elf_header, elf_phdr) < 0)
        return -1;
    
    #ifdef DEBUG_ARGS
    for (i = 0; i < bin_prog.argc + bin_prog.envc + 1; i++) {
        dbgprint("bin prog: arg %d=>%p\n", i, bin_prog.ustack[i]);
    }
    #endif

    sp = bin_prog.sp;

    frame->a1 = sp;
    frame->ra = 0;
    frame->sp = sp;

    #if 0
    unsigned long arg_bottom = 0;
    arg_bottom = vmm->stack_end - 32;
    if (build_arg_stack(vmm, vmm->stack_end - PAGE_SIZE, &arg_bottom, totalstack, argc, j) < 0) {
        mem_space_unmmap2(vmm, vmm->stack_start, vmm->stack_end - vmm->stack_start);
        return -1;
    }
    /* sp保存参数信息，通过sp就可以找到所有参数了 */
    frame->sp = arg_bottom;
    #endif
    return 0;
}

int process_frame_init2(task_t *task, vmm_t *vmm, trap_frame_t *frame, char **argv, char **envp)
{
    /* 先将2者组合到一个数组 */
    char *totalstack[MAX_TASK_STACK_ARG_NR + 1] = {0};
    int argc = 0;
    int i, j = 0;
    for (i = 0; i < MAX_TASK_STACK_ARG_NR; i++) {
        if (argv[i] != NULL) {
            totalstack[j++] = argv[i];
#ifdef DEBUG_ARGS
            dbgprint("[proc] argv[%d]=%s\n", i, argv[i]);
#endif
        } else {
            if (i == 0) {   /* 没有任何参数，不过还是需要至少保留一个参数 */
                totalstack[j++] = NULL;
            }
            break;
        }
    }
    argc = j;
    totalstack[j++] = NULL; // 中间预留一个0，因为环境和参数中间要间隔一个0
    for (i = 0; i < MAX_TASK_STACK_ARG_NR; i++) {
        if (envp[i] != NULL) {
            totalstack[j++] = envp[i];
#ifdef DEBUG_ARGS
            dbgprint("[proc] envp[%d]=%s\n", i, envp[i]);
#endif
        } else {
            if (i == 0) {   /* 没有任何参数，不过还是需要至少保留一个参数 */
                dbgprint("[proc] no envp at %d\n", j);
                totalstack[j++] = NULL;
            }
            break;
        }
    }
    totalstack[j++] = NULL;
    /* 保留aux的位置，在env后面 */
    dbgprint("[proc] pad aux at %d\n", j);
    totalstack[j++] = NULL;
    totalstack[j] = NULL;
    
#ifdef DEBUG_ARGS
    dbgprint("task %d argc=%d, total=%d\n", task->pid, argc, j);
#endif

    if (j >= MAX_TASK_STACK_ARG_NR) {
        errprint("task %d too many args\n", task->pid);
        return -1;
    }
    vmm->stack_end = USER_STACK_TOP;
    vmm->stack_start = vmm->stack_end - MEM_SPACE_STACK_SIZE_DEFAULT;

    if (mem_space_mmap2(vmm, vmm->stack_start, 0, vmm->stack_end - vmm->stack_start , PROT_USER | PROT_WRITE | PROT_READ,
        MEM_SPACE_MAP_FIXED | MEM_SPACE_MAP_STACK) == ((void *)-1)) {
        return -1;
    }

    

    /**
     * 参数布局：
     * --------*
     * aux arg * # 不支持，只保留
     * --------*
     * env arg *
     * --------*
     * arg     *
     * --------*
     * argc    * <- sp
     * --------*
     */
    unsigned long arg_bottom = 0;
    arg_bottom = vmm->stack_end - 32;
    if (build_arg_stack(vmm, vmm->stack_end - PAGE_SIZE, &arg_bottom, totalstack, argc, j) < 0) {
        mem_space_unmmap2(vmm, vmm->stack_start, vmm->stack_end - vmm->stack_start);
        return -1;
    }
    /* sp保存参数信息，通过sp就可以找到所有参数了 */
    frame->sp = arg_bottom;
    return 0;
}
void kernel_switch_to_user(trap_frame_t *frame)
{
    task_activate_when_sched(task_current);  // 激活页表
    usertrapret();
}

void user_frame_init(trap_frame_t *frame)
{
    memset(frame, 0, sizeof(trap_frame_t));
}

extern pgdir_t kernel_pgdir;
int task_stack_build_when_forking(task_t *child)
{
    trap_frame_t *frame = child->trapframe;
    /* 设置a0为0，就相当于设置了子任务的返回值为0 */
    frame->a0 = 0;
    memset(&child->context, 0, sizeof(child->context));
    child->context.ra = (uint64_t)forkret;      // 子进程第一次获得执行权时会跳转到该处
    child->context.sp = (uint64_t)(child->kstack + PAGE_SIZE); 
    return 0;
}

void proc_set_stack_pointer(task_t *task, unsigned long sp)
{
    task->trapframe->sp = sp;
}