#ifndef NEXUS_V1_PRS3_H
#define NEXUS_V1_PRS3_H

#include <stddef.h>
#include <stdint.h>

/*
 * PRS3 decompression for Dungeon Master Nexus (Sega Saturn).
 *
 * PRS3 is used in DM.BIN, FACE.BIN, and MENU.BPK.  The format is a simple
 * LZ77 variant with an explicit 16-byte big-endian header followed by a
 * bitstream of literal / back-reference commands.
 */

/* Decode a PRS3-compressed buffer.
 *
 * src        — pointer to the PRS3 data (must start with the 16-byte header)
 * src_size   — total size of the compressed data including header
 * dst        — caller-allocated output buffer
 * dst_size   — size of the output buffer in bytes
 *
 * Returns the number of bytes written to dst, or 0 on error (bad magic,
 * truncated input, output buffer too small). */
size_t nexus_v1_prs3_decode(const uint8_t *src, size_t src_size,
                            uint8_t *dst, size_t dst_size);

#endif /* NEXUS_V1_PRS3_H */
