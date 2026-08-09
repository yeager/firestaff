#include "nexus_v1_vdp2_capture_compositor.h"
#include "nexus_v1_viewport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void wb16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)(value >> 8U);
    p[1] = (uint8_t)value;
}

int main(void)
{
    uint8_t *bitmap = (uint8_t *)calloc(1, NEXUS_V1_VDP2_NBG1_BITMAP_BYTES);
    uint8_t *cram = (uint8_t *)calloc(1, 4096);
    uint8_t registers[NEXUS_V1_VDP2_REGISTERS_BYTES] = {0};
    Nexus_V1_Vdp2CaptureCompositeInput input;
    Nexus_V1_Vdp2CaptureCompositeReceipt receipt;
    Nexus_Viewport viewport;
    int i;

    if (!bitmap || !cram) return 1;
    wb16(registers + 0x20, 0x0002);
    wb16(registers + 0x28, 0x1211);
    wb16(registers + 0x2c, 0x0000);
    for (i = 0; i < 16; ++i) {
        bitmap[20 * 512 + 30 + i] = (uint8_t)(i + 1);
        wb16(cram + i * 2, 0x8000U | (unsigned int)i);
    }
    memset(&input, 0, sizeof(input));
    input.capture_bitmap = bitmap;
    input.capture_bitmap_size = NEXUS_V1_VDP2_NBG1_BITMAP_BYTES;
    input.capture_cram = cram;
    input.capture_cram_size = 4096;
    input.vdp2_registers = registers;
    input.vdp2_registers_size = sizeof(registers);
    input.source_bitmap = bitmap;
    input.source_bitmap_size = NEXUS_V1_VDP2_NBG1_BITMAP_BYTES;
    input.source_palette = cram;
    input.source_palette_size = NEXUS_V1_VDP2_NBG1_PALETTE_BYTES;
    input.source_hash_verified = 1;
    input.original_saturn_capture_verified = 1;
    input.transparent_index_zero_verified = 1;
    input.source_x = 30;
    input.source_y = 20;
    input.destination_x = 40;
    input.destination_y = 50;
    input.width = 16;
    input.height = 1;
    input.palette_base = 0;
    nexus_viewport_init(&viewport);
    if (!nexus_viewport_replay_vdp2_nbg1_capture(&viewport, &input,
                                                 &receipt) ||
        !receipt.valid || receipt.written_pixels != 16 ||
        viewport.fb.color_buffer[50 * NEXUS_FB_W + 40] != 1U) {
        fprintf(stderr, "FAIL: authenticated VDP2 NBG1 replay\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    input.original_saturn_capture_verified = 0;
    if (nexus_v1_vdp2_capture_composite_nbg1_bitmap(
            &viewport.fb, &input, &receipt) || receipt.renderer_permitted) {
        fprintf(stderr, "FAIL: unauthenticated VDP2 replay was admitted\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    wb16(registers + 0x2c, 0x0000);
    wb16(registers + NEXUS_V1_VDP2_CRAOFA_OFFSET, 0x0010);
    if (nexus_v1_vdp2_capture_composite_nbg1_bitmap(
            &viewport.fb, &input, &receipt) || receipt.renderer_permitted) {
        fprintf(stderr, "FAIL: unbound NBG1 CRAM offset was admitted\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    input.original_saturn_capture_verified = 1;
    wb16(registers + 0x2c, 0x0100);
    if (nexus_v1_vdp2_capture_composite_nbg1_bitmap(
            &viewport.fb, &input, &receipt) || receipt.renderer_permitted) {
        fprintf(stderr, "FAIL: NBG0 palette bits were treated as NBG1\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    free(bitmap);
    free(cram);
    puts("test_nexus_v1_vdp2_capture_compositor: PASS");
    return 0;
}
