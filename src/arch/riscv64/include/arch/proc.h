#ifndef __RISCV64_PROC_H
#define __RISCV64_PROC_H

#include <stdint.h>
#include <types.h>
#include <xbook/spinlock.h>

#define NPROC 32

// Saved registers for kernel context switches.
struct context {
  uint64_t ra;
  uint64_t sp;

  // callee-saved
  uint64_t s0;
  uint64_t s1;
  uint64_t s2;
  uint64_t s3;
  uint64_t s4;
  uint64_t s5;
  uint64_t s6;
  uint64_t s7;
  uint64_t s8;
  uint64_t s9;
  uint64_t s10;
  uint64_t s11;
};

enum procstate { UNUSED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
    spinlock_t lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  pid_t pid;                     // Process ID

    void *thread_arg;       // 线程执行时传入参数
  // these are private to the process, so p->lock need not be held.
  uint64_t kstack;               // Virtual address of kernel stack
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
  char name[16];               // Process name (debugging)
};

void
procinit(void);

struct proc*
allocproc(task_func_t func, void *arg);

void
yield_to(struct proc *next);

#endif  /* __RISCV64_PROC_H */