/*
 * test_csb_v22_inplace_draw_pc34.c — CSB V2.2 in-place bitmap cache + render pass
 *
 * Tests the V22 in-place foundation:
 *   - Init/shutdown lifecycle
 *   - Active flag reflects cache load state
 *   - Cache file presence/format validation (skip if missing)
 *   - get_cell_bitmap returns NULL when no V22 cache populated
 *   - get_cell_asset_id returns NULL when V22 not active
 *   - csb_v22_inplace_render_pass() draws bitmaps into a framebuffer
 *   - Source evidence citation
 *
 * Test does NOT modify any V1/V2 state — read-only verification.
 * Skips cache-dependent assertions when ~/.firestaff/assets/csb/modern/
 * v22_inplace_cache.bin is missing or invalid.
 */

#include "csb_v22_inplace_draw_pc34.h"
#include "csb_v22_modern_assets_pc34.h"
#include "csb_v22_shape_cache_pc34.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;
static int checks = 0;

static unsigned fnv1a32(const char* text) {
    unsigned h = 2166136261u;
    while (*text) h = (h ^ (unsigned char)*text++) * 16777619u;
    return h;
}

static void put_u32(unsigned char* out, unsigned value) {
    out[0] = (unsigned char)(value & 0xffu);
    out[1] = (unsigned char)((value >> 8) & 0xffu);
    out[2] = (unsigned char)((value >> 16) & 0xffu);
    out[3] = (unsigned char)((value >> 24) & 0xffu);
}

static int mkdir_p(const char* path) {
    char command[512];
    int n = snprintf(command, sizeof(command), "mkdir -p '%s'", path);
    return n > 0 && (size_t)n < sizeof(command) && system(command) == 0;
}

static int write_file(const char* path, const void* bytes, size_t size) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    int ok = fwrite(bytes, 1, size, fp) == size;
    fclose(fp);
    return ok;
}

#define CHECK(expr, msg) \
    do { checks++; if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s — %s\n", __FILE__, __LINE__, #expr, (msg)); \
        failures++; } } while (0)

static void test_init_shutdown(void) {
    int active_before = csb_v22_inplace_draw_active();
    (void)active_before;
    int r1 = csb_v22_inplace_draw_init();
    int r2 = csb_v22_inplace_draw_init();
    CHECK(r1 == r2, "init is idempotent (same return on repeat call)");
    int active_after = csb_v22_inplace_draw_active();
    CHECK((active_after == 0) || (active_after == 1), "active is 0 or 1");
    csb_v22_inplace_draw_shutdown();
    CHECK(csb_v22_inplace_draw_active() == 0, "active==0 after shutdown");
    int r3 = csb_v22_inplace_draw_init();
    CHECK((r3 == 0) || (r3 == 1), "re-init returns 0 or 1");
    csb_v22_inplace_draw_shutdown();
}

static void test_get_cell_bitmap_no_cache(void) {
    csb_v22_inplace_draw_shutdown();
    int w = -1, h = -1;
    const uint32_t* p = csb_v22_inplace_get_cell_bitmap(1, 0, &w, &h);
    CHECK(p == NULL, "no cache -> bitmap NULL");
    CHECK(w == 0 && h == 0, "no cache -> dims 0");
}

static void test_get_cell_asset_id_no_cache(void) {
    csb_v22_inplace_draw_shutdown();
    const char* aid = csb_v22_inplace_get_cell_asset_id(1, 0);
    CHECK(aid == NULL, "no cache -> asset_id NULL");
}

static void test_source_evidence(void) {
    const char* ev = csb_v22_inplace_draw_source_evidence();
    CHECK(ev != NULL, "evidence non-null");
    CHECK(strlen(ev) > 0, "evidence non-empty");
    CHECK(strstr(ev, "ReDMCSB") != NULL || strstr(ev, "csb_v22") != NULL,
          "evidence cites source");
}

static void test_cache_load_path(void) {
    int r = csb_v22_inplace_draw_init();
    int active = csb_v22_inplace_draw_active();
    if (r == 1) {
        CHECK(active == 1, "active==1 after successful init");
    } else {
        CHECK(active == 0, "active==0 when cache missing/invalid");
    }
    csb_v22_inplace_draw_shutdown();
}

static void test_cache_uses_configured_manifest_root(void) {
    const char* root = "/tmp/scratch/csb-v22-configured-root";
    const char* data_dir = "/tmp/scratch/csb-v22-configured-root/data/csb";
    const char* modern_dir = "/tmp/scratch/csb-v22-configured-root/assets/csb/modern";
    char cache_path[512];
    unsigned char cache[68];
    const uint32_t* pixels;
    int w = 0, h = 0;

    CHECK(mkdir_p(data_dir), "create configured data directory");
    CHECK(mkdir_p(modern_dir), "create configured modern directory");
    snprintf(cache_path, sizeof(cache_path), "%s/v22_inplace_cache.bin", modern_dir);
    memset(cache, 0, sizeof(cache));
    memcpy(cache, "FSV22C\0\0", 8);
    put_u32(cache + 8, 1);  /* format version */
    put_u32(cache + 12, 1); /* entry count */
    put_u32(cache + 32, fnv1a32("wall_shapes"));
    put_u32(cache + 36, fnv1a32("wall_dungeon_d0_01"));
    put_u32(cache + 40, 1);
    put_u32(cache + 44, 1);
    put_u32(cache + 48, 4);
    put_u32(cache + 52, 64);
    cache[64] = 0x33; /* B */
    cache[65] = 0x22; /* G */
    cache[66] = 0x11; /* R */
    cache[67] = 0xff; /* A */
    CHECK(write_file(cache_path, cache, sizeof(cache)), "write configured cache");

    csb_v22_inplace_draw_shutdown();
    csb_v22_set_manifest_path(data_dir);
    CHECK(csb_v22_inplace_draw_init() == 1, "configured cache initializes");
    pixels = csb_v22_inplace_get_bitmap_by_id("wall_shapes", "wall_dungeon_d0_01", &w, &h);
    CHECK(pixels != NULL, "configured cache bitmap is addressable");
    CHECK(w == 1 && h == 1, "configured cache bitmap dimensions");
    CHECK(pixels && pixels[0] == 0xff112233u, "configured cache preserves AARRGGBB pixel order");
    csb_v22_inplace_draw_shutdown();
    (void)root;
}

static void test_double_shutdown_safe(void) {
    csb_v22_inplace_draw_init();
    csb_v22_inplace_draw_shutdown();
    csb_v22_inplace_draw_shutdown();
    CHECK(csb_v22_inplace_draw_active() == 0, "double shutdown safe");
}

static void test_render_pass_safe_when_no_cache(void) {
    /* Render pass must return 0 and not crash when no cache loaded */
    csb_v22_inplace_draw_shutdown();
    unsigned char fb[320 * 200];
    memset(fb, 0xAA, sizeof(fb));  /* sentinel value */
    int painted = csb_v22_inplace_render_pass(fb, 320, 200);
    CHECK(painted == 0, "no cache -> render paints 0 cells");
    /* Framebuffer should be unchanged */
    int all_sentinel = 1;
    for (int i = 0; i < 320 * 200; ++i) if (fb[i] != 0xAA) { all_sentinel = 0; break; }
    CHECK(all_sentinel, "no cache -> framebuffer unchanged");
}

static void test_render_pass_safe_with_null_args(void) {
    csb_v22_inplace_draw_init();
    int painted = csb_v22_inplace_render_pass(NULL, 320, 200);
    CHECK(painted == 0, "NULL fb -> 0 cells painted");
    unsigned char fb[10];
    memset(fb, 0, sizeof(fb));
    painted = csb_v22_inplace_render_pass(fb, 0, 200);
    CHECK(painted == 0, "zero width -> 0 cells painted");
    painted = csb_v22_inplace_render_pass(fb, 320, 0);
    CHECK(painted == 0, "zero height -> 0 cells painted");
    csb_v22_inplace_draw_shutdown();
}

static void test_render_pass_safe_when_no_shape_cache(void) {
    /* Even if bitmap cache loaded, without shape cache populated,
     * the render pass must return 0 (no V22-active cells to paint). */
    csb_v22_inplace_draw_init();
    if (!csb_v22_inplace_draw_active()) {
        /* Cache not present on this machine — skip */
        csb_v22_inplace_draw_shutdown();
        return;
    }
    unsigned char fb[320 * 200];
    memset(fb, 0xAA, sizeof(fb));
    int painted = csb_v22_inplace_render_pass(fb, 320, 200);
    CHECK(painted == 0, "no shape cache populated -> 0 cells painted");
    csb_v22_inplace_draw_shutdown();
}

int main(void) {
    test_init_shutdown();
    test_get_cell_bitmap_no_cache();
    test_get_cell_asset_id_no_cache();
    test_source_evidence();
    test_cache_load_path();
    test_cache_uses_configured_manifest_root();
    test_double_shutdown_safe();
    test_render_pass_safe_when_no_cache();
    test_render_pass_safe_with_null_args();
    test_render_pass_safe_when_no_shape_cache();

    printf("csb_v22_inplace_draw_pc34: checks=%d failures=%d\n", checks, failures);
    if (failures > 0) {
        printf("csb_v22_inplace_draw_pc34: FAIL\n");
        return 1;
    }
    printf("csb_v22_inplace_draw_pc34: PASS\n");
    return 0;
}
