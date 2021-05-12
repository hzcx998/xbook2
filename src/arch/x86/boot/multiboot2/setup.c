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

#include "multiboot2.h"

// if not use bios, it will be used
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
        if (tag->type == MULTIBOOT_TAG_TYPE_BASIC_MEMINFO) {
            grub2_read_memory_bytes =
                ((((struct multiboot_tag_basic_meminfo *)tag)->mem_upper
                    -((struct multiboot_tag_basic_meminfo *)tag)->mem_lower) << 10)
                + 0x100000;
            break;
        }
    }

    return 0;
}
