#include "dm2_v1_mac_sound.h"

#include <string.h>

static uint16_t be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

typedef struct {
    const uint8_t *payload;
    size_t payload_size;
    int16_t resource_id;
} MacSndResource;

static int resource_map(const uint8_t *fork, size_t size,
                        size_t *data_offset, size_t *data_length,
                        size_t *map_offset, size_t *map_length,
                        size_t *type_list, size_t *ref_list,
                        uint16_t *ref_count) {
    uint16_t type_offset, type_count;
    size_t i;
    if (!fork || size < 256u || !data_offset || !data_length ||
        !map_offset || !map_length || !type_list || !ref_list || !ref_count)
        return -1;
    *data_offset = be32(fork);
    *map_offset = be32(fork + 4u);
    *data_length = be32(fork + 8u);
    *map_length = be32(fork + 12u);
    if (*data_offset > size || *data_length > size - *data_offset ||
        *map_offset > size || *map_length > size - *map_offset ||
        *map_length < 30u) return -1;
    type_offset = be16(fork + *map_offset + 24u);
    if ((size_t)type_offset + 2u > *map_length) return -1;
    *type_list = *map_offset + type_offset;
    type_count = (uint16_t)(be16(fork + *type_list) + 1u);
    if (type_count == 0u || type_count > 4096u ||
        type_count > (*map_length - type_offset - 2u) / 8u) return -1;
    for (i = 0u; i < type_count; ++i) {
        const uint8_t *entry = fork + *type_list + 2u + i * 8u;
        uint16_t count;
        if (memcmp(entry, "snd ", 4u) != 0) continue;
        count = (uint16_t)(be16(entry + 4u) + 1u);
        if (count == 0u || count > 4096u ||
            be16(entry + 6u) > *map_length - type_offset ||
            count > (*map_length - type_offset - be16(entry + 6u)) / 12u)
            return -1;
        *ref_list = *type_list + be16(entry + 6u);
        *ref_count = count;
        return 0;
    }
    return -1;
}

static int get_resource(const uint8_t *fork, size_t size, size_t index,
                        MacSndResource *out) {
    size_t data_offset, data_length, map_offset, map_length;
    size_t type_list, ref_list;
    uint16_t ref_count;
    const uint8_t *ref;
    uint32_t offset, length;
    if (!out || resource_map(fork, size, &data_offset, &data_length,
                             &map_offset, &map_length, &type_list,
                             &ref_list, &ref_count) != 0 ||
        index >= ref_count) return -1;
    ref = fork + ref_list + index * 12u;
    offset = ((uint32_t)ref[5] << 16) | ((uint32_t)ref[6] << 8) | ref[7];
    if (offset > data_length || data_length - offset < 4u) return -1;
    length = be32(fork + data_offset + offset);
    if (length > data_length - offset - 4u) return -1;
    out->resource_id = (int16_t)be16(ref);
    out->payload = fork + data_offset + offset + 4u;
    out->payload_size = length;
    return 0;
}

static int parse_sample(const MacSndResource *resource,
                        DM2_V1_MacSoundSample *out) {
    const uint8_t *p;
    uint16_t format, data_formats, data_type, command_count;
    uint32_t init_option, header_offset, sample_length;
    size_t commands_offset, i, header;
    if (!resource || !out || resource->payload_size < 12u) return -1;
    p = resource->payload;
    format = be16(p);
    if (format != 1u) return -1;
    data_formats = be16(p + 2u);
    if (data_formats == 0u || data_formats > 16u ||
        4u + (size_t)data_formats * 6u + 2u > resource->payload_size)
        return -1;
    data_type = be16(p + 4u);
    init_option = be32(p + 6u);
    commands_offset = 4u + (size_t)data_formats * 6u;
    command_count = be16(p + commands_offset);
    if (command_count == 0u || command_count > 256u ||
        commands_offset + 2u + (size_t)command_count * 8u > resource->payload_size)
        return -1;
    header = 0u;
    for (i = 0u; i < command_count; ++i) {
        const uint8_t *command = p + commands_offset + 2u + i * 8u;
        if (be16(command) == 0x8051u) {
            header_offset = be32(command + 4u);
            header = (size_t)header_offset;
            break;
        }
    }
    if (header == 0u || header > resource->payload_size ||
        resource->payload_size - header < 22u) return -1;
    p += header;
    sample_length = be32(p + 4u);
    if (sample_length > resource->payload_size - header - 22u) return -1;
    memset(out, 0, sizeof(*out));
    out->valid = 1;
    out->resource_id = resource->resource_id;
    out->format = format;
    out->data_type = data_type;
    out->init_option = init_option;
    out->sample_rate_fixed = be32(p + 8u);
    out->sample_length = sample_length;
    out->loop_start = be32(p + 12u);
    out->loop_end = be32(p + 16u);
    out->encode = p[20];
    out->base_frequency = p[21];
    out->sample_data = p + 22u;
    out->sample_data_size = sample_length;
    return 0;
}

size_t dm2_v1_mac_sound_count(const uint8_t *fork, size_t fork_size) {
    size_t data_offset, data_length, map_offset, map_length;
    size_t type_list, ref_list;
    uint16_t ref_count;
    return resource_map(fork, fork_size, &data_offset, &data_length,
                        &map_offset, &map_length, &type_list, &ref_list,
                        &ref_count) == 0 ? ref_count : 0u;
}

int dm2_v1_mac_sound_find(const uint8_t *fork, size_t fork_size,
                          int16_t resource_id,
                          DM2_V1_MacSoundSample *out) {
    size_t count = dm2_v1_mac_sound_count(fork, fork_size);
    size_t i;
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    for (i = 0u; i < count; ++i) {
        MacSndResource resource;
        if (get_resource(fork, fork_size, i, &resource) == 0 &&
            resource.resource_id == resource_id)
            return parse_sample(&resource, out);
    }
    return -1;
}
