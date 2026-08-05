#ifndef REDMCSB_THERON_TOUCH_CLICK_ZONE_MATRIX_PC34_COMPAT_H
#define REDMCSB_THERON_TOUCH_CLICK_ZONE_MATRIX_PC34_COMPAT_H

/*
 * Theron's Quest per-view click/touch zone inventory.
 *
 * PROVENANCE (honest, read first): Theron's Quest is a TurboGrafx-CD /
 * PC Engine CD title.  Unlike the DM1/CSB/DM2 matrix siblings, there
 * are NO extractable original per-view zone tables for this game:
 *   - ReDMCSB WIP20210206 contains zero Theron/TurboGrafx coverage
 *     (verified 2026-07-20: 1184 files, no THERON/TG16/PC-Engine
 *     branch in COMMAND.C or anywhere else);
 *   - the only locally staged disassembly is the IPL + stage2 boot
 *     loaders (docs/source-lock/theron-disassembly/, da65 HuC6280);
 *     they contain no input or UI-zone code;
 *   - the main THQUEST.BIN game binary is not disassembled locally,
 *     and the lane's Track 02 descriptor binding is shape-driven only
 *     (TODO.md Nexus/Theron sections).
 * What this module therefore inventories is the IMPLEMENTED Theron
 * chrome geometry the Firestaff lane actually renders and hit-guards
 * today — host-authored constants anchored to THQUEST.ASM T600/T900
 * claims:
 *   - V1 chrome (320x240 extended canvas): TQR_* constants in
 *     include/theron_v1_viewport.h + the TR_UI_ / TR_CHAMP_SLOT_
 *     defines in src/theron/theron_v1_ui_chrome.h, the same six zone
 *     kinds the existing theron_v2_hud_target_size_pc34 audit
 *     consumes;
 *   - V2 HUD overlay (256x224 PC Engine native framebuffer):
 *     the THERON_V2_HUD_ / THERON_V2_CHAMP_BAR_ / THERON_V2_ACTION_
 *     constants in include/theron_v2_hud_overlay_pc34.h plus the
 *     render coordinates in include/theron_v2_hud_overlay_pc34.h
 *     (compass 16,12; rune slots 36+i*6,8 4x4; action strip
 *     16+i*32,208 28x14; champion bars 4+i*62,184 60x8).
 * The original game is gamepad-driven: Theron has no mouse route
 * tables and no per-button hit zones.  The Firestaff touch runtime
 * (src/theron/theron_v2_touch_runtime.c) is gesture-based
 * (swipe -> ReDMCSB C001-C006 movement commands) and treats HUD
 * chrome as an exclusion guard, so most zones below are
 * presentational indicators, not command targets — commandId 0 marks
 * "no command bound" honestly.
 *
 * Coordinate spaces: V1 view zones live in the 320x240 extended
 * canvas (TQR_SCREEN_W/H); V2 view zones live in the 256x224 native
 * framebuffer (TQR_FB_W/H).  Both are stored screen-relative in
 * their own space; the view enum tells the caller which space a
 * zone uses.
 *
 * The matrix is evidence/probe data only; it does not replace
 * controller routing or the gesture runtime.
 */

#include "touch_click_zone_matrix_pc34_compat.h"

typedef enum TheronTouchClickViewPc34Compat {
    THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT = 1, /* 320x240 ext */
    THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT /* 256x224     */
} TheronTouchClickViewPc34Compat;

typedef struct TheronTouchClickZonePc34Compat {
    unsigned int commandId;     /* 0 = no command bound (indicator)  */
    unsigned int zoneIndex;     /* ordinal inside the view           */
    TheronTouchClickViewPc34Compat view;
    TouchClickCoordModePc34Compat coordMode; /* always SCREEN_RELATIVE */
    unsigned int buttonMask;    /* LEFT: touch synthesizes left click */
    int x;
    int y;
    int w;
    int h;
    const char* groupName;
    const char* sourceEvidence;
} TheronTouchClickZonePc34Compat;

unsigned int THERON_TOUCHCLICK_Compat_GetZoneCount(void);
int THERON_TOUCHCLICK_Compat_GetZone(unsigned int ordinal,
                                     TheronTouchClickZonePc34Compat* outZone);
unsigned int THERON_TOUCHCLICK_Compat_GetViewZoneCount(
    TheronTouchClickViewPc34Compat view);
int THERON_TOUCHCLICK_Compat_GetViewZone(TheronTouchClickViewPc34Compat view,
                                         unsigned int viewOrdinal,
                                         TheronTouchClickZonePc34Compat* outZone);
/* Source-ordered hit-test inside one view: first matching zone whose
 * box contains the point and whose button mask intersects wins.  The
 * caller must probe in the view's own coordinate space (320x240 for
 * V1 chrome, 256x224 for the V2 overlay). */
int THERON_TOUCHCLICK_Compat_HitTestInView(TheronTouchClickViewPc34Compat view,
                                           int screenX, int screenY,
                                           unsigned int buttonMask,
                                           TheronTouchClickZonePc34Compat* outZone);
const char* THERON_TOUCHCLICK_Compat_GetViewName(
    TheronTouchClickViewPc34Compat view);
const char* THERON_TOUCHCLICK_Compat_GetSourceEvidence(void);

#endif
