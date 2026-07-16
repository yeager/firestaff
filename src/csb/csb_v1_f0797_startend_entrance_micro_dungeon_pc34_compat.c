#include "csb_v1_f0797_startend_entrance_micro_dungeon_pc34_compat.h"

#include <string.h>

static int csb_v1_f0797_square_index_pc34(int x, int y)
{
    return y * CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34 + x;
}

uint32_t csb_v1_f0797_entrance_micro_dungeon_corridor_mask_pc34(void)
{
    uint32_t mask = 0u;
    int x;

    for (x = 0; x < CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34; ++x) {
        mask |= 1u << csb_v1_f0797_square_index_pc34(
            x, CSB_V1_F0797_CORRIDOR_ROW_Y_PC34);
    }
    mask |= 1u << csb_v1_f0797_square_index_pc34(
        CSB_V1_F0797_CORRIDOR_SPUR_X_PC34,
        CSB_V1_F0797_CORRIDOR_SPUR_Y_PC34);
    return mask;
}

int csb_v1_f0797_entrance_micro_dungeon_square_kind_pc34(int x, int y)
{
    uint32_t bit;

    if (x < 0 || y < 0 ||
        x >= CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34 ||
        y >= CSB_V1_F0797_MICRO_DUNGEON_HEIGHT_PC34) {
        return -1;
    }

    bit = 1u << csb_v1_f0797_square_index_pc34(x, y);
    return (csb_v1_f0797_entrance_micro_dungeon_corridor_mask_pc34() & bit) ?
        CSB_V1_F0797_MICRO_DUNGEON_SQUARE_CORRIDOR_PC34 :
        CSB_V1_F0797_MICRO_DUNGEON_SQUARE_WALL_PC34;
}

static int csb_v1_f0797_entrance_boundary_matches_pc34(
    const CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *receipt)
{
    return receipt && receipt->valid &&
        (receipt->accepted_stage_mask &
         CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34) &&
        receipt->real_asset_matched &&
        receipt->host_view_consumed &&
        receipt->host_draw_consumed &&
        receipt->draw_consumes_receipt_only &&
        receipt->no_synthetic_payloads &&
        receipt->no_fallback_graphics &&
        receipt->route_wrappers_retired;
}

void csb_v1_f0797_entrance_micro_dungeon_receipt_init_pc34(
    CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0797_STARTEND_DrawEntranceMicroDungeon(
    const CSB_V1_F0797_EntranceMicroDungeonFacts_PC34 *facts,
    CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0797_entrance_micro_dungeon_source_evidence_pc34();

    csb_v1_f0797_entrance_micro_dungeon_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        facts->entrance_map_index != CSB_V1_F0797_ENTRANCE_MAP_INDEX_PC34 ||
        facts->map_width != CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34 ||
        facts->map_height != CSB_V1_F0797_MICRO_DUNGEON_HEIGHT_PC34 ||
        facts->square_count != CSB_V1_F0797_MICRO_DUNGEON_SQUARE_COUNT_PC34 ||
        facts->wall_square_count !=
            CSB_V1_F0797_MICRO_DUNGEON_WALL_COUNT_PC34 ||
        facts->corridor_square_count !=
            CSB_V1_F0797_MICRO_DUNGEON_CORRIDOR_COUNT_PC34 ||
        facts->corridor_square_mask !=
            csb_v1_f0797_entrance_micro_dungeon_corridor_mask_pc34() ||
        facts->current_map_data_row_count !=
            CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34 ||
        !facts->draw_floor_and_ceiling_requested ||
        !facts->current_map_pointer_owned_by_micro_dungeon ||
        !facts->current_map_not_loaded_dungeon ||
        !facts->source_wall_fill_reviewed ||
        !facts->source_corridor_row_reviewed ||
        !facts->source_corridor_spur_reviewed ||
        !facts->draw_cpsf_route_reviewed ||
        facts->view_direction != CSB_V1_F0797_VIEW_DIRECTION_SOUTH_PC34 ||
        facts->view_x != CSB_V1_F0797_VIEW_X_PC34 ||
        facts->view_y != CSB_V1_F0797_VIEW_Y_PC34 ||
        !facts->no_loaded_dungeon_substitute ||
        !facts->no_synthetic_viewport_pixels ||
        !facts->no_legacy_micro_dungeon_wrapper ||
        !csb_v1_f0797_entrance_boundary_matches_pc34(
            &facts->entrance_boundary)) {
        if (out_receipt) {
            out_receipt->no_loaded_dungeon_substitute = 1;
            out_receipt->no_synthetic_viewport_pixels = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->entrance_map_index = CSB_V1_F0797_ENTRANCE_MAP_INDEX_PC34;
    out_receipt->map_width = CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34;
    out_receipt->map_height = CSB_V1_F0797_MICRO_DUNGEON_HEIGHT_PC34;
    out_receipt->square_count = CSB_V1_F0797_MICRO_DUNGEON_SQUARE_COUNT_PC34;
    out_receipt->wall_square_count =
        CSB_V1_F0797_MICRO_DUNGEON_WALL_COUNT_PC34;
    out_receipt->corridor_square_count =
        CSB_V1_F0797_MICRO_DUNGEON_CORRIDOR_COUNT_PC34;
    out_receipt->corridor_square_mask =
        csb_v1_f0797_entrance_micro_dungeon_corridor_mask_pc34();
    out_receipt->draw_floor_and_ceiling_requested = 1;
    out_receipt->view_direction = CSB_V1_F0797_VIEW_DIRECTION_SOUTH_PC34;
    out_receipt->view_x = CSB_V1_F0797_VIEW_X_PC34;
    out_receipt->view_y = CSB_V1_F0797_VIEW_Y_PC34;
    out_receipt->entrance_boundary_consumed = 1;
    out_receipt->no_loaded_dungeon_substitute = 1;
    out_receipt->no_synthetic_viewport_pixels = 1;
    out_receipt->no_legacy_micro_dungeon_wrapper = 1;
    out_receipt->source_evidence = evidence;
    return 1;
}

const char *csb_v1_f0797_entrance_micro_dungeon_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:57-81 F0797_STARTEND_DrawEntranceMicroDungeon "
           "builds the source 5x5 C255 entrance micro-dungeon in memory, fills "
           "it with wall squares, opens corridor row y=2 plus spur square "
           "(2,1), requests floor and ceiling drawing, and calls "
           "F0128_DUNGEONVIEW_Draw_CPSF(C2_DIRECTION_SOUTH,2,0); "
           "ENTRANCE.C:363-365/517-520 routes it behind the opening doors";
}
