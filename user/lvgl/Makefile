X_LIBS		+= libxlibc-0.0.1.a libgapi-0.1.0.a

X_CFLAGS    := -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=c99 -O2

NAME		:= lv
SRC			+= main.c 
SRC			+= lvgl/src/lv_core/*.c 
SRC			+= lvgl/src/lv_draw/*.c 
SRC			+= lvgl/src/lv_font/*.c 
SRC			+= lvgl/src/lv_gpu/*.c 
SRC			+= lvgl/src/lv_hal/*.c 
SRC			+= lvgl/src/lv_misc/*.c 
SRC			+= lvgl/src/lv_themes/*.c
SRC			+= lvgl/src/lv_widgets/*.c
SRC			+= lvgl/examples/lv_demo_benchmark/*.c
SRC			+= lvgl/examples/lv_demo_keypad_encoder/*.c
SRC			+= lvgl/examples/lv_demo_printer/*.c
SRC			+= lvgl/examples/lv_demo_printer/images/*.c
SRC			+= lvgl/examples/lv_demo_stress/*.c
SRC			+= lvgl/examples/lv_demo_widgets/*.c
SRC			+= lvgl/examples/lv_demo_started/*.c
SRC			+= lvgl/examples/lv_ex_get_started/*.c
SRC			+= lvgl/examples/lv_ex_get_style/*.c
SRC			+= lvgl/examples/lv_ex_get_widgets/*.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin; \
rm $@
endef
