#include "dm2_v1_fire_blit_rows.h"

static uint8_t dm2_get_4bpp_pixel(const uint8_t* bytes, uint16_t pixel) {
    uint8_t packed = bytes[pixel >> 1];
    return (pixel & 1U) ? (uint8_t)(packed & 0x0fU) : (uint8_t)(packed >> 4);
}

static void dm2_set_4bpp_pixel(uint8_t* bytes, uint16_t pixel, uint8_t value) {
    uint8_t* packed = &bytes[pixel >> 1];
    value &= 0x0fU;
    if (pixel & 1U) {
        *packed = (uint8_t)((*packed & 0xf0U) | value);
    } else {
        *packed = (uint8_t)((*packed & 0x0fU) | (uint8_t)(value << 4));
    }
}

void dm2_v1_fire_blit_rows_receipt_clear(DM2_V1_FireBlitRowsReceipt* receipt) {
    if (!receipt) {
        return;
    }
    receipt->handled = 0;
    receipt->source_locked = 0;
    receipt->bounds_ok = 0;
    receipt->copied_pixels = 0;
    receipt->skipped_colorkey_pixels = 0;
    receipt->symbol = 0;
    receipt->source_path = 0;
}

static int dm2_4bpp_range_ok(size_t size, uint16_t offset, uint16_t width) {
    size_t end;
    if (width == 0U) {
        return 1;
    }
    end = (size_t)offset + (size_t)width - 1U;
    return (end >> 1) < size;
}

static int dm2_8bpp_range_ok(size_t size, uint16_t offset, uint16_t width) {
    size_t end;
    if (width == 0U) {
        return 1;
    }
    end = (size_t)offset + (size_t)width;
    return end <= size;
}

static int dm2_4bpp_rect_ok(size_t size,
                            uint16_t offset,
                            uint16_t stride,
                            uint16_t width,
                            uint16_t height) {
    size_t last;
    if (width == 0U || height == 0U || stride == 0U || width > stride) {
        return 0;
    }
    last = (size_t)offset + (size_t)(height - 1U) * (size_t)stride +
           (size_t)(width - 1U);
    return (last >> 1) < size;
}

int dm2_v1_fire_blit_to_memory_row_4to4bpp(
    const uint8_t* src4,
    size_t src4_size,
    uint16_t off_src_pixels,
    uint8_t* dst4,
    size_t dst4_size,
    uint16_t off_dst_pixels,
    uint16_t width_pixels,
    DM2_V1_FireBlitRowsReceipt* out_receipt)
{
    uint16_t i;

    dm2_v1_fire_blit_rows_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "FIRE_BLIT_TO_MEMORY_ROW_4TO4BPP";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:6321";
    }
    if (!src4 || !dst4 ||
        !dm2_4bpp_range_ok(src4_size, off_src_pixels, width_pixels) ||
        !dm2_4bpp_range_ok(dst4_size, off_dst_pixels, width_pixels)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->bounds_ok = 1;
    }
    for (i = 0; i < width_pixels; ++i) {
        dm2_set_4bpp_pixel(dst4, (uint16_t)(off_dst_pixels + i),
                           dm2_get_4bpp_pixel(src4, (uint16_t)(off_src_pixels + i)));
    }
    if (out_receipt) {
        out_receipt->copied_pixels = (int)width_pixels;
    }
    return 1;
}

int dm2_v1_fire_blit_to_memory_row_4to8bpp(
    const uint8_t* src4,
    size_t src4_size,
    uint16_t off_src_pixels,
    uint8_t* dst8,
    size_t dst8_size,
    uint16_t off_dst_pixels,
    uint16_t width_pixels,
    const uint8_t palette16[16],
    int colorkey,
    DM2_V1_FireBlitRowsReceipt* out_receipt)
{
    uint16_t i;
    uint8_t key = (uint8_t)(colorkey & 0x0f);

    dm2_v1_fire_blit_rows_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "FIRE_BLIT_TO_MEMORY_ROW_4TO8BPP";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:7368";
    }
    if (!src4 || !dst8 || !palette16 ||
        !dm2_4bpp_range_ok(src4_size, off_src_pixels, width_pixels) ||
        !dm2_8bpp_range_ok(dst8_size, off_dst_pixels, width_pixels)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->bounds_ok = 1;
    }
    for (i = 0; i < width_pixels; ++i) {
        uint8_t pixel = dm2_get_4bpp_pixel(src4, (uint16_t)(off_src_pixels + i));
        if (pixel == key) {
            if (out_receipt) {
                out_receipt->skipped_colorkey_pixels++;
            }
            continue;
        }
        dst8[(size_t)off_dst_pixels + i] = palette16[pixel];
        if (out_receipt) {
            out_receipt->copied_pixels++;
        }
    }
    return 1;
}

int dm2_v1_fire_stretch_blit_to_memory_4to4bpp(
    const uint8_t* src4,
    size_t src4_size,
    uint16_t off_src_pixels,
    uint16_t src_stride_pixels,
    uint16_t src_width_pixels,
    uint16_t src_height_pixels,
    uint8_t* dst4,
    size_t dst4_size,
    uint16_t off_dst_pixels,
    uint16_t dst_stride_pixels,
    uint16_t dst_width_pixels,
    uint16_t dst_height_pixels,
    DM2_V1_FireBlitRowsReceipt* out_receipt)
{
    uint16_t dy;

    dm2_v1_fire_blit_rows_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "FIRE_STRETCH_BLIT_TO_MEMORY_4TO4BPP";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:6592";
    }
    if (!src4 || !dst4 ||
        !dm2_4bpp_rect_ok(src4_size, off_src_pixels, src_stride_pixels,
                          src_width_pixels, src_height_pixels) ||
        !dm2_4bpp_rect_ok(dst4_size, off_dst_pixels, dst_stride_pixels,
                          dst_width_pixels, dst_height_pixels)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->bounds_ok = 1;
    }
    for (dy = 0; dy < dst_height_pixels; ++dy) {
        uint16_t dx;
        uint16_t sy = (uint16_t)(((uint32_t)dy * src_height_pixels) /
                                 dst_height_pixels);
        for (dx = 0; dx < dst_width_pixels; ++dx) {
            uint16_t sx = (uint16_t)(((uint32_t)dx * src_width_pixels) /
                                     dst_width_pixels);
            uint16_t src_pixel = (uint16_t)(off_src_pixels +
                                            sy * src_stride_pixels + sx);
            uint16_t dst_pixel = (uint16_t)(off_dst_pixels +
                                            dy * dst_stride_pixels + dx);
            dm2_set_4bpp_pixel(dst4, dst_pixel,
                               dm2_get_4bpp_pixel(src4, src_pixel));
            if (out_receipt) {
                out_receipt->copied_pixels++;
            }
        }
    }
    return 1;
}

int dm2_v1_ibmio_load_4to8bpp_pal(
    const uint8_t* pal16,
    size_t pal16_size,
    uint8_t out_palette16[16],
    DM2_V1_FireBlitRowsReceipt* out_receipt)
{
    size_t i;

    dm2_v1_fire_blit_rows_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "IBMIO_LOAD_4TO8BPP_PAL";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:1412";
    }
    if (!pal16 || !out_palette16 || pal16_size < 16U) {
        return 0;
    }
    for (i = 0; i < 16U; ++i) {
        out_palette16[i] = pal16[i];
    }
    if (out_receipt) {
        out_receipt->bounds_ok = 1;
        out_receipt->copied_pixels = 16;
    }
    return 1;
}

int dm2_v1_ibmio_blit_row_4to8bpp(
    const uint8_t* src4,
    size_t src4_size,
    uint16_t off_src_pixels,
    uint8_t* dst8,
    size_t dst8_size,
    uint16_t off_dst_pixels,
    uint16_t width_pixels,
    const uint8_t palette16[16],
    DM2_V1_FireBlitRowsReceipt* out_receipt)
{
    uint16_t i;

    dm2_v1_fire_blit_rows_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "IBMIO_BLIT_ROW_4TO8BPP";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:1427";
    }
    if (!src4 || !dst8 || !palette16 ||
        !dm2_4bpp_range_ok(src4_size, off_src_pixels, width_pixels) ||
        !dm2_8bpp_range_ok(dst8_size, off_dst_pixels, width_pixels)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->bounds_ok = 1;
    }
    for (i = 0; i < width_pixels; ++i) {
        uint8_t pixel = dm2_get_4bpp_pixel(src4, (uint16_t)(off_src_pixels + i));
        dst8[(size_t)off_dst_pixels + i] = palette16[pixel];
    }
    if (out_receipt) {
        out_receipt->copied_pixels = (int)width_pixels;
    }
    return 1;
}

const char* dm2_v1_fire_blit_rows_source_evidence(void) {
    return "skproject SKWIN/SkWinCore.cpp FIRE_BLIT_TO_MEMORY_ROW_4TO4BPP:6321 "
           "FIRE_STRETCH_BLIT_TO_MEMORY_4TO4BPP:6592 and "
           "FIRE_BLIT_TO_MEMORY_ROW_4TO8BPP:7368 "
           "IBMIO_LOAD_4TO8BPP_PAL:1412 IBMIO_BLIT_ROW_4TO8BPP:1427";
}
