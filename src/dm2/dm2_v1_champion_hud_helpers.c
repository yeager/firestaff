#include "dm2_v1_champion_hud_helpers.h"

#include <string.h>

static void dm2_champion_hud_receipt_begin(
    DM2_V1_ChampionHudReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_champion_hud_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

static int16_t dm2_clamp_i16(int value, int minimum, int maximum)
{
    if (value < minimum) {
        return (int16_t)minimum;
    }
    if (value > maximum) {
        return (int16_t)maximum;
    }
    return (int16_t)value;
}

void dm2_v1_champion_hud_receipt_clear(
    DM2_V1_ChampionHudReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int16_t dm2_v1_PROCESS_ITEM_BONUS(
    const DM2_V1_ProcessItemBonusInput *input,
    DM2_V1_ChampionHudReceipt *out_receipt)
{
    int value;
    int result;

    dm2_champion_hud_receipt_begin(out_receipt,
                                   "PROCESS_ITEM_BONUS",
                                   "SKWIN/SkWinCore.cpp:5277");
    if (!input || input->item_ref == DM2_V1_CHAMPION_HUD_NULL_ITEM ||
        input->minimum_value > input->maximum_value) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    value = (int)input->current_value + (int)input->item_bonus;
    result = dm2_clamp_i16(value,
                           input->minimum_value,
                           input->maximum_value);
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = result;
        out_receipt->dirty = result != input->current_value;
    }
    return (int16_t)result;
}

uint16_t dm2_v1_QUERY_PLAYER_SKILL_LV(
    const DM2_V1_PlayerSkillInput *input,
    DM2_V1_ChampionHudReceipt *out_receipt)
{
    uint32_t xp;
    uint16_t level;
    uint16_t maximum;

    dm2_champion_hud_receipt_begin(out_receipt,
                                   "QUERY_PLAYER_SKILL_LV",
                                   "SKWIN/SkWinCore.cpp:8381");
    if (!input) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    maximum = input->maximum_level;
    if (maximum == 0u || maximum > DM2_V1_CHAMPION_HUD_MAX_SKILL_LEVEL) {
        maximum = DM2_V1_CHAMPION_HUD_MAX_SKILL_LEVEL;
    }
    level = input->base_level;
    if (level > maximum) {
        level = maximum;
    }
    xp = input->experience;
    while (level < maximum && xp >= ((uint32_t)(level + 1u) * 1024u)) {
        xp -= (uint32_t)(level + 1u) * 1024u;
        ++level;
    }
    if (input->temporary_bonus > 0u) {
        uint32_t boosted = (uint32_t)level + input->temporary_bonus;
        level = boosted > maximum ? maximum : (uint16_t)boosted;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = (int)level;
        out_receipt->dirty = input->temporary_bonus > 0u;
    }
    return level;
}

int dm2_v1_REFRESH_PLAYER_STAT_DISP(
    const DM2_V1_PlayerStatDisplayInput *input,
    DM2_V1_PlayerStatDisplay *out_display,
    DM2_V1_ChampionHudReceipt *out_receipt)
{
    int percent;
    int dirty;

    if (out_display) {
        memset(out_display, 0, sizeof(*out_display));
    }
    dm2_champion_hud_receipt_begin(out_receipt,
                                   "REFRESH_PLAYER_STAT_DISP",
                                   "SKWIN/SkWinCore.cpp:14573");
    if (!input || !out_display || input->maximum_value <= 0 ||
        input->current_value < 0 || input->bar_color < 0) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    out_display->maximum_value = input->maximum_value;
    out_display->current_value =
        dm2_clamp_i16(input->current_value, 0, input->maximum_value);
    out_display->bar_color = input->bar_color;
    percent = ((int)out_display->current_value * 100) /
              (int)out_display->maximum_value;
    out_display->percent = (uint8_t)dm2_clamp_i16(percent, 0, 100);
    out_display->redraw_value =
        out_display->current_value != input->previous_current_value;
    out_display->redraw_maximum =
        out_display->maximum_value != input->previous_maximum_value;
    out_display->redraw_bar =
        out_display->redraw_value || out_display->redraw_maximum;
    dirty = out_display->redraw_value ||
            out_display->redraw_maximum ||
            out_display->redraw_bar;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = out_display->percent;
        out_receipt->dirty = dirty;
    }
    return 1;
}

int16_t dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(
    int16_t gdat_value,
    int16_t default_color,
    DM2_V1_BarColorReceipt *out_receipt)
{
    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (gdat_value >= 0) {
        int16_t color = (int16_t)(256 + gdat_value);
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->color = color;
            out_receipt->gdat_override = 1;
        }
        return color;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->color = default_color;
        out_receipt->gdat_override = 0;
    }
    return default_color;
}

int16_t dm2_v1_QUERY_3STAT_BAR_COLOR(
    int16_t gdat_value,
    int16_t default_color,
    DM2_V1_BarColorReceipt *out_receipt)
{
    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (gdat_value >= 0) {
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->color = gdat_value;
            out_receipt->gdat_override = 1;
        }
        return gdat_value;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->color = default_color;
        out_receipt->gdat_override = 0;
    }
    return default_color;
}

const char *dm2_v1_champion_hud_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp PROCESS_ITEM_BONUS:5277 "
           "QUERY_PLAYER_SKILL_LV:8381 REFRESH_PLAYER_STAT_DISP:14573 "
           "QUERY_FOOD_WATER_BAR_COLOR:13194 QUERY_3STAT_BAR_COLOR:13203; "
           "bounded champion/HUD receipts only.";
}
