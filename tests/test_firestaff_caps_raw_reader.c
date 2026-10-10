#include "firestaff_caps_raw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct VisitorState {
    size_t calls;
    int invalid_order;
} VisitorState;

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static void write_be32(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}

static uint32_t crc32_update(uint32_t crc, const uint8_t *bytes, size_t count) {
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

static uint32_t header_crc(const uint8_t *bytes, size_t size) {
    static const uint8_t zero[4] = { 0, 0, 0, 0 };
    uint32_t crc = 0xffffffffu;
    crc = crc32_update(crc, bytes, 8u);
    crc = crc32_update(crc, zero, sizeof(zero));
    crc = crc32_update(crc, bytes + 12u, size - 12u);
    return crc ^ 0xffffffffu;
}

static int visit_pack(const FirestaffCapsRawPack *pack, void *opaque) {
    VisitorState *state = (VisitorState *)opaque;
    if (!pack || !pack->packed_bytes || pack->packed_size == 0 ||
        pack->track_index >= 164u || pack->pack_index > 1u ||
        pack->unpacked_size == 0) {
        state->invalid_order = 1;
    }
    if (pack && pack->pack_index != (state->calls % 2u)) {
        state->invalid_order = 1;
    }
    ++state->calls;
    return 0;
}

static int expect_rejected(const uint8_t *bytes, size_t size) {
    FirestaffCapsRawResult result;
    return firestaff_caps_raw_read(bytes, size, NULL, NULL, &result) !=
           FIRESTAFF_CAPS_RAW_OK;
}

static int malformed_cases(const uint8_t *original, size_t size) {
    uint8_t *copy;
    size_t body_size;
    size_t trck_offset;
    int ok = 1;

    if (size < 64u) return 0;
    copy = (uint8_t *)malloc(size);
    if (!copy) return 0;

    /* A short outer header and a stream truncated in its last record fail. */
    if (!expect_rejected(original, 11u) || !expect_rejected(original, size - 1u)) {
        ok = 0;
    }

    /* Body lengths cannot escape the member, even with a valid DATA-header CRC. */
    memcpy(copy, original, size);
    write_be32(copy + 24u, 0xffffffffu);
    write_be32(copy + 20u, 0u);
    write_be32(copy + 20u, header_crc(copy + 12u, 28u));
    if (!expect_rejected(copy, size)) ok = 0;

    /* The first extent cannot claim bytes beyond its DATA area. */
    memcpy(copy, original, size);
    write_be32(copy + 40u, 0xffffffffu);
    if (!expect_rejected(copy, size)) ok = 0;

    /* Payload corruption is detected by the packed CRC. */
    memcpy(copy, original, size);
    copy[48u + 24u] ^= 0x80u;
    if (!expect_rejected(copy, size)) ok = 0;

    /* The paired TRCK key must agree with this DATA record's key. */
    body_size = (size_t)read_be32(original + 24u);
    if (body_size > size - 40u) {
        ok = 0;
    } else {
        trck_offset = 40u + body_size;
        if (trck_offset > size || size - trck_offset < 28u) {
            ok = 0;
        } else {
            memcpy(copy, original, size);
            write_be32(copy + trck_offset + 24u,
                       read_be32(copy + trck_offset + 24u) + 1u);
            write_be32(copy + trck_offset + 8u, 0u);
            write_be32(copy + trck_offset + 8u,
                       header_crc(copy + trck_offset, 28u));
            if (!expect_rejected(copy, size)) ok = 0;
        }
    }

    free(copy);
    return ok;
}

int main(void) {
    uint8_t *bytes;
    size_t count = 0;
    int extra;
    VisitorState state = { 0, 0 };
    FirestaffCapsRawResult result;
    FirestaffCapsRawStatus status;

    bytes = (uint8_t *)malloc(FIRESTAFF_CAPS_RAW_MAX_MEMBER_BYTES);
    if (!bytes) {
        fputs("FAIL: allocation failed\n", stderr);
        return 1;
    }
    while (count < FIRESTAFF_CAPS_RAW_MAX_MEMBER_BYTES) {
        size_t got = fread(bytes + count, 1,
                           FIRESTAFF_CAPS_RAW_MAX_MEMBER_BYTES - count,
                           stdin);
        count += got;
        if (got == 0) break;
    }
    extra = fgetc(stdin);
    if (extra != EOF || ferror(stdin)) {
        fputs("FAIL: member exceeds input bound or read failed\n", stderr);
        free(bytes);
        return 1;
    }

    status = firestaff_caps_raw_read(bytes, count, visit_pack, &state, &result);
    if (status != FIRESTAFF_CAPS_RAW_OK || result.track_count != 164u ||
        result.pack_count != 328u || state.calls != 328u || state.invalid_order) {
        fprintf(stderr,
                "FAIL: parse status=%s tracks=%zu packs=%zu callbacks=%zu bad_order=%d at=%zu\n",
                firestaff_caps_raw_status_name(status), result.track_count,
                result.pack_count, state.calls, state.invalid_order,
                result.error_offset);
        free(bytes);
        return 1;
    }
    if (!malformed_cases(bytes, count)) {
        fputs("FAIL: malformed-input checks did not reject every mutation\n",
              stderr);
        free(bytes);
        return 1;
    }
    printf("PASS: tracks=%zu packs=%zu malformed-input checks passed\n",
           result.track_count, result.pack_count);
    free(bytes);
    return 0;
}
