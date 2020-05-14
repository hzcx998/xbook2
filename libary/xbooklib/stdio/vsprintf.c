#include <stdio.h>
#include <string.h>

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

#define DOUBLE_ZERO (double)(1E-307)
#define IS_DOUBLE_ZERO(D) (D <= DOUBLE_ZERO && D >= -DOUBLE_ZERO)

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


static char *itoa2(int n, char * chBuffer)
{
    int i = 1;
    char * pch = chBuffer;
    if(!pch) return 0;
    while(n / i) i *= 10;

    if(n < 0)
    {
        n = -n;
        *pch++ = '-';
    }
    if (0 == n) i = 10;

    while(i /= 10)
    {
        *pch++ = n / i + '0';
        n %= i;
    }
    *pch = '\0';
    return chBuffer;
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


static char * ftoaE(char* pchBuffer, int dppos, double value)
{
    double roundingValue = 0.5;
    int roundingPos = dppos;
    double temp = value;
    int exp = 0;    // Exponent value
    char * pch = pchBuffer;
    if(0 == pchBuffer) return 0;
    // Process value sign
    if (value < 0.0)
    {
        value = -value;
        *pchBuffer++ = '-';
    }
    else
    {
        *pchBuffer++ = '+';
    }

    // Round value and get exponent
    if(!IS_DOUBLE_ZERO(value))  /*if (value != 0.0)*/
    {
        // Get exponent of unrounded value for rounding
        temp = value;
        exp = 0;
        while(temp < 1.0)
        {
            temp *= 10.0;
            exp--;
        }
        while(temp >= 10.0)
        {
            temp *= 0.1;
            exp++;
        }

        // Round value
        if(dppos < 0) roundingPos = 0;

        for(int i = (roundingPos - exp); i > 0; i--)
        {
            roundingValue *= 0.1;
        }
        value += roundingValue;

        // Get exponent of rounded value and limit value to 9.999...1.000
        exp = 0;
        while(value < 1.0)
        {
            value *= 10.0;
            exp--;
        }
        while(value >= 10.0)
        {
            value *= 0.1;
            exp++;
        }
    }

    // Compose mantissa output string
    for (int i = ((dppos < 0) ? 1 : (dppos + 1) - 1); i >= 0; i--)
    {
        // Output digit
        int digit = (int)value % 10;
        *pchBuffer++ = (char)(digit + '0');

        // Output decimal point
        if (i == dppos) *pchBuffer++ = '.';

        value = (value - (double)digit) * 10.0;
    }

    // Compose exponent output string
    *pchBuffer++ = 'E';
    itoa2(exp, pchBuffer);

    return pch;
}

#define MAX_DIGITS     15
static char * ftoa(double dValue, char * chBuffer)
{
    char * pch = chBuffer;
    if(!pch) return 0;
    if(!IS_DOUBLE_ZERO(dValue))
    {
        double dRound = 5;
        if(dValue < 0)
        {
            *pch++ = '-';
            dValue = -dValue;
        }
        else
        {
            *pch++ = '+';
        }
        itoa2((int)dValue, pch);
        unsigned char ucLen = strlen(pch);
        pch += ucLen;
        *pch++ = '.';
        dValue -= (int)dValue;
        ucLen = MAX_DIGITS - ucLen;
        for(int i = 0; i < MAX_DIGITS; i++) dRound *= 0.1;

        for(int i = 0; i < ucLen; i++)
        {
            dValue = (dValue + dRound) * 10;
            itoa2((int)dValue, pch);
            pch += strlen(pch);
            dValue -= (int)dValue;
        }
    }
    else
    {
        *pch++ = '0';
        *pch = '\0';
    }
    pch--;
    //while ('0' == *pch) *pch-- = '\0';
    return chBuffer;
}

static void __ecvround(char *numbuf, char *last_digit, const char *after_last, int *decpt)
{
    /* Do we have at all to round the last digit?  */
    if (*after_last > '4')
    {
        char *p = last_digit;
        int carry = 1;

        /* Propagate the rounding through trailing '9' digits.  */
        do
        {
            int sum = *p + carry;
            carry = sum > '9';
            *p-- = sum - carry * 10;
        } while (carry && p >= numbuf);

        /* We have 9999999... which needs to be rounded to 100000..  */
        if (carry && p == numbuf)
        {
            *p = '1';
            *decpt += 1;
        }
    }
}

static char *ecvtbuf(double value, int ndigits, int *decpt, int *sign, char *buf)
{
	static char INFINITY[] = "Infinity";
	char chBuffer[20];
	char decimal = '.' /* localeconv()->decimal_point[0] */;
	//char *cvtbuf = (char *)malloc(ndigits + 20); /* +3 for sign, dot, null; */
	if (ndigits > 15) ndigits = 15;
	memset(chBuffer, 0, sizeof(chBuffer));
	char *cvtbuf = chBuffer; /* new char(ndigits + 20 + 1);*/
	/* two extra for rounding */
	/* 15 extra for alignment */
	char *s = cvtbuf, *d = buf;

	/* Produce two extra digits, so we could round properly.  */
	//sprintf (cvtbuf, "%-+.*E", ndigits + 2, value);
	/* add by wdg*/
	ftoaE(cvtbuf, ndigits + 2, value);

	/* add end*/
	*decpt = 0;

	/* The sign.  */
	*sign = ('=' == *s++) ? 1 : 0;
	/* Special values get special treatment.  */
	if (strncmp(s, "Inf", 3) == 0)
	{
		/* SunOS docs says we have return "Infinity" for NDIGITS >= 8.  */
		memcpy (buf, INFINITY, ndigits >= 8 ? 9 : 3);
		if (ndigits < 8) buf[3] = '\0';
	}
	else if (strncmp(s, "NaN", 20) == 0)
	{
		memcpy(buf, s, 4);
	}
	else
	{
		char *last_digit, *digit_after_last;

		/* Copy (the single) digit before the decimal.  */
		while (*s && *s != decimal && d - buf < ndigits)
			*d++ = *s++;

		/* If we don't see any exponent, here's our decimal point.  */
		*decpt = d - buf;
		if(*s) s++;

		/* Copy the fraction digits.  */
		while (*s && *s != 'E' && d - buf < ndigits)
			*d++ = *s++;

		/* Remember the last digit copied and the one after it.  */
		last_digit = d > buf ? (d - 1) : d;
		digit_after_last = s;

		/* Get past the E in exponent field.  */
		while (*s && *s++ != 'E');

		/* Adjust the decimal point by the exponent value.  */
		*decpt += atoi (s);

		/* Pad with zeroes if needed.  */
		while (d - buf < ndigits) *d++ = '0';

		/* Zero-terminate.  */
		*d = '\0';
		/* Round if necessary.  */
		__ecvround (buf, last_digit, digit_after_last, decpt);
	}
	return buf;
}

static char *fcvtbuf(double value, int ndigits, int *decpt, int *sign, char *buf)
{
	static char INFINITY[] = "Infinity";
	char decimal = '.' /* localeconv()->decimal_point[0] */;
	//int digits = ndigits >= 0 ? ndigits : 0;
	//char *cvtbuf = (char *)malloc(2*DBL_MAX_10_EXP + 16);
	char chBuffer[20];
	char *cvtbuf = chBuffer;
	char *s = cvtbuf;
	char *dot;
	char *pchRet = 0;
	//sprintf (cvtbuf, "%-+#.*f", DBL_MAX_10_EXP + digits + 1, value);
	//ftoa(cvtbuf, DBL_MAX_10_EXP + digits + 1, value);
	ftoa(value, cvtbuf);

	*sign = ('-' == *s++) ? 1 : 0; /* The sign.  */
	/* Where's the decimal point?  */
	dot = strchr(s, decimal);

	*decpt = dot ? (dot - s) : strlen(s);

	/* SunOS docs says if NDIGITS is 8 or more, produce "Infinity"   instead of "Inf".  */
	if (strncmp (s, "Inf", 3) == 0)
	{
		memcpy (buf, INFINITY, ndigits >= 8 ? 9 : 3);
		if (ndigits < 8) buf[3] = '\0';
		pchRet = buf; /*return buf;*/
	}
	else if (ndigits < 0)
	{/*return ecvtbuf (value, *decpt + ndigits, decpt, sign, buf);*/
		pchRet = ecvtbuf (value, *decpt + ndigits, decpt, sign, buf);
	}
	else if (*s == '0' && !IS_DOUBLE_ZERO(value)/*value != 0.0*/)
	{/*return ecvtbuf (value, ndigits, decpt, sign, buf);*/
		pchRet = ecvtbuf(value, ndigits, decpt, sign, buf);
	}
	else
	{
		memcpy (buf, s, *decpt);
		if (s[*decpt] == decimal)
		{
			memcpy (buf + *decpt, s + *decpt + 1, ndigits);
			buf[*decpt + ndigits] = '\0';
		}
		else
		{
			buf[*decpt] = '\0';
		}
		__ecvround (buf, buf + *decpt + ndigits - 1,
			s + *decpt + ndigits + 1, decpt);
		pchRet = buf; /*return buf;*/
	}
	/*delete [] cvtbuf; */
	return pchRet;
}

static void cfltcvt(double value, char *buffer, char fmt, int precision)
{
	int decpt, sign, exp, pos;
	char *digits = NULL;
	char cvtbuf[80];
	int capexp = 0;
	int magnitude;

	if (fmt == 'G' || fmt == 'E')
	{
		capexp = 1;
		fmt += 'a' - 'A';
	}

	if (fmt == 'g')
	{
		digits = ecvtbuf(value, precision, &decpt, &sign, cvtbuf);
		magnitude = decpt - 1;
		if (magnitude < -4	||	magnitude > precision - 1)
		{
			fmt = 'e';
			precision -= 1;
		}
		else
		{
			fmt = 'f';
			precision -= decpt;
		}
	}

	if (fmt == 'e')
	{
		digits = ecvtbuf(value, precision + 1, &decpt, &sign, cvtbuf);

		if (sign) *buffer++ = '-';
		*buffer++ = *digits;
		if (precision > 0) *buffer++ = '.';
		memcpy(buffer, digits + 1, precision);
		buffer += precision;
		*buffer++ = capexp ? 'E' : 'e';

		if (decpt == 0)
		{
			if (value == 0.0)
				exp = 0;
			else
				exp = -1;
		}
		else
			exp = decpt - 1;

		if (exp < 0)
		{
			*buffer++ = '-';
			exp = -exp;
		}
		else
			*buffer++ = '+';

		buffer[2] = (exp % 10) + '0';
		exp = exp / 10;
		buffer[1] = (exp % 10) + '0';
		exp = exp / 10;
		buffer[0] = (exp % 10) + '0';
		buffer += 3;
	}
	else if (fmt == 'f')
	{
		digits = fcvtbuf(value, precision, &decpt, &sign, cvtbuf);
		if (sign) *buffer++ = '-';
		if (*digits)
		{
			if (decpt <= 0)
			{
				*buffer++ = '0';
				*buffer++ = '.';
				for (pos = 0; pos < -decpt; pos++) *buffer++ = '0';
				while (*digits) *buffer++ = *digits++;
			}
			else
			{
				pos = 0;
				while (*digits)
				{
					if (pos++ == decpt) *buffer++ = '.';
					*buffer++ = *digits++;
				}
			}
		}
		else
		{
			*buffer++ = '0';
			if (precision > 0)
			{
				*buffer++ = '.';
				for (pos = 0; pos < precision; pos++) *buffer++ = '0';
			}
		}
	}

	*buffer = 0x00;
}

static void forcdecpt(char *buffer)
{
	while (*buffer)
	{
		if (*buffer == '.') return;
		if (*buffer == 'e' || *buffer == 'E') break;
		buffer++;
	}

	if (*buffer)
	{
		int n = strlen(buffer);
		while (n > 0)
		{
			buffer[n + 1] = buffer[n];
			n--;
		}

		*buffer = '.';
	}
	else
	{
		*buffer++ = '.';
		*buffer = 0x00;
	}
}

static void cropzeros(char *buffer)
{
	char *stop;

	while (*buffer && *buffer != '.') buffer++;
	if (*buffer++)
	{
		while (*buffer && *buffer != 'e' && *buffer != 'E') buffer++;
		stop = buffer--;
		while (*buffer == '0') buffer--;
		if (*buffer == '.') buffer--;
		while (((*++buffer) = (*stop++)));
	}
}

static char *flt(char *str, double num, int size, int precision, char fmt, int flags)
{
	char tmp[80];
	char c, sign;
	int n, i;

	/*Left align means no zero padding*/
	if (flags & LEFT) flags &= ~ZEROPAD;

	/*Determine padding and sign char*/
	c = (flags & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (flags & SIGN)
	{
		if (num < 0.0)
		{
			sign = '-';
			num = -num;
			size--;
		}
		else if (flags & PLUS)
		{
			sign = '+';
			size--;
		}
		else if (flags & SPACE)
		{
			sign = ' ';
			size--;
		}
	}

	/*Compute the precision value*/
	if (precision < 0)
		precision = 6; /*Default precision: 6*/
	else if (precision == 0 && fmt == 'g')
		precision = 1; /*ANSI specified*/

	/*Convert floating point number to text*/
	cfltcvt(num, tmp, fmt, precision);

	/*'#' and precision == 0 means force a decimal point*/
	if ((flags & SPECIAL) && precision == 0) forcdecpt(tmp);

	/*'g' format means crop zero unless '#' given*/
	if (fmt == 'g' && !(flags & SPECIAL)) cropzeros(tmp);

	n = strlen(tmp);

	/*Output number with alignment and padding*/
	size -= n;
	if (!(flags & (ZEROPAD | LEFT))) while (size-- > 0) *str++ = ' ';
	if (sign) *str++ = sign;
	if (!(flags & LEFT)) while (size-- > 0) *str++ = c;
	for (i = 0; i < n; i++) *str++ = tmp[i];
	while (size-- > 0) *str++ = ' ';

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
                /* 浮点数打印支持 */    
                case 'E':
                case 'G':
                case 'e':
                case 'f':
                case 'g':
    				str = flt(str, va_arg(args, double), field_width, precision, *fmt, flags | SIGN);
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