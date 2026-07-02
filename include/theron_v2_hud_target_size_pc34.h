#ifndef FIRESTAFF_THERON_V2_HUD_TARGET_SIZE_PC34_H
#define FIRESTAFF_THERON_V2_HUD_TARGET_SIZE_PC34_H

/*
 * theron_v2_hud_target_size_pc34.h
 *
 * Theron's Quest V2.2 Phase 6 — UI target-size audit.
 *
 * Phase 6 rule: every interactive chrome zone must meet a minimum
 * touch-target size at the active presentation resolution so that
 * touch input does not require sub-pixel accuracy. The data-free
 * audit reads the documented TQR chrome-zone geometry (from
 * theron_v1_viewport.h) plus the active presentation resolution
 * and reports each zone's effective touch target in pixels at the
 * active canvas size.
 *
 * Tier thresholds (data-free, deterministic):
 *   - 0..23 px effective: BLOCKED       — too small for any touch input
 *   - 24..31 px effective: TIGHT        — borderline, may require stylus
 *   - 32..47 px effective: ACCEPTABLE   — meets the 32px Android minimum
 *   - 48+ px effective:    COMFORTABLE  — meets the 44px iOS HIG minimum
 *
 * Source-lock anchors:
 * - theron_v1_viewport.h TQR_VP_X/Y/W/H TQR_TOPBAR_H TQR_RIGHT_W
 *                         TQR_BOTTOM_H / TQR_MSG_H / TQR_CHAMP_SLOT_*
 * - THQUEST.ASM T600 (UI overlay zones: top bar / viewport /
 *                      right panel / bottom panel / message bar)
 * - ReDMCSB COMMAND.C F0380 (V1 input wait loop)
 * - theron_v2_presentation_mode_pc34.h (V2.0/V2.1/V2.2 scale factors)
 *
 * Module: src/theron/theron_v2_hud_target_size_pc34.c
 * Test:   tests/test_theron_v2_hud_target_size_pc34.c
 * Probe:  probes/firestaff_theron_v2_hud_target_size_probe.c
 */

#include <stdint.h>

#include "theron_v1_viewport.h"
#include "theron_v2_presentation_mode_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Tier thresholds (data-free, deterministic). Exposed so unit tests
 * can verify the boundary conditions directly. */
#define THERON_V2_HUD_TARGET_BLOCKED_MAX        23
#define THERON_V2_HUD_TARGET_TIGHT_MAX          31
#define THERON_V2_HUD_TARGET_ACCEPTABLE_MAX     47

/* Phase 6 chrome zones — derived from theron_v1_viewport.h constants.
 * Each zone is a rectangle in the 320x240 TQR_SCREEN canvas; the
 * audit scales the rectangle to the active presentation resolution.
 *
 * The rectangle fields use the documented chrome zone geometry:
 *   TQR_VP_X/Y/W/H = viewport (192x160 starting at 32,24)
 *   TQR_TOPBAR_H   = top bar height (24 px)
 *   TQR_RIGHT_W    = right panel width (96 px)
 *   TQR_BOTTOM_H   = bottom panel height (56 px)
 *   TQR_MSG_H      = message bar height (16 px inside bottom)
 *   TQR_CHAMP_SLOT_W/H = champion slot (80x56)
 *
 * Source-lock: theron_v1_viewport.h:48-67 + THQUEST.ASM T600. */
typedef enum {
    THERON_V2_HUD_ZONE_TOPBAR = 0,
    THERON_V2_HUD_ZONE_VIEWPORT,
    THERON_V2_HUD_ZONE_RIGHT_PANEL,
    THERON_V2_HUD_ZONE_BOTTOM_PANEL,
    THERON_V2_HUD_ZONE_MESSAGE_BAR,
    THERON_V2_HUD_ZONE_CHAMPION_SLOT,
    THERON_V2_HUD_ZONE_COUNT
} THERON_V2_HudZoneKind;

typedef enum {
    THERON_V2_HUD_TARGET_BLOCKED = 0,     /* < 24 px effective */
    THERON_V2_HUD_TARGET_TIGHT = 1,       /* 24..31 px */
    THERON_V2_HUD_TARGET_ACCEPTABLE = 2,   /* 32..47 px */
    THERON_V2_HUD_TARGET_COMFORTABLE = 3   /* 48+ px */
} THERON_V2_HudTargetTier;

typedef struct {
    int x, y, w, h;        /* source rectangle in TQR_SCREEN pixels */
    int minEdge;           /* min(w, h) of the source rectangle */
    int scaledMinEdge;     /* min edge after applying the active scale */
    THERON_V2_HudTargetTier tier;
    const char* name;
} THERON_V2_HudTargetSizeRecord;

/* Active canvas size, in pixels. Caller passes the presented canvas
 * dimensions; the audit computes per-zone scales as
 *   scaleX = canvasWidth  / TQR_SCREEN_W (320)
 *   scaleY = canvasHeight / TQR_SCREEN_H (240)
 * Each zone uses min(scaleX, scaleY) so a non-uniform upscale does
 * not over-state the effective touch target. */
typedef struct {
    int canvasWidth;
    int canvasHeight;
    Theron_V2_PresentationModeKind mode;
} THERON_V2_HudTargetSizeContext;

/* Single-zone query. */
THERON_V2_HudTargetSizeRecord theron_v2_hud_target_size_for_zone(
    const THERON_V2_HudTargetSizeContext* ctx,
    THERON_V2_HudZoneKind zone);

/* Full audit: iterate every chrome zone and return a summary record. */
typedef struct {
    int zoneCount;
    int blockedCount;
    int tightCount;
    int acceptableCount;
    int comfortableCount;
    int minScaledEdge;     /* min across all zones (worst case) */
    THERON_V2_HudTargetTier worstTier;
    THERON_V2_HudTargetSizeRecord zones[THERON_V2_HUD_ZONE_COUNT];
} THERON_V2_HudTargetSizeAudit;

THERON_V2_HudTargetSizeAudit theron_v2_hud_target_size_audit(
    const THERON_V2_HudTargetSizeContext* ctx);

/* Tier-name helpers. */
const char* theron_v2_hud_target_size_tier_name(THERON_V2_HudTargetTier tier);
const char* theron_v2_hud_target_size_zone_name(THERON_V2_HudZoneKind zone);

/* Source-evidence string. Cited by probes and audit gates. */
const char* theron_v2_hud_target_size_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_HUD_TARGET_SIZE_PC34_H */
