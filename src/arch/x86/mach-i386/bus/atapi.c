#include <xbook/debug.h>
#include <xbook/disk.h>
#include <xbook/hardirq.h>
#include <xbook/schedule.h>
#include <xbook/waitqueue.h>

#include <assert.h>
#include <arch/io.h>
#include <arch/atapi.h>

#define ATAPI_DEBUG 0

static volatile int _ide_irq_lock = 0;

static struct IDE_Device ata_dev[4] =
{
    {.command = ATA_BUS_PRIMARY,   .control = ATA_REG_CTL_PRI, .irq = ATA_IRQ_PRIMARY,   .slave = ATA_MASTER},
    {.command = ATA_BUS_PRIMARY,   .control = ATA_REG_CTL_PRI, .irq = ATA_IRQ_PRIMARY,   .slave = ATA_SLAVE},
    {.command = ATA_BUS_SECONDARY, .control = ATA_REG_CTL_PRI, .irq = ATA_IRQ_SECONDARY, .slave = ATA_MASTER},
    {.command = ATA_BUS_SECONDARY, .control = ATA_REG_CTL_PRI, .irq = ATA_IRQ_SECONDARY, .slave = ATA_SLAVE},
};

static uint8_t *_atapi_buf = (uint8_t *)ATAPI_BUFFER_ADDR;

static char _atapi_read_cmd[12] =
{
    0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char ide_dev_error(struct IDE_Device *ide_dev)
{
    uint8_t msg = in8(ide_dev->command + 7);

    if ((msg >> 0) & 1)
    {
        if (msg & 0x80)
        {
            errprint("[ide]: bad sector\n");
        }
        else if (msg & 0x40)
        {
            errprint("[ide]: uncorrectable data\n");
        }
        else if (msg & 0x20)
        {
            errprint("[ide]: no media\n");
        }
        else if (msg & 0x10)
        {
            errprint("[ide]: id mark not found\n");
        }
        else if (msg & 0x08)
        {
            errprint("[ide]: no media\n");
        }
        else if (msg & 0x04)
        {
            errprint("[ide]: command aborted\n");
        }
        else if (msg & 0x02)
        {
            errprint("[ide]: track 0 not found\n");
        }
        else if (msg & 0x01)
        {
            errprint("[ide]: no address mark\n");
        }

        return 1;
    }

    return 0;
}

static void ide_wait_for_ready(struct IDE_Device *ide_dev)
{
    uint8_t dev = 0;

    while ((dev = in8(ide_dev->command + 7)) & ATA_SR_BSY)
    {
        if (dev & ATA_SR_DF)
        {
            panic("[ide]: ERROR ATA_SR_DF(0x%x)\n", ATA_SR_DF);
        }
        if (dev & ATA_SR_ERR)
        {
            panic("[ide]: ERROR ATA_SR_ERR(0x%x)\n", ATA_SR_ERR);
        }
    }
}

static void ide_reset_irq()
{
    _ide_irq_lock = 0;
}

static void ide_wait_irq()
{
    while (_ide_irq_lock == 0)
    {
        task_yield();
    }
}

static int atapi_handler(irqno_t irq, void *data)
{
#if ATAPI_DEBUG
    dbgprint("[atapi] handler\n");
#endif

    _ide_irq_lock = 1;
    in8(ATA_BUS_SECONDARY_CONTROL_PRI);
    in8(ATA_BUS_PRIMARY_CONTROL_PRI);
    out8(0x20, ATA_EOI);
    out8(0xA0, ATA_EOI);

    return 0;
}

static void atapi_read_sector(struct IDE_Device *ide_dev, unsigned long lba, uint8_t count, uint16_t *half_buf)
{
    uint16_t *cmd_ptr = (uint16_t *)&_atapi_read_cmd;
    uint16_t size;
    int i;

    ide_dev_error(ide_dev);
    ide_wait_for_ready(ide_dev);

    out8(ide_dev->command + 6, ide_dev->slave == ATA_SLAVE ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER);
    out8(ide_dev->command + 1, 0x00);
    out8(ide_dev->command + 4, ATAPI_SECTOR_SIZE & 0xff);
    out8(ide_dev->command + 5, ATAPI_SECTOR_SIZE >> 8);
    out8(ide_dev->command + 7, 0xA0);

    ide_dev_error(ide_dev);
    ide_wait_for_ready(ide_dev);

    _atapi_read_cmd[9] = count;
    _atapi_read_cmd[2] = (lba >> 0x18) & 0xFF;
    _atapi_read_cmd[3] = (lba >> 0x10) & 0xFF;
    _atapi_read_cmd[4] = (lba >> 0x08) & 0xFF;
    _atapi_read_cmd[5] = (lba >> 0x00) & 0xFF;
    _atapi_read_cmd[0] = 0xA8;

    ide_reset_irq();
    ide_dev_error(ide_dev);
    ide_wait_for_ready(ide_dev);

    for (i = 0; i < 6; i++)
    {
        outportw(ide_dev->command + 0, cmd_ptr[i]);
    }

    ide_dev_error(ide_dev);
    ide_wait_irq();
    ide_wait_for_ready(ide_dev);

    size = (((int)in8(ide_dev->command + 5)) << 8) | (int)(in8(ide_dev->command + 4));
    ide_wait_for_ready(ide_dev);

    for (i = 0; i < (size / 2); i++)
    {
        if (ide_dev_error(ide_dev) == 1)
        {
            return;
        }
        ide_wait_for_ready(ide_dev);
        *half_buf++ = inportw(ide_dev->command + 0);
    }

    ide_wait_for_ready(ide_dev);
}

static void ata_read_sector(struct IDE_Device *ide_dev, unsigned long LBA, uint8_t count, uint16_t *half_buf)
{
    uint8_t cunt = count;
    int i = 0;

    ide_reset_irq();

    out8(ide_dev->command + 6, 0xE0 | (ide_dev->slave << 4) | ((LBA >> 24) & 0x0F));
    out8(ide_dev->command + 2, (uint8_t)cunt);
    out8(ide_dev->command + 3, (uint8_t)LBA);
    out8(ide_dev->command + 4, (uint8_t)(LBA >> 8));
    out8(ide_dev->command + 5, (uint8_t)(LBA >> 16));
    out8(ide_dev->command + 7, 0x20);

    ide_wait_irq();

    for (i = 0; i < 256; ++i)
    {
        *half_buf++ = (uint16_t)inportw(ide_dev->command);
    }
}

static void ata_write_sector(struct IDE_Device *ide_dev, unsigned long LBA, uint8_t count, uint16_t *half_buf)
{
    uint8_t cunt = count;
    int i = 0;

    ide_reset_irq();

    out8(ide_dev->command + 6, 0xE0 | (ide_dev->slave << 4) | ((LBA >> 24) & 0x0F));
    out8(ide_dev->command + 2, (uint8_t)cunt);
    out8(ide_dev->command + 3, (uint8_t)LBA);
    out8(ide_dev->command + 4, (uint8_t)(LBA >> 8));
    out8(ide_dev->command + 5, (uint8_t)(LBA >> 16));
    out8(ide_dev->command + 7, 0x30);

    ide_wait_for_ready(ide_dev);

    for (i = 0; i < 256; i++)
    {
        outportw(ide_dev->command, *half_buf++);
    }

    ide_wait_for_ready(ide_dev);
    ide_reset_irq();

    out8(ide_dev->command + 6, 0xE0 | (ide_dev->slave << 4));
    out8(ide_dev->command + 2, (uint8_t)0);
    out8(ide_dev->command + 3, (uint8_t)0);
    out8(ide_dev->command + 4, (uint8_t)0);
    out8(ide_dev->command + 5, (uint8_t)0);
    out8(ide_dev->command + 7, 0xE7);

    ide_wait_irq();
}

static void init_ide_device(struct IDE_Device *ide_dev)
{
    int i;

    irq_register(ide_dev->irq, atapi_handler, IRQF_DISABLED, "atapi", "atapi-ide", NULL);

    infoprint("[ide]: initialising device CMD=0x%x CTRL=0x%x IRQ=0x%x SLV=0x%x\n",
            ide_dev->command,
            ide_dev->control,
            ide_dev->irq,
            ide_dev->slave == ATA_SLAVE ? "SLAVE" : "MASTER");

    ide_reset_irq();

    out8(ide_dev->command + 6, ide_dev->slave == ATA_SLAVE ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER);
    out8(ide_dev->command + 2, 0);
    out8(ide_dev->command + 3, 0);
    out8(ide_dev->command + 4, 0);
    out8(ide_dev->command + 5, 0);
    out8(ide_dev->command + 7, 0xEC);

    if (in8(ide_dev->command + 7) == 0)
    {
        warnprint("[ide]: device does not exist!\n");

        return;
    }

    for (;;)
    {
        if ((in8(ide_dev->command + 7) & 0x80) > 0)
        {
            break;
        }
        else if (!(in8(ide_dev->command + 4) == 0 && in8(ide_dev->command + 5) == 0))
        {
            break;
        }
        else if(_ide_irq_lock)
        {
            break;
        }
    }

    if (in8(SATA_LBAMID_PRI) == SATA_INT_GET_RET || in8(SATA_LBAHI_PRI) == SATA_INT_GET_RET)
    {
        infoprint("[ide]: Device is sata");
        warnprint(" not supported yet\n");

        return;
    }

    if (in8(ide_dev->command + 4) == 0 && in8(ide_dev->command + 5) == 0)
    {
        uint8_t *identbuffer;
        struct IDE_IDENTIFY *ident;
        uint16_t datapart;
        uint8_t datapartA, datapartB;

        infoprint("[ide]: device is ata\n");

        identbuffer = (uint8_t *)mem_alloc(sizeof(struct IDE_IDENTIFY));

        for (i = 0; i < 256; i++)
        {
            datapart  = inportw(ide_dev->command);
            datapartA = (datapart >> 8) & 0xFF;
            datapartB = datapart & 0xFF;
            identbuffer[(i * 2) + 0] = datapartA;
            identbuffer[(i * 2) + 1] = datapartB;
        }

        ident = (struct IDE_IDENTIFY *)identbuffer;
        ident->unused2[0] = 0;
        ident->unused3[0] = 0;

        infoprint("[ide]: ata version=%s name=%s\n", ident->version, ident->name);
    }
    else
    {
        for (i = 0; i < 256; i++)
        {
            inportw(ide_dev->command);
        }

        out8(ide_dev->command + 6, ide_dev->slave == ATA_SLAVE ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER);
        out8(ide_dev->command + 2, 0);
        out8(ide_dev->command + 3, 0);
        out8(ide_dev->command + 4, 0);
        out8(ide_dev->command + 5, 0);
        out8(ide_dev->command + 7, 0xA1);

        if (in8(ide_dev->command + 7) == 0)
        {
            warnprint("[ide]: device does not exist!\n");

            return;
        }
        if (ide_dev_error(ide_dev) == 0)
        {
            uint8_t *identbuffer;
            struct IDE_IDENTIFY *ident;
            uint16_t datapart;
            uint8_t datapartA, datapartB;
            int choice = -1;

            infoprint("[ide]: device is atapi\n");
            identbuffer = (uint8_t *)mem_alloc(sizeof(struct IDE_IDENTIFY));

            for (i = 0; i < 256; i++)
            {
                datapart  = inportw(ide_dev->command);
                datapartA = (datapart >> 8) & 0xFF;
                datapartB = datapart & 0xFF;
                identbuffer[(i * 2) + 0] = datapartA;
                identbuffer[(i * 2) + 1] = datapartB;
            }

            ident = (struct IDE_IDENTIFY *)identbuffer;
            ident->unused2[0] = 0;
            ident->unused3[0] = 0;
            infoprint("[ide]: atapi version=%s name=%s\n", ident->version, ident->name);

            ide_dev->name = "cdrom";
            ide_dev->sector_size = ATAPI_SECTOR_SIZE;
            ide_dev->class_id = ATAPI_CLASS_ID;

            atapi_read_sector(ide_dev, 0, 1, (uint16_t *)_atapi_buf);
            infoprint("[atapi]: cdrom is %sbootable\n", (_atapi_buf[510] == 0x55 && _atapi_buf[511] == 0xAA) ? "" : "not ");

            for (i = 0; i < 10; i++)
            {
                atapi_read_sector(ide_dev, 0x10 + i, 1, (uint16_t *)_atapi_buf);
                if (_atapi_buf[1] == 'C' &&
                    _atapi_buf[2] == 'D' &&
                    _atapi_buf[3] == '0' &&
                    _atapi_buf[4] == '0' &&
                    _atapi_buf[5] == '1')
                {
                    choice = i;
                    break;
                }
            }

            if (choice == -1)
            {
                warnprint("[atapi]: %s is unknown filesystem\n", ide_dev->name);
            }
            else
            {
                infoprint("[atapi]: %s is ISO9660 filesystem\n", ide_dev->name);
            }
        }
    }
}

int disk_init()
{
    int i;
    for (i = 0; i < sizeof(ata_dev) / sizeof(ata_dev[0]); ++i)
    {
        init_ide_device(&ata_dev[i]);
    }

    return 0;
}

int disk_match(disk_t *disk)
{
    int i;
    for (i = 0; i < sizeof(ata_dev) / sizeof(ata_dev[0]); ++i)
    {
        if (ata_dev[i].name != NULL && !strcmp(disk->name, ata_dev[i].name))
        {
            disk->class_id = ata_dev[i].class_id;
            disk->sector_size = ata_dev[i].sector_size;
            disk->ptr = (void *)&ata_dev[i];
            return 0;
        }
    }

    return -1;
}

int disk_write_sector(disk_t *disk, unsigned long lba, uint8_t count, uint16_t *half_buf)
{
    switch (disk->class_id)
    {
    case ATAPI_CLASS_ID:
        return -1;
    case ATA_CLASS_ID:
        ata_write_sector((struct IDE_Device *)(disk->ptr), lba, count, (uint16_t *)half_buf);
        break;
    case SATA_CLASS_ID:
        return -1;
    }
    return 0;
}

int disk_read_sector(disk_t *disk, unsigned long lba, uint8_t count, uint16_t *half_buf)
{
    switch (disk->class_id)
    {
    case ATAPI_CLASS_ID:
        atapi_read_sector((struct IDE_Device *)(disk->ptr), lba, count, (uint16_t *)half_buf);
        break;
    case ATA_CLASS_ID:
        ata_read_sector((struct IDE_Device *)(disk->ptr), lba, count, (uint16_t *)half_buf);
        break;
    case SATA_CLASS_ID:
        return -1;
    }

    return 0;
}
