#include "nexus_v1_viewport.h"
#include "firestaff_x68k_media_receipt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_capture_file(const char *root, const char *name,
                                  size_t expected_size)
{
    char path[1024];
    FILE *file;
    uint8_t *bytes;
    long size;
    if (snprintf(path, sizeof(path), "%s/%s", root, name) >=
        (int)sizeof(path)) return NULL;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) < 0 || (size_t)size != expected_size ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc(expected_size);
    if (!bytes || fread(bytes, 1U, expected_size, file) != expected_size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    return bytes;
}

static int capture_hash_is(const uint8_t *bytes, size_t size,
                           const char *expected)
{
    char actual[65];
    return bytes && firestaff_x68k_media_receipt_sha256_hex(
        bytes, size, actual, sizeof(actual)) == 0 &&
        strcmp(actual, expected) == 0;
}

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8U) | bytes[1]);
}

/* Exercise the generic source-bound compositor against Ymir's real loaded
 * gameplay state. The manifest-pinned VDP2 image has NBG1 enabled in 8bpp
 * mode; its PND char numbers expose a doubled-stride error immediately. */
static int run_authentic_gameplay_8bpp_capture(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_VDP2_REAL_DUMPS");
    const size_t vram_size = NEXUS_V1_SATURN_VDP2_VRAM_BYTES;
    const size_t cram_size = NEXUS_V1_SATURN_VDP2_CRAM_BYTES;
    const size_t regs_size = NEXUS_V1_SATURN_VDP2_REG_BYTES;
    const size_t name_table_base = 0x5c000U;
    const int map_columns = 40;
    const int map_rows = 25;
    uint8_t name_table[40U * 25U * 4U];
    uint8_t *vram;
    uint8_t *cram;
    uint8_t *regs;
    Nexus_V1_Vdp2TilemapCaptureInput real_input;
    Nexus_V1_Vdp2TilemapCaptureReceipt real_receipt;
    Nexus_Viewport real_viewport;
    int ok = 0;

    if (!root || !*root) {
        puts("SKIP: set FIRESTAFF_NEXUS_VDP2_REAL_DUMPS to the authentic "
             "gameplay-mouse-control/dumps directory for the 8bpp check");
        return 1;
    }
    vram = read_capture_file(root, "vdp2-vram.bin", vram_size);
    cram = read_capture_file(root, "vdp2-cram.bin", cram_size);
    regs = read_capture_file(root, "vdp2-registers.bin", regs_size);
    if (!vram || !cram || !regs ||
        !capture_hash_is(vram, vram_size,
            "148eebc95b80465a278b7f6d9c1025ee378ec6d288563de3e52ba8af986d94cc") ||
        !capture_hash_is(cram, cram_size,
            "f97f5ebf7bcfab263514895097ec40e6c5dca6dd91fe878f1691e9f204d8515d") ||
        !capture_hash_is(regs, regs_size,
            "308b3c23bb1db102d3833801a836f0141540fe0870069abb37d2041e57f9b555") ||
        read_be16(regs + 0x20U) != 0x0007U ||
        read_be16(regs + 0x28U) != 0x1013U ||
        read_be16(regs + 0x32U) != 0x0000U ||
        read_be16(regs + 0x3cU) != 0x0000U ||
        read_be16(regs + 0x3eU) != 0x0000U ||
        read_be16(regs + 0x44U) != 0x1717U ||
        read_be16(regs + 0x46U) != 0x1717U ||
        read_be16(regs + 0x80U) != 0U ||
        read_be16(regs + 0x84U) != 0U ||
        name_table_base + 64U * 64U * 4U > vram_size ||
        read_be16(vram + name_table_base + 2U) != 0x1100U ||
        read_be16(vram + name_table_base + (1U * 64U + 2U) * 4U + 2U) !=
            0x1146U) goto cleanup;

    for (int y = 0; y < map_rows; ++y) {
        memcpy(name_table + (size_t)y * map_columns * 4U,
               vram + name_table_base + (size_t)y * 64U * 4U,
               (size_t)map_columns * 4U);
    }
    memset(&real_input, 0, sizeof(real_input));
    real_input.capture_name_table = name_table;
    real_input.capture_name_table_size = sizeof(name_table);
    real_input.capture_character_generator = vram;
    real_input.capture_character_generator_size = (int)vram_size;
    real_input.capture_cram = cram;
    real_input.capture_cram_size = (int)cram_size;
    real_input.vdp2_registers = regs;
    real_input.vdp2_registers_size = (int)regs_size;
    real_input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
    real_input.source_name_table = name_table;
    real_input.source_name_table_size = sizeof(name_table);
    real_input.source_character_generator = vram;
    real_input.source_character_generator_size = (int)vram_size;
    real_input.source_cram = cram;
    real_input.source_cram_size = (int)cram_size;
    real_input.map_columns = map_columns;
    real_input.map_rows = map_rows;
    real_input.source_hash_verified = 1;
    real_input.original_saturn_capture_verified = 1;
    real_input.transparent_index_zero_verified = 1;
    nexus_viewport_init(&real_viewport);
    nexus_fb_clear(&real_viewport.fb);
    if (!nexus_v1_vdp2_capture_composite_nbg1_tilemap(
            &real_viewport.fb, &real_input, &real_receipt) ||
        !real_receipt.valid || !real_receipt.layer_registers_verified ||
        real_receipt.bits_per_pixel != 8 ||
        real_receipt.tiles_decoded != (unsigned int)(map_columns * map_rows) ||
        real_receipt.written_pixels != 148 ||
        real_receipt.transparent_pixels != 63852 ||
        real_viewport.fb.color_buffer[9 * NEXUS_FB_W + 17] != 3U) {
        fprintf(stderr, "authentic NBG1 8bpp composition mismatch: valid=%d "
            "bpp=%d tiles=%d written=%d transparent=%d sample=%u\n",
            real_receipt.valid, real_receipt.bits_per_pixel,
            real_receipt.tiles_decoded, real_receipt.written_pixels,
            real_receipt.transparent_pixels,
            real_viewport.fb.color_buffer[9 * NEXUS_FB_W + 17]);
        goto cleanup;
    }
    ok = 1;
cleanup:
    free(regs);
    free(cram);
    free(vram);
    return ok;
}

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
    if (!run_authentic_gameplay_8bpp_capture()) return 1;
    puts("test_nexus_v1_vdp2_tilemap_capture_compositor: PASS");
    return 0;
}
