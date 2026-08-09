#include "nexus_v1_dungeon.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void wl16(unsigned char *p, unsigned int value) {
    p[0] = (unsigned char)value;
    p[1] = (unsigned char)(value >> 8);
}

int main(void) {
    unsigned char command[NEXUS_V1_VDP1_COMMAND_BYTES];
    static unsigned char vram[NEXUS_V1_VDP1_VRAM_BYTES];
    static unsigned char colour_ram[NEXUS_V1_VDP2_CRAM_BYTES];
    static unsigned char vdp2_registers[NEXUS_V1_VDP2_CAPTURE_REGISTERS_MIN_BYTES];
    const unsigned char texture[] = { 0x12U, 0x34U, 0x56U, 0x78U };
    const unsigned short expected[] = {
        0x9001U, 0x9002U, 0x9003U, 0x9004U,
        0x9005U, 0x9006U, 0x9007U, 0x9008U
    };
    unsigned short decoded[8];
    Nexus_V1_Vdp1LookupDecodeReceipt receipt;
    Nexus_V1_Vdp2ColourRamCapture vdp2_capture;
    Nexus_V1_Vdp1Mode1PaletteResolveReceipt palette_receipt;
    Nexus_V1_Vdp1Mode1PalettePixel resolved[8];
    Nexus_V1_Vdp1Mode1PalettePixel expected_pixels[8];
    int index;

    memset(command, 0, sizeof(command));
    memset(vram, 0, sizeof(vram));
    /* CMDPMOD mode 1, CMDCOLR table at 0x20, CMDSRCA texture at 0x40,
     * CMDSIZE = 8x1. Sega VDP1 User's Manual 6.3--6.6. */
    wl16(command + 4, 1U << 3);
    wl16(command + 6, 8U);
    wl16(command + 8, 8U);
    wl16(command + 10, 0x0101U);
    memcpy(vram + 0x40, texture, sizeof(texture));
    for (index = 0; index < 16; ++index)
        wl16(vram + 0x20 + index * 2, 0x9000U + (unsigned int)index);

    memset(&receipt, 0, sizeof(receipt));
    expect(nexus_v1_vdp1_decode_mode1_lookup_texture(
               command, sizeof(command), vram, sizeof(vram), texture,
               sizeof(texture), decoded, 8U, &receipt) == 1 &&
           receipt.valid && receipt.command_mode1_lookup &&
           receipt.complete_vdp1_vram_snapshot &&
           receipt.texture_lane_matches_vram && receipt.lookup_table_in_vram &&
           receipt.lookup_table_byte_offset == 0x20U &&
           receipt.texture_high_nibble_first && receipt.output_pixel_count == 8U &&
           receipt.output_byte_count == 16 && receipt.no_draw_only &&
           !receipt.pixel_colour_semantics_proven &&
           !receipt.palette_or_cram_semantics_proven,
           "mode-1 capture decodes source-ordered lookup colour codes only");
    expect(nexus_v1_vdp1_lookup_colour_codes_match(
               decoded, 8U, expected, 8U),
           "high then low texture nibbles select exact VDP1 lookup words");

    /* A complete captured VDP2 CRAM/register state is mandatory for palette
     * resolution. Texture 0 is transparent, texture F ends this scanline,
     * index 1 uses direct RGB555, and index 2 uses the captured SPCAOS CRAM
     * address. This remains an output witness test, never a host draw. */
    memset(command, 0, sizeof(command));
    memset(vram, 0, sizeof(vram));
    memset(colour_ram, 0, sizeof(colour_ram));
    memset(vdp2_registers, 0, sizeof(vdp2_registers));
    wl16(command + 4, 1U << 3); /* mode 1; SPD/ECD both enabled (zero). */
    wl16(command + 6, 8U);
    wl16(command + 8, 8U);
    wl16(command + 10, 0x0101U);
    vram[0x40] = 0x01U;
    vram[0x41] = 0x2fU;
    vram[0x42] = 0x34U;
    vram[0x43] = 0x50U;
    wl16(vram + 0x20, 0x8fffU);
    wl16(vram + 0x22, 0x8c41U); /* RGB: R=1, G=2, B=3. */
    wl16(vram + 0x24, 0x0123U);
    wl16(vram + 0x3e, 0x8fffU);
    wl16(vdp2_registers + NEXUS_V1_VDP2_RAMCTL_OFFSET, 1U << 12);
    wl16(vdp2_registers + NEXUS_V1_VDP2_CRAOFB_OFFSET, 1U << 4);
    /* Mode-1 CRAM: dot 0x123 plus SPCAOS 1 -> entry 0x223. */
    wl16(colour_ram + 0x223U * 2U, 0x1ca5U);
    memset(&vdp2_capture, 0, sizeof(vdp2_capture));
    vdp2_capture.colour_ram = colour_ram;
    vdp2_capture.colour_ram_size = sizeof(colour_ram);
    vdp2_capture.registers = vdp2_registers;
    vdp2_capture.registers_size = sizeof(vdp2_registers);
    vdp2_capture.original_saturn_capture_verified = 1;
    memset(&palette_receipt, 0, sizeof(palette_receipt));
    expect(nexus_v1_vdp1_resolve_mode1_palette_capture(
               command, sizeof(command), vram, sizeof(vram), vram + 0x40,
               4, &vdp2_capture, resolved, 8U, &palette_receipt) == 1 &&
           palette_receipt.valid &&
           palette_receipt.original_saturn_capture_verified &&
           palette_receipt.mode1_lookup_bound &&
           palette_receipt.vdp2_cram_bound &&
           palette_receipt.vdp2_registers_bound &&
           palette_receipt.vdp2_cram_mode == 1U &&
           palette_receipt.vdp2_sprite_colour_ram_offset == 1U &&
           palette_receipt.source_index_zero_transparent &&
           palette_receipt.source_index_f_end_code &&
           palette_receipt.direct_rgb555_proven &&
           palette_receipt.vdp2_cram_address_proven &&
           palette_receipt.vdp2_cram_rgb_proven &&
           palette_receipt.output_pixel_count == 8U &&
           palette_receipt.no_draw_only && !palette_receipt.renderer_permitted,
           "captured VDP2 CRAM/register state resolves mode-1 palette chain only");
    memset(expected_pixels, 0, sizeof(expected_pixels));
    expected_pixels[0].kind = NEXUS_V1_VDP1_MODE1_PIXEL_TRANSPARENT;
    expected_pixels[0].raw_colour_code = 0x8fffU;
    expected_pixels[1].kind = NEXUS_V1_VDP1_MODE1_PIXEL_RGB555;
    expected_pixels[1].texture_index = 1U;
    expected_pixels[1].raw_colour_code = 0x8c41U;
    expected_pixels[1].red = 8U;
    expected_pixels[1].green = 16U;
    expected_pixels[1].blue = 24U;
    expected_pixels[2].kind = NEXUS_V1_VDP1_MODE1_PIXEL_VDP2_CRAM_RGB555;
    expected_pixels[2].texture_index = 2U;
    expected_pixels[2].raw_colour_code = 0x0123U;
    expected_pixels[2].colour_ram_address = 0x223U;
    expected_pixels[2].red = 40U;
    expected_pixels[2].green = 40U;
    expected_pixels[2].blue = 56U;
    expected_pixels[3].kind = NEXUS_V1_VDP1_MODE1_PIXEL_END_CODE;
    expected_pixels[3].texture_index = 15U;
    expected_pixels[3].raw_colour_code = 0x8fffU;
    for (index = 4; index < 8; ++index) {
        expected_pixels[index].kind =
            NEXUS_V1_VDP1_MODE1_PIXEL_SUPPRESSED_AFTER_END;
        expected_pixels[index].texture_index = index == 4 ? 3U :
            (index == 5 ? 4U : (index == 6 ? 5U : 0U));
        expected_pixels[index].raw_colour_code = index == 4 ? 0U :
            (index == 5 ? 0U : (index == 6 ? 0U : 0x8fffU));
    }
    expect(nexus_v1_vdp1_mode1_palette_pixels_match(
               resolved, 8U, expected_pixels, 8U),
           "mode-1 transparent/RGB/CRAM/end-code pixels match captured palette semantics");
    vdp2_capture.original_saturn_capture_verified = 0;
    expect(nexus_v1_vdp1_resolve_mode1_palette_capture(
               command, sizeof(command), vram, sizeof(vram), vram + 0x40,
               4, &vdp2_capture, resolved, 8U, &palette_receipt) == 0 &&
           !palette_receipt.valid && palette_receipt.no_draw_only,
           "unattested VDP2 palette capture is blocked");

    vram[0x40] ^= 0x10U;
    memset(&receipt, 0, sizeof(receipt));
    expect(nexus_v1_vdp1_decode_mode1_lookup_texture(
               command, sizeof(command), vram, sizeof(vram), texture,
               sizeof(texture), decoded, 8U, &receipt) == 0 &&
           !receipt.valid && !receipt.texture_lane_matches_vram &&
           receipt.no_draw_only,
           "changed VDP1 texture bytes block the lookup decoder");
    wl16(command + 4, 0U);
    memcpy(vram + 0x40, texture, sizeof(texture));
    memset(&receipt, 0, sizeof(receipt));
    expect(nexus_v1_vdp1_decode_mode1_lookup_texture(
               command, sizeof(command), vram, sizeof(vram), texture,
               sizeof(texture), decoded, 8U, &receipt) == 0 &&
           !receipt.valid && receipt.no_draw_only,
           "non-lookup VDP1 commands cannot enter the mode-1 decoder");

    if (failures) return 1;
    puts("test_nexus_v1_vdp1_lookup_decode: PASS");
    return 0;
}
