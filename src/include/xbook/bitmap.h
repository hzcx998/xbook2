#ifndef _XBOOK_BITMAP_H
#define _XBOOK_BITMAP_H

#define BITMAP_MASK 1

typedef struct bitmap {
   unsigned long byte_length;
   /* 在遍历位图时,整体上以字节为单位,细节上是以位为单位,所以此处位图的指针必须是单字节 */
   unsigned char* bits;
} bitmap_t;

void bitmap_init(bitmap_t* btmp);
bool bitmap_scan_test(bitmap_t* btmp, unsigned long idx);
long bitmap_scan(bitmap_t* btmp, unsigned long cnt);
void bitmap_set(bitmap_t* btmp, unsigned long idx, char value);
long bitmap_change(bitmap_t *btmp, unsigned long idx);
long bitmap_test_and_change(bitmap_t *btmp, unsigned long idx);

#endif  /* _XBOOK_BITMAP_H */
