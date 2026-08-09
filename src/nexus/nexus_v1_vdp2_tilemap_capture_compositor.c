#include "nexus_v1_vdp2_tilemap_capture_compositor.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8U) | p[1]);
}

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[1] << 8U) | p[0]);
}

static uint16_t read_register16(const uint8_t *registers, size_t offset)
{
    uint16_t tvmd = read_be16(registers + 0x00U);

    /* The external producer has both historical big-endian and native
     * little-endian serializations. TVMD is the stable 0x0080 witness;
     * synthetic fixtures with a zero TVMD retain the original big-endian
     * contract. */
    if (tvmd != 0x0080U && read_le16(registers) == 0x0080U)
        return read_le16(registers + offset);
    return read_be16(registers + offset);
}

static uint8_t expand5(uint16_t value, unsigned shift)
{
    uint8_t result = (uint8_t)(((value >> shift) & 0x1fU) << 3U);
    return (uint8_t)(result | (result >> 5U));
}

static uint32_t cram_to_rgba(const uint8_t *entry)
{
    uint16_t value = read_be16(entry);
    return UINT32_C(0xff000000) |
        ((uint32_t)expand5(value, 0U) << 16U) |
        ((uint32_t)expand5(value, 5U) << 8U) |
        (uint32_t)expand5(value, 10U);
}

int nexus_v1_vdp2_capture_composite_nbg1_tilemap(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp2TilemapCaptureInput *input,
    Nexus_V1_Vdp2TilemapCaptureReceipt *out_receipt)
{
    Nexus_V1_Vdp2TilemapCaptureReceipt receipt;
    uint16_t bgon;
    uint16_t chctla;
    uint16_t pncn1;
    uint16_t craofa;
    int bpp;
    int map_bytes;
    int x;
    int y;

    memset(&receipt, 0, sizeof(receipt));
    if (!out_receipt) return 0;
    if (!framebuffer || !input || !input->capture_name_table ||
        !input->capture_character_generator || !input->capture_cram ||
        !input->vdp2_registers || !input->source_name_table ||
        !input->source_character_generator || !input->source_cram ||
        input->map_columns <= 0 || input->map_rows <= 0 ||
        input->map_columns > 1024 || input->map_rows > 1024 ||
        input->source_tile_x != 0 || input->source_tile_y != 0 ||
        input->destination_x < 0 || input->destination_y < 0 ||
        input->destination_x + input->map_columns * 8 > NEXUS_FB_W ||
        input->destination_y + input->map_rows * 8 > NEXUS_FB_H ||
        input->vdp2_registers_size < (int)NEXUS_V1_VDP2_TILEMAP_REGISTERS_BYTES ||
        input->capture_cram_size != 4096 || input->source_cram_size != 4096 ||
        input->capture_name_table_size != input->map_columns * input->map_rows * 4 ||
        input->source_name_table_size != input->map_columns * input->map_rows * 4 ||
        input->capture_character_generator_size <= 0 ||
        input->source_character_generator_size !=
            input->capture_character_generator_size ||
        !input->source_hash_verified || !input->original_saturn_capture_verified ||
        !input->transparent_index_zero_verified) {
        *out_receipt = receipt;
        return 0;
    }

    map_bytes = input->map_columns * input->map_rows * 4;
    bgon = read_register16(input->vdp2_registers,
                           NEXUS_V1_VDP2_TILEMAP_BGON_OFFSET);
    chctla = read_register16(input->vdp2_registers,
                             NEXUS_V1_VDP2_TILEMAP_CHCTLA_OFFSET);
    pncn1 = read_register16(input->vdp2_registers,
                            NEXUS_V1_VDP2_TILEMAP_PNCN1_OFFSET);
    craofa = read_register16(input->vdp2_registers,
                             NEXUS_V1_VDP2_TILEMAP_CRAOFA_OFFSET);
    /* CHCTLA bit 9 is NBG1 bitmap enable; clear means tilemap. Bit 8 is
     * 8x8/16x16 character size; only 8x8 is admitted by this bounded lane. */
    bpp = ((chctla >> 12U) & 3U) == 0U ? 4 :
        (((chctla >> 12U) & 3U) == 1U ? 8 : 0);
    if ((bgon & 0x0002U) == 0U || (chctla & 0x0200U) != 0U ||
        (chctla & 0x0100U) != 0U || bpp == 0 ||
        (pncn1 & 0x8000U) != 0U || ((craofa >> 4U) & 7U) != 0U ||
        memcmp(input->capture_name_table, input->source_name_table,
               (size_t)map_bytes) != 0 ||
        memcmp(input->capture_character_generator,
               input->source_character_generator,
               (size_t)input->capture_character_generator_size) != 0 ||
        memcmp(input->capture_cram, input->source_cram, 4096U) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.layer_registers_verified = 1;
    receipt.nbg1_tilemap_mode = 1;
    receipt.pnd_size_two_words = 1;
    receipt.colour_code_4_or_8bpp = 1;
    receipt.name_table_span_join_verified = 1;
    receipt.character_generator_span_join_verified = 1;
    receipt.cram_span_join_verified = 1;
    receipt.original_saturn_capture_verified = 1;
    receipt.bits_per_pixel = bpp;
    receipt.destination_x = input->destination_x;
    receipt.destination_y = input->destination_y;
    receipt.map_columns = input->map_columns;
    receipt.map_rows = input->map_rows;
    for (x = 0; x < 256; ++x)
        framebuffer->palette[x] = cram_to_rgba(input->source_cram + x * 2);

    for (y = 0; y < input->map_rows; ++y) {
        for (x = 0; x < input->map_columns; ++x) {
            int map_offset = (y * input->map_columns + x) * 4;
            uint16_t attr = read_be16(input->source_name_table + map_offset);
            uint16_t charno = read_be16(input->source_name_table + map_offset + 2);
            int palno = attr & 0x7f;
            int hflip = (attr & 0x4000U) != 0U;
            int vflip = (attr & 0x8000U) != 0U;
            int pcco = bpp == 4 ? palno * 16 : (palno * 16) & ~255;
            int tile_bytes = bpp == 4 ? 32 : 64;
            int cg_offset = (int)charno * tile_bytes;
            int py;
            if (cg_offset < 0 || cg_offset + tile_bytes >
                input->source_character_generator_size || pcco + ((1 << bpp) - 1) >= 256) {
                *out_receipt = (Nexus_V1_Vdp2TilemapCaptureReceipt){0};
                return 0;
            }
            ++receipt.tiles_decoded;
            for (py = 0; py < 8; ++py) {
                int sy = vflip ? 7 - py : py;
                int px;
                for (px = 0; px < 8; ++px) {
                    int sx = hflip ? 7 - px : px;
                    uint8_t index;
                    if (bpp == 4) {
                        uint8_t packed = input->source_character_generator[
                            cg_offset + sy * 4 + sx / 2];
                        index = (uint8_t)((sx & 1) == 0 ? packed >> 4 : packed & 0xf);
                    } else {
                        index = input->source_character_generator[cg_offset + sy * 8 + sx];
                    }
                    if (index == 0U) {
                        ++receipt.transparent_pixels;
                        continue;
                    }
                    framebuffer->color_buffer[(input->destination_y + y * 8 + py) *
                        NEXUS_FB_W + input->destination_x + x * 8 + px] =
                        (uint8_t)(pcco + index);
                    ++receipt.written_pixels;
                }
            }
        }
    }
    receipt.valid = receipt.written_pixels > 0;
    receipt.renderer_permitted = receipt.valid;
    *out_receipt = receipt;
    return receipt.valid;
}
