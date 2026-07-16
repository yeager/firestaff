#include "csb_v1_f0797_startend_entrance_micro_dungeon_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static CSB_V1_StartEndEntranceBoundaryReceipt_PC34 make_entrance_boundary(void)
{
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.accepted_stage_mask = CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34;
    receipt.real_asset_matched = 1;
    receipt.host_view_consumed = 1;
    receipt.host_draw_consumed = 1;
    receipt.draw_consumes_receipt_only = 1;
    receipt.no_synthetic_payloads = 1;
    receipt.no_fallback_graphics = 1;
    receipt.route_wrappers_retired = 1;
    receipt.source_evidence = "test F0439 entrance boundary receipt";
    return receipt;
}

static CSB_V1_F0797_EntranceMicroDungeonFacts_PC34 make_complete_facts(void)
{
    CSB_V1_F0797_EntranceMicroDungeonFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.entrance_map_index = CSB_V1_F0797_ENTRANCE_MAP_INDEX_PC34;
    facts.map_width = CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34;
    facts.map_height = CSB_V1_F0797_MICRO_DUNGEON_HEIGHT_PC34;
    facts.square_count = CSB_V1_F0797_MICRO_DUNGEON_SQUARE_COUNT_PC34;
    facts.wall_square_count = CSB_V1_F0797_MICRO_DUNGEON_WALL_COUNT_PC34;
    facts.corridor_square_count =
        CSB_V1_F0797_MICRO_DUNGEON_CORRIDOR_COUNT_PC34;
    facts.corridor_square_mask =
        csb_v1_f0797_entrance_micro_dungeon_corridor_mask_pc34();
    facts.current_map_data_row_count = CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34;
    facts.draw_floor_and_ceiling_requested = 1;
    facts.current_map_pointer_owned_by_micro_dungeon = 1;
    facts.current_map_not_loaded_dungeon = 1;
    facts.source_wall_fill_reviewed = 1;
    facts.source_corridor_row_reviewed = 1;
    facts.source_corridor_spur_reviewed = 1;
    facts.draw_cpsf_route_reviewed = 1;
    facts.view_direction = CSB_V1_F0797_VIEW_DIRECTION_SOUTH_PC34;
    facts.view_x = CSB_V1_F0797_VIEW_X_PC34;
    facts.view_y = CSB_V1_F0797_VIEW_Y_PC34;
    facts.no_loaded_dungeon_substitute = 1;
    facts.no_synthetic_viewport_pixels = 1;
    facts.no_legacy_micro_dungeon_wrapper = 1;
    facts.entrance_boundary = make_entrance_boundary();
    return facts;
}

static void test_accepts_source_micro_dungeon_shape(void)
{
    CSB_V1_F0797_EntranceMicroDungeonFacts_PC34 facts =
        make_complete_facts();
    CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34 receipt;

    CHECK(F0797_STARTEND_DrawEntranceMicroDungeon(&facts, &receipt) == 1);
    CHECK(receipt.valid == 1);
    CHECK(receipt.entrance_map_index == 255);
    CHECK(receipt.map_width == 5);
    CHECK(receipt.map_height == 5);
    CHECK(receipt.wall_square_count == 19);
    CHECK(receipt.corridor_square_count == 6);
    CHECK(receipt.corridor_square_mask == ((uint32_t)0x7c80u));
    CHECK(receipt.view_direction == CSB_V1_F0797_VIEW_DIRECTION_SOUTH_PC34);
    CHECK(receipt.view_x == 2);
    CHECK(receipt.view_y == 0);
    CHECK(receipt.entrance_boundary_consumed == 1);
    CHECK(receipt.no_loaded_dungeon_substitute == 1);
    CHECK(receipt.no_synthetic_viewport_pixels == 1);
    CHECK(receipt.no_legacy_micro_dungeon_wrapper == 1);
}

static void test_square_helper_matches_source_corridor_layout(void)
{
    int x;
    int y;
    int corridor_count = 0;
    int wall_count = 0;

    for (y = 0; y < CSB_V1_F0797_MICRO_DUNGEON_HEIGHT_PC34; ++y) {
        for (x = 0; x < CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34; ++x) {
            int kind =
                csb_v1_f0797_entrance_micro_dungeon_square_kind_pc34(x, y);
            if (y == 2 || (x == 2 && y == 1)) {
                CHECK(kind ==
                      CSB_V1_F0797_MICRO_DUNGEON_SQUARE_CORRIDOR_PC34);
                ++corridor_count;
            } else {
                CHECK(kind == CSB_V1_F0797_MICRO_DUNGEON_SQUARE_WALL_PC34);
                ++wall_count;
            }
        }
    }

    CHECK(csb_v1_f0797_entrance_micro_dungeon_square_kind_pc34(-1, 0) == -1);
    CHECK(csb_v1_f0797_entrance_micro_dungeon_square_kind_pc34(5, 0) == -1);
    CHECK(corridor_count == CSB_V1_F0797_MICRO_DUNGEON_CORRIDOR_COUNT_PC34);
    CHECK(wall_count == CSB_V1_F0797_MICRO_DUNGEON_WALL_COUNT_PC34);
}

static void test_rejects_substitutes_and_unverified_routes(void)
{
    CSB_V1_F0797_EntranceMicroDungeonFacts_PC34 facts =
        make_complete_facts();
    CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34 receipt;

    facts.current_map_not_loaded_dungeon = 0;
    CHECK(F0797_STARTEND_DrawEntranceMicroDungeon(&facts, &receipt) == 0);
    CHECK(receipt.no_loaded_dungeon_substitute == 1);
    CHECK(receipt.no_synthetic_viewport_pixels == 1);

    facts = make_complete_facts();
    facts.corridor_square_mask ^= 1u << 0;
    CHECK(F0797_STARTEND_DrawEntranceMicroDungeon(&facts, &receipt) == 0);

    facts = make_complete_facts();
    facts.view_y = 1;
    CHECK(F0797_STARTEND_DrawEntranceMicroDungeon(&facts, &receipt) == 0);

    facts = make_complete_facts();
    facts.entrance_boundary.no_fallback_graphics = 0;
    CHECK(F0797_STARTEND_DrawEntranceMicroDungeon(&facts, &receipt) == 0);

    facts = make_complete_facts();
    facts.no_legacy_micro_dungeon_wrapper = 0;
    CHECK(F0797_STARTEND_DrawEntranceMicroDungeon(&facts, &receipt) == 0);
}

static void test_evidence_string(void)
{
    const char *evidence =
        csb_v1_f0797_entrance_micro_dungeon_source_evidence_pc34();

    check_contains(evidence, "ENTRANCE.C:57-81");
    check_contains(evidence, "corridor row y=2 plus spur square (2,1)");
    check_contains(evidence, "F0128_DUNGEONVIEW_Draw_CPSF");
    check_contains(evidence, "opening doors");
}

int main(void)
{
    test_accepts_source_micro_dungeon_shape();
    test_square_helper_matches_source_corridor_layout();
    test_rejects_substitutes_and_unverified_routes();
    test_evidence_string();
    return 0;
}
