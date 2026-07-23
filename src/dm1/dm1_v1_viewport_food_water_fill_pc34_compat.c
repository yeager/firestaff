#include "dm1_v1_viewport_planar_fill_material_pc34_compat.h"

#include <limits.h>
#include <string.h>

enum {
    DM1_V1_F0344_BLACK_PC34 = 0,
    DM1_V1_F0344_RED_PC34 = 8,
    DM1_V1_F0344_YELLOW_PC34 = 11,
    DM1_V1_F0344_SHADOW_OFFSET_PC34 = 2,
    DM1_V1_F0344_FULL_UNITS_PC34 = 3072,
    DM1_V1_F0344_FULL_PROPORTION_PC34 = 10000
};

static uint16_t dm1_v1_f0344_color_for_amount_pc34(int amount,
                                                     uint16_t base_color)
{
    if (amount < -512) return DM1_V1_F0344_RED_PC34;
    if (amount < 0) return DM1_V1_F0344_YELLOW_PC34;
    return base_color;
}

int dm1_v1_viewport_fill_food_water_bar_f0344_pc34(
    DM1_V1_ViewportPlanarFillMaterialPc34 *material,
    const int16_t full_box[4],
    int amount,
    uint16_t base_color,
    DM1_V1_ViewportFoodWaterFillReceiptPc34 *receipt)
{
    int64_t normalized;
    int64_t width;
    int64_t proportional_units;
    int16_t bar_box[4];
    int16_t shadow_box[4];
    uint16_t color;

    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!material || !full_box || base_color > 15u ||
        full_box[0] < 0 || full_box[2] < 0 ||
        full_box[0] > full_box[1] || full_box[2] > full_box[3]) {
        return 0;
    }

    /* PANEL.C F0344 adds 1024, then F0637 scales the parent width by
     * ((amount + 1024) * 10000) / 3072. Source champion counters are within
     * this domain; rejecting other data avoids manufacturing a bar. */
    normalized = (int64_t)amount + 1024;
    if (normalized < 0 || normalized > DM1_V1_F0344_FULL_UNITS_PC34) {
        return 0;
    }
    proportional_units =
        (normalized * DM1_V1_F0344_FULL_PROPORTION_PC34) /
        DM1_V1_F0344_FULL_UNITS_PC34;
    width = ((int64_t)(full_box[1] - full_box[0] + 1) * proportional_units) /
            DM1_V1_F0344_FULL_PROPORTION_PC34;
    if (width <= 0 || width > INT16_MAX ||
        (int64_t)full_box[0] + width - 1 > INT16_MAX ||
        full_box[0] + width - 1 + DM1_V1_F0344_SHADOW_OFFSET_PC34 > INT16_MAX ||
        full_box[2] + DM1_V1_F0344_SHADOW_OFFSET_PC34 > INT16_MAX ||
        full_box[3] + DM1_V1_F0344_SHADOW_OFFSET_PC34 > INT16_MAX) {
        return 0;
    }

    bar_box[0] = full_box[0];
    bar_box[1] = (int16_t)(full_box[0] + width - 1);
    bar_box[2] = full_box[2];
    bar_box[3] = full_box[3];
    shadow_box[0] = (int16_t)(bar_box[0] + DM1_V1_F0344_SHADOW_OFFSET_PC34);
    shadow_box[1] = (int16_t)(bar_box[1] + DM1_V1_F0344_SHADOW_OFFSET_PC34);
    shadow_box[2] = (int16_t)(bar_box[2] + DM1_V1_F0344_SHADOW_OFFSET_PC34);
    shadow_box[3] = (int16_t)(bar_box[3] + DM1_V1_F0344_SHADOW_OFFSET_PC34);
    color = dm1_v1_f0344_color_for_amount_pc34(amount, base_color);

    /* PANEL.C F0344: F0135 shadow fill precedes the live proportional bar. */
    if (!dm1_v1_viewport_fill_material_box_f0135_pc34(
            material, shadow_box, DM1_V1_F0344_BLACK_PC34) ||
        !dm1_v1_viewport_fill_material_box_f0135_pc34(material, bar_box, color)) {
        return 0;
    }
    if (receipt) {
        receipt->drawn = 1;
        receipt->proportional_units = (int)proportional_units;
        receipt->bar_color = color;
        memcpy(receipt->bar_box, bar_box, sizeof(bar_box));
        memcpy(receipt->shadow_box, shadow_box, sizeof(shadow_box));
    }
    return 1;
}
