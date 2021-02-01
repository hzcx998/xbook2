#pragma once
#include "color.h"
#include "surface.h"
#include "point.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct shadow_t
    {
        float shadow_h;
        float shadow_v;
        float blur;
        // float spread ; Not support
        color_t color;
    } shadow_t;

    typedef struct style_t
    {
        color_t fill_color;
        color_t stroke_color;
        unsigned char transparency;
        float stroke_width;
        shadow_t *shadow;
        int n_shadow;
        int top;
        int left;
        char border_radius[4];
        surface_t *clip_image;
        float background_blur;
    } style_t;

    enum line_join_t
    {
        JOIN_MITER = 0,
        JOIN_ROUND = 1,
        JOIN_BEVEL = 2,
    };

    enum line_cap_t
    {
        CAP_BUTT = 0,
        CAP_ROUND = 1,
        CAP_SQUARE = 2,
    };

    enum fill_rule_t
    {
        FILLRULE_NONZERO = 0,
        FILLRULE_EVENODD = 1,
    };

    enum point_type_t
    {
        POINT_CORNER_LEFT = 0,
        POINT_CORNER_RIGHT = 1,
    };

    enum point_path_type_t
    {
        POINT_PATH = 0,
        POINT_PATH_BEGIN = 1,
        POINT_PATH_CLOSE = 2,
    };

    typedef struct cube_bez_t
    {
        point_t p[4];
    } cube_bez_t;

    typedef struct expand_point_t
    {
        point_t p;
        point_t v, h;
        point_t p1, p2, p3, p4;
        point_t cusp;
        enum point_type_t type;
        enum point_path_type_t pp_type;
    } expand_point_t;

    typedef struct edge_t
    {
        point_t start, end;
        int dir;
    } edge_t;

    typedef struct context_t context_t;

    typedef struct context_t
    {
        float width;

        int close;
        edge_t *es;
        expand_point_t *e_ps;
        int nes;
        int ne_ps;
        int ses;
        int se_ps;
        enum line_join_t join;
        enum line_cap_t cap;
        enum fill_rule_t rule;
        point_t min, max, origin;
        surface_t *s, *base;

        void (*move_to)(context_t *ctx, float x, float y);
        void (*line_to)(context_t *ctx, float x, float y);
        void (*cubic_bezto)(context_t *ctx, point_t p1, point_t p2, point_t p3, point_t p4);
        void (*quad_bezto)(context_t *ctx, point_t p1, point_t p2, point_t p3);
        void (*arc_to)(context_t *ctx, float rx, float ry, float rotation, int large, int sweep, point_t p0, point_t p1);
        void (*close_path)(context_t *ctx);
        void (*render)(context_t *ctx, float x, float y, style_t style);
    } context_t;

    void context_init(context_t *ctx, surface_t *base);
    void context_exit(context_t *ctx);

    void draw_line(surface_t *base, point_t p0, point_t p1, style_t style);
    void draw_polyline(surface_t *base, point_t *p, int n, style_t style);
    void draw_polygon(surface_t *base, point_t *p, int n, style_t style);
    void draw_bezier(surface_t *base, point_t p0, point_t p1, point_t p2, point_t p3, style_t style);
    void draw_arc(surface_t *base, float x, float y, float radius, float a1, float a2, style_t style);
    void draw_circle(surface_t *base, float x, float y, float radius, style_t style);

    void draw_ellipse(surface_t *base, float x, float y, float w, float h, style_t style);
    void draw_rectangle(surface_t *base, float x, float y, float w, float h, float radius, style_t style);
    void draw_svg(surface_t *base, char *path, float vb_w, float vb_h, float w, float h, int x, int y, color_t color);
    void draw_text(surface_t *base, int x, int y, char *c, float size, color_t color);
    void draw_image(surface_t *base, const char *file, int x, int y, int w, int h);
    surface_t *surface_svg_get(char *path, float vb_w, float vb_h, float w, float h, color_t color);

#ifdef __cplusplus
}
#endif