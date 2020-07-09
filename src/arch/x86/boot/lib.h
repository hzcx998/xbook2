#ifndef BOOT_LIB_H
#define BOOT_LIB_H

void print_str(char *str);
void *memset(void* src, unsigned char value, unsigned int size);
void memcpy(const void* dst, const void* src, unsigned int size);
int memcmp(const void * s1, const void *s2, int n);
char* itoa(int num,char* dst,int radix);
char* uitoa(unsigned int num,char* dst,int radix);
void print_int(int num);
void print_hex(unsigned int num);
void put_char(char ch);

#endif  /* BOOT_LIB_H */
