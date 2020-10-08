# 渲染器使用指南
* **[总览](#总览)**
* **[开始](#getting-started)**

## 总览
渲染器的结构如下:
```
call `mu_render_init`
call `mu_render_set_...` functions 
call `mu_render_loop()` 
```

## 开始
在使用一个 `mu_Context` 之前，需要做初始化:
```c
mu_Context *ctx = mu_render_init("title", win_x_pos, win_y_pos, win_width, win_heiht);
```
并且需要保证初始化成功，如果初始化不成功，则返回值是NULL:
```c
if (ctx == NULL) 
    exit(-1);
```
在完成初始化之后，就需要对渲染器进行一定的设置，然后再渲染去循环种会自动调用设置的属性或者是回调函数。  
首先需要设置一个帧处理过程，也就是要把哪些内容放到渲染器里面，于是就在帧处理过程种添加对应的代码即可：
```c
void process_frame(mu_Context *ctx)
{
    mu_begin(ctx);
    ... add your mui code here...
	mu_end(ctx);
}

mu_render_set_frame(process_frame);
```
接下来就需要调用渲染器主循环，调用后，渲染器就可以把`process_frame`函数里面摆放的内容绘制到窗口上。
```c
mu_render_loop(ctx);
```
如果你还想要再完成对frame的渲染后，再进行一些代码的处理，就需要设置`handler`处理函数，在里面可以添加一些你自己的代码。可以通过调用`mu_render_set_handler`来进行设置：
```c
void process_handler(mu_Context *ctx)
{
    
    ... add your code here...
	printf("hello, world!\n");
}

mu_render_set_handler(process_handler);
```
除此之外，你还可以设置每次渲染时的背景颜色，只要那个位置没有mui的内容，就都是你设置的背景颜色。默认的背景颜色时黑色。可以调用`mu_render_set_bgcolor`来进行设置：

```c
mu_render_set_bgcolor(mu_color(192, 192, 192, 255));
```

一个简单的例子：
```c
#include <mui_renderer.h>

static void process_frame(mu_Context *ctx)
{
	mu_begin(ctx);
	
	mu_end(ctx);
}


static void process_handler(mu_Context *ctx)
{
	printf("hello, world!\n");
}

int main()
{
    // 窗口标题"mui-example"， 位置 100， 100，宽度 800， 高度 600
    mu_Context *ctx = mu_render_init("mui-example", 100, 100, 800, 600);
	if (ctx == NULL)
        return -1;

    // 设置帧处理函数
    mu_render_set_frame(process_frame);
    // 设置普通代码处理函数
    mu_render_set_handler(process_handler);
    // 设置渲染时的背景颜色
    mu_render_set_bgcolor(mu_color(192, 192, 192, 255));
    // 调用主循环
    mu_render_loop(ctx);
	return 0;
}
```
