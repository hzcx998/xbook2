#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/time.h>

/** 定义每月的天数 */
const char monthDay[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

/* 定义每月的名字 */
static const char *month_name[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

/**
 * isLeap - 判断是否是闰年 
 * @year: 传入的年份 
 * 
 * 四年一闰，百年不闰，四百年再闰
 */
int isLeap(int year)
{
	if(year % 4) return 0;
    if(year % 400) return 1;
    if(year % 100) return 0;
    return 1;
}

/**
 * getWeek - 返回某天是周几 
 * @year: 年
 * @month: 月
 * @day: 日
 * 
 * 返回周几（0-6） 
 */
int getWeek(int year, int month, int day)
{
    int c, y, week;
    if(month == 1 || month == 2) //判断month是否为1或2　
    {
        year--;
        month+=12;
    }
    c= year / 100;
    y = year - c * 100;
    week = (c / 4) - 2 * c + (y + y / 4) + (13 * (month + 1) / 5) + day - 1;
    while(week < 0) {week += 7;}
    week %= 7;
    return week;
}
 
/**
 * display - 显示出来 
 * @year: 年
 @ month：月 
 * 
 */
void display(int year, int month)
{
    int monthDays, weekFirst, i;
    monthDays = monthDay[month] + (month==2 ? isLeap(year) : 0);
    weekFirst = getWeek(year, month, 1);
    printf("      %10s %d  \n", month_name[month - 1], year);
    printf("  Su  Mo  Tu  We  Th  Fr  Sa\n");
    for(i=0; i<weekFirst; i++) printf("    ");
    for(i=1; i<=monthDays; i++)
    {
        printf("%4d", i);
        weekFirst++;
        if(weekFirst>=7) {printf("\n"); weekFirst=0;}
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    int year, month;
    walltime_t wt;

    /* 解析参数 */
    if (argc == 1) {    /* 只有一个参数，就直接显示当前时间的日历 */
        walltime(&wt);
        year = wt.year;
        month = wt.month;
        month++; // 0~11 -> 1~12
        display(year, month);
    } else {
        /* 参数：
            -d year-mon 
            year
        */
        char *p = (char *)argv[1];
        char *q;
        /* 如果是可选参数 */
        if (*p == '-') {
            p++;
            switch (*p)
            {
            case 'd':   /* date: -d year-mon */
                /* 解析下一个参数：期待值是year-mon */
                if (argc < 3) {
                    printf("cal: -d format must be year-month! example: -d 2020-2 \n");
                    return -1;
                }

                p = (char *)argv[2];
                q = strchr(p, '-');
                if (q == NULL) {
                    printf("cal: -d format must be year-month! example: -d 2020-2 \n");
                }
                *q++ = '\0';  /* 将'-'设置为0，当做字符串结尾 */
                
                /* 判断年是否正确 */
                if (isdigitstr(p)) {
                    year = atoi(p);
                    if (year < 0) {
                        printf("cal: year must > 0!\n");
                        return -1;
                    }
                } else {
                    printf("cal: not a right year!\n");
                    return -1;
                }

                /* 判断年是否正确 */
                if (isdigitstr(q)) {
                    month = atoi(q);
                    if (month < 1 || month > 12) {
                        printf("cal: month must in [1-12]!\n");
                        return -1;
                    }
                } else {
                    printf("cal: not a right month!\n");
                    return -1;
                }

                /* 开始执行 */
                display(year, month);
                break;
            case 'h':   /* 帮助参数 */
                printf("Usage: cal [option] [year-month]...\n");
                printf("Option:\n");
                printf("  -d    Get a month of a year. Example: cal -d 2020-2 \n");
                printf("  -h    Get help of cal. Example: cal -h \n");
                printf("  [year]  Get calender of year. Example: cal 2020 \n");
                printf("Note: If no arguments, will get current date.\n");
                
                break;
            default:
                printf("cal: no option after - !\n");
                break;
            }
        } else {    /* 没有参数，可能是year */
            /* 判断是否是数字 */
            if (isdigitstr(p)) {
                year = atoi(p);
                if (year < 0) {
                    printf("cal: year must > 0!\n");
                    return -1;
                }
                /* 输出这一年的每一个月 */
                for (month = 1; month <= 12; month++) {
                    display(year, month);
                }
            } else {
                printf("cal: not a right year!\n");
                return -1;
            }
        }
    }
    /* 执行成功后到这里来换行 */
    //printf("\n");
    return 0; 
}