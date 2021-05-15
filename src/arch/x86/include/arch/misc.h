#ifndef _X86_MISC_H
#define _X86_MISC_H

void sys_reboot(void);
void sys_shutdown(void);

void __attribute__((noreturn)) reboot(void);
void __attribute__((noreturn)) halt(void);

#endif /* _X86_MISC_H */
