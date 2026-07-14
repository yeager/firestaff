#ifndef REDMCSB_F0934_CONVERT_VALUE_TO_HEX_DIGITS_H
#define REDMCSB_F0934_CONVERT_VALUE_TO_HEX_DIGITS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB PRIM1.C F0934_ConvertValueToHexDigits.
 *
 * Writes the minimal uppercase hexadecimal representation of `value` to
 * `string`, without a terminating NUL, and returns the first unwritten byte.
 * The caller must provide room for at least eight digits.
 */
char *redmcsb_f0934_convert_value_to_hex_digits(char *string, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_F0934_CONVERT_VALUE_TO_HEX_DIGITS_H */
