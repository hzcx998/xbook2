
include  common.mak


INCLUDE = -I../include



# drivers
OBJS  = screen.o  keyboard.o  mtjt.o  lock.o  driver.o


# color an palette
OBJS += color_match.o  palette.o  palette_conversion.o


# dc
OBJS  += pen.o  brush.o  dc.o  default.o


# counter and timer
OBJS  += counter.o  timer.o


# message
OBJS  += message.o


# bank
OBJS  += gui_bank.o


# 2D
OBJS  += d2_pixel.o  d2_line.o  d2_rect.o  d2_triangle.o  d2_circle.o  d2_ellipse.o


# font and text
OBJS  += font_ops.o text_ops.o  text_glyph.o


# charsets
OBJS  += ascii_latin_dot_0612.o  ascii_latin_dot_0816.o \
    gb2312_dot_1212_song.o  gb2312_dot_1616_song.o 


# tools
OBJS  += code_tools.o


# image
OBJS  += image_comm.o image_bitmap.o  lgui_bitmap_rotate.o  lgui_bitmap_transform.o \
	 image_icon.o image_gif.o  snapshot.o


# rect ops tools
OBJS  += rect_ops.o


# cursor
OBJS  += cursor.o


# file
OBJS  += file_ops.o


# gui
OBJS  += gui.o


#windows version
OBJS  += win_tools.o  win_dc.o  win_default.o  win_caret.o  win_clip.o  win_bar.o  win_scbar.o  win_desktop.o  \
    win_arithmetic.o  win_callback.o  win_invalidate.o  win_interface.o  win_init.o  win_widget.o  

# widgets
OBJS  +=  win_widget_group.o \
    win_group_box.o  win_cell.o  win_label.o  win_frame.o  win_push_button.o  win_radio_button.o \
    win_check_box.o  win_line_edit.o  win_list_box.o  win_com_box.o  win_progress_bar.o  win_slider_bar.o  win_image.o



.PHONY: all


all: SO_LINK  A_LINK


SO_LINK: $(OBJS)
	$(LD)  $(SO_LDFLAGS)  $(OBJS)  -o $(SO_TARGET)
	ln -f -s $(SO_TARGET)  $(SO_TARGET_S)

A_LINK: $(OBJS)
	$(AR)  $(A_LDFLAGS)  $(A_TARGET)  $(OBJS)
	$(RANLIB)  $(A_TARGET)
	ln -f -s $(A_TARGET)  $(A_TARGET_S)

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

install:
	cp -f -a $(SO_TARGET)       /usr/lib/
	cp -f -a $(SO_TARGET_S)     /usr/lib/



clean: CLEAN_TOP


CLEAN_TOP:
	rm -f *.o
	rm -f $(SO_TARGET_S)
	rm -f $(SO_TARGET)
	rm -f $(A_TARGET)
	rm -f $(A_TARGET_S)

