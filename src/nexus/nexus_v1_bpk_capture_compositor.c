#include "nexus_v1_bpk_capture_compositor.h"

#include <stdlib.h>
#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8U) | p[1]);
}

static uint8_t expand5(uint16_t value, unsigned int shift)
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

static int valid_dimensions(const Nexus_V1_BpkSurfaceEntry *surface,
                            const Nexus_V1_BpkCaptureSurfaceInput *input)
{
    if (!surface || !input || surface->width == 0U || surface->height == 0U)
        return 0;
    if (input->capture_width != (int)surface->width ||
        input->capture_height != (int)surface->height ||
        input->capture_stride < input->capture_width ||
        input->capture_stride <= 0)
        return 0;
    if ((size_t)input->capture_stride * (size_t)input->capture_height >
        input->capture_pixels_size)
        return 0;
    if (input->destination_x < 0 || input->destination_y < 0 ||
        input->destination_x + input->capture_width > NEXUS_FB_W ||
        input->destination_y + input->capture_height > NEXUS_FB_H)
        return 0;
    return 1;
}

int nexus_v1_bpk_capture_surface(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_BpkCaptureSurfaceInput *input,
    Nexus_V1_BpkCaptureSurfaceReceipt *out_receipt)
{
    Nexus_V1_BpkCaptureSurfaceReceipt receipt;
    Nexus_V1_BpkSurfaceEntry surface;
    uint8_t *decoded = NULL;
    uint8_t palette_bytes[NEXUS_V1_BPK_PALT_ENTRY_COUNT * 2U];
    uint16_t palette_words[NEXUS_V1_BPK_PALT_ENTRY_COUNT];
    size_t written = 0U;
    uint64_t palette_fnv = 0U;
    int x;
    int y;

    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    if (!out_receipt) return 0;
    if (!framebuffer || !input || !input->archive ||
        !input->capture_pixels || !input->capture_palette ||
        input->capture_palette_size != sizeof(palette_bytes) ||
        !input->bpk_hash_verified ||
        !input->original_saturn_capture_verified ||
        !input->transparent_index_zero_verified) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.bpk_hash_verified = 1;
    receipt.original_saturn_capture_verified = 1;
    receipt.transparent_index_zero_verified = 1;

    /* PRS3 output is bounded by the declared indexed pixel count. Allocate
     * only after the archive decoder has validated that count. */
    {
        Nexus_V1_BpkEntryPrefix prefix;
        if (nexus_v1_bpk_archive_get_entry_prefix(
                input->archive, input->archive_size, input->entry_index,
                &prefix) != 0 || !prefix.prefix_complete ||
            prefix.width == 0U || prefix.height == 0U ||
            (size_t)prefix.width * (size_t)prefix.height > 512U * 256U) {
            *out_receipt = receipt;
            return 0;
        }
        decoded = (uint8_t *)malloc((size_t)prefix.width * prefix.height);
        if (!decoded) {
            *out_receipt = receipt;
            return 0;
        }
    }
    if (nexus_v1_bpk_archive_decode_surface(
            input->archive, input->archive_size, input->entry_index,
            decoded, 512U * 256U, &surface, &written) !=
            NEXUS_V1_BPK_DECODE_OK ||
        surface.layout.surface_class != NEXUS_V1_BPK_SURFACE_INDEXED_8BPP ||
        written != (size_t)surface.width * (size_t)surface.height ||
        !valid_dimensions(&surface, input)) {
        free(decoded);
        *out_receipt = receipt;
        return 0;
    }
    receipt.prs3_pixel_join_verified = 1;

    /* The capture crop is an observed indexed framebuffer region. Join it
     * row-for-row; dimensions alone are not evidence that PRS3 produced the
     * pixels displayed by Saturn. */
    for (y = 0; y < input->capture_height; ++y) {
        if (memcmp(decoded + (size_t)y * input->capture_width,
                   input->capture_pixels + (size_t)y * input->capture_stride,
                   (size_t)input->capture_width) != 0) {
            free(decoded);
            *out_receipt = receipt;
            return 0;
        }
    }

    if (nexus_v1_bpk_archive_copy_palette_words_be16(
            input->archive, input->archive_size, palette_words,
            &palette_fnv) != 0) {
        free(decoded);
        *out_receipt = receipt;
        return 0;
    }
    for (x = 0; x < (int)NEXUS_V1_BPK_PALT_ENTRY_COUNT; ++x) {
        palette_bytes[x * 2] = (uint8_t)(palette_words[x] >> 8U);
        palette_bytes[x * 2 + 1] = (uint8_t)palette_words[x];
    }
    if (memcmp(input->capture_palette, palette_bytes,
               sizeof(palette_bytes)) != 0) {
        free(decoded);
        *out_receipt = receipt;
        return 0;
    }
    receipt.palt_cram_join_verified = 1;
    receipt.explicit_placement_verified = 1;
    receipt.entry_index = input->entry_index;
    receipt.width = surface.width;
    receipt.height = surface.height;
    receipt.destination_x = input->destination_x;
    receipt.destination_y = input->destination_y;
    receipt.palt_fnv1a64 = palette_fnv;

    /* The BPK PALT bytes have joined the captured CRAM bytes exactly. The
     * colour conversion is the same Saturn 5-bit expansion used by the
     * authenticated VDP2 capture compositor. */
    for (x = 0; x < (int)NEXUS_V1_BPK_PALT_ENTRY_COUNT; ++x)
        framebuffer->palette[x] = cram_to_rgba(palette_bytes + x * 2);
    for (y = 0; y < input->capture_height; ++y) {
        for (x = 0; x < input->capture_width; ++x) {
            uint8_t index = decoded[y * input->capture_width + x];
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
    free(decoded);
    receipt.valid = 1;
    receipt.renderer_permitted = 1;
    /* This adapter deliberately does not prove these higher-level owners. */
    receipt.menu_semantics_proven = 0;
    receipt.vdp2_layer_owner_proven = 0;
    *out_receipt = receipt;
    return 1;
}
