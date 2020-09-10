#include <stdlib.h>
#include <string.h>

void swap(char * a, char * b, size_t width)
{
    char tmp;
    while (width--) {
        tmp = *a;
        *a++ = *b;
        *b++ = tmp;
    }
}
 
char * partition(char * lo, char * hi, int (*comp)(const void *, const void *), size_t width) 
{
    char * i = lo;
	char * j = hi;
 
    while (i <= j) {
        do {
            i += width;
            if (i == hi)   break;
        } while ((*comp)(i, lo) < 0);
        
        while (1) {
			if ((*comp)(j, lo) <= 0)
				break;
			j -= width;
		}
        
        if (i <= j) 
            swap(i, j, width);
    }        
    swap(j, lo, width);
    return j;
}

static void sort(char * lo, char * hi, int (*comp)(const void *, const void *), size_t width) {
    char * p;
    if (lo >= hi)    return;
 
    p = partition(lo, hi, comp, width);
    sort(lo, p - width, comp, width);
    sort(p + width, hi, comp, width);
}

void qsort(void * base, size_t num, size_t width, int (*comp)(const void *, const void *)) {
    char * lo = (char *) base;
    char * hi = (char *) base;
    hi += (num - 1) * width;
    sort(lo, hi, comp, width);
}
 
