#ifndef _LIB_x86_CONST_H
#define _LIB_x86_CONST_H

/* 页大小 */
#define PG_SIZE     4096
#define PG_ALIGN(x)    ((x + (PG_SIZE-1)) & (~PG_SIZE))

#endif  /* _LIB_x86_CONST_H */
