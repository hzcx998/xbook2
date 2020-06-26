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
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void *__malloc(size_t size)
{
    void *addr = sbrk(0);
    if (sbrk(size) == NULL)
        return NULL;
    return addr;
}

static void __free(void *ptr)
{

}

#if 0
u32_t sys_now()
{
  return clock();
}
#endif

#define UMAX(a, b)      ((a) > (b) ? (a) : (b))

static struct timeval starttime;

#if !NO_SYS

#if SYS_LIGHTWEIGHT_PROT
static pthread_mutex_t lwprot_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t lwprot_thread = (pthread_t)0xDEAD;
static int lwprot_count = 0;
#endif /* SYS_LIGHTWEIGHT_PROT */

sys_thread_t
sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
  int code;
  pthread_t tmp;
  LWIP_UNUSED_ARG(name);
  LWIP_UNUSED_ARG(stacksize);
  LWIP_UNUSED_ARG(prio);

  code = pthread_create(&tmp,
                        NULL, 
                        (void *(*)(void *)) 
                        function, 
                        arg);
  
  if (0 != code) {
    LWIP_DEBUGF(SYS_DEBUG, ("sys_thread_new: pthread_create %d, thread = 0x%lx",
                       code, (unsigned long)tmp));
    abort();
  }
  
  return tmp;
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
  err_t err;
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_UNUSED_ARG(size);

  err = sys_sem_new(&(mbox->sem), 0);
  LWIP_ASSERT("Error creating semaphore", err == ERR_OK);
  if(err != ERR_OK) {
    return ERR_MEM;
  }
  err = sys_mutex_new(&(mbox->mutex));
  LWIP_ASSERT("Error creating mutex", err == ERR_OK);
  if(err != ERR_OK) {
  	sys_sem_free(&(mbox->sem));
    return ERR_MEM;
  }
  
  memset(&mbox->q_mem, 0, sizeof(void *)*MAX_QUEUE_ENTRIES);
  mbox->head = 0;
  mbox->tail = 0;
  mbox->msg_num = 0;
  
  return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
  /* parameter check */

  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  
  sys_sem_free(&(mbox->sem));
  sys_mutex_free(&(mbox->mutex));

  mbox->sem = NULL;
  mbox->mutex = NULL;
}

void sys_mbox_post(sys_mbox_t *q, void *msg)
{

  //SYS_ARCH_DECL_PROTECT(lev);

  /* parameter check */
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  //queue is full, we wait for some time
  while(q->msg_num >= MAX_QUEUE_ENTRIES)
  {
    //OSTimeDly(1);//sys_msleep(20);
    sys_msleep(1);
  }
  
  //SYS_ARCH_PROTECT(lev);
  sys_mutex_lock(&(q->mutex));
  if(q->msg_num >= MAX_QUEUE_ENTRIES)
  {
    LWIP_ASSERT("mbox post error, we can not handle it now, Just drop msg!", 0);
	//SYS_ARCH_UNPROTECT(lev);
	sys_mutex_unlock(&(q->mutex));
	return;
  }
  q->q_mem[q->head] = msg;
  (q->head)++;
  if (q->head >= MAX_QUEUE_ENTRIES) {
    q->head = 0;
  }

  q->msg_num++;
  if(q->msg_num == MAX_QUEUE_ENTRIES)
  {
    printf("mbox post, box full\n");
  }
  
  //Err = OSSemPost(q->sem);
  sys_sem_signal(&(q->sem));
  //if(Err != OS_ERR_NONE)
  //{
    //add error log here
  //  printf("[Sys_arch]:mbox post sem fail\n");
  //}

  //SYS_ARCH_UNPROTECT(lev);
  sys_mutex_unlock(&(q->mutex));
}

err_t sys_mbox_trypost(sys_mbox_t *q, void *msg)
{
  //SYS_ARCH_DECL_PROTECT(lev);

  /* parameter check */
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  //SYS_ARCH_PROTECT(lev);
  sys_mutex_lock(&(q->mutex));

  if (q->msg_num >= MAX_QUEUE_ENTRIES) {
    //SYS_ARCH_UNPROTECT(lev);
    sys_mutex_unlock(&(q->mutex));
	printf("[Sys_arch]:mbox try post mbox full\n");
    return ERR_MEM;
  }

  q->q_mem[q->head] = msg;
  (q->head)++;
  if (q->head >= MAX_QUEUE_ENTRIES) {
    q->head = 0;
  }

  q->msg_num++;
  if(q->msg_num == MAX_QUEUE_ENTRIES)
  {
    printf("mbox try post, box full\n");
  }
  
  //Err = OSSemPost(q->sem);
  sys_sem_signal(&(q->sem));
  //if(Err != OS_ERR_NONE)
  //{
    //add error log here
  //  printf("[Sys_arch]:mbox try post sem fail\n");
  //}
  //SYS_ARCH_UNPROTECT(lev);
  sys_mutex_unlock(&(q->mutex));
  return ERR_OK;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)
{
  u8_t Err;
  u32_t wait_ticks;
  u32_t start, end;
  u32_t tmp_num;
  //SYS_ARCH_DECL_PROTECT(lev);

  // parameter check 
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  //while(q->msg_num == 0)
  //{
  //  OSTimeDly(1);//sys_msleep(20);
  //}
  
  wait_ticks = 0;
  if(timeout!=0){
	 wait_ticks = (timeout * CLOCKS_PER_SEC)/1000;
	 if(wait_ticks < 1)
		wait_ticks = 1;
	 else if(wait_ticks > 65535)
			wait_ticks = 65535;
  }
  
  //start = sys_now();
  //OSSemPend(q->sem, (u16_t)wait_ticks, &Err);
  start = sys_arch_sem_wait(&(q->sem), timeout);
  //end = sys_now();

  if (start != SYS_ARCH_TIMEOUT)
  {
    //SYS_ARCH_PROTECT(lev);
    sys_mutex_lock(&(q->mutex));
	
	if(q->head == q->tail)
	{
        printf("mbox fetch queue abnormal [%u]\n", q->msg_num);
		if(msg != NULL) {
			*msg  = NULL;
	    }
		//SYS_ARCH_UNPROTECT(lev);
		sys_mutex_unlock(&(q->mutex));
		return SYS_ARCH_TIMEOUT;
	}
	
    if(msg != NULL) {
      *msg  = q->q_mem[q->tail];
    }

    (q->tail)++;
    if (q->tail >= MAX_QUEUE_ENTRIES) {
      q->tail = 0;
    }

	if(q->msg_num > 0)
	{
      q->msg_num--;
	}
	else
	{
      printf("mbox fetch queue error [%u]\n", q->msg_num);
	}

	tmp_num = (q->head >= q->tail)?(q->head - q->tail):(MAX_QUEUE_ENTRIES + q->head - q->tail);

	if(tmp_num != q->msg_num)
	{
        printf("mbox fetch error, umatch [%u] with tmp [%u]\n", q->msg_num, tmp_num);
	}
	
	//SYS_ARCH_UNPROTECT(lev);
	sys_mutex_unlock(&(q->mutex));
	//printf("mbox fetch ok, match [%u] with tmp [%u] \n", q->msg_num, tmp_num);
	//return (u32_t)(end - start);		//���ȴ�ʱ������Ϊtimeout/2;
	return start;
  }
  else
  {
    //printf("mbox fetch time out error");
    if(msg != NULL) {
      *msg  = NULL;
    }
	
	return SYS_ARCH_TIMEOUT;
  }
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)
{
  u32_t tmp_num;
  //SYS_ARCH_DECL_PROTECT(lev);
  u32_t start;
  /* parameter check */
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  if(q->msg_num == 0)
  	return SYS_MBOX_EMPTY;
  
  start = sys_arch_sem_wait(&(q->sem), 1);
  
  if (start != SYS_ARCH_TIMEOUT) {
    //SYS_ARCH_PROTECT(lev);
    sys_mutex_lock(&(q->mutex));
	if(q->head == q->tail)
	{
        printf("mbox tryfetch queue abnormal [%u]\n", q->msg_num);
		if(msg != NULL) {
			*msg  = NULL;
	    }
		//SYS_ARCH_UNPROTECT(lev);
		sys_mutex_unlock(&(q->mutex));
		return SYS_MBOX_EMPTY;
	}
		
    if(msg != NULL) {
      *msg  = q->q_mem[q->tail];
    }

    (q->tail)++;
    if (q->tail >= MAX_QUEUE_ENTRIES) {
      q->tail = 0;
    }

    if(q->msg_num > 0)
	{
      q->msg_num--;
	}

	tmp_num = (q->head >= q->tail)?(q->head - q->tail):(MAX_QUEUE_ENTRIES + q->head - q->tail);
    
	
	if(tmp_num != q->msg_num)
	{
        printf("mbox try fetch error, umatch [%u] with tmp [%u]\n", q->msg_num, tmp_num);
	}
	
    //SYS_ARCH_UNPROTECT(lev);
    sys_mutex_unlock(&(q->mutex));
    return 0;
  }
  else
  {
    printf("mbox try fetch uknow error\n");
    if(msg != NULL) {
      *msg  = NULL;
    }

    return SYS_MBOX_EMPTY;
  }
}

#if 0
/*-----------------------------------------------------------------------------------*/
static struct sys_sem *
sys_sem_new_internal(u8_t count)
{
  struct sys_sem *sem;

  sem = (struct sys_sem *)malloc(sizeof(struct sys_sem));
  if (sem != NULL) {
    sem->c = count;
    pthread_cond_init(&(sem->cond), NULL);
    pthread_mutex_init(&(sem->mutex), NULL);
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
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static u32_t
cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex, u32_t timeout)
{
  time_t tdiff;
  time_t sec, usec;
  struct timeval rtime1, rtime2;
  struct timespec ts;
  int retval;

  if (timeout > 0) {
    /* Get a timestamp and add the timeout value. */
    gettimeofday(&rtime1, NULL);
    sec = rtime1.tv_sec;
    usec = rtime1.tv_usec;
    usec += timeout % 1000 * 1000;
    sec += (int)(timeout / 1000) + (int)(usec / 1000000);
    usec = usec % 1000000;
    ts.tv_nsec = usec * 1000;
    ts.tv_sec = sec;

    retval = pthread_cond_timedwait(cond, mutex, &ts);

    if (retval == ETIMEDOUT) {
      return SYS_ARCH_TIMEOUT;
    } else {
      /* Calculate for how long we waited for the cond. */
      gettimeofday(&rtime2, NULL);
      tdiff = (rtime2.tv_sec - rtime1.tv_sec) * 1000 +
        (rtime2.tv_usec - rtime1.tv_usec) / 1000;

      if (tdiff <= 0) {
        return 0;
      }
      return (u32_t)tdiff;
    }
  } else {
    pthread_cond_wait(cond, mutex);
    return 0;
  }
}
/*-----------------------------------------------------------------------------------*/
u32_t
sys_arch_sem_wait(struct sys_sem **s, u32_t timeout)
{
  u32_t time_needed = 0;
  struct sys_sem *sem;
  LWIP_ASSERT("invalid sem", (s != NULL) && (*s != NULL));
  sem = *s;

  pthread_mutex_lock(&(sem->mutex));
  while (sem->c <= 0) {
    if (timeout > 0) {
      time_needed = cond_wait(&(sem->cond), &(sem->mutex), timeout);

      if (time_needed == SYS_ARCH_TIMEOUT) {
        pthread_mutex_unlock(&(sem->mutex));
        return SYS_ARCH_TIMEOUT;
      }
      /*      pthread_mutex_unlock(&(sem->mutex));
              return time_needed; */
    } else {
      cond_wait(&(sem->cond), &(sem->mutex), 0);
    }
  }
  sem->c--;
  pthread_mutex_unlock(&(sem->mutex));
  return (u32_t)time_needed;
}
/*-----------------------------------------------------------------------------------*/
void
sys_sem_signal(struct sys_sem **s)
{
  struct sys_sem *sem;
  LWIP_ASSERT("invalid sem", (s != NULL) && (*s != NULL));
  sem = *s;

  pthread_mutex_lock(&(sem->mutex));
  sem->c++;

  if (sem->c > 1) {
    sem->c = 1;
  }

  pthread_cond_broadcast(&(sem->cond));
  pthread_mutex_unlock(&(sem->mutex));
}
/*-----------------------------------------------------------------------------------*/
static void
sys_sem_free_internal(struct sys_sem *sem)
{
  pthread_cond_destroy(&(sem->cond));
  pthread_mutex_destroy(&(sem->mutex));
  free(sem);
}
/*-----------------------------------------------------------------------------------*/
void
sys_sem_free(struct sys_sem **sem)
{
  if ((sem != NULL) && (*sem != SYS_SEM_NULL)) {
    SYS_STATS_DEC(sem.used);
    sys_sem_free_internal(*sem);
  }
}
#endif


err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
  sem_t *new_sem = NULL;

  LWIP_ASSERT("[Sys_arch]sem != NULL", sem != NULL);

    new_sem = __malloc(sizeof(sem_t));
    if (new_sem == NULL) {
        printf("sem alloc failed!\n");
        return ERR_MEM;
    }
    if (sem_init(new_sem, 0, (u16_t)count) < 0) {
        printf("sem init failed!\n");
        __free(new_sem);
        return ERR_MEM;
    }

  //new_sem = OSSemCreate((u16_t)count);
  LWIP_ASSERT("[Sys_arch]Error creating sem", new_sem != NULL);
  if(new_sem != NULL) {
    *sem = (void *)new_sem;
    return ERR_OK;
  }
   __free(new_sem);
  *sem = SYS_SEM_NULL;
  return ERR_MEM;
}

void sys_sem_free(sys_sem_t *sem)
{
  //u8_t Err;
  // parameter check 
  LWIP_ASSERT("sem != NULL", sem != NULL);
  
    
  //OSSemDel(*sem, OS_DEL_ALWAYS, &Err);
#if 0	
  if(Err != OS_ERR_NONE)
  {
    //add error log here
    printf("[Sys_arch]free sem fail\n");
  }
#endif

    if (sem_destroy(*sem) < 0) {
        printf("[Sys_arch]free sem fail\n");
        return;
    }

  *sem = NULL;
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  //u8_t Err;
  u32_t wait_ticks;
  u32_t start, end;
  LWIP_ASSERT("sem != NULL", sem != NULL);

  if (!sem_trywait(*sem))		  // ����Ѿ��յ�, �򷵻�0 
  {
	  //printf("debug:sem accept ok\n");
	  return 0;
  }   
  
  wait_ticks = 0;
  if(timeout!=0){
	 wait_ticks = (timeout * CLOCKS_PER_SEC)/1000;
	 if(wait_ticks < 1)
		wait_ticks = 1;
	 else if(wait_ticks > 65535)
			wait_ticks = 65535;
  }

  start = sys_now();
  int ret = sem_timedwait2(*sem, wait_ticks * (1000 / CLOCKS_PER_SEC));
  //OSSemPend(*sem, (u16_t)wait_ticks, &Err);
  end = sys_now();
  
  if (ret == ETIMEDOUT)
    return SYS_ARCH_TIMEOUT;
  else
    return (u32_t)(end - start);

  #if 0
  if (Err == OS_NO_ERR)
		return (u32_t)(end - start);		//���ȴ�ʱ������Ϊtimeout/2
  else
		return SYS_ARCH_TIMEOUT;
  #endif
}

void sys_sem_signal(sys_sem_t *sem)
{
  //u8_t Err;
  LWIP_ASSERT("sem != NULL", sem != NULL);
#if 0
  Err = OSSemPost(*sem);
  if(Err != OS_ERR_NONE)
  {
        //add error log here
        printf("[Sys_arch]:signal sem fail\n");
  }
#endif

    int ret = sem_post(*sem); 
    if (ret != 0)
        printf("[Sys_arch]:signal sem fail %d\n", ret);

  LWIP_ASSERT("Error releasing semaphore", ret == 0);
}

#endif /* !NO_SYS */
/*-----------------------------------------------------------------------------------*/
u32_t
sys_now(void)
{
    #if 0
  struct timeval tv;
  u32_t sec, usec, msec;
  gettimeofday(&tv, NULL);

  sec = (u32_t)(tv.tv_sec - starttime.tv_sec);
  usec = (u32_t)(tv.tv_usec - starttime.tv_usec);
  msec = sec * 1000 + usec / 1000;
#endif

  return clock();
  //return msec;
}
/*-----------------------------------------------------------------------------------*/
void
sys_init(void)
{
    //printf("[Sys_arch] init ok");
  gettimeofday(&starttime, NULL);
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
#if 0
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

    gettimeofday(&tv,NULL);
    sec = tv.tv_sec - starttime.tv_sec;
    usec = tv.tv_usec;

    if (sec >= (MAX_JIFFY_OFFSET / HZ))
      return MAX_JIFFY_OFFSET;
    usec += 1000000L / HZ - 1;
    usec /= 1000000L / HZ;
    return HZ * sec + usec;
}
#endif
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
