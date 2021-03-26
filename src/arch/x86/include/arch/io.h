#ifndef _X86_IO_H
#define _X86_IO_H
/* copyright (c) 2021 AlanCui*/
__attribute__((always_inline)) inline static unsigned long ioport_in32(unsigned short port)
{
    unsigned long tmp = 0;
    __asm__ __volatile__("inl %%dx,%%eax"
                         : "=a"(tmp)
                         : "d"(port)
                         :);
    return tmp;
}
__attribute__((always_inline)) inline static unsigned short ioport_in16(unsigned short port)
{
    unsigned short tmp = 0;
    __asm__ __volatile__("inw %%dx,%%ax"
                         : "=a"(tmp)
                         : "d"(port)
                         :);
    return tmp;
}
__attribute__((always_inline)) inline static unsigned char ioport_in8(unsigned short port)
{
    unsigned char tmp = 0;
    __asm__ __volatile__("inb %%dx,%%al"
                         : "=a"(tmp)
                         : "d"(port)
                         :);
    return tmp;
}
__attribute__((always_inline)) inline static void ioport_out32(unsigned short port, unsigned long data)
{
    __asm__ __volatile__("outl %%eax,%%dx" ::"d"(port), "a"(data)
                         :);
}
__attribute__((always_inline)) inline static void ioport_out16(unsigned short port, unsigned short data)
{
    __asm__ __volatile__("outw %%ax,%%dx" ::"d"(port), "a"(data)
                         :);
}
__attribute__((always_inline)) inline static void ioport_out8(unsigned short port, unsigned char data)
{
    __asm__ __volatile__("outb %%al,%%dx" ::"d"(port), "a"(data)
                         :);
}
/* copyright (c) 2021 AlanCui END*/
void ioport_read_bytes(unsigned short port, void* buf, unsigned int n);
void ioport_write_bytes(unsigned short port, void* buf, unsigned int n);

#define in8         ioport_in8
#define in16        ioport_in16
#define in32        ioport_in32

#define out8        ioport_out8
#define out16       ioport_out16
#define out32       ioport_out32

#define io_read     ioport_read_bytes
#define io_write    ioport_write_bytes

#endif  /* _X86_IO_H */
