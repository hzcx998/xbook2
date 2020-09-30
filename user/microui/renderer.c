#include <assert.h>
#include <gapi.h>
#include <unistd.h>
#include <stdio.h>

#include "renderer.h"
#include "atlas.inl"

#define BUFFER_SIZE 16384

static mu_Rect tex_buf[BUFFER_SIZE];
static mu_Rect vert_buf[BUFFER_SIZE];
static mu_Color color_buf[BUFFER_SIZE];
static mu_Rect clip_rect;

int fb;
static g_bitmap_t * render;
static int width;
static int height;
static int buf_idx;

#define WIN_WIDTH   640
#define WIN_HEIGHT   480

void r_init(void) {
    /* 初始化 */
    if (g_init() < 0) {
        printf("g_init failed!\n");
        exit(-1);
    }
    width = WIN_WIDTH;
	height = WIN_HEIGHT;

    fb = g_new_window("microui", 100, 100, width, height, GW_SHOW);
    if (fb < 0){
        printf("g_new_window failed!\n");
        exit(-1);
    }

	render = g_new_bitmap(width, height);
	
	clip_rect = (mu_Rect) { 0, 0, width, height};
}

#define BIT_GET(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] & (1<<((p) % (sizeof(char) * 8))))

#if 0
static u32_t neon(u32_t src, u32_t color, u32_t factor)
{
	u32_t ret;
	asm (
		"vmov d0,%1,%2\n\t"
		"vmovl.u8 q0,d0\n\t"
		"vsub.u16 d1,d1,d0\n\t"
		"vmov d2,%3,%3\n\t"
		"vmov.u16 d2[1],%3\n\t"
		"vmov.u16 d2[3],%3\n\t"
		"vmull.u16 q1,d2,d1\n\t"
		"vshrn.u32 d2,q1,#16\n\t"
		"vadd.u16 d0,d2,d0\n\t"
		"vmovn.u16 d0,q0\n\t"
		"vmov.u32 %0,d0[0]\n\t"
		: "+r" (ret) 
		: "r" (src), "r" (color), "r" (factor)
		: "memory", "d0", "d1", "d2", "d3"
	);
	return ret;
}
#endif

static void disp_fill(mu_Rect dst, mu_Rect src, mu_Color color)
{
    int32_t x;
    int32_t y;
	float factor_x = ((float)src.w) / (dst.w);
	float factor_y = ((float)src.h) / (dst.h);

	int sx, sy, ex, ey;
	if(dst.x < clip_rect.x) sx = clip_rect.x;
	else if(dst.x >= clip_rect.x + clip_rect.w) return; else sx = dst.x;
	if(dst.y < clip_rect.y) sy = clip_rect.y;
	else if(dst.y >= clip_rect.y + clip_rect.h) return; else sy = dst.y;
	if(dst.x + dst.w > clip_rect.x + clip_rect.w) ex = clip_rect.x + clip_rect.w;
	else if(dst.x + dst.w <= clip_rect.x) return; else ex = dst.x + dst.w;
	if(dst.y + dst.h > clip_rect.y + clip_rect.h) ey = clip_rect.y + clip_rect.h;
	else if(dst.y + dst.h <= clip_rect.y) return; else ey = dst.y + dst.h;

	if(color.a == 0xff) {
		for(y = sy; y < ey; y++) {
			mu_Color *base = (mu_Color *)&((u32_t *)render->buffer)[y * width];
			for(x = sx; x < ex; x++) base[x] = color;
		}
	} else {
	    for(y = sy; y < ey; y++) {
			mu_Color *base = (mu_Color *)&((u32_t *)render->buffer)[y * width];
			for(x = sx; x < ex; x++) {
				mu_Color *c = &base[x];
				int a = color.a;
				c->r = ((int) (a * (color.r - c->r)) >> 8) + c->r;
				c->g = ((int) (a * (color.g - c->g)) >> 8) + c->g;
				c->b = ((int) (a * (color.b - c->b)) >> 8) + c->b;
			}
		}
	}
}

static void disp_texture(mu_Rect dst, mu_Rect src, mu_Color color)
{
    int32_t x;
    int32_t y;
	float factor_x = ((float)src.w) / (dst.w);
	float factor_y = ((float)src.h) / (dst.h);

	int sx, sy, ex, ey;
	if(dst.x < clip_rect.x) sx = clip_rect.x;
	else if(dst.x >= clip_rect.x + clip_rect.w) return; else sx = dst.x;
	if(dst.y < clip_rect.y) sy = clip_rect.y;
	else if(dst.y >= clip_rect.y + clip_rect.h) return; else sy = dst.y;
	if(dst.x + dst.w > clip_rect.x + clip_rect.w) ex = clip_rect.x + clip_rect.w;
	else if(dst.x + dst.w <= clip_rect.x) return; else ex = dst.x + dst.w;
	if(dst.y + dst.h > clip_rect.y + clip_rect.h) ey = clip_rect.y + clip_rect.h;
	else if(dst.y + dst.h <= clip_rect.y) return; else ey = dst.y + dst.h;

    for(y = sy; y < ey; y++) {
		mu_Color *base = (mu_Color *)&((u32_t *)render->buffer)[y * width];
        for(x = sx; x < ex; x++) {
			mu_Color *c = &base[x];
			int src_x = src.x + ((int)((x - dst.x) * factor_x));
			int src_y = src.y + ((int)((y - dst.y) * factor_y));
			int a = color.a * atlas_texture[src_y * ATLAS_HEIGHT + src_x];
#if 1
			c->r = ((int) (a * (color.r - c->r)) >> 16) + c->r;
			c->g = ((int) (a * (color.g - c->g)) >> 16) + c->g;
			c->b = ((int) (a * (color.b - c->b)) >> 16) + c->b;
#else
			*(u32_t *)c = neon(*(u32_t *)c, *(u32_t *)&color, a);
#endif
        }
    }
}

static void disp_custom(mu_Rect dst, mu_Color *color)
{
    int32_t x;
    int32_t y;
	int sx, sy, ex, ey;
	if(dst.x < clip_rect.x) sx = clip_rect.x;
	else if(dst.x >= clip_rect.x + clip_rect.w) return; else sx = dst.x;
	if(dst.y < clip_rect.y) sy = clip_rect.y;
	else if(dst.y >= clip_rect.y + clip_rect.h) return; else sy = dst.y;
	if(dst.x + dst.w > clip_rect.x + clip_rect.w) ex = clip_rect.x + clip_rect.w;
	else if(dst.x + dst.w <= clip_rect.x) return; else ex = dst.x + dst.w;
	if(dst.y + dst.h > clip_rect.y + clip_rect.h) ey = clip_rect.y + clip_rect.h;
	else if(dst.y + dst.h <= clip_rect.y) return; else ey = dst.y + dst.h;

	for(y = sy; y < ey; y++) {
		mu_Color *base = (mu_Color *)&((u32_t *)render->buffer)[y * width];
		for(x = sx; x < ex; x++) {
			mu_Color *c = &base[x];
			int src_x = x - dst.x;
			int src_y = y - dst.y;
			mu_Color *cc = &color[src_y * dst.w + src_x];
			int a = cc->a;
			c->r = ((int) (a * (cc->r - c->r)) >> 8) + c->r;
			c->g = ((int) (a * (cc->g - c->g)) >> 8) + c->g;
			c->b = ((int) (a * (cc->b - c->b)) >> 8) + c->b;
		}
	}
}

static void disp_clear(mu_Color color)
{
    int32_t x;
    int32_t y;
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
			((u32_t *)render->buffer)[y * width + x] = *(int *)&color;
        }
    }
}

static void flush(void) {
	if (buf_idx == 0) { return; }

	for(int i = 0; i < buf_idx; i++){
		if(tex_buf[i].x == 125)
			disp_fill(vert_buf[i], tex_buf[i], color_buf[i]);
		else
			disp_texture(vert_buf[i], tex_buf[i], color_buf[i]);
	}
	buf_idx = 0;
}


static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
	if (buf_idx == BUFFER_SIZE) { flush(); }
	/* update texture buffer */
	tex_buf[buf_idx] = src;
	/* update vertex buffer */
	vert_buf[buf_idx] = dst;
	/* update color buffer */
	color_buf[buf_idx] = color;
	buf_idx++;
}


void r_draw_rect(mu_Rect rect, mu_Color color) {
	push_quad(rect, atlas[ATLAS_WHITE], color);
}


void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
	mu_Rect dst = { pos.x, pos.y, 0, 0 };
	for (const char *p = text; *p; p++) {
		if ((*p & 0xc0) == 0x80) { continue; }
		int chr = mu_min((unsigned char) *p, 127);
		mu_Rect src = atlas[ATLAS_FONT + chr];
		dst.w = src.w;
		dst.h = src.h;
		push_quad(dst, src, color);
		dst.x += dst.w;
	}
}


void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
	mu_Rect src = atlas[id];
	int x = rect.x + (rect.w - src.w) / 2;
	int y = rect.y + (rect.h - src.h) / 2;
	push_quad(mu_rect(x, y, src.w, src.h), src, color);
}

void r_draw_custom(mu_Rect rect, mu_Color *color) {
	flush();
	disp_custom(rect, color);
}

int r_get_text_width(const char *text, int len) {
	int res = 0;
	for (const char *p = text; *p && len--; p++) {
		if ((*p & 0xc0) == 0x80) { continue; }
		int chr = mu_min((unsigned char) *p, 127);
		res += atlas[ATLAS_FONT + chr].w;
	}
	return res;
}


int r_get_text_height(void) {
	return 18;
}


void r_set_clip_rect(mu_Rect rect) {
	flush();

	if(rect.x < 0) rect.x = 0;
	if(rect.y < 0) rect.y = 0;
	if(rect.x + rect.w > width) rect.w = width - rect.x;
	if(rect.y + rect.h > height) rect.h = height - rect.y;
	clip_rect = rect; 	// 可以注释这条语句，检查是否有组件进行了错误的裁剪
}


void r_clear(mu_Color clr) {
	flush();
	disp_clear(clr);
}

void r_present(void) {
	flush();
	// framebuffer_present_render(fb, render, NULL, 0);
    g_paint_window(fb, 0, 0, render);
}
