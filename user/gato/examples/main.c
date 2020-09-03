#define _XOPEN_SOURCE 600


//#include <SDL2/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <gapi.h>

#include "render.h"
#include "surface.h"

#if 0
static SDL_Window *gWindow = NULL;
static SDL_Surface *gSurface = NULL;
#else
static int gWindow = -1;
static g_bitmap_t *gSurface = NULL;
#endif

static void frambuffer_init()
{
    printf("frambuffer_init.\n");
    
    #if 0
    SDL_Init(SDL_INIT_VIDEO);
    gWindow = SDL_CreateWindow("SDL", 100, 100, W, H, SDL_WINDOW_SHOWN);
    gSurface = SDL_GetWindowSurface(gWindow);
    #else
    if (g_init() < 0) {
        printf("gui init failed!\n");
        exit(-1);
    }
    printf("gui new window start.\n");
        
    gWindow = g_new_window("gato", 100, 100, W, H);
    if (gWindow < 0) {
        printf("gui new window failed!\n");
        g_quit();
        exit(-1);
    }
    g_show_window(gWindow);
    gSurface = g_new_bitmap(W, H);
    if (gSurface == NULL) {
        printf("gui new bitmap failed!\n");
        g_quit();
        exit(-1);
    }
    #endif
}

static void frambuffer_close()
{
    #if 0
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    #else
    g_del_bitmap(gSurface);
    g_del_window(gWindow);
    g_quit();
    #endif
}
// #define PROFILE

#ifdef PROFILE
#include <gperftools/profiler.h>
#endif

int main(int argc, char *argv[])
{
    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};
#ifdef PROFILE
    ProfilerStart("test.prof"); //开启性能分析
    atexit(ProfilerStop);
#endif
    frambuffer_init();
    float fps = 0;

#if 0
    surface_t *surface = surface_alloc(W, H);

    while (1)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                exit(EXIT_SUCCESS);
                break;
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &time1);
        draw_new_UI(surface);
        clock_gettime(CLOCK_MONOTONIC, &time2);
        unsigned long long mtime = (time2.tv_sec - time1.tv_sec) * 1000000 + (time2.tv_nsec - time1.tv_nsec) / 1000;
        fps =  1000000.0f / mtime;
    }
#else

    surface_t *surface = &(surface_t){0};
    #if 0
    surface_wrap(surface, gSurface->pixels, W, H);
    #else
    surface_wrap(surface, (color_t *)gSurface->buffer, W, H);
    #endif
    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &time1);
        #if 0
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                exit(EXIT_SUCCESS);
                break;
            }
        }
        #else
        g_msg_t m;
        while (!g_try_get_msg(&m)) 
        {
            if (g_is_quit_msg(&m))
                goto exit_main;
            /* 有外部消息则处理消息 */
            g_dispatch_msg(&m);
        }
        #endif

        sample(surface, fps);
        #if 0
        SDL_UpdateWindowSurface(gWindow);
        #else
        g_window_paint(gWindow,  0, 0, gSurface);
        #endif
        clock_gettime(CLOCK_MONOTONIC, &time2);
        unsigned long long mtime = (time2.tv_sec - time1.tv_sec) * 1000000 + (time2.tv_nsec - time1.tv_nsec) / 1000;
        fps =  1000000.0f / mtime;
    }
exit_main:
    frambuffer_close();
#endif
    return 0;
}