#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define UTF8_END   -1
#define UTF8_ERROR -2
typedef struct {
    char mask;    /* the char data is in these bits */
    char lead;    /* the start bytes of a utf-8 encoded char */
    uint32_t beg; /* beginning of codepoint range */
    uint32_t end; /* end of codepoint range */
}utf_t;

utf_t * utf[] = {
    [0] = &(utf_t){0b00111111, 0b10000000, 0,       0       },
    [1] = &(utf_t){0b01111111, 0b00000000, 0000,    0177    },
    [2] = &(utf_t){0b00011111, 0b11000000, 0200,    03777   },
    [3] = &(utf_t){0b00001111, 0b11100000, 04000,   0177777 },
    [4] = &(utf_t){0b00000111, 0b11110000, 0200000, 04177777},
    &(utf_t){0},
};

int utf8_len(const char ch)
{
    int len = 0;
    for(utf_t **u = utf; u; ++u) {
        if((ch & ~(*u)->mask) == (*u)->lead) {
            break;
        }
        ++len;
    }
    if(len > 4) { /* Malformed leading byte */
        return -1;
    }
    return len;
}

uint32_t to_cp(const char chr[4], uint32_t * ret)
{
    int bytes = utf8_len(*chr);
    int shift = 6 * (bytes - 1);
    uint32_t codep = (*chr++ & utf[bytes]->mask) << shift;

    for(int i = 1; i < bytes; ++i, ++chr) {
        shift -= 6;
        codep |= ((char)*chr & utf[0]->mask) << shift;
    }

    ret[0] = bytes;
    ret[1] = codep;
    return codep;
}