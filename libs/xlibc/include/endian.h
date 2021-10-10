#ifndef _LIB_ENDIAN_H
#define _LIB_ENDIAN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions for byte order, according to significance of bytes,
   from low addresses to high addresses.  The value is what you get by
   putting '4' in the most significant byte, '3' in the second most
   significant byte, '2' in the second least significant byte, and '1'
   in the least significant byte, and then writing down one digit for
   each byte, starting with the byte at the lowest address at the left,
   and proceeding to the byte with the highest address at the right.  */

#define	__LITTLE_ENDIAN	1234
#define	__BIG_ENDIAN	4321
#define	__PDP_ENDIAN	3412

#define __BYTE_ORDER __LITTLE_ENDIAN
#define __FLOAT_WORD_ORDER __BYTE_ORDER

# define LITTLE_ENDIAN	__LITTLE_ENDIAN
# define BIG_ENDIAN	__BIG_ENDIAN
# define PDP_ENDIAN	__PDP_ENDIAN
# define BYTE_ORDER	__BYTE_ORDER

#include <bits/byteswap.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN

#ifndef htole16
#define htole16(x) (x)
#endif
#ifndef htole32
#define htole32(x) (x)
#endif
#ifndef htole64
#define htole64(x) (x)
#endif

#ifndef le16toh
#define le16toh(x) (x)
#endif

#ifndef le32toh
#define le32toh(x) (x)
#endif

#ifndef le64toh
#define le64toh(x) (x)
#endif

#else /* __BYTE_ORDER */

#ifndef htole16
#define htole16(x) __bswap_16(x)
#endif
#ifndef htole32
#define htole32(x) __bswap_32(x)
#endif
#ifndef htole64
#define htole64(x) __bswap_64(x)
#endif

#ifndef le16toh
#define le16toh(x) __bswap_16(x)
#endif

#ifndef le32toh
#define le32toh(x) __bswap_32(x)
#endif

#ifndef le64toh
#define le64toh(x) __bswap_64(x)
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif  /* _LIB_ENDIAN_H */
