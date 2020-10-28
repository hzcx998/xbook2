#include <fsal/dir.h>
#include <string.h>
#include <assert.h>
#include <xbook/kmalloc.h>

/* 目录表指针 */
fsal_dir_t *fsal_dir_table;

/**
 * init_fsal_dir_table - 初始化目录表
 * 
 * 分配目录表内存，并清空
 */
int init_fsal_dir_table()
{
    fsal_dir_table = kmalloc(FSAL_DIR_OPEN_NR * sizeof(fsal_dir_t));
    if (fsal_dir_table == NULL) 
        return -1;
    memset(fsal_dir_table, 0, FSAL_DIR_OPEN_NR * sizeof(fsal_dir_t));
    return 0;
}

/**
 * fsal_dir_alloc - 从表中分配一个目录
 * 
 */
fsal_dir_t *fsal_dir_alloc()
{
    int i;
    for (i = 0; i < FSAL_DIR_OPEN_NR; i++) {
        if (!fsal_dir_table[i].flags) {
            /* 清空目录表的内容 */
            memset(&fsal_dir_table[i], 0, sizeof(fsal_dir_t));
            
            /* 记录使用标志 */
            fsal_dir_table[i].flags = FSAL_DIR_USED;

            return &fsal_dir_table[i];
        }
    }
    return NULL;
}

/**
 * fsal_dir_free - 释放一个目录
 * 
 */
int fsal_dir_free(fsal_dir_t *dir)
{
    if (!dir->flags)
        return -1;
    dir->flags = 0;
    return 0;
}


/* 将最上层路径名称解析出来 */
/**
 * kern_vir_addr2phy_addrrse_path_afterward - 朝后解析路径
 * @path: 路径名
 * @name: 储存名字的地址
 * 
 *  成功返回解析到的位置，失败返回NULL
 */
static char *kern_vir_addr2phy_addrrse_path_afterward(char *path, char *name)
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
 * wash_path - 对路径进行清洗
 * @old_path: 旧的路径
 * @new_path: 新的路径
 * 
 * 转换路径中的.和..，使路径没有这些，并且是一个正确的路径
 * 转换后的路径存放到new_path中
 */
void wash_path(char *old_path, char *new_path)
{
    ASSERT(old_path[0] == '/');
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
static void __make_abs_path(const char *path, char *abspath)
{
    /*
    判断是否有磁盘符，如果有，就说明是绝对路径，不然就是相对路径。
    如果是相对路径，那么就需要读取当前的工作目录
    */
    if (*path != '/') { /* 不是'/'，表明不是绝对路径 */
        /* 获取当前工作目录 */
        if (!sys_getcwd(abspath, MAX_PATH)) {
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
        abspath[0] = '/';
        abspath[1] = '\0';
    } else {
        /* 不是进入根目录。如果是相对路径，就会和工作路径拼合，
        不是的话就是绝对路径。
        */
        strcat(abspath, path);

        /* 没有'/'，那么就需要在这个后面添加一个'/' */
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
    wash_path(p, out_path);
}