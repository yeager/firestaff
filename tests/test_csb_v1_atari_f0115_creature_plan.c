#include "csb_v1_atari_f0115_creature_plan.h"
#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"
#include "dm1_v1_creature_render_pc34_compat.h"
#include "asset_loader_m11.h"
#include "memory_creature_ai_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check_cell_order(uint16_t word, int expected_count,
                            int expected_door_pass,
                            const uint8_t *expected_cells,
                            const uint8_t *expected_last_back)
{
    CSB_V1_AtariF0115CellOrder order;
    int result = csb_v1_atari_f0115_decode_cell_order(word, &order);
    int i;
    if (result != expected_count || order.door_front_pass != expected_door_pass)
        return 0;
    for (i = 0; i < expected_count; ++i) {
        if (order.view_cell[i] != expected_cells[i] ||
            order.drawing_last_back_row_cell[i] != expected_last_back[i])
            return 0;
        if (i + 1 < expected_count && order.remaining_after[i] == 0u)
            return 0;
        if (i + 1 == expected_count && order.remaining_after[i] != 0u)
            return 0;
    }
    return 1;
}

static int check_cell_order_unit(void)
{
    static const uint8_t c01[] = { 0u, 1u };
    static const uint8_t c01_last[] = { 0u, 1u };
    static const uint8_t c32[] = { 3u, 2u };
    static const uint8_t c32_last[] = { 0u, 1u };
    static const uint8_t all_cells[] = { 0u, 1u, 3u, 2u };
    static const uint8_t all_cells_last[] = { 0u, 1u, 0u, 0u };
    CSB_V1_AtariF0115CellOrder order;
    CSB_V1_AtariF0115PassEvent events[16];

    if (!check_cell_order(0x0218u, 2, 1, c01, c01_last) ||
        !check_cell_order(0x0349u, 2, 2, c32, c32_last) ||
        !check_cell_order(0x3421u, 4, 0, all_cells, all_cells_last)) {
        fprintf(stderr, "ReDMCSB MEDIA020 F0115 cell order mismatch\n");
        return 1;
    }
    if (csb_v1_atari_f0115_decode_cell_order(0u, &order) != 0 ||
        !order.alcove_only || order.count != 0u ||
        csb_v1_atari_f0115_decode_cell_order(0x0005u, &order) != -1 ||
        csb_v1_atari_f0115_decode_cell_order(0x00a8u, &order) != -1) {
        fprintf(stderr, "invalid/alcove ReDMCSB F0115 order handling mismatch\n");
        return 2;
    }
    if (csb_v1_atari_f0115_scaled_dimension(56u, 16u) != 28u ||
        csb_v1_atari_f0115_scaled_dimension(84u, 16u) != 42u ||
        csb_v1_atari_f0115_scaled_dimension(56u, 20u) != 35u ||
        csb_v1_atari_f0115_scaled_dimension(84u, 20u) != 52u ||
        csb_v1_atari_f0115_scaled_dimension(56u, 32u) != 56u ||
        csb_v1_atari_f0115_scale_for_depth(2) != 16u ||
        csb_v1_atari_f0115_scale_for_depth(1) != 20u ||
        csb_v1_atari_f0115_scale_for_depth(0) != 32u ||
        csb_v1_atari_f0115_scale_for_depth(-1) != 0u ||
        csb_v1_atari_f0115_scale_for_depth(3) != 0u) {
        fprintf(stderr, "ReDMCSB M078 scaled-dimension rounding mismatch\n");
        return 3;
    }
    {
        static const uint8_t normal_cells[] = {0u, 1u, 3u, 2u};
        static const uint8_t door1_cells[] = {0u, 1u};
        static const uint8_t door2_cells[] = {3u, 2u};
        const uint16_t orders[] = {0x3421u, 0x0218u, 0x0349u};
        const uint8_t *expected_cells[] = {
            normal_cells, door1_cells, door2_cells
        };
        const unsigned int expected_cells_count[] = {4u, 2u, 2u};
        const uint8_t expected_pass[] = {0u, 1u, 2u};
        unsigned int sample;

        for (sample = 0u; sample < 3u; ++sample) {
            const unsigned int cell_count = expected_cells_count[sample];
            const int count = csb_v1_atari_f0115_plan_pass_events(
                orders[sample], events,
                sizeof(events) / sizeof(events[0]));
            unsigned int cell;
            const unsigned int expected_total = cell_count * 3u + 1u +
                (expected_pass[sample] == 1u ? 0u : 1u);
            if (count != (int)expected_total) {
                fprintf(stderr,
                        "F0115 event count mismatch for order 0x%04x\n",
                        orders[sample]);
                return 6;
            }
            for (cell = 0u; cell < cell_count; ++cell) {
                static const uint8_t expected_layers[] = {
                    CSB_V1_ATARI_F0115_OBJECT_LAYER,
                    CSB_V1_ATARI_F0115_CREATURE_LAYER,
                    CSB_V1_ATARI_F0115_PROJECTILE_LAYER
                };
                unsigned int layer;
                for (layer = 0u; layer < 3u; ++layer) {
                    const CSB_V1_AtariF0115PassEvent *event =
                        &events[cell * 3u + layer];
                    if (event->view_cell != expected_cells[sample][cell] ||
                        event->layer != expected_layers[layer] ||
                        event->door_front_view_pass != expected_pass[sample] ||
                        event->remaining_view_cells !=
                            events[cell * 3u].remaining_view_cells ||
                        event->drawing_last_back_row_cell !=
                            events[cell * 3u].drawing_last_back_row_cell) {
                        fprintf(stderr,
                                "F0115 cell event order mismatch for order 0x%04x cell %u layer %u\n",
                                orders[sample], cell, layer);
                        return 7;
                    }
                }
                if (events[cell * 3u].remaining_view_cells !=
                        events[cell * 3u + 1u].remaining_view_cells ||
                    events[cell * 3u].remaining_view_cells !=
                        events[cell * 3u + 2u].remaining_view_cells ||
                    events[cell * 3u].drawing_last_back_row_cell !=
                        events[cell * 3u + 1u].drawing_last_back_row_cell ||
                    events[cell * 3u].drawing_last_back_row_cell !=
                        events[cell * 3u + 2u].drawing_last_back_row_cell) {
                    fprintf(stderr,
                            "F0115 cell metadata mismatch for order 0x%04x cell %u\n",
                            orders[sample], cell);
                    return 7;
                }
            }
            {
                const unsigned int post_index = cell_count * 3u;
                const unsigned int expected_post_count =
                    expected_pass[sample] == 1u ? 1u : 2u;
                if (events[post_index].layer !=
                        CSB_V1_ATARI_F0115_EXPLOSION_LAYER ||
                    events[post_index].view_cell !=
                        CSB_V1_ATARI_F0115_POST_CELL ||
                    events[post_index].door_front_view_pass !=
                        expected_pass[sample] ||
                    (expected_post_count == 2u &&
                     (events[post_index + 1u].layer !=
                          CSB_V1_ATARI_F0115_FLUXCAGE_LAYER ||
                      events[post_index + 1u].view_cell !=
                          CSB_V1_ATARI_F0115_POST_CELL ||
                      events[post_index + 1u].door_front_view_pass !=
                          expected_pass[sample]))) {
                    fprintf(stderr,
                            "F0115 post-cell event order mismatch for order 0x%04x\n",
                            orders[sample]);
                    return 8;
                }
            }
        }
        if (csb_v1_atari_f0115_plan_pass_events(0u, events,
                sizeof(events) / sizeof(events[0])) != 0 ||
            csb_v1_atari_f0115_plan_pass_events(0x3421u, events, 13u) != -1 ||
            csb_v1_atari_f0115_plan_pass_events(0x0005u, events,
                sizeof(events) / sizeof(events[0])) != -1) {
            fprintf(stderr, "F0115 pass event validation mismatch\n");
            return 9;
        }
    }
    for (int direction = 0; direction < 4; ++direction) {
        for (int view_cell = 0; view_cell < 4; ++view_cell) {
            for (unsigned int pile = 0u; pile < 8u; ++pile) {
                unsigned int shift_index = 99u;
                const unsigned int absolute_cell =
                    ((unsigned int)direction + (unsigned int)view_cell) & 3u;
                const unsigned int expected =
                    ((absolute_cell & 1u) << 3) + pile;
                if (!csb_v1_atari_f0115_object_shift_index(
                        direction, view_cell, pile, &shift_index) ||
                    shift_index != expected) {
                    fprintf(stderr,
                            "ReDMCSB F0115 G0217 shift index mismatch "
                            "direction=%d view-cell=%d pile=%u\n",
                            direction, view_cell, pile);
                    return 4;
                }
            }
        }
    }
    {
        unsigned int shift_index = 0u;
        if (csb_v1_atari_f0115_object_shift_index(4, 0, 0u,
                                                   &shift_index) ||
            csb_v1_atari_f0115_object_shift_index(0, 4, 0u,
                                                   &shift_index) ||
            csb_v1_atari_f0115_object_shift_index(0, 0, 8u,
                                                   &shift_index)) {
            fprintf(stderr, "invalid Atari G0217 shift index was accepted\n");
            return 5;
        }
    }
    puts("ReDMCSB MEDIA020 F0115 cell orders verified");
    return 0;
}

/* This test consumes original Atari ST GRAPHICS.DAT (or a supported virtual
 * media path such as archive::member). It returns 77 when media is absent. */
int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--unit") == 0)
        return check_cell_order_unit();
    const char *path = argc > 1 ? argv[1] : getenv("FIRESTAFF_CSB_ATARI_GRAPHICS");
    CSB_AtariStLoader loader;
    CSB_V1_AtariF0115CreatureInput input = {0};
    CSB_V1_AtariF0115CreaturePlan plan;
    uint8_t *c558;
    int result;

    if (check_cell_order_unit() != 0) return 6;

    if (!path || !path[0]) return 77;
    csb_atari_st_graphics_loader_init(&loader);
    if (!csb_atari_st_graphics_loader_open(&loader, path) ||
        loader.item_count != 563u ||
        loader.items[558u].decompressed_size != CSB_V1_ATARI_F0115_C558_SIZE) {
        csb_atari_st_graphics_loader_close(&loader);
        fprintf(stderr, "cannot read authentic Atari C558 from %s\n", path);
        return 1;
    }
    c558 = (uint8_t *)malloc(CSB_V1_ATARI_F0115_C558_SIZE);
    if (!c558 || csb_atari_st_graphics_loader_read_item(
            &loader, 558u, c558, CSB_V1_ATARI_F0115_C558_SIZE) !=
            CSB_V1_ATARI_F0115_C558_SIZE) {
        free(c558);
        csb_atari_st_graphics_loader_close(&loader);
        return 2;
    }

    /* ReDMCSB G0243 Atari Scorpion profile: size 2, GraphicInfo 0x623D.
     * The fixture's C04 state describes one centered creature. */
    input.c558 = c558;
    input.c558_size = CSB_V1_ATARI_F0115_C558_SIZE;
    input.creature_type = 0;
    input.creature_count_minus_one = 0;
    input.cells = CSB_V1_ATARI_F0115_SINGLE_CENTERED;
    input.directions = 2u; /* Face the party: F0115 front pose, delta 2. */
    input.creature_size = 2;
    input.graphic_info = 0x623d;
    input.view_square = 0; /* D3C */
    input.view_cell = 0;

    result = csb_v1_atari_f0115_plan_creature(&input, &plan);
    if (result != 1 || plan.coordinate_set != 1 ||
        plan.coordinate_cell != 4 || plan.anchor_x != 111 ||
        plan.anchor_y != 75 || plan.bitmap_index != 446 ||
        plan.source_depth != 2 || plan.source_byte_width != 56 ||
        plan.height != 84 || plan.transparent_color != 13 ||
        plan.palette_transparent_color != 2 ||
        !plan.requires_derived_depth_bitmap) {
        fprintf(stderr, "Atari F0115 centered D3C mismatch: result=%d set=%d cell=%d anchor=%d,%d graphic=%d depth=%d byte-width=%d height=%d transparent=%d mapped-transparent=%d derived=%d\n",
                result, plan.coordinate_set, plan.coordinate_cell,
                plan.anchor_x, plan.anchor_y, plan.bitmap_index,
                plan.source_depth, plan.source_byte_width, plan.height,
                plan.transparent_color, plan.palette_transparent_color,
                plan.requires_derived_depth_bitmap);
        free(c558);
        csb_atari_st_graphics_loader_close(&loader);
        return 3;
    }

    /* D0C is explicitly skipped by F0115. G0224 row 9 is D0L; it must not
     * accidentally be consumed as D0C. D0L folds to row 9 and uses depth 0. */
    input.view_square = 9;
    if (csb_v1_atari_f0115_plan_creature(&input, &plan) != 0) {
        fprintf(stderr, "Atari F0115 must skip creature draw on D0C\n");
        free(c558);
        csb_atari_st_graphics_loader_close(&loader);
        return 4;
    }
    input.view_square = 10;
    result = csb_v1_atari_f0115_plan_creature(&input, &plan);
    if (result != 1 || plan.anchor_x != 57 || plan.anchor_y != 130 ||
        plan.source_depth != 0 || plan.transparent_color != 13 ||
        plan.palette_transparent_color != 13 ||
        plan.requires_derived_depth_bitmap) {
        fprintf(stderr, "Atari F0115 D0L fold/depth plan disagrees with C558\n");
        free(c558);
        csb_atari_st_graphics_loader_close(&loader);
        return 5;
    }

    /* Walk original C558 poses for every CSB creature profile. The test
     * intentionally uses authentic type/size/GraphicInfo tables and checks
     * every visible M618 selection through Firestaff's real Atari decoder.
     * Archive decompressed_size is packed storage size, not pixel dimensions. */
    {
        /* MEDIA389 DUNGEON.C G0243 Attributes & C03_SIZE mask. */
        static const uint8_t csb_atari_creature_sizes[27] = {
            2u, 0u, 0u, 0u, 1u, 1u, 0u, 0u, 0u,
            2u, 0u, 2u, 0u, 2u, 0u, 1u, 0u, 0u,
            0u, 0u, 2u, 2u, 0u, 2u, 2u, 2u, 2u
        };
        M11_AssetLoader assets;
        unsigned int plans_checked = 0u;
        unsigned int side_plans = 0u;
        unsigned int back_plans = 0u;
        unsigned int attack_plans = 0u;
        unsigned int special_d2_plans = 0u;
        memset(&assets, 0, sizeof(assets));
        if (!M11_AssetLoader_InitCsbAtariStFromFile(&assets, path) ||
            !assets.csbAtariSt || assets.graphicCount != 563u) {
            M11_AssetLoader_Shutdown(&assets);
            free(c558);
            csb_atari_st_graphics_loader_close(&loader);
            fprintf(stderr, "cannot initialize original CSB Atari M618 decoder\n");
            return 10;
        }
        for (int creature_type = 0; creature_type < 27; ++creature_type) {
            const struct CreatureBehaviorProfile_Compat *profile =
                CREATURE_GetProfile_Compat(creature_type);
            const uint8_t *creature_aspect = c558 +
                CSB_V1_ATARI_F0115_G0219_OFFSET +
                (size_t)creature_type * CSB_V1_ATARI_F0115_G0219_STRIDE;
            if (!profile || profile->creatureType != creature_type ||
                (profile->attributes & 3) != csb_atari_creature_sizes[creature_type]) {
                fprintf(stderr, "missing CSB G0243 behavior profile for creature %d\n",
                        creature_type);
                M11_AssetLoader_Shutdown(&assets);
                free(c558);
                csb_atari_st_graphics_loader_close(&loader);
                return 7;
            }
            for (int view_square = 0; view_square <= 6; view_square += 3) {
                for (int party_direction = 0; party_direction < 4; ++party_direction) {
                    for (int direction_delta = 0; direction_delta < 4; ++direction_delta) {
                        for (int attacking = 0; attacking <= 1; ++attacking) {
                          for (int flip = 0; flip <= 1; ++flip) {
                            const unsigned int facing =
                                (unsigned int)((party_direction - direction_delta) & 3);
                            unsigned int bitmap_index;
                            unsigned int graphic_info;
                            unsigned int expected_offset = 0u;
                            int expected_side;
                            int expected_back;
                            int expected_attack;
                            int expected_special_d2;
                            int expected_mirror;
                            unsigned short bitmap_width = 0u;
                            unsigned short bitmap_height = 0u;
                            const M11_AssetSlot *bitmap_asset;
                            const uint8_t frame = (uint8_t)(
                                (attacking ? 0x80u : 0u) | (flip ? 0x40u : 0u));

                            input.creature_type = creature_type;
                            input.creature_count_minus_one = 0;
                            input.cells = CSB_V1_ATARI_F0115_SINGLE_CENTERED;
                            input.directions = (uint16_t)facing;
                            memset(input.aspect, 0, sizeof(input.aspect));
                            input.aspect[0] = frame;
                            input.creature_size = profile->attributes & 3;
                            graphic_info = dm1_creature_graphic_info(creature_type);
                            input.graphic_info = (uint16_t)graphic_info;
                            input.party_direction = party_direction;
                            input.view_square = view_square;
                            input.view_cell = 2;
                            input.remaining_view_cells = 0;
                            input.drawing_last_back_row_cell = 0;
                            input.door_front_view_pass = 0;

                            result = csb_v1_atari_f0115_plan_creature(&input, &plan);
                            if (result < 0) {
                                fprintf(stderr,
                                        "invalid authentic F0115 plan type=%d square=%d party=%d delta=%d attacking=%d\n",
                                        creature_type, view_square, party_direction,
                                        direction_delta, attacking);
                                M11_AssetLoader_Shutdown(&assets);
                                free(c558);
                                csb_atari_st_graphics_loader_close(&loader);
                                return 8;
                            }
                            if (result == 0) continue; /* C558 says not visible. */
                            expected_side = (graphic_info & 0x0008u) &&
                                            (direction_delta & 1);
                            expected_back = !expected_side &&
                                            (graphic_info & 0x0010u) &&
                                            direction_delta == 0;
                            expected_attack = !expected_side && !expected_back &&
                                              attacking &&
                                              (graphic_info & 0x0020u);
                            expected_special_d2 = view_square >= 3 && view_square <= 5 &&
                                (graphic_info & 0x0080u) && !expected_side &&
                                !expected_back && !expected_attack;
                            if (expected_side) expected_offset = 1u;
                            else if (expected_back)
                                expected_offset = (graphic_info & 0x0008u) ? 2u : 1u;
                            else if (expected_attack)
                                expected_offset = 1u +
                                    ((graphic_info & 0x0008u) != 0u) +
                                    ((graphic_info & 0x0010u) != 0u);
                            expected_mirror = expected_side
                                ? direction_delta == 1
                                : expected_special_d2
                                    ? (graphic_info & 0x0100u) != 0u
                                    : expected_attack
                                        ? flip != 0
                                        : !expected_back &&
                                          (graphic_info & 0x0004u) && flip;
                            if (plan.uses_side_bitmap != expected_side ||
                                plan.uses_back_bitmap != expected_back ||
                                plan.uses_attack_bitmap != expected_attack ||
                                plan.uses_special_d2_front != expected_special_d2 ||
                                plan.bitmap_index != (int)(446u +
                                    ((unsigned int)creature_aspect[0] << 8 |
                                     creature_aspect[1]) + expected_offset) ||
                                plan.mirrored != expected_mirror) {
                                fprintf(stderr,
                                        "F0115 pose selection differs from source type=%d square=%d delta=%d attack=%d flip=%d bitmap=%d mirror=%d\n",
                                        creature_type, view_square, direction_delta,
                                        attacking, flip, plan.bitmap_index,
                                        plan.mirrored);
                                M11_AssetLoader_Shutdown(&assets);
                                free(c558);
                                csb_atari_st_graphics_loader_close(&loader);
                                return 11;
                            }
                            ++plans_checked;
                            if (expected_side) ++side_plans;
                            if (expected_back) ++back_plans;
                            if (expected_attack) ++attack_plans;
                            if (expected_special_d2) ++special_d2_plans;
                            bitmap_index = (unsigned int)plan.bitmap_index;
                            if (bitmap_index < 446u || bitmap_index >= 563u ||
                                !M11_AssetLoader_QuerySize(&assets, bitmap_index,
                                                          &bitmap_width,
                                                          &bitmap_height) ||
                                bitmap_width != (unsigned short)(plan.source_byte_width * 2) ||
                                bitmap_height != (unsigned short)plan.height ||
                                !(bitmap_asset = M11_AssetLoader_Load(&assets,
                                                                      bitmap_index)) ||
                                !bitmap_asset->loaded || !bitmap_asset->pixels ||
                                bitmap_asset->width != bitmap_width ||
                                bitmap_asset->height != bitmap_height) {
                                fprintf(stderr,
                                        "authentic F0115 pose selects missing/mismatched M618 asset type=%d square=%d delta=%d attack=%d bitmap=%u expected=%dx%d\n",
                                        creature_type, view_square, direction_delta,
                                        attacking, bitmap_index,
                                        plan.source_byte_width * 2, plan.height);
                                M11_AssetLoader_Shutdown(&assets);
                                free(c558);
                                csb_atari_st_graphics_loader_close(&loader);
                                return 9;
                            }
                          }
                        }
                    }
                }
            }
        }
        M11_AssetLoader_Shutdown(&assets);
        if (plans_checked == 0u || side_plans == 0u || back_plans == 0u ||
            attack_plans == 0u || special_d2_plans == 0u) {
            fprintf(stderr,
                    "F0115 real-media pose matrix did not cover all source branches: plans=%u side=%u back=%u attack=%u specialD2=%u\n",
                    plans_checked, side_plans, back_plans, attack_plans,
                    special_d2_plans);
            free(c558);
            csb_atari_st_graphics_loader_close(&loader);
            return 12;
        }
        printf("verified %u authentic Atari creature plans (side=%u back=%u attack=%u specialD2=%u)\n",
               plans_checked, side_plans, back_plans, attack_plans,
               special_d2_plans);
    }

    free(c558);
    csb_atari_st_graphics_loader_close(&loader);
    printf("authentic Atari C558 F0115 creature plans verified: %s\n", path);
    return 0;
}
