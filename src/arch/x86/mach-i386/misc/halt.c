#include <arch/misc.h>
#include <arch/acpi.h>
#include <xbook/debug.h>

void __attribute__((noreturn)) halt(void) {
    keprint(PRINT_INFO "halt.\n");
    __asm__ __volatile__ ("cli");

    // ACPI shutdown
    if (!acpi_init()) {
        keprint(PRINT_WARING "ACPI init fail.\n");
    }
    acpi_shutdown();

    /*
     *  Summary: https://wiki.osdev.org/Shutdown
     *  Shutdown in virtual machine
     */

    // Works for QEMU (newer)
    __asm__ __volatile__ ("outw %%ax, %%dx" :: "d" (0x604), "a" (0x2000));
    // Works for QEMU (than 2.0) and bochs.
    __asm__ __volatile__ ("outw %%ax, %%dx" :: "d" (0xb004), "a" (0x2000));

    // Magic shutdown code for Bochs and QEMU.
    // Removed in newer QEMU releases.
    const char *MSDC = "Shutdown";
    while (*MSDC) {
        __asm__ __volatile__ ("outb %%al, %%dx" :: "d" (0x8900), "a" (*MSDC++));
    }

    // Works for Virtualbox
    __asm__ __volatile__ ("outw %%ax, %%dx" :: "d" (0x4004), "a" (0x3400));

    // Magic code for VMWare. Also a hard lock.
    __asm__ __volatile__ ("cli; hlt");

#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)
#endif
#if __GNUC__ && GCC_VERSION >= 40500
    __builtin_unreachable();
#else
    for (;;);
#endif
}
