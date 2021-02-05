#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if(argc < 3){
		printf("rename: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		printf("rename: pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		printf("rename: new name can't be . or .. \n");	
		return -1;
	}

	if(!rename(argv[1], argv[2])){
		//printf("rename: %s to %s sucess!\n", argv[1], argv[2]);	
		return 0;
	}else{
        printf("rename: %s to %s faild!\n", argv[1], argv[2]);	
		return -1;
	}

    return 0;
}
