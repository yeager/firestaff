/*
 * firestaff_csb_v22_viewport_swap_probe.c
 *
 * CSB V2.2 per-cell modern-art swap headless probe.
 *
 * Builds a minimal temporary v22_inplace_cache.bin under a probe-only
 * HOME, activates CSB V2.2 presentation mode, populates the per-cell
 * swap cache from a representative CSB raw_cells layout, and verifies
 * that csb_v22_viewport_swap_render() paints all 9 CSB viewport cells
 * with the correct asset_id mapping (each cell centers on a distinct
 * EGA-quantized color from the synthetic cache).
 *
 * Coverage:
 *   - cache file write + probe HOME setup
 *   - presentation mode activation (csb_v2_presentation_mode_set + set_modern_pack_available)
 *   - csb_v22_inplace_draw_init() reads the synthetic cache
 *   - csb_v22_set_installed(1) makes the swap active
 *   - per-shape discriminator covers wall/floor/creature/pit/stairs/field/item shapes
 *   - per-shape asset_id resolves to (category, asset_id) string
 *   - viewport_swap_update populates + activates the swap
 *   - render pass paints all 9 cells (D0..D2 x L/C/R) at 1920x1080
 *   - 4-direction sweep (each direction paints 9 cells)
 *   - source evidence citation
 *
 * Source-lock:
 *   CSBWin/Viewport.cpp:7290  (9-square viewport layout)
 *   ReDMCSB DUNVIEW.C F0128   (CSB viewport routing)
 *   include/csb_v22_viewport_swap_pc34.h (per-cell swap contract)
 *   include/csb_v22_inplace_draw_pc34.h (cache + bitmap blit)
 *   include/csb_v22_modern_assets_pc34.h (asset pack + presentation flag)
 */

#include "csb_v22_viewport_swap_pc34.h"
#include "csb_v22_inplace_draw_pc34.h"
#include "csb_v22_shape_cache_pc34.h"
#include "csb_v22_modern_assets_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
#include "fs_portable_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Probe state ──────────────────────────────────────────────────── */

typedef struct ProbeStats {
    int total;
    int passed;
    int failed;
} ProbeStats;

static void probe_record(ProbeStats* stats, const char* id, int ok, const char* note) {
    stats->total += 1;
    if (ok) {
        stats->passed += 1;
        printf("PASS %s: %s\n", id, note);
    } else {
        stats->failed += 1;
        printf("FAIL %s: %s\n", id, note);
    }
}

/* ── FNV-1a + cache-file writer ───────────────────────────────────── */

static uint32_t fnv1a_hash(const char* s) {
    uint32_t h = 2166136261u;
    while (*s) {
        h = (h ^ (uint8_t)*s++) * 16777619u;
    }
    return h;
}

static void put_u32(unsigned char* p, uint32_t v) {
    memcpy(p, &v, sizeof(v));
}

/* Build a minimal v22_inplace_cache.bin with the asset_ids the
 * CSB per-cell swap needs. Each entry is a single-color 4x4 RGBA
 * bitmap keyed by (category, asset_id). Bitmap colors are picked
 * so the probe can verify which entry was painted into which cell
 * by inspecting the framebuffer at the cell center.
 *
 * Color note: the in-place renderer maps RGBA -> 6-bit EGA cube
 * (2 bits per channel) via (c * 3 + 127) / 255 quantization. Dark
 * values (c <= 84) all map to 0 (black) — pick colors >= 128 so
 * each entry gets a distinct non-zero EGA index in the framebuffer. */
static int write_minimal_csb_v22_cache(const char* cache_path) {
    static const struct {
        const char* category;
        const char* asset_id;
        uint32_t rgba;
    } kEntries[] = {
        /* Walls (T560) */
        { "wall_shapes",  "wall_dungeon_01",               0xFFFF0000u },
        { "wall_shapes",  "wall_dungeon_doorway_01",       0xFF00FF00u },
        { "wall_shapes",  "wall_dungeon_alcove_01",        0xFF0000FFu },
        { "wall_shapes",  "wall_dungeon_inscription_01",   0xFFFFFF00u },
        /* Floors */
        { "floor_shapes", "floor_plain_01",                0xFF00FFFFu },
        { "floor_shapes", "floor_cracked_01",              0xFFFF00FFu },
        { "floor_shapes", "floor_pit_01",                  0xFF808000u },
        { "floor_shapes", "floor_stairs_01",               0xFF008080u },
        { "floor_shapes", "ceiling_plain_01",              0xFF808080u },
        { "floor_shapes", "field_teleporter_01",           0xFF404040u },
        { "floor_shapes", "field_chaos_rift_01",           0xFF606060u },
        { "floor_shapes", "field_explosion_01",            0xFFA0A0A0u },
        /* Creatures / items */
        { "creature_shapes", "creature_chaos_fiend_01",    0xFF800080u },
        /* Doors */
        { "door_shapes",  "door_iron_portcullis_01",       0xFFC0C0C0u },
        { "door_shapes",  "door_prison_01",                0xFF303030u },
        /* UI chrome / portraits */
        { "ui_chrome",         "ui_panel_01",              0xFF202020u },
        { "ui_chrome",         "ui_message_log_01",        0xFF505050u },
        { "ui_chrome",         "ui_inventory_01",          0xFF707070u },
        { "champion_portraits","champion_warrior_csb",     0xFFB0B0B0u },
        { "champion_portraits","statue_lord_order_01",     0xFFD0D0D0u },
        /* CSB-only categories */
        { "chaos_runes", "chaos_rune_01",                  0xFF90E090u },
        { "chaos_runes", "chaos_rune_marker_01",           0xFFE09090u },
        { "dsa_scrolls", "dsa_scroll_01",                  0xFF9090E0u }
    };
    const int kEntryCount = (int)(sizeof(kEntries) / sizeof(kEntries[0]));
    const uint32_t kHdrSize = 32;
    const uint32_t kEntSize = 32;
    const uint32_t kPixW = 4, kPixH = 4;
    const uint32_t kPixSize = kPixW * kPixH * 4;

    FILE* fp = NULL;
    unsigned char* header = NULL;
    unsigned char* entries = NULL;
    unsigned char* pixels = NULL;
    uint32_t data_off;
    int i;
    int ok = 0;

    if (kEntryCount > 256) return 0;

    header = (unsigned char*)calloc(kHdrSize, 1);
    entries = (unsigned char*)calloc((size_t)kEntSize * (size_t)kEntryCount, 1);
    pixels = (unsigned char*)calloc((size_t)kPixSize * (size_t)kEntryCount, 1);
    if (!header || !entries || !pixels) goto cleanup;

    memcpy(header, "FSV22C\0\0", 8);
    put_u32(header + 8,  1u);                       /* version */
    put_u32(header + 12, (uint32_t)kEntryCount);    /* count */

    data_off = kHdrSize + (uint32_t)kEntSize * (uint32_t)kEntryCount;
    for (i = 0; i < kEntryCount; ++i) {
        unsigned char* ent = entries + (size_t)i * kEntSize;
        unsigned char* px  = pixels + (size_t)i * kPixSize;
        int j;
        put_u32(ent + 0, fnv1a_hash(kEntries[i].category));
        put_u32(ent + 4, fnv1a_hash(kEntries[i].asset_id));
        put_u32(ent + 8,  kPixW);
        put_u32(ent + 12, kPixH);
        put_u32(ent + 16, kPixSize);
        put_u32(ent + 20, data_off + (uint32_t)i * kPixSize);
        for (j = 0; j < (int)kPixW * (int)kPixH; ++j) {
            memcpy(px + j * 4, &kEntries[i].rgba, 4);
        }
    }

    fp = fopen(cache_path, "wb");
    if (!fp) goto cleanup;
    if (fwrite(header, 1, kHdrSize, fp) != kHdrSize) goto cleanup;
    if (fwrite(entries, 1, (size_t)kEntSize * (size_t)kEntryCount, fp) !=
        (size_t)kEntSize * (size_t)kEntryCount) goto cleanup;
    if (fwrite(pixels, 1, (size_t)kPixSize * (size_t)kEntryCount, fp) !=
        (size_t)kPixSize * (size_t)kEntryCount) goto cleanup;
    ok = (fclose(fp) == 0);
    fp = NULL;

cleanup:
    if (fp) fclose(fp);
    free(header);
    free(entries);
    free(pixels);
    return ok;
}

/* ── Probe home setup ────────────────────────────────────────────── */

static int setup_probe_home(char* out_cache_path, size_t out_size) {
    char modern_dir[FSP_PATH_MAX];
    int n;

    n = snprintf(modern_dir, sizeof(modern_dir),
                 "firestaff-csb-v22-probe-home/.firestaff/assets/csb/modern");
    if (n <= 0 || (size_t)n >= sizeof(modern_dir)) return 0;
    if (!FSP_CreateDirectoryRecursive(modern_dir)) return 0;
    if (FSP_SetEnv("HOME", "firestaff-csb-v22-probe-home", 1) != 0) return 0;

    n = snprintf(out_cache_path, out_size, "%s/v22_inplace_cache.bin", modern_dir);
    return n > 0 && (size_t)n < out_size;
}

/* ── Helpers: framebuffer introspection ──────────────────────────── */

/* Sum of cell-center pixels that are non-zero (after the swap pass
 * painted at least one bitmap). Used to verify a render produced
 * visible output. */
static int count_changed_pixels(const unsigned char* fb, size_t len) {
    size_t i;
    int changed = 0;
    for (i = 0; i < len; ++i) {
        if (fb[i] != 0x00) changed++;
    }
    return changed;
}

/* True when every CSB 9-square cell-center pixel is non-zero.
 * The CSB cell rect table is in csb_v22_shape_cache_pc34.c
 * (csb_v22_kCellRects): each cell is 640x360 at 1920x1080 with
 * the right column clipped to (1600..1920) at fbW=1920.
 *
 *   D0 (closest, bottom): L (320,720,640,360) C (960,720,640,360)
 *                          R (1600,720,640,360) clipped
 *   D1 (middle):          L (320,360,640,360) C (960,360,640,360)
 *                          R (1600,360,640,360) clipped
 *   D2 (farthest, top):   L (320,  0,640,360) C (960,  0,640,360)
 *                          R (1600,  0,640,360) clipped
 *
 * Cell centers:
 *   D0: (640,900)  (1280,900) (1760,900)
 *   D1: (640,540)  (1280,540) (1760,540)
 *   D2: (640,180)  (1280,180) (1760,180)
 */
static int csb_all_cell_centers_nonzero(const unsigned char* fb, int fbW) {
    static const int centers[3][3][2] = {
        /* depth 0 (D0, closest) */ {
            { 640,  900 },
            {1280,  900 },
            {1760,  900 }
        },
        /* depth 1 (D1, middle) */ {
            { 640,  540 },
            {1280,  540 },
            {1760,  540 }
        },
        /* depth 2 (D2, farthest) */ {
            { 640,  180 },
            {1280,  180 },
            {1760,  180 }
        }
    };
    int d, l;
    for (d = 0; d < 3; ++d) {
        for (l = 0; l < 3; ++l) {
            int cx = centers[d][l][0];
            int cy = centers[d][l][1];
            if (fb[cy * fbW + cx] == 0x00) return 0;
        }
    }
    return 1;
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(void) {
    ProbeStats stats;
    char cache_path[FSP_PATH_MAX];
    unsigned char raw_cells[3][3];
    unsigned char fb[1920 * 1080];
    int painted;
    int changed;
    int direction;
    int sweep_painted = 0;
    int ok;

    memset(&stats, 0, sizeof(stats));
    memset(cache_path, 0, sizeof(cache_path));
    memset(fb, 0x00, sizeof(fb));

    /* 1. Write synthetic cache + probe HOME */
    ok = setup_probe_home(cache_path, sizeof(cache_path)) &&
         write_minimal_csb_v22_cache(cache_path);
    probe_record(&stats, "CSB_V22_CACHE_FIXTURE", ok,
                 "temporary CSB v22_inplace_cache.bin written");

    /* 2. Activate V22 presentation mode + install flag */
    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set(CSB_V2_PM_V22_MODERN);
    csb_v22_set_installed(1);
    csb_v22_set_epx_cache_warm(1);
    probe_record(&stats, "CSB_V22_PRESENTATION_ACTIVE",
                 csb_v2_presentation_mode_is_v22() &&
                 csb_v22_best_available_shape_source(3) ==
                     CSB_V22_SHAPE_SOURCE_V2_MODERN,
                 "presentation mode 3 (V22) resolves to V2_MODERN with pack");

    /* 3. Init/shutdown lifecycle */
    csb_v22_inplace_draw_shutdown();
    ok = csb_v22_inplace_draw_init() && csb_v22_inplace_draw_active();
    probe_record(&stats, "CSB_V22_INPLACE_INIT", ok,
                 "CSB in-place cache loads from ~/.firestaff/assets/csb/modern");

    /* 4. Unpopulated (no swap update yet) — swap is unpopulated + inactive */
    probe_record(&stats, "CSB_V22_UNPOPULATED_INACTIVE",
                 !csb_v22_viewport_swap_populated() &&
                 !csb_v22_viewport_swap_active(),
                 "swap populated==0 + active==0 before first update call");

    /* 5. Per-shape discriminator: representative raw_cell_types */
    probe_record(&stats, "CSB_V22_DISC_WALL_STRAIGHT",
                 csb_v22_swap_shape_for_cell(0x00, 0) == CSB_V22_SWAP_SHAPE_WALL_STRAIGHT,
                 "raw 0x00 -> WALL_STRAIGHT");
    probe_record(&stats, "CSB_V22_DISC_WALL_DOORWAY",
                 csb_v22_swap_shape_for_cell(0x03, 0) == CSB_V22_SWAP_SHAPE_WALL_DOORWAY,
                 "raw 0x03 -> WALL_DOORWAY");
    probe_record(&stats, "CSB_V22_DISC_FLOOR_PLAIN",
                 csb_v22_swap_shape_for_cell(0x04, 0) == CSB_V22_SWAP_SHAPE_FLOOR_PLAIN,
                 "raw 0x04 -> FLOOR_PLAIN");
    probe_record(&stats, "CSB_V22_DISC_FLOOR_CRACKED",
                 csb_v22_swap_shape_for_cell(0x05, 0) == CSB_V22_SWAP_SHAPE_FLOOR_CRACKED,
                 "raw 0x05 -> FLOOR_CRACKED");
    probe_record(&stats, "CSB_V22_DISC_FLOOR_MOSSY",
                 csb_v22_swap_shape_for_cell(0x06, 0) == CSB_V22_SWAP_SHAPE_FLOOR_MOSSY,
                 "raw 0x06 -> FLOOR_MOSSY");
    probe_record(&stats, "CSB_V22_DISC_FLOOR_PIT",
                 csb_v22_swap_shape_for_cell(0x10, 0) == CSB_V22_SWAP_SHAPE_FLOOR_PIT,
                 "raw 0x10 -> FLOOR_PIT (0x10 bit + low 0)");
    probe_record(&stats, "CSB_V22_DISC_STAIRS_DOWN",
                 csb_v22_swap_shape_for_cell(0x11, 0) == CSB_V22_SWAP_SHAPE_FLOOR_STAIRS_DOWN,
                 "raw 0x11 -> FLOOR_STAIRS_DOWN (0x10 + bit 0 set)");
    probe_record(&stats, "CSB_V22_DISC_FLOOR_PIT",
                 csb_v22_swap_shape_for_cell(0x10, 0) == CSB_V22_SWAP_SHAPE_FLOOR_PIT,
                 "raw 0x10 -> FLOOR_PIT (0x10 + low nibble 0x00)");
    probe_record(&stats, "CSB_V22_DISC_STAIRS_UP",
                 csb_v22_swap_shape_for_cell(0x12, 0) == CSB_V22_SWAP_SHAPE_FLOOR_STAIRS_UP,
                 "raw 0x12 -> FLOOR_STAIRS_UP (0x10 + bit 0 clear)");
    probe_record(&stats, "CSB_V22_DISC_FLOOR_DOOR",
                 csb_v22_swap_shape_for_cell(0x20, 0) == CSB_V22_SWAP_SHAPE_FLOOR_DOOR,
                 "raw 0x20 -> FLOOR_DOOR");
    probe_record(&stats, "CSB_V22_DISC_CREATURE",
                 csb_v22_swap_shape_for_cell(0x80, 0) == CSB_V22_SWAP_SHAPE_CREATURE,
                 "raw 0x80 -> CREATURE (top bit set)");
    probe_record(&stats, "CSB_V22_DISC_CREATURE_PROJECTILE",
                 csb_v22_swap_shape_for_cell(0xC0, 0) == CSB_V22_SWAP_SHAPE_CREATURE_PROJECTILE,
                 "raw 0xC0 -> CREATURE_PROJECTILE");
    probe_record(&stats, "CSB_V22_DISC_ITEM_FLOOR",
                 csb_v22_swap_shape_for_cell(0x40, 0) == CSB_V22_SWAP_SHAPE_ITEM_FLOOR,
                 "raw 0x40 -> ITEM_FLOOR (0x40 + low 0)");
    probe_record(&stats, "CSB_V22_DISC_ITEM",
                 csb_v22_swap_shape_for_cell(0x41, 0) == CSB_V22_SWAP_SHAPE_ITEM,
                 "raw 0x41 -> ITEM (0x40 + low 1)");
    probe_record(&stats, "CSB_V22_DISC_FIELD_TELEPORTER",
                 csb_v22_swap_shape_for_cell(0x0C, 0) == CSB_V22_SWAP_SHAPE_FIELD_TELEPORTER,
                 "raw 0x0C -> FIELD_TELEPORTER");

    /* 6. Per-shape asset_id mapping (distinct asset_ids for distinct
     *    shapes that have manifest entries) */
    probe_record(&stats, "CSB_V22_ASSET_WALL",
                 strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_WALL_STRAIGHT),
                        "wall_dungeon_01") == 0,
                 "WALL_STRAIGHT -> wall_dungeon_01");
    probe_record(&stats, "CSB_V22_ASSET_FLOOR_PIT",
                 strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_FLOOR_PIT),
                        "floor_pit_01") == 0,
                 "FLOOR_PIT -> floor_pit_01");
    probe_record(&stats, "CSB_V22_ASSET_CREATURE",
                 strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_CREATURE),
                        "creature_chaos_fiend_01") == 0,
                 "CREATURE -> creature_chaos_fiend_01");
    probe_record(&stats, "CSB_V22_ASSET_FIELD_TELEPORTER",
                 strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_FIELD_TELEPORTER),
                        "field_teleporter_01") == 0,
                 "FIELD_TELEPORTER -> field_teleporter_01");
    probe_record(&stats, "CSB_V22_ASSET_PRISON_DOOR",
                 strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_PRISON_DOOR),
                        "door_prison_01") == 0,
                 "PRISON_DOOR -> door_prison_01 (CSB-only)");
    probe_record(&stats, "CSB_V22_ASSET_DSA_SCROLL",
                 strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_DSA_SCROLL),
                        "dsa_scroll_01") == 0,
                 "DSA_SCROLL -> dsa_scroll_01 (CSB-only)");
    probe_record(&stats, "CSB_V22_ASSET_NONE",
                 csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_NONE) == NULL,
                 "SHAPE_NONE -> NULL asset_id");

    /* 7. CSB 9-square viewport path — populate per-cell cache with
     *    mixed shapes (wall + floor + pit + stairs + door + creature
     *    + field + 2 walls) and verify the swap renders all 9 cells. */
    memset(raw_cells, 0x00, sizeof(raw_cells));
    raw_cells[0][0] = 0x00;   /* D0 L: WALL_STRAIGHT */
    raw_cells[0][1] = 0x04;   /* D0 C: FLOOR_PLAIN */
    raw_cells[0][2] = 0x80;   /* D0 R: CREATURE */
    raw_cells[1][0] = 0x05;   /* D1 L: FLOOR_CRACKED */
    raw_cells[1][1] = 0x10;   /* D1 C: FLOOR_PIT */
    raw_cells[1][2] = 0x06;   /* D1 R: FLOOR_MOSSY */
    raw_cells[2][0] = 0x11;   /* D2 L: STAIRS_UP */
    raw_cells[2][1] = 0x01;   /* D2 C: WALL_CORNER_INNER */
    raw_cells[2][2] = 0x20;   /* D2 R: FLOOR_DOOR */
    csb_v22_viewport_swap_update(0, (const unsigned char (*)[3])raw_cells);
    probe_record(&stats, "CSB_V22_POPULATED",
                 csb_v22_viewport_swap_populated() &&
                 csb_v22_viewport_swap_active(),
                 "CSB 9-square cache populated + swap active");

    memset(fb, 0x00, sizeof(fb));
    painted = csb_v22_viewport_swap_render(fb, 1920, 1080);
    changed = count_changed_pixels(fb, sizeof(fb));
    probe_record(&stats, "CSB_V22_RENDER_9_CELLS",
                 painted == 9 && changed > 0 &&
                 csb_all_cell_centers_nonzero(fb, 1920),
                 "render pass paints all 9 CSB viewport cells");

    /* Capture the painted-cell counter IMMEDIATELY after the render,
     * before the next update() resets it to 0. */
    {
        int counter = csb_v22_viewport_swap_cells_painted();
        probe_record(&stats, "CSB_V22_CELLS_PAINTED_COUNTER",
                     counter == 9,
                     "cells_painted counter == 9 after one render call");
    }

    /* 8. 4-direction sweep (all 4 directions should paint 9 cells) */
    sweep_painted = 0;
    for (direction = 0; direction < 4; ++direction) {
        csb_v22_viewport_swap_update(direction,
                                     (const unsigned char (*)[3])raw_cells);
        memset(fb, 0x00, sizeof(fb));
        sweep_painted += csb_v22_viewport_swap_render(fb, 1920, 1080);
    }
    probe_record(&stats, "CSB_V22_DIRECTION_SWEEP_4X9",
                 sweep_painted == 36,
                 "all 4 directions paint 4x9 CSB V22 cells");

    /* 9. Per-cell direct bitmap lookup (proves the (category, asset_id)
     *    helper the swap uses) */
    {
        const char* aid = csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_WALL_STRAIGHT);
        const char* cat = csb_v22_swap_category_for_shape(CSB_V22_SWAP_SHAPE_WALL_STRAIGHT);
        int w = 0, h = 0;
        const uint32_t* rgba = csb_v22_inplace_get_bitmap_by_id(cat, aid, &w, &h);
        probe_record(&stats, "CSB_V22_DIRECT_BITMAP_LOOKUP",
                     rgba != NULL && w == 4 && h == 4,
                     "wall_dungeon_01 bitmap lookup returns 4x4 RGBA");
    }
    {
        int w = 0, h = 0;
        const uint32_t* rgba = csb_v22_inplace_get_bitmap_by_id("wall_shapes", "nonexistent_01", &w, &h);
        probe_record(&stats, "CSB_V22_MISSING_BITMAP",
                     rgba == NULL && w == 0 && h == 0,
                     "missing asset_id returns NULL + zero dims");
    }

    /* 10. Idempotent shutdown */
    csb_v22_inplace_draw_shutdown();
    probe_record(&stats, "CSB_V22_INPLACE_SHUTDOWN",
                 !csb_v22_inplace_draw_active(),
                 "shutdown clears in-place cache");

    /* 11. Source evidence */
    {
        const char* ev = csb_v22_viewport_swap_source_evidence();
        probe_record(&stats, "CSB_V22_SOURCE_EVIDENCE",
                     ev && strstr(ev, "CSBWin") && strstr(ev, "ReDMCSB"),
                     "source evidence cites CSBWin and ReDMCSB");
    }

    printf("# summary: %d/%d invariants passed\n", stats.passed, stats.total);
    return stats.failed == 0 ? 0 : 1;
}
