#include "csb_v1_atari_switch_dat.h"

#include <string.h>

#define FTL_HEADER_BYTES 20u
#define FTL_SEGMENT_BYTES 12u
#define SWITCH_IMAGE_BYTES 52u
#define FTL_MAX_SEGMENTS 100u

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8u) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24u) | ((uint32_t)bytes[1] << 16u) |
           ((uint32_t)bytes[2] << 8u) | bytes[3];
}

static int has_span(size_t byte_count, size_t offset, size_t length)
{
    return offset <= byte_count && length <= byte_count - offset;
}

static uint16_t header_checksum(const uint8_t *header, const uint8_t *segments,
                                size_t segment_bytes)
{
    uint32_t sum = 0u;
    size_t index;
    for (index = 4u; index < FTL_HEADER_BYTES; ++index)
        sum += (uint32_t)header[index] * index;
    for (index = 0u; index < segment_bytes; ++index)
        sum += (uint32_t)segments[index] * ((index & 0xffu) + 1u);
    return (uint16_t)sum;
}

static int find_segment(const uint8_t *bytes, size_t byte_count,
                        const uint8_t *segments, uint16_t segment_count,
                        uint16_t type, uint16_t id, size_t *out_offset,
                        size_t *out_size)
{
    size_t index;
    for (index = 0u; index < segment_count; ++index) {
        const uint8_t *segment = segments + index * FTL_SEGMENT_BYTES;
        size_t offset;
        size_t size;
        if (read_be16(segment) != type || read_be16(segment + 2u) != id)
            continue;
        offset = (size_t)read_be32(segment + 4u);
        size = (size_t)read_be32(segment + 8u);
        if (!has_span(byte_count, offset, size)) return 0;
        *out_offset = offset;
        *out_size = size;
        return 1;
    }
    (void)bytes;
    return 0;
}

int csb_v1_atari_switch_dat_parse(const uint8_t *bytes, size_t byte_count,
                                  CSB_V1_AtariSwitchDatReceipt *out)
{
    const uint8_t *segments;
    const uint8_t *switch_data;
    size_t segment_bytes;
    size_t switch_offset;
    size_t switch_size;
    uint16_t segment_count;
    uint16_t option_count;
    size_t index;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!bytes || byte_count < FTL_HEADER_BYTES ||
        read_be16(bytes) != 0x6160u)
        return 0;
    segment_count = read_be16(bytes + 18u);
    if (segment_count > FTL_MAX_SEGMENTS ||
        !has_span(byte_count, FTL_HEADER_BYTES,
                  (size_t)segment_count * FTL_SEGMENT_BYTES))
        return 0;
    segments = bytes + FTL_HEADER_BYTES;
    segment_bytes = (size_t)segment_count * FTL_SEGMENT_BYTES;
    if (header_checksum(bytes, segments, segment_bytes) != read_be16(bytes + 2u) ||
        read_be16(bytes + 4u) != 0x5000u ||
        !find_segment(bytes, byte_count, segments, segment_count, 0x30u, 0u,
                      &switch_offset, &switch_size) || switch_size < 2u)
        return 0;
    switch_data = bytes + switch_offset;
    option_count = read_be16(switch_data);
    if (option_count > CSB_V1_ATARI_SWITCH_OPTION_LIMIT ||
        !has_span(switch_size, 2u, (size_t)option_count * SWITCH_IMAGE_BYTES))
        return 0;

    out->header_segment_count = segment_count;
    out->option_count = option_count;
    for (index = 0u; index < option_count; ++index) {
        const uint8_t *image = switch_data + 2u + index * SWITCH_IMAGE_BYTES;
        CSB_V1_AtariSwitchOption *option = &out->options[index];
        size_t graphic_offset;
        size_t graphic_size;
        size_t name_length;
        option->segment_type = read_be16(image);
        option->segment_id = read_be16(image + 2u);
        option->x = (int16_t)read_be16(image + 4u);
        option->y = (int16_t)read_be16(image + 6u);
        option->transparent_color = (int16_t)read_be16(image + 8u);
        memcpy(option->ftl_file_name, image + 10u, 42u);
        option->ftl_file_name[42] = '\0';
        name_length = strnlen(option->ftl_file_name, 42u);
        if (name_length == 42u) return 0;
        if (option->segment_type == 0u) continue;
        if (!find_segment(bytes, byte_count, segments, segment_count,
                          option->segment_type, option->segment_id,
                          &graphic_offset, &graphic_size) || graphic_size < 4u)
            return 0;
        option->pixel_width = read_be16(bytes + graphic_offset);
        option->pixel_height = read_be16(bytes + graphic_offset + 2u);
        if (option->pixel_width == 0u || option->pixel_height == 0u) return 0;
        option->graphic_offset = graphic_offset;
        option->graphic_byte_count = graphic_size;
        option->enabled = 1;
    }
    {
        size_t palette_offset;
        size_t palette_size;
        if (find_segment(bytes, byte_count, segments, segment_count, 0x30u, 1u,
                         &palette_offset, &palette_size) &&
            palette_size >= CSB_V1_ATARI_SWITCH_PALETTE_BYTES) {
            memcpy(out->palette, bytes + palette_offset,
                   CSB_V1_ATARI_SWITCH_PALETTE_BYTES);
            out->has_palette = 1;
        }
    }
    out->valid = 1;
    return 1;
}
