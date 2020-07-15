#ifndef _X86_DEBUG_H
#define _X86_DEBUG_H

void (*debug_putchar) (char ch);

void init_kernel_debug();
void init_serial_debug();
void serial_putchar(char ch);
void init_console_debug();
void console_putchar(char ch);

#endif  /* _X86_DEBUG_H */
