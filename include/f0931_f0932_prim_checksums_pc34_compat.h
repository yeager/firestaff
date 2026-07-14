#ifndef F0931_F0932_PRIM_CHECKSUMS_PC34_COMPAT_H
#define F0931_F0932_PRIM_CHECKSUMS_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB PRIM1.C F0931/F0932: unsigned 16-bit wrapping sums used by
 * the original loader validation paths. */
uint16_t f0931_checksum_words_pc34_compat(
    const int16_t *words, size_t byte_count);
uint16_t f0932_checksum_bytes_pc34_compat(
    const uint8_t *bytes, size_t byte_count);

#endif
