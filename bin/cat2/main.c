#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    printf("cat2: I will print everything from stdin:\n");
	if (argc == 1) {
        char buf = 0;
        int ret = 0;
        do {
            ret = read(STDIN_FILENO, &buf, 1);
            if (ret > 0)
                write(STDOUT_FILENO, &buf, 1);
        } while (ret > 0);
        return 0;
	}
	return -1;
}
