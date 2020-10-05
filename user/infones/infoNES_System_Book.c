#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <gapi.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <InfoNES.h>
#include <InfoNES_System.h>
#include <InfoNES_pAPU.h>

#define CONFIG_SOUND

#ifdef CONFIG_SOUND
#define SOUND_DEVICE_SB16 "sb16"
#define SOUND_DEVICE_BEEP "buzzer"

// #define SOUND_DEVICE_BEEP_UP

#define SOUND_DEVICE SOUND_DEVICE_SB16

#endif /* CONFIG_SOUND */

void start_application( char *filename );
int InfoNES_Load( const char *pszFileName );
void InfoNES_Main();
/*-------------------------------------------------------------------*/
/*  ROM image file information                                       */
/*-------------------------------------------------------------------*/

char szRomName[ 256 ];
char szSaveName[ 256 ];
int nSRAM_SaveFlag;


/* Pad state */
DWORD dwKeyPad1;
DWORD dwKeyPad2;
DWORD dwKeySystem;

int LoadSRAM();
int SaveSRAM();

/* Palette data */
WORD NesPalette[ 64 ] =
{
  0x39ce, 0x1071, 0x0015, 0x2013, 0x440e, 0x5402, 0x5000, 0x3c20,
  0x20a0, 0x0100, 0x0140, 0x00e2, 0x0ceb, 0x0000, 0x0000, 0x0000,
  0x5ef7, 0x01dd, 0x10fd, 0x401e, 0x5c17, 0x700b, 0x6ca0, 0x6521,
  0x45c0, 0x0240, 0x02a0, 0x0247, 0x0211, 0x0000, 0x0000, 0x0000,
  0x7fff, 0x1eff, 0x2e5f, 0x223f, 0x79ff, 0x7dd6, 0x7dcc, 0x7e67,
  0x7ae7, 0x4342, 0x2769, 0x2ff3, 0x03bb, 0x0000, 0x0000, 0x0000,
  0x7fff, 0x579f, 0x635f, 0x6b3f, 0x7f1f, 0x7f1b, 0x7ef6, 0x7f75,
  0x7f94, 0x73f4, 0x57d7, 0x5bf9, 0x4ffe, 0x0000, 0x0000, 0x0000
};

/* For Sound Emulation */
BYTE final_wave[2048];
int waveptr;
int wavflag;
int sound_fd;

g_bitmap_t *screen_bitmap;
int g_win;
/* 绘制缓冲区 */
DWORD graphBuffer[NES_DISP_WIDTH*NES_DISP_HEIGHT];

int win_proc(g_msg_t *msg);

#define DEF_NES_FILE    "/res/nes/mario.nes"

int main(int argc, char **argv)
{
  /*-------------------------------------------------------------------*/
  /*  Pad Control                                                      */
  /*-------------------------------------------------------------------*/

  /* Initialize a pad state */
  dwKeyPad1   = 0;
  dwKeyPad2   = 0;
  dwKeySystem = 0;
  //printf("infoNes \n");
  /* If a rom name specified, start it */
  if ( argc >= 2 )
  {
    printf("infones: arg %s\n", argv[ 1 ]);
    start_application( argv[ 1 ] );
  } else {
    /* 打开图形管理界面 */
    start_application( DEF_NES_FILE);
    printf("infoNES: too few arguments!\n");

  }
  //printf("arg error!\n");
  //while(1);
	return 0;	
}

void exit_application()
{
    InfoNES_Fin(); /* 结束nes */

    /* 应该在退出时检测 */
    g_del_bitmap(screen_bitmap);

    g_quit();   /* 退出图形 */

    exit(-1);   /* 退出进程 */
}

/*===================================================================*/
/*                                                                   */
/*     start_application() : Start NES Hardware                      */
/*                                                                   */
/*===================================================================*/
void start_application( char *filename )
{
    
  /* Set a ROM image name */
  strcpy( szRomName, filename );

  //printf("file name:%s\n", filename);
  //printf("load ");
  /* Load cassette */
  if ( InfoNES_Load ( szRomName ) == 0 )
  { 
    /* Set graphic content */
    //printf("load2 ");
    /* Load SRAM */
    if (LoadSRAM()) {
        return;
    }

    char title[32] = {0};
    sprintf(title, "infones - %s", filename);

    if (g_init() < 0) {
        printf("[infones]: init gui failed!\n");
        return;
    }

    g_win = g_new_window(title, 10, 100, NES_DISP_WIDTH, NES_DISP_HEIGHT, GW_NO_MAXIM);
    if (g_win < 0) {
        printf("[infones] create window failed!\n");
        g_quit();
        return;
    }
    
    screen_bitmap = g_new_bitmap(NES_DISP_WIDTH, NES_DISP_HEIGHT);
    if (screen_bitmap == NULL) {
        printf("[infones] new bitmap failed!\n");
        g_quit();
        return;
    }
    /* 注册消息回调函数 */
    g_set_msg_routine(win_proc);

    g_set_window_icon(g_win, "res/icon/infones.png");

    /* 设置窗口界面 */
    g_show_window(g_win);

    InfoNES_Main();
    
    g_quit();
    
  } else {
      printf("[infones] InfoNES_Load failed!\n");
      return;
  }
}

int win_proc(g_msg_t *msg)
{
    int x, y;
    uint32_t w, h;
    int keycode;
    int win;

    switch (g_msg_get_type(msg))
    {
    case GM_KEY_DOWN:
        keycode = g_msg_get_key_code(msg);
        switch (keycode)
        {
        case GK_d:
        case GK_D:
            dwKeyPad1 |= (1 << 7);
            break;
        case GK_a:
        case GK_A:
        
            dwKeyPad1 |= (1 << 6);
            break;
        case GK_s:
        case GK_S:
        
            dwKeyPad1 |= (1 << 5);
            break;
        case GK_w:
        case GK_W:
        
            dwKeyPad1 |= (1 << 4);
            break;
        case GK_n:
        case GK_N:
        
            dwKeyPad1 |= (1 << 3);
            break;			/* Start */
        case GK_m:
        case GK_M:
            dwKeyPad1 |= (1 << 2);
            break;			/* Select */
        case GK_j:
        case GK_J:
            dwKeyPad1 |= (1 << 1);
            break;			/* 'B' */
        case GK_k:
        case GK_K:
            dwKeyPad1 |= (1 << 0);
            break;			/* 'A' */
        case GK_c:
        case GK_C:
            /* 切换剪裁 */
            PPU_UpDown_Clip = (PPU_UpDown_Clip ? 0 : 1);
            break;
        case GK_v:
        case GK_V:
            SaveSRAM();
            break;
        
        default:
            break;
        }
        break;
    case GM_KEY_UP:
        keycode = g_msg_get_key_code(msg);
        switch (keycode)
        {
        case GK_d:
        case GK_D:
            dwKeyPad1 &= ~(1 << 7);
            break;
        case GK_a:
        case GK_A:
            dwKeyPad1 &= ~(1 << 6);
            break;
        case GK_s:
        case GK_S:
            dwKeyPad1 &= ~(1 << 5);
            break;
        case GK_w:
        case GK_W:
            dwKeyPad1 &= ~(1 << 4);
            break;
        case GK_n:
        case GK_N:
            dwKeyPad1 &= ~(1 << 3);
            break;			/* Start */
        case GK_m:
        case GK_M:
            dwKeyPad1 &= ~(1 << 2);
            break;			/* Select */
        case GK_j:
        case GK_J:
            dwKeyPad1 &= ~(1 << 1);
            break;			/* 'B' */
        case GK_k:
        case GK_K:
            dwKeyPad1 &= ~(1 << 0);
            break;			/* 'A' */
        case GK_q:
        case GK_Q:
            exit_application();
            break;
        default:
            break;
        }					/* 按键松开 */
        break;
    case GM_PAINT:
        win = g_msg_get_target(msg);
        g_get_invalid(win, &x, &y, &w, &h);
        InfoNES_LoadFrame(); /* 重绘 */
        break;
    default:
        break;
    }
    return 0;
}



/*===================================================================*/
/*                                                                   */
/*           LoadSRAM() : Load a SRAM                                */
/*                                                                   */
/*===================================================================*/
int LoadSRAM()
{
/*
 *  Load a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be read
 */

  int fd = -1;
  unsigned char pSrcBuf[ SRAM_SIZE ];
  unsigned char chData;
  unsigned char chTag;
  int nRunLen;
  int nDecoded;
  int nDecLen;
  int nIdx;

  // It doesn't need to save it
  nSRAM_SaveFlag = 0;

  // It is finished if the ROM doesn't have SRAM
  if ( !ROM_SRAM )
    return 0;

  // There is necessity to save it
  nSRAM_SaveFlag = 1;

  // The preparation of the SRAM file name
  strcpy( szSaveName, szRomName );
  strcpy( strrchr( szSaveName, '.' ) + 1, "srm" );

  /*-------------------------------------------------------------------*/
  /*  Read a SRAM data                                                 */
  /*-------------------------------------------------------------------*/

  // Open SRAM file
  fd = open( szSaveName, O_RDONLY , 0);
  if ( fd == -1 ) {
      printf("[infones] open file %s failed!\n", szSaveName);
    return -1;

  }
    
  // Read SRAM data
  read( fd, pSrcBuf, SRAM_SIZE );

  // Close SRAM file
  close( fd );

  /*-------------------------------------------------------------------*/
  /*  Extract a SRAM data                                              */
  /*-------------------------------------------------------------------*/

  nDecoded = 0;
  nDecLen = 0;

  chTag = pSrcBuf[ nDecoded++ ];

  while ( nDecLen < 8192 )
  {
    chData = pSrcBuf[ nDecoded++ ];

    if ( chData == chTag )
    {
      chData = pSrcBuf[ nDecoded++ ];
      nRunLen = pSrcBuf[ nDecoded++ ];
      for ( nIdx = 0; nIdx < nRunLen + 1; ++nIdx )
      {
        SRAM[ nDecLen++ ] = chData;
      }
    }
    else
    {
      SRAM[ nDecLen++ ] = chData;
    }
  }

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*           SaveSRAM() : Save a SRAM                                */
/*                                                                   */
/*===================================================================*/
int SaveSRAM()
{
/*
 *  Save a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be written
 */

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*                  InfoNES_Menu() : Menu screen                     */
/*                                                                   */
/*===================================================================*/
int InfoNES_Menu()
{
/*
 *  Menu screen
 *
 *  Return values
 *     0 : Normally
 *    -1 : Exit InfoNES
 */

  /* Nothing to do here */
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*               InfoNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
int InfoNES_ReadRom( const char *pszFileName )
{
/*
 *  Read ROM image file
 *
 *  Parameters
 *    const char *pszFileName          (Read)
 *
 *  Return values
 *     0 : Normally
 *    -1 : Error
 */

  int fd;

  /* Open ROM file */
  fd = open( pszFileName, O_RDONLY , 0);
  if ( fd == -1 ) {
      printf("[infones] open file %s failed!\n", pszFileName);
    return -1;
  }

  /* Read ROM Header */
  read( fd, &NesHeader, sizeof NesHeader );
  if ( memcmp( NesHeader.byID, "NES\x1a", 4 ) != 0 )
  {
      printf("[infones ] not a NES file!\n");
    /* not .nes file */
    close( fd );
    return -1;
  }

  /* Clear SRAM */
  memset( SRAM, 0, SRAM_SIZE );

  /* If trainer presents Read Triner at 0x7000-0x71ff */
  if ( NesHeader.byInfo1 & 4 )
  {
    read( fd, &SRAM[ 0x1000 ], 512);
  }

  /* Allocate Memory for ROM Image */
  ROM = (BYTE *)malloc( NesHeader.byRomSize * 0x4000 );
    if (ROM == NULL) {
        printf("[infones ] malloc failed!\n");
        close( fd );
        return -1;
    }
  /* Read ROM Image */
  read( fd, ROM, 0x4000*NesHeader.byRomSize);

  if ( NesHeader.byVRomSize > 0 )
  {
    /* Allocate Memory for VROM Image */
    VROM = (BYTE *)malloc( NesHeader.byVRomSize * 0x2000 );
    if (VROM == NULL) {
        printf("[infones ] malloc failed!\n");
        close( fd );
        return -1;
    }
    /* Read VROM Image */
    read( fd, VROM, 0x2000*NesHeader.byVRomSize );
  }

  /* File close */
  close( fd );

  /* Successful */
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*           InfoNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*===================================================================*/
void InfoNES_ReleaseRom()
{
/*
 *  Release a memory for ROM
 *
 */
//printf("release ");
  if ( ROM )
  {
    free( ROM );
    ROM = NULL;
  }

  if ( VROM )
  {
    free( VROM );
    VROM = NULL;
  }
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemoryCopy() : memcpy                         */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemoryCopy( void *dest, const void *src, int count )
{
/*
 *  memcpy
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the copied block's destination
 *
 *    const void *src                  (Read)
 *      Points to the starting address of the block of memory to copy
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to copy
 *
 *  Return values
 *    Pointer of destination
 */

  memcpy( dest, src, count );
  return dest;
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemorySet() : memset                          */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemorySet( void *dest, int c, int count )
{
/*
 *  memset
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the block of memory to fill
 *
 *    int c                            (Read)
 *      Specifies the byte value with which to fill the memory block
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to fill
 *
 *  Return values
 *    Pointer of destination
 */

  memset( dest, c, count);  
  return dest;
}
typedef unsigned char   guchar;

guchar  pbyRgbBuf[ NES_DISP_WIDTH * NES_DISP_HEIGHT * 3 ];


/*===================================================================*/
/*                                                                   */
/*      InfoNES_LoadFrame() :                                        */
/*           Transfer the contents of work frame on the screen       */
/*                                                                   */
/*===================================================================*/
void InfoNES_LoadFrame()
{

/*
 *  Transfer the contents of work frame on the screen
 *
 */

  //register guchar* pBuf;

  //pBuf = pbyRgbBuf;
#if 0
    register DWORD *q = graphBuffer;
    // Exchange 16-bit to 24-bit  
  for ( register int y = 0; y < NES_DISP_HEIGHT; y++ )
  {
    for ( register int x = 0; x < NES_DISP_WIDTH; x++ )
    {  
      WORD wColor = WorkFrame[ ( y << 8 ) + x ];
	  
      *q = (guchar)( ( wColor & 0x7c00 ) >> 7 )<<16;
        *q |= (guchar)( ( wColor & 0x03e0 ) >> 2 )<<8;
        *q |= (guchar)( ( wColor & 0x001f ) << 3 );
        *q |= (0xff << 24);
        q++;
      //*(pBuf++) = (guchar)( ( wColor & 0x7c00 ) >> 7 );
      
      //*(pBuf++) = (guchar)( ( wColor & 0x03e0 ) >> 2 );
      
      //*(pBuf++) = (guchar)( ( wColor & 0x001f ) << 3 );
    }
  }
    /* 绘制到窗口中 */
    g_window_pixmap(g_win, 0, 0, NES_DISP_WIDTH, NES_DISP_HEIGHT, (g_color_t *) graphBuffer);
    g_refresh_window_rect(g_win, 0, 0, NES_DISP_WIDTH, NES_DISP_HEIGHT);
#else
    register DWORD *q = (DWORD *) screen_bitmap->buffer;
    // Exchange 16-bit to 24-bit  
  for ( register int y = 0; y < NES_DISP_HEIGHT; y++ )
  {
    for ( register int x = 0; x < NES_DISP_WIDTH; x++ )
    {  
      WORD wColor = WorkFrame[ ( y << 8 ) + x ];
	  
      *q = (guchar)( ( wColor & 0x7c00 ) >> 7 )<<16;
        *q |= (guchar)( ( wColor & 0x03e0 ) >> 2 )<<8;
        *q |= (guchar)( ( wColor & 0x001f ) << 3 );
        *q |= (0xff << 24);
        q++;
      //*(pBuf++) = (guchar)( ( wColor & 0x7c00 ) >> 7 );
      
      //*(pBuf++) = (guchar)( ( wColor & 0x03e0 ) >> 2 );
      
      //*(pBuf++) = (guchar)( ( wColor & 0x001f ) << 3 );
    }
  }
    /* 已经绘制好内容了，直接绘制到窗口 */
    g_paint_window(g_win, 0, 0, screen_bitmap);
#endif
    
    mdelay(15);
    
}

int PollEvent(void)
{
    g_msg_t msg;
    /* 获取消息，非阻塞式 */
    if (g_try_get_msg(&msg) < 0)
        return -1;
    
    if (g_is_quit_msg(&msg))
        exit_application();

    /* 有外部消息则处理消息 */
    g_dispatch_msg(&msg);
    return 0;
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_PadState( DWORD *pdwKeyPad1, DWORD *pdwPad2, DWORD *pdwSystem )
{
/*
 *  Get a joypad state
 *
 *  Parameters
 *    DWORD *pdwKeyPad1                   (Write)
 *      Joypad 1 State
 *
 *    DWORD *pdwPad2                   (Write)
 *      Joypad 2 State
 *
 *    DWORD *pdwSystem                 (Write)
 *      Input for InfoNES
 *
 */
    /* 如果有多个事件就一直获取 */
    while (!PollEvent());
  /* Transfer joypad state */
  *pdwKeyPad1   = dwKeyPad1;
  *pdwPad2   = dwKeyPad2;
  *pdwSystem = dwKeySystem;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundInit() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundInit( void ) 
{
    sound_fd = 0;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate ) 
{
    
#ifdef CONFIG_SOUND
    printf("InfoNES_SoundOpen: samples_per_sync=%d, sample_rate=%d\n", samples_per_sync, sample_rate);
    sound_fd = open(SOUND_DEVICE, O_DEVEX, 0);
    if (sound_fd < 0) {
        sound_fd = -1;
        return 0;
    }
    #ifdef SOUND_DEVICE_BEEP_UP
    /* 开始播放声音 */
    ioctl(sound_fd, SNDIO_PLAY, 0);
    #endif
#endif
  /* Successful */
  return 1;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundClose( void ) 
{

#ifdef CONFIG_SOUND
  if (sound_fd != -1) {
      #ifdef SOUND_DEVICE_BEEP_UP
      ioctl(sound_fd, SNDIO_STOP, 0);
      #endif
      close(sound_fd);
    }
#endif
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput() : Sound Output 5 Waves           */           
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput( int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5 )
{

#ifdef CONFIG_SOUND
  
  if ( sound_fd != -1) 
  {
    int i;
    for (i = 0; i < samples; i++) 
    {
#if 1
      final_wave[ waveptr ] = 
	( wave1[i] + wave2[i] + wave3[i] + wave4[i] + wave5[i] ) / 5;
#else
      final_wave[ waveptr ] = wave4[i];
#endif


      waveptr++;
      if ( waveptr == 2048 ) 
      {
	waveptr = 0;
	wavflag = 2;
      } 
      else if ( waveptr == 1024 )
      {
	wavflag = 1;
      }
    }
	
    if ( wavflag )
    {
      
      #ifdef SOUND_DEVICE_BEEP_UP
      BYTE *buf = &final_wave[(wavflag - 1) << 10];
      write(sound_fd, buf, 1024);
      int i;
      
      for (i = 0; i < 1024; i++) {
        if (buf[i]) {
            ioctl(sound_fd, SNDIO_SETFREQ, buf[i] * 5);
        } else {
            ioctl(sound_fd, SNDIO_SETFREQ, 20000);  /* 没数据 */
        }
      }
      ioctl(sound_fd, SNDIO_SETFREQ, 20000);  /* 无声音 */
      #else
      if ( write( sound_fd, &final_wave[(wavflag - 1) << 10], 1024) < 1024 ) 
      {
          printf("wrote less than 1024 bytes\n");
      }
      #endif 
      wavflag = 0;
    }
  }
#endif /* CONFIG_SOUND */
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait() 
{
    
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_MessageBox() : Print System Message            */
/*                                                                   */
/*===================================================================*/
void InfoNES_MessageBox( char *pszMsg, ... )
{
  char pszErr[ 1024 ];

	va_list args = (va_list)((char*)(&pszMsg) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(pszErr, pszMsg, args);

  //printf("%s", pszErr);
}

#include <InfoNES.c>
#include <InfoNES_Mapper.c>
#include <InfoNES_pAPU.c>
#include <K6502.c>
