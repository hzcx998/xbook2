#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("sh: hello, shell!\n");
    char buf[12 + 1];
    int count = 0;
    while (count < 10)
    {
        memset(buf, 0, 12);
        read(0, buf, 12);
        printf("sh: %s\n", buf);
        count++;
    }
    printf("sh: exit!\n");
	return 0;
}
