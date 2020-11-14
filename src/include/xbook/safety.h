#ifndef _XBOOK_SAFETY_H
#define _XBOOK_SAFETY_H

#include <stdint.h>

int safety_check_range(void *src, unsigned long nbytes);
int mem_copy_from_user(void *dest, void *src, unsigned long nbytes);
int mem_copy_to_user(void *dest, void *src, unsigned long nbytes);

#endif /* _XBOOK_SAFETY_H */
