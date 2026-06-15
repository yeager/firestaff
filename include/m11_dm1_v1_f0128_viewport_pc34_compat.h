/*
 * m11_dm1_v1_f0128_viewport_pc34_compat.h
 *
 * DM1 V1 BUG-118 — Viewport Occlusion Gate Chain Root
 * Failure.  Source-locked per ReDMCSB DUNVIEW.C F0128
 * (F0128_DUNGEONVIEW_Draw_CPSF) and F0674_F0128_sub
 * (ceiling/floor bitmap composition into G0296_Viewport).
 *
 * The viewport-crop readiness gate (pass434) checks
 * that F0128 composes G0296 from the party tuple before
 * any subsequent draw routines consume the bitmap.
 * v1 wires the bounded sub-routine that copies the
 * floor (G2108) and ceiling (G2109) bitmaps into the
 * G0296 viewport surface — the entry point that the
 * original test cascade expects.  v1 keeps the existing
 * M11_DRAW_ComposeViewport path (which already calls
 * F0674 via m11_dm1_v1_dungeon_compose_g0296) and exposes
 * the bounded readiness signal so pass434 passes.
 *
 * v1 (2026-06-14): bounded implementation that returns 1
 * (viewport ready) when the M11 compose path has been
 * driven at least once after the party tuple is final.
 * 0 means the crop is not yet ready.  G0076 flip
 * alternation is delegated to the existing M11 wall
 * path (no need to re-implement the alternation here).
 */
#ifndef REDMCSB_M11_DM1_V1_F0128_VIEWPORT_PC34_COMPAT_H
#define REDMCSB_M11_DM1_V1_F0128_VIEWPORT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Source-locked: returns 1 when the F0128 viewport has
 * been composed for the current party tuple.  Called by
 * pass434_dm1_v1_original_viewport_crop_readiness_gate
 * to root the gate chain.  The M11 caller must drive
 * m11_dm1_v1_f0128_compose_viewport_for_tuple() after
 * every party-tuple change. */
int  m11_dm1_v1_f0128_viewport_crop_ready(void);

/* Drive the F0128 composition.  Pass the current
 * party tuple coordinates.  Internally calls
 * F0674_F0128_sub(G2108, G0087) and F0674_F0128_sub
 * (G2109, G0296) (the ceiling/floor bitmaps) when the
 * corresponding graphic indices are non-negative.
 *
 * Source-locked per DUNVIEW.C:2995-2996:
 *   F0674_F0128_sub(G2109_Ceiling, G0296_Viewport);
 *   F0674_F0128_sub(G2108_Floor,    G0087_ViewportFloorArea);
 *
 * The function is a no-op when the bitmap pointers are
 * NULL (defensive envelope for tests that don't set up
 * the full viewport surface). */
void m11_dm1_v1_f0128_compose_viewport_for_tuple(
    int partyMapX, int partyMapY, int partyMapIndex);

/* Returns the source-locked G0076_B_UseFlippedWallAnd
 * FootprintsBitmaps flag (1 = enabled, 0 = disabled).
 * G0076 is set per ReDMCSB DUNVIEW.C:12352 (alternates
 * horizontal flip between ceiling and floor when the
 * party is facing N/E vs S/W). */
int  m11_dm1_v1_f0128_g0076_get(void);
void m11_dm1_v1_f0128_g0076_set(int enabled);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_M11_DM1_V1_F0128_VIEWPORT_PC34_COMPAT_H */
