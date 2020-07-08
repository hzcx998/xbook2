#include <ctype.h>

int isspace(char c)
{
	char comp[] = {' ', '\t', '\n', '\r', '\v', '\f'};
	int i;
	const int len = 6; //comp数组的长度，这个你也可以用strlen()来求，但是要包括string.h头文件
	// 也可以使用宏来定义
	for (i = 0; i < len; i++) {
		if (c == comp[i])
			return 1;
	}
	return 0;
}

//测试参数ch是否是字母(A-Z,大小写均可)或数字（0-9）
int isalnum(int ch)
{
	return (unsigned int)((ch | 0x20) - 'a') < 26u ||
		(unsigned int)( ch - '0') < 10u;
}

/*
判断是否是16进制字母 0123456789abcdefABCDEF
*/
int isxdigit (int c)
{
	if (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F')) {
		return 1;
	}
	return 0;
}

//判断字符c是否为数字
int isdigit( int ch )
{
    return (unsigned int)(ch - '0') < 10u;
}


#define TOLOWER(x) ((x) | 0x20)
/*
#define isxdigit(c)    (('0' <= (c) && (c) <= '9') /
             || ('a' <= (c) && (c) <= 'f') /
             || ('A' <= (c) && (c) <= 'F'))
 
#define isdigit(c)    ('0' <= (c) && (c) <= '9')
*/
unsigned long strtoul(const char *cp,char **endp,unsigned int base)
{
    unsigned long result = 0,value;
 
    if (!base) {
        base = 10;
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((TOLOWER(*cp) == 'x') && isxdigit(cp[1])) {
                cp++;
                base = 16;
            }
        }
    } else if (base == 16) {
        if (cp[0] == '0' && TOLOWER(cp[1]) == 'x')
            cp += 2;
    }
    while (isxdigit(*cp) &&
           (value = isdigit(*cp) ? *cp-'0' : TOLOWER(*cp)-'a'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;
    return result;
}

int isalpha(int ch)
{
	if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')) {
		return 1;
	}
	return 0;
}

int tolower(int c)
{
	if ((c >= 'A') && (c <= 'Z'))
		return c + ('a' - 'A');
	return c;
}
 
int toupper(int c)
{
	if ((c >= 'a') && (c <= 'z'))
		return c + ('A' - 'a');
	return c;
}

/* 判断字符是不是纯数字 */
int isdigitstr(const char *str)
{
    const char *p = str;
               
    while (*p) {
        /* 如果有一个字符不是数字，就返回0 */
        if (!isdigit(*p)) { 
            return 0;
        }
        p++;
    }
    return 1;
}

int isgraph(int ch)
{
    return (unsigned int)(ch - '!') < 127u - '!';
}   //判断字符c是否为除空格外的可打印字符。可打印字符（0x21-0x7e）。
int islower (int ch)
{
    return (unsigned int) (ch - 'a') < 26u;
}//判断字符c是否为小写英文字母

int iscntrl(int ch)
{
    return (unsigned int)ch < 32u  ||  ch == 127;
}//判断字符c是否为控制字符。当c在0x00-0x1F之间或等于0x7F(DEL)时，返回非零值，否则返回零。

int isupper(int ch)
{
    return (unsigned int)(ch - 'A') < 26u;
}//判断字符c是否为大写英文字母

int ispunct(int ch)
{
    return isprint(ch)  &&  !isalnum (ch)  &&  !isspace (ch);
}//判断字符c是否为标点符号。标点符号指那些既不是字母数字，也不是空格的可打印字符。

int isprint( int ch )
{
    return (unsigned int)(ch - ' ') < 127u - ' ';
}//判断字符c是否为可打印字符（含空格）。当c为可打印字符（0x20-0x7e）时，返回非零值，否则返回零。
