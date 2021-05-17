#include <stddef.h>
#include <arch/proc.h>
#include <arch/riscv.h>
#include <arch/page.h>
#include <xbook/debug.h>
#include <assert.h>
#include <string.h>
// #include <xbook/memcache.h>

struct proc proc[NPROC];

struct proc *cur_proc;

int nextpid = 1;
struct spinlock pid_lock;

void reg_info(void) {
  keprint("register info: {\n");
  keprint("sstatus: %p\n", r_sstatus());
  keprint("sip: %p\n", r_sip());
  keprint("sie: %p\n", r_sie());
  keprint("sepc: %p\n", r_sepc());
  keprint("stvec: %p\n", r_stvec());
  keprint("satp: %p\n", r_satp());
  keprint("scause: %p\n", r_scause());
  keprint("stval: %p\n", r_stval());
  keprint("sp: %p\n", r_sp());
  keprint("tp: %p\n", r_tp());
  keprint("ra: %p\n", r_ra());
  keprint("}\n");
}

struct proc *p0;
struct proc *p1;

void thread_a(void *arg)
{
    keprint("thread a running.\n");
    while (1)
    {
        yield_to(p1);
        infoprint("[A] I am running...\n");

    }
    
}

void thread_b(void *arg)
{
    keprint("thread b running.\n");
    while (1)
    {
        yield_to(p0);
        infoprint("[B] I am running...\n");

    }
    
}

// initialize the proc table at boot time.
void
procinit(void)
{
    struct proc *p;
  
    spinlock_init(&pid_lock);
    for(p = proc; p < &proc[NPROC]; p++) {
        spinlock_init(&p->lock);
        p->state = UNUSED;
        p->pid = -1;  
    }
    cur_proc = NULL;

    dbgprint("[proc] init proc done.\n");

    p0 = allocproc(thread_a, NULL);
    if (p0 == NULL)
        panic("alloc proc 0 failed!\n");
    cur_proc = p0;
    p0->state = RUNNING;

    p1 = allocproc(thread_b, NULL);
    if (p1 == NULL)
        panic("alloc proc 1 failed!\n");
    

    infoprint("[proc] init done.\n");

    while (1)
    {
        yield_to(p1);
        infoprint("[main] I am running...\n");
    }
}

// Return the current struct proc *, or zero if none.
struct proc*
myproc(void) {
  return cur_proc;
}

int
allocpid() {
  int pid;
  
  spin_lock(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  spin_unlock(&pid_lock);

  return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
struct proc*
allocproc(task_func_t func, void *arg)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++) {
    spin_lock(&p->lock);
    if(p->state == UNUSED) {
      goto found;
    } else {
      spin_unlock(&p->lock);
    }
  }
  return NULL;

found:
  p->pid = allocpid();

    /* 分配内核栈，用于内核线程执行环境 */
    p->kstack = (uint64_t) kern_phy_addr2vir_addr(page_alloc_normal(1));
    assert(p->kstack != 0);

    p->thread_arg = arg;
  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)func;      // 函数执行入口
  p->context.sp = p->kstack + PAGE_SIZE;   
    p->state = RUNNABLE;

    spin_unlock(&p->lock);
  return p;
}


// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void
yield_to(struct proc *next)
{
  int intena;
  struct proc *p = myproc();

  spin_lock(&p->lock);

  p->state = RUNNABLE;  // yield out

  if(p->state == RUNNING)
    panic("cur not running");

  if(next->state == RUNNING)
    panic("next is running");
  
  cur_proc = next;
  thread_switch_to_next(&p->context, &next->context);

  spin_unlock(&p->lock);
}