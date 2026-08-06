#include "nexus_v1_hud_hit_rects.h"

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
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
        int empty = 0;
        out[i].x1 = (int16_t)read_be16(entry + 0U);
        out[i].y1 = (int16_t)read_be16(entry + 2U);
        out[i].x2 = (int16_t)read_be16(entry + 4U);
        out[i].y2 = (int16_t)read_be16(entry + 6U);
        empty = out[i].x1 == 0 && out[i].y1 == 0 &&
                out[i].x2 == 0 && out[i].y2 == 0;
        /* DM.BIN's rectangles are Saturn screen coordinates, not host-space
         * placeholders. The retail display envelope is 320x224. Exact
         * zeroed entries are unused retail table slots; any other degenerate
         * rectangle is malformed and must not become a HUD region. */
        if ((!empty && (out[i].x2 <= out[i].x1 ||
                        out[i].y2 <= out[i].y1)) ||
            out[i].x1 < 0 || out[i].y1 < 0 ||
            out[i].x2 > 320 || out[i].y2 > 224) {
            if (out_count) *out_count = 0U;
            return -2;
        }
    }
    if (out_count) *out_count = NEXUS_HIT_RECT_COUNT;
    return 0;
}
