#ifndef _X86_REGISTERS_H
#define _X86_REGISTERS_H

#define task_register_set(tr) __asm__ __volatile__ ("ldr %%eax"::"a"(tr)::"memory")
void gdt_register_set(unsigned int limit, unsigned int addr);
void idt_register_set(unsigned int limit, unsigned int addr);
unsigned int eflags_save_to(void);

void gdt_register_get(unsigned int gdtr);
void idt_register_get(unsigned int idtr);
void eflags_restore_from(unsigned int eflags);
/* copyright (c) 2021 AlanCui*/
__attribute__((always_inline)) inline static unsigned long cpu_cr0_read()
{
    unsigned long tmp = 0;
    __asm__ __volatile__("movl %%cr0,%%eax"
                         : "=a"(tmp)
                         :
                         :);
    return tmp;
}
__attribute__((always_inline)) inline static unsigned long cpu_cr2_read()
{
    unsigned long tmp = 0;
    __asm__ __volatile__("movl %%cr2,%%eax"
                         : "=a"(tmp)
                         :
                         :);
    return tmp;
}
__attribute__((always_inline)) inline static unsigned long cpu_cr3_read()
{
    unsigned long tmp = 0;
    __asm__ __volatile__("movl %%cr3,%%eax"
                         : "=a"(tmp)
                         :
                         :);
    return tmp;
}
__attribute__((always_inline)) inline static void cpu_cr0_write(unsigned long data)
{
    __asm__ __volatile__("movl %%eax,%%cr0" ::"a"(data)
                         :"memory");
}
__attribute__((always_inline)) inline static void cpu_cr3_write(unsigned long data)
{
    __asm__ __volatile__("movl %%eax,%%cr3" ::"a"(data)
                         :"memory");
}
/* copyright (c) 2021 AlanCui END*/
#endif  /* _X86_REGISTERS_H */
