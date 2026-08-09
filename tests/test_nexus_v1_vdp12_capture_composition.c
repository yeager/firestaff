#include "nexus_v1_viewport.h"

#include <stdio.h>
#include <string.h>

static void wl16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8U);
}

static void wb16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)(value >> 8U);
    p[1] = (uint8_t)value;
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
    composition.layer_order_verified = 0;
    if (nexus_viewport_replay_vdp12_capture_composition(
            &viewport, &composition, &receipt) != 0 || receipt.valid) {
        fprintf(stderr, "FAIL: unverified layer order was admitted\n");
        return 1;
    }
    puts("test_nexus_v1_vdp12_capture_composition: PASS");
    return 0;
}
