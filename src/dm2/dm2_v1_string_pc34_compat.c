#include "dm2_v1_string_pc34_compat.h"
#include <string.h>

char dm2_v1_skchr_to_scriptchr(char c)
{
    if (c >= 'A' && c <= 'Z')
        return (char)(c - 'A');
    return (c == '.') ? (char)0x1b : (char)0x1a;
}

static char fnum[5];

char *dm2_v1_ltoa10(int32_t value, char *dst)
{
    char buf[16];
    buf[13] = '\0';
    char *p = buf + 13;

    int32_t abs_val = value;
    if (abs_val < 0)
        abs_val = -abs_val;

    do {
        *--p = (char)(abs_val % 10 + '0');
        abs_val /= 10;
    } while (abs_val != 0);

    if (value < 0)
        *--p = '-';

    return strcpy(dst, p);
}

char *dm2_v1_fmt_num(int16_t value, int32_t padded, int16_t width)
{
    if (padded != 0)
        dm2_v1_fill_str(fnum, ' ', 1, 4);

    char *end = &fnum[4];
    *end = '\0';

    if (value != 0) {
        int16_t v = value;
        while (v != 0) {
            int16_t prev = v;
            v /= 10;
            *--end = (char)((uint16_t)prev + '0' - 10 * (uint16_t)v);
        }
    } else {
        *--end = '0';
    }

    if (padded == 0)
        return end;
    return &fnum[4 - (uint16_t)width];
}

void dm2_v1_fill_str(char *buf, char entry, int16_t step, int16_t count)
{
    int16_t idx = 0;
    for (int16_t i = 0; i < count; i++) {
        buf[idx] = entry;
        idx += step;
    }
}
