#include <xbook/ktime.h>

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

    print_ktime();
}