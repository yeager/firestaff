#include "nexus_v1_champion_panel.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static int parse_rects(const uint8_t *data, size_t data_size,
                       size_t offset, Nexus_PanelRect *out, size_t count)
{
    size_t i;
    size_t bytes = count * NEXUS_PANEL_RECT_BYTES;

    if (!data || !out || offset > data_size || bytes > data_size - offset)
        return -1;
    for (i = 0U; i < count; ++i) {
        const uint8_t *record = data + offset + i * NEXUS_PANEL_RECT_BYTES;
        out[i].x1 = (int)read_be16(record + 0U);
        out[i].y1 = (int)read_be16(record + 2U);
        out[i].x2 = (int)read_be16(record + 4U);
        out[i].y2 = (int)read_be16(record + 6U);
        out[i].action = (int)read_be16(record + 8U);
        out[i].param = (int)(int16_t)read_be16(record + 10U);
        if (out[i].x2 <= out[i].x1 || out[i].y2 <= out[i].y1)
            return -1;
    }
    return 0;
}

int nexus_v1_champion_panel_parse_dm_bin(
    const uint8_t *data, size_t data_size,
    Nexus_PanelRect stat_bars[NEXUS_STAT_BAR_RECT_COUNT],
    Nexus_PanelRect inv_slots[NEXUS_INV_SLOT_RECT_COUNT],
    Nexus_PanelRect equip_slots[NEXUS_EQUIP_SLOT_RECT_COUNT])
{
    if (!stat_bars || !inv_slots || !equip_slots)
        return -1;
    if (parse_rects(data, data_size, NEXUS_PANEL_STAT_OFFSET,
                    stat_bars, NEXUS_STAT_BAR_RECT_COUNT) != 0 ||
        parse_rects(data, data_size, NEXUS_PANEL_INV_OFFSET,
                    inv_slots, NEXUS_INV_SLOT_RECT_COUNT) != 0 ||
        parse_rects(data, data_size, NEXUS_PANEL_EQUIP_OFFSET,
                    equip_slots, NEXUS_EQUIP_SLOT_RECT_COUNT) != 0)
        return -1;
    return 0;
}

const Nexus_PanelRect* nexus_v1_panel_hit_test(
    const Nexus_PanelRect* rects, int count, int x, int y)
{
    int i;
    if (!rects || count <= 0) return NULL;
    for (i = 0; i < count; ++i) {
        if (x >= rects[i].x1 && x < rects[i].x2 &&
            y >= rects[i].y1 && y < rects[i].y2)
            return &rects[i];
    }
    return NULL;
}
