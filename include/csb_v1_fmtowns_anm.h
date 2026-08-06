#ifndef CSB_V1_FMTOWNS_ANM_H
#define CSB_V1_FMTOWNS_ANM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CSB FM Towns ANM animation format parser.
 *
 * FM Towns animation files (TITLE.ANM, STORY.ANM, ENDING.ANM) use a
 * chunk-based format with the following structure:
 *
 *   "AN" header: 2-byte magic, version, flags, dimensions (320x200, 4bpp)
 *   Subchunks (2-byte ID + 2-byte BE16 size + data):
 *     "PL" — palette (16 colors, 4-bit RGB 0-15 per component)
 *     "SD" — sound data reference (CDDA track mapping)
 *     "EN" — encoded frame data
 *     "DL" — delta frame data
 *     "KD" — keyframe data (large compressed frame)
 *
 *   "BR" — wrapper/container (ENDING.ANM wraps multiple AN sections)
 *
 * ReDMCSB ANIM.C F2275 dispatches EN/DL chunks to ANIMIMG.C F8288.
 * The decoder below is a bounded, indexed-pixel rendition of that command
 * stream.  It has no host artwork or generated frame fallback: callers must
 * supply the original ANM bytes and a 320x200 source surface.
 */

#define CSB_FMTOWNS_ANM_MAGIC_AN     0x414Eu  /* "AN" */
#define CSB_FMTOWNS_ANM_MAGIC_BR     0x4252u  /* "BR" */
#define CSB_FMTOWNS_ANM_MAX_CHUNKS   256
#define CSB_FMTOWNS_ANM_PALETTE_SIZE 16
#define CSB_FMTOWNS_ANM_MAX_WIDTH    320u
#define CSB_FMTOWNS_ANM_MAX_HEIGHT   200u
#define CSB_FMTOWNS_ANM_FRAME_PIXELS \
    (CSB_FMTOWNS_ANM_MAX_WIDTH * CSB_FMTOWNS_ANM_MAX_HEIGHT)
#define CSB_FMTOWNS_ANM_MAX_LOOP_DEPTH 16u

typedef enum {
    CSB_FMTOWNS_ANM_CHUNK_UNKNOWN = 0,
    CSB_FMTOWNS_ANM_CHUNK_PL = 1,  /* palette */
    CSB_FMTOWNS_ANM_CHUNK_SD = 2,  /* sound data */
    CSB_FMTOWNS_ANM_CHUNK_EN = 3,  /* encoded frame */
    CSB_FMTOWNS_ANM_CHUNK_DL = 4,  /* delta frame */
    CSB_FMTOWNS_ANM_CHUNK_KD = 5,  /* keyframe data */
    CSB_FMTOWNS_ANM_CHUNK_BR = 6,  /* container wrapper */
    CSB_FMTOWNS_ANM_CHUNK_AN = 7,  /* animation header */
    CSB_FMTOWNS_ANM_CHUNK_FO = 8,  /* loop begin */
    CSB_FMTOWNS_ANM_CHUNK_NE = 9   /* loop end */
} CSB_V1_FmtownsAnmChunkType;

typedef struct {
    CSB_V1_FmtownsAnmChunkType type;
    char     id[3];
    uint32_t offset;
    uint16_t size;
} CSB_V1_FmtownsAnmChunk;

typedef struct {
    uint8_t r, g, b;
} CSB_V1_FmtownsAnmColor;

typedef struct {
    int      valid;
    uint16_t width;
    uint16_t height;
    uint8_t  bpp;
    uint8_t  version;
    uint8_t  flags;
    int      has_br_wrapper;
    int      chunk_count;
    int      palette_count;
    int      frame_count;
    int      keyframe_count;
    int      delta_count;
    CSB_V1_FmtownsAnmColor palette[CSB_FMTOWNS_ANM_PALETTE_SIZE];
    int      has_palette;
    uint32_t file_size;
} CSB_V1_FmtownsAnmReceipt;

typedef struct {
    int valid;
    uint16_t width;
    uint16_t height;
    uint32_t frame_index;
    uint32_t source_chunk_offset;
    uint32_t source_chunk_bytes;
    /* ReDMCSB ANIM.C F2275 stores the chunk attribute in G8204_ before
     * displaying EN/DL. FM Towns clamps it to five Timer-A ticks.  These are
     * source timer units, deliberately not host milliseconds. */
    uint16_t source_delay_ticks;
    uint16_t timer_a_ticks;
    int source_was_delta;
    int palette_applied;
    CSB_V1_FmtownsAnmColor palette[CSB_FMTOWNS_ANM_PALETTE_SIZE];
    uint32_t pixel_fnv1a;
} CSB_V1_FmtownsAnmFrameReceipt;

/* Stateful F2275 interpreter. The caller supplies and retains the indexed
 * 320x200 work surface, matching the original animation player's retained
 * bitmap. It deliberately exposes Timer-A units rather than a guessed host
 * duration. */
typedef struct {
    const uint8_t *data;
    size_t size;
    size_t offset;
    uint16_t width;
    uint16_t height;
    uint16_t loop_count[CSB_FMTOWNS_ANM_MAX_LOOP_DEPTH];
    size_t loop_item_offset[CSB_FMTOWNS_ANM_MAX_LOOP_DEPTH];
    uint32_t presentation_frame_index;
    uint32_t chunks_visited;
    uint16_t loop_depth;
    int valid;
    int finished;
    int break_allowed;
    int palette_seen;
    CSB_V1_FmtownsAnmColor palette[CSB_FMTOWNS_ANM_PALETTE_SIZE];
} CSB_V1_FmtownsAnmPlayback;

/* Probe whether a buffer looks like an FM Towns ANM file.
 * Checks for "AN" or "BR" magic at the start. */
int csb_v1_fmtowns_anm_probe(const uint8_t *data, size_t size);

/* Parse the ANM structure and extract palette and chunk inventory.
 * Returns 0 on success, -1 on error. */
int csb_v1_fmtowns_anm_parse(const uint8_t *data, size_t size,
                               CSB_V1_FmtownsAnmReceipt *out);

/* Decode frame_index (zero based) into one palette index per output pixel.
 * out_pixels must hold width*height bytes.  Earlier EN/DL chunks are applied
 * in source order, exactly as the F2275/F8288 animation path retains its
 * working bitmap.  Returns 0 on success and -1 on malformed or incomplete
 * source data. */
int csb_v1_fmtowns_anm_decode_frame(const uint8_t *data, size_t size,
                                    uint32_t frame_index,
                                    uint8_t *out_pixels,
                                    size_t out_pixel_capacity,
                                    CSB_V1_FmtownsAnmFrameReceipt *out);

/* Initialise and advance the original F2275 chunk stream. Step returns 1
 * when it has decoded one EN/DL frame, 0 at end of stream and -1 for malformed
 * source data. pixels must remain allocated between calls. */
int csb_v1_fmtowns_anm_playback_init(const uint8_t *data, size_t size,
                                     CSB_V1_FmtownsAnmPlayback *out);
int csb_v1_fmtowns_anm_playback_step(CSB_V1_FmtownsAnmPlayback *playback,
                                     uint8_t *pixels,
                                     size_t pixel_capacity,
                                     CSB_V1_FmtownsAnmFrameReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_ANM_H */
