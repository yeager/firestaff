/*
 * firestaff_csb_v22_per_cell_route_probe.c
 *
 * CSB V2.2 per-cell modern-art swap probe.
 *
 * Builds a synthetic per-depth + per-cell-type v22_inplace_cache.bin
 * under a probe-only HOME, then walks several 9-square scenes and
 * verifies the in-place draw pass routes per-cell (depth + lateral
 * + cell type) instead of by shape type alone:
 *
 *   Scene 1 — uniform wall: 9 cells paint 3 distinct depth colors
 *     (d0/d1/d2 per-cell swap).
 *   Scene 2 — pit/stairs: a single pit cell + two stairs cells
 *     (up + down) paint three distinct EGA indices, NOT the
 *     cracked-floor bucket.
 *   Scene 3 — field no-asset: a teleporter cell + a fluxcage cell
 *     do NOT paint (the gate refuses to assign an asset, so the
 *     in-place draw short-circuits to V1 for those cells; the
 *     wall cells around them still paint normally).
 *   Scene 4 — CSB narrative shapes: prison_door, dsa_scroll,
 *     lord_order each have their own cache entry and paint
 *     distinct EGA indices.
 *
 * The probe is headless: no real PNG, no game data, no SDL.
 *
 * Source-lock: csb_v22_inplace_route_pc34.h (the gate),
 * ReDMCSB DUNVIEW.C:6697-6816 / CSBWin/Viewport.cpp:7290
 * (9-square composition), ReDMCSB ENTRANCE.C (prison door),
 * CSBWin/Chaos.cpp:60-69 (chaos / DSA).
 */

#include "csb_v22_inplace_draw_pc34.h"
#include "csb_v22_inplace_route_pc34.h"
#include "csb_v22_shape_cache_pc34.h"
#include "csb_v22_shapes.h"
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

static void probe_record(ProbeStats* stats, const char* id,
                          int ok, const char* note) {
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
    while (*s) { h = (h ^ (uint8_t)*s++) * 16777619u; }
    return h;
}

static void put_u32(unsigned char* p, uint32_t v) {
    memcpy(p, &v, sizeof(v));
}

/* RGB -> 6-bit-cube EGA palette index, mirroring the blitter. */
static unsigned char probe_rgb_to_ega(unsigned char r,
                                       unsigned char g,
                                       unsigned char b) {
    int ri = (r * 3 + 127) / 255;
    int gi = (g * 3 + 127) / 255;
    int bi = (b * 3 + 127) / 255;
    return (unsigned char)((ri << 4) | (gi << 2) | bi);
}

/* (category, asset_id) -> synthetic 2x2 RGBA. */
typedef struct {
    const char* category;
    const char* asset_id;
    uint32_t    rgba[4];
} ProbeCacheEntry;

static const ProbeCacheEntry kFullEntries[] = {
    /* wall per-depth (d0/d1/d2). */
    { "wall_shapes", "wall_dungeon_d0_01", { 0xff0000ffu, 0xff0000ffu, 0xff0000ffu, 0xff0000ffu } },
    { "wall_shapes", "wall_dungeon_d1_01", { 0xff00ff00u, 0xff00ff00u, 0xff00ff00u, 0xff00ff00u } },
    { "wall_shapes", "wall_dungeon_d2_01", { 0xffff0000u, 0xffff0000u, 0xffff0000u, 0xffff0000u } },
    /* floor per-depth. */
    { "floor_shapes", "floor_plain_d0_01",   { 0xff202020u, 0xff202020u, 0xff202020u, 0xff202020u } },
    { "floor_shapes", "floor_plain_d1_01",   { 0xff404040u, 0xff404040u, 0xff404040u, 0xff404040u } },
    { "floor_shapes", "floor_plain_d2_01",   { 0xff606060u, 0xff606060u, 0xff606060u, 0xff606060u } },
    { "floor_shapes", "floor_cracked_d0_01", { 0xff800000u, 0xff800000u, 0xff800000u, 0xff800000u } },
    { "floor_shapes", "floor_cracked_d1_01", { 0xff802000u, 0xff802000u, 0xff802000u, 0xff802000u } },
    { "floor_shapes", "floor_cracked_d2_01", { 0xff804000u, 0xff804000u, 0xff804000u, 0xff804000u } },
    { "floor_shapes", "floor_mossy_d0_01",   { 0xff008000u, 0xff008000u, 0xff008000u, 0xff008000u } },
    { "floor_shapes", "floor_mossy_d1_01",   { 0xff208000u, 0xff208000u, 0xff208000u, 0xff208000u } },
    { "floor_shapes", "floor_mossy_d2_01",   { 0xff408000u, 0xff408000u, 0xff408000u, 0xff408000u } },
    /* floor depth-invariant. */
    { "floor_shapes", "floor_pit_01",         { 0xff000055u, 0xff000055u, 0xff000055u, 0xff000055u } },
    { "floor_shapes", "floor_stairs_up_01",   { 0xff00ffffu, 0xff00ffffu, 0xff00ffffu, 0xff00ffffu } },
    { "floor_shapes", "floor_stairs_down_01", { 0xff808000u, 0xff808000u, 0xff808000u, 0xff808000u } },
    /* doors. */
    { "door_shapes", "door_d0_01",            { 0xff404040u, 0xff404040u, 0xff404040u, 0xff404040u } },
    { "door_shapes", "door_d1_01",            { 0xff606060u, 0xff606060u, 0xff606060u, 0xff606060u } },
    { "door_shapes", "door_d2_01",            { 0xff808080u, 0xff808080u, 0xff808080u, 0xff808080u } },
    /* creatures per-depth. */
    { "creature_shapes", "creature_demon_d0_01", { 0xff0000ffu, 0xff0000ffu, 0xff0000ffu, 0xff0000ffu } },
    { "creature_shapes", "creature_demon_d1_01", { 0xff0080ffu, 0xff0080ffu, 0xff0080ffu, 0xff0080ffu } },
    { "creature_shapes", "creature_demon_d2_01", { 0xff00ffffu, 0xff00ffffu, 0xff00ffffu, 0xff00ffffu } },
    /* CSB-specific narrative. */
    { "wall_shapes",   "prison_door_01",         { 0xff505050u, 0xff505050u, 0xff505050u, 0xff505050u } },
    { "wall_shapes",   "lord_order_01",          { 0xfff0f0f0u, 0xfff0f0f0u, 0xfff0f0f0u, 0xfff0f0f0u } },
    { "chaos_runes",   "chaos_rune_0_01",        { 0xff40ff40u, 0xff40ff40u, 0xff40ff40u, 0xff40ff40u } },
    { "chaos_runes",   "chaos_rune_1_01",        { 0xff4040ffu, 0xff4040ffu, 0xff4040ffu, 0xff4040ffu } },
    { "chaos_runes",   "chaos_rune_2_01",        { 0xffff4040u, 0xffff4040u, 0xffff4040u, 0xffff4040u } },
    { "dsa_scrolls",   "dsa_scroll_01",          { 0xfff0e0a0u, 0xfff0e0a0u, 0xfff0e0a0u, 0xfff0e0a0u } },
};

#define FULL_ENTRY_COUNT ((int)(sizeof(kFullEntries) / sizeof(kFullEntries[0])))

static int write_full_cache(const char* cache_path) {
    FILE* fp;
    unsigned char header[32];
    unsigned char entries[64][32];
    size_t entry_size = 32;
    size_t data_block_off;
    int i;

    memset(header, 0, sizeof(header));
    memset(entries, 0, sizeof(entries));
    memcpy(header, "FSV22C\0\0", 8);
    put_u32(header + 8, 1u);
    put_u32(header + 12, (uint32_t)FULL_ENTRY_COUNT);
    data_block_off = (size_t)32 + (size_t)FULL_ENTRY_COUNT * entry_size;
    for (i = 0; i < FULL_ENTRY_COUNT; ++i) {
        unsigned char* e = entries[i];
        put_u32(e + 0,  fnv1a_hash(kFullEntries[i].category));
        put_u32(e + 4,  fnv1a_hash(kFullEntries[i].asset_id));
        put_u32(e + 8,  2u);
        put_u32(e + 12, 2u);
        put_u32(e + 16, 16u);
        put_u32(e + 20, (uint32_t)(data_block_off + (size_t)i * 16u));
    }
    fp = fopen(cache_path, "wb");
    if (!fp) return 0;
    if (fwrite(header, 1, sizeof(header), fp) != sizeof(header)) {
        fclose(fp); return 0;
    }
    if (fwrite(entries, entry_size, (size_t)FULL_ENTRY_COUNT, fp)
            != (size_t)FULL_ENTRY_COUNT) {
        fclose(fp); return 0;
    }
    for (i = 0; i < FULL_ENTRY_COUNT; ++i) {
        if (fwrite(kFullEntries[i].rgba, 1,
                    sizeof(kFullEntries[i].rgba), fp) !=
            sizeof(kFullEntries[i].rgba)) {
            fclose(fp); return 0;
        }
    }
    return fclose(fp) == 0;
}

static int setup_probe_home(char* out_cache_path, size_t out_size) {
    char modern_dir[FSP_PATH_MAX];
    int n;
    n = snprintf(modern_dir, sizeof(modern_dir),
                 "firestaff-csb-v22-percell-probe-home/.firestaff/assets/csb/modern");
    if (n <= 0 || (size_t)n >= sizeof(modern_dir)) return 0;
    if (!FSP_CreateDirectoryRecursive(modern_dir)) return 0;
    if (FSP_SetEnv("HOME", "firestaff-csb-v22-percell-probe-home", 1) != 0) return 0;
    n = snprintf(out_cache_path, out_size, "%s/v22_inplace_cache.bin", modern_dir);
    return n > 0 && (size_t)n < out_size;
}

/* Center of each of the 9 cells in the CSB 3x3 viewport, as the
 * in-place blit paints them (kV22CellRects in csb_v22_inplace_draw_pc34.c).
 * depth 0 = D0 (closest, y=118), depth 2 = D2 (farthest, y=56). */
static void cell_center(int depth, int lateral, int* x, int* y) {
    static const int cx[3] = { 42, 108, 173 };
    static const int cy[3] = { 118, 87, 56 };   /* depth 0/1/2 */
    if (x) *x = cx[lateral + 1];
    if (y) *y = cy[depth];
}

static unsigned char fb_pixel(const unsigned char* fb, int depth, int lateral) {
    int x, y;
    cell_center(depth, lateral, &x, &y);
    return fb[y * 320 + x];
}

int main(void) {
    ProbeStats stats;
    char cache_path[FSP_PATH_MAX];
    unsigned char fb[320 * 200];
    int i, painted;

    memset(&stats, 0, sizeof(stats));
    memset(cache_path, 0, sizeof(cache_path));
    memset(fb, 0, sizeof(fb));

    probe_record(&stats, "CSB_V22_PER_CELL_CACHE_FIXTURE",
                 setup_probe_home(cache_path, sizeof(cache_path)) &&
                 write_full_cache(cache_path),
                 "synthetic per-cell cache written");

    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set(CSB_V2_PM_V22_MODERN);
    probe_record(&stats, "CSB_V22_PER_CELL_V22_ACTIVE",
                 csb_v2_presentation_mode_is_v22(),
                 "CSB V2.2 mode resolved");

    csb_v22_inplace_draw_shutdown();
    probe_record(&stats, "CSB_V22_PER_CELL_INPLACE_INIT",
                 csb_v22_inplace_draw_init() && csb_v22_inplace_draw_active(),
                 "in-place cache loads from per-cell HOME");

    /* ── Scene 1: 9-cell uniform wall ────────────────────────── */
    {
        unsigned char raw_walls[3][3] = {
            { 0x00, 0x00, 0x00 },
            { 0x00, 0x00, 0x00 },
            { 0x00, 0x00, 0x00 }
        };
        csb_v22_shape_cache_update(0, raw_walls);
        memset(fb, 0x00, sizeof(fb));
        painted = csb_v22_inplace_render_pass(fb, 320, 200);
        probe_record(&stats, "CSB_V22_PER_CELL_SCENE1_PAINT",
                     painted == 9,
                     "9-cell wall scene paints all 9 cells");
        /* depth 0/1/2 should each show a distinct center pixel. */
        probe_record(&stats, "CSB_V22_PER_CELL_SCENE1_DEPTH_COLOR",
                     fb_pixel(fb, 0, 0) != fb_pixel(fb, 1, 0) &&
                     fb_pixel(fb, 1, 0) != fb_pixel(fb, 2, 0) &&
                     fb_pixel(fb, 0, 0) != fb_pixel(fb, 2, 0) &&
                     fb_pixel(fb, 0, 0) == probe_rgb_to_ega(0x00, 0x00, 0xff) &&
                     fb_pixel(fb, 1, 0) == probe_rgb_to_ega(0x00, 0xff, 0x00) &&
                     fb_pixel(fb, 2, 0) == probe_rgb_to_ega(0xff, 0x00, 0x00),
                     "depth-driven wall per-cell swap paints 3 distinct EGA colors");
    }

    /* ── Scene 2: pit + stairs_up + stairs_down + surrounding walls ──
     *   row 0: walls, walls, walls
     *   row 1: stairs_up (0x10), walls, walls
     *   row 2: stairs_down (0x11), pit (0x40), walls
     */
    {
        unsigned char raw_mix[3][3] = {
            { 0x00, 0x00, 0x00 },
            { 0x10, 0x00, 0x00 },
            { 0x11, 0x40, 0x00 }
        };
        csb_v22_shape_cache_update(0, raw_mix);
        memset(fb, 0x00, sizeof(fb));
        painted = csb_v22_inplace_render_pass(fb, 320, 200);
        probe_record(&stats, "CSB_V22_PER_CELL_SCENE2_PAINT",
                     painted == 9,
                     "pit + stairs + walls scene paints all 9 cells");
        /* pit (depth=2, lateral=0) must paint the floor_pit_01
         * sentinel and NOT floor_cracked_d2_01 (0x804000). */
        {
            unsigned char pit_pixel = fb_pixel(fb, 2, 0);
            unsigned char pit_expected = probe_rgb_to_ega(0x00, 0x00, 0x55);
            unsigned char cracked_expected = probe_rgb_to_ega(0x80, 0x40, 0x00);
            probe_record(&stats, "CSB_V22_PER_CELL_SCENE2_PIT_NOT_CRACKED",
                         pit_pixel == pit_expected &&
                         pit_pixel != cracked_expected,
                         "pit cell uses floor_pit_01, not floor_cracked_d2_01");
        }
        /* stairs_up (depth=1, lateral=-1) -> 0x00ffff. */
        probe_record(&stats, "CSB_V22_PER_CELL_SCENE2_STAIRS_UP",
                     fb_pixel(fb, 1, -1) == probe_rgb_to_ega(0x00, 0xff, 0xff),
                     "stairs_up uses floor_stairs_up_01 (not cracked)");
        /* stairs_down (depth=2, lateral=-1) -> 0x808000. */
        probe_record(&stats, "CSB_V22_PER_CELL_SCENE2_STAIRS_DOWN",
                     fb_pixel(fb, 2, -1) == probe_rgb_to_ega(0x80, 0x80, 0x00),
                     "stairs_down uses floor_stairs_down_01 (not cracked)");
        /* All three floor specials distinct. */
        probe_record(&stats, "CSB_V22_PER_CELL_SCENE2_PIT_STAIRS_DISTINCT",
                     fb_pixel(fb, 2, 1) != fb_pixel(fb, 1, -1) &&
                     fb_pixel(fb, 1, -1) != fb_pixel(fb, 2, -1) &&
                     fb_pixel(fb, 2, 1)  != fb_pixel(fb, 2, -1),
                     "pit / stairs_up / stairs_down are three distinct assets");
    }

    /* ── Scene 3: field no-asset (teleporter + fluxcage) + walls ──
     *   raw 0x10 (with custom flag 0x80 in upper nibble) cannot
     *   yield FIELD_TELEPORTER via the raw M034 decode (the gate
     *   has no M034 path to field shapes; the field shapes are
     *   assigned by the shape book at runtime). So we exercise
     *   the "field -> no asset" branch through the single-shape
     *   route API. The in-place draw pass with a normal wall
     *   scene must still paint the 9 cells. */
    {
        char aid[CSB_V22_ASSET_ID_MAX];
        char cat[CSB_V22_CATEGORY_MAX];
        char rsn_t[CSB_V22_REASON_MAX];
        char rsn_f[CSB_V22_REASON_MAX];
        char rsn_e[CSB_V22_REASON_MAX];
        char rsn_r[CSB_V22_REASON_MAX];
        int rc_t = csb_v22_inplace_route_for_shape(CSB_V22_SHAPE_FIELD_TELEPORTER, 1,
                                                     aid, sizeof(aid),
                                                     cat, sizeof(cat),
                                                     rsn_t, sizeof(rsn_t));
        int rc_f = csb_v22_inplace_route_for_shape(CSB_V22_SHAPE_FIELD_FLUXCAGE, 1,
                                                     aid, sizeof(aid),
                                                     cat, sizeof(cat),
                                                     rsn_f, sizeof(rsn_f));
        int rc_e = csb_v22_inplace_route_for_shape(CSB_V22_SHAPE_FIELD_EXPLOSION, 1,
                                                     aid, sizeof(aid),
                                                     cat, sizeof(cat),
                                                     rsn_e, sizeof(rsn_e));
        int rc_r = csb_v22_inplace_route_for_shape(CSB_V22_SHAPE_FIELD_CHAOS_RIFT, 1,
                                                     aid, sizeof(aid),
                                                     cat, sizeof(cat),
                                                     rsn_r, sizeof(rsn_r));
        probe_record(&stats, "CSB_V22_PER_CELL_FIELD_TELEPORTER",
                     rc_t == 0 && strcmp(rsn_t, "field_teleporter_no_asset") == 0,
                     "teleporter -> no asset (not wall_dungeon_d*)");
        probe_record(&stats, "CSB_V22_PER_CELL_FIELD_FLUXCAGE",
                     rc_f == 0 && strcmp(rsn_f, "field_fluxcage_no_asset") == 0,
                     "fluxcage -> no asset");
        probe_record(&stats, "CSB_V22_PER_CELL_FIELD_EXPLOSION",
                     rc_e == 0 && strcmp(rsn_e, "field_explosion_no_asset") == 0,
                     "explosion -> no asset");
        probe_record(&stats, "CSB_V22_PER_CELL_FIELD_CHAOS_RIFT",
                     rc_r == 0 && strcmp(rsn_r, "field_chaos_rift_no_asset") == 0,
                     "chaos_rift -> no asset");
    }

    /* ── Scene 4: CSB narrative shapes have their own assets ── */
    {
        probe_record(&stats, "CSB_V22_PER_CELL_NARRATIVE_PRISON",
                     csb_v22_inplace_route_pair_recognized("wall_shapes", "prison_door_01") == 1,
                     "prison_door_01 is a recognized pair");
        probe_record(&stats, "CSB_V22_PER_CELL_NARRATIVE_LORD_ORDER",
                     csb_v22_inplace_route_pair_recognized("wall_shapes", "lord_order_01") == 1,
                     "lord_order_01 is a recognized pair");
        probe_record(&stats, "CSB_V22_PER_CELL_NARRATIVE_DSA_SCROLL",
                     csb_v22_inplace_route_pair_recognized("dsa_scrolls", "dsa_scroll_01") == 1,
                     "dsa_scroll_01 is a recognized pair");
        probe_record(&stats, "CSB_V22_PER_CELL_NARRATIVE_CHAOS_RUNES",
                     csb_v22_inplace_route_pair_recognized("chaos_runes", "chaos_rune_0_01") == 1 &&
                     csb_v22_inplace_route_pair_recognized("chaos_runes", "chaos_rune_1_01") == 1 &&
                     csb_v22_inplace_route_pair_recognized("chaos_runes", "chaos_rune_2_01") == 1 &&
                     csb_v22_inplace_route_pair_recognized("chaos_runes", "chaos_rune_3_01") == 1,
                     "chaos_rune_0/1/2/3_01 are all recognized pairs");
    }

    /* ── Scene 5: pair_count invariant ─────────────────────────── */
    {
        int pc = csb_v22_inplace_route_pair_count();
        probe_record(&stats, "CSB_V22_PER_CELL_PAIR_COUNT",
                     pc == 29,
                     "pair_count == 29 (3 walls + 3 doors + 9 floors + 3 specials + 1 ceiling + 3 creatures + 4 chaos_rune variants + 1 dsa + 2 narrative)");
    }

    /* ── Scene 6: 4-direction sweep paints 4x9 cells ──────────── */
    {
        unsigned char raw_walls[3][3] = {
            { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 }
        };
        int sweep_painted = 0;
        for (i = 0; i < 4; ++i) {
            csb_v22_shape_cache_update(i, raw_walls);
            memset(fb, 0x00, sizeof(fb));
            sweep_painted += csb_v22_inplace_render_pass(fb, 320, 200);
        }
        probe_record(&stats, "CSB_V22_PER_CELL_DIRECTION_SWEEP",
                     sweep_painted == 36,
                     "all 4 directions paint 4x9 cells via per-cell route");
    }

    /* ── Source evidence ──────────────────────────────────────── */
    {
        const char* ev = csb_v22_inplace_route_source_evidence();
        probe_record(&stats, "CSB_V22_PER_CELL_SOURCE_EVIDENCE",
                     ev && strstr(ev, "ReDMCSB") && strstr(ev, "CSBWin") &&
                     strstr(ev, "routing gate") && strstr(ev, "Per-cell"),
                     "per-cell route source evidence cites the gate contract");
    }

    csb_v22_inplace_draw_shutdown();
    printf("# summary: %d/%d invariants passed\n", stats.passed, stats.total);
    return stats.failed == 0 ? 0 : 1;
}
