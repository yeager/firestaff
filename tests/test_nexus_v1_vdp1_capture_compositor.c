#include "nexus_v1_vdp1_capture_compositor.h"

#include <stdio.h>
#include <string.h>

static void wl16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

int main(void)
{
    uint8_t command[NEXUS_V1_VDP1_COMMAND_BYTES] = {0};
    uint8_t texture[32] = {0};
    uint8_t dgn_image[32];
    uint8_t palette_state[32] = {0};
    uint8_t dgn_palette[32];
    Nexus_Framebuffer framebuffer;
    Nexus_V1_Vdp1CaptureCompositeInput input;
    Nexus_V1_Vdp1CaptureCompositeReceipt receipt;
    int i;

    /* 16x4 mode-1 quad, signed coordinates around the Saturn display
     * centre. Capture lanes are little-endian word images; DGN source lanes
     * are their canonical big-endian word order. */
    wl16(command + 0, 0x1002U);
    wl16(command + 4, 1U << 3);
    wl16(command + 6, 0x3278U);
    wl16(command + 8, 0x0040U);
    wl16(command + 10, 0x0402U);
    wl16(command + 12, 0xfff0U);
    wl16(command + 14, 0xfff0U);
    wl16(command + 16, 0x0010U);
    wl16(command + 18, 0xfff0U);
    wl16(command + 20, 0x0010U);
    wl16(command + 22, 0x0010U);
    wl16(command + 24, 0xfff0U);
    wl16(command + 26, 0x0010U);
    for (i = 0; i < (int)sizeof(texture); ++i) texture[i] = 0x12U;
    for (i = 0; i < (int)sizeof(texture); i += 2) {
        dgn_image[i] = texture[i + 1];
        dgn_image[i + 1] = texture[i];
    }
    for (i = 0; i < 16; ++i) {
        wl16(palette_state + i * 2, (unsigned int)(0x8000U + i));
        dgn_palette[i * 2] = palette_state[i * 2 + 1];
        dgn_palette[i * 2 + 1] = palette_state[i * 2];
    }
    memset(&input, 0, sizeof(input));
    input.command = command;
    input.command_size = sizeof(command);
    input.texture_span = texture;
    input.texture_span_size = sizeof(texture);
    input.palette_state = palette_state;
    input.palette_state_size = sizeof(palette_state);
    input.dgn_image = dgn_image;
    input.dgn_image_size = sizeof(dgn_image);
    input.dgn_palette = dgn_palette;
    input.dgn_palette_size = sizeof(dgn_palette);
    input.dgn_source_hash_verified = 1;
    input.original_saturn_capture_verified = 1;
    input.screen_origin_x = 160;
    input.screen_origin_y = 112;
    input.palette_slot_base = 32;

    nexus_fb_init(&framebuffer);
    nexus_fb_clear(&framebuffer);
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_vdp1_capture_composite_mode1(
            &framebuffer, &input, &receipt) || !receipt.valid ||
        !receipt.source_join_verified || !receipt.palette_join_verified ||
        !receipt.renderer_permitted || receipt.written_pixels <= 0 ||
        framebuffer.color_buffer[112 * NEXUS_FB_W + 160] != 33U) {
        fprintf(stderr, "FAIL: authenticated VDP1 capture composite\n");
        return 1;
    }
    /* ECD is active in this command: an F texel ends its source row. */
    texture[0] = 0x1fU;
    for (i = 0; i < (int)sizeof(texture); i += 2) {
        dgn_image[i] = texture[i + 1];
        dgn_image[i + 1] = texture[i];
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_vdp1_capture_composite_mode1(
            &framebuffer, &input, &receipt) || receipt.end_code_pixels <= 0 ||
        receipt.written_pixels <= 0) {
        fprintf(stderr, "FAIL: VDP1 end-code row termination\n");
        return 1;
    }
    input.original_saturn_capture_verified = 0;
    if (nexus_v1_vdp1_capture_composite_mode1(
            &framebuffer, &input, &receipt) || receipt.renderer_permitted) {
        fprintf(stderr, "FAIL: unauthenticated VDP1 composite was admitted\n");
        return 1;
    }
    puts("test_nexus_v1_vdp1_capture_compositor: PASS");
    return 0;
}
