#include <arch/io.h>
#include <arch/cmos.h>
#include <arch/time.h>

static unsigned char read_cmos(unsigned char p)
{
	unsigned char data;
	ioport_out8(CMOS_INDEX, p);
	data = ioport_in8(CMOS_DATA);
	ioport_out8(CMOS_INDEX, 0x80);
	return data;
}

unsigned int cmos_get_hour_hex()
{
	return BCD_HEX(read_cmos(CMOS_CUR_HOUR));
}

unsigned int cmos_get_min_hex()
{
	return BCD_HEX(read_cmos(CMOS_CUR_MIN));
}
unsigned char cmos_get_min_hex8()
{
	return BCD_HEX(read_cmos(CMOS_CUR_MIN));
}
unsigned int cmos_get_sec_hex()
{
	return BCD_HEX(read_cmos(CMOS_CUR_SEC));
}
unsigned int cmos_get_day_of_month()
{
	return BCD_HEX(read_cmos(CMOS_MON_DAY));
}
unsigned int cmos_get_day_of_week()
{
	return BCD_HEX(read_cmos(CMOS_WEEK_DAY));
}
unsigned int cmos_get_mon_hex()
{
	return BCD_HEX(read_cmos(CMOS_CUR_MON));
}
unsigned int cmos_get_year()
{
	return (BCD_HEX(read_cmos(CMOS_CUR_CEN))*100) + \
		BCD_HEX(read_cmos(CMOS_CUR_YEAR))+1980;
}
