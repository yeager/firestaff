#ifndef FIRESTAFF_DM2_V1_FOOD_WATER_BRIDGE_H
#define FIRESTAFF_DM2_V1_FOOD_WATER_BRIDGE_H

/*
 * DM2 Food/Water/Poison Panel Bridge.
 *
 * Computes food/water bar percentages and poison state from raw hero
 * stats, resolving bar colors via QUERY_FOOD_WATER_BAR_COLOR.
 *
 * Source: skproject SKULLWIN/c_gui_draw.cpp:1215 DM2_DRAW_FOOD_WATER_POISON_PANEL
 *         skproject SKWIN/SkWinCore.cpp:13194 QUERY_FOOD_WATER_BAR_COLOR
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_FOOD_WATER_MAX_VALUE 2048

typedef struct {
    int16_t food;
    int16_t water;
    int16_t poison;
    uint8_t poisoned;
} DM2_V1_FoodWaterInput;

typedef struct {
    int valid;
    uint8_t food_pct;
    uint8_t water_pct;
    uint8_t poison_pct;
    int16_t food_color;
    int16_t water_color;
    int16_t poison_color;
    uint8_t show_poison;
} DM2_V1_FoodWaterReceipt;

int dm2_v1_food_water_bridge_compute(
    const DM2_V1_FoodWaterInput *input,
    int16_t gdat_food_color_override,
    int16_t gdat_water_color_override,
    DM2_V1_FoodWaterReceipt *out);

const char *dm2_v1_food_water_bridge_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
