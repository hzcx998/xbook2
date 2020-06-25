#ifndef _SYS_DIR_H
#define _SYS_DIR_H

#define ROOT_DIR_BUF    "c:/"

void build_path(const char *path, char *out_path);
void __setcwd(char *path);

#endif   /* _SYS_DIR_H */