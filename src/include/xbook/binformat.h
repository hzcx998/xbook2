/**
 * 这个文件来自startOS，感谢作者开源！
 */
#ifndef _XBOOK_BINFORMAT_H
#define _XBOOK_BINFORMAT_H

#include <stdint.h>
#include <stddef.h>

/* Symbolic values for the entries in the auxiliary table
   put on the initial stack */
#define AT_NULL 0      /* end of vector */
#define AT_IGNORE 1    /* entry should be ignored */
#define AT_EXECFD 2    /* file descriptor of program */
#define AT_PHDR 3      /* program headers for program */
#define AT_PHENT 4     /* size of program header entry */
#define AT_PHNUM 5     /* number of program headers */
#define AT_PAGESZ 6    /* system page size */
#define AT_BASE 7      /* base address of interpreter */
#define AT_FLAGS 8     /* flags */
#define AT_ENTRY 9     /* entry point of program */
#define AT_NOTELF 10   /* program is not ELF */
#define AT_UID 11      /* real uid */
#define AT_EUID 12     /* effective uid */
#define AT_GID 13      /* real gid */
#define AT_EGID 14     /* effective gid */
#define AT_PLATFORM 15 /* string identifying CPU for optimizations */
#define AT_HWCAP 16    /* arch dependent hints at CPU capabilities */
#define AT_CLKTCK 17   /* frequency at which times() increments */
/* AT_* values 18 through 22 are reserved */
#define AT_SECURE 23 /* secure mode boolean */
#define AT_BASE_PLATFORM 24 /* string identifying real platform, may differ from AT_PLATFORM. */
#define AT_RANDOM 25 /* address of 16 random bytes */
#define AT_HWCAP2 26	/* extension of AT_HWCAP */

#define AT_EXECFN 31 /* filename of program */

#define AT_VECTOR_SIZE_BASE 20 /* NEW_AUX_ENT entries in auxiliary table */
  /* number of "#define AT_.*" above, minus {AT_NULL, AT_IGNORE, AT_NOTELF} */

// const int kElfInfoNum = 30;

// 该结构体用来维护程序的参数
typedef struct bin_program {
    uint64_t    sp;
    uint64_t    stackbase;
    void* pagetable;
    char *      filename;
    int         argc, envc;
    uint64_t *  ustack;  // 用户栈(低->高): [argc|argv|env|elf_info]
    int         stack_top;
} bin_program_t;

void bin_program_init(bin_program_t *bin);
int bin_program_copy_string2stack(bin_program_t *bin, char *strs[]);
uint64_t bin_program_copy_string(bin_program_t *bin, const char *s);
uint64_t bin_program_copy_nbytes(bin_program_t *bin, char *buf, size_t len);

#endif   /* _XBOOK_BINFORMAT_H */