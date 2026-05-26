#ifndef FIRESTAFF_COLOR_PRESETS_M11_H
#define FIRESTAFF_COLOR_PRESETS_M11_H

/*
 * color_presets_m11 — color grading presets for the M11 RGBA present
 * buffer.
 *
 * A preset is a 256-entry per-channel LUT (RGB) that is applied as a
 * straight lookup on the RGBA presentBuffer after the existing V2.0
 * filter chain.  Preset 0 (Original) is a hard-coded identity and the
 * RGBA path short-circuits when it is selected, so V1 launches stay
 * bit-identical to the original.
 *
 * Source: no ReDMCSB equivalent — color grading is a Firestaff V2.0
 * presentation extra. See DM1_V2_PLAN.md (visual extras).
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define M11_COLOR_PRESET_COUNT 7

enum {
    M11_COLOR_PRESET_ORIGINAL = 0,
    M11_COLOR_PRESET_ATARI_ST_WARM = 1,
    M11_COLOR_PRESET_AMIGA_GOLD = 2,
    M11_COLOR_PRESET_VGA_CLEAN = 3,
    M11_COLOR_PRESET_SEPIA = 4,
    M11_COLOR_PRESET_HIGH_CONTRAST = 5,
    M11_COLOR_PRESET_COOL_BLUE = 6
};

/* Returns a stable English label for the given preset index, or NULL
 * when out of range. */
const char* M11_ColorPreset_GetLabel(int preset);

/* Returns 1 if preset is a valid index in [0, M11_COLOR_PRESET_COUNT). */
int M11_ColorPreset_IsValid(int preset);

/* Returns 1 if the preset is the identity (Original) — callers can use
 * this to skip the LUT pass entirely. */
int M11_ColorPreset_IsIdentity(int preset);

/* Apply preset to the given RGBA buffer in place. width*height pixels,
 * 4 bytes/pixel (R,G,B,A) with the alpha channel left untouched.  Safe
 * for null/invalid preset (no-op).  Identity preset short-circuits. */
void M11_ColorPreset_ApplyRGBA(int preset, unsigned char* rgba, int width, int height);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_COLOR_PRESETS_M11_H */
