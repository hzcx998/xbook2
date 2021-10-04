#ifndef _SYS_WALLTIME_H
#define _SYS_WALLTIME_H
/* WR: write RD: read */
#define WTM_WR_TIME(hou, min, sec) ((unsigned short)((((hou) & 0x1f) << 11) | \
        (((min) & 0x3f) << 5) | (((sec) / 2) & 0x1f)))
#define WTM_WR_DATE(yea, mon, day) (((unsigned short)(((yea) - 1980) & 0x7f) << 9) | \
        (((mon) & 0xf) << 5) | ((day) & 0x1f))

#define WTM_RD_HOUR(data)    ((unsigned int)(((data) >> 11) & 0x1f))
#define WTM_RD_MINUTE(data)  ((unsigned int)(((data) >> 5) & 0x3f))
#define WTM_RD_SECOND(data)  ((unsigned int)(((data) & 0x1f) * 2))
#define WTM_RD_YEAR(data)    ((unsigned int)((((data) >> 9) & 0x7f) + 1980))
#define WTM_RD_MONTH(data)   ((unsigned int)(((data) >> 5) & 0xf))
#define WTM_RD_DAY(data)     ((unsigned int)((data) & 0x1f))

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

static inline int walltime_t2unixtime(const walltime_t *datetime, signed int *nix)
{
    signed int ret;
    int y4, ay;

    const unsigned short monthssum[12] =
    {
        0,
        31,
        31 + 28,
        31 + 28 + 31,
        31 + 28 + 31 + 30,
        31 + 28 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30
    };
    const unsigned char months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const int SECPERMIN = 60;
    const int SECPERHOUR = 60 * SECPERMIN;
    const int SECPERDAY = 24 * SECPERHOUR;
    const int SECPERYEAR = 365 * SECPERDAY;
    const int SECPER4YEARS = 4 * SECPERYEAR + SECPERDAY;

    if (datetime->year > 2038 || datetime->year < 1901)
    {
        return 0;
    }
    if (datetime->month > 12 || datetime->month < 1)
    {
        return 0;
    }

    /* In the period of validity of unixtime all years divisible by 4 are bissextile*/
    /* Convenience: let's have 3 consecutive non-bissextile years at the beginning of the epoch. So count from 1973 instead of 1970 */
    ret = 3 * SECPERYEAR + SECPERDAY;

    /* Transform C divisions and modulos to mathematical ones */
    y4 = ((datetime->year - 1) >> 2) - (1973 / 4);
    ay = datetime->year - 1973 - 4 * y4;
    ret += y4 * SECPER4YEARS;
    ret += ay * SECPERYEAR;

    ret += monthssum[datetime->month - 1] * SECPERDAY;
    if (ay == 3 && datetime->month >= 3)
    {
        ret += SECPERDAY;
    }

    ret += (datetime->day - 1) * SECPERDAY;

    if ((datetime->day > months[datetime->month - 1] && (!ay || datetime->month != 2 || datetime->day != 29)) || datetime->day < 1)
    {
        return 0;
    }

    ret += datetime->hour * SECPERHOUR;
    if (datetime->hour > 23)
    {
        return 0;
    }
    ret += datetime->minute * 60;
    if (datetime->minute > 59)
    {
        return 0;
    }

    ret += datetime->second;
    /* Accept leap seconds.  */
    if (datetime->second > 60)
    {
        return 0;
    }

    if ((datetime->year > 1980 && ret < 0) || (datetime->year < 1960 && ret > 0))
    {
        return 0;
    }

    *nix = ret;

    return 1;
}

#endif   /* _SYS_WALLTIME_H */