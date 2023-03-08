/*
https://www.mycplus.com/source-code/c-source-code/base64-encode-decode/
https://github.com/lpereira/lwan/blob/master/src/lib/base64.c
https://github.com/lpereira/lwan/blob/master/src/lib/base64.h
*/

#include "base64.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ENABLE_ASSERT

#ifdef ENABLE_ASSERT
#define ASSERT(c) if (!(c)) __builtin_trap()
#else
#define ASSERT(c)
#endif

static char base64_encode_table[] = {
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
'w', 'x', 'y', 'z', '0', '1', '2', '3',
'4', '5', '6', '7', '8', '9', '+', '/'};

static const unsigned char base64_decode_table[256] = {
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x3e, 0x80, 0x80, 0x80, 0x3f,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x80, 0x80,
    0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12,
    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80};



size_t base64_encoded_len(size_t len)
{
    /* This counts the padding bytes (by rounding to the next multiple of 4). */
    //return ((4u * decoded_len / 3u) + 3u) & ~3u;
    return 4 * ((len + 2) / 3);
}

size_t base64_decoded_len(const char *data, size_t len)
{
    ASSERT(data);
    ASSERT((len % 4) == 0);
    size_t r = len / 4 * 3;
    r -= (data[len - 1] == '=');
    r -= (data[len - 2] == '=');
    return r;
}
 
void base64_encode(uint8_t const *src, size_t srclen, char *dst, size_t dstlen)
{
    ASSERT(src);
    ASSERT(dst);
    for (size_t i = 0, j = 0; i < srclen;)
    {
        uint32_t octet_a = (i < srclen) ? (unsigned char)src[i++] : 0;
        uint32_t octet_b = (i < srclen) ? (unsigned char)src[i++] : 0;
        uint32_t octet_c = (i < srclen) ? (unsigned char)src[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        dst[j++] = base64_encode_table[(triple >> 3 * 6) & 0x3F];
        dst[j++] = base64_encode_table[(triple >> 2 * 6) & 0x3F];
        dst[j++] = base64_encode_table[(triple >> 1 * 6) & 0x3F];
        dst[j++] = base64_encode_table[(triple >> 0 * 6) & 0x3F];
    }
    int mod_table[] = {0, 2, 1};
    for (int i = 0; i < mod_table[srclen % 3]; i++)
    {
        dst[dstlen - 1 - i] = '=';
    }
}
 
 
void base64_decode(char const *src, size_t srclen, uint8_t *dst, size_t dstlen)
{
    ASSERT(src);
    ASSERT(dst);
    for (size_t i = 0, j = 0; i < srclen;)
    {
        uint32_t sextet_a = (src[i] == '=') ? (0 & i++) : base64_decode_table[(int)src[i++]];
        uint32_t sextet_b = (src[i] == '=') ? (0 & i++) : base64_decode_table[(int)src[i++]];
        uint32_t sextet_c = (src[i] == '=') ? (0 & i++) : base64_decode_table[(int)src[i++]];
        uint32_t sextet_d = (src[i] == '=') ? (0 & i++) : base64_decode_table[(int)src[i++]];
        uint32_t triple = 
        (sextet_a << 3 * 6) +
        (sextet_b << 2 * 6) +
        (sextet_c << 1 * 6) +
        (sextet_d << 0 * 6);
        if (j < dstlen) {dst[j++] = (triple >> 2 * 8) & 0xFF;}
        if (j < dstlen) {dst[j++] = (triple >> 1 * 8) & 0xFF;}
        if (j < dstlen) {dst[j++] = (triple >> 0 * 8) & 0xFF;}
    }
}









void base64_testunit1()
{
    for (int i = 0; i < 64; i++)
    {
        int j = base64_decode_table[(int) base64_encode_table[i]];
        ASSERT(j == i);
    }
}