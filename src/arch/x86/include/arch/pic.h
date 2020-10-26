#ifndef _X86_PIC_H
#define _X86_PIC_H

#define PIC_MASTER_CTL		0x20	// I/O port for interrupt controller         <Master>
#define PIC_MASTER_CTLMASK  0x21	// setting bits in this port disables ints   <Master>
#define PIC_SLAVE_CTL	    0xa0	// I/O port for second interrupt controller  <Slave>
#define PIC_SLAVE_CTLMASK	0xa1	// setting bits in this port disables ints   <Slave>

#define PIC_EIO             0x20    /* end of IO port */

void pic_init();

#endif	/* _X86_PIC_H */