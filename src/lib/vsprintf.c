#include <xbook/vsprintf.h>
#include <xbook/string.h>
#include <xbook/memops.h>


#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

#define is_digit(c)	((c) >= '0' && (c) <= '9')

int skip_atoi(const char **s);

/*

*/

/*
#define do_div(n,base) ({ \
int __res; \
__asm__("divq %%rcx":"=a" (n),"=d" (__res):"0" (n),"1" (0),"c" (base)); \
__res; })
*/

static unsigned long do_div(unsigned long *num, char base) 
{
    unsigned long ret = *num % base;
    *num = *num / base;
    return ret;
}

/*

*/

static char * number(char * str, long num, int base, int size, int precision ,int type);

/*

*/

int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

/**
 * number - 对数字进行处理
 * 
 * 
 */
static char *number(char * str, long num, int base, int size, int precision,	int type)
{
	char c,sign,tmp[50];
	const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type&SMALL) digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (type&LEFT) type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ' ;
	sign = 0;
	if (type&SIGN && num < 0) {
		sign='-';
		num = -num;
	} else
		sign=(type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);
	if (sign) size--;
	if (type & SPECIAL) {
        if (base == 16) size -= 2;
		else if (base == 8) size--;
    }
    i = 0;
	if (num == 0)
		tmp[i++]='0';
	else while (num!=0)
		tmp[i++]=digits[do_div((unsigned long *)&num,base)];
	if (i > precision) precision=i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT))) {
        while(size-- > 0)
			*str++ = ' ';
    }
    if (sign)
		*str++ = sign;
	if (type & SPECIAL) {
        if (base == 8)
			*str++ = '0';
		else if (base==16) 
		{
			*str++ = '0';
			*str++ = digits[33];
		}
    }
    if (!(type & LEFT))
		while(size-- > 0)
			*str++ = c;

	while(i < precision--)
		*str++ = '0';
	while(i-- > 0)
		*str++ = tmp[i];
	while(size-- > 0)
		*str++ = ' ';
	return str;
}


/**
 * vsprintf - 格式化字符串
 * @buf: 要储存的缓冲区
 * @fmt: 格式
 * @args: 参数
 * 
 * 此版本是从MINEOS中移植
 * 
 * 格式：%[标志][最小宽度][.精度][类型长度]类型
 * 
 * 支持的数据类型（type）：
 *      c,s,o,p,x,X,d,i,u,n,%
 * 标志（flags）：
 *      1.-（左对齐）
 *      2.+（输出符号）
 *      3.space（输出值为正时加上空格，为负时加-号）
 *      4.#（type是o,x,X时，添加前缀0,0x,0X。）
 *      5.0(将输出的前面补上0，知道沾满指定列宽位置（不可以搭配使用'-'）)
 * 最小宽度（width）：
 *      1.数值（输出指定宽度）
 *      2.*（在参数中给出宽度）
 * 精度（precision）：
 *      1.以.开头。数值（对于整形[d,i,o,u,x,X],表示输出的最小数字个数，
 *          不足补前导零，超过不截断）
 *      2.对于字符串，表示最大可输出字符数，不足正常输出，超过则阶段。
 *          不显示指定，则默认为0
 *      3.*，用参数指定位数。
 * 类型长度（length）：
 *      h，short int类型
 *      l，long int类型
 *      L，long double类型
 *      Z，unused
 *      
 */
int vsprintf(char * buf,const char *fmt, va_list args)
{
	char * str,*s;
	int flags;
	int field_width;
	int precision;
	int len,i;

	int qualifier;		/* 'h', 'l', 'L' or 'Z' for integer fields */

	for(str = buf; *fmt; fmt++)
	{
        
		if(*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}
		flags = 0;
		repeat:
			fmt++;
			switch(*fmt)
			{
				case '-':flags |= LEFT;	
				goto repeat;
				case '+':flags |= PLUS;	
				goto repeat;
				case ' ':flags |= SPACE;	
				goto repeat;
				case '#':flags |= SPECIAL;	
				goto repeat;
				case '0':flags |= ZEROPAD;	
				goto repeat;
			}

			/* get field width */

			field_width = -1;
			if(is_digit(*fmt))
				field_width = skip_atoi(&fmt);
			else if(*fmt == '*')
			{
				fmt++;
				field_width = va_arg(args, int);
				if(field_width < 0)
				{
					field_width = -field_width;
					flags |= LEFT;
				}
			}
			
			/* get the precision */

			precision = -1;
			if(*fmt == '.')
			{
				fmt++;
				if(is_digit(*fmt))
					precision = skip_atoi(&fmt);
				else if(*fmt == '*')
				{	
					fmt++;
					precision = va_arg(args, int);
				}
				if(precision < 0)
					precision = 0;
			}
			
			qualifier = -1;
			if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z')
			{	
				qualifier = *fmt;
				fmt++;
			}
							
			switch(*fmt)
			{
				case 'c':

					if(!(flags & LEFT))
						while(--field_width > 0)
							*str++ = ' ';
					*str++ = (unsigned char)va_arg(args, int);
					while(--field_width > 0)
						*str++ = ' ';
					break;

				case 's':
				
					s = va_arg(args,char *);
					if(!s)
						s = '\0';
					len = strlen(s);
					if(precision < 0)
						precision = len;
					else if(len > precision)
						len = precision;
					
					if(!(flags & LEFT))
						while(len < field_width--)
							*str++ = ' ';
					for(i = 0;i < len ;i++)
						*str++ = *s++;
					while(len < field_width--)
						*str++ = ' ';
					break;

				case 'o':
					
					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),8,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),8,field_width,precision,flags);
					break;

				case 'p':

					if(field_width == -1)
					{
						field_width = 2 * sizeof(void *);
						flags |= ZEROPAD;
					}

					str = number(str,(unsigned long)va_arg(args,void *),16,field_width,precision,flags);
					break;

				case 'x':

					flags |= SMALL;

				case 'X':

					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),16,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),16,field_width,precision,flags);
					break;

				case 'd':
				case 'i':

					flags |= SIGN;
				case 'u':

					if(qualifier == 'l')
						str = number(str,va_arg(args,long),10,field_width,precision,flags);
					else
						str = number(str,va_arg(args,int),10,field_width,precision,flags);
					break;

				case 'n':
					
					if(qualifier == 'l')
					{
						long *ip = va_arg(args,long *);
						*ip = (str - buf);
					}
					else
					{
						int *ip = va_arg(args,int *);
						*ip = (str - buf);
					}
					break;

				case '%':
					
					*str++ = '%';
					break;

				default:

					*str++ = '%';	
					if(*fmt)
						*str++ = *fmt;
					else
						fmt--;
					break;
			}

	}
	*str = '\0';
	return str - buf;
}

/**
 * vsprintf_old - 格式化字符串到缓冲区
 * 
 * xbook旧版本
 */
int vsprintf_old(char *buf, const char *fmt, va_list args)
{
	char*	p;

	va_list	p_next_arg = args;
	int	m;

	char	inner_buf[STR_DEFAULT_LEN];
	char	cs;
	int	align_nr;

	for (p=buf;*fmt;fmt++) {
		if (*fmt != '%') {
			*p++ = *fmt;
			continue;
		}
		else {		/* a format string begins */
			align_nr = 0;
		}

		fmt++;

		if (*fmt == '%') {
			*p++ = *fmt;
			continue;
		}
		else if (*fmt == '0') {
			cs = '0';
			fmt++;
		}
		else {
			cs = ' ';
		}
		while (((unsigned char)(*fmt) >= '0') && ((unsigned char)(*fmt) <= '9')) {
			align_nr *= 10;
			align_nr += *fmt - '0';
			fmt++;
		}

		char * q = inner_buf;
		
		memset(q, 0, sizeof(inner_buf));

		switch (*fmt) {
		case 'c':
			*q++ = *((char*)p_next_arg);
			p_next_arg += 4;
			break;
		case 'x':
			m = *((int*)p_next_arg);

			itoa16_align(q, m);	//对齐的16进制数
			//itoa(&q, m, 16);
			p_next_arg += 4;
			break;
		case 'd':
			m = *((int*)p_next_arg);
			if (m < 0) {
				m = m * (-1);
				*q++ = '-';
			}
			itoa(&q, m, 10);
			p_next_arg += 4;
			break;
		case 's':
			strcpy(q, (*((char**)p_next_arg)));
			q += strlen(*((char**)p_next_arg));
			p_next_arg += 4;
			break;
		default:
			break;
		}

		int k;
		for (k = 0; k < ((align_nr > strlen(inner_buf)) ? (align_nr - strlen(inner_buf)) : 0); k++) {
			*p++ = cs;
		}
		q = inner_buf;
		while (*q) {
			*p++ = *q++;
		}
	}

	*p = 0;

	return (p - buf);
}


/**
 * vsnprintf - 格式化字符串
 * @buf: 要储存的缓冲区
 * @buflen: 缓冲区长度
 * @fmt: 格式
 * @args: 参数
 * 
 * 有长度限定的字符串格式化，每次只需要从vsprintf复制过来修改参数即可
 */
int vsnprintf(char * buf, int buflen, const char *fmt, va_list args)
{
	char * str,*s;
	int flags;
	int field_width;
	int precision;
	int len,i;

	int qualifier;		/* 'h', 'l', 'L' or 'Z' for integer fields */

    /* 需要添加长度判断，buflen */
	for(str = buf; *fmt && buflen--; fmt++)
	{
        
		if(*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}
		flags = 0;
		repeat:
        fmt++;
        switch(*fmt)
        {
            case '-':flags |= LEFT;	
            goto repeat;
            case '+':flags |= PLUS;	
            goto repeat;
            case ' ':flags |= SPACE;	
            goto repeat;
            case '#':flags |= SPECIAL;	
            goto repeat;
            case '0':flags |= ZEROPAD;	
            goto repeat;
        }

        /* get field width */

        field_width = -1;
        if(is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if(*fmt == '*')
        {
            fmt++;
            field_width = va_arg(args, int);
            if(field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }
        
        /* get the precision */

        precision = -1;
        if(*fmt == '.')
        {
            fmt++;
            if(is_digit(*fmt))
                precision = skip_atoi(&fmt);
            else if(*fmt == '*')
            {	
                fmt++;
                precision = va_arg(args, int);
            }
            if(precision < 0)
                precision = 0;
        }
        
        qualifier = -1;
        if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z')
        {	
            qualifier = *fmt;
            fmt++;
        }
                        
        switch(*fmt)
        {
        case 'c':

            if(!(flags & LEFT))
                while(--field_width > 0)
                    *str++ = ' ';
            *str++ = (unsigned char)va_arg(args, int);
            while(--field_width > 0)
                *str++ = ' ';
            break;

        case 's':
        
            s = va_arg(args,char *);
            if(!s)
                s = '\0';
            len = strlen(s);
            if(precision < 0)
                precision = len;
            else if(len > precision)
                len = precision;
            
            if(!(flags & LEFT))
                while(len < field_width--)
                    *str++ = ' ';
            for(i = 0;i < len ;i++)
                *str++ = *s++;
            while(len < field_width--)
                *str++ = ' ';
            break;

        case 'o':
            
            if(qualifier == 'l')
                str = number(str,va_arg(args,unsigned long),8,field_width,precision,flags);
            else
                str = number(str,va_arg(args,unsigned int),8,field_width,precision,flags);
            break;

        case 'p':

            if(field_width == -1)
            {
                field_width = 2 * sizeof(void *);
                flags |= ZEROPAD;
            }

            str = number(str,(unsigned long)va_arg(args,void *),16,field_width,precision,flags);
            break;

        case 'x':

            flags |= SMALL;

        case 'X':

            if(qualifier == 'l')
                str = number(str,va_arg(args,unsigned long),16,field_width,precision,flags);
            else
                str = number(str,va_arg(args,unsigned int),16,field_width,precision,flags);
            break;

        case 'd':
        case 'i':

            flags |= SIGN;
        case 'u':

            if(qualifier == 'l')
                str = number(str,va_arg(args,long),10,field_width,precision,flags);
            else
                str = number(str,va_arg(args,int),10,field_width,precision,flags);
            break;

        case 'n':
            
            if(qualifier == 'l')
            {
                long *ip = va_arg(args,long *);
                *ip = (str - buf);
            }
            else
            {
                int *ip = va_arg(args,int *);
                *ip = (str - buf);
            }
            break;

        case '%':
            
            *str++ = '%';
            break;

        default:

            *str++ = '%';	
            if(*fmt)
                *str++ = *fmt;
            else
                fmt--;
            break;
        }
	}
	*str = '\0';
	return str - buf;
}


int sprintf(char *buf, const char *fmt, ...)
{
	va_list arg = (va_list)((char*)(&fmt) + 4);        /* 4 是参数 fmt 所占堆栈中的大小 */
	return vsprintf(buf, fmt, arg);
}

int snprintf(char *buf, int buflen, const char *fmt, ...)
{
	va_list arg = (va_list)((char*)(&fmt) + 4);        /* 4 是参数 fmt 所占堆栈中的大小 */
	return vsnprintf(buf, buflen, fmt, arg);
}