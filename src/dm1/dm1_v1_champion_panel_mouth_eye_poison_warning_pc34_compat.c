#include "dm1/dm1_v1_champion_panel_mouth_eye_poison_warning_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_CPMEP_COLOR_RED = 8,
    DM1_V1_CPMEP_COLOR_YELLOW = 11,
    DM1_V1_CPMEP_COLOR_FOOD_NORMAL = 5,
    DM1_V1_CPMEP_COLOR_WATER_NORMAL = 14
};

static const DM1_V1_ChampionPanelMouthEyePoisonWarningEvidencePc34Compat
    s_dm1_v1_cpmep_evidence = {
        1,
        "CHAMDRAW.C:F0292_CHAMPION_DrawState:755-759; "
        "CHAMDRAW.C:F0292_CHAMPION_DrawState:898-935; "
        "CHAMDRAW.C:F0292_CHAMPION_DrawState:1060-1078",
        "PANEL.C:F0344_INVENTORY_DrawPanel_FoodOrWaterBar:1519-1526; "
        "PANEL.C:F0345_INVENTORY_DrawPanel_FoodWaterPoisoned:1579-1615",
        "PANEL.C:F0355_INVENTORY_Toggle_CPSE:2299-2363; "
        "PANEL.C:F0355_INVENTORY_Toggle_CPSE:2375-2377; "
        "CHAMPION.C:F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:677-711",
        "BLTSHRNK.C:F0129_VIDEO_BlitShrinkWithPaletteChanges:202-230; "
        "FLIPHORI.C:F0130_VIDEO_FlipHorizontal:12-29; "
        "BLITMASK.C:F0133_VIDEO_BlitBoxFilledWithMaskedBitmap:30-72; "
        "BASE.C:F0658_BlitBitmapIndexToZoneIndexWithTransparency:1341-1364",
        "DEFS.H:C04/C08/C12_COLOR_AND_C030_C034_GRAPHICS:2082-2090,2190-2194; "
        "DEFS.H:C500_C502_AND_C545_C546_ZONES:3869-3871,3914-3915"
    };

const DM1_V1_ChampionPanelMouthEyePoisonWarningEvidencePc34Compat *
DM1_V1_ChampionPanelMouthEyePoisonWarning_EvidencePc34Compat(void)
{
    return &s_dm1_v1_cpmep_evidence;
}

void DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat *state)
{
    int champion_index;
    int statistic_index;

    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->contract_only = 1;
    state->leader_champion_index = 0;
    state->active_inventory_champion_index = 0;
    for (champion_index = 0;
         champion_index < DM1_V1_CPMEP_CHAMPION_COUNT;
         ++champion_index) {
        state->champions[champion_index].current_health = 100;
        state->champions[champion_index].food = 1024;
        state->champions[champion_index].water = 1024;
        state->champions[champion_index].poison_event_count = 0;
        for (statistic_index = 0;
             statistic_index < DM1_V1_CPMEP_STATISTIC_COUNT;
             ++statistic_index) {
            state->champions[champion_index].statistic_current[statistic_index] = 50;
            state->champions[champion_index].statistic_maximum[statistic_index] = 50;
        }
    }
}

static int dm1_v1_cpmep_valid_index(int champion_index)
{
    return champion_index >= 0 && champion_index < DM1_V1_CPMEP_CHAMPION_COUNT;
}

static int dm1_v1_cpmep_first_low_statistic(
    const DM1_V1_ChampionPanelMouthEyePoisonWarningChampionPc34Compat *champion)
{
    int statistic_index;

    for (statistic_index = 0;
         statistic_index < DM1_V1_CPMEP_STATISTIC_COUNT;
         ++statistic_index) {
        if (champion->statistic_current[statistic_index] <
            champion->statistic_maximum[statistic_index]) {
            return statistic_index;
        }
    }
    return -1;
}

static int dm1_v1_cpmep_food_or_water_color(int amount, int normal_color)
{
    if (amount < -512) {
        return DM1_V1_CPMEP_COLOR_RED;
    }
    if (amount < 0) {
        return DM1_V1_CPMEP_COLOR_YELLOW;
    }
    return normal_color;
}

static void dm1_v1_cpmep_fill_warning_flash(int critical,
                                            int *drawn,
                                            int *flashes,
                                            int *dark_palette,
                                            int *lit_palette)
{
    if (drawn) {
        *drawn = 1;
    }
    if (flashes) {
        *flashes = critical ? 1 : 0;
    }
    if (dark_palette) {
        *dark_palette = critical ? DM1_V1_CPMEP_COLOR_RED_FLASH_DARK : -1;
    }
    if (lit_palette) {
        *lit_palette = critical ? DM1_V1_CPMEP_COLOR_RED_FLASH_LIT : -1;
    }
}

int DM1_V1_ChampionPanelMouthEyePoisonWarning_BuildPc34Compat(
    const DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat *state,
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat *out_result)
{
    const DM1_V1_ChampionPanelMouthEyePoisonWarningChampionPc34Compat *champion;
    const DM1_V1_ChampionPanelMouthEyePoisonWarningChampionPc34Compat *leader;
    int selected_index;
    int mouth_warning;
    int first_low_statistic;
    int food_critical;
    int water_critical;
    int poison_critical;

    if (!state || !out_result || !state->contract_only ||
        !dm1_v1_cpmep_valid_index(state->leader_champion_index)) {
        return 0;
    }

    memset(out_result, 0, sizeof(*out_result));
    out_result->contract_only = 1;
    out_result->first_low_statistic_index = -1;

    if (!state->inventory_open) {
        out_result->valid = 1;
        out_result->selected_champion_index = state->leader_champion_index;
        return 1;
    }

    if (!dm1_v1_cpmep_valid_index(state->active_inventory_champion_index)) {
        return 0;
    }

    selected_index = state->active_inventory_champion_index;
    champion = &state->champions[selected_index];
    leader = &state->champions[state->leader_champion_index];
    mouth_warning = (champion->food < 0) ||
                    (champion->water < 0) ||
                    (champion->poison_event_count > 0);
    first_low_statistic = dm1_v1_cpmep_first_low_statistic(champion);
    food_critical = champion->food < -512;
    water_critical = champion->water < -512;
    poison_critical = champion->poison_event_count > 0;

    out_result->valid = 1;
    out_result->selected_champion_index = selected_index;
    out_result->leader_index_switched_to_inventory_champion =
        selected_index != state->leader_champion_index;
    out_result->follows_inventory_champion_not_party_leader =
        selected_index != state->leader_champion_index &&
        ((champion->food < 0) != (leader->food < 0) ||
         (champion->water < 0) != (leader->water < 0) ||
         (champion->poison_event_count > 0) != (leader->poison_event_count > 0));

    out_result->mouth_border_drawn = 1;
    out_result->mouth_warning_border_drawn = mouth_warning ? 1 : 0;
    out_result->mouth_zone = DM1_V1_CPMEP_ZONE_MOUTH;
    out_result->mouth_border_graphic = mouth_warning
        ? DM1_V1_CPMEP_GFX_SLOT_WOUNDED
        : DM1_V1_CPMEP_GFX_SLOT_NORMAL;
    out_result->mouth_transparent_color = DM1_V1_CPMEP_COLOR_DARKEST_GRAY;

    out_result->eye_border_drawn = 1;
    out_result->eye_warning_border_drawn = first_low_statistic >= 0;
    out_result->eye_zone = DM1_V1_CPMEP_ZONE_EYE;
    out_result->eye_border_graphic = first_low_statistic >= 0
        ? DM1_V1_CPMEP_GFX_SLOT_WOUNDED
        : DM1_V1_CPMEP_GFX_SLOT_NORMAL;
    out_result->eye_transparent_color = DM1_V1_CPMEP_COLOR_DARKEST_GRAY;
    out_result->first_low_statistic_index = first_low_statistic;

    out_result->panel_drawn = 1;
    out_result->food_label_graphic = DM1_V1_CPMEP_GFX_FOOD_LABEL;
    out_result->food_label_zone = DM1_V1_CPMEP_ZONE_FOOD;
    out_result->food_bar_color =
        dm1_v1_cpmep_food_or_water_color(champion->food,
                                         DM1_V1_CPMEP_COLOR_FOOD_NORMAL);
    dm1_v1_cpmep_fill_warning_flash(food_critical,
                                    &out_result->food_warning_border_drawn,
                                    &out_result->food_warning_border_flashes,
                                    &out_result->food_warning_palette_dark,
                                    &out_result->food_warning_palette_lit);

    out_result->water_label_graphic = DM1_V1_CPMEP_GFX_WATER_LABEL;
    out_result->water_label_zone = DM1_V1_CPMEP_ZONE_WATER;
    out_result->water_bar_color =
        dm1_v1_cpmep_food_or_water_color(champion->water,
                                         DM1_V1_CPMEP_COLOR_WATER_NORMAL);
    dm1_v1_cpmep_fill_warning_flash(water_critical,
                                    &out_result->water_warning_border_drawn,
                                    &out_result->water_warning_border_flashes,
                                    &out_result->water_warning_palette_dark,
                                    &out_result->water_warning_palette_lit);

    out_result->poison_label_graphic = DM1_V1_CPMEP_GFX_POISONED_LABEL;
    out_result->poison_label_zone = DM1_V1_CPMEP_ZONE_POISONED;
    out_result->poison_label_drawn = poison_critical ? 1 : 0;
    dm1_v1_cpmep_fill_warning_flash(poison_critical,
                                    &out_result->poison_warning_border_drawn,
                                    &out_result->poison_warning_border_flashes,
                                    &out_result->poison_warning_palette_dark,
                                    &out_result->poison_warning_palette_lit);

    return 1;
}
