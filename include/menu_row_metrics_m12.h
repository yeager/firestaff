#ifndef FIRESTAFF_MENU_ROW_METRICS_M12_H
#define FIRESTAFF_MENU_ROW_METRICS_M12_H

/*
 * menu_row_metrics_m12 — single source of truth for launcher menu-row
 * geometry and the font-scale hit-height audit.
 *
 * This header owns the launcher menu-row inventory for the "UI scaling
 * and touch-target audit across launcher and game views" cross-cutting
 * TODO (launcher half: "a launcher menu-row hit-height audit at
 * fontScale 1..3").  Both the draw code (menu_startup_m12.c) and the
 * audit CTest (test_m12_menu_row_hit_height_audit.c) consume these
 * constants so the audit can never drift from the shipped layout.
 *
 * Inventory (legacy 320x200 indexed launcher, menu_startup_m12.c):
 *   - settings view (classic):      base pitch 18, frame 24, label at
 *     row top +5, value chip at +3.  Rows overlap: the next row's
 *     frame overpaints the bottom of the previous row, so the
 *     effective hit height is the pitch, not the frame height.
 *   - settings view (dense):        fixed pitch 34 (already computed
 *     visibleRows); text fits at every fontScale (11*3 = 33 <= 34).
 *   - save browser:                 base pitch 22, frame at row top -4,
 *     label at row top +0.
 *
 * Inventory (modern 1080p launcher, menu_startup_render_modern_m12.c):
 *   - settings rows: height 50, pitch 70; tab strip: height 34.
 *   - the modern renderer does not apply the fontScale setting, so
 *     these rows are fontScale-independent and meet the 44 px
 *     recommended target natively (tab strip: 24 px floor).
 *
 * Glyph metrics: ASCII labels draw from a 5x7 bitmap font (7 rows),
 * localized labels fall back to the 9x11 Unicode glyph table
 * (menu_unicode_glyphs_m12.c, height 11 rows; Swedish A-ring/A-umlaut
 * labels are Unicode glyphs, so 11 is the honest conservative bound).
 * Text height at integer scale N is rows * N.
 *
 * fontScale contract: the legacy draw path resolves text scale through
 * the m12_effective_text_scale rule (base + fontScale - 1, clamped to
 * [1, 4]).  At fontScale 3 an 11-row label needs 33 px, which the
 * fixed 18/22 px base pitches cannot contain — the scale-aware pitch
 * helpers below grow the row pitch so the hit height always contains
 * the presented label.  visibleRows callers adapt from the returned
 * pitch, matching the pre-existing dense-view pattern.
 *
 * Touch floors mirror fs_gesture_navigation_gate.h
 * (FS_GG_PLATFORM_MIN_TARGET_PX / FS_GG_PLATFORM_RECOMMENDED_PX); the
 * audit cross-checks the values stay in sync.
 *
 * Data-free: pure constants + inline arithmetic, no assets, no SDL.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Touch-target floors (mirror fs_gesture_navigation_gate.h) ─────── */
#define M12_MENU_ROW_MIN_TARGET_PX         24
#define M12_MENU_ROW_RECOMMENDED_TARGET_PX 44

/* ── Glyph metrics ─────────────────────────────────────────────────── */
#define M12_MENU_ASCII_GLYPH_ROWS    7
#define M12_MENU_UNICODE_GLYPH_ROWS  11

/* ── Legacy launcher row inventory (320x200 source space) ──────────── */
#define M12_MENU_ROW_SETTINGS_CLASSIC_BASE_PITCH  18
#define M12_MENU_ROW_SETTINGS_CLASSIC_FRAME       24
#define M12_MENU_ROW_SETTINGS_CLASSIC_TEXT_TOP     5
#define M12_MENU_ROW_SETTINGS_CLASSIC_TEXT_BOTTOM  1
#define M12_MENU_ROW_SETTINGS_CLASSIC_MAX_VISIBLE  6
/* Usable vertical space inside the classic settings panel: rows start
 * at y=70 and the panel frame ends at y=176 (56+120). */
#define M12_MENU_ROW_SETTINGS_CLASSIC_USABLE_H   106

#define M12_MENU_ROW_SETTINGS_DENSE_PITCH         34

#define M12_MENU_ROW_SAVE_BROWSER_BASE_PITCH      22
#define M12_MENU_ROW_SAVE_BROWSER_TEXT_TOP         4
#define M12_MENU_ROW_SAVE_BROWSER_TEXT_BOTTOM      1

/* ── Modern launcher row inventory (1080p canvas) ──────────────────── */
#define M12_MENU_ROW_MODERN_SETTINGS_HEIGHT       50
#define M12_MENU_ROW_MODERN_SETTINGS_PITCH        70
#define M12_MENU_ROW_MODERN_TAB_HEIGHT            34

/* ── Audit classification ──────────────────────────────────────────── */
typedef enum {
    M12_MENU_ROW_FIT_BELOW_MINIMUM = 0, /* under the 24 px floor       */
    M12_MENU_ROW_FIT_MINIMUM,           /* floor met, under 44 px rec. */
    M12_MENU_ROW_FIT_RECOMMENDED        /* 44 px recommendation met    */
} M12_MenuRowFit;

/* ── Pure helpers ──────────────────────────────────────────────────── */

/* Clamp a raw fontScale setting to the shipped [1, 3] range. */
static inline int m12_menu_row_clamp_font_scale(int fontScale) {
    if (fontScale < 1) return 1;
    if (fontScale > 3) return 3;
    return fontScale;
}

/* The m12_effective_text_scale rule, exported as a pure function so
 * the audit verifies the exact contract the draw path applies:
 * base + (fontScale - 1), clamped to [1, 4]. */
static inline int m12_menu_row_effective_text_scale(int baseScale,
                                                    int fontScale) {
    int scale = baseScale;
    int fs = m12_menu_row_clamp_font_scale(fontScale);
    if (scale <= 0) return 1;
    if (fs > 1 && scale < 4) {
        scale += fs - 1;
        if (scale > 4) scale = 4;
    }
    return scale;
}

/* Presented text height in source pixels for a glyph row count at an
 * integer text scale. */
static inline int m12_menu_row_text_height_px(int glyphRows, int textScale) {
    if (glyphRows <= 0 || textScale <= 0) return 0;
    return glyphRows * textScale;
}

/* Conservative label height (11-row Unicode bound) at the effective
 * scale a base-scale-1 row label resolves to. */
static inline int m12_menu_row_label_height_px(int fontScale) {
    return m12_menu_row_text_height_px(
        M12_MENU_UNICODE_GLYPH_ROWS,
        m12_menu_row_effective_text_scale(1, fontScale));
}

/* Scale-aware classic settings-row pitch: the base 18 px pitch grows
 * so the effective hit height always contains the presented label
 * (top pad + label + bottom pad).  Never shrinks below the shipped
 * base pitch, so fontScale 1 stays bit-identical. */
static inline int m12_menu_row_settings_classic_pitch(int fontScale) {
    int needed = M12_MENU_ROW_SETTINGS_CLASSIC_TEXT_TOP
               + m12_menu_row_label_height_px(fontScale)
               + M12_MENU_ROW_SETTINGS_CLASSIC_TEXT_BOTTOM;
    return needed > M12_MENU_ROW_SETTINGS_CLASSIC_BASE_PITCH
         ? needed : M12_MENU_ROW_SETTINGS_CLASSIC_BASE_PITCH;
}

/* How many classic settings rows fit the usable panel height at this
 * fontScale.  A row only needs its TOP edge inside the panel (the
 * shipped 18 px pitch rows overlap and the last row's frame may paint
 * past the panel frame), so the count is (usable-1)/pitch + 1, which
 * pins the shipped 6 rows at fontScale 1; clamped to
 * [1, M12_MENU_ROW_SETTINGS_CLASSIC_MAX_VISIBLE]. */
static inline int m12_menu_row_settings_classic_visible_rows(int fontScale) {
    int pitch = m12_menu_row_settings_classic_pitch(fontScale);
    int rows = (M12_MENU_ROW_SETTINGS_CLASSIC_USABLE_H - 1) / pitch + 1;
    if (rows < 1) rows = 1;
    if (rows > M12_MENU_ROW_SETTINGS_CLASSIC_MAX_VISIBLE) {
        rows = M12_MENU_ROW_SETTINGS_CLASSIC_MAX_VISIBLE;
    }
    return rows;
}

/* Scale-aware save-browser pitch (text top pad 4 inside the frame,
 * bottom pad 1). */
static inline int m12_menu_row_save_browser_pitch(int fontScale) {
    int needed = M12_MENU_ROW_SAVE_BROWSER_TEXT_TOP
               + m12_menu_row_label_height_px(fontScale)
               + M12_MENU_ROW_SAVE_BROWSER_TEXT_BOTTOM;
    return needed > M12_MENU_ROW_SAVE_BROWSER_BASE_PITCH
         ? needed : M12_MENU_ROW_SAVE_BROWSER_BASE_PITCH;
}

/* Classify a presented (post-presentation-scale) hit height against
 * the touch floors. */
static inline M12_MenuRowFit m12_menu_row_classify(int presentedHeightPx) {
    if (presentedHeightPx >= M12_MENU_ROW_RECOMMENDED_TARGET_PX) {
        return M12_MENU_ROW_FIT_RECOMMENDED;
    }
    if (presentedHeightPx >= M12_MENU_ROW_MIN_TARGET_PX) {
        return M12_MENU_ROW_FIT_MINIMUM;
    }
    return M12_MENU_ROW_FIT_BELOW_MINIMUM;
}

/* Human-readable classification name for audit output. */
static inline const char* m12_menu_row_fit_name(M12_MenuRowFit fit) {
    switch (fit) {
        case M12_MENU_ROW_FIT_RECOMMENDED:   return "recommended";
        case M12_MENU_ROW_FIT_MINIMUM:       return "minimum";
        case M12_MENU_ROW_FIT_BELOW_MINIMUM: return "below-minimum";
        default:                           return "unknown";
    }
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MENU_ROW_METRICS_M12_H */
