#ifndef _SYS_KTIME_H
#define _SYS_KTIME_H


/* 时间和数据互相转换 */
#define KTIME_TIME(hou, min, sec) ((unsigned short)(((hou & 0x1f) << 11) | \
        ((min & 0x3f) << 5) | ((sec / 2) & 0x1f)))

#define KTIME_HOUR(data)    ((unsigned int)((data >> 11) & 0x1f))
#define KTIME_MINUTE(data)  ((unsigned int)((data >> 5) & 0x3f))
#define KTIME_SECOND(data)  ((unsigned int)((data & 0x1f) * 2))

/* 日期和数据互相转换 */
#define KTIME_DATE(yea, mon, day) ((unsigned short)((((yea - 1980) & 0x7f) << 9) | \
        ((mon & 0xf) << 5) | (day & 0x1f)))

#define KTIME_YEAR(data)    ((unsigned int)(((data >> 9) & 0x7f) + 1980))
#define KTIME_MONTH(data)   ((unsigned int)((data >> 5) & 0xf))
#define KTIME_DAY(data)     ((unsigned int)(data & 0x1f))

/**
 * 时间转换：
 * 1秒（s） = 1000毫秒（ms）
 * 1毫秒（ms） = 1000微秒（us）
 * 1微秒（us）= 1000纳秒（ns）
 * 1纳秒（ns）= 1000皮秒（ps）
 */
typedef struct ktime {
	int second;         /* [0-59] */
	int minute;         /* [0-59] */
	int hour;           /* [0-23] */
	int day;            /* [1-31] */
	int month;          /* [1-12] */
	int year;           /* year */
	int week_day;        /* [0-6] */
	int year_day;        /**/
} ktime_t;

#endif   /* _SYS_KTIME_H */