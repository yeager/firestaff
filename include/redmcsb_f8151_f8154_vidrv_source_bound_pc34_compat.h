/* ReDMCSB VIDEODRV.C F8151..F8154 source-bound PC 3.4 host contract. */
#ifndef FIRESTAFF_REDMCSB_F8151_F8154_VIDRV_SOURCE_BOUND_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8151_F8154_VIDRV_SOURCE_BOUND_PC34_COMPAT_H

#include "redmcsb_f0134_f0135_presentation_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RedmcsbF8151F8154SourceSurfacePc34 {
    const uint8_t *pixels;
    size_t pixel_count;
    int width;
    int height;
    uint32_t frame_fingerprint;
    uint32_t palette_fingerprint;
    int source_frame_verified;
    int no_synthetic_fallback;
} RedmcsbF8151F8154SourceSurfacePc34;

typedef uint8_t (*RedmcsbF8151F8154VideoStatusReaderPc34)(void *context);

typedef struct RedmcsbF8151F8154VideoDriverPc34 {
    Redmcsb_F0134F0135_PresentationTargetPc34 target;
    RedmcsbF8151F8154VideoStatusReaderPc34 read_status;
    void *status_context;
    size_t maximum_vblank_polls;
} RedmcsbF8151F8154VideoDriverPc34;

/* Binds the host presentation memory to a verified original source frame and
 * its exact PC34 16-colour palette. No generated or palette-substituted
 * target can enter the F8151/F8152/F8154 path. */
int redmcsb_f8151_f8154_vidrv_bind_pc34(
    RedmcsbF8151F8154VideoDriverPc34 *driver,
    uint8_t *target_pixels,
    size_t target_pixel_count,
    int target_width,
    int target_height,
    int target_pitch,
    uint32_t source_frame_fingerprint,
    const uint8_t *palette,
    size_t palette_bytes,
    int source_frame_verified,
    int no_synthetic_fallback,
    RedmcsbF8151F8154VideoStatusReaderPc34 read_status,
    void *status_context,
    size_t maximum_vblank_polls);

/* F8151: indexed source blit, clipped by F0134/F0135 and palette locked. */
int redmcsb_f8151_vidrv_source_blit_pc34(
    RedmcsbF8151F8154VideoDriverPc34 *driver,
    const RedmcsbF8151F8154SourceSurfacePc34 *source,
    int dst_x,
    int dst_y,
    int transparent_color);

/* F8152: inclusive FillBox through the shared source-bound F0135 primitive. */
int redmcsb_f8152_vidrv_source_fill_box_pc34(
    RedmcsbF8151F8154VideoDriverPc34 *driver,
    const int16_t box[4],
    uint16_t color);

/* F8153: bounded version of the exact inactive-then-active 0x3DA bit-3
 * sequence. It fails closed instead of letting an unavailable host spin. */
int redmcsb_f8153_vidrv_wait_vertical_blank_pc34(
    RedmcsbF8151F8154VideoDriverPc34 *driver);

/* F8154: inclusive xor-0x04 indexed rectangle on the already admitted
 * F0134/F0135 target. Reversed source endpoints remain no-ops. */
int redmcsb_f8154_vidrv_source_invert_box_pc34(
    RedmcsbF8151F8154VideoDriverPc34 *driver,
    int16_t x1,
    int16_t x2,
    int16_t y1,
    int16_t y2);

const char *redmcsb_f8151_f8154_vidrv_source_bound_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
