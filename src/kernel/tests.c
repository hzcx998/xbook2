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

void kernel_test_init()
{
    #ifdef TIMER_TEST
    timer_tests();
    #endif
}