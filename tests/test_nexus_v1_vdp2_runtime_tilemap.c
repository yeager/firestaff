#include "nexus_v1_vdp2_tilemap_capture_compositor.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void le16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8U);
}

static void be16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)(value >> 8U);
    p[1] = (uint8_t)value;
}

static int run_external_tilemap_capture(void)
{
    const char *path = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE");
    const char *frame_text = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME");
    FILE *file;
    long file_size;
    uint8_t *capture;
    Nexus_Framebuffer framebuffer;
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_SaturnVdp2RegisterReceipt registers;
    Nexus_V1_Vdp2TilemapCaptureReceipt receipt;
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
    if (!nexus_v1_saturn_runtime_capture_frame(
            capture, (size_t)file_size, frame_index, &frame) ||
        !frame.valid) {
        free(capture);
        return 0;
    }
    {
        Nexus_V1_Vdp2TilemapCaptureInput bounds_input;
        memset(&bounds_input, 0, sizeof(bounds_input));
        bounds_input.capture_name_table = frame.vdp2_vram;
        bounds_input.capture_name_table_size = 4;
        bounds_input.capture_character_generator = frame.vdp2_vram;
        bounds_input.capture_character_generator_size = 64;
        bounds_input.capture_cram = frame.vdp2_cram;
        bounds_input.capture_cram_size = (int)frame.vdp2_cram_size;
        bounds_input.vdp2_registers = frame.vdp2_registers;
        bounds_input.vdp2_registers_size = (int)frame.vdp2_register_size;
        bounds_input.register_byte_order = frame.vdp2_word_order;
        bounds_input.source_name_table = frame.vdp2_vram;
        bounds_input.source_name_table_size = 4;
        bounds_input.source_character_generator = frame.vdp2_vram;
        bounds_input.source_character_generator_size = 64;
        bounds_input.source_cram = frame.vdp2_cram;
        bounds_input.source_cram_size = (int)frame.vdp2_cram_size;
        bounds_input.map_columns = 1;
        bounds_input.map_rows = 1;
        bounds_input.destination_x = INT_MAX;
        if (nexus_v1_vdp2_capture_composite_nbg1_tilemap(
                &framebuffer, &bounds_input, &receipt) || receipt.valid ||
            receipt.renderer_permitted) {
            free(capture);
            return 0;
        }
        bounds_input.destination_x = 0;
        bounds_input.destination_y = INT_MAX;
        if (nexus_v1_vdp2_capture_composite_nbg1_tilemap(
                &framebuffer, &bounds_input, &receipt) || receipt.valid ||
            receipt.renderer_permitted) {
            free(capture);
            return 0;
        }
    }
    nexus_fb_init(&framebuffer);
    nexus_fb_clear(&framebuffer);
    if (nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, capture, (size_t)file_size, frame_index,
            INT_MAX, 0, 1, 1, 0, 0, &frame, &registers, &receipt) ||
        nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, capture, (size_t)file_size, frame_index,
            0, INT_MAX, 1, 1, 0, 0, &frame, &registers, &receipt) ||
        nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, capture, (size_t)file_size, frame_index,
            0, 0, 1, 1, INT_MAX, 0, &frame, &registers, &receipt) ||
        nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, capture, (size_t)file_size, frame_index,
            0, 0, 1, 1, 0, INT_MAX, &frame, &registers, &receipt)) {
        free(capture);
        return 0;
    }
    memset(&frame, 0, sizeof(frame));
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    ok = nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
        &framebuffer, capture, (size_t)file_size, frame_index,
        0, 0, 64, 64, 0, 0, &frame, &registers, &receipt) &&
        frame.valid && registers.valid && registers.nbg1_enabled &&
        !registers.nbg1_bitmap_mode && receipt.valid && receipt.capture_only &&
        !receipt.renderer_permitted && receipt.original_saturn_capture_verified &&
        receipt.tiles_decoded == 64;
    if (!ok) {
        fprintf(stderr,
            "NBG1 capture decode rejected: frame=%u frame_valid=%d "
            "registers_valid=%d enabled=%d bitmap=%d bpp=%d "
            "receipt_valid=%d capture_only=%d renderer=%d tiles=%d "
            "transparent=%d written=%d\n",
            frame_index, frame.valid, registers.valid, registers.nbg1_enabled,
            registers.nbg1_bitmap_mode, registers.nbg1_colour_code,
            receipt.valid, receipt.capture_only, receipt.renderer_permitted,
            receipt.tiles_decoded, receipt.transparent_pixels,
            receipt.written_pixels);
    }
    free(capture);
    return ok;
}

int main(void)
{
    const size_t prefix = sizeof(NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC) - 1U;
    const size_t vdp1_marker = sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1) - 1U;
    const size_t vdp2_marker = sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U;
    const size_t blob_size = prefix + sizeof("frame=0\n") - 1U + vdp1_marker +
        NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES + vdp2_marker +
        NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES;
    uint8_t *blob = (uint8_t *)calloc(1U, blob_size);
    uint8_t *legacy_blob = NULL;
    uint8_t source_name[4] = {0, 0, 0, 0};
    uint8_t source_character[64];
    uint8_t source_cram[NEXUS_V1_SATURN_VDP2_CRAM_BYTES];
    Nexus_V1_Vdp2RuntimeTilemapBinding binding;
    Nexus_V1_Vdp2TilemapCaptureReceipt receipt;
    Nexus_V1_SaturnVdp2RegisterReceipt registers;
    Nexus_Framebuffer framebuffer;
    size_t offset = 0U;
    size_t vdp2_marker_offset;
    uint8_t *vdp2_payload;
    uint8_t *vdp2_vram;
    uint8_t *vdp2_regs;
    int i;

    if (!blob) return 1;
    memcpy(blob + offset, NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC, prefix);
    offset += prefix;
    memcpy(blob + offset, "frame=0\n", sizeof("frame=0\n") - 1U);
    offset += sizeof("frame=0\n") - 1U;
    memcpy(blob + offset, NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1, vdp1_marker);
    offset += vdp1_marker + NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES;
    vdp2_marker_offset = offset;
    memcpy(blob + offset, NEXUS_V1_SATURN_VDP2_RAW_MAGIC, vdp2_marker);
    offset += vdp2_marker;
    vdp2_payload = blob + offset;
    vdp2_regs = vdp2_payload;
    vdp2_vram = vdp2_payload + NEXUS_V1_SATURN_VDP2_REG_BYTES;
    le16(vdp2_regs + 0x00, 0x8020U);
    le16(vdp2_regs + 0x20, 0x0002U);
    le16(vdp2_regs + 0x28, 0x1000U);
    le16(vdp2_regs + 0xe4, 0x0000U);
    le16(vdp2_vram + 0, 0U);
    le16(vdp2_vram + 2, 0U);
    for (i = 0; i < 64; ++i) {
        source_character[i] = 0x11U;
        vdp2_vram[0x100 + i] = 0x11U;
    }
    le16(vdp2_vram + 0, 0U);
    memcpy(source_cram,
           vdp2_vram + NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
           sizeof(source_cram));
    memset(&binding, 0, sizeof(binding));
    binding.capture_name_table_offset = 0U;
    binding.capture_character_generator_offset = 0x100U;
    binding.capture_character_generator_size = sizeof(source_character);
    binding.source_name_table = source_name;
    binding.source_name_table_size = sizeof(source_name);
    binding.source_character_generator = source_character;
    binding.source_character_generator_size = sizeof(source_character);
    binding.source_cram = source_cram;
    binding.source_cram_size = sizeof(source_cram);
    binding.map_columns = 1;
    binding.map_rows = 1;
    binding.destination_x = 20;
    binding.destination_y = 30;
    binding.source_hash_verified = 1;
    binding.transparent_index_zero_verified = 1;
    nexus_fb_init(&framebuffer);
    nexus_fb_clear(&framebuffer);
    if (!nexus_v1_vdp2_capture_replay_runtime_frame_nbg1_tilemap(
            &framebuffer, blob, blob_size, 0U, &binding, &receipt) ||
        !receipt.valid || receipt.layer_registers_verified != 1 ||
        receipt.tiles_decoded != 1 || framebuffer.color_buffer[
            30 * NEXUS_FB_W + 20] != 0x11U) {
        fprintf(stderr, "FAIL: raw Saturn VDP2 tilemap replay\n");
        free(blob);
        return 1;
    }
    /* The same frame must also be decodable as a raw capture-only consumer;
     * this path intentionally does not grant source or host-composition
     * admission. */
    /* VDP2 character numbers advance in 32-byte units, including at 8bpp. */
    le16(vdp2_vram + 2, 8U);
    le16(vdp2_regs + 0x3c, 0U);
    le16(vdp2_regs + 0x44, 0U);
    le16(vdp2_regs + 0x46, 0U);
    nexus_fb_clear(&framebuffer);
    if (!nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, blob, blob_size, 0U, 0, 0, 1, 1, 25, 35,
            NULL, NULL, &receipt) || !receipt.valid || !receipt.capture_only ||
        receipt.renderer_permitted || framebuffer.color_buffer[
            35 * NEXUS_FB_W + 25] != 0x11U) {
        fprintf(stderr, "FAIL: raw Saturn VDP2 tilemap capture decode\n");
        free(blob);
        return 1;
    }
    /* NBG1 SCXIN1 (0x80) and SCYIN1 (0x84) offset the name-table lookup.
     * Put the only visible character at map cell (1,1) and require an 8x8
     * source scroll to select it. */
    le16(vdp2_vram + 2, 0U);
    le16(vdp2_vram + (65U * 4U), 0U);
    le16(vdp2_vram + (65U * 4U) + 2U, 8U);
    le16(vdp2_regs + 0x80U, 8U);
    le16(vdp2_regs + 0x84U, 8U);
    nexus_fb_clear(&framebuffer);
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, blob, blob_size, 0U, 0, 0, 1, 1, 27, 37,
            NULL, &registers, &receipt) || !receipt.valid ||
        registers.nbg1_scroll_x != 8U || registers.nbg1_scroll_y != 8U ||
        framebuffer.color_buffer[37 * NEXUS_FB_W + 27] != 0x11U) {
        fprintf(stderr, "FAIL: NBG1 scroll is not applied to tile fetch\n");
        free(blob);
        return 1;
    }
    le16(vdp2_regs + 0x80U, 0U);
    le16(vdp2_regs + 0x84U, 0U);
    /* The upstream Mednafen candidate serializes every Saturn word in
     * big-endian bus order. Exercise the same capture-only decoder with that
     * representation so the local little-endian fixture cannot mask a
     * producer-order regression. */
    be16(vdp2_regs + 0x00, 0x8020U);
    be16(vdp2_regs + 0x20, 0x0002U);
    be16(vdp2_regs + 0x28, 0x1000U);
    be16(vdp2_regs + 0xe4, 0x0000U);
    be16(vdp2_regs + 0x3c, 0x0000U);
    be16(vdp2_regs + 0x44, 0x0000U);
    be16(vdp2_regs + 0x46, 0x0000U);
    be16(vdp2_vram + 0, 0U);
    be16(vdp2_vram + 2, 8U);
    nexus_fb_clear(&framebuffer);
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, blob, blob_size, 0U, 0, 0, 1, 1, 26, 36,
            NULL, &registers, &receipt) || !receipt.valid ||
        registers.byte_order != NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG ||
        !registers.nbg1_enabled || receipt.renderer_permitted ||
        framebuffer.color_buffer[36 * NEXUS_FB_W + 26] != 0x11U) {
        fprintf(stderr, "FAIL: big-endian Mednafen VDP2 tilemap capture decode\n");
        free(blob);
        return 1;
    }
    /* Early V2 captures carried one redundant draw-buffer byte before the
     * VDP2 marker. The transport reader must retain it without shifting VDP1
     * or VDP2 regions. */
    legacy_blob = (uint8_t *)calloc(1U, blob_size + 1U);
    if (!legacy_blob) {
        free(blob);
        return 1;
    }
    memcpy(legacy_blob, blob, vdp2_marker_offset);
    legacy_blob[vdp2_marker_offset] = 1U;
    memcpy(legacy_blob + vdp2_marker_offset + 1U,
           blob + vdp2_marker_offset, blob_size - vdp2_marker_offset);
    {
        Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame_receipt;
        if (!nexus_v1_saturn_runtime_capture_frame(
                legacy_blob, blob_size + 1U, 0U, &frame_receipt) ||
            !frame_receipt.valid || !frame_receipt.vdp1_draw_which ||
            *frame_receipt.vdp1_draw_which != 1U) {
            fprintf(stderr, "FAIL: legacy draw-buffer transport\n");
            free(legacy_blob);
            free(blob);
            return 1;
        }
    }
    free(legacy_blob);
    free(blob);
    if (getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE") &&
        getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME") &&
        !run_external_tilemap_capture()) {
        fprintf(stderr, "FAIL: external VDP2 NBG1 tilemap capture decode\n");
        return 1;
    }
    puts("test_nexus_v1_vdp2_runtime_tilemap: PASS");
    return 0;
}
