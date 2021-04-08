#include <sys/syscall.h>
#include <sys/ipc.h>

int shmget(char *name, unsigned long size, unsigned long flags)
{
    return syscall3(int, SYS_SHMGET, name, size, flags);
}

int shmput(int shmid)
{
    return syscall1(int, SYS_SHMPUT, shmid);
}

void *shmmap(int shmid, void *shmaddr, int shmflg)
{
    return syscall3(void *, SYS_SHMMAP, shmid, shmaddr, shmflg);
}

int shmunmap(const void *shmaddr, int shmflg)
{
    return syscall2(int, SYS_SHMUNMAP, shmaddr, shmflg);
}

int semget(char *name, int value, int semflg)
{
    return syscall3(int, SYS_SEMGET, name, value, semflg);
}

int semput(int semid)
{
    return syscall1(int, SYS_SEMPUT, semid);
}

int semdown(int semid, int semflg)
{
    return syscall2(int, SYS_SEMDOWN, semid, semflg);
}

int semup(int semid)
{
    return syscall1(int, SYS_SEMUP, semid);
}


int msgget(char *name, unsigned long flags)
{
    return syscall2(int, SYS_MSGGET, name, flags);
}

int msgput(int msgid)
{
    return syscall1(int, SYS_MSGPUT, msgid);
}

int msgsend(int msgid, void *msgbuf, size_t size, int msgflg)
{
    return syscall4(int, SYS_MSGSEND, msgid, msgbuf, size, msgflg);
}

int msgrecv(int msgid, void *msgbuf, size_t msgsz, int msgflg)
{
    return syscall4(int, SYS_MSGRECV, msgid, msgbuf, msgsz, msgflg);
}
