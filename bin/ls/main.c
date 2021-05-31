#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#if defined(__XLIBC__)
#include <sys/stat.h>
#include <dirent.h>
#include <sys/time.h>

static void __ls(char *pathname, int detail)
{
	DIR *dir = opendir(pathname);
	if(dir == NULL){
		printf("ls: pathname %s not exist or no permission to access it!\n", pathname);
        return;
	}
	rewinddir(dir);
	
	struct dirent *de;

    char subpath[MAX_PATH] = {0};

    struct stat fstat;  
    char type;
    char attrR, attrW, attrX;   /* 读写，执行属性 */ 

    do {
        if ((de = readdir(dir)) == NULL) {
            break;
        }
        //printf("de.type:%d", de.type);
        
        if (detail) {
            /* 列出详细信息 */
            if (de->d_attr & DE_DIR) {
                type = 'd';
            } else if (de->d_attr & DE_BLOCK) {
                type = 'b';
            } else if (de->d_attr & DE_CHAR) {
                type = 'c';
            } else {
                type = '-';
            }
            
            memset(subpath, 0, MAX_PATH);
            /* 合并路径 */
            strcat(subpath, pathname);
            strcat(subpath, "/");
            strcat(subpath, de->d_name);
                
            memset(&fstat, 0, sizeof(struct stat));

            /* 如果获取失败就获取下一个 */
            if (stat(subpath, &fstat)) {
                printf("ls: get state %s error!\n", subpath);    
                continue;
            }
            
            if (fstat.st_mode & S_IREAD) {
                attrR = 'r';
            } else {
                attrR = '-';
            }

            if (fstat.st_mode & S_IWRITE) {
                attrW = 'w';
            } else {
                attrW = '-';
            }

            if (fstat.st_mode & S_IEXEC) {
                attrX = 'x';
            } else {
                attrX = '-';
            }

            /* 类型,属性，文件日期，大小，名字 */
            printf("%c%c%c%c %04d/%02d/%02d %02d:%02d:%02d %12d %s\n",
                type, attrR, attrW, attrX,
                WTM_RD_YEAR(fstat.st_mtime>>16),
                WTM_RD_MONTH(fstat.st_mtime>>16),
                WTM_RD_DAY(fstat.st_mtime>>16),
                WTM_RD_HOUR(fstat.st_mtime&0xffff),
                WTM_RD_MINUTE(fstat.st_mtime&0xffff),
                WTM_RD_SECOND(fstat.st_mtime&0xffff),
                fstat.st_size, de->d_name);
            //printf("type:%x inode:%d name:%s\n", de.type, de.inode, de.name);
        } else {
            printf("%s ", de->d_name);
        }
    } while (1);
	
	closedir(dir);
    if (!detail) {
        printf("\n");
    }
}
#elif defined(__TINYLIBC__)


enum {
    DT_UNKNOWN = 0,         //未知类型
#define DT_UNKNOWN DT_UNKNOWN
    DT_FIFO = 1,            //管道
#define DT_FIFO DT_FIFO
    DT_CHR = 2,             //字符设备
#define DT_CHR DT_CHR
    DT_DIR = 4,             //目录
#define DT_DIR DT_DIR
    DT_BLK = 6,             //块设备
#define DT_BLK DT_BLK
    DT_REG = 8,             //常规文件
#define DT_REG DT_REG
    DT_LNK = 10,            //符号链接
#define DT_LNK DT_LNK
    DT_SOCK = 12,           //套接字
#define DT_SOCK DT_SOCK
    DT_WHT = 14             //链接
#define DT_WHT DT_WHT
};
/* WR: write RD: read */
#define WTM_WR_TIME(hou, min, sec) ((unsigned short)((((hou) & 0x1f) << 11) | \
        (((min) & 0x3f) << 5) | (((sec) / 2) & 0x1f)))
#define WTM_WR_DATE(yea, mon, day) (((unsigned short)(((yea) - 1980) & 0x7f) << 9) | \
        (((mon) & 0xf) << 5) | ((day) & 0x1f))

#define WTM_RD_HOUR(data)    ((unsigned int)(((data) >> 11) & 0x1f))
#define WTM_RD_MINUTE(data)  ((unsigned int)(((data) >> 5) & 0x3f))
#define WTM_RD_SECOND(data)  ((unsigned int)(((data) & 0x1f) * 2))
#define WTM_RD_YEAR(data)    ((unsigned int)((((data) >> 9) & 0x7f) + 1980))
#define WTM_RD_MONTH(data)   ((unsigned int)(((data) >> 5) & 0xf))
#define WTM_RD_DAY(data)     ((unsigned int)((data) & 0x1f))

#define AT_CWD -100

#define MAX_PATH 260
char* strcat(char* strDest , const char* strSrc)
{
    char* address = strDest;
    while(*strDest)
    {
        strDest++;
    }
    while((*strDest++=*strSrc++));
    return (char* )address;
}


/*
 * st_mode flags
 */
#define         S_IFMT  0170000 /* type of file ，文件类型掩码*/
#define         S_IFREG 0100000 /* regular 普通文件*/
#define         S_IFBLK 0060000 /* block special 块设备文件*/
#define         S_IFDIR 0040000 /* directory 目录文件*/
#define         S_IFCHR 0020000 /* character special 字符设备文件*/
#define         S_IFIFO 0010000 /* fifo */
#define         S_IFNAM 0050000 /* special named file */
#if !defined(_M_XOUT)
#define         S_IFLNK 0120000 /* symbolic link 链接文件*/
#endif /* !defined(_M_XOUT) */
#define         S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#define         S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)
#define         S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#define         S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)
#define         S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)
#define         S_ISNAM(m)      (((m) & S_IFMT) == S_IFNAM)
#if !defined(_M_XOUT)
#define         S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)
#endif /* !defined(_M_XOUT) */
#define         S_IREAD 0x04     //文件所有者具可读取权限
#define         S_IWRITE 0x02    //文件所有者具可写入权限
#define         S_IEXEC 0x01     //文件所有者具可执行权限


static void __ls(char *pathname, int detail)
{
	int dir = open(pathname, O_DIRECTORY);
	if(dir < 0){
		printf("ls: pathname %s not exist or no permission to access it!\n", pathname);
        return;
	}
    int nread;
    struct linux_dirent64 *dirp64, *d;
    char buf[512];
    dirp64 = (struct linux_dirent64 *)buf;

    char subpath[MAX_PATH] = {0};

    struct kstat st;  
    char type;
    char attrR, attrW, attrX;   /* 读写，执行属性 */ 

    while (1) {
        nread = getdents(dir, dirp64, 512);
        //printf("getdents fd:%d\n", nread);
        if (nread <= 0) {
            //printf("getdents done\n");
            break;
        }
        for(int bpos = 0; bpos < nread; bpos += d->d_reclen){
            d = (struct linux_dirent64 *)(buf + bpos);
            if (detail) {
                #if 1
                /* 列出详细信息 */
                if (d->d_type == DT_DIR) {
                    type = 'd';
                } else if (d->d_type == DT_BLK) {
                    type = 'b';
                } else if (d->d_type == DT_CHR) {
                    type = 'c';
                } else {
                    type = '-';
                }
                
                memset(subpath, 0, MAX_PATH);
                /* 合并路径 */
                strcat(subpath, pathname);
                strcat(subpath, "/");
                strcat(subpath, d->d_name);
                
                memset(&st, 0, sizeof(struct kstat));
                int fd = openat(AT_CWD, subpath, O_RDONLY);
                if (fd < 0) {
                    printf("ls: open file %s error!\n", subpath);    
                    continue;
                }
                
                /* 如果获取失败就获取下一个 */
                if (fstat(fd, &st)) {
                    printf("ls: get state %s error!\n", subpath);    
                    close(fd);
                    continue;
                }
                close(fd);
                if (st.st_mode & S_IREAD) {
                    attrR = 'r';
                } else {
                    attrR = '-';
                }

                if (st.st_mode & S_IWRITE) {
                    attrW = 'w';
                } else {
                    attrW = '-';
                }

                if (st.st_mode & S_IEXEC) {
                    attrX = 'x';
                } else {
                    attrX = '-';
                }

                /* 类型,属性，文件日期，大小，名字 */
                char attr_str[5] = {type, attrR, attrW, attrX, 0};
                printf("%s ", attr_str);
                printf("%d/%d/%d %d:%d:%d %d %s\n",
                    WTM_RD_YEAR(st.st_mtime_sec>>16),
                    WTM_RD_MONTH(st.st_mtime_sec>>16),
                    WTM_RD_DAY(st.st_mtime_sec>>16),
                    WTM_RD_HOUR(st.st_mtime_sec&0xffff),
                    WTM_RD_MINUTE(st.st_mtime_sec&0xffff),
                    WTM_RD_SECOND(st.st_mtime_sec&0xffff),
                    st.st_size, d->d_name);
                #endif
                //printf("type:%x inode:%d name:%s\n", de.type, de.inode, de.name);
            } else {
                printf("%s ", d->d_name);
            }
        }
    }
	close(dir);
    if (!detail) {
        printf("\n");
    }
}
#endif
int main(int argc, char *argv[])
{
    /* 只有一个参数 */
    if (argc == 1) {
        /* 列出当前工作目录所在的文件 */
        __ls(".", 0);
    } else {
        /*  */
        int arg_path_nr = 0;
        int arg_idx = 1;	//跳过argv[0]
        char *path = NULL;

        int detail = 0;
        while(arg_idx < argc){
            if(argv[arg_idx][0] == '-'){//可选参数
                char *option = &argv[arg_idx][1];
                /* 有可选参数 */
                if (*option) {
                    /* 列出详细信息 */
                    if (*option == 'l') {
                        detail = 1;
                    } else if (*option == 'h') {
                        printf("Usage: ls [option]\n");
                        printf("Option:\n");
                        printf("  -l    Print all infomation about file or directory. Example: ls -l \n");
                        printf("  -h    Get help of ls. Example: ls -h \n");
                        printf("  [dir] Print [dir] file or directory. Example: ls / \n");
                        printf("Note: If no arguments, only print name in cwd.\n");
                        
                        return 0;
                    } 
                }
            } else {
                if(arg_path_nr == 0){
                    /* 获取路径 */
                    path = argv[arg_idx];
                    
                    arg_path_nr = 1;
                }else{
                    printf("ls: only support one path!\n");
                    return -1;
                }
            }
            arg_idx++;
        }
        if (path == NULL) { /* 没有路径就列出当前目录 */
            __ls(".", detail);
        } else {    /* 有路径就列出路径 */
            __ls(path, detail);
        }
    }
    return 0;
}
