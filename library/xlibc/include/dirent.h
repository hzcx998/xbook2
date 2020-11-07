#ifndef _LIB_DIRENT_H
#define _LIB_DIRENT_H
#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>

typedef struct _dirdes {
    int flags;      /* 文件描述符的标志 */
    dir_t diridx;    /* 文件索引 */
} DIR;

DIR *opendir(const char *path);
int closedir(DIR *dir);
struct dirent *readdir(DIR *dir);
int rewinddir(DIR *dir);

#ifdef __cplusplus
}
#endif

#endif  /* _LIB_DIRENT_H */
