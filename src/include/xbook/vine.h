#ifndef _XBOOK_VINE_H
#define _XBOOK_VINE_H

#include "driver.h"

/* 藤蔓vine：藤蔓延伸到每一个驱动程序中，驱动程序需要在藤蔓头文件中给出声明 */
extern iostatus_t serial_driver_vine(driver_object_t *driver);
extern iostatus_t console_driver_vine(driver_object_t *driver);
extern iostatus_t ide_driver_vine(driver_object_t *driver);
extern iostatus_t rtl8139_driver_vine(driver_object_t *driver);
extern iostatus_t keyboard_driver_vine(driver_object_t *driver);

#endif  /* _XBOOK_VINE_H */
