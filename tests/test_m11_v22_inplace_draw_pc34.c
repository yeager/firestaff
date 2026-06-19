/*
 * test_m11_v22_inplace_draw_pc34.c — DM1 V2.2 in-place bitmap cache
 *
 * Focused tests for the V22 in-place foundation:
 *   - Init/shutdown lifecycle
 *   - Active flag reflects cache load state
 *   - Cache file presence/format validation (skip if missing)
 *   - get_cell_bitmap returns NULL when no V22 cache populated
 *   - get_cell_asset_id returns NULL when V22 not active
 *   - Source evidence citation
 *
 * Test does NOT modify any V1/V2 state — read-only verification.
 * Skips cache-dependent assertions when ~/.firestaff/assets/dm1/modern/
 * v22_inplace_cache.bin is missing or invalid.
 */

#include "m11_v22_inplace_draw_pc34.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;
static int checks = 0;

#define CHECK(expr, msg) \
    do { checks++; if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s — %s\n", __FILE__, __LINE__, #expr, (msg)); \
        failures++; } } while (0)

static void test_init_shutdown(void) {
    int active_before = m11_v22_inplace_draw_active();
    /* Idempotent init */
    int r1 = m11_v22_inplace_draw_init();
    int r2 = m11_v22_inplace_draw_init();
    CHECK(r1 == r2, "init is idempotent (same return on repeat call)");
    int active_after = m11_v22_inplace_draw_active();
    CHECK((active_after == 0) || (active_after == 1), "active is 0 or 1");
    m11_v22_inplace_draw_shutdown();
    CHECK(m11_v22_inplace_draw_active() == 0, "active==0 after shutdown");
    /* Re-init after shutdown */
    int r3 = m11_v22_inplace_draw_init();
    CHECK((r3 == 0) || (r3 == 1), "re-init returns 0 or 1");
    m11_v22_inplace_draw_shutdown();
}

static void test_get_cell_bitmap_no_cache(void) {
    m11_v22_inplace_draw_shutdown();
    int w = -1, h = -1;
    const uint32_t* p = m11_v22_inplace_get_cell_bitmap(1, 0, &w, &h);
    CHECK(p == NULL, "no cache -> bitmap NULL");
    CHECK(w == 0 && h == 0, "no cache -> dims 0");
}

static void test_get_cell_asset_id_no_cache(void) {
    m11_v22_inplace_draw_shutdown();
    const char* aid = m11_v22_inplace_get_cell_asset_id(1, 0);
    CHECK(aid == NULL, "no cache -> asset_id NULL");
}

static void test_source_evidence(void) {
    const char* ev = m11_v22_inplace_draw_source_evidence();
    CHECK(ev != NULL, "evidence non-null");
    CHECK(strlen(ev) > 0, "evidence non-empty");
    CHECK(strstr(ev, "ReDMCSB") != NULL || strstr(ev, "m11_v22") != NULL,
          "evidence cites source");
}

static void test_cache_load_path(void) {
    /* If cache exists, init should succeed */
    int r = m11_v22_inplace_draw_init();
    int active = m11_v22_inplace_draw_active();
    if (r == 1) {
        CHECK(active == 1, "active==1 after successful init");
    } else {
        /* Cache missing or invalid — that's OK for environments without it */
        CHECK(active == 0, "active==0 when cache missing/invalid");
    }
    m11_v22_inplace_draw_shutdown();
}

static void test_double_shutdown_safe(void) {
    m11_v22_inplace_draw_init();
    m11_v22_inplace_draw_shutdown();
    m11_v22_inplace_draw_shutdown();  /* idempotent */
    CHECK(m11_v22_inplace_draw_active() == 0, "double shutdown safe");
}

int main(void) {
    test_init_shutdown();
    test_get_cell_bitmap_no_cache();
    test_get_cell_asset_id_no_cache();
    test_source_evidence();
    test_cache_load_path();
    test_double_shutdown_safe();

    printf("m11_v22_inplace_draw_pc34: checks=%d failures=%d\n", checks, failures);
    if (failures > 0) {
        printf("m11_v22_inplace_draw_pc34: FAIL\n");
        return 1;
    }
    printf("m11_v22_inplace_draw_pc34: PASS\n");
    return 0;
}
