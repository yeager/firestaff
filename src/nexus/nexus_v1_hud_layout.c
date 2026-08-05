#include "nexus_v1_hud_layout.h"

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

int nexus_v1_hud_layout_parse_dm_bin(
    const uint8_t *data,
    size_t data_size,
    Nexus_HudElement *out,
    size_t out_capacity,
    size_t *out_count)
{
    size_t i;
    size_t required = (size_t)NEXUS_HUD_LAYOUT_ENTRY_COUNT *
                      NEXUS_HUD_LAYOUT_ENTRY_BYTES;

    if (out_count) *out_count = 0U;
    if (!data || !out || out_capacity < NEXUS_HUD_LAYOUT_ENTRY_COUNT ||
        data_size < (size_t)NEXUS_HUD_LAYOUT_DM_BIN_OFFSET + required) {
        return -1;
    }
    for (i = 0U; i < NEXUS_HUD_LAYOUT_ENTRY_COUNT; ++i) {
        const uint8_t *entry = data + NEXUS_HUD_LAYOUT_DM_BIN_OFFSET +
                               i * NEXUS_HUD_LAYOUT_ENTRY_BYTES;
        /* DM.BIN yam\\menuctrl.c stores (element, 0, x, y) as BE16. */
        if (read_be16(entry + 2U) != 0U) return -2;
        out[i].element_id = read_be16(entry + 0U);
        out[i].x = read_be16(entry + 4U);
        out[i].y = read_be16(entry + 6U);
    }
    if (out_count) *out_count = NEXUS_HUD_LAYOUT_ENTRY_COUNT;
    return 0;
}
