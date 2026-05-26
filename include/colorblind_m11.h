#ifndef FIRESTAFF_COLORBLIND_M11_H
#define FIRESTAFF_COLORBLIND_M11_H

/*
 * colorblind_m11 — colorblind-friendly color remapping.
 *
 * Provides a small RGB transform applied to UI / HUD elements that the
 * caller wants daltonized. The supported modes match the existing
 * M12_Config.colorblindMode enum (0..3).
 *
 * Source: no ReDMCSB equivalent — Firestaff accessibility extra.
 * Default mode is 0 (Off), which leaves V1 launches bit-identical to
 * the original.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M11_COLORBLIND_OFF          = 0,
    M11_COLORBLIND_DEUTERANOPIA = 1,
    M11_COLORBLIND_PROTANOPIA   = 2,
    M11_COLORBLIND_TRITANOPIA   = 3,
    M11_COLORBLIND_COUNT
};

/* Returns a stable English label for the given mode, or NULL when out
 * of range. */
const char* M11_Colorblind_GetLabel(int mode);

/* Returns 1 if the mode is identity (Off) — callers may use this to
 * short-circuit and skip the remap. */
int M11_Colorblind_IsIdentity(int mode);

/* Remap a single RGB triplet in-place using a 3x3 daltonization
 * matrix selected by mode. mode==0 is a no-op. Out-of-range modes are
 * treated as Off. r/g/b may not be NULL. */
void M11_Colorblind_RemapRGB(uint8_t* r, uint8_t* g, uint8_t* b, int mode);

/* Apply daltonization to an entire RGBA buffer in place. The alpha
 * channel is left untouched. Off mode short-circuits.  rgba may be NULL
 * (no-op). width/height are in pixels. */
void M11_Colorblind_ApplyRGBA(int mode, uint8_t* rgba, int width, int height);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_COLORBLIND_M11_H */
