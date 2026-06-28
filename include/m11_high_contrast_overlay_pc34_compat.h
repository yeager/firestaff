#ifndef FIRESTAFF_M11_HIGH_CONTRAST_OVERLAY_PC34_COMPAT_H
#define FIRESTAFF_M11_HIGH_CONTRAST_OVERLAY_PC34_COMPAT_H

/*
 * m11_high_contrast_overlay_pc34_compat.h
 *
 * M11 in-game high-contrast overlay gate.
 *
 * Closes the gap "High-contrast presentation hardening: launcher
 * output is remapped to a restricted high-contrast palette;
 * remaining work is in-game overlay coverage." (TODO.md /
 * docs/FIRESTAFF_GAP_LIST.md)
 *
 * What this module does:
 *
 *   1. Holds a single global boolean (the "overlay gate") sourced
 *      from M12_Config.highContrast. Default off.
 *   2. Exposes M11_HighContrast_RemapPresentedColor() that M11
 *      chrome / dialog / action-area / hit-flash / log callers use
 *      INSTEAD of hard-coded M11_COLOR_* constants. When the gate
 *      is off this is a strict identity function, so V1 launches
 *      stay bit-identical.
 *   3. Exposes a "manifest" — the documented contract for which
 *      M11 surfaces the gate covers and which it deliberately does
 *      NOT cover. The V1 dungeon viewport (320x200 indexed
 *      framebuffer pixels painted from GRAPHICS.DAT) is excluded,
 *      so original fidelity is preserved.
 *   4. Exposes M11_HighContrast_ApplyActiveRGBA() for callers that
 *      already hold a framebuffer pointer and want to remap chrome
 *      pixels only (NOT viewport pixels).
 *   5. Exposes M11_HighContrast_ApplyActiveRGBAExceptRect() for
 *      callers that need an explicit rectangle fence around the
 *      dungeon viewport instead of relying only on palette-index
 *      exclusions. This is the direct viewport-fence helper for
 *      broad chrome-overlay passes.
 *
 * What this module does NOT do (kept honest in the manifest):
 *
 *   - It does not touch raw 320x200 dungeon-viewport pixels. The
 *     original V1/V2 palette is preserved bit-exact because users
 *     that need pixel-perfect originals (capture parity, archive
 *     screenshots, ROM/render-trace matching) cannot accept a
 *     runtime palette remap on the dungeon scene.
 *   - It does not introduce a second palette-rewrite pass on the
 *     full presentBuffer. The existing M11_ColorPreset_ApplyRGBA
 *     pipeline already handles V2.0 color grading via
 *     M11_Render_SetColorPreset; this module is the M11-side
 *     counterpart that runs ONLY on chrome/dialog/log pixels.
 *   - It does not change font glyphs. The bitmap font in
 *     font_m11.c stays as-is; only its foreground color index is
 *     remapped when the gate is on.
 *
 * Source-lock: this is pure accessibility glue; no ReDMCSB
 * equivalent. The launcher side of the same contract lives in
 * m12_presented_color() inside src/ui/menu_startup_m12.c and
 * already pairs every DM1 PC 3.4 palette slot (BLACK/NAVY/MAROON/
 * BROWN/DARK_GRAY → BLACK, YELLOW → YELLOW, LIGHT_CYAN/CYAN →
 * LIGHT_CYAN, default → WHITE). The remap table below mirrors
 * that contract verbatim so launcher and game surfaces share the
 * same restricted palette when the gate is on.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Gate state ────────────────────────────────────────────────────── */

/* Set the global in-game overlay gate. The launcher (M12) pushes
 * M12_Config.highContrast here at launch so the same toggle that
 * remaps the launcher also gates the M11 chrome surface. Default
 * state is OFF. */
void M11_HighContrast_SetActive(int active);

/* Read the global gate state. Returns 1 when active, 0 when off. */
int  M11_HighContrast_IsActive(void);

/* Manifest accessor. Returns a stable C string describing which
 * surfaces the gate covers and which it deliberately preserves.
 * Used by tests, the runtime probe, and the docs gap-list note.
 * The string is owned by the module and must not be freed. */
const char* M11_HighContrast_GetManifest(void);

/* ── Color remap (chrome / dialog / log / hit-flash / action area) ── */

/* Remap a single M11_COLOR_* palette index for a chrome/dialog/log
 * caller. When the gate is off this is the identity function; when
 * on, it collapses muted slots to BLACK and unknown slots to
 * WHITE so chrome stays readable on the indexed framebuffer.
 *
 * The 8-bit input index follows the M11 palette convention
 * declared in src/engine/m11_game_view.c:
 *   0  BLACK, 1  GRAY,   2  LIGHT_GRAY, 3  BROWN,
 *   4  CYAN/LIGHT_CYAN, 5  DARK_BROWN, 6  GREEN,
 *   7  LIGHT_GREEN, 8  RED, 9  LIGHT_RED/ORANGE,
 *  10  MAGENTA, 11 YELLOW, 12 DARK_GRAY, 13 SILVER,
 *  14 NAVY/LIGHT_BLUE, 15 WHITE.
 *
 * The function never returns a value outside [0, 15]. */
unsigned char M11_HighContrast_RemapPresentedColor(unsigned char color);

/* Apply the gate's chrome-only remap to a region of an indexed
 * framebuffer. Pixels whose original index appears in `excludeMask`
 * (a bitmask, bit n = preserve index n) are not touched; this lets
 * callers fence off the 320x200 dungeon-viewport subrect while
 * remapping the HUD chrome around it.
 *
 * Returns 1 if any pixel was remapped, 0 if the gate was off or
 * the inputs were invalid. The function never writes pixels outside
 * the supplied (x, y, width, height) rect. */
int  M11_HighContrast_ApplyActiveRGBA(unsigned char* framebuffer,
                                      int framebufferWidth,
                                      int framebufferHeight,
                                      int x, int y, int width, int height,
                                      unsigned int excludeMask);

/* Same indexed-framebuffer remap as M11_HighContrast_ApplyActiveRGBA(),
 * but skips every pixel inside the supplied preserve rectangle
 * BEFORE consulting excludeMask. This is the direct viewport-fence
 * helper for callers that draw one broad chrome overlay pass around
 * the dungeon scene: (preserveX, preserveY, preserveWidth,
 * preserveHeight) is never modified, even if its pixels use muted
 * palette slots that would otherwise collapse to BLACK.
 *
 * A non-positive preserveWidth / preserveHeight disables the
 * rectangle fence and leaves only excludeMask active. Returns 1 if
 * any pixel outside the preserve rectangle was remapped, 0 otherwise.
 * The function never writes pixels outside the supplied (x, y,
 * width, height) rect. */
int  M11_HighContrast_ApplyActiveRGBAExceptRect(unsigned char* framebuffer,
                                                int framebufferWidth,
                                                int framebufferHeight,
                                                int x, int y, int width, int height,
                                                int preserveX,
                                                int preserveY,
                                                int preserveWidth,
                                                int preserveHeight,
                                                unsigned int excludeMask);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_HIGH_CONTRAST_OVERLAY_PC34_COMPAT_H */
