#include "nexus_v1_stabg_capture_compositor.h"

#include "nexus_v1_ui_surfaces.h"

#include <limits.h>
#include <string.h>

#define STABG_WIDTH 320
#define STABG_HEIGHT 168
#define STABG_PALETTE_BYTES 512U

static uint16_t read_le16(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8U);
}

static uint8_t expand5(uint16_t value, unsigned int shift)
{
    uint8_t result = (uint8_t)(((value >> shift) & 0x1fU) << 3U);
    return (uint8_t)(result | (result >> 5U));
}

static uint32_t palette_to_rgba(const uint8_t *entry)
{
    uint16_t value = read_le16(entry);
    return UINT32_C(0xff000000) |
        ((uint32_t)expand5(value, 0U) << 16U) |
        ((uint32_t)expand5(value, 5U) << 8U) |
        (uint32_t)expand5(value, 10U);
}

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int nexus_v1_stabg_capture_composite(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_StabgCaptureInput *input,
    Nexus_V1_StabgCaptureReceipt *out_receipt)
{
    Nexus_V1_StabgCaptureReceipt receipt;
    Nexus_UI_StabgDmwebReceipt dmweb;
    Nexus_UI_StabgPixelDecodeReceipt decoded_receipt;
    uint8_t pixels[STABG_WIDTH * STABG_HEIGHT];
    uint16_t palette_words[256];
    int x;
    int y;

    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    if (!out_receipt) return 0;
    if (!framebuffer || !input || !input->stabg ||
        !input->capture_pixels || !input->capture_palette ||
        input->capture_palette_size != STABG_PALETTE_BYTES ||
        input->capture_width != STABG_WIDTH ||
        input->capture_height != STABG_HEIGHT ||
        input->capture_stride < STABG_WIDTH ||
        input->capture_stride <= 0 ||
        (size_t)input->capture_stride * STABG_HEIGHT >
            input->capture_pixels_size ||
        input->destination_x < 0 || input->destination_y < 0 ||
        input->destination_x > NEXUS_FB_W - STABG_WIDTH ||
        input->destination_y > NEXUS_FB_H - STABG_HEIGHT ||
        input->stabg_size > (size_t)INT_MAX ||
        !input->source_hash_verified ||
        !input->original_saturn_capture_verified ||
        !input->transparent_index_zero_verified ||
        nexus_ui_stabg_dmweb_decode_receipt(input->stabg,
                                             (int)input->stabg_size,
                                             &dmweb) != 0 ||
        nexus_ui_stabg_decode_first_map(input->stabg,
                                        (int)input->stabg_size,
                                        pixels, sizeof(pixels), palette_words,
                                        &decoded_receipt) != 0 ||
        !dmweb.valid || !decoded_receipt.valid ||
        decoded_receipt.width != STABG_WIDTH ||
        decoded_receipt.height != STABG_HEIGHT) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_hash_verified = 1;
    receipt.original_saturn_capture_verified = 1;
    receipt.transparent_index_zero_verified = 1;
    receipt.dmweb_decode_verified = 1;

    for (y = 0; y < STABG_HEIGHT; ++y) {
        if (memcmp(pixels + (size_t)y * STABG_WIDTH,
                   input->capture_pixels + (size_t)y * input->capture_stride,
                   STABG_WIDTH) != 0) {
            *out_receipt = receipt;
            return 0;
        }
    }
    if (memcmp(input->capture_palette,
               input->stabg + dmweb.part2_offset,
               STABG_PALETTE_BYTES) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.pixel_join_verified = 1;
    receipt.palette_join_verified = 1;
    receipt.explicit_placement_verified = 1;
    receipt.destination_x = input->destination_x;
    receipt.destination_y = input->destination_y;
    receipt.width = STABG_WIDTH;
    receipt.height = STABG_HEIGHT;
    receipt.palette_fnv1a64 = fnv1a64(input->capture_palette,
                                      STABG_PALETTE_BYTES);

    for (x = 0; x < 256; ++x)
        framebuffer->palette[x] = palette_to_rgba(
            input->stabg + dmweb.part2_offset + (size_t)x * 2U);
    for (y = 0; y < STABG_HEIGHT; ++y) {
        for (x = 0; x < STABG_WIDTH; ++x) {
            uint8_t index = pixels[y * STABG_WIDTH + x];
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
    receipt.valid = 1;
    receipt.renderer_permitted = 1;
    receipt.vdp2_layer_owner_proven = 0;
    *out_receipt = receipt;
    return 1;
}
