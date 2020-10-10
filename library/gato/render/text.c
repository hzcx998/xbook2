#include "surface.h"
#include "render.h"
#include "svg.h"
#include "consolas-font.h"
#include <string.h>


void draw_text(surface_t *base, int x, int y, char *c, float size, color_t color)
{
	context_t *ctx = &(context_t){0};
	context_init(ctx, base);

	float scale = size * 16 / (2048);

	svg_style_t style = (svg_style_t){
		.scale = scale,
		.translate_x = x + 0 * scale,
		.translate_y = y + 1521 * scale,
		.mirror_y = 1,
	};

	for (int i = 0; i < strlen(c); i++, style.translate_x += 1126 * scale)
		svg_to(ctx, consolas_font[c[i]], style);

	ctx->render(ctx, x, y, (style_t){
		fill_color : color
	});
	context_exit(ctx);
}