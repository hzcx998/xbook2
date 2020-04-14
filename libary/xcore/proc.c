#include <xcore/syscall.h>
#include <xcore/xcore.h>

/**
 * exit() - exit process
 * 
 * @status: exit status, give to parent.
 * 
 * exit process, after that, the process end of it's life.
 */
void exit(int status)
{
    syscall1(int , SYS_EXIT, status);
}

/**
 * fork() - fork process
 * 
 * after fork, we have 2 process samed, but different pid.
 * p A is parent, p B is child, so B's parent pid os A's pid.
 * 
 * @return: process pid, there are 3 case:
 *          1. pid > 0: current process is A, pid is B's pid
 *          2. pid = 0: current process is B
 *          3. pid < 0: fork failed
 */
pid_t fork()
{
    return syscall0(pid_t, SYS_FORK); /* return a pid (int type) */
}

/**
 * wait() - wait process
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
int wait(int *status)
{
    return syscall1(int, SYS_WAIT, status);
}

/**
 * execraw() - execute raw block
 * 
 * @name: raw block name
 * @argv: arguments array
 * 
 * execute a raw block process, replaces the current process with the
 * raw block and runs the process corresponding to the raw block.
 * 
 * @return: -1 is failed, no success return, if success, run the new process. 
 */
int execraw(char *name, char *argv[])
{
    return syscall2(int, SYS_EXECR, name, argv);
}

/**
 * execfile() - execute file
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
int execfile(char *name, xfile_t *file, char *argv[])
{
    return syscall3(int, SYS_EXECF, name, file, argv);
}

pid_t getpid()
{
    return syscall0(pid_t, SYS_GETPID); /* return a pid (int type) */
}

pid_t getppid()
{
    return syscall0(pid_t, SYS_GETPPID); /* return a pid (int type) */
}

int trigger(int trig, trighandler_t handler)
{
    return syscall2(int, SYS_TRIGGER, trig, handler);
}

int trigger_action(int trig, trig_action_t *act, trig_action_t *oldact)
{
    return syscall3(int, SYS_TRIGGERACT, trig, act, oldact);
}

int triggeron(int trig, pid_t pid)
{
    return syscall2(int, SYS_TRIGGERON, trig, pid);
}
