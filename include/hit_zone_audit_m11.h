#ifndef FIRESTAFF_HIT_ZONE_AUDIT_M11_H
#define FIRESTAFF_HIT_ZONE_AUDIT_M11_H

/*
 * hit_zone_audit_m11 — pure audit helpers for the M11 in-game hit-zone
 * touch-target audit (the M11 half of the "UI scaling and touch-target
 * audit across launcher and game views" cross-cutting TODO).
 *
 * The audit consumes the live DM1 V1 hit-zone inventory from
 * touch_click_zone_matrix_pc34_compat.c (83 source-locked zones from
 * ReDMCSB COMMAND.C route tables + the I34E layout-696 ZONES table) —
 * this header owns no zone geometry, only the audit arithmetic, so the
 * audit can never drift from the shipped hit-test table.
 *
 * Scaling model (audited, not assumed):
 *
 *  1. Hit-testing runs in 320x200 source space
 *     (TOUCHCLICK_Compat_NormalizeScaledScreenPoint letterboxes
 *     physical points back to source coordinates), so a zone's
 *     presented physical size is source size x presentation scale.
 *
 *  2. The M11 UI scale percent (100/150/200, ui_scale_m11.h) is a
 *     dormant accessibility extra: M11_UIScale_Apply /
 *     M11_UIScale_GetFontScale have no consumer in the hit-test or
 *     HUD-geometry path today, so zone geometry is UI-scale
 *     independent.  The audit pins that finding (identical floor
 *     counts at every percent) and additionally records the
 *     hypothetical M11_UIScale_Apply-adjusted sizes so a future
 *     UI-scale-aware HUD can be re-audited against the same contract.
 *
 * Floors mirror fs_gesture_navigation_gate.h
 * (FS_GG_PLATFORM_MIN_TARGET_PX / FS_GG_PLATFORM_RECOMMENDED_PX); the
 * audit test cross-checks the values stay in sync.
 *
 * Data-free: pure constants + inline arithmetic, no assets, no SDL.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Touch-target floors (mirror fs_gesture_navigation_gate.h) ─────── */
#define M11_HIT_ZONE_MIN_TARGET_PX         24
#define M11_HIT_ZONE_RECOMMENDED_TARGET_PX 44

/* Presentation scales the audit evaluates (320x200 letterboxed to
 * physical surfaces; 1x is diagnostic only — every shipped window
 * mode presents at >= 2x). */
#define M11_HIT_ZONE_PRESENT_SCALE_MIN 1
#define M11_HIT_ZONE_PRESENT_SCALE_MAX 4

/* ── Audit classification ──────────────────────────────────────────── */
typedef enum {
    M11_HIT_ZONE_FIT_BELOW_MINIMUM = 0, /* under the 24 px floor       */
    M11_HIT_ZONE_FIT_MINIMUM,           /* floor met, under 44 px rec. */
    M11_HIT_ZONE_FIT_RECOMMENDED        /* 44 px recommendation met    */
} M11HitZoneFit;

/* ── Pure helpers ──────────────────────────────────────────────────── */

/* Mirror of the M11_UIScale_Apply integer math ((v * percent + 50) /
 * 100) so the audit can compute hypothetical UI-scale-adjusted sizes
 * without linking the runtime module. */
static inline int m11_hit_zone_apply_ui_scale(int value, int percent) {
    if (percent <= 0) return value;
    return (value * percent + 50) / 100;
}

/* Presented physical size of one source-space side: UI-scale-adjusted
 * source size x presentation scale.  With the current UI-scale-
 * independent hit-test geometry, callers pass percent=100 for the
 * shipped contract. */
static inline int m11_hit_zone_presented_px(int sourcePx,
                                            int uiScalePercent,
                                            int presentationScale) {
    int scaled = m11_hit_zone_apply_ui_scale(sourcePx, uiScalePercent);
    if (presentationScale <= 0) return scaled;
    return scaled * presentationScale;
}

/* Classify a presented shorter-side size against the touch floors. */
static inline M11HitZoneFit m11_hit_zone_classify(int presentedMinSidePx) {
    if (presentedMinSidePx >= M11_HIT_ZONE_RECOMMENDED_TARGET_PX) {
        return M11_HIT_ZONE_FIT_RECOMMENDED;
    }
    if (presentedMinSidePx >= M11_HIT_ZONE_MIN_TARGET_PX) {
        return M11_HIT_ZONE_FIT_MINIMUM;
    }
    return M11_HIT_ZONE_FIT_BELOW_MINIMUM;
}

static inline const char* m11_hit_zone_fit_name(M11HitZoneFit fit) {
    switch (fit) {
        case M11_HIT_ZONE_FIT_RECOMMENDED:   return "recommended";
        case M11_HIT_ZONE_FIT_MINIMUM:       return "minimum";
        case M11_HIT_ZONE_FIT_BELOW_MINIMUM: return "below-minimum";
        default:                           return "unknown";
    }
}

/* Per-zone decision: the minimum presentation scale in
 * [M11_HIT_ZONE_PRESENT_SCALE_MIN, M11_HIT_ZONE_PRESENT_SCALE_MAX] at
 * which the zone's shorter side clears the 24 px floor at the given UI
 * scale percent.  Returns 0 when even the maximum audited presentation
 * scale cannot lift the zone (e.g. the hidden 2x2 freeze-game debug
 * box), which the audit records as an explicit exemption decision. */
static inline int m11_hit_zone_min_lifting_scale(int sourceMinSidePx,
                                                 int uiScalePercent) {
    int scale;
    for (scale = M11_HIT_ZONE_PRESENT_SCALE_MIN;
         scale <= M11_HIT_ZONE_PRESENT_SCALE_MAX; ++scale) {
        if (m11_hit_zone_presented_px(sourceMinSidePx, uiScalePercent, scale)
                >= M11_HIT_ZONE_MIN_TARGET_PX) {
            return scale;
        }
    }
    return 0;
}

/* Human-readable decision name for audit output. */
static inline const char* m11_hit_zone_decision_name(int minLiftingScale) {
    switch (minLiftingScale) {
        case 1:  return "floor-at-1x";
        case 2:  return "needs-2x";
        case 3:  return "needs-3x";
        case 4:  return "needs-4x";
        default: return "never-lifts-exempt";
    }
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_HIT_ZONE_AUDIT_M11_H */
