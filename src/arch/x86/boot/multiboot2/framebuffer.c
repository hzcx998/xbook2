/*  framebuffer.c - framebuffer for boot screen. */
/*  The MIT License (MIT)
 *  Copyright 2018 ~ 2021 xbook2
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <arch/module.h>
#include <cpio.h>
#include <drivers/vbe.h>
#include <string.h>

#include "multiboot2.h"

#ifdef KERN_VBE_MODE

static uint8_t *fb_buffer = NULL;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;
static int (*fb_out_pixel)(int, int, uint32_t);

#define GREY16(color) ((color & 0xf8) << 8 | (color & 0xfc) << 3 | color >> 3)
static int screen_out_pixel16_gray(int x, int y, uint32_t color)
{
    color = GREY16(color);
    uint16_t *pixel = (uint16_t *)(fb_buffer + ((y * fb_width) + x) * 2);
    *pixel = color;
    return 0;
}

#define GREY24(color) ((color & 0xff) << 16 | (color & 0xff) << 8 | color)
static int screen_out_pixel24_gray(int x, int y, uint32_t color)
{
    if (x < 0 || y < 0)
        return -1;
    color = GREY24(color);
    *((fb_buffer) + 3*((fb_width) * y + x) + 0) = color & 0xFF;
    *((fb_buffer) + 3*((fb_width) * y + x) + 1) = (color & 0xFF00) >> 8;
    *((fb_buffer) + 3*((fb_width) * y + x) + 2) = (color & 0xFF0000) >> 16;
    return  0;
}

#define GREY32(color) (((color & 0xff) << 24) | ((color & 0xff) << 16) | ((color & 0xff) << 8) | color)
static int screen_out_pixel32_gray(int x, int y, uint32_t color)
{
    if (x < 0 || y < 0)
        return -1;
    color = GREY32(color);
    *((unsigned int*)((fb_buffer) + 4 * ((fb_width) * y + x))) = (unsigned int)color;
    return  0;
}

void mutboot2_init_framebuffer(struct multiboot_tag *tag)
{
    struct multiboot_tag_framebuffer *framebuffer_tag = (struct multiboot_tag_framebuffer *)tag;

    struct vbe_mode_info_block *mode_info = (struct vbe_mode_info_block *)VBE_BASE_MODE_ADDR;

    struct vbe_info_block *vbe_info = (struct vbe_info_block *)VBE_BASE_INFO_ADDR;
    if (!vbe_info->vbeVeision)
        vbe_info->vbeVeision = 0x0300;   /* modify as v 0.3.0 */

    mode_info->xResolution = framebuffer_tag->common.framebuffer_width;
    mode_info->yResolution = framebuffer_tag->common.framebuffer_height;
    mode_info->bitsPerPixel = framebuffer_tag->common.framebuffer_bpp;
    mode_info->phyBasePtr = framebuffer_tag->common.framebuffer_addr;
    
    switch (mode_info->bitsPerPixel) {
    case 16:
        fb_out_pixel = screen_out_pixel16_gray;
        break;
    case 24:
        fb_out_pixel = screen_out_pixel24_gray;
        break;
    case 32:
        fb_out_pixel = screen_out_pixel32_gray;
        break;
    default:
        fb_out_pixel = NULL;
        break;
    }
    fb_buffer = (uint8_t *) mode_info->phyBasePtr;
    fb_width = mode_info->xResolution;
    fb_height = mode_info->yResolution;

    // TODO: get mode attributes value
    // mode_info->modeAttributes = ;
    mode_info->bytesPerScanLine = framebuffer_tag->common.framebuffer_pitch;
}

void multiboot2_framebuffer_bootres_show()
{
    unsigned char* file_buf = NULL;
    unsigned long file_sz;
    int i, w, h, y, x, limit_x;
    int count, color;
    struct vbe_mode_info_block *mode_info = (struct vbe_mode_info_block *)VBE_BASE_MODE_ADDR;

    file_buf = cpio_get_file(module_info_find(0, MODULE_INITRD), "boot/bootres.img", &file_sz);
    if (file_buf == NULL) {
        return;
    }
    if (fb_out_pixel == NULL) {
        return;
    }
    i = 0;
    y = mode_info->yResolution / 6;
    w = file_buf[i++];
    x = (mode_info->xResolution - w) >> 1;
    limit_x = x + w;
    h = file_buf[i++] + y;
    for (; y < h; ++y) {
        while (x < limit_x) {
            count = file_buf[i++];
            color = file_buf[i++];
            for (; count > 0; --count, ++x) {
                fb_out_pixel(x, y, color);
            }
        }
        x -= w;
    }

    y = mode_info->yResolution - mode_info->yResolution / 6;
    w = file_buf[i++];
    x = (mode_info->xResolution - w) >> 1;
    limit_x = x + w;
    h = file_buf[i++] + y;
    for (; y < h; ++y) {
        while (x < limit_x) {
            count = file_buf[i++];
            color = file_buf[i++];
            for (; count > 0; --count, ++x) {
                fb_out_pixel(x, y, color);
            }
        }
        x -= w;
    }
}

#endif /* KERN_VBE_MODE */
