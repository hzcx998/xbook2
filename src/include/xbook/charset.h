#ifndef _XBOOK_CHARSET_H
#define _XBOOK_CHARSET_H

#include <stdint.h>
#include <stddef.h>

#define UINT8_1_LEADINGBIT    0x80
#define UINT8_2_LEADINGBITS   0xc0
#define UINT8_3_LEADINGBITS   0xe0
#define UINT8_4_LEADINGBITS   0xf0
#define UINT8_5_LEADINGBITS   0xf8
#define UINT8_6_LEADINGBITS   0xfc
#define UINT8_7_LEADINGBITS   0xfe

#define UINT8_1_TRAILINGBIT   0x01
#define UINT8_2_TRAILINGBITS  0x03
#define UINT8_3_TRAILINGBITS  0x07
#define UINT8_4_TRAILINGBITS  0x0f
#define UINT8_5_TRAILINGBITS  0x1f
#define UINT8_6_TRAILINGBITS  0x3f

#define MAX_UTF8_PER_UTF16    4
/*
 * You need at least one UTF-8 byte to have one UTF-16 word
 * You need at least three UTF-8 bytes to have 2 UTF-16 words (surrogate pairs)
 */
#define MAX_UTF16_PER_UTF8      1
#define MAX_UTF8_PER_CODEPOINT  4

#define UCS2_LIMIT 0x10000
#define UTF16_UPPER_SURROGATE(code) (0xd800 | ((((code) - UCS2_LIMIT) >> 10) & 0x3ff))
#define UTF16_LOWER_SURROGATE(code) (0xdc00 | (((code) - UCS2_LIMIT) & 0x3ff))

/*
 * Process one character from UTF8 sequence
 * At beginning set *code = 0, *count = 0. Returns 0 on failure and
 * 1 on success. *count holds the number of trailing bytes
 */
static inline int utf8_process(uint8_t c, uint32_t *code, int *count)
{
    if (*count)
    {
        if ((c & UINT8_2_LEADINGBITS) != UINT8_1_LEADINGBIT)
        {
            *count = 0;
            /* invalid */
            return 0;
        }
        else
        {
            *code <<= 6;
            *code |= (c & UINT8_6_TRAILINGBITS);
            (*count)--;
            /* Overlong */
            if ((*count == 1 && *code <= 0x1f) || (*count == 2 && *code <= 0xf))
            {
                *code = 0;
                *count = 0;
                return 0;
            }
            return 1;
        }
    }

    if ((c & UINT8_1_LEADINGBIT) == 0)
    {
        *code = c;
        return 1;
    }
    if ((c & UINT8_3_LEADINGBITS) == UINT8_2_LEADINGBITS)
    {
        *count = 1;
        *code = c & UINT8_5_TRAILINGBITS;
        /* Overlong */
        if (*code <= 1)
        {
            *count = 0;
            *code = 0;
            return 0;
        }
        return 1;
    }
    if ((c & UINT8_4_LEADINGBITS) == UINT8_3_LEADINGBITS)
    {
        *count = 2;
        *code = c & UINT8_4_TRAILINGBITS;
        return 1;
    }
    if ((c & UINT8_5_LEADINGBITS) == UINT8_4_LEADINGBITS)
    {
        *count = 3;
        *code = c & UINT8_3_TRAILINGBITS;
        return 1;
    }
    return 0;
}

/*
 * Convert a (possibly null-terminated) UTF-8 string of at most SRCSIZE
 * bytes (if SRCSIZE is -1, it is ignored) in length to a UTF-16 string
 *
 * Return the number of characters converted. DEST must be able to hold
 * at least DESTSIZE characters. If an invalid sequence is found, return -1
 *
 * If SRCEND is not NULL, then *SRCEND is set to the next byte after the
 * last byte used in SRC.
 */
static inline size_t utf8_to_utf16(uint16_t *dest, size_t destsize, const uint8_t *src, size_t srcsize, const uint8_t **srcend)
{
    uint16_t *p = dest;
    int count = 0;
    uint32_t code = 0;

    if (srcend)
    {
        *srcend = src;
    }

    while (srcsize && destsize)
    {
        int was_count = count;
        if (srcsize != (size_t)-1)
        {
            --srcsize;
        }
        if (!utf8_process(*src++, &code, &count))
        {
            code = '?';
            count = 0;
            /* Character c may be valid, don't eat it */
            if (was_count)
            {
                src--;
            }
        }
        if (count != 0)
        {
            continue;
        }
        if (code == 0)
        {
            break;
        }
        if (destsize < 2 && code >= UCS2_LIMIT)
        {
            break;
        }
        if (code >= UCS2_LIMIT)
        {
            *p++ = UTF16_UPPER_SURROGATE(code);
            *p++ = UTF16_LOWER_SURROGATE(code);
            destsize -= 2;
        }
        else
        {
            *p++ = code;
            destsize--;
        }
    }

    if (srcend)
    {
        *srcend = src;
    }

    return p - dest;
}

/* Determine the last position where the UTF-8 string [beg, end) can be safely cut */
static inline size_t getend(const char *beg, const char *end)
{
    const char *ptr;

    for (ptr = end - 1; ptr >= beg; --ptr)
    {
        if ((*ptr & UINT8_2_LEADINGBITS) != UINT8_1_LEADINGBIT)
        {
            break;
        }
    }

    if (ptr < beg)
    {
      return 0;
    }
    if ((*ptr & UINT8_1_LEADINGBIT) == 0)
    {
        return ptr + 1 - beg;
    }
    if ((*ptr & UINT8_3_LEADINGBITS) == UINT8_2_LEADINGBITS && ptr + 2 <= end)
    {
        return ptr + 2 - beg;
    }
    if ((*ptr & UINT8_4_LEADINGBITS) == UINT8_3_LEADINGBITS && ptr + 3 <= end)
    {
        return ptr + 3 - beg;
    }
    if ((*ptr & UINT8_5_LEADINGBITS) == UINT8_4_LEADINGBITS && ptr + 4 <= end)
    {
        return ptr + 4 - beg;
    }

    /* Invalid character or incomplete. Cut before it */
    return ptr - beg;
}

/* Convert UTF-16 to UTF-8 */
static inline uint8_t *utf16_to_utf8(uint8_t *dest, const uint16_t *src, size_t size)
{
    uint32_t code_high = 0;

    while (size--)
    {
        uint32_t code = *src++;

        if (code_high)
        {
            if (code >= 0xdc00 && code <= 0xdfff)
            {
                /* Surrogate pair */
                code = ((code_high - 0xd800) << 10) + (code - 0xdc00) + 0x10000;

                *dest++ = (code >> 18) | 0xf0;
                *dest++ = ((code >> 12) & 0x3f) | 0x80;
                *dest++ = ((code >> 6) & 0x3f) | 0x80;
                *dest++ = (code & 0x3f) | 0x80;
            }
            else
            {
                /* Error...  */
                *dest++ = '?';
                /* *src may be valid. Don't eat it */
                --src;
            }
            code_high = 0;
        }
        else
        {
            if (code <= 0x007f)
            {
                *dest++ = code;
            }
            else if (code <= 0x07ff)
            {
                *dest++ = (code >> 6) | 0xc0;
                *dest++ = (code & 0x3f) | 0x80;
            }
            else if (code >= 0xd800 && code <= 0xdbff)
            {
                code_high = code;
                continue;
            }
            else if (code >= 0xdc00 && code <= 0xdfff)
            {
                /* Error */
                *dest++ = '?';
            }
            else if (code < 0x10000)
            {
                *dest++ = (code >> 12) | 0xe0;
                *dest++ = ((code >> 6) & 0x3f) | 0x80;
                *dest++ = (code & 0x3f) | 0x80;
            }
            else
            {
                *dest++ = (code >> 18) | 0xf0;
                *dest++ = ((code >> 12) & 0x3f) | 0x80;
                *dest++ = ((code >> 6) & 0x3f) | 0x80;
                *dest++ = (code & 0x3f) | 0x80;
            }
        }
    }

    return dest;
}

#define MAX_UTF8_PER_LATIN1 2

/* Convert Latin1 to UTF-8.  */
static inline uint8_t *latin1_to_utf8 (uint8_t *dest, const uint8_t *src, size_t size)
{
    while (size--)
    {
        if (!(*src & 0x80))
        {
            *dest++ = *src;
        }
        else
        {
            *dest++ = (*src >> 6) | 0xc0;
            *dest++ = (*src & 0x3f) | 0x80;
        }
        ++src;
    }

    return dest;
}

#endif /* _XBOOK_CHARSET_H */
