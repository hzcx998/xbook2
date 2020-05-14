
#ifndef _XBOOK_ELF32_H
#define _XBOOK_ELF32_H

/* 自定义的elf类型 */
typedef unsigned int Elf32_Word, Elf32_Addr, Elf32_Off;
typedef unsigned short Elf32_Half;

/* 32位elf头 */
struct Elf32_Ehdr {
   unsigned char e_ident[16];
   Elf32_Half    e_type;
   Elf32_Half    e_machine;
   Elf32_Word    e_version;
   Elf32_Addr    e_entry;
   Elf32_Off     e_phoff;
   Elf32_Off     e_shoff;
   Elf32_Word    e_flags;
   Elf32_Half    e_ehsize;
   Elf32_Half    e_phentsize;
   Elf32_Half    e_phnum;
   Elf32_Half    e_shentsize;
   Elf32_Half    e_shnum;
   Elf32_Half    e_shstrndx;
};

/* 程序头表Program header.就是段描述头 */
struct Elf32_Phdr {
   Elf32_Word p_type;		 // 见下面的enum segment_type
   Elf32_Off  p_offset;
   Elf32_Addr p_vaddr;
   Elf32_Addr p_paddr;
   Elf32_Word p_filesz;
   Elf32_Word p_memsz;
   Elf32_Word p_flags;
   Elf32_Word p_align;
};

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

#define ELF32_PF_X        (1 << 0) /* Segment is executable */
#define ELF32_PF_W        (1 << 1) /* Segment is writable */
#define ELF32_PF_R        (1 << 2) /* Segment is readable */
#define ELF32_PF_MASKOS   0x0ff00000 /* OS-specific */
#define ELF32_PF_MASKPROC 0xf0000000 /* Processor-specific */

#define ELF32_PHDR_CODE (ELF32_PF_X | ELF32_PF_R) /* 代码段 */

#define ELF32_PHDR_DATA (ELF32_PF_W | ELF32_PF_R) /* 数据段 */

#endif   /* _XBOOK_ELF32_H */
