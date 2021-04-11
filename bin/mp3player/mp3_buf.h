#ifndef __MP3_BUF_H__
#define __MP3_BUF_H__

#define OUTPUT_BUFFER_SIZE 2304 //一帧有1152个点

unsigned short *get_framebuf();
void submit_framebuf();
void open_sound();
void close_sound();

#endif
