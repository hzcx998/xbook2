#include "lib.h"
#include "page.h"
#include "elf32.h"

#define DEBUG_SETUP

/* 内核文件被加载到物理内存的地址 */
#define KERNEL_FILE_ADDR    0x10000

typedef void (*entry_point_t)();
void put_char2(char ch);

static int read_file(unsigned char *base, void *buffer, unsigned int offset, unsigned int size)
{
    unsigned char *ptr = base + offset;
    memcpy(buffer, ptr, size);
    return size;
}

void setup_kernel()
{
    print_str("setup kernel.\n"); 

    /* 读取文件头 */
    struct Elf32_Ehdr elf_header;
    memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));
    unsigned char *file = (unsigned char *) KERNEL_FILE_ADDR;

    read_file(file, &elf_header, 0, sizeof(struct Elf32_Ehdr));
    
    /* 检验elf头，看它是否为一个elf格式的程序 */
    if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) || \
        elf_header.e_type != 2 || \
        elf_header.e_machine != 3 || \
        elf_header.e_version != 1 || \
        elf_header.e_phnum > 1024 || \
        elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) {
        
        /* 头文件检测出错 */
        print_str("setup_kernel: kernel file format not elf!\n");
        char *v = (char *)0xb8002;
        *v = 'q';
        *(v+1) = 0x05;
        while (1);
    }

    struct Elf32_Phdr prog_header;
    /* 获取程序头起始偏移 */
    Elf32_Off prog_header_off = elf_header.e_phoff;
    Elf32_Half prog_header_size = elf_header.e_phentsize;
    #ifdef DEBUG_SETUP
    print_str("program header: offset:");
    print_int(prog_header_off);
    print_str(" size:");
    print_int(prog_header_size);
    print_str("\n");
    #endif
    /* 遍历所有程序头 */
    unsigned long grog_idx = 0;
    while (grog_idx < elf_header.e_phnum) {
        memset(&prog_header, 0, prog_header_size);
        
        /* 读取程序头 */
        read_file(file, (void *)&prog_header, prog_header_off, prog_header_size);
#ifdef DEBUG_SETUP      
        print_str("segment : vma:");
        print_hex(prog_header.p_vaddr);
        print_str(" file offset:");
        print_hex(prog_header.p_offset);
        
        print_str(" size:");
        print_int(prog_header.p_filesz);
        print_str(" type:");
        print_int(prog_header.p_type);
        
        print_str("\n");
#endif
        /* 如果是可加载的段就加载到内存中 */
        if (prog_header.p_type == PT_LOAD) {
            /* 由于bss段不占用文件大小，但是要占用内存，
            所以这个地方我们加载的时候就加载成memsz，
            运行的时候访问未初始化的全局变量时才可以正确
            filesz用于读取磁盘上的数据，而memsz用于内存中的扩展，
            因此filesz<=memsz
             */
            read_file(file, (void *)prog_header.p_vaddr, prog_header.p_offset, prog_header.p_filesz);
            /* 如果内存占用大于文件占用，就要把内存中多余的部分置0 */
            if (prog_header.p_memsz > prog_header.p_filesz) {
                memset((void *)(prog_header.p_vaddr + prog_header.p_filesz), 0,
                    prog_header.p_memsz - prog_header.p_filesz);
            }
        }
        /* 指向下一个程序头 */
        prog_header_off += prog_header_size;
        grog_idx++;
    }

    entry_point_t entry_point = (entry_point_t)elf_header.e_entry;
    entry_point();    /* 跳转到入口执行 */
}

/**
 * setup_entry - 进入内核前的准备
 */
void setup_entry()
{   
    /* 设置并启动分页机制 */
    setup_paging();

    /* 分析内核文件，并加载到内存 */
    setup_kernel();

    
    print_str("error: kernel shold never be here!\n");
}

int cx = 0, cy = 0;

/**
 * put_char - 控制台上输出一个字符
 * @console: 控制台
 * @ch: 字符
 */
void put_char2(char ch)
{

	char *vram = (char *)(0xb8000 + 
        (cy * 80 + cx) *2) ;

    *vram++ = ch;
    *vram++ = 0x07;

    cx = cx + 1;
    
    if (cx == 0) {
        *vram++ = 'E';
        *vram = 0x05;
        while (1)
        {
            /* code */
        }
        
    }
    if (cx > 80 - 1) {
        cx = 0;
        cy++;
    }
}