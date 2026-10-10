#include "firestaff_caps_raw.h"

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
        default: return "unknown";
    }
}
