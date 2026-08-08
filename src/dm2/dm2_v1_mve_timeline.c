#include "dm2_v1_mve_timeline.h"

#include "dm2_v1_mve_stream.h"

#include <string.h>

static uint16_t timeline_le16(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8u));
}

static int timeline_find_header(const uint8_t *bytes, size_t byte_count,
                                size_t *out_offset)
{
    static const uint8_t magic[] = {
        'I','n','t','e','r','p','l','a','y',' ','M','V','E',' ',
        'F','i','l','e',0x1a,0x00,0x1a,0x00,0x00,0x01,0x33,0x11
    };
    size_t at;
    if (!bytes || !out_offset || byte_count < sizeof(magic)) return 0;
    for (at = 0u; at <= byte_count - sizeof(magic); ++at) {
        if (memcmp(bytes + at, magic, sizeof(magic)) == 0) {
            *out_offset = at;
            return 1;
        }
    }
    return 0;
}

static int timeline_known_opcode(uint8_t opcode, uint8_t version, uint16_t size)
{
    switch (opcode) {
    case 0x00u: case 0x01u: return version == 0u && size == 0u;
    case 0x02u: return version == 0u && size == 6u;
    case 0x03u: return version == 0u && size == 8u;
    case 0x04u: return version == 0u && size == 0u;
    case 0x05u: return version == 2u && size == 8u;
    case 0x07u: return version == 1u && size == 6u;
    case 0x08u: return version == 0u && size >= 6u;
    case 0x09u: case 0x0au: return version == 0u && size == 6u;
    case 0x0cu: return version == 0u && size == 766u;
    case 0x0fu: return version == 0u && size == 500u;
    case 0x11u: return version == 3u && size > 0u;
    case 0x13u: return version == 0u && size == 132u;
    default: return 0;
    }
}

static int timeline_scan(const uint8_t *bytes, size_t byte_count,
                         DM2_V1_MveDisplayAudioBoundary *boundaries,
                         size_t capacity, DM2_V1_MveTimelineReceipt *receipt)
{
    DM2_V1_MveStreamReceipt stream;
    size_t offset;
    uint32_t audio_sequence = 0u;
    uint32_t pending_first = 0u;
    uint32_t pending_count = 0u;
    uint64_t pending_bytes = 0u;
    uint64_t pending_frames = 0u;
    unsigned int terminal_silent = 0u;
    int audio_format_seen = 0;
    int ended = 0;

    memset(receipt, 0, sizeof(*receipt));
    if (!dm2_v1_mve_stream_parse(bytes, byte_count, &stream) || !stream.valid ||
        !timeline_find_header(bytes, byte_count, &offset) ||
        offset > byte_count - 26u)
        return 0;
    offset += 26u;
    receipt->timer_rate_us = stream.timer_rate_us;
    receipt->timer_subdivision = stream.timer_subdivision;
    while (offset < byte_count) {
        uint16_t chunk_size, chunk_type;
        size_t chunk_end;
        if (byte_count - offset < 4u) return 0;
        chunk_size = timeline_le16(bytes + offset);
        chunk_type = timeline_le16(bytes + offset + 2u);
        offset += 4u;
        if (chunk_size > byte_count - offset) return 0;
        if (chunk_type == 5u) {
            if (chunk_size != 0u || !ended) return 0;
            break;
        }
        chunk_end = offset + chunk_size;
        while (offset < chunk_end) {
            uint16_t size;
            uint8_t opcode, version;
            const uint8_t *data;
            if (chunk_end - offset < 4u) return 0;
            size = timeline_le16(bytes + offset);
            opcode = bytes[offset + 2u];
            version = bytes[offset + 3u];
            offset += 4u;
            if (size > chunk_end - offset ||
                !timeline_known_opcode(opcode, version, size)) return 0;
            data = bytes + offset;
            offset += size;
            if (opcode == 0x03u) {
                const uint16_t flags = timeline_le16(data + 2u);
                if (audio_format_seen || timeline_le16(data + 4u) != 22050u ||
                    flags != 1u) return 0;
                audio_format_seen = 1;
            } else if (opcode == 0x08u) {
                const uint16_t sequence = timeline_le16(data);
                const uint16_t payload_bytes = timeline_le16(data + 4u);
                if (!audio_format_seen || sequence != audio_sequence ||
                    timeline_le16(data + 2u) != 1u ||
                    payload_bytes != (uint16_t)(size - 6u) ||
                    (payload_bytes & 1u) != 0u) return 0;
                if (pending_count == 0u) pending_first = sequence;
                ++pending_count;
                ++audio_sequence;
                pending_bytes += payload_bytes;
                pending_frames += payload_bytes / 2u;
                receipt->audio_sample_bytes += payload_bytes;
                receipt->audio_sample_frames += payload_bytes / 2u;
            } else if (opcode == 0x07u) {
                DM2_V1_MveDisplayAudioBoundary boundary;
                if (receipt->presentation_count >= capacity && boundaries) return 0;
                memset(&boundary, 0, sizeof(boundary));
                boundary.valid = 1;
                boundary.presentation_index = receipt->presentation_count;
                boundary.presentation_time_us =
                    (uint64_t)boundary.presentation_index * stream.timer_rate_us *
                    stream.timer_subdivision;
                boundary.first_audio_source_sequence = pending_first;
                boundary.audio_packet_count = pending_count;
                boundary.audio_sample_bytes = pending_bytes;
                boundary.audio_sample_frames = pending_frames;
                if (boundaries) boundaries[receipt->presentation_count] = boundary;
                if (boundary.presentation_index == 0u)
                    receipt->prebuffer_audio_packet_count = pending_count;
                if (pending_count == 0u) ++terminal_silent;
                else terminal_silent = 0u;
                ++receipt->presentation_count;
                pending_count = 0u;
                pending_bytes = 0u;
                pending_frames = 0u;
            } else if (opcode == 0x00u) {
                ended = 1;
            }
        }
        if (offset != chunk_end || ended) {
            if (!ended) return 0;
        }
    }
    if (!ended || offset != byte_count || receipt->presentation_count != stream.display_count ||
        audio_sequence != stream.audio_frame_count) return 0;
    receipt->audio_packet_count = audio_sequence;
    receipt->unpresented_audio_packet_count = pending_count;
    receipt->terminal_silent_presentation_count = terminal_silent;
    receipt->valid = 1;
    return 1;
}

int dm2_v1_mve_display_audio_timeline(
    const uint8_t *movie_bytes, size_t movie_byte_count,
    DM2_V1_MveDisplayAudioBoundary *boundaries, size_t boundary_capacity,
    DM2_V1_MveTimelineReceipt *out)
{
    DM2_V1_MveTimelineReceipt receipt;
    if (out) memset(out, 0, sizeof(*out));
    if (!movie_bytes || !out || (boundaries == NULL && boundary_capacity != 0u) ||
        !timeline_scan(movie_bytes, movie_byte_count, NULL, 0u, &receipt) ||
        (boundaries && boundary_capacity < receipt.presentation_count) ||
        (boundaries && !timeline_scan(movie_bytes, movie_byte_count, boundaries,
                                      boundary_capacity, &receipt)))
        return 0;
    *out = receipt;
    return 1;
}
