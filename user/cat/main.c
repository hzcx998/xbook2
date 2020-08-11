#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
	/* 如果没有参数，就接收输入，输入类型：管道，文件，设备 */
    if (argc == 1) {
        /*  */
        /*char buf;
        while (read(STDIN_FILENO, &buf, 1) > 0) {
            printf("%c", buf);
        }*/

        /* 只接受4096字节输入 */
        char *buf = malloc(4096 + 1);
        memset(buf, 0, 4096 + 1);
        int readBytes = read(STDIN_FILENO, buf, 4096);
        //printf("read fd0:%d\n", readBytes);
        if (readBytes > 0) {            
            char *p = buf;
            while (readBytes--) {
                printf("%c", *p);
                p++;
            }
        }
        free(buf);
        return 0;
	}
	if(argc > 2){
		printf("cat: only support 2 argument!\n");
		return -1;
	}
	
    const char *path = (const char *)argv[1];

	int fd = open(path, O_RDONLY, 0);
	if(fd == -1){
		printf("cat: file %s not exist!\n", path);
		return -1;
	}
	
	struct stat fstat;
	stat(path, &fstat);
	
	char *buf = (char *)malloc(fstat.st_size);
	
	int bytes = read(fd, buf, fstat.st_size);
	//printf("read %s fd%d:%d\n", path,  fd, bytes);
	close(fd);
	
	int i = 0;
	while(i < bytes){
		printf("%c", buf[i]);
		i++;
	}
	free(buf);
	//printf("\n");
	return 0;
}
