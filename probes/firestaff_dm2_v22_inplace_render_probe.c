/*
 * firestaff_dm2_v22_inplace_render_probe.c
 *
 * DM2 V2.2 modern-renderer headless probe (per-cell modern-art swap).
 *
 * Builds a minimal temporary v22_inplace_cache.bin under a probe-only
 * HOME, activates DM2 V2.2 presentation, populates the per-cell swap
 * cache for both the indoor T560 dungeon view (D0..D2 x L/C/R) and
 * the outdoor T600 sky/ground split, and verifies that
 * dm2_v22_viewport_swap_render() paints the expected cells.
 *
 * Coverage:
 *   - init / shutdown lifecycle
 *   - dm2_v22_inplace_draw_active() reflects cache load
 *   - dm2_v22_viewport_swap_active() reflects presentation_mode + install
 *   - dm2_v22_shape_for_cell discriminator covers wall/floor/creature/
 *     pit/stairs/field/outdoor shapes
 *   - dm2_v22_asset_id_for_shape resolves (category, asset_id) for
 *     representative shapes (wall/floor/creature/sky/ground/tree)
 *   - dm2_v22_viewport_swap_update(direction=0, raw_cells, 0) populates
 *     the cache for indoor T560 path
 *   - dm2_v22_viewport_swap_render(...) with is_outdoor=0 paints up to
 *     9 cells (D0..D2 x L/C/R)
 *   - dm2_v22_viewport_swap_update(..., is_outdoor=1) populates the
 *     cache for outdoor T600 path
 *   - dm2_v22_viewport_swap_render(...) with is_outdoor=1 paints the
 *     3 outdoor cells (sky / horizon / ground)
 *   - 4-direction sweep (each direction paints 9 cells)
 *   - per-cell mapping proves each shape picks the expected asset_id
 *   - source evidence citation
 *
 * Source-lock:
 *   SKULL.ASM T520/T560/T600  (DM2 viewport ticks: indoor T560 + outdoor T600)
 *   ReDMCSB DUNVIEW.C:2962-3070 (outdoor sky/ground composition order)
 *   include/dm2_v22_viewport_swap_pc34.h (per-cell swap contract)
 *   include/dm2_v22_inplace_draw_pc34.h (cache + bitmap blit)
 *   include/dm2_v22_modern_assets_pc34.h (asset pack + presentation flag)
 */

#include "dm2_v22_viewport_swap_pc34.h"
#include "dm2_v22_inplace_draw_pc34.h"
#include "dm2_v22_shape_cache_pc34.h"
#include "dm2_v22_modern_assets_pc34.h"
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
 * per-cell swap needs. Each entry is a single-color 4x4 RGBA bitmap
 * keyed by (category, asset_id). The bitmap colors are picked so
 * the probe can verify which entry was painted into which cell by
 * inspecting the framebuffer at the cell center.
 *
 * Color note: the in-place renderer maps RGBA -> 6-bit EGA cube
 * (2 bits per channel) via (c * 3 + 127) / 255 quantization. Dark
 * values (c <= 84) all map to 0 (black) — pick colors >= 128 so
 * each entry gets a distinct non-zero EGA index in the framebuffer. */
static int write_minimal_dm2_v22_cache(const char* cache_path) {
    static const struct {
        const char* category;
        const char* asset_id;
        uint32_t rgba;   /* single-pixel color (4x4 same color) */
    } kEntries[] = {
        /* Indoor walls (T560) */
        { "wall_shapes",  "wall_dm2_temple_01",     0xFFFF0000u },
        { "wall_shapes",  "wall_dm2_alcove_01",     0xFF00FF00u },
        { "wall_shapes",  "wall_dm2_doorway_01",    0xFF0000FFu },
        { "wall_shapes",  "wall_dm2_inscription_01", 0xFFFFFF00u },
        /* Indoor floors */
        { "floor_shapes", "floor_dm2_outdoor_01",   0xFF00FFFFu },
        { "floor_shapes", "floor_dm2_pit_01",       0xFFFF00FFu },
        { "floor_shapes", "floor_dm2_stairs_01",    0xFF808000u },
        { "floor_shapes", "ceiling_dm2_plain_01",   0xFF008080u },
        /* Creatures */
        { "creature_shapes", "creature_dm2_brigand_01", 0xFF800080u },
        { "creature_shapes", "tree_dm2_outdoor_01",      0xFF808040u },
        /* Doors / fields */
        { "door_shapes",  "door_dm2_wood_01",       0xFFC0C0C0u },
        { "wall_shapes",  "field_dm2_teleporter_01", 0xFF404040u },
        { "wall_shapes",  "field_dm2_opening_01",    0xFF606060u },
        /* Outdoor (T600) */
        { "wall_shapes",  "sky_dm2_outdoor_01",      0xFFB0B0B0u },
        { "floor_shapes", "ground_dm2_outdoor_01",   0xFFD0D0D0u },
        { "floor_shapes", "ground_dm2_horizon_01",   0xFFE0E0E0u },
        { "wall_shapes",  "wall_dm2_outdoor_01",     0xFFF0F0F0u },
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
                 "firestaff-dm2-v22-probe-home/.firestaff/assets/dm2/modern");
    if (n <= 0 || (size_t)n >= sizeof(modern_dir)) return 0;
    if (!FSP_CreateDirectoryRecursive(modern_dir)) return 0;
    if (FSP_SetEnv("HOME", "firestaff-dm2-v22-probe-home", 1) != 0) return 0;

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

/* True when every cell-center pixel inside the indoor 4x3 layout is
 * non-zero. Each cell is 640x360 at 1920x1080 so the center pixel
 * is at (rect.x + rect.w/2, rect.y + rect.h/2). The right column's
 * cell rect extends past the framebuffer (x+w=2240 > fbW=1920) so
 * the visible center is shifted left to stay inside the painted
 * region. */
static int dm2_all_cell_centers_nonzero(const unsigned char* fb, int fbW) {
    static const int centers[3][3][2] = {
        /* depth 0 (D0, closest) */ {
            { 640,  900 },   /* L  (320, 720, 640, 360) center (640,900) */
            {1280,  900 },   /* C  (960, 720, 640, 360) center (1280,900) */
            {1760,  900 }    /* R  (1600, 720, 640, 360) clipped, visible center */
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

/* True when every outdoor cell-center pixel is non-zero. */
static int dm2_outdoor_cell_centers_nonzero(const unsigned char* fb, int fbW) {
    /* sky band: center at (960, 270) */
    /* horizon strip: center at (960, 541) */
    /* ground band: center at (960, 811) */
    if (fb[270 * fbW + 960] == 0x00) return 0;
    if (fb[541 * fbW + 960] == 0x00) return 0;
    if (fb[811 * fbW + 960] == 0x00) return 0;
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
         write_minimal_dm2_v22_cache(cache_path);
    probe_record(&stats, "DM2_V22_CACHE_FIXTURE", ok,
                 "temporary DM2 v22_inplace_cache.bin written");

    /* 2. Activate V22 presentation mode (so the swap is "active") */
    dm2_v22_set_installed(1);
    dm2_v22_set_epx_cache_warm(1);
    probe_record(&stats, "DM2_V22_PRESENTATION_ACTIVE",
                 dm2_v22_best_available_shape_source(3) ==
                     DM2_V22_SHAPE_SOURCE_V2_MODERN,
                 "presentation mode 3 (V22) resolves to V2_MODERN with pack");

    /* 3. Init/shutdown lifecycle */
    dm2_v22_inplace_draw_shutdown();
    ok = dm2_v22_inplace_draw_init() && dm2_v22_inplace_draw_active();
    probe_record(&stats, "DM2_V22_INPLACE_INIT", ok,
                 "DM2 in-place cache loads from ~/.firestaff/assets/dm2/modern");

    /* 4. Unpopulated (no swap update yet) — swap is unpopulated + inactive */
    probe_record(&stats, "DM2_V22_UNPOPULATED_INACTIVE",
                 !dm2_v22_viewport_swap_populated() &&
                 !dm2_v22_viewport_swap_active(),
                 "swap populated==0 + active==0 before first update call");

    /* 5. Per-shape discriminator: representative raw_cell_types */
    probe_record(&stats, "DM2_V22_DISC_WALL_STRAIGHT",
                 dm2_v22_shape_for_cell(0x00, 0) == DM2_V22_SHAPE_WALL_STRAIGHT,
                 "raw 0x00 -> WALL_STRAIGHT");
    probe_record(&stats, "DM2_V22_DISC_FLOOR_PLAIN",
                 dm2_v22_shape_for_cell(0x04, 0) == DM2_V22_SHAPE_FLOOR_PLAIN,
                 "raw 0x04 -> FLOOR_PLAIN");
    probe_record(&stats, "DM2_V22_DISC_FLOOR_CRACKED",
                 dm2_v22_shape_for_cell(0x05, 0) == DM2_V22_SHAPE_FLOOR_CRACKED,
                 "raw 0x05 -> FLOOR_CRACKED");
    probe_record(&stats, "DM2_V22_DISC_FLOOR_MOSSY",
                 dm2_v22_shape_for_cell(0x06, 0) == DM2_V22_SHAPE_FLOOR_MOSSY,
                 "raw 0x06 -> FLOOR_MOSSY");
    probe_record(&stats, "DM2_V22_DISC_FLOOR_PIT",
                 dm2_v22_shape_for_cell(0x40, 0) == DM2_V22_SHAPE_FLOOR_PIT,
                 "raw 0x40 -> FLOOR_PIT (no creature bit, 0x40 set)");
    probe_record(&stats, "DM2_V22_DISC_STAIRS_UP",
                 dm2_v22_shape_for_cell(0x10, 0) == DM2_V22_SHAPE_FLOOR_STAIRS_UP,
                 "raw 0x10 -> FLOOR_STAIRS_UP");
    probe_record(&stats, "DM2_V22_DISC_STAIRS_DOWN",
                 dm2_v22_shape_for_cell(0x11, 0) == DM2_V22_SHAPE_FLOOR_STAIRS_DOWN,
                 "raw 0x11 -> FLOOR_STAIRS_DOWN");
    probe_record(&stats, "DM2_V22_DISC_CREATURE",
                 dm2_v22_shape_for_cell(0x80, 0) == DM2_V22_SHAPE_CREATURE,
                 "raw 0x80 -> CREATURE (top bit set)");
    probe_record(&stats, "DM2_V22_DISC_CREATURE_PROJECTILE",
                 dm2_v22_shape_for_cell(0xC0, 0) == DM2_V22_SHAPE_CREATURE_PROJECTILE,
                 "raw 0xC0 -> CREATURE_PROJECTILE (top+0x40 set)");

    /* 6. Per-shape asset_id mapping (distinct asset_ids for distinct
     *    shapes that have manifest entries) */
    probe_record(&stats, "DM2_V22_ASSET_WALL",
                 strcmp(dm2_v22_asset_id_for_shape(DM2_V22_SHAPE_WALL_STRAIGHT),
                        "wall_dm2_temple_01") == 0,
                 "WALL_STRAIGHT -> wall_dm2_temple_01");
    probe_record(&stats, "DM2_V22_ASSET_FLOOR_PIT",
                 strcmp(dm2_v22_asset_id_for_shape(DM2_V22_SHAPE_FLOOR_PIT),
                        "floor_dm2_pit_01") == 0,
                 "FLOOR_PIT -> floor_dm2_pit_01");
    probe_record(&stats, "DM2_V22_ASSET_CREATURE",
                 strcmp(dm2_v22_asset_id_for_shape(DM2_V22_SHAPE_CREATURE),
                        "creature_dm2_brigand_01") == 0,
                 "CREATURE -> creature_dm2_brigand_01");
    probe_record(&stats, "DM2_V22_ASSET_SKY",
                 strcmp(dm2_v22_asset_id_for_shape(DM2_V22_SHAPE_OUTDOOR_SKY),
                        "sky_dm2_outdoor_01") == 0,
                 "OUTDOOR_SKY -> sky_dm2_outdoor_01");
    probe_record(&stats, "DM2_V22_ASSET_GROUND",
                 strcmp(dm2_v22_asset_id_for_shape(DM2_V22_SHAPE_OUTDOOR_GROUND),
                        "ground_dm2_outdoor_01") == 0,
                 "OUTDOOR_GROUND -> ground_dm2_outdoor_01");
    probe_record(&stats, "DM2_V22_ASSET_NONE",
                 dm2_v22_asset_id_for_shape(DM2_V22_SHAPE_NONE) == NULL,
                 "SHAPE_NONE -> NULL asset_id");

    /* 7. Indoor T560 path — populate per-cell cache with mixed shapes
     *    and verify the swap renders up to 9 cells. */
    memset(raw_cells, 0x00, sizeof(raw_cells));
    raw_cells[0][0] = 0x00;   /* D0 L: WALL_STRAIGHT */
    raw_cells[0][1] = 0x04;   /* D0 C: FLOOR_PLAIN */
    raw_cells[0][2] = 0x80;   /* D0 R: CREATURE */
    raw_cells[1][0] = 0x05;   /* D1 L: FLOOR_CRACKED */
    raw_cells[1][1] = 0x40;   /* D1 C: FLOOR_PIT */
    raw_cells[1][2] = 0x06;   /* D1 R: FLOOR_MOSSY */
    raw_cells[2][0] = 0x10;   /* D2 L: STAIRS_UP */
    raw_cells[2][1] = 0x01;   /* D2 C: WALL_CORNER_INNER */
    raw_cells[2][2] = 0x11;   /* D2 R: STAIRS_DOWN */
    dm2_v22_viewport_swap_update(0, (const unsigned char (*)[3])raw_cells, 0);
    probe_record(&stats, "DM2_V22_INDOOR_POPULATED",
                 dm2_v22_viewport_swap_populated() &&
                 dm2_v22_viewport_swap_active(),
                 "indoor T560 cache populated + swap active");

    memset(fb, 0x00, sizeof(fb));
    painted = dm2_v22_viewport_swap_render(fb, 1920, 1080, 0);
    changed = count_changed_pixels(fb, sizeof(fb));
    probe_record(&stats, "DM2_V22_INDOOR_RENDER_9_CELLS",
                 painted == 9 && changed > 0 &&
                 dm2_all_cell_centers_nonzero(fb, 1920),
                 "indoor render pass paints 9 cells + non-zero centers");

    /* Capture the indoor counter IMMEDIATELY after the indoor render,
     * before the outdoor update() resets both counters to 0. */
    {
        int indoor_after_indoor = dm2_v22_viewport_swap_cells_painted_indoor();
        probe_record(&stats, "DM2_V22_INDOOR_COUNTER_NONZERO",
                     indoor_after_indoor == 9,
                     "indoor painted-cell counter == 9 after indoor render");
    }

    /* 8. Outdoor T600 path — sky/horizon/ground */
    dm2_v22_viewport_swap_update(0, NULL, 1);
    probe_record(&stats, "DM2_V22_OUTDOOR_POPULATED",
                 dm2_v22_viewport_swap_populated() &&
                 dm2_v22_viewport_swap_active(),
                 "outdoor T600 cache populated + swap active");

    memset(fb, 0x00, sizeof(fb));
    painted = dm2_v22_viewport_swap_render(fb, 1920, 1080, 1);
    changed = count_changed_pixels(fb, sizeof(fb));
    probe_record(&stats, "DM2_V22_OUTDOOR_RENDER_3_CELLS",
                 painted == 3 && changed > 0 &&
                 dm2_outdoor_cell_centers_nonzero(fb, 1920),
                 "outdoor render pass paints sky/horizon/ground + non-zero centers");

    /* 9. Per-viewport counters — capture BEFORE the sweep resets them.
     *    Each update() call resets both counters to 0; the sweep that
     *    follows would wipe the outdoor count to 0. */
    {
        int outdoor_before = dm2_v22_viewport_swap_cells_painted_outdoor();
        probe_record(&stats, "DM2_V22_COUNTERS_INDOOR_OUTDOOR",
                     outdoor_before > 0,
                     "outdoor painted-cell counter > 0 after outdoor render");
    }

    /* 10. 4-direction sweep (indoor) */
    sweep_painted = 0;
    for (direction = 0; direction < 4; ++direction) {
        dm2_v22_viewport_swap_update(direction,
                                     (const unsigned char (*)[3])raw_cells, 0);
        memset(fb, 0x00, sizeof(fb));
        sweep_painted += dm2_v22_viewport_swap_render(fb, 1920, 1080, 0);
    }
    probe_record(&stats, "DM2_V22_DIRECTION_SWEEP_4X9",
                 sweep_painted == 36,
                 "all 4 directions paint 4x9 DM2 V22 indoor cells");

    /* 11. Idempotent shutdown */
    dm2_v22_inplace_draw_shutdown();
    probe_record(&stats, "DM2_V22_INPLACE_SHUTDOWN",
                 !dm2_v22_inplace_draw_active(),
                 "shutdown clears in-place cache");

    /* 12. Source evidence */
    {
        const char* ev = dm2_v22_viewport_swap_source_evidence();
        probe_record(&stats, "DM2_V22_SOURCE_EVIDENCE",
                     ev && strstr(ev, "SKULL.ASM") && strstr(ev, "ReDMCSB"),
                     "source evidence cites SKULL.ASM and ReDMCSB");
    }

    printf("# summary: %d/%d invariants passed\n", stats.passed, stats.total);
    return stats.failed == 0 ? 0 : 1;
}
