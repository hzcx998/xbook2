#include <stdio.h>
#include <stdlib.h>

#include <sh_shell.h>
#include <sh_console.h>
#include <sh_window.h>

int init_shell()
{
    if (init_console() < 0) {
        printf("bosh: init console failed!\n");
        return -1;
    }
    return 0;
}

void main_shell()
{
    main_window();
}

void exit_shell()
{
    exit_console();
    exit(0);
}
