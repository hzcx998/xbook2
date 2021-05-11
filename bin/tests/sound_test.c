#include "test.h"
#include <math.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#define WAVE_SIZE (44100 / 8)
static short wave[WAVE_SIZE][2];
static int sound_fd = 0;

static void sound_output(int hz)
{
    if (sound_fd != -1)
    {
        for (int i = 0; i < WAVE_SIZE; i++)
        {
            wave[i][1] = wave[i][0] = 30000 * sin(2 * 3.1415926f * hz * i / 44100.0f);
        }
        short(*channel)[WAVE_SIZE] = wave;

        write(sound_fd, channel[0], WAVE_SIZE * sizeof(short));
        write(sound_fd, channel[1], WAVE_SIZE * sizeof(short));
    }
}

void sound(int second, int hz)
{
    int time[9] = {0, 1, 2, 0, 4, 0, 0, 0, 8};
    for (int i = 0; i < time[second]; i++)
        sound_output(hz);
}

static void test()
{
    int f[8] = {0, 494, 554, 622, 659, 740, 881, 932};
    int fh[8] = {0, 988, 1109, 1245, 1318, 1480, 1661, 1865};
    int fhh[8] = {0, 1975, 1109, 1245, 1318, 1480, 1661, 1865};
    int p[4] = {1, 2, 4, 8};
    sound(p[1], f[5]);
    sound(p[1], fh[1]);
    sound(p[1], fh[2]);

    sound(p[1], fh[2]);
    sound(p[1], fh[3]);
    sound(p[1], fh[3]);
    sound(p[1], f[0]);
    sound(p[1], fh[2]);
    sound(p[1], fh[3]);
    sound(p[1], fh[3]);
    sound(p[1], f[0]);

    sound(p[1], fh[2]);
    sound(p[1], fh[3]);
    sound(p[1], fh[5]);
    sound(p[1], fh[3]);
    sound(p[1], fh[1]);
    sound(p[1], f[0]);
    sound(p[1], fh[1]);
    sound(p[1], f[5]);

    sound(p[2], f[6]);
    sound(p[1], f[0]);
    sound(p[1], fh[3]);
    sound(p[2], fh[2]);
    sound(p[1], fh[1]);
    sound(p[1], fh[2]);
    //
    sound(p[3], fh[3]);
    sound(p[1], f[0]);
    sound(p[1], f[5]);
    sound(p[1], fh[1]);
    sound(p[1], fh[2]);

    sound(p[1], fh[2]);
    sound(p[1], fh[3]);
    sound(p[1], fh[3]);
    sound(p[1], f[0]);
    sound(p[1], fh[2]);
    sound(p[1], fh[3]);
    sound(p[1], fh[3]);
    sound(p[1], f[0]);

    sound(p[1], fh[2]);
    sound(p[1], fh[3]);
    sound(p[1], fh[5]);
    sound(p[1], fh[3]);
    sound(p[1], fh[1]);
    sound(p[1], f[0]);
    sound(p[1], fh[1]);
    sound(p[1], f[7]);

    sound(p[2], f[6]);
    sound(p[1], f[0]);
    sound(p[1], fh[3]);
    sound(p[2], fh[2]);
    sound(p[1], fh[1]);
    sound(p[1], fh[2]);

    sound(p[3], fh[1]);
    sound(p[2], f[0]);
    sound(p[1], fh[1]);
    sound(p[1], f[7]);

    sound(p[2], f[6]);
    sound(p[2], fh[1]);
    sound(p[1], fh[2]);
    sound(p[1], f[0]);
    sound(p[1], fh[1]);
    sound(p[1], fh[2]);

    //
    sound(p[1], fh[3]);
    sound(p[1], fh[5]);
    sound(p[1], fh[2]);
    sound(p[1], fh[3]);
    sound(p[1], fh[1]);
    sound(p[1], f[0]);
    sound(p[1], fh[1]);
    sound(p[1], f[7]);

    sound(p[2], f[6]);
    sound(p[2], fh[1]);
    sound(p[1], fh[2]);
    sound(p[0], fh[3]);
    sound(p[0], fh[2]);
    sound(p[1], fh[1]);
    sound(p[1], fh[2]);

    sound(p[3], fh[3]);
    sound(p[2], f[0]);
    sound(p[1], fh[1]);
    sound(p[1], f[7]);

    //
    sound(p[2], f[6]);
    sound(p[1], fh[1]);
    sound(p[1], f[0]);
    sound(p[1], fh[2]);
    sound(p[0], fh[3]);
    sound(p[0], fh[2]);
    sound(p[1], fh[1]);
    sound(p[1], fh[2]);

    sound(p[1], fh[3]);
    sound(p[1], fh[5]);
    sound(p[1], fh[5]);
    sound(p[1], fh[3]);
    sound(p[1], fh[6]);
    sound(p[1], f[0]);
    sound(p[1], fh[3]);
    sound(p[1], fh[2]);

    sound(p[2], fh[1]);
    sound(p[1], f[0]);
    sound(p[1], fh[3]);
    sound(p[1], fh[2]);
    sound(p[1], fh[2]);
    sound(p[1], fh[1]);
    sound(p[1], fh[2]);

    //
    sound(p[3], fh[1]);
    sound(p[1], f[0]);
    sound(p[1], fh[1]);
    sound(p[1], fh[3]);
    sound(p[1], fh[4]);

    sound(p[1], fh[5]);
    sound(p[1], fh[1]);
    sound(p[1], fh[3]);
    sound(p[1], fh[4]);
    sound(p[2], fh[5]);
    sound(p[1], fh[6]);
    sound(p[1], fh[7]);

    sound(p[1], fhh[1]);
    sound(p[1], fh[3]);
    sound(p[1], fh[3]);
    sound(p[1], fh[4]);
    sound(p[2], fh[5]);
    sound(p[2], f[0]);
}

int sound_test(int argc, char *argv[])
{
    sound_fd = open("/dev/sb16", O_RDWR);
    test();
    close(sound_fd);
    return 0;
}