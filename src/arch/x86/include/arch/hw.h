#ifndef _X86_HW_H
#define _X86_HW_H

#include "config.h"

#ifdef X86_SERIAL_HW
void serial_hw_init();
void serial_putchar(char ch);
#endif /* X86_SERIAL_HW */

void console_hw_init();

#ifdef X86_CONSOLE_HW
void console_putchar(char ch);
#endif /* X86_CONSOLE_HW */

#endif	/* _X86_HW_H */