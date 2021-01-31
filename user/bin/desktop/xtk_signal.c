#include "xtk.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



int xtk_signal_create(xtk_spirit_t *spirit, const char *name)
{
    xtk_signal_t *sig = (xtk_signal_t *)  malloc(sizeof(xtk_signal_t));
    if (!sig)
        return -1;
    sig->callback = NULL;
    sig->calldata = NULL;
    sig->spirit = NULL;
    memset(sig->name, 0, XTK_SIGNAL_NAME_LEN + 1);   
    strncpy(sig->name, name, XTK_SIGNAL_NAME_LEN);
    list_add_tail(&sig->list, &spirit->signal_list);
    return 0;
}

int xtk_signal_destroy(xtk_spirit_t *spirit, const char *name)
{
    if (!spirit)
        return -1;
    xtk_signal_t *sig;
    list_for_each_owner (sig, &spirit->signal_list, list) {
        if (!strcmp(sig->name, name)) {
            list_del(&sig->list);
            free(sig);
            return 0;
        }
    }
    return -1;
}

int xtk_signal_destroy_all(xtk_spirit_t *spirit)
{
    if (!spirit)
        return -1;
    xtk_signal_t *sig, *next;
    list_for_each_owner_safe (sig, next, &spirit->signal_list, list) {
        list_del(&sig->list);
        free(sig);
    }
    return 0;
}

int xtk_signal_connect(xtk_spirit_t *spirit,
    const char *name,
    void *func,
    void *data)
{
    if (!spirit || !name)
        return -1;
    xtk_signal_t *sig;
    list_for_each_owner (sig, &spirit->signal_list, list) {
        if (!strcmp(sig->name, name)) {
            sig->callback = func;
            sig->calldata = data;
            return 0;
        }
    }
    return -1;
}

bool xtk_signal_emit_by_name(xtk_spirit_t *spirit,
    const char *name)
{
    if (!spirit || !name)
        return false;
    xtk_signal_t *sig;
    list_for_each_owner (sig, &spirit->signal_list, list) {
        if (!strcmp(sig->name, name)) {
            if (sig->callback) {
                xtk_callback_t callback = (xtk_callback_t) sig->callback;
                return callback(spirit, sig->calldata);
            }
            break;
        }
    }
    // printf("waring: xtk signal %x:%s not found!\n", spirit, name);
    return false;
}

/**
 * 发送信号，并传入一个参数
 */
bool xtk_signal_emit_arg(xtk_spirit_t *spirit,
    const char *name, void *arg)
{
    if (!spirit || !name)
        return false;
    xtk_signal_t *sig;
    list_for_each_owner (sig, &spirit->signal_list, list) {
        if (!strcmp(sig->name, name)) {
            if (sig->callback) {
                xtk_callback_event_t callback = (xtk_callback_event_t) sig->callback;
                return callback(spirit, arg, sig->calldata);
            }
            break;
        }
    }
    // printf("waring: xtk signal %x:%s not found!\n", spirit, name);
    return false;
}

bool xtk_signal_emit(xtk_spirit_t *spirit,
    int signal_id)
{
    if (!spirit)
        return false;
    int id = 0;
    xtk_signal_t *sig;
    list_for_each_owner (sig, &spirit->signal_list, list) {
        if (id == signal_id) {
            if (sig->callback) {
                xtk_callback_t callback = (xtk_callback_t) sig->callback;
                return callback(spirit, sig->calldata);
            }
            break;
        }
        id++;
    }
    return false;
}