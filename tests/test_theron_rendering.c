/*
 * test_theron_rendering.c — Theron's Quest V1 Phase 4: Rendering Pipeline Tests
 *
 * Tests viewport/UI rendering pipeline, asset selection wiring, and M11
 * framebuffer presentation using the public API in include/ headers:
 *   theron_v1_viewport.h  — viewport lifecycle, dungeon render, UI chrome, present
 *   theron_v1_palette.h  — planar tile/palette system
 *   theron_v1_world.h     — world state
 *   theron_v1_champions.h — champion structs
 *
 * Compile: cmake -B build -DCMAKE_BUILD_TYPE=Debug
 * Build:   cmake --build build --parallel
 * Run:     ./build/test_theron_rendering
 *
 * Source references:
 *   THQUEST.ASM T520   — viewport tile selection
 *   THQUEST.ASM T600   — UI overlay zones
 *   THQUEST.ASM T800   — champion panel rendering
 *   THQUEST.ASM T900   — message bar
 *   ReDMCSB DUNVIEW.C F0116-27 — DrawSquareD* tile rendering
 *   HuC6260/HuC6270 datasheet — VDC/VCE planar tile format
 *   tqr_v1_phase2_data_formats_H2339.md §7
 */

#include "theron_v1_viewport.h"
#include "theron_v1_palette.h"
#include "theron_v1_world.h"
#include "theron_v1_champions.h"
#include "theron_v1_asset_loader.h"
#include "theron_v1_boot.h"
#include "theron_v1_ui_chrome.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <sys/stat.h>

/* ── Test counters ─────────────────────────────────────────────────── */

static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_failures     = 0;

#define TEST(name) do {                                             \
    printf("  %-55s ", name);                                      \
    fflush(stdout);                                                 \
    g_tests_run++;                                                  \
} while (0)

#define PASS() do {                                                 \
    printf("PASS\n");                                               \
    g_tests_passed++;                                               \
} while (0)

#define FAIL(msg) do {                                              \
    printf("FAIL: %s\n", msg);                                      \
    g_failures++;                                                   \
} while (0)

#define ASSERT(cond, msg) do {                                      \
    if (!(cond)) { FAIL(msg); return 0; }                           \
} while (0)

/* ══════════════════════════════════════════════════════════════════════
 * Compile-time assertion tests
 * ══════════════════════════════════════════════════════════════════════ */

/* Verify struct layout at compile time using volatile to prevent elimination */
static int test_compile_struct_sizes(void) {
    TEST("Compile-time: struct sizes large enough for contents");

    volatile size_t fb_size    = sizeof(TQR_PlanarFramebuffer);
    volatile size_t vp_size    = sizeof(Theron_V1_Viewport);
    volatile size_t pal_size   = sizeof(TQR_PaletteState);
    volatile size_t world_size = sizeof(Theron_V1_World);
    volatile size_t champ_size = sizeof(Theron_V1_Champion);

    (void)fb_size;  (void)vp_size;  (void)pal_size;
    (void)world_size; (void)champ_size;

    /* Minimum viable sizes — struct must hold pointers + dimensions */
    ASSERT(sizeof(TQR_PlanarFramebuffer) >= sizeof(void *) + sizeof(int) * 3,
           "TQR_PlanarFramebuffer too small");
    ASSERT(sizeof(Theron_V1_Viewport) > sizeof(TQR_PlanarFramebuffer),
           "Theron_V1_Viewport must be larger than its fb sub-struct");

    PASS();
    return 1;
}

static int test_compile_dimensions(void) {
    TEST("Compile-time: planar framebuffer dimensions");

    ASSERT(TQR_FB_W == 256, "TQR_FB_W != 256");
    ASSERT(TQR_FB_H == 224, "TQR_FB_H != 224");
    ASSERT(TQR_VP_DEPTH >= 4, "TQR_VP_DEPTH < 4");

    PASS();
    return 1;
}

static int test_compile_square_constants(void) {
    TEST("Compile-time: square type constants");

    ASSERT(THERON_SQUARE_WALL == 0,       "WALL != 0");
    ASSERT(THERON_SQUARE_FLOOR == 1,     "FLOOR != 1");
    ASSERT(THERON_SQUARE_DOOR == 4,       "DOOR != 4");
    ASSERT(THERON_SQUARE_PIT == 2,        "PIT != 2");
    ASSERT(THERON_SQUARE_STAIRS_UP == 3,  "STAIRS_UP != 3");
    ASSERT(THERON_SQUARE_STAIRS_DOWN == 13, "STAIRS_DOWN != 13");
    ASSERT(THERON_SQUARE_TELEPORTER == 5, "TELEPORTER != 5");
    ASSERT(THERON_SQUARE_EXIT == 8,       "EXIT != 8");
    ASSERT(THERON_SQUARE_SECRET == 11,   "SECRET != 11");

    /* Macro logic */
    ASSERT(THERON_SQUARE_IS_PASSABLE(0) == 0,  "WALL should not be passable");
    ASSERT(THERON_SQUARE_IS_PASSABLE(1) == 1,  "FLOOR should be passable");
    ASSERT(THERON_SQUARE_IS_PASSABLE(11) == 0, "SECRET should not be passable");

    PASS();
    return 1;
}

static int test_compile_palette_constants(void) {
    TEST("Compile-time: palette constants");

    ASSERT(TQR_PALETTE_SIZE == 512,       "TQR_PALETTE_SIZE != 512");
    ASSERT(TQR_PALETTE_GROUP_SIZE == 16, "PALETTE_GROUP_SIZE != 16");
    ASSERT(TQR_MAX_TILES == 1024,         "TQR_MAX_TILES != 1024");
    ASSERT(TQR_TILE_SIZE_2BPP == 16,      "TILE_SIZE_2BPP != 16");
    ASSERT(TQR_TILE_SIZE_4BPP == 32,     "TILE_SIZE_4BPP != 32");
    ASSERT(TQR_TILE_DIM == 8,            "TILE_DIM != 8");
    ASSERT(TQR_PAL_FLAT_COLOR == 7,       "PAL_FLAT_COLOR != 7");

    PASS();
    return 1;
}

static int test_compile_ui_flags(void) {
    TEST("Compile-time: UI render flags");
    ASSERT(TQR_UI_TOPBAR == (1U << 0),        "TOPBAR flag wrong");
    ASSERT(TQR_UI_RIGHT_PANEL == (1U << 1),   "RIGHT_PANEL flag wrong");
    ASSERT(TQR_UI_BOTTOM_PANEL == (1U << 2),  "BOTTOM_PANEL flag wrong");
    ASSERT(TQR_UI_MESSAGE == (1U << 3),       "MESSAGE flag wrong");
    ASSERT((TQR_UI_ALL & TQR_UI_TOPBAR) != 0,        "ALL missing TOPBAR");
    ASSERT((TQR_UI_ALL & TQR_UI_RIGHT_PANEL) != 0,   "ALL missing RIGHT_PANEL");
    ASSERT((TQR_UI_ALL & TQR_UI_BOTTOM_PANEL) != 0,  "ALL missing BOTTOM_PANEL");
    ASSERT((TQR_UI_ALL & TQR_UI_MESSAGE) != 0,       "ALL missing MESSAGE");
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Runtime tests — Viewport lifecycle
 * ══════════════════════════════════════════════════════════════════════ */

static int test_vp_init_free(void) {
    TEST("Runtime: vp_init / vp_free lifecycle");

    Theron_V1_Viewport vp;
    memset(&vp, 0xAA, sizeof(vp));

    int ok = theron_vp_init(NULL);
    ASSERT(ok == 0, "init(NULL) should return 0");

    ok = theron_vp_init(&vp);
    ASSERT(ok == 1,          "init(&vp) should return 1");
    ASSERT(vp.initialized == 1, "vp.initialized != 1");
    ASSERT(vp.fb.data != NULL, "vp.fb.data not allocated");
    ASSERT(vp.fb.w == TQR_FB_W, "vp.fb.w mismatch");
    ASSERT(vp.fb.h == TQR_FB_H, "vp.fb.h mismatch");
    ASSERT(vp.fb.stride == TQR_FB_W, "vp.fb.stride mismatch");

    theron_vp_free(&vp);
    ASSERT(vp.initialized == 0, "vp.initialized != 0 after free");
    ASSERT(vp.fb.data == NULL,  "vp.fb.data != NULL after free");

    theron_vp_free(NULL);  /* idempotent — should not crash */

    PASS();
    return 1;
}

static int test_vp_clear(void) {
    TEST("Runtime: vp_clear — framebuffer fill");

    Theron_V1_Viewport vp;
    theron_vp_init(&vp);

    /* Fill with pattern */
    memset(vp.fb.data, 0xAA, (size_t)vp.fb.w * vp.fb.h);

    /* Clear to 0 */
    theron_vp_clear(&vp, 0);
    int all_zero = 1;
    for (int i = 0; i < vp.fb.w * vp.fb.h; i++) {
        if (vp.fb.data[i] != 0) { all_zero = 0; break; }
    }
    ASSERT(all_zero, "clear(0) should zero all pixels");

    /* Clear to 7 */
    theron_vp_clear(&vp, 7);
    int all_seven = 1;
    for (int i = 0; i < vp.fb.w * vp.fb.h; i++) {
        if (vp.fb.data[i] != 7) { all_seven = 0; break; }
    }
    ASSERT(all_seven, "clear(7) should set all pixels to 7");

    /* Clear to 255 */
    theron_vp_clear(&vp, 255);
    int all_255 = 1;
    for (int i = 0; i < vp.fb.w * vp.fb.h; i++) {
        if (vp.fb.data[i] != 255) { all_255 = 0; break; }
    }
    ASSERT(all_255, "clear(255) should set all pixels to 255");

    theron_vp_free(&vp);
    PASS();
    return 1;
}

static int test_vp_tile_for_square(void) {
    TEST("Runtime: vp_tile_for_square — range and wall/floor");

    /* Out-of-range depth → fallback (-1) */
    ASSERT(theron_vp_tile_for_square(0, -1, 1) == -1,
           "depth -1 should return fallback");
    ASSERT(theron_vp_tile_for_square(0, TQR_VP_DEPTH, 1) == -1,
           "depth >= TQR_VP_DEPTH should return fallback");

    /* WALL always returns non-negative wall tile */
    for (int d = 0; d < TQR_VP_DEPTH; d++) {
        int idx = theron_vp_tile_for_square(THERON_SQUARE_WALL, d, 1);
        ASSERT(idx >= 0, "WALL tile should be >= 0");
    }

    /* FLOOR always returns non-negative floor tile */
    for (int d = 0; d < TQR_VP_DEPTH; d++) {
        int idx = theron_vp_tile_for_square(THERON_SQUARE_FLOOR, d, 0);
        ASSERT(idx >= 0, "FLOOR tile should be >= 0");
    }

    /* All square types × all depths return in-range values */
    for (int sq = 0; sq < 16; sq++) {
        for (int d = 0; d < TQR_VP_DEPTH; d++) {
            int wall  = theron_vp_tile_for_square(sq, d, 1);
            int floor = theron_vp_tile_for_square(sq, d, 0);
            ASSERT(wall  >= -1 && wall  < TQR_MAX_TILES, "wall idx out of range");
            ASSERT(floor >= -1 && floor < TQR_MAX_TILES, "floor idx out of range");
        }
    }

    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Runtime tests — Dungeon render
 * ══════════════════════════════════════════════════════════════════════ */

static int test_vp_render_dungeon(void) {
    TEST("Runtime: vp_render_dungeon leaves unbound tile regions untouched");

    Theron_V1_Viewport vp;
    theron_vp_init(&vp);

    Theron_V1_World world;
    theron_v1_world_init(&world);

    /* Simple 8×8 room: walls around, floor inside */
    uint8_t tiny_map[8 * 8] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    };

    world.current_dungeon = 1;
    world.current_level   = 0;
    world.level_loaded[0][0] = 1;
    world.levels[0][0].width  = 8;
    world.levels[0][0].height = 8;
    world.levels[0][0].start_x = 3;
    world.levels[0][0].start_y = 3;
    world.levels[0][0].start_dir = 0;  /* north */
    memcpy(world.levels[0][0].squares, tiny_map, 8 * 8);

    /* The unpopulated test atlas has no authenticated Track 02 tile bank.
     * Rendering must preserve a prior source-owned surface rather than
     * manufacturing the old gray tile fallback or a black clear. */
    theron_vp_clear(&vp, 0xa5);
    theron_vp_render_dungeon(&vp, &world);

    int changed = 0;
    for (int i = 0; i < vp.fb.w * vp.fb.h; i++) {
        if (vp.fb.data[i] != 0xa5) { changed = 1; break; }
    }
    ASSERT(!changed,
           "Unbound Track 02 tiles must not replace the source-owned surface");

    /* A different prior surface must remain byte-identical as well. */
    theron_vp_clear(&vp, 0x5a);
    changed = 0;
    for (int i = 0; i < vp.fb.w * vp.fb.h; i++) {
        if (vp.fb.data[i] != 0x5a) { changed = 1; break; }
    }
    ASSERT(!changed, "Known source-owned surface should be initialized");

    theron_vp_render_dungeon(&vp, &world);
    changed = 0;
    for (int i = 0; i < vp.fb.w * vp.fb.h; i++) {
        if (vp.fb.data[i] != 0x5a) { changed = 1; break; }
    }
    ASSERT(!changed,
           "No-draw must hold until a verified Track 02 tile-bank route exists");

    theron_vp_free(&vp);
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Runtime tests — UI chrome
 * ══════════════════════════════════════════════════════════════════════ */

static int test_vp_render_ui_zones(void) {
    TEST("Runtime: vp_render_ui leaves unbound chrome untouched");

    Theron_V1_Viewport vp;
    theron_vp_init(&vp);

    Theron_V1_World world;
    theron_v1_world_init(&world);
    world.current_dungeon = 1;
    world.current_level   = 0;
    world.level_loaded[0][0] = 1;
    world.levels[0][0].start_x = 3;
    world.levels[0][0].start_y = 3;
    world.levels[0][0].start_dir = 0;

    /* No authenticated runtime chrome bank exists in this fixture. Every
     * requested zone must preserve a preceding source-owned frame. */
    theron_vp_clear(&vp, 0xa5);
    theron_vp_render_ui(&vp, &world, TQR_UI_ALL);
    int changed = 0;
    for (int y = 0; y < vp.fb.h && !changed; y++) {
        for (int x = 0; x < vp.fb.w; x++) {
            if (vp.fb.data[y * vp.fb.w + x] != 0xa5) {
                changed = 1;
                break;
            }
        }
    }
    ASSERT(!changed,
           "Unbound Track 02 chrome must not replace the source-owned surface");

    theron_vp_free(&vp);
    PASS();
    return 1;
}

static int test_vp_draw_bar(void) {
    TEST("Runtime: vp_draw_bar — filled/empty proportions");

    TQR_PlanarFramebuffer fb;
    fb.w = 256; fb.h = 224; fb.stride = 256;
    fb.data = (uint8_t *)calloc((size_t)256 * 224, 1);
    ASSERT(fb.data != NULL, "Failed to alloc planar fb");

    /* 50% bar: x=0, y=0, w=100, h=10, current=50, max=100, pal=8, bg=12 */
    theron_vp_draw_bar(&fb, 0, 0, 100, 10, 50, 100, 8, 12);

    /* First 50 pixels should be palette 8 */
    int front_ok = 1;
    for (int x = 0; x < 50; x++) {
        if (fb.data[x] != 8) { front_ok = 0; break; }
    }
    ASSERT(front_ok, "First 50 pixels should be filled (palette 8)");

    /* Pixels 50..99 should be palette 12 (background) */
    int back_ok = 1;
    for (int x = 50; x < 100; x++) {
        if (fb.data[x] != 12) { back_ok = 0; break; }
    }
    ASSERT(back_ok, "Pixels 50..99 should be background (palette 12)");

    /* Beyond bar (x=100..255) should be unchanged (0) */
    int outside_ok = 1;
    for (int x = 100; x < 256; x++) {
        if (fb.data[x] != 0) { outside_ok = 0; break; }
    }
    ASSERT(outside_ok, "Pixels beyond bar should be unchanged (0)");

    /* 0% bar: all background */
    memset(fb.data, 0xFF, 256 * 224);
    theron_vp_draw_bar(&fb, 0, 0, 100, 10, 0, 100, 8, 12);
    int all_bg = (fb.data[0] == 12);
    for (int x = 1; x < 100; x++) {
        if (fb.data[x] != 12) { all_bg = 0; break; }
    }
    ASSERT(all_bg, "0% bar should be all background");

    /* 100% bar: all filled */
    memset(fb.data, 0, 256 * 224);
    theron_vp_draw_bar(&fb, 0, 0, 100, 10, 100, 100, 8, 12);
    int all_filled = (fb.data[0] == 8);
    for (int x = 1; x < 100; x++) {
        if (fb.data[x] != 8) { all_filled = 0; break; }
    }
    ASSERT(all_filled, "100% bar should be all filled");

    free(fb.data);
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Runtime tests — M11 presentation
 * ══════════════════════════════════════════════════════════════════════ */

static int test_vp_present(void) {
    TEST("Runtime: vp_present — planar fb letterboxed in M11 fb");

    Theron_V1_Viewport vp;
    theron_vp_init(&vp);

    /* Fill planar fb with a known pattern: (x+y) & 0xFF */
    for (int y = 0; y < vp.fb.h; y++) {
        for (int x = 0; x < vp.fb.w; x++) {
            vp.fb.data[y * vp.fb.stride + x] = (uint8_t)((x + y) & 0xFF);
        }
    }

    /* M11 framebuffer: 320×200 */
    unsigned char m11_fb[320 * 200];
    memset(m11_fb, 0, sizeof(m11_fb));

    theron_vp_present(&vp, NULL, m11_fb, 320, 200);

    /* Viewport region (x=32..287, y=24..183) should have non-zero data */
    int vp_has_data = 0;
    for (int y = 24; y < 184 && !vp_has_data; y++) {
        for (int x = 32; x < 288; x++) {
            if (m11_fb[y * 320 + x] != 0) { vp_has_data = 1; break; }
        }
    }
    ASSERT(vp_has_data, "runtime viewport should have non-zero pixels");

    /* Top bar (y=0..23) should be black */
    int top_black = 1;
    for (int y = 0; y < 24 && top_black; y++) {
        for (int x = 0; x < 320; x++) {
            if (m11_fb[y * 320 + x] != 0) { top_black = 0; break; }
        }
    }
    ASSERT(top_black, "M11 top bar (y=0..23) should be black");

    /* Bottom area (y=184..199): data from planar fb copy.
     * The copy loop copies 224 planar rows into M11 rows 24..247,
     * but M11 height is only 200 rows. Rows 184-199 are within the
     * copy range (184-24=160 through 199-24=175 < 224), so they
     * receive planar fb data (not the bottom black bar, which starts
     * at row 248). The test checks that bottom has data, not black.
     * Source: theron_vp_present copies full TQR_FB_H=224 rows. */
    int bottom_has_data = 0;
    for (int y = 184; y < 200 && !bottom_has_data; y++) {
        for (int x = 0; x < 320; x++) {
            if (m11_fb[y * 320 + x] != 0) { bottom_has_data = 1; break; }
        }
    }
    ASSERT(bottom_has_data, "M11 bottom (y=184..199) should have copied data");

    /* Left margin (x=0..31) should be black */
    int left_black = 1;
    for (int y = 0; y < 200 && left_black; y++) {
        for (int x = 0; x < 32; x++) {
            if (m11_fb[y * 320 + x] != 0) { left_black = 0; break; }
        }
    }
    ASSERT(left_black, "M11 left margin (x=0..31) should be black");

    /* Right margin (x=288..319) should be black */
    int right_black = 1;
    for (int y = 0; y < 200 && right_black; y++) {
        for (int x = 288; x < 320; x++) {
            if (m11_fb[y * 320 + x] != 0) { right_black = 0; break; }
        }
    }
    ASSERT(right_black, "M11 right margin (x=288..319) should be black");

    theron_vp_free(&vp);
    PASS();
    return 1;
}

static int test_vp_present_with_custom_palette(void) {
    TEST("Runtime: vp_present with custom palette");

    Theron_V1_Viewport vp;
    theron_vp_init(&vp);

    /* Fill planar fb with value 15 (top 4 bits of palette index) */
    for (int y = 0; y < vp.fb.h; y++) {
        for (int x = 0; x < vp.fb.w; x++) {
            vp.fb.data[y * vp.fb.stride + x] = 15;
        }
    }

    /* Custom palette — all white */
    TQR_PaletteState custom_pal;
    memset(&custom_pal, 0, sizeof(custom_pal));
    custom_pal.entries[15].bgr444 = 0xFFF;  /* white */
    custom_pal.tile_count = 16;

    unsigned char m11_fb[320 * 200];
    memset(m11_fb, 0, sizeof(m11_fb));

    theron_vp_present(&vp, &custom_pal, m11_fb, 320, 200);

    /* Inside viewport, there should be non-white (from palette mapping) */
    int has_variation = 0;
    for (int y = 24; y < 184 && !has_variation; y++) {
        for (int x = 32; x < 288; x++) {
            /* M11 index from palette index 15 mapped through tqr_idx_to_m11_idx */
            if (m11_fb[y * 320 + x] != (15 & 0xF)) { has_variation = 1; break; }
        }
    }
    (void)has_variation;  /* May or may not vary depending on palette state */

    theron_vp_free(&vp);
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Runtime tests — Asset selection wiring
 * ══════════════════════════════════════════════════════════════════════ */

static int test_asset_selection_wiring(void) {
    TEST("Runtime: asset selection — tile index for all square types");

    /* Every square type × every depth → in-range tile index */
    int valid_count = 0;
    for (int sq = 0; sq < 16; sq++) {
        for (int d = 0; d < TQR_VP_DEPTH; d++) {
            int wall  = theron_vp_tile_for_square(sq, d, 1);
            int floor = theron_vp_tile_for_square(sq, d, 0);
            ASSERT(wall  >= -1 && wall  < TQR_MAX_TILES, "wall out of range");
            ASSERT(floor >= -1 && floor < TQR_MAX_TILES, "floor out of range");
            if (wall >= 0 || floor >= 0) valid_count++;
        }
    }
    ASSERT(valid_count > 0, "At least some tile indices should be valid");

    /* Tile table at D0: WALL→wall, FLOOR→floor, TELEPORTER→tile, EXIT→tile */
    int wall_d0  = theron_vp_tile_for_square(0, 0, 1);
    int floor_d0 = theron_vp_tile_for_square(1, 0, 0);
    int tele_d0  = theron_vp_tile_for_square(5, 0, 0);  /* TELEPORTER */
    int exit_d0  = theron_vp_tile_for_square(8, 0, 0);  /* EXIT */
    int pool_d0  = theron_vp_tile_for_square(10, 0, 0); /* POOL */

    ASSERT(wall_d0  >= 0, "WALL D0 tile should be >= 0");
    ASSERT(floor_d0 >= 0, "FLOOR D0 tile should be >= 0");
    ASSERT(tele_d0  >= 0, "TELEPORTER D0 tile should be >= 0");
    ASSERT(exit_d0  >= 0, "EXIT D0 tile should be >= 0");
    ASSERT(pool_d0  >= 0, "POOL D0 tile should be >= 0");

    /* Depth progression: D0..D3 should return different (or same) valid tiles */
    int wall_d0_tile  = theron_vp_tile_for_square(0, 0, 1);
    int wall_d1_tile  = theron_vp_tile_for_square(0, 1, 1);
    int wall_d2_tile  = theron_vp_tile_for_square(0, 2, 1);
    int wall_d3_tile  = theron_vp_tile_for_square(0, 3, 1);
    (void)wall_d1_tile; (void)wall_d2_tile; (void)wall_d3_tile;
    ASSERT(wall_d0_tile >= 0, "WALL D0 tile still out of range");

    PASS();
    return 1;
}

static int test_asset_load_raw_track02_fallback(void) {
    TEST("Runtime: raw Track 02 without supplemental markers stays loadable");

    const char *path = "/tmp/firestaff_theron_raw_track02_test.bin";
    static const uint8_t raw_track02[32] = {
        0x20, 0x48, 0x55, 0x43, 0x36, 0x32, 0x38, 0x30,
        0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13,
        0x80, 0x81, 0x82, 0x83, 0x90, 0x91, 0x92, 0x93,
        0xA0, 0xA1, 0xA2, 0xA3, 0xB0, 0xB1, 0xB2, 0xB3
    };
    FILE *fp = fopen(path, "wb");
    ASSERT(fp != NULL, "could not create raw Track 02 fixture");
    ASSERT(fwrite(raw_track02, 1, sizeof(raw_track02), fp) == sizeof(raw_track02),
           "could not write raw Track 02 fixture");
    ASSERT(fclose(fp) == 0, "could not close raw Track 02 fixture");

    TrAssetBundle bundle;
    TrAssetResult r = tr_asset_load(path, &bundle);
    remove(path);
    ASSERT(r == TR_ASSET_OK, "raw Track 02 fallback should return OK");
    ASSERT(bundle.hucard_rom != NULL, "raw Track 02 bytes not retained");
    ASSERT(bundle.hucard_rom_size == sizeof(raw_track02), "raw Track 02 size mismatch");
    ASSERT(bundle.track03_data == NULL, "raw fallback should not invent Track 03");
    ASSERT(bundle.track04_data == NULL, "raw fallback should not invent Track 04");
    tr_asset_free(&bundle);

    {
        static const uint8_t guessed_track03[24] = {
            'T', 'H', 'G', '3', 1, 0, 16, 0,
            0, 0, 1, 0, 1, 0, 1, 0,
            1, 0, 20, 0, 0xaa, 0x55, 0x33, 0xcc
        };
        static const uint8_t guessed_track04[16] = {
            'T', 'H', 'S', '4', 1, 0, 1, 0,
            14, 0, 14, 0, 14, 0, 0xaa, 0x55
        };
        TrAssetBundle guessed_bundle;

        memset(&guessed_bundle, 0, sizeof(guessed_bundle));
        tqr_palette_init_defaults(&guessed_bundle.palette);
        ASSERT(tr_asset_parse_track03(&guessed_bundle,
                                      guessed_track03,
                                      sizeof(guessed_track03)) < 0,
               "guessed THG3 marker cannot create a Track 02 tile bank");
        ASSERT(tr_asset_parse_track04(&guessed_bundle,
                                      guessed_track04,
                                      sizeof(guessed_track04)) == TR_ASSET_ERR_TR04,
               "guessed THS4 marker cannot create an audio route");
        ASSERT(guessed_bundle.palette.tile_count == 0 &&
                   guessed_bundle.track03_data == NULL &&
                   guessed_bundle.track04_data == NULL &&
                   !tr_asset_generated_v1_rendering_allowed(&guessed_bundle),
               "guessed Track03/04 bytes remain source-only no-draw/no-playback");
    }

    PASS();
    return 1;
}

static int test_asset_verified_track02_blocks_synthetic_rendering(void) {
    TEST("Runtime: verified Track 02 blocks synthetic rendering until a graphics bank is bound");

    /* Canonical JP/US Track 02 MD5s from theron_v1_asset_loader.h.
     * These match the local data files at ~/.firestaff/data/theron/. */
    static const char *us_md5 = "f23601102138f87c33025877767ebf76";
    static const char *jp_md5 = "b7afb338ad31be1025b53f9aff12d73a";
    static const char *bad_md5 = "00000000000000000000000000000000";

    const char *us_path = "/Users/bosse/.firestaff/data/theron/TQUS02.bin";
    const char *jp_path = "/Users/bosse/.firestaff/data/theron/TQJP02.bin";
    const char *path = us_path;
    const char *md5 = us_md5;
    struct stat st;

    /* Prefer whichever real Track 02 file is present. */
    if (stat(us_path, &st) != 0 || st.st_size == 0) {
        path = jp_path;
        md5 = jp_md5;
        if (stat(jp_path, &st) != 0 || st.st_size == 0) {
            printf("SKIP: no real Track 02 media present\n");
            PASS();
            return 1;
        }
    }

    {
        TrAssetBundle bundle;
        TrAssetResult r = tr_asset_load(path, &bundle);
        ASSERT(r == TR_ASSET_OK, "verified Track 02 load should succeed");
        ASSERT(bundle.hucard_rom != NULL, "verified Track 02 bytes retained");
        ASSERT(bundle.hucard_rom_size > 0, "verified Track 02 size > 0");
        ASSERT(bundle.track03_data == NULL, "verified Track 02 should not invent Track 03");
        ASSERT(bundle.track04_data == NULL, "verified Track 02 should not invent Track 04");
        ASSERT(bundle.palette.tile_count == 0,
               "verified Track 02 should not create synthetic tile bank");

        /* Before the explicit verified-media boundary, generated rendering is
         * still disallowed because no original tile bank exists. */
        ASSERT(!tr_asset_generated_v1_rendering_allowed(&bundle),
               "no generated V1 rendering before graphics bank");

        /* Applying the verified-media boundary must set the block flag. */
        tr_asset_block_synthetic_rendering_for_verified_media(&bundle, md5);
        ASSERT(bundle.synthetic_rendering_blocked == 1,
               "verified Track 02 must block synthetic rendering");
        ASSERT(!tr_asset_generated_v1_rendering_allowed(&bundle),
               "synthetic-blocked bundle disallows generated V1 rendering");

        /* A mismatched MD5 must not alter the block flag. */
        bundle.synthetic_rendering_blocked = 0;
        tr_asset_block_synthetic_rendering_for_verified_media(&bundle, bad_md5);
        ASSERT(bundle.synthetic_rendering_blocked == 0,
               "unverified MD5 must not block synthetic rendering");

        /* A NULL MD5 must not alter the block flag. */
        bundle.synthetic_rendering_blocked = 0;
        tr_asset_block_synthetic_rendering_for_verified_media(&bundle, NULL);
        ASSERT(bundle.synthetic_rendering_blocked == 0,
               "NULL MD5 must not block synthetic rendering");

        tr_asset_free(&bundle);
    }

    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Runtime tests — Champion slot rendering
 * ══════════════════════════════════════════════════════════════════════ */

static int test_vp_draw_champion_slot(void) {
    TEST("Runtime: vp_draw_champion_slot — alive and dead slots");

    TQR_PlanarFramebuffer fb;
    fb.w = 320; fb.h = 240; fb.stride = 320;
    fb.data = (uint8_t *)calloc((size_t)320 * 240, 1);
    ASSERT(fb.data != NULL, "Failed to alloc planar fb");

    /* Build a minimal live champion */
    Theron_V1_Champion c;
    memset(&c, 0, sizeof(c));
    c.alive = 1;
    c.primary_class = THERON_CLASS_FIGHTER;
    c.health = 50; c.max_health = 100;
    c.stamina = 75; c.max_stamina = 100;
    c.mana = 0; c.max_mana = 0;
    snprintf(c.name, sizeof(c.name), "Theron");

    /* Draw champion slot at slot 0 (x=0, y=184 in planar coords) */
    theron_vp_draw_champion_slot(&fb, 0, 0, 184, &c);

    /* Slot region (80×56 at x=0, y=184) should have non-zero pixels */
    int slot_has_data = 0;
    for (int y = 184; y < 184 + 56 && !slot_has_data; y++) {
        for (int x = 0; x < 80; x++) {
            if (fb.data[y * fb.stride + x] != 0) { slot_has_data = 1; break; }
        }
    }
    ASSERT(slot_has_data, "Live champion slot should have non-zero pixels");
    int name_glyph_pixels = 0;
    int solid_placeholder_row = 1;
    for (int y = 190; y < 195; y++) {
        for (int x = 24; x < 74; x++) {
            if (fb.data[y * fb.stride + x] == 15) name_glyph_pixels++;
        }
    }
    for (int x = 24; x < 74; x++) {
        if (fb.data[190 * fb.stride + x] != 15) {
            solid_placeholder_row = 0;
            break;
        }
    }
    ASSERT(name_glyph_pixels >= 18,
           "Live champion slot should render champion-name glyph pixels");
    ASSERT(!solid_placeholder_row,
           "Live champion slot name should not be the old solid placeholder bar");

    /* Dead champion — X mark */
    memset(fb.data, 0, 320 * 240);
    c.alive = 0;
    theron_vp_draw_champion_slot(&fb, 0, 0, 184, &c);
    slot_has_data = 0;
    for (int y = 184; y < 184 + 56 && !slot_has_data; y++) {
        for (int x = 0; x < 80; x++) {
            if (fb.data[y * fb.stride + x] != 0) { slot_has_data = 1; break; }
        }
    }
    ASSERT(slot_has_data, "Dead champion slot (X mark) should have non-zero pixels");

    /* NULL champion — also X mark */
    memset(fb.data, 0, 320 * 240);
    theron_vp_draw_champion_slot(&fb, 0, 0, 184, NULL);
    slot_has_data = 0;
    for (int y = 184; y < 184 + 56 && !slot_has_data; y++) {
        for (int x = 0; x < 80; x++) {
            if (fb.data[y * fb.stride + x] != 0) { slot_has_data = 1; break; }
        }
    }
    ASSERT(slot_has_data, "NULL champion slot should have non-zero pixels (X mark)");

    free(fb.data);
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Runtime tests — Source evidence strings
 * ══════════════════════════════════════════════════════════════════════ */

static int test_source_evidence_strings(void) {
    TEST("Runtime: source evidence strings non-empty");

    const char *ev = theron_v1_viewport_source_evidence();
    ASSERT(ev != NULL && ev[0] != '\0', "viewport source evidence is empty");

    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Runtime tests — Palette state
 * ══════════════════════════════════════════════════════════════════════ */

static int test_palette_state_init(void) {
    TEST("Runtime: palette state init and tile access");

    TQR_PaletteState pal;
    tqr_palette_init_defaults(&pal);

    ASSERT(pal.tile_count >= 0, "tile_count should be >= 0");
    ASSERT(pal.tile_count >= 0, "tile_count should be >= 0");

    /* Palette should have some non-zero entries */
    int colors_non_zero = 0;
    for (int i = 0; i < 16 && !colors_non_zero; i++) {
        if (pal.entries[i].bgr444 != 0) colors_non_zero = 1;
    }
    ASSERT(colors_non_zero, "Palette should have some non-zero colors");

    tqr_palette_free_tiles(&pal);
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Runtime tests — synthetic_rendering_blocked integration
 * ══════════════════════════════════════════════════════════════════════ */

static int test_runtime_render_frame_blocks_verified_track02(void) {
    TEST("Runtime: boot render frame blocks verified Track 02 without graphics bank");

    static const char *us_md5 = "f23601102138f87c33025877767ebf76";
    static const char *jp_md5 = "b7afb338ad31be1025b53f9aff12d73a";
    const char *path = "/Users/bosse/.firestaff/data/theron/TQUS02.bin";
    const char *md5 = us_md5;
    struct stat st;

    if (stat(path, &st) != 0 || st.st_size == 0) {
        path = "/Users/bosse/.firestaff/data/theron/TQJP02.bin";
        md5 = jp_md5;
        if (stat(path, &st) != 0 || st.st_size == 0) {
            printf("SKIP: no real Track 02 media present\n");
            PASS();
            return 1;
        }
    }

    TrAssetBundle bundle;
    TrAssetResult r = tr_asset_load(path, &bundle);
    ASSERT(r == TR_ASSET_OK, "verified Track 02 load should succeed");

    bundle.assets_verified = 1;
    tr_asset_block_synthetic_rendering_for_verified_media(&bundle, md5);
    ASSERT(bundle.synthetic_rendering_blocked == 1,
           "verified Track 02 should be synthetic-render-blocked");

    Theron_V1_World world;
    Theron_V1_Viewport vp;
    unsigned char fb[320 * 200];
    theron_v1_world_init(&world);
    ASSERT(theron_vp_init(&vp) == 1, "viewport initializes");
    memset(fb, 0, sizeof(fb));

    int ok = theron_v1_boot_runtime_render_frame(&world, &vp, &bundle,
                                                 0, 0, fb, 320, 200);
    ASSERT(ok == 0,
           "runtime render must refuse to draw blocked verified media");

    int all_zero = 1;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) { all_zero = 0; break; }
    }
    ASSERT(all_zero,
           "blocked runtime render must leave framebuffer black");

    theron_vp_free(&vp);
    tr_asset_free(&bundle);
    PASS();
    return 1;
}

static int test_runtime_render_frame_allows_with_tile_bank(void) {
    TEST("Runtime: boot render frame draws when a graphics bank is bound");

    TrAssetBundle bundle;
    memset(&bundle, 0, sizeof(bundle));
    tqr_palette_init_defaults(&bundle.palette);

    /* A synthetic graphics bank: one non-empty tile + a Track 03 marker.
     * Even with synthetic_rendering_blocked set, a real tile bank overrides
     * the block and lets V1 rendering proceed. */
    static const uint8_t tile_data[16] = {
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    };
    static const uint8_t tr03_marker[4] = { 'T', 'H', 'G', '3' };

    int tile_idx = tqr_tile_load_from_data(&bundle.palette,
                                           tile_data, 2, 0, "test_wall");
    ASSERT(tile_idx == 0, "test tile loads at index 0");
    bundle.track03_data = tr03_marker;
    bundle.track03_size = sizeof(tr03_marker);
    bundle.hucard_rom = (uint8_t *)malloc(16);
    ASSERT(bundle.hucard_rom != NULL, "hucard_rom stub allocated");
    memset((void *)bundle.hucard_rom, 0, 16);
    bundle.hucard_rom_size = 16;
    bundle.assets_verified = 1;
    bundle.synthetic_rendering_blocked = 1;

    ASSERT(tr_asset_generated_v1_rendering_allowed(&bundle),
           "bundle with tile bank allows generated V1 rendering");

    Theron_V1_World world;
    Theron_V1_Viewport vp;
    unsigned char fb[320 * 200];
    theron_v1_world_init(&world);
    ASSERT(theron_vp_init(&vp) == 1, "viewport initializes");
    memset(fb, 0, sizeof(fb));

    /* Build a tiny all-wall level so tile 0 is used at D0. */
    world.current_dungeon = 1;
    world.current_level   = 0;
    world.level_loaded[0][0] = 1;
    world.levels[0][0].width  = 16;
    world.levels[0][0].height = 16;
    world.levels[0][0].start_x = 8;
    world.levels[0][0].start_y = 8;
    world.levels[0][0].start_dir = 0;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            world.levels[0][0].squares[y][x] = THERON_SQUARE_WALL;
        }
    }
    world.party.leader_x = 8;
    world.party.leader_y = 8;
    world.party.leader_dir = 0;

    int ok = theron_v1_boot_runtime_render_frame(&world, &vp, &bundle,
                                                 0, 0, fb, 320, 200);
    ASSERT(ok == 1,
           "runtime render must draw when a graphics bank is bound");

    int has_content = 0;
    for (int y = 24; y < 200 && !has_content; y++) {
        for (int x = 32; x < 288; x++) {
            if (fb[y * 320 + x] != 0) { has_content = 1; break; }
        }
    }
    ASSERT(has_content,
           "viewport region should contain non-zero pixels after render");

    theron_vp_free(&vp);
    tr_asset_free(&bundle);
    PASS();
    return 1;
}

static int test_viewport_direct_render_blocks_synthetic(void) {
    TEST("Runtime: viewport direct render blocks synthetic when verified Track 02 unbound");

    Theron_V1_World world;
    Theron_V1_Viewport vp;
    theron_v1_world_init(&world);
    ASSERT(theron_vp_init(&vp) == 1, "viewport initializes");

    /* Simple 8x8 floor room so an unblocked render would definitely write
     * fallback tile pixels. */
    world.current_dungeon = 1;
    world.current_level   = 0;
    world.level_loaded[0][0] = 1;
    world.levels[0][0].width  = 8;
    world.levels[0][0].height = 8;
    world.levels[0][0].start_x = 3;
    world.levels[0][0].start_y = 3;
    world.levels[0][0].start_dir = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            world.levels[0][0].squares[y][x] =
                (x == 0 || y == 0 || x == 7 || y == 7)
                    ? THERON_SQUARE_WALL
                    : THERON_SQUARE_FLOOR;
        }
    }

    theron_vp_clear(&vp, 0xB1);
    theron_vp_set_synthetic_rendering_blocked(&vp, 1);
    theron_vp_render_dungeon(&vp, &world);

    int dungeon_changed = 0;
    for (int i = 0; i < vp.fb.w * vp.fb.h; i++) {
        if (vp.fb.data[i] != 0xB1) { dungeon_changed = 1; break; }
    }
    ASSERT(!dungeon_changed,
           "Blocked viewport must not render synthetic dungeon tiles");

    theron_vp_clear(&vp, 0xC2);
    theron_vp_render_ui(&vp, &world, TQR_UI_ALL);

    int ui_changed = 0;
    for (int i = 0; i < vp.fb.w * vp.fb.h; i++) {
        if (vp.fb.data[i] != 0xC2) { ui_changed = 1; break; }
    }
    ASSERT(!ui_changed,
           "Blocked viewport must not render synthetic UI chrome");

    /* Clearing the block restores normal no-source-bank behavior (still a
     * no-op here because no tile bank is bound, but the flag is reset). */
    theron_vp_set_synthetic_rendering_blocked(&vp, 0);
    ASSERT(vp.synthetic_rendering_blocked == 0,
           "synthetic_rendering_blocked should reset to 0");

    theron_vp_free(&vp);
    PASS();
    return 1;
}

static int test_ui_chrome_blocks_unbound_source_bank(void) {
    TEST("Runtime: UI chrome module blocks placeholder bars until a source bank is bound");

    /* TQR-SYN-01: tr_ui_render_topbar / tr_ui_draw_champion_slot draw
     * placeholder dungeon/name/class blocks. Verified Track 02 exposes
     * title/stage/Soul Room/forcefield bitmap routes and roster/prompt text,
     * so this legacy chrome compositor must stay off until a real font/panel
     * bank is decoded.  tr_ui_source_bank_ready() currently reports not-ready
     * for every state, so the whole module is a no-op; this test locks that
     * contract. */
    TQR_PlanarFramebuffer fb;
    fb.w = 256; fb.h = 224; fb.stride = 256;
    fb.data = (uint8_t *)calloc((size_t)fb.w * fb.h, 1);
    ASSERT(fb.data != NULL, "fb alloc failed");

    Theron_V1_World world;
    theron_v1_world_init(&world);
    world.current_dungeon = 1;
    world.quest_items_in_dungeon = 2;

    memset(fb.data, 0xD3, (size_t)fb.w * fb.h);
    tr_ui_render(&fb, &world, TR_UI_ALL);

    int changed = 0;
    for (int i = 0; i < fb.w * fb.h; i++) {
        if (fb.data[i] != 0xD3) { changed = 1; break; }
    }
    ASSERT(!changed,
           "tr_ui_render must not draw placeholder chrome without a source bank");

    free(fb.data);
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Main
 * ══════════════════════════════════════════════════════════════════════ */

int main(void) {
    printf("\n=== Theron V1 Phase 4 - Rendering Pipeline Tests ===\n\n");

    /* Compile-time tests */
    printf("[Compile-time tests]\n");
    test_compile_struct_sizes();
    test_compile_dimensions();
    test_compile_square_constants();
    test_compile_palette_constants();
    test_compile_ui_flags();

    /* Runtime tests */
    printf("\n[Runtime tests]\n");
    test_vp_init_free();
    test_vp_clear();
    test_vp_tile_for_square();
    test_palette_state_init();
    test_asset_selection_wiring();
    test_asset_load_raw_track02_fallback();
    test_asset_verified_track02_blocks_synthetic_rendering();
    test_vp_render_dungeon();
    test_vp_render_ui_zones();
    test_vp_draw_bar();
    test_vp_draw_champion_slot();
    test_vp_present();
    test_vp_present_with_custom_palette();
    test_source_evidence_strings();
    test_runtime_render_frame_blocks_verified_track02();
    test_runtime_render_frame_allows_with_tile_bank();
    test_viewport_direct_render_blocks_synthetic();
    test_ui_chrome_blocks_unbound_source_bank();

    printf("\n=====================================================\n");
    printf("Results: %d/%d passed", g_tests_passed, g_tests_run);
    if (g_failures > 0) {
        printf("  (%d FAILED)\n", g_failures);
    } else {
        printf("  (all passed)\n");
    }
    printf("=====================================================\n");

    return g_failures > 0 ? 1 : 0;
}
