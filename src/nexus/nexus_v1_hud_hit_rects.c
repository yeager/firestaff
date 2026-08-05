
#include "nexus_v1_hud_hit_rects.h"

/* Ring menu hit-test rectangles from DM.BIN yam\menuctrl.c at 0x038000.
 * 40 entries of (x1,y1,x2,y2) as screen pixel rectangles.
 * Empty entries have all-zero coordinates and are skipped by hit testing.
 * Source: DM.BIN 0x038000, Saturn binary. */

static const Nexus_HitRect g_hit_rects[NEXUS_HIT_RECT_COUNT] = {
    /* [ 0] */ {294, 104, 310, 120},
    /* [ 1] */ {277,  72, 293,  88},
    /* [ 2] */ {277,  91, 293, 107},
    /* [ 3] */ {277, 110, 293, 126},
    /* [ 4] */ {277, 136, 293, 152},
    /* [ 5] */ {253,  80, 269,  96},
    /* [ 6] */ {253, 136, 269, 152},
    /* [ 7] */ {144,  72, 240, 200},
    /* [ 8] */ {251,  70, 295, 137},
    /* [ 9] */ {  0,   0,   0,   0},
    /* [10] */ { 14,   6,  74,  45},
    /* [11] */ {  0,   0,   0,   0},
    /* [12] */ {144, 136, 240, 200},
    /* [13] */ {260, 104, 276, 120},
    /* [14] */ {294, 104, 310, 120},
    /* [15] */ {277,  72, 293,  88},
    /* [16] */ {277,  91, 293, 107},
    /* [17] */ {277, 110, 293, 126},
    /* [18] */ {277, 136, 293, 152},
    /* [19] */ {253,  80, 269,  96},
    /* [20] */ {253, 136, 269, 152},
    /* [21] */ {251,  70, 295, 137},
    /* [22] */ {  0,   0,   0,   0},
    /* [23] */ {144,  96, 240, 208},
    /* [24] */ {  0,   0,   0,   0},
    /* [25] */ { 27, 142, 129, 207},
    /* [26] */ {  0,   0,   0,   0},
    /* [27] */ {  8,  48, 136, 136},
    /* [28] */ {146,  58, 247,  71},
    /* [29] */ {  0,  47, 320, 213},
    /* [30] */ {  0,   0,   0,   0},
    /* [31] */ {148,  84, 202, 131},
    /* [32] */ {204,  84, 250, 131},
    /* [33] */ {253,  84, 299, 131},
    /* [34] */ {187, 138, 229, 188},
    /* [35] */ {192, 152, 211, 171},
    /* [36] */ {  0,   0,   0,   0},
    /* [37] */ {144, 120, 240, 204},
    /* [38] */ {260, 104, 276, 120},
    /* [39] */ {294, 104, 310, 120},
};

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

int nexus_v1_hud_hit_rects(const Nexus_HitRect **out) {
    if (out) *out = g_hit_rects;
    return NEXUS_HIT_RECT_COUNT;
}

int nexus_v1_hud_hit_rects_parse_dm_bin(
    const uint8_t *data,
    size_t data_size,
    Nexus_HitRect *out,
    size_t out_capacity,
    size_t *out_count)
{
    size_t i;
    size_t required = (size_t)NEXUS_HIT_RECT_COUNT *
                      NEXUS_HIT_RECT_ENTRY_BYTES;

    if (out_count) *out_count = 0U;
    if (!data || !out || out_capacity < NEXUS_HIT_RECT_COUNT ||
        data_size < (size_t)NEXUS_HIT_RECT_DM_BIN_OFFSET + required) {
        return -1;
    }
    for (i = 0U; i < NEXUS_HIT_RECT_COUNT; ++i) {
        const uint8_t *entry = data + NEXUS_HIT_RECT_DM_BIN_OFFSET +
                               i * NEXUS_HIT_RECT_ENTRY_BYTES;
        out[i].x1 = (int16_t)read_be16(entry + 0U);
        out[i].y1 = (int16_t)read_be16(entry + 2U);
        out[i].x2 = (int16_t)read_be16(entry + 4U);
        out[i].y2 = (int16_t)read_be16(entry + 6U);
        if (out[i].x2 < out[i].x1 || out[i].y2 < out[i].y1) {
            if (out_count) *out_count = 0U;
            return -2;
        }
    }
    if (out_count) *out_count = NEXUS_HIT_RECT_COUNT;
    return 0;
}

int nexus_v1_hud_hit_test(int screen_x, int screen_y) {
    int i;
    for (i = 0; i < NEXUS_HIT_RECT_COUNT; i++) {
        const Nexus_HitRect *r = &g_hit_rects[i];
        if (r->x1 == 0 && r->y1 == 0 && r->x2 == 0 && r->y2 == 0)
            continue;
        if (screen_x >= r->x1 && screen_x < r->x2 &&
            screen_y >= r->y1 && screen_y < r->y2)
            return i;
    }
    return -1;
}
