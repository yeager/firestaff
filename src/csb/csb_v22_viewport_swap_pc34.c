/*
 * csb_v22_viewport_swap_pc34.c
 *
 * Retired CSB V2.2 per-cell modern-art swap compatibility surface.
 *
 * V1 source ownership: this module never touches V1 state. When V22
 * is not the active presentation mode (modern pack missing, cache
 * not loaded, or presentation_mode != V22_MODERN), the render pass
 * is a no-op and the V1 draw pipeline stays in charge.
 *
 * A raw square byte is insufficient to identify a source raster, palette,
 * projection, clip or draw order. The public compatibility queries therefore
 * fail closed until a complete F0128 command receipt owns a replacement.
 *
 * Source-lock:
 *   CSBWin/Viewport.cpp:7290   (9-square viewport layout)
 *   ReDMCSB DUNVIEW.C F0128    (CSB viewport routing)
 *   ReDMCSB DUNGEON.C:35-44    (direction step tables)
 *   include/csb_v22_shapes.h   (parallel CSB_V22_ShapeType enum)
 *   include/csb_v22_modern_assets_pc34.h (asset pack paths + flags)
 *   include/csb_v22_inplace_draw_pc34.h (cache bitmap lookup)
 *   include/csb_v22_shape_cache_pc34.h (raw cell type store)
 *   csb_v22_inplace_draw_pc34.c (cache file format + bitmap blit)
 *   csb_v22_modern_assets_pc34.c (manifest path resolution)
 */

#include "csb_v22_viewport_swap_pc34.h"
#include "csb_v22_inplace_draw_pc34.h"
#include "csb_v22_inplace_route_pc34.h"
#include "csb_v22_shape_cache_pc34.h"
#include "csb_v22_modern_assets_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"

#include <string.h>

/* ── Module state ────────────────────────────────────────────────── */

/* Retains raw-cell observations for the sibling inspector only. The shape
 * slots stay NONE: this module is not allowed to invent an art mapping. */
typedef struct {
    CSB_V22_SwapShapeType shapes[3][3];   /* D0..D2 x L/C/R */
    unsigned char          raw_cells[3][3];
    int                   direction;
    int                   populated;
} CSB_V22_SwapCellCache;

static CSB_V22_SwapCellCache g_csb_swap_cache;

/* Counter for tests/probes — accumulated by the render pass and
 * reset on every update(). */
static int g_csb_cells_painted = 0;

/* ── Discriminator ───────────────────────────────────────────────── */

/* A byte-sized square code does not identify an F0128 raster, palette,
 * projection, or draw order. Until a caller supplies that complete original
 * command receipt, this retired per-cell route must name no V2.2 shape. */
CSB_V22_SwapShapeType csb_v22_swap_shape_for_cell(uint8_t raw_cell_type,
                                                    uint8_t direction) {
    (void)raw_cell_type;
    (void)direction;
    return CSB_V22_SWAP_SHAPE_NONE;
}

/* csb_v22_swap_asset_id_for_shape — single-row lookup over the
 * mapping table. Returns NULL when the shape has no mapping. The
 * returned string is owned by the static mapping table. */
const char* csb_v22_swap_asset_id_for_shape(CSB_V22_SwapShapeType shape) {
    (void)shape;
    return NULL;
}

/* csb_v22_swap_category_for_shape — single-row lookup over the
 * mapping table. Returns NULL when the shape has no mapping. The
 * returned string is owned by the static mapping table. */
const char* csb_v22_swap_category_for_shape(CSB_V22_SwapShapeType shape) {
    (void)shape;
    return NULL;
}

/* ── Update + activation ─────────────────────────────────────────── */

void csb_v22_viewport_swap_update(int direction,
                                    const unsigned char raw_cells[3][3]) {
    int d, l;
    g_csb_swap_cache.direction = direction;

    if (raw_cells) {
        /* Deliberately produces no modern shape without an F0128 receipt. */
        for (d = 0; d < 3; ++d) {
            for (l = 0; l < 3; ++l) {
                g_csb_swap_cache.shapes[d][l] =
                    csb_v22_swap_shape_for_cell(raw_cells[d][l], (uint8_t)direction);
                g_csb_swap_cache.raw_cells[d][l] = raw_cells[d][l];
            }
        }
    } else {
        /* No raw_cells: blank cache. */
        for (d = 0; d < 3; ++d) {
            for (l = 0; l < 3; ++l) {
                g_csb_swap_cache.shapes[d][l] = CSB_V22_SWAP_SHAPE_NONE;
                g_csb_swap_cache.raw_cells[d][l] = 0u;
            }
        }
    }

    g_csb_swap_cache.populated = 1;
    g_csb_cells_painted = 0;

    /* Forward the cells to the sibling shape cache so any other V22
     * consumer (overlay pass, debug inspector) sees the same
     * raw_cell_type + direction that the swap uses. The sibling
     * shape cache populates its own internal state from raw_cells +
     * direction, but only when V22 presentation mode is active —
     * see csb_v22_shape_cache_update() in csb_v22_shape_cache_pc34.c.
     *
     * We deliberately call the sibling only when our own
     * presentation_mode is V22. This compatibility layer never becomes a
     * renderer; the sibling cache is a parallel viewer only. */
    if (raw_cells && csb_v2_presentation_mode_is_v22()) {
        csb_v22_shape_cache_update(direction, raw_cells);
    }
}

int csb_v22_viewport_swap_populated(void) {
    return g_csb_swap_cache.populated;
}

int csb_v22_viewport_swap_active(void) {
    /* Cache presence and a modern pack do not authenticate an F0128 command. */
    return 0;
}

int csb_v22_viewport_swap_cells_painted(void) {
    return g_csb_cells_painted;
}

/* ── Render ──────────────────────────────────────────────────────── */

int csb_v22_viewport_swap_render(unsigned char* framebuffer,
                                   int fbW, int fbH) {
    /* The historical swap used synthetic 3x3 rectangles. It has no exact
     * F0128 placement or draw-order proof, so it must never replace a live
     * CSB frame. The accepted door route is consumed by the source-command
     * compositor in csb_v1_viewport_pc34_compat.c instead. */
    (void)framebuffer;
    (void)fbW;
    (void)fbH;
    return 0;
}

/* ── Source evidence ─────────────────────────────────────────────── */

const char* csb_v22_viewport_swap_source_evidence(void) {
    return "CSBWin/Viewport.cpp:7290 (9-square viewport layout); "
           "ReDMCSB DUNVIEW.C F0128 (CSB viewport routing); "
           "ReDMCSB DUNGEON.C:35-44 (direction step tables N/E/S/W); "
           "ReDMCSB LIGHT.C F0212 (CSB torchlight + lighting model); "
           "ReDMCSB PANEL.C F0354 (CSB champion panel refresh); "
           "ReDMCSB COMMAND.C:108-113/254-291 (command dispatch); "
           "ReDMCSB CLIKMENU.C:142/180 (input click routing); "
           "include/csb_v22_shapes.h (parallel CSB_V22_ShapeType enum); "
           "include/csb_v22_modern_assets_pc34.h (asset pack paths + flags); "
           "include/csb_v22_shape_cache_pc34.h (raw cell type store); "
           "csb_v1_viewport_pc34_compat.c (authenticated F0128 command compositor).";
}
