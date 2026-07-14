#include "redmcsb_f0661_get_shrinked_bitmap_pc34_compat.h"

#include <stddef.h>
#include <string.h>

static int16_t bitmap_dimension(const uint8_t *bitmap, int offset)
{
    int16_t value;

    memcpy(&value, bitmap + offset, sizeof(value));
    return value;
}

static int valid_renderer(const redmcsb_f0661_renderer_pc34_compat *renderer)
{
    return renderer != NULL && renderer->is_derived_bitmap_cached != NULL &&
           renderer->get_native_bitmap != NULL &&
           renderer->get_derived_bitmap != NULL &&
           renderer->blit_shrink_palette != NULL &&
           renderer->add_derived_bitmap_to_cache != NULL;
}

uint8_t *redmcsb_f0661_get_shrinked_bitmap_pc34_compat(
    const redmcsb_f0661_renderer_pc34_compat *renderer,
    int16_t native_bitmap_index,
    int16_t derived_bitmap_index,
    int16_t width,
    int16_t height,
    const uint8_t *palette_changes)
{
    const uint8_t *native_bitmap;
    uint8_t *derived_bitmap;

    if (!valid_renderer(renderer)) return NULL;
    if (renderer->is_derived_bitmap_cached(renderer->context,
                                            derived_bitmap_index)) {
        return renderer->get_derived_bitmap(renderer->context,
                                            derived_bitmap_index);
    }
    native_bitmap = renderer->get_native_bitmap(renderer->context,
                                                 native_bitmap_index);
    derived_bitmap = renderer->get_derived_bitmap(renderer->context,
                                                   derived_bitmap_index);
    if (native_bitmap == NULL || derived_bitmap == NULL) return NULL;
    memcpy(derived_bitmap - 4, &width, sizeof(width));
    memcpy(derived_bitmap - 2, &height, sizeof(height));
    renderer->blit_shrink_palette(renderer->context, native_bitmap,
                                  derived_bitmap,
                                  bitmap_dimension(native_bitmap, -4),
                                  bitmap_dimension(native_bitmap, -2),
                                  width, height, palette_changes);
    renderer->add_derived_bitmap_to_cache(renderer->context,
                                          derived_bitmap_index);
    return derived_bitmap;
}

const char *redmcsb_f0661_get_shrinked_bitmap_source_evidence_pc34(void)
{
    return "ReDMCSB BASE.C F0661_GetShrinkedBitmap (1526-1546); "
           "DEFS.H M100/M101 (3444-3445); F0491/F0492/F0493 cache contract";
}
