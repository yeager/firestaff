#include <stdio.h>
#include <string.h>

#include "redmcsb_f0933_get_hex_string_from_value.h"

static int failures;

#define CHECK(expression) do { \
    if (!(expression)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #expression, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static void check_value(uint32_t value, const char *expected) {
    char string[10] = "unchanged";

    CHECK(F0933_GetHexStringFromValue(value, string) == 0);
    CHECK(strcmp(string, expected) == 0);
}

int main(void) {
    check_value(0, "0");
    check_value(0xF, "F");
    check_value(0x10, "10");
    check_value(0x12AB34CD, "12AB34CD");
    check_value(UINT32_MAX, "FFFFFFFF");

    return failures != 0;
}
