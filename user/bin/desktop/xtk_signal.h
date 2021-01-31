#ifndef _LIB_XTK_SIGNAL_H
#define _LIB_XTK_SIGNAL_H

#include <stdbool.h>
#include <sys/list.h>
#include "xtk_spirit.h"
#include "xtk_event.h"

#define XTK_SIGNAL_NAME_LEN 32

typedef bool (*xtk_callback_t) (xtk_spirit_t *, void *);
typedef bool (*xtk_callback_event_t) (xtk_spirit_t *, xtk_event_t *,void *);

typedef struct {
    list_t list;
    void *callback;
    void *calldata;
    char name[XTK_SIGNAL_NAME_LEN + 1];  
    void *spirit;    
} xtk_signal_t;

#define XTK_CALLBACK(func) ((void *)(func))

int xtk_signal_create(xtk_spirit_t *spirit, const char *name);

int xtk_signal_destroy(xtk_spirit_t *spirit, const char *name);

int xtk_signal_destroy_all(xtk_spirit_t *spirit);

int xtk_signal_connect(xtk_spirit_t *spirit,
    const char *name,
    void *func,
    void *data);

bool xtk_signal_emit(xtk_spirit_t *spirit,
    int signal_id);

bool xtk_signal_emit_by_name(xtk_spirit_t *spirit,
    const char *name);

bool xtk_signal_emit_arg(xtk_spirit_t *spirit,
    const char *name, void *arg);

#endif /* _LIB_XTK_SIGNAL_H */