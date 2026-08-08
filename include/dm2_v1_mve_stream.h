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

/* One fully bounded presentation unit from the original MVE member.  The
 * payload offsets are into the caller-owned executable bytes; no video or
 * audio is materialised to disk.  DM2's DOS corpus uses a 40x25 code map,
 * MVE video opcode 0x11 version 3 and one 132-byte opcode-0x13 transport
 * payload per presented image. */
typedef struct {
    int valid;
    uint32_t presentation_index;
    uint64_t presentation_time_us;
    uint32_t code_map_offset;
    uint16_t code_map_size;
    uint32_t video_data_offset;
    uint16_t video_data_size;
    uint8_t video_version;
    uint32_t palette_offset;
    uint16_t palette_size;
    uint32_t audio_offset;
    uint16_t audio_size;
    uint32_t transport13_offset;
    uint16_t transport13_size;
} DM2_V1_MvePresentation;

/* Strict memory-only iterator for the exact Interplay MVE opcode grammar
 * admitted by the verified DM2 PC-DOS INTRO/END corpus.  It is deliberately
 * not a video decoder: a caller receives only original bounded payloads and
 * timing, then a later indexed-video owner must decode them into its own RAM
 * buffers. Unknown opcode/version pairs fail closed. */
typedef struct {
    const uint8_t *bytes;
    size_t byte_count;
    size_t offset;
    uint32_t timer_rate_us;
    uint16_t timer_subdivision;
    uint16_t width;
    uint16_t height;
    uint32_t presentation_count;
    int initialized;
    int ended;
} DM2_V1_MvePresentationIterator;

/* Parses one complete MVE stream embedded in bytes.  Unknown opcodes remain
 * structurally valid; malformed lengths, missing end-of-stream, unsupported
 * video dimensions, or trailing bytes after the terminal chunk fail closed. */
int dm2_v1_mve_stream_parse(const uint8_t *bytes, size_t byte_count,
                            DM2_V1_MveStreamReceipt *out);

int dm2_v1_mve_presentation_iterator_init(
    DM2_V1_MvePresentationIterator *iterator,
    const uint8_t *bytes, size_t byte_count);

/* Returns 1 and fills `out` for a source presentation, 0 at the terminal
 * chunk, and -1 for any malformed or unadmitted stream grammar. */
int dm2_v1_mve_presentation_iterator_next(
    DM2_V1_MvePresentationIterator *iterator,
    DM2_V1_MvePresentation *out);

#endif /* FIRESTAFF_DM2_V1_MVE_STREAM_H */
