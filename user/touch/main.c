#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if(argc == 1){	//只有一个参数，自己的名字，退出
		printf("touch: please input filename!\n");
		return 0;
	}
	if(argc > 2){
		printf("touch: only support 2 argument!\n");
		return -1;
	}
	
    const char *path = (const char *)argv[1];

	int fd = open(path, O_CREAT | O_RDWR, 0);
	if(fd == -1){
		printf("touch: fd %d error\n", fd);
		return 0;
	}

	close(fd);
	return 0;
}
