#ifndef _X86_REGISTERS_H
#define _X86_REGISTERS_H

void task_register_set(unsigned int tr);
void gdt_register_set(unsigned int limit, unsigned int addr);
void idt_register_set(unsigned int limit, unsigned int addr);
unsigned int eflags_save_to(void);

void gdt_register_get(unsigned int gdtr);
void idt_register_get(unsigned int idtr);
void eflags_restore_from(unsigned int eflags);

unsigned int cpu_cr0_read(void );
unsigned int cpu_cr2_read(void );
unsigned int cpu_cr3_read(void );

void cpu_cr0_write(unsigned int address);
void cpu_cr3_write(unsigned int address);

#endif  /* _X86_REGISTERS_H */
