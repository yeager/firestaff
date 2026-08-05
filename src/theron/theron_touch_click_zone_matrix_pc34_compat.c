#include "theron_touch_click_zone_matrix_pc34_compat.h"

#include <string.h>

/* Source-anchored Theron per-view zone inventory — implemented chrome
 * geometry, NOT an extracted original table (see the header's
 * provenance block).  The matrix is evidence/probe data only. */

#define TR_V1 THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT
#define TR_V2 THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT
#define TR_SCR TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT
#define TR_LEFT TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT

static const TheronTouchClickZonePc34Compat kTheronTouchClickZones[] = {
    /* ── V1 chrome, 320x240 extended canvas (TQR_SCREEN_W/H) ────── */
    {  1u, 0u, TR_V1, TR_SCR, TR_LEFT,   0,   0, 320,  24, "chrome.topbar",
       "theron_v1_viewport.h TQR_SCREEN_W/TQR_TOPBAR_H (320,24); theron_v1_ui_chrome.h TR_UI_TOPBAR (1<<0); anchored to THQUEST.ASM T600" },
    {  0u, 1u, TR_V1, TR_SCR, TR_LEFT,  32,  24, 192, 160, "chrome.viewport",
       "theron_v1_viewport.h TQR_VP_X/Y/W/H (32,24,192,160); no TR_UI flag and no command bound — gamepad-driven original; anchored to THQUEST.ASM T600" },
    {  2u, 2u, TR_V1, TR_SCR, TR_LEFT, 224,  24,  96, 160, "chrome.right_panel",
       "theron_v1_viewport.h TQR_SCREEN_W-TQR_RIGHT_W=224, TQR_RIGHT_W (96), y=TQR_TOPBAR_H, h=SCREEN_H-TOPBAR_H-BOTTOM_H=160; theron_v1_ui_chrome.h TR_UI_RIGHT_PANEL (1<<1)" },
    {  4u, 3u, TR_V1, TR_SCR, TR_LEFT,   0, 184, 320,  56, "chrome.bottom_panel",
       "theron_v1_viewport.h TQR_SCREEN_H-TQR_BOTTOM_H=184, TQR_BOTTOM_H (56); theron_v1_ui_chrome.h TR_UI_BOTTOM_PANEL (1<<2)" },
    {  8u, 4u, TR_V1, TR_SCR, TR_LEFT,   0, 184, 320,  16, "chrome.message_bar",
       "theron_v1_viewport.h TQR_CHAMP_SLOT_Y=184, TQR_MSG_H (16); theron_v1_ui_chrome.h TR_UI_MESSAGE (1<<3); nested inside bottom_panel (coarse source panels)" },
    {  0u, 5u, TR_V1, TR_SCR, TR_LEFT,   0, 184,  80,  56, "champion.slot_0",
       "theron_v1_ui_chrome.h TR_CHAMP_SLOT_W/H (80,56) slot 0 at x=0, y=TQR_CHAMP_SLOT_Y=184; nested inside bottom_panel/message_bar (coarse source panels)" },
    {  0u, 6u, TR_V1, TR_SCR, TR_LEFT,  80, 184,  80,  56, "champion.slot_1",
       "theron_v1_ui_chrome.h TR_CHAMP_SLOT_W/H (80,56) slot 1 at x=80, y=184" },
    {  0u, 7u, TR_V1, TR_SCR, TR_LEFT, 160, 184,  80,  56, "champion.slot_2",
       "theron_v1_ui_chrome.h TR_CHAMP_SLOT_W/H (80,56) slot 2 at x=160, y=184" },
    {  0u, 8u, TR_V1, TR_SCR, TR_LEFT, 240, 184,  80,  56, "champion.slot_3",
       "theron_v1_ui_chrome.h TR_CHAMP_SLOT_W/H (80,56) slot 3 at x=240, y=184" },

    /* ── V2 HUD overlay, 256x224 PC Engine native (TQR_FB_W/H) ──── */
    {  0u, 0u, TR_V2, TR_SCR, TR_LEFT,   4,   0,  24,  24, "hud.compass",
       "theron_v2_hud_overlay_pc34.h THERON_V2_HUD_COMPASS_CX/CY (16,12) center +/- 12; production HUD seam remains no-draw; presentation-only indicator" },
    {  0u, 1u, TR_V2, TR_SCR, TR_LEFT,  36,   8,   4,   4, "hud.rune_slot_0",
       "theron_v2_hud_overlay_pc34.h rune indicator route at (36,8), 4 slots 4x4 px at 6 px pitch (slot 0); production route remains no-draw" },
    {  0u, 2u, TR_V2, TR_SCR, TR_LEFT,  42,   8,   4,   4, "hud.rune_slot_1",
       "theron_v2_hud_overlay_pc34.h rune indicator slot 1 at (42,8) 4x4; production route remains no-draw" },
    {  0u, 3u, TR_V2, TR_SCR, TR_LEFT,  48,   8,   4,   4, "hud.rune_slot_2",
       "theron_v2_hud_overlay_pc34.h rune indicator slot 2 at (48,8) 4x4; production route remains no-draw" },
    {  0u, 4u, TR_V2, TR_SCR, TR_LEFT,  54,   8,   4,   4, "hud.rune_slot_3",
       "theron_v2_hud_overlay_pc34.h rune indicator slot 3 at (54,8) 4x4; production route remains no-draw" },
    {  0u, 5u, TR_V2, TR_SCR, TR_LEFT,  64,   4,  24,   5, "hud.quest_items",
       "theron_v2_hud_overlay_pc34.h THERON_V2_HUD_QUEST_X/Y (64,4); 6 glyph cells (Qx/Qy) at 4 px pitch x 5 px glyph height; text indicator" },
    {  0u, 6u, TR_V2, TR_SCR, TR_LEFT, 160,   4,  24,   5, "hud.dungeon_progress",
       "theron_v2_hud_overlay_pc34.h THERON_V2_HUD_DUNGEON_X/Y (160,4); 6 glyph cells (D1/7); text indicator" },
    {  0u, 7u, TR_V2, TR_SCR, TR_LEFT, 220,   4,  24,   5, "hud.relic_counter",
       "theron_v2_hud_overlay_pc34.h THERON_V2_HUD_RELIC_X/Y (220,4); 6 glyph cells (R1/7); text indicator" },
    {  0u, 8u, TR_V2, TR_SCR, TR_LEFT,   4, 184,  60,   8, "hud.champion_bar_0",
       "theron_v2_hud_overlay_pc34.h THERON_V2_CHAMP_BAR_X_START/Y/W/H (4,184,60,8) + SPACING (2) slot 0; presentation-only mini-bar" },
    {  0u, 9u, TR_V2, TR_SCR, TR_LEFT,  66, 184,  60,   8, "hud.champion_bar_1",
       "theron_v2_hud_overlay_pc34.h champion mini-bar slot 1 at x=4+62" },
    {  0u, 10u, TR_V2, TR_SCR, TR_LEFT, 128, 184,  60,  8, "hud.champion_bar_2",
       "theron_v2_hud_overlay_pc34.h champion mini-bar slot 2 at x=4+124" },
    {  0u, 11u, TR_V2, TR_SCR, TR_LEFT, 190, 184,  60,  8, "hud.champion_bar_3",
       "theron_v2_hud_overlay_pc34.h champion mini-bar slot 3 at x=4+186" },
    {  1u, 12u, TR_V2, TR_SCR, TR_LEFT,  16, 208,  28,  14, "action.attack",
       "theron_v2_hud_overlay_pc34.h THERON_V2_ACTION_ICONS_X_START/STRIP_Y/ICON_W/STRIP_H (16,208,28,14) + 4 px gap icon 0; commandId = 1-based Theron_V2_ActionIcon (ATTACK)" },
    {  2u, 13u, TR_V2, TR_SCR, TR_LEFT,  48, 208,  28,  14, "action.cast",
       "theron_v2_hud_overlay_pc34.h action strip icon 1 (CAST) at x=16+32" },
    {  3u, 14u, TR_V2, TR_SCR, TR_LEFT,  80, 208,  28,  14, "action.use",
       "theron_v2_hud_overlay_pc34.h action strip icon 2 (USE) at x=16+64" },
    {  4u, 15u, TR_V2, TR_SCR, TR_LEFT, 112, 208,  28,  14, "action.drop",
       "theron_v2_hud_overlay_pc34.h action strip icon 3 (DROP) at x=16+96" },
    {  5u, 16u, TR_V2, TR_SCR, TR_LEFT, 144, 208,  28,  14, "action.move",
       "theron_v2_hud_overlay_pc34.h action strip icon 4 (MOVE) at x=16+128" },
};

#define THERON_ZONE_COUNT (sizeof(kTheronTouchClickZones) / sizeof(kTheronTouchClickZones[0]))

unsigned int THERON_TOUCHCLICK_Compat_GetZoneCount(void) {
    return (unsigned int)THERON_ZONE_COUNT;
}

int THERON_TOUCHCLICK_Compat_GetZone(unsigned int ordinal,
                                     TheronTouchClickZonePc34Compat* outZone) {
    if (!outZone || ordinal >= THERON_ZONE_COUNT) return 0;
    *outZone = kTheronTouchClickZones[ordinal];
    return 1;
}

unsigned int THERON_TOUCHCLICK_Compat_GetViewZoneCount(
    TheronTouchClickViewPc34Compat view) {
    unsigned int count = 0;
    unsigned int i;
    for (i = 0; i < THERON_ZONE_COUNT; ++i) {
        if (kTheronTouchClickZones[i].view == view) ++count;
    }
    return count;
}

int THERON_TOUCHCLICK_Compat_GetViewZone(TheronTouchClickViewPc34Compat view,
                                         unsigned int viewOrdinal,
                                         TheronTouchClickZonePc34Compat* outZone) {
    unsigned int seen = 0;
    unsigned int i;
    if (!outZone) return 0;
    for (i = 0; i < THERON_ZONE_COUNT; ++i) {
        if (kTheronTouchClickZones[i].view != view) continue;
        if (seen == viewOrdinal) {
            *outZone = kTheronTouchClickZones[i];
            return 1;
        }
        ++seen;
    }
    return 0;
}

int THERON_TOUCHCLICK_Compat_HitTestInView(TheronTouchClickViewPc34Compat view,
                                           int screenX, int screenY,
                                           unsigned int buttonMask,
                                           TheronTouchClickZonePc34Compat* outZone) {
    unsigned int i;
    for (i = 0; i < THERON_ZONE_COUNT; ++i) {
        const TheronTouchClickZonePc34Compat* zone = &kTheronTouchClickZones[i];
        if (zone->view != view) continue;
        if ((zone->buttonMask & buttonMask) == 0) continue;
        if (screenX < zone->x || screenX >= zone->x + zone->w ||
                screenY < zone->y || screenY >= zone->y + zone->h)
            continue;
        if (outZone) *outZone = *zone;
        return 1;
    }
    return 0;
}

const char* THERON_TOUCHCLICK_Compat_GetViewName(
    TheronTouchClickViewPc34Compat view) {
    switch (view) {
        case THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT:
            return "v1_chrome";
        case THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT:
            return "v2_hud_overlay";
        default: return "unknown";
    }
}

const char* THERON_TOUCHCLICK_Compat_GetSourceEvidence(void) {
    return "Implemented-geometry inventory (NOT an extracted original "
           "table): ReDMCSB WIP20210206 verified 2026-07-20 to contain "
           "zero Theron/TurboGrafx coverage (1184 files); the only local "
           "disassembly is the IPL + stage2 boot loaders "
           "(docs/source-lock/theron-disassembly/, da65 HuC6280) with no "
           "input/UI-zone code; THQUEST.BIN is not disassembled locally.  "
           "V1 chrome zones from include/theron_v1_viewport.h "
           "(TQR_SCREEN_W/H 320x240, TQR_VP_X/Y/W/H 32/24/192/160, "
           "TQR_TOPBAR_H 24, TQR_RIGHT_W 96, TQR_BOTTOM_H 56, TQR_MSG_H "
           "16, TQR_CHAMP_SLOT_W/H/Y 80/56/184) and "
           "src/theron/theron_v1_ui_chrome.h (TR_UI_TOPBAR/RIGHT_PANEL/"
           "BOTTOM_PANEL/MESSAGE bits), the same six zone kinds the "
           "theron_v2_hud_target_size_pc34 audit consumes; V2 overlay "
           "zones from include/theron_v2_hud_overlay_pc34.h "
           "(THERON_V2_HUD_COMPASS_CX/CY 16/12, QUEST 64/4, DUNGEON "
           "160/4, RELIC 220/4, CHAMP_BAR 4+62i/184/60x8, ACTION "
           "16+32i/208/28x14) and include/theron_v2_hud_overlay_pc34.h "
           "render coordinates (rune slots 36+6i,8 4x4); anchored to "
           "THQUEST.ASM T600/T900 claims; the original game is "
           "gamepad-driven with no mouse route tables — gesture runtime "
           "in src/theron/theron_v2_touch_runtime.c treats HUD chrome "
           "as an exclusion guard.";
}
