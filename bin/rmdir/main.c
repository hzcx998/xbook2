#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int ret = -1;
	if(argc != 2){
		fprintf(stderr,"mkdir: no argument support!\n");
	}else{
        if(rmdir(argv[1]) == 0){
            //printf("rmdir: remove %s success.\n", argv[1]);
            ret = 0;
        }else{
            fprintf(stderr,"rmdir: remove %s faild!\n", argv[1]);
        }
		
	}
	return ret;
}
