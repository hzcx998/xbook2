#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <sys/dir.h>
#include <sys/syscall.h>

/* 任务可以打开的目录数量 */
#define _MAX_DIRDES_NR     32

struct _dirdes __dirdes_table[_MAX_DIRDES_NR] = {{0, -1}, }; 

/* 将最上层路径名称解析出来 */
/**
 * kern_vir_addr2phy_addrrse_path_afterward - 朝后解析路径
 * @path: 路径名
 * @name: 储存名字的地址
 * 
 *  成功返回解析到的位置，失败返回NULL
 */
char *kern_vir_addr2phy_addrrse_path_afterward(char *path, char *name)
{
    if (path[0] == '/') {   // 根目录不需要单独解析
        /* 路径中出现1个或多个连续的字符'/',将这些'/'跳过,如"///a/b" */
        while(*(++path) == '/');
    }

    /* 开始一般的路径解析 */
    while (*path != '/' && *path != 0) {
        *name++ = *path++;
    }

    if (path[0] == 0) {   // 若路径字符串为空则返回NULL
        return NULL;
    }
    return path; 
}

/**
 * __wash_path - 对路径进行清洗
 * @old_path: 旧的路径
 * @new_path: 新的路径
 * 
 * 转换路径中的.和..，使路径没有这些，并且是一个正确的路径
 * 转换后的路径存放到new_path中
 */
void __wash_path(char *old_path, char *new_path)
{
    assert(old_path[0] == '/');
    char name[MAX_PATH] = {0};    
    char* sub_path = old_path;
    sub_path = kern_vir_addr2phy_addrrse_path_afterward(sub_path, name);
    if (name[0] == 0) { // 若只有"/",直接将"/"存入new_path后返回 
        new_path[0] = '/';
        new_path[1] = 0;
        return;
    }
    new_path[0] = 0;	   // 避免传给new_path的缓冲区不干净
    strcat(new_path, "/");

    while (name[0]) {
        /* 如果是上一级目录“..” */
        if (!strcmp("..", name)) {
	        char* slash_ptr =  strrchr(new_path, '/');
            /*如果未到new_path中的顶层目录,就将最右边的'/'替换为0,
	        这样便去除了new_path中最后一层路径,相当于到了上一级目录 */
	        if (slash_ptr != new_path) {	// 如new_path为“/a/b”,".."之后则变为“/a”
	            *slash_ptr = 0;
	        } else {	      // 如new_path为"/a",".."之后则变为"/"
                /* 若new_path中只有1个'/',即表示已经到了顶层目录,
	            就将下一个字符置为结束符0. */
	            *(slash_ptr + 1) = 0;
	        }

        } else if (strcmp(".", name)) {	  // 如果路径不是‘.’,就将name拼接到new_path
	        if (strcmp(new_path, "/")) {	  // 如果new_path不是"/",就拼接一个"/",此处的判断是为了避免路径开头变成这样"//"
	            strcat(new_path, "/");
	        }
	        strcat(new_path, name);
        }  // 若name为当前目录".",无须处理new_path

        /* 继续遍历下一层路径 */
        memset(name, 0, MAX_PATH);
        if (sub_path) {
	        sub_path = kern_vir_addr2phy_addrrse_path_afterward(sub_path, name);
        }
    }
}

/**
 * __make_abs_path - 根据路径名生成绝对路径
 * @path: 路径名
 * @abspath: 绝对路径存放的地址
 * 
 * 有可能路径名是相对路径，所以需要进行路径合并处理
 */
void __make_abs_path(const char *path, char *abspath)
{
    /*
    判断是否有磁盘符，如果有，就说明是绝对路径，不然就是相对路径。
    如果是相对路径，那么就需要读取当前的工作目录
    */
    if (*path != '/') { /* 不是'/'，表明不是绝对路径 */
        /* 获取当前工作目录 */
        if (!getcwd(abspath, MAX_PATH)) {
            //printf("cwd:%s\n", abspath);
            /* 检测当前工作目录是否是合格的目录
            必须要有一个'/'，表明是根目录 */
            char *p = strchr(abspath, '/');
            if (p != NULL) {    /* 找到一个'/' */
                if (!((p[0] == '/') && (p[1] == 0))) { /* 在'/'后面还有内容 */
                    strcat(abspath, "/");
                }
            }
            //printf("getcwd done!\n");
        }
    }

    /* 想要直接进入根目录'/' */
    if (path[0] == '/' && path[1] == '\0') {
        //printf("will into root dir!\n");
        abspath[0] = '/';
        abspath[1] = '\0';
    } else {
        /* 不是进入根目录。如果是相对路径，就会和工作路径拼合，
        不是的话就是绝对路径。
        */
        strcat(abspath, path);

        /* 没有'/'，那么就需要在这个后面添加一个'/' */
        if (strchr(abspath, '/') == NULL) {
            printf("path %s only drive, add a '/'.\n", path);
            strcat(abspath, "/");
        }
    }
}

/**
 * build_path - 构建完整的目录
 * @path: 输入路径
 * @out_path: 输出路径
 * 
 * 把传入的一个路径经过转换后，获取一个完整的路径
 * 
 */
void build_path(const char *path, char *out_path)
{
    /* 生成绝对路径测试 */
    char abs_path[MAX_PATH] = {0};
    __make_abs_path(path, abs_path);

    /* 移动到第一个'/'处，也就是根目录处 */
    char *p = strchr(abs_path, '/');
    __wash_path(p, out_path);
}

int chdir(const char *path)
{
    const char *p;
    /* is root dir, do nothing */
    if (!(*path == '/' && *(path+1) == '\0')) {
        char full_path[MAX_PATH] = {0};
        build_path(path, full_path);
        p = (const char *) full_path;
    } else {
        p = path;
    }
    
    return syscall1(int, SYS_CHDIR, p);
}

int getcwd(char *buf, int bufsz)
{
    return syscall2(int, SYS_GETCWD, buf, bufsz);
}

static struct _dirdes *__alloc_dirdes()
{
    int i;
    for (i = 0; i < _MAX_DIRDES_NR; i++) {
        if (__dirdes_table[i].flags == 0) {
            __dirdes_table[i].flags = 1;
            __dirdes_table[i].diridx = -1;
            return &__dirdes_table[i];
        }
    }
    return NULL;
}

static void __free_dirdes(struct _dirdes *_dir)
{
    _dir->flags = 0;
    _dir->diridx = -1;
}

DIR *opendir(const char *path)
{
    if (path == NULL)
        return NULL;

    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);

    char *p = (char *) full_path;

    /* 文件描述符地址 */
    struct _dirdes *_dir = __alloc_dirdes();
    if (_dir == NULL) {
        printf("alloc dirdes failed!\n");
        return NULL;        
    }
    dir_t diridx = syscall1(int, SYS_OPENDIR, p);
    if (diridx < 0) {
        __free_dirdes(_dir);
        return NULL;
    }
    _dir->diridx = diridx;
    return _dir;
}

int closedir(DIR *dir)
{
    if (dir < __dirdes_table || dir >= __dirdes_table + _MAX_DIRDES_NR)
        return -1;

    if (dir->flags == 0)
        return -1;
    
    if (syscall1(int, SYS_CLOSEDIR, dir->diridx) < 0)
        return -1;

    __free_dirdes(dir);
    return -1;
}

static struct dirent __dirent_buf;

struct dirent *readdir(DIR *dir)
{
    if (dir < __dirdes_table || dir >= __dirdes_table + _MAX_DIRDES_NR)
        return NULL;

    if (dir->flags == 0)
        return NULL;
    memset(&__dirent_buf, 0, sizeof(struct dirent));
    if (syscall2(int, SYS_READDIR, dir->diridx, &__dirent_buf) < 0)
        return NULL;
    return &__dirent_buf;
}

int rewinddir(DIR *dir)
{
    if (dir < __dirdes_table || dir >= __dirdes_table + _MAX_DIRDES_NR)
        return -1;
    if (dir->flags == 0)
        return -1;

    return syscall1(int, SYS_REWINDDIR, dir->diridx);
}

int mkdir(const char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);
    const char *p = (const char *) full_path;

    return syscall2(int, SYS_MKDIR, p, mode);
}

int rmdir(const char *path)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);
    const char *p = (const char *) full_path;

    return syscall1(int, SYS_RMDIR, p);
}

int _rename(const char *source, const char *target)
{
    if (source == NULL || target == NULL)
        return -1;
    char full_path0[MAX_PATH] = {0};
    build_path(source, full_path0);
    char full_path1[MAX_PATH] = {0};
    build_path(target, full_path1);

    const char *src = (const char *) full_path0;
    const char *dest = (const char *) full_path1;

    return syscall2(int, SYS_RENAME, src, dest);
}
