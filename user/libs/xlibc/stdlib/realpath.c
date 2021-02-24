#include <stdlib.h>
#include <stddef.h>
#include <sys/dir.h>
#include <errno.h>

char *realpath(const char *path, char *resolved_path)
{
    if (!path) {
        _set_errno(EINVAL);
        return NULL;       
    }
    if (!resolved_path) {
        resolved_path = malloc(MAX_PATH);
        if (!resolved_path) {
            _set_errno(ENOMEM);
            return NULL;
        }
    }
    build_path(path, resolved_path);
    return resolved_path;
}
