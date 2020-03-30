#include <stdarg.h>
#include <vsprintf.h>
#include <string.h>
#include <conio.h>
#include <xbook.h>

/** 
 * printf - 格式化打印输出
 * @fmt: 格式以及字符串
 * @...: 可变参数
 */
int printf(const char *fmt, ...)
{
	//int i;
	char buf[STR_DEFAULT_LEN];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(buf, fmt, arg);
	
    /* 输出到控制台 */
    x_write(0x100000, 0, buf, strlen(buf));
	return 0;
}
