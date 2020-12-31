X_LIBS		+= libxlibc.a libpthread.a libpng.a libzlib.a libjpeg.a libxgui.a liblvgl.a

X_INCDIRS	+= xbrower/include xbrower

NAME		:= xbrower
SRC			+= main.c
SRC			+= hal/*.c
SRC			+= core/*.c
SRC			+= lv_drivers/display/*.c
SRC			+= lv_drivers/indev/*.c
SRC			+= lv_window.c
SRC			+= examples/lv_demo_widgets/
SRC			+= examples/lv_demo_keypad_encoder/

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../../develop/rom/bin
endef