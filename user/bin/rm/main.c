#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int ret = -1;
	if(argc != 2){
		fprintf(stderr,"rm: no argument support!\n");
	}else{
        if(remove(argv[1]) == 0){
            //printf("rm: delete %s success.\n", argv[1]);
            ret = 0;
        }else{
            fprintf(stderr,"rm: delete %s faild!\n", argv[1]);
        }
	}
	return ret;
}
