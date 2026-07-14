#include "redmcsb_f0933_get_hex_string_from_value.h"

static char *convert_value_to_hex_digits(char *string, uint32_t value) {
    static const char hex_digits[] = "0123456789ABCDEF";
    char local_string[8];
    char *digit = local_string;
    int digit_count = 0;

    do {
        *digit++ = hex_digits[value % 16];
        value >>= 4;
        ++digit_count;
    } while (value > 0);

    while (digit_count-- > 0) {
        *string++ = *--digit;
    }

    return string;
}

int F0933_GetHexStringFromValue(uint32_t value, char *string) {
    *convert_value_to_hex_digits(string, value) = '\0';
    return 0;
}
