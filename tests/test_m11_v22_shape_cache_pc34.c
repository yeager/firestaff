/* test_m11_v22_shape_cache_pc34.c
 *
 * DM1 V2.2 per-frame V22 shape cache test. The cache is the M11
 * GPU render path's data-flow seam: it is populated once per
 * m11_draw_viewport frame from the sampled cells, and consulted by
 * the per-cell draw passes via m11_v22_shape_cache_get(depth, lateral).
 *
 * Source: src/engine/m11_game_view.c (m11_v22_shape_cache_update +
 * m11_v22_shape_cache_get + m11_v22_shape_cache_active).
 */
#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v22_shapes.h"
#include "dm1_v2_shape_runtime_pc34.h"
#include "m11_v22_shape_cache_pc34.h"

#include <stdio.h>
#include <string.h>

static int g_failed = 0;
static int g_total = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void t_v1_default(void) {
    unsigned char squares[3][3] = {
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 }
    };
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_reset();
    m11_v22_shape_cache_update(0, squares);
    check(m11_v22_shape_cache_populated() == 1, "V1: cache populated after update");
    /* V1 path: all 9 cells active=0. */
    check(m11_v22_shape_cache_active(1, 0) == 0, "V1: D1L active=0");
    check(m11_v22_shape_cache_active(2, 1) == 0, "V1: D2C active=0");
    check(m11_v22_shape_cache_active(3, -1) == 0, "V1: D3L active=0");
    check(m11_v22_shape_cache_active(3, 0) == 0, "V1: D3C active=0");
    check(m11_v22_shape_cache_active(3, 1) == 0, "V1: D3R active=0");
}

static void t_v22_active(void) {
    unsigned char squares[3][3] = {
        { 0x04, 0x00, 0x00 },  /* D1L = door, others = wall */
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 }
    };
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, squares);
    /* V22 path: cells with valid raw squares resolve. */
    check(m11_v22_shape_cache_active(1, 0) == 1, "V22: D1L active=1");
    check(m11_v22_shape_cache_active(2, 1) == 1, "V22: D2C active=1");
    check(m11_v22_shape_cache_active(3, 0) == 1, "V22: D3C active=1");
    /* The cache's params are the V22 shape book entry. */
    const DM1_V2_ShapeRuntimeResult* r = m11_v22_shape_cache_get(1, 0);
    check(r != NULL, "V22: D1L cache_get non-null");
    check(r->material != NULL, "V22: D1L material non-null");
}

static void t_v1_to_v22_to_v1_transition(void) {
    unsigned char squares[3][3] = {
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 }
    };
    /* V1: cells inactive. */
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_reset();
    m11_v22_shape_cache_update(0, squares);
    check(m11_v22_shape_cache_active(2, 0) == 0, "V1->V1: D2C inactive");
    /* Switch to V22: cells active. */
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, squares);
    check(m11_v22_shape_cache_active(2, 0) == 1, "V1->V22: D2C active");
    /* Back to V1: cells inactive again. */
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_update(0, squares);
    check(m11_v22_shape_cache_active(2, 0) == 0, "V22->V1: D2C inactive");
}

static void t_oob(void) {
    /* Out-of-range depth/lateral returns NULL. */
    check(m11_v22_shape_cache_get(0, 0) == NULL, "D0 (out of range) -> NULL");
    check(m11_v22_shape_cache_get(4, 0) == NULL, "D4 (out of range) -> NULL");
    check(m11_v22_shape_cache_get(1, -2) == NULL, "side=-2 (out of range) -> NULL");
    check(m11_v22_shape_cache_get(2, 2) == NULL, "side=+2 (out of range) -> NULL");
    /* active() also returns 0 for out-of-range. */
    check(m11_v22_shape_cache_active(0, 0) == 0, "D0 active=0 (OOB)");
    check(m11_v22_shape_cache_active(4, 0) == 0, "D4 active=0 (OOB)");
}

static void t_reset_unpopulated(void) {
    unsigned char squares[3][3] = { { 0 } };
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, squares);
    check(m11_v22_shape_cache_populated() == 1,
          "reset: cache starts populated after update");
    m11_v22_shape_cache_reset();
    check(m11_v22_shape_cache_populated() == 0,
          "reset: cache returns to unpopulated state");
    check(m11_v22_shape_cache_get(1, 0) == NULL,
          "reset: cache_get returns NULL before next update");
    check(m11_v22_shape_cache_active(1, 0) == 0,
          "reset: active returns 0 before next update");
}

static void t_evidence(void) {
    const char* ev = m11_v22_shape_cache_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "V22") != NULL, "ev V22");
    check(strstr(ev, "DUNVIEW") != NULL, "ev DUNVIEW");
}

static void t_null_safe(void) {
    /* NULL raw_squares is a no-op (not a crash). */
    m11_v22_shape_cache_update(0, NULL);
    check(1, "NULL raw_squares tolerated");
}

int main(void) {
    printf("=== M11 V22 shape cache test ===\n");
    t_v1_default();
    t_v22_active();
    t_v1_to_v22_to_v1_transition();
    t_oob();
    t_reset_unpopulated();
    t_evidence();
    t_null_safe();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
