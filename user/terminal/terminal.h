#ifndef __TERMINAL_APP_H__
#define __TERMINAL_APP_H__

#include <sgi/sgi.h>
#include <stddef.h>

#define APP_NAME        "terminal"
#define APP_VERSION     0x01

/*
/// 终端显示 ///
光标: 大中小，形状，颜色。
命令记录：缓冲区
编辑选项: 复制粘贴文本
字体编码：代码页。ANSI,utf-8
字体：大小，类型，字符宽高
布局：屏幕缓冲区大小（可以存放多少字），窗口大小（窗口内显示的字符数），窗口位置。
颜色：屏幕文字，屏幕背景

/// 内部逻辑 ///
父子间通信：接收子进程发送来的数据，以及传输给子进程的数据。
重定向问题 > < >> << >>> <<<
管道机制 | 
*/

#endif  /* __TERMINAL_APP_H__ */