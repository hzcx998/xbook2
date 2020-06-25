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



/**
 * dev_scan - 扫描设备
 * @de: 输入设备项，为NULL时表示第一次扫描开始
 * @type: 设备的类型
 * @out: 输出的设备项
 * 
 * 成功返回0，失败返回-1
 */
int dev_scan(devent_t *de, device_type_t type, devent_t *out)
{
    return syscall3(int, SYS_DEVSCAN, de, type, out);
}

/**
 * sys_mmap - 映射资源
 * @res: 资源
 * @length: 长度
 * @flags: 映射标志
 * 
 * @return: 成功返回映射后的地址，失败返回NULL
 */
void *res_mmap(int res, size_t length, int flags)
{
    return syscall3(void *, SYS_MMAP, res, length, flags);
}

/**
 * unid - 生成一个唯一的id值
 * @id: 参考值
 */
unid_t res_unid(int id)
{
    return syscall1(unid_t , SYS_UNID, id);
}


/**
 * res_redirect() - res redirect
 * 
 * @old_res: old_res index in table
 * @new_res: new_res index in table
 * 
 * redirect old res to new res, if new res exist, close new res first,
 * then redirect old res to new res, after that, remove old res index
 * int table.
 * 
 * @return: new res >= 0 is sucess, -1 is failed 
 */
int res_redirect(int old_res, int new_res)
{
    return res_ioctl(old_res, RES_REDIR, new_res);
}

