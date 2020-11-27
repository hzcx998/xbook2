#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int ret = -1;
	if(argc != 2){
		printf("mkdir: no argument support!\n");
	}else{
    
        if(mkdir(argv[1], 0) == 0){
            //printf("mkdir: create a dir %s success.\n", argv[1]);
            ret = 0;
        }else{
            printf("mkdir: create directory %s faild!\n", argv[1]);
        }
	}
	return ret;
}
