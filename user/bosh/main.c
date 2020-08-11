#include <stdio.h>
#include <xcons.h>
#include <string.h>
#include <unistd.h>

#include "cmd.h"
#include "shell.h"

int main(int argc, char *argv[]) {
    printf("book os shell -v 0.01\n");

    if (init_cmd_man() < 0) {
        printf("init cmd failed!\n");
        return -1;
    }
    cmd_loop();

    exit_cmd_man();
    return 0;
}