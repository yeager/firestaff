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
 * This module provides structural parsing and palette extraction.
 * Full animation playback is a later iteration.
 */

#define CSB_FMTOWNS_ANM_MAGIC_AN     0x414Eu  /* "AN" */
#define CSB_FMTOWNS_ANM_MAGIC_BR     0x4252u  /* "BR" */
#define CSB_FMTOWNS_ANM_MAX_CHUNKS   256
#define CSB_FMTOWNS_ANM_PALETTE_SIZE 16

typedef enum {
    CSB_FMTOWNS_ANM_CHUNK_UNKNOWN = 0,
    CSB_FMTOWNS_ANM_CHUNK_PL = 1,  /* palette */
    CSB_FMTOWNS_ANM_CHUNK_SD = 2,  /* sound data */
    CSB_FMTOWNS_ANM_CHUNK_EN = 3,  /* encoded frame */
    CSB_FMTOWNS_ANM_CHUNK_DL = 4,  /* delta frame */
    CSB_FMTOWNS_ANM_CHUNK_KD = 5,  /* keyframe data */
    CSB_FMTOWNS_ANM_CHUNK_BR = 6,  /* container wrapper */
    CSB_FMTOWNS_ANM_CHUNK_AN = 7   /* animation header */
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

/* Probe whether a buffer looks like an FM Towns ANM file.
 * Checks for "AN" or "BR" magic at the start. */
int csb_v1_fmtowns_anm_probe(const uint8_t *data, size_t size);

/* Parse the ANM structure and extract palette and chunk inventory.
 * Returns 0 on success, -1 on error. */
int csb_v1_fmtowns_anm_parse(const uint8_t *data, size_t size,
                               CSB_V1_FmtownsAnmReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_ANM_H */
