#include <sys/usrmsg.h>
#include <xbook/kernel.h>
#include <xbook/debug.h>
#include <xbook/device.h>
#include <xbook/process.h>
#include <xbook/clock.h>

void usrmsg_dump(umsg_t *msg)
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
    dev_t devno;
    //usrmsg_dump(msg);
    switch (msg->type)
    {
    case UMSG_OPEN:
        devno = get_devno_by_name((char *)msg->arg0);
        if (!devno) {   /* not found a devno */
            msg->retval = -2;
            return -1;
        }
        //printk(KERN_DEBUG "usrmsg open\n");
        msg->retval = dev_open(devno, (flags_t) msg->arg1); 
        break;
    case UMSG_CLOSE:
        devno = get_devno_by_name((char *)msg->arg0);
        if (!devno) {   /* not found a devno */
            msg->retval = -2;
            return -1;
        }
        msg->retval = dev_close(devno); 
        break;
    case UMSG_READ:
        devno = get_devno_by_name((char *)msg->arg0);
        if (!devno) {   /* not found a devno */
            msg->retval = -2;
            return -1;
        }
        
        msg->retval = dev_read(devno, (off_t) msg->arg1, (void *) msg->arg2, (size_t) msg->arg3);
        break;
    case UMSG_WRITE:
        devno = get_devno_by_name((char *)msg->arg0);
        if (!devno) {   /* not found a devno */
            msg->retval = -2;
            return -1;
        }
        msg->retval = dev_write(devno, (off_t) msg->arg1, (void *) msg->arg2, (size_t) msg->arg3);
        break;
    case UMSG_IOCTL:
        devno = get_devno_by_name((char *)msg->arg0);
        if (!devno) {   /* not found a devno */
            msg->retval = -2;
            return -1;
        }
        msg->retval = dev_ioctl(devno, (unsigned int) msg->arg1, (unsigned long) msg->arg2);
        break;
    case UMSG_PUTC:
        devno = get_devno_by_name((char *)msg->arg0);
        if (!devno) {   /* not found a devno */
            msg->retval = -2;
            return -1;
        }
        msg->retval = dev_putc(devno, (unsigned long) msg->arg1);
        break;
    case UMSG_GETC:
        devno = get_devno_by_name((char *)msg->arg0);
        if (!devno) {   /* not found a devno */
            msg->retval = -2;
            return -1;
        }
        msg->retval = dev_getc(devno, (unsigned long *) msg->arg1);
        break;
    case UMSG_FORK:
        printk("in UMSG_FORK");
        proc_fork((long *)&msg->retval);
        printk("task %s-%d will return!\n", current_task->name, current_task->pid);
        break;
    case UMSG_EXIT:
        printk("in UMSG_EXIT");
        proc_exit((int )msg->arg0);
        break;
    case UMSG_WAIT:
        printk("in UMSG_WAIT");
        msg->retval = proc_wait((int *)&msg->arg0);
        break;
    case UMSG_MSLEEP:
        clock_msleep((unsigned long )msg->arg0);
        break;
    default:
        break;
    }
    // printk(">return\n");
    return 0;
}
