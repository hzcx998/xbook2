#ifndef _X86_INTERRUPT_H
#define _X86_INTERRUPT_H

#include "atomic.h"
#include "registers.h"


#define interrupt_disable() __asm__ __volatile__("cli")
#define interrupt_enable() __asm__ __volatile__("sti")
void interrupt_enable(void);

#define interrupt_save_and_disable(flags)                  \
    do {                                    \
        flags = (unsigned int)eflags_save_to();\
        interrupt_disable();                   \
    } while (0)

#define interrupt_restore_state(flags)               \
        eflags_restore_from((unsigned int)flags)
    
#define	IRQ0_CLOCK          0   // 时钟
#define	IRQ1_KEYBOARD       1   // 键盘
#define	IRQ2_CONNECT        2   // 连接从片
#define	IRQ3_SERIAL2        3   // 串口2
#define	IRQ4_SERIAL1        4   // 串口1
#define	IRQ5_PARALLEL2      5   // 并口2
#define	IRQ6_FLOPPY         6   // 软盘
#define	IRQ7_PARALLEL1      7   // 并口1

#define	IRQ8_RTCLOCK        8   // 实时时钟（real-time clock）
#define	IRQ9_REDIRECT       9   // 重定向的IRQ2
#define	IRQ10_RESERVE       10  // 保留
#define	IRQ11_RESERVE       11  // 保留
#define	IRQ12_MOUSE         12  // PS/2鼠标
#define	IRQ13_FPU           13  // FPU异常
#define	IRQ14_HARDDISK      14  // 硬盘
#define	IRQ15_RESERVE       15  // 保留

#define	EFLAGS_MBS (1<<1)
#define	EFLAGS_IF_1 (1<<9)
#define	EFLAGS_IF_0 0
#define	EFLAGS_IOPL_3 (3<<12)
#define	EFLAGS_IOPL_1 (1<<12)
#define	EFLAGS_IOPL_0 (0<<12)

#define EFLAGS_IF (EFLAGS_IF_1 << 9)

enum {
    EP_DIVIDE = 0,                          /* 除法错误：DIV和IDIV指令 */
    EP_DEBUG,                               /* 调试异常：任何代码和数据的访问 */
    EP_INTERRUPT,                           /* 非屏蔽中断：非屏蔽外部中断 */
    EP_BREAKPOINT,                          /* 调试断点：指令INT3 */
    EP_OVERFLOW,                            /* 溢出：指令INTO */
    EP_BOUND_RANGE,                         /* 越界：指令BOUND */
    EP_INVALID_OPCODE,                      /* 无效（未定义）的操作码：指令UD2或者无效指令 */
    EP_DEVICE_NOT_AVAILABLE,                /* 设备不可用（无数学处理器）：浮点或WAIT/FWAIT指令 */
    EP_DOUBLE_FAULT,                        /* 双重错误：所有能产生异常或NMI或INTR的指令 */
    EP_COPROCESSOR_SEGMENT_OVERRUN,         /* 协助处理器段越界：浮点指令（386之后的IA32处理器不再产生此种异常） */
    EP_INVALID_TSS,                         /* 无效TSS：任务切换或访问TSS时 */
    EP_SEGMENT_NOT_PRESENT,                 /* 段不存在：加载段寄存器或访问系统段时 */
    EP_STACK_FAULT,                         /* 堆栈段错误：堆栈操作或加载SS时 */
    EP_GENERAL_PROTECTION,                  /* 常规保护错误：内存或其它保护检验 */
    EP_PAGE_FAULT,                          /* 页故障：内存访问 */
    EP_RESERVED,                            /* INTEL保留，未使用 */
    EP_X87_FLOAT_POINT,                     /* X87FPU浮点错（数学错）：X87FPU浮点指令或WAIT/FWAIIT指令 */
    EP_ALIGNMENT_CHECK,                     /* 对齐检验：内存中的数据访问（486开始支持） */
    EP_MACHINE_CHECK,                       /* Machine Check：错误码（如果有的话）和源依赖于具体模式（奔腾CPU开始支持） */
    EP_SIMD_FLOAT_POINT,                    /* SIMD浮点异常：SSE和SSE2浮点指令（奔腾III开始支持） */
};

#define IRQ_OFF_IN_IDT	0X20
#define MAX_INTERRUPT_NR 0x81

typedef struct trap_frame {
    unsigned int vec_no;
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    // 虽然pushad把esp也压入,但esp是不断变化的,所以会被popad忽略
    unsigned int esp_dummy;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
    unsigned int gs;
    unsigned int fs;
    unsigned int es;
    unsigned int ds;
    // error_code会被压入在eip之后
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;

    /* 以下由cpu从低特权级进入高特权级时压入 */
    unsigned int esp;
    unsigned int ss;
} __attribute__((packed)) trap_frame_t;

typedef void* interrupt_handler_t;

void interrupt_register_handler(unsigned char interrupt, interrupt_handler_t function);
void unregister_interrupt_handler(unsigned char interrupt);

int irq_register_handler(unsigned char irq, interrupt_handler_t function);
int irq_unregister_handler(unsigned char irq);

void trap_frame_dump(trap_frame_t *frame);

void interrupt_expection_init(void);

void interrupt_exit();

enum {
    IRQ0 = 0,
    IRQ1,
    IRQ2,
    IRQ3,
    IRQ4,
    IRQ5,
    IRQ6,
    IRQ7,
    IRQ8,
    IRQ9,
    IRQ10,
    IRQ11,
    IRQ12,
    IRQ13,
    IRQ14,
    IRQ15,
    NR_IRQS
};

#endif  /* _X86_INTERRUPT_H */
