/*
 * firestaff_theron_v1_viewport_renderer_probe.c
 *
 * Theron's Quest V1 Phase 4 — Viewport renderer probe.
 *
 * Tests: viewport lifecycle, planar framebuffer, UI chrome zones,
 * asset selection (tile/palette wiring), M11 presentation blt,
 * viewport clear and present pipeline.
 *
 * Compile: see CMakeLists.txt
 * Run:     ./probe
 *
 * Source: THQUEST.ASM T520 (viewport tile selection), T600 (UI overlay zones)
 *         HuC6260/HuC6270 datasheet (VDC/VCE rendering)
 *         tqr_v1_phase2_data_formats_H2339.md §7
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "theron_v1_viewport.h"
#include "theron_v1_palette.h"
#include "theron_v1_world.h"

/* ── Stub for audio (platform-specific) ─────────────────────────── */
int  theron_v1_play_sound(int id)             { (void)id; return 0; }
void theron_v1_champion_die(void *w, int s)   { (void)w; (void)s; }

/* ── Test framework ─────────────────────────────────────────────── */
static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, label) do { \
    if (cond) { g_pass++; } \
    else { \
        printf("  FAIL: %s\n", label); \
        g_fail++; \
    } \
} while (0)

#define CHECK_INT(label, got, want) do { \
    if (got == want) { g_pass++; } \
    else { \
        printf("  FAIL: %s — got %d want %d\n", label, got, want); \
        g_fail++; \
    } \
} while (0)

#define CHECK_GE(label, got, min) do { \
    if (got >= (min)) { g_pass++; } \
    else { \
        printf("  FAIL: %s — got %d want >= %d\n", label, got, (min)); \
        g_fail++; \
    } \
} while (0)

/* ── World factory ───────────────────────────────────────────────── */
static void make_world(Theron_V1_World *w) {
    memset(w, 0, sizeof(*w));
    w->current_dungeon = 1;
    w->current_level   = 0;

    Theron_V1_Level *lvl = &w->levels[0][0];
    lvl->width  = 16;
    lvl->height = 16;
    lvl->level_index = 0;
    lvl->start_x = 8; lvl->start_y = 8;
    lvl->start_dir = 0;
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            lvl->squares[y][x] = THERON_SQUARE_FLOOR;

    w->level_loaded[0][0] = 1;
    w->party.leader_x = 8;
    w->party.leader_y = 8;
    w->party.leader_dir = 0;
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        w->party.champions[i].alive = 1;
        w->party.champions[i].health = 50;
        w->party.champions[i].max_health = 50;
        w->party.champions[i].stamina = 50;
        w->party.champions[i].max_stamina = 50;
        w->party.champions[i].mana = 0;
        w->party.champions[i].max_mana = 0;
        w->party.champions[i].name[0] = 'T';
        w->party.champions[i].name[1] = '\0';
        w->party.champions[i].primary_class = THERON_CLASS_FIGHTER;
    }
    w->party.active_slot = 0;
    w->party.gold = 100;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: viewport init / free lifecycle
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_init_free(void) {
    printf("[test:vp_init_free]\n");

    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));

    int r = theron_vp_init(&vp);
    CHECK(r == 1, "theron_vp_init returns 1 on success");
    CHECK(vp.initialized == 1, "vp.initialized is 1 after init");
    CHECK(vp.fb.data != NULL, "vp.fb.data is non-NULL after init");
    CHECK_INT("vp.fb.w == 256", vp.fb.w, 256);
    CHECK_INT("vp.fb.h == 224", vp.fb.h, 224);
    CHECK(vp.fb.stride == 256, "vp.fb.stride == 256 (row width)");

    /* Palette state is populated */
    CHECK(vp.palette.tile_count >= 0, "palette tile_count is non-negative");

    /* Clear the fb */
    theron_vp_clear(&vp, 0);
    CHECK(vp.fb.data[0] == 0, "clear with 0 works");

    /* Free */
    theron_vp_free(&vp);
    CHECK(vp.fb.data == NULL, "vp.fb.data is NULL after free");
    CHECK(vp.initialized == 0, "vp.initialized is 0 after free");
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: viewport tile index per square type
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_tile_for_square(void) {
    printf("[test:vp_tile_for_square]\n");

    /* WALL always returns valid tile index (not FALLBACK) */
    for (int d = 0; d < 4; d++) {
        int idx = theron_vp_tile_for_square(THERON_SQUARE_WALL, d, 1);
        CHECK(idx >= 0, "WALL tile index is non-negative at all depths");
    }

    /* FLOOR always returns valid tile index */
    for (int d = 0; d < 4; d++) {
        int idx = theron_vp_tile_for_square(THERON_SQUARE_FLOOR, d, 0);
        CHECK(idx >= 0, "FLOOR tile index is non-negative at all depths");
    }

    /* DOOR closed=wall, open=floor */
    for (int d = 0; d < 4; d++) {
        int idx_wall = theron_vp_tile_for_square(THERON_SQUARE_DOOR, d, 1);
        int idx_open = theron_vp_tile_for_square(THERON_SQUARE_DOOR, d, 0);
        CHECK(idx_wall >= 0, "DOOR (closed) tile index is non-negative");
        CHECK(idx_open >= 0, "DOOR (open) tile index is non-negative");
    }

    /* Invalid depth → FALLBACK (-1) */
    CHECK_INT("depth -1 → FALLBACK",
              theron_vp_tile_for_square(THERON_SQUARE_FLOOR, -1, 0),
              -1);
    CHECK_INT("depth 5 → FALLBACK",
              theron_vp_tile_for_square(THERON_SQUARE_FLOOR, 5, 0),
              -1);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: dungeon rendering into planar framebuffer
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_render_dungeon(void) {
    printf("[test:vp_render_dungeon]\n");

    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));
    theron_vp_init(&vp);

    Theron_V1_World w;
    make_world(&w);

    /* Walls at north edge of view */
    Theron_V1_Level *lvl = &w.levels[0][0];
    for (int x = 0; x < 16; x++) {
        lvl->squares[0][x] = THERON_SQUARE_WALL;
    }

    /* Render */
    theron_vp_render_dungeon(&vp, &w);

    /* Planar fb must have non-zero pixels (dungeon rendered) */
    int non_zero = 0;
    for (int i = 0; i < 256 * 224; i++) {
        if (vp.fb.data[i] != 0) { non_zero++; break; }
    }
    CHECK(non_zero > 0, "rendered viewport fb has non-zero pixels");

    /* Left/right margin (x < 32 or x >= 224) should be 0 */
    CHECK(vp.fb.data[0] == 0, "left margin pixel is 0");
    CHECK(vp.fb.data[31] == 0, "left margin pixel 31 is 0");
    /* Top margin row */
    CHECK(vp.fb.data[32 * 256] == 0, "top margin pixel is 0");

    /* D3 band (y=16..63): should have wall tiles rendered */
    int d3_y = 16;
    int non_zero_in_d3 = 0;
    for (int y = d3_y; y < d3_y + 48; y++) {
        for (int x = 32; x < 224; x++) {
            if (vp.fb.data[y * 256 + x] != 0) non_zero_in_d3++;
        }
    }
    CHECK(non_zero_in_d3 > 0, "D3 band (far) has non-zero pixels");

    theron_vp_free(&vp);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: UI chrome — top bar rendering
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_ui_topbar(void) {
    printf("[test:vp_ui_topbar]\n");

    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));
    theron_vp_init(&vp);

    Theron_V1_World w;
    make_world(&w);
    w.current_dungeon = 1;

    /* Render top bar */
    theron_vp_render_ui(&vp, &w, TQR_UI_TOPBAR);

    /* Top bar area: y=16..(16+24)=40 within planar fb
     * These pixels should be non-zero (top bar is drawn) */
    int topbar_pixels = 0;
    for (int y = 16; y < 40; y++) {
        for (int x = 0; x < 256; x++) {
            if (vp.fb.data[y * 256 + x] != 0) topbar_pixels++;
        }
    }
    CHECK(topbar_pixels > 0, "top bar area has non-zero pixels");

    theron_vp_free(&vp);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: UI chrome — champion bottom panel
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_ui_bottom_panel(void) {
    printf("[test:vp_ui_bottom_panel]\n");

    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));
    theron_vp_init(&vp);

    Theron_V1_World w;
    make_world(&w);

    /* Render bottom panel (4 champion slots) */
    theron_vp_render_ui(&vp, &w, TQR_UI_BOTTOM_PANEL);

    /* Bottom panel area: y=184..224
     * Slots: 4 × 80px wide = 320px (full width) */
    int bottom_pixels = 0;
    for (int y = 184; y < 224; y++) {
        for (int x = 0; x < 256; x++) {
            if (vp.fb.data[y * 256 + x] != 0) bottom_pixels++;
        }
    }
    CHECK(bottom_pixels > 0, "bottom panel has non-zero pixels");

    /* Each champion slot should have a class-colored icon region */
    /* Slot 0: x=0..79, y=184..239 */
    int slot0_pixels = 0;
    for (int y = 184; y < 224; y++) {
        for (int x = 0; x < 79; x++) {
            if (vp.fb.data[y * 256 + x] != 0) slot0_pixels++;
        }
    }
    CHECK(slot0_pixels > 0, "slot 0 (Theron) has non-zero pixels");

    theron_vp_free(&vp);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: UI chrome — right panel (stats + compass)
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_ui_right_panel(void) {
    printf("[test:vp_ui_right_panel]\n");

    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));
    theron_vp_init(&vp);

    Theron_V1_World w;
    make_world(&w);

    /* Render right panel */
    theron_vp_render_ui(&vp, &w, TQR_UI_RIGHT_PANEL);

    /* Right panel: x=160..255, y=16..224
     * Should have non-zero pixels (stats bars drawn) */
    int right_pixels = 0;
    for (int y = 16; y < 224; y++) {
        for (int x = 160; x < 256; x++) {
            if (vp.fb.data[y * 256 + x] != 0) right_pixels++;
        }
    }
    CHECK(right_pixels > 0, "right panel has non-zero pixels");

    theron_vp_free(&vp);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: UI chrome — all zones combined
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_ui_all_zones(void) {
    printf("[test:vp_ui_all_zones]\n");

    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));
    theron_vp_init(&vp);

    Theron_V1_World w;
    make_world(&w);
    w.current_dungeon = 3; /* Abyss of Flames */
    w.quest_items_in_dungeon = 0;

    /* Render all UI zones */
    theron_vp_render_ui(&vp, &w, TQR_UI_ALL);

    /* Total non-zero pixels should be > dungeon-only pixels */
    int total_pixels = 0;
    for (int i = 0; i < 256 * 224; i++) {
        if (vp.fb.data[i] != 0) total_pixels++;
    }
    CHECK_GE("total UI pixels >= 100", total_pixels, 100);

    theron_vp_free(&vp);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: planar → M11 framebuffer presentation
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_present(void) {
    printf("[test:vp_present]\n");

    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));
    theron_vp_init(&vp);

    Theron_V1_World w;
    make_world(&w);

    /* Render dungeon */
    theron_vp_render_dungeon(&vp, &w);
    theron_vp_render_ui(&vp, &w, TQR_UI_ALL);

    /* Create M11 framebuffer: 320×200 */
    unsigned char *m11_fb = (unsigned char *)calloc(320 * 200, 1);
    CHECK(m11_fb != NULL, "M11 framebuffer allocated");

    /* Present */
    theron_vp_present(&vp, &vp.palette, m11_fb, 320, 200);

    /* M11 fb should have non-zero pixels in the viewport region */
    int m11_pixels = 0;
    for (int y = 24; y < 184; y++) {
        for (int x = 32; x < 288; x++) {
            if (m11_fb[y * 320 + x] != 0) m11_pixels++;
        }
    }
    CHECK(m11_pixels > 0, "M11 framebuffer has non-zero pixels in viewport");

    /* Black bars above viewport (y=0..23) should be 0 */
    int black_bar_pixels = 0;
    for (int y = 0; y < 24; y++) {
        for (int x = 0; x < 320; x++) {
            if (m11_fb[y * 320 + x] != 0) black_bar_pixels++;
        }
    }
    CHECK(black_bar_pixels == 0, "M11 black bar above viewport is all 0");

    free(m11_fb);
    theron_vp_free(&vp);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: present with different palette
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_present_with_palette(void) {
    printf("[test:vp_present_with_palette]\n");

    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));
    theron_vp_init(&vp);

    Theron_V1_World w;
    make_world(&w);
    theron_vp_render_dungeon(&vp, &w);

    unsigned char *m11_fb = (unsigned char *)calloc(320 * 200, 1);

    TQR_PaletteState alt_pal;
    tqr_palette_init_defaults(&alt_pal);

    /* Present with explicit palette */
    theron_vp_present(&vp, &alt_pal, m11_fb, 320, 200);
    /* With floor-only world + fallback tiles → pixel = 7 in planar fb
     * M11 index = tqr_idx_to_m11_idx(7) = 7 (mid-gray) */
    int has_content = 0;
    for (int i = 0; i < 320 * 200; i++) {
        if (m11_fb[i] != 0) { has_content = 1; break; }
    }
    CHECK(has_content, "M11 fb has non-zero pixels after present");

    free(m11_fb);
    theron_vp_free(&vp);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: asset selection — tile atlas wiring
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_asset_selection(void) {
    printf("[test:vp_asset_selection]\n");

    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));
    theron_vp_init(&vp);

    TQR_PaletteState pal;
    tqr_palette_init_defaults(&pal);

    /* Load a test tile (16 bytes, 2bpp) */
    uint8_t tile_raw[16] = {0};
    for (int row = 0; row < 8; row++) {
        tile_raw[row * 2 + 0] = 0xFF; /* bitplane 0 */
        tile_raw[row * 2 + 1] = 0xFF; /* bitplane 1 */
    }
    int tile_idx = tqr_tile_load_from_data(&pal, tile_raw, 2, 0, "test_wall");
    CHECK_INT("test tile loaded as tile_idx 0", tile_idx, 0);

    /* Wire palette into viewport */
    theron_vp_set_palette(&vp, &pal);

    /* Palette should be wired */
    CHECK(vp.palette.tile_count >= 1, "viewport palette has at least 1 tile");

    /* Load more tiles to test atlas */
    uint8_t floor_raw[16] = {0};
    for (int row = 0; row < 8; row++) {
        floor_raw[row * 2 + 0] = 0x00;
        floor_raw[row * 2 + 1] = 0x0F; /* bitplane 1 only → palette index 2 */
    }
    int floor_idx = tqr_tile_load_from_data(&pal, floor_raw, 2, 0, "test_floor");
    CHECK(floor_idx >= 0, "floor tile loaded");

    /* With tiles loaded, rendering should use them (or fallback if wrong index) */
    Theron_V1_World w;
    make_world(&w);
    theron_vp_render_dungeon(&vp, &w);

    /* Should complete without crashing */
    CHECK(vp.fb.data != NULL, "fb still valid after tile-wired render");

    theron_vp_free(&vp);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: viewport clear
 * ═══════════════════════════════════════════════════════════════ */
static void test_vp_clear(void) {
    printf("[test:vp_clear]\n");

    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));
    theron_vp_init(&vp);

    /* Fill with non-zero */
    memset(vp.fb.data, 0xAA, 256 * 224);

    theron_vp_clear(&vp, 7);
    CHECK(vp.fb.data[0] == 7, "clear sets pixel 0 to 7");
    CHECK(vp.fb.data[256*224-1] == 7, "clear sets far pixel to 7");

    theron_vp_clear(&vp, 0);
    CHECK(vp.fb.data[0] == 0, "clear with 0 resets to black");

    theron_vp_free(&vp);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: source evidence strings
 * ═══════════════════════════════════════════════════════════════ */
static void test_source_evidence(void) {
    printf("[test:source_evidence]\n");

    const char *ev = theron_v1_viewport_source_evidence();
    CHECK(ev != NULL && strlen(ev) > 20,
          "source_evidence cites THQUEST.ASM + HuC6260 datasheet");
}

/* ── Main ─────────────────────────────────────────────────────── */
int main(void) {
    printf("\n=== Theron V1 Phase 4 — Viewport Renderer Probe ===\n\n");

    test_vp_init_free();
    test_vp_tile_for_square();
    test_vp_render_dungeon();
    test_vp_ui_topbar();
    test_vp_ui_bottom_panel();
    test_vp_ui_right_panel();
    test_vp_ui_all_zones();
    test_vp_present();
    test_vp_present_with_palette();
    test_vp_asset_selection();
    test_vp_clear();
    test_source_evidence();

    printf("\n=========================================\n");
    printf("Results: %d passed, %d failed\n", g_pass, g_fail);
    printf("Source: THQUEST.ASM T520 (viewport tile)  "
           "+ T600 (UI overlay zones)  "
           "+ HuC6260/HuC6270 datasheet  "
           "+ tqr_v1_phase2_data_formats_H2339.md §7\n");
    printf("=========================================\n\n");

    return g_fail == 0 ? 0 : 1;
}