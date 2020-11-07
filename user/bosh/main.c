#include <stdio.h>
#include <stdlib.h>
#include <sh_console.h>
#include <sh_cmd.h>
#include <sh_window.h>

int main(int argc, char *argv[]) 
{
    if (init_console() < 0) {
        printf("bosh: init console failed!\n");
        return -1;
    }
    
    if (init_cmd_man() < 0) {
        printf("bosh: init cmd failed!\n");
        exit_console();
        return -1;
    }

    #if 0
    char *argv[3] = {"/bin/infones", "/res/nes/mario.nes", NULL};
    execute_cmd(2, argv);
    #endif
    window_loop();
    exit_cmd_man();
    exit_console();
    return 0;
}
