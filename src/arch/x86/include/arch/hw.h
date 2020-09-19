#ifndef _X86_HW_H
#define _X86_HW_H

#include "config.h"

#ifdef X86_SERIAL_HW
void init_serial_hw();
void serial_putchar(char ch);
#endif /* X86_SERIAL_HW */

#ifdef X86_CONSOLE_HW
void init_console_hw();
void console_putchar(char ch);
#endif /* X86_CONSOLE_HW */

#endif	/* _X86_HW_H */