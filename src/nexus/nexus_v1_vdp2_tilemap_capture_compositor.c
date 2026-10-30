#include "nexus_v1_vdp2_tilemap_capture_compositor.h"

#include <string.h>
#include <stdlib.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8U) | p[1]);
}

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[1] << 8U) | p[0]);
}

static uint16_t read_vram16_ordered(
    const uint8_t *p, Nexus_V1_SaturnVdp2RegisterByteOrder byte_order)
{
    /* The Mednafen review candidate preserves Saturn bus order explicitly;
     * older Firestaff fixtures remain readable through the little-endian
     * compatibility branch selected by the register receipt. */
    return byte_order == NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE
        ? read_le16(p) : read_be16(p);
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

static uint16_t read_register16_ordered(
    const uint8_t *registers, size_t offset,
    Nexus_V1_SaturnVdp2RegisterByteOrder order)
{
    if (order == NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG)
        return read_be16(registers + offset);
    if (order == NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE)
        return read_le16(registers + offset);
    return read_register16(registers, offset);
}

static uint8_t expand5(uint16_t value, unsigned shift)
{
    uint8_t result = (uint8_t)(((value >> shift) & 0x1fU) << 3U);
    return (uint8_t)(result | (result >> 5U));
}

static uint32_t cram_to_rgba_ordered(
    const uint8_t *entry, Nexus_V1_SaturnVdp2RegisterByteOrder byte_order)
{
    uint16_t value = byte_order == NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE
        ? read_le16(entry) : read_be16(entry);
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
        input->destination_x > NEXUS_FB_W ||
        input->destination_y > NEXUS_FB_H ||
        input->map_columns * 8 > NEXUS_FB_W - input->destination_x ||
        input->map_rows * 8 > NEXUS_FB_H - input->destination_y ||
        input->vdp2_registers_size < (int)NEXUS_V1_VDP2_TILEMAP_REGISTERS_BYTES ||
        input->capture_cram_size != 4096 || input->source_cram_size != 4096 ||
        input->capture_name_table_size != input->map_columns * input->map_rows * 4 ||
        input->source_name_table_size != input->map_columns * input->map_rows * 4 ||
        input->capture_character_generator_size <= 0 ||
        input->source_character_generator_size !=
            input->capture_character_generator_size) {
        *out_receipt = receipt;
        return 0;
    }
    if (!input->source_hash_verified ||
        !input->original_saturn_capture_verified ||
        !input->transparent_index_zero_verified) {
        *out_receipt = receipt;
        return 0;
    }

    map_bytes = input->map_columns * input->map_rows * 4;
    bgon = read_register16_ordered(input->vdp2_registers,
                                   NEXUS_V1_VDP2_TILEMAP_BGON_OFFSET,
                                   input->register_byte_order);
    chctla = read_register16_ordered(input->vdp2_registers,
                                     NEXUS_V1_VDP2_TILEMAP_CHCTLA_OFFSET,
                                     input->register_byte_order);
    pncn1 = read_register16_ordered(input->vdp2_registers,
                                    NEXUS_V1_VDP2_TILEMAP_PNCN1_OFFSET,
                                    input->register_byte_order);
    craofa = read_register16_ordered(input->vdp2_registers,
                                     NEXUS_V1_VDP2_TILEMAP_CRAOFA_OFFSET,
                                     input->register_byte_order);
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
        framebuffer->palette[x] = cram_to_rgba_ordered(
            input->source_cram + x * 2, input->register_byte_order);

    for (y = 0; y < input->map_rows; ++y) {
        for (x = 0; x < input->map_columns; ++x) {
            int map_offset = (y * input->map_columns + x) * 4;
            uint16_t attr = read_vram16_ordered(
                input->source_name_table + map_offset,
                input->register_byte_order);
            uint16_t charno = read_vram16_ordered(
                input->source_name_table + map_offset + 2,
                input->register_byte_order);
            int palno = attr & 0x7f;
            int hflip = (attr & 0x4000U) != 0U;
            int vflip = (attr & 0x8000U) != 0U;
            int pcco = bpp == 4 ? palno * 16 : (palno * 16) & ~255;
            int tile_bytes = bpp == 4 ? 32 : 64;
            /* Saturn/Ymir character numbers advance in 32-byte units in
             * both color modes; an 8bpp tile reads 64 bytes from that base
             * and can overlap the next character number. */
            uint32_t cg_offset = ((uint32_t)(charno & 0x7fffU) << 5U) &
                UINT32_C(0x7ffff);
            int py;
            if ((input->source_character_generator_size <
                    (int)NEXUS_V1_SATURN_VDP2_VRAM_BYTES &&
                 cg_offset + (uint32_t)tile_bytes >
                    (uint32_t)input->source_character_generator_size) ||
                pcco + ((1 << bpp) - 1) >= 256) {
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
                        uint32_t pixel_offset = (cg_offset +
                            (uint32_t)sy * 4U + (uint32_t)(sx / 2)) &
                            UINT32_C(0x7ffff);
                        uint8_t packed = input->source_character_generator[
                            pixel_offset];
                        index = (uint8_t)((sx & 1) == 0 ? packed >> 4 : packed & 0xf);
                    } else {
                        uint32_t pixel_offset = (cg_offset +
                            (uint32_t)sy * 8U + (uint32_t)sx) &
                            UINT32_C(0x7ffff);
                        index = input->source_character_generator[pixel_offset];
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
    {
        Nexus_V1_SaturnVdp2RegisterReceipt registers;
        if (!nexus_v1_saturn_runtime_capture_vdp2_register_receipt(
                &frame, &registers) || !registers.valid) return 0;
        input.register_byte_order = registers.byte_order;
    }
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
    uint8_t visited_tiles[64 * 64] = {0};

    memset(&frame, 0, sizeof(frame));
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_register_receipt) *out_register_receipt = registers;
    if (out_receipt) *out_receipt = receipt;
    if (!framebuffer || !capture_bytes ||
        source_x < 0 || source_y < 0 || width <= 0 || height <= 0 ||
        source_x > 512 || source_y > 512 ||
        width > 512 - source_x || height > 512 - source_y ||
        destination_x < 0 || destination_y < 0 ||
        destination_x > NEXUS_FB_W || destination_y > NEXUS_FB_H ||
        width > NEXUS_FB_W - destination_x ||
        height > NEXUS_FB_H - destination_y ||
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
    pncn1 = read_register16_ordered(frame.vdp2_registers, 0x32U,
                                    registers.byte_order);
    plsz = read_register16_ordered(frame.vdp2_registers, 0x3aU,
                                   registers.byte_order);
    mpofn = read_register16_ordered(frame.vdp2_registers, 0x3cU,
                                    registers.byte_order);
    map_a = read_register16_ordered(frame.vdp2_registers, 0x44U,
                                    registers.byte_order) & 0x3fU;
    map_b = read_register16_ordered(frame.vdp2_registers, 0x46U,
                                    registers.byte_order) & 0x3fU;
    map_offset = (mpofn >> 4U) & 7U;
    bpp = registers.nbg1_colour_code == 0 ? 4 : 8;
    /* This is the smallest fully specified Mednafen TileFetcher lane:
     * PNDSize=0, CharSize=0, PlaneSize=0 and identical A/B maps. */
    if ((pncn1 & 0x8000U) != 0U || (plsz & 0x000cU) != 0U ||
        map_a != map_b || map_a >= 0x20U) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_register_receipt) *out_register_receipt = registers;
        return 0;
    }
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
        framebuffer->palette[x] = cram_to_rgba_ordered(
            frame.vdp2_cram + x * 2, registers.byte_order);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            int tile_x = source_x + x + registers.nbg1_scroll_x;
            int tile_y = source_y + y + registers.nbg1_scroll_y;
            int map_index = ((tile_y >> 3) & 0x3f) * 64 +
                ((tile_x >> 3) & 0x3f);
            /* Match the bounded TileFetcher address derivation mirrored by
             * scripts/analyze_nexus_vdp2_composition.py::
             * visible_character_spans (name_word wrap and byte conversion). */
            uint32_t name_word_address =
                ((((uint32_t)map_offset << 6U) + map_a) << 13U) +
                (uint32_t)map_index * 2U;
            uint32_t map_address =
                (name_word_address & 0x3ffffU) * 2U;
            uint16_t attr = read_vram16_ordered(
                frame.vdp2_vram + map_address, registers.byte_order);
            uint16_t charno = read_vram16_ordered(
                frame.vdp2_vram + map_address + 2U, registers.byte_order);
            int palno = attr & 0x7f;
            uint32_t char_address = ((uint32_t)charno << 5U) & 0x7ffffU;
            int px = tile_x & 7;
            int py = tile_y & 7;
            uint8_t index;
            int palette_index;
            int destination;
            if ((attr & 0x4000U) != 0U) px = 7 - px;
            if ((attr & 0x8000U) != 0U) py = 7 - py;
            if (bpp == 4) {
                uint32_t pixel_address = (char_address +
                    (uint32_t)py * 4U + (uint32_t)(px / 2)) & 0x7ffffU;
                uint8_t packed = frame.vdp2_vram[pixel_address];
                index = (uint8_t)((px & 1) == 0 ? packed >> 4 : packed & 0xf);
                palette_index = palno * 16 + index;
            } else {
                uint32_t pixel_address = (char_address +
                    (uint32_t)py * 8U + (uint32_t)px) & 0x7ffffU;
                index = frame.vdp2_vram[pixel_address];
                palette_index = ((palno * 16) & ~255) + index;
            }
            destination = (destination_y + y) * NEXUS_FB_W +
                destination_x + x;
            if (!visited_tiles[map_index]) {
                visited_tiles[map_index] = 1U;
                ++receipt.tiles_decoded;
            }
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

int nexus_v1_vdp2_capture_decode_ymir_nbg2_tilemap_plane(
    Nexus_V1_Vdp2CapturePlane *plane,
    const uint8_t *vdp2_vram, size_t vdp2_vram_size,
    const uint8_t *vdp2_cram, size_t vdp2_cram_size,
    const uint8_t *vdp2_registers, size_t vdp2_registers_size,
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order,
    int original_ymir_capture_verified,
    Nexus_V1_Vdp2Nbg2CapturePlaneReceipt *out_receipt)
{
    Nexus_V1_Vdp2Nbg2CapturePlaneReceipt receipt;
    uint16_t bgon;
    uint16_t chctlb;
    uint16_t pncnc;
    uint16_t plsz;
    uint16_t mpofn;
    uint16_t mapa;
    uint16_t mapb;
    uint16_t craofa;
    uint32_t map_base;
    unsigned int y;
    unsigned int x;

    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    receipt.name_table_vram_offset = -1;
    if (out_receipt) *out_receipt = receipt;
    if (!plane || !vdp2_vram || !vdp2_cram || !vdp2_registers ||
        vdp2_vram_size != NEXUS_V1_SATURN_VDP2_VRAM_BYTES ||
        vdp2_cram_size != NEXUS_V1_SATURN_VDP2_CRAM_BYTES ||
        vdp2_registers_size < 0x100U ||
        register_byte_order == NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_UNKNOWN ||
        !original_ymir_capture_verified) return 0;
#define NBG2_REG(offset) read_register16_ordered( \
        vdp2_registers, (offset), register_byte_order)
    bgon = NBG2_REG(0x20U);
    chctlb = NBG2_REG(0x2aU);
    pncnc = NBG2_REG(0x34U);
    plsz = NBG2_REG(0x3aU);
    mpofn = NBG2_REG(0x3cU);
    mapa = NBG2_REG(0x48U) & 0x3fU;
    mapb = NBG2_REG(0x4aU) & 0x3fU;
    craofa = NBG2_REG(NEXUS_V1_VDP2_TILEMAP_CRAOFA_OFFSET);
    if (NBG2_REG(0x00U) != 0x8000U || bgon != 0x0007U ||
        chctlb != 0x0002U || pncnc != 0x0000U ||
        (plsz & 0x0030U) != 0U || (mpofn & 0x0700U) != 0U ||
        mapa != 0x18U || mapb != 0x18U ||
        NBG2_REG(0x90U) != 0U || NBG2_REG(0x92U) != 0U ||
        ((craofa >> 8U) & 7U) != 0U ||
        (NBG2_REG(0xfaU) & 7U) != 4U) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    /* NBG2 map A/B are both 0x18 in this frame. The name-table word base is
     * ((MPOFN_NBG2 << 6) + MAP) << 13; convert wrapped Saturn word addresses
     * to bytes only after the 18-bit word-address wrap. */
    map_base = (((uint32_t)((mpofn >> 8U) & 7U) << 6U) + mapa) << 13U;
    map_base = (map_base & 0x3ffffU) * 2U;
    if (map_base != 0x60000U || map_base > vdp2_vram_size - 64U * 64U * 4U) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    memset(plane, 0, sizeof(*plane));
    receipt.registers_verified = 1;
    receipt.original_ymir_capture_verified = 1;
    receipt.nbg2_tilemap_mode = 1;
    receipt.pnd_size_two_words = 1;
    receipt.colour_code_256 = 1;
    receipt.name_table_vram_offset = (int)map_base;
    receipt.map_page_columns = 64;
    receipt.map_page_rows = 64;
    receipt.visible_columns = (int)(NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH / 8U);
    receipt.visible_rows = (int)(NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT / 8U);
    for (y = 0U; y < NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT; ++y) {
        for (x = 0U; x < NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH; ++x) {
            unsigned int tile_x = x >> 3U;
            unsigned int tile_y = y >> 3U;
            uint32_t map_offset = (tile_y * 64U + tile_x) * 4U;
            uint16_t attr = read_vram16_ordered(
                vdp2_vram + map_base + map_offset, register_byte_order);
            uint16_t charno = read_vram16_ordered(
                vdp2_vram + map_base + map_offset + 2U, register_byte_order);
            uint32_t char_base = ((uint32_t)(charno & 0x7fffU) << 5U) &
                UINT32_C(0x7ffff);
            unsigned int px = x & 7U;
            unsigned int py = y & 7U;
            uint32_t pixel_offset;
            uint8_t index;
            size_t destination = (size_t)y *
                NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH + x;
            if ((attr & 0x4000U) != 0U) px = 7U - px;
            if ((attr & 0x8000U) != 0U) py = 7U - py;
            pixel_offset = (char_base + py * 8U + px) & UINT32_C(0x7ffff);
            index = vdp2_vram[pixel_offset];
            plane->index_buffer[destination] = index;
            if (index == 0U) {
                ++receipt.transparent_pixels;
            } else {
                plane->rgba_buffer[destination] = cram_to_rgba_ordered(
                    vdp2_cram + (size_t)index * 2U, register_byte_order);
                ++receipt.written_pixels;
            }
        }
    }
    receipt.tiles_decoded = receipt.visible_columns * receipt.visible_rows;
    receipt.valid = 1;
    receipt.renderer_permitted = 0;
    if (out_receipt) *out_receipt = receipt;
#undef NBG2_REG
    return 1;
}

int nexus_v1_vdp2_capture_decode_ymir_nbg0_bitmap_plane(
    Nexus_V1_Vdp2CapturePlane *plane,
    const uint8_t *vdp2_vram, size_t vdp2_vram_size,
    const uint8_t *vdp2_cram, size_t vdp2_cram_size,
    const uint8_t *vdp2_registers, size_t vdp2_registers_size,
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order,
    int original_ymir_capture_verified,
    Nexus_V1_Vdp2Nbg0CapturePlaneReceipt *out_receipt)
{
    Nexus_V1_Vdp2Nbg0CapturePlaneReceipt receipt;
    uint16_t bgon;
    uint16_t chctla;
    uint16_t bmpna;
    uint16_t mpofn;
    uint16_t craofa;
    unsigned int y;
    unsigned int x;

    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    receipt.bitmap_vram_offset = -1;
    if (out_receipt) *out_receipt = receipt;
    if (!plane || !vdp2_vram || !vdp2_cram || !vdp2_registers ||
        vdp2_vram_size != NEXUS_V1_SATURN_VDP2_VRAM_BYTES ||
        vdp2_cram_size != NEXUS_V1_SATURN_VDP2_CRAM_BYTES ||
        vdp2_registers_size < 0x100U ||
        register_byte_order == NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_UNKNOWN ||
        !original_ymir_capture_verified) return 0;
#define NBG0_REG(offset) read_register16_ordered( \
        vdp2_registers, (offset), register_byte_order)
    bgon = NBG0_REG(0x20U);
    chctla = NBG0_REG(0x28U);
    bmpna = NBG0_REG(0x2cU);
    mpofn = NBG0_REG(0x3cU);
    craofa = NBG0_REG(NEXUS_V1_VDP2_TILEMAP_CRAOFA_OFFSET);
    if (NBG0_REG(0x00U) != 0x8000U || bgon != 0x0007U ||
        chctla != 0x1013U || (bmpna & 7U) != 0U || mpofn != 0U ||
        (craofa & 7U) != 0U || (NBG0_REG(0xf8U) & 7U) != 4U ||
        NBG0_REG(0x70U) != 0U || NBG0_REG(0x72U) != 0U ||
        NBG0_REG(0x74U) != 0U || NBG0_REG(0x76U) != 0U ||
        NBG0_REG(0x78U) != 1U || NBG0_REG(0x7aU) != 0U ||
        NBG0_REG(0x7cU) != 1U || NBG0_REG(0x7eU) != 0U) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    memset(plane, 0, sizeof(*plane));
    receipt.registers_verified = 1;
    receipt.original_ymir_capture_verified = 1;
    receipt.nbg0_bitmap_mode = 1;
    receipt.colour_code_256 = 1;
    receipt.bitmap_vram_offset = 0;
    for (y = 0U; y < NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT; ++y) {
        for (x = 0U; x < NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH; ++x) {
            size_t source = (size_t)y * 512U + x;
            size_t destination = (size_t)y *
                NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH + x;
            uint8_t index = vdp2_vram[source];
            plane->index_buffer[destination] = index;
            if (index == 0U) {
                ++receipt.transparent_pixels;
            } else {
                plane->rgba_buffer[destination] = cram_to_rgba_ordered(
                    vdp2_cram + (size_t)index * 2U, register_byte_order);
                ++receipt.written_pixels;
            }
        }
    }
    receipt.valid = 1;
    receipt.renderer_permitted = 0;
    if (out_receipt) *out_receipt = receipt;
#undef NBG0_REG
    return 1;
}

int nexus_v1_vdp2_capture_decode_ymir_nbg1_tilemap_plane(
    Nexus_V1_Vdp2CapturePlane *plane,
    const uint8_t *vdp2_vram, size_t vdp2_vram_size,
    const uint8_t *vdp2_cram, size_t vdp2_cram_size,
    const uint8_t *vdp2_registers, size_t vdp2_registers_size,
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order,
    int original_ymir_capture_verified,
    Nexus_V1_Vdp2Nbg1CapturePlaneReceipt *out_receipt)
{
    Nexus_V1_Vdp2Nbg1CapturePlaneReceipt receipt;
    uint16_t bgon, chctla, pncn1, plsz, mpofn, mapa, mapb, craofa;
    uint32_t map_base;
    unsigned int x, y;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    receipt.name_table_vram_offset = -1;
    if (out_receipt) *out_receipt = receipt;
    if (!plane || !vdp2_vram || !vdp2_cram || !vdp2_registers ||
        vdp2_vram_size != NEXUS_V1_SATURN_VDP2_VRAM_BYTES ||
        vdp2_cram_size != NEXUS_V1_SATURN_VDP2_CRAM_BYTES ||
        vdp2_registers_size < 0x100U ||
        register_byte_order == NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_UNKNOWN ||
        !original_ymir_capture_verified) return 0;
#define NBG1_REG(offset) read_register16_ordered( \
        vdp2_registers, (offset), register_byte_order)
    bgon = NBG1_REG(0x20U); chctla = NBG1_REG(0x28U);
    pncn1 = NBG1_REG(0x32U); plsz = NBG1_REG(0x3aU);
    mpofn = NBG1_REG(0x3cU); mapa = NBG1_REG(0x44U) & 0x3fU;
    mapb = NBG1_REG(0x46U) & 0x3fU;
    craofa = NBG1_REG(NEXUS_V1_VDP2_TILEMAP_CRAOFA_OFFSET);
    if (NBG1_REG(0x00U) != 0x8000U || bgon != 7U || chctla != 0x1013U ||
        pncn1 != 0U || (plsz & 0x000cU) != 0U ||
        ((mpofn >> 4U) & 7U) != 0U || mapa != 0x17U || mapb != 0x17U ||
        (craofa & 0x70U) != 0U || (NBG1_REG(0xf8U) & 0x0700U) != 0x0400U ||
        (NBG1_REG(0xfaU) & 7U) != 4U ||
        NBG1_REG(0x80U) || NBG1_REG(0x82U) || NBG1_REG(0x84U) ||
        NBG1_REG(0x86U) || NBG1_REG(0x88U) != 1U ||
        NBG1_REG(0x8aU) || NBG1_REG(0x8cU) != 1U || NBG1_REG(0x8eU))
        return 0;
    map_base = ((((uint32_t)((mpofn >> 4U) & 7U) << 6U) + mapa) << 13U);
    map_base = (map_base & 0x3ffffU) * 2U;
    if (map_base != 0x5c000U || map_base > vdp2_vram_size - 64U*64U*4U)
        return 0;
    memset(plane, 0, sizeof(*plane));
    receipt.registers_verified = receipt.original_ymir_capture_verified = 1;
    receipt.nbg1_tilemap_mode = receipt.pnd_size_two_words = 1;
    receipt.colour_code_256 = 1;
    receipt.name_table_vram_offset = (int)map_base;
    receipt.map_page_columns = receipt.map_page_rows = 64;
    receipt.visible_columns = 40; receipt.visible_rows = 28;
    for (y = 0; y < 224; ++y) for (x = 0; x < 320; ++x) {
        uint32_t mi = (y >> 3U) * 64U + (x >> 3U);
        uint32_t ma = map_base + mi * 4U;
        uint16_t attr = read_vram16_ordered(vdp2_vram + ma, register_byte_order);
        uint16_t charno = read_vram16_ordered(vdp2_vram + ma + 2U, register_byte_order);
        unsigned int px = x & 7U, py = y & 7U;
        uint32_t ca = ((uint32_t)(charno & 0x7fffU) << 5U) & 0x7ffffU;
        uint8_t index;
        size_t dst = (size_t)y * 320U + x;
        if (attr & 0x4000U) px = 7U - px;
        if (attr & 0x8000U) py = 7U - py;
        index = vdp2_vram[(ca + py * 8U + px) & 0x7ffffU];
        plane->index_buffer[dst] = index;
        if (!index) ++receipt.transparent_pixels;
        else { plane->rgba_buffer[dst] = cram_to_rgba_ordered(vdp2_cram + (size_t)index*2U, register_byte_order); ++receipt.written_pixels; }
    }
    receipt.tiles_decoded = receipt.visible_columns * receipt.visible_rows;
    receipt.valid = 1; receipt.renderer_permitted = 0;
    if (out_receipt) *out_receipt = receipt;
#undef NBG1_REG
    return 1;
}

static uint32_t ymir_rgb555_shift(uint16_t value)
{
    uint32_t red = ((uint32_t)value & 0x1fU) << 3U;
    uint32_t green = ((uint32_t)value >> 2U) & 0xf8U;
    uint32_t blue = ((uint32_t)value >> 7U) & 0xf8U;
    return UINT32_C(0xff000000) | (red << 16U) | (green << 8U) | blue;
}

static uint8_t ymir_sprite_priority(const uint8_t *registers,
                                    unsigned int priority_index)
{
    unsigned int register_offset = 0xf0U + (priority_index / 2U) * 2U;
    unsigned int shift = (priority_index & 1U) ? 8U : 0U;
    return (uint8_t)((read_register16_ordered(registers, register_offset,
        NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG) >> shift) & 7U);
}

static void ymir_stack_consider(
    Nexus_V1_Vdp2YmirFrame239StackPlane *plane, size_t pixel,
    uint8_t priority, uint8_t ymir_layer, uint8_t output_layer,
    uint32_t rgba)
{
    uint8_t current_layer = plane->top_layer_buffer[pixel];
    uint8_t current_priority = plane->priority_buffer[pixel];
    if (priority == 0U ||
        (current_layer != NEXUS_V1_VDP2_STACK_LAYER_EMPTY &&
         (priority < current_priority ||
          (priority == current_priority &&
           ymir_layer >= (current_layer == NEXUS_V1_VDP2_STACK_LAYER_VDP1 ?
               0U : current_layer + 1U)))))
        return;
    plane->top_layer_buffer[pixel] = output_layer;
    plane->priority_buffer[pixel] = priority;
    plane->rgba_buffer[pixel] = rgba;
}

int nexus_v1_vdp2_capture_compose_ymir_frame239_priority_stack(
    Nexus_V1_Vdp2YmirFrame239StackPlane *plane,
    const uint8_t *vdp1_framebuffer, size_t vdp1_framebuffer_size,
    const uint8_t *vdp2_vram, size_t vdp2_vram_size,
    const uint8_t *vdp2_cram, size_t vdp2_cram_size,
    const uint8_t *vdp2_registers, size_t vdp2_registers_size,
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order,
    int original_ymir_capture_verified,
    int vdp1_framebuffer_verified,
    Nexus_V1_Vdp2YmirFrame239StackReceipt *out_receipt)
{
    Nexus_V1_Vdp2YmirFrame239StackReceipt receipt;
    Nexus_V1_Vdp2CapturePlane *nbg0 = NULL, *nbg1 = NULL, *nbg2 = NULL;
    Nexus_V1_Vdp2Nbg0CapturePlaneReceipt r0;
    Nexus_V1_Vdp2Nbg1CapturePlaneReceipt r1;
    Nexus_V1_Vdp2Nbg2CapturePlaneReceipt r2;
    uint16_t sfprmd, ccctl, spctl, craofb;
    size_t pixel;
    int ok = 0;

    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    if (out_receipt) *out_receipt = receipt;
    if (!plane || !vdp1_framebuffer || !vdp2_vram || !vdp2_cram ||
        !vdp2_registers || vdp1_framebuffer_size !=
            NEXUS_V1_SATURN_VDP1_VRAM_BYTES ||
        vdp2_vram_size != NEXUS_V1_SATURN_VDP2_VRAM_BYTES ||
        vdp2_cram_size != NEXUS_V1_SATURN_VDP2_CRAM_BYTES ||
        vdp2_registers_size < 0x120U ||
        register_byte_order != NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG ||
        !original_ymir_capture_verified || !vdp1_framebuffer_verified)
        return 0;
#define STACK_REG(offset) read_register16_ordered( \
        vdp2_registers, (offset), register_byte_order)
    sfprmd = STACK_REG(0xeaU);
    ccctl = STACK_REG(0xecU);
    spctl = STACK_REG(0xe0U);
    craofb = STACK_REG(0xe6U);
    if (STACK_REG(0x00U) != 0x8000U || STACK_REG(0x20U) != 0x0007U ||
        STACK_REG(0x28U) != 0x1013U || STACK_REG(0x2aU) != 0x0002U ||
        STACK_REG(0x2cU) != 0U || STACK_REG(0x32U) != 0U ||
        STACK_REG(0x34U) != 0U || STACK_REG(0x3aU) != 0U ||
        STACK_REG(0x3cU) != 0U || STACK_REG(0x40U) != 0U ||
        STACK_REG(0x42U) != 0U || STACK_REG(0x44U) != 0x1717U ||
        STACK_REG(0x46U) != 0x1717U || STACK_REG(0x48U) != 0x1818U ||
        STACK_REG(0x4aU) != 0x1818U || STACK_REG(0xe4U) != 0U ||
        craofb != 0U || STACK_REG(0xf8U) != 0x0404U ||
        STACK_REG(0xfaU) != 0x0004U || sfprmd != 0x0015U ||
        ccctl != 0x0142U || STACK_REG(0xeeU) != 0x0004U ||
        spctl != 0x1536U || STACK_REG(0xf0U) != 0x0102U ||
        STACK_REG(0xf2U) != 0x0300U || STACK_REG(0xf4U) != 0x0504U ||
        STACK_REG(0xf6U) != 0x0706U ||
        STACK_REG(0x70U) || STACK_REG(0x72U) ||
        STACK_REG(0x74U) || STACK_REG(0x76U) ||
        STACK_REG(0x78U) != 1U || STACK_REG(0x7aU) ||
        STACK_REG(0x7cU) != 1U || STACK_REG(0x7eU) ||
        STACK_REG(0x80U) || STACK_REG(0x82U) ||
        STACK_REG(0x84U) || STACK_REG(0x86U) ||
        STACK_REG(0x88U) != 1U || STACK_REG(0x8aU) ||
        STACK_REG(0x8cU) != 1U || STACK_REG(0x8eU) ||
        STACK_REG(0x90U) || STACK_REG(0x92U)) {
        goto cleanup;
    }
    nbg0 = (Nexus_V1_Vdp2CapturePlane *)malloc(sizeof(*nbg0));
    nbg1 = (Nexus_V1_Vdp2CapturePlane *)malloc(sizeof(*nbg1));
    nbg2 = (Nexus_V1_Vdp2CapturePlane *)malloc(sizeof(*nbg2));
    if (!nbg0 || !nbg1 || !nbg2 ||
        !nexus_v1_vdp2_capture_decode_ymir_nbg0_bitmap_plane(
            nbg0, vdp2_vram, vdp2_vram_size, vdp2_cram, vdp2_cram_size,
            vdp2_registers, vdp2_registers_size, register_byte_order, 1, &r0) ||
        !nexus_v1_vdp2_capture_decode_ymir_nbg1_tilemap_plane(
            nbg1, vdp2_vram, vdp2_vram_size, vdp2_cram, vdp2_cram_size,
            vdp2_registers, vdp2_registers_size, register_byte_order, 1, &r1) ||
        !nexus_v1_vdp2_capture_decode_ymir_nbg2_tilemap_plane(
            nbg2, vdp2_vram, vdp2_vram_size, vdp2_cram, vdp2_cram_size,
            vdp2_registers, vdp2_registers_size, register_byte_order, 1, &r2)) {
        goto cleanup;
    }
    memset(plane->rgba_buffer, 0, sizeof(plane->rgba_buffer));
    memset(plane->top_layer_buffer, NEXUS_V1_VDP2_STACK_LAYER_EMPTY,
           sizeof(plane->top_layer_buffer));
    memset(plane->priority_buffer, 0, sizeof(plane->priority_buffer));
    receipt.registers_verified = receipt.vdp1_framebuffer_verified = 1;
    receipt.original_ymir_capture_verified = 1;
    receipt.pixel_priority_stack_verified = 1;

    for (pixel = 0U; pixel < NEXUS_V1_VDP2_CAPTURE_PIXELS; ++pixel) {
        unsigned int x = (unsigned int)(pixel % NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH);
        unsigned int y = (unsigned int)(pixel / NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH);
        unsigned int tile_index = (y >> 3U) * 64U + (x >> 3U);
        uint16_t nbg1_meta = read_vram16_ordered(
            vdp2_vram + 0x5c000U + (size_t)tile_index * 4U,
            register_byte_order);
        uint16_t nbg2_meta = read_vram16_ordered(
            vdp2_vram + 0x60000U + (size_t)tile_index * 4U,
            register_byte_order);
        uint8_t i0 = nbg0->index_buffer[pixel];
        uint8_t i1 = nbg1->index_buffer[pixel];
        uint8_t i2 = nbg2->index_buffer[pixel];
        uint16_t sprite = read_be16(vdp1_framebuffer +
            ((size_t)y * 512U + x) * 2U);
        if (i0) ymir_stack_consider(plane, pixel, 4U, 2U,
            NEXUS_V1_VDP2_STACK_LAYER_NBG0,
            ymir_rgb555_shift(read_vram16_ordered(
                vdp2_cram + (size_t)i0 * 2U, register_byte_order)));
        if (i1) {
            uint8_t priority = (uint8_t)((STACK_REG(0xf8U) >> 8U) & 7U);
            priority = (uint8_t)((priority & (uint8_t)~1U) |
                ((sfprmd & 0x000cU) == 0x0004U ? ((nbg1_meta >> 13U) & 1U) : 0U));
            if ((nbg1_meta & 0x1000U) != 0U) {
                ++receipt.color_calculation_cases_unsupported;
                goto cleanup;
            }
            ymir_stack_consider(plane, pixel, priority, 1U,
                NEXUS_V1_VDP2_STACK_LAYER_NBG1,
                ymir_rgb555_shift(read_vram16_ordered(
                    vdp2_cram + (size_t)i1 * 2U, register_byte_order)));
        }
        if (i2) {
            uint8_t priority = (uint8_t)(STACK_REG(0xfaU) & 7U);
            priority = (uint8_t)((priority & (uint8_t)~1U) |
                ((sfprmd & 0x0030U) == 0x0010U ? ((nbg2_meta >> 13U) & 1U) : 0U));
            ymir_stack_consider(plane, pixel, priority, 2U,
                NEXUS_V1_VDP2_STACK_LAYER_NBG2,
                ymir_rgb555_shift(read_vram16_ordered(
                    vdp2_cram + (size_t)i2 * 2U, register_byte_order)));
        }
        if ((sprite & 0x8000U) != 0U) {
            ymir_stack_consider(plane, pixel,
                ymir_sprite_priority(vdp2_registers, 0U), 0U,
                NEXUS_V1_VDP2_STACK_LAYER_VDP1,
                ymir_rgb555_shift((uint16_t)(sprite & 0x7fffU)));
        } else if (sprite != 0U) {
            uint8_t priority = ymir_sprite_priority(vdp2_registers,
                ((unsigned int)sprite >> 12U) & 7U);
            uint16_t colour_code = sprite & 0x03ffU;
            if (colour_code == 0x03feU) {
                ++receipt.shadow_cases_unsupported;
                goto cleanup;
            }
            if (priority == 5U) {
                ++receipt.color_calculation_cases_unsupported;
                goto cleanup;
            }
            ymir_stack_consider(plane, pixel, priority, 0U,
                NEXUS_V1_VDP2_STACK_LAYER_VDP1,
                ymir_rgb555_shift(read_vram16_ordered(
                    vdp2_cram + (size_t)colour_code * 2U,
                    register_byte_order)));
        }
    }
    for (pixel = 0U; pixel < NEXUS_V1_VDP2_CAPTURE_PIXELS; ++pixel) {
        uint8_t current = plane->top_layer_buffer[pixel];
        if (current == NEXUS_V1_VDP2_STACK_LAYER_EMPTY) {
            ++receipt.empty_pixels;
            continue;
        }
        switch (current) {
        case NEXUS_V1_VDP2_STACK_LAYER_VDP1:
            if ((read_be16(vdp1_framebuffer +
                    ((pixel / NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH) * 512U +
                     pixel % NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH) * 2U) &
                 0x8000U) != 0U)
                ++receipt.vdp1_rgb_pixels;
            else
                ++receipt.vdp1_cram_pixels;
            break;
        case NEXUS_V1_VDP2_STACK_LAYER_NBG0: ++receipt.nbg0_pixels; break;
        case NEXUS_V1_VDP2_STACK_LAYER_NBG1: ++receipt.nbg1_pixels; break;
        case NEXUS_V1_VDP2_STACK_LAYER_NBG2: ++receipt.nbg2_pixels; break;
        default: goto cleanup;
        }
    }
    receipt.valid = receipt.empty_pixels == 0;
    ok = receipt.valid;
cleanup:
    free(nbg2);
    free(nbg1);
    free(nbg0);
    if (out_receipt) *out_receipt = receipt;
    if (ok) return 1;
    memset(plane->rgba_buffer, 0, sizeof(plane->rgba_buffer));
    memset(plane->top_layer_buffer, NEXUS_V1_VDP2_STACK_LAYER_EMPTY,
           sizeof(plane->top_layer_buffer));
    memset(plane->priority_buffer, 0, sizeof(plane->priority_buffer));
#undef STACK_REG
    return 0;
}
