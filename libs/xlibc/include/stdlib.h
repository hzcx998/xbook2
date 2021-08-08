#ifndef _LIB_STDLIB_H
#define _LIB_STDLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include <sys/types.h>
#include "malloc.h"
#include "exit.h"
#include "stdint.h"
#include "environ.h"

#define RAND_MAX 0x7fff
#define MB_CUR_MAX 1

#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
typedef struct { long long quot, rem; } lldiv_t;

div_t div(int num, int den);
ldiv_t ldiv(long num, long den);
lldiv_t lldiv(long long num, long long den);

void srand(unsigned long seed);
int rand(void);

int random(void);
void srandom(unsigned long seed);

void qsort( void  * base, size_t n_elements, size_t el_size,
    int  (* compare ) (void const *, void const *) );

long strtol(const char * nptr, char ** endptr, int base);
long long strtoll(const char * nptr, char ** endptr, int base);
unsigned long strtoul(const char * nptr, char ** endptr, int base);
unsigned long long strtoull(const char * nptr, char ** endptr, int base);
double strtod(const char * nptr, char ** endptr);
float strtof(const char * nptr, char ** endptr);
#define strtold strtod

intmax_t strtoimax(const char * nptr, char ** endptr, int base);
uintmax_t strtoumax(const char * nptr, char ** endptr, int base);
intmax_t strntoimax(const char * nptr, char ** endptr, int base, size_t n);
uintmax_t strntoumax(const char * nptr, char ** endptr, int base, size_t n);

int atoi(const char *src);
long long atoll(const char * nptr);
double atof(const char * nptr);
long atol(const char * nptr);

int abs(int const i);
long int labs(long int const i);
void atexit(void (*func)(void));

int system(const char * cmd);

char *realpath(const char *path, char *resolved_path);

void* bsearch (const void* key, const void* base,
    size_t num, size_t size,
    int (*compar)(const void*,const void*));

#ifdef __cplusplus
}
#endif

#endif  /* _LIB_STDLIB_H */
