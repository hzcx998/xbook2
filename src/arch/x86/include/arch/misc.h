#ifndef _X86_MISC_H
#define _X86_MISC_H

#define sys_reboot reboot
#define sys_shutdown halt

void __attribute__((noreturn)) reboot(void);
void __attribute__((noreturn)) halt(void);

#endif /* _X86_MISC_H */
