#include <sys/syscall.h>
#include <sys/res.h>

/**
 * res_open() - get device resource
 * 
 * @name: res name
 * @flags: res flags 
 * @arg: res arg 
 * 
 * res_open a device, if didn't res_open, can't operate the device. 
 * 
 * @return: res idx in table >= 0, -1 is failed. 
 */
int res_open(char *name, unsigned long flags, unsigned long arg)
{
    return syscall3(int, SYS_GETRES, name, flags, arg);
}

/**
 * res_close() - release device resource
 * 
 * @res: res index in table
 * 
 * remember res_close at the end of program.
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int res_close(int res)
{
    return syscall1(int, SYS_PUTRES, res);
}

/**
 * res_write() - write data to device resource
 * 
 * @res: res index in table
 * @buffer: data buffer
 * @size: buffer length (bytes)
 * 
 * @return: (write bytes) >= 0 is sucess, -1 is failed 
 */
int res_write(int res, off_t off, void *buffer, size_t size)
{
    return syscall4(int, SYS_WRITERES, res, off, buffer, size);
}

/**
 * res_read() - read data from device resource
 * 
 * @res: res index in table
 * @buffer: data buffer
 * @size: buffer length (bytes)
 * 
 * @return: (read bytes) >= 0 is sucess, -1 is failed 
 */
int res_read(int res, off_t off, void *buffer, size_t size)
{
    return syscall4(int, SYS_READRES, res, off, buffer, size);
}

/**
 * res_ioctl() - control resource
 * 
 * @res: res index in table
 * @cmd: command for res 
 * @arg: command arg
 * 
 * control device by cmd, cmd always support by driver.
 * 
 * @return: (info) >= 0 is sucess, -1 is failed 
 */
int res_ioctl(int res, unsigned int cmd, unsigned long arg)
{
    return syscall3(int, SYS_CTLRES, res, cmd, arg);
}
