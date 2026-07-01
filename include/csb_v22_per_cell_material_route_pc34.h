/*
 * csb_v22_per_cell_material_route_pc34.h
 *
 * CSB V2.2 GPU render path: per-cell MODULE routing gate.
 *
 * Where csb_v22_inplace_draw_pc34.{c,h} maps each (depth, lateral)
 * cell to a single bitmap asset_id and csb_v22_shape_cache_pc34 caches
 * the V22 ShapeParams (CSB_V22_ShapeType + material_id + color_tint),
 * this module is the *material-routing gate*: it walks the 9-square
 * viewport, resolves each cell's material slot (diffuse + normal +
 * specular + emission texture IDs, plus roughness/metallic/emission
 * scalars) against the loaded V22 in-place bitmap cache, and reports
 * a per-cell routing verdict that distinguishes:
 *
 *   - ROUTE_OK                 every PBR channel resolves (synthetic
 *                              CI baseline; real art promotes from here)
 *   - MISSING_DIFFUSE          no diffuse texture for this cell
 *   - MISSING_NORMAL           diffuse present, normal map missing
 *   - MISSING_SPECULAR         diffuse+normal present, specular missing
 *   - MISSING_EMISSION         diffuse+normal+specular present, emission missing
 *   - NO_BITMAP_CACHE          in-place cache not loaded (V22 assets missing)
 *   - NOT_POPULATED            shape cache not populated for this cell
 *   - V1_INACTIVE              presentation_mode != V22 (gate off)
 *
 * The verdict is the seam between the runtime (which only needs to
 * know "is the cell routable?") and an operator-facing tool (which
 * needs to know "which PBR channels are still missing for the
 * per-cell swap to be observably complete?"). The same verdict is
 * surfaced via csb_v22_per_cell_route_summary() so the CI probe can
 * gate on the FINISHED_REAL promotion criterion.
 *
 * Material slot table (9 cells):
 *
 *   D2 (farthest):  L=lighter  C=center  R=lighter
 *   D1 (middle):    L=lighter  C=center  R=lighter
 *   D0 (closest):   L=lighter  C=center  R=lighter
 *
 * The center column (lateral=0) maps to the canonical material book
 * (plain stone floor + iron walls). The lateral columns get a
 * slightly distinct material slot so the per-cell swap is observable
 * without a per-cell art pack (lateral bands light up different
 * texture subsets of the loaded cache).
 *
 * This module is intentionally narrow:
 *   - It does NOT touch V1 state.
 *   - It does NOT paint pixels (use csb_v22_inplace_render_pass).
 *   - It does NOT decide presentation mode (use csb_v2_presentation_mode_is_v22).
 *
 * Source-lock:
 *   ReDMCSB DUNVIEW.C F0128  CSB 9-square viewport routing
 *   ReDMCSB LIGHT.C   F0212  CSB torchlight + lighting model
 *   ReDMCSB PANEL.C   F0354  CSB champion panel refresh (champion material slots)
 *   CSBWin/Viewport.cpp:7290 9-square grid mapping
 *   CSBWin/Chaos.cpp:60-69   DSA / chaos rune dispatch (CSB-only material)
 *   include/csb_v22_shapes.h            material book (10 materials)
 *   include/csb_v22_shape_cache_pc34.h  per-cell V22 shape cache
 *   include/csb_v22_inplace_draw_pc34.h in-place bitmap cache
 *   include/csb_v22_modern_assets_pc34.h asset pack paths + flags
 *
 * Module: src/csb/csb_v22_per_cell_material_route_pc34.c
 * Test:   tests/test_csb_v22_per_cell_material_route_pc34.c
 * Probe:  probes/firestaff_csb_v22_per_cell_material_route_probe.c
 */

#ifndef FIRESTAFF_CSB_V22_PER_CELL_MATERIAL_ROUTE_PC34_H
#define FIRESTAFF_CSB_V22_PER_CELL_MATERIAL_ROUTE_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Per-cell routing verdict. The enum is kept narrow so the verdict
 * can drive both runtime short-circuits (ROUTE_OK == ready to render)
 * and CI promotion (NOT_POPULATED / NO_BITMAP_CACHE / MISSING_* can be
 * reasons to keep the cell on the V1 placeholder path). */
typedef enum {
    CSB_V22_ROUTE_OK            = 0,  /* every PBR channel resolves */
    CSB_V22_ROUTE_MISSING_DIFFUSE  = 1,  /* no diffuse texture */
    CSB_V22_ROUTE_MISSING_NORMAL   = 2,  /* no normal map */
    CSB_V22_ROUTE_MISSING_SPECULAR = 3,  /* no specular map */
    CSB_V22_ROUTE_MISSING_EMISSION = 4,  /* no emission map */
    CSB_V22_ROUTE_NO_BITMAP_CACHE  = 5,  /* in-place cache not loaded */
    CSB_V22_ROUTE_NOT_POPULATED    = 6,  /* shape cache not populated */
    CSB_V22_ROUTE_V1_INACTIVE      = 7,  /* presentation_mode != V22 */
    CSB_V22_ROUTE_COUNT
} CSB_V22_RouteVerdict;

/* Per-cell material slot — what the runtime would draw for a cell
 * if every PBR channel resolved. The slot captures the texture
 * identifiers (diffuse / normal / specular / emission) and the
 * material scalars (roughness / metallic / emission_strength) that
 * the runtime needs to commit before the actual GPU draw.
 *
 * The slot is intentionally read-only (the verdict belongs to the
 * gate, not the slot). The runtime reads texture_id and the
 * scalars; the verifier reads the verdict and the missing-channel
 * bitmask. */
typedef struct {
    int      diffuse_texture_id;   /* 0 = unset */
    int      normal_texture_id;    /* 0 = unset (allowed: not every cell needs normal) */
    int      specular_texture_id;  /* 0 = unset (allowed: not every cell needs specular) */
    int      emission_texture_id;  /* 0 = unset (allowed: not every cell needs emission) */
    float    roughness;            /* [0.0, 1.0] */
    float    metallic;             /* [0.0, 1.0] */
    float    emission_strength;    /* >= 0.0 */
    int      cell_type;            /* CSB raw cell type (M034_SQUARE_TYPE) */
    uint8_t  depth;                /* 0..2 */
    int8_t   lateral;              /* -1 / 0 / +1 */
} CSB_V22_MaterialSlot;

/* Per-cell route — slot + verdict, plus a stable ordinal so the
 * probe can iterate cells in the same order the runtime does
 * (D0L, D0C, D0R, D1L, D1C, D1R, D2L, D2C, D2R). */
typedef struct {
    int                ordinal;     /* 0..8 */
    CSB_V22_MaterialSlot slot;
    CSB_V22_RouteVerdict verdict;
    /* The "expected" channel set the runtime would ask for, so a
     * MISSING_EMISSION on a cell that never asked for emission is
     * a false positive and should not count toward the verdict. */
    int                expected_channels;  /* bitmask (1=diffuse, 2=normal,
                                              4=specular, 8=emission) */
} CSB_V22_PerCellRoute;

/* Population entry point: walks the per-cell V22 shape cache and
 * fills the supplied route array (must be at least 9 entries).
 * Returns the number of cells populated (always 9 on success, 0
 * if the V22 shape cache has not been populated).
 *
 * The function is read-only with respect to the V22 shape cache;
 * it does not call csb_v22_shape_cache_update() itself. Callers
 * should populate the shape cache first, then call this function
 * to walk the per-cell route. */
int csb_v22_per_cell_material_route_populate(
    CSB_V22_PerCellRoute out_routes[9]);

/* Verdict lookup by ordinal. Returns CSB_V22_ROUTE_NOT_POPULATED
 * if no populate has been called yet. */
CSB_V22_RouteVerdict csb_v22_per_cell_route_verdict(int ordinal);

/* Slot lookup by ordinal. Returns NULL if no populate has been
 * called yet. The returned pointer is owned by module state and
 * remains valid until the next populate call. */
const CSB_V22_MaterialSlot* csb_v22_per_cell_route_slot(int ordinal);

/* Coverage summary for CI promotion gates. Fills:
 *   - out_total_cells        (always 9 once populated)
 *   - out_route_ok_cells     (number of cells with verdict == ROUTE_OK)
 *   - out_missing_diffuse    (number of cells missing diffuse texture)
 *   - out_missing_normal     (number of cells missing normal map)
 *   - out_missing_specular   (number of cells missing specular map)
 *   - out_missing_emission   (number of cells missing emission map)
 *   - out_v1_inactive_cells  (number of cells where V22 is not active)
 *   - out_not_populated_cells (number of cells where the shape cache
 *                              was not populated)
 *
 * Any out_ pointer may be NULL. The function returns 1 if the
 * summary was filled, 0 if no populate has been called yet. */
int csb_v22_per_cell_route_summary(int* out_total_cells,
                                    int* out_route_ok_cells,
                                    int* out_missing_diffuse,
                                    int* out_missing_normal,
                                    int* out_missing_specular,
                                    int* out_missing_emission,
                                    int* out_v1_inactive_cells,
                                    int* out_not_populated_cells);

/* Reset module state (called by probe teardown so a fresh populate
 * starts from a clean baseline). */
void csb_v22_per_cell_material_route_reset(void);

/* Human-readable name for a verdict (used by probe diagnostics).
 * Returns the static string "?" for unknown verdicts. */
const char* csb_v22_per_cell_route_verdict_name(CSB_V22_RouteVerdict v);

/* Source evidence for tests/probes. */
const char* csb_v22_per_cell_material_route_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V22_PER_CELL_MATERIAL_ROUTE_PC34_H */
