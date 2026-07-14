#include "theron_v1_dungeon_handoff.h"

#include <string.h>

enum {
    TQR_DESCRIPTOR_BYTES = 18u,
    TQR_JP_CANDIDATE_RAW_OFFSET = 0x700c84u,
    TQR_US_CANDIDATE_RAW_OFFSET = 0x7015b4u,
    TQR_JP_DESCRIPTOR_RAW_OFFSET = 0x70ffd4u,
    TQR_US_DESCRIPTOR_RAW_OFFSET = 0x710904u,
    TQR_JP_CUE_INDEX01_RAW_SECTOR = 224u,
    TQR_US_CUE_INDEX01_RAW_SECTOR = 225u,
    TQR_INITIAL_ENVELOPE_RAW_SECTOR_OFFSET = 0x124u,
    TQR_INITIAL_BOUNDARY_RAW_SECTOR_OFFSET = 0x490u
};

static const uint8_t g_descriptor[TQR_DESCRIPTOR_BYTES] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20,
    0x10, 0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

static int raw_range_present(size_t track_bytes, size_t offset, size_t bytes) {
    return offset <= track_bytes && bytes <= track_bytes - offset;
}

static uint16_t read_be16(const uint8_t *bytes) {
    return (uint16_t)(((uint16_t)bytes[0] << 8u) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes) {
    return ((uint32_t)bytes[0] << 24u) | ((uint32_t)bytes[1] << 16u) |
        ((uint32_t)bytes[2] << 8u) | bytes[3];
}

typedef struct {
    uint32_t state[4];
    uint8_t block[64];
    size_t block_bytes;
} TqrMd5;

#define TQR_MD5_F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define TQR_MD5_G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define TQR_MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define TQR_MD5_I(x, y, z) ((y) ^ ((x) | ~(z)))
#define TQR_MD5_ROTATE(x, count) (((x) << (count)) | ((x) >> (32u - (count))))
#define TQR_MD5_STEP(fn, a, b, c, d, x, count, constant) do { \
    (a) += fn((b), (c), (d)) + (x) + (constant); \
    (a) = TQR_MD5_ROTATE((a), (count)); \
    (a) += (b); \
} while (0)

static uint32_t read_le32(const uint8_t *bytes) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8u) |
        ((uint32_t)bytes[2] << 16u) | ((uint32_t)bytes[3] << 24u);
}

static void tqr_md5_block(TqrMd5 *md5, const uint8_t block[64]) {
    uint32_t x[16], a = md5->state[0], b = md5->state[1];
    uint32_t c = md5->state[2], d = md5->state[3];
    size_t i;

    for (i = 0; i < 16u; ++i) x[i] = read_le32(block + i * 4u);
    /* The explicit operation schedule keeps this verifier self-contained. */
    TQR_MD5_STEP(TQR_MD5_F,a,b,c,d,x[0],7,0xd76aa478); TQR_MD5_STEP(TQR_MD5_F,d,a,b,c,x[1],12,0xe8c7b756); TQR_MD5_STEP(TQR_MD5_F,c,d,a,b,x[2],17,0x242070db); TQR_MD5_STEP(TQR_MD5_F,b,c,d,a,x[3],22,0xc1bdceee);
    TQR_MD5_STEP(TQR_MD5_F,a,b,c,d,x[4],7,0xf57c0faf); TQR_MD5_STEP(TQR_MD5_F,d,a,b,c,x[5],12,0x4787c62a); TQR_MD5_STEP(TQR_MD5_F,c,d,a,b,x[6],17,0xa8304613); TQR_MD5_STEP(TQR_MD5_F,b,c,d,a,x[7],22,0xfd469501);
    TQR_MD5_STEP(TQR_MD5_F,a,b,c,d,x[8],7,0x698098d8); TQR_MD5_STEP(TQR_MD5_F,d,a,b,c,x[9],12,0x8b44f7af); TQR_MD5_STEP(TQR_MD5_F,c,d,a,b,x[10],17,0xffff5bb1); TQR_MD5_STEP(TQR_MD5_F,b,c,d,a,x[11],22,0x895cd7be);
    TQR_MD5_STEP(TQR_MD5_F,a,b,c,d,x[12],7,0x6b901122); TQR_MD5_STEP(TQR_MD5_F,d,a,b,c,x[13],12,0xfd987193); TQR_MD5_STEP(TQR_MD5_F,c,d,a,b,x[14],17,0xa679438e); TQR_MD5_STEP(TQR_MD5_F,b,c,d,a,x[15],22,0x49b40821);
    TQR_MD5_STEP(TQR_MD5_G,a,b,c,d,x[1],5,0xf61e2562); TQR_MD5_STEP(TQR_MD5_G,d,a,b,c,x[6],9,0xc040b340); TQR_MD5_STEP(TQR_MD5_G,c,d,a,b,x[11],14,0x265e5a51); TQR_MD5_STEP(TQR_MD5_G,b,c,d,a,x[0],20,0xe9b6c7aa);
    TQR_MD5_STEP(TQR_MD5_G,a,b,c,d,x[5],5,0xd62f105d); TQR_MD5_STEP(TQR_MD5_G,d,a,b,c,x[10],9,0x02441453); TQR_MD5_STEP(TQR_MD5_G,c,d,a,b,x[15],14,0xd8a1e681); TQR_MD5_STEP(TQR_MD5_G,b,c,d,a,x[4],20,0xe7d3fbc8);
    TQR_MD5_STEP(TQR_MD5_G,a,b,c,d,x[9],5,0x21e1cde6); TQR_MD5_STEP(TQR_MD5_G,d,a,b,c,x[14],9,0xc33707d6); TQR_MD5_STEP(TQR_MD5_G,c,d,a,b,x[3],14,0xf4d50d87); TQR_MD5_STEP(TQR_MD5_G,b,c,d,a,x[8],20,0x455a14ed);
    TQR_MD5_STEP(TQR_MD5_G,a,b,c,d,x[13],5,0xa9e3e905); TQR_MD5_STEP(TQR_MD5_G,d,a,b,c,x[2],9,0xfcefa3f8); TQR_MD5_STEP(TQR_MD5_G,c,d,a,b,x[7],14,0x676f02d9); TQR_MD5_STEP(TQR_MD5_G,b,c,d,a,x[12],20,0x8d2a4c8a);
    TQR_MD5_STEP(TQR_MD5_H,a,b,c,d,x[5],4,0xfffa3942); TQR_MD5_STEP(TQR_MD5_H,d,a,b,c,x[8],11,0x8771f681); TQR_MD5_STEP(TQR_MD5_H,c,d,a,b,x[11],16,0x6d9d6122); TQR_MD5_STEP(TQR_MD5_H,b,c,d,a,x[14],23,0xfde5380c);
    TQR_MD5_STEP(TQR_MD5_H,a,b,c,d,x[1],4,0xa4beea44); TQR_MD5_STEP(TQR_MD5_H,d,a,b,c,x[4],11,0x4bdecfa9); TQR_MD5_STEP(TQR_MD5_H,c,d,a,b,x[7],16,0xf6bb4b60); TQR_MD5_STEP(TQR_MD5_H,b,c,d,a,x[10],23,0xbebfbc70);
    TQR_MD5_STEP(TQR_MD5_H,a,b,c,d,x[13],4,0x289b7ec6); TQR_MD5_STEP(TQR_MD5_H,d,a,b,c,x[0],11,0xeaa127fa); TQR_MD5_STEP(TQR_MD5_H,c,d,a,b,x[3],16,0xd4ef3085); TQR_MD5_STEP(TQR_MD5_H,b,c,d,a,x[6],23,0x04881d05);
    TQR_MD5_STEP(TQR_MD5_H,a,b,c,d,x[9],4,0xd9d4d039); TQR_MD5_STEP(TQR_MD5_H,d,a,b,c,x[12],11,0xe6db99e5); TQR_MD5_STEP(TQR_MD5_H,c,d,a,b,x[15],16,0x1fa27cf8); TQR_MD5_STEP(TQR_MD5_H,b,c,d,a,x[2],23,0xc4ac5665);
    TQR_MD5_STEP(TQR_MD5_I,a,b,c,d,x[0],6,0xf4292244); TQR_MD5_STEP(TQR_MD5_I,d,a,b,c,x[7],10,0x432aff97); TQR_MD5_STEP(TQR_MD5_I,c,d,a,b,x[14],15,0xab9423a7); TQR_MD5_STEP(TQR_MD5_I,b,c,d,a,x[5],21,0xfc93a039);
    TQR_MD5_STEP(TQR_MD5_I,a,b,c,d,x[12],6,0x655b59c3); TQR_MD5_STEP(TQR_MD5_I,d,a,b,c,x[3],10,0x8f0ccc92); TQR_MD5_STEP(TQR_MD5_I,c,d,a,b,x[10],15,0xffeff47d); TQR_MD5_STEP(TQR_MD5_I,b,c,d,a,x[1],21,0x85845dd1);
    TQR_MD5_STEP(TQR_MD5_I,a,b,c,d,x[8],6,0x6fa87e4f); TQR_MD5_STEP(TQR_MD5_I,d,a,b,c,x[15],10,0xfe2ce6e0); TQR_MD5_STEP(TQR_MD5_I,c,d,a,b,x[6],15,0xa3014314); TQR_MD5_STEP(TQR_MD5_I,b,c,d,a,x[13],21,0x4e0811a1);
    TQR_MD5_STEP(TQR_MD5_I,a,b,c,d,x[4],6,0xf7537e82); TQR_MD5_STEP(TQR_MD5_I,d,a,b,c,x[11],10,0xbd3af235); TQR_MD5_STEP(TQR_MD5_I,c,d,a,b,x[2],15,0x2ad7d2bb); TQR_MD5_STEP(TQR_MD5_I,b,c,d,a,x[9],21,0xeb86d391);
    md5->state[0] += a; md5->state[1] += b;
    md5->state[2] += c; md5->state[3] += d;
}

static void tqr_md5_update(TqrMd5 *md5, const uint8_t *bytes, size_t count) {
    while (count != 0u) {
        size_t take = 64u - md5->block_bytes;
        if (take > count) take = count;
        memcpy(md5->block + md5->block_bytes, bytes, take);
        md5->block_bytes += take;
        bytes += take;
        count -= take;
        if (md5->block_bytes == 64u) {
            tqr_md5_block(md5, md5->block);
            md5->block_bytes = 0u;
        }
    }
}

int theron_v1_track02_raw_bytes_match_md5(const uint8_t *bytes, size_t count,
                                          const char *expected_hex) {
    static const char hex[] = "0123456789abcdef";
    TqrMd5 md5 = {{0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u},
                  {0}, 0u};
    uint8_t digest[16], padding[64] = {0x80u};
    uint64_t bit_count = (uint64_t)count * 8u;
    char actual_hex[33];
    size_t i;

    if (!bytes || !expected_hex || strlen(expected_hex) != 32u) return 0;
    tqr_md5_update(&md5, bytes, count);
    tqr_md5_update(&md5, padding,
                   md5.block_bytes < 56u ? 56u - md5.block_bytes :
                   120u - md5.block_bytes);
    for (i = 0; i < 8u; ++i) padding[i] = (uint8_t)(bit_count >> (i * 8u));
    tqr_md5_update(&md5, padding, 8u);
    for (i = 0; i < 4u; ++i) {
        digest[i * 4u] = (uint8_t)md5.state[i];
        digest[i * 4u + 1u] = (uint8_t)(md5.state[i] >> 8u);
        digest[i * 4u + 2u] = (uint8_t)(md5.state[i] >> 16u);
        digest[i * 4u + 3u] = (uint8_t)(md5.state[i] >> 24u);
    }
    for (i = 0; i < sizeof(digest); ++i) {
        actual_hex[i * 2u] = hex[digest[i] >> 4u];
        actual_hex[i * 2u + 1u] = hex[digest[i] & 0x0fu];
    }
    actual_hex[32] = '\0';
    return strcmp(actual_hex, expected_hex) == 0;
}

int theron_v1_dungeon_handoff_select_initial_level(
    const Theron_V1DungeonHandoffFacts *facts,
    Theron_V1DungeonHandoffReceipt *out_receipt) {
    Theron_V1DungeonHandoffReceipt receipt = {0};
    size_t candidate_offset;
    size_t descriptor_offset;
    uint32_t cue_index01_sector;
    uint32_t raw_sector;
    uint32_t raw_sector_offset;
    uint16_t header_width;
    uint16_t header_height;
    uint32_t header_seed;
    uint16_t header_identifier;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!facts || !out_receipt || !facts->runtime_admission ||
        !facts->runtime_admission->attached ||
        !facts->runtime_admission->admitted || !facts->track02_hash_verified ||
        !facts->track02_md5 || !facts->raw_track02 ||
        facts->raw_track02_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u) {
        return 0;
    }

    if (strcmp(facts->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0) {
        candidate_offset = TQR_JP_CANDIDATE_RAW_OFFSET;
        descriptor_offset = TQR_JP_DESCRIPTOR_RAW_OFFSET;
        cue_index01_sector = TQR_JP_CUE_INDEX01_RAW_SECTOR;
    } else if (strcmp(facts->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0) {
        candidate_offset = TQR_US_CANDIDATE_RAW_OFFSET;
        descriptor_offset = TQR_US_DESCRIPTOR_RAW_OFFSET;
        cue_index01_sector = TQR_US_CUE_INDEX01_RAW_SECTOR;
    } else {
        return 0;
    }

    raw_sector = (uint32_t)(candidate_offset /
        THERON_V1_TRACK02_RAW_SECTOR_BYTES);
    raw_sector_offset = (uint32_t)(candidate_offset %
        THERON_V1_TRACK02_RAW_SECTOR_BYTES);

    if (facts->cue_track02_index01_raw_sector != cue_index01_sector ||
        !theron_v1_track02_raw_bytes_match_md5(
            facts->raw_track02, facts->raw_track02_bytes, facts->track02_md5) ||
        !raw_range_present(facts->raw_track02_bytes, candidate_offset,
                           THERON_V1_INITIAL_ENVELOPE_BYTES) ||
        !raw_range_present(facts->raw_track02_bytes, descriptor_offset,
                           sizeof(g_descriptor))) {
        return 0;
    }

    header_width = read_be16(facts->raw_track02 + candidate_offset);
    header_height = read_be16(facts->raw_track02 + candidate_offset + 2u);
    header_seed = read_be32(facts->raw_track02 + candidate_offset + 4u);
    header_identifier = read_be16(facts->raw_track02 + candidate_offset + 8u);
    if (raw_sector_offset != TQR_INITIAL_ENVELOPE_RAW_SECTOR_OFFSET ||
        raw_sector_offset != THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET +
            THERON_V1_TRACK02_MODE1_HEADER_BYTES ||
        (candidate_offset + THERON_V1_INITIAL_ENVELOPE_BYTES) %
            THERON_V1_TRACK02_RAW_SECTOR_BYTES !=
            TQR_INITIAL_BOUNDARY_RAW_SECTOR_OFFSET ||
        memcmp(facts->raw_track02 + descriptor_offset, g_descriptor,
               sizeof(g_descriptor)) != 0 ||
        header_width != THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        header_height != THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        header_seed != THERON_V1_INITIAL_ENVELOPE_HEADER_SEED ||
        header_identifier != THERON_V1_INITIAL_ENVELOPE_HEADER_IDENTIFIER) {
        return 0;
    }

    receipt.selected = 1;
    receipt.runtime_route_consumed = 1;
    receipt.record = raw_sector - cue_index01_sector;
    receipt.record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    receipt.envelope_bytes = THERON_V1_INITIAL_ENVELOPE_BYTES;
    receipt.header_width = header_width;
    receipt.header_height = header_height;
    receipt.header_seed = header_seed;
    receipt.header_identifier = header_identifier;
    receipt.cue_track02_index01_raw_sector = cue_index01_sector;
    receipt.track02_raw_sector = raw_sector;
    receipt.raw_sector_offset = raw_sector_offset;
    receipt.raw_track02_md5_verified = 1;
    receipt.adjacent_boundary_opaque = 1;
    receipt.route = "raw_track02_initial_envelope";
    *out_receipt = receipt;
    return 1;
}
