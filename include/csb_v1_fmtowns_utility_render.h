#ifndef FIRESTAFF_CSB_V1_FMTOWNS_UTILITY_RENDER_H
#define FIRESTAFF_CSB_V1_FMTOWNS_UTILITY_RENDER_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_fmtowns_game.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_FMTOWNS_UTILITY_SCREEN_WIDTH 320u
#define CSB_V1_FMTOWNS_UTILITY_SCREEN_HEIGHT 200u
#define CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS \
    (CSB_V1_FMTOWNS_UTILITY_SCREEN_WIDTH * CSB_V1_FMTOWNS_UTILITY_SCREEN_HEIGHT)

typedef struct CSB_V1_FmtownsUtilityRenderReceipt {
    int valid;
    CSB_V1_FmtownsSwitchLanguage language;
    uint32_t source_fnv1a;
    uint32_t pixel_fnv1a;
    uint16_t rendered_champion_count;
    uint16_t selected_champion_index;
    uint8_t selected_color_index;
    const char *source_evidence;
} CSB_V1_FmtownsUtilityRenderReceipt;

/* Recreate C06_CEDT's first F31E editor frame directly from the selected
 * retail executable and MINI.DAT payload.  Every glyph, menu label, portrait
 * and raster comes from a verified receipt; no fallback font, translated
 * label, placeholder portrait or host artwork is accepted.  F31J is
 * deliberately rejected until its native Shift-JIS/TBIOS text consumer is
 * recovered from runtime evidence. */
int csb_v1_fmtowns_utility_render_editor(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint16_t selected_champion_index, uint8_t selected_color_index,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt);

/* First F7042 frame: champion and palette index zero. */
int csb_v1_fmtowns_utility_render_initial(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_FMTOWNS_UTILITY_RENDER_H */
