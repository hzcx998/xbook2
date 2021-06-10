/*  setup.c - setup environment. */
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

#include <drivers/vbe.h>
#include <string.h>

#include "multiboot2.h"

static void init_memory(struct multiboot_tag *tag);
static void init_vbe(struct multiboot_tag *tag);
static void init_framebuffer(struct multiboot_tag *tag);

// global data
unsigned long grub2_read_memory_bytes = 0;

int setup_entry(unsigned long magic, unsigned long addr)
{
    // whether a multiboot
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC || addr & 7) return -1;
    struct multiboot_tag *tag;

    for (tag = (struct multiboot_tag*)(addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag*)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
    {
        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            init_memory(tag);
        break;
        case MULTIBOOT_TAG_TYPE_VBE:
            init_vbe(tag);
        break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
            init_framebuffer(tag);
        break;
        }
    }

    return 0;
}

static void init_memory(struct multiboot_tag *tag) {
    unsigned long mem_upper = ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper;
    unsigned long mem_lower = ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower;
    // 0x100000 = 1MB
    grub2_read_memory_bytes = ((mem_upper - mem_lower) << 10) + 0x100000;
}

static void init_vbe(struct multiboot_tag *tag)
{
    struct multiboot_tag_vbe *vbe_tag = (struct multiboot_tag_vbe *)tag;

    struct vbe_info_block *vbe_info = (struct vbe_info_block *)VBE_BASE_INFO_ADDR;
    struct vbe_mode_info_block *mode_info = (struct vbe_mode_info_block *)VBE_BASE_MODE_ADDR;

    memcpy(vbe_info, &(vbe_tag->vbe_control_info), sizeof(struct vbe_info_block));
    memcpy(mode_info, &(vbe_tag->vbe_mode_info), sizeof(struct vbe_mode_info_block));
}

static void init_framebuffer(struct multiboot_tag *tag)
{
    struct multiboot_tag_framebuffer *framebuffer_tag = (struct multiboot_tag_framebuffer *)tag;

    struct vbe_mode_info_block *mode_info = (struct vbe_mode_info_block *)VBE_BASE_MODE_ADDR;

    mode_info->xResolution = framebuffer_tag->common.framebuffer_width;
    mode_info->yResolution = framebuffer_tag->common.framebuffer_height;
    mode_info->bitsPerPixel = framebuffer_tag->common.framebuffer_bpp;
    mode_info->phyBasePtr = framebuffer_tag->common.framebuffer_addr;

    // TODO: get mode attributes value
    // mode_info->modeAttributes = ;
    mode_info->bytesPerScanLine = framebuffer_tag->common.framebuffer_pitch;
}
