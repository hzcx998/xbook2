#ifndef _SYS_LPC_H /* local procedure call */
#define _SYS_LPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "portcomm.h"

#define FIRST_CALL_CODE  (0x00000001)
#define LAST_CALL_CODE   (0x000FFFFF)

/* the args we support now */
#define LPC_PARCEL_ARG_NR 8

// TODO: add FLOAT & DOUBLE arg support

enum lpc_parcel_arg_type {
    LPC_PARCEL_ARG_NONE = 0,
    LPC_PARCEL_ARG_CHAR,
    LPC_PARCEL_ARG_SHORT,
    LPC_PARCEL_ARG_INT,
    LPC_PARCEL_ARG_LONG,
    LPC_PARCEL_ARG_FLOAT,   // not support
    LPC_PARCEL_ARG_DOUBLE,  // not support
    LPC_PARCEL_ARG_STRING,
    LPC_PARCEL_ARG_SEQUENCE
};

typedef struct {
    uint32_t args[LPC_PARCEL_ARG_NR];   // 参数值
    uint16_t arglen[LPC_PARCEL_ARG_NR]; // 表明参数的长度
    uint16_t argtype[LPC_PARCEL_ARG_NR];// 表明参数的类型
    uint32_t argused;       // 如果某位是1，则表示这个参数已经被使用
    uint32_t nextpos;       // 当参数是字符串或者缓冲区时，记录了下一个缓冲区的位置
} lpc_parcel_header_t;

#define LPC_PARCEL_HEADER_SIZE sizeof(lpc_parcel_header_t)
#define LPC_PARCEL_BUF_SIZE (PORT_MSG_SIZE - LPC_PARCEL_HEADER_SIZE)

typedef struct {
    lpc_parcel_header_t header;
    uint32_t code;      // call code
    uint8_t data[1];    // 柔性数组数据
} *lpc_parcel_t;

typedef bool (*lpc_handler_t)(uint32_t, lpc_parcel_t, lpc_parcel_t);
typedef bool (*lpc_remote_handler_t) (lpc_parcel_t, lpc_parcel_t);

#define LPC_ID_TEST     PORT_COMM_TEST
#define LPC_ID_NET      PORT_COMM_NET
#define LPC_ID_GRAPH    PORT_COMM_GRAPH


lpc_parcel_t lpc_parcel_get();
int lpc_parcel_put(lpc_parcel_t parcel);
void lpc_parcel_dump_args(lpc_parcel_t parcel);
int lpc_parcel_write_string(lpc_parcel_t parcel, char *str);
int lpc_parcel_write_sequence(lpc_parcel_t parcel, void *buf, size_t len);
int lpc_parcel_write_int(lpc_parcel_t parcel, uint32_t num);
int lpc_parcel_write_long(lpc_parcel_t parcel, uint64_t num);
int lpc_parcel_write_short(lpc_parcel_t parcel, uint16_t num);
int lpc_parcel_write_char(lpc_parcel_t parcel, uint8_t num);
int lpc_parcel_read_int(lpc_parcel_t parcel, uint32_t *num);
int lpc_parcel_read_long(lpc_parcel_t parcel, uint64_t *num);
int lpc_parcel_read_short(lpc_parcel_t parcel, uint16_t *num);
int lpc_parcel_read_char(lpc_parcel_t parcel, uint8_t *num);
int lpc_parcel_read_string(lpc_parcel_t parcel, char **str);
int lpc_parcel_read_sequence(lpc_parcel_t parcel, void **buf, size_t *len);
int lpc_echo(uint32_t port, lpc_handler_t func);
int lpc_call(uint32_t port, uint32_t code, lpc_parcel_t data, lpc_parcel_t reply);

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_LPC_H */