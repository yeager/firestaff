#include "firestaff_caps_raw.h"

#include <stdlib.h>
#include <string.h>

#define CAPS_RAW_OUTER_HEADER_SIZE 12u
#define CAPS_RAW_CHUNK_HEADER_SIZE 28u
#define CAPS_RAW_PACK_HEADER_SIZE 24u
#define CAPS_RAW_DATA_EXTENTS_SIZE 8u

static uint32_t caps_raw_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint32_t caps_raw_crc_update(uint32_t crc,
                                    const uint8_t *bytes,
                                    size_t count) {
    size_t i;
    unsigned bit;
    for (i = 0; i < count; ++i) {
        crc ^= bytes[i];
        for (bit = 0; bit < 8; ++bit) {
            uint32_t mask = (uint32_t)-(int32_t)(crc & 1u);
            crc = (crc >> 1) ^ (0xedb88320u & mask);
        }
    }
    return crc;
}

static uint32_t caps_raw_crc32(const uint8_t *bytes, size_t count) {
    return caps_raw_crc_update(0xffffffffu, bytes, count) ^ 0xffffffffu;
}

static uint32_t caps_raw_header_crc32(const uint8_t *bytes,
                                      size_t header_size,
                                      size_t crc_offset) {
    static const uint8_t zeros[4] = { 0, 0, 0, 0 };
    uint32_t crc = 0xffffffffu;
    crc = caps_raw_crc_update(crc, bytes, crc_offset);
    crc = caps_raw_crc_update(crc, zeros, sizeof(zeros));
    crc = caps_raw_crc_update(crc, bytes + crc_offset + 4u,
                              header_size - crc_offset - 4u);
    return crc ^ 0xffffffffu;
}

static int caps_raw_fits(size_t total, size_t offset, size_t count) {
    return offset <= total && count <= total - offset;
}

static FirestaffCapsRawStatus caps_raw_error(FirestaffCapsRawResult *result,
                                              FirestaffCapsRawStatus status,
                                              size_t offset) {
    result->error_offset = offset;
    return status;
}

static FirestaffCapsRawStatus caps_raw_validate_chunk(
    const uint8_t *bytes,
    size_t byte_count,
    size_t offset,
    const char signature[4],
    FirestaffCapsRawResult *result) {
    uint32_t size;
    uint32_t expected_crc;
    if (!caps_raw_fits(byte_count, offset, 8u)) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_TRUNCATED, offset);
    }
    if (memcmp(bytes + offset, signature, 4) != 0) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_SIGNATURE, offset);
    }
    size = caps_raw_be32(bytes + offset + 4u);
    if (size != CAPS_RAW_CHUNK_HEADER_SIZE) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_LENGTH,
                              offset + 4u);
    }
    if (!caps_raw_fits(byte_count, offset, size)) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_TRUNCATED, offset);
    }
    expected_crc = caps_raw_be32(bytes + offset + 8u);
    if (caps_raw_header_crc32(bytes + offset, size, 8u) != expected_crc) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_CRC,
                              offset + 8u);
    }
    return FIRESTAFF_CAPS_RAW_OK;
}

FirestaffCapsRawStatus firestaff_caps_raw_read(
    const uint8_t *bytes,
    size_t byte_count,
    FirestaffCapsRawPackVisitor visitor,
    void *user_data,
    FirestaffCapsRawResult *out_result) {
    FirestaffCapsRawResult local_result = { 0, 0, 0 };
    FirestaffCapsRawResult *result = out_result ? out_result : &local_result;
    size_t cursor;
    size_t total_unpacked = 0;

    result->track_count = 0;
    result->pack_count = 0;
    result->error_offset = 0;
    if (!bytes || byte_count == 0) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_INVALID_ARGUMENT, 0);
    }
    if (byte_count > FIRESTAFF_CAPS_RAW_MAX_MEMBER_BYTES) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_TOO_LARGE, 0);
    }
    if (!caps_raw_fits(byte_count, 0, CAPS_RAW_OUTER_HEADER_SIZE)) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_TRUNCATED, byte_count);
    }
    if (memcmp(bytes, "CAPS", 4) != 0) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_SIGNATURE, 0);
    }
    if (caps_raw_be32(bytes + 4u) != CAPS_RAW_OUTER_HEADER_SIZE) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_LENGTH, 4u);
    }
    if (caps_raw_header_crc32(bytes, CAPS_RAW_OUTER_HEADER_SIZE, 8u) !=
        caps_raw_be32(bytes + 8u)) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_CRC, 8u);
    }

    /* The container stores each track's DATA area before its TRCK metadata:
     * CAPS, DATA[0], TRCK[0], DATA[1], TRCK[1], ... */
    cursor = CAPS_RAW_OUTER_HEADER_SIZE;
    while (cursor < byte_count) {
        size_t data_offset = cursor;
        size_t body_offset;
        size_t body_size;
        size_t body_end;
        size_t trck_offset;
        size_t pack_offset;
        size_t extent_size[FIRESTAFF_CAPS_RAW_PACKS_PER_TRACK];
        uint32_t body_bits;
        uint32_t key;
        size_t pack_index;
        FirestaffCapsRawStatus status;

        if (result->track_count >= FIRESTAFF_CAPS_RAW_MAX_TRACKS) {
            return caps_raw_error(result, FIRESTAFF_CAPS_RAW_LIMIT_EXCEEDED,
                                  cursor);
        }
        status = caps_raw_validate_chunk(bytes, byte_count, data_offset,
                                         "DATA", result);
        if (status != FIRESTAFF_CAPS_RAW_OK) return status;

        body_size = (size_t)caps_raw_be32(bytes + data_offset + 12u);
        body_bits = caps_raw_be32(bytes + data_offset + 16u);
        key = caps_raw_be32(bytes + data_offset + 24u);
        body_offset = data_offset + CAPS_RAW_CHUNK_HEADER_SIZE;
        if (body_size < CAPS_RAW_DATA_EXTENTS_SIZE ||
            body_size > byte_count - body_offset) {
            return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_LENGTH,
                                  data_offset + 12u);
        }
        if (body_size > UINT32_MAX / 8u ||
            body_bits != (uint32_t)(body_size * 8u)) {
            return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_LENGTH,
                                  data_offset + 16u);
        }
        body_end = body_offset + body_size;
        extent_size[0] = (size_t)caps_raw_be32(bytes + body_offset);
        extent_size[1] = (size_t)caps_raw_be32(bytes + body_offset + 4u);
        if (extent_size[0] < CAPS_RAW_PACK_HEADER_SIZE ||
            extent_size[1] < CAPS_RAW_PACK_HEADER_SIZE ||
            extent_size[0] > body_size - CAPS_RAW_DATA_EXTENTS_SIZE ||
            extent_size[1] != body_size - CAPS_RAW_DATA_EXTENTS_SIZE -
                                  extent_size[0]) {
            return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_LENGTH,
                                  body_offset);
        }

        trck_offset = body_end;
        status = caps_raw_validate_chunk(bytes, byte_count, trck_offset,
                                         "TRCK", result);
        if (status != FIRESTAFF_CAPS_RAW_OK) return status;
        if (caps_raw_be32(bytes + trck_offset + 24u) != key) {
            return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_LENGTH,
                                  trck_offset + 24u);
        }

        pack_offset = body_offset + CAPS_RAW_DATA_EXTENTS_SIZE;
        for (pack_index = 0; pack_index < FIRESTAFF_CAPS_RAW_PACKS_PER_TRACK;
             ++pack_index) {
            size_t extent = extent_size[pack_index];
            uint32_t unpacked_size;
            uint32_t unpacked_crc;
            uint32_t packed_size;
            uint32_t packed_crc;
            size_t payload_offset;
            FirestaffCapsRawPack pack;

            if (!caps_raw_fits(trck_offset, pack_offset,
                               CAPS_RAW_PACK_HEADER_SIZE) ||
                extent < CAPS_RAW_PACK_HEADER_SIZE ||
                !caps_raw_fits(trck_offset, pack_offset, extent)) {
                return caps_raw_error(result, FIRESTAFF_CAPS_RAW_TRUNCATED,
                                      pack_offset);
            }
            if (memcmp(bytes + pack_offset, "PACK", 4) != 0) {
                return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_SIGNATURE,
                                      pack_offset);
            }
            unpacked_size = caps_raw_be32(bytes + pack_offset + 4u);
            unpacked_crc = caps_raw_be32(bytes + pack_offset + 8u);
            packed_size = caps_raw_be32(bytes + pack_offset + 12u);
            packed_crc = caps_raw_be32(bytes + pack_offset + 16u);
            if (unpacked_size > FIRESTAFF_CAPS_RAW_MAX_PACK_UNPACKED_BYTES ||
                (size_t)unpacked_size >
                    FIRESTAFF_CAPS_RAW_MAX_TOTAL_UNPACKED_BYTES -
                        total_unpacked) {
                return caps_raw_error(result, FIRESTAFF_CAPS_RAW_LIMIT_EXCEEDED,
                                      pack_offset + 4u);
            }
            if ((size_t)packed_size != extent - CAPS_RAW_PACK_HEADER_SIZE) {
                return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_LENGTH,
                                      pack_offset + 12u);
            }
            payload_offset = pack_offset + CAPS_RAW_PACK_HEADER_SIZE;
            if (!caps_raw_fits(trck_offset, payload_offset, packed_size)) {
                return caps_raw_error(result, FIRESTAFF_CAPS_RAW_TRUNCATED,
                                      payload_offset);
            }
            if (caps_raw_header_crc32(bytes + pack_offset,
                                      CAPS_RAW_PACK_HEADER_SIZE, 20u) !=
                caps_raw_be32(bytes + pack_offset + 20u)) {
                return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_CRC,
                                      pack_offset + 20u);
            }
            if (caps_raw_crc32(bytes + payload_offset, packed_size) !=
                packed_crc) {
                return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_CRC,
                                      pack_offset + 16u);
            }

            pack.track_index = (uint32_t)result->track_count;
            pack.pack_index = (uint32_t)pack_index;
            pack.packed_bytes = bytes + payload_offset;
            pack.packed_size = (size_t)packed_size;
            pack.unpacked_size = unpacked_size;
            pack.unpacked_crc32 = unpacked_crc;
            if (visitor && visitor(&pack, user_data) != 0) {
                return caps_raw_error(result,
                                      FIRESTAFF_CAPS_RAW_CALLBACK_ABORTED,
                                      pack_offset);
            }

            total_unpacked += (size_t)unpacked_size;
            ++result->pack_count;
            pack_offset += extent;
        }
        if (pack_offset != body_end) {
            return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_LENGTH,
                                  pack_offset);
        }

        ++result->track_count;
        cursor = trck_offset + CAPS_RAW_CHUNK_HEADER_SIZE;
    }

    if (result->track_count == 0 ||
        result->pack_count != result->track_count *
                                  FIRESTAFF_CAPS_RAW_PACKS_PER_TRACK) {
        return caps_raw_error(result, FIRESTAFF_CAPS_RAW_BAD_LENGTH, cursor);
    }
    return FIRESTAFF_CAPS_RAW_OK;
}

FirestaffCapsRawStatus firestaff_caps_raw_unpack_density(
    const FirestaffCapsRawPack *pack,
    uint8_t *out_bytes,
    size_t out_capacity) {
    size_t source;
    size_t word_count;
    size_t destination;

    if (!pack || !pack->packed_bytes || !out_bytes ||
        pack->pack_index != 0u || pack->packed_size == 0u ||
        pack->unpacked_size == 0u || (pack->unpacked_size & 3u) != 0u ||
        pack->unpacked_size > out_capacity) {
        return FIRESTAFF_CAPS_RAW_INVALID_ARGUMENT;
    }

    word_count = (size_t)pack->unpacked_size / 4u;
    source = pack->packed_size;
    destination = word_count;

    /* Density PACK is decoded from the end of both streams. Each destination
     * item is a 32-bit timing symbol serialized big-endian for stable CRCs. */
    while (destination != 0u) {
        uint8_t control;
        unsigned mode;
        size_t count;
        size_t distance = 0u;

        if (source == 0u) return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
        control = pack->packed_bytes[--source];
        mode = control & 3u;

        if (mode == 0u) {
            if ((control & 8u) != 0u) {
                if (source == 0u) return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
                count = (((size_t)control >> 4) << 8) |
                        pack->packed_bytes[--source];
            } else {
                count = ((size_t)control >> 4) + 1u;
            }
            if (count == 0u || count > destination)
                return FIRESTAFF_CAPS_RAW_CODEC_ERROR;

            while (count-- != 0u) {
                size_t output = --destination * 4u;
                if ((control & 4u) != 0u) {
                    if (source < 4u) return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
                    source -= 4u;
                    out_bytes[output] = pack->packed_bytes[source + 3u];
                    out_bytes[output + 1u] = pack->packed_bytes[source + 2u];
                    out_bytes[output + 2u] = pack->packed_bytes[source + 1u];
                    out_bytes[output + 3u] = pack->packed_bytes[source];
                } else {
                    if (source == 0u) return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
                    out_bytes[output] = 0u;
                    out_bytes[output + 1u] = 0u;
                    out_bytes[output + 2u] = 0u;
                    out_bytes[output + 3u] = pack->packed_bytes[--source];
                }
            }
            continue;
        }

        if (mode == 1u) {
            count = ((size_t)control >> 2) + 1u;
            if (source == 0u) return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
            distance = (size_t)pack->packed_bytes[--source] + 1u;
        } else if (mode == 2u) {
            count = ((size_t)control >> 2) + 1u;
            if (source < 2u) return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
            source -= 2u;
            distance = (size_t)pack->packed_bytes[source] |
                       ((size_t)pack->packed_bytes[source + 1u] << 8);
        } else {
            if (source < 3u) return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
            source -= 3u;
            count = (((size_t)control >> 2) << 8) |
                    pack->packed_bytes[source + 2u];
            distance = (size_t)pack->packed_bytes[source] |
                       ((size_t)pack->packed_bytes[source + 1u] << 8);
        }

        if (count == 0u || count > destination || distance == 0u ||
            distance > word_count - destination) {
            return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
        }
        while (count-- != 0u) {
            size_t output = --destination;
            size_t input = output + distance;
            memcpy(out_bytes + output * 4u, out_bytes + input * 4u, 4u);
        }
    }

    if (source != 0u) return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
    if (caps_raw_crc32(out_bytes, (size_t)pack->unpacked_size) !=
        pack->unpacked_crc32) {
        return FIRESTAFF_CAPS_RAW_BAD_CRC;
    }
    return FIRESTAFF_CAPS_RAW_OK;
}

static uint16_t caps_raw_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

FirestaffCapsRawStatus firestaff_caps_raw_unpack_tracks(
    const FirestaffCapsRawPack *pack,
    FirestaffCapsRawTrackSet *out_tracks) {
    size_t sizes[FIRESTAFF_CAPS_RAW_MAX_TRACK_VARIANTS] = { 0 };
    size_t offsets[FIRESTAFF_CAPS_RAW_MAX_TRACK_VARIANTS] = { 0 };
    size_t total = 0;
    size_t source;
    size_t i;
    size_t base_size;
    uint8_t variant_count;
    uint8_t *decoded;

    if (!out_tracks) return FIRESTAFF_CAPS_RAW_INVALID_ARGUMENT;
    memset(out_tracks, 0, sizeof(*out_tracks));
    if (!pack || !pack->packed_bytes || pack->pack_index != 1u ||
        pack->packed_size < 3u || pack->unpacked_size == 0u) {
        return FIRESTAFF_CAPS_RAW_INVALID_ARGUMENT;
    }
    if (pack->unpacked_size > FIRESTAFF_CAPS_RAW_MAX_PACK_UNPACKED_BYTES) {
        return FIRESTAFF_CAPS_RAW_LIMIT_EXCEEDED;
    }

    variant_count = pack->packed_bytes[0];
    if (variant_count == 0u ||
        variant_count > FIRESTAFF_CAPS_RAW_MAX_TRACK_VARIANTS ||
        pack->packed_size < 1u + (size_t)variant_count * 2u) {
        return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
    }

    source = 1u;
    for (i = 0; i < variant_count; ++i) {
        sizes[i] = caps_raw_be16(pack->packed_bytes + source);
        source += 2u;
        if (sizes[i] == 0u || sizes[i] > pack->unpacked_size - total) {
            return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
        }
        offsets[i] = total;
        total += sizes[i];
    }
    if (total != (size_t)pack->unpacked_size) {
        return FIRESTAFF_CAPS_RAW_BAD_LENGTH;
    }

    decoded = (uint8_t *)malloc(total);
    if (!decoded) return FIRESTAFF_CAPS_RAW_LIMIT_EXCEEDED;
    base_size = sizes[0];
    if (base_size > pack->packed_size - source) {
        free(decoded);
        return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
    }
    memcpy(decoded, pack->packed_bytes + source, base_size);
    source += base_size;

    for (i = 1u; i < variant_count; ++i) {
        size_t produced = 0u;
        size_t target_size = sizes[i];
        uint8_t *target = decoded + offsets[i];

        while (produced < target_size) {
            uint8_t command;
            size_t count;
            if (source >= pack->packed_size) {
                free(decoded);
                return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
            }
            command = pack->packed_bytes[source++];
            if ((command & 0x80u) == 0u) {
                size_t literal_count;
                if (source >= pack->packed_size) {
                    free(decoded);
                    return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
                }
                literal_count = ((size_t)command << 8) |
                                pack->packed_bytes[source++];
                if (literal_count == 0u ||
                    literal_count > target_size - produced ||
                    literal_count > pack->packed_size - source) {
                    free(decoded);
                    return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
                }
                memcpy(target + produced, pack->packed_bytes + source,
                       literal_count);
                source += literal_count;
                produced += literal_count;
                continue;
            }

            if (!caps_raw_fits(pack->packed_size, source, 3u)) {
                free(decoded);
                return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
            }
            count = (((size_t)command & 0x0fu) << 8) |
                    pack->packed_bytes[source++];
            {
                unsigned shift = (unsigned)((command >> 4) & 7u);
                size_t base_offset = ((size_t)pack->packed_bytes[source] << 8) |
                                     pack->packed_bytes[source + 1u];
                const uint8_t *base = decoded;
                size_t needed = count;
                size_t j;
                uint32_t accumulator;
                source += 2u;
                if (count == 0u || count > target_size - produced ||
                    base_offset > base_size ||
                    (shift == 0u ? count : count + 1u) >
                        base_size - base_offset) {
                    free(decoded);
                    return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
                }
                if (shift == 0u) {
                    memcpy(target + produced, base + base_offset, count);
                    produced += count;
                    continue;
                }
                accumulator = base[base_offset++];
                needed = count;
                for (j = 0; j < needed; ++j) {
                    accumulator = (accumulator << 8) | base[base_offset++];
                    target[produced++] = (uint8_t)(accumulator >> shift);
                }
            }
        }
    }

    if (source != pack->packed_size) {
        free(decoded);
        return FIRESTAFF_CAPS_RAW_CODEC_ERROR;
    }
    if (caps_raw_crc32(decoded, total) != pack->unpacked_crc32) {
        free(decoded);
        return FIRESTAFF_CAPS_RAW_BAD_CRC;
    }

    out_tracks->variant_count = variant_count;
    memcpy(out_tracks->variant_sizes, sizes, sizeof(sizes));
    out_tracks->variant_bytes = decoded;
    return FIRESTAFF_CAPS_RAW_OK;
}

void firestaff_caps_raw_free_tracks(FirestaffCapsRawTrackSet *tracks) {
    if (!tracks) return;
    free(tracks->variant_bytes);
    memset(tracks, 0, sizeof(*tracks));
}

const char *firestaff_caps_raw_status_name(FirestaffCapsRawStatus status) {
    switch (status) {
        case FIRESTAFF_CAPS_RAW_OK: return "ok";
        case FIRESTAFF_CAPS_RAW_INVALID_ARGUMENT: return "invalid-argument";
        case FIRESTAFF_CAPS_RAW_TOO_LARGE: return "too-large";
        case FIRESTAFF_CAPS_RAW_TRUNCATED: return "truncated";
        case FIRESTAFF_CAPS_RAW_BAD_SIGNATURE: return "bad-signature";
        case FIRESTAFF_CAPS_RAW_BAD_LENGTH: return "bad-length";
        case FIRESTAFF_CAPS_RAW_BAD_CRC: return "bad-crc";
        case FIRESTAFF_CAPS_RAW_LIMIT_EXCEEDED: return "limit-exceeded";
        case FIRESTAFF_CAPS_RAW_CALLBACK_ABORTED: return "callback-aborted";
        case FIRESTAFF_CAPS_RAW_CODEC_ERROR: return "codec-error";
        default: return "unknown";
    }
}
