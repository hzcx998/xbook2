#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#if defined(__XLIBC__)
#include <sys/stat.h>

#define _HAS_FPRINTF
#define _HAS_MALLOC
#elif defined(__TINYLIBC__)
#ifndef STDIN_FILENO
#define STDIN_FILENO STDIN
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO STDOUT
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO STDERR
#endif

#define _HAS_FSTAT
#endif

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
        #ifdef _HAS_FPRINTF
		fprintf(stderr,"cat: only support 2 argument!\n");
		#else
        printf("cat: only support 2 argument!\n");
        #endif
        return -1;
	}
	
    const char *path = (const char *)argv[1];
    int fd = open(path, O_RDONLY);
	if(fd < 0){
        #ifdef _HAS_FPRINTF
		fprintf(stderr,"cat: file %s not exist!\n", path);
		#else
        printf("cat: file %s not exist!\n", path);
        #endif
        return -1;
	}
	#ifdef _HAS_FSTAT
	struct kstat st;
	fstat(fd, &st);
    #else
	struct stat st;
	stat(path, &st);
	#endif
    #ifdef _HAS_MALLOC
	char *buf = (char *)malloc(st.st_size);
	#else
    char buf[1024] = {0};
    if (st.st_size > 1024)
        st.st_size = 1024;
    #endif
    int bytes = read(fd, buf, st.st_size);
	//printf("read %s fd%d:%d\n", path,  fd, bytes);
    if (bytes <= 0) {
        close(fd);
	    return -1;
    }
	close(fd);
	
	int i = 0;
	while(i < bytes){
        char s[2] = {buf[i], 0};
		printf("%s", s);
		i++;
	}
    #ifdef _HAS_MALLOC
	free(buf);
    #endif
	//printf("\n");
	return 0;
}
