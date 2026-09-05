#include "firestaff_cp932.h"
#include <stdint.h>
#include <limits.h>
#include "firestaff_cp932_table.inc"

int firestaff_cp932_to_utf8(const char *input, size_t length,
                           char *output, size_t capacity)
{
    size_t cursor = 0, used = 0;
    if (!output || !capacity) return -1;
    output[0] = 0;
    if (!input) return -1;
    while (cursor < length) {
        unsigned byte = (unsigned char)input[cursor++];
        unsigned code;
        size_t count;
        if (byte <= 0x80) code = byte;
        else if (byte >= 0xa1 && byte <= 0xdf) code = 0xff61 + byte - 0xa1;
        else if (byte == 0xa0) code = 0xf8f0;
        else if (byte >= 0xfd) code = 0xf8f1 + byte - 0xfd;
        else {
            unsigned row, trail, column;
            if (cursor == length) goto invalid;
            if (byte >= 0x81 && byte <= 0x9f) row = byte - 0x81;
            else if (byte >= 0xe0 && byte <= 0xfc) row = 31 + byte - 0xe0;
            else goto invalid;
            trail = (unsigned char)input[cursor++];
            if (trail >= 0x40 && trail <= 0x7e) column = trail - 0x40;
            else if (trail >= 0x80 && trail <= 0xfc) column = 63 + trail - 0x80;
            else goto invalid;
            code = fs_cp932_pairs[row * 188 + column];
            if (!code) goto invalid;
        }
        count = code < 0x80 ? 1 : code < 0x800 ? 2 : 3;
        if (count >= capacity - used || used > (size_t)INT_MAX - count)
            goto invalid;
        if (count == 1) output[used++] = (char)code;
        else if (count == 2) {
            output[used++] = (char)(0xc0 | (code >> 6));
            output[used++] = (char)(0x80 | (code & 63));
        } else {
            output[used++] = (char)(0xe0 | (code >> 12));
            output[used++] = (char)(0x80 | ((code >> 6) & 63));
            output[used++] = (char)(0x80 | (code & 63));
        }
    }
    output[used] = 0;
    return (int)used;
invalid:
    output[0] = 0;
    return -1;
}
