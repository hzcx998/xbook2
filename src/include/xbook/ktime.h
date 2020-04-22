#ifndef _XBOOK_KTIME_H
#define _XBOOK_KTIME_H

#include <arch/time.h>
#include <sys/ktime.h>

extern ktime_t ktime;
void update_ktime();

void print_ktime();

void sys_get_ktime(ktime_t *time);

void init_ktime();


static inline unsigned long ktime2uint32()
{
	unsigned short date = KTIME_DATE(ktime.year, ktime.month, ktime.day);
	unsigned short time = KTIME_TIME(ktime.hour, ktime.minute, ktime.second);
	return (date << 16) | time;
}

#endif   /* _XBOOK_KTIME_H */
