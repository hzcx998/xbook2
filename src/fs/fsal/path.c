#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <string.h>
#include <unistd.h>
#include <xbook/memalloc.h>
#include <xbook/debug.h>

/* 路径转换表 */
fsal_path_t *fsal_path_table;

/* 主路径 */
fsal_path_t *fsal_master_path;

// #define DEBUG_FSPATH

/**
 * init_fsal_path_table - 初始化路径转换表
 * 
 * 分配路径表内存，并清空
 */
int init_fsal_path_table()
{
    fsal_path_table = mem_alloc(FSAL_PATH_TABLE_SIZE);
    if (fsal_path_table == NULL) 
        return -1;
    memset(fsal_path_table, 0, FSAL_PATH_TABLE_SIZE);
    fsal_master_path = NULL;
    return 0;
}

int fsal_path_insert(void *path, char *alpath, fsal_t *fsal)
{
    /* 参数检测 */
    if (path == NULL || alpath == NULL || fsal == NULL)
        return -1;

    /* 插入新路径的时候，不去主路径中查找 */
    fsal_path_t *fpath = fsal_path_find(alpath, 0);
    if (fpath) /* 不能挂载一个已经存在的路径 */
        return -1;
    
    /* 分配一个 */
    fpath = fsal_path_alloc();
    if (fpath == NULL)
        return -1;

    /* 填写内容 */
    fpath->fsal = fsal;
    strcpy(fpath->path, path); /* 复制路径 */
    fpath->path[strlen(path)] = '\0';
    
    strcpy(fpath->alpath, alpath); /* 复制路径 */
    fpath->alpath[strlen(alpath)] = '\0';
    
    /* 第一个挂载的路径就是主路径 */
    if (fsal_master_path == NULL)
        fsal_master_path = fpath;
    return 0;
}

fsal_path_t *fsal_path_alloc()
{
    /* 比较是否已经在表中 */
    fsal_path_t *fpath;
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal == NULL) {
            return fpath;
        }
    }
    return NULL;
}

int fsal_path_remove(void *path)
{
    char *p = (char *) path;
    /* 比较是否已经在表中 */
    fsal_path_t *fpath;
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal) {
            if (!strcmp(p, fpath->path)) {
                /* 已经在表中，不能再插入 */
                fpath->fsal     = NULL;
                memset(fpath->path, 0, FASL_PATH_LEN);
                return 0;
            }
        }
    }
    return -1;
}

/**
 * 查找路径对应的路径结构
 * 
 * @alpath: 抽象层路径
 * @inmaster: 是否在主路径中去查找
 * 
 * @成功返回路径结构地址，失败返回NULL
 */
fsal_path_t *fsal_path_find(void *alpath, int inmaster)
{
    /* 隔离出第一层的路径 */
    char *p = strchr(alpath, '/');
    if (p == NULL) {    /* 尝试在主文件系统中查找 */
        return NULL;
    }
    /* 如果只有/根目录，就返回主路径 */
    if (*(p + 1) == 0 && inmaster)
        return fsal_master_path;
    
    /* name保存的是挂载的目录的名字 */
    char name[FASL_PATH_LEN] = {0, };
    p++;
    p = strchr(p, '/'); /* 匹配一个子目录 */
    if (p) {    /* 找到子目录，就把第1层的内容截取 */
        memcpy(name, alpath, p - (char *)alpath);
    } else {
        strcpy(name, alpath);   /* 没有子目录，直接就是第一层目录 */
    }

    fsal_path_t *fpath;
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal) {
            if (!strcmp(name, fpath->alpath)) {
                return fpath;
            }
        }
    }
    /* 没找到路径，就尝试到主路径去寻找 */
    if (inmaster)
        return fsal_master_path;

    return NULL;
}

/**
 * fsal_path_switch - 将抽象路径转换成具体路径
 * 
 * fatfs: /root/bin -> 1:/bin
 *      /root -> 1:
 * 
 */
int fsal_path_switch(fsal_path_t *fpath, char *new_path, char *old_path)
{
    if (!fpath || !new_path || !old_path)
        return -1;

    #ifdef DEBUG_FSPATH
    srvprint("old path: %s\n", old_path);
    #endif

    /* 根目录树 /root, /c, /dvd */
    char *start = strchr(old_path, '/');
    if (start == NULL)
        return -1;
    
    /* 复制具体文件系统路径 */
    strcpy(new_path, fpath->path);
    char *top_level = start;    /* 记录顶层名字 */
    start++;
    
    /* 获取文件系统主目录 /root/bin /c/bin -> /bin */
    start = strchr(start, '/');
    if (start == NULL) {    /* 只有磁盘主目录,/root */
        #ifdef DEBUG_FSPATH
        srvprint("only / for path %s\n", new_path);
        #endif
        /* 判断是否为主路径，并且顶层后面有名字，而且这个名字和主目录的名字不一样，才是主目录下面的其它目录 */
        if (fpath == fsal_master_path && *(top_level + 1) && strcmp(fpath->alpath, top_level)) {
            #ifdef DEBUG_FSPATH
            srvprint("%s not master path %s, append to tail.\n", top_level, fpath->alpath);
            #endif
            /* 追加后续路径 */
            strcat(new_path, top_level);
        }
        #ifdef DEBUG_FSPATH
        srvprint("final path %s\n", new_path);
        #endif
        
        return 0;
    } else {    /* 后面有其它路径，/root/ or /root/abc */
        if (*(start + 1) == 0) {    /* 后面只有1个'/' */
            *start = 0; /* 截断 */
            #ifdef DEBUG_FSPATH
            srvprint("only / 2 for path %s\n", new_path);
            #endif
            /* 判断是否为主路径，并且顶层后面有名字，而且这个名字和主目录的名字不一样，才是主目录下面的其它目录 */
            if (fpath == fsal_master_path && *(top_level + 1)  && strcmp(fpath->alpath, top_level)) {
                #ifdef DEBUG_FSPATH
                srvprint("%s not master path %s, append to tail.\n", top_level, fpath->alpath);
                #endif
                /* 追加后续路径 */
                strcat(new_path, top_level);
            }
            #ifdef DEBUG_FSPATH
            srvprint("final path %s\n", new_path);
            #endif
            
            return 0;
        }

        /* 如果顶层路径不是挂载目录的话，那么就把起始位置设置成顶层路径
        如果已经是挂载路径了，那么就沿用当前已经解析的位置 */
        if (fsal_path_find(top_level, 0) == NULL) {
            #ifdef DEBUG_FSPATH
            srvprint("not a mount path %s\n", top_level);
            #endif
            start = top_level;
        }
        
    }
    /* /root/ or /root/abc */
    #ifdef DEBUG_FSPATH
    srvprint("real path %s.\n", start);
    #endif
#if 0    
    start++;
    if (*start == 0) {   /* xx: or xx 后面没有字符 */
        strcat(new_path, "/");
    }
#endif

    /* 复制文件路径内容 */
    strcat(new_path, start);
    #ifdef DEBUG_FSPATH
    srvprint("final path %s\n", new_path);
    #endif
    return 0;
}

void fsal_path_print()
{
    printk("%s: fsal path info:\n", FS_MODEL_NAME);
    fsal_path_t *fpath;
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal > 0) {
            printk("fasl alpath=%s path=%s fsal=%x\n", fpath->alpath, fpath->path, fpath->fsal);
        }
    }
}
