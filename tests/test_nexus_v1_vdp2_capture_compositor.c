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

static void wl16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8U);
}

static int run_external_bitmap_capture(void)
{
    const char *path = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE");
    const char *frame_text = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME");
    FILE *file;
    long file_size;
    uint8_t *capture;
    Nexus_V1_Vdp2BitmapCaptureFramebuffer framebuffer;
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_SaturnVdp2RegisterReceipt registers;
    Nexus_V1_Vdp2BitmapCaptureReceipt receipt;
    unsigned int frame_index;
    int ok;

    if (!path || !*path || !frame_text || !*frame_text) return 1;
    frame_index = (unsigned int)strtoul(frame_text, NULL, 0);
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    capture = (uint8_t *)malloc((size_t)file_size);
    if (!capture || fread(capture, 1U, (size_t)file_size, file) !=
            (size_t)file_size) {
        free(capture);
        fclose(file);
        return 0;
    }
    fclose(file);
    memset(&framebuffer, 0, sizeof(framebuffer));
    memset(&frame, 0, sizeof(frame));
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    ok = nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_bitmap(
        &framebuffer, capture, (size_t)file_size, frame_index, &frame,
        &registers, &receipt) && receipt.valid && receipt.capture_only &&
        receipt.nbg1_bitmap_mode && receipt.colour_code_256 &&
        receipt.bitmap_vram_offset == 0U && receipt.transparent_pixels > 0 &&
        !receipt.renderer_permitted;
    free(capture);
    return ok;
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
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
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
    /* The same source-bound bitmap lane must accept the native
     * little-endian register serialization used by later frames. */
    memset(registers, 0, sizeof(registers));
    wl16(registers + 0x00, 0x8020);
    wl16(registers + 0x20, 0x0002);
    wl16(registers + 0x28, 0x1211);
    wl16(registers + 0x2c, 0x0000);
    input.original_saturn_capture_verified = 1;
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE;
    if (!nexus_v1_vdp2_capture_composite_nbg1_bitmap(
            &viewport.fb, &input, &receipt) || !receipt.valid) {
        fprintf(stderr, "FAIL: little-endian VDP2 NBG1 replay\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    memset(registers, 0, sizeof(registers));
    wb16(registers + 0x20, 0x0002);
    wb16(registers + 0x28, 0x1211);
    wb16(registers + 0x2c, 0x0000);
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
    if (getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE") &&
        getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME") &&
        !run_external_bitmap_capture()) {
        fprintf(stderr, "FAIL: external VDP2 NBG1 bitmap capture decode\n");
        return 1;
    }
    puts("test_nexus_v1_vdp2_capture_compositor: PASS");
    return 0;
}
