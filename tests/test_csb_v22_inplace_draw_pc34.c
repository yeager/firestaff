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

static void test_f0128_door_command_consumes_admitted_source_material(void) {
    const char* root = "/tmp/scratch/csb-v22-command-root";
    const char* data_dir = "/tmp/scratch/csb-v22-command-root/data/csb";
    const char* modern_dir = "/tmp/scratch/csb-v22-command-root/assets/csb/modern";
    const char manifest[] =
        "{\n"
        "  \"routeProvenance\": [\n"
        "    {\n"
        "      \"id\": \"door_d0_01\",\n"
        "      \"category\": \"door_shapes\",\n"
        "      \"sourceGraphicIndex\": 248,\n"
        "      \"sourceRecordSha256\": \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\",\n"
        "      \"sourceDimensions\": [96, 88],\n"
        "      \"outputDimensions\": [96, 88]\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"door_d1_01\",\n"
        "      \"category\": \"door_shapes\",\n"
        "      \"sourceGraphicIndex\": 247,\n"
        "      \"sourceRecordSha256\": \"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\",\n"
        "      \"sourceDimensions\": [64, 61],\n"
        "      \"outputDimensions\": [64, 96]\n"
        "    }\n"
        "  ]\n"
        "}\n";
    char cache_path[512];
    char manifest_path[512];
    unsigned char cache[112];
    unsigned char framebuffer[320 * 200];
    uint8_t palette[256][3];
    CSB_V1_ViewportRuntimeDrawCommandPc34 command;

    CHECK(mkdir_p(data_dir), "create command-test data directory");
    CHECK(mkdir_p(modern_dir), "create command-test modern directory");
    snprintf(cache_path, sizeof(cache_path), "%s/v22_inplace_cache.bin", modern_dir);
    snprintf(manifest_path, sizeof(manifest_path), "%s/modern_asset_manifest.json", modern_dir);
    CHECK(write_file(manifest_path, manifest, strlen(manifest)),
          "write command-test route provenance manifest");

    memset(cache, 0, sizeof(cache));
    memcpy(cache, "FSV22C\0\0", 8);
    put_u32(cache + 8, 1);  /* format version */
    put_u32(cache + 12, 2); /* entry count */
    put_u32(cache + 32, fnv1a32("door_shapes"));
    put_u32(cache + 36, fnv1a32("door_d0_01"));
    put_u32(cache + 40, 2);
    put_u32(cache + 44, 1);
    put_u32(cache + 48, 8);
    put_u32(cache + 52, 96);
    put_u32(cache + 64, fnv1a32("door_shapes"));
    put_u32(cache + 68, fnv1a32("door_d1_01"));
    put_u32(cache + 72, 2);
    put_u32(cache + 76, 1);
    put_u32(cache + 80, 8);
    put_u32(cache + 84, 104);
    /* First source pixel is opaque red; the second is C10-style transparent.
     * Scaling the 2x1 source across the D1 clip makes alpha preservation easy
     * to observe without relying on any generated art. */
    cache[96] = 0x00; cache[97] = 0x00; cache[98] = 0xff; cache[99] = 0xff;
    cache[104] = 0xff; cache[105] = 0x00; cache[106] = 0x00; cache[107] = 0xff;
    CHECK(write_file(cache_path, cache, sizeof(cache)), "write command-test cache");

    memset(&command, 0, sizeof(command));
    command.route = CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D1_F0111_DOOR_PC34;
    command.source_graphics_item_index = 248;
    memset(command.source_record_sha256, 'a', 64);
    command.source_record_sha256[64] = '\0';
    command.transparent_color = 10;
    command.clip_x = 48;
    command.clip_y = 33;
    command.clip_w = 128;
    command.clip_h = 102;
    command.draw_order = 0x0111;
    memset(framebuffer, 0x5a, sizeof(framebuffer));

    csb_v22_inplace_draw_shutdown();
    csb_v22_set_manifest_path(data_dir);
    CHECK(csb_v22_inplace_draw_init() == 1, "command-test cache initializes");
    CHECK(csb_v22_inplace_render_f0128_command(&command, framebuffer, 320, 200) == 0,
          "unbound palette leaves the source F0128 command untouched");
    memset(palette, 0, sizeof(palette));
    palette[0x30][0] = 63; /* original indexed red used by the fixture */
    palette[0x03][2] = 63; /* original indexed blue used by the D2 fixture */
    CHECK(csb_v22_inplace_draw_set_indexed_palette_rgb6(palette) == 1,
          "command-test binds an original indexed palette");
    CHECK(csb_v22_inplace_render_f0128_command(&command, framebuffer, 320, 200) == 1,
          "admitted D1 door command consumes exact source route");
    CHECK(framebuffer[33 * 320 + 48] == 0x30,
          "opaque modern pixel replaces the left half of the F0128 clip");
    CHECK(framebuffer[33 * 320 + 112] == 0x5a,
          "transparent modern pixel preserves the source framebuffer");
    CHECK(framebuffer[32 * 320 + 48] == 0x5a &&
          framebuffer[33 * 320 + 47] == 0x5a &&
          framebuffer[33 * 320 + 176] == 0x5a,
          "door replacement never escapes the original F0128 clip");

    memset(framebuffer, 0x5a, sizeof(framebuffer));
    command.route = CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D2_F0111_DOOR_PC34;
    command.source_graphics_item_index = 247;
    memset(command.source_record_sha256, 'b', 64);
    command.source_record_sha256[64] = '\0';
    command.clip_x = 76;
    command.clip_y = 47;
    command.clip_w = 72;
    command.clip_h = 74;
    command.draw_order = 0x0111;
    CHECK(csb_v22_inplace_render_f0128_command(&command, framebuffer, 320, 200) == 1,
          "admitted D2 door command consumes its distinct source route");
    CHECK(framebuffer[47 * 320 + 76] == 0x03,
          "D2 opaque pixel uses its own cache entry within the F0128 clip");
    CHECK(framebuffer[47 * 320 + 112] == 0x5a,
          "D2 transparent modern pixel preserves the source framebuffer");
    CHECK(framebuffer[46 * 320 + 76] == 0x5a &&
          framebuffer[47 * 320 + 75] == 0x5a &&
          framebuffer[47 * 320 + 148] == 0x5a,
          "D2 replacement never escapes its original F0128 clip");
    command.transparent_color = 0;
    CHECK(csb_v22_inplace_render_f0128_command(&command, framebuffer, 320, 200) == 0,
          "unproven transparency contract remains source-owned");
    csb_v22_inplace_draw_shutdown();
    (void)root;
}

static void test_f0128_door_uses_bound_source_palette(void) {
    const char* root = "/tmp/scratch/csb-v22-palette-root";
    const char* data_dir = "/tmp/scratch/csb-v22-palette-root/data/csb";
    const char* modern_dir = "/tmp/scratch/csb-v22-palette-root/assets/csb/modern";
    const char manifest[] =
        "{\n"
        "  \"routeProvenance\": [\n"
        "  {\n"
        "    \"id\": \"door_d0_01\",\n"
        "    \"category\": \"door_shapes\",\n"
        "    \"sourceGraphicIndex\": 248,\n"
        "    \"sourceRecordSha256\": \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\",\n"
        "    \"sourceDimensions\": [96, 88],\n"
        "    \"outputDimensions\": [96, 88]\n"
        "  }]\n"
        "}\n";
    char cache_path[512];
    char manifest_path[512];
    unsigned char cache[68];
    unsigned char framebuffer[320 * 200];
    uint8_t palette[256][3];
    CSB_V1_ViewportRuntimeDrawCommandPc34 command;
    CSB_V22_RouteProvenancePc34 provenance;

    CHECK(mkdir_p(data_dir), "create palette-test data directory");
    CHECK(mkdir_p(modern_dir), "create palette-test modern directory");
    snprintf(cache_path, sizeof(cache_path), "%s/v22_inplace_cache.bin", modern_dir);
    snprintf(manifest_path, sizeof(manifest_path), "%s/modern_asset_manifest.json", modern_dir);
    CHECK(write_file(manifest_path, manifest, strlen(manifest)),
          "write palette-test route provenance manifest");
    memset(cache, 0, sizeof(cache));
    memcpy(cache, "FSV22C\0\0", 8);
    put_u32(cache + 8, 1);
    put_u32(cache + 12, 1);
    put_u32(cache + 32, fnv1a32("door_shapes"));
    put_u32(cache + 36, fnv1a32("door_d0_01"));
    put_u32(cache + 40, 1);
    put_u32(cache + 44, 1);
    put_u32(cache + 48, 4);
    put_u32(cache + 52, 64);
    /* AARRGGBB: this deliberately does not have an EGA-cube exact match. */
    cache[64] = 0x11; cache[65] = 0x22; cache[66] = 0x33; cache[67] = 0xff;
    CHECK(write_file(cache_path, cache, sizeof(cache)), "write palette-test cache");

    memset(palette, 0, sizeof(palette));
    /* 8-bit RGB (16,32,49) after six-bit expansion: exact source match. */
    palette[7][0] = 4; palette[7][1] = 8; palette[7][2] = 12;
    memset(&command, 0, sizeof(command));
    command.route = CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D1_F0111_DOOR_PC34;
    command.source_graphics_item_index = 248;
    memset(command.source_record_sha256, 'a', 64);
    command.source_record_sha256[64] = '\0';
    command.transparent_color = 10;
    command.clip_x = 48; command.clip_y = 33;
    command.clip_w = 96; command.clip_h = 88;
    command.draw_order = 0x0111;
    memset(framebuffer, 0, sizeof(framebuffer));

    csb_v22_inplace_draw_shutdown();
    csb_v22_set_manifest_path(data_dir);
    CHECK(csb_v22_inplace_draw_init() == 1, "palette-test cache initializes");
    memset(&provenance, 0, sizeof(provenance));
    CHECK(csb_v22_get_route_provenance("door_shapes", "door_d0_01", &provenance) == 1,
          "palette-test provenance is parsed");
    CHECK(strcmp(provenance.source_record_sha256, command.source_record_sha256) == 0,
          "palette-test command has exact source record identity");
    CHECK(csb_v22_inplace_draw_set_indexed_palette_rgb6(palette) == 1,
          "bind original indexed palette");
    CHECK(csb_v22_inplace_render_f0128_command(&command, framebuffer, 320, 200) == 1,
          "palette-bound source command paints");
    CHECK(framebuffer[33 * 320 + 48] == 7,
          "RGBA pixel maps to exact bound source palette index");
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
    test_f0128_door_command_consumes_admitted_source_material();
    test_f0128_door_uses_bound_source_palette();
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
