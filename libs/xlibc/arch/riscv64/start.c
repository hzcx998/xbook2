#include <stdlib.h>
#include <unistd.h>

extern int main();
extern void _enter_preload(int argc, char *const argv[], char *const envp[]);

int __start_main(long *p)
{
	int argc = p[0];
	char **argv = (void *)(p+1);
    /* do some init */
    _enter_preload(argc, argv, NULL);
	exit(main(argc, argv));
	return 0;
}
