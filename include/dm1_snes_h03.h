#ifndef DM1_SNES_H03_H
#define DM1_SNES_H03_H

#include <stddef.h>
#include <stdint.h>

#define DM1_SNES_H03_SLOT_SIZE 16384u

/* Decode one 16 KiB SNES H03 source slot. The slot starts with a big-endian
 * 32-bit decoded length, followed by literal and backward-copy commands.
 * Returns 1 on success and 0 for malformed input or insufficient capacity.
 * consumed receives the compressed header+command length, excluding padding.
 */
int dm1_snes_h03_decode_slot(const uint8_t *source, size_t source_size,
                            uint8_t *destination, size_t destination_capacity,
                            size_t *decoded_size, size_t *consumed);

#endif
