#include "redmcsb_f0934_convert_value_to_hex_digits.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void check_conversion(uint32_t value, const char *expected, size_t expectedLength)
{
    char output[9] = { '!', '!', '!', '!', '!', '!', '!', '!', '!' };
    char *end = redmcsb_f0934_convert_value_to_hex_digits(output, value);

    check((size_t)(end - output) == expectedLength,
          "returns the first unwritten byte");
    check(end[0] == '!', "does not append a terminator");
    check(memcmp(output, expected, expectedLength) == 0,
          "writes uppercase hexadecimal digits");
}

int main(void)
{
    check_conversion(0u, "0", 1u);
    check_conversion(0x0000000fu, "F", 1u);
    check_conversion(0x00000100u, "100", 3u);
    check_conversion(UINT32_MAX, "FFFFFFFF", 8u);

    return failures ? 1 : 0;
}
