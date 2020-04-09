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
 * x_execfile() - execute file
 * 
 * @name: file name
 * @file: file info
 * @argv: arguments array
 * 
 * execute file in process, replaces the current process with the
 * file image and runs the process corresponding to the file.
 * 
 * @return: -1 is failed, no success return, if success, run the new process. 
 */
int x_execfile(char *name, x_file_t *file, char *argv[])
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_EXECFILE);
    umsg_set_arg0(msg, name);
    umsg_set_arg1(msg, file);
    umsg_set_arg2(msg, argv);
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
int x_write(x_dev_t devno, off_t off, void *buf, size_t size)
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
int x_read(x_dev_t devno, off_t off, void *buf, size_t size)
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

/**
 * x_heap() - memory heap operate
 * 
 * @heap: heap value
 * 
 * if heap = 0, return current heap value.
 * if heap > old heap, expand heap up.
 * if heap < old heap, shrink heap down.
 * if heap = old heap, do nothing.
 * 
 * @return: always return newest heap value
 */
unsigned long x_heap(unsigned long heap)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_HEAP);
    umsg_set_arg0(msg, heap);
    umsg(msg);
    return umsg_get_retval(msg, unsigned long);
}

/**
 * x_shmget() - get a share memory
 * 
 * @name: shm name
 * @size: shm size
 * @flags: shm flags
 *         SHM_CREAT: create a new shm or open a shm.
 *          if ok, return shmid, or not return -1
 *         SHM_EXCL: only create a new shm.
 *          this should use with SHM_CREAT, to make sure
 *          that the shm not exist. example: SHM_CREAT|SHM_EXCL
 * 
 * get a share memory area from kernel
 *  
 * @return: return shmid >= 0 is success, -1 means failed!
 */
int x_shmget(char *name, size_t size, unsigned long flags)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_SHMGET);
    umsg_set_arg0(msg, name);
    umsg_set_arg1(msg, size);
    umsg_set_arg2(msg, flags);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_shmput() - put a share memory
 * 
 * @shmid: share memory id
 * 
 * put(free) a share memory area from kernel
 *  
 * @return: 0 is success, -1 is failed!
 */
int x_shmput(int shmid)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_SHMPUT);
    umsg_set_arg0(msg, shmid);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_shmmap() - map a share memory
 * 
 * @shmid: share memory id
 * @shmaddr: process addr.
 *      this can be a fixed addr or null addr.
 *      if fixed addr, map this addr with share memory.
 *      if null addr, auto select a addr map with share memory.
 * 
 * map shmaddr with share memory addr, you can use shmaddr to access share memory.
 *  
 * @return: not -1 is success, -1 is failed!
 */
void *x_shmmap(int shmid, const void *shmaddr)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_SHMMAP);
    umsg_set_arg0(msg, shmid);
    umsg_set_arg1(msg, shmaddr);
    umsg(msg);
    return umsg_get_retval(msg, void *);
}

/**
 * x_shmunmap() - unmap a share memory
 * 
 * @shmaddr: process addr.
 *    
 * unmap shmaddr with share memory addr, after that you can't use
 * shmaddr to access share memory.
 *  
 * @return: 0 is success, -1 is failed!
 */
int x_shmunmap(const void *shmaddr)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_SHMUNMAP);
    umsg_set_arg0(msg, shmaddr);
    umsg(msg);
    return umsg_get_retval(msg, int);
}


/**
 * x_msgget() - get a message queue
 * 
 * @name: msg name
 * @flags: msg flags
 *         IPC_CREAT: create a new msgq or open a msgq.
 *          if ok, return msgid, or not return -1
 *         IPC_EXCL: only create a new msgq.
 *          this should use with IPC_CREAT, to make sure
 *          that the msgq not exist. example: IPC_CREAT|IPC_EXCL
 * 
 * get a message queue from kernel
 *  
 * @return: return msgid >= 0 is success, -1 means failed!
 */
int x_msgget(char *name, int flags)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_MSGGET);
    umsg_set_arg0(msg, name);
    umsg_set_arg1(msg, flags);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_msgput() - put a message queue
 * 
 * @msgid: message queue id
 * 
 * put(free) a message queue from kernel
 *  
 * @return: 0 is success, -1 is failed!
 */
int x_msgput(int msgid)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_MSGPUT);
    umsg_set_arg0(msg, msgid);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_msgsnd() - send a message to message queue
 * 
 * @msgid: message queue id
 * @msgbuf: the message buffer addr, this is a special struct,
 * 			the first member must be 'long msgtype'! you can 
 * 			use x_msgbuf_t for general work.
 * @msgsz:  the message buf size, not include msgtype in msgbuf
 * 			struct. msgsz = sizeof(msgbuf) - sizeof(long)
 * @msgflg: message send flags.
 * 			IPC_NOWAIT: if message queue fulled, process won't block, return -1
 * 
 * send a message to message queue(msgid)
 * 
 * @return: 0 is success, -1 is failed!
 */
int x_msgsnd(int msgid, const void *msgbuf, size_t msgsz, int msgflg)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_MSGSND);
    umsg_set_arg0(msg, msgid);
    umsg_set_arg1(msg, msgbuf);
	umsg_set_arg2(msg, msgsz);
	umsg_set_arg3(msg, msgflg);
    umsg(msg);
    return umsg_get_retval(msg, int);
}
/**
 * x_msgrcv() - receive a message from message queue
 * 
 * @msgid: message queue id
 * @msgbuf: the message buffer addr, this is a special struct,
 * 			the first member must be 'long msgtype'! you can 
 * 			use x_msgbuf_t for general work.
 * @msgsz:  the message buf size, not include msgtype in msgbuf
 * 			struct. msgsz = sizeof(msgbuf) - sizeof(long)
 * @msgtype: message type for receving priority
 *          =0：get first message in queue.
 *          >0：get first message in queue which message->msgtype = msgtype
 *          <0：get first message in queue which message->msgtype <= abs(msgtype)
 * @msgflg: message send flags.
 * 			IPC_NOWAIT: if message queue fulled, process won't block, return -1
 * 			IPC_NOERROR：the message longer than msgsz are truncated.
 * 			IPC_EXCEPT：if msgtype > 0, receive a message that message->msgtype <= abs(msgtype)
 * 
 * receive a message from message queue(msgid)
 * 
 * @return: return the bytes read, -1 is failed!
 */
int x_msgrcv(int msgid, const void *msgbuf, size_t msgsz, long msgtype, int msgflg)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_MSGRCV);
    umsg_set_arg0(msg, msgid);
    umsg_set_arg1(msg, msgbuf);
	umsg_set_arg2(msg, msgsz);
	umsg_set_arg3(msg, msgtype);
	umsg_set_arg4(msg, msgflg);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_semget() - get a semaphore
 * 
 * @name: sem name
 * @flags: sem flags
 *         IPC_CREAT: create a new semq or open a semq.
 *          if ok, return semid, or not return -1
 *         IPC_EXCL: only create a new semq.
 *          this should use with IPC_CREAT, to make sure
 *          that the semq not exist. example: IPC_CREAT|IPC_EXCL
 * 
 * get a semaphore from kernel
 *  
 * @return: return semid >= 0 is success, -1 means failed!
 */
int x_semget(char *name, int value, int flags)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_SEMGET);
    umsg_set_arg0(msg, name);
    umsg_set_arg1(msg, value);
    umsg_set_arg2(msg, flags);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_semput() - put a semaphore
 * 
 * @semid: semaphore id
 * 
 * put(free) a semaphore from kernel
 *  
 * @return: 0 is success, -1 is failed!
 */
int x_semput(int semid)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_SEMPUT);
    umsg_set_arg0(msg, semid);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_semdown() - down a semaphore
 * 
 * @semid: semaphore id
 * @semflg: message send flags.
 * 			IPC_NOWAIT: if try down semaphore failed, process won't block,
 *          return -1
 * 
 * to avoid mutex or make sync.
 * 
 * @return: 0 is success, -1 is failed!
 */
int x_semdown(int semid, int semflg)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_SEMDOWN);
    umsg_set_arg0(msg, semid);
    umsg_set_arg1(msg, semflg);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_semup() - up a semaphore
 * 
 * @semid: semaphore id
 * 
 * @return: 0 is success, -1 is failed!
 */
int x_semup(int semid)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_SEMUP);
    umsg_set_arg0(msg, semid);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_trigger() - set trigger handler
 * 
 * @trig: trigger
 * @handler: handler
 * 
 * @return: 0 is success, -1 is failed!
 */
int x_trigger(int trig, trighandler_t handler)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_TRIGGER);
    umsg_set_arg0(msg, trig);
    umsg_set_arg1(msg, handler);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_trigger_action() - set trigger action
 * 
 * @trig: trigger
 * @act: new action
 * @oldact: old action
 * 
 * @return: 0 is success, -1 is failed!
 */
int x_trigger_action(int trig, trig_action_t *act, trig_action_t *oldact)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_TRIGACT);
    umsg_set_arg0(msg, trig);
    umsg_set_arg1(msg, act);
    umsg_set_arg2(msg, oldact);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_triggeron() - trigger on
 * 
 * @trig: trigger
 * @pid: process id
 * 
 * active a trigger
 * 
 * @return: 0 is success, -1 is failed!
 */
int x_triggeron(int trig, pid_t pid)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_TRIGON);
    umsg_set_arg0(msg, trig);
    umsg_set_arg1(msg, pid);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

/**
 * x_getpid() - get pid
 * 
 * get task pid
 * 
 * @return: pid
 */
pid_t x_getpid()
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_GETPID);
    umsg(msg);
    return umsg_get_retval(msg, pid_t);
}

/**
 * x_getppid() - get parent pid
 * 
 * get task parent pid
 * 
 * @return: pid
 */
pid_t x_getppid()
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_GETPPID);
    umsg(msg);
    return umsg_get_retval(msg, pid_t);
}
