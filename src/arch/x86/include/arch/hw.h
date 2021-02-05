#ifndef _X86_HARDWARE_H
#define _X86_HARDWARE_H

#include "config.h"

#ifdef X86_SERIAL_HW
void serial_hardware_init();
void serial_hardware_putchar(char ch);
#endif /* X86_SERIAL_HW */

void console_hardware_init();

#ifdef X86_CONSOLE_HW
void console_hardware_putchar(char ch);
#endif /* X86_CONSOLE_HW */

#endif	/* _X86_HARDWARE_H */