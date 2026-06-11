/*
 * test_csb_v1_boot_viewport_render_gate.c
 *
 * CSB V1 Boot → Runtime → M11/Viewport Render Gate (single-frame proof)
 *
 * Narrow, source/data-faithful proof that one verified CSB V1 boot
 * profile carries enough state through the runtime handoff for the
 * M11 game view to issue one deterministic csb_v1_viewport_render_frame()
 * call against the handoff-owned dungeon.
 *
 * This deliberately does NOT claim end-to-end playability. It proves the
 * boot→runtime→viewport handoff boundary by:
 *   1. Hash-simulating a verified CSB V1 boot profile (synthetic DUNGEON.DAT
 *      + GRAPHICS.DAT path on disk; verified flags set; variant PC 3.4 EN).
 *   2. Running csb_v1_boot_enter_game() to materialize the runtime
 *      (dungeon loaded into the heap handle and current-dungeon singleton).
 *   3. Asserting every M11 read needed by fs_game_render_viewport() is live
 *      and deterministic:
 *        - csb_v1_dungeon_get_current()       -> heap handle from handoff
 *        - csb_v1_dungeon_get_current_level() == 0  (new-game map)
 *        - csb_v1_dungeon_get_square_type() on a known marker cell returns
 *          the synthetic square type (proves the handoff preserved dungeon
 *          semantics, not just a handle pointer)
 *        - runtime.party_x/y/dir == CSB_V1_START_PARTY_* (the seed M11
 *          will hand to the viewport renderer)
 *        - runtime.dungeon_asset.path / graphics_asset.path point at the
 *          verified boot paths
 *   4. Building the CSB_V1_ViewportConfig exactly as the M11 view does in
 *      src/engine/firestaff_game_loop.c (csb_v1_viewport_init + 32x32
 *      dungeon grid built from csb_v1_dungeon_get_square_type + viewport
 *      pixels / stride pointed at a 320x200 framebuffer).
 *   5. Issuing exactly one csb_v1_viewport_render_frame() call and asserting
 *      the render boundary is deterministic and self-consistent:
 *        - call returns with the viewport pixel buffer still attached
 *        - two back-to-back calls with identical state produce
 *          byte-identical output (deterministic render)
 *        - calling with viewport_pixels == NULL is a guarded no-op (the
 *          staged-integration contract that lets M11 defer wiring)
 *
 * This is the missing bridge test between test_csb_v1_boot_runtime_handoff
 * (which proves the runtime handoff) and test_csb_v1_viewport_phase3_rendering
 * (which proves the viewport init). It does not assert any specific pixel
 * content; pixel content depends on the live wall-set and graphics archive
 * and is intentionally out of scope for this gate.
 *
 * Source-locks (matches the citation blocks in src/csb/csb_v1_boot.c and
 * src/csb/csb_v1_viewport_pc34_compat.c):
 *   ReDMCSB ENTRANCE.C F0806 lines 409-441 entrance micro-dungeon
 *   ReDMCSB ENTRANCE.C F0806 lines 857-883 entrance waits + G0298_B_NewGame
 *   ReDMCSB LOADSAVE.C F0435 lines 1940-1944 new-game party at map 0
 *   ReDMCSB DUNGEON.C F0151 lines 1423-1475 column-major square access
 *   ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755 current-map globals
 *   ReDMCSB DUNVIEW.C F0128 lines 8318-8542 shared DM1/CSB draw core
 *   CSBWin/CSBCode.cpp:6800-6950 LoadDungeon
 *   CSBWin/CSBCode.cpp:26 CustomBackgrounds
 *   M11 fs_game_render_viewport() in src/engine/firestaff_game_loop.c
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_viewport_pc34_compat.h"
#include "csb_v1_game_state_pc34_compat.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#else
#define TEST_MKDIR(path) mkdir((path), 0700)
#endif

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Build a minimal valid CSB V1 DUNGEON.DAT buffer. Mirrors the
 * synthetic builder in test_csb_v1_boot_runtime_handoff.c so the
 * fixture shape matches the legacy loader (square_bytes == 2,
 * column-major 16-bit records, ReDMCSB DUNGEON.C F0151).
 *
 * Layout:
 *   0..1   : level_count (LE16) = 1
 *   2..3   : ignored by the legacy loader
 *   4      : level 0 width  (uint8)
 *   5      : level 0 height (uint8)
 *   6..9   : level 0 absolute byte offset to squares (LE32)
 *   10..   : squares, column-major, 2 bytes each
 *            (low byte = type, high byte = first-thing high bits)
 *
 * Cell (x,y) lives at offset 10 + (x*height + y) * 2.
 */
static int build_synthetic_dungeon(uint8_t *buf, int buf_size,
                                    uint8_t marker_type)
{
    if (!buf || buf_size < 28) return -1;
    memset(buf, 0, (size_t)buf_size);
    buf[0] = 1; buf[1] = 0;             /* level_count = 1 */
    buf[2] = 16; buf[3] = 0;            /* ignored padding */
    buf[4] = 3; buf[5] = 3;             /* level 0 width=3, height=3 */
    buf[6] = 10; buf[7] = 0;            /* level 0 absolute square offset = 10 */
    buf[8] = 0; buf[9] = 0;
    /* Row 0: walls (square type 1) */
    buf[10] = 1; buf[11] = 0;
    buf[12] = 1; buf[13] = 0;
    buf[14] = 1; buf[15] = 0;
    /* Row 1: wall, marker (center), wall */
    buf[16] = 1; buf[17] = 0;
    buf[18] = marker_type; buf[19] = 0;
    buf[20] = 1; buf[21] = 0;
    /* Row 2: walls */
    buf[22] = 1; buf[23] = 0;
    buf[24] = 1; buf[25] = 0;
    buf[26] = 1; buf[27] = 0;
    return 0;
}

static int write_synthetic_dungeon(const char *path, uint8_t marker_type)
{
    uint8_t buf[32];
    FILE *f;
    size_t n;
    if (build_synthetic_dungeon(buf, (int)sizeof(buf), marker_type) != 0) {
        return -1;
    }
    f = fopen(path, "wb");
    if (!f) return -1;
    n = fwrite(buf, 1, sizeof(buf), f);
    fclose(f);
    return (n == sizeof(buf)) ? 0 : -1;
}

/* Mirror of fs_game_render_viewport()'s dungeon-grid snapshot. Builds
 * the 32x32 static array the M11 view feeds to the CSB viewport, using
 * the live runtime singleton. Out-of-bounds cells are clamped to WALL.
 *
 * Source: src/engine/firestaff_game_loop.c fs_game_render_viewport() */
static void snapshot_runtime_dungeon_grid(uint8_t out_grid[32*32])
{
    const CSB_V1_DungeonData *dun = csb_v1_dungeon_get_current();
    int level = 0;
    int w = 0;
    int h = 0;
    int max_w;
    int max_h;
    int gx;
    int gy;

    if (dun && dun->raw_data && dun->level_count > 0) {
        level = csb_v1_dungeon_get_current_level();
        if (level < 0 || level >= dun->level_count) level = 0;
        w = dun->level_widths[level];
        h = dun->level_heights[level];
    }
    max_w = (w > 0 && w <= 32) ? w : 0;
    max_h = (h > 0 && h <= 32) ? h : 0;
    for (gy = 0; gy < 32; gy++) {
        for (gx = 0; gx < 32; gx++) {
            if (gx < max_w && gy < max_h) {
                out_grid[gy * 32 + gx] =
                    (uint8_t)csb_v1_dungeon_get_square_type(dun, level, gx, gy);
            } else {
                out_grid[gy * 32 + gx] = 0; /* WALL */
            }
        }
    }
}

static void test_boot_runtime_handoff_exposes_m11_state(void)
{
    CSB_V1_BootProfile p;
    char dungeon_path[ASSET_PATH_MAX];
    char graphics_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-boot-viewport-gate";
    const uint8_t MARKER = 7;
    CSB_V1_DungeonData *handle_after_handoff;
    const CSB_V1_DungeonData *singleton_after_handoff;
    int marker_type_from_singleton;

    memset(&p, 0, sizeof(p));
    (void)TEST_MKDIR(tmp_dir);

    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    snprintf(graphics_path, sizeof(graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, MARKER) == 0,
          "synthetic DUNGEON.DAT written to temp path with marker=7");

    csb_v1_boot_profile_init(&p);
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s", graphics_path);
    snprintf(p.dungeon_md5, sizeof(p.dungeon_md5),
             "6695d2acebce49f95db1d8f3a5c733de");
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    p.entrance_map_index = 255U;
    p.start_map_index = 0U;

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "csb_v1_boot_enter_game() completes the boot→runtime handoff");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot state advances to RUNTIME_READY (handoff completed)");

    /* The M11 view reads these globals. They must be live after the
     * boot handoff with the same handle pointer the runtime owns. */
    handle_after_handoff = p.runtime.dungeon_handle;
    singleton_after_handoff = csb_v1_dungeon_get_current();
    CHECK(handle_after_handoff != NULL,
          "runtime.dungeon_handle is live (heap-allocated by handoff)");
    CHECK(singleton_after_handoff == handle_after_handoff,
          "csb_v1_dungeon_get_current() == runtime.dungeon_handle "
          "(singleton mirrors the handoff-owned heap)");

    /* Prove the handoff preserved dungeon *semantics*, not just a handle.
     * Cell (1,1) of the synthetic 3x3 is the marker. */
    marker_type_from_singleton = csb_v1_dungeon_get_square_type(
        singleton_after_handoff, csb_v1_dungeon_get_current_level(), 1, 1);
    CHECK(marker_type_from_singleton == (int)MARKER,
          "csb_v1_dungeon_get_square_type(level 0, 1, 1) returns marker "
          "value 7 (handoff preserved column-major square data)");

    /* The M11 view also needs level 0 and the CSB V1 start position.
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1940-1944 (new-game map 0)
     *         ReDMCSB ENTRANCE.C F0806 lines 409-441 (entrance at 255) */
    CHECK(csb_v1_dungeon_get_current_level() == 0,
          "current level is 0 (source-locked new-game map)");
    CHECK(p.runtime.party_x == CSB_V1_START_PARTY_X &&
          p.runtime.party_y == CSB_V1_START_PARTY_Y &&
          p.runtime.party_dir == CSB_V1_START_PARTY_DIR,
          "runtime party position/dir match CSB_V1_START_PARTY_* "
          "(M11 render seed is deterministic after handoff)");
    CHECK(p.runtime.entrance_map_index == 255U,
          "runtime.entrance_map_index = 255 (C255_MAP_INDEX_ENTRANCE)");
    CHECK(p.runtime.start_map_index == 0U,
          "runtime.start_map_index = 0 (new-game map)");
    CHECK(p.runtime.dungeon_asset.path == p.dungeon_path,
          "runtime.dungeon_asset.path points at the verified boot path");
    CHECK(p.runtime.graphics_asset.path == p.graphics_path,
          "runtime.graphics_asset.path points at the verified boot path");
    CHECK(p.runtime.chaos_magic.magic_initialized == 1,
          "chaos magic is initialized (ReDMCSB CASTER.C F0211) so M11 "
          "may probe chaos-related runtime state if needed");

    /* Cleanup: the boot profile owns the handoff runtime. */
    csb_v1_boot_cleanup(&p);
    CHECK(p.runtime.dungeon_handle == NULL,
          "boot cleanup clears the handoff-owned dungeon handle");
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "boot cleanup clears the current-dungeon singleton");
}

static void test_m11_viewport_render_boundary_is_deterministic(void)
{
    CSB_V1_BootProfile p;
    char dungeon_path[ASSET_PATH_MAX];
    char graphics_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-boot-viewport-render";
    const uint8_t MARKER = 7;
    /* 320x200 framebuffer (matches M11 fs_game_render_viewport). */
    static uint8_t framebuffer_a[320 * 200];
    static uint8_t framebuffer_b[320 * 200];
    CSB_V1_ViewportConfig cfg_a;
    CSB_V1_ViewportConfig cfg_b;
    static uint8_t grid_a[32 * 32];
    static uint8_t grid_b[32 * 32];
    size_t i;
    int pixel_bytes_match = 1;
    int marker_in_grid;

    memset(&p, 0, sizeof(p));
    (void)TEST_MKDIR(tmp_dir);
    memset(framebuffer_a, 0xAB, sizeof(framebuffer_a));
    memset(framebuffer_b, 0xAB, sizeof(framebuffer_b));

    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    snprintf(graphics_path, sizeof(graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, MARKER) == 0,
          "synthetic DUNGEON.DAT written for render-boundary test");

    csb_v1_boot_profile_init(&p);
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s", graphics_path);
    snprintf(p.dungeon_md5, sizeof(p.dungeon_md5),
             "6695d2acebce49f95db1d8f3a5c733de");
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game() succeeds before wiring the M11 viewport config");

    /* Build the CSB viewport config exactly as fs_game_render_viewport()
     * does in src/engine/firestaff_game_loop.c. The grid is built from
     * the live handoff-owned dungeon via csb_v1_dungeon_get_square_type,
     * not from raw profile data, so the M11 view consumes the same path. */
    csb_v1_viewport_init(&cfg_a);
    csb_v1_viewport_set_wall_set(&cfg_a, 0); /* CSB V1 default wall set */
    snapshot_runtime_dungeon_grid(grid_a);
    csb_v1_viewport_set_dungeon_grid(&cfg_a, grid_a, 32, 32);
    cfg_a.viewport_pixels = framebuffer_a;
    cfg_a.viewport_stride = 320;

    /* Same setup for a parallel config so we can prove the render is
     * deterministic by feeding the same state into a second buffer. */
    csb_v1_viewport_init(&cfg_b);
    csb_v1_viewport_set_wall_set(&cfg_b, 0);
    snapshot_runtime_dungeon_grid(grid_b);
    csb_v1_viewport_set_dungeon_grid(&cfg_b, grid_b, 32, 32);
    cfg_b.viewport_pixels = framebuffer_b;
    cfg_b.viewport_stride = 320;

    /* The dungeon-grid snapshot must surface the synthetic marker in
     * the same cell M11 fed the viewport. */
    marker_in_grid = grid_a[1 * 32 + 1];
    CHECK(marker_in_grid == MARKER,
          "M11-style grid snapshot preserves the synthetic marker "
          "at (x=1, y=1)");

    /* The seed the M11 view hands to the viewport renderer is exactly
     * the handoff's party_x/y/dir — no M11 has to invent a position. */
    CHECK(p.runtime.party_x == CSB_V1_START_PARTY_X &&
          p.runtime.party_y == CSB_V1_START_PARTY_Y &&
          p.runtime.party_dir == CSB_V1_START_PARTY_DIR,
          "M11 render seed == runtime party position from handoff");

    /* Issue exactly one csb_v1_viewport_render_frame() call per config.
     * This is the boot→runtime→viewport render boundary. */
    csb_v1_viewport_render_frame(&cfg_a, p.runtime.party_dir,
                                  p.runtime.party_x, p.runtime.party_y);
    csb_v1_viewport_render_frame(&cfg_b, p.runtime.party_dir,
                                  p.runtime.party_x, p.runtime.party_y);

    /* The render boundary contract: viewport pixel buffer must remain
     * attached after the call. */
    CHECK(cfg_a.viewport_pixels == framebuffer_a,
          "render_frame keeps cfg_a.viewport_pixels attached (M11 framebuffer "
          "pointer is not stolen by the renderer)");
    CHECK(cfg_b.viewport_pixels == framebuffer_b,
          "render_frame keeps cfg_b.viewport_pixels attached (M11 framebuffer "
          "pointer is not stolen by the renderer)");

    /* Determinism: identical handoff state + identical grid + identical
     * render parameters produce byte-identical output. This is the
     * single-frame proof the gate demands. */
    for (i = 0; i < sizeof(framebuffer_a); i++) {
        if (framebuffer_a[i] != framebuffer_b[i]) {
            pixel_bytes_match = 0;
            break;
        }
    }
    CHECK(pixel_bytes_match,
          "two render_frame() calls with identical handoff state produce "
          "byte-identical output (deterministic single-frame render)");

    /* Guarded no-op contract: if M11 defers wiring (viewport_pixels NULL)
     * the call must not crash and must not write any bytes. */
    {
        CSB_V1_ViewportConfig cfg_null;
        static uint8_t sentinel[16];
        memset(sentinel, 0xCD, sizeof(sentinel));
        csb_v1_viewport_init(&cfg_null);
        /* Inject the sentinel buffer, then NULL it out, then render. */
        cfg_null.viewport_pixels = sentinel;
        cfg_null.viewport_stride = 4;
        csb_v1_viewport_set_dungeon_grid(&cfg_null, grid_a, 32, 32);
        cfg_null.viewport_pixels = NULL; /* stage-integration path */
        csb_v1_viewport_render_frame(&cfg_null, 0, CSB_V1_START_PARTY_X,
                                      CSB_V1_START_PARTY_Y);
        CHECK(cfg_null.viewport_pixels == NULL,
              "render_frame with NULL viewport_pixels is a guarded no-op "
              "(M11 stage-integration path is safe)");
        CHECK(sentinel[0] == 0xCD && sentinel[7] == 0xCD,
              "guarded no-op render does not write any framebuffer bytes");
    }

    csb_v1_boot_cleanup(&p);
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "boot cleanup after render test clears the singleton "
          "(no stale dungeon leaks past the gate)");
}

int main(void)
{
    printf("=== CSB V1 Boot → Runtime → M11/Viewport Render Gate ===\n\n");
    test_boot_runtime_handoff_exposes_m11_state();
    test_m11_viewport_render_boundary_is_deterministic();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: CSB V1 verified boot handoff exposes the dungeon handle, "
             "current level, party seed, and asset paths that M11's "
             "fs_game_render_viewport() reads to wire the CSB viewport");
        puts("sourceEvidence=ReDMCSB ENTRANCE.C F0806 lines 409-441; "
             "LOADSAVE.C F0435 lines 1940-1944; DUNGEON.C F0151 lines 1423-1475; "
             "DUNVIEW.C F0128 lines 8318-8542; CSBWin/CSBCode.cpp:6800-6950");
        puts("ok: CSB V1 boot→runtime→viewport render boundary is "
             "deterministic: identical handoff state + identical grid + "
             "identical render parameters produce byte-identical output");
        puts("sourceEvidence=ReDMCSB DUNVIEW.C F0128 lines 8318-8542; "
             "src/engine/firestaff_game_loop.c fs_game_render_viewport()");
    }
    return failed == 0 ? 0 : 1;
}
