#include <dwin/dwin.h>
#include <dwin/hal.h>

/* xbook kernel header */
#include <xbook/driver.h>
#include <sys/ioctl.h>
#include <sys/input.h>
#include <unistd.h>
#include <xbook/virmem.h>
#include <xbook/memspace.h>

#ifndef   KDEVICE_LCD_NAME
#define   KDEVICE_LCD_NAME        "video"
#endif

static int lcd_init(struct dwin_lcd *lcd)
{
    int fd = device_open(KDEVICE_LCD_NAME, O_RDWR);
    if (fd < 0)
    {
        dwin_log("video device %s not found!\n", KDEVICE_LCD_NAME);
        return -1;
    }
    lcd->handle = fd;
    return 0;
}

static int lcd_exit(struct dwin_lcd *lcd)
{
    if (!lcd)
    {
        return -1;
    }
    if (lcd->handle >= 0)
    {
        device_close(lcd->handle);
        lcd->handle = -1;
    }
    return 0;
}

int lcd_map(struct dwin_lcd *lcd)
{
    if (lcd->handle < 0)
    {
        return -1;
    }

    video_info_t video_info;
    int ret = device_devctl(lcd->handle, VIDEOIO_GETINFO, (unsigned long) &video_info);
    if (ret < 0)
    {
        errprint("lcd: video get info failed!\n");
        return -1;
    }
    lcd->width = video_info.x_resolution;
    lcd->height = video_info.y_resolution;
    lcd->bpp = video_info.bits_per_pixel;
    keprint("lcd: width %d, height %d, bpp %d\n", lcd->width, lcd->height, lcd->bpp);
    unsigned int vram_size = video_info.x_resolution * video_info.y_resolution * lcd->bpp / 8;
    if (!vram_size)
    {
        errprint("lcd: video ram size is 0!\n");
        return -1;
    }
    keprint("lcd: lcd memory map size %x\n", vram_size);

    unsigned char *vram = device_mmap(lcd->handle, vram_size, IO_KERNEL);
    if (vram == NULL)
    {
        errprint("lcd: video mapped failed!\n");
        return -1;
    }
    lcd->vram_start = vram;
    keprint("lcd: lcd map addr %x\n", lcd->vram_start);

    return 0;
}

int lcd_unmap(struct dwin_lcd *lcd)
{
    if (lcd->handle < 0)
    {
        return -1;
    }
    if (!lcd->vram_start)
    {
        return -1;
    }
    memio_unmap(lcd->vram_start);
    lcd->vram_start = NULL;
    return 0;
}

struct dwin_hal_lcd __kdevice_lcd_hal = {
    .init = lcd_init,
    .exit = lcd_exit,
    .map = lcd_map,
    .unmap = lcd_unmap,
    
    .extension = NULL,
};
