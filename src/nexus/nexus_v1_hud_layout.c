#include "nexus_v1_hud_layout.h"

/* DM.BIN yam\\menuctrl.c stores these group terminators in the authenticated
 * 80-entry region.  Keeping the positions here makes a shifted or synthetic
 * table fail admission instead of being exposed as a HUD layout. */
static const size_t g_hud_layout_sentinel_indices[NEXUS_HUD_LAYOUT_SENTINEL_COUNT] = {
    19U, 24U, 26U, 28U, 30U, 51U, 63U, 73U, 74U
};

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
    size_t sentinel;
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
    for (sentinel = 0U; sentinel < NEXUS_HUD_LAYOUT_SENTINEL_COUNT;
         ++sentinel) {
        const size_t index = g_hud_layout_sentinel_indices[sentinel];
        if (out[index].element_id != 0xffffU || out[index].x != 0U ||
            out[index].y != 0U) return -3;
    }
    for (i = 0U; i < NEXUS_HUD_LAYOUT_ENTRY_COUNT; ++i) {
        size_t sentinel_index;
        if (out[i].element_id != 0xffffU) continue;
        for (sentinel_index = 0U;
             sentinel_index < NEXUS_HUD_LAYOUT_SENTINEL_COUNT;
             ++sentinel_index) {
            if (g_hud_layout_sentinel_indices[sentinel_index] == i) break;
        }
        if (sentinel_index == NEXUS_HUD_LAYOUT_SENTINEL_COUNT) return -4;
    }
    if (out_count) *out_count = NEXUS_HUD_LAYOUT_ENTRY_COUNT;
    return 0;
}
