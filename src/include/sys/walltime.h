#ifndef _SYS_WALLTIME_H
#define _SYS_WALLTIME_H
/* WR: write RD: read */
#define WTM_WR_TIME(hou, min, sec) ((unsigned short)(((hou & 0x1f) << 11) | \
        ((min & 0x3f) << 5) | ((sec / 2) & 0x1f)))
#define WTM_WR_DATE(yea, mon, day) ((unsigned short)((((yea - 1980) & 0x7f) << 9) | \
        ((mon & 0xf) << 5) | (day & 0x1f)))

#define WTM_RD_HOUR(data)    ((unsigned int)((data >> 11) & 0x1f))
#define WTM_RD_MINUTE(data)  ((unsigned int)((data >> 5) & 0x3f))
#define WTM_RD_SECOND(data)  ((unsigned int)((data & 0x1f) * 2))
#define WTM_RD_YEAR(data)    ((unsigned int)(((data >> 9) & 0x7f) + 1980))
#define WTM_RD_MONTH(data)   ((unsigned int)((data >> 5) & 0xf))
#define WTM_RD_DAY(data)     ((unsigned int)(data & 0x1f))

typedef struct {
	int second;         /* [0-59] */
	int minute;         /* [0-59] */
	int hour;           /* [0-23] */
	int day;            /* [1-31] */
	int month;          /* [1-12] */
	int year;           /* year */
	int week_day;       /* [0-6] */
	int year_day;       /* [0-366] */
} walltime_t;

#endif   /* _SYS_WALLTIME_H */