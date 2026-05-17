#include "dm1_v2_movement_engine_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"

#include <stdio.h>
#include <stdlib.h>

static int failures = 0;



static const char* dm1_default_dungeon_dat_path(void) {
    static char path[1024];
    const char* home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    snprintf(path, sizeof(path), "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT", home);
    return path;
}

static unsigned char* read_file_bytes(const char* path, int* outSize) {
    FILE* f = fopen(path, "rb");
    unsigned char* data = NULL;
    long size = 0;
    if (outSize) *outSize = 0;
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    size = ftell(f);
    if (size <= 0) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    data = (unsigned char*)malloc((size_t)size);
    if (!data) { fclose(f); return NULL; }
    if (fread(data, 1, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    if (outSize) *outSize = (int)size;
    return data;
}


#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static void test_collision_column_major(void);
static void test_movement_basics(void) {
    DM1_V2_PlayerPos p;
    DM1_V2_MoveParams params = {256, 1, 256, 0, 0};
    int8_t map[9] = {
        0, 0, 0,
        0, 1, 0,
        0, 0, 0,
    };

    dm1_v2_pos_init(&p, 2, 3, 0);
    CHECK(dm1_v2_get_x(&p) == 2 * DM1_V2_SUBPIXEL_SCALE);
    CHECK(dm1_v2_get_y(&p) == 3 * DM1_V2_SUBPIXEL_SCALE);
    CHECK(p.facingDir == 0);
    CHECK(!dm1_v2_has_moved(&p));

    dm1_v2_set_subpixel(&p, -5, DM1_V2_SUBPIXEL_SCALE + 9);
    CHECK(p.xSub == 0);
    CHECK(p.ySub == DM1_V2_SUBPIXEL_SCALE - 1);

    dm1_v2_turn(&p, -1);
    CHECK(p.facingDir == 7);
    dm1_v2_turn(&p, 1);
    CHECK(p.facingDir == 0);

    dm1_v2_move_step(&p, &params, 0, 1000);
    CHECK(dm1_v2_has_moved(&p));
    CHECK(dm1_v2_collides_at(1, 1, map, 3, 3) == 1);
    CHECK(dm1_v2_collides_at(0, 0, map, 3, 3) == 0);
    CHECK(dm1_v2_collides_at(-1, 0, map, 3, 3) == 1);
    CHECK(dm1_v2_collides_at(0, 0, NULL, 3, 3) == 1);
}

static void test_viewport_basics(void) {
    DM1_V2_ViewportState vp;
    DM1_V2_Color c;

    dm1_v2_vp_init(&vp);
    CHECK(dm1_v2_vp_is_dirty(&vp));
    CHECK(!dm1_v2_vp_is_scrolling(&vp));
    CHECK(vp.light.fogDensity[0] == 0);
    CHECK(vp.light.fogDensity[3] == 192);

    dm1_v2_vp_clear(&vp, 10, 20, 30);
    c = dm1_v2_vp_get_pixel(&vp, 5, 6);
    CHECK(c.r == 10 && c.g == 20 && c.b == 30 && c.a == 255);

    dm1_v2_vp_set_pixel(&vp, 5, 6, 100, 110, 120, 130);
    c = dm1_v2_vp_get_pixel(&vp, 5, 6);
    CHECK(c.r == 100 && c.g == 110 && c.b == 120 && c.a == 130);

    c = dm1_v2_vp_get_pixel(&vp, -1, -1);
    CHECK(c.r == 0 && c.g == 0 && c.b == 0 && c.a == 255);

    dm1_v2_vp_present(&vp, 1234);
    CHECK(!dm1_v2_vp_is_dirty(&vp));
    CHECK(vp.frameCount == 1);
    CHECK(vp.lastRenderMs == 1234);

    dm1_v2_vp_begin_scroll(&vp, 8, 0, 16);
    CHECK(dm1_v2_vp_is_scrolling(&vp));
    dm1_v2_vp_tick_scroll(&vp, 250);
    CHECK(vp.scroll.scrollOffX == 4);
    CHECK(vp.scroll.scrollOffX <= vp.scroll.scrollTargetX);
    dm1_v2_vp_tick_scroll(&vp, 250);
    CHECK(vp.scroll.scrollOffX == 8);
    CHECK(!dm1_v2_vp_is_scrolling(&vp));

    dm1_v2_vp_begin_scroll(&vp, -8, 0, 16);
    CHECK(dm1_v2_vp_is_scrolling(&vp));
    dm1_v2_vp_tick_scroll(&vp, 250);
    CHECK(vp.scroll.scrollOffX == 4);
    dm1_v2_vp_tick_scroll(&vp, 250);
    CHECK(vp.scroll.scrollOffX == 0);
    CHECK(!dm1_v2_vp_is_scrolling(&vp));
}


static void test_viewport_wall_occlusion_source_lock(void) {
    /* V2 seam: viewport/wall/occlusion, deliberately separate from command/movement.
       Source refs are locked by tools/verify_dm1_v2_viewport_wall_occlusion_source_lock.py. */
    CHECK(dm1_v2_vp_use_flipped_wall_bitmaps(10, 20, 0) == 0);
    CHECK(dm1_v2_vp_use_flipped_wall_bitmaps(10, 20, 1) == 1);
    CHECK(dm1_v2_vp_use_flipped_wall_bitmaps(11, 20, 1) == 0);

    CHECK(dm1_v2_vp_square_occludes_beyond(DM1_V2_VIEW_SQUARE_D3C, DM1_V2_ELEMENT_WALL) == 1);
    CHECK(dm1_v2_vp_square_occludes_beyond(DM1_V2_VIEW_SQUARE_D3C, DM1_V2_ELEMENT_DOOR_FRONT) == 0);
    CHECK(dm1_v2_vp_square_occludes_beyond(DM1_V2_VIEW_SQUARE_D3C, DM1_V2_ELEMENT_CORRIDOR) == 0);
    CHECK(dm1_v2_vp_square_occludes_beyond(DM1_V2_VIEW_SQUARE_OTHER, DM1_V2_ELEMENT_WALL) == 0);
}



static void test_viewport_d0_d3_draw_list_comparator_source_lock(void) {
    DM1_V2_ViewportCompositionInput input;
    DM1_V2_DrawCommand expected[DM1_V2_MAX_DRAW_COMMANDS];
    DM1_V2_DrawCommand actual[DM1_V2_MAX_DRAW_COMMANDS];
    int expectedCount;
    int actualCount;
    int mismatch = -1;

    dm1_v2_vp_composition_init(&input);
    input.mapX = 10;
    input.mapY = 20;
    input.direction = 1;
    input.squares[3][0].element = DM1_V2_ELEMENT_WALL;        /* D3L */
    input.squares[3][1].element = DM1_V2_ELEMENT_DOOR_FRONT;  /* D3C */
    input.squares[2][2].element = DM1_V2_ELEMENT_TELEPORTER;  /* D2R */
    input.squares[2][2].hasField = 1;
    input.squares[1][1].hasObjects = 1;                       /* D1C */
    input.squares[0][1].element = DM1_V2_ELEMENT_STAIRS_FRONT;/* D0C */

    expectedCount = dm1_v2_vp_emit_d0_d3_draw_list(&input, expected, DM1_V2_MAX_DRAW_COMMANDS);
    actualCount = dm1_v2_vp_emit_d0_d3_draw_list(&input, actual, DM1_V2_MAX_DRAW_COMMANDS);

    CHECK(expectedCount == actualCount);
    CHECK(expectedCount == 9);
    CHECK(dm1_v2_vp_compare_draw_lists(expected, expectedCount, actual, actualCount, &mismatch) == 1);
    CHECK(mismatch == -1);

    CHECK(actual[0].op == DM1_V2_DRAW_FLOOR_CEILING);
    CHECK(actual[1].square == DM1_V2_VIEW_SQUARE_D3L && actual[1].op == DM1_V2_DRAW_WALL && actual[1].order == 1);
    CHECK(actual[2].square == DM1_V2_VIEW_SQUARE_D3C && actual[2].op == DM1_V2_DRAW_FLOOR_ORNAMENT && actual[2].order == 3);
    CHECK(actual[3].square == DM1_V2_VIEW_SQUARE_D3C && actual[3].op == DM1_V2_DRAW_OBJECTS_CREATURES_PROJECTILES && actual[3].order == 3);
    CHECK(actual[4].square == DM1_V2_VIEW_SQUARE_D3C && actual[4].op == DM1_V2_DRAW_DOOR_FRONT && actual[4].order == 3);
    CHECK(actual[5].square == DM1_V2_VIEW_SQUARE_D3C && actual[5].op == DM1_V2_DRAW_OBJECTS_CREATURES_PROJECTILES && actual[5].order == 3);
    CHECK(actual[6].square == DM1_V2_VIEW_SQUARE_D2R && actual[6].op == DM1_V2_DRAW_FIELD && actual[6].order == 5);
    CHECK(actual[7].square == DM1_V2_VIEW_SQUARE_D1C && actual[7].op == DM1_V2_DRAW_OBJECTS_CREATURES_PROJECTILES && actual[7].order == 9);
    CHECK(actual[8].square == DM1_V2_VIEW_SQUARE_D0C && actual[8].op == DM1_V2_DRAW_STAIRS_FRONT && actual[8].order == 12);

    actual[8].order = 11;
    CHECK(dm1_v2_vp_compare_draw_lists(expected, expectedCount, actual, actualCount, &mismatch) == 0);
    CHECK(mismatch == 8);
    CHECK(dm1_v2_vp_emit_d0_d3_draw_list(&input, actual, 2) == 0);
}


static void test_viewport_real_state_fixture_draw_list(void) {
    const DM1_V2_DungeonStateFixture* fixture = dm1_v2_vp_dm1_pc34_entry_state_fixture();
    DM1_V2_ViewportCompositionInput input;
    DM1_V2_DrawCommand commands[DM1_V2_MAX_DRAW_COMMANDS];
    int x = 0;
    int y = 0;
    int count = 0;

    CHECK(fixture != NULL);
    CHECK(fixture->name != NULL);
    CHECK(fixture->startMapX == 1);
    CHECK(fixture->startMapY == 3);
    CHECK(fixture->startDirection == 2);
    CHECK(dm1_v2_vp_relative_coords(2, 1, 3, 1, 0, &x, &y) == 1);
    CHECK(x == 1 && y == 4);

    CHECK(dm1_v2_vp_build_composition_from_fixture(fixture,
                                                    fixture->startMapX,
                                                    fixture->startMapY,
                                                    fixture->startDirection,
                                                    &input) == 1);
    CHECK(input.mapX == 1 && input.mapY == 3 && input.direction == 2);
    CHECK(input.squares[1][1].element == DM1_V2_ELEMENT_WALL); /* D1C/front wall */
    CHECK(input.squares[0][1].element == DM1_V2_ELEMENT_CORRIDOR); /* D0C/current square */

    count = dm1_v2_vp_emit_d0_d3_draw_list(&input, commands, DM1_V2_MAX_DRAW_COMMANDS);
    CHECK(count == 2);
    CHECK(commands[0].op == DM1_V2_DRAW_FLOOR_CEILING);
    CHECK(commands[1].square == DM1_V2_VIEW_SQUARE_D1C);
    CHECK(commands[1].op == DM1_V2_DRAW_WALL);
    CHECK(commands[1].depth == 1 && commands[1].lateral == 0 && commands[1].order == 9);
}


static void test_viewport_dungeon_dat_decoder_entry_draw_list(void) {
    const char* path = getenv("DM1_PC34_DUNGEON_DAT");
    int size = 0;
    unsigned char* bytes = NULL;
    DM1_V2_DungeonDatState dungeon;
    DM1_V2_ViewportCompositionInput input;
    DM1_V2_DrawCommand commands[DM1_V2_MAX_DRAW_COMMANDS];
    uint8_t raw = 0;
    int count = 0;
    int sawD3LeftWall = 0;
    int sawD3RightWall = 0;
    int sawD0Field = 0;
    int sawD1Objects = 0;

    if (!path) path = dm1_default_dungeon_dat_path();
    bytes = read_file_bytes(path, &size);
    CHECK(bytes != NULL);
    if (!bytes) return;

    CHECK(size == 33357);
    CHECK(dm1_v2_vp_dungeon_dat_init(&dungeon, bytes, size) == 1);
    CHECK(dungeon.ornamentRandomSeed == 99);
    CHECK(dungeon.rawMapDataByteCount == 12283);
    CHECK(dungeon.rawMapDataFileOffset == 21072);
    CHECK(dungeon.checksumFileOffset == 33355);
    CHECK(dungeon.mapCount == 14);
    CHECK(dungeon.textDataWordCount == 1749);
    CHECK(dungeon.initialPartyLocation == 0x0861);
    CHECK(dungeon.initialMapX == 1);
    CHECK(dungeon.initialMapY == 3);
    CHECK(dungeon.initialDirection == 2);
    CHECK(dungeon.squareFirstThingCount == 1679);
    CHECK(dungeon.maps[0].rawMapDataByteOffset == 0);
    CHECK(dungeon.maps[0].level == 0);
    CHECK(dungeon.maps[0].width == 18);
    CHECK(dungeon.maps[0].height == 19);

    CHECK(dm1_v2_vp_dungeon_dat_get_square_raw(&dungeon, 0, 1, 3, &raw) == 1);
    CHECK(raw == 0xB0);
    CHECK(dm1_v2_vp_square_element_from_raw(raw, dungeon.initialDirection) == DM1_V2_ELEMENT_TELEPORTER);
    CHECK(dm1_v2_vp_dungeon_dat_get_square_raw(&dungeon, 0, 1, 4, &raw) == 1);
    CHECK(raw == 0x30);
    CHECK(dm1_v2_vp_square_element_from_raw(raw, dungeon.initialDirection) == DM1_V2_ELEMENT_CORRIDOR);
    CHECK(dm1_v2_vp_dungeon_dat_get_square_raw(&dungeon, 0, 0, 6, &raw) == 1);
    CHECK(raw == 0x00);
    CHECK(dm1_v2_vp_square_element_from_raw(raw, dungeon.initialDirection) == DM1_V2_ELEMENT_WALL);

    CHECK(dm1_v2_vp_build_composition_from_dungeon(&dungeon, 0,
                                                    dungeon.initialMapX,
                                                    dungeon.initialMapY,
                                                    dungeon.initialDirection,
                                                    &input) == 1);
    CHECK(input.mapX == 1 && input.mapY == 3 && input.direction == 2);
    CHECK(input.squares[0][1].element == DM1_V2_ELEMENT_TELEPORTER); /* D0C/current decoded raw 0xB0 */
    CHECK(input.squares[0][1].hasObjects == 1);
    CHECK(input.squares[1][1].element == DM1_V2_ELEMENT_CORRIDOR);   /* D1C/near front decoded raw 0x30 */
    CHECK(input.squares[1][1].hasObjects == 1);
    CHECK(input.squares[3][0].element == DM1_V2_ELEMENT_WALL);       /* D3L decoded wall from raw map */
    CHECK(input.squares[3][2].element == DM1_V2_ELEMENT_WALL);       /* D3R decoded wall from raw map */

    count = dm1_v2_vp_emit_d0_d3_draw_list(&input, commands, DM1_V2_MAX_DRAW_COMMANDS);
    CHECK(count >= 5);
    for (int i = 0; i < count; i++) {
        if (commands[i].square == DM1_V2_VIEW_SQUARE_D3L && commands[i].op == DM1_V2_DRAW_WALL) sawD3LeftWall = 1;
        if (commands[i].square == DM1_V2_VIEW_SQUARE_D3R && commands[i].op == DM1_V2_DRAW_WALL) sawD3RightWall = 1;
        if (commands[i].square == DM1_V2_VIEW_SQUARE_D0C && commands[i].op == DM1_V2_DRAW_FIELD) sawD0Field = 1;
        if (commands[i].square == DM1_V2_VIEW_SQUARE_D1C && commands[i].op == DM1_V2_DRAW_OBJECTS_CREATURES_PROJECTILES) sawD1Objects = 1;
    }
    CHECK(sawD3LeftWall == 1);
    CHECK(sawD3RightWall == 1);
    CHECK(sawD0Field == 1);
    CHECK(sawD1Objects == 1);
    free(bytes);
}

static void test_viewport_region_comparator_scaffold(void) {
    DM1_V2_ViewportState expected;
    DM1_V2_ViewportState actual;
    DM1_V2_ViewportRegion region = {10, 20, 3, 2, "fixture-region"};
    DM1_V2_RegionCompareResult result;

    dm1_v2_vp_init(&expected);
    dm1_v2_vp_init(&actual);
    dm1_v2_vp_clear(&expected, 7, 8, 9);
    dm1_v2_vp_clear(&actual, 7, 8, 9);
    CHECK(dm1_v2_vp_compare_viewport_region(&expected.framebuffer[0][0], &actual.framebuffer[0][0],
                                            DM1_V2_VIEWPORT_W, region, &result) == 1);
    CHECK(result.comparedPixels == 6);
    CHECK(result.mismatchedPixels == 0);

    dm1_v2_vp_set_pixel(&actual, 11, 21, 1, 2, 3, 255);
    CHECK(dm1_v2_vp_compare_viewport_region(&expected.framebuffer[0][0], &actual.framebuffer[0][0],
                                            DM1_V2_VIEWPORT_W, region, &result) == 0);
    CHECK(result.mismatchedPixels == 1);
    CHECK(result.firstMismatchX == 11 && result.firstMismatchY == 21);
}

int main(void) {
    test_movement_basics();
    test_collision_column_major();
    test_viewport_basics();
    test_viewport_wall_occlusion_source_lock();
    test_viewport_d0_d3_draw_list_comparator_source_lock();
    test_viewport_real_state_fixture_draw_list();
    test_viewport_dungeon_dat_decoder_entry_draw_list();
    test_viewport_region_comparator_scaffold();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_movement_viewport_pc34: ok");
    return 0;
}

/* V2-BUG-001 regression test: verify column-major collision indexing.
 * Asymmetric 3x2 map (width=3, height=2) laid out column-major:
 *   col0: (0,0)=0 (0,1)=0
 *   col1: (1,0)=1 (1,1)=0   <-- wall at (1,0) only
 *   col2: (2,0)=0 (2,1)=0
 * Column-major flat: {0,0, 1,0, 0,0}
 * If code wrongly uses row-major (py*w+px), (1,0) would read index 1 (=0) → miss.
 */
static void test_collision_column_major(void) {
    int8_t map[6] = {
        0, 0,   /* column 0: (0,0)=0, (0,1)=0 */
        1, 0,   /* column 1: (1,0)=1, (1,1)=0 */
        0, 0    /* column 2: (2,0)=0, (2,1)=0 */
    };
    /* Wall at (1,0): column-major index = 1*2+0 = 2 → map[2] = 1 */
    CHECK(dm1_v2_collides_at(1, 0, map, 3, 2) == 1);
    /* No wall at (0,1): column-major index = 0*2+1 = 1 → map[1] = 0 */
    CHECK(dm1_v2_collides_at(0, 1, map, 3, 2) == 0);
    /* No wall at (1,1): column-major index = 1*2+1 = 3 → map[3] = 0 */
    CHECK(dm1_v2_collides_at(1, 1, map, 3, 2) == 0);
    /* No wall at (2,0): column-major index = 2*2+0 = 4 → map[4] = 0 */
    CHECK(dm1_v2_collides_at(2, 0, map, 3, 2) == 0);
}
