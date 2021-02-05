#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/sys.h>
#include <sys/ioctl.h>

/**
 * readline - 读取一行输入
 * @buf: 缓冲区
 * @count: 数据量
 * 
 * 输入回车结束输入
 */
void readline(char *buf, uint32_t count)
{
    int len = 0;
    char *pos = buf;
    while (len < count)
    {
        read(STDIN_FILENO, pos, 1);
        if (*pos == '\n') {
            *(pos) = '\0'; // 修改成0
            break;
        } else if (*pos == '\b') {
            if (pos > buf) {
                *(pos) = '\0'; // 修改成0
                --pos;
                *(pos) = '\0'; // 修改成0
                len--;
            } else {
                len = 0;
            }
        } else {
            len++;
            pos++;
        }
    }
}

/* 
login:
纯命令，需要输入账户和密码。
login: -u username -p password [-s shellname] 
 */
int main(int argc, char *argv[])
{
    char *p;
    int i = 0;
    int flags = 0;
    char *username = NULL;
    char *password = NULL;
    char *shellpath = NULL;
    char pwdbuf[32 + 1] = {0};
    char namebuf[32 + 1] = {0};
    while ((p = argv[i])) {
        if (*p == '-' && (p+1)) {
            if (*(p+1) == 'u') { // -u
                flags = 1;
                i++;
                continue;
            } else if (*(p+1) == 'p') { // -u
                flags = 2;
            } else if (*(p+1) == 's') { // -s
                flags = 3;
                i++;
                continue;
            } else {
                printf("not support option `%c`!\n", *(p + 1));
            }
        }
        if (flags == 1) {   // username
            username = argv[i];
            flags = 0;
            if ((username == NULL) || (*username == 0) || (*username == '-')) { // no username
                printf("login failed! no user name.\n");
                return -1;
            }
        } else if (flags == 2) {   // password
            i++;
            password = argv[i];
            flags = 0;
            if (password == NULL || (*password == 0) || (*password == '-')) { // need input password
                printf("password: ");
                uint32_t oldflgs = 0;
                ioctl(STDIN_FILENO, TIOCGFLGS, &oldflgs);
                uint32_t newflgs = oldflgs & ~TTYFLG_ECHO;
                ioctl(STDIN_FILENO, TIOCSFLGS, &newflgs);
                readline(pwdbuf, 32);
                ioctl(STDIN_FILENO, TIOCSFLGS, &oldflgs);
                printf("\n");
                password = pwdbuf;
                break;
            }
            i--;
        } else if (flags == 3) {   // new shell
            shellpath = argv[i];
            flags = 0;
        }
        i++;
    }
    if (username && password) {
        if (login(username, password) < 0) {
            printf("login failed! please check your user name: %s, password: %s.\n", username, password); 
            return -1;
        }
    } else if (!username && !password) {
        printf("please input user name and password to login:\n");
        while (1) {
            username = NULL;
            password = NULL;
            memset(namebuf, 0, 32 + 1);
            while (!namebuf[0]) {    
                printf("user name: ");
                readline(namebuf, 32);
            }
            memset(pwdbuf, 0, 32 + 1);     
            uint32_t oldflgs = 0;
            ioctl(STDIN_FILENO, TIOCGFLGS, &oldflgs);
            uint32_t newflgs = oldflgs & ~TTYFLG_ECHO;
            ioctl(STDIN_FILENO, TIOCSFLGS, &newflgs);       
            while (!pwdbuf[0]) {    
                printf("password: ");
                readline(pwdbuf, 32);
            }
            ioctl(STDIN_FILENO, TIOCSFLGS, &oldflgs);
            printf("\n");
            username = namebuf;
            password = pwdbuf;
            if (!login(namebuf, pwdbuf)) {
                break;
            }
            printf("login failed! please check your user name: %s, password: %s.\n", username, password);            
        }
    } else {
        printf("login failed! please check your user name: %s, password: %s.\n",
                username == NULL ? "(null)" : username,
                password == NULL ? "(null)" : password);
        return -1;    
    }
    printf("user %s login success!\n", username);
    if (shellpath) { /* if need shell, execute it! */
        exit(execv(shellpath, NULL));
    }
	return 0;
}
