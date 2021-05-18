#include <xbook/tests.h>
#include <xbook/debug.h>

#ifdef TIMER_TEST
#include <xbook/timer.h>
#include <arch/time.h>

static void timer_tests_func(timer_t *timer, void *arg)
{
    dbgprintln("[TEST] timer test: timer %d occur! %s", timer->id, (char *)arg);
    timer_modify(timer, 10);
    timer_add(timer);
}

void timer_tests()
{
    static timer_t timer;    
    timer_init(&timer, 10, "hello, timer!\n", timer_tests_func);
    timer_add(&timer);
}
#endif

#ifdef TASK_TEST
#include <xbook/task.h>
#include <xbook/schedule.h>

#define THREAD_YIELD_ENABLE

#ifndef THREAD_YIELD_ENABLE
int thread_i = 0;
#endif

void thread_a(void *arg)
{
    keprint("thread running. %s\n", (char *)arg);
    
    #if 0   /* 只有进程才能这么测试 */
    /* 进行内存映射 */
    page_map_addr(0x1000, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);

    // vmprint(GET_CUR_PGDIR(), 2);

    dbgprintln("vir=%p phy=%p\n", 0x1000, addr_vir2phy(0x1000));

    /* 尝试访问不存在的虚拟地址 */
    uint64_t *p = (uint64_t *) 0x1000;
    *p = 0x12345678ABCDEF;
    dbgprintln("DATA=%lx\n", *p);
    #endif
    task_t *cur = task_current;
    while (1)
    {
        #ifdef THREAD_YIELD_ENABLE
        infoprint("[%s] I am running...\n", cur->name);
        task_yield();
        #else
        thread_i++;
        if (thread_i % 0xffffff == 0) {
            infoprint("[%s] I am running...\n", cur->name);
            infoprint("[%s] interrupt state %d\n", cur->name, interrupt_enabled());
        }
        #endif
    }
}
#endif

void kernel_test_init()
{
    #ifdef TIMER_TEST
    timer_tests();
    #endif

    #ifdef TASK_TEST
    task_t *p = kern_thread_start("testA", TASK_PRIORITY_HIGH, thread_a, "hello, A!");
    if (!p)
        infoprint("create testA bad.\n");

    p = kern_thread_start("testB", TASK_PRIORITY_HIGH, thread_a, "hello, B!");
    if (!p)
        infoprint("create testB bad.\n");

    p = kern_thread_start("testC", TASK_PRIORITY_HIGH, thread_a, "hello, C!");
    if (!p)
        infoprint("create testC bad.\n");
    #endif
}