#include "test.h"
#include <sys/reboot.h>

#define RB_AUTOBOOT     0x01234567
#define RB_HALT_SYSTEM  0xcdef0123
#define RB_ENABLE_CAD   0x89abcdef
#define RB_DISABLE_CAD  0
#define RB_POWER_OFF    0x4321fedc
#define RB_SW_SUSPEND   0xd000fce2
#define RB_KEXEC        0x45584543

int test_reboot(int argc, char *argv[])
{
    printf("%s: %d\n", $(RB_AUTOBOOT), reboot(RB_AUTOBOOT));
    printf("%s: %d\n", $(RB_HALT_SYSTEM), reboot(RB_HALT_SYSTEM));
    printf("%s: %d\n", $(RB_ENABLE_CAD), reboot(RB_ENABLE_CAD));
    printf("%s: %d\n", $(RB_DISABLE_CAD), reboot(RB_DISABLE_CAD));
    printf("%s: %d\n", $(RB_POWER_OFF), reboot(RB_POWER_OFF));
    printf("%s: %d\n", $(RB_SW_SUSPEND), reboot(RB_SW_SUSPEND));
    printf("%s: %d\n", $(RB_KEXEC), reboot(RB_KEXEC));
    return 0;
}
