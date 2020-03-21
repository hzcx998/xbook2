#include <xbook/unit.h>
#include <xbook/debug.h>
#include <xbook/string.h>
#include <xbook/device.h>

/* import unit */
IMPORT_UNIT(ide_unit);
IMPORT_UNIT(uart_unit);

void init_unit()
{
    /* login here */
    unit_login(&uart_unit);
    
    dev_open(DEV_COM1, 0);
    dev_write(DEV_COM1, 0, "hello, com1\n", 12);

    printk(KERN_INFO "runs well at file:%s line:%d\n", __FILE__, __LINE__);
    
    logger("hello, wrold! %d %s\n", 123, "jason");
    /** unit_logout("uart"); */
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