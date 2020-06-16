#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sgi/sgi.h>
#include "terminal.h"
#include "window.h"

int main(int argc, char *argv[])
{
    printf("The app %s is started.\n", APP_NAME);

    if (init_con_screen() < 0) {
        return -1;
    }
    if (con_open_window() < 0) {
        return -1;
    }

    screen.outs("hello, world!\nabc\n\rdef!");

    con_event_loop();
    con_close_window();
    return 0;
}
