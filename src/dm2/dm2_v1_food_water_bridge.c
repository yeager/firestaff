#include "dm2_v1_food_water_bridge.h"
#include "dm2_v1_champion_hud_helpers.h"

#include <string.h>

int dm2_v1_food_water_bridge_compute(
    const DM2_V1_FoodWaterInput *input,
    int16_t gdat_food_color_override,
    int16_t gdat_water_color_override,
    DM2_V1_FoodWaterReceipt *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    if (!input) return 0;

    out->valid = 1;

    int16_t food = input->food;
    if (food < -1024) food = -1024;
    if (food > DM2_V1_FOOD_WATER_MAX_VALUE) food = DM2_V1_FOOD_WATER_MAX_VALUE;
    int food_range = DM2_V1_FOOD_WATER_MAX_VALUE + 1024;
    out->food_pct = (uint8_t)(((int)(food + 1024) * 100) / food_range);

    int16_t water = input->water;
    if (water < -1024) water = -1024;
    if (water > DM2_V1_FOOD_WATER_MAX_VALUE) water = DM2_V1_FOOD_WATER_MAX_VALUE;
    out->water_pct = (uint8_t)(((int)(water + 1024) * 100) / food_range);

    out->food_color = dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(
        gdat_food_color_override,
        DM2_V1_CHAMPION_HUD_DEFAULT_FOOD_COLOR, NULL);
    out->water_color = dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(
        gdat_water_color_override,
        DM2_V1_CHAMPION_HUD_DEFAULT_WATER_COLOR, NULL);

    out->show_poison = input->poisoned != 0;
    if (out->show_poison && input->poison > 0) {
        int poison_range = 0xC00;
        int16_t p = input->poison;
        if (p > poison_range) p = (int16_t)poison_range;
        out->poison_pct = (uint8_t)(((int)p * 100) / poison_range);
        out->poison_color = 8;
    }

    return 1;
}

const char *dm2_v1_food_water_bridge_source_evidence(void)
{
    return "skproject SKULLWIN/c_gui_draw.cpp DM2_DRAW_FOOD_WATER_POISON_PANEL:1215; "
           "SKWIN/SkWinCore.cpp QUERY_FOOD_WATER_BAR_COLOR:13194; "
           "bridges food/water/poison stats to bar percentages.";
}
