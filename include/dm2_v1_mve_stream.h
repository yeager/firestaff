#ifndef FIRESTAFF_DM2_V1_MVE_STREAM_H
#define FIRESTAFF_DM2_V1_MVE_STREAM_H

#include <stddef.h>
#include <stdint.h>

/* Interplay MVE stream reader for the PC-DOS DM2 wrapper executables.
 *
 * INTRO and END contain a DOS program before their MVE stream.  This reader
 * locates the actual `Interplay MVE File\x1a\0` member inside caller-owned
 * bytes, validates every chunk/opcode boundary, and retains only facts that
 * a video/audio consumer can prove from the original stream.  It never
 * writes a movie, decoded frame, palette, or audio buffer to disk.
 *
 * Format reference: http://wiki.multimedia.cx/index.php/Interplay_MVE
 * and the original DM2 PC-DOS INTRO/END executables. */
typedef struct {
    int valid;
    uint32_t mve_offset;
    uint32_t mve_byte_count;
    uint32_t chunk_count;
    uint32_t opcode_count;
    uint32_t video_frame_count;
    uint32_t palette_update_count;
    uint32_t audio_frame_count;
    uint32_t display_count;
    uint32_t timer_rate_us;
    uint16_t timer_subdivision;
    uint16_t width;
    uint16_t height;
    uint32_t receipt_hash;
} DM2_V1_MveStreamReceipt;

/* Parses one complete MVE stream embedded in bytes.  Unknown opcodes remain
 * structurally valid; malformed lengths, missing end-of-stream, unsupported
 * video dimensions, or trailing bytes after the terminal chunk fail closed. */
int dm2_v1_mve_stream_parse(const uint8_t *bytes, size_t byte_count,
                            DM2_V1_MveStreamReceipt *out);

#endif /* FIRESTAFF_DM2_V1_MVE_STREAM_H */
