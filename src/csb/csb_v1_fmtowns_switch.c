#include "csb_v1_fmtowns_switch.h"

#include <string.h>

/* ReDMCSB SWITCHDA.C, MEDIA670_F31E_F31J data declarations. These are the
 * compressed stream lengths in the retail executable, not replacement art. */
static const size_t k_switch_resource_bytes[6] = {
    499u, 315u, 698u, 304u, 108u, 7314u
};
static const uint16_t k_switch_widths[6] = {
    51u, 42u, 62u, 55u, 55u, 320u
};
static const uint16_t k_switch_heights[6] = {
    38u, 40u, 39u, 31u, 31u, 200u
};
static const uint16_t k_switch_button_x[CSB_FMTOWNS_SWITCH_BUTTON_COUNT] = {
    52u, 57u, 47u, 50u
};
static const uint16_t k_switch_button_y[CSB_FMTOWNS_SWITCH_BUTTON_COUNT] = {
    15u, 59u, 105u, 150u
};

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)(bytes[0] | ((uint16_t)bytes[1] << 8u));
}

static uint32_t fnv1a32(const uint8_t *bytes, size_t byte_count)
{
    uint32_t hash = 2166136261u;
    size_t index;
    for (index = 0u; index < byte_count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static int aligned_after(const uint8_t *bytes, size_t byte_count,
                         size_t offset, size_t resource_bytes,
                         uint16_t expected_width, uint16_t expected_height,
                         size_t *out_offset)
{
    size_t padding;
    if (!bytes || !out_offset || offset > byte_count ||
        resource_bytes > byte_count - offset) return 0;
    offset += resource_bytes;
    for (padding = 0u; padding <= 3u && offset + padding + 4u <= byte_count;
         ++padding) {
        if (padding != 0u && bytes[offset + padding - 1u] != 0u) break;
        if (read_le16(bytes + offset + padding) == expected_width &&
            read_le16(bytes + offset + padding + 2u) == expected_height) {
            *out_offset = offset + padding;
            return 1;
        }
    }
    return 0;
}

static int decode_resource(const uint8_t *bytes, size_t byte_count,
                           size_t offset, size_t resource_bytes,
                           uint16_t width, uint16_t height,
                           uint8_t *pixels, size_t pixel_capacity,
                           CSB_V1_FmtownsItemDecodeReceipt *out)
{
    if (!bytes || offset > byte_count || resource_bytes > byte_count - offset ||
        !pixels || pixel_capacity < (size_t)width * height) return 0;
    if (!csb_v1_fmtowns_img2_decode(bytes + offset, resource_bytes,
                                    width, height, pixels, pixel_capacity, out))
        return 0;
    return out && out->stream_bytes_consumed == resource_bytes &&
           out->pixel_count == (size_t)width * height;
}

int csb_v1_fmtowns_switch_parse(const uint8_t *executable,
                                size_t executable_size,
                                CSB_V1_FmtownsSwitchReceipt *out)
{
    size_t candidate;
    uint8_t pixels[CSB_FMTOWNS_SWITCH_PIXELS];

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!executable || executable_size < 16000u) return 0;

    /* The P3 executable contains unrelated 320x200 streams. Require the
     * exact F2279 registration sequence immediately preceding G4167/G4168. */
    for (candidate = 0u; candidate + k_switch_resource_bytes[0] < executable_size;
         ++candidate) {
        size_t offsets[7];
        size_t index;
        CSB_V1_FmtownsItemDecodeReceipt decoded[7];
        int matches = 1;

        if (read_le16(executable + candidate) != k_switch_widths[0] ||
            read_le16(executable + candidate + 2u) != k_switch_heights[0])
            continue;
        offsets[0] = candidate;
        for (index = 0u; index < 6u; ++index) {
            size_t next;
            if (read_le16(executable + offsets[index]) != k_switch_widths[index] ||
                read_le16(executable + offsets[index] + 2u) != k_switch_heights[index] ||
                !decode_resource(executable, executable_size, offsets[index],
                                 k_switch_resource_bytes[index],
                                 k_switch_widths[index], k_switch_heights[index],
                                 pixels, sizeof(pixels), &decoded[index]) ||
                !aligned_after(executable, executable_size, offsets[index],
                               k_switch_resource_bytes[index],
                               index == 5u ? 320u : k_switch_widths[index + 1u],
                               index == 5u ? 200u : k_switch_heights[index + 1u],
                               &next)) {
                matches = 0;
                break;
            }
            offsets[index + 1u] = next;
        }
        if (!matches || read_le16(executable + offsets[6]) != 320u ||
            read_le16(executable + offsets[6] + 2u) != 200u ||
            !decode_resource(executable, executable_size, offsets[6], 6541u,
                             320u, 200u, pixels, sizeof(pixels), &decoded[6]))
            continue;

        out->valid = 1;
        out->executable_fnv1a = fnv1a32(executable, executable_size);
        out->japanese_page_offset = offsets[5];
        out->japanese_page_byte_count = k_switch_resource_bytes[5];
        out->english_page_offset = offsets[6];
        out->english_page_byte_count = 6541u;
        out->japanese_page = decoded[5];
        out->english_page = decoded[6];
        for (index = 0u; index < CSB_FMTOWNS_SWITCH_BUTTON_COUNT; ++index) {
            out->buttons[index].x = k_switch_button_x[index];
            out->buttons[index].y = k_switch_button_y[index];
            out->buttons[index].width = k_switch_widths[index];
            out->buttons[index].height = k_switch_heights[index];
            out->buttons[index].source_offset = offsets[index];
            out->buttons[index].source_byte_count = k_switch_resource_bytes[index];
            out->buttons[index].image = decoded[index];
        }
        return 1;
    }
    return 0;
}

int csb_v1_fmtowns_switch_decode_page(const uint8_t *executable,
                                      size_t executable_size,
                                      const CSB_V1_FmtownsSwitchReceipt *receipt,
                                      CSB_V1_FmtownsSwitchLanguage language,
                                      uint8_t *out_pixels,
                                      size_t out_pixel_capacity,
                                      CSB_V1_FmtownsItemDecodeReceipt *out)
{
    size_t offset;
    size_t byte_count;
    if (!receipt || !receipt->valid ||
        (language != CSB_FMTOWNS_SWITCH_JAPANESE &&
         language != CSB_FMTOWNS_SWITCH_ENGLISH)) return 0;
    offset = language == CSB_FMTOWNS_SWITCH_JAPANESE
                 ? receipt->japanese_page_offset : receipt->english_page_offset;
    byte_count = language == CSB_FMTOWNS_SWITCH_JAPANESE
                     ? receipt->japanese_page_byte_count : receipt->english_page_byte_count;
    return decode_resource(executable, executable_size, offset, byte_count,
                           CSB_FMTOWNS_SWITCH_WIDTH, CSB_FMTOWNS_SWITCH_HEIGHT,
                           out_pixels, out_pixel_capacity, out);
}
