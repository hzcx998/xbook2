/* multiboots.S - bootstrap the setup */
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

#define ASM_FILE 1
#include "multiboot2.h"

# the size of stack is 16KB
#define STACK_SIZE 0x4000

#ifdef KERN_VBE_MODE
#define FRAMEBUFFER_WIDTH 1024
#define FRAMEBUFFER_HEIGHT 768
#define FRAMEBUFFER_BPP 24
#endif /* KERN_VBE_MODE */

.code32

.extern setup_entry, kernel_start

.text
.globl start, _start

start:
_start:
    jmp multiboot_entry

.align 8
multiboot_header:
    .long MULTIBOOT2_HEADER_MAGIC                  # magic number (multiboot 2)
    .long MULTIBOOT_ARCHITECTURE_I386              # architecture 0 (protected mode i386)
    .long multiboot_header_end - multiboot_header  # header length
    # checksum
    .long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header))
    # insert optional multiboot tags here

#ifdef KERN_VBE_MODE
    .align 8
    tag_framebuffer:
        .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
        .short MULTIBOOT_HEADER_TAG_OPTIONAL
        .long  tag_framebuffer_end - tag_framebuffer
        .long  FRAMEBUFFER_WIDTH    # screen width
        .long  FRAMEBUFFER_HEIGHT   # screen height
        .long  FRAMEBUFFER_BPP      # bit per pixel
    tag_framebuffer_end:
#endif /* KERN_VBE_MODE */

    # required end tag
    .align 8
    .short MULTIBOOT_HEADER_TAG_END # type
    .short 0                        # flags
    .long  8                        # size
multiboot_header_end:
multiboot_entry:
    # initialize the stack pointer
    movl $(stack + STACK_SIZE), %esp

    # reset EFLAGS
    pushl $0
    popf

    # push the pointer to the Multiboot information structure
    pushl %ebx
    # push the magic value
    pushl %eax

    # jump to setup_entry
    call setup_entry

    # jump to setup_fail if setup_entry return -1
    popl %eax
    cmpl $-1, %eax
    je setup_fail
    
    # print "Multiboot End!" in protected mode
    movl $0x2f752f4d, 0xb8000
    movl $0x2f742f6c, 0xb8004
    movl $0x2f622f69, 0xb8008
    movl $0x2f6f2f6f, 0xb800c
    movl $0x2f202f74, 0xb8010
    movl $0x2f6e2f45, 0xb8014
    movl $0x2f212f64, 0xb8018

    # jump to kernel_start
    movl $kernel_start, %eax
    jmp *%eax

setup_fail:
    # print "Error!" in protected mode
    movl $0xcf72cf45, 0xb8000
    movl $0xcf6fcf72, 0xb8004
    movl $0xcf21cf72, 0xb8008

multiboot_hlt:
    hlt
    jmp multiboot_hlt

    .comm stack, STACK_SIZE
