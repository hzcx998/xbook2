#ifndef _DWIN_CLIENT_H
#define _DWIN_CLIENT_H

#include <dwin/dwin_config.h>
#include <dwin/layer.h>

enum dwin_client_state
{
    DWIN_CLIENT_INIT = 1,
    DWIN_CLIENT_CONNECT,
    DWIN_CLIENT_DISCONNECT,
};

enum dwin_client_cmd
{
    DWC_CREATE = 1,
    DWC_DELETE,
    DWC_FLUSH,
    DWC_ZORDER,
    DWC_MOVE,
    DWC_RELMOVE,    /* relatively move */
    DWC_RESIZE,
    DWC_CHANGE_PRIORITY,
    DWC_BITBLT,
};

struct dwin_client
{
    dwin_layer_t *layer;
    enum dwin_client_state state;
    int flags;  /* client flags */
};
typedef struct dwin_client dwin_client_t;

union dwin_client_arg
{
    struct {
        unsigned int width;
        unsigned int height;
        int flags;
    } create;
    struct {
        int z;
    } zorder;
    struct {
        int priority;
    } priority;
    struct {
        int x;
        int y;
    } move;
    
    struct {
        int x;
        int y;
    } relmove;
};
typedef union dwin_client_arg dwin_client_arg_t;

int dwin_client_init(dwin_client_t *client, dwin_layer_t *layer);
int dwin_client_detach(dwin_client_t *client);

int dwin_client_open(dwin_client_t *client);
int dwin_client_close(dwin_client_t *client);

int dwin_client_read(dwin_client_t *client, void *buf);
int dwin_client_write(dwin_client_t *client, void *buf);
int dwin_client_ioctl(dwin_client_t *client, int cmd, void *cmd_arg);

void dwin_client_test(void);

#endif   /* _DWIN_CLIENT_H */
