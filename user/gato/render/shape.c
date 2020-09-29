#include <math.h>
#include "render.h"
#include "surface.h"
#include "macro.h"

void draw_line(surface_t *base, point_t p0, point_t p1, style_t style)
{
    context_t *ctx = &(context_t){0};
    context_init(ctx, base);
    ctx->move_to(ctx, p0.x, p0.y);
    ctx->line_to(ctx, p1.x, p1.y);
    ctx->render(ctx, 0, 0, style);
    context_exit(ctx);
}

void draw_polyline(surface_t *base, point_t *p, int n, style_t style)
{
    context_t *ctx = &(context_t){0};
    context_init(ctx, base);
    ctx->move_to(ctx, p[0].x, p[0].y);
    for (int i = 1; i < n; i++)
        ctx->line_to(ctx, p[i].x, p[i].y);
    ctx->render(ctx, 0, 0, style);
    context_exit(ctx);
}

void draw_polygon(surface_t *base, point_t *p, int n, style_t style)
{
    context_t *ctx = &(context_t){0};
    context_init(ctx, base);
    ctx->move_to(ctx, p[0].x, p[0].y);
    for (int i = 1; i < n; i++)
        ctx->line_to(ctx, p[i].x, p[i].y);
    ctx->close_path(ctx);
    ctx->render(ctx, 0, 0, style);
    context_exit(ctx);
}

void draw_bezier(surface_t *base, point_t p0, point_t p1, point_t p2, point_t p3, style_t style)
{
    context_t *ctx = &(context_t){0};
    context_init(ctx, base);
    ctx->move_to(ctx, p0.x, p0.y);
    ctx->cubic_bezto(ctx, p0, p1, p2, p3);
    ctx->render(ctx, 0, 0, style);
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

    ctx->move_to(ctx, p.x + h1.x, p.y + h1.y);
    for (int i = 0; i < n; i++, begin += step)
    {
        h2 = (point_t){-h1.y, h1.x};

        point_t v1 = point_add_point(p, point_add_point(h1, point_mul_factor(h2, KAPPA90)));
        point_t v2 = point_add_point(p, point_add_point(h2, point_mul_factor(h1, KAPPA90)));

        ctx->cubic_bezto(ctx, point_add_point(p, h1), v1, v2, point_add_point(p, h2));

        h1 = h2;
    }

    if (begin < end)
    {
        float a = end - begin;
        h2 = (point_t){cosf(a) * h1.x - sinf(a) * h1.y, sinf(a) * h1.x + cosf(a) * h1.y};
        float h = 4.0 * tanf(a / 4) / 3.0;

        point_t v1 = point_add_point(p, point_add_point(h1, point_mul_factor((point_t){-h1.y, h1.x}, h)));
        point_t v2 = point_add_point(p, point_add_point(h2, point_mul_factor((point_t){h2.y, -h2.x}, h)));
        ctx->cubic_bezto(ctx, point_add_point(p, h1), v1, v2, point_add_point(p, h2));
    }

    ctx->render(ctx, x, y, style);
    context_exit(ctx);
}

void draw_circle(surface_t *base, float x, float y, float radius, style_t style)
{
    context_t *ctx = &(context_t){0};
    context_init(ctx, base);

    ctx->cubic_bezto(ctx, (point_t){x + radius, y},
                     (point_t){x + radius, y + radius * KAPPA90},
                     (point_t){x + radius * KAPPA90, y + radius},
                     (point_t){x, y + radius});
    ctx->cubic_bezto(ctx, (point_t){x, y + radius},
                     (point_t){x - radius * KAPPA90, y + radius},
                     (point_t){x - radius, y + radius * KAPPA90},
                     (point_t){x - radius, y});
    ctx->cubic_bezto(ctx, (point_t){x - radius, y},
                     (point_t){x - radius, y - radius * KAPPA90},
                     (point_t){x - radius * KAPPA90, y - radius},
                     (point_t){x, y - radius});
    ctx->cubic_bezto(ctx, (point_t){x, y - radius},
                     (point_t){x + radius * KAPPA90, y - radius},
                     (point_t){x + radius, y - radius * KAPPA90},
                     (point_t){x + radius, y});
    ctx->close_path(ctx);
    ctx->render(ctx, x, y, style);
    context_exit(ctx);
}

void draw_ellipse(surface_t *base, float x, float y, float w, float h, style_t style)
{
    context_t *ctx = &(context_t){0};
    context_init(ctx, base);

    ctx->cubic_bezto(ctx, (point_t){x + w, y},
                     (point_t){x + w, y + h * KAPPA90},
                     (point_t){x + w * KAPPA90, y + h},
                     (point_t){x, y + h});
    ctx->cubic_bezto(ctx, (point_t){x, y + h},
                     (point_t){x - w * KAPPA90, y + h},
                     (point_t){x - w, y + h * KAPPA90},
                     (point_t){x - w, y});
    ctx->cubic_bezto(ctx, (point_t){x - w, y},
                     (point_t){x - w, y - h * KAPPA90},
                     (point_t){x - w * KAPPA90, y - h},
                     (point_t){x, y - h});
    ctx->cubic_bezto(ctx, (point_t){x, y - h},
                     (point_t){x + w * KAPPA90, y - h},
                     (point_t){x + w, y - h * KAPPA90},
                     (point_t){x + w, y});
    ctx->close_path(ctx);
    ctx->render(ctx, x, y, style);
    context_exit(ctx);
}

void draw_rectangle(surface_t *base, float x, float y, float w, float h, float radius, style_t style)
{
    context_t *ctx = &(context_t){0};

    context_init(ctx, base);

    //FIXME: unable to add two same points
    if (radius != 0.0)
    {

        if (style.border_radius[1])
        {
            ctx->move_to(ctx, x + w - radius, y);
            ctx->cubic_bezto(ctx, (point_t){x + w - radius, y},
                             (point_t){x + w - radius * (1 - KAPPA90), y},
                             (point_t){x + w, y + radius * (1 - KAPPA90)},
                             (point_t){x + w, y + radius});
        }
        else
        {
            ctx->move_to(ctx, x + w, y);
        }

        if (style.border_radius[2])
        {
            ctx->line_to(ctx, x + w, y + h - radius);
            ctx->cubic_bezto(ctx, (point_t){x + w, y + h - radius},
                             (point_t){x + w, y + h - radius * (1 - KAPPA90)},
                             (point_t){x + w - radius * (1 - KAPPA90), y + h},
                             (point_t){x + w - radius, y + h});
        }
        else
        {
            ctx->line_to(ctx, x + w, y + h);
        }

        if (style.border_radius[3])
        {
            ctx->line_to(ctx, x + radius, y + h);
            ctx->cubic_bezto(ctx, (point_t){x + radius, y + h},
                             (point_t){x + radius * (1 - KAPPA90), y + h},
                             (point_t){x, y + h - radius * (1 - KAPPA90)},
                             (point_t){x, y + h - radius});
        }
        else
        {
            ctx->line_to(ctx, x, y + h);
        }

        if (style.border_radius[0])
        {
            ctx->line_to(ctx, x, y + radius);
            ctx->cubic_bezto(ctx, (point_t){x, y + radius},
                             (point_t){x, y + radius * (1 - KAPPA90)},
                             (point_t){x + radius * (1 - KAPPA90), y},
                             (point_t){x + radius, y});
        }
        else
        {
            ctx->line_to(ctx, x, y);
        }
    }
    else
    {
        ctx->move_to(ctx, x, y);
        ctx->line_to(ctx, x + w, y);
        ctx->line_to(ctx, x + w, y + h);
        ctx->line_to(ctx, x, y + h);
    }

    ctx->close_path(ctx);
    ctx->render(ctx, x, y, style);
    context_exit(ctx);
}