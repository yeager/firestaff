/* test_m11_v22_render_overlay_pc34.c
 *
 * DM1 V2.2 GPU render path: V22 modern-art overlay unit test.
 * Verifies that the overlay:
 *   - Is a no-op when V22 is inactive (V1 path).
 *   - Paints 9 cells (D1..D3, L/C/R) when V22 is active.
 *   - Writes a material-category fill, optionally source-palette
 *     shadowed, to the right framebuffer region.
 *   - Has a 1-pixel border.
 *   - Returns 0 cells painted when the cache is unpopulated.
 */
#include "m11_v22_render_overlay_pc34.h"
#include "m11_v22_shape_cache_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"

#include <stdio.h>
#include <string.h>

static int g_failed = 0;
static int g_total = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void t_v1_inactive(void) {
    /* V1 default: cache reports active=0; overlay paints 0 cells. */
    unsigned char fb[320 * 200];
    memset(fb, 0x00, sizeof(fb));
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_reset();
    /* Manually mark the cache as populated but cells inactive by
     * updating with a V1 setup (no actual populate function in this
     * test path, so we call update with V1 active). */
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});
    int n = m11_v22_render_overlay(fb, 320, 200);
    check(n == 0, "V1 inactive -> 0 cells painted");
    /* All pixels should still be 0 (no draw). */
    int all_zero = 1;
    for (int i = 0; i < 320 * 200; ++i) {
        if (fb[i] != 0x00) { all_zero = 0; break; }
    }
    check(all_zero, "V1 inactive -> no pixels changed");
}

static void t_unpopulated(void) {
    /* Cache not populated: overlay returns 0 cells without crashing. */
    unsigned char fb[320 * 200];
    memset(fb, 0x00, sizeof(fb));
    m11_v22_shape_cache_reset();
    check(m11_v22_shape_cache_populated() == 0,
          "unpopulated: cache reset clears populated flag");
    check(m11_v22_render_overlay(fb, 320, 200) == 0,
          "unpopulated: overlay paints 0 cells");
    check(fb[0] == 0x00 && fb[(200 * 320) - 1] == 0x00,
          "unpopulated: framebuffer unchanged at bounds");
}

static void t_v22_paints_9_cells(void) {
    unsigned char fb[320 * 200];
    memset(fb, 0x00, sizeof(fb));
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    /* Cache with all-zeros raw squares. The V22 cells will still be
     * active (m11_v22_shape_for_cell returns active=1 for any input). */
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});
    int n = m11_v22_render_overlay(fb, 320, 200);
    check(n == 9, "V22 active -> 9 cells painted");
    /* The cell rects are at (8, 103, 69, 30) etc. Spot-check that
     * the D1L cell's center is filled with a non-zero color. */
    int d1l_x = 8 + 35;  /* center of D1L rect */
    int d1l_y = 103 + 15; /* center of D1L rect */
    int idx = d1l_y * 320 + d1l_x;
    check(fb[idx] != 0x00, "V22: D1L center filled (non-zero)");
    /* The D3C cell (top-center) should also be filled. */
    int d3c_x = 78 + 30;
    int d3c_y = 41 + 15;
    idx = d3c_y * 320 + d3c_x;
    check(fb[idx] != 0x00, "V22: D3C center filled (non-zero)");
}

static void t_v22_placeholder_index(void) {
    /* When the V22 color_tint is all-zero, the placeholder uses
     * M11_V22_OVERLAY_PLACEHOLDER_INDEX (0xFF). */
    unsigned char fb[320 * 200];
    memset(fb, 0x00, sizeof(fb));
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});
    int n = m11_v22_render_overlay(fb, 320, 200);
    check(n == 9, "V22 placeholder: 9 cells painted");
    /* The D1L border is at (8, 103) (top-left corner of the rect).
     * The 1-pixel border uses M11_V22_OVERLAY_PLACEHOLDER_INDEX. */
    int idx = 103 * 320 + 8;
    check(fb[idx] == M11_V22_OVERLAY_PLACEHOLDER_INDEX,
          "V22: D1L top-left border = placeholder index");
}

static void t_v22_source_palette_shadow(void) {
    unsigned char bright[320 * 200];
    unsigned char dark[320 * 200];
    int d1l_x = 8 + 35;
    int d1l_y = 103 + 15;
    int center = d1l_y * 320 + d1l_x;
    int border = 103 * 320 + 8;
    memset(bright, 0x00, sizeof(bright));
    memset(dark, 0x00, sizeof(dark));
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});

    check(m11_v22_render_overlay_with_palette(bright, 320, 200, 0) == 9,
          "V22 palette shadow: bright paints 9 cells");
    check(m11_v22_render_overlay_with_palette(dark, 320, 200, 5) == 9,
          "V22 palette shadow: dark paints 9 cells");
    check(dark[center] < bright[center],
          "V22 palette shadow: palette 5 darkens center fill");
    check(dark[border] == M11_V22_OVERLAY_PLACEHOLDER_INDEX,
          "V22 palette shadow: border remains placeholder index");
}

static void t_v22_material_categories(void) {
    unsigned char fb[320 * 200];
    unsigned char raw_cells[3][3] = {
        { 0x00, 0x20, 0x40 },
        { 0x60, 0x80, 0xa0 },
        { 0x00, 0x20, 0x40 }
    };
    unsigned char wall;
    unsigned char floor;
    unsigned char pit;
    unsigned char stairs;
    unsigned char field;
    memset(fb, 0x00, sizeof(fb));
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, (const unsigned char (*)[3])raw_cells);

    check(m11_v22_render_overlay_with_palette(fb, 320, 200, 0) == 9,
          "V22 material categories: paints 9 cells");
    wall = fb[(103 + 15) * 320 + (8 + 35)];
    floor = fb[(103 + 15) * 320 + (78 + 30)];
    pit = fb[(103 + 15) * 320 + (139 + 35)];
    stairs = fb[(72 + 15) * 320 + (8 + 35)];
    field = fb[(72 + 15) * 320 + (139 + 35)];
    check(wall != floor, "V22 material categories: wall differs from floor");
    check(floor != pit, "V22 material categories: floor differs from pit");
    check(stairs != floor, "V22 material categories: stairs differs from floor");
    check(field != floor, "V22 material categories: field differs from floor");
}

static void t_cell_rect_api(void) {
    const M11_V22_CellRect* d1l = m11_v22_cell_rect(1, -1);
    const M11_V22_CellRect* d2c = m11_v22_cell_rect(2, 0);
    const M11_V22_CellRect* d3r = m11_v22_cell_rect(3, 1);
    check(d1l != NULL && d1l->x == 8 && d1l->y == 103 &&
              d1l->w == 69 && d1l->h == 30,
          "V22 cell rect API: D1L source geometry");
    check(d2c != NULL && d2c->x == 78 && d2c->y == 72 &&
              d2c->w == 61 && d2c->h == 30,
          "V22 cell rect API: D2C source geometry");
    check(d3r != NULL && d3r->x == 139 && d3r->y == 41 &&
              d3r->w == 69 && d3r->h == 30,
          "V22 cell rect API: D3R source geometry");
    check(m11_v22_cell_rect(0, 0) == NULL,
          "V22 cell rect API: rejects depth 0");
    check(m11_v22_cell_rect(1, 2) == NULL,
          "V22 cell rect API: rejects lateral 2");
}

static void t_null_safe(void) {
    /* NULL framebuffer: no crash, 0 cells painted. */
    int n = m11_v22_render_overlay(NULL, 320, 200);
    check(n == 0, "NULL framebuffer -> 0 cells painted");
    /* Zero-sized framebuffer: no crash. */
    unsigned char fb[1] = { 0 };
    n = m11_v22_render_overlay(fb, 0, 0);
    check(n == 0, "zero-sized framebuffer -> 0 cells painted");
}

static void t_evidence(void) {
    const char* ev = m11_v22_render_overlay_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "V22") != NULL, "ev V22");
    check(strstr(ev, "DUNVIEW") != NULL, "ev DUNVIEW");
}

int main(void) {
    printf("=== M11 V22 render overlay test ===\n");
    t_v1_inactive();
    t_unpopulated();
    t_v22_paints_9_cells();
    t_v22_placeholder_index();
    t_v22_source_palette_shadow();
    t_v22_material_categories();
    t_cell_rect_api();
    t_null_safe();
    t_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
