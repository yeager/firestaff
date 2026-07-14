#include "redmcsb_f0674_viewport_bitmap_copy_pc34_compat.h"

#include <stddef.h>
#include <string.h>

int redmcsb_f0674_copy_viewport_bitmap_pc34_compat(
    int16_t graphic_index,
    uint8_t *destination_bitmap,
    size_t destination_byte_count,
    const redmcsb_f0674_viewport_bitmap_runtime_pc34_compat *runtime,
    size_t *copied_byte_count)
{
    const uint8_t *bitmap;
    size_t bitmap_byte_count;

    if (copied_byte_count != NULL) *copied_byte_count = 0U;
    if (destination_bitmap == NULL || runtime == NULL ||
        runtime->get_bitmap == NULL || runtime->get_bitmap_byte_count == NULL) {
        return 0;
    }
    bitmap = runtime->get_bitmap(runtime->context, graphic_index);
    if (bitmap == NULL) return 0;
    bitmap_byte_count = runtime->get_bitmap_byte_count(runtime->context, bitmap);
    if (bitmap_byte_count > destination_byte_count) return 0;
    memcpy(destination_bitmap, bitmap, bitmap_byte_count);
    if (copied_byte_count != NULL) *copied_byte_count = bitmap_byte_count;
    return 1;
}

const char *redmcsb_f0674_viewport_bitmap_copy_source_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C F0674_F0128_sub (3006-3015), "
           "PC I34E/I34M F0631 lookup followed by F0653-sized F0007 copy";
}
