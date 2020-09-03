#include <math.h>	// fmaxf(), sinf(), cosf()
#include <stdlib.h> // abs()
#include <string.h> // memset()
#include <time.h>
#include <assert.h>
#include "color.h"
#include "svg.h"
#include "render.h"

#include "surface.h"
#include "consolas-font.h"
#include "image.h"

#define M_PI 3.14159265358979323846
#define PI 3.14159265359f
#define KAPPA90 (0.5522847493f)
#define fclampf(v, a, b) fminf(fmaxf(a, v), b)

#if 0
#define min(a, b) ({typeof(a) _amin = (a); typeof(b) _bmin = (b); (void)(&_amin == &_bmin); _amin < _bmin ? _amin : _bmin; })
#define max(a, b) ({typeof(a) _amax = (a); typeof(b) _bmax = (b); (void)(&_amax == &_bmax); _amax > _bmax ? _amax : _bmax; })
#endif

point_t constant_point_add_point(point_t p1, point_t p2, float ratio)
{
	return (point_t){(p1.x + p2.x) * ratio, (p1.y + p2.y) * ratio};
}

point_t point_sub_point(point_t p1, point_t p2)
{
	return (point_t){(p1.x - p2.x), (p1.y - p2.y)};
}

point_t point_add_point(point_t p1, point_t p2)
{
	return (point_t){(p1.x + p2.x), (p1.y + p2.y)};
}

point_t point_min(point_t min, point_t p)
{
	return (point_t){fminf(min.x, p.x), fminf(min.y, p.y)};
}

point_t point_max(point_t min, point_t p)
{
	return (point_t){fmaxf(min.x, p.x), fmaxf(min.y, p.y)};
}

point_t unit_point(point_t p1)
{
	float l = sqrtf(p1.x * p1.x + p1.y * p1.y);
	return (point_t){(p1.x / l), (p1.y / l)};
}

float point_cross_point(point_t p1, point_t p2)
{
	return p1.x * p2.y - p1.y * p2.x;
}

float point_dot_point(point_t p1, point_t p2)
{
	return p1.x * p2.x + p1.y * p2.y;
}

point_t point_mul_factor(point_t p1, float factor)
{
	return (point_t){(p1.x * factor), (p1.y * factor)};
}

void set_color(surface_t *s, int x, int y, float alpha, color_t color)
{
	if (x >= 0 && x < s->width && y >= 0 && y < s->height)
	{
		color.a = alpha * color.a;
		color_t *c = s->pixels + y * s->width + x;
		blend(c, &color);
	}
}

void alphablend(surface_t *s, int x, int y, float alpha, color_t color)
{
	if (x >= 0 && x < s->width && y >= 0 && y < s->height)
	{
		alpha = alpha * color.a / 255.0;
		color_t *c = s->pixels + y * s->width + x;
		*c = (color_t){c->b * (1 - alpha) + color.b * alpha, c->g * (1 - alpha) + color.g * alpha, c->r * (1 - alpha) + color.r * alpha, 255};
	}
}

float capsuleSDF(point_t p, point_t a, point_t b, float r)
{
	point_t pa = point_sub_point(p, a);
	point_t ba = point_sub_point(b, a);
	float h = fmaxf(fminf((pa.x * ba.x + pa.y * ba.y) / (ba.x * ba.x + ba.y * ba.y), 1.0f), 0.0f);
	float dx = pa.x - ba.x * h, dy = pa.y - ba.y * h;
	return sqrtf(dx * dx + dy * dy) - r;
}

void lineSDFAABB(surface_t *s, float ax, float ay, float bx, float by, color_t color, float r)
{
	int x0 = (int)floorf(fminf(ax, bx) - r);
	int x1 = (int)ceilf(fmaxf(ax, bx) + r);
	int y0 = (int)floorf(fminf(ay, by) - r);
	int y1 = (int)ceilf(fmaxf(ay, by) + r);
	for (int y = y0; y <= y1; y++)
		for (int x = x0; x <= x1; x++)
		{
			// alphablend(s, x, y, fmaxf(fminf(0.5f - capsuleSDF((point_t){x, y}, (point_t){ax, ay}, (point_t){bx, by}, r), 1.0f), 0.0f), color);

			float a = fmaxf(fminf(0.5f - capsuleSDF((point_t){x, y}, (point_t){ax, ay}, (point_t){bx, by}, r), 1.0f), 0.0f);
			set_color(s, x, y, a, color);
		}
}

int draw_sdf_line(surface_t *s, point_t p1, point_t p2, color_t color, float thickness)
{
	lineSDFAABB(s, p1.x, p1.y, p2.x, p2.y, color, thickness / 2);
}

static int cmp_edge(const void *restrict p, const void *restrict q)
{
	const edge_t *restrict a = (const edge_t *)p;
	const edge_t *restrict b = (const edge_t *)q;
	if (a->start.y < b->start.y)
		return -1;
	if (a->start.y > b->start.y)
		return 1;
	return 0;
}

struct active_edge_t
{
	float x;
	int dir;
};

static int cmp_info(const void *restrict p, const void *restrict q)
{
	struct active_edge_t *restrict a = (struct active_edge_t *)p;
	struct active_edge_t *restrict b = (struct active_edge_t *)q;

	if (a->x != b->x)
	{
		return a->x > b->x ? 1 : -1;
	}

	return 0;
}

static void fill_scanline(context_t *ctx, int *scanline, float x0, float x1, int y)
{
	int i0, i1;
	i0 = fclampf(roundf(x0 - 0.5), 0, ctx->s->width - 1);
	i1 = fclampf(roundf(x1 + 0.5), 0, ctx->s->width - 1);
	float center = (x0 + x1) / 2.0;
	float r = 0.5 + fabsf(x0 - x1) / 2.0;

	while (i0 <= i1)
	{
		float d = r - fabsf(i0 - center);
		float w = d > 1.0f ? 1.0f : (d < 0.0f ? 0.0f : d);
		if (w == 0.0f)
		{
			i0++;
			continue;
		}

		scanline[i0++] += w * 51; // 51 == 255/5

		if (w == 1.0f)
		{
			break;
		}
	}

	while (i0 <= i1)
	{
		float d = r - fabsf(i1 - center);
		float w = d > 1.0f ? 1.0f : (d < 0.0f ? 0.0f : d);
		if (w == 0.0f)
		{
			i1--;
			continue;
		}

		scanline[i1--] += w * 51; // 51 == 255/5

		if (w == 1.0f)
		{
			break;
		}
	}

	for (int i = i0; i <= i1; i++)
	{
		scanline[i] += 51; // 51 == 255/5
	}
}

static void rasterize_sorted_edges(context_t *ctx, edge_t es[], int elen, color_t color)
{

	//FIXME: dynamic alloc
	struct active_edge_t a[1000] = {0};
	int width = ctx->s->width;
	int *scanline = malloc(sizeof(int) * width);

	assert(scanline);
	qsort(es, elen, sizeof(edge_t), cmp_edge);

	for (int y = 0; y < ctx->s->height; y++)
	{
		memset(scanline, 0, sizeof(int) * width);
		for (int s = 0; s < 5; s++)
		{
			float scan_y = s * 0.2 + y + ctx->origin.y;
			int count = 0;
			//TODO:optimize
			for (int i = 0; i < elen; i++)
			{
				edge_t e = es[i];
				if (scan_y >= e.start.y)
				{
					if (scan_y < e.end.y)
					{
						a[count].x = e.start.x + (scan_y - e.start.y) * (e.end.x - e.start.x) / (e.end.y - e.start.y);
						a[count].dir = e.dir;
						// draw_sdf_line(ctx->s, point_sub_point(e[i].start, ctx->origin), point_sub_point(e[i].start, ctx->origin), RGB(0xff0000), 3);
						// draw_sdf_line(ctx->s, point_sub_point(e[i].end, ctx->origin), point_sub_point(e[i].end, ctx->origin), RGB(0xff0000), 3);
						// draw_sdf_line(ctx->s, point_sub_point(e[i].start,ctx->origin), point_sub_point(e[i].end,ctx->origin), RGB(0xffff00), 3);

						count++;
					}
				}
				else
				{
					break;
				}
			}
			qsort(a, count, sizeof(struct active_edge_t), cmp_info);

			if (ctx->rule == FILLRULE_EVENODD)
			{
				for (int i = 0; i < count - 1; i++)
				{
					if (a[i].x != a[i + 1].x)
					{
						fill_scanline(ctx, scanline, a[i].x - ctx->origin.x, a[i + 1].x - ctx->origin.x, y);
						i++;
					}
				}
			}
			else
			{
				float x0 = 0;
				int w = 0;
				for (int i = 0; i < count; i++)
				{
					if (w == 0)
					{
						x0 = a[i].x;
						w += a[i].dir;
					}
					else
					{
						float x1 = a[i].x;
						w += a[i].dir;

						if (w == 0)
						{
							fill_scanline(ctx, scanline, x0 - ctx->origin.x, x1 - ctx->origin.x, y);
						}
					}
				}
			}
		}
		color_t *dl = ctx->s->pixels + y * width;
		color_t c = color;
		int a = color.a;
		for (int x = 0; x < width; x++)
		{
			color_t *d = &dl[x];
			//TODO:
#if 1
			c.a = idiv255(a * scanline[x]);
			blend(d, &c);
#else
			if (scanline[x] == 255)
			{
				color_t c = color;
				c.a = 255;
				blend(d, &c);
				d->a = color.a;
			}
			else if (scanline[x] > 0)
			{
				color_t c = color;
				c.a = c.a * scanline[x] / 255.0;

				if (d->a > 0)
				{
					blend(d, &c);
					d->a = color.a;
				}
				else
				{
					blend(d, &c);
				}
			}
#endif
		}
	}

	if (scanline)
	{
		free(scanline);
		scanline = NULL;
	}
}

static void context_init(context_t *ctx, surface_t *base)
{
	*ctx = (context_t){0};
	ctx->stroke_color = ctx->fill_color = (color_t){0, 0, 0, 255};
	ctx->base = base;
	ctx->min.y = ctx->min.x = 1000000;
	ctx->max.y = ctx->max.x = -1000000;
	ctx->se_ps = 1024;
	ctx->ses = 1024;
	ctx->e_ps = malloc(sizeof(expand_point_t) * 1024);
	ctx->es = malloc(sizeof(edge_t) * 1024);
}

static void context_surface_set(context_t *ctx, surface_t *s)
{
	ctx->s = s;
}

static void context_reset(context_t *ctx)
{
	ctx->nes = 0;
	ctx->ne_ps = 0;
	ctx->min.y = ctx->min.x = 1000000;
	ctx->max.y = ctx->max.x = -1000000;
	ctx->origin.y = ctx->origin.x = 0;
}

static void context_clear(context_t *ctx)
{
	ctx->nes = 0;
	ctx->min.y = ctx->min.x = 1000000;
	ctx->max.y = ctx->max.x = -1000000;
}

static void context_exit(context_t *ctx)
{
	if (ctx->s)
		surface_free(ctx->s);

	if (ctx->e_ps)
	{
		free(ctx->e_ps);
	}
	if (ctx->es)
	{
		free(ctx->es);
	}
	*ctx = (context_t){0};
}

static void context_line_width_set(context_t *ctx, float line_width)
{
	ctx->thickness = line_width;
}

static void context_fill_color_set(context_t *ctx, color_t color)
{
	ctx->fill_color = color;
}

static void context_stroke_color_set(context_t *ctx, color_t color)
{
	ctx->stroke_color = color;
}

static void context_cap_style_set(context_t *ctx, enum line_cap_t cap)
{
	ctx->cap = cap;
}

static void context_join_style_set(context_t *ctx, enum line_join_t join)
{
	ctx->join = join;
}

static void context_fill_rule_set(context_t *ctx, enum fill_rule_t rule)
{
	ctx->rule = rule;
}

static void context_surface_alloc(context_t *ctx)
{
	ctx->min.x = floorf(ctx->min.x) - 1;
	ctx->min.y = floorf(ctx->min.y) - 1;
	ctx->max.x = ceilf(ctx->max.x) + 1;
	ctx->max.y = ceilf(ctx->max.y) + 1;
	//TODO:
	// ctx->min = point_max(ctx->min, (point_t){0, 0});
	// ctx->max = point_min(ctx->max, (point_t){ctx->base->width - 1, ctx->base->height - 1});

	// if (ctx->min.x >= ctx->max.x || ctx->min.y >= ctx->max.y)
	// {
	// 	assert(0);
	// 	return;
	// }
	if (ctx->s)
	{
		int x = ctx->origin.x - ctx->min.x;
		int y = ctx->origin.y - ctx->min.y;
		int w = ctx->max.x - ctx->min.x + 1;
		int h = ctx->max.y - ctx->min.y + 1;
		surface_t *s = surface_clone(ctx->s, x, y, w, h);
		surface_free(ctx->s);
		ctx->s = s;
	}
	else
	{
		ctx->s = surface_alloc(ctx->max.x - ctx->min.x + 1, ctx->max.y - ctx->min.y + 1);
		assert(ctx->s != NULL);
	}
	ctx->origin = ctx->min;
}

static void context_surface_free(context_t *ctx)
{
	context_clear(ctx);
	if (ctx->s)
	{
		surface_free(ctx->s);
		ctx->s = NULL;
	}
}

static void set_point_path_type(context_t *ctx, enum point_path_type_t pp_type)
{
	if (ctx->ne_ps > 0)
		ctx->e_ps[ctx->ne_ps - 1].pp_type = pp_type;
}

static void close_path(context_t *ctx)
{
	set_point_path_type(ctx, POINT_PATH_CLOSE);
}

static void add_point(context_t *ctx, float x, float y, enum point_path_type_t pp_type)
{
	ctx->e_ps[ctx->ne_ps] = (expand_point_t){0};
	ctx->e_ps[ctx->ne_ps].p = (point_t){x, y};
	ctx->e_ps[ctx->ne_ps].pp_type = pp_type;
	ctx->ne_ps++;

	if (ctx->ne_ps >= ctx->se_ps)
	{
		ctx->se_ps *= 2;
		ctx->e_ps = realloc(ctx->e_ps, sizeof(expand_point_t) * ctx->se_ps);
	}
	assert(x >= -1000000 && x <= 1000000);
	assert(y >= -1000000 && y <= 1000000);

	ctx->min = point_min(ctx->min, (point_t){x, y});
	ctx->max = point_max(ctx->max, (point_t){x, y});
	// draw_sdf_line(ctx->s, (point_t){x,y}, (point_t){x,y}, (color_t){0, 0, 255, 255}, 2);
}

static void add_edge(context_t *ctx, point_t a, point_t b)
{
	int dir = 0;

	if (a.y < b.y)
	{
		dir = 1;
	}
	else
	{
		dir = -1;
	}
	// draw_sdf_line(ctx->s, a, b, (color_t){0, 0, 0, 255}, 1);
	// draw_sdf_line(ctx->s, a, a, (color_t){0, 0, 255, 255}, 2);
	// draw_sdf_line(ctx->s, b, b, (color_t){0, 0, 255, 255}, 2);
	ctx->min = point_min(ctx->min, a);
	ctx->min = point_min(ctx->min, b);
	ctx->max = point_max(ctx->max, a);
	ctx->max = point_max(ctx->max, b);

	if (a.y < b.y)
	{
		ctx->es[ctx->nes++] = (edge_t){a, b, dir};
	}
	else
	{
		ctx->es[ctx->nes++] = (edge_t){b, a, dir};
	}

	if (ctx->nes >= ctx->ses)
	{
		ctx->ses *= 2;
		ctx->e_ps = realloc(ctx->e_ps, sizeof(edge_t) * ctx->ses);
	}
}

static void move_to(context_t *ctx, float x, float y)
{
	add_point(ctx, x, y, POINT_PATH_BEGIN);
}

static void line_to(context_t *ctx, float x, float y)
{
	add_point(ctx, x, y, POINT_PATH);
}

static void cubic_bez(context_t *ctx, point_t p1, point_t p2, point_t p3, point_t p4, int level)
{
	if (level > 10)
		return;

	point_t d = point_sub_point(p4, p1);
	point_t d24 = point_sub_point(p2, p4);
	point_t d34 = point_sub_point(p3, p4);

	float d2 = fabsf(d24.x * d.y - d24.y * d.x);
	float d3 = fabsf(d34.x * d.y - d34.y * d.x);
	float S1 = d.x * d.x + d.y * d.y;
	float S2 = (d2 + d3) * (d2 + d3);

	if (S2 <= 0.25 * S1)
	{
		line_to(ctx, p4.x, p4.y);
		return;
	}

	point_t p12 = constant_point_add_point(p1, p2, 0.5);
	point_t p23 = constant_point_add_point(p2, p3, 0.5);
	point_t p34 = constant_point_add_point(p3, p4, 0.5);
	point_t p123 = constant_point_add_point(p12, p23, 0.5);
	point_t p234 = constant_point_add_point(p23, p34, 0.5);
	point_t p1234 = constant_point_add_point(p123, p234, 0.5);

	cubic_bez(ctx, p1, p12, p123, p1234, level + 1);
	cubic_bez(ctx, p1234, p234, p34, p4, level + 1);
}

static void cubic_bezto(context_t *ctx, point_t p1, point_t p2, point_t p3, point_t p4)
{
	cubic_bez(ctx, p1, p2, p3, p4, 0);
}

static void q_bezto(context_t *ctx, point_t p1, point_t p2, point_t p3, int level)
{
	if (level > 10)
		return;

	point_t d = point_sub_point(p3, p1);
	point_t d23 = point_sub_point(p2, p3);

	float d2 = fabsf(d23.x * d.y - d23.y * d.x);
	float S1 = d.x * d.x + d.y * d.y;
	float S2 = 4 * d2 * d2;

	if (S2 <= 0.25 * S1)
	{
		line_to(ctx, p3.x, p3.y);
		return;
	}

	point_t p12 = constant_point_add_point(p1, p2, 0.5);
	point_t p23 = constant_point_add_point(p2, p3, 0.5);
	point_t p123 = constant_point_add_point(p12, p23, 0.5);

	q_bezto(ctx, p1, p12, p123, level + 1);
	q_bezto(ctx, p123, p23, p3, level + 1);
}

static void quad_bezto(context_t *ctx, point_t p1, point_t p2, point_t p3)
{
	q_bezto(ctx, p1, p2, p3, 0);
}

static void arc_tobez(float a1, float da, point_t *v1, point_t *v2, point_t *h2)
{
	point_t *h1 = &(point_t){cosf(a1), sinf(a1)}, t1, t2;
	float h = 0;
	// da = -da;
	float a = M_PI / 2;
	// if (da == M_PI / 2)
	// {
	//     *h2 = (point_t){-h1->y, h1->x};
	//     h = KAPPA90;
	//     t1 = *h2;
	//     t2 = *h1;
	// }
	// else if (da == - M_PI / 2)
	// {
	//     *h2 = (point_t){-h1->y, h1->x};
	//     h = KAPPA90;
	//     t1 = *h2;
	//     t2 = *h1;
	// }
	// else
	{
		*h2 = (point_t){cosf(da) * h1->x - sinf(da) * h1->y, sinf(da) * h1->x + cosf(da) * h1->y};
		h = 4.0 * tanf(da / 4) / 3.0;
		t1 = (point_t){-sinf(a) * h1->y, sinf(a) * h1->x};
		t2 = (point_t){sinf(a) * h2->y, -sinf(a) * h2->x};
	}

	*v1 = point_add_point(*h1, point_mul_factor(t1, h));
	*v2 = point_add_point(*h2, point_mul_factor(t2, h));
}

static point_t unit_c2arc(float rx, float ry, float rotation, point_t p)
{
	float _cos = cosf(rotation);
	float _sin = sinf(rotation);
	p = (point_t){p.x * rx, p.y * ry};
	return (point_t){_cos * p.x - _sin * p.y, _sin * p.x + _cos * p.y};
}

static point_t arc2unit_c(float rx, float ry, float rotation, point_t p)
{
	float _cos = cosf(-rotation);
	float _sin = sinf(-rotation);
	p = (point_t){_cos * p.x - _sin * p.y, _sin * p.x + _cos * p.y};
	return (point_t){p.x / rx, p.y / ry};
}

//https://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
static void arc_to(context_t *ctx, float rx, float ry, float rotation, int large, int sweep, point_t p0, point_t p1)
{
	point_t dp, cp;
	rotation *= M_PI / 180.0;

	if (rx == 0 || ry == 0)
	{
		line_to(ctx, p1.x, p1.y);
		return;
	}

	rx = fabsf(rx);
	ry = fabsf(ry);

	dp = point_sub_point(p1, p0);
	cp = point_mul_factor(point_add_point(p1, p0), 0.5);

	float v = sqrtf((dp.x / 2.0) * (dp.x / 2.0) / (rx * rx) + (dp.y / 2.0) * (dp.y / 2.0) / (ry * ry));

	if (v > 1)
	{
		rx = v * rx;
		ry = v * ry;
	}

	dp = arc2unit_c(rx, ry, rotation, dp);
	float l = sqrtf(dp.x * dp.x + dp.y * dp.y);
	float sin = fclampf(l / 2, -1.0, 1.0);
	float cos = sqrtf(1 - sin * sin);
	point_t c;
	c = point_mul_factor(unit_point((large == sweep) ? (point_t){dp.y, -dp.x} : (point_t){-dp.y, dp.x}), cos);
	c = point_add_point(cp, unit_c2arc(rx, ry, rotation, c));

	point_t start = arc2unit_c(rx, ry, rotation, point_sub_point(p0, c));
	point_t end = arc2unit_c(rx, ry, rotation, point_sub_point(p1, c));

	float a0 = acosf(fclampf(point_dot_point((point_t){1, 0}, start), -1.0, 1.0));

	// float a0 = atan2f(start.y, start.x);
	a0 = point_cross_point((point_t){1, 0}, start) < 0 ? -a0 : a0;
	float da = acosf(fclampf(point_dot_point(start, end), -1.0, 1.0));
	da = point_cross_point(start, end) < 0 ? -da : da;
	da = fmodf(da, 2 * M_PI);

	// da = 2 * M_PI + da;
	if (sweep)
	{
		if (da < 0)
		{
			da = ((da < 0) ? 1 : -1) * 2 * M_PI + da;
		}
	}
	else
	{
		if (da > 0)
		{
			da = -2 * M_PI + da;
		}
	}

	int n = fabsf(da) * 2.0 / M_PI;
	float step = (da > 0) ? (M_PI / 2.0) : (-M_PI / 2.0);

	point_t h1 = start;
	for (int i = 0; i < n; i++, a0 += step, da -= step)
	{
		point_t v1, v2, h2;
		arc_tobez(a0, step, &v1, &v2, &h2);
		cubic_bezto(ctx,
					point_add_point(c, unit_c2arc(rx, ry, rotation, h1)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, v1)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, v2)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, h2)));

		h1 = h2;
	}

	if ((step > 0 && da > 0) || (step < 0 && da < 0))
	{
		point_t v1, v2, h2;
		arc_tobez(a0, da, &v1, &v2, &h2);
		cubic_bezto(ctx,
					point_add_point(c, unit_c2arc(rx, ry, rotation, h1)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, v1)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, v2)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, h2)));
	}
}

static void svg_to(context_t *ctx, const char *buf, svg_style_t style)
{
	point_t pos;
	point_t mirror;
	svg_parser_t *parser = &(svg_parser_t){0};
	// const char *buf = "M10,315L110,215A30,50,0,0,1,162.55,162.45L172.55,152.45A30,50,-45,0,1,215.1,109.9L315,10";
	svg_parser_init(parser, buf);
	svg_cmd_t cmd;

	while (svg_cmd_parser(parser, &cmd) == 1)
	{
		svg_cmd_transform(&cmd, style);
		switch (cmd.cmd)
		{
		case 'M':
			if (cmd.short_format)
			{
				line_to(ctx, cmd.M.x, cmd.M.y);
				pos = (point_t){cmd.M.x, cmd.M.y};
			}
			else
			{
				move_to(ctx, cmd.M.x, cmd.M.y);
				pos = (point_t){cmd.M.x, cmd.M.y};
			}

			break;
		case 'L':
			line_to(ctx, cmd.M.x, cmd.M.y);
			pos = (point_t){cmd.L.x, cmd.L.y};
			break;
		case 'H':
			line_to(ctx, cmd.H.x, pos.y);
			pos.x = cmd.H.x;
			break;
		case 'V':
			line_to(ctx, pos.x, cmd.V.y);
			pos.y = cmd.V.y;
			break;
		case 'C':
			cubic_bezto(ctx, pos, (point_t){cmd.C.x1, cmd.C.y1}, (point_t){cmd.C.x2, cmd.C.y2}, (point_t){cmd.C.x, cmd.C.y});
			mirror = (point_t){cmd.C.x2, cmd.C.y2};
			pos = (point_t){cmd.C.x, cmd.C.y};
			break;
		case 'S':
		{
			point_t p = point_add_point(pos, point_sub_point(pos, mirror));
			cubic_bezto(ctx, pos, p, (point_t){cmd.S.x2, cmd.S.y2}, (point_t){cmd.S.x, cmd.S.y});
			mirror = (point_t){cmd.S.x2, cmd.S.y2};
			pos = (point_t){cmd.S.x, cmd.S.y};
			break;
		}
		case 'Q':
			quad_bezto(ctx, pos, (point_t){cmd.Q.x1, cmd.Q.y1}, (point_t){cmd.Q.x, cmd.Q.y});
			mirror = (point_t){cmd.Q.x1, cmd.Q.y1};
			pos = (point_t){cmd.Q.x, cmd.Q.y};
			break;
		case 'T':
		{
			point_t p = point_add_point(pos, point_sub_point(pos, mirror));
			quad_bezto(ctx, pos, p, (point_t){cmd.T.x, cmd.T.y});
			mirror = p;
			pos = (point_t){cmd.T.x, cmd.T.y};
			break;
		}
		case 'A':
			arc_to(ctx, cmd.A.rx, cmd.A.ry, cmd.A.rotation, cmd.A.large, cmd.A.sweep, pos, (point_t){cmd.A.x, cmd.A.y});
			pos = (point_t){cmd.A.x, cmd.A.y};
			break;
		case 'm':
			move_to(ctx, pos.x + cmd.m.dx, pos.y + cmd.m.dy);
			pos = point_add_point(pos, (point_t){cmd.m.dx, cmd.m.dy});
			break;

		case 'l':
			line_to(ctx, pos.x + cmd.l.dx, pos.y + cmd.l.dy);
			pos = point_add_point(pos, (point_t){cmd.l.dx, cmd.l.dy});
			break;
		case 'h':
			line_to(ctx, pos.x + cmd.h.dx, pos.y);
			pos.x += cmd.h.dx;
			break;
		case 'v':
			line_to(ctx, pos.x, pos.y + cmd.v.dy);
			pos.y += cmd.v.dy;
			break;
		case 'c':
			cubic_bezto(ctx, pos, point_add_point(pos, (point_t){cmd.c.dx1, cmd.c.dy1}), point_add_point(pos, (point_t){cmd.c.dx2, cmd.c.dy2}), point_add_point(pos, (point_t){cmd.c.dx, cmd.c.dy}));
			mirror = point_add_point(pos, (point_t){cmd.c.dx2, cmd.c.dy2});
			pos = point_add_point(pos, (point_t){cmd.c.dx, cmd.c.dy});
			break;
		case 's':
		{
			point_t p = point_add_point(pos, point_sub_point(pos, mirror));
			cubic_bezto(ctx, pos, p, point_add_point(pos, (point_t){cmd.s.dx2, cmd.s.dy2}), point_add_point(pos, (point_t){cmd.s.dx, cmd.s.dy}));
			mirror = point_add_point(pos, (point_t){cmd.s.dx2, cmd.s.dy2});
			pos = point_add_point(pos, (point_t){cmd.s.dx, cmd.s.dy});
			break;
		}
		case 'q':
			quad_bezto(ctx, pos, point_add_point(pos, (point_t){cmd.q.dx1, cmd.q.dy1}), point_add_point(pos, (point_t){cmd.q.dx, cmd.q.dy}));
			mirror = point_add_point(pos, (point_t){cmd.q.dx1, cmd.q.dy1});
			pos = point_add_point(pos, (point_t){cmd.q.dx, cmd.q.dy});
			break;
		case 't':
		{
			point_t p = point_add_point(pos, point_sub_point(pos, mirror));
			quad_bezto(ctx, pos, p, point_add_point(pos, (point_t){cmd.t.dx, cmd.t.dy}));
			mirror = p;
			pos = point_add_point(pos, (point_t){cmd.t.dx, cmd.t.dy});
			break;
		}
		case 'a':
			arc_to(ctx, cmd.a.rx, cmd.a.ry, cmd.a.rotation, cmd.a.large, cmd.a.sweep, pos, point_add_point(pos, (point_t){cmd.a.dx, cmd.a.dy}));
			pos = point_add_point(pos, (point_t){cmd.a.dx, cmd.a.dy});
			break;
		case 'Z':
		case 'z':
			close_path(ctx);
			break;
		default:
			break;
		}

		// if (cmd.cmd == 'c')
		// {
		// 	printf("c %f %f %f %f %f %f\n", cmd.c.dx1, cmd.c.dy1, cmd.c.dx2, cmd.c.dy2, cmd.c.dx, cmd.c.dy);
		// }
		// else
		// {
		// 	printf("%c \n", cmd.cmd);
		// }

		if (cmd.cmd != 'C' && cmd.cmd != 'c' && cmd.cmd != 'S' && cmd.cmd != 's' && cmd.cmd != 'Q' && cmd.cmd != 'q' && cmd.cmd != 'T' && cmd.cmd != 't')
		{
			mirror = pos;
		}
	}
}

//FIXME: check h1, h2 not in once line
static point_t calc_intersection(point_t p1, point_t p2, point_t h1, point_t h2)
{
	return point_add_point(p1, point_mul_factor(h1, point_cross_point(point_sub_point(p2, p1), h2) / point_cross_point(h1, h2)));
}

static void add_bezier_edge(context_t *ctx, point_t p1, point_t p2, point_t p3, point_t p4, int level)
{
	if (level > 10)
		return;

	point_t d = point_sub_point(p4, p1);
	point_t d24 = point_sub_point(p2, p4);
	point_t d34 = point_sub_point(p3, p4);

	float d2 = fabsf(d24.x * d.y - d24.y * d.x);
	float d3 = fabsf(d34.x * d.y - d34.y * d.x);
	float S1 = d.x * d.x + d.y * d.y;
	float S2 = (d2 + d3) * (d2 + d3);

	if (S2 < 0.25 * S1)
	{
		add_edge(ctx, p1, p4);
		return;
	}

	point_t p12 = constant_point_add_point(p1, p2, 0.5);
	point_t p23 = constant_point_add_point(p2, p3, 0.5);
	point_t p34 = constant_point_add_point(p3, p4, 0.5);
	point_t p123 = constant_point_add_point(p12, p23, 0.5);
	point_t p234 = constant_point_add_point(p23, p34, 0.5);
	point_t p1234 = constant_point_add_point(p123, p234, 0.5);

	add_bezier_edge(ctx, p1, p12, p123, p1234, level + 1);
	add_bezier_edge(ctx, p1234, p234, p34, p4, level + 1);
}

static void mitter_join(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1)
{
	add_edge(ctx, ep0->p2, ep1->p1);
	add_edge(ctx, ep1->p3, ep0->p4);

	if (ep1->type == POINT_CORNER_LEFT)
	{
		add_edge(ctx, ep0->p1, ep0->cusp);
		add_edge(ctx, ep0->cusp, ep0->p2);
		add_edge(ctx, ep0->p4, ep0->p3);
	}
	else
	{
		add_edge(ctx, ep0->p4, ep0->cusp);
		add_edge(ctx, ep0->cusp, ep0->p3);
		add_edge(ctx, ep0->p1, ep0->p2);
	}
}

static void bevel_join(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1)
{
	add_edge(ctx, ep0->p2, ep1->p1);
	add_edge(ctx, ep1->p3, ep0->p4);

	add_edge(ctx, ep0->p4, ep0->p3);
	add_edge(ctx, ep0->p1, ep0->p2);
}

static void round_join(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1)
{
	float da = acosf(point_dot_point(ep0->h, ep1->h));
	float h = (ctx->thickness / 2.0) * (4.0 * tanf(da / 4) / 3.0);
	point_t h1, h2;

	add_edge(ctx, ep0->p2, ep1->p1);
	add_edge(ctx, ep1->p3, ep0->p4);

	if (ep0->type == POINT_CORNER_LEFT)
	{
		h1 = point_add_point(ep0->p1, point_mul_factor(ep0->h, h));
		h2 = point_add_point(ep0->p2, point_mul_factor(ep1->h, -h));
		add_bezier_edge(ctx, ep0->p1, h1, h2, ep0->p2, 0);
		add_edge(ctx, ep0->p4, ep0->p3);
	}
	else
	{
		h1 = point_add_point(ep0->p3, point_mul_factor(ep0->h, h));
		h2 = point_add_point(ep0->p4, point_mul_factor(ep1->h, -h));
		add_bezier_edge(ctx, ep0->p4, h2, h1, ep0->p3, 0);
		add_edge(ctx, ep0->p1, ep0->p2);
	}
}

static void butt_cap(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1, int head)
{
	if (head)
	{
		add_edge(ctx, ep0->p2, ep1->p1);
		add_edge(ctx, ep1->p3, ep0->p4);
		add_edge(ctx, ep0->p4, ep0->p2);
	}
	else
	{
		add_edge(ctx, ep0->p1, ep0->p3);
	}
}

static void square_cap(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1, int head)
{
	point_t h1, h2;
	float w = (ctx->thickness / 2.0);

	if (head)
	{
		h1 = point_add_point(ep0->p2, point_mul_factor(ep1->h, -w));
		h2 = point_add_point(ep0->p4, point_mul_factor(ep1->h, -w));
		add_edge(ctx, ep0->p2, ep1->p1);
		add_edge(ctx, ep1->p3, ep0->p4);

		add_edge(ctx, ep0->p4, h2);
		add_edge(ctx, h1, ep0->p2);
		add_edge(ctx, h2, h1);
	}
	else
	{
		h1 = point_add_point(ep0->p1, point_mul_factor(ep0->h, w));
		h2 = point_add_point(ep0->p3, point_mul_factor(ep0->h, w));

		add_edge(ctx, ep0->p1, h1);
		add_edge(ctx, h1, h2);
		add_edge(ctx, h2, ep0->p3);
	}
}

static void round_cap(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1, int head)
{
	point_t h[5];
	float w = (ctx->thickness / 2.0);

	if (head)
	{
		add_edge(ctx, ep0->p2, ep1->p1);
		add_edge(ctx, ep1->p3, ep0->p4);

		h[0] = point_add_point(ep0->p4, point_mul_factor(ep1->h, -w * KAPPA90));
		h[2] = point_add_point(ep0->p, point_mul_factor(ep1->h, -w));
		h[4] = point_add_point(ep0->p2, point_mul_factor(ep1->h, -w * KAPPA90));
		h[1] = point_add_point(h[2], point_mul_factor(ep1->v, -w * KAPPA90));
		h[3] = point_add_point(h[2], point_mul_factor(ep1->v, w * KAPPA90));

		add_bezier_edge(ctx, ep0->p4, h[0], h[1], h[2], 0);
		add_bezier_edge(ctx, h[2], h[3], h[4], ep0->p2, 0);
	}
	else
	{
		h[0] = point_add_point(ep0->p1, point_mul_factor(ep0->h, w * KAPPA90));
		h[2] = point_add_point(ep0->p, point_mul_factor(ep0->h, w));
		h[4] = point_add_point(ep0->p3, point_mul_factor(ep0->h, w * KAPPA90));
		h[1] = point_add_point(h[2], point_mul_factor(ep0->v, w * KAPPA90));
		h[3] = point_add_point(h[2], point_mul_factor(ep0->v, -w * KAPPA90));

		add_bezier_edge(ctx, ep0->p1, h[0], h[1], h[2], 0);
		add_bezier_edge(ctx, h[2], h[3], h[4], ep0->p3, 0);
	}
}

static void prepare_stroke(context_t *ctx, int s, int ne_ps)
{
	ctx->nes = 0;
	expand_point_t *e_ps = ctx->e_ps + s;

	for (int i = 0; i < ne_ps; i++)
	{
		point_t p1 = e_ps[i % ne_ps].p;
		point_t p2 = e_ps[(i + 1) % ne_ps].p;
		point_t d = unit_point(point_sub_point(p2, p1)); //FIXME: p2 = p1
		e_ps[(i + 1) % ne_ps].v = (point_t){-d.y, d.x};
		e_ps[(i + 1) % ne_ps].h = d;
	}

	for (int i = 0; i < ne_ps; i++)
	{
		expand_point_t *e_p0 = &e_ps[i % ne_ps];
		expand_point_t *e_p1 = &e_ps[(i + 1) % ne_ps];
		float w = ctx->thickness / 2.0;

		point_t p = e_p0->p;
		point_t v1 = e_p0->v;
		point_t v2 = e_p1->v;
		point_t h1 = e_p0->h;
		point_t h2 = e_p1->h;

		point_t t1 = point_mul_factor(v1, w);
		point_t t2 = point_mul_factor(v2, w);
		point_t t3 = point_mul_factor(v1, -w);
		point_t t4 = point_mul_factor(v2, -w);

		e_p0->p1 = point_add_point(p, t1);
		e_p0->p2 = point_add_point(p, t2);
		e_p0->p3 = point_add_point(p, t3);
		e_p0->p4 = point_add_point(p, t4);

		float cross = point_cross_point(h1, h2);

		if (cross > 0.0)
		{
			e_p0->type = POINT_CORNER_RIGHT;
			e_p0->cusp = calc_intersection(e_p0->p3, e_p0->p4, h1, h2);
		}
		else if (cross < 0.0)
		{
			e_p0->type = POINT_CORNER_LEFT;
			e_p0->cusp = calc_intersection(e_p0->p1, e_p0->p2, h1, h2);
		}
		else
		{
			e_p0->type = POINT_CORNER_LEFT;
			e_p0->cusp = point_add_point(p, t1);
		}
	}
}

static void expand_stroke(context_t *ctx, int s, int ne_ps)
{
	expand_point_t *e_ps = ctx->e_ps + s;
	int close = e_ps[ne_ps - 1].pp_type == POINT_PATH_CLOSE;

	for (int i = 0; i < ne_ps; i++)
	{
		if ((!close) && (i == 0 || i == ne_ps - 1))
		{
			if (ctx->cap == CAP_ROUND)
				round_cap(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps], i == 0);
			else if (ctx->cap == CAP_SQUARE)
				square_cap(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps], i == 0);
			else if (ctx->cap == CAP_BUTT)
				butt_cap(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps], i == 0);
		}
		else
		{
			if (ctx->join == JOIN_BEVEL)
				bevel_join(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps]);
			else if (ctx->join == JOIN_MITER)
				mitter_join(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps]);
			else if (ctx->join == JOIN_ROUND)
				round_join(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps]);
		}
	}
}

static void add_path(context_t *ctx, int s, int ne_ps)
{
	expand_point_t *e_ps = ctx->e_ps + s;

	if (ne_ps < 2)
		return;

	int close = e_ps[ne_ps - 1].pp_type == POINT_PATH_CLOSE;

	for (int i = 0; i < ne_ps - (!close); i++)
	{
		add_edge(ctx, e_ps[i % ne_ps].p, e_ps[(i + 1) % ne_ps].p);
	}
}

static void stroke(context_t *ctx)
{
	for (int i = 1, begin = 0; i < ctx->ne_ps; i++)
	{
		if (ctx->e_ps[i % ctx->ne_ps].pp_type == POINT_PATH_BEGIN)
		{
			prepare_stroke(ctx, begin, i - begin);
			expand_stroke(ctx, begin, i - begin);
			begin = i;
		}
		else if (i == ctx->ne_ps - 1)
		{
			prepare_stroke(ctx, begin, i - begin + 1);
			expand_stroke(ctx, begin, i - begin + 1);
		}
	}
	rasterize_sorted_edges(ctx, ctx->es, ctx->nes, ctx->stroke_color);
	ctx->nes = 0;
}

static void fill(context_t *ctx)
{
	for (int i = 1, begin = 0; i < ctx->ne_ps; i++)
	{
		if (ctx->e_ps[i % ctx->ne_ps].pp_type == POINT_PATH_BEGIN)
		{
			add_path(ctx, begin, i - begin);
			begin = i;
		}
		else if (i == ctx->ne_ps - 1)
		{
			add_path(ctx, begin, i - begin + 1);
		}
	}

	qsort(ctx->es, ctx->nes, sizeof(edge_t), cmp_edge);
}

static void render(context_t *ctx, float x, float y, style_t style)
{
	if (style.fill_color.a != 0)
	{
		context_fill_color_set(ctx, style.fill_color);

		for (int i = 1, begin = 0; i < ctx->ne_ps; i++)
		{
			if (ctx->e_ps[i % ctx->ne_ps].pp_type == POINT_PATH_BEGIN)
			{
				add_path(ctx, begin, i - begin);
				begin = i;
			}
			else if (i == ctx->ne_ps - 1)
			{
				add_path(ctx, begin, i - begin + 1);
			}
		}

		context_surface_alloc(ctx);
		rasterize_sorted_edges(ctx, ctx->es, ctx->nes, ctx->fill_color);
		context_clear(ctx);
	}

	if (style.stroke_width != 0.0 && style.stroke_color.a != 0)
	{
		context_line_width_set(ctx, style.stroke_width);
		context_stroke_color_set(ctx, style.stroke_color);

		for (int i = 1, begin = 0; i < ctx->ne_ps; i++)
		{
			if (ctx->e_ps[i % ctx->ne_ps].pp_type == POINT_PATH_BEGIN)
			{
				prepare_stroke(ctx, begin, i - begin);
				expand_stroke(ctx, begin, i - begin);
				begin = i;
			}
			else if (i == ctx->ne_ps - 1)
			{
				prepare_stroke(ctx, begin, i - begin + 1);
				expand_stroke(ctx, begin, i - begin + 1);
			}
		}

		context_surface_alloc(ctx);
		rasterize_sorted_edges(ctx, ctx->es, ctx->nes, ctx->stroke_color);
	}

	for (int i = 0; i < style.n_shadow; i++)
	{
		//TODO: How to calc shadow's range
		float range = style.shadow[i].blur * 2;
		surface_t *shadow = surface_clone(ctx->s, range, range, ctx->s->width + 2 * range, ctx->s->height + 2 * range);
		// surface_t *shadow = surface_copy(ctx->s);
		surface_mono(shadow, style.shadow[i].color);
		surface_filter_blur(shadow, style.shadow[i].blur);
		surface_blit(ctx->base, shadow, ctx->origin.x - range + style.shadow[i].shadow_h, ctx->origin.y - range + style.shadow[i].shadow_v);
		surface_free(shadow);
	}

	if (ctx->s)
	{
		if (style.clip_image != 0)
		{
			surface_mask(ctx->s, style.clip_image, ceilf(x - ctx->origin.x), ceilf(y - ctx->origin.y));
		}
		//TODO: conflict with clip image, shadow
		if (style.background_blur != 0.0f)
		{
			surface_t *cp = surface_copy(ctx->s);
			surface_t *cp1 = surface_copy(ctx->s);
			surface_cover(cp, ctx->base, ceilf(-ctx->origin.x), ceilf(-ctx->origin.y));
			surface_filter_blur(cp, style.background_blur);
			surface_clip(ctx->s, cp, 0, 0);
			surface_blit(ctx->s, cp1, 0, 0);
			surface_free(cp);
			surface_free(cp1);
		}
		if (style.transparency)
			surface_blit_with_opacity(ctx->base, ctx->s, ctx->origin.x, ctx->origin.y, 255 - style.transparency);
		else
			surface_blit(ctx->base, ctx->s, ctx->origin.x, ctx->origin.y);
	}
	context_surface_free(ctx);
}

void draw_line(surface_t *base, point_t p0, point_t p1, style_t style)
{
	context_t *ctx = &(context_t){0};

	context_init(ctx, base);

	move_to(ctx, p0.x, p0.y);
	line_to(ctx, p1.x, p1.y);
	render(ctx, 0, 0, style);
	context_exit(ctx);
}

void draw_polyline(surface_t *base, point_t *p, int n, style_t style)
{
	context_t *ctx = &(context_t){0};

	context_init(ctx, base);

	move_to(ctx, p[0].x, p[0].y);

	for (int i = 1; i < n; i++)
	{
		line_to(ctx, p[i].x, p[i].y);
	}

	render(ctx, 0, 0, style);
	context_exit(ctx);
}

void draw_polygon(surface_t *base, point_t *p, int n, style_t style)
{
	context_t *ctx = &(context_t){0};

	context_init(ctx, base);

	move_to(ctx, p[0].x, p[0].y);

	for (int i = 1; i < n; i++)
	{
		line_to(ctx, p[i].x, p[i].y);
	}
	close_path(ctx);
	render(ctx, 0, 0, style);
	context_exit(ctx);
}

void draw_bezier(surface_t *base, point_t p0, point_t p1, point_t p2, point_t p3, style_t style)
{
	context_t *ctx = &(context_t){0};

	context_init(ctx, base);
	move_to(ctx, p0.x, p0.y);
	cubic_bezto(ctx, p0, p1, p2, p3);
	render(ctx, 0, 0, style);
	context_exit(ctx);
}

void draw_arc(surface_t *base, float x, float y, float radius, float a1, float a2, style_t style)
{
	context_t *ctx = &(context_t){0};

	context_init(ctx, base);

	a1 = a1 * M_PI / 180;
	a2 = a2 * M_PI / 180;

	int n = (fabsf(a2 - a1) * 2.0 / M_PI);
	float begin = fminf(a1, a2);
	float end = fmaxf(a1, a2);
	float step = M_PI / 2;
	point_t p = (point_t){x, y};
	point_t h1 = (point_t){radius * cosf(begin), radius * sinf(begin)}, h2;

	move_to(ctx, p.x + h1.x, p.y + h1.y);
	for (int i = 0; i < n; i++, begin += step)
	{
		h2 = (point_t){-h1.y, h1.x};

		point_t v1 = point_add_point(p, point_add_point(h1, point_mul_factor(h2, KAPPA90)));
		point_t v2 = point_add_point(p, point_add_point(h2, point_mul_factor(h1, KAPPA90)));

		cubic_bezto(ctx, point_add_point(p, h1), v1, v2, point_add_point(p, h2));

		h1 = h2;
	}

	if (begin < end)
	{
		float a = end - begin;
		h2 = (point_t){cosf(a) * h1.x - sinf(a) * h1.y, sinf(a) * h1.x + cosf(a) * h1.y};
		float h = 4.0 * tanf(a / 4) / 3.0;

		point_t v1 = point_add_point(p, point_add_point(h1, point_mul_factor((point_t){-h1.y, h1.x}, h)));
		point_t v2 = point_add_point(p, point_add_point(h2, point_mul_factor((point_t){h2.y, -h2.x}, h)));
		cubic_bezto(ctx, point_add_point(p, h1), v1, v2, point_add_point(p, h2));
	}

	render(ctx, x, y, style);
	context_exit(ctx);
}

void draw_circle(surface_t *base, float x, float y, float radius, style_t style)
{
	context_t *ctx = &(context_t){0};

	context_init(ctx, base);

	cubic_bezto(ctx, (point_t){x + radius, y},
				(point_t){x + radius, y + radius * KAPPA90},
				(point_t){x + radius * KAPPA90, y + radius},
				(point_t){x, y + radius});
	cubic_bezto(ctx, (point_t){x, y + radius},
				(point_t){x - radius * KAPPA90, y + radius},
				(point_t){x - radius, y + radius * KAPPA90},
				(point_t){x - radius, y});
	cubic_bezto(ctx, (point_t){x - radius, y},
				(point_t){x - radius, y - radius * KAPPA90},
				(point_t){x - radius * KAPPA90, y - radius},
				(point_t){x, y - radius});
	cubic_bezto(ctx, (point_t){x, y - radius},
				(point_t){x + radius * KAPPA90, y - radius},
				(point_t){x + radius, y - radius * KAPPA90},
				(point_t){x + radius, y});
	close_path(ctx);
	render(ctx, x, y, style);
	context_exit(ctx);
}

void draw_ellipse(surface_t *base, float x, float y, float w, float h, style_t style)
{
	context_t *ctx = &(context_t){0};

	context_init(ctx, base);

	cubic_bezto(ctx, (point_t){x + w, y},
				(point_t){x + w, y + h * KAPPA90},
				(point_t){x + w * KAPPA90, y + h},
				(point_t){x, y + h});
	cubic_bezto(ctx, (point_t){x, y + h},
				(point_t){x - w * KAPPA90, y + h},
				(point_t){x - w, y + h * KAPPA90},
				(point_t){x - w, y});
	cubic_bezto(ctx, (point_t){x - w, y},
				(point_t){x - w, y - h * KAPPA90},
				(point_t){x - w * KAPPA90, y - h},
				(point_t){x, y - h});
	cubic_bezto(ctx, (point_t){x, y - h},
				(point_t){x + w * KAPPA90, y - h},
				(point_t){x + w, y - h * KAPPA90},
				(point_t){x + w, y});
	close_path(ctx);
	render(ctx, x, y, style);
	context_exit(ctx);
}

void draw_rectage(surface_t *base, float x, float y, float w, float h, float radius, style_t style)
{
	context_t *ctx = &(context_t){0};

	context_init(ctx, base);

	//FIXME: unable to add two same points
	if (radius != 0.0)
	{

		if (style.border_radius[1])
		{
			move_to(ctx, x + w - radius, y);
			cubic_bezto(ctx, (point_t){x + w - radius, y},
						(point_t){x + w - radius * (1 - KAPPA90), y},
						(point_t){x + w, y + radius * (1 - KAPPA90)},
						(point_t){x + w, y + radius});
		}
		else
		{
			move_to(ctx, x + w, y);
		}

		if (style.border_radius[2])
		{
			line_to(ctx, x + w, y + h - radius);
			cubic_bezto(ctx, (point_t){x + w, y + h - radius},
						(point_t){x + w, y + h - radius * (1 - KAPPA90)},
						(point_t){x + w - radius * (1 - KAPPA90), y + h},
						(point_t){x + w - radius, y + h});
		}
		else
		{
			line_to(ctx, x + w, y + h);
		}

		if (style.border_radius[3])
		{
			line_to(ctx, x + radius, y + h);
			cubic_bezto(ctx, (point_t){x + radius, y + h},
						(point_t){x + radius * (1 - KAPPA90), y + h},
						(point_t){x, y + h - radius * (1 - KAPPA90)},
						(point_t){x, y + h - radius});
		}
		else
		{
			line_to(ctx, x, y + h);
		}

		if (style.border_radius[0])
		{
			line_to(ctx, x, y + radius);
			cubic_bezto(ctx, (point_t){x, y + radius},
						(point_t){x, y + radius * (1 - KAPPA90)},
						(point_t){x + radius * (1 - KAPPA90), y},
						(point_t){x + radius, y});
		}
		else
		{
			line_to(ctx, x, y);
		}
	}
	else
	{
		move_to(ctx, x, y);
		line_to(ctx, x + w, y);
		line_to(ctx, x + w, y + h);
		line_to(ctx, x, y + h);
	}

	close_path(ctx);
	render(ctx, x, y, style);
	context_exit(ctx);
}

void draw_svg(surface_t *base, char *path, float vb_w, float vb_h, float w, float h, int x, int y, color_t color)
{
	context_t *ctx = &(context_t){0};
	context_init(ctx, base);
	context_cap_style_set(ctx, CAP_BUTT);
	context_join_style_set(ctx, JOIN_BEVEL);
	context_fill_color_set(ctx, color);

	svg_style_t style = (svg_style_t){
		scale : w / vb_w,
		translate_x : x,
		translate_y : y,
	};
	svg_to(ctx, path, style);
	render(ctx, x, y, (style_t){
		fill_color : color
	});
	context_exit(ctx);
}

void draw_text(surface_t *base, int x, int y, char *c, float size, color_t color)
{
	context_t *ctx = &(context_t){0};
	context_init(ctx, base);
	context_fill_color_set(ctx, color);
	context_cap_style_set(ctx, CAP_BUTT);
	context_join_style_set(ctx, JOIN_BEVEL);

	float scale = size * 16 / (2048);

	svg_style_t style = (svg_style_t){
		.scale = scale,
		.translate_x = x + 0 * scale,
		.translate_y = y + 1521 * scale,
		.mirror_y = 1,
	};
	//FIXME:0.8
	for (int i = 0; i < strlen(c); i++, style.translate_x += 1126 * scale)
	{
		svg_to(ctx, consolas_font[c[i]], style);
	}

	render(ctx, x, y, (style_t){
		fill_color : color
	});
	context_exit(ctx);
}

void draw_image(surface_t *base, const char *file, int x, int y, int w, int h)
{
	surface_clear(base, RGB(0xEAEBED), 0, 0, base->width, base->height);
	surface_t *s = surface_image_load(file, w, h);
	surface_blit(base, s, x, y);
	surface_free(s);
}
