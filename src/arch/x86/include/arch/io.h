#ifndef _ARCH_IO_H
#define _ARCH_IO_H

unsigned char __in8(unsigned int port);
unsigned short __in16(unsigned int port);
unsigned int __in32(unsigned int port);
void __out8(unsigned int port, unsigned int data);
void __out16(unsigned int port, unsigned int data);
void __out32(unsigned int port, unsigned int data);

void __io_read(unsigned short port, void* buf, unsigned int n);
void __io_write(unsigned short port, void* buf, unsigned int n);

#define in8         __in8
#define in16        __in16
#define in32        __in32

#define out8        __out8
#define out16       __out16
#define out32       __out32

#define io_read     __io_read
#define io_write    __io_write

#endif  /* _ARCH_IO_H */
