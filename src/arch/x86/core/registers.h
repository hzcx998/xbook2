#ifndef _X86_REGISTERS_H
#define _X86_REGISTERS_H

void load_tr(unsigned int tr);
void load_gdtr(unsigned int limit, unsigned int addr);
void load_idtr(unsigned int limit, unsigned int addr);
unsigned int load_eflags(void);

void store_gdtr(unsigned int gdtr);
void store_idtr(unsigned int idtr);
void store_eflags(unsigned int eflags);

unsigned int read_cr0(void );
unsigned int read_cr2(void );
unsigned int read_cr3(void );

void write_cr0(unsigned int address);
void write_cr3(unsigned int address);

#endif  /* _X86_REGISTERS_H */
