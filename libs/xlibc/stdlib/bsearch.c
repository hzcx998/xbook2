#include <stdlib.h>

void* bsearch (const void* key, const void* base,
    size_t num, size_t size,
    int (*compar)(const void*,const void*))
{
    size_t low = 0;
    size_t high = num-1;
    size_t mid;
    while (low <= high) {
        mid = (low + high) >> 1;
        if (compar(key, (char *)base + mid * size) == 0) {
            break;
        }
        else if (compar(key, (char *)base + mid * size) < 0) {
            high = mid-1;
        }
        else {
            low = mid+1;
        }
    }

    if (low <= high) {
        return (char *)base + mid * size;
    }

    return NULL;
}