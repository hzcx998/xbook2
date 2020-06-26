#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <assert.h>

#include <srv/guisrv.h>
#include <sys/srvcall.h>
#include <sys/proc.h>
#include <sys/res.h>
#include <sgi/sgi.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/dir.h>

#include "cond_sem.h"
#include  <sys/types.h>
#include  <errno.h>

 
#define	NBUFF	 8
#define BUFFSIZE 128
 
struct {	/* data shared by producer and consumer */
  struct {
    char	data[BUFFSIZE];			/* a buffer */
    ssize_t	n;						/* count of #bytes in the buffer */
  } buff[NBUFF];					/* NBUFF of these buffers/counts */
  cond_sem_t nempty, nfull;		/* semaphores, not pointers */
  cond_sem_t writer_mutex, reader_mutex;
} shared;
 
int writer_index = 0, reader_index = 0;
 
int		fd;							/* input file to copy to stdout */
void* produce(void *), *consume(void *);
void* produce_tryP(void *arg);
 
 
int main(int argc, char **argv)
{
    pthread_t tid_produce1, tid_produce2, tid_produce3;
    pthread_t tid_consume1, tid_consume2;
 
    if (argc != 2)
    {
        printf("use <pathname> as pramater \n");
        exit(1);
    }
 
    fd = open(argv[1], O_RDONLY);
    if( fd == -1 )
    {
        printf("cann't open the file\n");
        return -1;
    }
 
    printf("init sem.\n");
        
    cond_sem_init(&shared.writer_mutex, 1);
    cond_sem_init(&shared.reader_mutex, 1);
    cond_sem_init(&shared.nempty, NBUFF);
    cond_sem_init(&shared.nfull, 0);
 
    /*
    pthread_init(&tid_produce1);
    pthread_init(&tid_produce2);
    pthread_init(&tid_produce3);
    pthread_init(&tid_consume1);
    pthread_init(&tid_consume2);
 
    pthread_create(&tid_consume1, NULL, consume);
    pthread_create(&tid_consume2, NULL, consume);
    pthread_create(&tid_produce1, NULL, produce);
    pthread_create(&tid_produce2, NULL, produce);
    pthread_create(&tid_produce3, NULL, produce_tryP);
 
    pthread_start(&tid_consume1, NULL);
    pthread_start(&tid_consume2, NULL);
    pthread_start(&tid_produce1, NULL);
    pthread_start(&tid_produce2, NULL);
    pthread_start(&tid_produce3, NULL);
    */
    printf("create thread.\n");
    
    pthread_create(&tid_produce1, NULL, produce, NULL);
    pthread_create(&tid_produce2, NULL, produce, NULL);
    pthread_create(&tid_produce3, NULL, produce_tryP, NULL);
    pthread_create(&tid_consume1, NULL, consume, NULL);
    pthread_create(&tid_consume2, NULL, consume, NULL);
    
    printf("join thread.\n");
    
    pthread_join(tid_consume1, NULL);
    pthread_join(tid_consume2, NULL);
 
    pthread_join(tid_produce1, NULL);
    pthread_join(tid_produce2, NULL);
    pthread_join(tid_produce3, NULL);

    printf("destroy sem.\n");
    
 /*
    pthread_destroy(tid_consume1);
    pthread_destroy(tid_consume2);
    pthread_destroy(tid_produce1);
    pthread_destroy(tid_produce2);
    pthread_destroy(tid_produce3);
 */
 
    cond_sem_destroy(&shared.writer_mutex);
    cond_sem_destroy(&shared.reader_mutex);
    cond_sem_destroy(&shared.nempty);
    cond_sem_destroy(&shared.nfull);
 
    printf("test end.\n");
    exit(0);
    return 0;
}
 
 
 
void *produce(void *arg)
{
    while( 1 )
    {
        cond_sem_p(&shared.nempty);	/* wait for at least 1 empty slot */
 
        cond_sem_p(&shared.writer_mutex);
 
        shared.buff[writer_index].n =
                read(fd, shared.buff[writer_index].data, BUFFSIZE);
        //printf("read.\n");

        if( shared.buff[writer_index].n <= 0 )
        {
            cond_sem_v(&shared.nfull);
            cond_sem_v(&shared.writer_mutex);
            return NULL;
        }
 
        writer_index = (writer_index+1)%NBUFF;
 
        cond_sem_v(&shared.nfull);
        cond_sem_v(&shared.writer_mutex);
    }
 
    return NULL;
}
 
 
void* produce_tryP(void *arg)
{
    int status;
    while( 1 )
    {
        /* wait for at least 1 empty slot */
        while( 1 )
        {
            status = cond_sem_tryP(&shared.nempty);
            if( status == 0 )
                break;
            else if( status == EAGAIN )
            {
                //usleep(10*1000); //sleep 10 毫秒
                clock_t start = clock();
                while ((clock() - start) < 5)
                {
                }
                continue;
            }
            else
                return NULL;
        }
 
        cond_sem_p(&shared.writer_mutex);
 
        shared.buff[writer_index].n =
                read(fd, shared.buff[writer_index].data, BUFFSIZE);
 
        if( shared.buff[writer_index].n <= 0 )
        {
            cond_sem_v(&shared.nfull);
            cond_sem_v(&shared.writer_mutex);
            return NULL;
        }
 
        writer_index = (writer_index+1)%NBUFF;
 
        cond_sem_v(&shared.nfull);
        cond_sem_v(&shared.writer_mutex);
    }
 
    return NULL;
}

void* consume(void *arg)
{
    while( 1 )
    {
        cond_sem_p(&shared.nfull);
        cond_sem_p(&shared.reader_mutex);
 
        if( shared.buff[reader_index].n <= 0)
        {
            cond_sem_v(&shared.nempty);
            cond_sem_v(&shared.reader_mutex);
            return NULL;
        }

        //printf("write.\n");
        
        /*write(STDOUT_FILENO, shared.buff[reader_index].data,
                shared.buff[reader_index].n);*/
        printf("%s\n", shared.buff[reader_index].data);

        reader_index = (reader_index+1)%NBUFF;
 
        cond_sem_v(&shared.nempty);
        cond_sem_v(&shared.reader_mutex);
    }
 
    return NULL;
}

#if 0

int main(int argc, char *argv[])
{
    printf("hello, test!\n");
    sleep(1);
    
    SGI_Display *display = SGI_OpenDisplay();
    if (display == NULL) {
        printf("[test] open gui failed!\n");
        return -1;
    }
    printf("[test] open display ok!\n");

    SGI_Window win = SGI_CreateSimpleWindow(
        display,
        display->root_window,
        10,
        100,
        320,
        240,
        0XffFAFA55
    );

    if (win < 0) {
        printf("[test] create window failed!\n");
    }
    printf("[test] create window success!\n");

    SGI_SetWMName(display, win, "new title, 123 abc #$");

    static SGI_Argb icon[5*5*4];
    int i;
    for (i = 0; i < 5*5; i++) {
        icon[i] = SGI_RGB(i * 20, i* 15, i* 5);
    }
    SGI_SetWMIcon(display, win, icon, 5, 5);

    if (SGI_MapWindow(display, win)) {
        printf("[test] map window failed!\n");
    } else {
        printf("[test] map window success!\n");
    }
    int x, y;
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 320; x++) {
            SGI_DrawPixel(display, win, x, y, SGIC_RED);
        }
    }
    
    SGI_DrawRect(display, win, 50, 50, 100, 50, SGIC_BLUE);
    SGI_DrawFillRect(display, win, 70, 100, 50, 100, SGIC_GREEN);


    SGI_Argb pixmap[10*10*sizeof(SGI_Argb)];
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 10; x++) {
            pixmap[y * 10 + x] = SGI_ARGB(0xff, x * 10, x * 15, y * 10);
        }
    }
    SGI_DrawPixmap(display, win, 100, 200, 10, 10, pixmap);
    
    SGI_DrawString(display, win, 100, 50, "hello, text!\nabc\n\rdef", 30, SGIC_BLUE);

    if (SGI_SetFont(display, win, SGI_LoadFont(display, "standard-8*16")) < 0) {
        printf("[test] set font failed!\n");
    }

    SGI_DrawString(display, win, 100, 200, "hello, text!\nabc\n\rdef", 30, SGIC_BLUE);
    
    if (SGI_UpdateWindow(display, win, 0, 0, 320, 240))
        printf("[test] update window failed!\n");
    else
        printf("[test] update window success!\n");

    
    printf("[test] window handle %d\n", win);

    SGI_SelectInput(display, win, SGI_ButtonPressMask | SGI_ButtonRleaseMask |
        SGI_KeyPressMask | SGI_KeyRleaseMask | SGI_EnterWindow | SGI_LeaveWindow);

    SGI_Event event;
    SGI_Window event_window;
    while (1) {
        if (SGI_NextEvent(display, &event))
            continue;
        
        event_window = SGI_DISPLAY_EVENT_WINDOW(display);
        // printf("[test] event window %d\n", event_window);
        switch (event.type)
        {
        case SGI_MOUSE_BUTTON:
            if (event.button.state == SGI_PRESSED) {    // 按下
                if (event.button.button == 0) {
                    printf("[test] left button pressed.\n");
                } else if (event.button.button == 1) {
                    printf("[test] middle button pressed.\n");
                } else if (event.button.button == 2) {
                    printf("[test] right button pressed.\n");
                }
            } else {
                if (event.button.button == 0) {
                    printf("[test] left button released.\n");
                } else if (event.button.button == 1) {
                    printf("[test] middle button released.\n");
                } else if (event.button.button == 2) {
                    printf("[test] right button released.\n");
                }
            }
            break;
        case SGI_MOUSE_MOTION:
            if (event.motion.state == SGI_ENTER) {
                printf("[test] mouse enter window motion %d, %d.\n", event.motion.x, event.motion.y);
            } else if (event.motion.state == SGI_LEAVE) {
                printf("[test] mouse leave window motion %d, %d.\n", event.motion.x, event.motion.y);
            } else {
                printf("[test] mouse motion %d, %d.\n", event.motion.x, event.motion.y);
            }
            break;
        case SGI_KEY:
            if (event.key.state == SGI_PRESSED) {
                printf("[test] keyboard key pressed [%x, %c] modify %x.\n", event.key.keycode.code, 
                    event.key.keycode.code, event.key.keycode.modify);
                
                
            } else {
                printf("[test] keyboard key released [%x, %c] modify %x.\n", event.key.keycode.code, 
                    event.key.keycode.code, event.key.keycode.modify);
                if (event.key.keycode.code == SGIK_Q || event.key.keycode.code == SGIK_q) {
                    goto exit_gui;
                }
            }
            break;
        case SGI_QUIT:
            printf("[test] get quit event.\n");
            goto exit_gui;
            break;
        default:
            break;
        }
    }
    sleep(1);
exit_gui:
    
    if (SGI_UnmapWindow(display, win) < 0) {
        printf("[test] unmap window failed!\n");
    } else {
        printf("[test] unmap window success!\n");
    }

    if (SGI_DestroyWindow(display, win) < 0) {
        printf("[test] destroy window failed!\n");
    } else {
        printf("[test] destroy window success!\n");
    }
    
    SGI_CloseDisplay(display);
    printf("[test] close display ok!\n");

    return 0;
}

#endif

#if 0
DIR *sys_list_dir(char* path)
{
    struct dirent *de;
    int i;
    DIR *dir = opendir(path);
    if (dir) {
        while (1) {
            /* 读取目录项 */
            if ((de = readdir(dir)) == NULL)
                break;
            
            if (de->d_attr & DE_DIR) {   /* 是目录，就需要递归扫描 */
                printf("%s/%s\n", path, de->d_name);
                /* 构建新的路径 */
                i = strlen(path);
                sprintf(&path[i], "/%s", de->d_name);
                sys_list_dir(path);

                path[i] = 0;
            } else {    /* 直接列出文件 */
                printf("%s/%s  size=%d\n", path, de->d_name, de->d_size);
            }
        }
        closedir(dir);
        return NULL;
    }
    return dir;
}

int main(int argc, char *argv[])
{
    printf("hello, test!\n");
    printf("this is string: %s %c value:%d %x!\n", "abc", 'A', 123456789, 0x1234abcd);
#if 0
    /* 文件操作测试 */
    int fd = open("c:/gcc", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }

    char *str = "hello, test!";
    int wr = write(fd, str, strlen(str));
    if (wr < 0) {
        printf("write file failed!\n");
    }

    printf("write %d bytes.\n", wr);

    lseek(fd, 0, SEEK_SET);

    char buf[32] = {0};
    read(fd, buf, 12);
    printf("read buf:%s\n", buf);

    if (fsync(fd))
        printf("> fsync failed!\n");

    ftruncate(fd, 5);

    //lseek(fd, 0, SEEK_SET);
    rewind(fd);

    memset(buf, 0, 32);
    printf("read bytes:%d\n", read(fd, buf, 12));

    printf("read buf:%s\n", buf);

    if (fchmod(fd, 0))
        printf("fchmod failed!\n");

    printf("tell file pos:%d fsize:%d\n", tell(fd), _size(fd));

    close(fd);

    char path[MAX_PATH] = {0};
    strcpy(path, "c:");
    sys_list_dir(path);

    /*
    if (mkfs("ram0", "fat16", 0)) {
        printf("make fs failed!\n");
    }*/

    if (mount("ram0", "d:", "fat16", 0)) {
        printf("mount fs failed!\n");
    }

    fd = open("d:/test", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }

    char *str2 = "hello, ram!\n";
    write(fd, str2, strlen(str2));

    rewind(fd);

    memset(buf, 0, 32);

    read(fd, buf, 32);

    printf("buf:%s\n", buf);

    fsync(fd);
    close(fd);

    memset(path, 0, MAX_PATH);
    strcpy(path, "d:");
    sys_list_dir(path);

    if (unmount("d:",  0)) {
        printf("unmount fs failed!\n");
    }

    fd = open("d:/test", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file after unmount failed!\n");
    }
#endif


    char cwd[32];
    getcwd(cwd, 32);
    printf("cwd:%s\n", cwd);

    /* 生成绝对路径测试 */
    char abs_path[MAX_PATH] = {0};
    build_path("c:/abc", abs_path);
    printf("path:%s\n", abs_path);

    build_path("/abc", abs_path);
    printf("path:%s\n", abs_path);
    
    if (mkdir("c:/xbook", 0))
        printf("mk root failed!\n");

    chdir("c:/xbook");

    build_path("def", abs_path);
    printf("path:%s\n", abs_path);
    
    build_path("./abc", abs_path);
    printf("path:%s\n", abs_path);
    
    if (mkdir("c:/xbook/log", 0))
        printf("mk xbook/log failed!\n");

    chdir("c:/xbook/log");
    
    build_path("../abc/def", abs_path);
    printf("path:%s\n", abs_path);
    
    build_path("/abc/", abs_path);
    printf("path:%s\n", abs_path);
    

    build_path("abc/", abs_path);
    printf("path:%s\n", abs_path);


    /* 文件操作测试 */
    int fd = open("nasm", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }

    char *str = "hello, nasm!";
    int wr = write(fd, str, strlen(str));
    if (wr < 0) {
        printf("write file failed!\n");
    }

    printf("write %d bytes.\n", wr);

    lseek(fd, 0, SEEK_SET);

    char buf[32] = {0};
    read(fd, buf, 12);
    printf("read buf:%s\n", buf);

    printf("tell file pos:%d fsize:%d\n", tell(fd), _size(fd));

    close(fd);

    char path[MAX_PATH] = {0};
    strcpy(path, "c:");
    sys_list_dir(path);

#if 0
    if (mkdir("c:/tmp", 0) < 0)
        printf("mkdir failed!\n");

    if (mkdir("c:/tmp/test", 0) < 0)
        printf("mkdir failed!\n");

    if (mkdir("c:/share", 0) < 0)
        printf("mkdir failed!\n");

    memset(path, 0, MAX_PATH);
    strcpy(path, "c:");
    sys_list_dir(path);

    if (rmdir("c:/share"))
        printf("rmdir share failed!\n");
    
    if (unlink("c:/tmp"))  
        printf("unlink tmp failed!\n");

    if (unlink("c:/tmp/test"))  
        printf("unlink tmp/test failed!\n");
    
    if (unlink("c:/tmp"))  
        printf("unlink tmp failed!\n");

    if (unlink("c:/gcc"))  
        printf("unlink gcc failed!\n");


    if (rename("c:/tmp", "c:/tmp2"))  
        printf("rename tmp failed!\n");

    if (rename("c:/tmp2/test", "c:/tmp2/app"))  
        printf("rename tmp2/test failed!\n");

    if (rename("c:/null", "c:/dev"))  
        printf("rename null failed!\n");

    const struct utimbuf utimebuf = {0, 0x12345678};
    if (utime("c:/gcc", &utimebuf))
        printf("utime failed!\n");

    if (chmod("c:/gcc", 0))
        printf("chmod failed!\n");

    struct stat sbuf;
    memset(&sbuf, 0, sizeof(struct stat));
    if (stat("c:/gcc", &sbuf) < 0)
        printf("stat failed!\n");

    printf("state: size=%d, date=%x, time=%x, attr=%x\n", 
        sbuf.st_size, sbuf.st_date, sbuf.st_time, sbuf.st_attr);




    //sys_list_dir(path);

    fd = open("c:/gcc", O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }
    printf("ready read file:\n");
    char ch;
    while (!_eof(fd)) {
        read(fd, &ch, 1);
        printf("%c", ch);
    }
    close(fd);
#endif






    int i = 0;
    
    char cha;
    while ((cha = getchar()) != '\n')
    {
        putchar(cha);
    }

    return 0;
}
#endif