#include <string.h>
#include <stdio.h>
#include <math.h>
#include "svg.h"

static inline int svg_isspace(char c)
{
    return strchr(" \t\n\v\f\r", c) != 0;
}

static inline int svg_isdigit(char c)
{
    return ((c >= '0') && (c <= '9'));
}

static inline int svg_isnum(char c)
{
    return strchr("0123456789+-.eE", c) != 0;
}

static inline int svg_iscmd(char c)
{
    return strchr("MLHVCSQTAZmlhvcsqtaz", c) != 0;
}

static void skip_space(svg_parser_t *parser)
{
    char c = parser->buf[parser->index];
    while (svg_isspace(c) || c == ',')
    {
        c = parser->buf[++parser->index];
    }
}

static const char *svg_parse_number(const char *s, char *it, const int size)
{
    const int last = size - 1;
    int i = 0;

    if (*s == '-' || *s == '+')
    {
        if (i < last)
            it[i++] = *s;
        s++;
    }
    while (*s && svg_isdigit(*s))
    {
        if (i < last)
            it[i++] = *s;
        s++;
    }
    if (*s == '.')
    {
        if (i < last)
            it[i++] = *s;
        s++;
        while (*s && svg_isdigit(*s))
        {
            if (i < last)
                it[i++] = *s;
            s++;
        }
    }
    if ((*s == 'e' || *s == 'E') && (s[1] != 'm' && s[1] != 'x'))
    {
        if (i < last)
            it[i++] = *s;
        s++;
        if (*s == '-' || *s == '+')
        {
            if (i < last)
                it[i++] = *s;
            s++;
        }
        while (*s && svg_isdigit(*s))
        {
            if (i < last)
                it[i++] = *s;
            s++;
        }
    }
    it[i] = '\0';

    return s;
}

static double svg_atof(const char *s)
{
    char *cur = (char *)s;
    char *end = NULL;
    double res = 0.0, sign = 1.0;
    long long intPart = 0, fracPart = 0;
    char hasIntPart = 0, hasFracPart = 0;

    if (*cur == '+')
    {
        cur++;
    }
    else if (*cur == '-')
    {
        sign = -1;
        cur++;
    }
    if (svg_isdigit(*cur))
    {
        intPart = strtoll(cur, &end, 10);
        if (cur != end)
        {
            res = (double)intPart;
            hasIntPart = 1;
            cur = end;
        }
    }
    if (*cur == '.')
    {
        cur++;
        if (svg_isdigit(*cur))
        {
            fracPart = strtoll(cur, &end, 10);
            if (cur != end)
            {
                res += (double)fracPart / pow(10.0, (double)(end - cur));
                hasFracPart = 1;
                cur = end;
            }
        }
    }
    if (!hasIntPart && !hasFracPart)
        return 0.0;
    if (*cur == 'e' || *cur == 'E')
    {
        long expPart = 0;
        cur++;
        expPart = strtol(cur, &end, 10);
        if (cur != end)
        {
            res *= pow(10.0, (double)expPart);
        }
    }
    return res * sign;
}

static token_t get_token(svg_parser_t *parser)
{
    skip_space(parser);
    if (parser->index >= parser->len)
    {
        return (token_t){
            .type = TOKEN_TYPE_END};
    }

    char c = parser->buf[parser->index];
    if (svg_iscmd(c))
    {
        parser->index++;
        return (token_t){
            .type = TOKEN_TYPE_CMD, .cmd = c};
    }
    else if (svg_isnum(c))
    {
        char buf[60];
        char *s = &parser->buf[parser->index];
        char *e = svg_parse_number(s, buf, 60);
        parser->index += e - s;
        float num = svg_atof(buf);
        return (token_t){
            .type = TOKEN_TYPE_NUM, .num = num};
    }
}

static token_t peek_token(svg_parser_t *parser)
{
    skip_space(parser);
    if (parser->index >= parser->len)
    {
        return (token_t){
            .type = TOKEN_TYPE_END};
    }

    char c = parser->buf[parser->index];
    if (svg_iscmd(c))
    {
        return (token_t){
            .type = TOKEN_TYPE_CMD, .cmd = c};
    }
    else if (svg_isnum(c))
    {
        char buf[60];
        char *s = &parser->buf[parser->index];
        char *e = svg_parse_number(s, buf, 60);
        float num = svg_atof(buf);
        return (token_t){
            .type = TOKEN_TYPE_NUM, .num = num};
    }
}

static int if_number(svg_parser_t *parser)
{
    return peek_token(parser).type == TOKEN_TYPE_NUM;
}

static int get_number(svg_parser_t *parser, float *f)
{
    token_t t = peek_token(parser);

    if (t.type != TOKEN_TYPE_NUM)
    {
        printf("ERROR\n");
        return 0;
    }
    else
    {
        *f = t.num;
        get_token(parser);
        return 1;
    }
}

static int get_int(svg_parser_t *parser, int *i)
{
    token_t t = peek_token(parser);

    if (t.type != TOKEN_TYPE_NUM)
    {
        printf("ERROR\n");
        return 0;
    }
    else
    {
        *i = roundf(t.num);
        get_token(parser);
        return 1;
    }
}

int svg_cmd_transform(svg_cmd_t *cmd, svg_style_t style)
{
#define MIRROR_X(f, v)
#define MIRROR_X(f, v)

#define SCALE_X(f, v) ((style.mirror_x ? (-(f)) : (f)) * (v))
#define SCALE_Y(f, v) ((style.mirror_y ? (-(f)) : (f)) * (v))

#define TRANSLATE(v, f) ((v) + (f))

    switch (cmd->cmd)
    {
    case 'M':
        cmd->M.x = TRANSLATE(SCALE_X(style.scale, cmd->M.x), style.translate_x);
        cmd->M.y = TRANSLATE(SCALE_Y(style.scale, cmd->M.y), style.translate_y);
        break;
    case 'L':
        cmd->L.x = TRANSLATE(SCALE_X(style.scale, cmd->L.x), style.translate_x);
        cmd->L.y = TRANSLATE(SCALE_Y(style.scale, cmd->L.y), style.translate_y);
        break;
    case 'H':
        cmd->H.x = TRANSLATE(SCALE_X(style.scale, cmd->H.x), style.translate_x);
        break;
    case 'V':
        cmd->V.y = TRANSLATE(SCALE_Y(style.scale, cmd->V.y), style.translate_y);
        break;
    case 'C':
        cmd->C.x1 = TRANSLATE(SCALE_X(style.scale, cmd->C.x1), style.translate_x);
        cmd->C.y1 = TRANSLATE(SCALE_Y(style.scale, cmd->C.y1), style.translate_y);
        cmd->C.x2 = TRANSLATE(SCALE_X(style.scale, cmd->C.x2), style.translate_x);
        cmd->C.y2 = TRANSLATE(SCALE_Y(style.scale, cmd->C.y2), style.translate_y);
        cmd->C.x = TRANSLATE(SCALE_X(style.scale, cmd->C.x), style.translate_x);
        cmd->C.y = TRANSLATE(SCALE_Y(style.scale, cmd->C.y), style.translate_y);
        break;
    case 'S':
        cmd->S.x2 = TRANSLATE(SCALE_X(style.scale, cmd->S.x2), style.translate_x);
        cmd->S.y2 = TRANSLATE(SCALE_Y(style.scale, cmd->S.y2), style.translate_y);
        cmd->S.x = TRANSLATE(SCALE_X(style.scale, cmd->S.x), style.translate_x);
        cmd->S.y = TRANSLATE(SCALE_Y(style.scale, cmd->S.y), style.translate_y);
        break;
    case 'Q':
        cmd->Q.x1 = TRANSLATE(SCALE_X(style.scale, cmd->Q.x1), style.translate_x);
        cmd->Q.y1 = TRANSLATE(SCALE_Y(style.scale, cmd->Q.y1), style.translate_y);
        cmd->Q.x = TRANSLATE(SCALE_X(style.scale, cmd->Q.x), style.translate_x);
        cmd->Q.y = TRANSLATE(SCALE_Y(style.scale, cmd->Q.y), style.translate_y);
        break;
    case 'T':
        cmd->T.x = TRANSLATE(SCALE_X(style.scale, cmd->T.x), style.translate_x);
        cmd->T.y = TRANSLATE(SCALE_Y(style.scale, cmd->T.y), style.translate_y);
        break;
    case 'A':
        cmd->A.rx = SCALE_X(style.scale, cmd->A.rx);
        cmd->A.ry = SCALE_Y(style.scale, cmd->A.ry);
        // cmd->A.rotation
        cmd->A.x = TRANSLATE(SCALE_X(style.scale, cmd->A.x), style.translate_x);
        cmd->A.y = TRANSLATE(SCALE_Y(style.scale, cmd->A.y), style.translate_y);
        break;
    case 'm':
        cmd->m.dx = SCALE_X(style.scale, cmd->m.dx);
        cmd->m.dy = SCALE_Y(style.scale, cmd->m.dy);
        break;

    case 'l':
        cmd->l.dx = SCALE_X(style.scale, cmd->l.dx);
        cmd->l.dy = SCALE_Y(style.scale, cmd->l.dy);
        break;
    case 'h':
        cmd->h.dx = SCALE_X(style.scale, cmd->h.dx);
        break;
    case 'v':
        cmd->v.dy = SCALE_Y(style.scale, cmd->v.dy);
        break;
    case 'c':
        cmd->c.dx1 = SCALE_X(style.scale, cmd->c.dx1);
        cmd->c.dy1 = SCALE_Y(style.scale, cmd->c.dy1);
        cmd->c.dx2 = SCALE_X(style.scale, cmd->c.dx2);
        cmd->c.dy2 = SCALE_Y(style.scale, cmd->c.dy2);
        cmd->c.dx = SCALE_X(style.scale, cmd->c.dx);
        cmd->c.dy = SCALE_Y(style.scale, cmd->c.dy);
        break;
    case 's':
        cmd->s.dx2 = SCALE_X(style.scale, cmd->s.dx2);
        cmd->s.dy2 = SCALE_Y(style.scale, cmd->s.dy2);
        cmd->s.dx = SCALE_X(style.scale, cmd->s.dx);
        cmd->s.dy = SCALE_Y(style.scale, cmd->s.dy);
        break;
    case 'q':
        cmd->q.dx1 = SCALE_X(style.scale, cmd->q.dx1);
        cmd->q.dy1 = SCALE_Y(style.scale, cmd->q.dy1);
        cmd->q.dx = SCALE_X(style.scale, cmd->q.dx);
        cmd->q.dy = SCALE_Y(style.scale, cmd->q.dy);
        break;
    case 't':
        cmd->t.dx = SCALE_X(style.scale, cmd->t.dx);
        cmd->t.dy = SCALE_Y(style.scale, cmd->t.dy);
        break;
    case 'a':
        cmd->a.rx = SCALE_X(style.scale, cmd->a.rx);
        cmd->a.ry = SCALE_Y(style.scale, cmd->a.ry);
        // cmd->a.rotation
        cmd->a.dx = SCALE_X(style.scale, cmd->a.dx);
        cmd->a.dy = SCALE_Y(style.scale, cmd->a.dy);
        break;
    case 'Z':
    case 'z':
        break;
    default:
        return -1;
        break;
    }

    return 1;
}

int svg_cmd_parser(svg_parser_t *parser, svg_cmd_t *cmd)
{

    token_t t = peek_token(parser);

    if (t.type == TOKEN_TYPE_END)
    {
        get_token(parser);
        return 0;
    }
    else if (t.type == TOKEN_TYPE_NUM)
    {
        cmd->short_format = 1;
        t.cmd = parser->cmd;
    }
    else
    {
        cmd->short_format = 0;
        parser->cmd = t.cmd;
        get_token(parser);
    }

    cmd->cmd = t.cmd;

    switch (t.cmd)
    {
    case 'M':
        get_number(parser, &cmd->M.x);
        get_number(parser, &cmd->M.y);
        break;
    case 'L':
        get_number(parser, &cmd->L.x);
        get_number(parser, &cmd->L.y);
        break;
    case 'H':
        get_number(parser, &cmd->H.x);

        break;
    case 'V':
        get_number(parser, &cmd->V.y);
        break;
    case 'C':
        get_number(parser, &cmd->C.x1);
        get_number(parser, &cmd->C.y1);
        get_number(parser, &cmd->C.x2);
        get_number(parser, &cmd->C.y2);
        get_number(parser, &cmd->C.x);
        get_number(parser, &cmd->C.y);
        break;
    case 'S':
        get_number(parser, &cmd->S.x2);
        get_number(parser, &cmd->S.y2);
        get_number(parser, &cmd->S.x);
        get_number(parser, &cmd->S.y);
        break;
    case 'Q':
        get_number(parser, &cmd->Q.x1);
        get_number(parser, &cmd->Q.y1);
        get_number(parser, &cmd->Q.x);
        get_number(parser, &cmd->Q.y);
        break;
    case 'T':
        get_number(parser, &cmd->T.x);
        get_number(parser, &cmd->T.y);
        break;
    case 'A':
        get_number(parser, &cmd->A.rx);
        get_number(parser, &cmd->A.ry);
        get_number(parser, &cmd->A.rotation);
        get_number(parser, &cmd->A.large);
        cmd->A.large = roundf(cmd->A.large);
        get_number(parser, &cmd->A.sweep);
        cmd->A.sweep = roundf(cmd->A.sweep);
        get_number(parser, &cmd->A.x);
        get_number(parser, &cmd->A.y);
        break;
    case 'm':
        get_number(parser, &cmd->m.dx);
        get_number(parser, &cmd->m.dy);
        break;

    case 'l':
        get_number(parser, &cmd->l.dx);
        get_number(parser, &cmd->l.dy);
        break;
    case 'h':
        get_number(parser, &cmd->h.dx);
        break;
    case 'v':
        get_number(parser, &cmd->v.dy);
        break;
    case 'c':
        get_number(parser, &cmd->c.dx1);
        get_number(parser, &cmd->c.dy1);
        get_number(parser, &cmd->c.dx2);
        get_number(parser, &cmd->c.dy2);
        get_number(parser, &cmd->c.dx);
        get_number(parser, &cmd->c.dy);
        break;
    case 's':
        get_number(parser, &cmd->s.dx2);
        get_number(parser, &cmd->s.dy2);
        get_number(parser, &cmd->s.dx);
        get_number(parser, &cmd->s.dy);
        break;
    case 'q':
        get_number(parser, &cmd->q.dx1);
        get_number(parser, &cmd->q.dy1);
        get_number(parser, &cmd->q.dx);
        get_number(parser, &cmd->q.dy);
        break;
    case 't':
        get_number(parser, &cmd->t.dx);
        get_number(parser, &cmd->t.dy);
        break;
    case 'a':
        get_number(parser, &cmd->a.rx);
        get_number(parser, &cmd->a.ry);
        get_number(parser, &cmd->a.rotation);
        get_number(parser, &cmd->a.large);
        cmd->a.large = roundf(cmd->a.large);
        get_number(parser, &cmd->a.sweep);
        cmd->a.sweep = roundf(cmd->a.sweep);
        get_number(parser, &cmd->a.dx);
        get_number(parser, &cmd->a.dy);
        break;
    case 'Z':
    case 'z':
        break;
    default:
        return -1;
        break;
    }

    return 1;
}

void svg_parser_init(svg_parser_t *parser, const char *buf)
{
    parser->buf = buf;
    parser->len = strlen(parser->buf);
    parser->index = 0;
}

void svg_parser()
{
    svg_parser_t *parser = &(svg_parser_t){
        0};
    const char *buf = "M960 512c0 6.9-0.2 13.7-0.5 20.5-0.2 4.1-0.4 8.3-0.7 12.4-0.1 1.3-0.2 2.6-0.3 3.8-1.2 14.5-3 28.7-5.5 42.7 0 0.1-0.1 0.3-0.1 0.4-1.1 6.2-2.4 12.4-3.8 18.6-1.4 6.3-3 12.5-4.6 18.7 0 0.2-0.1 0.4-0.1 0.6-3.2 12-7 23.8-11.2 35.3-1.3 3.5-2.6 7-3.9 10.4-0.7 1.9-1.5 3.7-2.2 5.6-2 4.8-4 9.5-6.1 14.2-1.8 4-3.6 7.9-5.5 11.8-8.6 17.8-18.4 35-29.3 51.4-0.2 0.3-0.4 0.7-0.7 1-2.1 3.2-4.3 6.5-6.6 9.6-3.9 5.5-7.8 10.9-11.9 16.2-4.5 5.9-9.2 11.6-14 17.2-24.5 28.9-52.6 54.5-83.5 76.3-16.9 11.9-34.7 22.7-53.2 32.1C655 942.3 585.6 960 512 960c-118.7 0-226.5-46.1-306.7-121.4-2.2-2.1-4.5-4.3-6.7-6.5l-6.3-6.3c-12.5-12.7-24.3-26.2-35.2-40.4-3.3-4.3-6.6-8.7-9.7-13.1-2.9-4.1-5.8-8.2-8.6-12.4-0.2-0.2-0.3-0.5-0.5-0.7-0.1-0.2-0.3-0.4-0.4-0.6-2.8-4.2-5.5-8.4-8.1-12.7-7.7-12.5-14.8-25.4-21.2-38.7-1.5-3.1-3-6.2-4.4-9.4-1.8-3.8-3.4-7.7-5.1-11.6-1.2-2.8-2.3-5.5-3.4-8.3-0.5-1.1-0.9-2.3-1.4-3.5s-0.9-2.4-1.4-3.6c-0.7-1.9-1.5-3.9-2.2-5.8-2.5-7-4.9-14-7.1-21.2-1.4-4.7-2.8-9.4-4.1-14.1-0.1-0.2-0.1-0.5-0.2-0.7-3.5-12.8-6.4-25.9-8.7-39.2-2.4-13.5-4.2-27.3-5.3-41.2-0.1-1.2-0.2-2.5-0.3-3.8-0.2-2.1-0.3-4.2-0.4-6.4-0.1-2-0.2-4.1-0.3-6.1-0.3-6.8-0.5-13.7-0.5-20.5 0-8.2 0.2-16.2 0.7-24.3 0.2-4 0.5-8 0.8-12 0-0.3 0-0.7 0.1-1 0.9-10.6 2.1-21.1 3.7-31.5 0.7-4.3 1.4-8.5 2.2-12.7 1-5.6 2.2-11.2 3.4-16.7 4.3-19.3 9.9-38 16.6-56.3C154.3 186.2 318.9 64 512 64c63.6 0 124.1 13.2 178.9 37.2 26.9 11.7 52.4 26 76.2 42.5 75 52.1 133.4 126.5 165.5 213.6 6.7 18.3 12.3 37.1 16.7 56.5 1.2 5.5 2.4 11.1 3.4 16.7 0.8 4.2 1.5 8.4 2.1 12.6 1.7 10.8 3 21.6 3.8 32.7 0.8 10.1 1.3 20.2 1.4 30.5v5.7z";

    svg_parser_init(parser, buf);
    svg_cmd_t cmd;
    while (svg_cmd_parser(parser, &cmd) == 1)
    {
        if (cmd.cmd == 'c')
        {
            printf("c %f %f %f %f %f %f\n", cmd.c.dx1, cmd.c.dy1, cmd.c.dx2, cmd.c.dy2, cmd.c.dx, cmd.c.dy);
        }
        else
        {
            printf("%c \n", cmd.cmd);
        }
    }
}