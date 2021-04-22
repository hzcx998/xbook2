#include <xbook/path.h>
#include <string.h>
#include <unistd.h>
#include <xbook/memalloc.h>
#include <xbook/debug.h>
#include <assert.h>
#include <xbook/fs.h>

fsal_path_t *fsal_path_table;
fsal_path_t *fsal_master_path;
DEFINE_SPIN_LOCK(fsal_path_table_lock);

int fsal_path_init()
{
    fsal_path_table = mem_alloc(FSAL_PATH_TABLE_SIZE);
    if (fsal_path_table == NULL) 
        return -1;
    memset(fsal_path_table, 0, FSAL_PATH_TABLE_SIZE);
    fsal_master_path = NULL;
    return 0;
}

int fsal_path_insert(char *devpath, void *path, char *alpath, fsal_t *fsal)
{
    if (devpath == NULL || path == NULL || alpath == NULL || fsal == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(alpath, 0);
    if (fpath)
        return -1;
    fpath = fsal_path_alloc();
    if (fpath == NULL)
        return -1;
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_path_table_lock, irq_flags);
    fpath->fsal = fsal;
    
    memset(fpath->path, 0, FASL_PATH_LEN);
    strcpy(fpath->path, path);
    fpath->path[FASL_PATH_LEN - 1] = '\0';
    
    memset(fpath->alpath, 0, FASL_PATH_LEN);
    strcpy(fpath->alpath, alpath);
    fpath->alpath[FASL_PATH_LEN - 1] = '\0';
    
    memset(fpath->devpath, 0, FASL_PATH_LEN);
    /* 截取设备名 */
    char *p = strrchr(devpath, '/');
    strcpy(fpath->devpath, p != NULL ? (p + 1) : devpath);

    fpath->devpath[FASL_PATH_LEN - 1] = '\0';
    if (fsal_master_path == NULL)
        fsal_master_path = fpath;
    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
    return 0;
}

fsal_path_t *fsal_path_alloc()
{
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_path_table_lock, irq_flags);
    fsal_path_t *fpath;
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal == NULL) {
            spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
            return fpath;
        }
    }
    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
    return NULL;
}

int fsal_path_remove(void *path)
{
    char *p = (char *) path;
    fsal_path_t *fpath;
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_path_table_lock, irq_flags);
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal) {
            /* 检测物理路径 */
            if (!strcmp(p, fpath->path)) {
                fpath->fsal     = NULL;
                memset(fpath->path, 0, FASL_PATH_LEN);
                spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
                return 0;
            }
            /* 尝试检测设备路径 */
            char *q = strrchr(p, '/');
            if (q) {
                q++;
                /* 比较设备名 */
                if (!strcmp(q, fpath->devpath)) {
                    fpath->fsal     = NULL;
                    memset(fpath->path, 0, FASL_PATH_LEN);
                    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
                    return 0;
                }
            }
        }
    }
    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
    return -1;
}

/**
 * 查找路径对应的路径结构
 * @ptype:  路径类型
 * @path: 路径
 * @inmaster: 是否在主路径中去查找
 * @成功返回路径结构地址，失败返回NULL
 */
fsal_path_t *fsal_path_find_with_type(fsal_path_type_t ptype, void *path, int inmaster)
{
    if (ptype < 0 || ptype >= FSAL_PATH_TYPE_NR)
        return NULL;
    char *p = strchr(path, '/');
    if (p == NULL) {
        return NULL;
    }
    if (*(p + 1) == 0 && inmaster)
        return fsal_master_path;
    char name[FASL_PATH_LEN] = {0, };
    p++;
    p = strchr(p, '/');
    if (p) {
        memcpy(name, path, p - (char *)path);
    } else {
        strcpy(name, path);
    }
    char *cmp_name = name;
    char *cmp_path = NULL;
            
    fsal_path_t *fpath;
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_path_table_lock, irq_flags);
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal) {
            switch (ptype) {
            case FSAL_PATH_TYPE_PHYSIC:
                cmp_path = fpath->path;
                break;
            case FSAL_PATH_TYPE_VIRTUAL:
                cmp_path = fpath->alpath;
                break;
            case FSAL_PATH_TYPE_DEVICE:
                cmp_path = fpath->devpath;
                //cmp_name = path; /* 比较名是全路径 */
                break;
            default:
                cmp_name = "unknown path!\n";
                break;
            }
            if (!strcmp(cmp_name, cmp_path)) {
                spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
                return fpath;
            }
        }
    }
    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
    if (inmaster)
        return fsal_master_path;
    return NULL;
}

/**
 * fsal_path_switch - 将抽象路径转换成具体路径
 * fatfs: /root/bin -> 1:/bin
 *      /root -> 1:
 */
int fsal_path_switch(fsal_path_t *fpath, char *new_path, char *old_path)
{
    if (!fpath || !new_path || !old_path)
        return -1;
    char *start = strchr(old_path, '/');
    if (start == NULL)
        return -1;
    strcpy(new_path, fpath->path);
    char *top_level = start;
    start++;
    start = strchr(start, '/');
    if (start == NULL) {
        if (fpath == fsal_master_path && *(top_level + 1) && strcmp(fpath->alpath, top_level)) {
            strcat(new_path, top_level);
        }
        return 0;
    } else {
        if (*(start + 1) == 0) {
            *start = 0;
            if (fpath == fsal_master_path && *(top_level + 1)  && strcmp(fpath->alpath, top_level)) {
                strcat(new_path, top_level);
            }
            return 0;
        }
        if (fsal_path_find(top_level, 0) == NULL) {
            start = top_level;
        }
    }
    strcat(new_path, start);
    return 0;
}

void fsal_path_print()
{
    keprint("%s: fsal path info:\n", FS_MODEL_NAME);
    fsal_path_t *fpath;
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_path_table_lock, irq_flags);
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal > 0) {
            keprint("fasl alpath=%s path=%s fsal=%x\n", fpath->alpath, fpath->path, fpath->fsal);
        }
    }
    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
}


static char *parse_path_afterward(char *path, char *name)
{
    if (path[0] == '/') {
        /* 路径中出现1个或多个连续的字符'/',将这些'/'跳过,如"///a/b" */
        while(*(++path) == '/');
    }
    while (*path != '/' && *path != 0) {
        *name++ = *path++;
    }
    if (path[0] == 0) {
        return NULL;
    }
    return path; 
}

void wash_path(char *old_path, char *new_path)
{
    assert(old_path[0] == '/');
    char name[MAX_PATH] = {0};    
    char* sub_path = old_path;
    sub_path = parse_path_afterward(sub_path, name);
    if (name[0] == 0) {
        new_path[0] = '/';
        new_path[1] = 0;
        return;
    }
    new_path[0] = 0;	   // 避免传给new_path的缓冲区不干净
    strcat(new_path, "/");
    while (name[0]) {
        if (!strcmp("..", name)) {
	        char* slash_ptr =  strrchr(new_path, '/');
	        if (slash_ptr != new_path) {
	            *slash_ptr = 0;
	        } else {
	            *(slash_ptr + 1) = 0;
	        }
        } else if (strcmp(".", name)) {
	        if (strcmp(new_path, "/")) {
	            strcat(new_path, "/");
	        }
	        strcat(new_path, name);
        }
        memset(name, 0, MAX_PATH);
        if (sub_path) {
	        sub_path = parse_path_afterward(sub_path, name);
        }
    }
}

static void make_abs_path(const char *path, char *abspath)
{
    /*
    判断是否有磁盘符，如果有，就说明是绝对路径，不然就是相对路径。
    如果是相对路径，那么就需要读取当前的工作目录
    */
    if (*path != '/') { /* 不是'/'，表明不是绝对路径 */
        /* 获取当前工作目录 */
        if (!kfile_getcwd(abspath, MAX_PATH)) {
            /* 检测当前工作目录是否是合格的目录
            必须要有一个'/'，表明是根目录 */
            char *p = strchr(abspath, '/');
            if (p != NULL) {    /* 找到一个'/' */
                if (!((p[0] == '/') && (p[1] == 0))) { /* 在'/'后面还有内容 */
                    strcat(abspath, "/");
                }
            }
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
        strcat(abspath, "/");   // 添加分割符，避免末尾没有分隔符多余的会在清洗阶段删除
        strcat(abspath, path);
        /* 没有'/'，那么就需要在这个后面添加一个'/' */
        if (strchr(abspath, '/') == NULL) {
            warnprint("path %s only drive, add a '/'.\n", path);
            strcat(abspath, "/");
        }
    }
}

void build_path(const char *path, char *out_path)
{
    char abs_path[MAX_PATH] = {0};
    make_abs_path(path, abs_path);
    char *p = strchr(abs_path, '/');
    wash_path(p, out_path);
}