#include <arch/misc.h>
#include <xbook/debug.h>

void __attribute__((noreturn)) reboot(void) {
    keprint(PRINT_INFO "reboot.\n");

    __asm__ __volatile__ ("cli");

    __asm__ __volatile__ (
        "movb $0x64, %%al;\n"
        "orb $0xfe, %%al;\n"
        "outb %%al, $0x64;\n"
        "movb $0xfe, %%al;\n"
        "outb %%al, $0x64;\n"
        :
        :);

#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)
#endif
#if __GNUC__ && GCC_VERSION >= 40500
    __builtin_unreachable();
#else
    __asm__ __volatile__ ("cli; hlt");
    for (;;);
#endif
}
