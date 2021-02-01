#define _XOPEN_SOURCE 600


//#include <SDL2/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <xtk.h>

#include <gato.h>

#if 0
static SDL_Window *gWindow = NULL;
static SDL_Surface *gSurface = NULL;
#else
static xtk_spirit_t *gWindow = NULL;
static xtk_surface_t *gSurface = NULL;

static int mouse_x, mouse_y;

#endif

bool mouse_motion(xtk_spirit_t *spirit, xtk_event_t *event, void *arg)
{
    mouse_x = event->motion.x;
    mouse_y = event->motion.y;
    //printf("%d %d\n", mouse_x, mouse_y);
    return true;
}

static void frambuffer_init()
{
    printf("frambuffer_init.\n");
    
    #if 0
    SDL_Init(SDL_INIT_VIDEO);
    gWindow = SDL_CreateWindow("SDL", 100, 100, W, H, SDL_WINDOW_SHOWN);
    gSurface = SDL_GetWindowSurface(gWindow);
    #else
    if (xtk_init(NULL, NULL) < 0) {
        printf("xtk init failed!\n");
        exit(-1);
    }
    printf("xtk init done!\n");
      
    gWindow = xtk_window_create(XTK_WINDOW_TOPLEVEL);
    if (!gWindow) {
        printf("xtk new window failed!\n");
        xtk_exit(-1);
    }
    xtk_window_set_title(XTK_WINDOW(gWindow), "gato");
    printf("xtk win done!\n");
    
    xtk_window_set_default_size(XTK_WINDOW(gWindow), W, H);
    //xtk_window_set_position(XTK_WINDOW(gWindow), XTK_WIN_POS_CENTER);
    
    xtk_spirit_show(gWindow);

    xtk_signal_connect(gWindow, "motion_notify", mouse_motion, NULL);
    #endif
}

static void frambuffer_close()
{
    #if 0
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    #else
    xtk_exit(0);
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
    #endif
    while (xtk_poll())
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
        #endif
        if (!gSurface) {
            gSurface = xtk_window_get_surface(XTK_WINDOW(gWindow));
            surface_wrap(surface, (color_t *)gSurface->pixels, W, H);
        }
        sample(surface, (float)fps);
        #if 0
        SDL_UpdateWindowSurface(gWindow);
        #else
        xtk_window_flip(XTK_WINDOW(gWindow));
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
    printf("xtk exit\n");
exit_main:
    frambuffer_close();
#endif
    return 0;
}