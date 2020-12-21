#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xgui.h>

int main(int argc, char *argv[])
{
    printf("hello xgui!\n");
    xgui_init();
    xgui_loop();
    return 0;
}