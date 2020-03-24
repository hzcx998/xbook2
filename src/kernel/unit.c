#include <xbook/unit.h>
#include <xbook/debug.h>
#include <xbook/string.h>
#include <xbook/device.h>

/* import unit */
IMPORT_UNIT(ide_unit);
IMPORT_UNIT(uart_unit);
IMPORT_UNIT(console_unit);

void init_unit()
{
    /* login here */
    unit_login(&uart_unit);
    unit_login(&console_unit);
    unit_login(&ide_unit);

#if 0
    dev_open(DEV_COM1, 0);
    dev_write(DEV_COM1, 0, "hello, com1\n", 12);

    printk(KERN_INFO "runs well at file:%s line:%d\n", __FILE__, __LINE__);
    
    logger("hello, wrold! %d %s\n", 123, "jason");

    dev_open(DEV_CON0, 0);
    dev_write(DEV_CON0, 0, "hello, con0\n", 12);
    dev_close(DEV_CON0);

    dev_open(DEV_HD0, 0);
    
    char *buf = kmalloc(10 * SECTOR_SIZE);
    dev_read(DEV_HD0, 0, buf, 10);
    
    printk("%x, %x, %x, %x\n", buf[0], buf[511],buf[512],buf[1023]);

    memset(buf, 0x11, 10 * SECTOR_SIZE);
    
    dev_write(DEV_HD0, 0, buf, 10);
    memset(buf, 0, 10 * SECTOR_SIZE);
    
    dev_read(DEV_HD0, 0, buf, 10);    
    printk("%x, %x, %x, %x\n", buf[0], buf[511],buf[512],buf[1023]);

    dev_close(DEV_CON0);
#endif
}

/* 所有单元链表头 */
LIST_HEAD(unit_list_head);

int unit_login(unit_t *unit)
{
    if (unit->login) {
        if (!unit->login()) {
            list_add_tail(&unit->list, &unit_list_head);
            return 0;
        }
    }
    return -1;
}


unit_t *unit_find(char *name)
{
    unit_t *unit;
    list_for_each_owner (unit, &unit_list_head, list) {
        if (!strcmp(unit->name, name)) {
            return unit;
        }
    }
    return NULL;
}


int unit_logout(char *name)
{
    unit_t *unit = unit_find(name);
    if (unit) {
        unit->logout();
        list_del(&unit->list);
        return 0;
    }
    printk("unit %s not found!\n", name);
    return -1;
}