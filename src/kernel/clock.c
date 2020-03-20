#include <xbook/clock.h>
#include <xbook/assert.h>
#include <xbook/math.h>
#include <xbook/softirq.h>
#include <xbook/debug.h>
#include <xbook/softirq.h>
#include <arch/interrupt.h>

volatile clock_t systicks;

ktime_t ktime;

/* 每月对应的天数，2月的会在闰年是加1 */
const char month_day[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

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
 * sleep_by_ticks - 休眠ticks时间
 * @sleep_ticks: 要休眠多少个ticks
 */
static void sleep_by_ticks(unsigned long sleep_ticks)
{
	/* 获取起始ticks，后面进行差值运算 */
	unsigned long start_ticks = systicks;

	/* 如果最新ticks和开始ticks差值小于要休眠的ticks，就继续休眠 */
	while (systicks - start_ticks < sleep_ticks) {
		/* 让出cpu占用，相当于休眠 */
		//TaskYield();
        
        //CpuNop();
	}
}

/**
 
 * sys_msleep - 以毫秒为单位进行休眠
 */
void sys_msleep(unsigned long msecond)
{
	/* 把毫秒转换成ticks */
	unsigned long sleep_ticks = DIV_ROUND_UP(msecond, MS_PER_TICKS);
	ASSERT(sleep_ticks > 0);
	/* 进行休眠 */
	sleep_by_ticks(sleep_ticks);
}

/* 定时器软中断处理 */
void timer_softirq_handler(softirq_action_t *action)
{
	/* 改变系统时间 */
    if (systicks % HZ == 0) {  /* 1s更新一次 */
        /* 唤醒每秒时间工作 */
        update_ktime();
        //BlockDiskSync();
        //printk("<%d>", systicks);
    }
    
    /* 更新闹钟 */
    //UpdateAlarmSystem();

	/* 更新定时器 */
	//UpdateTimerSystem();
    //printk("s");
}

/* sched_softirq_handler - 调度程序软中断处理
 * @action: 中断行为
 * 
 * 在这里进行调度的抢占，也就是在这里面决定时间片轮训的时机。
 */
void sched_softirq_handler(softirq_action_t *action)
{
	#if 0
    struct Task *current = CurrentTask();

	/* 检测内核栈是否溢出 */
	ASSERT(current->stackMagic == TASK_STACK_MAGIC);
	/* 更新任务调度 */
	current->elapsedTicks++;
	
    /* 需要进行调度的时候才会去调度 */
	if (current->ticks <= 0) {
		ScheduleInClock();
	} else {
		
		current->ticks--;
	}
    #endif
}

/**
 * get_ktime - 获取获取内核时间
 * @time: 时间结构体
 */
void get_ktime(struct ktime *time)
{
    *time = ktime;
}
#if 0
/**
 * SysTime - 获取时间
 * @tm: 时间
 * 
 * 返回系统ticks
 */
unsigned long SysTime(struct tm *tm)
{
    if (tm) {
        /* 复制数据 */
        tm->tm_sec = ktime.second;
        tm->tm_min = ktime.minute;
        tm->tm_hour = ktime.hour;
        tm->tm_year = ktime.year - 1900;
        tm->tm_mon = ktime.month - 1;
        tm->tm_mday = ktime.day;
        tm->tm_wday = ktime.week_day;
        tm->tm_yday = ktime.year_day - 1;
        tm->tm_isdst = ktime.isDst;      /* 默认不是夏令时 */
    }

    return systicks;
}
#endif

/**
 * print_ktime - 打印系统时间
 */
void print_ktime()
{
	printk("Time:%d:%d:%d Date:%d/%d/%d\n", \
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
	printk("week_day:%d-%s year_day:%d\n", ktime.week_day, week_day[ktime.week_day], ktime.year_day);
}


/**
 * clock_handler - 时钟中断处理函数
 */
int clock_handler(unsigned long irq, unsigned long data)
{
    /* 改变ticks计数 */
	systicks++;
	
	/* 激活定时器软中断 */
	active_softirq(TIMER_SOFTIRQ);

	/* 激活调度器软中断 */
	active_softirq(SCHED_SOFTIRQ);
    return 0;
}

/**
 * init_clock - 初始化时钟系统
 * 多任务的运行依赖于此
 */
void init_clock()
{
    /* 初始化系统时间 */
    //用一个循环让秒相等
	do{
		ktime.year      = get_time_year();
		ktime.month     = get_time_month();
		ktime.day       = get_time_day();
		ktime.hour      = get_time_hour();
		ktime.minute    =  get_time_minute();
		ktime.second    = get_time_second();
		
		/* 转换成本地时间 */
		/* 自动转换时区 */
#if CONFIG_TIMEZONE_AUTO == 1
        if(ktime.hour >= 16){
			ktime.hour -= 16;
		}else{
			ktime.hour += 8;
		}
#endif /* CONFIG_TIMEZONE_AUTO */
	}while(ktime.second != get_time_second());

    ktime.week_day = get_week_day(ktime.year, ktime.month, ktime.day);
    ktime.year_day = get_year_days();    
    ktime.is_dst = 0;
    
    print_ktime();

	/* 初始化定时器 */
	//InitTimer();

    systicks = 0;
    
    /* 初始化时钟硬件 */
    init_clock_hardware();
	/* 注册定时器软中断处理 */
	build_softirq(TIMER_SOFTIRQ, timer_softirq_handler);
	/* 注册定时器软中断处理 */
	build_softirq(SCHED_SOFTIRQ, sched_softirq_handler);
	/* 注册时钟中断并打开中断 */	
	if (register_irq(IRQ0_CLOCK, &clock_handler, IRQF_DISABLED, "clockirq", "kclock", 0))
        printk("register failed!\n");

    enable_intr();
    printk("waiting...\n");
    
    while (1);
}
