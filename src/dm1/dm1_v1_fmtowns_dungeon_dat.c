#include "dm1_v1_fmtowns_dungeon_dat.h"
#include <string.h>

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

int dm1_v1_fmtowns_dungeon_probe(const uint8_t *data, size_t size) {
    if (!data || size < 64) return 0;
    uint16_t map_count = rd16le(data);
    if (map_count != DM1_FMTOWNS_DUNGEON_MAP_COUNT) return 0;
    if (size != DM1_FMTOWNS_DUNGEON_EN_SIZE &&
        size != DM1_FMTOWNS_DUNGEON_JP_SIZE) return 0;
    return 1;
}

/* MD5 — same minimal implementation as the graphics module. */
typedef struct { uint32_t s[4]; uint64_t count; uint8_t buf[64]; } MD5Ctx_D;

static const uint32_t md5_k_d[64] = {
    0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
    0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
    0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
    0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
    0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
    0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
    0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
    0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};
static const uint8_t md5_r_d[64] = {
    7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
    5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
    4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
    6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
};

static uint32_t rotl32_d(uint32_t v, unsigned n) { return (v << n) | (v >> (32 - n)); }

static void md5_transform_d(MD5Ctx_D *c, const uint8_t *block) {
    uint32_t a = c->s[0], b = c->s[1], cc = c->s[2], d = c->s[3];
    uint32_t m[16];
    for (int i = 0; i < 16; i++) m[i] = rd32le(block + (size_t)i * 4);
    for (int i = 0; i < 64; i++) {
        uint32_t f, g;
        if (i < 16) { f = (b & cc) | (~b & d); g = (uint32_t)i; }
        else if (i < 32) { f = (d & b) | (~d & cc); g = (5u * (uint32_t)i + 1u) % 16u; }
        else if (i < 48) { f = b ^ cc ^ d; g = (3u * (uint32_t)i + 5u) % 16u; }
        else { f = cc ^ (b | ~d); g = (7u * (uint32_t)i) % 16u; }
        uint32_t tmp = d; d = cc; cc = b;
        b = b + rotl32_d(a + f + md5_k_d[i] + m[g], md5_r_d[i]);
        a = tmp;
    }
    c->s[0] += a; c->s[1] += b; c->s[2] += cc; c->s[3] += d;
}

static void md5_init_d(MD5Ctx_D *c) {
    c->s[0] = 0x67452301; c->s[1] = 0xefcdab89;
    c->s[2] = 0x98badcfe; c->s[3] = 0x10325476;
    c->count = 0;
}

static void md5_update_d(MD5Ctx_D *c, const uint8_t *data, size_t len) {
    size_t offset = (size_t)(c->count & 63);
    c->count += len;
    for (size_t i = 0; i < len; i++) {
        c->buf[offset++] = data[i];
        if (offset == 64) { md5_transform_d(c, c->buf); offset = 0; }
    }
}

static void md5_final_d(MD5Ctx_D *c, uint8_t out[16]) {
    size_t offset = (size_t)(c->count & 63);
    c->buf[offset++] = 0x80;
    if (offset > 56) {
        memset(c->buf + offset, 0, 64 - offset);
        md5_transform_d(c, c->buf);
        offset = 0;
    }
    memset(c->buf + offset, 0, 56 - offset);
    uint64_t bits = c->count * 8;
    for (int i = 0; i < 8; i++) c->buf[56 + i] = (uint8_t)(bits >> (i * 8));
    md5_transform_d(c, c->buf);
    for (int i = 0; i < 4; i++) {
        out[i * 4 + 0] = (uint8_t)(c->s[i]);
        out[i * 4 + 1] = (uint8_t)(c->s[i] >> 8);
        out[i * 4 + 2] = (uint8_t)(c->s[i] >> 16);
        out[i * 4 + 3] = (uint8_t)(c->s[i] >> 24);
    }
}

static const uint8_t dm1_fmtowns_en_dng_md5[16] = {
    0x3d, 0xc0, 0xa9, 0x32, 0xd0, 0xe0, 0xad, 0xfe,
    0x59, 0x87, 0x8f, 0x07, 0xc5, 0x17, 0x00, 0xc5
};

static const uint8_t dm1_fmtowns_jp_dng_md5[16] = {
    0xfe, 0x09, 0x8f, 0x70, 0xce, 0x83, 0xcf, 0xe3,
    0xf2, 0x33, 0x35, 0x65, 0x09, 0x3d, 0xaf, 0x35
};

int dm1_v1_fmtowns_dungeon_receipt(const uint8_t *data, size_t size,
                                    DM1_V1_FmtownsDungeonReceipt *out) {
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!dm1_v1_fmtowns_dungeon_probe(data, size)) return -1;

    out->is_fmtowns = 1;
    out->file_size = (uint32_t)size;
    out->map_count = rd16le(data);

    MD5Ctx_D md5;
    md5_init_d(&md5);
    md5_update_d(&md5, data, size);
    md5_final_d(&md5, out->md5);

    if (memcmp(out->md5, dm1_fmtowns_en_dng_md5, 16) == 0)
        out->lang = 1;
    else if (memcmp(out->md5, dm1_fmtowns_jp_dng_md5, 16) == 0)
        out->lang = 2;
    else
        out->lang = 0;

    return 0;
}
