
#ifndef _XBOOK_ELF_H
#define _XBOOK_ELF_H

#include <stdint.h>

/* 32-bit ELF base types. */
typedef uint32_t	Elf32_Addr;
typedef uint16_t	Elf32_Half;
typedef uint32_t	Elf32_Off;
typedef int32_t	    Elf32_Sword;
typedef uint32_t	Elf32_Word;

/* 64-bit ELF base types. */
typedef uint64_t	Elf64_Addr;
typedef uint16_t	Elf64_Half;
typedef int16_t	    Elf64_SHalf;
typedef uint64_t	Elf64_Off;
typedef int32_t	    Elf64_Sword;
typedef uint32_t	Elf64_Word;
typedef uint64_t	Elf64_Xword;
typedef int64_t	    Elf64_Sxword;

#define EI_NIDENT	16

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

typedef struct elf32_hdr {
    unsigned char	e_ident[EI_NIDENT];
    Elf32_Half	e_type;
    Elf32_Half	e_machine;
    Elf32_Word	e_version;
    Elf32_Addr	e_entry;  /* Entry point */
    Elf32_Off	    e_phoff;
    Elf32_Off	    e_shoff;
    Elf32_Word	e_flags;
    Elf32_Half	e_ehsize;
    Elf32_Half	e_phentsize;
    Elf32_Half	e_phnum;
    Elf32_Half	e_shentsize;
    Elf32_Half	e_shnum;
    Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

typedef struct elf64_hdr {
    uint32_t magic;  // must equal ELF_MAGIC
    unsigned char e_ident[EI_NIDENT - 4];	/* ELF "magic number" */
    Elf64_Half    e_type;
    Elf64_Half    e_machine;
    Elf64_Word    e_version;
    Elf64_Addr    e_entry;		/* Entry point virtual address */
    Elf64_Off     e_phoff;		/* Program header table file offset */
    Elf64_Off     e_shoff;		/* Section header table file offset */
    Elf64_Word    e_flags;
    Elf64_Half    e_ehsize;
    Elf64_Half    e_phentsize;
    Elf64_Half    e_phnum;
    Elf64_Half    e_shentsize;
    Elf64_Half    e_shnum;
    Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

/* 程序头表Program header.就是段描述头 */
typedef struct elf32_phdr {
    Elf32_Word	p_type;
    Elf32_Off	p_offset;
    Elf32_Addr	p_vaddr;
    Elf32_Addr	p_paddr;
    Elf32_Word	p_filesz;
    Elf32_Word	p_memsz;
    Elf32_Word	p_flags;
    Elf32_Word	p_align;
} Elf32_Phdr;

typedef struct elf64_phdr {
    Elf64_Word    p_type;
    Elf64_Word    p_flags;
    Elf64_Off     p_offset;		/* Segment file offset */
    Elf64_Addr    p_vaddr;		/* Segment virtual address */
    Elf64_Addr    p_paddr;		/* Segment physical address */
    Elf64_Xword   p_filesz;		/* Segment size in file */
    Elf64_Xword   p_memsz;		/* Segment size in memory */
    Elf64_Xword   p_align;		/* Segment alignment, file & memory */
} Elf64_Phdr;

/* 段类型 */
enum segment_type {
    PT_NULL,            // 忽略
    PT_LOAD,            // 可加载程序段
    PT_DYNAMIC,         // 动态加载信息 
    PT_INTERP,          // 动态加载器名称
    PT_NOTE,            // 一些辅助信息
    PT_SHLIB,           // 保留
    PT_PHDR             // 程序头表
};

#define PF_X        (1 << 0) /* Segment is executable */
#define PF_W        (1 << 1) /* Segment is writable */
#define PF_R        (1 << 2) /* Segment is readable */
#define PF_MASKOS   0x0ff00000 /* OS-specific */
#define PF_MASKPROC 0xf0000000 /* Processor-specific */

#define PHDR_CODE (PF_X | PF_R) /* 代码段 */

#define PHDR_DATA (PF_W | PF_R) /* 数据段 */

#define PHDR_CODE_DATA (PHDR_CODE | PHDR_DATA) /* 数据段 */

#endif   /* _XBOOK_ELF_H */
