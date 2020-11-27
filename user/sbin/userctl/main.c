#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
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
 *add user:usrctl username password 
 *del user:usrctl -d username 
 */
int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("too few argments!\n");
        // TODO: show usage
        return -1;
    }
    char *p;
    int del_account = 0;
    p = argv[1];
    if (*p == '-' && *(p+1)) {
        if (*(p+1) == 'd') {
            del_account = 1;
        }
    }

    char buf[129] = {0};
    while (!buf[0]) {
        printf("enter your password: ");
        uint32_t oldflgs = 0;
        ioctl(STDIN_FILENO, TIOCGFLGS, &oldflgs);
        uint32_t newflgs = oldflgs & ~TTYFLG_ECHO;
        ioctl(STDIN_FILENO, TIOCSFLGS, &newflgs);     
        readline(buf, 128);
        ioctl(STDIN_FILENO, TIOCSFLGS, &oldflgs);
        printf("\n");
    }
    if (accountverify(buf) < 0) {
        printf("password error!\n");
        return -1;
    }
    if (del_account) {
        if (unregister_account(argv[2]) < 0) {
            printf("account %s not exist or no permission to unregister it!\n", argv[2]);
            return -1;
        }
        printf("unregister account %s success.\n", argv[2]);    
    } else {
        if (register_account(argv[1], argv[2]) < 0) {
            printf("account %s had existed or no permission to register it!\n", argv[2]);
            return -1;
        }
        printf("register account %s success.\n", argv[2]);    
    }
    return 0;
}
