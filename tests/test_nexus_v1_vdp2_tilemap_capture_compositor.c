#include "nexus_v1_viewport.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    static uint8_t name_table[4] = {0x00, 0x00, 0x00, 0x00};
    static uint8_t cg[32] = {0};
    static uint8_t cram[4096] = {0};
    static uint8_t regs[0xe8] = {0};
    Nexus_V1_Vdp2TilemapCaptureInput input;
    Nexus_V1_Vdp2TilemapCaptureReceipt receipt;
    Nexus_Viewport viewport;

    /* NBG1, tilemap, 4bpp, 8x8 chars, two-word name entries, CRAOFA=0. */
    regs[0x20] = 0x00; regs[0x21] = 0x02;
    regs[0x28] = 0x00; regs[0x29] = 0x00;
    regs[0x32] = 0x00; regs[0x33] = 0x00;
    regs[0xe4] = 0x00; regs[0xe5] = 0x00;
    cg[0] = 0x11;
    cg[1] = 0x11;
    memset(&input, 0, sizeof(input));
    input.capture_name_table = name_table;
    input.capture_name_table_size = sizeof(name_table);
    input.capture_character_generator = cg;
    input.capture_character_generator_size = sizeof(cg);
    input.capture_cram = cram;
    input.capture_cram_size = sizeof(cram);
    input.vdp2_registers = regs;
    input.vdp2_registers_size = sizeof(regs);
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
    input.source_name_table = name_table;
    input.source_name_table_size = sizeof(name_table);
    input.source_character_generator = cg;
    input.source_character_generator_size = sizeof(cg);
    input.source_cram = cram;
    input.source_cram_size = sizeof(cram);
    input.map_columns = 1;
    input.map_rows = 1;
    input.source_hash_verified = 1;
    input.original_saturn_capture_verified = 1;
    input.transparent_index_zero_verified = 1;
    nexus_viewport_init(&viewport);
    if (!nexus_viewport_replay_vdp2_nbg1_tilemap_capture(
            &viewport, &input, &receipt) || !receipt.valid ||
        receipt.tiles_decoded != 1 || receipt.bits_per_pixel != 4 ||
        receipt.written_pixels <= 0) {
        fprintf(stderr, "tilemap replay did not admit authenticated fixture\n");
        return 1;
    }
    input.original_saturn_capture_verified = 0;
    if (nexus_viewport_replay_vdp2_nbg1_tilemap_capture(
            &viewport, &input, &receipt) != 0) {
        fprintf(stderr, "unauthenticated tilemap replay admitted\n");
        return 1;
    }
    /* Native little-endian register serialization is also present in the
     * authenticated Mednafen witness; the source spans stay byte-identical. */
    regs[0x00] = 0x80; regs[0x01] = 0x00;
    regs[0x20] = 0x02; regs[0x21] = 0x00;
    input.original_saturn_capture_verified = 1;
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE;
    if (!nexus_viewport_replay_vdp2_nbg1_tilemap_capture(
            &viewport, &input, &receipt) || !receipt.valid) {
        fprintf(stderr, "little-endian tilemap register replay rejected\n");
        return 1;
    }
    /* A raw little-endian capture must retain the same word order for its
     * name-table and CRAM bytes.  Reading this 0x001f BGR555 word as
     * big-endian turns a source-red palette entry into a blue/green artefact.
     */
    memset(regs, 0, sizeof(regs));
    regs[0x00] = 0x20; regs[0x01] = 0x80;
    regs[0x20] = 0x02; regs[0x21] = 0x00;
    regs[0x28] = 0x00; regs[0x29] = 0x00;
    regs[0x32] = 0x00; regs[0x33] = 0x00;
    regs[0xe4] = 0x00; regs[0xe5] = 0x00;
    cram[2] = 0x1f; cram[3] = 0x00;
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE;
    if (!nexus_viewport_replay_vdp2_nbg1_tilemap_capture(
            &viewport, &input, &receipt) || !receipt.valid ||
        viewport.fb.palette[1] != UINT32_C(0xffff0000)) {
        fprintf(stderr, "little-endian tilemap CRAM palette order changed\n");
        return 1;
    }
    /* Authentic post-render frames can have TVMD=0 while BGON remains
     * populated.  Register order must then be selected from the VDP2 layer
     * fields, not from TVMD alone. */
    memset(regs, 0, sizeof(regs));
    regs[0x20] = 0x02; regs[0x21] = 0x00;
    input.original_saturn_capture_verified = 1;
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE;
    if (!nexus_viewport_replay_vdp2_nbg1_tilemap_capture(
            &viewport, &input, &receipt) || !receipt.valid) {
        fprintf(stderr, "TVMD-zero little-endian tilemap replay rejected\n");
        return 1;
    }
    /* The authenticated VDP2 envelope also has a big-endian legacy witness.
     * TVMD=0x0080 must not make BGON/CHCTLA read as little-endian. */
    memset(regs, 0, sizeof(regs));
    regs[0x00] = 0x00; regs[0x01] = 0x80;
    regs[0x20] = 0x00; regs[0x21] = 0x02;
    input.original_saturn_capture_verified = 1;
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
    if (!nexus_viewport_replay_vdp2_nbg1_tilemap_capture(
            &viewport, &input, &receipt) || !receipt.valid ||
        receipt.bits_per_pixel != 4) {
        fprintf(stderr, "big-endian tilemap register replay rejected\n");
        return 1;
    }
    puts("test_nexus_v1_vdp2_tilemap_capture_compositor: PASS");
    return 0;
}
