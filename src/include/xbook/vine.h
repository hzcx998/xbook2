#ifndef _XBOOK_VINE_H
#define _XBOOK_VINE_H

#include "driver.h"

/* 藤蔓vine：藤蔓延伸到每一个驱动程序中，驱动程序需要在藤蔓头文件中给出声明 */
extern iostatus_t serial_driver_vine(driver_object_t *driver);
extern iostatus_t console_driver_vine(driver_object_t *driver);
extern iostatus_t ide_driver_vine(driver_object_t *driver);
extern iostatus_t rtl8139_driver_vine(driver_object_t *driver);
extern iostatus_t keyboard_driver_vine(driver_object_t *driver);
extern iostatus_t tty_driver_vine(driver_object_t *driver);
extern iostatus_t ramdisk_driver_vine(driver_object_t *driver);
extern iostatus_t null_driver_vine(driver_object_t *driver);
extern iostatus_t vfloppy_driver_vine(driver_object_t *driver);
extern iostatus_t vbe_driver_vine(driver_object_t *driver);
extern iostatus_t mouse_driver_vine(driver_object_t *driver);
extern iostatus_t ahci_driver_vine(driver_object_t *driver);
extern iostatus_t e1000_driver_vine(driver_object_t *driver);

#endif  /* _XBOOK_VINE_H */
