#ifndef _XBOOK_BYTEORDER_H
#define _XBOOK_BYTEORDER_H

#include <stdint.h>

static inline uint16_t byte_swap16(uint16_t data)
{
  return (uint16_t)((data & 0xFF) << 8) | ((data & 0xFF00) >> 8);
}

static inline uint32_t byte_swap32(uint32_t data)
{
  return (uint32_t)((data & 0xFF) << 24) | ((data & 0xFF00) << 8) | \
        ((data & 0xFF0000) >> 8) | ((data & 0xFF000000) >> 24);
}

static inline int byte_get_cpu_endian()  
{  
    short s = 0x0110;  
    char *p = (char *) &s;  
    if (p[0] == 0x10)
        return 0;
    else  
        return 1;
}

static inline uint16_t byte_cpu_to_little_endian16(uint16_t data)
{
    if (byte_get_cpu_endian()) {
        return byte_swap16(data);
    } else {
        return data;
    }
}

static inline uint32_t byte_cpu_to_little_endian32(uint32_t data)
{
    if (byte_get_cpu_endian()) {
        return byte_swap32(data);
    } else {
        return data;
    }
}

static inline uint16_t byte_cpu_to_big_endian16(uint16_t data)
{
    if (byte_get_cpu_endian()) {
        return data;
    } else {
        return byte_swap16(data);
    }
}

static inline uint32_t byte_cpu_to_big_endian32(uint32_t data)
{
    if (byte_get_cpu_endian()) {
        return data;
    } else {
        return byte_swap32(data);
    }
}

static inline uint16_t byte_big_endian16_to_cpu(uint16_t data)
{
    if (byte_get_cpu_endian()) {
        return data;
    } else {
        return byte_swap16(data);
    }
}

static inline uint32_t byte_big_endian32_to_cpu(uint32_t data)
{
    if (byte_get_cpu_endian()) {
        return data;
    } else {
        return byte_swap32(data);
    }
}

static inline uint16_t byte_little_endian16_to_cpu(uint16_t data)
{
    if (byte_get_cpu_endian()) {
        return byte_swap16(data);
    } else {
        return data;
    }
}

static inline uint32_t byte_little_endian32_to_cpu(uint32_t data)
{
    if (byte_get_cpu_endian()) {
        return byte_swap32(data);
    } else {
        return data;
    }
}

#endif   /* _XBOOK_BYTEORDER_H */
