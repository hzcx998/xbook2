#pragma once

typedef struct svg_cmd_t
{
    union
    {
        struct
        {
            float x, y;
        } M, L;
        struct
        {
            float dx, dy;
        } m, l;
        struct
        {
            float x;
        } H;
        struct
        {
            float dx;
        } h;
        struct
        {
            float y;
        } V;
        struct
        {
            float dy;
        } v;
        struct
        {
            float x1, y1, x2, y2, x, y;
        } C;
        struct
        {
            float dx1, dy1, dx2, dy2, dx, dy;
        } c;
        struct
        {
            float x2, y2, x, y;
        } S;
        struct
        {
            float dx2, dy2, dx, dy;
        } s;
        struct
        {
            float x1, y1, x, y;
        } Q;
        struct
        {
            float dx1, dy1, dx, dy;
        } q;
        struct
        {
            float x, y;
        } T;
        struct
        {
            float dx, dy;
        } t;
        struct
        {
            float rx, ry, rotation, x, y, large, sweep;
        } A;
        struct
        {
            float rx, ry, rotation, dx, dy, large, sweep;
        } a;

        struct
        {
            float f[7];
        } COMMON;
    };
    char cmd;
    int short_format;
}svg_cmd_t;

typedef struct svg_style_t
{
    float scale;
    float translate_x,translate_y;
    float viewbox_width,viewbox_height;
    int mirror_x,mirror_y;
} svg_style_t;

enum token_type_t {
    TOKEN_TYPE_CMD,
    TOKEN_TYPE_NUM,
    TOKEN_TYPE_END
};

typedef struct token_t
{
    enum token_type_t type;
    float num;
    char cmd;
} token_t;

typedef struct svg_parser_t
{
    char * buf;
    int len;
    int index;
    char cmd;
}svg_parser_t;
int svg_cmd_transform(svg_cmd_t *cmd, svg_style_t style);
int svg_cmd_parser(svg_parser_t *parser, svg_cmd_t *cmd);
void svg_parser_init(svg_parser_t *parser, const char *buf);