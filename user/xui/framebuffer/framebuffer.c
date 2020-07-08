/*
 * driver/framebuffer/framebuffer.c
 *
 * Copyright(c) 2007-2020 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <xboot.h>
#include <framebuffer/framebuffer.h>
#include <drivers/fb_sandbox.h>

extern struct framebuffer_t * fb_sandbox;

struct framebuffer_t * search_framebuffer(const char * name)
{
	return (struct framebuffer_t *)fb_sandbox;
}

struct framebuffer_t * search_first_framebuffer(void)
{
	return (struct framebuffer_t *)fb_sandbox;
}

void framebuffer_set_backlight(struct framebuffer_t * fb, int brightness)
{
	if(fb && fb->setbl)
	{
		if(brightness < 0)
			brightness = 0;
		else if(brightness > CONFIG_MAX_BRIGHTNESS)
			brightness = CONFIG_MAX_BRIGHTNESS;
		fb->setbl(fb, brightness);
	}
}

int framebuffer_get_backlight(struct framebuffer_t * fb)
{
	if(fb && fb->getbl)
		return fb->getbl(fb);
	return 0;
}
