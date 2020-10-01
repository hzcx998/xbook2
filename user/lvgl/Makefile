X_LIBS		+= libxlibc-0.0.1.a libgapi-0.1.0.a

X_CFLAGS    := -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=c99 -O2 

NAME		:= lv
SRC			+= main.c 
SRC			+= lvgl/src/lv_core/
SRC			+= lvgl/src/lv_draw/ 
SRC			+= lvgl/src/lv_font/ 
SRC			+= lvgl/src/lv_gpu/ 
SRC			+= lvgl/src/lv_hal/ 
SRC			+= lvgl/src/lv_misc/ 
SRC			+= lvgl/src/lv_themes/
SRC			+= lvgl/src/lv_widgets/
SRC			+= lvgl/examples/lv_demo_benchmark/
SRC			+= lvgl/examples/lv_demo_keypad_encoder/
SRC			+= lvgl/examples/lv_demo_printer/
SRC			+= lvgl/examples/lv_demo_printer/images/
SRC			+= lvgl/examples/lv_demo_stress/
SRC			+= lvgl/examples/lv_demo_widgets/
SRC			+= lvgl/examples/lv_demo_started/
SRC			+= lvgl/examples/lv_ex_get_started/
SRC			+= lvgl/examples/lv_ex_get_style/
SRC			+= lvgl/examples/lv_ex_get_widgets/

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin; \
rm $@
endef
