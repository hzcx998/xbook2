#include "surface.h"
#include "render.h"
#include "image.h"

#define W 600
#define H 500

void sample(surface_t *base, float fps)
{
    surface_clear(base, (color_t){255, 255, 255, 255}, 0, 0, base->width, base->height);
    static int w = 0;
    w = (w + 1) % 30;

    draw_line(base, (point_t){100, 10}, (point_t){200, 110},
              (style_t){
                  stroke_width : w,
                  stroke_color : (color_t){0, 255, 0, 128}
              });
    draw_polygon(base,
                 (point_t[]){
                     {100, 100 + 200},
                     {200, 100 + 200},
                     {100, 200 + 200},
                 },
                 3,
                 (style_t){
                     stroke_width : w,
                     stroke_color : (color_t){0, 255, 0, 128}
                 });
    draw_bezier(base, (point_t){100, 100},
                (point_t){200, 100},
                (point_t){100, 200},
                (point_t){200, 200},
                (style_t){
                    stroke_width : w,
                    stroke_color : (color_t){0, 255, 0, 128}
                });
    draw_rectage(base, 300, 300, 100, 50, 0,
                 (style_t){
                     stroke_width : 10,
                     stroke_color : (color_t){0, 255, 255, 255}
                 });
    draw_ellipse(base, 350, 100, 100, 50,
                 (style_t){
                     stroke_width : 10,
                     stroke_color : (color_t){0, 255, 255, 255}
                 });
    draw_circle(base, 350, 300, 100,
                (style_t){
                    stroke_width : 10,
                    stroke_color : (color_t){0, 255, 255, 255}
                });
    draw_arc(base, 300, 220, 50, 0, 30,
             (style_t){
                 stroke_width : 30,
                 stroke_color : (color_t){0, 255, 255, 255}
             });
}

//#include "main.c"