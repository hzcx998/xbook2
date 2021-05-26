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
        char buf = 0;
        int ret = 0;
        do {
            ret = read(STDIN_FILENO, &buf, 1);
        } while (ret > 0);
        return 0;
	}
	if(argc > 2){
		fprintf(stderr,"cat: only support 2 argument!\n");
		return -1;
	}
	
    const char *path = (const char *)argv[1];
    int fd = open(path, O_RDONLY);
	if(fd == -1){
		fprintf(stderr,"cat: file %s not exist!\n", path);
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
