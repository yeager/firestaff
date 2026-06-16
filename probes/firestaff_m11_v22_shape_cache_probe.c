/**
 * firestaff_m11_v22_shape_cache_probe.c
 *
 * DM1 V2.2 per-frame V22 shape cache headless probe. Verifies that
 * the cache is populated correctly for the V1 default, V22 active,
 * V1->V22->V1 transition, and that the accessors handle out-of-
 * range indices.
 *
 * Source: src/engine/m11_game_view.c (m11_v22_shape_cache_*).
 */
#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v2_shape_runtime_pc34.h"
#include <stdio.h>
#include <string.h>

extern void m11_v22_shape_cache_update(int direction,
                                       const unsigned char raw_squares[3][3]);
extern const DM1_V2_ShapeRuntimeResult* m11_v22_shape_cache_get(int depth,
                                                                int lateral);
extern int m11_v22_shape_cache_active(int depth, int lateral);
extern const char* m11_v22_shape_cache_source_evidence(void);

static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_v1_default(void) {
    unsigned char squares[3][3] = { 0 };
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_update(0, squares);
    check(m11_v22_shape_cache_active(1, 0) == 0, "V1: D1L inactive");
    check(m11_v22_shape_cache_active(2, 0) == 0, "V1: D2C inactive");
    check(m11_v22_shape_cache_active(3, 0) == 0, "V1: D3C inactive");
}

static void p_v22_all_cells(void) {
    unsigned char squares[3][3] = { 0 };
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, squares);
    /* All 9 cells in the V22 3x3 cache should be active. */
    int active_count = 0;
    for (int d = 1; d <= 3; ++d) {
        for (int s = -1; s <= 1; ++s) {
            if (m11_v22_shape_cache_active(d, s)) active_count++;
        }
    }
    check(active_count == 9, "V22: 9/9 cells active");
}

static void p_transition(void) {
    unsigned char squares[3][3] = { 0 };
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_update(0, squares);
    int v1 = m11_v22_shape_cache_active(2, 0);
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, squares);
    int v22 = m11_v22_shape_cache_active(2, 0);
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_update(0, squares);
    int back_to_v1 = m11_v22_shape_cache_active(2, 0);
    check(v1 == 0, "V1 -> D2C inactive");
    check(v22 == 1, "V22 -> D2C active");
    check(back_to_v1 == 0, "V1 (back) -> D2C inactive");
}

static void p_oob_indices(void) {
    unsigned char squares[3][3] = { 0 };
    m11_v22_shape_cache_update(0, squares);
    check(m11_v22_shape_cache_get(0, 0) == NULL, "D0 OOB -> NULL");
    check(m11_v22_shape_cache_get(4, 0) == NULL, "D4 OOB -> NULL");
    check(m11_v22_shape_cache_get(1, -2) == NULL, "side=-2 OOB -> NULL");
    check(m11_v22_shape_cache_get(2, 2) == NULL, "side=+2 OOB -> NULL");
    check(m11_v22_shape_cache_active(0, 0) == 0, "D0 active=0 OOB");
    check(m11_v22_shape_cache_active(4, 0) == 0, "D4 active=0 OOB");
}

static void p_null_safe(void) {
    m11_v22_shape_cache_update(0, NULL);
    check(1, "NULL raw_squares tolerated (no crash)");
}

static void p_evidence(void) {
    const char* ev = m11_v22_shape_cache_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "V22") != NULL, "ev V22");
    check(strstr(ev, "DUNVIEW") != NULL, "ev DUNVIEW");
}

int main(void) {
    printf("=== M11 V22 shape cache probe ===\n");
    p_v1_default();
    p_v22_all_cells();
    p_transition();
    p_oob_indices();
    p_null_safe();
    p_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
