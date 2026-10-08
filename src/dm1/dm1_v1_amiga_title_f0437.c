#include "dm1_v1_amiga_title_f0437.h"

#include "dm1_v1_amiga_graphics_dat.h"
#include "dm1_v1_amiga_swsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void md5_to_hex(const uint8_t md5[16], char out[33]) {
    static const char digits[] = "0123456789abcdef";
    unsigned int index;
    for (index = 0u; index < 16u; ++index) {
        out[index * 2u] = digits[md5[index] >> 4u];
        out[index * 2u + 1u] = digits[md5[index] & 0x0fu];
    }
    out[32] = '\0';
}

static unsigned int fingerprint_pixels(const uint8_t *pixels, size_t bytes) {
    unsigned int hash = 2166136261u;
    size_t index;
    for (index = 0u; index < bytes; ++index) {
        hash ^= pixels[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static unsigned int count_nonzero(const uint8_t *pixels,
                                  unsigned int width,
                                  unsigned int y,
                                  unsigned int height) {
    size_t start = (size_t)y * width;
    size_t bytes = (size_t)height * width;
    size_t index;
    unsigned int count = 0u;
    for (index = 0u; index < bytes; ++index) {
        count += pixels[start + index] != 0u;
    }
    return count;
}

int dm1_v1_amiga_title_f0437_receipt(
    const uint8_t *graphics_dat,
    size_t graphics_dat_bytes,
    const uint8_t *swsh_executable,
    size_t swsh_executable_bytes,
    DM1_V1_AmigaTitleF0437Receipt *out_receipt) {
    static const uint8_t expected_graphics_md5[16] = {
        0x6a, 0x2f, 0x13, 0x5b, 0x53, 0xc2, 0x22, 0x0f,
        0x02, 0x51, 0xfa, 0x10, 0x3e, 0x2a, 0x6e, 0x7e
    };
    static const char expected_a20_executable_md5[] =
        "a0ffbcc7ae8cecac03128ddb32887ef4";
    DM1_V1_AmigaGraphicsReceipt graphics_receipt;
    DM1_V1_AmigaSwshAssets swsh_assets;
    DM1_V1_AmigaTitleF0437Receipt receipt;
    uint8_t *c001_pixels = NULL;
    uint16_t width = 0u;
    uint16_t height = 0u;
    size_t pixel_bytes =
        (size_t)DM1_V1_AMIGA_TITLE_WIDTH * DM1_V1_AMIGA_TITLE_HEIGHT;
    unsigned int index;
    int result = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!graphics_dat || !swsh_executable || !out_receipt ||
        !dm1_v1_amiga_swsh_parse(swsh_executable, swsh_executable_bytes,
                                 &swsh_assets) ||
        !swsh_assets.executableMd5 ||
        strcmp(swsh_assets.executableMd5, expected_a20_executable_md5) != 0 ||
        dm1_v1_amiga_graphics_receipt(graphics_dat, graphics_dat_bytes,
                                       &graphics_receipt) != 0 ||
        !graphics_receipt.is_amiga ||
        graphics_receipt.version != DM1_AMIGA_VER_2_0 ||
        graphics_receipt.lang != DM1_AMIGA_LANG_EN ||
        memcmp(graphics_receipt.md5, expected_graphics_md5,
               sizeof(expected_graphics_md5)) != 0) {
        return 0;
    }

    c001_pixels = (uint8_t *)malloc(pixel_bytes);
    if (!c001_pixels ||
        !dm1_v1_amiga_graphics_decode(
            graphics_dat, graphics_dat_bytes,
            DM1_V1_AMIGA_TITLE_GRAPHIC_C001,
            c001_pixels, pixel_bytes, &width, &height) ||
        width != DM1_V1_AMIGA_TITLE_WIDTH ||
        height != DM1_V1_AMIGA_TITLE_HEIGHT) {
        free(c001_pixels);
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    md5_to_hex(graphics_receipt.md5, receipt.graphics_md5);
    (void)snprintf(receipt.executable_md5,
                   sizeof(receipt.executable_md5), "%s",
                   swsh_assets.executableMd5);
    receipt.c001_width = width;
    receipt.c001_height = height;
    receipt.c001_pixel_fingerprint =
        fingerprint_pixels(c001_pixels, pixel_bytes);
    receipt.presents_nonzero_pixels = count_nonzero(c001_pixels, width,
                                                     137u, 16u);
    receipt.dungeon_nonzero_pixels = count_nonzero(c001_pixels, width,
                                                    0u, 80u);
    receipt.master_nonzero_pixels = count_nonzero(c001_pixels, width,
                                                   80u, 57u);
    receipt.presents_source_y = 137u;
    receipt.presents_source_height = 16u;
    receipt.presents_destination_y = 90u;
    receipt.master_source_y = 80u;
    receipt.master_source_height = 57u;
    receipt.master_destination_y = 118u;
    receipt.dungeon_source_y = 0u;
    receipt.dungeon_source_height = 80u;
    receipt.pre_zoom_vblanks = 1u;
    receipt.source_zoom_steps = DM1_V1_AMIGA_TITLE_ZOOM_STEP_COUNT;
    receipt.source_delay_ticks = 25u;
    receipt.source_beam_wait_line = 152u;
    for (index = 0u; index < DM1_V1_AMIGA_TITLE_RGB4_ENTRIES; ++index) {
        receipt.initial_palette[index] = 0x0004u;
        receipt.presents_palette[index] = 0x0004u;
        receipt.zoom_palette[index] = 0x0004u;
        receipt.final_palette[index] = 0x0004u;
    }
    receipt.presents_palette[15] = 0x0fffu;
    receipt.zoom_palette[3] = 0x0a82u;
    receipt.zoom_palette[4] = 0x0842u;
    receipt.zoom_palette[5] = 0x0ca2u;
    receipt.zoom_palette[6] = 0x0a62u;
    receipt.zoom_palette[8] = 0x0ff4u;
    receipt.zoom_palette[15] = 0x0f00u;
    receipt.final_palette[3] = 0x0a82u;
    receipt.final_palette[4] = 0x0842u;
    receipt.final_palette[5] = 0x0ca2u;
    receipt.final_palette[6] = 0x0a62u;
    receipt.final_palette[8] = 0x0ff4u;
    receipt.final_palette[10] = 0x0000u;
    receipt.final_palette[12] = 0x0f00u;
    receipt.final_palette[15] = 0x0f00u;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437 A20 branch: C001_GRAPHIC_TITLE; "
        "DATA.C G0003-G0005/G1075; DEFS.H MEDIA425 RGB4 palette; "
        "18 zoom blits, VBeamPos line 152, Delay(25L)";
    result = receipt.presents_nonzero_pixels > 0u &&
             receipt.dungeon_nonzero_pixels > 0u &&
             receipt.master_nonzero_pixels > 0u;
    if (result) *out_receipt = receipt;
    else memset(out_receipt, 0, sizeof(*out_receipt));
    free(c001_pixels);
    return result;
}

int dm1_v1_amiga_title_f0437_zoom_step(
    const DM1_V1_AmigaTitleF0437Receipt *receipt,
    unsigned int index,
    DM1_V1_AmigaTitleF0437ZoomStep *out_step) {
    DM1_V1_AmigaTitleF0437ZoomStep step;
    unsigned int width;
    unsigned int height;
    if (out_step) memset(out_step, 0, sizeof(*out_step));
    if (!receipt || !receipt->valid || !out_step ||
        index >= DM1_V1_AMIGA_TITLE_ZOOM_STEP_COUNT) {
        return 0;
    }
    width = 48u + index * 16u;
    height = 12u + index * 4u;
    memset(&step, 0, sizeof(step));
    step.index = index;
    step.source_width = DM1_V1_AMIGA_TITLE_WIDTH;
    step.source_height = 80u;
    step.destination_x = (DM1_V1_AMIGA_TITLE_WIDTH - width) / 2u;
    step.destination_y = (160u - height) / 2u;
    step.destination_width = width;
    step.destination_height = height;
    step.wait_vblanks_before = 0u;
    step.wait_for_beam_line = receipt->source_beam_wait_line;
    *out_step = step;
    return 1;
}
