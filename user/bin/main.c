#include <xcore/xcore.h>
#include <xcore/ioctl.h>
#include <xcore/ipc.h>
#include <conio.h>
#include <string.h>

void semtest1()
{
    int semid = getres("sem_test", RES_IPC | IPC_SEM | IPC_CREAT, 1);
    if (semid < 0)
        exit(-1);
    printf("get sem id %d\n", semid);
    while (1)
    {
        writeres(semid, 0, NULL, 0);
        printf("bin1:  abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg-%d\n", semid);
        printf("bin2:  abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg-%d\n", semid);
        printf("bin3:  abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg-%d\n", semid);
        printf("bin4:  abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg-%d\n", semid);
        printf("bin5:  abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg-%d\n", semid);
        readres(semid, 0, NULL, 0);
    }
}

void msgtest()
{
    
    int msgid = getres("msg_test", RES_IPC | IPC_MSG | IPC_CREAT, 0);
    if (msgid < 0) {
        printf("test: parent: get msg failed!");
        return;
    }
    printf("bin: parent: get msg %d.\n", msgid);
    
    unsigned long hp = heap(0);
    heap(hp + 1024 + sizeof(x_msgbuf_t));

    x_msgbuf_t *msgbuf = (x_msgbuf_t *) hp;
    while (1)
    {
        int i;
        /*
        int i, j, k;
        for (j = 0; j < 1000; j++) {
            for (k = 0; k < 10000; k++);
        }*/
        msgbuf->type = 100; /* 读取消息的类型 */
        memset(msgbuf->text, 0, 1024);
        int read = readres(msgid, 0, msgbuf, 1024);
        printf("bin: parent: rcv msg %d type=%d!\n", read, msgbuf->type);
        for (i = 0; i < 16; i++) {
            printf("%x ", msgbuf->text[i]);
        }
        printf("\n");
    }
    
    printf("bin: parent: rcv msg ok!");

}

void shmtest2()
{ 
    /* 共享内存 */
    int shmid2 = getres("test", RES_IPC | IPC_SHM | IPC_CREAT, 4095);
    if (shmid2 < 0)
        exit(-1);

    printf("get shm id %d\n", shmid2);
    unsigned long shmaddr2;
    if (writeres(shmid2, 0, NULL, (size_t )&shmaddr2))
        printf("1: shm map failed!\n");

    printf("shm map at %x\n", shmaddr2);
    
    int *shmdata = (int *)shmaddr2;
    int j;
    for (j = 0; j < 16; j++) {
        printf("%x ", shmdata[j]);
    }
    readres(shmid2, 0, shmaddr2, 0);
}
int handle_count = 0;
void trigger_handler(int trig)
{
    printf("bin: trigger %d handled.\n", trig);
    handle_count++;
    if (handle_count > 10)
        triggeron(TRIGHSOFT, getpid());
}

void trigger_test()
{

    /*
    int a = 0;
    int b = a / 0;

    char *addr = (char *)0;
    *addr = 0;
    */

    trigger(TRIGHW, trigger_handler);
    trigger(TRIGLSOFT, TRIG_IGN);

    trig_action_t act = {trigger_handler, 0};
    trig_action_t oldact;
    
    trigger_action(TRIGHW, &act, &oldact);
    printf("oldact: handler=%x flags=%x\n", oldact.handler, oldact.flags);
    printf("act: handler=%x flags=%x\n", act.handler, act.flags);
    
    char *addr = (char *)0;
    *addr = 0;
    
    int a = 0;
    int b = a / 0;

    triggeron(TRIGUSR0, getpid());
    triggeron(TRIGLSOFT, getpid());
    triggeron(TRIGHW, getpid());
    
}

void pipe_test_read()
{
    int pip = getres("pipe_test", RES_IPC | IPC_PIPE | IPC_CREAT | IPC_READER, 0);
    if (pip < 0)
        return;
    char buf[8192];
    memset(buf, 0, 8192);
    //while (1) {
        int i, j;
        for (i = 0; i < 300; i++)
            for (j = 0; j < 100000; j++);
            
        int read = readres(pip, IPC_NOSYNC, buf, 4096);

        printf("child read pipe done! %d bytes.\n", read);
        for (i = 0; i < 32; i++) {
            printf("%c ", buf[i]);
        }
        printf("\n");
        
    //}
    putres(pip);
}

int main(int argc, char *argv[])
{
    printf("hello, bin!\n");
    
    pipe_test_read();
    trigger_test();
    
    shmtest2();
    //semtest1();
    //msgtest();
    putres(1);
    
    return 0;   
}
