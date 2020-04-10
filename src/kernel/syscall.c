#include <xbook/syscall.h>
#include <xbook/process.h>
#include <xbook/vmspace.h>
#include <xbook/resource.h>


/* 系统调用表 */ 
syscall_t syscall_table[SYSCALL_NR];

void init_syscall()
{
    /* 进程管理 */
    syscall_table[SYS_EXIT] = proc_exit;
    syscall_table[SYS_FORK] = proc_fork;
    syscall_table[SYS_EXECR] = proc_exec_raw;
    syscall_table[SYS_EXECF] = proc_exec_file;
    syscall_table[SYS_WAIT] = proc_wait;
    syscall_table[SYS_GETPID] = task_get_pid;
    syscall_table[SYS_GETPPID] = task_get_ppid;
    
    /* 内存管理 */
    syscall_table[SYS_HEAP] = vmspace_heap;
    
    /* 设备资源管理 */
    syscall_table[SYS_GETRES] = sys_getres;
    syscall_table[SYS_PUTRES] = sys_putres;
    syscall_table[SYS_READRES] = sys_readres;
    syscall_table[SYS_WRITERES] = sys_writeres;
    syscall_table[SYS_CTLRES] = sys_ctlres;
}