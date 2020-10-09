#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include  "lv_examples.h"
#include  "lv_lib.h"

int main(int argc, char **argv)
{
    if (lv_window_init("lv2", 300, 300, 640, 480) < 0)
        return -1;

    #if LV_USE_DEMO_WIDGETS
    lv_demo_widgets();
    #endif
    #if LV_USE_DEMO_PRINTER
    lv_demo_printer();
    #endif
    #if LV_USE_DEMO_KEYPAD_AND_ENCODER
    lv_demo_keypad_encoder();
    #endif
    lv_window_loop();
    return 0;
}