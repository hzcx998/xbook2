
#ifndef _LIB_DOTFONT_H
#define _LIB_DOTFONT_H

#include <types.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* 支持的字体数量，可以配置 */
#define DOTF_MAX_NR 2
#define DOTF_NAME_LEN 24

#define DOTF_TYPE_STATIC    0
#define DOTF_TYPE_DYNAMIC   1

typedef struct {
	char name[DOTF_NAME_LEN];	            /* 字体名字 */
    char type;          /* 静态字体和动态字体 */
	uint8_t *addr;		/* 字体数据地址 */
	uint32_t char_width;  /* 单字宽度 */
    uint32_t char_height; /* 单字宽度 */
} dotfont_t;

typedef struct {
    dotfont_t fonts[DOTF_MAX_NR];
    dotfont_t *current;
} dotfont_library_t;

static dotfont_t *dotfont_find(dotfont_library_t *dotflib, char *name)
{
    dotfont_t *dotfont;
    int i;
    for (i = 0; i < DOTF_MAX_NR; i++) {
        dotfont = &dotflib->fonts[i];
        if (!strcmp(dotfont->name, name)) {
            return dotfont;
        }
    }
    return NULL;
}

static int dotfont_install(dotfont_library_t *dotflib,
        char type, char *name, uint8_t *addr,
        uint32_t char_width, uint32_t char_height)
{
    dotfont_t *dotfont;
    int i;
    for (i = 0; i < DOTF_MAX_NR; i++) {
        dotfont = &dotflib->fonts[i];
        if (dotfont->addr == NULL) {
            dotfont->addr = addr;
            dotfont->char_width = char_width;
            dotfont->char_height = char_height;
            dotfont->type = type;
            strcpy(dotfont->name, name);
            dotfont->name[DOTF_NAME_LEN - 1] = '\0';
            return 0;
        }
    }
    return -1;
}

static int dotfont_uninstall(dotfont_library_t *dotflib, char *name)
{
    dotfont_t *dotfont = dotfont_find(dotflib, name);
    if (!dotfont)
        return -1;
    if (dotfont->type == DOTF_TYPE_DYNAMIC) {
        free(dotfont->addr);
    }
    dotfont->addr = NULL;
    memset(dotfont->name, 0, DOTF_NAME_LEN);
    return 0;
}

static int dotfont_set_default(dotfont_library_t *dotflib, char *name)
{
    dotfont_t *dotfont = dotfont_find(dotflib, name);
    if (!dotfont)
        return -1;
    dotflib->current = dotfont;
    return 0;
}

static dotfont_t *dotfont_get_current(dotfont_library_t *dotflib)
{
    return dotflib->current;
}

#include "dotfont_standard.h"

static int dotfont_init(dotfont_library_t *dotflib) 
{
    if (!dotflib)
        return -1;
    int i;
    for (i = 0; i < DOTF_MAX_NR; i++) {
        memset((void *)&dotflib->fonts[i], 0, sizeof(dotfont_t));
    }
    dotfont_install(dotflib, DOTF_TYPE_STATIC, DOTF_STANDARD_NAME,
        dotfont_data_standard, DOTF_STANDARD_W, DOTF_STANDARD_H);
    dotfont_set_default(dotflib, DOTF_STANDARD_NAME);
    return 0;
}

static int dotfont_exit(dotfont_library_t *dotflib)
{
    if (!dotflib)
        return -1;
    dotfont_t *dotfont;
    int i;
    for (i = 0; i < DOTF_MAX_NR; i++) {
        dotfont = &dotflib->fonts[i];
        if (dotfont->addr)
            dotfont_uninstall(dotflib, dotfont->name);
    }
    dotflib->current = NULL;
    return 0;
}

#define dotfont_get_char_width(dotfont) ((dotfont)->char_width)
#define dotfont_get_char_height(dotfont) ((dotfont)->char_height)
#define dotfont_get_addr(dotfont, ch) ((dotfont)->addr + (ch) * dotfont_get_char_height(dotfont))

#endif /* _LIB_DOTFONT_H */