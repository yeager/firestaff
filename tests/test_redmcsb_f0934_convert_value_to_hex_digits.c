#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0934_convert_value_to_hex_digits.h"

static int failures;

static void check_conversion(uint32_t value, const char *expected, size_t count)
{
    char output[9] = { '!', '!', '!', '!', '!', '!', '!', '!', '!' };
    char *end = redmcsb_f0934_convert_value_to_hex_digits(output, value);
    if ((size_t)(end - output) != count || end[0] != '!' ||
        memcmp(output, expected, count) != 0) ++failures;
}

int main(void)
{
    check_conversion(0u, "0", 1u);
    check_conversion(0x0fu, "F", 1u);
    check_conversion(0x100u, "100", 3u);
    check_conversion(UINT32_MAX, "FFFFFFFF", 8u);
    if (failures) fprintf(stderr, "F0934 conversion failed\n");
    return failures != 0;
}
