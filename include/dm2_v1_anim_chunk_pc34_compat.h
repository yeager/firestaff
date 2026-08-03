#ifndef FIRESTAFF_DM2_V1_ANIM_CHUNK_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_ANIM_CHUNK_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#define DM2_V1_ANIM_CHUNK_AN 0x414e
#define DM2_V1_ANIM_CHUNK_PL 0x504c
#define DM2_V1_ANIM_CHUNK_EN 0x454e
#define DM2_V1_ANIM_CHUNK_DL 0x444c
#define DM2_V1_ANIM_CHUNK_SD 0x5344
#define DM2_V1_ANIM_CHUNK_BR 0x4252
#define DM2_V1_ANIM_CHUNK_SO 0x534f
#define DM2_V1_ANIM_CHUNK_DO 0x444f
#define DM2_V1_ANIM_CHUNK_FO 0x464f
#define DM2_V1_ANIM_CHUNK_NE 0x4e45
#define DM2_V1_ANIM_CHUNK_BN 0x424e

#define DM2_V1_ANIM_MAX_PALETTE_COLORS 256
#define DM2_V1_ANIM_CHUNK_OVERHEAD 6

typedef struct {
    uint16_t tag;
    uint16_t payload_size;
    const uint8_t *payload;
    uint16_t trailer;
    uint32_t file_offset;
} DM2_V1_AnimChunk;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t flags;
    uint16_t extra;
} DM2_V1_AnimAnHeader;

typedef struct {
    uint16_t start_color;
    uint16_t num_colors;
    uint8_t entries[DM2_V1_ANIM_MAX_PALETTE_COLORS][4];
} DM2_V1_AnimPalette;

typedef struct {
    int valid;
    uint32_t chunk_count;
    uint32_t an_count;
    uint32_t pl_count;
    uint32_t en_count;
    uint32_t dl_count;
    uint32_t sd_count;
    uint32_t br_count;
    uint32_t so_count;
    uint32_t do_count;
    uint32_t fo_count;
    uint32_t ne_count;
    uint32_t bn_count;
    uint32_t unknown_count;
    uint32_t bytes_consumed;
} DM2_V1_AnimChunkScanReceipt;

int dm2_v1_anim_chunk_read(const uint8_t *data,
                           size_t data_size,
                           uint32_t offset,
                           DM2_V1_AnimChunk *out_chunk);

int dm2_v1_anim_chunk_scan(const uint8_t *data,
                           size_t data_size,
                           DM2_V1_AnimChunkScanReceipt *out_receipt);

int dm2_v1_anim_parse_an_header(const DM2_V1_AnimChunk *chunk,
                                DM2_V1_AnimAnHeader *out_header);

int dm2_v1_anim_parse_palette(const DM2_V1_AnimChunk *chunk,
                              DM2_V1_AnimPalette *out_palette);

int dm2_v1_anim_decode_en_keyframe(const DM2_V1_AnimChunk *chunk,
                                   uint8_t *dst,
                                   size_t dst_size,
                                   uint16_t *out_width,
                                   uint16_t *out_height);

int dm2_v1_anim_apply_dl_delta(const DM2_V1_AnimChunk *chunk,
                               uint8_t *framebuf,
                               size_t framebuf_size,
                               uint16_t frame_width,
                               uint16_t frame_height);

#endif
