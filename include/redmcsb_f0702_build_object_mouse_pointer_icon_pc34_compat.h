#ifndef FIRESTAFF_REDMCSB_F0702_BUILD_OBJECT_MOUSE_POINTER_ICON_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0702_BUILD_OBJECT_MOUSE_POINTER_ICON_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB IO.C F0702_BuildObjectMousePointerIcon, MEDIA463_I34E_I34M.
 *
 * PC 3.4 passes a packed 16x16 4bpp object bitmap. The routine produces an
 * 18x18 packed 4bpp mouse bitmap: all pixels initially use color 12, then a
 * palette-remapped shadow is drawn at (2,2), followed by the unmodified
 * object at (0,0). Both blits use color 12 as the transparent color.
 */
enum {
    REDMCSB_F0702_OBJECT_WIDTH_PC34 = 16,
    REDMCSB_F0702_OBJECT_HEIGHT_PC34 = 16,
    REDMCSB_F0702_OBJECT_BITMAP_BYTES_PC34 = 128,
    REDMCSB_F0702_POINTER_WIDTH_PC34 = 18,
    REDMCSB_F0702_POINTER_HEIGHT_PC34 = 18,
    REDMCSB_F0702_POINTER_BITMAP_BYTES_PC34 = 162,
    REDMCSB_F0702_TRANSPARENT_COLOR_PC34 = 12
};

/* Returns false for undersized or null buffers. On failure target is not
 * modified. The object and target formats use the high nibble as the left
 * pixel, matching M075_BITMAP_BYTE_COUNT and F0132_VIDEO_Blit on PC 3.4.
 */
bool redmcsb_f0702_build_object_mouse_pointer_icon_pc34_compat(
    const uint8_t *object_bitmap,
    size_t object_bitmap_size,
    uint8_t *target_bitmap,
    size_t target_bitmap_size);

const char *redmcsb_f0702_build_object_mouse_pointer_icon_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
