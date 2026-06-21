/*
 * firestaff_dm1_v22_inplace_render_probe.c
 *
 * DM1 V2.2 modern-renderer headless probe.
 *
 * Builds a minimal temporary v22_inplace_cache.bin under a probe-only
 * HOME, activates DM1 V2.2, populates the 3x3 DM1 shape cache, and
 * verifies that m11_v22_inplace_render_pass paints all 9 viewport cells.
 *
 * Source-lock follows the module under test:
 * ReDMCSB DUNVIEW.C:6697-6816 for DM1 composition order,
 * DUNGEON.C:2238-2246 for square-type decode, and dm1_v22_shapes.c
 * for DM1-specific V2.2 shape classification.
 */

#include "dm1_v2_presentation_mode_pc34.h"
#include "fs_portable_compat.h"
#include "m11_v22_inplace_draw_pc34.h"
#include "m11_v22_shape_cache_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct ProbeStats {
    int total;
    int passed;
    int failed;
} ProbeStats;

static void probe_record(ProbeStats* stats,
                         const char* id,
                         int ok,
                         const char* note) {
    stats->total += 1;
    if (ok) {
        stats->passed += 1;
        printf("PASS %s: %s\n", id, note);
    } else {
        stats->failed += 1;
        printf("FAIL %s: %s\n", id, note);
    }
}

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

static int write_minimal_dm1_v22_cache(const char* cache_path) {
    FILE* fp;
    unsigned char header[32];
    unsigned char entry[32];
    const uint32_t rgba[4] = {
        0x00ffffffu, 0x0000ff00u,
        0x000000ffu, 0x00ffffffu
    };

    memset(header, 0, sizeof(header));
    memset(entry, 0, sizeof(entry));
    memcpy(header, "FSV22C\0\0", 8);
    put_u32(header + 8, 1u);
    put_u32(header + 12, 1u);

    put_u32(entry + 0, fnv1a_hash("wall_shapes"));
    put_u32(entry + 4, fnv1a_hash("wall_d3_carved_01"));
    put_u32(entry + 8, 2u);
    put_u32(entry + 12, 2u);
    put_u32(entry + 16, (uint32_t)sizeof(rgba));
    put_u32(entry + 20, 64u);

    fp = fopen(cache_path, "wb");
    if (!fp) return 0;
    if (fwrite(header, 1, sizeof(header), fp) != sizeof(header) ||
        fwrite(entry, 1, sizeof(entry), fp) != sizeof(entry) ||
        fwrite(rgba, 1, sizeof(rgba), fp) != sizeof(rgba)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int setup_probe_home(char* out_cache_path, size_t out_size) {
    char modern_dir[FSP_PATH_MAX];
    int n;

    n = snprintf(modern_dir, sizeof(modern_dir),
                 "firestaff-dm1-v22-probe-home/.firestaff/assets/dm1/modern");
    if (n <= 0 || (size_t)n >= sizeof(modern_dir)) return 0;
    if (!FSP_CreateDirectoryRecursive(modern_dir)) return 0;
    if (FSP_SetEnv("HOME", "firestaff-dm1-v22-probe-home", 1) != 0) return 0;

    n = snprintf(out_cache_path, out_size, "%s/v22_inplace_cache.bin", modern_dir);
    return n > 0 && (size_t)n < out_size;
}

static int count_changed_pixels(const unsigned char* fb, size_t len) {
    size_t i;
    int changed = 0;
    for (i = 0; i < len; ++i) {
        if (fb[i] != 0x00) changed++;
    }
    return changed;
}

static int dm1_all_cell_centers_nonzero(const unsigned char* fb, int fbW) {
    static const int centers[9][2] = {
        { 42, 118 }, {108, 118 }, {173, 118 },
        { 42,  87 }, {108,  87 }, {173,  87 },
        { 42,  56 }, {108,  56 }, {173,  56 }
    };
    int i;
    for (i = 0; i < 9; ++i) {
        if (fb[centers[i][1] * fbW + centers[i][0]] == 0x00) return 0;
    }
    return 1;
}

int main(void) {
    ProbeStats stats;
    char cache_path[FSP_PATH_MAX];
    unsigned char raw_cells[3][3] = {
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 }
    };
    unsigned char fb[320 * 200];
    int painted;
    int changed;
    int w = 0;
    int h = 0;
    const uint32_t* bitmap;
    int direction;
    int sweep_painted = 0;

    memset(&stats, 0, sizeof(stats));
    memset(cache_path, 0, sizeof(cache_path));

    probe_record(&stats, "DM1_V22_CACHE_FIXTURE",
                 setup_probe_home(cache_path, sizeof(cache_path)) &&
                 write_minimal_dm1_v22_cache(cache_path),
                 "temporary DM1 v22_inplace_cache.bin written");

    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    probe_record(&stats, "DM1_V22_PRESENTATION_ACTIVE",
                 dm1_v2_presentation_mode_is_v22(),
                 "DM1 V2.2 presentation mode resolves to V22 with pack available");

    m11_v22_inplace_draw_shutdown();
    probe_record(&stats, "DM1_V22_INPLACE_INIT",
                 m11_v22_inplace_draw_init() && m11_v22_inplace_draw_active(),
                 "DM1 in-place cache loads from ~/.firestaff/assets/dm1/modern");

    dm1_v2_presentation_mode_set(DM1_V2_PM_V1_FAITHFUL);
    m11_v22_shape_cache_update(0, (const unsigned char (*)[3])raw_cells);
    probe_record(&stats, "DM1_V22_V1_CACHE_INACTIVE",
                 !m11_v22_shape_cache_active(1, 0),
                 "shape cache remains inactive outside V22 mode");

    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, (const unsigned char (*)[3])raw_cells);
    bitmap = m11_v22_inplace_get_cell_bitmap(1, 0, &w, &h);
    probe_record(&stats, "DM1_V22_CELL_BITMAP",
                 bitmap != NULL && w == 2 && h == 2,
                 "D1 center maps to wall_d3_carved_01 bitmap");

    memset(fb, 0x00, sizeof(fb));
    painted = m11_v22_inplace_render_pass(fb, 320, 200);
    changed = count_changed_pixels(fb, sizeof(fb));
    probe_record(&stats, "DM1_V22_RENDER_9_CELLS",
                 painted == 9 && changed > 0 && dm1_all_cell_centers_nonzero(fb, 320),
                 "render pass paints all 9 DM1 viewport cells");

    for (direction = 0; direction < 4; ++direction) {
        memset(fb, 0x00, sizeof(fb));
        m11_v22_shape_cache_update(direction, (const unsigned char (*)[3])raw_cells);
        sweep_painted += m11_v22_inplace_render_pass(fb, 320, 200);
    }
    probe_record(&stats, "DM1_V22_DIRECTION_SWEEP_4X9",
                 sweep_painted == 36,
                 "all 4 directions paint 4x9 DM1 V22 cells");

    {
        const char* ev = m11_v22_inplace_draw_source_evidence();
        probe_record(&stats, "DM1_V22_SOURCE_EVIDENCE",
                     ev && strstr(ev, "ReDMCSB") && strstr(ev, "v22_inplace_cache.bin"),
                     "source evidence cites ReDMCSB and the in-place cache");
    }

    m11_v22_inplace_draw_shutdown();
    printf("# summary: %d/%d invariants passed\n", stats.passed, stats.total);
    return stats.failed == 0 ? 0 : 1;
}
