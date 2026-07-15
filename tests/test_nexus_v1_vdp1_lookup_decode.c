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
    const unsigned char texture[] = { 0x12U, 0x34U, 0x56U, 0x78U };
    const unsigned short expected[] = {
        0x9001U, 0x9002U, 0x9003U, 0x9004U,
        0x9005U, 0x9006U, 0x9007U, 0x9008U
    };
    unsigned short decoded[8];
    Nexus_V1_Vdp1LookupDecodeReceipt receipt;
    int index;

    memset(command, 0, sizeof(command));
    memset(vram, 0, sizeof(vram));
    /* CMDPMOD mode 1, CMDCOLR table at 0x20, CMDSRCA texture at 0x40,
     * CMDSIZE = 8x1. Sega VDP1 User's Manual 6.3--6.6. */
    wl16(command + 4, 1U << 3);
    wl16(command + 6, 4U);
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
