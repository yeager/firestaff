#include "dm1_v1_amiga_entrance_f0441.h"

#include "dm1_v1_amiga_graphics_dat.h"
#include "dm1_v1_amiga_title_f0437.h"

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

static unsigned int count_nonzero(const uint8_t *pixels, size_t count) {
    size_t index;
    unsigned int nonzero = 0u;
    for (index = 0u; index < count; ++index) {
        nonzero += pixels[index] != 0u;
    }
    return nonzero;
}

int dm1_v1_amiga_entrance_f0441_receipt(
    const uint8_t *graphics_dat,
    size_t graphics_dat_bytes,
    const uint8_t *swsh_executable,
    size_t swsh_executable_bytes,
    DM1_V1_AmigaEntranceF0441Receipt *out_receipt) {
    static const uint16_t a20_entrance_palette_rgb4[
        DM1_V1_AMIGA_ENTRANCE_F0441_PALETTE_ENTRIES] = {
        0x000u, 0x666u, 0x888u, 0x840u, 0xca8u, 0x0c0u, 0x080u, 0x0a0u,
        0x864u, 0xf00u, 0xa86u, 0x642u, 0x444u, 0xaaau, 0x620u, 0xfffu
    };
    DM1_V1_AmigaTitleF0437Receipt title_receipt;
    DM1_V1_AmigaGraphicsReceipt graphics_receipt;
    DM1_V1_AmigaEntranceF0441Receipt receipt;
    uint8_t *screen = NULL;
    uint8_t *left = NULL;
    uint8_t *right = NULL;
    const size_t pixel_capacity = 320u * 200u;
    uint16_t screen_width = 0u;
    uint16_t screen_height = 0u;
    uint16_t left_width = 0u;
    uint16_t left_height = 0u;
    uint16_t right_width = 0u;
    uint16_t right_height = 0u;
    int valid = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!graphics_dat || !swsh_executable || !out_receipt ||
        !dm1_v1_amiga_title_f0437_receipt(
            graphics_dat, graphics_dat_bytes, swsh_executable,
            swsh_executable_bytes, &title_receipt) ||
        dm1_v1_amiga_graphics_receipt(
            graphics_dat, graphics_dat_bytes, &graphics_receipt) != 0) {
        return 0;
    }

    screen = (uint8_t *)malloc(pixel_capacity);
    left = (uint8_t *)malloc(pixel_capacity);
    right = (uint8_t *)malloc(pixel_capacity);
    if (!screen || !left || !right ||
        !dm1_v1_amiga_graphics_decode(
            graphics_dat, graphics_dat_bytes,
            DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_SCREEN,
            screen, pixel_capacity, &screen_width, &screen_height) ||
        !dm1_v1_amiga_graphics_decode(
            graphics_dat, graphics_dat_bytes,
            DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_LEFT_DOOR,
            left, pixel_capacity, &left_width, &left_height) ||
        !dm1_v1_amiga_graphics_decode(
            graphics_dat, graphics_dat_bytes,
            DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_RIGHT_DOOR,
            right, pixel_capacity, &right_width, &right_height)) {
        goto cleanup;
    }

    if (screen_width != 320u || screen_height != 200u ||
        left_width != 128u || left_height != 161u ||
        right_width != 128u || right_height != 161u) {
        goto cleanup;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    md5_to_hex(graphics_receipt.md5, receipt.graphics_md5);
    (void)snprintf(receipt.executable_md5,
                   sizeof(receipt.executable_md5), "%s",
                   title_receipt.executable_md5);
    receipt.screen_width = screen_width;
    receipt.screen_height = screen_height;
    receipt.left_door_width = left_width;
    receipt.left_door_height = left_height;
    receipt.right_door_width = right_width;
    receipt.right_door_height = right_height;
    receipt.screen_nonzero_pixels = count_nonzero(
        screen, (size_t)screen_width * screen_height);
    receipt.left_door_nonzero_pixels = count_nonzero(
        left, (size_t)left_width * left_height);
    receipt.right_door_nonzero_pixels = count_nonzero(
        right, (size_t)right_width * right_height);
    receipt.door_frame_count = DM1_V1_AMIGA_ENTRANCE_F0441_DOOR_FRAMES;
    receipt.opening_steps = DM1_V1_AMIGA_ENTRANCE_F0441_OPENING_STEPS;
    receipt.switch_delay_ticks =
        DM1_V1_AMIGA_ENTRANCE_F0441_SWITCH_DELAY_TICKS;
    receipt.mouse_input_only = 1;
    memcpy(receipt.entrance_palette_rgb4, a20_entrance_palette_rgb4,
           sizeof(a20_entrance_palette_rgb4));
    receipt.source_evidence =
        "ReDMCSB ENTRANCE.C F0441 A20: C002/C003 128x161, C004 320x200, "
        "8 door frames, mouse-only input, C01 switch and Delay(20); "
        "DATA.C MEDIA424 G0020 RGB4 palette";

    valid = receipt.screen_nonzero_pixels > 0u &&
            receipt.left_door_nonzero_pixels > 0u &&
            receipt.right_door_nonzero_pixels > 0u;
    if (valid) *out_receipt = receipt;

cleanup:
    free(screen);
    free(left);
    free(right);
    return valid;
}
