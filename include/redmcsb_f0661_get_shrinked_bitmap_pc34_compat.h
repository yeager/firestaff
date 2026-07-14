#ifndef FIRESTAFF_REDMCSB_F0661_GET_SHRINKED_BITMAP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0661_GET_SHRINKED_BITMAP_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB BASE.C F0661, with all graphic/cache ownership supplied by caller. */
typedef int (*redmcsb_f0661_is_derived_bitmap_cached_pc34_compat)(
    void *context, int16_t derived_bitmap_index);

typedef const uint8_t *(*redmcsb_f0661_get_native_bitmap_pc34_compat)(
    void *context, int16_t native_bitmap_index);

typedef uint8_t *(*redmcsb_f0661_get_derived_bitmap_pc34_compat)(
    void *context, int16_t derived_bitmap_index);

typedef void (*redmcsb_f0661_blit_shrink_palette_pc34_compat)(
    void *context,
    const uint8_t *source_bitmap,
    uint8_t *destination_bitmap,
    int16_t source_pixel_width,
    int16_t source_height,
    int16_t destination_pixel_width,
    int16_t destination_height,
    const uint8_t *palette_changes);

typedef void (*redmcsb_f0661_add_derived_bitmap_to_cache_pc34_compat)(
    void *context, int16_t derived_bitmap_index);

typedef struct redmcsb_f0661_renderer_pc34_compat {
    redmcsb_f0661_is_derived_bitmap_cached_pc34_compat is_derived_bitmap_cached;
    redmcsb_f0661_get_native_bitmap_pc34_compat get_native_bitmap;
    redmcsb_f0661_get_derived_bitmap_pc34_compat get_derived_bitmap;
    redmcsb_f0661_blit_shrink_palette_pc34_compat blit_shrink_palette;
    redmcsb_f0661_add_derived_bitmap_to_cache_pc34_compat add_derived_bitmap_to_cache;
    void *context;
} redmcsb_f0661_renderer_pc34_compat;

/*
 * Executes BASE.C F0661's cache hit/miss contract. Bitmaps point at pixels;
 * their width/height prefixes are copied only on a cache miss, as in source.
 */
uint8_t *redmcsb_f0661_get_shrinked_bitmap_pc34_compat(
    const redmcsb_f0661_renderer_pc34_compat *renderer,
    int16_t native_bitmap_index,
    int16_t derived_bitmap_index,
    int16_t width,
    int16_t height,
    const uint8_t *palette_changes);

const char *redmcsb_f0661_get_shrinked_bitmap_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
