#include "redmcsb_f8167_f8168_mouse_pointer_c25_pc34_compat.h"

bool redmcsb_f8167_capture_mouse_pointer_c25_pc34_compat(
    const RedmcsbF0680C25VgaAperturePc34Compat *aperture,
    int16_t x, int16_t y, uint8_t *prefixed_bitmap,
    size_t prefixed_bitmap_byte_count)
{
    uint16_t width;
    uint16_t height;

    if (x < 0 || y < 0 || (uint16_t)x > REDMCSB_F8167_SCREEN_WIDTH_PC34 ||
        (uint16_t)y > REDMCSB_F8167_SCREEN_HEIGHT_PC34) {
        return false;
    }
    width = (uint16_t)(REDMCSB_F8167_SCREEN_WIDTH_PC34 - (uint16_t)x);
    if (width > REDMCSB_F8167_MOUSE_POINTER_MAX_WIDTH_PC34) {
        width = REDMCSB_F8167_MOUSE_POINTER_MAX_WIDTH_PC34;
    }
    height = (uint16_t)(REDMCSB_F8167_SCREEN_HEIGHT_PC34 - (uint16_t)y);
    if (height > REDMCSB_F8167_MOUSE_POINTER_MAX_HEIGHT_PC34) {
        height = REDMCSB_F8167_MOUSE_POINTER_MAX_HEIGHT_PC34;
    }

    /* VIDEODRV.C:3811-3822 calls F8165 once with operation one. */
    return redmcsb_f8165_prefixed_bitmap_c25_pc34_compat(
        aperture, (uint16_t)x, y, (int16_t)width, height, 1,
        prefixed_bitmap, prefixed_bitmap_byte_count, NULL);
}

bool redmcsb_f8168_restore_mouse_pointer_c25_pc34_compat(
    const uint8_t *prefixed_bitmap, size_t prefixed_bitmap_byte_count,
    RedmcsbF0680C25VgaAperturePc34Compat *aperture)
{
    /* VIDEODRV.C:3830-3833 delegates the saved G8133 bitmap to F8166. */
    return redmcsb_f8166_blit_prefixed_bitmap_c25_pc34_compat(
        prefixed_bitmap, prefixed_bitmap_byte_count, aperture);
}

const char *redmcsb_f8167_f8168_mouse_pointer_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3804-3835 C25 F8167/F8168: clamp the mouse "
           "snapshot to 18 by 18 and screen edges, capture through F8165, "
           "then restore the same prefixed bitmap through F8166.";
}
