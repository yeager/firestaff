#include "nexus_v1_viewport.h"
#include "nexus_v1_ui_surfaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STABG_DIR_SIZE (8U + 4U + 2U * 40U * 21U)
#define STABG_PART2_OFFSET (32U + STABG_DIR_SIZE)
#define STABG_PART3_OFFSET (STABG_PART2_OFFSET + 512U)
#define STABG_SIZE (STABG_PART3_OFFSET + 791U * 64U)

static void wb16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)(value >> 8U);
    p[1] = (uint8_t)value;
}

static void wb32(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)(value >> 24U);
    p[1] = (uint8_t)(value >> 16U);
    p[2] = (uint8_t)(value >> 8U);
    p[3] = (uint8_t)value;
}

static uint8_t *make_stabg_fixture(void)
{
    uint8_t *data = (uint8_t *)calloc(1U, STABG_SIZE);
    size_t i;
    if (!data) return NULL;
    memcpy(data, "STMP", 4);
    wb32(data + 4, STABG_SIZE);
    wb32(data + 8, 32U);
    wb32(data + 12, STABG_DIR_SIZE);
    wb32(data + 16, STABG_PART2_OFFSET);
    wb32(data + 20, 512U);
    wb32(data + 24, STABG_PART3_OFFSET);
    wb32(data + 28, 791U * 64U);
    wb32(data + 32, 48U);
    wb32(data + 36, 0U);
    wb16(data + 40, 40U);
    wb16(data + 42, 21U);
    for (i = 0U; i < 40U * 21U; ++i)
        wb16(data + 44U + i * 2U, 0U);
    for (i = 0U; i < 64U; ++i)
        data[STABG_PART3_OFFSET + i] = 1U;
    return data;
}

static void wl16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8U);
}

int main(void)
{
    uint8_t command[NEXUS_V1_VDP1_COMMAND_BYTES] = {0};
    uint8_t texture[32];
    uint8_t dgn_image[32];
    uint8_t palette[32] = {0};
    uint8_t dgn_palette[32];
    uint8_t bitmap[NEXUS_V1_VDP2_NBG1_BITMAP_BYTES] = {0};
    uint8_t cram[4096] = {0};
    uint8_t registers[NEXUS_V1_VDP2_REGISTERS_BYTES] = {0};
    Nexus_V1_Vdp1CaptureCompositeInput vdp1;
    Nexus_V1_Vdp1CaptureSequenceInput sequence;
    Nexus_V1_Vdp2CaptureCompositeInput vdp2;
    Nexus_V1_Vdp12CaptureCompositionInput composition;
    Nexus_V1_Vdp12CaptureCompositionReceipt receipt;
    Nexus_V1_StabgCaptureInput stabg_input;
    uint8_t *stabg;
    uint8_t stabg_pixels[320U * 168U];
    uint8_t stabg_palette[512];
    uint16_t stabg_palette_words[256];
    Nexus_UI_StabgPixelDecodeReceipt stabg_decode;
    Nexus_Viewport viewport;
    int i;

    memset(texture, 0x12, sizeof(texture));
    for (i = 0; i < (int)sizeof(texture); i += 2) {
        dgn_image[i] = texture[i + 1];
        dgn_image[i + 1] = texture[i];
    }
    for (i = 0; i < 16; ++i) {
        wl16(palette + i * 2, 0x8000U | (unsigned)i);
        dgn_palette[i * 2] = palette[i * 2 + 1];
        dgn_palette[i * 2 + 1] = palette[i * 2];
        wl16(cram + i * 2, 0x8000U | (unsigned)i);
    }
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
    memset(&vdp1, 0, sizeof(vdp1));
    vdp1.command = command; vdp1.command_size = sizeof(command);
    vdp1.texture_span = texture; vdp1.texture_span_size = sizeof(texture);
    vdp1.palette_state = palette; vdp1.palette_state_size = sizeof(palette);
    vdp1.dgn_image = dgn_image; vdp1.dgn_image_size = sizeof(dgn_image);
    vdp1.dgn_palette = dgn_palette; vdp1.dgn_palette_size = sizeof(dgn_palette);
    vdp1.dgn_source_hash_verified = 1;
    vdp1.original_saturn_capture_verified = 1;
    vdp1.screen_origin_x = 160; vdp1.screen_origin_y = 112;
    vdp1.palette_slot_base = 32;
    memset(&sequence, 0, sizeof(sequence));
    sequence.commands = &vdp1; sequence.command_count = 1;
    sequence.system_clip_state_verified = 1;
    sequence.local_coordinate_state_verified = 1;
    sequence.command_order_verified = 1;
    sequence.end_record_verified = 1;
    bitmap[0] = 1;
    wb16(registers + 0x20, 0x0002);
    wb16(registers + 0x28, 0x1211);
    memset(&vdp2, 0, sizeof(vdp2));
    vdp2.capture_bitmap = bitmap; vdp2.capture_bitmap_size = sizeof(bitmap);
    vdp2.capture_cram = cram; vdp2.capture_cram_size = sizeof(cram);
    vdp2.vdp2_registers = registers; vdp2.vdp2_registers_size = sizeof(registers);
    vdp2.source_bitmap = bitmap; vdp2.source_bitmap_size = sizeof(bitmap);
    vdp2.source_palette = cram; vdp2.source_palette_size = 512;
    vdp2.source_hash_verified = 1; vdp2.original_saturn_capture_verified = 1;
    vdp2.transparent_index_zero_verified = 1;
    vdp2.source_x = 0; vdp2.source_y = 0; vdp2.destination_x = 0;
    vdp2.destination_y = 0; vdp2.width = 1; vdp2.height = 1;
    memset(&composition, 0, sizeof(composition));
    composition.vdp2_bitmap = &vdp2; composition.vdp1_sequence = &sequence;
    composition.vdp1_over_vdp2 = 1; composition.layer_order_verified = 1;
    nexus_viewport_init(&viewport);
    if (!nexus_viewport_replay_vdp12_capture_composition(
            &viewport, &composition, &receipt) || !receipt.valid ||
        !receipt.vdp2_verified || !receipt.vdp1_verified ||
        !receipt.renderer_permitted) {
        fprintf(stderr, "FAIL: authenticated VDP2→VDP1 composition\n");
        return 1;
    }
    stabg = make_stabg_fixture();
    if (!stabg || nexus_ui_stabg_decode_first_map(
            stabg, STABG_SIZE, stabg_pixels, sizeof(stabg_pixels),
            stabg_palette_words, &stabg_decode) != 0 || !stabg_decode.valid) {
        free(stabg);
        fprintf(stderr, "FAIL: STABG fixture preparation\n");
        return 1;
    }
    memcpy(stabg_palette, stabg + STABG_PART2_OFFSET,
           sizeof(stabg_palette));
    memset(&stabg_input, 0, sizeof(stabg_input));
    stabg_input.stabg = stabg;
    stabg_input.stabg_size = STABG_SIZE;
    stabg_input.capture_pixels = stabg_pixels;
    stabg_input.capture_pixels_size = sizeof(stabg_pixels);
    stabg_input.capture_width = 320;
    stabg_input.capture_height = 168;
    stabg_input.capture_stride = 320;
    stabg_input.capture_palette = stabg_palette;
    stabg_input.capture_palette_size = sizeof(stabg_palette);
    stabg_input.source_hash_verified = 1;
    stabg_input.original_saturn_capture_verified = 1;
    stabg_input.transparent_index_zero_verified = 1;
    composition.vdp2_bitmap = NULL;
    composition.vdp2_stabg = &stabg_input;
    composition.vdp2_is_stabg = 1;
    composition.vdp1_over_vdp2 = 0;
    composition.vdp2_over_vdp1 = 1;
    if (!nexus_viewport_replay_vdp12_capture_composition(
            &viewport, &composition, &receipt) || !receipt.valid ||
        !receipt.vdp2_source_stabg || !receipt.vdp2_over_vdp1 ||
        !receipt.vdp2_verified || !receipt.vdp1_verified) {
        free(stabg);
        fprintf(stderr, "FAIL: STABG HUD over viewport composition\n");
        return 1;
    }
    free(stabg);
    composition.vdp2_stabg = NULL;
    composition.vdp2_is_stabg = 0;
    composition.vdp2_bitmap = &vdp2;
    composition.vdp2_over_vdp1 = 0;
    composition.vdp1_over_vdp2 = 1;
    composition.layer_order_verified = 0;
    if (nexus_viewport_replay_vdp12_capture_composition(
            &viewport, &composition, &receipt) != 0 || receipt.valid) {
        fprintf(stderr, "FAIL: unverified layer order was admitted\n");
        return 1;
    }
    composition.layer_order_verified = 1;
    composition.vdp1_over_vdp2 = 0;
    composition.vdp2_over_vdp1 = 1;
    if (!nexus_viewport_replay_vdp12_capture_composition(
            &viewport, &composition, &receipt) || !receipt.valid ||
        !receipt.vdp2_over_vdp1 || receipt.vdp1_over_vdp2) {
        fprintf(stderr, "FAIL: authenticated VDP1→VDP2 composition\n");
        return 1;
    }
    composition.vdp1_over_vdp2 = 1;
    if (nexus_viewport_replay_vdp12_capture_composition(
            &viewport, &composition, &receipt) != 0 || receipt.valid) {
        fprintf(stderr, "FAIL: ambiguous layer order was admitted\n");
        return 1;
    }
    puts("test_nexus_v1_vdp12_capture_composition: PASS");
    return 0;
}
