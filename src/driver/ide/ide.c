#include <xbook/unit.h>
#include <xbook/debug.h>

static int ide_init()
{
    printk("hello, ide!\n");
    return 0;
}

static void ide_exit()
{
	
    printk("bye, ide!\n");
}

unit_t ide_unit = {
    .name = "ide",
    .version = "0.01",
    .login = ide_init,
    .logout = ide_exit,
};
