#include "nexus_v1_vdp2_tilemap_capture_compositor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void le16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8U);
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
    uint8_t source_name[4] = {0, 0, 0, 0};
    uint8_t source_character[64];
    uint8_t source_cram[NEXUS_V1_SATURN_VDP2_CRAM_BYTES];
    Nexus_V1_Vdp2RuntimeTilemapBinding binding;
    Nexus_V1_Vdp2TilemapCaptureReceipt receipt;
    Nexus_Framebuffer framebuffer;
    size_t offset = 0U;
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
    le16(vdp2_vram + 2, 4U);
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
    free(blob);
    puts("test_nexus_v1_vdp2_runtime_tilemap: PASS");
    return 0;
}
