#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/vmm.h>

#define CONFIG_USER   "xbook"
#define CONFIG_PWD    "1234"

#define INPUT_BUF_LEN   80

//#define DISK_NAME   "ide0"
#define DISK_NAME   "vfloppy"

/* bosh arg */
#define BIN_OFFSET      400
#define BIN_SECTORS     100
#define BIN_SIZE        (BIN_SECTORS*512)
#define BIN_NAME        "bosh"

int read_key();
void input_buf(char *buf, char pwd);

int main(int argc, char *argv[])
{
    printf("login: say, hello!\n");
    printf("login: welcome to xbook kernel, please login.\n");
    char user_name[INPUT_BUF_LEN] = {0, };
    char pwd_name[INPUT_BUF_LEN] = {0, };
    res_ioctl(RES_STDINNO, TTYIO_HOLDER, getpid()); /* set keyboard holder */
    while (1) {
        printf("user name: ");
        /* input user */
        memset(user_name, 0, INPUT_BUF_LEN);
        input_buf(user_name, 0);
        printf("\npassword : ");
        /* input pwd */
        memset(pwd_name, 0, INPUT_BUF_LEN);
        input_buf(pwd_name, 1);

        /* match user&pwd */
        if (!strcmp(user_name, CONFIG_USER) && !strcmp(pwd_name, CONFIG_PWD)) {
            break;
        }
        printf("\nlogin: user name or password error! :(\n");
    }
    
    printf("\nlogin: login success! :)\n");
    /* open disk */
    int ide0 = res_open(DISK_NAME, RES_DEV, 0);
    if (ide0 < 0) {
        printf("nlogin: open disk '%s' failed! exit now.", DISK_NAME);
        return -1;
    }
    /* alloc memory for file */
    unsigned long heap_pos = heap(0);
    //printf("init-child: heap addr %x.\n", heap_pos);

    heap(heap_pos + BIN_SIZE); // 40 kb memory

    unsigned char *buf = (unsigned char *) heap_pos;
    memset(buf, 0, BIN_SIZE);
    // printf("init-child: alloc data at %x for 40 kb.\n", buf);
    
    /* read disk sector for file: offset=200, sectors=50 */
    if (res_read(ide0, BIN_OFFSET, buf, BIN_SIZE) < 0) {
        printf("nlogin: read disk sectors failed! exit now\n");
        return -1;
    }
    //printf("init-child: load data success.\n");
    
    kfile_t file = {buf, BIN_SIZE};
    /*int i;
    for (i = 0; i < 32; i++) {
        printf("%x ", buf[i]);
    }*/
    //printf("\ninit-child: free resource.\n");
    res_close(ide0); /* close ide0 */

    char *_argv[4] = {BIN_NAME, "tty0", "1234", 0};
    /* login no longer need， override it by shell. */
    exit(execfile(BIN_NAME, &file, _argv));
    
    while (1);  /* loop forever */

    return 0;   
}

int read_key()
{
    int key = 0;
    while ((res_read(RES_STDINNO, 0, &key, 1)) <= 0);
    return key & 0xff;  /* 只取8位 */
}

void input_buf(char *buf, char pwd)
{
    int i = 0;
    int key = 0;
    while (i < INPUT_BUF_LEN) {
        key = read_key();
        if (key == '\n') {  /* enter */
            buf[i] = '\0';  /* end of string */    
            break;
        }
        if ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'z') ||
        (key >= 'A' && key <= 'Z') || (key == '\b' && i > 0) || key == ' ') {
            if (!pwd) 
                printf("%c", key);
            
            if (key == '\b') {
                buf[i] = '\0';
                i--;
            } else {
                buf[i] = key;
                i++;
            }
        }
        
    }
}
