#ifndef __STDLIB_H__
#define __STDLIB_H__

void panic(char *);

#define WEXITSTATUS(s) (((s) & 0xff00) >> 8)

#ifndef assert
#define assert(f) \
    if (!(f))     \
	panic("\n --- Assert Fatal ! ---\n")
#endif

#endif //__STDLIB_H__
