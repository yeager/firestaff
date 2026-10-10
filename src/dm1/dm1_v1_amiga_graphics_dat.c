#include "dm1_v1_amiga_graphics_dat.h"
#include "dm1_v1_legacy_graphics_dat.h"
#include <stdlib.h>
#include <string.h>

static uint16_t rd16be(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

int dm1_v1_amiga_graphics_probe(const uint8_t *data, size_t size) {
    if (!data || size < 6) return 0;

    uint16_t count_be = rd16be(data);
    if (count_be != DM1_AMIGA_GRAPHICS_EXPECTED_COUNT) return 0;
    if (size < DM1_AMIGA_GRAPHICS_MIN_SIZE ||
        size > DM1_AMIGA_GRAPHICS_MAX_SIZE) return 0;

    /* Reject PC 3.4 new-format: 0x8001 marker (LE: 0x01 0x80) */
    if (data[0] == 0x01 && data[1] == 0x80) return 0;

    size_t header_min = 2 + (size_t)count_be * 4;
    if (size < header_min) return 0;

    size_t comp_base = 2;
    size_t decomp_base = 2 + (size_t)count_be * 2;
    uint32_t comp_sum = 0;
    for (uint16_t i = 0; i < count_be; i++) {
        uint16_t cs = rd16be(data + comp_base + (size_t)i * 2);
        uint16_t ds = rd16be(data + decomp_base + (size_t)i * 2);
        if (cs != ds) return 0;
        comp_sum += cs;
    }

    size_t data_area = size - header_min;
    if (comp_sum != data_area) return 0;

    return 1;
}

/* MD5 — minimal implementation for receipt hashing. */
typedef struct { uint32_t s[4]; uint64_t count; uint8_t buf[64]; } MD5Ctx;

static const uint32_t md5_k[64] = {
    0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
    0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
    0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
    0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
    0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
    0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
    0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
    0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};
static const uint8_t md5_r[64] = {
    7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
    5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
    4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
    6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
};

static uint32_t rotl32(uint32_t v, unsigned n) { return (v << n) | (v >> (32 - n)); }

static void md5_transform(MD5Ctx *c, const uint8_t *block) {
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
        b = b + rotl32(a + f + md5_k[i] + m[g], md5_r[i]);
        a = tmp;
    }
    c->s[0] += a; c->s[1] += b; c->s[2] += cc; c->s[3] += d;
}

static void md5_init(MD5Ctx *c) {
    c->s[0] = 0x67452301; c->s[1] = 0xefcdab89;
    c->s[2] = 0x98badcfe; c->s[3] = 0x10325476;
    c->count = 0;
}

static void md5_update(MD5Ctx *c, const uint8_t *data, size_t len) {
    size_t offset = (size_t)(c->count & 63);
    c->count += len;
    for (size_t i = 0; i < len; i++) {
        c->buf[offset++] = data[i];
        if (offset == 64) { md5_transform(c, c->buf); offset = 0; }
    }
}

static void md5_final(MD5Ctx *c, uint8_t out[16]) {
    size_t offset = (size_t)(c->count & 63);
    c->buf[offset++] = 0x80;
    if (offset > 56) {
        memset(c->buf + offset, 0, 64 - offset);
        md5_transform(c, c->buf);
        offset = 0;
    }
    memset(c->buf + offset, 0, 56 - offset);
    uint64_t bits = c->count * 8;
    for (int i = 0; i < 8; i++) c->buf[56 + i] = (uint8_t)(bits >> (i * 8));
    md5_transform(c, c->buf);
    for (int i = 0; i < 4; i++) {
        out[i * 4 + 0] = (uint8_t)(c->s[i]);
        out[i * 4 + 1] = (uint8_t)(c->s[i] >> 8);
        out[i * 4 + 2] = (uint8_t)(c->s[i] >> 16);
        out[i * 4 + 3] = (uint8_t)(c->s[i] >> 24);
    }
}

/* Known DM1 Amiga GRAPHICS.DAT MD5 hashes. */
static const struct {
    uint8_t md5[16];
    DM1_V1_AmigaLang lang;
    DM1_V1_AmigaVersion version;
} dm1_amiga_known[] = {
    /* 6a2f135b53c2220f0251fa103e2a6e7e  DM Amiga 2.0 English */
    {{0x6a,0x2f,0x13,0x5b,0x53,0xc2,0x22,0x0f,0x02,0x51,0xfa,0x10,0x3e,0x2a,0x6e,0x7e},
     DM1_AMIGA_LANG_EN, DM1_AMIGA_VER_2_0},
    /* dd373954b3fb127db7387946131ea322  DM Amiga 2.0 French */
    {{0xdd,0x37,0x39,0x54,0xb3,0xfb,0x12,0x7d,0xb7,0x38,0x79,0x46,0x13,0x1e,0xa3,0x22},
     DM1_AMIGA_LANG_FR, DM1_AMIGA_VER_2_0},
    /* 0679e39da9dcc2e855cb33c6c64ddcb5  DM Amiga 2.0/2.2 German */
    {{0x06,0x79,0xe3,0x9d,0xa9,0xdc,0xc2,0xe8,0x55,0xcb,0x33,0xc6,0xc6,0x4d,0xdc,0xb5},
     DM1_AMIGA_LANG_DE, DM1_AMIGA_VER_2_0},
    /* b35931b55db649a1bd2d415b61b29801  DM Amiga 2.1/2.2 English */
    {{0xb3,0x59,0x31,0xb5,0x5d,0xb6,0x49,0xa1,0xbd,0x2d,0x41,0x5b,0x61,0xb2,0x98,0x01},
     DM1_AMIGA_LANG_EN, DM1_AMIGA_VER_2_1},
    /* 7f9458e4a3972d06e649a6fa85a7f34b  DM Amiga 3.6 Multilanguage */
    {{0x7f,0x94,0x58,0xe4,0xa3,0x97,0x2d,0x06,0xe6,0x49,0xa6,0xfa,0x85,0xa7,0xf3,0x4b},
     DM1_AMIGA_LANG_MULTI, DM1_AMIGA_VER_3_6},
    /* 491ca939f9abb33ceeb26619b841fe91  DM Amiga Demo English */
    {{0x49,0x1c,0xa9,0x39,0xf9,0xab,0xb3,0x3c,0xee,0xb2,0x66,0x19,0xb8,0x41,0xfe,0x91},
     DM1_AMIGA_LANG_EN, DM1_AMIGA_VER_DEMO},
};

int dm1_v1_amiga_graphics_receipt(const uint8_t *data, size_t size,
                                  DM1_V1_AmigaGraphicsReceipt *out) {
    static const uint8_t a36_md5[16] = {
        0x7f,0x94,0x58,0xe4,0xa3,0x97,0x2d,0x06,
        0xe6,0x49,0xa6,0xfa,0x85,0xa7,0xf3,0x4b
    };
    int is_a36_img3;
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    is_a36_img3 = data && size == 458108u &&
                  data[0] == 0x80u && data[1] == 0x01u;
    if (!dm1_v1_amiga_graphics_probe(data, size) && !is_a36_img3) return -1;

    out->is_amiga = 1;
    out->graphic_count = rd16be(data);
    out->file_size = (uint32_t)size;

    MD5Ctx md5;
    md5_init(&md5);
    md5_update(&md5, data, size);
    md5_final(&md5, out->md5);

    out->lang = DM1_AMIGA_LANG_UNKNOWN;
    out->version = DM1_AMIGA_VER_UNKNOWN;
    for (size_t i = 0; i < sizeof(dm1_amiga_known)/sizeof(dm1_amiga_known[0]); i++) {
        if (memcmp(out->md5, dm1_amiga_known[i].md5, 16) == 0) {
            out->lang = dm1_amiga_known[i].lang;
            out->version = dm1_amiga_known[i].version;
            break;
        }
    }

    /* A36M uses the 0x8001 table envelope, but ReDMCSB MEDIA752 routes its
     * item streams to EXPAND.C F0466, not IMAGE2.C's PC IMG3 expander. Bind
     * exact original identity here; the production loader selects F0466 for
     * this receipt instead of treating the shared marker as format proof. */
    if (is_a36_img3) {
        if (memcmp(out->md5, a36_md5, sizeof(a36_md5)) != 0) {
            memset(out, 0, sizeof(*out));
            return -1;
        }
        out->graphic_count = DM1_AMIGA_GRAPHICS_EXPECTED_COUNT;
        out->lang = DM1_AMIGA_LANG_MULTI;
        out->version = DM1_AMIGA_VER_3_6;
    }

    return 0;
}

int dm1_v1_amiga_graphics_decode(const uint8_t *data, size_t size,
                                 uint16_t graphic_index,
                                 uint8_t *indexed_pixels,
                                 size_t pixel_capacity,
                                 uint16_t *out_width,
                                 uint16_t *out_height) {
    if (out_width) *out_width = 0u;
    if (out_height) *out_height = 0u;
    if (!dm1_v1_amiga_graphics_probe(data, size)) return 0;
    return dm1_v1_legacy_graphics_decode(data, size, 1, graphic_index,
                                         indexed_pixels, pixel_capacity,
                                         out_width, out_height);
}

static uint16_t read_be16(const uint8_t *bytes) {
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

int dm1_v1_amiga36_graphic_expand(const uint8_t *stream, size_t stream_size,
                                  uint8_t *indexed_pixels,
                                  size_t pixel_capacity,
                                  uint16_t *out_width,
                                  uint16_t *out_height) {
    size_t source = 0u;
    size_t pixel = 0u;
    size_t visible_total;
    size_t padded_total;
    size_t padded_width;
    uint8_t *padded_pixels = NULL;
    uint16_t width;
    uint16_t height;

    if (out_width) *out_width = 0u;
    if (out_height) *out_height = 0u;
    if (!stream || !indexed_pixels) return 0;
    /* ReDMCSB EXPAND.C F0466 skips optional FF81 prefix words before the
     * big-endian bitmap width and height. */
    while (source + 2u <= stream_size &&
           stream[source] == 0xffu && stream[source + 1u] == 0x81u) {
        source += 2u;
    }
    if (source + 4u > stream_size) return 0;
    width = read_be16(stream + source);
    height = read_be16(stream + source + 2u);
    source += 4u;
    if (width == 0u || height == 0u ||
        (size_t)height > SIZE_MAX / (size_t)width ||
        (visible_total = (size_t)width * (size_t)height) > pixel_capacity ||
        (size_t)width > SIZE_MAX - 15u) {
        return 0;
    }
    /* F0466 stores each output row in 16-pixel units: EXPAND.C rounds the
     * width with (width + 15) & ~15 before expanding. Compact only after the
     * complete padded bitmap has been produced. */
    padded_width = ((size_t)width + 15u) & ~(size_t)15u;
    if ((size_t)height > SIZE_MAX / padded_width ||
        (padded_total = padded_width * (size_t)height) > UINT16_MAX * 4096u) {
        return 0;
    }
    padded_pixels = (uint8_t *)calloc(padded_total, 1u);
    if (!padded_pixels) return 0;
    /* F0466 rounds only the destination row stride. The command cursor uses
     * the unrounded width; row transitions advance by the padded stride. */
    while (pixel < visible_total) {
        uint8_t command;
        uint8_t color;
        size_t count;
        if (source >= stream_size) goto fail;
        command = stream[source++];
        color = (uint8_t)(command & 0x0fu);
        /* EXPAND.C F0466's A?/E? commands advance the destination without
         * changing it. On a fresh bitmap the untouched pixels stay zero.
         * Their length uses the low nibble, optionally extended by bit 6;
         * it is not the normal one/two-byte run length. */
        if ((command & 0x80u) != 0u && (command & 0x10u) == 0u &&
            (command & 0x20u) != 0u) {
            size_t length_code = command & 0x0fu;
            if (source >= stream_size) goto fail;
            if ((command & 0x40u) != 0u) length_code |= 0x10u;
            if (length_code < 0x1du) {
                count = length_code + 1u;
            } else if (length_code == 0x1du) {
                count = (size_t)stream[source++] + 1u;
            } else if (length_code == 0x1eu) {
                count = 256u + (size_t)stream[source++] + 1u;
            } else {
                size_t high;
                size_t low;
                if (source + 2u > stream_size) goto fail;
                high = stream[source++];
                low = stream[source++];
                count = (high << 8u) + low + 1u;
            }
            if (count > visible_total - pixel) goto fail;
            pixel += count;
            continue;
        }
        if ((command & 0x80u) == 0u) {
            count = (size_t)(command >> 4u) + 1u;
        } else {
            if (source >= stream_size) goto fail;
            count = (size_t)stream[source++] + 1u;
            if ((command & 0x40u) != 0u) {
                if (source >= stream_size) goto fail;
                count = (count - 1u) * 256u + (size_t)stream[source++] + 1u;
            }
        }
        if (count == 0u || count > visible_total - pixel) goto fail;
        if ((command & 0x80u) == 0u || (command & 0x10u) == 0u) {
            while (count-- != 0u) {
                size_t destination = (pixel / (size_t)width) * padded_width +
                                    pixel % (size_t)width;
                padded_pixels[destination] = color;
                ++pixel;
            }
        } else if ((command & 0x20u) == 0u) {
            if ((count & 1u) != 0u) {
                size_t destination = (pixel / (size_t)width) * padded_width +
                                     pixel % (size_t)width;
                padded_pixels[destination] = color;
                ++pixel;
                --count;
            }
            while (count != 0u) {
                uint8_t packed;
                if (source >= stream_size) goto fail;
                packed = stream[source++];
                size_t destination = (pixel / (size_t)width) * padded_width +
                                     pixel % (size_t)width;
                padded_pixels[destination] = (uint8_t)(packed >> 4u);
                ++pixel;
                destination = (pixel / (size_t)width) * padded_width +
                              pixel % (size_t)width;
                padded_pixels[destination] = (uint8_t)(packed & 0x0fu);
                ++pixel;
                count -= 2u;
            }
        } else {
            if (pixel < (size_t)width) goto fail;
            while (count-- != 0u) {
                size_t destination = (pixel / (size_t)width) * padded_width +
                                     pixel % (size_t)width;
                padded_pixels[destination] =
                    padded_pixels[destination - padded_width];
                ++pixel;
            }
            /* ReDMCSB EXPAND.C F0466 does not stop after F10F7E copies the
             * previous-line run. Both copy paths (10DE2 and 10E0A) then call
             * F10EF2 once with d2 still holding the command's low nibble,
             * followed by F10E3E. That emits one additional color pixel. */
            if (pixel >= visible_total) goto fail;
            padded_pixels[(pixel / (size_t)width) * padded_width +
                          pixel % (size_t)width] = color;
            ++pixel;
        }
    }
    for (size_t row = 0u; row < height; ++row) {
        memcpy(indexed_pixels + row * (size_t)width,
               padded_pixels + row * padded_width, (size_t)width);
    }
    free(padded_pixels);
    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    return 1;

fail:
    free(padded_pixels);
    return 0;
}
