#ifndef FIRESTAFF_REDMCSB_F0657_F0658_VIEWPORT_BITMAP_INDEX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0657_F0658_VIEWPORT_BITMAP_INDEX_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB COORD.C F0630's caller-visible STRUCT2 fields. */
typedef struct redmcsb_f0657_f0658_bitmap_struct2_pc34_compat {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} redmcsb_f0657_f0658_bitmap_struct2_pc34_compat;

typedef const uint8_t *(*redmcsb_f0657_f0658_init_bitmap_struct2_pc34_compat)(
    void *context,
    int16_t bitmap_index,
    redmcsb_f0657_f0658_bitmap_struct2_pc34_compat *bitmap_struct2);

/* Mirrors F0635_'s truth value plus XYZ/X/Y output contract. */
typedef int (*redmcsb_f0657_f0658_resolve_zone_pc34_compat)(
    void *context,
    const uint8_t *bitmap,
    int16_t xyz[4],
    int16_t zone_index,
    int16_t *x,
    int16_t *y);

typedef int16_t (*redmcsb_f0657_f0658_bitmap_pixel_width_pc34_compat)(
    void *context,
    const uint8_t *bitmap);

typedef void (*redmcsb_f0657_f0658_video_blit_pc34_compat)(
    void *context,
    const uint8_t *source_bitmap,
    uint8_t *destination_bitmap,
    const int16_t xyz[4],
    int16_t source_x,
    int16_t source_y,
    int16_t source_pixel_width,
    int16_t destination_pixel_width,
    int16_t transparent_color,
    int16_t flip);

typedef struct redmcsb_f0657_f0658_renderer_pc34_compat {
    redmcsb_f0657_f0658_init_bitmap_struct2_pc34_compat init_bitmap_struct2;
    redmcsb_f0657_f0658_resolve_zone_pc34_compat resolve_zone;
    redmcsb_f0657_f0658_bitmap_pixel_width_pc34_compat bitmap_pixel_width;
    redmcsb_f0657_f0658_video_blit_pc34_compat video_blit;
    void *context;
    uint8_t *viewport_bitmap;
    int16_t viewport_pixel_width;
} redmcsb_f0657_f0658_renderer_pc34_compat;

enum { REDMCSB_F0657_F0658_PC34_NO_FLIP = 0 };

/* Executes BASE.C F0657. Returns 1 only when the original-data dispatch ran. */
int redmcsb_f0657_blit_bitmap_index_to_viewport_zone_with_transparency_pc34_compat(
    const redmcsb_f0657_f0658_renderer_pc34_compat *renderer,
    int16_t bitmap_index,
    const int16_t xyz[4],
    int16_t transparent_color);

/* Executes BASE.C F0658. Returns 1 only after F0630, F0635, and F0132 ran. */
int redmcsb_f0658_blit_bitmap_index_to_zone_index_with_transparency_pc34_compat(
    const redmcsb_f0657_f0658_renderer_pc34_compat *renderer,
    int16_t bitmap_index,
    int16_t zone_index,
    int16_t transparent_color);

const char *redmcsb_f0657_f0658_viewport_bitmap_index_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
