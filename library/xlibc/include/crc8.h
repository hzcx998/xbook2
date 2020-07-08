#ifndef __XLIBC_CRC8_H__
#define __XLIBC_CRC8_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint8_t crc8_sum(uint8_t crc, const uint8_t * buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* __XLIBC_CRC8_H__ */
