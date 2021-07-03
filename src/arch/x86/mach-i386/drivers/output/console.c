#include <xbook/debug.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/kernel.h>
#include <arch/io.h>
#include <arch/config.h>
#include <arch/hw.h>
#include <sys/ioctl.h>
#include <stdio.h>

#ifdef KERN_VBE_MODE
#include <arch/module.h>
#include <arch/page.h>
#include <cpio.h>
#endif /* KERN_VBE_MODE */

#define DRV_NAME    "vga-console"
#define DRV_VERSION "0.1"

#define DEV_NAME    "con"

#define MAX_CONSOLE_NR  8   /* 8个控制台 */

// #define DEBUG_DRV

#define VGA_VRAM        (KERN_BASE_VIR_ADDR + 0x000B8000)

#define CRTC_ADDR_REG   0x3D4   /* CRT Controller Registers - Addr Register */
#define CRTC_DATA_REG   0x3D5   /* CRT Controller Registers - Data Register */

#define START_ADDR_H    0xC     /* reg index of video mem start addr (MSB) */
#define START_ADDR_L    0xD     /* reg index of video mem start addr (LSB) */
#define CURSOR_H        0xE     /* reg index of cursor position (MSB) */
#define CURSOR_L        0xF     /* reg index of cursor position (LSB) */

#define V_MEM_BASE      VGA_VRAM    /* base of color video memory */
#define V_MEM_SIZE      0x8000      /* 32K: B8000H -> BFFFFH */

#define COLOR_DEFAULT   (MAKE_COLOR(TEXT_BLACK, TEXT_WHITE))

/*
 *  颜色生成方法
 *  MAKE_COLOR(BLUE, RED)
 *  MAKE_COLOR(BLACK, RED) | BRIGHT
 *  MAKE_COLOR(BLACK, RED) | BRIGHT | FLASH
 */

#define TEXT_BLACK      0x0     /* 0000 */
#define TEXT_WHITE      0x7     /* 0111 */
#define TEXT_RED        0x4     /* 0100 */
#define TEXT_GREEN      0x2     /* 0010 */
#define TEXT_BLUE       0x1     /* 0001 */

#define TEXT_FLASH      0x80    /* 1000 0000 */
#define TEXT_BRIGHT     0x08    /* 0000 1000 */

#define MAKE_COLOR(x, y)    ((x << 4) | y)  /* MAKE_COLOR(Background, Foreground) */

#define SCREEN_UP       (-1)
#define SCREEN_DOWN     1
#ifdef KERN_VBE_MODE
static unsigned short SCREEN_WIDTH  = 80;
static unsigned short SCREEN_HEIGHT = 25;
#else
#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   25
#endif /* KERN_VBE_MODE */
#define SCREEN_SIZE     (80 * 25)

#define TAB_WIDTH       4

typedef struct _device_extension {
    string_t device_name;           /* 设备名字 */
    device_object_t *device_object; /* 设备对象 */

    unsigned int original_addr;     /* 控制台对应的显存的位置 */
    unsigned int screen_size;       /* 控制台占用的显存大小 */
    unsigned char color;            /* 字符的颜色 */
    unsigned int x, y;              /* 偏移坐标位置 */
} device_extension_t;

#ifdef KERN_VBE_MODE
#define UGA_FONT_W 8
#define UGA_FONT_H 16
#define UGA_CUR_CODE 219

static struct {
    unsigned short *addr;       /* 显存映射到内核的虚拟地址 */
    unsigned short x_sz, y_sz;
    unsigned short fill, clear;
    unsigned char *fonts;
    unsigned char enable;
} uga;

#define RGB16(r, g, b) ((r & 0xf8) << 8 | (g & 0xfc) << 3 | b >> 3)

static void uga_outchar(unsigned short x, unsigned short y, unsigned char ch) {
    if (uga.enable) {
        unsigned int fx = x * UGA_FONT_W;
        unsigned int fy = y * UGA_FONT_H;
        unsigned int fex = fx + UGA_FONT_W;
        unsigned int fey = fy + UGA_FONT_H;
        unsigned int fi = ch * UGA_FONT_H;

        for (; fy < fey; ++fy, ++fi) {
            fy *= UGA_FONT_W;
            for (; fx < fex; ++fx) {
                if (uga.fonts[fi] >> (fex - fx) & 1) {
                    uga.addr[fy * SCREEN_WIDTH + fx] = uga.fill;
                } else {
                    uga.addr[fy * SCREEN_WIDTH + fx] = uga.clear;
                }
            }
            fy /= UGA_FONT_W;
            fx -= UGA_FONT_W;
        }
    }
}

static void uga_clean() {
    if (uga.enable) {
        unsigned int size = uga.x_sz * uga.y_sz;
        for (; size > 0; --size) {
            uga.addr[size] = 0x0000;
        }
    }
}
#endif /* KERN_VBE_MODE */

static unsigned short get_cursor()
{
    unsigned short pos_low, pos_high;   // 设置光标位置的高位的低位

    // 取得光标位置
    out8(CRTC_ADDR_REG, CURSOR_H);      // 光标高位
    pos_high = in8(CRTC_DATA_REG);
    out8(CRTC_ADDR_REG, CURSOR_L);      // 光标低位
    pos_low = in8(CRTC_DATA_REG);

    return (pos_high << 8 | pos_low);   // 返回合成后的值
}

static void set_cursor(unsigned short cursor)
{
    unsigned long flags;

    // 执行前保存flags状态，然后关闭中断
    interrupt_save_and_disable(flags);
    out8(CRTC_ADDR_REG, CURSOR_H);      // 光标高位
    out8(CRTC_DATA_REG, (cursor >> 8) & 0xFF);
    out8(CRTC_ADDR_REG, CURSOR_L);      // 光标低位
    out8(CRTC_DATA_REG, cursor & 0xFF);

    // 恢复之前的flags状态
    interrupt_restore_state(flags);
}

static void set_video_start_addr(unsigned short addr)
{
    unsigned long flags;

    // 执行前保存flags状态，然后关闭中断
    interrupt_save_and_disable(flags);
    out8(CRTC_ADDR_REG, START_ADDR_H);
    out8(CRTC_DATA_REG, (addr >> 8) & 0xFF);
    out8(CRTC_ADDR_REG, START_ADDR_L);
    out8(CRTC_DATA_REG, addr & 0xFF);

    // 恢复之前的flags状态
    interrupt_restore_state(flags);
}

/**
 * flush - 刷新光标和起始位置
 * @console: 控制台
 */
static void flush(device_extension_t *ext)
{
    // 计算光标位置，并设置
#ifdef KERN_VBE_MODE
    uga_outchar(ext->x, ext->y, UGA_CUR_CODE);
    uga_outchar(ext->x+1, ext->y, ' ');
#endif /* KERN_VBE_MODE */
    set_cursor(ext->original_addr + ext->y * SCREEN_WIDTH + ext->x);
    set_video_start_addr(ext->original_addr);
}

/**
 * console_clean - 清除控制台
 * @console: 控制台
 */
static void console_clean(device_extension_t *ext)
{
    // 指向显存
    unsigned char *vram = (unsigned char *)(V_MEM_BASE + ext->original_addr * 2);
    int i;

    for (i = ext->screen_size * 2; i > 0; i -= 2) {
        *vram++ = '\0';             // 所有字符都置0
        *vram++ = COLOR_DEFAULT;    // 颜色设置为黑白
    }
    ext->x = ext->y = 0;

#ifdef KERN_VBE_MODE
    uga_clean();
#endif /* KERN_VBE_MODE */

    flush(ext);
}

#ifdef DEBUG_CONSOLE
static void dump_console(device_extension_t *ext)
{
    keprint(PART_TIP "----Console----\n");
    keprint(PART_TIP "origin:%d size:%d x:%d y:%d color:%x dev:%x\n",
        ext->original_addr, ext->screen_size, ext->x, ext->x, ext->color, ext->dev);
}
#endif /* DEBUG_CONSOLE */

/**
 * console_scroll - 滚屏
 * @console: 控制台
 * @direction: 滚动方向 [SCREEN_UP: 上 | SCREEN_DOWN: 下]
 */
static void console_scroll(device_extension_t *ext, int direction)
{
    // 指向显存
    unsigned char *vram = (unsigned char *)(V_MEM_BASE + ext->original_addr * 2);
    int i;

    if (direction == SCREEN_UP) {
        // 起始地址
        for (i = SCREEN_WIDTH * 2 * 24; i > SCREEN_WIDTH * 2; i -= 2) {
            vram[i] = vram[i - SCREEN_WIDTH * 2];
            vram[i + 1] = vram[i + 1 - SCREEN_WIDTH * 2];
        }
        for (i = 0; i < SCREEN_WIDTH * 2; i += 2) {
            vram[i] = '\0';
            vram[i + 1] = COLOR_DEFAULT;
        }
    } else if (direction == SCREEN_DOWN) {
        // 起始地址
        for (i = 0; i < SCREEN_WIDTH * 2 * 24; i += 2) {
            vram[i] = vram[i + SCREEN_WIDTH * 2];
            vram[i + 1] = vram[i + 1 + SCREEN_WIDTH * 2];
        }
        for (i = SCREEN_WIDTH * 2 * 24; i < SCREEN_WIDTH * 2 * 25; i += 2) {
            vram[i] = '\0';
            vram[i + 1] = COLOR_DEFAULT;
        }
        --ext->y;
    }

    flush(ext);
}

/**
 * vga_outchar - 控制台上输出一个字符
 * @console: 控制台
 * @ch: 字符
 */
static void vga_outchar(device_extension_t *ext, unsigned char ch)
{
    unsigned char *vram = (unsigned char *)(V_MEM_BASE +
        (ext->original_addr + ext->y * SCREEN_WIDTH + ext->x) * 2);

    switch (ch) {
    case '\n':
        // 如果是回车，那还是要把回车写进去
        *vram++ = '\0';
        *vram = COLOR_DEFAULT;

#ifdef KERN_VBE_MODE
        uga_outchar(ext->x, ext->y, ' ');
#endif /* KERN_VBE_MODE */

        ext->x = 0;
        ext->y++;

    break;
    case '\b':
        if (ext->x >= 0 && ext->y >= 0) {
            ext->x--;

            // 调整为上一行尾
            if (ext->x < 0) {
                ext->x = SCREEN_WIDTH - 1;
                ext->y--;
                // 对y进行修复
                if (ext->y < 0) {
                    ext->y = 0;
                }
            }

            *(vram-2) = '\0';
            *(vram-1) = COLOR_DEFAULT;
        }
    break;
    case '\r':
        /* 忽略掉 */
    break;
    case '\t':
        {
            int spaces = ((ext->x + TAB_WIDTH) & (~(TAB_WIDTH - 1))) - ext->x;
            while (spaces--) {
                vga_outchar(ext, ' ');
            }
        }
    default:
        *vram++ = ch;
        *vram = ext->color;

#ifdef KERN_VBE_MODE
        uga_outchar(ext->x, ext->y, ch);
#endif /* #ifdef KERN_VBE_MODE */

        ++ext->x;
        if (ext->x > SCREEN_WIDTH - 1) {
            ext->x = 0;
            ++ext->y;
        }
    break;
    }

    // 滚屏
    while (ext->y > SCREEN_HEIGHT - 1) {
        console_scroll(ext, SCREEN_DOWN);
    }

    flush(ext);
}


/**
 * vga_inchar - 控制台上读取一个字符
 * @console: 控制台
 * @ch: 字符
 */
static void vga_inchar(device_extension_t *ext, unsigned char *ch)
{
    unsigned char *vram = (unsigned char *)(V_MEM_BASE + 
        (ext->original_addr + ext->y * SCREEN_WIDTH + ext->x) *2);

    *ch = *vram;
    ++ext->x;

    if (ext->x > SCREEN_WIDTH - 1) {
        ext->x = 0;
        ext->y++;
    }

    // 滚屏
    while (ext->y > SCREEN_HEIGHT - 1) {
        console_scroll(ext, SCREEN_DOWN);
    }

    flush(ext);
}

/**
 * console_setpos - 光标移动到一个指定位置
 * @pos: 位置，x是高16位，y是低16位
 */
static void console_setpos(device_extension_t *ext, unsigned int pos)
{
    ext->x = pos % SCREEN_WIDTH;
    ext->y = pos / SCREEN_WIDTH;

    // 调整x位置
    if (ext->x < 0) {
        ext->x = 0;
    } else if (ext->x >= SCREEN_WIDTH) {
        ext->x = SCREEN_WIDTH - 1;
    }

    // 调整y位置
    if (ext->y < 0) {
        ext->y = 0;
    } else if (ext->y >= SCREEN_HEIGHT) {
        ext->y = SCREEN_HEIGHT - 1;
    }

    flush(ext);
}

/**
 * console_getpos - 获取光标位置
 * @pos: 位置，x是高16位，y是低16位
 */
static int console_getpos(device_extension_t *ext)
{
    return ext->y * SCREEN_WIDTH + ext->x;    
}

/**
 * console_set_color - 设置控制台字符颜色
 * @color: 颜色
 */
static void console_set_color(device_extension_t *ext, unsigned char color)
{
    ext->color = color;
}

/**
 * console_get_color - 获取控制台字符颜色
 * @return: 返回控制台颜色
 */
static unsigned char console_get_color(device_extension_t *ext)
{
    return ext->color;
}

iostatus_t console_read(device_object_t *device, io_request_t *ioreq)
{
    unsigned long len = ioreq->parame.read.length;
    uint8_t *buf = (uint8_t *)ioreq->system_buffer; 
    int i = len;

    while (i > 0) {
        vga_inchar(device->device_extension, buf);
        i--;
        buf++;
    }

#ifdef DEBUG_DRV  
    buf = (uint8_t *)ioreq->system_buffer; 
    keprint(PRINT_DEBUG "console_read: %s\n", buf);
#endif /* DEBUG_DRV */

    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = len;

    // 调用完成请求
    io_complete_request(ioreq);

    return IO_SUCCESS;
}

iostatus_t console_write(device_object_t *device, io_request_t *ioreq)
{
    unsigned long len = ioreq->parame.write.length;

    uint8_t *buf = (uint8_t *)ioreq->system_buffer; 
    int i = len;

#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "console_write: %s\n", buf);
#endif /* DEBUG_DRV */

    while (i > 0) {
        vga_outchar(device->device_extension, *buf);
#ifdef X86_SERIAL_HW
        serial_hardware_putchar(*buf);
#endif /* X86_SERIAL_HW */
        --i;
        ++buf;
    }

    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = len;

    // 调用完成请求
    io_complete_request(ioreq);

    return IO_SUCCESS;
}

iostatus_t console_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;
    unsigned long arg = ioreq->parame.devctl.arg;

    iostatus_t status = IO_SUCCESS;
    int infomation = 0;

    switch (ctlcode)
    {
    case CONIO_SCROLL:
        console_scroll(device->device_extension, arg);
    break;
    case CONIO_CLEAR:
        console_clean(device->device_extension);
    break;
    case CONIO_SETCOLOR:
        console_set_color(device->device_extension, arg);
    break;
    case CONIO_GETCOLOR:
        infomation = console_get_color(device->device_extension);
    break;
    case CONIO_SETPOS:
        console_setpos(device->device_extension, arg);
    break;
    case CONIO_GETPOS:
        infomation = console_getpos(device->device_extension);
    break;
    default:
        status = IO_FAILED;
    break;
    }

    ioreq->io_status.status = status;
    ioreq->io_status.infomation = infomation;
    io_complete_request(ioreq);

    return status;
}

#ifdef KERN_VBE_MODE
static void device_be_notify(driver_object_t *driver, int tag, void *param) {
    if (tag == 0) {
        unsigned long file_sz;
        uga.addr = (unsigned short *)(*((void **)param));
        uga.x_sz = *(unsigned short *)(*((void **)param+1));
        uga.y_sz = *(unsigned short *)(*((void **)param+2));

        uga.fonts = cpio_get_file(
            module_info_find(KERN_BASE_VIR_ADDR, MODULE_INITRD), "boot/uga.img", &file_sz);
        if (uga.fonts == NULL || file_sz != 32 * UGA_FONT_W * UGA_FONT_H * sizeof(char)) {
            uga.fonts = NULL;
            return;
        }

        uga.fill = RGB16(168, 168, 168);
        uga.clear = 0x0000;
        SCREEN_WIDTH = uga.x_sz / UGA_FONT_W;
        SCREEN_HEIGHT = uga.y_sz / UGA_FONT_H;
        uga.enable = 1;
    } else if (tag == 1 && uga.fonts != NULL) {
        uga.enable = *(unsigned char*)param;
    }
}
#endif /* KERN_VBE_MODE */

static iostatus_t console_enter(driver_object_t *driver)
{
    iostatus_t status;

    device_object_t *devobj;
    device_extension_t *devext;

    int id;
    char devname[DEVICE_NAME_LEN] = {0};

    for (id = 0; id < MAX_CONSOLE_NR; id++) {
        sprintf(devname, "%s%d", DEV_NAME, id);
        // 初始化一些其它内容
        status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_SCREEN, &devobj);

        if (status != IO_SUCCESS) {
            keprint(PRINT_ERR "console_enter: create device failed!\n");
            return status;
        }

        // IO缓存模式
        devobj->flags = DO_BUFFERED_IO;

        devext = (device_extension_t *)devobj->device_extension;
        string_new(&devext->device_name, devname, DEVICE_NAME_LEN);
        devext->device_object = devobj;

#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "console_enter: device extension: device name=%s object=%x\n",
            devext->device_name.text, devext->device_object);
#endif /* DEBUG_DRV */

        // 设置屏幕大小
        devext->screen_size = SCREEN_SIZE;

        // 控制台起始地址
        devext->original_addr = id * SCREEN_SIZE;
        
        /* 设置默认颜色 */
        devext->color = COLOR_DEFAULT;

#ifdef KERN_VBE_MODE
        uga.addr = NULL;
        uga.enable = 0;
#endif /* KERN_VBE_MODE */

        // 默认在左上角
        if (id == 0) {
            // 继承现有位置
            unsigned short cursor = get_cursor();
            devext->x = cursor % SCREEN_WIDTH;
            devext->y = cursor / SCREEN_WIDTH;
        } else {
            // 默认左上角位置
            devext->x = 0;
            devext->y = 0;
        }
    }

    return IO_SUCCESS;
}

static iostatus_t console_exit(driver_object_t *driver)
{
    device_object_t *devobj, *next;

    // 遍历所有对象，由于涉及到要释放devobj，所以需要使用safe版本
    list_for_each_owner_safe (devobj, next, &driver->device_list, list) {
        // 删除每一个设备
        io_delete_device(devobj);
    }

    // 删除驱动名
    string_del(&driver->name);

    return IO_SUCCESS;
}

iostatus_t console_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;

#ifdef KERN_VBE_MODE
    driver->device_be_notify_callback = &device_be_notify;
#else
    driver->device_be_notify_callback = NULL;
#endif  /* KERN_VBE_MODE */

    // 绑定驱动信息
    driver->driver_enter = console_enter;
    driver->driver_exit = console_exit;

    driver->dispatch_function[IOREQ_READ] = console_read;
    driver->dispatch_function[IOREQ_WRITE] = console_write;
    driver->dispatch_function[IOREQ_DEVCTL] = console_devctl;

    // 初始化驱动名字
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);

#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "console_driver_func: driver name=%s\n", driver->name.text);
#endif /* DEBUG_DRV */

    return status;
}

static __init void console_driver_entry(void)
{
    if (driver_object_create(console_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(console_driver_entry);
