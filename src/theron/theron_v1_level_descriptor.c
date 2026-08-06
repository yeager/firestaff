#include "theron_v1_level_descriptor.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * UD offset 0x619900, 53 records x 6 bytes. */
static const Theron_LevelDescriptor g_descriptors[THERON_LEVEL_DESCRIPTOR_COUNT] = {
    { 1,  2, 0x0876, 0,   2 }, /*  0 — UD 0x619900 */
    { 1, 10, 0x49E0, 0,  14 }, /*  1 — UD 0x619906 */
    { 1,  6, 0x2BB8, 0,  23 }, /*  2 — UD 0x61990C */
    { 1,  4, 0x2000, 0,  27 }, /*  3 — UD 0x619912 */
    { 1,  8, 0x38E9, 0,  38 }, /*  4 — UD 0x619918 */
    { 1,  3, 0x1184, 0,  46 }, /*  5 — UD 0x61991E */
    { 1,  2, 0x0BD2, 0,  50 }, /*  6 — UD 0x619924 */
    { 1,  1, 0x0400, 0,  51 }, /*  7 — UD 0x61992A */
    { 1,  1, 0x0581, 0,  53 }, /*  8 — UD 0x619930 */
    { 1,  1, 0x0399, 0,  55 }, /*  9 — UD 0x619936 */
    { 1, 11, 0x5674, 0,  69 }, /* 10 — UD 0x61993C */
    { 1,  6, 0x2E7E, 0,  78 }, /* 11 — UD 0x619942 */
    { 1,  3, 0x153B, 0,  83 }, /* 12 — UD 0x619948 */
    { 1,  1, 0x0400, 0,  84 }, /* 13 — UD 0x61994E */
    { 1,  1, 0x03A1, 0,  86 }, /* 14 — UD 0x619954 */
    { 1,  4, 0x2000, 0,  90 }, /* 15 — UD 0x61995A */
    { 1, 28, 0xE000, 0, 118 }, /* 16 — UD 0x619960 */
    { 1,  3, 0x15C8, 0, 121 }, /* 17 — UD 0x619966 */
    { 1,  1, 0x0400, 0, 122 }, /* 18 — UD 0x61996C */
    { 1,  1, 0x0474, 0, 124 }, /* 19 — UD 0x619972 */
    { 1,  8, 0x4000, 0, 132 }, /* 20 — UD 0x619978 */
    { 1,  4, 0x2000, 0, 136 }, /* 21 — UD 0x61997E */
    { 1,  3, 0x13C8, 0, 141 }, /* 22 — UD 0x619984 */
    { 1,  1, 0x0400, 0, 142 }, /* 23 — UD 0x61998A */
    { 1,  1, 0x054B, 0, 144 }, /* 24 — UD 0x619990 */
    { 1,  7, 0x3422, 0, 152 }, /* 25 — UD 0x619996 */
    { 1,  5, 0x25FC, 0, 160 }, /* 26 — UD 0x61999C */
    { 1,  8, 0x39BF, 0, 168 }, /* 27 — UD 0x6199A2 */
    { 1,  7, 0x35E8, 0, 176 }, /* 28 — UD 0x6199A8 */
    { 1,  7, 0x341D, 0, 184 }, /* 29 — UD 0x6199AE */
    { 1,  7, 0x3366, 0, 192 }, /* 30 — UD 0x6199B4 */
    { 1,  5, 0x271D, 0, 200 }, /* 31 — UD 0x6199BA */
    { 1,  7, 0x3071, 0, 208 }, /* 32 — UD 0x6199C0 */
    { 1,  7, 0x3162, 0, 216 }, /* 33 — UD 0x6199C6 */
    { 1,  8, 0x3835, 0, 224 }, /* 34 — UD 0x6199CC */
    { 1,  1, 0x0280, 0, 225 }, /* 35 — UD 0x6199D2 */
    { 1,  1, 0x0280, 0, 226 }, /* 36 — UD 0x6199D8 */
    { 1,  1, 0x02A0, 0, 227 }, /* 37 — UD 0x6199DE */
    { 1,  1, 0x0280, 0, 228 }, /* 38 — UD 0x6199E4 */
    { 1,  1, 0x0280, 0, 229 }, /* 39 — UD 0x6199EA */
    { 1,  1, 0x0280, 0, 230 }, /* 40 — UD 0x6199F0 */
    { 1,  1, 0x0280, 0, 231 }, /* 41 — UD 0x6199F6 */
    { 1,  1, 0x0280, 0, 232 }, /* 42 — UD 0x6199FC */
    { 1,  1, 0x0280, 0, 233 }, /* 43 — UD 0x619A02 */
    { 1,  1, 0x0280, 0, 234 }, /* 44 — UD 0x619A08 */
    { 1,  1, 0x032A, 0, 237 }, /* 45 — UD 0x619A0E */
    { 1,  1, 0x040D, 0, 240 }, /* 46 — UD 0x619A14 */
    { 1,  1, 0x03B0, 0, 243 }, /* 47 — UD 0x619A1A */
    { 1,  1, 0x049A, 0, 246 }, /* 48 — UD 0x619A20 */
    { 1,  1, 0x0368, 0, 249 }, /* 49 — UD 0x619A26 */
    { 1,  1, 0x02C9, 0, 252 }, /* 50 — UD 0x619A2C */
    { 1,  1, 0x0544, 0, 255 }, /* 51 — UD 0x619A32 */
    { 1,  1, 0x023D, 0,   2 }, /* 52 — UD 0x619A38 */
};

const Theron_LevelDescriptor *theron_v1_level_descriptor(unsigned int index) {
    if (index >= THERON_LEVEL_DESCRIPTOR_COUNT) return NULL;
    return &g_descriptors[index];
}

size_t theron_v1_level_descriptor_count(void) {
    return THERON_LEVEL_DESCRIPTOR_COUNT;
}

int theron_v1_level_descriptor_read_us_track02(
    const uint8_t *user_data,
    size_t user_data_size,
    Theron_LevelDescriptor *out,
    size_t out_count) {
    size_t i;

    if (!user_data || !out || out_count < THERON_LEVEL_DESCRIPTOR_COUNT ||
        THERON_LEVEL_DESCRIPTOR_USER_DATA_OFFSET > user_data_size ||
        THERON_LEVEL_DESCRIPTOR_BYTES >
            user_data_size - THERON_LEVEL_DESCRIPTOR_USER_DATA_OFFSET) {
        return 0;
    }

    for (i = 0; i < THERON_LEVEL_DESCRIPTOR_COUNT; ++i) {
        const uint8_t *p = user_data +
            THERON_LEVEL_DESCRIPTOR_USER_DATA_OFFSET + i * 6u;
        Theron_LevelDescriptor parsed = {
            p[0], p[1], (uint16_t)p[2] | ((uint16_t)p[3] << 8), p[4], p[5]
        };

        /* The table is source-locked to the US BIN receipt above.  Reject a
         * plausible-looking table from an unproven offset or another dump. */
        if (parsed.flags != g_descriptors[i].flags ||
            parsed.sector_count != g_descriptors[i].sector_count ||
            parsed.data_size != g_descriptors[i].data_size ||
            parsed.reserved != g_descriptors[i].reserved ||
            parsed.cumulative_sector_offset !=
                g_descriptors[i].cumulative_sector_offset) {
            return 0;
        }
        out[i] = parsed;
    }
    return 1;
}
