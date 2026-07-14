#include "redmcsb_f0662_f0663_palette_bitmap_pc34_compat.h"

#include <stddef.h>
#include <string.h>

static int16_t bitmap_dimension(const uint8_t *bitmap, int offset)
{
    int16_t value;

    memcpy(&value, bitmap + offset, sizeof(value));
    return value;
}

static int valid_renderer(const redmcsb_f0662_f0663_renderer_pc34_compat *renderer)
{
    return renderer != NULL && renderer->blit_shrink_palette != NULL;
}

int redmcsb_f0662_apply_palette_changes_pc34_compat(
    const redmcsb_f0662_f0663_renderer_pc34_compat *renderer,
    uint8_t *bitmap,
    const uint8_t *palette_changes)
{
    int16_t width;
    int16_t height;

    if (!valid_renderer(renderer) || bitmap == NULL) return 0;
    width = bitmap_dimension(bitmap, -4);
    height = bitmap_dimension(bitmap, -2);
    renderer->blit_shrink_palette(renderer->context, bitmap, bitmap,
                                  width, height, width, height,
                                  palette_changes);
    return 1;
}

int redmcsb_f0663_copy_bitmap_with_palette_changes_pc34_compat(
    const redmcsb_f0662_f0663_renderer_pc34_compat *renderer,
    const uint8_t *source_bitmap,
    uint8_t *destination_bitmap,
    const uint8_t *palette_changes)
{
    int16_t width;
    int16_t height;

    if (!valid_renderer(renderer) || source_bitmap == NULL ||
        destination_bitmap == NULL) {
        return 0;
    }
    width = bitmap_dimension(source_bitmap, -4);
    height = bitmap_dimension(source_bitmap, -2);
    memcpy(destination_bitmap - 4, source_bitmap - 4, 4U);
    renderer->blit_shrink_palette(renderer->context, source_bitmap,
                                  destination_bitmap, width, height, width,
                                  height, palette_changes);
    return 1;
}

const char *redmcsb_f0662_f0663_palette_bitmap_source_evidence_pc34(void)
{
    return "ReDMCSB BASE.C F0662_ApplyPaletteChanges (1548-1553); "
           "F0663_CopyBitmapWithPaletteChanges (1555-1568); "
           "DEFS.H M100/M101 (3444-3445)";
}
