#include <sys/res.h>
#include <sys/vmm.h>
#include <sys/proc.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/pthread.h>
#include <sys/trigger.h>
#include <sys/time.h>
#include <ff.h>

#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/tcp.h>
#include <lwip/init.h>
#include <netif/etharp.h>
#include <lwip/timers.h>

#include <stdio.h>

#define TTY_NAME    "tty0"

#define DISK_NAME   "ide0"
//#define DISK_NAME   "vfloppy"

/* login arg */
#define BIN_OFFSET      200
#define BIN_SECTORS     100
#define BIN_SIZE        (BIN_SECTORS*512)
#define BIN_NAME        "login"

void thread_test();

void fatfs_test();

void lwip_test();

int main(int argc, char *argv[])
{
    res_open(TTY_NAME, RES_DEV, 0);
    res_open(TTY_NAME, RES_DEV, 0);
    res_open(TTY_NAME, RES_DEV, 0);
    res_ioctl(RES_STDINNO, TTYIO_CLEAR, 0);
    printf("init: say, hello!\n");

    int pid = fork();
    if (pid < 0) {
        printf("init: fork failed! exit now!\n");
        return -1;
    }

    if (pid > 0) {  /* parent */
        //printf("init-parent: pid is %d, my child pid is %d.\n", getpid(), pid);
        while (1) { /* loop */
#if 0
            int status = 0;
            pid = wait(&status);    /* wait one child exit */
            if (pid > 1) {
                printf("init: my child pid %d exit with status %d\n", pid, status);
            }
#endif
            int status = 0;
            int _pid;
            _pid = waitpid(-1, &status, 0);    /* wait one child exit */
            if (_pid > 1) {
                printf("init: my child pid %d exit with status %d\n", _pid, status);
            }
        }
    } else {
        /* execute a process */
        printf("init-child: pid is %d, my parent pid is %d.\n", getpid(), getppid());
        //exit(123);
        lwip_test();
        /* open disk */
        /*int ide0 = res_open(DISK_NAME, RES_DEV, 0);
        if (ide0 < 0) {
            printf("init-child: open disk '%s' %d failed! exit now.", DISK_NAME, ide0);
            return -1;
        }*/
        /* alloc memory for file */
        unsigned char *heap_pos = heap(0);
        //printf("init-child: heap addr %x.\n", heap_pos);

        heap(heap_pos + BIN_SIZE); // 40 kb memory

        unsigned char *buf = heap_pos;
        memset(buf, 0, BIN_SIZE);
       // printf("init-child: alloc data at %x for 40 kb.\n", buf);
        
        /* read disk sector for file: offset=200, sectors=50 */
        /*if (res_read(ide0, BIN_OFFSET, buf, BIN_SIZE) < 0) {
            printf("init-child: read disk sectors failed! exit now\n");
            return -1;
        }*/
        FATFS fs;           /* Filesystem object */
        FIL fil;            /* File object */
        FRESULT res;        /* API result code */
        UINT br;            /* Bytes written */
        f_mount(&fs, "hd1:", 0);

        res = f_open(&fil, "hd1:login", FA_OPEN_EXISTING | FA_READ);
        if (res != FR_OK) {
            printf("open file failed!\n");
            exit(res);
        }

        res = f_read(&fil, buf, BIN_SIZE, &br);
        if (res != FR_OK) {
            printf("read failed! %d\n", res);
            exit(res);
        }
        if (br != BIN_SIZE) {
            printf("read file failed! read %d\n", br);
            exit(br);
        }
        f_close(&fil);
        f_mount(0, "hd1:", 0);

        //printf("init-child: load data success.\n");
        
        kfile_t file = {buf, BIN_SIZE};
        /*int i;
        for (i = 0; i < 32; i++) {
            printf("%x ", buf[i]);
        }*/
        //printf("\ninit-child: free resource.\n");
        //res_close(ide0); /* close ide0 */

        char *_argv[4] = {BIN_NAME, "xbook", "1234", 0};
        exit(execfile(BIN_NAME, &file, _argv));
    }
    return 0;
}
#if 1
extern err_t
ethernetif_init(struct netif *netif);

extern void ethernetif_input(struct netif *netif);

struct netif rtl8139_netif;

void lwip_init_task(void)
{
    struct ip_addr ipaddr, netmask, gateway;

    lwip_init();

    IP4_ADDR(&gateway, 192,168,0,1);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&ipaddr, 192,168,0,105);
    
    netif_add(&rtl8139_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, ethernet_input);
    netif_set_default(&rtl8139_netif);
    netif_set_up(&rtl8139_netif);
}

void lwip_test()
{
    printf("ready test lwip\n");
    lwip_init_task();

    while (1)
    {
        ethernetif_input(&rtl8139_netif);
        sys_check_timeouts();
    }
    
}

#endif
#if 0
FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                printf("%s/%s\n", path, fno.fname);
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

void fatfs_test()
{
    printf("in fat fs test...\n");

    FATFS fs;           /* Filesystem object */
    FIL fil;            /* File object */
    FRESULT res;        /* API result code */
    UINT bw, br;            /* Bytes written */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */

    res = f_mkfs("hd1:", 0, work, sizeof(work));
    if (res != FR_OK) {
        printf("make fs on ram failed! %d \n", res);
        exit(res);
    }
    f_mount(&fs, "hd1:", 0);

    res = f_open(&fil, "hd1:test.txt", FA_CREATE_NEW | FA_WRITE | FA_READ);
    if (res != FR_OK) {
        printf("open file failed!\n");
        exit(res);
    }

    res = f_write(&fil, "hello, file!\n", 13, &bw);
    if (res != FR_OK) {
        printf("open write failed! %d\n", res);
        exit(res);
    }
    if (bw != 13) {
        printf("write file failed!\n");
        exit(bw);
    }
    printf("write file %d!\n", bw);

    f_lseek(&fil, 0);

    char fbuf[32];
    res = f_read(&fil, fbuf, bw, &br);
    if (res != FR_OK) {
        printf("open read failed! %d\n", res);
        exit(res);
    }

    printf("read bytes:%d -> %s\n", br, fbuf);

    f_printf(&fil, "hello, f_printf %d %x %s", 123, 0x123, "123");

    f_lseek(&fil, 13);
    
    res = f_read(&fil, fbuf, 32, &br);
    if (res != FR_OK) {
        printf("open read failed! %d\n", res);
        exit(res);
    }
    printf("read bytes:%d -> %s\n", br, fbuf);

    f_close(&fil);

    f_chdrive("hd1:");

    memset(fbuf, 0, 32);
    res = f_getcwd(fbuf, 32);
    if (res != FR_OK) {
        printf("getcwd failed! %d\n", res);
        exit(res);        
    } 
    printf("cwd/%s\n", fbuf);

    res = f_mkdir("/bin");
    if (res != FR_OK) {
        printf("f_mkdir %s failed! %d\n", "/bin", res);
        exit(res);
    }
    f_mkdir("/usr");
    if (res != FR_OK) {
        printf("f_mkdir %s failed! %d\n", "/usr", res);
        exit(res);
    }
    f_mkdir("/bin/supper");
    if (res != FR_OK) {
        printf("f_mkdir %s failed! %d\n", "/bin/supper", res);
        exit(res);
    }
    char pathbuf[256];
    strcpy(pathbuf, "/");
    scan_files(pathbuf);

    f_mount(NULL, "hd1:", 0);
    printf("test fatfs done!\n");
    
    /* mount drive */
    f_mount(&fs, "hd1:", 0);

    /* load disk to file system */
    /* open disk */
    int ide0 = res_open(DISK_NAME, RES_DEV, 0);
    if (ide0 < 0) {
        printf("init-child: open disk '%s' %d failed! exit now.", DISK_NAME, ide0);
        return;
    }
    /* alloc memory for file */
    unsigned char *heap_pos = heap(0);
    //printf("init-child: heap addr %x.\n", heap_pos);

    heap(heap_pos + BIN_SIZE); // 40 kb memory

    unsigned char *buf = heap_pos;
    memset(buf, 0, BIN_SIZE);
    // printf("init-child: alloc data at %x for 40 kb.\n", buf);
    
    /* read disk sector for file: offset=200, sectors=50 */
    if (res_read(ide0, BIN_OFFSET, buf, BIN_SIZE) < 0) {
        printf("init-child: read disk sectors failed! exit now\n");
        return;
    }

    memset(fbuf, 0, 32);
    res = f_getcwd(fbuf, 32);
    if (res != FR_OK) {
        printf("getcwd failed! %d\n", res);
        exit(res);        
    } 
    printf("cwd/%s\n", fbuf);

    res = f_open(&fil, "hd1:login", FA_CREATE_NEW | FA_WRITE | FA_READ);
    if (res != FR_OK) {
        printf("open file failed!\n");
        exit(res);
    }

    res = f_write(&fil, buf, BIN_SIZE, &bw);
    if (res != FR_OK) {
        printf("open write failed! %d\n", res);
        exit(res);
    }
    if (bw != BIN_SIZE) {
        printf("write file failed!\n");
        exit(bw);
    }
    printf("write file %d!\n", bw);
    f_close(&fil);
    f_mount(0, "hd1:", 0);
    return;
    while (1)
    {
        /* code */
    }
}
#endif 

#if 0
void *thread_test2(void *arg)
{
    printf("thread_test2: hello, thread %d, arg=%x\n", pthread_self(), (unsigned int)arg);
    //sleep(3);
    printf("thread_test2: will return soon!\n");
    return (void *)123;
}
void trigger_handler(int trig)
{
    printf("trigger_handler: trigger %d occur!\n", trig);

}

void *thread_test(void *arg)
{
    printf("thread_test: hello, thread %d, arg=%s\n", pthread_self(), arg);
    
    pthread_t tid;
    pthread_create(&tid, NULL, thread_test2, (void *)0x12345678);
    printf("thread_test: create thread %d\n", tid);
    
    pthread_join(tid, NULL);
    
    printf("thread_test: will return soon!\n");
    return (void *)pthread_self();
}

void thread_test()
{
    printf("init: testing...\n");
    unsigned char *stack_top;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
    /* get heap start addr */
    stack_top = heap(0);
    /* expand heap */
    heap(stack_top + attr.stacksize);
    memset(stack_top, 0, attr.stacksize);
    pthread_attr_setstackaddr(&attr, stack_top);
    /* create the thread */
    pthread_t tid;
    pthread_create(&tid, &attr, thread_test, "hello, THREAD1!");
    printf("init: create thread %d\n", tid);
    void *retval = 0;
    trigger(TRIGALARM, trigger_handler);
    alarm(1);
    /* wait tig thread */
    pthread_join(tid, &retval);
    printf("init: %d pthread_join %d status %x done!\n", pthread_self(), tid, retval);
    printf("init: will return!\n");
}
#endif