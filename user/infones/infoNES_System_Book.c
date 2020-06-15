#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sgi/sgi.h>
#include <sys/time.h>

#include <InfoNES.h>
#include <InfoNES_System.h>
#include <InfoNES_pAPU.h>

//#define CONFIG_SOUND

#ifdef CONFIG_SOUND
#define SOUND_DEVICE "sys:/dev/pcspeaker2"
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

/* 音频控制 */
int soundFd;
int wavflag;

SGI_Window win;
SGI_Display *display;

/* 绘制缓冲区 */
DWORD graphBuffer[NES_DISP_WIDTH*NES_DISP_HEIGHT];

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
    printf("infoNES: too few arguments!\n");  
  }
  //printf("arg error!\n");
  //while(1);
	return 0;	
}

void exit_application()
{
    if (SGI_UnmapWindow(display, win)) {
        printf("[infones] unmap window failed!\n");
        goto exit_end;
    } else {
        printf("[infones] unmap window success!\n");
    }

    if (SGI_DestroyWindow(display, win)) {
        printf("[infones] destroy window failed!\n");
        goto exit_end;
    } else {
        printf("[infones] destroy window success!\n");
    }
    
    SGI_CloseDisplay(display);
    printf("[infones] close display ok!\n");
exit_end:
    exit(0);
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
        exit_application();
    }

    char title[32] = {0};
    sprintf(title, "infones - %s", filename);

    sleep(1);
    
    
    display = SGI_OpenDisplay();
    if (display == NULL) {
        printf("[infones] open gui failed!\n");
        return;
    }
    printf("[infones] open display ok!\n");

    win = SGI_CreateSimpleWindow(
        display,
        display->root_window,
        10,
        100,
        NES_DISP_WIDTH, 
        NES_DISP_HEIGHT,
        0Xffffffff
    );

    if (win < 0) {
        printf("[infones] create window failed!\n");
        return;
    }
    printf("[infones] create window success!\n");

    if (SGI_MapWindow(display, win)) {
        printf("[infones] map window failed!\n");
        return;
    } else {
        printf("[infones] map window success!\n");
    }

    //printf("main ");
    InfoNES_Main();
       
  } else {
      printf("[infones] InfoNES_Load failed!\n");
      exit_application();
  }
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
  fd = open( szSaveName, O_RDONLY );
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
  fd = open( pszFileName, O_RDONLY );
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
  //printf("f");

/*
 *  Transfer the contents of work frame on the screen
 *
 */

  //register guchar* pBuf;

  //pBuf = pbyRgbBuf;

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
    //GUI_DrawBitmapPlus(0, 40, NES_DISP_WIDTH, NES_DISP_HEIGHT, (unsigned int *)graphBuffer);
    SGI_WindowDrawPixmap(display, win, 0, 0, NES_DISP_WIDTH, NES_DISP_HEIGHT, (SGI_Argb *) graphBuffer);
    SGI_UpdateWindow(display, win, 0, 0, NES_DISP_WIDTH, NES_DISP_HEIGHT);

    /* 延迟 */
    clock_t start;
    start = clock();
    while ((clock() - start) < 7);
}

// 按键处理
int PollEvent(void)
{
    SGI_Event event;
    if (SGI_PollEvent(display, &event))
        return -1;
    switch (event.type)
    {
    case SGI_KEY:
        if (event.key.state == SGI_PRESSED) {
            switch (event.key.keycode.code)
			{
			case SGIK_d:
			case SGIK_D:
            	dwKeyPad1 |= (1 << 7);
				break;
			case SGIK_a:
			case SGIK_A:
            
				dwKeyPad1 |= (1 << 6);
				break;
			case SGIK_s:
			case SGIK_S:
            
				dwKeyPad1 |= (1 << 5);
				break;
            case SGIK_w:
			case SGIK_W:
            
				dwKeyPad1 |= (1 << 4);
				break;
			case SGIK_n:
            case SGIK_N:
            
				dwKeyPad1 |= (1 << 3);
				break;			/* Start */
			case SGIK_m:
            case SGIK_M:
            	dwKeyPad1 |= (1 << 2);
				break;			/* Select */
			case SGIK_j:
            case SGIK_J:
            	dwKeyPad1 |= (1 << 1);
				break;			/* 'B' */
			case SGIK_k:
            case SGIK_K:
            	dwKeyPad1 |= (1 << 0);
				break;			/* 'A' */
			case SGIK_c:
            case SGIK_C:
            	/* 切换剪裁 */
				PPU_UpDown_Clip = (PPU_UpDown_Clip ? 0 : 1);
				break;
			case SGIK_v:
            case SGIK_V:
				SaveSRAM();
				break;
			case SGIK_q:
            case SGIK_Q:
                exit_application();
                break;
            default:
                break;
            }
        } else {
            switch (event.key.keycode.code)
			{
			case SGIK_d:
			case SGIK_D:
            	dwKeyPad1 &= ~(1 << 7);
				break;
			case SGIK_a:
			case SGIK_A:
            	dwKeyPad1 &= ~(1 << 6);
				break;
			case SGIK_s:
			case SGIK_S:
            	dwKeyPad1 &= ~(1 << 5);
				break;
			case SGIK_w:
			case SGIK_W:
            	dwKeyPad1 &= ~(1 << 4);
				break;
			case SGIK_n:
            case SGIK_N:
            	dwKeyPad1 &= ~(1 << 3);
				break;			/* Start */
			case SGIK_m:
            case SGIK_M:
            	dwKeyPad1 &= ~(1 << 2);
				break;			/* Select */
			case SGIK_j:
            case SGIK_J:
            	dwKeyPad1 &= ~(1 << 1);
				break;			/* 'B' */
			case SGIK_k:
            case SGIK_K:
            	dwKeyPad1 &= ~(1 << 0);
				break;			/* 'A' */
            default:
                break;
			}					/* 按键松开 */
        }
        break;
    case SGI_QUIT:
        /* 退出 */
        exit_application();
        break;
    default:
        break;
    }
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
    soundFd = 0;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate ) 
{
    
#ifdef CONFIG_SOUND
    soundFd = open(SOUND_DEVICE, O_RDWR);
    if (soundFd < 0) {
        soundFd = 0;
        return 0;
    }
    /* 开始播放声音 */
    ioctl(soundFd, 0, 0);
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
  if (soundFd) {
      close(soundFd);
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
  int i;
  BYTE finalWave;
  wavflag = 1;
  if (soundFd) {
      for (i = 0; i < samples; i++) {
#if 1
    finalWave = ( wave1[i] + wave2[i] + wave3[i] + wave4[i] + wave5[i] ) / 5;
#else   
          /* 读取数据 */
          finalWave = wave4[i];
#endif
        /* 设置频率 */
        if (finalWave) {
            if (wavflag == 1) {
                ioctl(soundFd, 1, 0);    
            }
            wavflag++;
                
            ioctl(soundFd, 4, finalWave + 30);
        } else {
            wavflag = 0;
            ioctl(soundFd, 0, 0);
            
        }
  
      }
      /* 设置频率 */
      //ioctl(soundFd, 4, finalWave);
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
