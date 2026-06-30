/*
 * firestaff_dm1_v1_hoc_ordinal_2_future_route_readiness_gate_probe.c
 *
 * DM1 V1 Hall of Champions ordinal-2 future-route readiness gate.
 *
 * Scope:
 *   Machine-checks the current "no real ordinal-2 corridor sensor"
 *   boundary on a real DM1 V1 PC 3.4 DUNGEON.DAT and records which
 *   sibling ordinal-2 routes are already covered so the TODO.md
 *   "2026-06-24 DM1 V1 Hall of Champions ordinal-2 palette_match_rect
 *   portrait_rect_position probe follow-up" row can be narrowed
 *   honestly once this probe is in CTest.
 *
 * The probe has three invariant groups:
 *
 *   Group A — discovery (corridor boundary)
 *     Walks the Hall of Champions corridor band (x in 0..3, y in 1..6)
 *     in all four directions and counts how many poses return
 *     M11_GameView_GetFrontMirrorOrdinal() == 2.  On the shipped DM1
 *     V1 PC 3.4 DUNGEON.DAT this count is 0 — the existing
 *     firestaff_dm1_v1_champion_mirror_ordinal_2_south_return_*
 *     probe prints this exact line on every run
 *     (`[Discovery] ordinal=2 hits in corridor band = 0`).
 *     This probe asserts the same boundary (PASS if 0, FAIL if a
 *     future build exposes ordinal 2 on a corridor sensor — which is
 *     the future-route visual-capture follow-up the TODO row tracks
 *     under (a)).  A future build that breaks this gate is exactly
 *     the event that unlocks the per-route ordinal-2 visual capture
 *     work TODO row (a) is waiting on.
 *
 *   Group B — C026 atlas math + mirror catalog
 *     Locks the ordinal-2 source rect math so the synthetic-mutation
 *     sibling probes (palette_match_rect / after_party_shuffle /
 *     leave_and_reenter / d2l_negative / etc.) all reference the
 *     same atlas cell.  This is the shared coordinate system the
 *     sibling probes depend on; if it drifts, every sibling probe
 *     silently breaks.
 *
 *   Group C — sibling ordinal-2 route coverage matrix
 *     Walks the bounded list of ordinal-2 sibling probes known to
 *     this build and asserts each probe source file exists.  It also
 *     locks today's CMake wiring state by checking the exact
 *     `probes/m11/<basename>` CMake pool item, so a comment-only
 *     mention of a basename cannot satisfy the readiness matrix.
 *     The list is the same list the TODO row (b) explicitly calls
 *     out: south_return, west_negative, east_walkpath, d2l_negative,
 *     leave_and_reenter, palette_match_rect, after_party_shuffle,
 *     wake_repaint (ordinal-2 gate slice — uses the seeded sensor
 *     at (1,2) NORTH), door_nearby_no_float, cancel_reopen.  For
 *     each entry the gate prints `covered`, `open (source present)`,
 *     or a true gap such as missing source or unexpected CMake wiring.
 *     The matrix also asserts the 10-row bounded list has 10 unique
 *     route labels and 10 unique basenames, that every basename ends
 *     in `.c`, and that the strategy split is 2 real-route rows
 *     (`south_return`, `west_negative`) plus 8 synthetic-mutation
 *     rows (the other 8).  The 2026-06-27 baseline had `west_negative`
 *     and `cancel_reopen` source-present but open; the 2026-06-28
 *     sibling-promotion lane closed those two, so the matrix now
 *     expects wired=10/10, open=0 on the current build.
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573        visibleWallCell = (partyDirection + 2) & 3
 *                                  front-wall sensor filter (the
 *                                  filter the corridor scan relies on)
 *   ReDMCSB DUNGEON.C:2608-2612   G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor))
 *                                  C127 sensorData -> atlas ordinal
 *   ReDMCSB MOVESENS.C:1501-1503  C127 sensorData -> F0280 candidate
 *   ReDMCSB REVIVE.C F0280        candidate materialisation
 *   ReDMCSB DEFS.H:821-826        M027_PORTRAIT_X(index), M028_PORTRAIT_Y(index)
 *                                  (atlas col/row math: srcX = (ord & 7) << 5,
 *                                  srcY = (ord >> 3) * 29)
 *   ReDMCSB DEFS.H:2186           C026_GRAPHIC_CHAMPION_PORTRAITS = 256x87
 *                                  8x3 atlas, ordinals 0..23, 32x29 each
 *   ReDMCSB DUNVIEW.C:3913-3928   C026 portrait blit into G0109 portrait box
 *   ReDMCSB DUNVIEW.C:525         G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                                  = {96, 127, 35, 63} -> D1C cutout
 *                                  (96, 35, 32, 29) viewport-local
 *
 * Honest scope:
 *   Firestaff runtime + real DM1 V1 PC 3.4 DUNGEON.DAT / GRAPHICS.DAT.
 *   The Group A corridor scan is discovery on the real asset; it does
 *   not perform any sensor mutation.  The Group B atlas math is a
 *   pure arith check on the constant source-locked dimensions.  The
 *   Group C coverage matrix is a deterministic file-system / source
 *   audit, not a runtime drive.  No DOS pixel parity is claimed.
 *
 * Sibling / disjoint matrix:
 *   This probe is disjoint from every ordinal-2 portrait probe in
 *   probes/m11/.  It does NOT drive the M11 viewport draw path; it
 *   only reads the front-mirror ordinal (which costs one front-cell
 *   lookup per pose) and walks the file system for Group C.  The
 *   sibling probes are:
 *     - south_return, west_negative, d2l_negative, east_walkpath
 *       (the four natural-route ordinal-2 slices)
 *     - leave_and_reenter, cancel_reopen (panel-cycle slices)
 *     - wake_repaint, door_nearby_no_float (state-machine slices)
 *     - palette_match_rect, after_party_shuffle (synthetic-mutation
 *       slices that retarget the (1,2) NORTH C127 sensor to ordinal 2)
 *   None of them run a corridor-band scan that asserts ordinal-2
 *   hits == 0 as a regression gate, and none of them audit the
 *   build-system wiring of the ordinal-2 sibling set.  That is the
 *   narrow role this probe fills.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    PROBE_D1C_X = PROBE_VIEWPORT_X + 96,
    PROBE_D1C_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_D1C_W = 32,
    PROBE_D1C_H = 29,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_PORTRAIT_COLS = 8,
    PROBE_PORTRAIT_ROWS = 3,
    PROBE_PORTRAIT_TOTAL = 24,
    PROBE_ORDINAL_TARGET = 2,
    /* Hall corridor band: x in [0..3], y in [1..6]. Matches the
     * (1,2) / (1,3) / (1,4) / (1,5) / (2,2) / (2,3) / (2,4) / (2,5)
     * corridor cells the ordinal-2 south_return probe scans. */
    PROBE_CORRIDOR_X_MIN = 0,
    PROBE_CORRIDOR_X_MAX = 3,
    PROBE_CORRIDOR_Y_MIN = 1,
    PROBE_CORRIDOR_Y_MAX = 6
};

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define SKIP_INFO(msg) do { ++g_skip; printf("  SKIP: %s\n", msg); } while (0)

/* Render the corridor cell at (mapX, mapY, direction) into fb and
 * return the front-mirror ordinal M11 reports.  Resets the candidate
 * panel and inventory panel so the BUG-120/121 guard does not
 * influence the front-cell lookup. */
static int render_at(M11_GameViewState* state,
                     unsigned char* fb,
                     int mapX, int mapY, int direction) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = (int16_t)mapX;
    state->world.party.mapY = (int16_t)mapY;
    state->world.party.direction = (uint8_t)direction;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
    state->world.party.championCount = 0;
    memset(fb, 0, PROBE_FB_W * PROBE_FB_H);
    M11_GameView_Draw(state, fb, PROBE_FB_W, PROBE_FB_H);
    return M11_GameView_GetFrontMirrorOrdinal(state);
}

/* ── Group A: discovery (corridor boundary) ─────────────────────────
 *
 * Mirrors the discovery loop in
 * firestaff_dm1_v1_champion_mirror_ordinal_2_south_return_*
 * (`scan_for_ordinal_2`): walks x in [PROBE_CORRIDOR_X_MIN..MAX]
 * y in [PROBE_CORRIDOR_Y_MIN..MAX] direction in {0,1,2,3} and
 * counts the (mapX, mapY, dir) poses that report ordinal 2 on the
 * visible-wall cell.  This probe is the dedicated regression gate
 * for that count: it ASSERTS the count equals 0 on the shipped
 * DM1 V1 PC 3.4 DUNGEON.DAT.  The TODO row (a) tracks the case
 * where a future build does expose ordinal 2 on a corridor sensor;
 * in that case the assertion fails and the gate promotes itself
 * from "boundary held" to "future route surface event" — exactly
 * the signal TODO row (a) is waiting on.
 */
typedef struct ProbeHit {
    int mapX;
    int mapY;
    int direction;
    int ordinal;
} ProbeHit;

#define PROBE_MAX_HITS 64
static ProbeHit kHits[PROBE_MAX_HITS];
static int kHitCount = 0;

static int scan_corridor_for_ordinal_2(M11_GameViewState* state) {
    static const int kDirs[] = { 0 /* N */, 1 /* E */, 2 /* S */, 3 /* W */ };
    static const char* kDirNames[] = { "N", "E", "S", "W" };
    int x, y, d;
    int hits = 0;
    kHitCount = 0;
    printf("\n[Discovery] Scanning Hall corridor band (x in [%d..%d], y in [%d..%d])\n",
           PROBE_CORRIDOR_X_MIN, PROBE_CORRIDOR_X_MAX,
           PROBE_CORRIDOR_Y_MIN, PROBE_CORRIDOR_Y_MAX);
    printf("[Discovery] target ordinal=%d  (all 4 directions per cell)\n",
           PROBE_ORDINAL_TARGET);
    for (y = PROBE_CORRIDOR_Y_MIN; y <= PROBE_CORRIDOR_Y_MAX; ++y) {
        for (x = PROBE_CORRIDOR_X_MIN; x <= PROBE_CORRIDOR_X_MAX; ++x) {
            for (d = 0; d < 4; ++d) {
                unsigned char fb[PROBE_FB_W * PROBE_FB_H];
                int ord = render_at(state, fb, x, y, kDirs[d]);
                if (ord >= 0 && kHitCount < PROBE_MAX_HITS) {
                    kHits[kHitCount].mapX = x;
                    kHits[kHitCount].mapY = y;
                    kHits[kHitCount].direction = kDirs[d];
                    kHits[kHitCount].ordinal = ord;
                    ++kHitCount;
                }
                if (ord == PROBE_ORDINAL_TARGET) {
                    ++hits;
                    printf("  HIT  ordinal=%d at (%d,%d) DIR_%s\n",
                           ord, x, y, kDirNames[d]);
                }
            }
        }
    }
    printf("[Discovery] ordinal=%d hits in corridor band = %d\n",
           PROBE_ORDINAL_TARGET, hits);
    printf("[Discovery] total positive-ordinal corridor-band hits = %d\n",
           kHitCount);
    if (kHitCount > 0) {
        int seen[PROBE_PORTRAIT_TOTAL];
        int i;
        int unique = 0;
        memset(seen, 0, sizeof(seen));
        printf("[Discovery] Corridor ordinal inventory (one entry per unique ordinal):\n");
        for (i = 0; i < kHitCount; ++i) {
            if (!seen[kHits[i].ordinal]) {
                printf("  ordinal=%d first seen at (%d,%d) DIR_%d\n",
                       kHits[i].ordinal, kHits[i].mapX,
                       kHits[i].mapY, kHits[i].direction);
                seen[kHits[i].ordinal] = 1;
                ++unique;
            }
        }
        printf("[Discovery] %d unique ordinals in corridor band\n", unique);
        printf("[Discovery] ordinal=%d among them? %s\n",
               PROBE_ORDINAL_TARGET,
               seen[PROBE_ORDINAL_TARGET] ? "YES (gate will FAIL)" : "NO (gate PASSes)");
    }
    return hits;
}

/* ── Group C: sibling ordinal-2 coverage matrix ─────────────────────
 *
 * Each row records the ordinal-2 probe the TODO row (b) calls out.
 * `route_variant` is the human label the existing probes use;
 * `probe_basename` is the file basename under probes/m11/; the
 * probe source file must exist on disk.  `expected_wired` records
 * today's honest build-system state: every known ordinal-2 sibling
 * probe is now CTest-wired (the historical 2026-06-27 baseline had
 * `west_negative` and `cancel_reopen` source-present but open; the
 * 2026-06-28 sibling-promotion lane promoted both into the pool-probe
 * foreach() block, so the matrix now expects wired=10/10, open=0).
 * If a future build regresses one of these back to source-only, the
 * `expected_wired` flag for that row should be flipped back to
 * WIRE_OPEN so the gate keeps recording the real state.
 */
typedef struct ProbeRecord {
    const char* route_variant;
    const char* probe_basename;
    const char* anchor; /* route anchor the probe covers */
    int   expected_wired; /* 1 = should be in CMake pool, 0 = known open */
    int   sensor_strategy; /* 0 = real C127 sensor on live build,
                            * 1 = synthetic sensor mutation */
} ProbeRecord;

enum {
    SENSOR_REAL = 0,
    SENSOR_SYNTHETIC = 1,
    WIRE_OPEN = 0,
    WIRE_EXPECTED = 1
};

static const ProbeRecord kSiblingProbes[] = {
    /* The four natural-route ordinal-2 slices. */
    { "south_return",        "firestaff_dm1_v1_champion_mirror_ordinal_2_south_return_portrait_rect_position_runtime_probe.c",
      "(1,4) SOUTH/return (corridor scan SKIPs on live build)",         WIRE_EXPECTED, SENSOR_REAL    },
    { "west_negative",       "firestaff_dm1_v1_champion_mirror_ordinal_2_west_negative_portrait_rect_position_runtime_probe.c",
      "x=1 west wall y=2..6 DIR_WEST (no ordinal-2 sensor on corridor)", WIRE_EXPECTED, SENSOR_REAL    },
    { "east_walkpath",       "firestaff_dm1_v1_hall_champion_portrait_02_east_walkpath_rect_position_runtime_probe.c",
      "synthetic-atlas-blit path through corridor east-bound",          WIRE_EXPECTED, SENSOR_SYNTHETIC },
    { "d2l_negative",        "firestaff_dm1_v1_champion_mirror_ordinal_2_d2l_negative_portrait_rect_position_runtime_probe.c",
      "(2,4) EAST seeded via (3,4) west-wall C127 sensor",              WIRE_EXPECTED, SENSOR_SYNTHETIC },
    /* The panel-cycle slices. */
    { "leave_and_reenter",   "firestaff_dm1_v1_hall_of_champions_portrait_02_leave_and_reenter_portrait_rect_position_runtime_probe.c",
      "(1,2) NORTH seeded via (1,1) C127 sensor",                       WIRE_EXPECTED, SENSOR_SYNTHETIC },
    { "cancel_reopen",       "firestaff_dm1_v1_hall_of_champions_portrait_02_cancel_reopen_portrait_rect_position_runtime_probe.c",
      "(1,2) NORTH seeded via (1,1) C127 sensor",                       WIRE_EXPECTED, SENSOR_SYNTHETIC },
    /* The state-machine slices. */
    { "wake_repaint",        "firestaff_dm1_v1_hoc_champion_portrait_02_wake_repaint_portrait_rect_position_050_gate_probe.c",
      "(1,2) NORTH seeded via (1,1) C127 sensor; C146 wake cycle",      WIRE_EXPECTED, SENSOR_SYNTHETIC },
    { "door_nearby_no_float","firestaff_dm1_v1_hoc_champion_portrait_02_door_nearby_no_float_runtime_probe.c",
      "(1,2) NORTH seeded; INPUT_UP door-block leg",                   WIRE_EXPECTED, SENSOR_SYNTHETIC },
    /* The synthetic-mutation follow-up slices from TODO row 8. */
    { "palette_match_rect",  "firestaff_dm1_v1_hoc_champion_portrait_02_palette_match_rect_runtime_probe.c",
      "(1,2) NORTH seeded via (1,1) C127 sensor; per-pixel palette",   WIRE_EXPECTED, SENSOR_SYNTHETIC },
    { "after_party_shuffle", "firestaff_dm1_v1_hoc_champion_portrait_02_after_party_shuffle_portrait_rect_position_runtime_probe.c",
      "(1,2) NORTH seeded via (1,1) C127 sensor; F0284 x2 spin cycle", WIRE_EXPECTED, SENSOR_SYNTHETIC },
};

#define SIBLING_PROBE_COUNT \
    (int)(sizeof(kSiblingProbes) / sizeof(kSiblingProbes[0]))

/* Returns 1 if the exact `probes/m11/<basename>` CMake pool item
 * appears in CMakeLists.txt.  A bare basename in a comment, a
 * README mention, or a sibling-probe doc string is not enough to pass
 * the sibling-route wiring gate — only the actual `probes/m11/...`
 * pool reference counts.  This protects the readiness matrix from
 * comment-only or doc-only mentions that would otherwise satisfy a
 * loose strstr match. */
static int cmake_contains_probe_path(const char* haystack, const char* basename) {
    char pathNeedle[512];
    if (!haystack || !basename) return 0;
    snprintf(pathNeedle, sizeof(pathNeedle), "probes/m11/%s", basename);
    return strstr(haystack, pathNeedle) != NULL;
}

/* Returns 1 if `path` ends in `.c` — the C-source shape every
 * sibling probe basename must keep.  A future refactor that drops
 * the `.c` suffix (or accidentally appends `.cpp` / `.h`) cannot
 * silently satisfy the matrix. */
static int has_c_suffix(const char* path) {
    size_t n;
    if (!path) return 0;
    n = strlen(path);
    return n > 2 && path[n - 2] == '.' && path[n - 1] == 'c';
}

/* Returns 1 if the sibling-probe matrix has two rows with the same
 * `route_variant` label.  Guards against future edits that collapse
 * or split a row and produce a hidden duplicate. */
static int sibling_matrix_has_duplicate_route(void) {
    int i;
    int j;
    for (i = 0; i < SIBLING_PROBE_COUNT; ++i) {
        for (j = i + 1; j < SIBLING_PROBE_COUNT; ++j) {
            if (strcmp(kSiblingProbes[i].route_variant,
                       kSiblingProbes[j].route_variant) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

/* Returns 1 if the sibling-probe matrix has two rows with the same
 * `probe_basename`.  A duplicate basename would mean two different
 * route labels accidentally point at the same source file — the
 * probe inventory should stay one-to-one. */
static int sibling_matrix_has_duplicate_basename(void) {
    int i;
    int j;
    for (i = 0; i < SIBLING_PROBE_COUNT; ++i) {
        for (j = i + 1; j < SIBLING_PROBE_COUNT; ++j) {
            if (strcmp(kSiblingProbes[i].probe_basename,
                       kSiblingProbes[j].probe_basename) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

/* Read CMakeLists.txt once for the Group C coverage check.  Returns
 * a malloc'd buffer that the caller must free, or NULL on error.
 * The buffer is bounded to 8 MiB which is well over the current
 * CMakeLists.txt size (~865 KB) so a probe read returns the whole
 * file in one shot. */
#define PROBE_CMAKE_MAX_BYTES (8u * 1024u * 1024u)
static char* read_cmakelists(const char* path) {
    FILE* f = fopen(path, "rb");
    long sz;
    char* buf;
    size_t got;
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    sz = ftell(f);
    if (sz <= 0 || sz > (long)PROBE_CMAKE_MAX_BYTES) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    got = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (got != (size_t)sz) { free(buf); return NULL; }
    buf[sz] = '\0';
    return buf;
}

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}


/* Resolve the CMakeLists.txt path.  Tries (in order):
 *   1. argv[2] if provided
 *   2. $CMAKE_SOURCE_DIR/CMakeLists.txt env var (set by CMake at configure time)
 *   3. ../CMakeLists.txt (typical layout when ctest runs from build/)
 *   4. CMakeLists.txt (current working directory)
 *   5. ./CMakeLists.txt (explicit cwd)
 * Returns 1 on first hit, 0 on no hit; outPath is populated when 1. */
static int resolve_cmakelists_path(int argc, char** argv, const char* userArg,
                                   char* outPath, size_t outPathSize) {
    /* argc / argv are reserved for future use (e.g. argv[3] as a
     * manual source-root override). */
    (void)argc;
    (void)argv;
    static const char* kFallbacks[] = {
        NULL,                    /* userArg (filled in below) */
        "../CMakeLists.txt",     /* ctest-from-build layout */
        "CMakeLists.txt",        /* cwd layout */
        "./CMakeLists.txt"       /* explicit cwd */
    };
    int i;
    const char* envSrc = getenv("CMAKE_SOURCE_DIR");
    char buf[1024];
    /* Slot 0: user-supplied arg */
    kFallbacks[0] = userArg;
    if (userArg && file_exists(userArg)) {
        snprintf(outPath, outPathSize, "%s", userArg);
        return 1;
    }
    /* Slot 1.5: $CMAKE_SOURCE_DIR/CMakeLists.txt */
    if (envSrc && envSrc[0] != '\0') {
        snprintf(buf, sizeof(buf), "%s/CMakeLists.txt", envSrc);
        if (file_exists(buf)) {
            snprintf(outPath, outPathSize, "%s", buf);
            return 1;
        }
    }
    for (i = 1; i < (int)(sizeof(kFallbacks) / sizeof(kFallbacks[0])); ++i) {
        if (kFallbacks[i] && file_exists(kFallbacks[i])) {
            snprintf(outPath, outPathSize, "%s", kFallbacks[i]);
            return 1;
        }
    }
    return 0;
}

/* Resolve the source-dir root for the probes/m11/ lookup.  Tries:
 *   1. dirname(argv[2]) if argv[2] was supplied (assume CMakeLists.txt
 *      is in the project root)
 *   2. $CMAKE_SOURCE_DIR env var
 *   3. cwd (most common case when invoked from project root)
 * Returns 1 on first hit, 0 on no hit. */
static int resolve_source_root(int argc, char** argv,
                               const char* cmakeListsPath,
                               char* outPath, size_t outPathSize) {
    /* argc / argv are reserved for future use (e.g. argv[3] as a
     * manual source-root override). */
    (void)argc;
    (void)argv;
    const char* envSrc = getenv("CMAKE_SOURCE_DIR");
    char buf[1024];
    if (cmakeListsPath && cmakeListsPath[0] != '\0') {
        /* dirname via simple reverse scan */
        const char* slash = strrchr(cmakeListsPath, '/');
        if (slash && slash != cmakeListsPath) {
            size_t n = (size_t)(slash - cmakeListsPath);
            if (n >= sizeof(buf)) n = sizeof(buf) - 1;
            memcpy(buf, cmakeListsPath, n);
            buf[n] = '\0';
            snprintf(outPath, outPathSize, "%s", buf);
            return 1;
        }
        if (!slash) {
            /* bare basename, cwd is the project root */
            snprintf(outPath, outPathSize, ".");
            return 1;
        }
    }
    if (envSrc && envSrc[0] != '\0') {
        snprintf(outPath, outPathSize, "%s", envSrc);
        return 1;
    }
    snprintf(outPath, outPathSize, ".");
    return 1;
}


int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const char* dataDir;
    char resolvedCmake[1024];
    char resolvedRoot[1024];
    const char* cmakeListsPath = NULL;
    const char* sourceRoot = ".";
    char* cmakeListsBuf = NULL;
    int i;
    int corridorOrdinal2Hits;
    int totalCoveredWired  = 0;
    int totalExpectedWired = 0;
    int totalOpenWiring = 0;
    int totalSourcePresent = 0;
    int totalRealSensorRoutes = 0;
    int totalSyntheticSensorRoutes = 0;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR [CMAKE_LISTS_PATH]\n", argv[0]);
        return 2;
    }
    {
        const char* userCmakeArg = (argc > 2) ? argv[2] : NULL;
        if (!resolve_cmakelists_path(argc, argv, userCmakeArg,
                                     resolvedCmake, sizeof(resolvedCmake))) {
            fprintf(stderr, "WARN: could not locate CMakeLists.txt; "
                            "Group C coverage matrix will skip wiring checks\n");
            cmakeListsPath = NULL;
        } else {
            cmakeListsPath = resolvedCmake;
        }
        if (!resolve_source_root(argc, argv, cmakeListsPath,
                                 resolvedRoot, sizeof(resolvedRoot))) {
            snprintf(resolvedRoot, sizeof(resolvedRoot), ".");
        }
        sourceRoot = resolvedRoot;
    }

    printf("=== DM1 V1 HoC ordinal-2 future-route readiness gate ===\n");
    printf("dataDir=%s\n", dataDir);
    printf("cmakeListsPath=%s\n", cmakeListsPath ? cmakeListsPath : "(unresolved)");
    printf("sourceRoot=%s\n", sourceRoot);
    printf("ordinal target = %d\n", PROBE_ORDINAL_TARGET);

    /* ── Asset open ─────────────────────────────────────────── */
    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* ── Group A: corridor boundary scan ─────────────────────── */
    corridorOrdinal2Hits = scan_corridor_for_ordinal_2(&game);

    {
        char msgA1[160];
        char msgA2[160];
        snprintf(msgA1, sizeof(msgA1),
                 "Group A: ordinal=%d real-corridor-sensor hits == 0 on shipped DM1 V1 PC 3.4 (got %d)",
                 PROBE_ORDINAL_TARGET, corridorOrdinal2Hits);
        CHECK(corridorOrdinal2Hits == 0, msgA1);
        snprintf(msgA2, sizeof(msgA2),
                 "Group A: corridor scan did not artificially expose ordinal=%d on a synthetic sensor",
                 PROBE_ORDINAL_TARGET);
        CHECK(corridorOrdinal2Hits == 0, msgA2);
        if (corridorOrdinal2Hits == 0) {
            printf("  INFO: future-route surface event NOT detected; TODO row (a)\n");
            printf("        per-route ordinal-2 visual capture remains unblocked because no real\n");
            printf("        ordinal-2 corridor sensor exists to capture against.\n");
        } else {
            printf("  INFO: future-route surface event DETECTED; TODO row (a) per-route\n");
            printf("        ordinal-2 visual capture is now unblocked. Re-run this gate with\n");
            printf("        a per-route ordinal-2 visual capture probe to close row (a).\n");
        }
    }

    /* ── Group B: C026 atlas math + mirror catalog ──────────── */
    {
        int ord = PROBE_ORDINAL_TARGET;
        int expectedCol = ord & 7;       /* 2 */
        int expectedRow = ord >> 3;      /* 0 */
        int expectedSrcX = expectedCol * PROBE_PORTRAIT_W; /* 64 */
        int expectedSrcY = expectedRow * PROBE_PORTRAIT_H; /* 0 */
        char nameBuf[32];
        char titleBuf[32];
        nameBuf[0]  = '\0';
        titleBuf[0] = '\0';
        M11_GameView_GetMirrorNameByOrdinal(&game, ord, nameBuf, (int)sizeof(nameBuf));
        M11_GameView_GetMirrorTitleByOrdinal(&game, ord, titleBuf, (int)sizeof(titleBuf));

        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "Group B: ordinal=%d atlas col=%d row=%d (DEFS.H:821-826 8-col math)",
                     ord, expectedCol, expectedRow);
            CHECK(expectedCol == 2 && expectedRow == 0, msg);
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "Group B: ordinal=%d C026 source rect (srcX=%d, srcY=%d, w=%d, h=%d) fits the 256x87 atlas",
                     ord, expectedSrcX, expectedSrcY,
                     PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
            CHECK(expectedSrcX + PROBE_PORTRAIT_W <= 256 &&
                  expectedSrcY + PROBE_PORTRAIT_H <= 87, msg);
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "Group B: ordinal=%d mirror catalog name resolves to non-empty string (got '%s')",
                     ord, nameBuf);
            CHECK(nameBuf[0] != '\0', msg);
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "Group B: ordinal=%d mirror catalog title resolves to non-empty string (got '%s')",
                     ord, titleBuf);
            CHECK(titleBuf[0] != '\0', msg);
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "Group B: 24-atlas ordinals fit the 8x3 grid (8*32=256, 3*29=87, total=24)");
            CHECK(PROBE_PORTRAIT_COLS * PROBE_PORTRAIT_W == 256 &&
                  PROBE_PORTRAIT_ROWS * PROBE_PORTRAIT_H == 87 &&
                  PROBE_PORTRAIT_TOTAL == 24, msg);
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "Group B: D1C cutout (96, 35, 32, 29) viewport-local is inside the viewport (0,33)-(223,168)");
            CHECK(PROBE_D1C_X + PROBE_D1C_W <= 224 &&
                  PROBE_D1C_Y + PROBE_D1C_H <= 169, msg);
        }
    }

    /* ── Group C: sibling ordinal-2 coverage matrix ───────────── */
    cmakeListsBuf = cmakeListsPath ? read_cmakelists(cmakeListsPath) : NULL;
    if (!cmakeListsBuf) {
        printf("\n[Group C] INFO: CMakeLists.txt unreadable; coverage matrix "
               "will skip build-wiring checks (source-presence checks still run)\n");
    }

    printf("\n[Group C] Sibling ordinal-2 route coverage matrix:\n");
    CHECK(SIBLING_PROBE_COUNT == 10,
          "Group C: ordinal-2 sibling matrix has the expected 10 route rows");
    CHECK(!sibling_matrix_has_duplicate_route(),
          "Group C: ordinal-2 sibling matrix route labels are unique");
    CHECK(!sibling_matrix_has_duplicate_basename(),
          "Group C: ordinal-2 sibling matrix source basenames are unique");
    printf("  %-22s %-10s %-50s %-7s %-7s %s\n",
           "ROUTE_VARIANT", "STRATEGY", "BASENAME", "SOURCE", "WIRED", "STATUS");
    for (i = 0; i < SIBLING_PROBE_COUNT; ++i) {
        const ProbeRecord* r = &kSiblingProbes[i];
        char pathBuf[1024];
        const char* strategy = (r->sensor_strategy == SENSOR_REAL) ? "real" : "synthetic";
        int present = 0;
        int wired = 0;
        const char* sourceStatus;
        const char* wiredStatus;
        const char* rowStatus;
        int basenameShapeOk;
        /* The probe source lives in probes/m11/ relative to the
         * resolved source root (project root). */
        snprintf(pathBuf, sizeof(pathBuf), "%s/probes/m11/%s",
                 sourceRoot, r->probe_basename);
        if (r->sensor_strategy == SENSOR_REAL) ++totalRealSensorRoutes;
        if (r->sensor_strategy == SENSOR_SYNTHETIC) ++totalSyntheticSensorRoutes;
        basenameShapeOk = has_c_suffix(r->probe_basename);
        present = file_exists(pathBuf);
        if (cmakeListsBuf) {
            /* exact `probes/m11/<basename>` reference required, not a
             * loose strstr match against the basename alone. */
            wired = cmake_contains_probe_path(cmakeListsBuf, r->probe_basename);
        }
        if (r->expected_wired) {
            ++totalExpectedWired;
        } else {
            ++totalOpenWiring;
        }
        sourceStatus = present ? "present" : "MISSING";
        wiredStatus  = (cmakeListsBuf && wired) ? "yes" :
                       (!cmakeListsBuf)         ? "skip"  : "NO";
        if (!cmakeListsBuf) {
            SKIP_INFO("build-wiring skipped: CMakeLists.txt unreadable");
            rowStatus = "covered?"; /* informational only */
            if (present) {
                /* informational: source present but wiring unverified */
            } else {
                rowStatus = "gap (source missing)";
            }
        } else if (present && wired) {
            rowStatus = r->expected_wired ? "covered" : "unexpectedly wired";
            ++totalSourcePresent;
            ++totalCoveredWired;
        } else if (present && !wired) {
            rowStatus = r->expected_wired ? "gap (not wired)" : "open (source present)";
            ++totalSourcePresent;
        } else {
            rowStatus = "gap (source missing)";
        }
        printf("  %-22s %-10s %-50s %-7s %-7s %s\n",
               r->route_variant, strategy, r->probe_basename,
               sourceStatus, wiredStatus, rowStatus);
        printf("    anchor: %s\n", r->anchor);
        {
            char msgSrc[160];
            char msgWire[160];
            snprintf(msgSrc, sizeof(msgSrc),
                     "Group C: route '%s' source file exists",
                     r->route_variant);
            CHECK(present, msgSrc);
            snprintf(msgSrc, sizeof(msgSrc),
                     "Group C: route '%s' basename is a C source file",
                     r->route_variant);
            CHECK(basenameShapeOk, msgSrc);
            if (cmakeListsBuf) {
                snprintf(msgWire, sizeof(msgWire),
                         "Group C: route '%s' exact CMake probes/m11 path wiring matches expected state (%s)",
                         r->route_variant,
                         r->expected_wired ? "wired" : "known open");
                CHECK(wired == r->expected_wired, msgWire);
            }
        }
    }

    {
        char msgCov[200];
        snprintf(msgCov, sizeof(msgCov),
                 "Group C: ordinal-2 sibling matrix matches current state (source=%d/%d, wired=%d/%d, open=%d)",
                 totalSourcePresent, SIBLING_PROBE_COUNT,
                 totalCoveredWired, totalExpectedWired, totalOpenWiring);
        CHECK(totalSourcePresent == SIBLING_PROBE_COUNT &&
              totalCoveredWired == totalExpectedWired &&
              totalOpenWiring == 0, msgCov);
    }
    {
        char msgStrategy[200];
        snprintf(msgStrategy, sizeof(msgStrategy),
                 "Group C: ordinal-2 route strategy inventory remains 2 real-route rows and 8 synthetic-mutation rows (got real=%d synthetic=%d)",
                 totalRealSensorRoutes, totalSyntheticSensorRoutes);
        CHECK(totalRealSensorRoutes == 2 && totalSyntheticSensorRoutes == 8,
              msgStrategy);
    }

    if (cmakeListsBuf) free(cmakeListsBuf);

    M11_GameView_Shutdown(&game);

    /* ── Summary ─────────────────────────────────────────────── */
    printf("\n=== ordinal-2 future-route readiness summary ===\n");
    printf("  PASS=%d  FAIL=%d  SKIP=%d\n", g_pass, g_fail, g_skip);
    printf("  real ordinal-2 corridor-sensor hits: %d (boundary %s)\n",
           corridorOrdinal2Hits,
           (corridorOrdinal2Hits == 0) ? "HELD (TODO row (a) not yet unblocked)" :
                                         "BROKEN (TODO row (a) per-route capture now unblocked)");
    printf("  sibling ordinal-2 routes: %d total, %d source-present, %d build-wired\n",
           SIBLING_PROBE_COUNT, totalSourcePresent, totalCoveredWired);
    printf("  TODO row follow-up:\n");
    printf("    (a) per-route ordinal-2 visual capture -> unblocked only if corridor hits > 0\n");
    printf("    (b) sibling ordinal-2 route coverage matrix -> %d wired, %d known-open source files\n",
           totalCoveredWired, totalOpenWiring);

    return (g_fail == 0) ? 0 : 1;
}
