#ifndef __XLIBC_CRC16_H__
#define __XLIBC_CRC16_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint16_t crc16_sum(uint16_t crc, const uint8_t * buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* __XLIBC_CRC16_H__ */
