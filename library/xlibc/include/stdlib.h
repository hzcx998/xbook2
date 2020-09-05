#ifndef _LIB_STDLIB_H
#define _LIB_STDLIB_H

#include "stddef.h"
#include "types.h"
#include "malloc.h"
#include "exit.h"
#include "stdint.h"

#define RAND_MAX 0x7fff

void srand(unsigned long seed);
int rand();

void qsort( void  * base, size_t n_elements, size_t el_size,
    int  (* compare ) (void const *, void const *) );
void sort(char * lo, char * hi, 
    int (*comp)(const void *, const void *), size_t width);

long strtol(const char * nptr, char ** endptr, int base);
long long strtoll(const char * nptr, char ** endptr, int base);
unsigned long strtoul(const char * nptr, char ** endptr, int base);
unsigned long long strtoull(const char * nptr, char ** endptr, int base);
double strtod(const char * nptr, char ** endptr);
float strtof(const char * nptr, char ** endptr);

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

#endif  /* _LIB_STDLIB_H */
