#include <sys/xbook.h>

/**
 * x_exit() - exit process
 * 
 * @status: exit status, give to parent.
 * 
 * exit process, after that, the process end of it's life.
 */
void x_exit(int status)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_EXIT);
    umsg_set_arg0(msg, status);
    umsg(msg);
}

/**
 * x_fork() - fork process
 * 
 * after fork, we have 2 process samed, but different pid.
 * p A is parent, p B is child, so B's parent pid os A's pid.
 * 
 * @return: process pid, there are 3 case:
 *          1. pid > 0: current process is A, pid is B's pid
 *          2. pid = 0: current process is B
 *          3. pid < 0: fork failed
 */
int x_fork()
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_FORK);
    umsg(msg);
    return umsg_get_retval(msg, int); /* return a pid (int type) */
}

/**
 * x_wait() - exit process
 * 
 * @status: [out] child process's exit status.
 *          notice that this is a ptr!
 * 
 * wait one child process to exit and fetch it's exit status.
 * 
 * @return: child pid, there are 2 case:
 *          if pid = -1: no child
 *          if pid > -1: exited child's pid 
 */
int x_wait(int *status)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_WAIT);
    umsg_set_arg0(msg, 0);
    umsg(msg);
    if (status) /* not null */
        *status = umsg_get_arg0(msg, int);
    return umsg_get_retval(msg, int);
}

/**
 * x_execraw() - execute raw block
 * 
 * @name: raw block name
 * @argv: arguments array
 * 
 * execute a raw block process, replaces the current process with the
 * raw block and runs the process corresponding to the raw block.
 * 
 * @return: -1 is failed, no success return, if success, run the new process. 
 */
int x_execraw(char *name, char *argv[])
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_EXECRAW);
    umsg_set_arg0(msg, name);
    umsg_set_arg1(msg, argv);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_open() - open a device
 * 
 * @name: device name
 * @flags: open flags 
 * 
 * open a device, if didn't open, can't operate the device. 
 * 
 * @return: device id, 0 means open failed, not 0 means the device id. 
 */
x_dev_t x_open(char *name, unsigned long flags)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_OPEN);
    umsg_set_arg0(msg, name);
	umsg_set_arg1(msg, flags);
    umsg(msg);
    return umsg_get_retval(msg, x_dev_t);
}

/**
 * x_close() - close a device
 * 
 * @devno: device id
 * 
 * close a device, remember close the device at the end of program.
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int x_close(x_dev_t devno)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_CLOSE);
    umsg_set_arg0(msg, devno);    
    return umsg(msg);
}

/**
 * x_write() - write data to device
 * 
 * @devno: device id
 * @off: seek offset(only for disk device)
 * @buf: data buffer
 * @size: sectors for disk device, bytes for char device
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int x_write(x_dev_t devno, x_off_t off, void *buf, x_size_t size)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_WRITE);
    umsg_set_arg0(msg, devno);
    umsg_set_arg1(msg, off);
    umsg_set_arg2(msg, buf);
    umsg_set_arg3(msg, size);
    return umsg(msg);
}

/**
 * x_read() - read data from device
 * 
 * @devno: device id
 * @off: seek offset(only for disk device)
 * @buf: data buffer
 * @size: sectors for disk device, bytes for char device
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int x_read(x_dev_t devno, x_off_t off, void *buf, x_size_t size)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_READ);
    umsg_set_arg0(msg, devno);
    umsg_set_arg1(msg, off);
    umsg_set_arg2(msg, buf);
    umsg_set_arg3(msg, size);
    return umsg(msg);
}

/**
 * x_ioctl() - device I/O control
 * 
 * @devno: device id
 * @cmd: command for device
 * @arg: command arg
 * 
 * control device by cmd, cmd always support by driver.
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int x_ioctl(x_dev_t devno, unsigned int cmd, unsigned long arg)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_IOCTL);
    umsg_set_arg0(msg, devno);
    umsg_set_arg1(msg, cmd);
    umsg_set_arg2(msg, arg);
    return umsg(msg);
}

/**
 * x_getc() - get one data from device.
 * 
 * @devno: device id
 * @data: [out] data storege ptr
 * 
 * get one data(byte, word, dword...) from device, the size
 * depends on device.
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int x_getc(x_dev_t devno, unsigned long *data)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_GETC);
    umsg_set_arg0(msg, devno);
    umsg_set_arg1(msg, data);
    return umsg(msg);
}

/**
 * x_putc() - put one data to device.
 * 
 * @devno: device id
 * @data: data value
 * 
 * put one data(byte, word, dword...) to device, the size
 * depends on your needs.
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int x_putc(x_dev_t devno, unsigned long data)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_PUTC);
    umsg_set_arg0(msg, devno);
    umsg_set_arg1(msg, data);
    return umsg(msg);
}
