#include "nexus_v1_dungeon.h"
#include "nexus_v1_vdp1_capture_compositor.h"

#include <stdio.h>
#include <stdlib.h>
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
    static unsigned char vdp2_registers[0x200U];
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
    {
        uint8_t indices[8];
        wl16(command + 6, 0x52f3U);
        wl16(command + 8, 8U);
        wl16(command + 10, 0x0101U);
        memcpy(vram + 0x40, texture, sizeof(texture));
        expect(nexus_v1_vdp1_decode_mode0_bank_texture(
                   command, sizeof(command), vram, sizeof(vram), indices, decoded, 8U) &&
               indices[0] == 1U && indices[7] == 8U &&
               decoded[0] == 0x52f1U && decoded[7] == 0x52f8U,
               "VDP1 colour-bank decode preserves bank and nibble order");
        expect(!nexus_v1_vdp1_decode_mode0_bank_texture(
                   command, sizeof(command), vram, sizeof(vram), indices, decoded, 7U),
               "VDP1 colour-bank decode rejects short output");
        memset(command, 0, sizeof(command));
    }
    {
        const char *path = getenv("FIRESTAFF_NEXUS_YMIR_DUMP");
        const char *trace = getenv("FIRESTAFF_NEXUS_VDP1_TRACE");
        const char *cram_path = getenv("FIRESTAFF_NEXUS_YMIR_CRAM");
        const char *regs_path = getenv("FIRESTAFF_NEXUS_YMIR_REGS");
        const char *fb_path = getenv("FIRESTAFF_NEXUS_YMIR_VDP1_FBS");
        if (path && trace) {
            FILE *vram_file = fopen(path, "rb"), *trace_file = fopen(trace, "rb");
            uint8_t record[36], indices[576];
            uint16_t codes[576];
            int found = 0;
            expect(vram_file && trace_file, "open authentic Ymir inputs");
            if (vram_file && trace_file && fread(vram, 1, sizeof(vram), vram_file) == sizeof(vram)) {
                while (fread(record, 1, sizeof(record), trace_file) == sizeof(record)) {
                    if (record[0] == 0xe0U && record[1] == 0xd8U &&
                        record[2] == 0U && record[3] == 0U) {
                        size_t n;
                        found = nexus_v1_vdp1_decode_mode0_bank_texture(
                            record + 4, 32, vram, sizeof(vram), indices, codes, 576);
                        for (n = 0; found && n < 576; ++n) {
                            uint8_t packed = vram[0x44060U + n / 2];
                            uint8_t expected_index = n & 1U ? packed & 15U : packed >> 4U;
                            if (indices[n] != expected_index || codes[n] != (0x52f0U | expected_index)) found = 0;
                        }
                        if (found && cram_path && regs_path) {
                            FILE *cram_file = fopen(cram_path, "rb"), *regs_file = fopen(regs_path, "rb");
                            static Nexus_V1_Vdp1Mode1PalettePixel real_pixels[576];
                            Nexus_V1_Vdp2ColourRamCapture real_capture;
                            size_t j;
                            if (!cram_file || !regs_file ||
                                fread(colour_ram, 1, sizeof(colour_ram), cram_file) != sizeof(colour_ram) ||
                                fread(vdp2_registers, 1, sizeof(vdp2_registers), regs_file) != sizeof(vdp2_registers)) {
                                found = 0;
                            } else {
                                /* Ymir emits big-endian register/CRAM words. */
                                for (j = 0; j < sizeof(colour_ram); j += 2) {
                                    uint8_t temp = colour_ram[j]; colour_ram[j] = colour_ram[j + 1]; colour_ram[j + 1] = temp;
                                }
                                for (j = 0; j < sizeof(vdp2_registers); j += 2) {
                                    uint8_t temp = vdp2_registers[j]; vdp2_registers[j] = vdp2_registers[j + 1]; vdp2_registers[j + 1] = temp;
                                }
                                memset(&real_capture, 0, sizeof(real_capture));
                                real_capture.colour_ram = colour_ram;
                                real_capture.colour_ram_size = sizeof(colour_ram);
                                real_capture.registers = vdp2_registers;
                                real_capture.registers_size = sizeof(vdp2_registers);
                                real_capture.original_saturn_capture_verified = 1;
                                found = nexus_v1_vdp1_resolve_mode0_type6_capture(record + 4, 32, vram,
                                    sizeof(vram), &real_capture, real_pixels, 576);
                                for (j = 0; found && j < 576; ++j) {
                                    uint32_t address = 0x2f0U | indices[j];
                                    uint16_t color = (uint16_t)colour_ram[address * 2U] |
                                        ((uint16_t)colour_ram[address * 2U + 1U] << 8U);
                                    if (indices[j] &&
                                        (real_pixels[j].colour_ram_address != address ||
                                         real_pixels[j].red != (uint8_t)((color & 31U) << 3U) ||
                                         real_pixels[j].green != (uint8_t)(((color >> 5U) & 31U) << 3U) ||
                                         real_pixels[j].blue != (uint8_t)(((color >> 10U) & 31U) << 3U))) found = 0;
                                }
                                if (found) {
                                    static Nexus_V1_Vdp1DirectColorFramebuffer preview;
                                    int written = 0;
                                    found = nexus_v1_vdp1_capture_preview_mode0_normal(
                                        &preview, record + 4, 32, vram, sizeof(vram), &real_capture,
                                        0, 0, 319, 223, &written) && written > 0;
                                    for (j = 0; found && j < 576; ++j) {
                                        unsigned int x = 104U + (unsigned int)(j % 24U);
                                        unsigned int y = 144U + (unsigned int)(j / 24U);
                                        uint32_t expected_pixel = indices[j] ? UINT32_C(0xff000000) |
                                            ((uint32_t)real_pixels[j].red << 16U) |
                                            ((uint32_t)real_pixels[j].green << 8U) | real_pixels[j].blue : 0U;
                                        if (preview.rgba_buffer[y * NEXUS_FB_W + x] != expected_pixel) found = 0;
                                    }
                                    memset(&preview, 0, sizeof(preview));
                                    if (found) found = nexus_v1_vdp1_capture_blend_mode0_normal(
                                        &preview, record + 4, 32, vram, sizeof(vram), &real_capture,
                                        0, 0, 319, 223, &written);
                                    for (j = 0; found && j < 576; ++j) {
                                        unsigned int x = 104U + (unsigned int)(j % 24U);
                                        unsigned int y = 144U + (unsigned int)(j / 24U);
                                        uint32_t expected_pixel = indices[j] ? UINT32_C(0xff000000) |
                                            ((uint32_t)(real_pixels[j].red * 31U / 32U) << 16U) |
                                            ((uint32_t)(real_pixels[j].green * 31U / 32U) << 8U) |
                                            (real_pixels[j].blue * 31U / 32U) : 0U;
                                        if (preview.rgba_buffer[y * NEXUS_FB_W + x] != expected_pixel) found = 0;
                                    }
                                    if (found) {
                                        uint32_t expected;
                                        const Nexus_V1_Vdp1Mode1PalettePixel *p = &real_pixels[103];
                                        for (j = 0; j < NEXUS_FB_W * NEXUS_FB_H; ++j)
                                            preview.rgba_buffer[j] = UINT32_C(0xff204060);
                                        found = nexus_v1_vdp1_capture_blend_mode0_normal(
                                            &preview, record + 4, 32, vram, sizeof(vram), &real_capture,
                                            0, 0, 319, 223, &written);
                                        expected = UINT32_C(0xff000000) |
                                            (((p->red * 31U + 32U) >> 5U) << 16U) |
                                            (((p->green * 31U + 64U) >> 5U) << 8U) |
                                            ((p->blue * 31U + 96U) >> 5U);
                                        if (!p->texture_index || preview.rgba_buffer[148 * NEXUS_FB_W + 111] != expected)
                                            found = 0;
                                    }
                                }
                            }
                            if (cram_file) fclose(cram_file);
                            if (regs_file) fclose(regs_file);
                        }
                        break;
                    }
                }
            }
            expect(found, "decode authentic Nexus TITLE.BIN colour-bank sprite");
            if (vram_file && trace_file && fb_path) {
                FILE *fb_file = fopen(fb_path, "rb");
                static uint8_t captured_fbs[512U * 1024U];
                uint8_t indices[832];
                uint16_t codes[832];
                int command_count = 0, pixel_count = 0, ok = 1;
                if (!fb_file || fread(captured_fbs, 1, sizeof(captured_fbs), fb_file) != sizeof(captured_fbs)) ok = 0;
                if (fb_file) fclose(fb_file);
                rewind(trace_file);
                while (ok && fread(record, 1, sizeof(record), trace_file) == sizeof(record)) {
                    uint32_t address = (uint32_t)record[0] | ((uint32_t)record[1] << 8U) |
                        ((uint32_t)record[2] << 16U) | ((uint32_t)record[3] << 24U);
                    Nexus_V1_Vdp1TextureCommand parsed;
                    static Nexus_V1_Vdp1DirectColorFramebuffer preview;
                    Nexus_V1_Vdp2ColourRamCapture preview_capture;
                    int preview_written = 0;
                    uint32_t x, y;
                    if (address != 0xd8e0U && address != 0xd900U && address != 0xd920U &&
                        address != 0xd940U && address != 0xd960U) continue;
                    if (nexus_v1_vdp1_texture_command_parse(record + 4, 32, &parsed) != 0 ||
                        (uint32_t)parsed.texture_width * parsed.texture_height > 832U ||
                        !nexus_v1_vdp1_decode_mode0_bank_texture(record + 4, 32, vram,
                            sizeof(vram), indices, codes, 832)) { ok = 0; break; }
                    if (cram_path && regs_path) {
                        memset(&preview, 0, sizeof(preview));
                        memset(&preview_capture, 0, sizeof(preview_capture));
                        preview_capture.colour_ram = colour_ram;
                        preview_capture.colour_ram_size = sizeof(colour_ram);
                        preview_capture.registers = vdp2_registers;
                        preview_capture.registers_size = sizeof(vdp2_registers);
                        preview_capture.original_saturn_capture_verified = 1;
                        if (!nexus_v1_vdp1_capture_preview_mode0_normal(&preview, record + 4,
                                32, vram, sizeof(vram), &preview_capture, 0, 0, 319, 223,
                                &preview_written) || !preview_written) { ok = 0; break; }
                    }
                    ++command_count;
                    for (y = 0; ok && y < parsed.texture_height; ++y) {
                        for (x = 0; x < parsed.texture_width; ++x) {
                            uint32_t u = (parsed.control & 0x10U) ? parsed.texture_width - 1U - x : x;
                            uint32_t v = (parsed.control & 0x20U) ? parsed.texture_height - 1U - y : y;
                            uint32_t source = v * parsed.texture_width + u;
                            uint32_t dest = ((uint32_t)parsed.ya + y) * 512U + (uint32_t)parsed.xa + x;
                            int plane;
                            if (!indices[source]) continue;
                            if (dest >= 131072U) { ok = 0; break; }
                            for (plane = 0; plane < 2; ++plane) {
                                size_t pos = (size_t)plane * 262144U + dest * 2U;
                                uint16_t value = ((uint16_t)captured_fbs[pos] << 8U) | captured_fbs[pos + 1U];
                                if (value != codes[source]) ok = 0;
                            }
                            if (cram_path && regs_path) {
                                uint32_t cram_index = codes[source] & 0x3ffU;
                                uint16_t color = (uint16_t)colour_ram[cram_index * 2U] |
                                    ((uint16_t)colour_ram[cram_index * 2U + 1U] << 8U);
                                uint32_t expected = UINT32_C(0xff000000) |
                                    ((uint32_t)(color & 31U) << 19U) |
                                    ((uint32_t)((color >> 5U) & 31U) << 11U) |
                                    ((uint32_t)((color >> 10U) & 31U) << 3U);
                                uint32_t output = ((uint32_t)parsed.ya + y) * NEXUS_FB_W +
                                    (uint32_t)parsed.xa + x;
                                if (output >= NEXUS_FB_W * NEXUS_FB_H ||
                                    preview.rgba_buffer[output] != expected) ok = 0;
                            }
                            ++pixel_count;
                        }
                    }
                }
                expect(ok && command_count == 5 && pixel_count == 1495,
                       "five authentic title sprites match both captured VDP1 framebuffers");
            }
            if (vram_file) fclose(vram_file);
            if (trace_file) fclose(trace_file);
            memset(vram, 0, sizeof(vram));
            memset(colour_ram, 0, sizeof(colour_ram));
            memset(vdp2_registers, 0, sizeof(vdp2_registers));
        }
    }
    {
        Nexus_V1_Vdp1TextureCommand parsed;
        wl16(command + 10, 0x0410U);
        expect(nexus_v1_vdp1_texture_command_parse(command, sizeof(command), &parsed) == 0 &&
               parsed.texture_width == 32U && parsed.texture_height == 16U,
               "VDP1 CMDSIZE uses high-byte width and low-byte height");
        wl16(command, 0x5000U);
        expect(nexus_v1_vdp1_texture_command_parse(command, sizeof(command), &parsed) == 0 &&
               !parsed.end_command && !parsed.texture_command &&
               !parsed.texture_source_range_valid,
               "VDP1 skipped jump does not decode a texture");
        wl16(command, 0U);
    }
    {
        Nexus_V1_Vdp1TextureCommand parsed;
        /* Decoder-only command fixture for Ymir's mode-5 address alignment;
         * it is not game data or a substitute for an authentic capture. */
        memset(command, 0, sizeof(command));
        wl16(command, 2U);
        wl16(command + 4, 5U << 3);
        wl16(command + 8, 1U);
        wl16(command + 10, 0x0101U);
        expect(nexus_v1_vdp1_texture_command_parse(
                   command, sizeof(command), &parsed) == 0 &&
               parsed.texture_byte_count == 16U &&
               parsed.texture_source_byte_offset == 0U &&
               parsed.texture_source_byte_end == 16U &&
               parsed.texture_source_range_valid,
               "16-bpp VDP1 fetch aligns odd eight-byte CMDSRCA down to 16 bytes");
        wl16(command + 8, 0xffffU);
        expect(nexus_v1_vdp1_texture_command_parse(
                   command, sizeof(command), &parsed) == 0 &&
               parsed.texture_source_byte_offset == 0x7fff0U &&
               parsed.texture_source_byte_end == NEXUS_V1_VDP1_VRAM_BYTES &&
               parsed.texture_source_range_valid,
               "aligned 16-bpp VDP1 fetch ending at VRAM boundary remains in range");
        memset(command, 0, sizeof(command));
    }
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

    /* CMDX/CMDY are signed 13-bit fields, not signed 16-bit host words.
     * The high three bits are ignored by the VDP1 coordinate decoder. */
    wl16(command + 12, 0xf000U | 0x1f82U);
    wl16(command + 14, 0xe000U | 0x0070U);
    wl16(command + 16, 0xffffU);
    wl16(command + 18, 0x1000U);
    {
        Nexus_V1_Vdp1TextureCommand parsed;
        memset(&parsed, 0, sizeof(parsed));
        expect(nexus_v1_vdp1_texture_command_parse(
                   command, sizeof(command), &parsed) == 0 &&
               parsed.xa == -126 && parsed.ya == 112 &&
               parsed.xb == -1 && parsed.yb == -4096,
               "VDP1 command coordinates use signed 13-bit fields");
    }

    /* The live Nexus command uses COLR=0x3278. Mednafen reports the LUT in
     * VDP1 words at 0xc9e0; this byte-buffer API must therefore inspect
     * 0x193c0, not 0xc9e0. */
    memset(command, 0, sizeof(command));
    memset(vram, 0, sizeof(vram));
    wl16(command + 4, 1U << 3);
    wl16(command + 6, 0x3278U);
    wl16(command + 8, 0x0400U);
    wl16(command + 10, 0x0101U);
    memcpy(vram + 0x2000, texture, sizeof(texture));
    for (index = 0; index < 16; ++index)
        wl16(vram + 0x193c0 + index * 2, 0xa000U + (unsigned int)index);
    memset(&receipt, 0, sizeof(receipt));
    expect(nexus_v1_vdp1_decode_mode1_lookup_texture(
               command, sizeof(command), vram, sizeof(vram), vram + 0x2000,
               sizeof(texture), decoded, 8U, &receipt) == 1 &&
           receipt.valid && receipt.lookup_table_byte_offset == 0x193c0U &&
           decoded[0] == 0xa001U && decoded[7] == 0xa008U,
           "authentic Nexus COLR word address converts to VDP1 byte offset");

    /* A complete captured VDP2 CRAM/register state is mandatory for palette
     * resolution. Texture 0 is transparent and two F texels end a scanline;
     * index 1 uses direct RGB555, and index 2 uses the captured SPCAOS CRAM
     * address. This remains an output witness test, never a host draw. */
    memset(command, 0, sizeof(command));
    memset(vram, 0, sizeof(vram));
    memset(colour_ram, 0, sizeof(colour_ram));
    memset(vdp2_registers, 0, sizeof(vdp2_registers));
    wl16(command + 4, 1U << 3); /* mode 1; SPD/ECD both enabled (zero). */
    wl16(command + 6, 4U);
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
        expected_pixels[index].kind = index == 7 ? NEXUS_V1_VDP1_MODE1_PIXEL_TRANSPARENT :
            NEXUS_V1_VDP1_MODE1_PIXEL_VDP2_CRAM_RGB555;
        expected_pixels[index].texture_index = index == 4 ? 3U :
            (index == 5 ? 4U : (index == 6 ? 5U : 0U));
        expected_pixels[index].raw_colour_code = index == 4 ? 0U :
            (index == 5 ? 0U : (index == 6 ? 0U : 0x8fffU));
        if (index != 7) expected_pixels[index].colour_ram_address = 0x100U;
    }
    expect(nexus_v1_vdp1_mode1_palette_pixels_match(
               resolved, 8U, expected_pixels, 8U),
           "mode-1 transparent/RGB/CRAM/end-code pixels match captured palette semantics");
    vram[0x42] = 0x3fU;
    expect(nexus_v1_vdp1_resolve_mode1_palette_capture(
               command, sizeof(command), vram, sizeof(vram), vram + 0x40,
               4, &vdp2_capture, resolved, 8U, &palette_receipt) &&
           resolved[3].kind == NEXUS_V1_VDP1_MODE1_PIXEL_END_CODE &&
           resolved[4].kind == NEXUS_V1_VDP1_MODE1_PIXEL_VDP2_CRAM_RGB555 &&
           resolved[5].kind == NEXUS_V1_VDP1_MODE1_PIXEL_END_CODE &&
           resolved[6].kind == NEXUS_V1_VDP1_MODE1_PIXEL_SUPPRESSED_AFTER_END,
           "second VDP1 end code terminates the texture row");
    vram[0x42] = 0x34U;
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
