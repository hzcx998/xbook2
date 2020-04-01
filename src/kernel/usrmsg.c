#include <sys/usrmsg.h>
#include <xbook/kernel.h>
#include <xbook/debug.h>
#include <xbook/device.h>
#include <xbook/process.h>
#include <xbook/clock.h>
#include <xbook/vmspace.h>

void dump_usrmsg(umsg_t *msg)
{
    printk(KERN_DEBUG "    > usrmsg: %x\n", msg);
    printk(KERN_DEBUG "type=%d arg0=%x arg1=%x arg2=%x arg3=%x arg4=%x\n",
        msg->type, msg->arg0, msg->arg1, msg->arg2, msg->arg3, msg->arg4);
}

/**
 * do_usrmsg - 用户消息处理
 * @msg: 消息
 * 
 * 执行过程中，默认中断是关闭的。
 * 
 * 具体错误消息放在msg->retval里面
 * return: 执行操作，成功返回0，失败返回-1
 */
int do_usrmsg(umsg_t *msg)
{
    if (!msg)
        return -1;
    
    /*   retval: do_usrmsg retval
    msg->retval: do real function retval */
    int retval = 0;
    switch (msg->type)
    {
    case UMSG_OPEN: /* return 0 is error, > 0 is sucees */
        msg->retval = get_devno_by_name((char *)msg->arg0);
        if (!msg->retval) {   /* not found a devno */
            retval = -1;
            break;
        }
        retval = dev_open(msg->retval, (flags_t) msg->arg1); 
        if (retval == -1)
            msg->retval = 0;

        break;
    case UMSG_CLOSE:
        msg->retval = dev_close((dev_t) msg->arg0);
        if (msg->retval == -1)
            retval = -1;
        break;
    case UMSG_READ:
        msg->retval = dev_read((dev_t) msg->arg0, (off_t) msg->arg1, (void *) msg->arg2, (size_t) msg->arg3);
        if (msg->retval == -1)
            retval = -1;
        break;
    case UMSG_WRITE:
        msg->retval = dev_write((dev_t) msg->arg0, (off_t) msg->arg1, (void *) msg->arg2, (size_t) msg->arg3);
        if (msg->retval == -1)
            retval = -1;
        break;
    case UMSG_IOCTL:
        msg->retval = dev_ioctl((dev_t) msg->arg0, (unsigned int) msg->arg1, (unsigned long) msg->arg2);
        if (msg->retval == -1)
            retval = -1;
        break;
    case UMSG_PUTC:
        msg->retval = dev_putc((dev_t) msg->arg0, (unsigned long) msg->arg1);
        if (msg->retval == -1)
            retval = -1;
        break;
    case UMSG_GETC:
        msg->retval = dev_getc((dev_t) msg->arg0, (unsigned long *) msg->arg1);
        if (msg->retval == -1)
            retval = -1;
        break;
    case UMSG_FORK:
        retval = proc_fork((long *)&msg->retval);
        break;
    case UMSG_EXIT:
        proc_exit((int )msg->arg0);
        break;
    case UMSG_WAIT:
        msg->retval = proc_wait((int *)&msg->arg0);
        if (msg->retval == -1)
            retval = -1;
        //printk("child %d exit status:%d\n", (int)msg->retval, (int)msg->arg0);
        break;
    case UMSG_EXECRAW:
        msg->retval = proc_exec_raw((char *)msg->arg0, (char **)msg->arg1);
        if (msg->retval == -1)
            retval = -1;
        break;
    case UMSG_EXECFILE:
        msg->retval = proc_exec_file((char *) msg->arg0, (x_file_t *) msg->arg1, (char **) msg->arg2);
        if (msg->retval == -1)
            retval = -1;
        break;
    case UMSG_HEAP:
        msg->retval = vmspace_heap((unsigned long) msg->arg0);
        break;
    default:
        break;
    }
    return retval;
}
