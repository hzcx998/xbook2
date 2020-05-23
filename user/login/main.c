#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/vmm.h>

#define CONFIG_USER   "xbook"
#define CONFIG_PWD    "1234"

#define INPUT_BUF_LEN   80

#define BIN_NAME        "0:/bosh"

int read_key();
void input_buf(char *buf, char pwd);

int main(int argc, char *argv[])
{
    printf("login: say, hello!\n");
    printf("login: welcome to xbook kernel, please login.\n");
    char user_name[INPUT_BUF_LEN] = {0, };
    char pwd_name[INPUT_BUF_LEN] = {0, };
    res_ioctl(RES_STDINNO, TTYIO_HOLDER, getpid()); /* set keyboard holder */
    goto login_ok;
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
login_ok:
    printf("\nlogin: login success! :)\n");

    /* 打开shell */
    if (execv(BIN_NAME, NULL)) {
        printf("login: execv shell failed!\n");
        exit(-1);
    }
    return 0;
}

int read_key()
{
    int key = 0;
    int ret = 0;
    while (1) {
        ret = res_read(RES_STDINNO, 0, &key, 1);
        if (ret > 0)
            break;
    }
    return key;  /* 只取8位 */
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
