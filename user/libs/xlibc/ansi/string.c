#include <string.h>

/*
 * 功能: n个字符比对
 * 参数: s1     字符串1
 *      s2      字符串2
 *      s3      要比较的字符数
 * 返回: 0 表示字符串一样
 *      小于0 表示s1小于s2
 *      大于0 表示s1大于s2
 * 说明: 引导和加载完成后，就会跳到这里
 */
int strncmp (const char * s1, const char * s2, int n)
{ 
	if(!n)return(0);

	while (--n && *s1 && *s1 == *s2)
	{ 
		s1++;
		s2++;
	}
	return( *s1 - *s2 );
}

char* itoa(char ** ps, int val, int base)
{
	int m = val % base;
	int q = val / base;
	if (q) {
		itoa(ps, q, base);
	}
	*(*ps)++ = (m < 10) ? (m + '0') : (m - 10 + 'A');

	return *ps;
}

void *memset(void* src, uint8_t value, uint32_t size) 
{
	
	//printf("void *memset-1-::0x%x-value-::%d-size-::0x%x:\n",src,value,size);
	uint8_t* s = (uint8_t*)src;
	//printf("void *memset-2-::0x%x:\n",s);
	while (size > 0){
		*s++ = value;
		--size;
               // printf("void *memset-3-::%s::-::0x%x:\n",s,size);
	}
	//printf("void *memset-4-src::%s::-::0x%x:\n",src,src);
	//printf("void *memset-5-:\n");
	return src;
}

void *memset16(void* src, uint16_t value, uint32_t size) 
{
	uint16_t* s = (uint16_t*)src;
	while (size-- > 0){
		*s++ = value;
	}
	return src;
}

void *memset32(void* src, uint32_t value, uint32_t size) 
{
	uint32_t* s = (uint32_t*)src;
	while (size-- > 0){
		*s++ = value;
	}
	return src;
}

void *memcpy(void* _dst, const void* _src, uint32_t size) {
 
   uint8_t* dst = _dst;
   const uint8_t* src = _src;
   while (size-- > 0)
      *dst++ = *src++;
    return _dst;
}

char* strcpy(char* _dst, const char* _src) {
  
   char* r = _dst;		       
   while((*_dst++ = *_src++));
   return r;
}

char* strncpy(char* _dst, const char* _src, int n) 
{
   char *s = (char *) _src;
   char* r = _dst;		      
   while((*_dst++ = *s++) && n > 0) n--;
   return r;
}

uint32_t strlen(const char* str) {
  
   const char* p = str;
   while(*p++);
   return (p - str - 1);
}

int8_t strcmp (const char* a, const char* b) {
  
   while (*a != 0 && *a == *b) {
      a++;
      b++;
   }
   return *a < *b ? -1 : *a > *b;
}

/**
 * strcoll - 需要根据本地语言做处理，为了简便，直接调用strcmp
 * 
*/
int strcoll(const char *str1, const char *str2)
{
    return strcmp(str1, str2);
}

int memcmp(const void * s1, const void *s2, int n)
{
	if ((s1 == 0) || (s2 == 0)) { /* for robustness */
		return (s1 - s2);
	}

	const char * p1 = (const char *)s1;
	const char * p2 = (const char *)s2;
	int i;
	for (i = 0; i < n; i++,p1++,p2++) {
		if (*p1 != *p2) {
			return (*p1 - *p2);
		}
	}
	return 0;
}

char * strrchr(const char * str,int ch)
{
    if (!str)
        return NULL;
    char * start = (char *)str;
    while(*str++);/*get the end of the string*/
    while(--str != start && *str != (char)ch);
    if(*str == (char)ch)
        return((char *)str);
    return NULL;
}

char* strcat(char* strDest , const char* strSrc)
{
    char* address = strDest;
    while(*strDest)
    {
        strDest++;
    }
    while((*strDest++=*strSrc++));
    return (char* )address;
}

int strpos(char *str, char ch)
{
	int i = 0;
	int flags = 0;
	while(*str){
		if(*str == ch){
			flags = 1;	//find ch
			break;
		}
		i++;
		str++;
	}
	if(flags){
		return i;
	}else{
		return -1;	//str over but not found
	}
}

char *strncat(char *dst, const char *src, int n)
{
	char *ret = dst;
	while(*dst != '\0'){
		dst++;
	}
	while(n && (*dst++ = *src++) != '\0'){
		n--;
	}
	dst = '\0';
	return ret;
}

char *strchr(const char *s, int c)
{
    if(s == NULL)
    {
        return NULL;
    }

    while(*s != '\0' || (*s == c))
    {
        if(*s == (char)c )
        {
            return (char *)s;
        }
        s++;
    }
    return NULL;
}

void* memmove(void* dst,const void* src,uint32_t count)
{
    char* tmpdst = (char*)dst;
    char* tmpsrc = (char*)src;

    if (tmpdst <= tmpsrc || tmpdst >= tmpsrc + count)
    {
        while(count--)
        {
            *tmpdst++ = *tmpsrc++; 
        }
    }
    else
    {
        tmpdst = tmpdst + count - 1;
        tmpsrc = tmpsrc + count - 1;
        while(count--)
        {
            *tmpdst-- = *tmpsrc--;
        }
    }

    return dst; 
}

char *itoa16_align(char * str, int num)
{
	char *	p = str;
	char	ch;
	int	i;
	//为0
	if(num == 0){
		*p++ = '0';
	}
	else{	//4位4位的分解出来
		for(i=28;i>=0;i-=4){		//从最高得4位开始
			ch = (num >> i) & 0xF;	//取得4位
			ch += '0';			//大于0就+'0'变成ASICA的数字
			if(ch > '9'){		//大于9就加上7变成ASICA的字母
				ch += 7;		
			}
			*p++ = ch;			//指针地址上记录下来。
		}
	}
	*p = 0;							//最后在指针地址后加个0用于字符串结束
	return str;
}

/**
 * strmet - 复制直到遇到某个字符串
 * @src: 要操作的字符串
 * @buf: 要保存的地方
 * @ch: 要遇到的字符串
 * 
 * 返回缓冲区中字符的长度
 */
int strmet(const char *src, char *buf, char ch)
{ 
	char *p = (char *)src;

    /* 没有遇到就一直复制直到字符串结束或者遇到 */
	while (*p && *p != ch) {
        *buf = *p++;
        buf++;
	}
    /* 最后添加结束字符 */
    *buf = '\0';
	return p - (char *)src;
}

/* 朴素的模式匹配算法 ，只用一个外层循环 */
char *strstr(const char *dest, const char *src) 
{
	char *tdest = (char *)dest;
	char *tsrc = (char *)src;
	int i = 0;//tdest 主串的元素下标位置，从下标0开始找，可以通过变量进行设置，从其他下标开始找！
	int j = 0;//tsrc 子串的元素下标位置
	while (i <= strlen(tdest) - 1 && j <= strlen(tsrc) - 1) {
		//字符相等，则继续匹配下一个字符
        if (tdest[i] == tsrc[j]) {
			i++;
			j++;
		} else { //在匹配过程中发现有一个字符和子串中的不等，马上回退到 下一个要匹配的位置
			i = i - j + 1;
			j = 0;
		}
	}
	//循环完了后j的值等于strlen(tsrc) 子串中的字符已经在主串中都连续匹配到了
	if (j == strlen(tsrc)) {
		return tdest + i - strlen(tsrc);
	}
 
	return NULL;
}

size_t strspn(const char *s, const char *accept)
{
    const char *p = s;
    const char *a;
    size_t count = 0;

    for (; *p != '\0'; ++p) {
        for (a = accept; *a != '\0'; ++a) {
            if (*p == *a)
                break;
        }
        if (*a == '\0')
            return count;
        ++count;
    }
    return count;
}

const char *strpbrk(const char *str1, const char *str2)
{
    if (str1 == NULL || str2 == NULL)
    {
        //perror("str1 or str2");
        return NULL;
    }
    const char *temp1 = str1;
    const char *temp2 = str2;

    while (*temp1 != '\0')
    {
        temp2 = str2; //将str2 指针从新指向在字符串的首地址
        while (*temp2 != '\0')
        {
            if (*temp2 == *temp1)
                return temp1;
            else
                temp2++;
        }
        temp1++;
    }
return NULL;
}

/*
 *本文件大部分都是从网上搜索到的代码，如有侵权，请联系我。
 */
