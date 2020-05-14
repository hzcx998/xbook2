# tools
AR 			= ar
NASM 		= nasm
CC			= gcc
RM			= rm

# dir
DIR_INCLUDE 	= ./include
DIR_CODE 		= ./code

XBOOKLIB_INCLUDE 	= ../xbooklib/include

# flags
ARFLAGS := -cr
C_FLAGS	:=  -c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
			-fno-stack-protector -std=c99 \
			-I $(DIR_INCLUDE) \
			-I $(XBOOKLIB_INCLUDE) \
			
NASM_FLAGS	:= -f elf \
			-I $(DIR_INCLUDE) \
			-I $(XBOOKLIB_INCLUDE) \

# drivers
DRIVERS_OBJS 	= $(DIR_CODE)/screen.o \
				$(DIR_CODE)/keyboard.o \
				$(DIR_CODE)/mtjt.o \
				$(DIR_CODE)/lock.o \
				$(DIR_CODE)/driver.o \

# color an palette
COLOR_PALETTE_OBJS 	= $(DIR_CODE)/color_match.o \
					$(DIR_CODE)/palette.o \
					$(DIR_CODE)/palette_conversion.o \

# dc
DC_OBJS 		= $(DIR_CODE)/pen.o \
				$(DIR_CODE)/brush.o \
				$(DIR_CODE)/dc.o \
				$(DIR_CODE)/default.o \

# counter and timer			
COUNTER_TIMER_OBJS 	= $(DIR_CODE)/counter.o \
					$(DIR_CODE)/timer.o \
					
# message			
MESSAGE_OBJS 	= $(DIR_CODE)/message.o \

# 2d
D2_OBJS 			= $(DIR_CODE)/d2_pixel.o \
					$(DIR_CODE)/d2_line.o \
					$(DIR_CODE)/d2_rect.o \
					$(DIR_CODE)/d2_triangle.o \
					$(DIR_CODE)/d2_circle.o \
					$(DIR_CODE)/d2_ellipse.o \
					
			
# font and text
FONT_TEXT_OBJS 		= $(DIR_CODE)/font_ops.o \
					$(DIR_CODE)/text_ops.o \
					$(DIR_CODE)/text_glyph.o \


# charsets
CHARSETS_OBJS 		= $(DIR_CODE)/ascii_latin_dot_0612.o \
					$(DIR_CODE)/ascii_latin_dot_0816.o \
					$(DIR_CODE)/gb2312_dot_1212_song.o \
					$(DIR_CODE)/gb2312_dot_1616_song.o \
					$(DIR_CODE)/uni_cjk_unified_dot_1212_song.o \
					$(DIR_CODE)/hz1212_punctuation.o \
					$(DIR_CODE)/uni_cjk_unified_dot_1616_song.o \
					$(DIR_CODE)/hz1616_punctuation.o \

# tools
TOOLS_OBJS 		= $(DIR_CODE)/code_tools.o \

# image
IMAGE_OBJS 			= $(DIR_CODE)/image_comm.o \
					$(DIR_CODE)/image_bitmap.o \
					$(DIR_CODE)/image_icon.o \
					$(DIR_CODE)/image_gif.o \
					$(DIR_CODE)/snapshot.o \
					

# rect ops tools
RECT_OPS_OBJS 		= $(DIR_CODE)/rect_ops.o \

# cursor
CURSOR_OBJS 		= $(DIR_CODE)/cursor.o \

# file
FILE_OPS_OBJS 		= $(DIR_CODE)/file_ops.o \

# gui
GUI_OBJS 		= $(DIR_CODE)/gui.o \


#windows version
WINDOWS_VERSION_OBJS= $(DIR_CODE)/win_tools.o \
					$(DIR_CODE)/win_dc.o \
					$(DIR_CODE)/win_default.o \
					$(DIR_CODE)/win_caret.o \
					$(DIR_CODE)/win_clip.o \
					$(DIR_CODE)/win_bar.o \
					$(DIR_CODE)/win_scbar.o \
					$(DIR_CODE)/win_desktop.o \
					$(DIR_CODE)/win_arithmetic.o \
					$(DIR_CODE)/win_callback.o \
					$(DIR_CODE)/win_invalidate.o \
					$(DIR_CODE)/win_interface.o \
					$(DIR_CODE)/win_init.o \
					$(DIR_CODE)/win_widget.o \

# widgets
WIDGETS_OBJS		= $(DIR_CODE)/win_widget_group.o \
					$(DIR_CODE)/win_group_box.o \
					$(DIR_CODE)/win_cell.o \
					$(DIR_CODE)/win_label.o \
					$(DIR_CODE)/win_frame.o \
					$(DIR_CODE)/win_push_button.o \
					$(DIR_CODE)/win_radio_button.o \
					$(DIR_CODE)/win_check_box.o \
					$(DIR_CODE)/win_line_edit.o \
					$(DIR_CODE)/win_list_box.o \
					$(DIR_CODE)/win_com_box.o \
					$(DIR_CODE)/win_progress_bar.o \
					$(DIR_CODE)/win_slider_bar.o \
					$(DIR_CODE)/win_image.o \

# objects
OBJS	:=  $(DRIVERS_OBJS) \
			$(COLOR_PALETTE_OBJS) \
			$(DC_OBJS) \
			$(COUNTER_TIMER_OBJS) \
			$(MESSAGE_OBJS) \
			$(D2_OBJS) \
			$(TOOLS_OBJS) \
			$(CHARSETS_OBJS) \
			$(FONT_TEXT_OBJS) \
			$(IMAGE_OBJS) \
			$(RECT_OPS_OBJS) \
			$(CURSOR_OBJS) \
			$(FILE_OPS_OBJS) \
			$(GUI_OBJS) \
			$(WINDOWS_VERSION_OBJS) \
			$(WIDGETS_OBJS) \

# out file
LIBNAME = learninggui-0.5.0.a
	
# all need to do
all: $(LIBNAME) 

# make a libary file
$(LIBNAME): $(OBJS) 
	@echo [AR] making libary file $(LIBNAME)
	@$(AR) $(ARFLAGS) $(LIBNAME) $(OBJS)

%.o : %.c
	@echo [CC] $@ $<
	@$(CC) $(C_FLAGS) -o $@ $<

%.o : %.asm
	@echo [NASM] $@ $<
	@$(NASM) $(NASM_FLAGS) -o $@ $<

# clean .o .a files
clean: 
	-$(RM) $(LIBNAME)
	-$(RM) $(OBJS)
	