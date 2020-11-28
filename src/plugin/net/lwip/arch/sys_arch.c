/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Wed Apr 17 16:05:29 EDT 2002 (James Roth)
 *
 *  - Fixed an unlikely sys_thread_new() race condition.
 *
 *  - Made current_thread() work with threads which where
 *    not created with sys_thread_new().  This includes
 *    the main thread and threads made with pthread_create().
 *
 *  - Catch overflows where more than SYS_MBOX_SIZE messages
 *    are waiting to be read.  The sys_mbox_post() routine
 *    will block until there is more room instead of just
 *    leaking messages.
 */
#include <lwip/opt.h>
#include <lwip/arch.h>
#include <lwip/stats.h>
#include <lwip/debug.h>
#include <lwip/sys.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <xbook/task.h>
#include <xbook/semaphore.h>
#include <xbook/walltime.h>
#include <xbook/mutexlock.h>

#if 0
u32_t sys_now()
{
  return clock();
}
#endif

// #define DEBUG_LWIP_ARCH

#define UMAX(a, b)      ((a) > (b) ? (a) : (b))

static struct timeval starttime;

#if !NO_SYS

static struct sys_thread *threads = NULL;
static DEFINE_MUTEX_LOCK(threads_mutex);

struct sys_mbox_msg {
  struct sys_mbox_msg *next;
  void *msg;
};

#define SYS_MBOX_SIZE 128

struct sys_mbox {
  int first, last;
  void *msgs[SYS_MBOX_SIZE];
  struct sys_sem *not_empty;
  struct sys_sem *not_full;
  struct sys_sem *mutex;
  int wait_send;
};

struct sys_sem {
  semaphore_t sem;
};

struct sys_thread {
  struct sys_thread *next;
  task_t *kthread;
};

#if SYS_LIGHTWEIGHT_PROT
static pthread_mutex_t lwprot_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t lwprot_thread = (pthread_t)0xDEAD;
static int lwprot_count = 0;
#endif /* SYS_LIGHTWEIGHT_PROT */

static struct sys_sem *sys_sem_new_internal(u8_t count);
static void sys_sem_free_internal(struct sys_sem *sem);

static void *__malloc(size_t size)
{
    return mem_alloc(size);
}

static void __free(void *ptr)
{
    mem_free(ptr);
}

/*-----------------------------------------------------------------------------------*/
static struct sys_thread * 
introduce_thread(task_t *kthread)
{
  struct sys_thread *thread;

  thread = (struct sys_thread *)__malloc(sizeof(struct sys_thread));

  if (thread != NULL) {
    mutex_lock(&threads_mutex);
    thread->next = threads;
    thread->kthread = kthread;
    threads = thread;
    mutex_unlock(&threads_mutex);
  }

  return thread;
}
/*-----------------------------------------------------------------------------------*/
sys_thread_t
sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
  task_t *thread;
  struct sys_thread *st = NULL;
  //LWIP_UNUSED_ARG(name);
  LWIP_UNUSED_ARG(stacksize);
  //LWIP_UNUSED_ARG(prio);
  
  thread = kern_thread_start((char *)name, prio, function, arg);

  if (NULL != thread) {
    st = introduce_thread(thread);
  }
  #ifdef DEBUG_LWIP_ARCH
  printk("%s: thread %x\n", __func__, thread);
  #endif
  if (NULL == st) {
    LWIP_DEBUGF(SYS_DEBUG, ("sys_thread_new: kern_thread_start %x, st = 0x%lx",
                       thread, (unsigned long)st));
    abort();
  }
  return st;
}
/*-----------------------------------------------------------------------------------*/
err_t
sys_mbox_new(struct sys_mbox **mb, int size)
{
  struct sys_mbox *mbox;
  LWIP_UNUSED_ARG(size);
  mbox = (struct sys_mbox *)__malloc(sizeof(struct sys_mbox));
  if (mbox == NULL) {
    return ERR_MEM;
  }
  #ifdef DEBUG_LWIP_ARCH
  printk("%s: mb: %x -> mbox: %x\n", __func__, mb, mbox);
  #endif
  mbox->first = mbox->last = 0;
  mbox->not_empty = sys_sem_new_internal(0);
  mbox->not_full = sys_sem_new_internal(0);
  mbox->mutex = sys_sem_new_internal(1);
  mbox->wait_send = 0;

  SYS_STATS_INC_USED(mbox);
  *mb = mbox;
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void
sys_mbox_free(struct sys_mbox **mb)
{
  if ((mb != NULL) && (*mb != SYS_MBOX_NULL)) {
    struct sys_mbox *mbox = *mb;
    SYS_STATS_DEC(mbox.used);
    sys_arch_sem_wait(&mbox->mutex, 0);
    #ifdef DEBUG_LWIP_ARCH
    printk("%s: mb: %x -> mbox: %x\n", __func__, mb, mbox);
    #endif
    sys_sem_free_internal(mbox->not_empty);
    sys_sem_free_internal(mbox->not_full);
    sys_sem_free_internal(mbox->mutex);
    mbox->not_empty = mbox->not_full = mbox->mutex = NULL;
    /*  LWIP_DEBUGF("sys_mbox_free: mbox 0x%lx\n", mbox); */
    __free(mbox);
  }
}
/*-----------------------------------------------------------------------------------*/
err_t
sys_mbox_trypost(struct sys_mbox **mb, void *msg)
{
  u8_t first;
  struct sys_mbox *mbox;
  LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
  mbox = *mb;
  #ifdef DEBUG_LWIP_ARCH
  printk("%s: mb %x -> mbox %x\n", __func__, mb, mbox);
  #endif
  sys_arch_sem_wait(&mbox->mutex, 0);

  LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_trypost: mbox %p msg %p\n",
                          (void *)mbox, (void *)msg));

  if ((mbox->last + 1) >= (mbox->first + SYS_MBOX_SIZE)) {
    sys_sem_signal(&mbox->mutex);
    return ERR_MEM;
  }

  mbox->msgs[mbox->last % SYS_MBOX_SIZE] = msg;

  if (mbox->last == mbox->first) {
    first = 1;
  } else {
    first = 0;
  }

  mbox->last++;

  if (first) {
    sys_sem_signal(&mbox->not_empty);
  }

  sys_sem_signal(&mbox->mutex);

  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void
sys_mbox_post(struct sys_mbox **mb, void *msg)
{
  u8_t first;
  struct sys_mbox *mbox;
  LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
  mbox = *mb;
  #ifdef DEBUG_LWIP_ARCH
  printk("%s: mb %x -> mbox %x\n", __func__, mb, mbox);
  #endif
  sys_arch_sem_wait(&mbox->mutex, 0);

  LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_post: mbox %p msg %p\n", (void *)mbox, (void *)msg));

  while ((mbox->last + 1) >= (mbox->first + SYS_MBOX_SIZE)) {
    mbox->wait_send++;
    sys_sem_signal(&mbox->mutex);
    sys_arch_sem_wait(&mbox->not_full, 0);
    sys_arch_sem_wait(&mbox->mutex, 0);
    mbox->wait_send--;
  }

  mbox->msgs[mbox->last % SYS_MBOX_SIZE] = msg;

  if (mbox->last == mbox->first) {
    first = 1;
  } else {
    first = 0;
  }

  mbox->last++;

  if (first) {
    sys_sem_signal(&mbox->not_empty);
  }

  sys_sem_signal(&mbox->mutex);
}
/*-----------------------------------------------------------------------------------*/
u32_t
sys_arch_mbox_tryfetch(struct sys_mbox **mb, void **msg)
{
  struct sys_mbox *mbox;
  LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
  mbox = *mb;
  #ifdef DEBUG_LWIP_ARCH
  printk("%s: mb %x -> mbox %x\n", __func__, mb, mbox);
  #endif
  sys_arch_sem_wait(&mbox->mutex, 0);

  if (mbox->first == mbox->last) {
    sys_sem_signal(&mbox->mutex);
    return SYS_MBOX_EMPTY;
  }

  if (msg != NULL) {
    LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_tryfetch: mbox %p msg %p\n", (void *)mbox, *msg));
    *msg = mbox->msgs[mbox->first % SYS_MBOX_SIZE];
  }
  else{
    LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_tryfetch: mbox %p, null msg\n", (void *)mbox));
  }

  mbox->first++;

  if (mbox->wait_send) {
    sys_sem_signal(&mbox->not_full);
  }

  sys_sem_signal(&mbox->mutex);

  return 0;
}
/*-----------------------------------------------------------------------------------*/
u32_t
sys_arch_mbox_fetch(struct sys_mbox **mb, void **msg, u32_t timeout)
{
  u32_t time_needed = 0;
  struct sys_mbox *mbox;
  LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
  mbox = *mb;
  #ifdef DEBUG_LWIP_ARCH
  printk("%s: mb %x -> mbox %x\n", __func__, mb, mbox);
  #endif
  /* The mutex lock is quick so we don't bother with the timeout
     stuff here. */
  sys_arch_sem_wait(&mbox->mutex, 0);

  while (mbox->first == mbox->last) {
    sys_sem_signal(&mbox->mutex);

    /* We block while waiting for a mail to arrive in the mailbox. We
       must be prepared to timeout. */
    if (timeout != 0) {
      time_needed = sys_arch_sem_wait(&mbox->not_empty, timeout);

      if (time_needed == SYS_ARCH_TIMEOUT) {
        return SYS_ARCH_TIMEOUT;
      }
    } else {
      sys_arch_sem_wait(&mbox->not_empty, 0);
    }

    sys_arch_sem_wait(&mbox->mutex, 0);
  }

  if (msg != NULL) {
    LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_fetch: mbox %p msg %p\n", (void *)mbox, *msg));
    *msg = mbox->msgs[mbox->first % SYS_MBOX_SIZE];
  }
  else{
    LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_fetch: mbox %p, null msg\n", (void *)mbox));
  }

  mbox->first++;

  if (mbox->wait_send) {
    sys_sem_signal(&mbox->not_full);
  }

  sys_sem_signal(&mbox->mutex);

  return time_needed;
}
/*-----------------------------------------------------------------------------------*/
static struct sys_sem *
sys_sem_new_internal(u8_t count)
{
  struct sys_sem *sem;

  sem = (struct sys_sem *)__malloc(sizeof(struct sys_sem));
  if (sem != NULL) {
    semaphore_init(&(sem->sem), count);
  }
  return sem;
}
/*-----------------------------------------------------------------------------------*/
err_t
sys_sem_new(struct sys_sem **sem, u8_t count)
{
  SYS_STATS_INC_USED(sem);
  *sem = sys_sem_new_internal(count);
  if (*sem == NULL) {
    return ERR_MEM;
  }
  #ifdef DEBUG_LWIP_ARCH
  printk("%s: sem %x *sem %x\n", __func__, sem, *sem);
  #endif
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/

/**
 * sys_arch_sem_wait - 等待信号量 
 * @s: 信号量指针（指向信号量地址的指针）
 * @timeout: 超时的最大值（毫秒为单位）
 * 
 * 如果timeout为0，则函数会移植阻塞，直到信号量到来;
 * 如果超时不为0，则表示函数阻塞的最大毫秒数，在这种情况下，
 * 返回值为在该信号量上面等待的时间（毫秒），若在timeout内没有
 * 等待到信号量，则返回值为SYS_ARCH_TIMEOUT，若信号量在调用函数时
 * 已经可用，则函数不会发生任何阻塞，返回值为0
 */
u32_t
sys_arch_sem_wait(struct sys_sem **s, u32_t timeout)
{
  struct sys_sem *sem;
  LWIP_ASSERT("invalid sem", (s != NULL) && (*s != NULL));
  sem = *s;
  #ifdef DEBUG_LWIP_ARCH
  printk("%s: sem %x *sem %x timeout %x\n", __func__, s, sem, timeout);
  #endif
  clock_t wait_ticks;
  clock_t start, end;
  
  int err;
  /* 如果有信号可用，直接返回 */
  if (!semaphore_try_down(&sem->sem)) {
    return 0;
  }
  /* 将毫秒等待数转换成时钟滴答数 */
  wait_ticks = 0;
  if (timeout != 0) {
    wait_ticks = MSEC_TO_TICKS(timeout);
    if (wait_ticks < 1)
      wait_ticks = 1;
    else if (wait_ticks > 65535)
      wait_ticks = 65535;
  }
  start = sys_now();
  /* 尝试获取信号量 */
  err = semaphore_down_timeout(&sem->sem, wait_ticks);
  end = sys_now();
  if (err == 0)
    return (u32_t) (end - start);
  else 
    return SYS_ARCH_TIMEOUT;
}
/*-----------------------------------------------------------------------------------*/
void
sys_sem_signal(struct sys_sem **s)
{
  struct sys_sem *sem;
  LWIP_ASSERT("invalid sem", (s != NULL) && (*s != NULL));
  sem = *s;

  #ifdef DEBUG_LWIP_ARCH
  printk("%s: sem %x *sem %x \n", __func__, s, sem);
  #endif
  semaphore_up(&(sem->sem));
}
/*-----------------------------------------------------------------------------------*/
static void
sys_sem_free_internal(struct sys_sem *sem)
{
  semaphore_destroy(&sem->sem);
  __free(sem);
  sem = NULL;
}
/*-----------------------------------------------------------------------------------*/
void
sys_sem_free(struct sys_sem **sem)
{
  if ((sem != NULL) && (*sem != SYS_SEM_NULL)) {
    SYS_STATS_DEC(sem.used);
    #ifdef DEBUG_LWIP_ARCH
    printk("%s: sem %x *sem %x \n", __func__, sem, *sem);
    #endif
    sys_sem_free_internal(*sem);
    *sem = NULL;
  }
}
#endif /* !NO_SYS */
/*-----------------------------------------------------------------------------------*/
u32_t
sys_now(void)
{
#if 1    
  struct timeval tv;
  u32_t sec, usec, msec;
  sys_gettimeofday(&tv, NULL);

  sec = (u32_t)(tv.tv_sec - starttime.tv_sec);
  usec = (u32_t)(tv.tv_usec - starttime.tv_usec);
  msec = sec * 1000 + usec / 1000;
  return msec;
#else
  return sys_get_ticks();
#endif
}
/*-----------------------------------------------------------------------------------*/
void
sys_init(void)
{
  sys_gettimeofday(&starttime, NULL);
}
/*-----------------------------------------------------------------------------------*/
#if SYS_LIGHTWEIGHT_PROT
/** sys_prot_t sys_arch_protect(void)

This optional function does a "fast" critical region protection and returns
the previous protection level. This function is only called during very short
critical regions. An embedded system which supports ISR-based drivers might
want to implement this function by disabling interrupts. Task-based systems
might want to implement this by using a mutex or disabling tasking. This
function should support recursive calls from the same task or interrupt. In
other words, sys_arch_protect() could be called while already protected. In
that case the return value indicates that it is already protected.

sys_arch_protect() is only required if your port is supporting an operating
system.
*/
sys_prot_t
sys_arch_protect(void)
{
    /* Note that for the UNIX port, we are using a lightweight mutex, and our
     * own counter (which is locked by the mutex). The return code is not actually
     * used. */
    if (lwprot_thread != pthread_self())
    {
        /* We are locking the mutex where it has not been locked before *
        * or is being locked by another thread */
        pthread_mutex_lock(&lwprot_mutex);
        lwprot_thread = pthread_self();
        lwprot_count = 1;
    }
    else
        /* It is already locked by THIS thread */
        lwprot_count++;
    return 0;
}
/*-----------------------------------------------------------------------------------*/
/** void sys_arch_unprotect(sys_prot_t pval)

This optional function does a "fast" set of critical region protection to the
value specified by pval. See the documentation for sys_arch_protect() for
more information. This function is only required if your port is supporting
an operating system.
*/
void
sys_arch_unprotect(sys_prot_t pval)
{
    LWIP_UNUSED_ARG(pval);
    if (lwprot_thread == pthread_self())
    {
        if (--lwprot_count == 0)
        {
            lwprot_thread = (pthread_t) 0xDEAD;
            pthread_mutex_unlock(&lwprot_mutex);
        }
    }
}
#endif /* SYS_LIGHTWEIGHT_PROT */

/*-----------------------------------------------------------------------------------*/

#ifndef MAX_JIFFY_OFFSET
#define MAX_JIFFY_OFFSET ((~0U >> 1)-1)
#endif

#ifndef HZ
#define HZ 100
#endif

u32_t
sys_jiffies(void)
{
    struct timeval tv;
    unsigned long sec;
    long usec;

    sys_gettimeofday(&tv,NULL);
    sec = tv.tv_sec - starttime.tv_sec;
    usec = tv.tv_usec;

    if (sec >= (MAX_JIFFY_OFFSET / HZ))
      return MAX_JIFFY_OFFSET;
    usec += 1000000L / HZ - 1;
    usec /= 1000000L / HZ;
    return HZ * sec + usec;
}

#if PPP_DEBUG

#include <stdarg.h>

void ppp_trace(int level, const char *format, ...)
{
    va_list args;

    (void)level;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#endif
