/*
 * firestaff_csb_v22_inplace_render_probe.c
 *
 * CSB V2.2 modern-renderer headless probe.
 *
 * Builds a minimal temporary v22_inplace_cache.bin under a probe-only
 * HOME, activates CSB V2.2, populates the 3x3 CSB shape cache, and
 * verifies that csb_v22_inplace_render_pass paints all 9 viewport cells.
 *
 * The synthetic cache is keyed by the per-cell material-routing gate
 * (csb_v22_inplace_route_pc34.h) so per-depth wall variants resolve
 * to distinct synthetic RGBA. A 9-cell uniform-wall scene therefore
 * paints all 9 cells, but the painted framebuffer uses three different
 * per-depth color indices (d0/d1/d2) — proving per-cell routing is
 * actually consulted by the draw pass.
 *
 * Source-lock follows the module under test:
 * ReDMCSB DUNVIEW.C F0128 / composition order, CSBWin/Viewport.cpp:7290
 * for the 9-square viewport, csb_v22_shapes.c for CSB-specific V2.2
 * shape classification, and csb_v22_inplace_route_pc34.h for the
 * per-cell asset-routing contract.
 */

#include "csb_v22_inplace_draw_pc34.h"
#include "csb_v22_inplace_route_pc34.h"
#include "csb_v22_shape_cache_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
#include "fs_portable_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

static int write_minimal_csb_v22_cache(const char* cache_path) {
    /* Per-depth wall_shapes + door_shapes so the per-cell route
     * gate's asset_id table resolves for the 9-cell uniform-wall
     * scene used by this probe. Each entry is a 2x2 RGBA bitmap
     * (4 pixels = 16 bytes), color-coded so the per-depth
     * distinction is observable in the rendered framebuffer. */
    typedef struct {
        const char* category;
        const char* asset_id;
        uint32_t    rgba[4];
    } ProbeCacheEntry;
    static const ProbeCacheEntry kEntries[] = {
        { "wall_shapes", "wall_dungeon_d0_01",
          { 0x00ff0000u, 0x00ff0000u, 0x00ff0000u, 0x00ff0000u } },
        { "wall_shapes", "wall_dungeon_d1_01",
          { 0x0000ff00u, 0x0000ff00u, 0x0000ff00u, 0x0000ff00u } },
        { "wall_shapes", "wall_dungeon_d2_01",
          { 0x000000ffu, 0x000000ffu, 0x000000ffu, 0x000000ffu } },
    };
    static const int kEntryCount = (int)(sizeof(kEntries) / sizeof(kEntries[0]));
    const uint32_t data_block_size = 32u /* header */
                                   + (uint32_t)kEntryCount * 32u /* entries */
                                   + (uint32_t)kEntryCount * 16u /* rgba blocks */;
    unsigned char header[32];
    unsigned char entries[16][32];
    FILE* fp;
    int i;

    memset(header, 0, sizeof(header));
    memset(entries, 0, sizeof(entries));
    memcpy(header, "FSV22C\0\0", 8);
    put_u32(header + 8, 1u);
    put_u32(header + 12, (uint32_t)kEntryCount);
    for (i = 0; i < kEntryCount; ++i) {
        unsigned char* e = entries[i];
        put_u32(e + 0,  fnv1a_hash(kEntries[i].category));
        put_u32(e + 4,  fnv1a_hash(kEntries[i].asset_id));
        put_u32(e + 8,  2u);
        put_u32(e + 12, 2u);
        put_u32(e + 16, 16u);
        put_u32(e + 20, (uint32_t)(32u + (uint32_t)i * 32u + (uint32_t)i * 16u));
        /* Caller rewrites the rgba_offset to be absolute (data
         * block follows the entries). */
    }
    /* Recompute rgba_offset to be absolute file offset. */
    for (i = 0; i < kEntryCount; ++i) {
        unsigned char* e = entries[i];
        put_u32(e + 20, 32u + (uint32_t)kEntryCount * 32u + (uint32_t)i * 16u);
    }

    fp = fopen(cache_path, "wb");
    if (!fp) return 0;
    if (fwrite(header, 1, sizeof(header), fp) != sizeof(header)) {
        fclose(fp); return 0;
    }
    if (fwrite(entries, 32, (size_t)kEntryCount, fp) != (size_t)kEntryCount) {
        fclose(fp); return 0;
    }
    for (i = 0; i < kEntryCount; ++i) {
        if (fwrite(kEntries[i].rgba, 1, sizeof(kEntries[i].rgba), fp) !=
            sizeof(kEntries[i].rgba)) {
            fclose(fp); return 0;
        }
    }
    (void)data_block_size;
    return fclose(fp) == 0;
}

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

static int count_changed_pixels(const unsigned char* fb, size_t len) {
    size_t i;
    int changed = 0;
    for (i = 0; i < len; ++i) {
        if (fb[i] != 0x00) changed++;
    }
    return changed;
}

static int csb_all_cell_centers_nonzero(const unsigned char* fb, int fbW) {
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

/* Return the EGA palette index the in-place blit produces for an
 * RGBA pixel. Mirrors rgb_to_ega_index in csb_v22_inplace_draw_pc34.c
 * so the probe can assert the per-depth color distinction. */
static unsigned char probe_rgb_to_ega_index(unsigned char r,
                                              unsigned char g,
                                              unsigned char b) {
    int ri = (r * 3 + 127) / 255;
    int gi = (g * 3 + 127) / 255;
    int bi = (b * 3 + 127) / 255;
    return (unsigned char)((ri << 4) | (gi << 2) | bi);
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

    probe_record(&stats, "CSB_V22_CACHE_FIXTURE",
                 setup_probe_home(cache_path, sizeof(cache_path)) &&
                 write_minimal_csb_v22_cache(cache_path),
                 "temporary CSB v22_inplace_cache.bin written");

    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set(CSB_V2_PM_V22_MODERN);
    probe_record(&stats, "CSB_V22_PRESENTATION_ACTIVE",
                 csb_v2_presentation_mode_is_v22(),
                 "CSB V2.2 presentation mode resolves to V22 with pack available");

    csb_v22_inplace_draw_shutdown();
    probe_record(&stats, "CSB_V22_INPLACE_INIT",
                 csb_v22_inplace_draw_init() && csb_v22_inplace_draw_active(),
                 "CSB in-place cache loads from ~/.firestaff/assets/csb/modern");

    csb_v2_presentation_mode_set(CSB_V2_PM_V1_FAITHFUL);
    csb_v22_shape_cache_update(0, (const unsigned char (*)[3])raw_cells);
    probe_record(&stats, "CSB_V22_V1_CACHE_INACTIVE",
                 !csb_v22_shape_cache_active(0, 0),
                 "shape cache remains inactive outside V22 mode");

    csb_v2_presentation_mode_set(CSB_V2_PM_V22_MODERN);
    csb_v22_shape_cache_update(0, (const unsigned char (*)[3])raw_cells);
    bitmap = csb_v22_inplace_get_cell_bitmap(0, 0, &w, &h);
    probe_record(&stats, "CSB_V22_CELL_BITMAP",
                 bitmap != NULL && w == 2 && h == 2,
                 "D0 center maps to wall_dungeon_d0_01 bitmap (per-cell route)");

    {
        const char* aid_d0 = csb_v22_inplace_get_cell_asset_id(0, 0);
        const char* aid_d1 = csb_v22_inplace_get_cell_asset_id(1, 0);
        const char* aid_d2 = csb_v22_inplace_get_cell_asset_id(2, 0);
        probe_record(&stats, "CSB_V22_PER_CELL_ROUTE_DEPTH",
                     aid_d0 && aid_d1 && aid_d2 &&
                     strcmp(aid_d0, "wall_dungeon_d0_01") == 0 &&
                     strcmp(aid_d1, "wall_dungeon_d1_01") == 0 &&
                     strcmp(aid_d2, "wall_dungeon_d2_01") == 0,
                     "depth-driven wall asset_id swap is per-cell (d0/d1/d2)");
    }

    memset(fb, 0x00, sizeof(fb));
    painted = csb_v22_inplace_render_pass(fb, 320, 200);
    changed = count_changed_pixels(fb, sizeof(fb));
    probe_record(&stats, "CSB_V22_RENDER_9_CELLS",
                 painted == 9 && changed > 0 && csb_all_cell_centers_nonzero(fb, 320),
                 "render pass paints all 9 CSB viewport cells");

    {
        /* Per-depth color distinctness: 0xff0000 -> ri=3 gi=0 bi=0
         * -> (3<<4)|(0<<2)|0 = 0x30; 0x00ff00 -> 0x0c; 0x0000ff -> 0x03. */
        static const int centers[3] = { 108, 108, 108 };   /* x coords (depth-different y) */
        static const int y_rows[3]   = { 118,  87,  56 };  /* depth 0/1/2 -> y */
        unsigned char idx_d0 = fb[y_rows[0] * 320 + centers[0]];
        unsigned char idx_d1 = fb[y_rows[1] * 320 + centers[1]];
        unsigned char idx_d2 = fb[y_rows[2] * 320 + centers[2]];
        probe_record(&stats, "CSB_V22_RENDER_PER_CELL_COLOR",
                     idx_d0 == probe_rgb_to_ega_index(0xff, 0x00, 0x00) &&
                     idx_d1 == probe_rgb_to_ega_index(0x00, 0xff, 0x00) &&
                     idx_d2 == probe_rgb_to_ega_index(0x00, 0x00, 0xff),
                     "per-depth wall art paints three distinct EGA indices");
    }

    for (direction = 0; direction < 4; ++direction) {
        memset(fb, 0x00, sizeof(fb));
        csb_v22_shape_cache_update(direction, (const unsigned char (*)[3])raw_cells);
        sweep_painted += csb_v22_inplace_render_pass(fb, 320, 200);
    }
    probe_record(&stats, "CSB_V22_DIRECTION_SWEEP_4X9",
                 sweep_painted == 36,
                 "all 4 directions paint 4x9 CSB V22 cells");

    {
        const char* ev = csb_v22_inplace_draw_source_evidence();
        probe_record(&stats, "CSB_V22_SOURCE_EVIDENCE",
                     ev && strstr(ev, "ReDMCSB") && strstr(ev, "CSBWin"),
                     "source evidence cites ReDMCSB and CSBWin");
    }
    {
        const char* rev = csb_v22_inplace_route_source_evidence();
        probe_record(&stats, "CSB_V22_ROUTE_SOURCE_EVIDENCE",
                     rev && strstr(rev, "routing gate") && strstr(rev, "ReDMCSB"),
                     "per-cell route source evidence cites the gate contract");
    }
    {
        int pair_count = csb_v22_inplace_route_pair_count();
        probe_record(&stats, "CSB_V22_ROUTE_PAIR_TABLE",
                     pair_count >= 27 && pair_count <= 64,
                     "per-cell route pair table is well-bounded");
    }

    csb_v22_inplace_draw_shutdown();
    printf("# summary: %d/%d invariants passed\n", stats.passed, stats.total);
    return stats.failed == 0 ? 0 : 1;
}
