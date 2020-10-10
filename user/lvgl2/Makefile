X_LIBS		:= libxlibc-0.0.1.a libgapi-0.1.0.a
# X_LIBS		+= lib/lvgl-7.6.0.a

X_CFLAGS    := -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99


LVGL_LIB	:= ../../library/lvgl

# LVGL src
LVGL_SRC	:= $(LVGL_LIB)/src/lv_core/ 
LVGL_SRC	+= $(LVGL_LIB)/src/lv_draw/ 
LVGL_SRC	+= $(LVGL_LIB)/src/lv_font/ 
LVGL_SRC	+= $(LVGL_LIB)/src/lv_gpu/ 
LVGL_SRC	+= $(LVGL_LIB)/src/lv_hal/ 
LVGL_SRC	+= $(LVGL_LIB)/src/lv_misc/ 
LVGL_SRC	+= $(LVGL_LIB)/src/lv_themes/
LVGL_SRC	+= $(LVGL_LIB)/src/lv_widgets/
LVGL_SRC	+= $(LVGL_LIB)/lv_drivers/indev/
LVGL_SRC	+= $(LVGL_LIB)/lv_drivers/display/
LVGL_SRC	+= $(LVGL_LIB)/lv_window.c

NAME		:= lv2
SRC			:= main.c 
#SRC			+= examples/lv_demo_keypad_encoder/
#SRC			+= examples/lv_demo_printer/
#SRC			+= examples/lv_demo_printer/images/
SRC			+= examples/lv_demo_stress/

SRC			+= $(LVGL_SRC)

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin; \
rm $@
endef
