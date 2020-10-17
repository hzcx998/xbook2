#include <stdio.h>
#include <gapi.h>
#include <string.h>
#include <sys/time.h>
#include <mui_renderer.h>

static char logbuf[64000];
static int logbuf_updated = 0;
static float bg[3] = {90, 95, 100};

static void write_log(const char *text)
{
	if (logbuf[0])
	{
		strcat(logbuf, "\n");
	}
	strcat(logbuf, text);
	logbuf_updated = 1;
}

static void test_window(mu_Context *ctx)
{
	static mu_Container window;

	/* init window manually so we can set its position and size */
	if (!window.inited)
	{
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(40, 40, 300, 450);
	}

	/* limit window to minimum size */
	window.rect.w = mu_max(window.rect.w, 240);
	window.rect.h = mu_max(window.rect.h, 300);

	/* do window */
	if (mu_begin_window(ctx, &window, "Demo Window"))
	{

		/* window info */
		static int show_info = 1;
		if (mu_header(ctx, &show_info, "Window Info"))
		{
			char buf[64];
			mu_layout_row(ctx, 2, (int[]){54, -1}, 0);
			mu_label(ctx, "Position:");
			sprintf(buf, "%d, %d", ctx->mouse_pos.x, ctx->mouse_pos.y);
			mu_label(ctx, buf);
			mu_label(ctx, "Size:");
			sprintf(buf, "%d, %d", window.rect.w, window.rect.h);
			mu_label(ctx, buf);
		}

		/* labels + buttons */
		static int show_buttons = 1;
		if (mu_header(ctx, &show_buttons, "Test Buttons"))
		{
			mu_layout_row(ctx, 3, (int[]){86, -110, -1}, 0);
			mu_label(ctx, "Test buttons 1:");
			if (mu_button(ctx, "Button 1"))
			{
				write_log("Pressed button 1");
			}
			if (mu_button(ctx, "Button 2"))
			{
				write_log("Pressed button 2");
			}
			mu_label(ctx, "Test buttons 2:");
			if (mu_button(ctx, "Button 3"))
			{
				write_log("Pressed button 3");
			}
			if (mu_button(ctx, "Button 4"))
			{
				write_log("Pressed button 4");
			}
		}

		/* tree */
		static int show_tree = 1;
		if (mu_header(ctx, &show_tree, "Tree and Text"))
		{
			mu_layout_row(ctx, 2, (int[]){140, -1}, 0);
			mu_layout_begin_column(ctx);
			static int states[8];
			if (mu_begin_treenode(ctx, &states[0], "Test 1"))
			{
				if (mu_begin_treenode(ctx, &states[1], "Test 1a"))
				{
					mu_label(ctx, "Hello");
					mu_label(ctx, "world");
					mu_end_treenode(ctx);
				}
				if (mu_begin_treenode(ctx, &states[2], "Test 1b"))
				{
					if (mu_button(ctx, "Button 1"))
					{
						write_log("Pressed button 1");
					}
					if (mu_button(ctx, "Button 2"))
					{
						write_log("Pressed button 2");
					}
					mu_end_treenode(ctx);
				}
				mu_end_treenode(ctx);
			}
			if (mu_begin_treenode(ctx, &states[3], "Test 2"))
			{
				mu_layout_row(ctx, 2, (int[]){54, 54}, 0);
				if (mu_button(ctx, "Button 3"))
				{
					write_log("Pressed button 3");
				}
				if (mu_button(ctx, "Button 4"))
				{
					write_log("Pressed button 4");
				}
				if (mu_button(ctx, "Button 5"))
				{
					write_log("Pressed button 5");
				}
				if (mu_button(ctx, "Button 6"))
				{
					write_log("Pressed button 6");
				}
				mu_end_treenode(ctx);
			}
			if (mu_begin_treenode(ctx, &states[4], "Test 3"))
			{
				static int checks[3] = {1, 0, 1};
				mu_checkbox(ctx, &checks[0], "Checkbox 1");
				mu_checkbox(ctx, &checks[1], "Checkbox 2");
				mu_checkbox(ctx, &checks[2], "Checkbox 3");
				mu_end_treenode(ctx);
			}
			mu_layout_end_column(ctx);

			mu_layout_begin_column(ctx);
			mu_layout_row(ctx, 1, (int[]){-1}, 0);
			mu_text(ctx, "Lorem ipsum dolor sit amet, consectetur adipiscing "
						 "elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus "
						 "ipsum, eu varius magna felis a nulla.");
			mu_layout_end_column(ctx);
		}

		/* background color sliders */
		static int show_sliders = 1;
		if (mu_header(ctx, &show_sliders, "Background Color"))
		{
			mu_layout_row(ctx, 2, (int[]){-78, -1}, 74);
			/* sliders */
			mu_layout_begin_column(ctx);
			mu_layout_row(ctx, 2, (int[]){46, -1}, 0);
			mu_label(ctx, "Red:");
			mu_slider(ctx, &bg[0], 0, 255);
			mu_label(ctx, "Green:");
			mu_slider(ctx, &bg[1], 0, 255);
			mu_label(ctx, "Blue:");
			mu_slider(ctx, &bg[2], 0, 255);
			mu_layout_end_column(ctx);
			/* color preview */
			mu_Rect r = mu_layout_next(ctx);
			mu_draw_rect(ctx, r, mu_color(bg[0], bg[1], bg[2], 255));
			char buf[32];
			sprintf(buf, "#%02X%02X%02X", (int)bg[0], (int)bg[1], (int)bg[2]);
			mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
		}

		mu_end_window(ctx);
	}
}

static void log_window(mu_Context *ctx)
{
	static mu_Container window;

	/* init window manually so we can set its position and size */
	if (!window.inited)
	{
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(350, 40, 300, 200);
	}

	if (mu_begin_window(ctx, &window, "Log Window"))
	{

		/* output text panel */
		static mu_Container panel;
		mu_layout_row(ctx, 1, (int[]){-1}, -28);
		mu_begin_panel(ctx, &panel);
		mu_layout_row(ctx, 1, (int[]){-1}, -1);
		mu_text(ctx, logbuf);
		mu_end_panel(ctx);
		if (logbuf_updated)
		{
			panel.scroll.y = panel.content_size.y;
			logbuf_updated = 0;
		}

		/* input textbox + submit button */
		static char buf[128];
		int submitted = 0;
		mu_layout_row(ctx, 2, (int[]){-70, -1}, 0);
		if (mu_textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT)
		{
			mu_set_focus(ctx, ctx->last_id);
			submitted = 1;
		}
		if (mu_button(ctx, "Submit"))
		{
			submitted = 1;
		}
		if (submitted)
		{
			write_log(buf);
			buf[0] = '\0';
		}

		mu_end_window(ctx);
	}
}

void write_result(char *buf, const char *text)
{
	strcat(buf, text);
}

// mu_Color pic[272][480];

// void PutPixel(uint16_t x, uint16_t y, uint32_t color)
// {
// 	pic[y][x] = *(mu_Color *)&color;
// }

// uint32_t GetPixel(uint16_t x, uint16_t y)
// {
// 	return *(uint32_t *)&pic[y][x];
// }

// static int mu_console(mu_Context *ctx, int *value)
// {
// 	mu_Id id = mu_get_id(ctx, &value, sizeof(value));
// 	mu_Rect r = mu_layout_next(ctx);
// 	mu_update_control(ctx, id, r, MU_OPT_HOLDFOCUS);
// 	/* draw */
// 	static char buf[32];
// 	/* handle input */
// 	int res = 0;
// 	if (ctx->focus == id)
// 	{
// 		(*value)++;
// 		res |= MU_RES_CHANGE;
// 		int posx = ctx->mouse_pos.x - r.x;
// 		int posy = ctx->mouse_pos.y - r.y;
// 		sprintf(buf, "%d %d\n", posx, posy);
// 		vt100_puts(ctx->text_input);
// 	}

// 	mu_Rect rect = {r.x, r.y, 480, 272};
// 	mu_draw_custom(ctx, rect, pic);
// 	return res;
// }

// static void test_window(mu_Context *ctx)
// {
// 	static mu_Container window;

// 	/* init window manually so we can set its position and size */
// 	if (!window.inited)
// 	{
// 		mu_init_window(ctx, &window, 0);
// 		window.rect = mu_rect(0, 0, 510, 320);
// 	}

// 	if (mu_begin_window(ctx, &window, "Console"))
// 	{
// 		static char buf[128];

// 		mu_layout_row(ctx, 1, (int[]){480}, 272);
// 		static int v = 0;
// 		mu_console(ctx, &v);
// 		mu_end_window(ctx);
// 	}
// }

static int uint8_slider(mu_Context *ctx, unsigned char *value, int low, int high)
{
	static float tmp;
	mu_push_id(ctx, &value, sizeof(value));
	tmp = *value;
	int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
	*value = tmp;
	mu_pop_id(ctx);
	return res;
}

static void style_window(mu_Context *ctx)
{
	static mu_Container window;

	/* init window manually so we can set its position and size */
	if (!window.inited)
	{
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(350, 250, 300, 240);
	}

	static struct
	{
		const char *label;
		int idx;
	} colors[] = {
		{"text:", MU_COLOR_TEXT},
		{"border:", MU_COLOR_BORDER},
		{"windowbg:", MU_COLOR_WINDOWBG},
		{"titlebg:", MU_COLOR_TITLEBG},
		{"titletext:", MU_COLOR_TITLETEXT},
		{"panelbg:", MU_COLOR_PANELBG},
		{"button:", MU_COLOR_BUTTON},
		{"buttonhover:", MU_COLOR_BUTTONHOVER},
		{"buttonfocus:", MU_COLOR_BUTTONFOCUS},
		{"base:", MU_COLOR_BASE},
		{"basehover:", MU_COLOR_BASEHOVER},
		{"basefocus:", MU_COLOR_BASEFOCUS},
		{"scrollbase:", MU_COLOR_SCROLLBASE},
		{"scrollthumb:", MU_COLOR_SCROLLTHUMB},
		{NULL}};

	if (mu_begin_window(ctx, &window, "Style Editor"))
	{
		int sw = mu_get_container(ctx)->body.w * 0.14;
		mu_layout_row(ctx, 6, (int[]){80, sw, sw, sw, sw, -1}, 0);
		for (int i = 0; colors[i].label; i++)
		{
			mu_label(ctx, colors[i].label);
			uint8_slider(ctx, &ctx->style->colors[i].r, 0, 255);
			uint8_slider(ctx, &ctx->style->colors[i].g, 0, 255);
			uint8_slider(ctx, &ctx->style->colors[i].b, 0, 255);
			uint8_slider(ctx, &ctx->style->colors[i].a, 0, 255);
			mu_draw_rect(ctx, mu_layout_next(ctx), ctx->style->colors[i]);
		}
		mu_end_window(ctx);
	}
}

static void process_frame(mu_Context *ctx)
{
	mu_begin(ctx);
	test_window(ctx);
	log_window(ctx);
	style_window(ctx);
	mu_end(ctx);
}

char text_buf[20];
//uint64_t now, before = ktime_to_us(ktime_get());
struct timeval time1 = {0, 0};
struct timeval time2 = {0, 0};
int fps = 0;

static void process_handler(mu_Context *ctx)
{
	mu_Rect rect = {
        .x = ctx->mouse_pos.x - 8,
        .y = ctx->mouse_pos.y - 8,
        .w = 16,
        .h = 16,
    };
    mu_Vec2 pos = {0, 0};
    mu_Color color = {255, 255, 255, 255};
    fps++;
    gettimeofday(&time2, NULL);
    unsigned long long mtime = (time2.tv_sec - time1.tv_sec) * 1000000 + (time2.tv_usec - time1.tv_usec);
    if (mtime > 1000000) {
        printf("fps %d\n", fps);
        sprintf(text_buf, "%d", fps);
        
        fps = 0;
        time1 = time2;
    }
    mu_render_draw_text(text_buf, pos, color);
    mu_render_draw_icon(MU_ICON_CLOSE, rect, color);
}


int main()
{
    mu_Context *ctx = mu_render_init("microui", 100, 100, 800, 600);
	if (ctx == NULL)
        return -1;

    mu_render_set_frame(process_frame);
    mu_render_set_handler(process_handler);
    mu_render_set_bgcolor(mu_color(192, 192, 192, 255));
    gettimeofday(&time1, NULL);
    mu_render_loop(ctx);

	return 0;
}
