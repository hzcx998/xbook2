#ifndef _LIB_x86_CONST_H
#define _LIB_x86_CONST_H

/* 页大小 */
#define PG_SIZE     4096
#define PG_ALIGN(x)    ((x + (PG_SIZE-1)) & (~PG_SIZE))

/* in 32 bits cpu, word size is 2. in 64 bits cpu, it's 4 */
#define _WORDSZ  2

#endif  /* _LIB_x86_CONST_H */
