#include "redmcsb_f0934_convert_value_to_hex_digits.h"

char *redmcsb_f0934_convert_value_to_hex_digits(char *string, uint32_t value)
{
    static const char hex_digits[] = "0123456789ABCDEF";
    char reversed[8];
    char *digit = reversed;

    do {
        *digit++ = hex_digits[value % 16u];
        value >>= 4;
    } while (value > 0u);
    while (digit != reversed) *string++ = *--digit;
    return string;
}
