#include <xbook/ktime.h>
#include <xbook/clock.h>
#include <xbook/schedule.h>
#include <xbook/debug.h>

ktime_t ktime;
/* 每月对应的天数，2月的会在闰年是加1 */
const char month_day[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

void loop_delay(int t)
{
    long i, j;
    for (i = 0; i < 100 * t; i++)
        for (j = 0; j < 10000; j++);
}

/**
 * is_leap - 判断是否是闰年 
 * @year: 传入的年份 
 * 
 * 四年一闰，百年不闰，四百年再闰
 */
int is_leap(int year)
{
	if (year % 4) return 0;
    if (year % 400) return 1;
    if (year % 100) return 0;
    return 1;
}

/**
 * get_week_day - 返回某天是周几 
 * @year: 年
 * @month: 月
 * @day: 日
 * 
 * 返回周几（0-6） 
 */
static int get_week_day(int year, int month, int day)
{
    int c, y, week;
    if (month == 1 || month == 2) { //判断month是否为1或2　
        year--;
        month+=12;
    }
    c = year / 100;
    y = year - c * 100;
    week = (c / 4) - 2 * c + (y + y / 4) + (13 * (month + 1) / 5) + day - 1;
    while(week < 0) 
        week += 7;
    week %= 7;
    return week;
}

static int get_month_day()
{
    /* 如果是2月，就需要判断是否是闰年 */
    if (ktime.month == 2) {
        /* 如果是闰年就要加1 */
        return month_day[ktime.month] + is_leap(ktime.year);
    } else {
        return month_day[ktime.month];
    }
}

static int get_year_days()
{
    int i;
    int sum = 0;
    /* 计算前几个月的日数 */
    for (i = 1; i < ktime.month; i++) {
        if (i == 2)   /* 对于2月份，都要判断闰年 */
            sum += month_day[i] + is_leap(ktime.year);
        else
            sum += month_day[i];
        
    }
    /* 计算当前月的日数 */
    sum += ktime.day;

    /* 如果是2月以上的，才进行闰年检测 */
    if (ktime.month >= 2) {
        /* 2月只有day是28的时候，才进行检测 */
        if (ktime.month == 2) {
            if (ktime.day == 28)
                sum += is_leap(ktime.year);    
        } else {
            sum += is_leap(ktime.year);
        }
    }

    return sum;
}

/**
 * update_ktime - 改变系统的时间
*/
void update_ktime()
{
	ktime.second++;
	if(ktime.second > 59){
		ktime.minute++;
		ktime.second = 0;
		if(ktime.minute > 59){
			ktime.hour++;
			ktime.minute = 0;
			if(ktime.hour > 23){
				ktime.day++;
				ktime.hour = 0;
                
                ktime.week_day++;
                /* 如果大于1-6是周一到周六，0是周日 */
                if (ktime.week_day > 6)
                    ktime.week_day = 0;

                /* 获取是今年的多少个日子 */
                ktime.year_day = get_year_days();

				//现在开始就要判断平年和闰年，每个月的月数之类的
				if(ktime.day > get_month_day()){
					ktime.month++;
					ktime.day = 1;
					if(ktime.month > 12){
						ktime.year++;	//到年之后就不用调整了
						ktime.month = 1;
					}
				}
			}
		}
	}
}

/**
 * sys_get_ktime - 获取获取内核时间
 * @time: 时间结构体
 */
void sys_get_ktime(ktime_t *time)
{
    *time = ktime;
    --time->month;
}

/* 这个算法来自linux内核 */
#define MINUTE 60
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define YEAR (365*DAY)

/* interestingly, we assume leap-years */
static int month[12] = {
	0,
	DAY*(31),
	DAY*(31+29),
	DAY*(31+29+31),
	DAY*(31+29+31+30),
	DAY*(31+29+31+30+31),
	DAY*(31+29+31+30+31+30),
	DAY*(31+29+31+30+31+30+31),
	DAY*(31+29+31+30+31+30+31+31),
	DAY*(31+29+31+30+31+30+31+31+30),
	DAY*(31+29+31+30+31+30+31+31+30+31),
	DAY*(31+29+31+30+31+30+31+31+30+31+30)
};

/**
 * make_timestamp - 生成时间戳
 * @tm: 时间结构
 * 
 * 返回时间戳，单位是秒(s)
 */
long make_timestamp(ktime_t * tm)
{
	long res;
	int year;

	year = tm->year - 70;
/* magic offsets (y+1) needed to get leapyears right.*/
	res = YEAR*year + DAY*((year+1)/4);
	res += month[tm->month];
/* and (y+2) here. If it wasn't a leap-year, we have to adjust */
	if (tm->month>1 && ((year+2)%4))
		res -= DAY;
	res += DAY*(tm->day-1);
	res += HOUR*tm->hour;
	res += MINUTE*tm->minute;
	res += tm->second;
	return res;
}

/**
 * sys_gettimeofday - 获取时间
 * @tv: 时间
 * @tz: 时区
 * 
 * 成功返回0，失败返回-1
 */
int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    if (!tv && !tv)
        return -1;

    if (tv) {
        tv->tv_sec = make_timestamp(&ktime);                    /* 生成秒数 */
        tv->tv_usec = ((systicks % HZ) * MS_PER_TICKS) * 1000;  /* 微秒 */
    }
    if (tz) {
        tz->tz_dsttime = 0;
        tz->tz_minuteswest = 0;
    }
    return 0;
}

int sys_clock_gettime(clockid_t clockid, struct timespec *ts)
{
    if (!ts)
        return -1; 
    switch (clockid)
    {
    case CLOCK_REALTIME:        /* 系统统当前时间，从1970年1.1日算起 */
        ts->tv_sec = make_timestamp(&ktime);                        /* 生成秒数 */
        ts->tv_nsec = ((systicks % HZ) * MS_PER_TICKS) * 1000000;   /* 纳秒 */
        break;
    case CLOCK_MONOTONIC:       /*系统的启动时间，不能被设置*/
        ts->tv_sec = (systicks / HZ);                               /* 生成秒数 */
        ts->tv_nsec = ((systicks % HZ) * MS_PER_TICKS) * 1000000;   /* 纳秒 */
        break;
    case CLOCK_PROCESS_CPUTIME_ID:  /* 本进程运行时间*/
        ts->tv_sec = current_task->elapsed_ticks / HZ;                        /* 生成秒数 */
        ts->tv_nsec = ((current_task->elapsed_ticks % HZ) * MS_PER_TICKS) * 1000000;   /* 纳秒 */
        break;
    case CLOCK_THREAD_CPUTIME_ID:   /*本线程运行时间*/
        ts->tv_sec = current_task->elapsed_ticks / HZ;                        /* 生成秒数 */
        ts->tv_nsec = ((current_task->elapsed_ticks % HZ) * MS_PER_TICKS) * 1000000;   /* 纳秒 */
        break;
    default:
        return -1;
    }
    return 0;
}

/**
 * 转换的ticks值得最大值
 */
#define MAX_SYSTICKS_VALUE  ((~0UL >> 1) -1)

/**
 * 将timeval格式的时间转换成systicks
 */
unsigned long timeval_to_systicks(struct timeval *tv)
{
    unsigned long sec = tv->tv_sec;
    unsigned long usec = tv->tv_usec;
    if (sec >= (MAX_SYSTICKS_VALUE / HZ)) /* 超过秒的ticks的最大值，就直接返回最大值 */
        return MAX_SYSTICKS_VALUE;
    //usec -= 1000000L / HZ - 1;  /* 向下对齐微妙秒数 */
    usec /= 1000000L / HZ;  /* 秒/HZ=1秒的ticks数 */
    return HZ * sec + usec;
}

/**
 * 将systicks转换成timeval格式的时间
 */
void systicks_to_timeval(unsigned long ticks, struct timeval *tv)
{
    unsigned long sec = ticks / HZ;     /* 获取秒数 */
    unsigned long usec = (ticks % HZ) * MS_PER_TICKS;    /* 获取毫秒数 */
    usec *= 1000L;      /* 获取微秒数 */
    if (sec >= (MAX_SYSTICKS_VALUE / HZ)) /* 超过秒的ticks的最大值，就修复它 */
        sec = MAX_SYSTICKS_VALUE;
    tv->tv_sec = sec;
    tv->tv_usec = usec;
}


/**
 * 将timespec格式的时间转换成systicks
 */
unsigned long timespec_to_systicks(struct timespec *ts)
{
    unsigned long sec = ts->tv_sec;
    unsigned long nsec = ts->tv_nsec;
    if (sec >= (MAX_SYSTICKS_VALUE / HZ)) /* 超过秒的ticks的最大值，就直接返回最大值 */
        return MAX_SYSTICKS_VALUE;
    //nsec -= 1000000000L / HZ - 1;  /* 向下对齐纳妙秒数 */
    nsec /= 1000000000L / HZ;  /* 秒/HZ=1秒的ticks数 */
    return HZ * sec + nsec;
}

/**
 * 将systicks转换成timespec格式的时间
 */
void systicks_to_timespec(unsigned long ticks, struct timespec *ts)
{
    unsigned long sec = ticks / HZ;     /* 获取秒数 */
    unsigned long nsec = (ticks % HZ) * MS_PER_TICKS;    /* 获取毫秒数 */
    nsec *= 1000000L;      /* 获取纳秒数 */
    if (sec >= (MAX_SYSTICKS_VALUE / HZ)) /* 超过秒的ticks的最大值，就修复它 */
        sec = MAX_SYSTICKS_VALUE;
    ts->tv_sec = sec;
    ts->tv_nsec = nsec;
}

/**
 * print_ktime - 打印系统时间
 */
void print_ktime()
{
	printk(KERN_INFO "time:%d:%d:%d date:%d/%d/%d\n", \
		ktime.hour, ktime.minute, ktime.second,\
		ktime.year, ktime.month, ktime.day);
	
	char *week_day[7];
	week_day[0] = "Sunday";
    week_day[1] = "Monday";
	week_day[2] = "Tuesday";
	week_day[3] = "Wednesday";
	week_day[4] = "Thursday";
	week_day[5] = "Friday";
	week_day[6] = "Saturday";
	printk(KERN_INFO "week day:%d %s year day:%d\n", ktime.week_day, week_day[ktime.week_day], ktime.year_day);
}

void init_ktime()
{
    /* 初始化系统时间 */
    //用一个循环让秒相等
	do{
		ktime.year      = time_get_year();
		ktime.month     = time_get_month();
		ktime.day       = time_get_day();
		ktime.hour      = time_get_hour();
		ktime.minute    =  time_get_minute();
		ktime.second    = time_get_second();
		
		/* 转换成本地时间 */
		/* 自动转换时区 */
#ifdef CONFIG_TIMEZONE_AUTO
        if(ktime.hour >= 16){
			ktime.hour -= 16;
		}else{
			ktime.hour += 8;
		}
#endif /* CONFIG_TIMEZONE_AUTO */
	}while(ktime.second != time_get_second());

    ktime.week_day = get_week_day(ktime.year, ktime.month, ktime.day);
    ktime.year_day = get_year_days();

    print_ktime();
}