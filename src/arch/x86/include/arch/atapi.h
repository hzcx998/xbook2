#ifndef _X86_ATAPI_H
#define _X86_ATAPI_H

#include <stdint.h>

/* ATAPI */
#define ATAPI_SECTOR_SIZE 2048
#define ATAPI_BUFFER_ADDR 0x2000

/* ATA */
#define ATA_SR_BSY  0x80    /* busy */
#define ATA_SR_DRDY 0x40    /* drive ready */
#define ATA_SR_DF   0x20    /* drive write fault */
#define ATA_SR_DSC  0x10    /* drive seek complete */
#define ATA_SR_DRQ  0x08    /* data request ready */
#define ATA_SR_CORR 0x04    /* corrected data */
#define ATA_SR_IDX  0x02    /* index */
#define ATA_SR_ERR  0x01    /* error */

#define ATA_CMD_IDENTIFY            0xec
#define ATA_CMD_READ_SECTORS        0x20
#define ATA_CMD_WRITE_SECTORS       0x30
#define ATA_CMD_READ_MULTIPLE       0xc4
#define ATA_CMD_WRITE_MULTIPLE      0xc5
#define ATA_CMD_SET_MULTIPLE_MODE   0xc6
#define ATA_CMD_SET_FEATURES        0xef

#define ATA_DRIVE_MASTER    0xA0
#define ATA_DRIVE_SLAVE     0xB0
#define ATA_MASTER          0   /* ATA_DRIVE | (ATA_MASTER << 4) */
#define ATA_SLAVE           1   /* ATA_DRIVE | (ATA_SLAVE << 4) */

#define ATA_BUS_PRIMARY     0x1F0
#define ATA_BUS_SECONDARY   0x170

#define ATA_BUS_PRIMARY_CONTROL_PRI     0x1F7
#define ATA_BUS_SECONDARY_CONTROL_PRI   0x177

#define ATA_IRQ_PRIMARY     0x0E
#define ATA_IRQ_SECONDARY   0x0F

#define ATA_EOI             0x20

#define ATA_REG_CTL_PRI     0x3f6

/* SATA */
#define SATA_LBAMID_PRI     0x1F4
#define SATA_LBAHI_PRI      0x1F5
#define SATA_INT_GET_RET    0x3C

struct IDE_IDENTIFY
{
    uint8_t unused1[46];
    uint8_t version[3];
    uint8_t unused2[4];
    uint8_t name[20];
    uint8_t unused3[439];
};

struct IDE_Device
{
    const char *name;
    uint32_t sector_size;
#define ATAPI_CLASS_ID  0
#define ATA_CLASS_ID    1
#define SATA_CLASS_ID   2
    uint32_t class_id;
    uint16_t command;
    uint16_t control;
    uint8_t irq;
    uint8_t slave;
};

#endif /* _X86_ATAPI_H */