#define _XOPEN_SOURCE 600


//#include <SDL2/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <gapi.h>

#include <gato.h>

#if 0
static SDL_Window *gWindow = NULL;
static SDL_Surface *gSurface = NULL;
#else
static int gWindow = -1;
static g_bitmap_t *gSurface = NULL;

static int mouse_x, mouse_y;
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
        
    gWindow = g_new_window("gato", 100, 100, W, H, GW_NO_MAXIM);
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

static void motion_get_xy(int *x, int *y)
{
    *x = mouse_x;
    *y = mouse_y;
}


// #define PROFILE

#ifdef PROFILE
#include <gperftools/profiler.h>
#endif

int main(int argc, char *argv[])
{
    #if 0
    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};
    #endif
    struct timeval time1 = {0, 0};
    struct timeval time2 = {0, 0};
    int fps = 0;
    gettimeofday(&time1, NULL);

#ifdef PROFILE
    ProfilerStart("test.prof"); //开启性能分析
    atexit(ProfilerStop);
#endif
    frambuffer_init();
    // float fps = 0;

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
        #if 0
        clock_gettime(CLOCK_MONOTONIC, &time1);
        #endif
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
        while (g_try_get_msg(&m)) 
        {
            if (g_is_quit_msg(&m))
                goto exit_main;
            switch (g_msg_get_type(&m))
            {
            case GM_MOUSE_MOTION:
                mouse_x = g_msg_get_mouse_x(&m);
                mouse_y = g_msg_get_mouse_y(&m);
                break;
            default:
                break;
            }
        }
        #endif

        sample(surface, (float)fps);
        #if 0
        SDL_UpdateWindowSurface(gWindow);
        #else
        g_paint_window(gWindow,  0, 0, gSurface);
        #endif
        fps++;
        gettimeofday(&time2, NULL);
        unsigned long long mtime = (time2.tv_sec - time1.tv_sec) * 1000000 + (time2.tv_usec - time1.tv_usec);
        if (mtime > 1000000) {
            printf("fps %d\n", fps);
            fps = 0;
            time1 = time2;
        }
        #if 0
        clock_gettime(CLOCK_MONOTONIC, &time2);
        unsigned long long mtime = (time2.tv_sec - time1.tv_sec) * 1000000 + (time2.tv_nsec - time1.tv_nsec) / 1000;
        fps =  1000000.0f / mtime;
        #endif
    }
exit_main:
    frambuffer_close();
#endif
    return 0;
}