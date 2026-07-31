/*
 * test_csb_v22_viewport_swap_pc34.c — retired CSB V2.2 swap boundary
 *
 * Raw-cell observations may reach the inspector cache, but cannot infer a
 * V2.2 asset or paint over an F0128 source frame without a command receipt.
 */

#include "csb_v22_viewport_swap_pc34.h"
#include "csb_v22_inplace_draw_pc34.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;
static int checks = 0;

#define CHECK(expr, msg) \
    do { checks++; if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s — %s\n", __FILE__, __LINE__, #expr, (msg)); \
        failures++; } } while (0)

static void reset_state(void) {
    csb_v22_inplace_draw_shutdown();
}

static void test_raw_cells_never_infer_shapes(void) {
    static const unsigned char raw_values[] = { 0x00, 0x03, 0x20, 0x40, 0x80, 0xff };
    int i, direction;
    for (i = 0; i < (int)(sizeof(raw_values) / sizeof(raw_values[0])); ++i) {
        for (direction = 0; direction < 4; ++direction) {
            CHECK(csb_v22_swap_shape_for_cell(raw_values[i], (uint8_t)direction) ==
                      CSB_V22_SWAP_SHAPE_NONE,
                  "raw cell cannot infer a V2.2 shape without an F0128 receipt");
        }
    }
}

static void test_legacy_shapes_never_select_assets(void) {
    static const CSB_V22_SwapShapeType shapes[] = {
        CSB_V22_SWAP_SHAPE_WALL_STRAIGHT,
        CSB_V22_SWAP_SHAPE_FLOOR_DOOR,
        CSB_V22_SWAP_SHAPE_CREATURE,
        CSB_V22_SWAP_SHAPE_FIELD_TELEPORTER,
        CSB_V22_SWAP_SHAPE_PRISON_DOOR,
        CSB_V22_SWAP_SHAPE_NONE
    };
    int i;
    for (i = 0; i < (int)(sizeof(shapes) / sizeof(shapes[0])); ++i) {
        CHECK(csb_v22_swap_asset_id_for_shape(shapes[i]) == NULL,
              "legacy shape cannot select an asset");
        CHECK(csb_v22_swap_category_for_shape(shapes[i]) == NULL,
              "legacy shape cannot select an asset category");
    }
}

static void test_update_is_observation_only(void) {
    unsigned char raw_cells[3][3] = {
        { 0x00, 0x04, 0x80 },
        { 0x05, 0x40, 0x06 },
        { 0x10, 0x01, 0x20 }
    };
    reset_state();
    CHECK(csb_v22_viewport_swap_populated() == 0, "not populated before update");
    csb_v22_viewport_swap_update(0, (const unsigned char (*)[3])raw_cells);
    CHECK(csb_v22_viewport_swap_populated() == 1, "observation is retained");
    CHECK(csb_v22_viewport_swap_active() == 0,
          "a populated cache cannot authenticate a source command");
    csb_v22_viewport_swap_update(0, NULL);
    CHECK(csb_v22_viewport_swap_populated() == 1, "NULL update is safe");
    CHECK(csb_v22_viewport_swap_active() == 0, "NULL update remains inactive");
}

static void test_render_is_permanent_no_op(void) {
    unsigned char fb[1920 * 1080];
    int i;
    memset(fb, 0xaa, sizeof(fb));
    CHECK(csb_v22_viewport_swap_render(fb, 1920, 1080) == 0,
          "no source receipt means no paint");
    for (i = 0; i < (int)sizeof(fb); ++i) {
        if (fb[i] != 0xaa) break;
    }
    CHECK(i == (int)sizeof(fb), "no-op preserves the source framebuffer");
    CHECK(csb_v22_viewport_swap_render(NULL, 1920, 1080) == 0,
          "NULL framebuffer is safe");
    CHECK(csb_v22_viewport_swap_cells_painted() == 0, "paint counter stays zero");
}

static void test_source_evidence(void) {
    const char* ev = csb_v22_viewport_swap_source_evidence();
    CHECK(ev && strstr(ev, "ReDMCSB") && strstr(ev, "CSBWin"),
          "evidence cites the source viewport routes");
    CHECK(ev && strstr(ev, "F0128"), "evidence cites the F0128 owner");
}

int main(void) {
    test_raw_cells_never_infer_shapes();
    test_legacy_shapes_never_select_assets();
    test_update_is_observation_only();
    test_render_is_permanent_no_op();
    test_source_evidence();
    printf("csb_v22_viewport_swap_pc34: checks=%d failures=%d\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
