/*
 * DM1 V1 real-data D2L2/D2R2 forward-walk wall-opacity probe.
 *
 * Finds a shipped corridor pose and its real one-square-forward successor
 * where both D2L2 and D2R2 cells are walls while D2L/D2R remain corridors.
 * This leaves C707/C708 visible, so the probe verifies their framebuffer
 * pixels against the selected GRAPHICS.DAT wall bitmap before and after
 * M12_MENU_INPUT_UP advances the party.
 *
 * ReDMCSB DUNVIEW.C F0678 lines 6837-6865 and F0679 lines 6868-6896 draw
 * the D2L2/D2R2 wall case through C707/C708 and return. F0128 lines
 * 8500-8508 dispatches D2L2 before D2R2 at relative offsets (2,-2)/(2,2).
 */
#include "asset_loader_m11.h"
#include "firestaff/dm1/v1/viewport/d2c_f0108_floor_ceiling_ornament_pc34_compat.h"
#include "dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_pc34_compat.h"
#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1l_d1r_f0115_thing_pass_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum { FB_W = 320, FB_H = 200, VIEWPORT_Y = 33 };

enum {
    DM1_WALLSET_FIRST_GRAPHIC = 86,
    DM1_WALLSET_GRAPHIC_COUNT = 40
};

static int expect_true(const char* label, int value)
{
    if (!value) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int square_at(const M11_GameViewState* game, int x, int y)
{
    const struct DungeonMapDesc_Compat* map;
    int index;
    if (!game || !game->world.dungeon || game->world.party.mapIndex < 0 ||
        game->world.party.mapIndex >= (int)game->world.dungeon->header.mapCount) {
        return -1;
    }
    map = &game->world.dungeon->maps[game->world.party.mapIndex];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) {
        return -1;
    }
    index = x * (int)map->height + y;
    return game->world.dungeon->tiles[game->world.party.mapIndex].squareData[index];
}

static int element_at(const M11_GameViewState* game, int x, int y)
{
    int square = square_at(game, x, y);
    return square < 0 ? -1 :
        (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static void direction_vectors(int direction, int* fx, int* fy, int* rx, int* ry)
{
    static const int forward_x[4] = {0, 1, 0, -1};
    static const int forward_y[4] = {-1, 0, 1, 0};
    if (fx) *fx = forward_x[direction & 3];
    if (fy) *fy = forward_y[direction & 3];
    if (rx) *rx = -forward_y[direction & 3];
    if (ry) *ry = forward_x[direction & 3];
}

static int d2_pair_is_real_wall_pose(const M11_GameViewState* game,
                                     int x, int y, int direction)
{
    int fx, fy, rx, ry;
    direction_vectors(direction, &fx, &fy, &rx, &ry);
    int left = square_at(game, x + 2 * fx - 2 * rx, y + 2 * fy - 2 * ry);
    int right = square_at(game, x + 2 * fx + 2 * rx, y + 2 * fy + 2 * ry);
    return left >= 0 && right >= 0 &&
           ((left & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_WALL &&
           ((right & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_WALL &&
           (left & DUNGEON_SQUARE_MASK_THING_LIST) == 0 &&
           (right & DUNGEON_SQUARE_MASK_THING_LIST) == 0;
}

static int d2_side_pair_is_corridor(const M11_GameViewState* game,
                                    int x, int y, int direction)
{
    int fx, fy, rx, ry;
    int left;
    int right;
    direction_vectors(direction, &fx, &fy, &rx, &ry);
    left = square_at(game, x + 2 * fx - rx, y + 2 * fy - ry);
    right = square_at(game, x + 2 * fx + rx, y + 2 * fy + ry);
    return left >= 0 && right >= 0 &&
           ((left & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_CORRIDOR &&
           ((right & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_CORRIDOR &&
           (left & DUNGEON_SQUARE_MASK_THING_LIST) == 0 &&
           (right & DUNGEON_SQUARE_MASK_THING_LIST) == 0;
}

static int d1_side_pair_is_corridor(const M11_GameViewState* game,
                                    int x, int y, int direction)
{
    int fx, fy, rx, ry;
    int left;
    int right;
    direction_vectors(direction, &fx, &fy, &rx, &ry);
    left = square_at(game, x + fx - rx, y + fy - ry);
    right = square_at(game, x + fx + rx, y + fy + ry);
    return left >= 0 && right >= 0 &&
           ((left & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_CORRIDOR &&
           ((right & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_CORRIDOR &&
           (left & DUNGEON_SQUARE_MASK_THING_LIST) == 0 &&
           (right & DUNGEON_SQUARE_MASK_THING_LIST) == 0;
}

static int d0_side_pair_is_empty_corridor(const M11_GameViewState* game,
                                          int x, int y, int direction)
{
    int fx, fy, rx, ry;
    int left;
    int right;
    direction_vectors(direction, &fx, &fy, &rx, &ry);
    left = square_at(game, x - rx, y - ry);
    right = square_at(game, x + rx, y + ry);
    return left >= 0 && right >= 0 &&
           ((left & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_CORRIDOR &&
           ((right & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_CORRIDOR &&
           (left & DUNGEON_SQUARE_MASK_THING_LIST) == 0 &&
           (right & DUNGEON_SQUARE_MASK_THING_LIST) == 0;
}

/* ReDMCSB DUNGEON.C F0154, used by DUNVIEW.C F0112: level changes retain
 * global map coordinates; they do not use mapIndex +/- 1. */
static int parent_square_at(const M11_GameViewState* game, int x, int y,
                            int* out_map, int* out_x, int* out_y)
{
    const struct DungeonDatState_Compat* dungeon;
    const struct DungeonMapDesc_Compat* source;
    int global_x;
    int global_y;
    int target_level;
    int i;

    if (!game || !(dungeon = game->world.dungeon) || !dungeon->maps ||
        !dungeon->tiles || game->world.party.mapIndex < 0 ||
        game->world.party.mapIndex >= (int)dungeon->header.mapCount) {
        return -1;
    }
    source = &dungeon->maps[game->world.party.mapIndex];
    global_x = (int)source->offsetMapX + x;
    global_y = (int)source->offsetMapY + y;
    target_level = (int)source->level - 1;
    for (i = 0; i < (int)dungeon->header.mapCount; ++i) {
        const struct DungeonMapDesc_Compat* parent = &dungeon->maps[i];
        int parent_x;
        int parent_y;
        int square;
        if ((int)parent->level != target_level ||
            global_x < (int)parent->offsetMapX ||
            global_x >= (int)parent->offsetMapX + (int)parent->width ||
            global_y < (int)parent->offsetMapY ||
            global_y >= (int)parent->offsetMapY + (int)parent->height ||
            !dungeon->tiles[i].squareData) {
            continue;
        }
        parent_x = global_x - (int)parent->offsetMapX;
        parent_y = global_y - (int)parent->offsetMapY;
        square = dungeon->tiles[i].squareData[
            parent_x * (int)parent->height + parent_y];
        if (out_map) *out_map = i;
        if (out_x) *out_x = parent_x;
        if (out_y) *out_y = parent_y;
        return square;
    }
    return -1;
}

static int parent_square_is_open_pit(const M11_GameViewState* game,
                                     int x, int y)
{
    int square = parent_square_at(game, x, y, NULL, NULL, NULL);
    return square >= 0 &&
           ((square & DUNGEON_SQUARE_MASK_TYPE) >> 5) ==
               DUNGEON_ELEMENT_PIT &&
           (square & 0x08) != 0;
}

static int later_f0128_ceiling_pits_are_inactive(const M11_GameViewState* game,
                                                  int x, int y, int direction)
{
    int fx, fy, rx, ry;
    int depth;
    int side;
    direction_vectors(direction, &fx, &fy, &rx, &ry);
    /* After F0679, F0128 dispatches D2R, D2C, D1L/R/C and D0L/R/C.
     * Each corridor branch can call F0112, which draws a ceiling-pit bitmap
     * over the existing viewport. Keep the raw C708 oracle only where every
     * one of those source-visible F0112 routes is inactive. */
    for (depth = 0; depth <= 2; ++depth) {
        for (side = -1; side <= 1; ++side) {
            if (depth == 2 && side < 0) continue; /* D2L precedes F0679. */
            if (parent_square_is_open_pit(game,
                                          x + depth * fx + side * rx,
                                          y + depth * fy + side * ry)) {
                return 0;
            }
        }
    }
    return 1;
}

static void print_later_f0128_ceiling_pit_receipt(const M11_GameViewState* game,
                                                   const char* phase)
{
    int fx, fy, rx, ry;
    int depth;
    int side;
    direction_vectors(game->world.party.direction, &fx, &fy, &rx, &ry);
    for (depth = 0; depth <= 2; ++depth) {
        for (side = -1; side <= 1; ++side) {
            int x;
            int y;
            int parent_map = -1;
            int parent_x = -1;
            int parent_y = -1;
            int parent_square;
            if (depth == 2 && side < 0) continue;
            x = game->world.party.mapX + depth * fx + side * rx;
            y = game->world.party.mapY + depth * fy + side * ry;
            parent_square = parent_square_at(game, x, y, &parent_map,
                                             &parent_x, &parent_y);
            printf("%s F0112 receipt rel=%d,%d cell=%d,%d raw=%02x parent="
                   "%d,%d,%d raw=%02x open-pit=%d\n",
                   phase, depth, side, x, y, square_at(game, x, y),
                   parent_map, parent_x, parent_y, parent_square,
                   parent_square >= 0 &&
                   ((parent_square & DUNGEON_SQUARE_MASK_TYPE) >> 5) ==
                       DUNGEON_ELEMENT_PIT && (parent_square & 0x08) != 0);
        }
    }
}

static int print_d2c_f0108_receipt(const M11_GameViewState* game,
                                    const char* phase)
{
    const DM1_V1_D2CF0108ModelPc34* model =
        dm1_v1_viewport_d2c_f0108_model_pc34();
    const DM1_V1_D2CF0108RectPc34* ornament;
    int ordinal = M11_GameView_GetFloorOrnamentOrdinal(game, 2, 0);
    int disjoint;

    if (!model || !model->ok) return 0;
    ornament = &model->ornament_rect;
    /* ReDMCSB DUNVIEW.C F0121:7357 calls F0108 with M592_VIEW_FLOOR_D2C.
     * The PC34 C1500 coordinate-set-2 zone is x=96..127,y=103..127 in the
     * 224x136 viewport. C708 from F0679 is x=216..223,y=24..75. */
    disjoint = ornament->x2 < 216 || ornament->x1 > 223 ||
               ornament->y2 < 24 || ornament->y1 > 75;
    printf("%s F0121/F0108 D2C receipt ordinal=%d zone=%d,%d..%d,%d "
           "C708=216,24..223,75 disjoint=%d\n",
           phase, ordinal, ornament->x1, ornament->y1,
           ornament->x2, ornament->y2, disjoint);
    return disjoint;
}

static int print_d1l_f0122_receipt(const M11_GameViewState* game,
                                    const char* phase)
{
    const DM1_ViewportWallDrawSpec* wall =
        dm1_viewport_3d_get_side_wall_draw_spec_for_rel(1, -1);
    const DM1V1D1LD1RF0115LanePc34Data* thing_lane =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_for_view_square_pc34(4);
    int fx, fy, rx, ry;
    int square;
    int ordinal;
    int zone_disjoint;
    int thing_list_empty;

    direction_vectors(game->world.party.direction, &fx, &fy, &rx, &ry);
    square = square_at(game, game->world.party.mapX + fx - rx,
                       game->world.party.mapY + fy - ry);
    ordinal = M11_GameView_GetFloorOrnamentOrdinal(game, 1, -1);
    /* ReDMCSB DUNVIEW.C F0122:7520-7536 calls F0108 M594_VIEW_FLOOR_D1L
     * then F0115 M607_VIEW_SQUARE_D1L. The source-bound D1L panel is the
     * left viewport zone x=0..59; C708/F0679 is x=216..223. */
    zone_disjoint = wall && wall->runtime_dst_x + wall->runtime_width - 1 < 216;
    thing_list_empty = square >= 0 &&
                       (square & DUNGEON_SQUARE_MASK_THING_LIST) == 0;
    printf("%s F0122 D1L receipt raw=%02x ordinal=%d F0108-zone=%d..%d "
           "F0115-view=%d thing-list-empty=%d C708=216..223 disjoint=%d\n",
           phase, square, ordinal,
           wall ? wall->runtime_dst_x : -1,
           wall ? wall->runtime_dst_x + wall->runtime_width - 1 : -1,
           thing_lane ? thing_lane->view_square : -1, thing_list_empty,
           zone_disjoint);
    return wall && thing_lane && thing_lane->relative_depth == 1 &&
           thing_lane->relative_lateral == -1 && zone_disjoint &&
           thing_list_empty;
}

static int print_d1r_f0123_receipt(const M11_GameViewState* game,
                                    const char* phase)
{
    const DM1_ViewportWallDrawSpec* wall =
        dm1_viewport_3d_get_side_wall_draw_spec_for_rel(1, 1);
    const DM1_V1_D1LD1RF0108SpecPc34* floor_spec =
        dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34(2);
    const DM1V1D1LD1RF0115LanePc34Data* thing_lane =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_for_view_square_pc34(5);
    int fx, fy, rx, ry;
    int square;
    int ordinal;
    int zone_overlaps;
    int thing_list_empty;

    direction_vectors(game->world.party.direction, &fx, &fy, &rx, &ry);
    square = square_at(game, game->world.party.mapX + fx + rx,
                       game->world.party.mapY + fy + ry);
    ordinal = M11_GameView_GetFloorOrnamentOrdinal(game, 1, 1);
    /* ReDMCSB DUNVIEW.C F0123:7684-7704 calls F0108 M596_VIEW_FLOOR_D1R
     * then F0115 M608_VIEW_SQUARE_D1R. The source-bound D1R panel spans
     * x=164..223, so it reaches C708 x=216..223; the real M558/M550 gates
     * must therefore be zero before raw F0679 pixels are an oracle. */
    zone_overlaps = wall && wall->runtime_dst_x <= 223 &&
                    wall->runtime_dst_x + wall->runtime_width - 1 >= 216;
    thing_list_empty = square >= 0 &&
                       (square & DUNGEON_SQUARE_MASK_THING_LIST) == 0;
    printf("%s F0123 D1R receipt raw=%02x ordinal=%d F0108-view=%d zone=%d "
           "F0115-view=%d thing-list-empty=%d panel=%d..%d C708=216..223 "
           "overlap=%d\n",
           phase, square, ordinal, floor_spec ? floor_spec->view_floor : -1,
           floor_spec ? floor_spec->floor_zone : -1,
           thing_lane ? thing_lane->view_square : -1, thing_list_empty,
           wall ? wall->runtime_dst_x : -1,
           wall ? wall->runtime_dst_x + wall->runtime_width - 1 : -1,
           zone_overlaps);
    return wall && floor_spec && thing_lane && zone_overlaps &&
           floor_spec->relative_depth == 1 && floor_spec->relative_lateral == 1 &&
           thing_lane->relative_depth == 1 && thing_lane->relative_lateral == 1 &&
           ordinal == 0 && thing_list_empty;
}

static int later_right_side_passes_are_unornamented(M11_GameViewState* game,
                                                    int x, int y, int direction)
{
    int saved_x = game->world.party.mapX;
    int saved_y = game->world.party.mapY;
    int saved_direction = game->world.party.direction;
    int clear;
    game->world.party.mapX = x;
    game->world.party.mapY = y;
    game->world.party.direction = direction;
    /* F0128 dispatches F0120_D2R, F0123_D1R, then F0126_D0R after F0679.
     * The corridor branches can run F0108/F0115, so raw C708 pixels are an
     * oracle only when these later right-side floor-ornament routes are idle. */
    clear = M11_GameView_GetFloorOrnamentOrdinal(game, 2, 1) == 0 &&
            M11_GameView_GetFloorOrnamentOrdinal(game, 1, 1) == 0 &&
            M11_GameView_GetFloorOrnamentOrdinal(game, 0, 1) == 0;
    game->world.party.mapX = saved_x;
    game->world.party.mapY = saved_y;
    game->world.party.direction = saved_direction;
    return clear;
}

static int forward_center_lane_is_corridor(const M11_GameViewState* game,
                                           int x, int y, int direction)
{
    int fx, fy, rx, ry;
    int depth;
    direction_vectors(direction, &fx, &fy, &rx, &ry);
    (void)rx;
    (void)ry;
    for (depth = 1; depth <= 4; ++depth) {
        if (element_at(game, x + depth * fx, y + depth * fy) !=
            DUNGEON_ELEMENT_CORRIDOR) {
            return 0;
        }
    }
    return 1;
}

static int find_forward_pair(M11_GameViewState* game)
{
    int map_index;
    for (map_index = 0; map_index < (int)game->world.dungeon->header.mapCount;
         ++map_index) {
        const struct DungeonMapDesc_Compat* map =
            &game->world.dungeon->maps[map_index];
        int y;
        game->world.party.mapIndex = map_index;
        for (y = 0; y < (int)map->height; ++y) {
            int x;
            for (x = 0; x < (int)map->width; ++x) {
                int direction;
                for (direction = 0; direction < 4; ++direction) {
                int fx, fy, rx, ry;
                int nx, ny;
                direction_vectors(direction, &fx, &fy, &rx, &ry);
                nx = x + fx;
                ny = y + fy;
                if (element_at(game, x, y) != DUNGEON_ELEMENT_CORRIDOR ||
                    element_at(game, nx, ny) != DUNGEON_ELEMENT_CORRIDOR ||
                    !forward_center_lane_is_corridor(game, x, y, direction) ||
                    !forward_center_lane_is_corridor(game, nx, ny, direction) ||
                    !d2_pair_is_real_wall_pose(game, x, y, direction) ||
                    !d2_pair_is_real_wall_pose(game, nx, ny, direction) ||
                    !d1_side_pair_is_corridor(game, x, y, direction) ||
                    !d1_side_pair_is_corridor(game, nx, ny, direction) ||
                    /* ReDMCSB DUNVIEW.C F0128 calls F0126_D0R after F0679.
                     * An adjacent D0 wall draws C717 over C708; an empty
                     * D0 corridor gives F0115 no thing chain to overpaint
                     * the D2R2 wall receipt. */
                    !d0_side_pair_is_empty_corridor(game, x, y, direction) ||
                    !d0_side_pair_is_empty_corridor(game, nx, ny, direction) ||
                    !later_right_side_passes_are_unornamented(
                        game, x, y, direction) ||
                    !later_right_side_passes_are_unornamented(
                        game, nx, ny, direction) ||
                    !later_f0128_ceiling_pits_are_inactive(
                        game, x, y, direction) ||
                    !later_f0128_ceiling_pits_are_inactive(
                        game, nx, ny, direction) ||
                    !d2_side_pair_is_corridor(game, x, y, direction) ||
                    !d2_side_pair_is_corridor(game, nx, ny, direction)) {
                    continue;
                }
                    game->world.party.mapX = x;
                    game->world.party.mapY = y;
                    game->world.party.direction = direction;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int check_side_zone(const M11_GameViewState* game,
                           const unsigned char* framebuffer,
                           int rel_side,
                           const char* phase)
{
    const DM1_ViewportWallDrawSpec* spec =
        dm1_viewport_3d_get_side_wall_draw_spec_for_rel(2, rel_side);
    const M11_AssetSlot* asset;
    DM1_WallSetIndex selected;
    unsigned int graphic;
    bool target_flip = false;
    int expected = 0;
    int matched = 0;
    int first_mismatch_x = -1;
    int first_mismatch_y = -1;
    int first_mismatch_source = -1;
    int first_mismatch_rendered = -1;
    int mismatch_by_x[8] = { 0 };
    int mismatch_by_source[16] = { 0 };
    int expected_by_source[16] = { 0 };
    int yy;
    int ok = 1;
    char label[160];

    snprintf(label, sizeof(label), "%s D2%s2 C70%d spec", phase,
             rel_side < 0 ? "L" : "R", rel_side < 0 ? 7 : 8);
    ok &= expect_true(label, spec && spec->runtime_width == 8 &&
                      spec->runtime_height == 52 && spec->runtime_dst_y == 24);
    if (!ok || !spec) return 0;
    selected = dm1_viewport_3d_select_wall_bitmap(
        spec,
        dm1_viewport_3d_use_flipped_walls_pc34(game->world.party.mapX,
                                                game->world.party.mapY,
                                                game->world.party.direction) != 0,
        &target_flip);
    graphic = (unsigned int)dm1_v1_graphic_wallset0_index_pc34((int)selected);
    /* ReDMCSB DUNVIEW.C F0096 materializes the active map's wallset before
     * F0678/F0679 consume C06/C05. Match M11's per-map asset selection;
     * wallset 0 is not a valid stand-in for every real map. */
    if (graphic >= DM1_WALLSET_FIRST_GRAPHIC &&
        graphic < DM1_WALLSET_FIRST_GRAPHIC + DM1_WALLSET_GRAPHIC_COUNT) {
        graphic += (unsigned int)game->world.dungeon->maps[
            game->world.party.mapIndex].wallSet * DM1_WALLSET_GRAPHIC_COUNT;
    }
    printf("source receipt %s D2%s2: ReDMCSB F067%d C70%d wall=%d "
           "GRAPHICS.DAT=%u src=0,0,%d,%d dst=%d,%d flip=%d\n",
           phase, rel_side < 0 ? "L" : "R", rel_side < 0 ? 8 : 9,
           rel_side < 0 ? 7 : 8, (int)selected, graphic,
           spec->runtime_width, spec->runtime_height,
           spec->runtime_dst_x, spec->runtime_dst_y, target_flip ? 1 : 0);
    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader, graphic);
    snprintf(label, sizeof(label), "%s D2%s2 GRAPHICS.DAT wall asset", phase,
             rel_side < 0 ? "L" : "R");
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                      asset->width == spec->runtime_width &&
                      asset->height == spec->runtime_height);
    if (!ok || !asset || !asset->pixels) return 0;
    /* ReDMCSB F0128 calls F0678/F0679 before F0119/F0120. The selected
     * pose makes D2L/D2R corridors, so their later callbacks do not draw a
     * wall panel over C707/C708. F0104/F0105 then leave every non-C10 source
     * pixel from the real D2L2/D2R2 bitmap observable in the framebuffer. */
    for (yy = 0; yy < spec->runtime_height; ++yy) {
        int xx;
        for (xx = 0; xx < spec->runtime_width; ++xx) {
            int source_x = target_flip ? spec->runtime_width - 1 - xx : xx;
            unsigned char source = (unsigned char)(asset->pixels[
                yy * (int)asset->width + source_x] & 0x0f);
            unsigned char rendered = (unsigned char)M11_FB_DECODE_INDEX(
                framebuffer[(VIEWPORT_Y + spec->runtime_dst_y + yy) * FB_W +
                            spec->runtime_dst_x + xx]);
            if (source == 10) continue;
            ++expected_by_source[source];
            ++expected;
            if (rendered == source) {
                ++matched;
            } else if (first_mismatch_x < 0) {
                first_mismatch_x = xx;
                first_mismatch_y = yy;
                first_mismatch_source = source;
                first_mismatch_rendered = rendered;
            }
            if (rendered != source) {
                if (xx < 8) ++mismatch_by_x[xx];
                ++mismatch_by_source[source];
            }
        }
    }
    snprintf(label, sizeof(label), "%s D2%s2 opaque wall pixels persist", phase,
             rel_side < 0 ? "L" : "R");
    printf("%s D2%s2 map=%d,%d dir=%d gfx=%d flip=%d opaque=%d/%d\n",
           phase, rel_side < 0 ? "L" : "R", game->world.party.mapX,
           game->world.party.mapY, game->world.party.direction,
           graphic, target_flip ? 1 : 0,
           matched, expected);
    printf("%s D2%s2 expected by source: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
           phase, rel_side < 0 ? "L" : "R", expected_by_source[0],
           expected_by_source[1], expected_by_source[2], expected_by_source[3],
           expected_by_source[4], expected_by_source[5], expected_by_source[6],
           expected_by_source[7], expected_by_source[8], expected_by_source[9],
           expected_by_source[10], expected_by_source[11], expected_by_source[12],
           expected_by_source[13], expected_by_source[14], expected_by_source[15]);
    if (first_mismatch_x >= 0) {
        printf("%s D2%s2 first mismatch local=%d,%d source=%d rendered=%d\n",
               phase, rel_side < 0 ? "L" : "R", first_mismatch_x,
               first_mismatch_y, first_mismatch_source, first_mismatch_rendered);
        printf("%s D2%s2 mismatches by x: %d %d %d %d %d %d %d %d\n",
               phase, rel_side < 0 ? "L" : "R", mismatch_by_x[0],
               mismatch_by_x[1], mismatch_by_x[2], mismatch_by_x[3],
               mismatch_by_x[4], mismatch_by_x[5], mismatch_by_x[6],
               mismatch_by_x[7]);
        printf("%s D2%s2 mismatches by source: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               phase, rel_side < 0 ? "L" : "R", mismatch_by_source[0],
               mismatch_by_source[1], mismatch_by_source[2], mismatch_by_source[3],
               mismatch_by_source[4], mismatch_by_source[5], mismatch_by_source[6],
               mismatch_by_source[7], mismatch_by_source[8], mismatch_by_source[9],
               mismatch_by_source[10], mismatch_by_source[11], mismatch_by_source[12],
               mismatch_by_source[13], mismatch_by_source[14], mismatch_by_source[15]);
    }
    ok &= expect_true(label, expected > 0 && matched == expected);
    return ok;
}

static void print_d2r2_trace(const char* phase)
{
    M11_D2R2WriteTrace trace;
    unsigned int i;
    M11_GameView_ProbeGetD2R2WriteTrace(&trace);
    printf("%s C708 write trace c707=%d/%d c708=%d/%d", phase,
           trace.c707_graphic, trace.c707_flipped,
           trace.c708_graphic, trace.c708_flipped);
    for (i = 0; i < trace.count; ++i) {
        printf(" %08x/%u", trace.hashes[i], trace.checkpoint_c708_matches[i]);
    }
    printf("\n");
    for (i = 0; i < trace.side_blit_count; ++i) {
        printf("%s C708 side blit rel=%d,%d gfx=%d hash=%08x\n", phase,
               trace.side_blit_rel_forward[i], trace.side_blit_rel_side[i],
               trace.side_blit_graphic[i], trace.side_blit_hashes[i]);
    }
    printf("%s C708 blit material gfx=%d size=%d,%d key=%d writes"
           " %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
           phase, trace.c708_materialized_graphic,
           trace.c708_source_width, trace.c708_source_height,
           trace.c708_transparent_color,
           trace.c708_source_index_writes[0], trace.c708_source_index_writes[1],
           trace.c708_source_index_writes[2], trace.c708_source_index_writes[3],
           trace.c708_source_index_writes[4], trace.c708_source_index_writes[5],
           trace.c708_source_index_writes[6], trace.c708_source_index_writes[7],
           trace.c708_source_index_writes[8], trace.c708_source_index_writes[9],
           trace.c708_source_index_writes[10], trace.c708_source_index_writes[11],
           trace.c708_source_index_writes[12], trace.c708_source_index_writes[13],
           trace.c708_source_index_writes[14], trace.c708_source_index_writes[15]);
    printf("%s C708 raw samples nonnibble=%u", phase,
           trace.c708_source_non_nibble_writes);
    for (i = 0; i < 16; ++i) {
        printf(" %02x", trace.c708_source_raw_sample[i]);
    }
    printf("\n");
    printf("%s C708 flipped helper immediate=%u/%u\n", phase,
           trace.c708_flipped_immediate_matched,
           trace.c708_flipped_immediate_expected);
}

int main(int argc, char** argv)
{
    M11_GameViewState game;
    unsigned char before[FB_W * FB_H];
    unsigned char after[FB_W * FB_H];
    const char* data_dir;
    int start_x;
    int start_y;
    int ok = 1;

    if (argc != 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    data_dir = argv[1];
    M11_GameView_Init(&game);
    if (!M11_GameView_StartDm1(&game, data_dir)) {
        fprintf(stderr, "FAIL could not open hash-verified DM1 data from %s\n", data_dir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    game.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    game.showDebugHUD = 0;
    /* The achievement toast is a host overlay, not a DUNVIEW.C pass. Keep
     * the real DM1 framebuffer observable through F0678/F0679. */
    memset(&game.retroAchievementsOverlay, 0,
           sizeof(game.retroAchievementsOverlay));
    game.world.party.championCount = 0;
    ok &= expect_true("found real forward-walk D2L2/D2R2 wall pair",
                      find_forward_pair(&game));
    if (ok) {
        start_x = game.world.party.mapX;
        start_y = game.world.party.mapY;
        printf("real pose before forward map=%d x=%d y=%d dir=%d\n",
               game.world.party.mapIndex, start_x, start_y,
               game.world.party.direction);
        print_later_f0128_ceiling_pit_receipt(&game, "before forward");
        ok &= expect_true("before forward F0121 D2C F0108 disjoint from C708",
                          print_d2c_f0108_receipt(&game, "before forward"));
        ok &= expect_true("before forward F0122 D1L F0108/F0115 exclude C708",
                          print_d1l_f0122_receipt(&game, "before forward"));
        ok &= expect_true("before forward F0123 D1R F0108/F0115 are idle over C708",
                          print_d1r_f0123_receipt(&game, "before forward"));
        memset(before, 0, sizeof(before));
        M11_GameView_Draw(&game, before, FB_W, FB_H);
        print_d2r2_trace("before forward");
        ok &= check_side_zone(&game, before, -2, "before forward");
        ok &= check_side_zone(&game, before, 2, "before forward");
        ok &= expect_true("real forward input advances party",
                          M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP) ==
                              M11_GAME_INPUT_REDRAW &&
                          (game.world.party.mapX != start_x ||
                           game.world.party.mapY != start_y));
        memset(after, 0, sizeof(after));
        print_later_f0128_ceiling_pit_receipt(&game, "after forward");
        ok &= expect_true("after forward F0121 D2C F0108 disjoint from C708",
                          print_d2c_f0108_receipt(&game, "after forward"));
        ok &= expect_true("after forward F0122 D1L F0108/F0115 exclude C708",
                          print_d1l_f0122_receipt(&game, "after forward"));
        ok &= expect_true("after forward F0123 D1R F0108/F0115 are idle over C708",
                          print_d1r_f0123_receipt(&game, "after forward"));
        M11_GameView_Draw(&game, after, FB_W, FB_H);
        print_d2r2_trace("after forward");
        ok &= check_side_zone(&game, after, -2, "after forward");
        ok &= check_side_zone(&game, after, 2, "after forward");
    }
    M11_GameView_Shutdown(&game);
    printf("%s DM1 V1 D2L2/D2R2 forward wall-opacity probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
