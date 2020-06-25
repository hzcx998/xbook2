#include <time.h>
#include <stdio.h>
#include <limits.h>
#include <stddef.h>

static const char *day_name[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

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

static const char format[] = "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n";
static char result[		 3+1+ 3+1+20+1+20+1+20+1+20+1+20+1 + 1];

static char *asctime_internal (const struct tm *tp, char *buf, size_t buflen)
{
    if (tp == NULL) {
        return NULL;
    }

    /* We limit the size of the year which can be printed.  Using the %d
     format specifier used the addition of 1900 would overflow the
     number and a negative vaue is printed.  For some architectures we
     could in theory use %ld or an evern larger integer format but
     this would mean the output needs more space.  This would not be a
     problem if the 'asctime_r' interface would be defined sanely and
     a buffer size would be passed.  */
    if ((tp->tm_year > INT_MAX - 1900)) {
eoverflow:
        return NULL;
    }

    int n = snprintf(buf, buflen, format,
		      (tp->tm_wday < 0 || tp->tm_wday >= 7 ?
		       "???" : day_name [tp->tm_wday]),
		      (tp->tm_mon < 0 || tp->tm_mon >= 12 ?
		       "???" : month_name [tp->tm_mon]),
		      tp->tm_mday, tp->tm_hour, tp->tm_min,
		      tp->tm_sec, 1900 + tp->tm_year);
    if (n < 0)
        return NULL;
    if (n >= buflen)
        goto eoverflow;
    return buf;
}

/* Like asctime, but write result to the user supplied buffer.  The
   buffer is only guaranteed to be 26 bytes in length.  */
char *asctime_r(const struct tm *tp, char *buf)
{
  return asctime_internal (tp, buf, 26);
}

char *asctime(const struct tm *tp)
{
    return asctime_internal (tp, result, sizeof (result));
}