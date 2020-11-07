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

#include "mp3common.h"
#include "mp3.h"
#include "mp3_buf.h"

#define READBUF_SIZE 1024 * 12		 //4000//4096//4000       // Value must min be 2xMAINBUF_SIZE = 2x1940 = 3880bytes
unsigned char readBuf[READBUF_SIZE]; // Read buffer where data from SD card is read to

int main(int argc, char *argv[])
{
	if (argc != 2)
		return 1;
	char *file_name = argv[1];
	// char *file_name = "res/test.mp3";
	FILE *fp = fopen(file_name, "rb");

	printf("begin to decode %s\n", file_name);
	int Status = MpegAudioDecoder(fp);
	printf("end decode %s\n", file_name);
	if (Status)
	{
		printf("an error occurred during decoding %s.\n", file_name);
	}
	fclose(fp);
	return (Status);
}

int MpegAudioDecoder(FILE *InputFp)
{
	// Content is the output from MP3GetLastFrameInfo,
	// we only read this once, and conclude it will be the same in all frames
	// Maybe this needs to be changed, for different requirements.
	MP3FrameInfo mp3FrameInfo;
	int bytesLeft;			// Saves how many bytes left in readbuf
	unsigned char *readPtr; // Pointer to the next new data
	int offset;				// Used to save the offset to the next frame

	//检查MP3文件格式
	MP3CTRL mp3ctrl; //mp3控制结构体

	u8 rst = mp3_get_info(InputFp, readBuf, READBUF_SIZE, &mp3ctrl);
	if (rst)
		return 1;
	printf("mp3_get_info:\n");

	/* Decode stdin to stdout. */
	printf("title:%s\n", mp3ctrl.title);
	printf("artist:%s\n", mp3ctrl.artist);
	printf("bitrate:%dbps\n", mp3ctrl.bitrate);
	printf("samplerate:%d\n", mp3ctrl.samplerate);
	printf("totalsec:%d\n", mp3ctrl.totsec);
	printf("mp3ctrl.datastart:%x\n", mp3ctrl.datastart);

	fseek(InputFp, mp3ctrl.datastart, SEEK_SET); //跳过文件头中tag信息

	/* Initilizes the MP3 Library */
	// hMP3Decoder: Content is the pointers to all buffers and information for the MP3 Library
	
	printf("MP3InitDecoder:\n");

	HMP3Decoder hMP3Decoder = MP3InitDecoder();
	if (hMP3Decoder == 0)
	{
		// 这意味着存储器分配失败。这通常在堆存储空间不足时发生。
		// 请使用其他堆存储空间重新编译代码。
		printf("MP3 Decoder 初始化失败\n");
		return 1;
	}

	printf("MP3InitDecoder:end\n");

	open_sound();

	while (1)
	{

		bytesLeft = 0;
		readPtr = readBuf;
		printf("#");
		int fres = fread(readBuf, 1, READBUF_SIZE, InputFp);
		if (fres <= 0)
		{
			printf("文件读取失败\n");
			goto exit;
		}

		bytesLeft += fres;
		//printf("1 readBuf = %X,readPtr = %X, bytesLeft %d\n", readBuf, readPtr, bytesLeft);

		while (1)
		{
			printf("#");
			/* find start of next MP3 frame - assume EOF if no sync found */
			int offset = MP3FindSyncWord(readPtr, bytesLeft);
			if (offset < 0)
			{
				break;
			}

			readPtr += offset;	 //data start point
			bytesLeft -= offset; //in buffer
			int errs = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, get_framebuf(), 0);

			if (errs != ERR_MP3_NONE)
			{
				printf("err code %d ,readBuf = %X,readPtr = %X, bytesLeft %d\n", errs, readBuf, readPtr, bytesLeft);
				switch (errs)
				{
				case ERR_MP3_INVALID_FRAMEHEADER:
					printf("INVALID_FRAMEHEADER\n");
					//bytesLeft = 0;
					//readPtr = readBuf;
					//continue;
					goto exit;
					break;
				case ERR_MP3_INDATA_UNDERFLOW:
					printf("INDATA_UNDERFLOW\n");
					goto exit;
					break;
				case ERR_MP3_MAINDATA_UNDERFLOW:
					printf("MAINDATA_UNDERFLOW\n");
					//bytesLeft = READBUF_SIZE;
					//readPtr = readBuf;
					//continue;
					goto exit;
					break;
				case ERR_MP3_FREE_BITRATE_SYNC:
					printf("FREE_BITRATE_SYNC\n");
					goto exit;
					break;
				default:
					printf("ERR\n");
					goto exit;
					break;
				}
			}
			MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);

			if (bytesLeft < MAINBUF_SIZE * 2)
			{
				memmove(readBuf, readPtr, bytesLeft);
				fres = fread(readBuf + bytesLeft, 1, READBUF_SIZE - bytesLeft, InputFp);
				if ((fres <= 0))
				{
					printf("fread exit\n");
					goto exit;
				}

				if (fres < READBUF_SIZE - bytesLeft)
					memset(readBuf + bytesLeft + fres, 0, READBUF_SIZE - bytesLeft - fres);
				bytesLeft = READBUF_SIZE;
				readPtr = readBuf;
			}

			submit_framebuf();
		}
	}

exit:
	close_sound();
	MP3FreeDecoder(hMP3Decoder);
	return 0;
}
