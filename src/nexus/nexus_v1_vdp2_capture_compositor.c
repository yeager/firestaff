#include "nexus_v1_vdp2_capture_compositor.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8U) | p[1]);
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

int nexus_v1_vdp2_capture_composite_nbg1_bitmap(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp2CaptureCompositeInput *input,
    Nexus_V1_Vdp2CaptureCompositeReceipt *out_receipt)
{
    Nexus_V1_Vdp2CaptureCompositeReceipt receipt;
    uint16_t bgon;
    uint16_t chctla;
    uint16_t bmpna;
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
    bgon = read_be16(input->vdp2_registers + 0x20U);
    chctla = read_be16(input->vdp2_registers + 0x28U);
    bmpna = read_be16(input->vdp2_registers + 0x2cU);
    if ((bgon & 0x0002U) == 0U || (chctla & 0x0200U) == 0U ||
        ((chctla >> 10U) & 3U) != 0U || ((chctla >> 12U) & 3U) != 1U ||
        /* NBG1 owns BMPNA bits 8..10; bits 0..2 belong to NBG0. */
        ((bmpna >> 8U) & 0x0007U) != 0U ||
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
