#include "nexus_v1_vdp2_capture_compositor.h"
#include "nexus_v1_vdp1_capture_compositor.h"

#include <stdlib.h>
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
    /* Match a raw producer's explicit Saturn bus order.  UNKNOWN is the
     * legacy-fixture compatibility route and historically decoded CRAM as
     * big-endian; defaulting it to little-endian here made the old register
     * fallback render the same source bytes with swapped colour components. */
    uint16_t value = byte_order == NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE
        ? read_le16(entry) : read_be16(entry);
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
    bgon = read_register16_ordered(input->vdp2_registers, 0x20U,
                                   input->register_byte_order);
    chctla = read_register16_ordered(input->vdp2_registers, 0x28U,
                                     input->register_byte_order);
    bmpna = read_register16_ordered(input->vdp2_registers, 0x2cU,
                                    input->register_byte_order);
    craofa = read_register16_ordered(
        input->vdp2_registers, NEXUS_V1_VDP2_CRAOFA_OFFSET,
        input->register_byte_order);
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
        framebuffer->palette[x] = cram_to_rgba_ordered(
            input->source_palette + x * 2, input->register_byte_order);
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
    bmpna = read_register16_ordered(frame.vdp2_registers, 0x2cU,
                                    registers.byte_order);
    craofa = read_register16_ordered(
        frame.vdp2_registers, NEXUS_V1_VDP2_CRAOFA_OFFSET,
        registers.byte_order);
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
    bitmap_offset = (((uint32_t)read_register16_ordered(
        frame.vdp2_registers, 0x3cU, registers.byte_order) >> 4U) & 7U) *
        UINT32_C(0x20000);
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

int nexus_v1_vdp2_capture_decode_runtime_frame_nbg0_bitmap(
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
    int i;

    memset(&frame, 0, sizeof(frame));
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
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
        !nexus_v1_saturn_runtime_capture_vdp2_register_receipt(
            &frame, &registers) || !registers.valid ||
        !registers.nbg0_enabled || !registers.nbg0_bitmap_mode ||
        registers.nbg0_colour_code != 1) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_register_receipt) *out_register_receipt = registers;
        return 0;
    }
    bmpna = read_register16_ordered(frame.vdp2_registers, 0x2cU,
                                    registers.byte_order);
    craofa = read_register16_ordered(
        frame.vdp2_registers, NEXUS_V1_VDP2_CRAOFA_OFFSET,
        registers.byte_order);
    /* The retail title witness has NBG0 at VRAM zero, a 512x256 8bpp plane,
     * BMPNA bank zero and CRAOFA bank zero. Other address/palette variants
     * require their own captured address proof. */
    if ((bmpna & 7U) != 0U || (craofa & 7U) != 0U) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_register_receipt) *out_register_receipt = registers;
        return 0;
    }
    receipt.layer_registers_verified = 1;
    receipt.nbg0_bitmap_mode = 1;
    receipt.colour_code_256 = 1;
    receipt.bitmap_span_framed = 1;
    receipt.cram_span_framed = 1;
    receipt.cram_word_order_verified = 1;
    receipt.original_saturn_capture_verified = 1;
    receipt.bitmap_vram_offset = 0U;
    receipt.cram_offset = 0U;
    for (i = 0; i < (int)(NEXUS_V1_VDP2_NBG0_BITMAP_WIDTH *
                          NEXUS_V1_VDP2_NBG0_BITMAP_HEIGHT); ++i) {
        uint8_t index = frame.vdp2_vram[i];
        if (index == 0U) {
            framebuffer->rgba_buffer[i] = 0U;
            ++receipt.transparent_pixels;
        } else {
            framebuffer->rgba_buffer[i] = cram_to_rgba_ordered(
                frame.vdp2_cram + (size_t)index * 2U, registers.byte_order);
            ++receipt.written_pixels;
        }
    }
    receipt.valid = 1;
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_register_receipt) *out_register_receipt = registers;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

static uint32_t capture_rgb555(uint16_t word)
{
    uint32_t red = ((uint32_t)word & 0x1fU) << 3U;
    uint32_t green = ((uint32_t)word >> 2U) & 0xf8U;
    uint32_t blue = ((uint32_t)word >> 7U) & 0xf8U;
    return UINT32_C(0xff000000) | (red << 16U) | (green << 8U) | blue;
}

static uint32_t capture_blend_555(uint16_t source, uint32_t lower,
                                  unsigned int ratio)
{
    uint32_t source_rgb = capture_rgb555(source);
    unsigned int inverse = 32U - ratio;
    unsigned int red = ((((source_rgb >> 16U) & 0xffU) * ratio) +
                        (((lower >> 16U) & 0xffU) * inverse)) / 32U;
    unsigned int green = ((((source_rgb >> 8U) & 0xffU) * ratio) +
                          (((lower >> 8U) & 0xffU) * inverse)) / 32U;
    unsigned int blue = (((source_rgb & 0xffU) * ratio) +
                         ((lower & 0xffU) * inverse)) / 32U;
    return UINT32_C(0xff000000) | ((uint32_t)red << 16U) |
        ((uint32_t)green << 8U) | blue;
}

int nexus_v1_vdp2_capture_replay_ymir_title_prompt(
    Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *framebuffer,
    const Nexus_V1_Vdp2YmirTitlePromptCaptureInput *input,
    Nexus_V1_Vdp2YmirTitlePromptCaptureReceipt *out_receipt)
{
    Nexus_V1_Vdp2YmirTitlePromptCaptureReceipt receipt;
    const uint8_t *registers;
    unsigned int x;
    unsigned int y;

    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    receipt.width = (int)NEXUS_V1_VDP2_YMIR_MENU_WIDTH;
    receipt.height = (int)NEXUS_V1_VDP2_YMIR_MENU_HEIGHT;
    if (out_receipt) *out_receipt = receipt;
    if (!framebuffer || !input || !input->vdp1_framebuffers ||
        !input->vdp2_vram || !input->vdp2_cram || !input->vdp2_registers ||
        input->vdp1_framebuffers_size !=
            NEXUS_V1_VDP1_YMIR_FRAMEBUFFER_PAIR_BYTES ||
        input->vdp2_vram_size != NEXUS_V1_SATURN_VDP2_VRAM_BYTES ||
        input->vdp2_cram_size != NEXUS_V1_SATURN_VDP2_CRAM_BYTES ||
        input->vdp2_registers_size < 0x120U ||
        input->register_byte_order !=
            NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG ||
        !input->original_ymir_capture_verified ||
        !input->nbg0_over_vdp1_order_verified ||
        memcmp(input->vdp1_framebuffers,
               input->vdp1_framebuffers +
                   NEXUS_V1_VDP1_YMIR_FRAMEBUFFER_BYTES,
               NEXUS_V1_VDP1_YMIR_FRAMEBUFFER_BYTES) != 0) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    registers = input->vdp2_registers;
#define TITLE_REG(offset) read_register16_ordered( \
        registers, (offset), input->register_byte_order)
    if (TITLE_REG(0x00U) != 0x8000U || TITLE_REG(0x0eU) != 0x0300U ||
        TITLE_REG(0x20U) != 0x0001U || TITLE_REG(0x28U) != 0x0013U ||
        TITLE_REG(0x2cU) != 0U || TITLE_REG(0x34U) != 0U ||
        TITLE_REG(0x70U) != 0U || TITLE_REG(0x72U) != 0U ||
        TITLE_REG(0x74U) != 0U || TITLE_REG(0x76U) != 0U ||
        TITLE_REG(NEXUS_V1_VDP2_CRAOFA_OFFSET) != 0U ||
        TITLE_REG(0xe6U) != 0U || TITLE_REG(0xecU) != 0x0043U ||
        TITLE_REG(0xf8U) != 0x0004U || TITLE_REG(0x100U) != 0x1000U) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    receipt.registers_verified = 1;
    receipt.original_ymir_capture_verified = 1;
    receipt.nbg0_over_vdp1_order_verified = 1;
    for (y = 0U; y < NEXUS_V1_VDP2_YMIR_MENU_HEIGHT; ++y) {
        for (x = 0U; x < NEXUS_V1_VDP2_YMIR_MENU_WIDTH; ++x) {
            size_t source_pixel =
                (size_t)y * NEXUS_V1_VDP2_NBG0_BITMAP_WIDTH + x;
            uint8_t index = input->vdp2_vram[source_pixel];
            uint16_t color;
            if (index != 0U) {
                color = read_be16(input->vdp2_cram + (size_t)index * 2U);
                framebuffer->rgba_buffer[
                    (size_t)y * NEXUS_V1_VDP2_YMIR_MENU_WIDTH + x] =
                    capture_rgb555(color);
                ++receipt.nbg0_overlay_pixels;
            } else {
                color = read_be16(input->vdp1_framebuffers +
                                  source_pixel * 2U);
                framebuffer->rgba_buffer[
                    (size_t)y * NEXUS_V1_VDP2_YMIR_MENU_WIDTH + x] =
                    capture_rgb555(color);
                ++receipt.vdp1_background_pixels;
            }
        }
    }
    receipt.valid = 1;
    receipt.renderer_permitted = 0;
    if (out_receipt) *out_receipt = receipt;
#undef TITLE_REG
    return 1;
}

int nexus_v1_vdp2_capture_replay_ymir_title_menu(
    Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *framebuffer,
    const Nexus_V1_Vdp2YmirMenuCaptureInput *input,
    Nexus_V1_Vdp2YmirMenuCaptureReceipt *out_receipt)
{
    Nexus_V1_Vdp2YmirMenuCaptureReceipt receipt;
    const uint8_t *registers;
    uint16_t bgon;
    uint16_t chctla;
    uint16_t bmpna;
    uint16_t craofa;
    uint16_t spctl;
    uint16_t ccctl;
    uint16_t nbg0_priority;
    uint16_t back_upper;
    uint16_t back_lower;
    uint32_t back_base;
    Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *working;
    unsigned int x;
    unsigned int y;
    int command_overlay_requested;
    size_t command_index;

    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    receipt.width = (int)NEXUS_V1_VDP2_YMIR_MENU_WIDTH;
    receipt.height = (int)NEXUS_V1_VDP2_YMIR_MENU_HEIGHT;
    if (out_receipt) *out_receipt = receipt;
    if (!framebuffer || !input || !input->vdp1_display_framebuffer ||
        !input->vdp2_vram || !input->vdp2_cram || !input->vdp2_registers ||
        input->vdp1_display_framebuffer_size !=
            NEXUS_V1_VDP1_YMIR_FRAMEBUFFER_BYTES ||
        input->vdp2_vram_size != NEXUS_V1_SATURN_VDP2_VRAM_BYTES ||
        input->vdp2_cram_size != NEXUS_V1_SATURN_VDP2_CRAM_BYTES ||
        input->vdp2_registers_size < 0x120U ||
        input->register_byte_order ==
            NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_UNKNOWN ||
        !input->original_ymir_capture_verified ||
        !input->command_trace_complete_verified) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    registers = input->vdp2_registers;
    command_overlay_requested = input->vdp1_vram ||
        input->vdp1_vram_size != 0U ||
        input->vdp1_command_byte_offset != 0U ||
        input->vdp1_command_source_join_verified ||
        input->vdp1_command_count != 0U ||
        input->vdp1_command_byte_offsets != NULL ||
        input->vdp1_command_source_join_verified_flags != NULL;
    if (command_overlay_requested &&
        (!input->vdp1_vram ||
         input->vdp1_vram_size != NEXUS_V1_SATURN_VDP1_VRAM_BYTES ||
         input->vdp2_registers_size <
             NEXUS_V1_SATURN_VDP2_REG_BYTES ||
         ((input->vdp1_command_count == 0U &&
           (input->vdp1_command_byte_offsets != NULL ||
            input->vdp1_command_source_join_verified_flags != NULL)) ||
          (input->vdp1_command_count > 0U &&
           (!input->vdp1_command_byte_offsets ||
            !input->vdp1_command_source_join_verified_flags ||
            input->vdp1_command_count >
                NEXUS_V1_VDP2_YMIR_MENU_MAX_VDP1_COMMANDS))))) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (command_overlay_requested) {
        size_t count = input->vdp1_command_count ?
            input->vdp1_command_count : 1U;
        for (command_index = 0U; command_index < count; ++command_index) {
            uint32_t offset = input->vdp1_command_count ?
                input->vdp1_command_byte_offsets[command_index] :
                input->vdp1_command_byte_offset;
            int join_verified = input->vdp1_command_count ?
                input->vdp1_command_source_join_verified_flags[command_index] != 0U :
                input->vdp1_command_source_join_verified;
            size_t prior;
            if (offset % NEXUS_V1_VDP1_COMMAND_BYTES != 0U ||
                offset > input->vdp1_vram_size -
                    NEXUS_V1_VDP1_COMMAND_BYTES || !join_verified) {
                if (out_receipt) *out_receipt = receipt;
                return 0;
            }
            for (prior = 0U; prior < command_index; ++prior) {
                uint32_t previous = input->vdp1_command_count ?
                    input->vdp1_command_byte_offsets[prior] :
                    input->vdp1_command_byte_offset;
                if (offset == previous) {
                    if (out_receipt) *out_receipt = receipt;
                    return 0;
                }
            }
        }
    }
#define CAPTURE_REG(offset) read_register16_ordered( \
        registers, (offset), input->register_byte_order)
    bgon = CAPTURE_REG(0x20U);
    chctla = CAPTURE_REG(0x28U);
    bmpna = CAPTURE_REG(0x2cU);
    craofa = CAPTURE_REG(0xe4U);
    spctl = CAPTURE_REG(0xe0U);
    ccctl = CAPTURE_REG(0xecU);
    nbg0_priority = CAPTURE_REG(0xf8U);
    back_upper = CAPTURE_REG(0xacU);
    back_lower = CAPTURE_REG(0xaeU);
    back_base = ((uint32_t)(back_upper & 7U) << 17U) |
        ((uint32_t)back_lower << 1U);
    if (CAPTURE_REG(0x00U) != 0x8000U || CAPTURE_REG(0x0eU) != 0x0300U ||
        bgon != 0x0001U ||
        chctla != 0x0013U || bmpna != 0U || CAPTURE_REG(0x3cU) != 0U ||
        craofa != 0U || CAPTURE_REG(0x70U) != 0U ||
        CAPTURE_REG(0x72U) != 0U || CAPTURE_REG(0x74U) != 0U ||
        CAPTURE_REG(0x76U) != 0U || CAPTURE_REG(0x78U) != 1U ||
        CAPTURE_REG(0x7aU) != 0U || CAPTURE_REG(0x7cU) != 1U ||
        CAPTURE_REG(0x7eU) != 0U || CAPTURE_REG(0xe8U) != 0U ||
        spctl != 0x1536U || ccctl != 0x0043U ||
        CAPTURE_REG(0xf0U) != 0x0102U ||
        CAPTURE_REG(0xf2U) != 0x0300U ||
        CAPTURE_REG(0xf4U) != 0x0504U ||
        CAPTURE_REG(0xf6U) != 0x0706U ||
        (nbg0_priority & 7U) != 4U ||
        CAPTURE_REG(0x100U) != 0x1000U ||
        CAPTURE_REG(0x108U) != 0x1000U ||
        back_upper != 0x8003U || back_lower != 0xf800U ||
        back_base != 0x7f000U ||
        back_base + NEXUS_V1_VDP2_YMIR_MENU_HEIGHT * 2U >
            input->vdp2_vram_size) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    working = (Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *)
        malloc(sizeof(*working));
    if (!working) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.registers_verified = 1;
    receipt.original_ymir_capture_verified = 1;
    receipt.command_trace_complete_verified = 1;

    for (y = 0; y < NEXUS_V1_VDP2_YMIR_MENU_HEIGHT; ++y) {
        uint16_t back = read_be16(input->vdp2_vram + back_base + y * 2U);
        if (back != 0U) {
            free(working);
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        for (x = 0; x < NEXUS_V1_VDP2_YMIR_MENU_WIDTH; ++x) {
            size_t source_pixel =
                (size_t)y * NEXUS_V1_VDP2_NBG0_BITMAP_WIDTH + x;
            size_t output_pixel =
                (size_t)y * NEXUS_V1_VDP2_YMIR_MENU_WIDTH + x;
            uint16_t vdp1 = read_be16(
                input->vdp1_display_framebuffer + source_pixel * 2U);
            uint8_t nbg0 = input->vdp2_vram[source_pixel];
            uint32_t lower;

            if ((vdp1 & 0x8000U) != 0U) {
                lower = capture_rgb555(vdp1);
                ++receipt.vdp1_direct_pixels;
            } else if (vdp1 != 0U) {
                unsigned int priority = (vdp1 >> 12U) & 7U;
                unsigned int ratio_index = (vdp1 >> 10U) & 3U;
                uint16_t priority_reg;
                uint16_t ratio_reg;
                unsigned int ratio;
                uint16_t color;
                if (priority != ((spctl >> 8U) & 7U) || ratio_index != 0U ||
                    nbg0 != 0U || (ccctl & 0x0040U) == 0U) {
                    free(working);
                    if (out_receipt) *out_receipt = receipt;
                    return 0;
                }
                priority_reg = CAPTURE_REG(0xf4U);
                ratio_reg = CAPTURE_REG(0x100U);
                if (((priority_reg >> 8U) & 7U) != priority ||
                    ((ccctl & 0x0700U) != 0x0000U)) {
                    free(working);
                    if (out_receipt) *out_receipt = receipt;
                    return 0;
                }
                ratio = 31U - (ratio_reg & 31U);
                color = read_be16(input->vdp2_cram +
                    (size_t)(vdp1 & 0x03ffU) * 2U);
                working->rgba_buffer[output_pixel] =
                    capture_blend_555(color, 0U, ratio);
                ++receipt.vdp1_palette_pixels;
                continue;
            } else {
                lower = UINT32_C(0xff000000);
            }

            if (nbg0 != 0U) {
                uint16_t color = read_be16(input->vdp2_cram +
                    (size_t)nbg0 * 2U);
                unsigned int ratio = 31U -
                    (CAPTURE_REG(0x108U) & 31U);
                working->rgba_buffer[output_pixel] =
                    capture_blend_555(color, lower, ratio);
                ++receipt.nbg0_overlay_pixels;
            } else {
                working->rgba_buffer[output_pixel] = lower;
            }
        }
    }
    if (command_overlay_requested) {
        uint8_t cram_little_endian[NEXUS_V1_SATURN_VDP2_CRAM_BYTES];
        uint8_t registers_little_endian[NEXUS_V1_SATURN_VDP2_REG_BYTES];
        Nexus_V1_Vdp2ColourRamCapture colour_capture;
        Nexus_V1_Vdp1DirectColorFramebuffer *vdp1_framebuffer;
        size_t byte;
        int total_written_pixels = 0;
        vdp1_framebuffer = (Nexus_V1_Vdp1DirectColorFramebuffer *)
            malloc(sizeof(*vdp1_framebuffer));
        if (!vdp1_framebuffer) {
            free(working);
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        for (byte = 0U; byte < sizeof(cram_little_endian); byte += 2U) {
            cram_little_endian[byte] = input->vdp2_cram[byte + 1U];
            cram_little_endian[byte + 1U] = input->vdp2_cram[byte];
        }
        for (byte = 0U; byte < sizeof(registers_little_endian); byte += 2U) {
            if (input->register_byte_order ==
                    NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG) {
                registers_little_endian[byte] = registers[byte + 1U];
                registers_little_endian[byte + 1U] = registers[byte];
            } else {
                registers_little_endian[byte] = registers[byte];
                registers_little_endian[byte + 1U] = registers[byte + 1U];
            }
        }
        memset(&colour_capture, 0, sizeof(colour_capture));
        colour_capture.colour_ram = cram_little_endian;
        colour_capture.colour_ram_size = sizeof(cram_little_endian);
        colour_capture.registers = registers_little_endian;
        colour_capture.registers_size = sizeof(registers_little_endian);
        colour_capture.original_saturn_capture_verified = 1;
        memcpy(vdp1_framebuffer->rgba_buffer, working->rgba_buffer,
               sizeof(vdp1_framebuffer->rgba_buffer));
        {
            size_t count = input->vdp1_command_count ?
                input->vdp1_command_count : 1U;
            for (command_index = 0U; command_index < count; ++command_index) {
                uint32_t offset = input->vdp1_command_count ?
                    input->vdp1_command_byte_offsets[command_index] :
                    input->vdp1_command_byte_offset;
                int written_pixels = 0;
                if (!nexus_v1_vdp1_capture_blend_mode0_normal(
                        vdp1_framebuffer,
                        input->vdp1_vram + offset,
                        (int)NEXUS_V1_VDP1_COMMAND_BYTES,
                        input->vdp1_vram, (int)input->vdp1_vram_size,
                        &colour_capture, 0, 0,
                        (int)NEXUS_V1_VDP2_YMIR_MENU_WIDTH,
                        (int)NEXUS_V1_VDP2_YMIR_MENU_HEIGHT,
                        &written_pixels) || written_pixels <= 0) {
                    free(vdp1_framebuffer);
                    free(working);
                    if (out_receipt) *out_receipt = receipt;
                    return 0;
                }
                total_written_pixels += written_pixels;
            }
        }
        memcpy(working->rgba_buffer, vdp1_framebuffer->rgba_buffer,
               sizeof(working->rgba_buffer));
        free(vdp1_framebuffer);
        receipt.vdp1_command_overlaid = 1;
        receipt.vdp1_command_source_join_verified = 1;
        receipt.vdp1_command_count = (int)(input->vdp1_command_count ?
            input->vdp1_command_count : 1U);
        receipt.vdp1_command_pixels = total_written_pixels;
    }
    receipt.valid = 1;
    receipt.renderer_permitted = 0;
    memcpy(framebuffer->rgba_buffer, working->rgba_buffer,
           sizeof(framebuffer->rgba_buffer));
    free(working);
    if (out_receipt) *out_receipt = receipt;
#undef CAPTURE_REG
    return 1;
}
