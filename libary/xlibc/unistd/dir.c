#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <sys/srvcall.h>
#include <srv/filesrv.h>
#include <sys/drive.h>
#include <sys/dir.h>

/* 任务可以打开的目录数量 */
#define _MAX_DIRDES_NR     32

struct _dirdes __dirdes_table[_MAX_DIRDES_NR] = {{0, -1}, }; 

#if 1
/* default work dir */
static char __current_work_dir[MAX_PATH_LEN];
#endif

/* 获取路径的磁盘符 */
int __get_path_drive(const char *path)
{
    char *p = (char *) path;
    if (ISBAD_DRIVE(*p)) {
        return 0;
    }
    if (*(p + 1) != ':') {  /* 没有分隔符 */
        return 0;
    }
    return *p;
}


/* 将最上层路径名称解析出来 */
/**
 * __parse_path_afterward - 朝后解析路径
 * @path: 路径名
 * @name: 储存名字的地址
 * 
 *  成功返回解析到的位置，失败返回NULL
 */
char *__parse_path_afterward(char *path, char *name)
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
    char name[DIR_NAME_LEN] = {0};    
    char* sub_path = old_path;
    sub_path = __parse_path_afterward(sub_path, name);
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
        memset(name, 0, DIR_NAME_LEN);
        if (sub_path) {
	        sub_path = __parse_path_afterward(sub_path, name);
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
    if (!__get_path_drive(path)) {  /* 为0表示没有磁盘符 */
        /* 获取当前工作目录 */
        if (!getcwd(abspath, MAX_PATH)) {
            //printf("cwd:%s\n", abspath);
            /* 检测当前工作目录是否是合格的目录，也就是说磁盘符后面
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

    /* 想要直接进入磁盘符的根目录'/' */
    if (path[0] == '/' && path[1] == '\0') {
        //printf("will into root dir! path %s\n", abspath);
        /* 移除根目录后面的内容，其实就是在'/'后面添加一个字符串结束 */
        char *q = strchr(abspath, '/');
        if (q != NULL) {
            //printf("sub dir %s\n", q);
            q[1] = '\0';
        }
    } else {
        /* 不是进入根目录。如果是相对路径，就会和工作路径拼合，
        不是的话就是绝对路径。
        */
        strcat(abspath, path);

        /* 只有磁盘符，例如c: , 那么就需要在这个后面添加一个'/' */
        if (strchr(abspath, '/') == NULL) {
            //printf("path %s only drive, add a '/'.\n", pathname);
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
    /* 添加磁盘符 */
    out_path[0] = abs_path[0];
    out_path[1] = abs_path[1];
    __wash_path(p, out_path + 2);
}

int __chdir(char *path)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_CHDIR, 0);
    SETSRV_ARG(&srvarg, 1, path, strlen(path) + 1);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

void __setcwd(char *path)
{
    memset(__current_work_dir, 0, MAX_PATH_LEN);
    strcpy(__current_work_dir, path);
    __current_work_dir[MAX_PATH_LEN - 1] = '\0';
}

int chdir(const char *path)
{
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);

    /* 发起改变目录请求 */
    if (__chdir(full_path) < 0) {
        printf("chdir %s failed!\n", full_path);
        return -1;
    }
        
    /* 设置路径为当前工作目录 */
    memset(__current_work_dir, 0, MAX_PATH_LEN);
    strcpy(__current_work_dir, full_path);
    __current_work_dir[MAX_PATH_LEN - 1] = '\0';
    return 0;
}

int getcwd(char *buf, int bufsz)
{
    /* 检测路径是否合法 */
    memset(buf, 0, bufsz);
    memcpy(buf, __current_work_dir, MIN(MAX_PATH_LEN, bufsz));
    return 0;
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

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_OPENDIR, 0);
    SETSRV_ARG(&srvarg, 1, p, strlen(p) + 1);
    SETSRV_RETVAL(&srvarg, -1);

    /* 文件描述符地址 */
    struct _dirdes *_dir = __alloc_dirdes();
    if (_dir == NULL) {
        return NULL;        
    }
    
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return NULL;
        }
        _dir->diridx = GETSRV_RETVAL(&srvarg, int);
        return _dir;
    }
    return NULL;
}

int closedir(DIR *dir)
{
    if (dir < __dirdes_table || dir >= __dirdes_table + _MAX_DIRDES_NR)
        return -1;

    if (dir->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_CLOSEDIR, 0);
    SETSRV_ARG(&srvarg, 1, dir->diridx, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        __free_dirdes(dir);
        return 0;
    }
    return -1;
}

struct dirent *readdir(DIR *dir)
{
    if (dir < __dirdes_table || dir >= __dirdes_table + _MAX_DIRDES_NR)
        return NULL;

    if (dir->flags == 0)
        return NULL;

    static struct dirent de;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_READDIR, 0);
    SETSRV_ARG(&srvarg, 1, dir->diridx, 0);
    SETSRV_ARG(&srvarg, 2, &de, sizeof(struct dirent));
    SETSRV_IO(&srvarg, (SRVIO_USER << 2));
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return NULL;
        }
        return &de;
    }
    return NULL;
}

int rewinddir(DIR *dir)
{
    if (dir < __dirdes_table || dir >= __dirdes_table + _MAX_DIRDES_NR)
        return -1;
    if (dir->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_REWINDDIR, 0);
    SETSRV_ARG(&srvarg, 1, dir->diridx, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int mkdir(const char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);

    char *p = (char *) full_path;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_MKDIR, 0);
    SETSRV_ARG(&srvarg, 1, p, strlen(p) + 1);
    SETSRV_ARG(&srvarg, 2, mode, 0);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int rmdir(const char *path)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);

    char *p = (char *) full_path;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_RMDIR, 0);
    SETSRV_ARG(&srvarg, 1, p, strlen(p) + 1);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int rename(const char *source, const char *target)
{
    if (source == NULL || target == NULL)
        return -1;
    char full_path0[MAX_PATH] = {0};
    build_path(source, full_path0);
    char full_path1[MAX_PATH] = {0};
    build_path(target, full_path1);

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_RENAME, 0);
    SETSRV_ARG(&srvarg, 1, full_path0, strlen(full_path0) + 1);
    SETSRV_ARG(&srvarg, 2, full_path1, strlen(full_path1) + 1);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}
