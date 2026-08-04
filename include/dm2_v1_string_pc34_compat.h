#ifndef FIRESTAFF_DM2_V1_STRING_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_STRING_PC34_COMPAT_H

/*
 * dm2_v1_string_pc34_compat.h — DM2 string utility functions.
 *
 * Ports string/number formatting from skproject/SKWINSPX/src/v5/skstr.cpp.
 *
 * Source: skproject/SKWINSPX/src/v5/skstr.{h,cpp}
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Convert an ASCII uppercase letter to the DM2 script character index.
 * A-Z → 0-25, '.' → 0x1b, everything else → 0x1a.
 * Source: skstr.cpp DM2_SKCHR_TO_SCRIPTCHR */
char dm2_v1_skchr_to_scriptchr(char c);

/* Convert a signed 32-bit integer to decimal string.
 * Writes into dst and returns dst.
 * Source: skstr.cpp DM2_LTOA10 */
char *dm2_v1_ltoa10(int32_t value, char *dst);

/* Format a 16-bit unsigned number into a static 5-byte buffer.
 * If padded != 0, left-pads with spaces to width digits.
 * Returns pointer into the formatted string.
 * Source: skstr.cpp DM2_FMT_NUM */
char *dm2_v1_fmt_num(int16_t value, int32_t padded, int16_t width);

/* Fill a string buffer: write `entry` at positions 0, step, 2*step, ...
 * for `count` iterations.
 * Source: skstr.cpp DM2_FILL_STR */
void dm2_v1_fill_str(char *buf, char entry, int16_t step, int16_t count);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_STRING_PC34_COMPAT_H */
