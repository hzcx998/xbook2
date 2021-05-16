#include <arch/time.h>
#include <k210_rtc.h>

/* dummy */

/* TODO: 根据手册实现 */
unsigned int rtc_get_hour_hex()
{
	return 12;
}

unsigned int rtc_get_min_hex()
{
	return 24;
}

unsigned int rtc_get_sec_hex()
{
	return 36;
}
unsigned int rtc_get_day_of_month()
{
	return 16;
}
unsigned int rtc_get_day_of_week()
{
	return 6;
}
unsigned int rtc_get_mon_hex()
{
	return 5;
}
unsigned int rtc_get_year()
{
	return 2021;
}
