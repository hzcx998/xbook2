/*
字序转换，大端字序和小端字序
*/

#ifndef _XBOOK_BYTEORDER_H
#define _XBOOK_BYTEORDER_H

#include "stdint.h"

/**
 * swap16 - 16位的字节交换
 * @data: 需要交换的数据
 * 
 * 返回交换后的数据
 */
static inline uint16_t swap16(uint16_t data)
{
  return (uint16_t)((data & 0xFF) << 8) | ((data & 0xFF00) >> 8);
}

/**
 * swap32 - 32位的字节交换
 * @data: 需要交换的数据
 * 
 * 返回交换后的数据
 */
static inline uint32_t swap32(uint32_t data)
{
  return (uint32_t)((data & 0xFF) << 24) | ((data & 0xFF00) << 8) | \
        ((data & 0xFF0000) >> 8) | ((data & 0xFF000000) >> 24);
}

/**
 * GetEndianness - 获取CPU字序
 * 
 * 是0就是小端，是1就是大端
 */
static inline int get_cpu_endian()  
{  
    short s = 0x0110;  
    char *p = (char *) &s;  
    if (p[0] == 0x10)
        return 0; // 小端格式  
    else  
        return 1; // 大端格式  
}


/**
 * cpu_to_le16 - cpu字序转换成小端字序
 * data: 需要转换的数据
 */
static inline uint16_t cpu_to_le16(uint16_t data)
{
    /* 如果是大端 */
    if (get_cpu_endian()) {
        return swap16(data);   /* 返回交换后的数据 */
    } else {    /* 是小端 */
        return data;    /* 已经是小端了，就不用转换，直接返回 */
    }
}

/**
 * cpu_to_le32 - cpu字序转换成小端字序
 * data: 需要转换的数据
 */
static inline uint32_t cpu_to_le32(uint32_t data)
{
    /* 如果是大端 */
    if (get_cpu_endian()) {
        return swap32(data);   /* 返回交换后的数据 */
    } else {    /* 是小端 */
        return data;    /* 已经是小端了，就不用转换，直接返回 */
    }
}

/**
 * cpu_to_be16 - cpu字序转换成大端字序
 * data: 需要转换的数据
 */
static inline uint16_t cpu_to_be16(uint16_t data)
{
    /* 如果是大端 */
    if (get_cpu_endian()) {
        return data;    /* 已经是大端了，就不用转换，直接返回 */
    } else {    /* 是小端 */
        return swap16(data);   /* 返回交换后的数据 */
    }
}

/**
 * cpu_to_be32 - cpu字序转换成大端字序
 * data: 需要转换的数据
 */
static inline uint32_t cpu_to_be32(uint32_t data)
{
    /* 如果是大端 */
    if (get_cpu_endian()) {
        return data;    /* 已经是大端了，就不用转换，直接返回 */
    } else {    /* 是小端 */
        return swap32(data);   /* 返回交换后的数据 */
    }
}

/**
 * be16_to_cpu - 大端字序转换成cpu字序
 * data: 需要转换的数据
 */
static inline uint16_t be16_to_cpu(uint16_t data)
{
    /* 如果是大端 */
    if (get_cpu_endian()) {
        return data;    /* 已经是大端了，就不用转换，直接返回 */
    } else {    /* 是小端 */
        return swap16(data);   /* 返回交换后的数据 */
    }
}

/**
 * be32_to_cpu - 大端字序转换成cpu字序
 * data: 需要转换的数据
 */
static inline uint32_t be32_to_cpu(uint32_t data)
{
    /* 如果是大端 */
    if (get_cpu_endian()) {
        return data;    /* 已经是大端了，就不用转换，直接返回 */
    } else {    /* 是小端 */
        return swap32(data);   /* 返回交换后的数据 */
    }
}


/**
 * le16_to_cpu - 小端字序转换成cpu字序
 * data: 需要转换的数据
 */
static inline uint16_t le16_to_cpu(uint16_t data)
{
    /* 如果是大端 */
    if (get_cpu_endian()) {
        return swap16(data);   /* 返回交换后的数据 */
    } else {    /* 是小端 */
        return data;    /* 已经是小端了，就不用转换，直接返回 */
    }
}

/**
 * le32_to_cpu - 小端字序转换成cpu字序
 * data: 需要转换的数据
 */
static inline uint32_t le32_to_cpu(uint32_t data)
{
    /* 如果是大端 */
    if (get_cpu_endian()) {
        return swap32(data);   /* 返回交换后的数据 */
    } else {    /* 是小端 */
        return data;    /* 已经是小端了，就不用转换，直接返回 */
    }
}

#endif   /* _XBOOK_BYTEORDER_H */
