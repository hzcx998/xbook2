#ifndef _X86_IO_H
#define _X86_IO_H

unsigned char __in8(unsigned int port);
unsigned short __in16(unsigned int port);
unsigned int __in32(unsigned int port);
void __out8(unsigned int port, unsigned int data);
void __out16(unsigned int port, unsigned int data);
void __out32(unsigned int port, unsigned int data);

void __io_read(unsigned short port, void* buf, unsigned int n);
void __io_write(unsigned short port, void* buf, unsigned int n);

#endif  /* _X86_IO_H */
