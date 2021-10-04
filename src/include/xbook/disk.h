#ifndef _XBOOK_DISK_H
#define _XBOOK_DISK_H

#include <stdint.h>

typedef struct
{
    const char *name;
    void *ptr;
    uint32_t class_id;

    uint32_t sector_size;
    uint8_t *data;
} disk_t;

int disk_init();
int disk_match(disk_t *disk);
int disk_write_sector(disk_t *disk, unsigned long lba, uint8_t count, uint16_t *half_buf);
int disk_read_sector(disk_t *disk, unsigned long lba, uint8_t count, uint16_t *half_buf);

#endif /* _XBOOK_DISK_H */
