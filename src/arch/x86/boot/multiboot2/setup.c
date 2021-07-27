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

#include <arch/module.h>
#include <cpio.h>
#include <drivers/vbe.h>
#include <string.h>

#include "multiboot2.h"

static inline void init_module(struct multiboot_tag *tag);
static inline void init_memory(struct multiboot_tag *tag);
#ifdef KERN_VBE_MODE
static inline void init_vbe(struct multiboot_tag *tag);
extern void mutboot2_init_framebuffer(struct multiboot_tag *tag);
extern void multiboot2_framebuffer_bootres_show();

#endif /* KERN_VBE_MODE */

int setup_entry(unsigned long magic, unsigned long addr)
{
    // whether a multiboot
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC || addr & 7) return -1;
    struct multiboot_tag *tag;

    module_info_init();

    for (tag = (struct multiboot_tag*)(addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag*)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
    {
        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_MODULE:
            init_module(tag);
        break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            init_memory(tag);
        break;
#ifdef KERN_VBE_MODE
        case MULTIBOOT_TAG_TYPE_VBE:
            init_vbe(tag);
        break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
            mutboot2_init_framebuffer(tag);
        break;
#endif /* KERN_VBE_MODE */
        }
    }

#ifdef KERN_VBE_MODE
    multiboot2_framebuffer_bootres_show();
#endif /* KERN_VBE_MODE */

    return 0;
}

#define cmdline_is(cmd) (!strcmp(((struct multiboot_tag_module *)tag)->cmdline, cmd))

static void init_module(struct multiboot_tag *tag) {
    struct modules_info_block *modules_info = (struct modules_info_block *)MODULE_INFO_ADDR;
    int index = modules_info->modules_num;

    if (index >= MAX_MODULES_NUM
        || modules_info->modules_size + ((struct multiboot_tag_module *)tag)->size > MAX_MODULES_SIZE) {
        return;
    }

    modules_info->modules[index].size = ((struct multiboot_tag_module *)tag)->size;
    modules_info->modules[index].start = ((struct multiboot_tag_module *)tag)->mod_start;
    modules_info->modules[index].end = ((struct multiboot_tag_module *)tag)->mod_end;

    if (cmdline_is("initrd")) {
        modules_info->modules[index].type = MODULE_INITRD;
    } else {
        modules_info->modules[index].type = MODULE_UNKNOWN;
    }

    modules_info->modules_size += modules_info->modules[index].size;
    ++modules_info->modules_num;
}

#undef cmdline_is

static void init_memory(struct multiboot_tag *tag) {
    unsigned long mem_upper = ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper;
    unsigned long mem_lower = ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower;
    // 0x000001000 defined in src/arch/x86/mach-i386/mm/ards.c, 0x100000 = 1MB
    *((unsigned int *)0x000001000) = ((mem_upper - mem_lower) << 10) + 0x100000;
}

#ifdef KERN_VBE_MODE

static void init_vbe(struct multiboot_tag *tag)
{
    struct multiboot_tag_vbe *vbe_tag = (struct multiboot_tag_vbe *)tag;

    struct vbe_info_block *vbe_info = (struct vbe_info_block *)VBE_BASE_INFO_ADDR;
    struct vbe_mode_info_block *mode_info = (struct vbe_mode_info_block *)VBE_BASE_MODE_ADDR;

    memcpy(vbe_info, &(vbe_tag->vbe_control_info), sizeof(struct vbe_info_block));
    memcpy(mode_info, &(vbe_tag->vbe_mode_info), sizeof(struct vbe_mode_info_block));
}

#endif /* KERN_VBE_MODE */
