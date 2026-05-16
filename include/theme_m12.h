/*
 * theme_m12.h
 *
 * Theme selector for the M12 launcher.  Provides a small set of
 * pre-defined colour palettes that the modern menu renderer can apply
 * to the startup menu chrome.  In-game V1 rendering is unaffected.
 */

#ifndef FIRESTAFF_THEME_M12_H
#define FIRESTAFF_THEME_M12_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M12_THEME_CLASSIC = 0,
    M12_THEME_DARK,
    M12_THEME_AMIGA,
    M12_THEME_CGA,
    M12_THEME_COUNT
} M12_Theme;

typedef struct {
    unsigned char bg_r, bg_g, bg_b;
    unsigned char fg_r, fg_g, fg_b;
    unsigned char accent_r, accent_g, accent_b;
    unsigned char panel_r, panel_g, panel_b;
} M12_ThemeColors;

/* Return the colour palette for the given theme index.
 * Out-of-range indices are clamped to M12_THEME_CLASSIC. */
const M12_ThemeColors* M12_Theme_GetColors(M12_Theme theme);

/* Return a human-readable label for the theme (never NULL). */
const char* M12_Theme_GetLabel(M12_Theme theme);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THEME_M12_H */
