#include <environment/statusbar.h>
#include <sys/time.h>
#include <stdio.h>

#define TIME_READ_INTERVAL  (20 * HZ_PER_CLOCKS)

clock_t last_clock = 0;

void statusbar_time_read()
{
    /* 间隔一定时间再获取 */
    clock_t cur = clock();
    if (cur - last_clock >= TIME_READ_INTERVAL) {
        last_clock = cur;
        /* 读取时间，然后显示在时间上面 */
        ktime_t ktm;
        ktime(&ktm);
        /* 生成一个时间字符串 */
        char timestr[6] = {0};
        sprintf(timestr, "%d:%d", ktm.hour, ktm.minute);
        statusbar_manager.time_item->set_text(statusbar_manager.time_item, timestr);
    }
}
