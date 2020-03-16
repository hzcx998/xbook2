#include <xbook/string.h>

void *memset(void* src, uint8_t value, uint32_t size) 
{
	uint8_t* s = (uint8_t*)src;
	while (size > 0){
		*s++ = value;
		--size;
	}
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

void memcpy(const void* dst, const void* src, uint32_t size)
{
    uint8_t *_dst = (uint8_t *)dst;
    uint8_t *_src = (uint8_t *)src;
    while (size-- > 0)
        *_dst++ = *_src++;
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
