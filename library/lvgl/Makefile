X_INCDIRS	:= xlibc/include gapi/include lvgl/
X_CFLAGS    := -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99

SRC			+= src/lv_core/
SRC			+= src/lv_draw/ 
SRC			+= src/lv_font/ 
SRC			+= src/lv_gpu/ 
SRC			+= src/lv_hal/ 
SRC			+= src/lv_misc/ 
SRC			+= src/lv_themes/
SRC			+= src/lv_widgets/
SRC			+= lv_drivers/indev/
SRC			+= lv_drivers/display/
SRC			+= lv_window.c

TARGET_TYPE	:= static
NAME		:= lvgl-7.6.0
