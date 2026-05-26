#ifndef FIRESTAFF_UI_SCALE_M11_H
#define FIRESTAFF_UI_SCALE_M11_H

/*
 * ui_scale_m11 — runtime UI scale factor for HUD and menu text.
 *
 * The original DM1 font in font_m11 already accepts a `scale` integer
 * (nearest-neighbor upscaling of each glyph). This module owns the
 * global scale state that HUD and menu callers use to read the
 * "scale" they should pass to M11_Font_DrawString, plus helpers to
 * convert between the percent-based config (100/150/200) and the
 * integer font scale value.
 *
 * Source: no ReDMCSB equivalent — Firestaff accessibility extra.
 * Default percent is 100, which always maps to font scale 1 (= the
 * original presentation), so V1 launches stay bit-identical.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M11_UI_SCALE_PERCENT_100 = 100,
    M11_UI_SCALE_PERCENT_150 = 150,
    M11_UI_SCALE_PERCENT_200 = 200
};

/* Set / get the global UI scale percent. Values are snapped to one of
 * {100, 150, 200}. Default 100. */
void M11_UIScale_SetPercent(int percent);
int  M11_UIScale_GetPercent(void);

/* Normalize an arbitrary percent value to the supported set
 * {100, 150, 200}. Useful for parsing config values. */
int  M11_UIScale_NormalizePercent(int percent);

/* Convert the current percent to a font scale step compatible with
 * M11_Font_DrawString / DrawChar. 100% -> 1, 150% -> 2, 200% -> 3.
 * The 150% bump uses an integer-only scale-2 step because the bitmap
 * font has no sub-pixel glyph grid; this keeps glyph edges crisp. */
int  M11_UIScale_GetFontScale(void);

/* Same conversion as a pure function for callers that already have a
 * percent in hand (e.g. when building previews). */
int  M11_UIScale_PercentToFontScale(int percent);

/* Convenience: multiply a coordinate or size by the current scale
 * factor. The math is integer ((value * percent + 50) / 100) to keep
 * pixel positions stable across frames. */
int  M11_UIScale_Apply(int value);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_UI_SCALE_M11_H */
