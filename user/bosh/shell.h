#ifndef _BOSH_SHELL_H
#define _BOSH_SHELL_H

#include <stddef.h>

#define APP_NAME "bosh"

int shell_event_poll(char *buf, int pid);
int shell_readline();
void shell_putchar(char ch);
int shell_printf(const char *fmt, ...);

#endif  /* _BOSH_SHELL_H */