#include <xcore/syscall.h>
#include <xcore/xcore.h>

/**
 * getres() - get device resource
 * 
 * @name: res name
 * @flags: res flags 
 * @arg: res arg 
 * 
 * getres a device, if didn't getres, can't operate the device. 
 * 
 * @return: res idx in table, -1 is failed. 
 */
int getres(char *name, unsigned long flags, unsigned long arg)
{
    return syscall3(int, SYS_GETRES, name, flags, arg);
}

/**
 * putres() - release device resource
 * 
 * @res: res index in table
 * 
 * remember putres at the end of program.
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int putres(int res)
{
    return syscall1(int, SYS_PUTRES, res);
}

/**
 * writeres() - write data to device resource
 * 
 * @res: res index in table
 * @buffer: data buffer
 * @size: buffer length (bytes)
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int writeres(int res, off_t off, void *buffer, size_t size)
{
    return syscall4(int, SYS_WRITERES, res, off, buffer, size);
}

/**
 * readres() - read data from device resource
 * 
 * @res: res index in table
 * @buffer: data buffer
 * @size: buffer length (bytes)
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int readres(int res, off_t off, void *buffer, size_t size)
{
    return syscall4(int, SYS_READRES, res, off, buffer, size);
}

/**
 * ctlres() - control resource
 * 
 * @res: res index in table
 * @cmd: command for res 
 * @arg: command arg
 * 
 * control device by cmd, cmd always support by driver.
 * 
 * @return: 0 is sucess, -1 is failed 
 */
int ctlres(int res, unsigned int cmd, unsigned long arg)
{
    return syscall3(int, SYS_CTLRES, res, cmd, arg);
}
