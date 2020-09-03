#include <stdio.h>
#include <stdlib.h>
#include <sh_shell.h>

int main(int argc, char *argv[]) 
{

    if (init_shell() < 0)
        return -1;
    main_shell();
    exit_shell();
    return 0;
}
