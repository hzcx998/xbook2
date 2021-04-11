#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
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
