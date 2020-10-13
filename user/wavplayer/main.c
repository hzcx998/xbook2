#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <gapi.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;

struct RIFF_BLOCK
{
    char riff[4];
    DWORD riffSize;
    char wave[4];
};

struct FMT_BLOCK
{
    char fmt[4];
    DWORD fmtSize;
    WORD formatTag;
    WORD channels;
    DWORD samplesPerSec;
    DWORD bytesPerSec;
    WORD blockAlign;
    WORD bitsPerSample;
};
struct FACT_BLOCK
{
    char fact[4];
    DWORD factSize;
    DWORD factData;
};
struct DATA_BLOCK
{
    char data[4];
    DWORD dataSize;
};

struct FILE_HEADER
{
    struct RIFF_BLOCK riff_block;
    struct FMT_BLOCK fmt_block;
    struct FACT_BLOCK fact_block;
    struct DATA_BLOCK data_block;
};

#define SOUND_DEVICE "sb16"

#define WAVE_SIZE (1024 * 32)
#define BUF_SIZE (WAVE_SIZE * 1)

static short sbuf[BUF_SIZE];
static int sound_fd = 0;
static short sbuf_half[BUF_SIZE / 2];

static int sound_output(short *buf, int len)
{
    if (sound_fd != -1)
    {
        int t = len / (WAVE_SIZE * sizeof(short));
        short (*s)[WAVE_SIZE] = buf;
        for (int i = 0; i < t; i++)
        {
            if (write(sound_fd, s[i], WAVE_SIZE * sizeof(short)) < WAVE_SIZE * sizeof(short))
            {
                printf("wrote less than WAVE_SIZE\n");
                return -1;
            }
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

#define PRINT(x) printf(#x " = %d\n", x);

int read_wav_file(const char *filename)
{
    struct FILE_HEADER file_header;
    int d;
    FILE *fp = fopen(filename, "rb");

    fread(&file_header, sizeof(struct FILE_HEADER), 1, fp);

    if (strncmp(file_header.riff_block.riff, "RIFF", 4))
    {
        printf("invaild format\n");
        goto exit;
    }

    PRINT(file_header.fmt_block.fmtSize);
    PRINT(file_header.fmt_block.samplesPerSec);
    PRINT(file_header.fmt_block.channels);
    int channels = file_header.fmt_block.channels;

    if (channels == 1)
    {
        for (;;)
        {
            if (fread(sbuf_half, sizeof(sbuf_half), 1, fp) <= 0)
                goto exit;

            for (int i = 0; i < BUF_SIZE / 2; i++)
            {
                short *d = sbuf;
                short *s = sbuf_half;
                d[2 * i + 1] = d[2 * i] = s[i];
            }

            if (sound_output(sbuf, sizeof(sbuf)) == -1)
                goto exit;
        }
    }
    else if (channels == 2)
    {
        for (;;)
        {
            if (fread(sbuf, sizeof(sbuf), 1, fp) <= 0)
                goto exit;
            if (sound_output(sbuf, sizeof(sbuf)) == -1)
                goto exit;
        }
    }
    else
    {
        printf("ERROR\n");
    }
exit:
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        return 1;
    sound_fd = open(SOUND_DEVICE, O_DEVEX, 0);
    read_wav_file(argv[1]);
    close(sound_fd);
    return 0;
}
