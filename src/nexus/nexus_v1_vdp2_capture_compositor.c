#include "nexus_v1_vdp2_capture_compositor.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8U) | p[1]);
}

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[1] << 8U) | p[0]);
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
     * older 0x0080 fixture witness as a compatibility serialization. */
    if (big == 0x0080U) return read_be16(registers + offset);
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

static uint32_t cram_to_rgba_ordered(
    const uint8_t *entry, Nexus_V1_SaturnVdp2RegisterByteOrder byte_order)
{
    /* Match the producer's explicit Saturn bus order instead of assuming the
     * host order used by the earliest local capture fixtures. */
    uint16_t value = byte_order == NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG
        ? read_be16(entry) : read_le16(entry);
    return UINT32_C(0xff000000) |
        ((uint32_t)expand5(value, 0U) << 16U) |
        ((uint32_t)expand5(value, 5U) << 8U) |
        (uint32_t)expand5(value, 10U);
}

int nexus_v1_vdp2_capture_composite_nbg1_bitmap(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp2CaptureCompositeInput *input,
    Nexus_V1_Vdp2CaptureCompositeReceipt *out_receipt)
{
    Nexus_V1_Vdp2CaptureCompositeReceipt receipt;
    uint16_t bgon;
    uint16_t chctla;
    uint16_t bmpna;
    uint16_t craofa;
    int x;
    int y;

    memset(&receipt, 0, sizeof(receipt));
    if (!out_receipt) return 0;
    if (!framebuffer || !input || !input->capture_bitmap ||
        !input->capture_cram || !input->vdp2_registers ||
        !input->source_bitmap || !input->source_palette ||
        input->capture_bitmap_size != (int)NEXUS_V1_VDP2_NBG1_BITMAP_BYTES ||
        input->source_bitmap_size != (int)NEXUS_V1_VDP2_NBG1_BITMAP_BYTES ||
        input->capture_cram_size != 4096 ||
        input->source_palette_size != (int)NEXUS_V1_VDP2_NBG1_PALETTE_BYTES ||
        input->vdp2_registers_size < (int)NEXUS_V1_VDP2_REGISTERS_BYTES ||
        !input->source_hash_verified ||
        !input->original_saturn_capture_verified ||
        !input->transparent_index_zero_verified ||
        input->source_x < 0 || input->source_y < 0 ||
        input->source_x + input->width > 512 ||
        input->source_y + input->height > 256 || input->width <= 0 ||
        input->height <= 0 || input->destination_x < 0 ||
        input->destination_y < 0 || input->destination_x + input->width > NEXUS_FB_W ||
        input->destination_y + input->height > NEXUS_FB_H ||
        input->palette_base < 0 || input->palette_base > 0) {
        *out_receipt = receipt;
        return 0;
    }
    bgon = read_register16(input->vdp2_registers, 0x20U);
    chctla = read_register16(input->vdp2_registers, 0x28U);
    bmpna = read_register16(input->vdp2_registers, 0x2cU);
    craofa = read_register16(input->vdp2_registers,
                             NEXUS_V1_VDP2_CRAOFA_OFFSET);
    if ((bgon & 0x0002U) == 0U || (chctla & 0x0200U) == 0U ||
        ((chctla >> 10U) & 3U) != 0U || ((chctla >> 12U) & 3U) != 1U ||
        /* NBG1 owns BMPNA bits 8..10; bits 0..2 belong to NBG0. */
        ((bmpna >> 8U) & 0x0007U) != 0U ||
        /* Mednafen maps CRAOFA bits 4..6 to NBG1's 0x100-entry offset. */
        ((craofa >> 4U) & 0x0007U) != 0U ||
        memcmp(input->capture_bitmap, input->source_bitmap,
               NEXUS_V1_VDP2_NBG1_BITMAP_BYTES) != 0 ||
        memcmp(input->capture_cram, input->source_palette,
               NEXUS_V1_VDP2_NBG1_PALETTE_BYTES) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.layer_registers_verified = 1;
    receipt.nbg1_bitmap_mode = 1;
    receipt.colour_code_256 = 1;
    receipt.bitmap_span_join_verified = 1;
    receipt.palette_span_join_verified = 1;
    receipt.original_saturn_capture_verified = 1;
    receipt.source_x = input->source_x;
    receipt.source_y = input->source_y;
    receipt.destination_x = input->destination_x;
    receipt.destination_y = input->destination_y;
    receipt.width = input->width;
    receipt.height = input->height;
    receipt.palette_base = input->palette_base;
    for (x = 0; x < 256; ++x)
        framebuffer->palette[x] = cram_to_rgba(input->source_palette + x * 2);
    for (y = 0; y < input->height; ++y) {
        for (x = 0; x < input->width; ++x) {
            uint8_t index = input->source_bitmap[
                (input->source_y + y) * 512 + input->source_x + x];
            int destination = (input->destination_y + y) * NEXUS_FB_W +
                input->destination_x + x;
            if (index == 0U) {
                ++receipt.transparent_pixels;
                continue;
            }
            framebuffer->color_buffer[destination] = index;
            framebuffer->z_buffer[destination] = 0.0f;
            ++receipt.written_pixels;
        }
    }
    receipt.valid = receipt.written_pixels > 0;
    receipt.renderer_permitted = receipt.valid;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_bitmap(
    Nexus_V1_Vdp2BitmapCaptureFramebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_SaturnVdp2RegisterReceipt *out_register_receipt,
    Nexus_V1_Vdp2BitmapCaptureReceipt *out_receipt)
{
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_SaturnVdp2RegisterReceipt registers;
    Nexus_V1_Vdp2BitmapCaptureReceipt receipt;
    uint16_t bmpna;
    uint16_t craofa;
    uint32_t bitmap_offset;
    int i;

    memset(&frame, 0, sizeof(frame));
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    receipt.renderer_permitted = 0;
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_register_receipt) *out_register_receipt = registers;
    if (out_receipt) *out_receipt = receipt;
    if (!framebuffer || !capture_bytes ||
        !nexus_v1_saturn_runtime_capture_frame(
            capture_bytes, capture_byte_count, frame_index, &frame) ||
        !frame.valid || !frame.vdp2_vram || !frame.vdp2_cram ||
        !frame.vdp2_registers ||
        frame.vdp2_vram_size != NEXUS_V1_SATURN_VDP2_VRAM_BYTES ||
        frame.vdp2_cram_size != NEXUS_V1_SATURN_VDP2_CRAM_BYTES ||
        frame.vdp2_register_size < NEXUS_V1_SATURN_VDP2_REG_BYTES ||
        !nexus_v1_saturn_runtime_capture_vdp2_register_receipt(
            &frame, &registers) || !registers.valid ||
        !registers.nbg1_enabled || !registers.nbg1_bitmap_mode ||
        registers.nbg1_colour_code != 1) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_register_receipt) *out_register_receipt = registers;
        return 0;
    }
    bmpna = read_register16(frame.vdp2_registers, 0x2cU);
    craofa = read_register16(frame.vdp2_registers,
                             NEXUS_V1_VDP2_CRAOFA_OFFSET);
    /* This bounded lane follows the 512x256, 8bpp source geometry used by
     * the authenticated frame analyzer. NBG1 BMPNA and CRAOFA non-zero
     * variants need their own Saturn address proof before admission. */
    if (((registers.chctla >> 10U) & 3U) != 0U ||
        ((registers.chctla >> 12U) & 3U) != 1U ||
        ((bmpna >> 8U) & 7U) != 0U || ((craofa >> 4U) & 7U) != 0U) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_register_receipt) *out_register_receipt = registers;
        return 0;
    }
    bitmap_offset = (((uint32_t)read_register16(
        frame.vdp2_registers, 0x3cU) >> 4U) & 7U) * UINT32_C(0x20000);
    if (bitmap_offset > frame.vdp2_vram_size -
            NEXUS_V1_VDP2_NBG1_BITMAP_BYTES) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_register_receipt) *out_register_receipt = registers;
        return 0;
    }
    receipt.layer_registers_verified = 1;
    receipt.nbg1_bitmap_mode = 1;
    receipt.colour_code_256 = 1;
    receipt.bitmap_span_framed = 1;
    receipt.cram_span_framed = 1;
    receipt.cram_word_order_verified = 1;
    receipt.original_saturn_capture_verified = 1;
    receipt.bitmap_vram_offset = bitmap_offset;
    receipt.cram_offset = 0U;
    for (i = 0; i < (int)(NEXUS_V1_VDP2_NBG1_BITMAP_WIDTH *
                          NEXUS_V1_VDP2_NBG1_BITMAP_HEIGHT); ++i) {
        uint8_t index = frame.vdp2_vram[bitmap_offset + (uint32_t)i];
        if (index == 0U) {
            framebuffer->rgba_buffer[i] = 0U;
            ++receipt.transparent_pixels;
        } else {
            framebuffer->rgba_buffer[i] = cram_to_rgba_ordered(
                frame.vdp2_cram + (size_t)index * 2U, registers.byte_order);
            ++receipt.written_pixels;
        }
    }
    /* A fully transparent captured plane is still a valid VDP2 state/span
     * witness; absence of non-zero pixels must not turn hardware capture into
     * a parse failure. */
    receipt.valid = 1;
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_register_receipt) *out_register_receipt = registers;
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid;
}
