/*
 * 1,支持16位单声道/立体声MP3的解码
 * 2,支持CBR/VBR格式MP3解码
 * 3,支持ID3V1和ID3V2标签解析
 * 4,支持所有比特率(MP3最高是320Kbps)解码
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include "mp3common.h"
#include "mp3.h"


#define AUDIO_MIN(x,y)	((x)<(y)? (x):(y))


static int get_file_size(FILE *file)
{
	struct stat buf;
	fstat(file->fd, &buf);
	return buf.st_size;
}

static u32 get_file_offset(FILE *file)
{
	return ftell(file);
}

/*
 * 解析ID3V1
 * buf:输入数据缓存区(大小固定是128字节)
 * pctrl:MP3控制器
 * 返回值:=0,正常,其他,获取失败
 */
u8 mp3_id3v1_decode(u8* buf, MP3CTRL *pctrl) {
	ID3V1_Tag *id3v1tag;
	id3v1tag = (ID3V1_Tag*)buf;
	if (strncmp("TAG", (char*)id3v1tag->id, 3) == 0) {
		//是MP3 ID3V1 TAG
		if (id3v1tag->title[0])
			strncpy((char*)pctrl->title, (char*)id3v1tag->title, 30);
		if (id3v1tag->artist[0])
			strncpy((char*)pctrl->artist, (char*)id3v1tag->artist, 30);
	} else
		return 1;
	return 0;
}
/*
 * 解析ID3V2
 * buf:输入数据缓存区(大小固定是128字节)
 * size:数据大小
 * pctrl:MP3控制器
 * 返回值:=0,正常,其他,获取失败
 */
u8 mp3_id3v2_decode(u8* buf, u32 size, MP3CTRL *pctrl) {
	u8* buff = buf;
	assert(size >= sizeof(ID3V2_TagHead));
	ID3V2_TagHead *taghead = (ID3V2_TagHead*)buf;
	buf += sizeof(ID3V2_TagHead);
	if (strncmp("ID3", taghead->id, 3)) {
		//不存在ID3,mp3数据是从0开始
		pctrl->datastart = 0;
		return 0;
	}
	//得到tag 大小
	//一共四个字节，但每个字节只用7位，最高位不使用恒为0。所以格式如下
	//0xxxxxxx 0xxxxxxx 0xxxxxxx 0xxxxxxx
	//计算大小时要将0 去掉，得到一个28 位的二进制数
	u32 tagsize = ((u32)taghead->size[0] << 21)
				  | ((u32)taghead->size[1] << 14)
				  | ((u16)taghead->size[2] << 7)
				  | taghead->size[3];

	pctrl->datastart = tagsize;		//得到mp3数据开始的偏移量
	if (tagsize > size) {
		//tagsize大于输入bufsize的时候,只处理输入size大小的数据
		tagsize = size;
	}

	if (taghead->mversion < 3) {
		printf("not supported mversion!\r\n");
		return 1;
	}
	while (buf < buff + size) {
		ID3V23_FrameHead *framehead = (ID3V23_FrameHead*)(buf);

		//得到帧大小
		u32 frame_size = ((u32)framehead->size[0] << 24)
						 | ((u32)framehead->size[1] << 16)
						 | ((u32)framehead->size[2] << 8)
						 | framehead->size[3];

		if (strncmp("TT2", framehead->id, 3) == 0
				|| strncmp("TIT2", framehead->id, 4) == 0) {
			//找到歌曲标题帧,不支持unicode格式!!
			strncpy((char*)pctrl->title, buf + sizeof(ID3V23_FrameHead) + 1, AUDIO_MIN(frame_size - 1, MP3_TITSIZE_MAX - 1));
		}
		if (strncmp("TP1", framehead->id, 3) == 0
				|| strncmp("TPE1", framehead->id, 4) == 0) {
			//找到歌曲艺术家帧
			strncpy((char*)pctrl->artist, (char*)(buf + sizeof(ID3V23_FrameHead) + 1), AUDIO_MIN(frame_size - 1, MP3_ARTSIZE_MAX - 1));
		}
		buf += frame_size + sizeof(ID3V23_FrameHead);
	}

	return 0;
}

/*
 * 获取MP3基本信息
 * pname:MP3文件路径
 * pctrl:MP3控制信息结构体
 * 返回值:0,成功
 *     其他,失败
 */
u8 mp3_get_info(FILE *fmp3, u8 *buf, u32 size, MP3CTRL *pctrl) {
	MP3FrameInfo frame_info;
	u32 br = size;
	u8 res = 0;
	u32 p;
	short samples_per_frame;	//一帧的采样个数
	u32 totframes;				//总帧数
	int ret = 0;

	//开始解析ID3V2/ID3V1以及获取MP3信息
	fseek(fmp3, 0, SEEK_SET);
	fread(buf, size, 1, fmp3);

	mp3_id3v2_decode(buf, br, pctrl);			//解析ID3V2数据
	fseek(fmp3, -128, SEEK_END);			//偏移到倒数128的位置
	fread(buf, 128, 1, fmp3);				//读取128字节
	mp3_id3v1_decode(buf, pctrl);				//解析ID3V1数据
	HMP3Decoder decoder = MP3InitDecoder(); 	//MP3解码申请内存
	fseek(fmp3, pctrl->datastart, SEEK_SET);			//偏移到数据开始的地方
	fread(buf, size, 1, fmp3);			//读取5K字节mp3数据
	int offset = MP3FindSyncWord(buf, br);		//查找帧同步信息

	if (offset < 0 || MP3GetNextFrameInfo(decoder, &frame_info, &buf[offset])) {
		res = 0XFE; //未找到同步帧
		printf("未找到同步帧\n");
		goto exit;
	}

	printf("%lu bp/s audio MPEG layer %d, %d Hz sample rate,  %d Chanel , outputSamps:%d.\n",
		   frame_info.bitrate, frame_info.layer, frame_info.samprate, frame_info.nChans, frame_info.outputSamps);
	if ((frame_info.layer < 3) || (frame_info.bitrate <= 0) || (frame_info.samprate <= 0)) {
		//((mp3FrameInfo.bitrate>=320000) 高码率的 inputbuf大小要设大，
		//helix只支持 MP3 解码, .vbr的文件 得到的一些参数莫名其妙
		printf("bitrate or layer not support!\n");
		res = 1;
		goto exit;
	}
	//找到帧同步信息了,且下一帧信息获取正常
	p = offset + 4 + 32;
	MP3_FrameVBRI *fvbri = (MP3_FrameVBRI*)(buf + p);
	if (strncmp("VBRI", fvbri->id, 4) == 0) {
		//存在VBRI帧(VBR格式)
		if (frame_info.version == MPEG1) {
			//MPEG1,layer3每帧采样数等于1152
			samples_per_frame = 1152;
		} else {
			//MPEG2/MPEG2.5,layer3每帧采样数等于576
			samples_per_frame = 576;
		}
		//得到总帧数
		totframes = ((u32)fvbri->frames[0] << 24)
					| ((u32)fvbri->frames[1] << 16)
					| ((u16)fvbri->frames[2] << 8)
					| fvbri->frames[3];

		//得到文件总长度
		pctrl->totsec = totframes * samples_per_frame / frame_info.samprate;
	} else {
		//不是VBRI帧,尝试是不是Xing帧(VBR格式)
		if (frame_info.version == MPEG1) {
			p = frame_info.nChans == 2 ? 32 : 17;
			//MPEG1,layer3每帧采样数等于1152
			samples_per_frame = 1152;
		} else {
			p = frame_info.nChans == 2 ? 17 : 9;
			//MPEG2/MPEG2.5,layer3每帧采样数等于576
			samples_per_frame = 576;
		}

		p += offset + 4;
		MP3_FrameXing *fxing = (MP3_FrameXing*)(buf + p);

		if (strncmp("Xing", (char*)fxing->id, 4) == 0 || strncmp("Info", (char*)fxing->id, 4) == 0) { //是Xing帧
			if (fxing->flags[3] & 0X01) {
				//存在总frame字段
				//得到总帧数
				totframes = ((u32)fxing->frames[0] << 24)
							| ((u32)fxing->frames[1] << 16)
							| ((u16)fxing->frames[2] << 8)
							| fxing->frames[3];

				pctrl->totsec = totframes * samples_per_frame / frame_info.samprate; //得到文件总长度
			} else {
				//不存在总frames字段
				pctrl->totsec = get_file_size(fmp3) / (frame_info.bitrate / 8);
			}
		} else {
			//CBR格式,直接计算总播放时间
			pctrl->totsec = get_file_size(fmp3) / (frame_info.bitrate / 8);
		}
	}
	pctrl->bitrate = frame_info.bitrate;		//得到当前帧的码率
	pctrl->samplerate = frame_info.samprate;	//得到采样率.
	if (frame_info.nChans == 2) {
		//输出PCM数据量大小
		pctrl->outsamples = frame_info.outputSamps;
	} else {
		//输出PCM数据量大小,对于单声道MP3,直接*2,补齐为双声道输出
		pctrl->outsamples = frame_info.outputSamps * 2;
	}
exit:
	MP3FreeDecoder(decoder);//释放内存
	return res;
}

//得到当前播放时间
void mp3_get_curtime(FILE *fmp3, MP3CTRL *mp3x) {
	u32 fpos = 0;
	u32 foffset = get_file_offset(fmp3);
	if (foffset > mp3x->datastart) {
		//得到当前文件播放到的地方
		fpos = foffset - mp3x->datastart;
	}

	//当前播放到第多少秒了?
	mp3x->cursec = fpos * mp3x->totsec / (get_file_size(fmp3) - mp3x->datastart);
}
