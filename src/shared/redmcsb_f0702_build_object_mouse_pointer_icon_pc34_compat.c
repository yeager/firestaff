#include "redmcsb_f0702_build_object_mouse_pointer_icon_pc34_compat.h"

#include <string.h>

/* ReDMCSB IO.C:532 and BLTSHRNK.C F0129, PC I34E/I34M branch. */
static const uint8_t k_shadow_palette[16] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 12, 0, 0, 0
};

static uint8_t get_pixel(const uint8_t *bitmap, size_t width, size_t x, size_t y)
{
    const uint8_t value = bitmap[(y * (width / 2U)) + (x / 2U)];

    return (x & 1U) == 0U ? (uint8_t)(value >> 4) : (uint8_t)(value & 0x0fU);
}

static void set_pixel(uint8_t *bitmap,
                      size_t width,
                      size_t x,
                      size_t y,
                      uint8_t color)
{
    uint8_t *const value = &bitmap[(y * (width / 2U)) + (x / 2U)];

    color &= 0x0fU;
    if ((x & 1U) == 0U) {
        *value = (uint8_t)((*value & 0x0fU) | (color << 4));
    } else {
        *value = (uint8_t)((*value & 0xf0U) | color);
    }
}

/* F0132_VIDEO_Blit for the two fixed F0702 PC rectangles. */
static void blit_with_transparency(const uint8_t *source,
                                   uint8_t *target,
                                   size_t target_x,
                                   size_t target_y)
{
    size_t y;

    for (y = 0U; y < REDMCSB_F0702_OBJECT_HEIGHT_PC34; y++) {
        size_t x;

        for (x = 0U; x < REDMCSB_F0702_OBJECT_WIDTH_PC34; x++) {
            const uint8_t color = get_pixel(source,
                                            REDMCSB_F0702_OBJECT_WIDTH_PC34,
                                            x, y);

            if (color != REDMCSB_F0702_TRANSPARENT_COLOR_PC34) {
                set_pixel(target, REDMCSB_F0702_POINTER_WIDTH_PC34,
                          target_x + x, target_y + y, color);
            }
        }
    }
}

bool redmcsb_f0702_build_object_mouse_pointer_icon_pc34_compat(
    const uint8_t *object_bitmap,
    size_t object_bitmap_size,
    uint8_t *target_bitmap,
    size_t target_bitmap_size)
{
    uint8_t shadow[REDMCSB_F0702_OBJECT_BITMAP_BYTES_PC34];
    size_t pixel;

    if (object_bitmap == NULL || target_bitmap == NULL ||
        object_bitmap_size < REDMCSB_F0702_OBJECT_BITMAP_BYTES_PC34 ||
        target_bitmap_size < REDMCSB_F0702_POINTER_BITMAP_BYTES_PC34) {
        return false;
    }

    /* IO.C:1711, 1720, 1723-1725. The stack buffer is the equivalent of the
     * original temporary-top-of-heap 128-byte allocation. */
    for (pixel = 0U;
         pixel < REDMCSB_F0702_OBJECT_WIDTH_PC34 * REDMCSB_F0702_OBJECT_HEIGHT_PC34;
         pixel++) {
        const uint8_t input = get_pixel(object_bitmap,
                                        REDMCSB_F0702_OBJECT_WIDTH_PC34,
                                        pixel % REDMCSB_F0702_OBJECT_WIDTH_PC34,
                                        pixel / REDMCSB_F0702_OBJECT_WIDTH_PC34);

        set_pixel(shadow, REDMCSB_F0702_OBJECT_WIDTH_PC34,
                  pixel % REDMCSB_F0702_OBJECT_WIDTH_PC34,
                  pixel / REDMCSB_F0702_OBJECT_WIDTH_PC34,
                  k_shadow_palette[input]);
    }
    memset(target_bitmap, 0xcc, REDMCSB_F0702_POINTER_BITMAP_BYTES_PC34);
    blit_with_transparency(shadow, target_bitmap, 2U, 2U);
    blit_with_transparency(object_bitmap, target_bitmap, 0U, 0U);
    return true;
}

const char *redmcsb_f0702_build_object_mouse_pointer_icon_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:1694-1732 (MEDIA463_I34E_I34M); "
           "IO.C:532,545-547; BLTSHRNK.C:1104-1146";
}
