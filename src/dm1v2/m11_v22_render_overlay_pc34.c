/*
 * m11_v22_render_overlay_pc34.c
 *
 * DM1 V2.2 GPU render path: V22 modern-art overlay pass.
 * See include/m11_v22_render_overlay_pc34.h for the design contract
 * and source-lock list.
 *
 * The overlay is the second half of the V2.2 data flow:
 *   1. m11_v22_shape_cache_update (m11_draw_viewport) populates the
 *      per-frame V22 shape cache.
 *   2. m11_v22_render_overlay (this module) paints a placeholder
 *      colored rectangle over each V22-active cell.
 *
 * The placeholder is intentionally minimal (a filled rectangle +
 * 1px border) so the V2.2 dispatch path is testable end-to-end
 * without the actual modern asset pack. The real V22 modern art
 * (PBR textures, normal maps, etc.) is a follow-up.
 */
#include "m11_v22_render_overlay_pc34.h"
#include "dm1_v2_lighting_dynamic_pc34.h"
#include "m11_v22_shape_cache_pc34.h"

#include <string.h>

/* V1 framebuffer cell rects for the 9 sampled cells. These are
 * approximate viewport-relative sub-regions, derived from the
 * V1 frames[4] depth rects. The 4×3 layout is:
 *   D3 (back, top of viewport)   — y = 41..71  (h=30)
 *   D2 (middle)                  — y = 72..102 (h=30)
 *   D1 (front, bottom of viewport) — y = 103..133 (h=30)
 * Each depth is split into 3 laterals (L, C, R) of equal width.
 *
 * The viewport starts at x=8, so the cells start there too.
 *   L: x = 8..77  (w=69)
 *   C: x = 78..138 (w=61)
 *   R: x = 139..208 (w=69)
 *
 * 1-pixel gaps separate cells horizontally and vertically. */
static const M11_V22_CellRect kV22CellRects[3][3] = {
    /* depth 0 = D1 (closest) */ {
        {  8, 103, 69, 30 },  /* D1L */
        { 78, 103, 61, 30 },  /* D1C */
        {139, 103, 69, 30 }   /* D1R */
    },
    /* depth 1 = D2 (middle) */ {
        {  8,  72, 69, 30 },  /* D2L */
        { 78,  72, 61, 30 },  /* D2C */
        {139,  72, 69, 30 }   /* D2R */
    },
    /* depth 2 = D3 (back) */ {
        {  8,  41, 69, 30 },  /* D3L */
        { 78,  41, 61, 30 },  /* D3C */
        {139,  41, 69, 30 }   /* D3R */
    }
};

/* Fill a horizontal span of the framebuffer with a single color. */
static void v22_overlay_hline(unsigned char* framebuffer,
                              int fbW, int fbH,
                              int x, int y, int w,
                              unsigned char color) {
    int i;
    if (y < 0 || y >= fbH || w <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > fbW) { w = fbW - x; }
    for (i = 0; i < w; ++i) {
        framebuffer[y * fbW + (x + i)] = color;
    }
}

/* Fill a vertical span of the framebuffer with a single color. */
static void v22_overlay_vline(unsigned char* framebuffer,
                              int fbW, int fbH,
                              int x, int y, int h,
                              unsigned char color) {
    int i;
    if (x < 0 || x >= fbW || h <= 0) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > fbH) { h = fbH - y; }
    for (i = 0; i < h; ++i) {
        framebuffer[(y + i) * fbW + x] = color;
    }
}

/* Draw a filled rectangle + 1-pixel border. */
static void v22_overlay_fill_rect(unsigned char* framebuffer,
                                 int fbW, int fbH,
                                 int x, int y, int w, int h,
                                 unsigned char fill, unsigned char border) {
    int row;
    if (w <= 0 || h <= 0) return;
    /* Fill. */
    for (row = y; row < y + h; ++row) {
        v22_overlay_hline(framebuffer, fbW, fbH, x, row, w, fill);
    }
    /* Border. */
    v22_overlay_hline(framebuffer, fbW, fbH, x, y, w, border);
    v22_overlay_hline(framebuffer, fbW, fbH, x, y + h - 1, w, border);
    v22_overlay_vline(framebuffer, fbW, fbH, x, y, h, border);
    v22_overlay_vline(framebuffer, fbW, fbH, x + w - 1, y, h, border);
}

static unsigned char v22_overlay_apply_source_palette_shadow(
    unsigned char color,
    int sourcePaletteIndex)
{
    M11_V2_SourcePaletteLighting lighting =
        v2_light_build_source_palette_lighting(sourcePaletteIndex, 1);
    unsigned int darken = ((unsigned int)lighting.shadow_alpha * 3u) / 8u;
    if (darken >= (unsigned int)color) {
        return 1u;
    }
    return (unsigned char)((unsigned int)color - darken);
}

static unsigned char v22_overlay_material_index_for_shape(
    const DM1_V2_ShapeRuntimeResult* r)
{
    if (!r || !r->active) {
        return M11_V22_OVERLAY_PLACEHOLDER_INDEX;
    }
    switch (r->params.type) {
        case M11_V22_SHAPE_WALL_STRAIGHT:
        case M11_V22_SHAPE_WALL_CORNER_INNER:
        case M11_V22_SHAPE_WALL_CORNER_OUTER:
        case M11_V22_SHAPE_WALL_DOORWAY:
        case M11_V22_SHAPE_WALL_ALCOVE:
        case M11_V22_SHAPE_WALL_INSCRIPTION:
            return 0x70u;
        case M11_V22_SHAPE_FLOOR_PLAIN:
        case M11_V22_SHAPE_FLOOR_CRACKED:
        case M11_V22_SHAPE_FLOOR_MOSSY:
        case M11_V22_SHAPE_FLOOR_DOOR:
        case M11_V22_SHAPE_CEILING_PLAIN:
        case M11_V22_SHAPE_CEILING_VAULTED:
            return 0x50u;
        case M11_V22_SHAPE_FLOOR_PIT:
            return 0x18u;
        case M11_V22_SHAPE_FLOOR_STAIRS_UP:
        case M11_V22_SHAPE_FLOOR_STAIRS_DOWN:
            return 0x90u;
        case M11_V22_SHAPE_FIELD_TELEPORTER:
        case M11_V22_SHAPE_FIELD_FLUXCAGE:
        case M11_V22_SHAPE_FIELD_EXPLOSION:
            return 0xc0u;
        case M11_V22_SHAPE_CREATURE:
        case M11_V22_SHAPE_CREATURE_PROJECTILE:
            return 0xe0u;
        case M11_V22_SHAPE_ITEM:
        case M11_V22_SHAPE_ITEM_FLOOR:
        case M11_V22_SHAPE_ITEM_PROJECTILE:
            return 0xa8u;
        default:
            break;
    }
    return (unsigned char)((r->params.color_tint[0] +
                            r->params.color_tint[1] +
                            r->params.color_tint[2]) / 3);
}

int m11_v22_render_overlay_with_palette(unsigned char* framebuffer,
                                        int fbW,
                                        int fbH,
                                        int sourcePaletteIndex) {
    int depth, lateral;
    int cells_painted = 0;
    if (!framebuffer || fbW <= 0 || fbH <= 0) return 0;
    if (!m11_v22_shape_cache_populated()) return 0;
    for (depth = 0; depth < 3; ++depth) {
        for (lateral = -1; lateral <= 1; ++lateral) {
            const DM1_V2_ShapeRuntimeResult* r =
                m11_v22_shape_cache_get(depth + 1, lateral);
            if (!r || !r->active) continue;
            {
                const M11_V22_CellRect* rect = &kV22CellRects[depth][lateral + 1];
                /* Placeholder fill: derive a stable material category from
                 * the V22 shape type. This keeps the no-asset fallback bound
                 * to the same source square/material classification as the
                 * real in-place V22 asset route. */
                unsigned char placeholder = v22_overlay_material_index_for_shape(r);
                if (placeholder == 0) placeholder = M11_V22_OVERLAY_PLACEHOLDER_INDEX;
                placeholder = v22_overlay_apply_source_palette_shadow(
                    placeholder, sourcePaletteIndex);
                v22_overlay_fill_rect(framebuffer, fbW, fbH,
                                       rect->x, rect->y, rect->w, rect->h,
                                       placeholder, M11_V22_OVERLAY_PLACEHOLDER_INDEX);
                cells_painted++;
            }
        }
    }
    return cells_painted;
}

int m11_v22_render_overlay(unsigned char* framebuffer, int fbW, int fbH) {
    return m11_v22_render_overlay_with_palette(framebuffer, fbW, fbH, 0);
}

const char* m11_v22_render_overlay_source_evidence(void) {
    return
        "DM1 V2.2 GPU render path: V22 modern-art overlay (m11_v22_render_overlay).\n"
        "  Second half of the V2.2 data flow: m11_v22_shape_cache_update populates\n"
        "  the per-frame cache, m11_v22_render_overlay paints a placeholder over\n"
        "  each V22-active cell. The placeholder is a filled rectangle using a\n"
        "  palette index derived from the V22 shape/material category, source-palette\n"
        "  shadowed through PANEL.C:F0337/DATA.C:359-360, with a 1-pixel\n"
        "  border. The real V22 modern art (PBR textures, normal maps, etc.)\n"
        "  is a follow-up; this overlay proves the data flow end-to-end.\n"
        "  Source: include/m11_v22_shape_cache_pc34.h + ReDMCSB DUNVIEW.C:6697-6816.\n";
}
