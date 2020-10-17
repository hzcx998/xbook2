#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/vmm.h>
#include <sys/stat.h>
#include <const.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if(argc < 3){
		printf("mv: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		printf("mv: src pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		printf("mv: dst pathname can't be . or .. \n");	
		return -1;
	}

    /* 如果2者相等则不能进行操作 */
    if (!strcmp(argv[1], argv[2])) {
        printf("mv: source file and dest file must be differern!\n");	
		return -1;
    }
    /* 复制逻辑：
        1. 打开两个文件，不存在则创建，已经存在则截断
        2. 复制数据
        3.关闭文件
     */
    int fdrd = open(argv[1], O_RDONLY);
    if (fdrd == -1) {
        printf("mv: open file %s failed!\n", argv[1]);
        return -1;
    }
    /* 如果文件已经存在则截断 */
    int fdwr = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC);
    if (fdwr == -1) {
        printf("mv: open file %s failed!\n", argv[2]);
        close(fdrd);
        return -1;
    }

    struct stat fstat;

    if (stat(argv[1], &fstat) < 0) {
        printf("mv: get file %s state failed!\n", argv[1]);
        close(fdrd);
        close(fdwr);
        return -1;
    }

    /* 每次操作512字节 */
    char *buf = malloc(fstat.st_size);
    if (buf == NULL) {
        printf("mv: malloc for size %d failed!\n", fstat.st_size);
        goto err;
    }

    char *p = buf;
    int size = fstat.st_size;
    int readBytes;

    /* 每次读取64kb */
    int chunk = (size & 0xffff) + 1;

    if (chunk == 0) {
        chunk = 0xffff;
        size -= 0xffff;
    }    
    while (size > 0) {  
        readBytes = read(fdrd, p, chunk);
        //printf("read:%d\n", readBytes);
        if (readBytes == -1) {  /* 应该检查是错误还是结束 */
            goto failed; 
        }
        if (write(fdwr, p, readBytes) == -1) {
            goto failed;  
        }
        p += chunk;
        size -= 0xffff;
        chunk = 0xffff;
    }

    /* 设置模式和原来文件一样 */
    chmod(argv[2], fstat.st_mode);
    
    free(buf);
    /* 复制结束 */
    close(fdrd);
    close(fdwr);

    /* 移动后删除源文件 */
    if(remove(argv[1]) == 0){
        //printf("mv: delete source file %s success.\n", argv[1]);
    }else{
        printf("mv: delete source file %s faild!\n", argv[1]);
        /* 删除复制后的文件 */
        remove(argv[2]);
    }

    return 0;
failed:
    printf("mv: transmit data error!\n");
    free(buf);
err:
    /* 复制结束 */
    close(fdrd);
    close(fdwr);
    return -1;
}
