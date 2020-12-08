#include <xbook/walltime.h>
#include <xbook/clock.h>
#include <xbook/schedule.h>
#include <xbook/debug.h>
#include <xbook/safety.h>

walltime_t walltime;
const char month_day[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

int walltime_is_leap_year(int year)
{
	if (year % 4) return 0;
    if (year % 400) return 1;
    if (year % 100) return 0;
    return 1;
}

static int walltime_get_week_day(int year, int month, int day)
{
    int c, y, week;
    if (month == 1 || month == 2) {
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

static int walltime_get_month_day()
{
    if (walltime.month == 2) {
        return month_day[walltime.month] + walltime_is_leap_year(walltime.year);
    } else {
        return month_day[walltime.month];
    }
}

static int walltime_get_year_days()
{
    int i;
    int sum = 0;
    for (i = 1; i < walltime.month; i++) {
        if (i == 2)
            sum += month_day[i] + walltime_is_leap_year(walltime.year);
        else
            sum += month_day[i];
        
    }
    sum += walltime.day;
    if (walltime.month >= 2) {
        if (walltime.month == 2) {
            if (walltime.day == 28)
                sum += walltime_is_leap_year(walltime.year);    
        } else {
            sum += walltime_is_leap_year(walltime.year);
        }
    }
    return sum;
}

void walltime_update_second()
{
	walltime.second++;
	if(walltime.second > 59){
		walltime.minute++;
		walltime.second = 0;
		if(walltime.minute > 59){
			walltime.hour++;
			walltime.minute = 0;
			if(walltime.hour > 23){
				walltime.day++;
				walltime.hour = 0;
                walltime.week_day++;
                if (walltime.week_day > 6)
                    walltime.week_day = 0;
                walltime.year_day = walltime_get_year_days();
				if(walltime.day > walltime_get_month_day()){
					walltime.month++;
					walltime.day = 1;
					if(walltime.month > 12){
						walltime.year++;
						walltime.month = 1;
					}
				}
			}
		}
	}
}

int sys_get_walltime(walltime_t *wt)
{
    walltime_t tmp = walltime;
    --tmp.month;
    return mem_copy_to_user(wt, &tmp, sizeof(walltime_t));
}

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

long walltime_make_timestamp(walltime_t *wt)
{
	long res;
	int year;

	year = wt->year - 70;
    /* magic offsets (y+1) needed to get leapyears right.*/
	res = YEAR*year + DAY*((year+1)/4);
	res += month[wt->month];
    /* and (y+2) here. If it wasn't a leap-year, we have to adjust */
	if (wt->month>1 && ((year+2)%4))
		res -= DAY;
	res += DAY*(wt->day-1);
	res += HOUR*wt->hour;
	res += MINUTE*wt->minute;
	res += wt->second;
	return res;
}

void walltime_printf()
{
	kprint(PRINT_INFO "time:%d:%d:%d date:%d/%d/%d\n", \
		walltime.hour, walltime.minute, walltime.second,\
		walltime.year, walltime.month, walltime.day);
	char *week_day[7];
	week_day[0] = "Sunday";
    week_day[1] = "Monday";
	week_day[2] = "Tuesday";
	week_day[3] = "Wednesday";
	week_day[4] = "Thursday";
	week_day[5] = "Friday";
	week_day[6] = "Saturday";
	kprint(PRINT_INFO "week day:%d %s year day:%d\n", walltime.week_day, week_day[walltime.week_day], walltime.year_day);
}

void walltime_init()
{
    /* 初始化系统时间 */
    //用一个循环让秒相等
	do{
		walltime.year      = time_get_year();
		walltime.month     = time_get_month();
		walltime.day       = time_get_day();
		walltime.hour      = time_get_hour();
		walltime.minute    =  time_get_minute();
		walltime.second    = time_get_second();
		
		/* 转换成本地时间 */
		/* 自动转换时区 */
#ifdef CONFIG_TIMEZONE_AUTO
        if(walltime.hour >= 16){
			walltime.hour -= 16;
		}else{
			walltime.hour += 8;
		}
#endif /* CONFIG_TIMEZONE_AUTO */
	}while(walltime.second != time_get_second());

    walltime.week_day = walltime_get_week_day(walltime.year, walltime.month, walltime.day);
    walltime.year_day = walltime_get_year_days();

    walltime_printf();
}