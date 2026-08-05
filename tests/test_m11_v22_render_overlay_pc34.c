/* test_m11_v22_render_overlay_pc34.c
 *
 * DM1 V2.2 GPU render path: V22 modern-art overlay unit test.
 * Verifies that the legacy overlay is always a no-op. Authenticated V2.2
 * pixels are owned by the in-place source-art renderer; this compatibility
 * API must never paint synthetic rectangles or palette entries.
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

static void t_v22_no_draw(void) {
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
    check(n == 0, "V22 active -> no synthetic cells painted");
    int all_zero = 1;
    for (int i = 0; i < 320 * 200; ++i) {
        if (fb[i] != 0x00) { all_zero = 0; break; }
    }
    check(all_zero, "V22 active -> framebuffer remains unchanged");
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
    t_v22_no_draw();
    t_cell_rect_api();
    t_null_safe();
    t_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
