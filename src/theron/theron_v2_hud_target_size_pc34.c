/*
 * theron_v2_hud_target_size_pc34.c
 *
 * Theron's Quest V2.2 Phase 6 — UI target-size audit implementation.
 *
 * Reads the documented TQR chrome-zone geometry (TQR_VP_X/Y/W/H, TQR_TOPBAR_H,
 * TQR_RIGHT_W, TQR_BOTTOM_H, TQR_MSG_H, TQR_CHAMP_SLOT_*) and applies
 * the active presentation resolution to compute effective touch-target
 * sizes per zone. Data-free: no game data is read.
 *
 * Phase 6 rule: BLOCKED zones (< 24 px effective) must be flagged so
 * the V2 presenter can grow them via the V2 HUD overlay before any
 * runtime touch input reaches them. TIGHT (24..31) zones are borderline
 * and surface as warnings in the audit record. ACCEPTABLE (32..47) is
 * the Android minimum touch target. COMFORTABLE (48+) meets the iOS
 * HIG minimum touch target.
 *
 * Source-lock anchors:
 * - theron_v1_viewport.h TQR_VP_X/Y/W/H TQR_TOPBAR_H TQR_RIGHT_W
 *                         TQR_BOTTOM_H TQR_MSG_H TQR_CHAMP_SLOT_W/H/Y
 * - THQUEST.ASM T600 (UI overlay zones: top bar / viewport /
 *                      right panel / bottom panel / message bar)
 * - ReDMCSB COMMAND.C F0380 (V1 input wait loop)
 * - theron_v2_presentation_mode_pc34.h (V2.0/V2.1/V2.2 scale factors)
 */

#include "theron_v2_hud_target_size_pc34.h"

#include <string.h>

/* Tier thresholds (data-free, deterministic). */
#define THERON_V2_HUD_TARGET_BLOCKED_MAX        23
#define THERON_V2_HUD_TARGET_TIGHT_MAX          31
#define THERON_V2_HUD_TARGET_ACCEPTABLE_MAX     47

/* Helper: classify a scaled min-edge into a tier. */
static THERON_V2_HudTargetTier tier_for_scaled_min_edge(int scaledMinEdge)
{
    if (scaledMinEdge <= THERON_V2_HUD_TARGET_BLOCKED_MAX) {
        return THERON_V2_HUD_TARGET_BLOCKED;
    }
    if (scaledMinEdge <= THERON_V2_HUD_TARGET_TIGHT_MAX) {
        return THERON_V2_HUD_TARGET_TIGHT;
    }
    if (scaledMinEdge <= THERON_V2_HUD_TARGET_ACCEPTABLE_MAX) {
        return THERON_V2_HUD_TARGET_ACCEPTABLE;
    }
    return THERON_V2_HUD_TARGET_COMFORTABLE;
}

/* Helper: clamp the scale factor to >= 1 (we never down-scale a
 * chrome zone in the audit). The audit is for the active canvas size,
 * which is always >= the documented TQR_SCREEN size in any V2 mode. */
static int compute_scale(int canvasDimension, int sourceDimension)
{
    if (sourceDimension <= 0) return 1;
    int scale = canvasDimension / sourceDimension;
    if (scale < 1) scale = 1;
    return scale;
}

/* Source rectangle per zone, derived from theron_v1_viewport.h. */
static void fill_source_rect(THERON_V2_HudZoneKind zone, int* x, int* y, int* w, int* h)
{
    /* Defaults: 0,0,0,0 means "skip" but every zone below overrides
     * all four fields, so the default branch is unreachable. */
    switch (zone) {
        case THERON_V2_HUD_ZONE_TOPBAR:
            *x = 0;
            *y = 0;
            *w = TQR_SCREEN_W;
            *h = TQR_TOPBAR_H;
            break;
        case THERON_V2_HUD_ZONE_VIEWPORT:
            *x = TQR_VP_X;
            *y = TQR_VP_Y;
            *w = TQR_VP_W;
            *h = TQR_VP_H;
            break;
        case THERON_V2_HUD_ZONE_RIGHT_PANEL:
            *x = TQR_SCREEN_W - TQR_RIGHT_W;
            *y = TQR_TOPBAR_H;
            *w = TQR_RIGHT_W;
            /* The right panel covers from top bar to bottom panel. */
            *h = TQR_SCREEN_H - TQR_TOPBAR_H - TQR_BOTTOM_H;
            break;
        case THERON_V2_HUD_ZONE_BOTTOM_PANEL:
            *x = 0;
            *y = TQR_SCREEN_H - TQR_BOTTOM_H;
            *w = TQR_SCREEN_W;
            *h = TQR_BOTTOM_H;
            break;
        case THERON_V2_HUD_ZONE_MESSAGE_BAR:
            *x = 0;
            *y = TQR_CHAMP_SLOT_Y;       /* message bar starts at slot row */
            *w = TQR_SCREEN_W;
            *h = TQR_MSG_H;
            break;
        case THERON_V2_HUD_ZONE_CHAMPION_SLOT:
            *x = 0;
            *y = TQR_CHAMP_SLOT_Y;
            *w = TQR_CHAMP_SLOT_W;
            *h = TQR_CHAMP_SLOT_H;
            break;
        case THERON_V2_HUD_ZONE_COUNT:
        default:
            *x = 0;
            *y = 0;
            *w = 0;
            *h = 0;
            break;
    }
}

const char* theron_v2_hud_target_size_zone_name(THERON_V2_HudZoneKind zone)
{
    switch (zone) {
        case THERON_V2_HUD_ZONE_TOPBAR: return "TOPBAR";
        case THERON_V2_HUD_ZONE_VIEWPORT: return "VIEWPORT";
        case THERON_V2_HUD_ZONE_RIGHT_PANEL: return "RIGHT_PANEL";
        case THERON_V2_HUD_ZONE_BOTTOM_PANEL: return "BOTTOM_PANEL";
        case THERON_V2_HUD_ZONE_MESSAGE_BAR: return "MESSAGE_BAR";
        case THERON_V2_HUD_ZONE_CHAMPION_SLOT: return "CHAMPION_SLOT";
        case THERON_V2_HUD_ZONE_COUNT:
        default: return "UNKNOWN";
    }
}

const char* theron_v2_hud_target_size_tier_name(THERON_V2_HudTargetTier tier)
{
    switch (tier) {
        case THERON_V2_HUD_TARGET_BLOCKED:     return "BLOCKED";
        case THERON_V2_HUD_TARGET_TIGHT:       return "TIGHT";
        case THERON_V2_HUD_TARGET_ACCEPTABLE:  return "ACCEPTABLE";
        case THERON_V2_HUD_TARGET_COMFORTABLE: return "COMFORTABLE";
        default:                               return "UNKNOWN";
    }
}

THERON_V2_HudTargetSizeRecord theron_v2_hud_target_size_for_zone(
    const THERON_V2_HudTargetSizeContext* ctx,
    THERON_V2_HudZoneKind zone)
{
    THERON_V2_HudTargetSizeRecord rec;
    int scaleX, scaleY, scale;
    int x = 0, y = 0, w = 0, h = 0;
    int minEdge, scaledMinEdge;

    memset(&rec, 0, sizeof(rec));
    rec.name = theron_v2_hud_target_size_zone_name(zone);

    /* Null context: report a zero-sized BLOCKED record so callers
     * can detect the failure mode. We deliberately do not crash. */
    if (!ctx || ctx->canvasWidth <= 0 || ctx->canvasHeight <= 0) {
        rec.tier = THERON_V2_HUD_TARGET_BLOCKED;
        rec.minEdge = 0;
        rec.scaledMinEdge = 0;
        return rec;
    }

    fill_source_rect(zone, &x, &y, &w, &h);
    scaleX = compute_scale(ctx->canvasWidth,  TQR_SCREEN_W);
    scaleY = compute_scale(ctx->canvasHeight, TQR_SCREEN_H);
    /* Effective touch target uses the smaller scale so that
     * non-uniform upscale does not over-state the effective size. */
    scale = (scaleX < scaleY) ? scaleX : scaleY;

    minEdge = (w < h) ? w : h;
    scaledMinEdge = minEdge * scale;

    rec.x = x;
    rec.y = y;
    rec.w = w;
    rec.h = h;
    rec.minEdge = minEdge;
    rec.scaledMinEdge = scaledMinEdge;
    rec.tier = tier_for_scaled_min_edge(scaledMinEdge);
    return rec;
}

THERON_V2_HudTargetSizeAudit theron_v2_hud_target_size_audit(
    const THERON_V2_HudTargetSizeContext* ctx)
{
    THERON_V2_HudTargetSizeAudit audit;
    int i;
    THERON_V2_HudTargetTier worst = THERON_V2_HUD_TARGET_COMFORTABLE;
    int worstMin = 0x7fffffff;

    memset(&audit, 0, sizeof(audit));
    audit.zoneCount = THERON_V2_HUD_ZONE_COUNT;
    audit.minScaledEdge = 0;
    audit.worstTier = THERON_V2_HUD_TARGET_COMFORTABLE;

    for (i = 0; i < THERON_V2_HUD_ZONE_COUNT; ++i) {
        THERON_V2_HudTargetSizeRecord rec =
            theron_v2_hud_target_size_for_zone(ctx, (THERON_V2_HudZoneKind)i);
        audit.zones[i] = rec;

        switch (rec.tier) {
            case THERON_V2_HUD_TARGET_BLOCKED:     audit.blockedCount++;     break;
            case THERON_V2_HUD_TARGET_TIGHT:       audit.tightCount++;       break;
            case THERON_V2_HUD_TARGET_ACCEPTABLE:  audit.acceptableCount++;  break;
            case THERON_V2_HUD_TARGET_COMFORTABLE: audit.comfortableCount++; break;
            default: break;
        }

        if (rec.scaledMinEdge < worstMin) {
            worstMin = rec.scaledMinEdge;
            worst = rec.tier;
        }
    }

    audit.minScaledEdge = worstMin;
    audit.worstTier = worst;
    return audit;
}

const char* theron_v2_hud_target_size_source_evidence(void)
{
    return "ReDMCSB/THQUEST T600 (UI overlay zones: top bar / viewport / right panel / bottom panel / message bar); "
           "ReDMCSB COMMAND.C F0380 (V1 input wait loop); "
           "theron_v1_viewport.h TQR_VP_X/Y/W/H (32, 24, 192, 160), TQR_TOPBAR_H (24), TQR_RIGHT_W (96), TQR_BOTTOM_H (56), "
           "TQR_MSG_H (16), TQR_CHAMP_SLOT_W/H/Y (80, 56, 184); "
           "theron_v2_presentation_mode_pc34.h V2.0/V2.1/V2.2 mode kinds for active canvas resolution; "
           "Phase 6 tier thresholds: BLOCKED <= 23 px (sub-pixel risk), TIGHT 24..31 px (stylus only), "
           "ACCEPTABLE 32..47 px (Android minimum touch target), COMFORTABLE 48+ px (iOS HIG minimum touch target); "
           "Source-lock: TQR_SCREEN_W/H (320, 240) anchors the canvas scale; non-uniform upscale uses min(scaleX, scaleY) "
           "so the audit never over-states the effective touch target.";
}
