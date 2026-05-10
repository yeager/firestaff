#ifndef FIRESTAFF_DM1_V1_VIEWPORT_STATUS_BAR_LAYOUT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_STATUS_BAR_LAYOUT_PC34_COMPAT_H

/*
 * DM1 V1 viewport status bar / champion status display layout gate.
 *
 * Primary source: ReDMCSB_WIP20210206 Toolchains/Common/Source.
 * The invariants here intentionally cover only source-visible layout:
 * champion HP/stamina/mana status bars, inventory-panel health value
 * text rows, skill-level text row placement, and the absence of an
 * original XP progress bar in this viewport/status UI.
 */

#define DM1_V1_PARTY_CHAMPION_COUNT 4
#define DM1_V1_STATUS_BOX_SPACING 69

#define DM1_V1_STATUS_BAR_STAT_COUNT 3
#define DM1_V1_STATUS_BAR_HEALTH 0
#define DM1_V1_STATUS_BAR_STAMINA 1
#define DM1_V1_STATUS_BAR_MANA 2
#define DM1_V1_STATUS_BAR_WIDTH 4
#define DM1_V1_STATUS_BAR_HEIGHT 25
#define DM1_V1_STATUS_BAR_TOP_Y 2
#define DM1_V1_STATUS_BAR_FIRST_X 46
#define DM1_V1_STATUS_BAR_STAT_SPACING 7
#define DM1_V1_STATUS_BAR_GRAPH_FIRST_ZONE 187
#define DM1_V1_STATUS_BAR_VALUE_FIRST_ZONE 195
#define DM1_V1_STATUS_BAR_BLANK_COLOR 12
#define DM1_V1_CHAMPION_COLOR_0 7
#define DM1_V1_CHAMPION_COLOR_1 11
#define DM1_V1_CHAMPION_COLOR_2 8
#define DM1_V1_CHAMPION_COLOR_3 14

#define DM1_V1_HEALTH_VALUE_ZONE 550
#define DM1_V1_STAMINA_VALUE_SOURCE_ZONE 551
#define DM1_V1_MANA_VALUE_SOURCE_ZONE 552
#define DM1_V1_HEALTH_VALUE_X 95
#define DM1_V1_HEALTH_VALUE_Y 116
#define DM1_V1_STAMINA_VALUE_X 95
#define DM1_V1_STAMINA_VALUE_Y 124
#define DM1_V1_MANA_VALUE_X 95
#define DM1_V1_MANA_VALUE_Y 132

#define DM1_V1_SKILL_VALUE_ZONE 557
#define DM1_V1_SKILL_VALUE_REL_X 28
#define DM1_V1_SKILL_VALUE_REL_Y 6
#define DM1_V1_SKILL_LINE_HEIGHT 7
#define DM1_V1_BASE_SKILL_COUNT 4
#define DM1_V1_SKILL_LEVEL_MIN_VISIBLE 2
#define DM1_V1_SKILL_LEVEL_MAX_CLAMP 16
#define DM1_V1_SKILL_TEXT_COLOR_NORMAL 13
#define DM1_V1_SKILL_TEXT_COLOR_RECENT_UPGRADE 7

#define DM1_V1_XP_BAR_PRESENT 0
#define DM1_V1_XP_BAR_WIDTH 0
#define DM1_V1_XP_BAR_HEIGHT 0

static inline int dm1_v1_status_bar_graph_zone_id(int champion_slot) {
    if (champion_slot < 0 || champion_slot >= DM1_V1_PARTY_CHAMPION_COUNT) return 0;
    return DM1_V1_STATUS_BAR_GRAPH_FIRST_ZONE + champion_slot;
}

static inline int dm1_v1_status_bar_value_zone_id(int champion_slot, int stat_index) {
    if (champion_slot < 0 || champion_slot >= DM1_V1_PARTY_CHAMPION_COUNT) return 0;
    if (stat_index < 0 || stat_index >= DM1_V1_STATUS_BAR_STAT_COUNT) return 0;
    return DM1_V1_STATUS_BAR_VALUE_FIRST_ZONE + stat_index * DM1_V1_PARTY_CHAMPION_COUNT + champion_slot;
}

static inline int dm1_v1_status_bar_x(int champion_slot, int stat_index) {
    if (champion_slot < 0 || champion_slot >= DM1_V1_PARTY_CHAMPION_COUNT) return -1;
    if (stat_index < 0 || stat_index >= DM1_V1_STATUS_BAR_STAT_COUNT) return -1;
    return champion_slot * DM1_V1_STATUS_BOX_SPACING + DM1_V1_STATUS_BAR_FIRST_X + stat_index * DM1_V1_STATUS_BAR_STAT_SPACING;
}

static inline int dm1_v1_status_bar_fill_height(int current, int maximum) {
    long scaled;
    if (current <= 0 || maximum <= 0) return 0;
    if (current >= maximum) return DM1_V1_STATUS_BAR_HEIGHT;
    scaled = ((long)DM1_V1_STATUS_BAR_HEIGHT * (long)current) / (long)maximum;
    if (scaled < 1) scaled = 1;
    return (int)scaled;
}

static inline int dm1_v1_status_bar_blank_height(int current, int maximum) {
    return DM1_V1_STATUS_BAR_HEIGHT - dm1_v1_status_bar_fill_height(current, maximum);
}

static inline int dm1_v1_skill_level_text_y(int visible_line_index) {
    if (visible_line_index < 0 || visible_line_index >= DM1_V1_BASE_SKILL_COUNT) return -1;
    return DM1_V1_SKILL_VALUE_REL_Y + visible_line_index * DM1_V1_SKILL_LINE_HEIGHT;
}

#endif
