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

static uint16_t read_vram16(const uint8_t *p)
{
    /* Raw Mednafen VDP2 payloads retain the host byte order of Saturn words. */
    return read_le16(p);
}

static int vdp2_register_score(const uint8_t *registers, int little)
{
    uint16_t tvmd = little ? read_le16(registers) : read_be16(registers);
    uint16_t bgon = little ? read_le16(registers + 0x20U) :
        read_be16(registers + 0x20U);
    uint16_t chctla = little ? read_le16(registers + 0x28U) :
        read_be16(registers + 0x28U);
    int score = 0;

    if ((tvmd & 0x8000U) != 0U) score += 3;
    if ((bgon & 0x001fU) != 0U) score += 4;
    if ((bgon & ~0x1f3fU) == 0U) score += 1;
    if ((bgon & 0x0002U) != 0U) {
        score += 2;
        if ((chctla & 0x0200U) != 0U) score += 1;
    }
    return score;
}

static uint16_t read_register16(const uint8_t *registers, size_t offset)
{
    uint16_t big = read_be16(registers);
    uint16_t little = read_le16(registers);

    /* Real captures use Saturn's TVMD display-enable bit 0x8000. Keep the
     * older 0x0080 fixture witness as a compatibility serialization. A
     * big-endian 0x0080 TVMD must stay big-endian; otherwise the little-endian
     * probe below reverses every subsequent VDP2 register. */
    if (big == 0x0080U)
        return read_be16(registers + offset);
    if (big != 0x0080U && little == 0x0080U)
        return read_le16(registers + offset);
    return vdp2_register_score(registers, 1) >=
        vdp2_register_score(registers, 0)
        ? read_le16(registers + offset) : read_be16(registers + offset);
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

int nexus_v1_vdp2_capture_replay_runtime_frame_nbg1_tilemap(
    Nexus_Framebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    const Nexus_V1_Vdp2RuntimeTilemapBinding *binding,
    Nexus_V1_Vdp2TilemapCaptureReceipt *out_receipt)
{
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_Vdp2TilemapCaptureInput input;
    int map_bytes;

    memset(&frame, 0, sizeof(frame));
    memset(&input, 0, sizeof(input));
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!framebuffer || !binding || !binding->source_name_table ||
        !binding->source_character_generator ||
        !binding->source_cram ||
        binding->capture_character_generator_size <= 0 ||
        binding->map_columns <= 0 || binding->map_rows <= 0 ||
        !nexus_v1_saturn_runtime_capture_frame(
            capture_bytes, capture_byte_count, frame_index, &frame) ||
        !frame.valid || !frame.vdp2_vram || !frame.vdp2_cram ||
        !frame.vdp2_registers || frame.vdp2_cram_size != 4096U ||
        frame.vdp2_register_size < NEXUS_V1_VDP2_TILEMAP_REGISTERS_BYTES) {
        return 0;
    }
    map_bytes = binding->map_columns * binding->map_rows * 4;
    if (binding->map_columns > 1024 || binding->map_rows > 1024 ||
        binding->source_name_table_size != map_bytes ||
        binding->capture_name_table_offset > frame.vdp2_vram_size ||
        (size_t)map_bytes > frame.vdp2_vram_size -
            binding->capture_name_table_offset ||
        binding->capture_character_generator_offset > frame.vdp2_vram_size ||
        (size_t)binding->capture_character_generator_size >
            frame.vdp2_vram_size - binding->capture_character_generator_offset ||
        binding->source_character_generator_size !=
            binding->capture_character_generator_size) return 0;
    input.capture_name_table = frame.vdp2_vram +
        binding->capture_name_table_offset;
    input.capture_name_table_size = map_bytes;
    input.capture_character_generator = frame.vdp2_vram +
        binding->capture_character_generator_offset;
    input.capture_character_generator_size =
        binding->capture_character_generator_size;
    input.capture_cram = frame.vdp2_cram;
    input.capture_cram_size = (int)frame.vdp2_cram_size;
    input.vdp2_registers = frame.vdp2_registers;
    input.vdp2_registers_size = (int)frame.vdp2_register_size;
    input.source_name_table = binding->source_name_table;
    input.source_name_table_size = binding->source_name_table_size;
    input.source_character_generator = binding->source_character_generator;
    input.source_character_generator_size =
        binding->source_character_generator_size;
    input.source_cram = binding->source_cram;
    input.source_cram_size = binding->source_cram_size;
    input.map_columns = binding->map_columns;
    input.map_rows = binding->map_rows;
    input.source_tile_x = binding->source_tile_x;
    input.source_tile_y = binding->source_tile_y;
    input.destination_x = binding->destination_x;
    input.destination_y = binding->destination_y;
    input.source_hash_verified = binding->source_hash_verified;
    input.original_saturn_capture_verified = 1;
    input.transparent_index_zero_verified =
        binding->transparent_index_zero_verified;
    return nexus_v1_vdp2_capture_composite_nbg1_tilemap(
        framebuffer, &input, out_receipt);
}

int nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
    Nexus_Framebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    int source_x, int source_y, int width, int height,
    int destination_x, int destination_y,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_SaturnVdp2RegisterReceipt *out_register_receipt,
    Nexus_V1_Vdp2TilemapCaptureReceipt *out_receipt)
{
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_SaturnVdp2RegisterReceipt registers;
    Nexus_V1_Vdp2TilemapCaptureReceipt receipt;
    uint16_t pncn1;
    uint16_t plsz;
    uint16_t mpofn;
    uint16_t map_a;
    uint16_t map_b;
    uint16_t map_offset;
    int bpp;
    int x;
    int y;

    memset(&frame, 0, sizeof(frame));
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_register_receipt) *out_register_receipt = registers;
    if (out_receipt) *out_receipt = receipt;
    if (!framebuffer || !capture_bytes ||
        source_x < 0 || source_y < 0 || width <= 0 || height <= 0 ||
        source_x + width > 512 || source_y + height > 512 ||
        destination_x < 0 || destination_y < 0 ||
        destination_x + width > NEXUS_FB_W ||
        destination_y + height > NEXUS_FB_H ||
        !nexus_v1_saturn_runtime_capture_frame(
            capture_bytes, capture_byte_count, frame_index, &frame) ||
        !frame.valid || !frame.vdp2_vram || !frame.vdp2_cram ||
        !frame.vdp2_registers ||
        frame.vdp2_vram_size != NEXUS_V1_SATURN_VDP2_VRAM_BYTES ||
        frame.vdp2_cram_size != NEXUS_V1_SATURN_VDP2_CRAM_BYTES ||
        frame.vdp2_register_size < NEXUS_V1_VDP2_TILEMAP_REGISTERS_BYTES ||
        !nexus_v1_saturn_runtime_capture_vdp2_register_receipt(
            &frame, &registers) || !registers.valid ||
        !registers.nbg1_enabled || registers.nbg1_bitmap_mode ||
        registers.nbg1_16x16_character_mode ||
        registers.nbg1_colour_code > 1) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_register_receipt) *out_register_receipt = registers;
        return 0;
    }
    pncn1 = read_register16(frame.vdp2_registers, 0x32U);
    plsz = read_register16(frame.vdp2_registers, 0x3aU);
    mpofn = read_register16(frame.vdp2_registers, 0x3cU);
    map_a = read_register16(frame.vdp2_registers, 0x44U) & 0x3fU;
    map_b = read_register16(frame.vdp2_registers, 0x46U) & 0x3fU;
    bpp = registers.nbg1_colour_code == 0 ? 4 : 8;
    /* This is the smallest fully specified Mednafen TileFetcher lane:
     * PNDSize=0, CharSize=0, PlaneSize=0 and identical A/B maps. */
    if ((pncn1 & 0x8000U) != 0U || (plsz & 0x000cU) != 0U ||
        map_a != map_b || map_a >= 0x20U ||
        (((uint32_t)(map_a) << 13U) + 0x1000U) >
            frame.vdp2_vram_size) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_register_receipt) *out_register_receipt = registers;
        return 0;
    }
    map_offset = (mpofn >> 4U) & 7U;
    receipt.valid = 1;
    receipt.capture_only = 1;
    receipt.layer_registers_verified = 1;
    receipt.nbg1_tilemap_mode = 1;
    receipt.pnd_size_two_words = 1;
    receipt.colour_code_4_or_8bpp = 1;
    receipt.original_saturn_capture_verified = 1;
    receipt.bits_per_pixel = bpp;
    receipt.destination_x = destination_x;
    receipt.destination_y = destination_y;
    receipt.map_columns = 64;
    receipt.map_rows = 64;
    for (x = 0; x < 256; ++x)
        framebuffer->palette[x] = cram_to_rgba(frame.vdp2_cram + x * 2);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            int tile_x = source_x + x;
            int tile_y = source_y + y;
            int map_index = ((tile_y >> 3) & 0x3f) * 64 +
                ((tile_x >> 3) & 0x3f);
            uint32_t map_address = (((uint32_t)map_offset << 6U) + map_a) *
                0x2000U + (uint32_t)map_index * 4U;
            uint16_t attr = read_vram16(frame.vdp2_vram + map_address);
            uint16_t charno = read_vram16(frame.vdp2_vram + map_address + 2U);
            int palno = attr & 0x7f;
            int tile_bytes = bpp == 4 ? 32 : 64;
            uint32_t char_address = (uint32_t)charno * (uint32_t)tile_bytes;
            int px = tile_x & 7;
            int py = tile_y & 7;
            uint8_t index;
            int palette_index;
            int destination;
            if (char_address + (uint32_t)tile_bytes > frame.vdp2_vram_size)
                continue;
            if ((attr & 0x4000U) != 0U) px = 7 - px;
            if ((attr & 0x8000U) != 0U) py = 7 - py;
            if (bpp == 4) {
                uint8_t packed = frame.vdp2_vram[char_address +
                    (uint32_t)py * 4U + (uint32_t)(px / 2)];
                index = (uint8_t)((px & 1) == 0 ? packed >> 4 : packed & 0xf);
                palette_index = palno * 16 + index;
            } else {
                index = frame.vdp2_vram[char_address + (uint32_t)py * 8U +
                    (uint32_t)px];
                palette_index = ((palno * 16) & ~255) + index;
            }
            destination = (destination_y + y) * NEXUS_FB_W +
                destination_x + x;
            if (index == 0U) {
                ++receipt.transparent_pixels;
            } else {
                framebuffer->color_buffer[destination] = (uint8_t)palette_index;
                ++receipt.written_pixels;
            }
        }
    }
    receipt.renderer_permitted = 0;
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_register_receipt) *out_register_receipt = registers;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}
