#include "redmcsb_f8230_palette_components_c25_pc34_compat.h"

static uint8_t redmcsb_f8230_expand_component(uint8_t component)
{
    return (uint8_t)((component << 2U) + 3U);
}

bool redmcsb_f8230_set_single_color_components_c25_pc34_compat(
    uint8_t full_palette[REDMCSB_F8156_C25_PALETTE_COLORS_PC34]
                        [REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34],
    RedmcsbF8156C25DacPc34Compat *dac, uint8_t curtain_flag,
    int16_t color_index, uint8_t red, uint8_t green, uint8_t blue,
    RedmcsbF8156WaitVerticalBlankPc34Compat wait_vertical_blank,
    void *context)
{
    if (full_palette == NULL || dac == NULL || wait_vertical_blank == NULL ||
        color_index < 0 ||
        color_index >= (int16_t)REDMCSB_F8156_C25_PALETTE_COLORS_PC34 ||
        red > REDMCSB_F8230_COMPONENT_MAX_PC34 ||
        green > REDMCSB_F8230_COMPONENT_MAX_PC34 ||
        blue > REDMCSB_F8230_COMPONENT_MAX_PC34) {
        return false;
    }

    /* VIDEODRV.C:3463-3465 updates the logical RGB6 palette before publishing. */
    full_palette[color_index][0] = redmcsb_f8230_expand_component(red);
    full_palette[color_index][1] = redmcsb_f8230_expand_component(green);
    full_palette[color_index][2] = redmcsb_f8230_expand_component(blue);
    if (curtain_flag == 1U) {
        return redmcsb_f8156_set_palette_c25_pc34_compat(
            (const uint8_t (*)[REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34])full_palette,
            dac, wait_vertical_blank, context);
    }
    return true;
}

const char *redmcsb_f8230_palette_components_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3456-3471 C25 F8230: expand each four-bit "
           "component as (component << 2)+3 into G8183, then call F8156 only "
           "when G4094 curtain equals one.";
}
