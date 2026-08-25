#include "nexus_v1_stabg_capture_compositor.h"
#include "nexus_v1_ui_surfaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR_SIZE (8U + 4U + 2U * 40U * 21U)
#define PALETTE_SIZE 512U
#define TILE_SIZE (791U * 64U)
#define PART2_OFFSET (32U + DIR_SIZE)
#define PART3_OFFSET (PART2_OFFSET + PALETTE_SIZE)
#define ARCHIVE_SIZE (PART3_OFFSET + TILE_SIZE)

static void wb32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24U);
    p[1] = (uint8_t)(value >> 16U);
    p[2] = (uint8_t)(value >> 8U);
    p[3] = (uint8_t)value;
}

static void wb16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8U);
    p[1] = (uint8_t)value;
}

static uint8_t *make_stabg(void)
{
    uint8_t *data = (uint8_t *)calloc(1U, ARCHIVE_SIZE);
    size_t i;
    if (!data) return NULL;
    memcpy(data, "STMP", 4);
    wb32(data + 4, ARCHIVE_SIZE);
    wb32(data + 8, 32U);
    wb32(data + 12, DIR_SIZE);
    wb32(data + 16, PART2_OFFSET);
    wb32(data + 20, PALETTE_SIZE);
    wb32(data + 24, PART3_OFFSET);
    wb32(data + 28, TILE_SIZE);
    wb32(data + 32, 48U);
    wb32(data + 36, 0U);
    wb16(data + 40, 40U);
    wb16(data + 42, 21U);
    for (i = 0U; i < 40U * 21U; ++i)
        wb16(data + 44U + i * 2U, 0U);
    for (i = 0U; i < 256U; ++i) {
        /* The source palette is little-endian BGR555. */
        data[PART2_OFFSET + i * 2U] = 0U;
        data[PART2_OFFSET + i * 2U + 1U] = (uint8_t)i;
    }
    for (i = 0U; i < 64U; ++i)
        data[PART3_OFFSET + i] = 1U;
    return data;
}

int main(void)
{
    uint8_t *stabg = make_stabg();
    uint8_t pixels[320U * 168U];
    uint8_t palette[PALETTE_SIZE];
    uint16_t decoded_palette[256];
    Nexus_UI_StabgPixelDecodeReceipt decoded;
    Nexus_V1_StabgCaptureInput input;
    Nexus_V1_StabgCaptureReceipt receipt;
    Nexus_Framebuffer framebuffer;

    if (!stabg) return 1;
    if (nexus_ui_stabg_decode_first_map(stabg, ARCHIVE_SIZE, pixels,
                                        sizeof(pixels), decoded_palette,
                                        &decoded) != 0 || !decoded.valid) {
        free(stabg);
        return 1;
    }
    memcpy(palette, stabg + PART2_OFFSET, sizeof(palette));
    memset(&input, 0, sizeof(input));
    input.stabg = stabg;
    input.stabg_size = ARCHIVE_SIZE;
    input.capture_pixels = pixels;
    input.capture_pixels_size = sizeof(pixels);
    input.capture_width = 320;
    input.capture_height = 168;
    input.capture_stride = 320;
    input.capture_palette = palette;
    input.capture_palette_size = sizeof(palette);
    input.destination_x = 0;
    input.destination_y = 0;
    input.source_hash_verified = 1;
    input.original_saturn_capture_verified = 1;
    input.transparent_index_zero_verified = 1;

    nexus_fb_init(&framebuffer);
    if (!nexus_v1_stabg_capture_composite(&framebuffer, &input, &receipt) ||
        !receipt.valid || !receipt.capture_only ||
        !receipt.dmweb_decode_verified || !receipt.pixel_join_verified ||
        !receipt.palette_join_verified || !receipt.explicit_placement_verified ||
        receipt.vdp2_layer_owner_proven || receipt.renderer_permitted ||
        receipt.written_pixels <= 0) {
        fprintf(stderr, "FAIL: exact STABG capture surface replay\n");
        free(stabg);
        return 1;
    }

    pixels[0] = 2U;
    if (nexus_v1_stabg_capture_composite(&framebuffer, &input, &receipt) != 0 ||
        receipt.valid) {
        fprintf(stderr, "FAIL: mismatched STABG crop was admitted\n");
        free(stabg);
        return 1;
    }
    free(stabg);
    puts("test_nexus_v1_stabg_capture_compositor: PASS");
    return 0;
}
