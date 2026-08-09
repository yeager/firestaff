#include "nexus_v1_bpk_capture_compositor.h"

#include <stdio.h>
#include <string.h>

static void wb32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24U);
    p[1] = (uint8_t)(value >> 16U);
    p[2] = (uint8_t)(value >> 8U);
    p[3] = (uint8_t)value;
}

static void make_archive(uint8_t *data, size_t size)
{
    uint32_t entry = 64U;
    uint32_t palt = (uint32_t)(size - 524U);
    uint32_t i;

    memset(data, 0, size);
    memcpy(data, "BPPK", 4);
    wb32(data + 4, (uint32_t)size);
    memcpy(data + 12, "BMPD", 4);
    wb32(data + 16, (uint32_t)(size - 20U));
    wb32(data + 20, 1U);
    wb32(data + 24, entry);
    data[entry + 12] = 0;
    data[entry + 13] = 2;
    data[entry + 15] = 2;
    data[entry + 19] = NEXUS_V1_BPK_MODE_8BPP;
    data[entry + 20] = 1;
    data[entry + 21] = 2;
    data[entry + 22] = 3;
    data[entry + 23] = 0;
    memcpy(data + palt, "PALT", 4);
    wb32(data + palt + 4, 524U);
    wb32(data + palt + 8, 256U);
    for (i = 0; i < 256U; ++i) {
        data[palt + 12U + i * 2U] = (uint8_t)(i >> 8U);
        data[palt + 13U + i * 2U] = (uint8_t)i;
    }
}

int main(void)
{
    uint8_t archive[64U + 24U + 524U];
    uint8_t pixels[4] = {1, 2, 3, 0};
    uint8_t palette[512];
    Nexus_V1_BpkCaptureSurfaceInput input;
    Nexus_V1_BpkCaptureSurfaceReceipt receipt;
    Nexus_Framebuffer framebuffer;
    uint32_t i;

    make_archive(archive, sizeof(archive));
    for (i = 0; i < 256U; ++i) {
        palette[i * 2U] = (uint8_t)(i >> 8U);
        palette[i * 2U + 1U] = (uint8_t)i;
    }
    memset(&input, 0, sizeof(input));
    input.archive = archive;
    input.archive_size = sizeof(archive);
    input.entry_index = 0U;
    input.capture_pixels = pixels;
    input.capture_pixels_size = sizeof(pixels);
    input.capture_width = 2;
    input.capture_height = 2;
    input.capture_stride = 2;
    input.capture_palette = palette;
    input.capture_palette_size = sizeof(palette);
    input.destination_x = 4;
    input.destination_y = 5;
    input.bpk_hash_verified = 1;
    input.original_saturn_capture_verified = 1;
    input.transparent_index_zero_verified = 1;

    nexus_fb_init(&framebuffer);
    if (!nexus_v1_bpk_capture_surface(&framebuffer, &input, &receipt) ||
        !receipt.valid || !receipt.capture_only ||
        !receipt.prs3_pixel_join_verified ||
        !receipt.palt_cram_join_verified ||
        !receipt.explicit_placement_verified ||
        receipt.menu_semantics_proven || receipt.vdp2_layer_owner_proven ||
        receipt.written_pixels != 3 || framebuffer.color_buffer[5 * 320 + 4] != 1) {
        fprintf(stderr, "FAIL: exact BPK capture surface replay\n");
        return 1;
    }

    input.original_saturn_capture_verified = 0;
    if (nexus_v1_bpk_capture_surface(&framebuffer, &input, &receipt) != 0 ||
        receipt.valid) {
        fprintf(stderr, "FAIL: unauthenticated BPK capture was admitted\n");
        return 1;
    }
    input.original_saturn_capture_verified = 1;
    pixels[1] = 0x7f;
    if (nexus_v1_bpk_capture_surface(&framebuffer, &input, &receipt) != 0 ||
        receipt.valid) {
        fprintf(stderr, "FAIL: mismatched BPK crop was admitted\n");
        return 1;
    }
    puts("test_nexus_v1_bpk_capture_compositor: PASS");
    return 0;
}
