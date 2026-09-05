#ifndef FIRESTAFF_CP932_H
#define FIRESTAFF_CP932_H
#include <stddef.h>
/* Decode a byte span to NUL-terminated UTF-8. Returns output bytes, or -1
 * for invalid input/insufficient capacity. Failure clears output[0].
 * Original input must not overlap output. No replacement characters. */
int firestaff_cp932_to_utf8(const char *input, size_t length,
                           char *output, size_t capacity);
#endif
