/*
 * CSB V1 Viewport Rendering — pc34 compat implementation
 *
 * Source-locked to ReDMCSB WIP20210206, Toolchains/Common/Source/:
 *   DUNVIEW.C:6226-6353 F0676/F0677 (CSB back-wall D3L2/D3R2)
 *   DUNVIEW.C:6837-6896 F0678/F0679 (CSB near-wall D2L2/D2R2)
 *   DUNVIEW.C:8318-8542 F0128 (shared DM1/CSB viewport draw core)
 *
 * CSB differences from DM1:
 *   - D3L2/D3R2 back-wall positions render all 8 element types
 *     (WALL, TELEPORTER, STAIRS_FRONT, PIT, CORRIDOR, DOOR_SIDE,
 *      DOOR_FRONT, STAIRS_SIDE)
 *   - D2L2/D2R2 four-sided decoration positions render WALL only
 *     (no stairs, pits, floor ornaments, creatures, items, projectiles)
 *   - Four-sided wall decoration rules differ from DM1 corridor sides
 *   - Custom backgrounds (CSBWin/CSBCode.cpp:26 CustomBackgrounds)
 *   - CSB wall set index selection per current map
 *
 * Reference: CSBWin/Viewport.cpp (7290 lines) · CSBWin/Graphics.cpp (3186 lines)
 *   CSBWin/CSBCode.cpp:26 CustomBackgrounds · CSBWin/CSBCode.cpp:9196
 */

#include "csb_v1_viewport_pc34_compat.h"
#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat.h"
#include "csb_v1_viewport_custom_backgrounds_room_slot_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum {
    CSB_V1_ORNAMENT_SLOT_RIGHT = 1, /* M551_RIGHT_WALL_ORNAMENT_ORDINAL */
    CSB_V1_ORNAMENT_SLOT_FRONT = 2, /* M552_FRONT_WALL_ORNAMENT_ORDINAL */
    CSB_V1_ORNAMENT_SLOT_LEFT = 3,  /* M553_LEFT_WALL_ORNAMENT_ORDINAL */
    CSB_V1_VIEW_WALL_D3L2_RIGHT = 0,
    CSB_V1_VIEW_WALL_D3R2_LEFT = 1,
    CSB_V1_VIEW_WALL_D2L_RIGHT = 7,
    CSB_V1_VIEW_WALL_D2R_LEFT = 8,
    CSB_V1_VIEW_WALL_D2L_FRONT = 9,
    CSB_V1_VIEW_WALL_D2C_FRONT = 10,
    CSB_V1_VIEW_WALL_D2R_FRONT = 11,
    CSB_V1_VIEW_WALL_D1L_RIGHT = 12,
    CSB_V1_VIEW_WALL_D1R_LEFT = 13,
    CSB_V1_VIEW_WALL_D1C_FRONT = 14,
    CSB_V1_NO_ORNAMENT_SLOT = -1,
    CSB_V1_NO_VIEW_WALL = -1,
    CSB_V1_VIEW_FLOOR_D3L2 = 0, /* C00_VIEW_FLOOR_D3L2 */
    CSB_V1_VIEW_FLOOR_D3R2 = 1, /* C01_VIEW_FLOOR_D3R2 */
    CSB_V1_CELL_ORDER_D3L2_DOORPASS1 = 0x0218,
    CSB_V1_CELL_ORDER_D3L2_DOORPASS2 = 0x0349,
    CSB_V1_CELL_ORDER_D3R2_DOORPASS1 = 0x0128,
    CSB_V1_CELL_ORDER_D3R2_DOORPASS2 = 0x0439,
    CSB_V1_CELL_ORDER_D3L2_CORRIDOR = 0x3421,
    CSB_V1_CELL_ORDER_D3R2_CORRIDOR = 0x4312,
    CSB_V1_CELL_ORDER_D3L2_SIDE = 0x0321,
    CSB_V1_CELL_ORDER_D3R2_SIDE = 0x0412,
    CSB_V1_ZONE_DOOR_D3L2 = 3700,
    CSB_V1_ZONE_DOOR_D3R2 = 3710,
    CSB_V1_REDMCSB_VIEW_SQUARE_D3L2 = 14, /* C14_VIEW_SQUARE_D3L2 */
    CSB_V1_REDMCSB_VIEW_SQUARE_D3R2 = 15, /* C15_VIEW_SQUARE_D3R2 */
    CSB_V1_VIEW_DEPTH_D3 = 3,
    CSB_V1_OBJECT_ROW_D3L2 = 3, /* G2028_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_OBJECT_ROW_D3R2 = 4, /* G2028_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_OBJECT_ZONE_BASE = 2500, /* C2500_ZONE_ */
    CSB_V1_OBJECT_ZONE_CELL_STRIDE = 4,
    CSB_V1_OBJECT_SHIFT_SET_D3_FRONT = 5, /* (viewDepth * 2) - 1 - (viewCell >> 1) for D3 back-row cells */
    CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL = 3,
    CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL = 4,
    CSB_V1_CREATURE_ROW_D3L2 = 3, /* G2033_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_CREATURE_ROW_D3R2 = 4, /* G2033_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_CREATURE_ZONE_BASE = 3200, /* C3200_ZONE_ */
    CSB_V1_CREATURE_COORDINATE_SET_STRIDE = 65,
    CSB_V1_CREATURE_ZONE_CELL_STRIDE = 5,
    CSB_V1_CREATURE_SHIFT_MASK = 0x8000, /* MASK0x8000_SHIFT_OBJECTS_AND_CREATURES */
    CSB_V1_PROJECTILE_ROW_D3L2 = 3, /* G2028_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_PROJECTILE_ROW_D3R2 = 4, /* G2028_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_PROJECTILE_ZONE_BASE = 2900, /* C2900_ZONE_ */
    CSB_V1_PROJECTILE_ZONE_STRIDE = 4,
    CSB_V1_EXPLOSION_ROW_D3L2 = 6, /* G2034_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_EXPLOSION_ROW_D3R2 = 7, /* G2034_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_FIELD_ASPECT_D3L2 = 0, /* G2035_ac_ViewSquareIndexToFieldAspectIndex[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_FIELD_ASPECT_D3R2 = 1, /* G2035_ac_ViewSquareIndexToFieldAspectIndex[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_FIELD_ASPECT_D2L2 = 5, /* G2035_ac_ViewSquareIndexToFieldAspectIndex[C09_VIEW_SQUARE_D2L2] */
    CSB_V1_FIELD_ASPECT_D2R2 = 6, /* G2035_ac_ViewSquareIndexToFieldAspectIndex[C10_VIEW_SQUARE_D2R2] */
    CSB_V1_EXPLOSION_REBIRTH_STEP1_ZONE_BASE = 3000, /* C3000_ZONE_ */
    CSB_V1_EXPLOSION_REBIRTH_STEP2_ZONE_BASE = 3007, /* C3007_ZONE_ */
    CSB_V1_EXPLOSION_CENTERED_ZONE_BASE = 3014, /* C3014_ZONE_ */
    CSB_V1_EXPLOSION_SIDE_ZONE_BASE = 3031, /* C3031_ZONE_ */
    CSB_V1_EXPLOSION_SIDE_ZONE_CELL_STRIDE = 2,
    CSB_V1_F0115_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_FIELD_ZONE_D3L2 = 702, /* C702_ZONE_WALL_D3L2 */
    CSB_V1_FIELD_ZONE_D3R2 = 703, /* C703_ZONE_WALL_D3R2 */
    CSB_V1_FIELD_ZONE_D2L2 = 707, /* C707_ZONE_WALL_D2L2 */
    CSB_V1_FIELD_ZONE_D2R2 = 708, /* C708_ZONE_WALL_D2R2 */
    CSB_V1_WALL_BITMAP_D2R2 = 5, /* DEFS.H:3428 C05_WALL_D2R2 */
    CSB_V1_WALL_BITMAP_D2L2 = 6, /* DEFS.H:3429 C06_WALL_D2L2 */
    CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED = 1,
    CSB_V1_DOOR_PANEL_PARENT_D3L2 = 129,
    CSB_V1_DOOR_PANEL_PARENT_D3R2 = 130,
    CSB_V1_DOOR_PANEL_CLIP_D3 = 126,
    CSB_V1_DOOR_PANEL_NATIVE_W_D3 = 48,
    CSB_V1_DOOR_PANEL_NATIVE_H_D3 = 41,
    CSB_V1_DOOR_PANEL_CLIPPED_H_D3 = 40,
    CSB_V1_DOOR_PANEL_D3L2_X = 24,
    CSB_V1_DOOR_PANEL_D3R2_X = 88,
    CSB_V1_DOOR_PANEL_D3_Y = 28,
    CSB_V1_DOOR_ORNAMENT_D3LCR = 0, /* C0_VIEW_DOOR_ORNAMENT_D3LCR */
    CSB_V1_DOOR_STATE_DESTROYED = 5, /* C5_DOOR_STATE_DESTROYED */
    CSB_V1_DOOR_ORNAMENT_DESTROYED_MASK = 15, /* C15_DOOR_ORNAMENT_DESTROYED_MASK */
    CSB_V1_DOOR_HORIZONTAL_FINAL_SHIFT_MASK = 0x4000, /* MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR */
    CSB_V1_DOOR_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_WALL_ORNAMENT_ZONE_BASE = 1004, /* C1004_ZONE_WALL_ORNAMENT */
    CSB_V1_WALL_ORNAMENT_COORD_STRIDE = 15, /* MEDIA720 C15_UNKNOWN */
    CSB_V1_WALL_ORNAMENT_SCALE_X_D3 = 30, /* C30_SCALE_ */
    CSB_V1_WALL_ORNAMENT_SCALE_Y_D3 = 14, /* C14_SCALE_ */
    CSB_V1_WALL_ORNAMENT_SCALE_D2 = 21, /* C21_SCALE_ */
    CSB_V1_WALL_ORNAMENT_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_WALL_ORNAMENT_ORDINAL_TO_INDEX_DELTA = -1,
    CSB_V1_WALL_ORNAMENT_D3L2_BITMAP_INCREMENT = 0,
    CSB_V1_WALL_ORNAMENT_D3R2_BITMAP_INCREMENT = 0,
    CSB_V1_WALL_ORNAMENT_D2_SIDE_DERIVED_INCREMENT = 2,
    CSB_V1_WALL_ORNAMENT_D2_FRONT_DERIVED_INCREMENT = 3,
    CSB_V1_WALL_ORNAMENT_D1_SIDE_DERIVED_INCREMENT = 4,
    CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE = -1, /* CM1_DERIVED_BITMAP_NONE */
    CSB_V1_FLOOR_ORNAMENT_ZONE_BASE = 1500, /* C1500_ZONE_FLOOR_ORNAMENT */
    CSB_V1_FLOOR_ORNAMENT_COORD_STRIDE = 11,
    CSB_V1_FLOOR_ORNAMENT_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_FLOOR_ORNAMENT_ORDINAL_TO_INDEX_DELTA = -1,
    CSB_V1_FLOOR_ORNAMENT_D3L2_BITMAP_INCREMENT = 0,
    CSB_V1_FLOOR_ORNAMENT_D3R2_BITMAP_INCREMENT = 0,
    CSB_V1_FLOOR_ORNAMENT_COORD_SET = 0,
    CSB_V1_FLIP_NONE = 0, /* MASK0x0000_NO_FLIP */
    CSB_V1_FLIP_HORIZONTAL = 1, /* MASK0x0001_FLIP_HORIZONTAL */
    CSB_V1_FLIP_VERTICAL = 2, /* MASK0x0002_FLIP_VERTICAL */
    CSB_V1_PROJECTILE_DERIVED_BITMAP_NONE = -1, /* CM1_DERIVED_BITMAP_NONE */
    CSB_V1_PROJECTILE_TRANSPARENT_COLOR = CSB_V1_F0115_TRANSPARENT_COLOR,
    CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_GRAPHIC_ID = 1,
    CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES = 18,
    CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_SKIN_DEF_INDEX = 0,
    CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_SKIN_DEF_INDEX = 1,
    CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_SKIN_DEF_INDEX = 2,
    CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_SKIN_DEF_INDEX = 4,
    CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_SKIN_DEF_INDEX = 5,
    CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_SKIN_DEF_INDEX = 6,
    CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_MIN_BYTES = 64,
    CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_MIN_BYTES = 64,
    CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_MIN_BYTES = 20,
    CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_MIN_BYTES = 7840,
    CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_MIN_BYTES = 3248,
    CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_MIN_BYTES = 4144,
    CSB_V1_CUSTOM_BACKGROUND_NEAR_ROOM_LIMIT = 5
};

static void csb_v1_viewport_runtime_relative_position(
    int party_dir,
    int party_x,
    int party_y,
    int map_x,
    int map_y,
    int *out_forward,
    int *out_side)
{
    int dx = map_x - party_x;
    int dy = map_y - party_y;
    int forward = 0;
    int side = 0;

    switch (party_dir & 3) {
    case 0:
        forward = -dy;
        side = dx;
        break;
    case 1:
        forward = dx;
        side = dy;
        break;
    case 2:
        forward = dy;
        side = -dx;
        break;
    default:
        forward = -dx;
        side = -dy;
        break;
    }
    if (out_forward) *out_forward = forward;
    if (out_side) *out_side = side;
}

static uint32_t csb_v1_viewport_mix_u32_pc34(uint32_t hash, uint32_t value)
{
    hash ^= value & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 8) & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 16) & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 24) & 0xffu;
    hash *= 16777619u;
    return hash;
}

int csb_v1_viewport_first_frame_material_proof_valid_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof)
{
    const unsigned int d3_routes =
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR |
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR;

    if (!proof || !proof->valid) {
        return 0;
    }
    if ((proof->route_mask & CSB_V1_VIEWPORT_FIRST_FRAME_REQUIRED_ROUTES) !=
        CSB_V1_VIEWPORT_FIRST_FRAME_REQUIRED_ROUTES) {
        return 0;
    }
    if (!proof->source_graphics_dat_bound ||
        !proof->no_synthetic_pixels ||
        !proof->no_fallback_visuals ||
        !proof->shared_palette_material_proof) {
        return 0;
    }
    if (proof->source_item_count < 5u) {
        return 0;
    }
    if (((proof->route_mask & d3_routes) != 0u) &&
        ((proof->route_mask & d3_routes) != d3_routes ||
         proof->d3l2_door_hash == 0u || proof->d3r2_door_hash == 0u)) {
        return 0;
    }
    if ((proof->route_mask & d3_routes) == d3_routes &&
        (!proof->d3_pair_real_asset_receipt.valid ||
         !proof->d3_pair_real_asset_receipt.route_backed_by_real_graphics_dat ||
         !proof->d3_pair_real_asset_receipt.source_graphics_dat_bound ||
         !proof->d3_pair_real_asset_receipt.no_synthetic_pixels ||
         !proof->d3_pair_real_asset_receipt.no_fallback_visuals ||
         proof->d3_pair_real_asset_receipt.source_graphics_item_index != 693 ||
         proof->d3_pair_real_asset_receipt.source_byte_count == 0u ||
         proof->d3_pair_real_asset_receipt.source_payload_hash == 0u ||
         proof->d3_pair_real_asset_receipt.source_payload_hash !=
             proof->d3l2_door_hash ||
         proof->d3_pair_real_asset_receipt.source_payload_hash !=
             proof->d3r2_door_hash ||
         proof->d3_pair_real_asset_receipt.d3l2_door_zone != 3700 ||
         proof->d3_pair_real_asset_receipt.d3r2_door_zone != 3710 ||
         proof->d3_pair_real_asset_receipt.transparent_color != 10 ||
         proof->d3_pair_real_asset_receipt.native_bitmap_byte_width != 48 ||
         proof->d3_pair_real_asset_receipt.native_bitmap_height != 41)) {
        return 0;
    }
    if ((proof->route_mask & d3_routes) == 0u &&
        (proof->d3l2_door_hash != 0u || proof->d3r2_door_hash != 0u)) {
        return 0;
    }
    /* DUNVIEW.C F0121 reaches F0111 with G0694 at the mandatory D2C route.
     * Keep the actual source receipt facts with the plan instead of treating
     * an unscoped payload FNV as a graphic identity. */
    if (!proof->d2_door_capture_valid ||
        !proof->d2_door_capture_real_graphics_dat ||
        !proof->d2_door_capture_no_synthetic_pixels ||
        !proof->d2_door_capture_no_fallback_visuals ||
        proof->d2_door_capture_item_index != 694 ||
        proof->d2_door_capture_byte_count == 0u ||
        proof->d2_door_capture_payload_hash == 0u ||
        proof->d2_door_capture_payload_hash != proof->d2_door_hash ||
        proof->d2_door_capture_width != 64 ||
        proof->d2_door_capture_height != 61 ||
        proof->d2_door_capture_zone != 3760 ||
        proof->d2_door_capture_transparent_color != 10) {
        return 0;
    }
    return proof->shared_palette_hash != 0u &&
           proof->d0_door_hash != 0u &&
           proof->d0_thing_hash != 0u &&
           proof->d1_door_hash != 0u &&
           proof->d1_thing_hash != 0u &&
           proof->d2_door_hash != 0u;
}

int csb_v1_viewport_admit_first_frame_materialization_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof,
    int real_graphics_session,
    CSB_V1_ViewportFirstFrameMaterializationReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    const unsigned int d3_routes =
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR |
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!real_graphics_session ||
        !csb_v1_viewport_first_frame_material_proof_valid_pc34(proof)) {
        return 0;
    }

    hash = csb_v1_viewport_mix_u32_pc34(hash, proof->route_mask);
    hash = csb_v1_viewport_mix_u32_pc34(hash, proof->shared_palette_hash);
    hash = csb_v1_viewport_mix_u32_pc34(hash, proof->d0_door_hash);
    hash = csb_v1_viewport_mix_u32_pc34(hash, proof->d0_thing_hash);
    hash = csb_v1_viewport_mix_u32_pc34(hash, proof->d1_door_hash);
    hash = csb_v1_viewport_mix_u32_pc34(hash, proof->d1_thing_hash);
    hash = csb_v1_viewport_mix_u32_pc34(hash, proof->d2_door_hash);
    if ((proof->route_mask & d3_routes) == d3_routes) {
        hash = csb_v1_viewport_mix_u32_pc34(hash, proof->d3l2_door_hash);
        hash = csb_v1_viewport_mix_u32_pc34(hash, proof->d3r2_door_hash);
    }

    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->consumed_by_m11_render = 1;
        out_receipt->route_mask = proof->route_mask;
        out_receipt->combined_material_hash = hash;
        out_receipt->required_route_count =
            (proof->route_mask & d3_routes) == d3_routes ? 7 : 5;
        out_receipt->real_graphics_session = 1;
        out_receipt->no_synthetic_pixels = proof->no_synthetic_pixels;
        out_receipt->no_fallback_visuals = proof->no_fallback_visuals;
        out_receipt->source_evidence = proof->source_evidence;
    }
    return 1;
}

static void csb_v1_viewport_runtime_draw_command_init_pc34(
    CSB_V1_ViewportRuntimeDrawCommandPc34 *command,
    CSB_V1_ViewportRuntimeDrawRoutePc34 route,
    unsigned int route_bit,
    uint32_t material_hash,
    uint32_t palette_hash,
    int view_depth,
    int view_square,
    int draw_order,
    int clip_x,
    int clip_y,
    int clip_w,
    int clip_h,
    int transparent_color,
    int party_dir,
    int party_x,
    int party_y,
    int input_forward,
    int input_side)
{
    if (!command) return;
    memset(command, 0, sizeof(*command));
    command->route = route;
    command->route_bit = route_bit;
    command->material_hash = material_hash;
    command->palette_hash = palette_hash;
    command->view_depth = view_depth;
    command->view_square = view_square;
    command->draw_order = draw_order;
    command->clip_x = clip_x;
    command->clip_y = clip_y;
    command->clip_w = clip_w;
    command->clip_h = clip_h;
    command->transparent_color = transparent_color;
    command->party_dir = party_dir;
    command->party_x = party_x;
    command->party_y = party_y;
    command->input_forward = input_forward;
    command->input_side = input_side;
}

int csb_v1_viewport_build_first_frame_runtime_draw_plan_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof,
    int real_graphics_session,
    int party_dir,
    int party_x,
    int party_y,
    CSB_V1_ViewportRuntimeDrawPlanPc34 *out_plan)
{
    uint32_t hash = 2166136261u;
    const unsigned int d3_routes =
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR |
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR;
    const int has_d3 = (proof && (proof->route_mask & d3_routes) == d3_routes);
    int command_offset = has_d3 ? 2 : 0;
    int i;

    if (out_plan) {
        memset(out_plan, 0, sizeof(*out_plan));
    }
    if (!out_plan || !real_graphics_session ||
        !csb_v1_viewport_first_frame_material_proof_valid_pc34(proof) ||
        party_dir < 0 || party_dir > 3 || party_x < 0 || party_y < 0) {
        return 0;
    }

    /* ReDMCSB DUNVIEW.C F0128 consumes the near-to-mid door and thing
     * routes through indexed GRAPHICS.DAT materials selected by the same
     * viewport input state.  This is a command plan for M11 consumption, not
     * a fallback rasterizer. */
    if (has_d3) {
        /* F0128 reaches the D3L2/D3R2 pair before D2.  Both commands share
         * G0693 but retain their separate source zones and clipped frames. */
        csb_v1_viewport_runtime_draw_command_init_pc34(
            &out_plan->commands[0],
            CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3L2_F0111_DOOR_PC34,
            CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR,
            proof->d3l2_door_decoded_hash ? proof->d3l2_door_decoded_hash :
                proof->d3l2_door_hash,
            proof->shared_palette_hash, 3, 14, 0x0218,
            24, 28, 48, 40, CSB_V1_DOOR_TRANSPARENT_COLOR,
            party_dir, party_x, party_y, 3, -2);
        csb_v1_viewport_runtime_draw_command_init_pc34(
            &out_plan->commands[1],
            CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3R2_F0111_DOOR_PC34,
            CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR,
            proof->d3r2_door_decoded_hash ? proof->d3r2_door_decoded_hash :
                proof->d3r2_door_hash,
            proof->shared_palette_hash, 3, 15, 0x0128,
            88, 28, 48, 40, CSB_V1_DOOR_TRANSPARENT_COLOR,
            party_dir, party_x, party_y, 3, 2);
    }
    csb_v1_viewport_runtime_draw_command_init_pc34(
        &out_plan->commands[0 + command_offset],
        CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D2_F0111_DOOR_PC34,
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D2_F0111_DOOR,
        proof->d2_door_decoded_hash ? proof->d2_door_decoded_hash :
            proof->d2_door_hash,
        proof->shared_palette_hash, 2, 10, 0x0111,
        76, 47, 72, 74, CSB_V1_DOOR_TRANSPARENT_COLOR,
        party_dir, party_x, party_y, 2, 0);
    csb_v1_viewport_runtime_draw_command_init_pc34(
        &out_plan->commands[1 + command_offset],
        CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D1_F0111_DOOR_PC34,
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0111_DOOR,
        proof->d1_door_hash, proof->shared_palette_hash, 1, 14, 0x0218,
        48, 33, 128, 102, CSB_V1_DOOR_TRANSPARENT_COLOR,
        party_dir, party_x, party_y, 1, 0);
    csb_v1_viewport_runtime_draw_command_init_pc34(
        &out_plan->commands[2 + command_offset],
        CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D1_F0115_THING_PC34,
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0115_THING,
        proof->d1_thing_hash, proof->shared_palette_hash, 1, 14, 0x0349,
        40, 33, 144, 104, CSB_V1_F0115_TRANSPARENT_COLOR,
        party_dir, party_x, party_y, 1, 0);
    csb_v1_viewport_runtime_draw_command_init_pc34(
        &out_plan->commands[3 + command_offset],
        CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D0_F0111_DOOR_PC34,
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0111_DOOR,
        proof->d0_door_hash, proof->shared_palette_hash, 0, 12, 0x0439,
        0, 33, 112, 136, CSB_V1_DOOR_TRANSPARENT_COLOR,
        party_dir, party_x, party_y, 0, -1);
    csb_v1_viewport_runtime_draw_command_init_pc34(
        &out_plan->commands[4 + command_offset],
        CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D0_F0115_THING_PC34,
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0115_THING,
        proof->d0_thing_hash, proof->shared_palette_hash, 0, 13, 0x0615,
        112, 33, 112, 136, CSB_V1_F0115_TRANSPARENT_COLOR,
        party_dir, party_x, party_y, 0, 1);

    out_plan->valid = 1;
    out_plan->consumed_by_m11_render = 1;
    out_plan->real_graphics_session = 1;
    out_plan->no_synthetic_pixels = proof->no_synthetic_pixels;
    out_plan->no_fallback_visuals = proof->no_fallback_visuals;
    out_plan->palette_bound = proof->shared_palette_hash != 0u;
    out_plan->clip_bound = 1;
    out_plan->state_bound = 1;
    out_plan->input_bound = 1;
    out_plan->route_mask = proof->route_mask;
    out_plan->command_count =
        CSB_V1_VIEWPORT_RUNTIME_FIRST_FRAME_DRAW_COMMAND_BASE_COUNT_PC34 +
        command_offset;
    out_plan->shared_palette_hash = proof->shared_palette_hash;
    out_plan->source_evidence = proof->source_evidence;

    hash = csb_v1_viewport_mix_u32_pc34(hash, out_plan->route_mask);
    hash = csb_v1_viewport_mix_u32_pc34(hash, out_plan->shared_palette_hash);
    hash = csb_v1_viewport_mix_u32_pc34(hash, (uint32_t)party_dir);
    hash = csb_v1_viewport_mix_u32_pc34(hash, (uint32_t)party_x);
    hash = csb_v1_viewport_mix_u32_pc34(hash, (uint32_t)party_y);
    for (i = 0; i < out_plan->command_count; ++i) {
        const CSB_V1_ViewportRuntimeDrawCommandPc34 *command =
            &out_plan->commands[i];
        if (command->route == CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_NONE_PC34 ||
            command->material_hash == 0u || command->palette_hash == 0u ||
            command->clip_w <= 0 || command->clip_h <= 0 ||
            command->clip_x < 0 || command->clip_y < 0 ||
            command->clip_x + command->clip_w > 224 ||
            command->clip_y + command->clip_h > 169 ||
            command->party_dir != party_dir ||
            command->party_x != party_x ||
            command->party_y != party_y) {
            memset(out_plan, 0, sizeof(*out_plan));
            return 0;
        }
        hash = csb_v1_viewport_mix_u32_pc34(hash, command->route_bit);
        hash = csb_v1_viewport_mix_u32_pc34(hash, command->material_hash);
        hash = csb_v1_viewport_mix_u32_pc34(hash, (uint32_t)command->draw_order);
        hash = csb_v1_viewport_mix_u32_pc34(hash, (uint32_t)command->clip_x);
        hash = csb_v1_viewport_mix_u32_pc34(hash, (uint32_t)command->clip_y);
        hash = csb_v1_viewport_mix_u32_pc34(hash, (uint32_t)command->clip_w);
        hash = csb_v1_viewport_mix_u32_pc34(hash, (uint32_t)command->clip_h);
        hash = csb_v1_viewport_mix_u32_pc34(
            hash, (uint32_t)(command->input_forward + 4));
        hash = csb_v1_viewport_mix_u32_pc34(
            hash, (uint32_t)(command->input_side + 4));
    }
    out_plan->plan_hash = hash ? hash : 1u;
    return 1;
}

static uint32_t csb_v1_viewport_fnv1a_bytes_pc34(const uint8_t *bytes,
                                                  size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static uint16_t csb_v1_viewport_read_be16_pc34(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static int csb_v1_viewport_md5_text_pc34(const char *text)
{
    size_t i;
    if (!text || strlen(text) != 32u) return 0;
    for (i = 0u; i < 32u; ++i) {
        if (!((text[i] >= '0' && text[i] <= '9') ||
              (text[i] >= 'a' && text[i] <= 'f') ||
              (text[i] >= 'A' && text[i] <= 'F'))) return 0;
    }
    return 1;
}

int csb_v1_viewport_admit_graphics_table_provenance_pc34(
    const uint8_t *table_bytes, size_t table_size,
    const char *source_path, const char *source_md5,
    uint32_t native_bitmap_index,
    CSB_V1_ViewportGraphicsTableProvenancePc34 *out_receipt)
{
    uint16_t signature;
    uint16_t entry_count;
    size_t required_size;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!table_bytes || !out_receipt || !source_path || !source_path[0] ||
        !csb_v1_viewport_md5_text_pc34(source_md5) ||
        native_bitmap_index == 0u || table_size < 4u) return 0;
    signature = csb_v1_viewport_read_be16_pc34(table_bytes);
    entry_count = csb_v1_viewport_read_be16_pc34(table_bytes + 2u);
    if (signature != 0x8001u || entry_count == 0u || entry_count > 2048u) {
        return 0;
    }
    required_size = 4u + (size_t)entry_count * 4u;
    if (table_size < required_size) return 0;
    out_receipt->valid = 1;
    out_receipt->original_graphics_dat_table = 1;
    out_receipt->native_bitmap_mapping_proven = 0;
    out_receipt->raster_blocked_without_mapping = 1;
    out_receipt->source_path = source_path;
    out_receipt->source_md5 = source_md5;
    out_receipt->native_bitmap_index = native_bitmap_index;
    out_receipt->graphics_entry_count = entry_count;
    out_receipt->table_span_bytes = required_size;
    out_receipt->table_span_fnv1a = csb_v1_viewport_fnv1a_bytes_pc34(
        table_bytes, required_size);
    return out_receipt->table_span_fnv1a != 0u;
}

static uint32_t csb_v1_viewport_proof_hash_for_route_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof, unsigned int route_bit)
{
    switch (route_bit) {
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0111_DOOR: return proof->d0_door_hash;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0115_THING: return proof->d0_thing_hash;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0111_DOOR: return proof->d1_door_hash;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0115_THING: return proof->d1_thing_hash;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D2_F0111_DOOR:
        return proof->d2_door_decoded_hash ? proof->d2_door_decoded_hash :
            proof->d2_door_hash;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR:
        return proof->d3l2_door_decoded_hash ? proof->d3l2_door_decoded_hash :
            proof->d3l2_door_hash;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR:
        return proof->d3r2_door_decoded_hash ? proof->d3r2_door_decoded_hash :
            proof->d3r2_door_hash;
    default: return 0u;
    }
}

int csb_v1_viewport_decode_d2_d3_native_packed_capture_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof,
    const CSB_V1_ViewportFirstFramePaletteSpanPc34 *palette,
    const CSB_V1_ViewportD2D3NativePackedCapturePc34 *packed_capture,
    uint8_t *d2_decoded_pixels, size_t d2_decoded_capacity,
    uint8_t *d3_decoded_pixels, size_t d3_decoded_capacity,
    CSB_V1_ViewportD2D3MaterialCaptureReceiptPc34 *out_receipt)
{
    const size_t d2_packed_required = 32u * 61u;
    const size_t d2_decoded_required = 64u * 61u;
    const size_t d3_packed_required = 24u * 41u;
    const size_t d3_decoded_required = 48u * 41u;
    const unsigned int d3_routes =
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR |
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR;
    int has_d3;
    size_t i;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!proof || !palette || !packed_capture || !out_receipt ||
        !csb_v1_viewport_first_frame_material_proof_valid_pc34(proof) ||
        !palette->decoded_palette || palette->decoded_size == 0u ||
        !palette->decoded_fnv1a ||
        palette->decoded_fnv1a != csb_v1_viewport_fnv1a_bytes_pc34(
            palette->decoded_palette, palette->decoded_size) ||
        !packed_capture->valid || !packed_capture->original_graphics_dat_capture ||
        !packed_capture->f0489_native_bitmap_selected ||
        !packed_capture->f0488_expand_4bpp ||
        !packed_capture->no_synthetic_pixels ||
        !packed_capture->no_fallback_visuals ||
        !packed_capture->source_path || !packed_capture->source_path[0] ||
        !csb_v1_viewport_md5_text_pc34(packed_capture->source_md5) ||
        !packed_capture->palette_source_path ||
        !packed_capture->palette_source_md5 ||
        strcmp(packed_capture->source_path,
               packed_capture->palette_source_path) != 0 ||
        strcmp(packed_capture->source_md5,
               packed_capture->palette_source_md5) != 0 ||
        packed_capture->palette_capture_fnv1a != palette->decoded_fnv1a ||
        packed_capture->capture_identity_hash == 0u ||
        !packed_capture->d2_table_provenance ||
        !packed_capture->d2_table_provenance->valid ||
        !packed_capture->d2_table_provenance->original_graphics_dat_table ||
        !packed_capture->d2_table_provenance->native_bitmap_mapping_proven ||
        packed_capture->d2_table_provenance->raster_blocked_without_mapping ||
        !packed_capture->d2_table_provenance->source_path ||
        !packed_capture->d2_table_provenance->source_md5 ||
        packed_capture->d2_table_provenance->native_bitmap_index != 694u ||
        strcmp(packed_capture->d2_table_provenance->source_path,
               packed_capture->source_path) != 0 ||
        strcmp(packed_capture->d2_table_provenance->source_md5,
               packed_capture->source_md5) != 0 ||
        packed_capture->d2_item_index != 694 ||
        packed_capture->d2_source_payload_hash != proof->d2_door_hash ||
        !packed_capture->d2_packed_pixels ||
        packed_capture->d2_packed_size != d2_packed_required ||
        packed_capture->d2_packed_fnv1a == 0u ||
        packed_capture->d2_packed_fnv1a != csb_v1_viewport_fnv1a_bytes_pc34(
            packed_capture->d2_packed_pixels, packed_capture->d2_packed_size) ||
        !d2_decoded_pixels || d2_decoded_capacity < d2_decoded_required) return 0;

    has_d3 = (proof->route_mask & d3_routes) == d3_routes;
    if (has_d3 &&
        (!packed_capture->d3_table_provenance ||
         !packed_capture->d3_table_provenance->valid ||
         !packed_capture->d3_table_provenance->original_graphics_dat_table ||
         !packed_capture->d3_table_provenance->native_bitmap_mapping_proven ||
         packed_capture->d3_table_provenance->raster_blocked_without_mapping ||
         !packed_capture->d3_table_provenance->source_path ||
         !packed_capture->d3_table_provenance->source_md5 ||
         packed_capture->d3_table_provenance->native_bitmap_index != 693u ||
         strcmp(packed_capture->d3_table_provenance->source_path,
                packed_capture->source_path) != 0 ||
         strcmp(packed_capture->d3_table_provenance->source_md5,
                packed_capture->source_md5) != 0 ||
         packed_capture->d3_item_index != 693 ||
         packed_capture->d3_source_payload_hash != proof->d3l2_door_hash ||
         packed_capture->d3_source_payload_hash != proof->d3r2_door_hash ||
         !packed_capture->d3_packed_pixels ||
         packed_capture->d3_packed_size != d3_packed_required ||
         packed_capture->d3_packed_fnv1a == 0u ||
         packed_capture->d3_packed_fnv1a != csb_v1_viewport_fnv1a_bytes_pc34(
             packed_capture->d3_packed_pixels, packed_capture->d3_packed_size) ||
         !d3_decoded_pixels || d3_decoded_capacity < d3_decoded_required)) return 0;
    if (!has_d3 && (packed_capture->d3_item_index != 0 ||
                    packed_capture->d3_source_payload_hash != 0u ||
                    packed_capture->d3_packed_pixels != NULL ||
                    packed_capture->d3_packed_size != 0u ||
                    packed_capture->d3_packed_fnv1a != 0u)) return 0;

    for (i = 0u; i < d2_packed_required; ++i) {
        d2_decoded_pixels[i * 2u] = packed_capture->d2_packed_pixels[i] >> 4;
        d2_decoded_pixels[i * 2u + 1u] = packed_capture->d2_packed_pixels[i] & 0x0fu;
    }
    if (has_d3) {
        for (i = 0u; i < d3_packed_required; ++i) {
            d3_decoded_pixels[i * 2u] = packed_capture->d3_packed_pixels[i] >> 4;
            d3_decoded_pixels[i * 2u + 1u] = packed_capture->d3_packed_pixels[i] & 0x0fu;
        }
    }
    out_receipt->valid = 1;
    out_receipt->original_graphics_dat_capture = 1;
    out_receipt->no_synthetic_pixels = 1;
    out_receipt->no_fallback_visuals = 1;
    out_receipt->source_path = packed_capture->source_path;
    out_receipt->source_md5 = packed_capture->source_md5;
    out_receipt->palette_source_path = packed_capture->palette_source_path;
    out_receipt->palette_source_md5 = packed_capture->palette_source_md5;
    out_receipt->palette_capture_fnv1a = palette->decoded_fnv1a;
    out_receipt->capture_identity_hash = packed_capture->capture_identity_hash;
    out_receipt->d2_item_index = 694;
    out_receipt->d2_source_byte_count = proof->d2_door_capture_byte_count;
    out_receipt->d2_source_payload_hash = proof->d2_door_hash;
    out_receipt->d2_decoded_pixels = d2_decoded_pixels;
    out_receipt->d2_decoded_size = d2_decoded_required;
    out_receipt->d2_decoded_fnv1a = csb_v1_viewport_fnv1a_bytes_pc34(
        d2_decoded_pixels, d2_decoded_required);
    out_receipt->d2_width = 64;
    out_receipt->d2_height = 61;
    if (has_d3) {
        out_receipt->d3_item_index = 693;
        out_receipt->d3_source_byte_count =
            proof->d3_pair_real_asset_receipt.source_byte_count;
        out_receipt->d3_source_payload_hash = proof->d3l2_door_hash;
        out_receipt->d3_decoded_pixels = d3_decoded_pixels;
        out_receipt->d3_decoded_size = d3_decoded_required;
        out_receipt->d3_decoded_fnv1a = csb_v1_viewport_fnv1a_bytes_pc34(
            d3_decoded_pixels, d3_decoded_required);
        out_receipt->d3_width = 48;
        out_receipt->d3_height = 41;
    }
    return 1;
}

static int csb_v1_viewport_d2_d3_capture_valid_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof,
    const CSB_V1_ViewportRuntimeDrawPlanPc34 *plan,
    const CSB_V1_ViewportFirstFrameMaterialBytesPc34 *bytes)
{
    const CSB_V1_ViewportD2D3MaterialCaptureReceiptPc34 *capture;
    int has_d3;
    int d2_command = -1;
    int d3l2_command = -1;
    int d3r2_command = -1;
    int i;

    if (!proof || !plan || !bytes) return 0;
    capture = &bytes->d2_d3_capture;
    if (!capture->valid || !capture->original_graphics_dat_capture ||
        !capture->no_synthetic_pixels || !capture->no_fallback_visuals ||
        !capture->source_path || !capture->source_md5 ||
        !capture->palette_source_path || !capture->palette_source_md5 ||
        strcmp(capture->source_path, bytes->source_path) != 0 ||
        strcmp(capture->source_md5, bytes->source_md5) != 0 ||
        strcmp(capture->palette_source_path, bytes->source_path) != 0 ||
        strcmp(capture->palette_source_md5, bytes->source_md5) != 0 ||
        capture->palette_capture_fnv1a == 0u ||
        capture->palette_capture_fnv1a != proof->shared_palette_hash ||
        capture->palette_capture_fnv1a != bytes->palette.decoded_fnv1a ||
        capture->capture_identity_hash == 0u ||
        capture->d2_item_index != 694 ||
        capture->d2_source_byte_count != proof->d2_door_capture_byte_count ||
        capture->d2_source_payload_hash != proof->d2_door_hash ||
        !capture->d2_decoded_pixels || capture->d2_decoded_size == 0u ||
        capture->d2_width != 64 || capture->d2_height != 61 ||
        capture->d2_decoded_size != (size_t)capture->d2_width * capture->d2_height ||
        capture->d2_decoded_fnv1a == 0u ||
        capture->d2_decoded_fnv1a != csb_v1_viewport_fnv1a_bytes_pc34(
            capture->d2_decoded_pixels, capture->d2_decoded_size)) return 0;

    has_d3 = (proof->route_mask &
              (CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR |
               CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR)) != 0u;
    if (has_d3 &&
        (capture->d3_item_index != 693 ||
         capture->d3_source_byte_count !=
             proof->d3_pair_real_asset_receipt.source_byte_count ||
         capture->d3_source_payload_hash != proof->d3l2_door_hash ||
         capture->d3_source_payload_hash != proof->d3r2_door_hash ||
         !capture->d3_decoded_pixels || capture->d3_decoded_size == 0u ||
         capture->d3_width != 48 || capture->d3_height != 41 ||
         capture->d3_decoded_size != (size_t)capture->d3_width * capture->d3_height ||
         capture->d3_decoded_fnv1a == 0u ||
         capture->d3_decoded_fnv1a != csb_v1_viewport_fnv1a_bytes_pc34(
             capture->d3_decoded_pixels, capture->d3_decoded_size))) return 0;
    if (!has_d3 && (capture->d3_item_index != 0 ||
                    capture->d3_source_byte_count != 0u ||
                    capture->d3_source_payload_hash != 0u ||
                    capture->d3_decoded_pixels != NULL ||
                    capture->d3_decoded_size != 0u ||
                    capture->d3_decoded_fnv1a != 0u)) return 0;

    for (i = 0; i < plan->command_count; ++i) {
        switch (plan->commands[i].route_bit) {
        case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D2_F0111_DOOR:
            d2_command = i;
            break;
        case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR:
            d3l2_command = i;
            break;
        case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR:
            d3r2_command = i;
            break;
        default:
            break;
        }
    }
    if (d2_command < 0 ||
        bytes->materials[d2_command].decoded_pixels != capture->d2_decoded_pixels ||
        bytes->materials[d2_command].decoded_size != capture->d2_decoded_size ||
        bytes->materials[d2_command].decoded_fnv1a != capture->d2_decoded_fnv1a ||
        bytes->materials[d2_command].width != capture->d2_width ||
        bytes->materials[d2_command].height != capture->d2_height) return 0;
    if (has_d3 &&
        (d3l2_command < 0 || d3r2_command < 0 ||
         bytes->materials[d3l2_command].decoded_pixels != capture->d3_decoded_pixels ||
         bytes->materials[d3r2_command].decoded_pixels != capture->d3_decoded_pixels ||
         bytes->materials[d3l2_command].decoded_size != capture->d3_decoded_size ||
         bytes->materials[d3r2_command].decoded_size != capture->d3_decoded_size ||
         bytes->materials[d3l2_command].decoded_fnv1a != capture->d3_decoded_fnv1a ||
         bytes->materials[d3r2_command].decoded_fnv1a != capture->d3_decoded_fnv1a ||
         bytes->materials[d3l2_command].width != capture->d3_width ||
         bytes->materials[d3r2_command].width != capture->d3_width ||
         bytes->materials[d3l2_command].height != capture->d3_height ||
         bytes->materials[d3r2_command].height != capture->d3_height)) return 0;
    return 1;
}

int csb_v1_viewport_bind_first_frame_material_bytes_pc34(
    const CSB_V1_ViewportFirstFrameMaterialProof *proof,
    CSB_V1_ViewportRuntimeDrawPlanPc34 *plan,
    const CSB_V1_ViewportFirstFrameMaterialBytesPc34 *bytes,
    CSB_V1_ViewportFirstFrameMaterializationReceipt *out_receipt)
{
    int i;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!proof || !plan || !bytes || !bytes->valid || !bytes->source_path ||
        !bytes->source_path[0] || !csb_v1_viewport_md5_text_pc34(bytes->source_md5) ||
        !csb_v1_viewport_first_frame_material_proof_valid_pc34(proof) ||
        !plan->valid || !plan->real_graphics_session ||
        !bytes->palette.decoded_palette || bytes->palette.decoded_size == 0u ||
        bytes->palette.decoded_fnv1a != proof->shared_palette_hash ||
        csb_v1_viewport_fnv1a_bytes_pc34(bytes->palette.decoded_palette,
                                          bytes->palette.decoded_size) !=
            proof->shared_palette_hash) return 0;
    if (!csb_v1_viewport_d2_d3_capture_valid_pc34(proof, plan, bytes)) return 0;
    for (i = 0; i < plan->command_count; ++i) {
        const CSB_V1_ViewportFirstFrameMaterialSpanPc34 *span = &bytes->materials[i];
        const CSB_V1_ViewportRuntimeDrawCommandPc34 *command = &plan->commands[i];
        if (!span->decoded_pixels || span->decoded_size == 0u || span->width <= 0 ||
            span->height <= 0 || (size_t)span->width * span->height != span->decoded_size ||
            span->route_bit != command->route_bit ||
            span->decoded_fnv1a != command->material_hash ||
            span->decoded_fnv1a != csb_v1_viewport_proof_hash_for_route_pc34(
                proof, command->route_bit) ||
            csb_v1_viewport_fnv1a_bytes_pc34(span->decoded_pixels,
                                              span->decoded_size) != span->decoded_fnv1a) return 0;
    }
    if (!csb_v1_viewport_admit_first_frame_materialization_pc34(proof, 1,
                                                                 out_receipt)) return 0;
    if (out_receipt) {
        out_receipt->capture_identity_hash =
            bytes->d2_d3_capture.capture_identity_hash;
        out_receipt->d2_source_payload_hash =
            bytes->d2_d3_capture.d2_source_payload_hash;
        out_receipt->d2_decoded_hash = bytes->d2_d3_capture.d2_decoded_fnv1a;
        out_receipt->d3_source_payload_hash =
            bytes->d2_d3_capture.d3_source_payload_hash;
        out_receipt->d3_decoded_hash = bytes->d2_d3_capture.d3_decoded_fnv1a;
    }
    for (i = 0; i < plan->command_count; ++i) {
        CSB_V1_ViewportRuntimeDrawCommandPc34 *command = &plan->commands[i];
        const CSB_V1_ViewportFirstFrameMaterialSpanPc34 *span = &bytes->materials[i];
        command->decoded_pixels = span->decoded_pixels;
        command->decoded_size = span->decoded_size;
        command->decoded_width = span->width;
        command->decoded_height = span->height;
        command->decoded_palette = bytes->palette.decoded_palette;
        command->decoded_palette_size = bytes->palette.decoded_size;
    }
    return 1;
}

int csb_v1_viewport_consume_first_frame_material_raster_pc34(
    const CSB_V1_ViewportFirstFrameMaterializationReceipt *receipt,
    const CSB_V1_ViewportRuntimeDrawPlanPc34 *plan,
    const CSB_V1_ViewportFirstFrameMaterialBytesPc34 *bytes,
    const char *expected_source_path, const char *expected_source_md5,
    uint8_t *framebuffer, int framebuffer_width, int framebuffer_height,
    CSB_V1_ViewportFirstFrameRasterReceiptPc34 *out_receipt)
{
    uint32_t raster_hash;
    int i;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!receipt || !receipt->valid || !receipt->consumed_by_m11_render ||
        !receipt->real_graphics_session || !receipt->no_synthetic_pixels ||
        !receipt->no_fallback_visuals || !plan || !plan->valid ||
        !plan->real_graphics_session || !plan->no_synthetic_pixels ||
        !plan->no_fallback_visuals ||
        (plan->command_count !=
             CSB_V1_VIEWPORT_RUNTIME_FIRST_FRAME_DRAW_COMMAND_BASE_COUNT_PC34 &&
         plan->command_count !=
             CSB_V1_VIEWPORT_RUNTIME_FIRST_FRAME_DRAW_COMMAND_CAP_PC34) ||
        receipt->required_route_count != plan->command_count ||
        receipt->route_mask != plan->route_mask || !bytes || !bytes->valid ||
        !bytes->source_path || !bytes->source_md5 || !expected_source_path ||
        !expected_source_md5 || !framebuffer || framebuffer_width <= 0 ||
        framebuffer_height <= 0 || strcmp(bytes->source_path, expected_source_path) != 0 ||
        strcmp(bytes->source_md5, expected_source_md5) != 0 ||
        !csb_v1_viewport_md5_text_pc34(expected_source_md5) ||
        !bytes->d2_d3_capture.valid ||
        !bytes->d2_d3_capture.no_synthetic_pixels ||
        !bytes->d2_d3_capture.no_fallback_visuals ||
        receipt->capture_identity_hash == 0u ||
        receipt->capture_identity_hash !=
            bytes->d2_d3_capture.capture_identity_hash ||
        receipt->d2_source_payload_hash !=
            bytes->d2_d3_capture.d2_source_payload_hash ||
        receipt->d2_decoded_hash != bytes->d2_d3_capture.d2_decoded_fnv1a ||
        receipt->d3_source_payload_hash !=
            bytes->d2_d3_capture.d3_source_payload_hash ||
        receipt->d3_decoded_hash != bytes->d2_d3_capture.d3_decoded_fnv1a) goto reject;
    for (i = 0; i < plan->command_count; ++i) {
        const CSB_V1_ViewportRuntimeDrawCommandPc34 *command = &plan->commands[i];
        const CSB_V1_ViewportFirstFrameMaterialSpanPc34 *span = &bytes->materials[i];
        int y;
        if (!span->decoded_pixels || span->decoded_size == 0u ||
            !bytes->palette.decoded_palette || bytes->palette.decoded_size == 0u ||
            command->decoded_pixels != span->decoded_pixels ||
            command->decoded_size != span->decoded_size ||
            command->decoded_width != span->width ||
            command->decoded_height != span->height ||
            command->decoded_palette != bytes->palette.decoded_palette ||
            command->decoded_palette_size != bytes->palette.decoded_size ||
            command->material_hash != span->decoded_fnv1a ||
            command->palette_hash != bytes->palette.decoded_fnv1a ||
            csb_v1_viewport_fnv1a_bytes_pc34(span->decoded_pixels,
                                              span->decoded_size) != span->decoded_fnv1a ||
            csb_v1_viewport_fnv1a_bytes_pc34(bytes->palette.decoded_palette,
                                              bytes->palette.decoded_size) !=
                bytes->palette.decoded_fnv1a ||
            command->clip_x < 0 || command->clip_y < 0 || command->clip_w <= 0 ||
            command->clip_h <= 0 || command->clip_x + command->clip_w > framebuffer_width ||
            command->clip_y + command->clip_h > framebuffer_height) goto reject;
        for (y = 0; y < command->clip_h; ++y) {
            int x;
            int sy = (y * span->height) / command->clip_h;
            for (x = 0; x < command->clip_w; ++x) {
                int sx = (x * span->width) / command->clip_w;
                uint8_t pixel = span->decoded_pixels[(size_t)sy * span->width + sx];
                if (pixel != (uint8_t)command->transparent_color)
                    framebuffer[(size_t)(command->clip_y + y) * framebuffer_width +
                                command->clip_x + x] = pixel;
            }
        }
    }
    raster_hash = csb_v1_viewport_fnv1a_bytes_pc34(
        framebuffer, (size_t)framebuffer_width * framebuffer_height);
    if (out_receipt) {
        out_receipt->valid = 1; out_receipt->consumed_by_raster = 1;
        out_receipt->command_count = plan->command_count;
        out_receipt->combined_material_hash = receipt->combined_material_hash;
        out_receipt->raster_hash = raster_hash;
    }
    return 1;
reject:
    if (out_receipt) out_receipt->rejected = 1;
    return 0;
}

static uint32_t csb_v1_viewport_live_source_identity_hash_pc34(
    const char *path, const char *md5, uint32_t palette_hash)
{
    uint32_t hash;
    if (!path || !md5) return 0u;
    hash = csb_v1_viewport_fnv1a_bytes_pc34((const uint8_t *)path, strlen(path));
    hash = csb_v1_viewport_mix_u32_pc34(hash, csb_v1_viewport_fnv1a_bytes_pc34(
        (const uint8_t *)md5, strlen(md5)));
    hash = csb_v1_viewport_mix_u32_pc34(hash, palette_hash);
    return hash ? hash : 1u;
}

static int csb_v1_viewport_live_frame_source_valid_pc34(
    const CSB_V1_ViewportLiveFrameSourcePc34 *source)
{
    int i;
    if (!source || !source->valid || !source->source_path || !source->source_path[0] ||
        !csb_v1_viewport_md5_text_pc34(source->source_md5) ||
        source->door_state < 0 || source->door_state > 5 ||
        !source->palette.decoded_palette || source->palette.decoded_size == 0u ||
        !source->palette.decoded_fnv1a ||
        csb_v1_viewport_fnv1a_bytes_pc34(source->palette.decoded_palette,
                                          source->palette.decoded_size) !=
            source->palette.decoded_fnv1a) return 0;
    for (i = 0; i < CSB_V1_VIEWPORT_LIVE_FRAME_SURFACE_COUNT_PC34; ++i) {
        const CSB_V1_ViewportLiveSurfaceSpanPc34 *surface = &source->surfaces[i];
        if (surface->kind != (CSB_V1_ViewportLiveSurfaceKindPc34)i ||
            !surface->decoded_pixels || surface->decoded_size == 0u ||
            surface->width <= 0 || surface->height <= 0 ||
            (size_t)surface->width * surface->height != surface->decoded_size ||
            !surface->decoded_fnv1a || surface->clip_x < 0 || surface->clip_y < 0 ||
            surface->clip_w <= 0 || surface->clip_h <= 0 ||
            csb_v1_viewport_fnv1a_bytes_pc34(surface->decoded_pixels,
                                              surface->decoded_size) !=
                surface->decoded_fnv1a) return 0;
    }
    return 1;
}

int csb_v1_viewport_admit_live_frame_progression_pc34(
    CSB_V1_ViewportLiveFrameProgressionPc34 *progression,
    const CSB_V1_ViewportFirstFrameMaterializationReceipt *base_receipt,
    const CSB_V1_ViewportLiveFrameSourcePc34 *source,
    const char *expected_source_path, const char *expected_source_md5,
    CSB_V1_ViewportLiveFrameReceiptPc34 *out_receipt)
{
    uint32_t identity;
    int door_delta;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!progression || !expected_source_path || !expected_source_md5 ||
        !csb_v1_viewport_live_frame_source_valid_pc34(source) ||
        strcmp(source->source_path, expected_source_path) != 0 ||
        strcmp(source->source_md5, expected_source_md5) != 0 ||
        !csb_v1_viewport_md5_text_pc34(expected_source_md5)) return 0;
    identity = csb_v1_viewport_live_source_identity_hash_pc34(
        source->source_path, source->source_md5, source->palette.decoded_fnv1a);
    if (!progression->valid) {
        if (source->frame_number != 0u || !base_receipt || !base_receipt->valid ||
            !base_receipt->consumed_by_m11_render || !base_receipt->real_graphics_session ||
            !base_receipt->no_synthetic_pixels || !base_receipt->no_fallback_visuals) return 0;
    } else {
        door_delta = source->door_state - progression->last_door_state;
        if (source->frame_number != progression->last_frame_number + 1u ||
            progression->source_identity_hash != identity ||
            progression->palette_hash != source->palette.decoded_fnv1a ||
            door_delta < -1 || door_delta > 1) return 0;
    }
    progression->valid = 1;
    progression->last_frame_number = source->frame_number;
    progression->last_door_state = source->door_state;
    progression->source_identity_hash = identity;
    progression->palette_hash = source->palette.decoded_fnv1a;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->consumed_by_m11_render = 1;
        out_receipt->frame_number = source->frame_number;
        out_receipt->door_state = source->door_state;
        out_receipt->source_identity_hash = identity;
        out_receipt->palette_hash = source->palette.decoded_fnv1a;
        out_receipt->wall_hash = source->surfaces[0].decoded_fnv1a;
        out_receipt->floor_hash = source->surfaces[1].decoded_fnv1a;
        out_receipt->door_hash = source->surfaces[2].decoded_fnv1a;
    }
    return 1;
}

int csb_v1_viewport_consume_live_frame_raster_pc34(
    const CSB_V1_ViewportLiveFrameReceiptPc34 *receipt,
    const CSB_V1_ViewportLiveFrameSourcePc34 *source,
    uint8_t *framebuffer, int framebuffer_width, int framebuffer_height,
    uint32_t *out_raster_hash)
{
    int i;
    if (out_raster_hash) *out_raster_hash = 0u;
    if (!receipt || !receipt->valid || !receipt->consumed_by_m11_render ||
        !csb_v1_viewport_live_frame_source_valid_pc34(source) || !framebuffer ||
        framebuffer_width <= 0 || framebuffer_height <= 0 ||
        receipt->frame_number != source->frame_number ||
        receipt->door_state != source->door_state ||
        receipt->palette_hash != source->palette.decoded_fnv1a ||
        receipt->source_identity_hash != csb_v1_viewport_live_source_identity_hash_pc34(
            source->source_path, source->source_md5, source->palette.decoded_fnv1a)) return 0;
    for (i = 0; i < CSB_V1_VIEWPORT_LIVE_FRAME_SURFACE_COUNT_PC34; ++i) {
        const CSB_V1_ViewportLiveSurfaceSpanPc34 *surface = &source->surfaces[i];
        uint32_t expected_hash = i == 0 ? receipt->wall_hash :
            (i == 1 ? receipt->floor_hash : receipt->door_hash);
        int y;
        if (surface->decoded_fnv1a != expected_hash ||
            surface->clip_x + surface->clip_w > framebuffer_width ||
            surface->clip_y + surface->clip_h > framebuffer_height) return 0;
        for (y = 0; y < surface->clip_h; ++y) {
            int x;
            int sy = (y * surface->height) / surface->clip_h;
            for (x = 0; x < surface->clip_w; ++x) {
                int sx = (x * surface->width) / surface->clip_w;
                uint8_t pixel = surface->decoded_pixels[(size_t)sy * surface->width + sx];
                if (pixel != (uint8_t)surface->transparent_color)
                    framebuffer[(size_t)(surface->clip_y + y) * framebuffer_width +
                                surface->clip_x + x] = pixel;
            }
        }
    }
    if (out_raster_hash) *out_raster_hash = csb_v1_viewport_fnv1a_bytes_pc34(
        framebuffer, (size_t)framebuffer_width * framebuffer_height);
    return 1;
}

void csb_v1_viewport_live_frame_material_free_pc34(
    CSB_V1_ViewportLiveFrameMaterialPc34 *material)
{
    int i;
    if (!material) return;
    for (i = 0; i < CSB_V1_VIEWPORT_LIVE_FRAME_SURFACE_COUNT_PC34; ++i) {
        free(material->decoded_surface_bytes[i]);
    }
    memset(material, 0, sizeof(*material));
}

int csb_v1_viewport_materialize_live_frame_from_csbgraphics_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_ViewportLiveFrameDeclarationPc34 *declaration,
    CSB_V1_ViewportLiveFrameMaterialPc34 *out_material)
{
    int i;
    if (out_material) csb_v1_viewport_live_frame_material_free_pc34(out_material);
    if (!cache || !cache->loaded || !cache->file_buffer || !declaration ||
        !out_material || !declaration->source_path || !declaration->source_md5 ||
        !declaration->palette_receipt || !declaration->palette_receipt->valid ||
        strcmp(declaration->source_path, cache->resolved_path) != 0 ||
        strcmp(declaration->source_md5, cache->matched_md5) != 0 ||
        strcmp(declaration->palette_receipt->source_path, cache->resolved_path) != 0 ||
        strcmp(declaration->palette_receipt->source_md5, cache->matched_md5) != 0 ||
        !csb_v1_viewport_md5_text_pc34(cache->matched_md5) ||
        !declaration->palette_receipt->decoded_fnv1a ||
        csb_v1_viewport_fnv1a_bytes_pc34(
            declaration->palette_receipt->decoded_bytes,
            sizeof(declaration->palette_receipt->decoded_bytes)) !=
            declaration->palette_receipt->decoded_fnv1a) return 0;

    memcpy(out_material->source_path, cache->resolved_path,
           sizeof(out_material->source_path) - 1u);
    memcpy(out_material->source_md5, cache->matched_md5,
           sizeof(out_material->source_md5) - 1u);
    memcpy(out_material->palette_bytes, declaration->palette_receipt->decoded_bytes,
           sizeof(out_material->palette_bytes));
    out_material->source.valid = 1;
    out_material->source.frame_number = declaration->frame_number;
    out_material->source.door_state = declaration->door_state;
    out_material->source.source_path = out_material->source_path;
    out_material->source.source_md5 = out_material->source_md5;
    out_material->source.palette.decoded_palette = out_material->palette_bytes;
    out_material->source.palette.decoded_size = sizeof(out_material->palette_bytes);
    out_material->source.palette.decoded_fnv1a = declaration->palette_receipt->decoded_fnv1a;

    for (i = 0; i < CSB_V1_VIEWPORT_LIVE_FRAME_SURFACE_COUNT_PC34; ++i) {
        const CSB_V1_ViewportLiveSurfaceDeclarationPc34 *decl =
            &declaration->surfaces[i];
        CSB_V1_CSBGraphicsEntrySpan span;
        CSB_V1_ViewportLiveSurfaceSpanPc34 *surface =
            &out_material->source.surfaces[i];
        size_t expected_size;
        size_t written = 0u;
        int rc;
        if (decl->width <= 0 || decl->height <= 0 || decl->decoded_fnv1a == 0u ||
            (size_t)decl->width > SIZE_MAX / (size_t)decl->height) goto reject;
        expected_size = (size_t)decl->width * (size_t)decl->height;
        rc = csb_v1_csbgraphics_dat_entry_span(cache->file_buffer, cache->file_size,
                                                decl->entry_index, &span);
        if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
            span.decompressed_size != expected_size || expected_size == 0u) goto reject;
        out_material->decoded_surface_bytes[i] = (uint8_t *)malloc(expected_size);
        if (!out_material->decoded_surface_bytes[i]) goto reject;
        rc = csb_v1_csbgraphics_dat_decode_entry(cache->file_buffer, cache->file_size,
                                                 decl->entry_index,
                                                 out_material->decoded_surface_bytes[i],
                                                 expected_size, &written);
        if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK || written != expected_size ||
            csb_v1_viewport_fnv1a_bytes_pc34(out_material->decoded_surface_bytes[i],
                                              written) != decl->decoded_fnv1a) goto reject;
        surface->kind = (CSB_V1_ViewportLiveSurfaceKindPc34)i;
        surface->decoded_pixels = out_material->decoded_surface_bytes[i];
        surface->decoded_size = written;
        surface->decoded_fnv1a = decl->decoded_fnv1a;
        surface->width = decl->width;
        surface->height = decl->height;
        surface->clip_x = decl->clip_x;
        surface->clip_y = decl->clip_y;
        surface->clip_w = decl->clip_w;
        surface->clip_h = decl->clip_h;
        surface->transparent_color = decl->transparent_color;
    }
    if (!csb_v1_viewport_live_frame_source_valid_pc34(&out_material->source)) goto reject;
    out_material->valid = 1;
    return 1;
reject:
    csb_v1_viewport_live_frame_material_free_pc34(out_material);
    return 0;
}

static uint32_t csb_v1_viewport_live_dungeon_state_hash_pc34(
    const CSB_V1_ViewportLiveDungeonStatePc34 *state)
{
    uint32_t hash;
    if (!state || !state->source_path || !state->source_md5) return 0u;
    hash = csb_v1_viewport_live_source_identity_hash_pc34(
        state->source_path, state->source_md5, 0u);
    hash = csb_v1_viewport_mix_u32_pc34(hash, state->frame_number);
    hash = csb_v1_viewport_mix_u32_pc34(hash, (uint32_t)state->door_state);
    hash = csb_v1_viewport_mix_u32_pc34(hash, state->wall_entry_index);
    hash = csb_v1_viewport_mix_u32_pc34(hash, state->floor_entry_index);
    hash = csb_v1_viewport_mix_u32_pc34(hash, state->door_entry_index);
    return hash ? hash : 1u;
}

int csb_v1_viewport_select_live_dungeon_state_pc34(
    const CSB_V1_ViewportLiveFrameDeclarationPc34 *declarations,
    size_t declaration_count,
    const CSB_V1_ViewportLiveDungeonStatePc34 *state,
    const CSB_V1_ViewportLiveDungeonSelectionPc34 *previous,
    CSB_V1_ViewportLiveDungeonSelectionPc34 *out_selection)
{
    uint32_t identity;
    size_t i;

    if (out_selection) memset(out_selection, 0, sizeof(*out_selection));
    if (!declarations || declaration_count == 0u || !state || !out_selection ||
        state->door_state < 0 || state->door_state > 5 || !state->source_path ||
        !state->source_path[0] || !csb_v1_viewport_md5_text_pc34(state->source_md5)) return 0;
    identity = csb_v1_viewport_live_dungeon_state_hash_pc34(state);
    out_selection->invalidated_previous = previous && previous->valid &&
        previous->state_identity_hash != identity;
    for (i = 0u; i < declaration_count; ++i) {
        const CSB_V1_ViewportLiveFrameDeclarationPc34 *decl = &declarations[i];
        if (!decl->source_path || !decl->source_md5 || !decl->palette_receipt ||
            !decl->palette_receipt->valid || decl->frame_number != state->frame_number ||
            decl->door_state != state->door_state ||
            decl->surfaces[CSB_V1_VIEWPORT_LIVE_SURFACE_WALL_PC34].entry_index !=
                state->wall_entry_index ||
            decl->surfaces[CSB_V1_VIEWPORT_LIVE_SURFACE_FLOOR_PC34].entry_index !=
                state->floor_entry_index ||
            decl->surfaces[CSB_V1_VIEWPORT_LIVE_SURFACE_DOOR_PC34].entry_index !=
                state->door_entry_index ||
            strcmp(decl->source_path, state->source_path) != 0 ||
            strcmp(decl->source_md5, state->source_md5) != 0) continue;
        out_selection->valid = 1;
        out_selection->frame_number = state->frame_number;
        out_selection->door_state = state->door_state;
        out_selection->state_identity_hash = identity;
        out_selection->declaration = decl;
        return 1;
    }
    return 0;
}

int csb_v1_viewport_live_dungeon_state_from_verified_ingress_pc34(
    const CSB_V1_ViewportVerifiedDungeonIngressPc34 *frame_receipt,
    const uint8_t *dungeon_grid, int dungeon_width, int dungeon_height,
    int square_x, int square_y,
    const CSB_V1_ViewportLiveDungeonStatePc34 *explicit_material_identity,
    CSB_V1_ViewportLiveDungeonStatePc34 *out_state)
{
    CSB_V1_ViewportLiveDungeonStatePc34 state;
    uint8_t raw_square;

    if (out_state) memset(out_state, 0, sizeof(*out_state));
    if (!frame_receipt || !dungeon_grid || dungeon_width <= 0 || dungeon_height <= 0 ||
        square_x < 0 || square_y < 0 || square_x >= dungeon_width ||
        square_y >= dungeon_height || !explicit_material_identity || !out_state ||
        !frame_receipt->valid || !frame_receipt->real_asset_matched ||
        !frame_receipt->terminal_session_owned || !frame_receipt->viewport_frame_consumed ||
        !frame_receipt->no_synthetic_surface || frame_receipt->session_generation == 0u ||
        !explicit_material_identity->source_path ||
        !csb_v1_viewport_md5_text_pc34(explicit_material_identity->source_md5)) return 0;
    raw_square = dungeon_grid[(size_t)square_y * (size_t)dungeon_width +
                              (size_t)square_x];
    if ((raw_square & 0x07u) > 5u) return 0;
    state = *explicit_material_identity;
    state.frame_number = frame_receipt->source_tick;
    state.door_state = raw_square & 0x07u;
    *out_state = state;
    return 1;
}

int csb_v1_viewport_admit_operator_declaration_corpus_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_ViewportOperatorDeclarationCorpusPc34 *corpus)
{
    size_t i;
    if (!cache || !cache->loaded || !cache->file_buffer || !corpus ||
        !corpus->valid || !corpus->source_path || !corpus->source_md5 ||
        !corpus->palette_receipt || !corpus->palette_receipt->valid ||
        !corpus->declarations || corpus->declaration_count == 0u ||
        strcmp(corpus->source_path, cache->resolved_path) != 0 ||
        strcmp(corpus->source_md5, cache->matched_md5) != 0 ||
        strcmp(corpus->palette_receipt->source_path, cache->resolved_path) != 0 ||
        strcmp(corpus->palette_receipt->source_md5, cache->matched_md5) != 0 ||
        !csb_v1_viewport_md5_text_pc34(cache->matched_md5)) return 0;
    for (i = 0u; i < corpus->declaration_count; ++i) {
        const CSB_V1_ViewportLiveFrameDeclarationPc34 *decl =
            &corpus->declarations[i];
        if (!decl->source_path || !decl->source_md5 ||
            decl->palette_receipt != corpus->palette_receipt ||
            strcmp(decl->source_path, corpus->source_path) != 0 ||
            strcmp(decl->source_md5, corpus->source_md5) != 0 ||
            decl->door_state < 0 || decl->door_state > 5) return 0;
    }
    return 1;
}

int csb_v1_viewport_parse_operator_declaration_manifest_pc34(
    const char *text, const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsDatPaletteSourceReceipt *palette_receipt,
    CSB_V1_ViewportOperatorDeclarationManifestPc34 *out_manifest)
{
    const char *cursor = text;
    char line[1024];
    int saw_source = 0;
    if (out_manifest) memset(out_manifest, 0, sizeof(*out_manifest));
    if (!text || !cache || !palette_receipt || !out_manifest) return 0;
    while (*cursor) {
        size_t n = 0u;
        int consumed = 0;
        while (cursor[n] && cursor[n] != '\n' && n + 1u < sizeof(line)) ++n;
        if (cursor[n] && cursor[n] != '\n') return 0;
        memcpy(line, cursor, n); line[n] = '\0';
        cursor += cursor[n] == '\n' ? n + 1u : n;
        if (!line[0] || line[0] == '#') continue;
        if (!saw_source) {
            char path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];
            char md5[CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP];
            char extra;
            if (sscanf(line, "source %511s %32s %c", path, md5, &extra) != 2 ||
                strcmp(path, cache->resolved_path) != 0 ||
                strcmp(md5, cache->matched_md5) != 0) return 0;
            snprintf(out_manifest->source_path, sizeof(out_manifest->source_path), "%s", path);
            snprintf(out_manifest->source_md5, sizeof(out_manifest->source_md5), "%s", md5);
            saw_source = 1;
            continue;
        }
        if (out_manifest->declaration_count >= CSB_V1_VIEWPORT_OPERATOR_DECLARATION_MAX_FRAMES_PC34) return 0;
        {
            CSB_V1_ViewportLiveFrameDeclarationPc34 *d =
                &out_manifest->declarations[out_manifest->declaration_count];
            unsigned int f, e[3], h[3]; int door, w[3], ht[3], x[3], y[3], cw[3], ch[3], tr[3]; char extra;
            size_t prior;
            int got = sscanf(line,
                "frame %u %d %u %d %d %u %d %d %d %d %d %u %d %d %u %d %d %d %d %d %u %d %d %u %d %d %d %d %d %c",
                &f,&door,&e[0],&w[0],&ht[0],&h[0],&x[0],&y[0],&cw[0],&ch[0],&tr[0],
                &e[1],&w[1],&ht[1],&h[1],&x[1],&y[1],&cw[1],&ch[1],&tr[1],
                &e[2],&w[2],&ht[2],&h[2],&x[2],&y[2],&cw[2],&ch[2],&tr[2],&extra);
            if (got != 29 || door < 0 || door > 5) return 0;
            for (prior = 0u; prior < out_manifest->declaration_count; ++prior) {
                if (out_manifest->declarations[prior].frame_number == f) return 0;
            }
            d->frame_number = f; d->door_state = door; d->source_path = out_manifest->source_path;
            d->source_md5 = out_manifest->source_md5; d->palette_receipt = palette_receipt;
            for (consumed = 0; consumed < 3; ++consumed) {
                CSB_V1_ViewportLiveSurfaceDeclarationPc34 *s = &d->surfaces[consumed];
                s->entry_index=e[consumed]; s->width=w[consumed]; s->height=ht[consumed]; s->decoded_fnv1a=h[consumed];
                s->clip_x=x[consumed]; s->clip_y=y[consumed]; s->clip_w=cw[consumed]; s->clip_h=ch[consumed]; s->transparent_color=tr[consumed];
            }
            ++out_manifest->declaration_count;
        }
    }
    if (!saw_source || out_manifest->declaration_count == 0u) return 0;
    {
        CSB_V1_ViewportOperatorDeclarationCorpusPc34 corpus;
        memset(&corpus, 0, sizeof(corpus)); corpus.valid=1; corpus.source_path=out_manifest->source_path;
        corpus.source_md5=out_manifest->source_md5; corpus.palette_receipt=palette_receipt;
        corpus.declarations=out_manifest->declarations; corpus.declaration_count=out_manifest->declaration_count;
        if (!csb_v1_viewport_admit_operator_declaration_corpus_pc34(cache, &corpus)) return 0;
    }
    out_manifest->valid = 1;
    out_manifest->palette_receipt = palette_receipt;
    return 1;
}

void csb_v1_viewport_runtime_map_from_relative(
    int party_dir,
    int party_x,
    int party_y,
    int forward,
    int side,
    int *out_x,
    int *out_y)
{
    int x = party_x;
    int y = party_y;

    /* ReDMCSB DUNVIEW.C F0128/F0115 walks visible squares by party
     * direction, forward depth, and side lane.  Runtime object/group
     * overlay scans use that same relative view-space envelope before
     * resolving C2500/C3200 placement. */
    switch (party_dir & 3) {
    case 0:
        x += side;
        y -= forward;
        break;
    case 1:
        x += forward;
        y += side;
        break;
    case 2:
        x -= side;
        y += forward;
        break;
    default:
        x -= forward;
        y -= side;
        break;
    }
    if (out_x) *out_x = x;
    if (out_y) *out_y = y;
}

int csb_v1_viewport_runtime_square_allows_thing_overlay(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int x,
    int y)
{
    int raw_square;
    int square_type;
    int door_state;

    if (!dungeon) {
        return 0;
    }
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, x, y);
    if (raw_square < 0) {
        return 0;
    }
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    if (square_type == 0) {
        return 0;
    }
    if (square_type == 4) {
        /* ReDMCSB CLIKMENU.C F0366 treats door states 0, 1, and 5 as
         * passable/open enough for square interaction.  CSB F0115 runtime
         * overlays follow the same open-door gate before thing-list drawing. */
        door_state = raw_square & 0x07;
        return door_state == 0 || door_state == 1 || door_state == 5;
    }
    if (square_type == 6) {
        /* ReDMCSB CLIKMENU.C F0366 permits only open or imaginary fakewalls. */
        return (raw_square & 0x04) || (raw_square & 0x01);
    }
    return square_type == 1 ||
           square_type == 2 ||
           square_type == 3 ||
           square_type == 5;
}

size_t csb_v1_viewport_runtime_collect_overlay_cells(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int party_dir,
    int party_x,
    int party_y,
    CSB_V1_ViewportRuntimeOverlayCell *out_cells,
    size_t out_capacity)
{
    size_t count = 0;
    int forward;

    if (!dungeon) {
        return 0;
    }
    /* ReDMCSB DUNVIEW.C F0128/F0115 scans far-to-near visible thing
     * cells.  Keeping this in the CSB viewport module prevents M11 from
     * carrying private CSB view-envelope and square-blocking rules. */
    for (forward = 3; forward >= 1; --forward) {
        int side_min;
        int side_max;
        int side;
        if (!csb_v1_viewport_runtime_overlay_side_range(
                forward, &side_min, &side_max)) {
            continue;
        }
        for (side = side_min; side <= side_max; ++side) {
            int map_x = 0;
            int map_y = 0;
            int first_thing;
            csb_v1_viewport_runtime_map_from_relative(party_dir,
                                                      party_x,
                                                      party_y,
                                                      forward,
                                                      side,
                                                      &map_x,
                                                      &map_y);
            if (!csb_v1_viewport_runtime_square_allows_thing_overlay(
                    dungeon, level, map_x, map_y)) {
                continue;
            }
            first_thing =
                csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
            if (first_thing < 0) {
                continue;
            }
            if (out_cells && count < out_capacity) {
                out_cells[count].forward = forward;
                out_cells[count].side = side;
                out_cells[count].map_x = map_x;
                out_cells[count].map_y = map_y;
                out_cells[count].first_thing = first_thing;
            }
            ++count;
        }
    }
    return count;
}

static void csb_v1_viewport_runtime_init_thing_overlay(
    CSB_V1_ViewportRuntimeThingOverlay *overlay)
{
    if (!overlay) return;
    memset(overlay, 0, sizeof(*overlay));
    overlay->kind = (CSB_V1_ViewportRuntimeThingOverlayKind)0;
    overlay->thing = THING_NONE;
    overlay->object_pile_index = -1;
    overlay->group_slot_index = -1;
    overlay->group_cell = -1;
}

size_t csb_v1_viewport_runtime_collect_thing_overlays(
    const CSB_V1_RuntimeProfile *runtime,
    CSB_V1_ViewportRuntimeThingOverlay *out_overlays,
    size_t out_capacity)
{
    const CSB_V1_DungeonData *dungeon;
    CSB_V1_ViewportRuntimeOverlayCell cells[16];
    size_t cell_count;
    size_t count = 0;
    size_t cell_index;

    if (!runtime || !runtime->dungeon_handle) {
        return 0;
    }
    dungeon = runtime->dungeon_handle;
    cell_count = csb_v1_viewport_runtime_collect_overlay_cells(
        dungeon,
        runtime->current_level,
        runtime->party_dir,
        runtime->party_x,
        runtime->party_y,
        cells,
        sizeof(cells) / sizeof(cells[0]));
    if (cell_count > sizeof(cells) / sizeof(cells[0])) {
        cell_count = sizeof(cells) / sizeof(cells[0]);
    }

    /* ReDMCSB DUNVIEW.C F0115 draws visible floor objects before creature
     * groups, then projectiles/explosions overpaint the thing pass.  The CSB
     * viewport owns that ordered overlay plan; M11 only performs asset-backed
     * draw attempts for each returned row. */
    for (cell_index = 0; cell_index < cell_count; ++cell_index) {
        const CSB_V1_ViewportRuntimeOverlayCell *cell = &cells[cell_index];
        unsigned short thing = (unsigned short)cell->first_thing;
        int object_pile_index = 0;
        int safety = 0;

        while (thing != THING_ENDOFLIST && thing != THING_NONE &&
               safety++ < 64) {
            CSB_V1_RuntimeObjectOverlayInfo object_info;
            if (csb_v1_runtime_object_overlay_info(runtime,
                                                   thing,
                                                   &object_info)) {
                CSB_V1_ViewportRuntimeThingOverlay overlay;
                csb_v1_viewport_runtime_init_thing_overlay(&overlay);
                if (csb_v1_viewport_runtime_object_overlay_pile_placement(
                        cell->forward,
                        cell->side,
                        object_info.relative_cell,
                        object_pile_index,
                        &overlay.object_placement)) {
                    overlay.kind = CSB_V1_VIEWPORT_RUNTIME_OVERLAY_OBJECT;
                    overlay.thing = thing;
                    overlay.forward = cell->forward;
                    overlay.side = cell->side;
                    overlay.map_x = cell->map_x;
                    overlay.map_y = cell->map_y;
                    overlay.object_pile_index = object_pile_index;
                    overlay.object_info = object_info;
                    overlay.object_placement.sprite_thing_type =
                        object_info.thing_type;
                    overlay.object_placement.sprite_subtype_index =
                        object_info.subtype_index;
                    overlay.object_placement.sprite_relative_cell =
                        object_info.relative_cell;
                    overlay.object_placement.sprite_pile_index =
                        object_pile_index;
                    overlay.object_placement.icon_index =
                        object_info.icon_index;
                    if (out_overlays && count < out_capacity) {
                        out_overlays[count] = overlay;
                    }
                    ++count;
                }
                ++object_pile_index;
            }
            thing = csb_v1_runtime_next_thing(dungeon, thing);
        }
    }

    for (cell_index = 0; cell_index < cell_count; ++cell_index) {
        const CSB_V1_ViewportRuntimeOverlayCell *cell = &cells[cell_index];
        unsigned short thing = (unsigned short)cell->first_thing;
        int safety = 0;

        while (thing != THING_ENDOFLIST && thing != THING_NONE &&
               safety++ < 64) {
            CSB_V1_RuntimeGroupOverlayInfo group_info;
            if (csb_v1_runtime_group_overlay_info(dungeon,
                                                  thing,
                                                  &group_info)) {
                int slot;
                for (slot = 0; slot < group_info.visible_count; ++slot) {
                    CSB_V1_ViewportRuntimeThingOverlay overlay;
                    int group_cell = group_info.cells[slot];
                    csb_v1_viewport_runtime_init_thing_overlay(&overlay);
                    if (!csb_v1_viewport_runtime_group_overlay_creature_placement(
                            cell->forward,
                            cell->side,
                            group_info.creature_type,
                            group_info.visible_count,
                            group_cell,
                            &overlay.group_placement)) {
                        continue;
                    }
                    overlay.kind = CSB_V1_VIEWPORT_RUNTIME_OVERLAY_GROUP;
                    overlay.thing = thing;
                    overlay.forward = cell->forward;
                    overlay.side = cell->side;
                    overlay.map_x = cell->map_x;
                    overlay.map_y = cell->map_y;
                    overlay.group_slot_index = slot;
                    overlay.group_cell = group_cell;
                    overlay.group_info = group_info;
                    overlay.group_placement.sprite_direction =
                        group_info.direction;
                    if (out_overlays && count < out_capacity) {
                        out_overlays[count] = overlay;
                    }
                    ++count;
                }
                break;
            }
            thing = csb_v1_runtime_next_thing(dungeon, thing);
        }
    }

    return count;
}

static void csb_v1_viewport_plot_runtime_overlay_pixel(
    CSB_V1_ViewportConfig *cfg,
    int viewport_x,
    int viewport_y,
    uint8_t color)
{
    int screen_x;
    int screen_y;
    int stride;

    if (!cfg || !cfg->viewport_pixels) return;
    if (viewport_x < 0 || viewport_x >= DM1_VIEWPORT_WIDTH ||
        viewport_y < 0 || viewport_y >= DM1_VIEWPORT_HEIGHT) {
        return;
    }
    stride = cfg->viewport_stride > 0 ? cfg->viewport_stride : 320;
    screen_x = DM1_VIEWPORT_SCREEN_X + viewport_x;
    screen_y = DM1_VIEWPORT_SCREEN_Y + viewport_y;
    cfg->viewport_pixels[screen_y * stride + screen_x] = color;
}

static void csb_v1_viewport_draw_runtime_overlay_cross(
    CSB_V1_ViewportConfig *cfg,
    int viewport_x,
    int viewport_y,
    uint8_t color,
    int radius)
{
    int d;

    csb_v1_viewport_plot_runtime_overlay_pixel(
        cfg, viewport_x, viewport_y, color);
    for (d = 1; d <= radius; ++d) {
        csb_v1_viewport_plot_runtime_overlay_pixel(
            cfg, viewport_x - d, viewport_y, color);
        csb_v1_viewport_plot_runtime_overlay_pixel(
            cfg, viewport_x + d, viewport_y, color);
        csb_v1_viewport_plot_runtime_overlay_pixel(
            cfg, viewport_x, viewport_y - d, color);
        csb_v1_viewport_plot_runtime_overlay_pixel(
            cfg, viewport_x, viewport_y + d, color);
    }
}

int csb_v1_viewport_projectile_material_overlay_color(int material_icon_index)
{
    if (material_icon_index < 0) return 0x0E;
    return 0x06 + (material_icon_index & 0x07);
}

static void csb_v1_viewport_plot_screen_overlay_pixel(
    uint8_t *screen_pixels,
    int screen_stride,
    int screen_height,
    int screen_x,
    int screen_y,
    uint8_t color)
{
    if (!screen_pixels || screen_stride <= 0 || screen_height <= 0) return;
    if (screen_x < 0 || screen_x >= screen_stride ||
        screen_y < 0 || screen_y >= screen_height) {
        return;
    }
    screen_pixels[screen_y * screen_stride + screen_x] = color;
}

static int csb_v1_viewport_draw_screen_overlay_cross(
    uint8_t *screen_pixels,
    int screen_stride,
    int screen_height,
    int screen_x,
    int screen_y,
    uint8_t color,
    int radius)
{
    int d;

    if (!screen_pixels || screen_stride <= 0 || screen_height <= 0 ||
        screen_x - radius < 0 || screen_x + radius >= screen_stride ||
        screen_y - radius < 0 || screen_y + radius >= screen_height) {
        return 0;
    }
    csb_v1_viewport_plot_screen_overlay_pixel(
        screen_pixels, screen_stride, screen_height, screen_x, screen_y, color);
    for (d = 1; d <= radius; ++d) {
        csb_v1_viewport_plot_screen_overlay_pixel(
            screen_pixels, screen_stride, screen_height,
            screen_x - d, screen_y, color);
        csb_v1_viewport_plot_screen_overlay_pixel(
            screen_pixels, screen_stride, screen_height,
            screen_x + d, screen_y, color);
        csb_v1_viewport_plot_screen_overlay_pixel(
            screen_pixels, screen_stride, screen_height,
            screen_x, screen_y - d, color);
        csb_v1_viewport_plot_screen_overlay_pixel(
            screen_pixels, screen_stride, screen_height,
            screen_x, screen_y + d, color);
    }
    return 1;
}

int csb_v1_viewport_draw_runtime_object_marker(
    uint8_t *screen_pixels,
    int screen_stride,
    int screen_height,
    const CSB_V1_ViewportRuntimeObjectOverlayPlacement *placement,
    int material_icon_index)
{
    uint8_t color;

    if (!placement || !placement->visible) return 0;
    color = (uint8_t)csb_v1_viewport_projectile_material_overlay_color(
        material_icon_index);
    return csb_v1_viewport_draw_screen_overlay_cross(
        screen_pixels,
        screen_stride,
        screen_height,
        placement->marker_screen_x,
        placement->marker_screen_y,
        color,
        1);
}

int csb_v1_viewport_creature_marker_overlay_color(int creature_type)
{
    if (creature_type < 0) {
        return 0x0Du;
    }
    /* Runtime group markers are Firestaff's no-sprite diagnostic fallback.
     * Keep them keyed by C04 GROUP.Type so data-free captures can distinguish
     * creature families while preserving the historic type-6 marker colour
     * used by the M11 CSB fallback gate. */
    return 0x07 + (creature_type & 0x07);
}

int csb_v1_viewport_draw_runtime_group_marker(
    uint8_t *screen_pixels,
    int screen_stride,
    int screen_height,
    const CSB_V1_ViewportRuntimeGroupOverlayPlacement *placement,
    int creature_type)
{
    if (!placement || !placement->visible) return 0;
    return csb_v1_viewport_draw_screen_overlay_cross(
        screen_pixels,
        screen_stride,
        screen_height,
        placement->marker_screen_x,
        placement->marker_screen_y,
        (uint8_t)csb_v1_viewport_creature_marker_overlay_color(creature_type),
        2);
}

static int csb_v1_viewport_runtime_overlay_position(
    int party_dir,
    int party_x,
    int party_y,
    int map_x,
    int map_y,
    int *out_x,
    int *out_y)
{
    int forward;
    int side;

    csb_v1_viewport_runtime_relative_position(
        party_dir, party_x, party_y, map_x, map_y, &forward, &side);
    if (forward < 0 || forward > 4 || side < -2 || side > 2) {
        return 0;
    }
    if (out_x) *out_x = (DM1_VIEWPORT_WIDTH / 2) + side * 42;
    if (out_y) *out_y = 106 - forward * 18;
    return 1;
}

static int csb_v1_viewport_f0115_view_square_index(int forward, int side)
{
    static const signed char k_view_square[3][3] = {
        { 4,  3,  5},
        { 7,  6,  8},
        {12, 11, 13}
    };
    if (forward < 1 || forward > 3) return -1;
    if (side == -2 && forward == 3) return 14;
    if (side == 2 && forward == 3) return 15;
    if (side < -1 || side > 1) return -1;
    return (int)k_view_square[forward - 1][side + 1];
}

static int csb_v1_viewport_f0115_c2500_c2900_row(int forward, int side)
{
    static const signed char k_g2028[23] = {
        11, -1, -1,  8,  9, 10,  5,  6,  7, -1, -1,
         0,  1,  2,  3,  4, -1, -1, -1, -1, -1, -1, -1
    };
    int view_square = csb_v1_viewport_f0115_view_square_index(forward, side);
    if (view_square < 0 || view_square >= 23) return -1;
    return (int)k_g2028[view_square];
}

static int csb_v1_viewport_c2500_source_zone_point(
    int row_index,
    int relative_cell,
    int *out_x,
    int *out_y)
{
    static const short k_c2500_raw[17][4][2] = {
        {{   0,   0}, {   0,   0}, { 127,  70}, {  98,  70}},
        {{   0,   0}, {   0,   0}, {  62,  70}, {  25,  70}},
        {{   0,   0}, {   0,   0}, { 200,  70}, { 162,  70}},
        {{   0,   0}, {   0,   0}, {   2,  70}, { -35,  70}},
        {{   0,   0}, {   0,   0}, { 258,  70}, { 222,  70}},
        {{  94,  78}, { 131,  78}, { 136,  88}, {  89,  88}},
        {{  10,  78}, {  53,  79}, {  41,  88}, { -14,  89}},
        {{ 171,  78}, { 218,  78}, { 236,  89}, { 184,  88}},
        {{  83,  99}, { 141,  99}, { 150, 115}, {  76, 115}},
        {{ -40, 101}, {  24,  99}, {   5, 114}, { -79, 117}},
        {{ 200,  99}, { 262, 101}, { 301, 117}, { 220, 114}},
        {{  66, 133}, { 158, 133}, {   0,   0}, {   0,   0}},
        {{ 113,  62}, {  46,  61}, { 180,  61}, { 115,  74}},
        {{   8,  73}, { 220,  74}, { 115,  92}, { 112,  60}},
        {{  45,  61}, { 179,  60}, { 114,  73}, {   4,  73}},
        {{ 219,  73}, { 114,  88}, { 113,  63}, {  45,  62}},
        {{ 181,  62}, { 114,  74}, {  11,  73}, { 218,  74}}
    };
    int x;
    int y;

    if (row_index < 0 || row_index >= 17 ||
        relative_cell < 0 || relative_cell > 3) {
        return 0;
    }
    x = (int)k_c2500_raw[row_index][relative_cell][0];
    y = (int)k_c2500_raw[row_index][relative_cell][1];
    if (x == 0 && y == 0) {
        return 0;
    }
    if (out_x) *out_x = x;
    if (out_y) *out_y = y;
    return 1;
}

int csb_v1_viewport_runtime_object_source_zone_row(
    int source_zone,
    int fallback_source_zone_row)
{
    int base_zone;

    if (source_zone < 0) {
        return fallback_source_zone_row;
    }
    /* ReDMCSB: DUNVIEW.C F0115, C2500 rows may be tagged with
     * MASK0x8000_SHIFT_OBJECTS_AND_CREATURES before the blit material is
     * consumed. */
    base_zone = source_zone & ~0x8000;
    if (base_zone >= 2500 && base_zone < 2900) {
        return (base_zone - 2500) / 4;
    }
    return fallback_source_zone_row;
}

static int csb_v1_viewport_creature_front_point_index(
    int coordinate_set,
    int visible_count,
    int slot_index)
{
    int point_index = 4;
    if (slot_index < 0) slot_index = 0;
    if (slot_index > 3) slot_index = 3;
    if (coordinate_set == 1) {
        if (visible_count > 1) {
            point_index = slot_index < 2 ? slot_index : 4;
        }
    } else if (visible_count > 1) {
        point_index = slot_index;
        if (point_index > 3) point_index = 3;
    }
    return point_index;
}

static int csb_v1_viewport_object_source_scale_index(
    int depth_index,
    int relative_cell)
{
    int front_row = relative_cell >= 2;
    int index;

    if (depth_index <= 0) return 0;
    index = depth_index * 2 - (front_row ? 1 : 0);
    if (index < 0) index = 0;
    if (index > 4) index = 4;
    return index;
}

static void csb_v1_viewport_object_pile_shift_indices(
    int pile_index,
    int *out_x_index,
    int *out_y_index)
{
    /* ReDMCSB DUNVIEW.C G0217_aauc_Graphic558_ObjectPileShiftSetIndices. */
    static const unsigned char k_indices[16][2] = {
        {2,5}, {0,6}, {5,7}, {3,0},
        {7,1}, {1,2}, {6,3}, {3,3},
        {5,5}, {2,6}, {7,7}, {1,0},
        {3,1}, {6,2}, {1,3}, {5,3}
    };

    if (pile_index < 0) pile_index = 0;
    pile_index &= 0x0F;
    if (out_x_index) *out_x_index = (int)k_indices[pile_index][0];
    if (out_y_index) *out_y_index = (int)k_indices[pile_index][1];
}

static int csb_v1_viewport_object_shift_value(
    int shift_set,
    int shift_index)
{
    /* ReDMCSB DUNVIEW.C G0223_aac_Graphic558_ShiftSets. */
    static const signed char k_sets[3][8] = {
        { 0, 1, 2, 3, 0,-3,-2,-1},
        { 0, 1, 1, 2, 0,-2,-1,-1},
        { 0, 1, 1, 1, 0,-1,-1,-1}
    };

    if (shift_set < 0) shift_set = 0;
    if (shift_set > 2) shift_set = 2;
    if (shift_index < 0) shift_index = 0;
    if (shift_index > 7) shift_index = 7;
    return (int)k_sets[shift_set][shift_index];
}

static int csb_v1_viewport_c3200_creature_zone_point(
    int coordinate_set,
    int depth_index,
    int side,
    int visible_count,
    int slot_index,
    int *out_x,
    int *out_y)
{
    static const short k_center[3][3][5][2] = {
        {
            {{ 83,106}, {141,106}, {148,119}, { 76,119}, {112,111}},
            {{ 92, 83}, {131, 83}, {132, 90}, { 91, 90}, {112, 85}},
            {{ 97, 67}, {125, 67}, {129, 72}, { 95, 72}, {112, 72}}
        },
        {
            {{ 81,119}, {142,119}, {112,105}, {112,111}, {112,119}},
            {{ 91, 90}, {132, 90}, {112, 83}, {112, 85}, {112, 89}},
            {{ 94, 73}, {128, 73}, {112, 70}, {112, 70}, {112, 73}}
        },
        {
            {{ 83, 79}, {141, 79}, {148, 85}, { 76, 85}, {112, 81}},
            {{ 92, 65}, {131, 65}, {132, 67}, { 91, 67}, {112, 66}},
            {{ 95, 59}, {127, 59}, {129, 61}, { 93, 61}, {112, 60}}
        }
    };
    static const short k_side[3][3][2][5][2] = {
        {
            {{{ 46,103}, {118,103}, {101,119}, {  0,  0}, { 79,111}},
             {{107,103}, {177,103}, {  0,  0}, {123,119}, {144,111}}},
            {{{ 99, 81}, {146, 81}, {135, 90}, { 80, 90}, {120, 85}},
             {{ 77, 81}, {124, 81}, {143, 90}, { 89, 90}, {105, 85}}},
            {{{131, 70}, {163, 70}, {158, 75}, {120, 75}, {145, 72}},
             {{ 59, 70}, { 91, 70}, {107, 75}, { 66, 75}, { 79, 72}}}
        },
        {
            {{{  0,  0}, {101,119}, { 84,105}, { 70,111}, { 77,119}},
             {{123,119}, {  0,  0}, {139,105}, {153,111}, {146,119}}},
            {{{ 80, 90}, {135, 90}, {125, 83}, {120, 85}, {125, 90}},
             {{ 89, 90}, {143, 90}, { 99, 83}, {105, 85}, { 98, 90}}},
            {{{120, 75}, {158, 75}, {149, 70}, {145, 72}, {150, 75}},
             {{ 66, 75}, {104, 75}, { 75, 70}, { 79, 72}, { 73, 75}}}
        },
        {
            {{{ 46, 79}, {118, 79}, {101, 85}, {  0,  0}, { 79, 81}},
             {{107, 79}, {177, 79}, {  0,  0}, {123, 85}, {144, 81}}},
            {{{ 99, 65}, {146, 65}, {135, 67}, { 80, 67}, {120, 66}},
             {{ 77, 65}, {124, 65}, {143, 67}, { 89, 67}, {105, 66}}},
            {{{131, 59}, {163, 59}, {158, 61}, {120, 61}, {145, 60}},
             {{ 59, 59}, { 91, 59}, {107, 61}, { 66, 61}, { 79, 60}}}
        }
    };
    int point_index;
    int side_index;

    if (coordinate_set < 0 || coordinate_set > 2) return 0;
    if (depth_index < 0) depth_index = 0;
    if (depth_index > 2) depth_index = 2;
    point_index = csb_v1_viewport_creature_front_point_index(
        coordinate_set, visible_count, slot_index);
    if (side < 0 || side > 0) {
        side_index = side < 0 ? 0 : 1;
        if (out_x) *out_x =
            (int)k_side[coordinate_set][depth_index][side_index][point_index][0];
        if (out_y) *out_y =
            (int)k_side[coordinate_set][depth_index][side_index][point_index][1];
    } else {
        if (out_x) *out_x =
            (int)k_center[coordinate_set][depth_index][point_index][0];
        if (out_y) *out_y =
            (int)k_center[coordinate_set][depth_index][point_index][1];
    }
    return 1;
}

int csb_v1_viewport_runtime_object_overlay_placement(
    int forward,
    int side,
    int relative_cell,
    CSB_V1_ViewportRuntimeObjectOverlayPlacement *out_placement)
{
    return csb_v1_viewport_runtime_object_overlay_pile_placement(
        forward,
        side,
        relative_cell,
        0,
        out_placement);
}

int csb_v1_viewport_runtime_object_overlay_pile_placement(
    int forward,
    int side,
    int relative_cell,
    int pile_index,
    CSB_V1_ViewportRuntimeObjectOverlayPlacement *out_placement)
{
    CSB_V1_ViewportRuntimeObjectOverlayPlacement placement;
    const CSB_V1_ViewportObjectBlitSpec *spec = NULL;
    int x = 0;
    int y = 0;

    memset(&placement, 0, sizeof(placement));
    placement.forward = forward;
    placement.side = side;
    placement.view_cell = relative_cell;
    placement.view_square =
        csb_v1_viewport_f0115_view_square_index(forward, side);
    placement.object_row =
        csb_v1_viewport_f0115_c2500_c2900_row(forward, side);
    placement.source_zone = -1;
    placement.sprite_thing_type = -1;
    placement.sprite_subtype_index = -1;
    placement.sprite_relative_cell = relative_cell;
    placement.sprite_pile_index = pile_index < 0 ? 0 : pile_index;
    /* ReDMCSB DUNVIEW.C F0115/F0128 draws runtime floor-object sprites
     * inside the 224x136 viewport pane, with row/depth selected by the
     * CSB F0115 view-square route.  Keep the full M11 sprite call contract
     * here so the shared renderer does not own CSB-specific geometry. */
    placement.sprite_viewport_x = 0;
    placement.sprite_viewport_y = DM1_VIEWPORT_SCREEN_Y;
    placement.sprite_viewport_w = DM1_VIEWPORT_WIDTH;
    placement.sprite_viewport_h = DM1_VIEWPORT_HEIGHT;
    placement.sprite_depth_index = forward - 1;
    placement.sprite_transparent_color = CSB_V1_F0115_TRANSPARENT_COLOR;
    placement.sprite_uses_f0791_blit = 1;
    if (placement.sprite_depth_index < 0) {
        placement.sprite_depth_index = 0;
    }
    placement.pile_index = placement.sprite_pile_index;
    placement.object_scale_index =
        csb_v1_viewport_object_source_scale_index(forward - 1,
                                                  relative_cell);
    placement.shift_set = (placement.object_scale_index + 1) >> 1;
    if (placement.shift_set > 2) placement.shift_set = 2;
    csb_v1_viewport_object_pile_shift_indices(placement.pile_index,
                                              &placement.shift_x_index,
                                              &placement.shift_y_index);
    placement.pile_shift_x =
        csb_v1_viewport_object_shift_value(placement.shift_set,
                                           placement.shift_x_index);
    placement.pile_shift_y =
        csb_v1_viewport_object_shift_value(placement.shift_set,
                                           placement.shift_y_index);
    if (placement.view_square < 0 ||
        relative_cell < 0 ||
        relative_cell > 3) {
        if (out_placement) *out_placement = placement;
        return 0;
    }
    if (placement.view_square >= 0) {
        spec = csb_v1_viewport_get_object_blit_spec_for_square(
            placement.view_square);
        placement.source_zone = csb_v1_viewport_object_blit_zone(
            spec, (unsigned char)relative_cell);
        if (spec) {
            placement.sprite_transparent_color = spec->transparent_color;
            placement.sprite_uses_f0791_blit = spec->uses_f0791_blit;
        }
    }
    if (placement.object_row >= 0 &&
        csb_v1_viewport_c2500_source_zone_point(
            placement.object_row, relative_cell, &x, &y)) {
        placement.visible = 1;
        placement.used_source_zone = 1;
        placement.viewport_x = x;
        placement.viewport_y = y;
        placement.screen_x = x;
        placement.screen_y = DM1_VIEWPORT_SCREEN_Y + y;
    } else {
        placement.visible = 1;
        placement.viewport_x = (DM1_VIEWPORT_WIDTH / 2) + side * 42;
        placement.viewport_y = 108 - forward * 24;
        placement.screen_x = DM1_VIEWPORT_SCREEN_X + placement.viewport_x;
        placement.screen_y = DM1_VIEWPORT_SCREEN_Y + placement.viewport_y;
    }
    placement.marker_screen_x = placement.screen_x + placement.pile_shift_x;
    placement.marker_screen_y = placement.screen_y + placement.pile_shift_y;
    placement.icon_index = -1;
    placement.icon_screen_x = placement.marker_screen_x;
    placement.icon_screen_y = placement.marker_screen_y;
    if (relative_cell == 0 || relative_cell == 2) {
        placement.icon_screen_x -= 5;
    }
    if (relative_cell == 1 || relative_cell == 3) {
        placement.icon_screen_x += 5;
    }
    if (relative_cell >= 2) {
        placement.icon_screen_y += 3;
    }
    /* The shared M11 icon blitter takes top-left coordinates, while the
     * CSB F0115 object placement contract exposes the source cell center.
     * Keep that 16x16 icon-origin conversion in the CSB viewport layer. */
    placement.icon_draw_x = placement.icon_screen_x - 8;
    placement.icon_draw_y = placement.icon_screen_y - 8;
    if (out_placement) *out_placement = placement;
    return placement.visible;
}

int csb_v1_viewport_runtime_object_sprite_blit(
    const CSB_V1_ViewportRuntimeObjectOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeObjectSpriteBlit *out_blit)
{
    CSB_V1_ViewportRuntimeObjectSpriteBlit blit;

    memset(&blit, 0, sizeof(blit));
    blit.thing_type = -1;
    blit.subtype_index = -1;
    if (!placement || !out_blit || !placement->visible ||
        placement->sprite_subtype_index < 0) {
        if (out_blit) *out_blit = blit;
        return 0;
    }

    /* ReDMCSB DUNVIEW.C F0115 object draw path routes all runtime
     * floor-object asset parameters through the current view cell's
     * C2500/object-row contract.  Expose that as one CSB-owned blit
     * record so callers only perform the asset copy. */
    blit.thing_type = placement->sprite_thing_type;
    blit.subtype_index = placement->sprite_subtype_index;
    blit.relative_cell = placement->sprite_relative_cell;
    blit.pile_index = placement->sprite_pile_index;
    blit.viewport_x = placement->sprite_viewport_x;
    blit.viewport_y = placement->sprite_viewport_y;
    blit.viewport_w = placement->sprite_viewport_w;
    blit.viewport_h = placement->sprite_viewport_h;
    blit.depth_index = placement->sprite_depth_index;
    blit.source_zone = placement->source_zone;
    blit.source_zone_row = placement->object_row;
    blit.transparent_color = placement->sprite_transparent_color;
    blit.uses_f0791_blit = placement->sprite_uses_f0791_blit;
    *out_blit = blit;
    return blit.thing_type >= 0;
}

int csb_v1_viewport_runtime_object_icon_blit(
    const CSB_V1_ViewportRuntimeObjectOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeObjectIconBlit *out_blit)
{
    CSB_V1_ViewportRuntimeObjectIconBlit blit;

    memset(&blit, 0, sizeof(blit));
    blit.icon_index = -1;
    if (!placement || !out_blit || !placement->visible ||
        placement->icon_index < 0) {
        if (out_blit) *out_blit = blit;
        return 0;
    }

    blit.icon_index = placement->icon_index;
    blit.draw_x = placement->icon_draw_x;
    blit.draw_y = placement->icon_draw_y;
    blit.transparent_color = 0;
    *out_blit = blit;
    return 1;
}

int csb_v1_viewport_runtime_group_overlay_placement(
    int forward,
    int side,
    int coordinate_set,
    CSB_V1_ViewportRuntimeGroupOverlayPlacement *out_placement)
{
    return csb_v1_viewport_runtime_group_overlay_slot_placement(
        forward, side, coordinate_set, 1, 0, out_placement);
}

int csb_v1_viewport_runtime_creature_coordinate_set(int creature_type)
{
    /* ReDMCSB DUNVIEW.C G0219_as_Graphic558_CreatureAspects, high nibble
     * of coordinateSet_transparentColor. This mirrors the CSB/DM1 runtime
     * creature type order already consumed by the M11 CSB overlay bridge. */
    static const unsigned char k_coordinate_sets[27] = {
        1, 0, 0, 2, 1, 1, 0, 0, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 2,
        0, 0, 1, 1, 0, 1, 1, 1, 1
    };

    if (creature_type < 0 ||
        creature_type >= (int)(sizeof(k_coordinate_sets) /
                               sizeof(k_coordinate_sets[0]))) {
        return 0;
    }
    return (int)k_coordinate_sets[creature_type];
}

int csb_v1_viewport_runtime_group_overlay_creature_placement(
    int forward,
    int side,
    int creature_type,
    int visible_count,
    int creature_cell,
    CSB_V1_ViewportRuntimeGroupOverlayPlacement *out_placement)
{
    int visible = csb_v1_viewport_runtime_group_overlay_slot_placement(
        forward,
        side,
        csb_v1_viewport_runtime_creature_coordinate_set(creature_type),
        visible_count,
        creature_cell,
        out_placement);
    if (out_placement) {
        out_placement->sprite_creature_type = creature_type;
        out_placement->sprite_relative_side = side;
    }
    return visible;
}

int csb_v1_viewport_runtime_group_overlay_slot_placement(
    int forward,
    int side,
    int coordinate_set,
    int visible_count,
    int slot_index,
    CSB_V1_ViewportRuntimeGroupOverlayPlacement *out_placement)
{
    CSB_V1_ViewportRuntimeGroupOverlayPlacement placement;
    const CSB_V1_ViewportCreatureVisibilitySpec *spec = NULL;
    int x = 0;
    int y = 0;

    memset(&placement, 0, sizeof(placement));
    placement.forward = forward;
    placement.side = side;
    placement.view_cell = slot_index;
    placement.coordinate_set = coordinate_set;
    placement.depth_index = forward - 1;
    placement.sprite_creature_type = -1;
    placement.sprite_direction = 0;
    placement.sprite_relative_side = side;
    placement.sprite_coordinate_set = coordinate_set;
    placement.sprite_source_zone = -1;
    placement.sprite_shift_mask = CSB_V1_CREATURE_SHIFT_MASK;
    placement.sprite_transparent_color = CSB_V1_F0115_TRANSPARENT_COLOR;
    placement.sprite_uses_f0791_blit = 1;
    placement.view_square =
        csb_v1_viewport_f0115_view_square_index(forward, side);
    placement.source_zone = -1;
    if (placement.view_square < 0 ||
        coordinate_set < 0 ||
        coordinate_set > 2) {
        if (out_placement) *out_placement = placement;
        return 0;
    }
    if (placement.view_square >= 0) {
        spec = csb_v1_viewport_get_creature_visibility_spec_for_square(
            placement.view_square);
        placement.source_zone = csb_v1_viewport_creature_visibility_zone(
            spec, coordinate_set, 0);
        placement.sprite_source_zone = placement.source_zone;
        if (spec) {
            placement.sprite_shift_mask = spec->shifts_objects_and_creatures;
        }
    }
    if (csb_v1_viewport_c3200_creature_zone_point(
            coordinate_set,
            forward - 1,
            side,
            visible_count,
            slot_index,
            &x,
            &y)) {
        placement.visible = 1;
        placement.used_source_zone = 1;
        placement.viewport_x = x;
        placement.viewport_y = y;
        placement.screen_x = x;
        placement.screen_y = DM1_VIEWPORT_SCREEN_Y + y;
    } else {
        placement.visible = 1;
        placement.viewport_x = (DM1_VIEWPORT_WIDTH / 2) + side * 42;
        placement.viewport_y = 86 - forward * 24;
        placement.screen_x = DM1_VIEWPORT_SCREEN_X + placement.viewport_x;
        placement.screen_y = DM1_VIEWPORT_SCREEN_Y + placement.viewport_y;
    }
    placement.sprite_w = 54 - placement.depth_index * 12;
    placement.sprite_h = 70 - placement.depth_index * 14;
    if (placement.sprite_w < 20) placement.sprite_w = 20;
    if (placement.sprite_h < 28) placement.sprite_h = 28;
    placement.sprite_x = placement.screen_x - placement.sprite_w / 2;
    placement.sprite_y = placement.screen_y - placement.sprite_h;
    placement.marker_screen_x = placement.screen_x;
    placement.marker_screen_y = placement.screen_y;
    if (out_placement) *out_placement = placement;
    return placement.visible;
}

int csb_v1_viewport_runtime_group_sprite_blit(
    const CSB_V1_ViewportRuntimeGroupOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeGroupSpriteBlit *out_blit)
{
    CSB_V1_ViewportRuntimeGroupSpriteBlit blit;

    memset(&blit, 0, sizeof(blit));
    blit.creature_type = -1;
    if (!placement || !out_blit || !placement->visible ||
        placement->sprite_creature_type < 0) {
        if (out_blit) *out_blit = blit;
        return 0;
    }

    blit.creature_type = placement->sprite_creature_type;
    blit.direction = placement->sprite_direction;
    blit.relative_side = placement->sprite_relative_side;
    blit.x = placement->sprite_x;
    blit.y = placement->sprite_y;
    blit.w = placement->sprite_w;
    blit.h = placement->sprite_h;
    blit.depth_index = placement->depth_index;
    blit.coordinate_set = placement->sprite_coordinate_set;
    blit.source_zone = placement->sprite_source_zone;
    blit.shift_mask = placement->sprite_shift_mask;
    blit.transparent_color = placement->sprite_transparent_color;
    blit.uses_f0791_blit = placement->sprite_uses_f0791_blit;
    *out_blit = blit;
    return 1;
}

int csb_v1_viewport_runtime_overlay_side_range(
    int forward,
    int *out_min_side,
    int *out_max_side)
{
    int min_side = -1;
    int max_side = 1;

    if (forward < 1 || forward > 3) {
        if (out_min_side) *out_min_side = 0;
        if (out_max_side) *out_max_side = -1;
        return 0;
    }
    if (forward == 3) {
        /* ReDMCSB DUNVIEW.C F0676/F0677 adds CSB's D3L2/D3R2
         * side squares around the normal D3L/D3C/D3R thing pass. */
        min_side = -2;
        max_side = 2;
    }
    if (out_min_side) *out_min_side = min_side;
    if (out_max_side) *out_max_side = max_side;
    return 1;
}

static int csb_v1_viewport_c2900_source_zone_point(
    int row_index,
    int view_cell,
    int *out_x,
    int *out_y)
{
    static const short k_c2900_raw[12][4][2] = {
        {{  0,  0}, {  0,  0}, {129, 47}, { 95, 47}},
        {{  0,  0}, {  0,  0}, { 62, 47}, { 25, 47}},
        {{  0,  0}, {  0,  0}, {200, 47}, {162, 47}},
        {{  0,  0}, {  0,  0}, {  2, 47}, {-35, 47}},
        {{  0,  0}, {  0,  0}, {258, 47}, {202, 47}},
        {{ 92, 47}, {132, 46}, {136, 47}, { 88, 47}},
        {{ 10, 47}, { 53, 47}, { 41, 47}, {-14, 47}},
        {{171, 47}, {218, 47}, {236, 47}, {183, 47}},
        {{ 83, 47}, {140, 47}, {148, 47}, { 76, 47}},
        {{-40, 47}, { 26, 47}, {  5, 47}, {-79, 47}},
        {{197, 47}, {262, 47}, {301, 47}, {220, 47}},
        {{ 66, 47}, {158, 47}, {  0,  0}, {  0,  0}}
    };
    int x;
    int y;

    if (row_index < 0 || row_index >= 12 ||
        view_cell < 0 || view_cell > 3) {
        return 0;
    }
    x = (int)k_c2900_raw[row_index][view_cell][0];
    y = (int)k_c2900_raw[row_index][view_cell][1];
    if (x == 0 && y == 0) {
        return 0;
    }
    if (out_x) *out_x = x;
    if (out_y) *out_y = y;
    return 1;
}

int csb_v1_viewport_runtime_projectile_sprite_rect(
    int source_zone,
    int viewport_x,
    int viewport_y,
    int *out_source_zone_row,
    int *out_x,
    int *out_y,
    int *out_w,
    int *out_h)
{
    int source_zone_row = -1;
    int x = viewport_x - 16;
    int y = DM1_VIEWPORT_SCREEN_Y + viewport_y - 16;
    int w = 32;
    int h = 32;

    if (source_zone >= 2900) {
        source_zone_row = (source_zone - 2900) / 4;
        x = 0;
        y = DM1_VIEWPORT_SCREEN_Y;
        w = DM1_VIEWPORT_WIDTH;
        h = DM1_VIEWPORT_HEIGHT;
    }
    if (out_source_zone_row) *out_source_zone_row = source_zone_row;
    if (out_x) *out_x = x;
    if (out_y) *out_y = y;
    if (out_w) *out_w = w;
    if (out_h) *out_h = h;
    return 1;
}

int csb_v1_viewport_runtime_projectile_overlay_placement(
    int party_dir,
    int party_x,
    int party_y,
    int projectile_map_x,
    int projectile_map_y,
    int projectile_cell,
    CSB_V1_ViewportRuntimeProjectileOverlayPlacement *out_placement)
{
    CSB_V1_ViewportRuntimeProjectileOverlayPlacement placement;
    const CSB_V1_ViewportProjectileBlitSpec *spec = NULL;
    int x;
    int y;

    memset(&placement, 0, sizeof(placement));
    placement.view_square = -1;
    placement.view_cell = projectile_cell;
    placement.source_zone = -1;
    placement.source_zone_row = -1;
    placement.sprite_aspect_index = -1;
    placement.sprite_relative_dir = -1;
    placement.sprite_relative_cell = -1;
    placement.sprite_graphic_index = -1;
    placement.sprite_flip_flags = 0;
    placement.sprite_derived_bitmap_cache_slot =
        CSB_V1_PROJECTILE_DERIVED_BITMAP_NONE;
    placement.sprite_transparent_color = CSB_V1_F0115_TRANSPARENT_COLOR;
    placement.sprite_uses_f0791_blit = 1;
    placement.material_thing = -1;
    placement.material_icon_index = -1;
    placement.viewport_x = 0;
    placement.viewport_y = 0;
    csb_v1_viewport_runtime_relative_position(
        party_dir,
        party_x,
        party_y,
        projectile_map_x,
        projectile_map_y,
        &placement.forward,
        &placement.side);
    if (placement.forward < 0 || placement.forward > 4 ||
        placement.side < -2 || placement.side > 2 ||
        projectile_cell < 0 || projectile_cell > 3) {
        if (out_placement) *out_placement = placement;
        return 0;
    }

    if (placement.forward == 3 &&
        (placement.side == -2 || placement.side == 2)) {
        placement.view_square =
            placement.side < 0 ? (int)DM1_VIEW_SQUARE_D3L2
                               : (int)DM1_VIEW_SQUARE_D3R2;
        spec = csb_v1_viewport_get_projectile_blit_spec_for_square(
            placement.view_square);
        placement.source_zone =
            csb_v1_viewport_projectile_blit_zone(
                spec, (unsigned char)projectile_cell);
        if (spec) {
            placement.sprite_derived_bitmap_cache_slot =
                spec->derived_bitmap_cache_slot_for_scaled_path;
            placement.sprite_transparent_color = spec->transparent_color;
            placement.sprite_uses_f0791_blit = spec->uses_f0791_blit;
        }
        if (placement.source_zone < 0 ||
            !csb_v1_viewport_c2900_source_zone_point(
                spec ? spec->projectile_visibility_row : -1,
                projectile_cell,
                &placement.viewport_x,
                &placement.viewport_y)) {
            if (out_placement) *out_placement = placement;
            return 0;
        }
        placement.visible = 1;
        csb_v1_viewport_runtime_projectile_sprite_rect(
            placement.source_zone,
            placement.viewport_x,
            placement.viewport_y,
            &placement.source_zone_row,
            &placement.sprite_x,
            &placement.sprite_y,
            &placement.sprite_w,
            &placement.sprite_h);
        if (out_placement) *out_placement = placement;
        return 1;
    }

    if (!csb_v1_viewport_runtime_overlay_position(
            party_dir,
            party_x,
            party_y,
            projectile_map_x,
            projectile_map_y,
            &x,
            &y)) {
        if (out_placement) *out_placement = placement;
        return 0;
    }
    placement.visible = 1;
    placement.viewport_x = x;
    placement.viewport_y = y;
    csb_v1_viewport_runtime_projectile_sprite_rect(
        placement.source_zone,
        placement.viewport_x,
        placement.viewport_y,
        &placement.source_zone_row,
        &placement.sprite_x,
        &placement.sprite_y,
        &placement.sprite_w,
        &placement.sprite_h);
    if (out_placement) *out_placement = placement;
    return 1;
}

int csb_v1_viewport_runtime_bind_projectile_sprite(
    int party_dir,
    const struct ProjectileInstance_Compat *projectile,
    CSB_V1_ViewportRuntimeProjectileOverlayPlacement *placement)
{
    int aspect;
    int relative_dir;
    int relative_cell;

    if (!projectile || !placement) {
        return 0;
    }
    aspect = dm1_v1_projectile_subtype_to_aspect(
        projectile->projectileSubtype);
    if (aspect < 0) {
        placement->sprite_aspect_index = -1;
        placement->sprite_relative_dir = -1;
        placement->sprite_relative_cell = -1;
        placement->sprite_graphic_index = -1;
        placement->sprite_flip_flags = 0;
        return 0;
    }

    relative_dir = (projectile->direction - party_dir) & 3;
    relative_cell = (placement->view_cell - party_dir) & 3;
    placement->sprite_aspect_index = aspect;
    placement->sprite_relative_dir = relative_dir;
    placement->sprite_relative_cell = relative_cell;
    placement->sprite_graphic_index =
        dm1_v1_projectile_graphic_index(aspect, relative_dir);
    placement->sprite_flip_flags =
        dm1_v1_projectile_flip_flags(aspect,
                                     relative_dir,
                                     relative_cell,
                                     projectile->mapX,
                                     projectile->mapY);
    return placement->sprite_graphic_index >= 0;
}

int csb_v1_viewport_runtime_bind_projectile_material(
    const CSB_V1_RuntimeProfile *runtime,
    const struct ProjectileInstance_Compat *projectile,
    CSB_V1_ViewportRuntimeProjectileOverlayPlacement *placement)
{
    unsigned short thing;

    if (!projectile || !placement) {
        return 0;
    }

    thing = (unsigned short)projectile->reserved1;
    placement->material_thing = (int)thing;
    placement->material_icon_index = -1;

    if (!runtime || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return 0;
    }

    placement->material_icon_index =
        csb_v1_runtime_object_icon_index(runtime, thing);
    return placement->material_icon_index >= 0;
}

int csb_v1_viewport_runtime_projectile_sprite_blit(
    const CSB_V1_ViewportRuntimeProjectileOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeProjectileSpriteBlit *out_blit)
{
    CSB_V1_ViewportRuntimeProjectileSpriteBlit blit;

    memset(&blit, 0, sizeof(blit));
    blit.graphic_index = -1;
    blit.relative_dir = -1;
    blit.relative_cell = -1;
    blit.source_zone_row = -1;
    if (!placement || !out_blit || !placement->visible ||
        placement->sprite_graphic_index < 0) {
        if (out_blit) *out_blit = blit;
        return 0;
    }

    /* ReDMCSB DUNVIEW.C F0115 projectile path binds the C2900/F0791
     * source row, view-relative direction/cell, flip flags, and target
     * rectangle before the bitmap copy. Keep that draw contract in the
     * CSB viewport layer so M11 only blits the resolved asset. */
    blit.view_cell = placement->view_cell;
    blit.source_zone = placement->source_zone;
    blit.source_zone_row = placement->source_zone_row;
    blit.viewport_x = placement->viewport_x;
    blit.viewport_y = placement->viewport_y;
    blit.x = placement->sprite_x;
    blit.y = placement->sprite_y;
    blit.w = placement->sprite_w;
    blit.h = placement->sprite_h;
    blit.aspect_index = placement->sprite_aspect_index;
    blit.graphic_index = placement->sprite_graphic_index;
    blit.forward = placement->forward;
    blit.relative_dir = placement->sprite_relative_dir;
    blit.relative_cell = placement->sprite_relative_cell;
    blit.flip_flags = placement->sprite_flip_flags;
    blit.source_zone_row = placement->source_zone_row;
    blit.derived_bitmap_cache_slot =
        placement->sprite_derived_bitmap_cache_slot;
    blit.transparent_color = placement->sprite_transparent_color;
    blit.uses_f0791_blit = placement->sprite_uses_f0791_blit;
    *out_blit = blit;
    return 1;
}

int csb_v1_viewport_runtime_projectile_material_icon_blit(
    const CSB_V1_ViewportRuntimeProjectileOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeObjectIconBlit *out_blit)
{
    CSB_V1_ViewportRuntimeObjectIconBlit blit;

    memset(&blit, 0, sizeof(blit));
    blit.icon_index = -1;
    if (!placement || !out_blit || !placement->visible ||
        placement->material_icon_index < 0) {
        if (out_blit) *out_blit = blit;
        return 0;
    }

    /* ReDMCSB DUNVIEW.C F0115 draws visible thrown-object material at the
     * projectile C2900 point when no spell projectile bitmap is bound.
     * Keep the centered icon fallback in the CSB viewport contract. */
    blit.icon_index = placement->material_icon_index;
    blit.draw_x = DM1_VIEWPORT_SCREEN_X + placement->viewport_x - 8;
    blit.draw_y = DM1_VIEWPORT_SCREEN_Y + placement->viewport_y - 8;
    blit.transparent_color = 0;
    *out_blit = blit;
    return 1;
}

int csb_v1_viewport_runtime_explosion_sprite_rect(
    int forward,
    int source_zone,
    int viewport_x,
    int viewport_y,
    int *out_depth_index,
    int *out_x,
    int *out_y,
    int *out_w,
    int *out_h)
{
    int depth_index = forward;
    int x = viewport_x - 16;
    int y = DM1_VIEWPORT_SCREEN_Y + viewport_y - 16;
    int w = 32;
    int h = 32;

    if (depth_index < 0) depth_index = 0;
    if (depth_index > 2) depth_index = 2;
    if (source_zone >= 0) {
        x = 0;
        y = DM1_VIEWPORT_SCREEN_Y;
        w = DM1_VIEWPORT_WIDTH;
        h = DM1_VIEWPORT_HEIGHT;
    }
    if (out_depth_index) *out_depth_index = depth_index;
    if (out_x) *out_x = x;
    if (out_y) *out_y = y;
    if (out_w) *out_w = w;
    if (out_h) *out_h = h;
    return 1;
}

int csb_v1_viewport_runtime_bind_explosion_sprite(
    const struct ExplosionInstance_Compat *explosion,
    CSB_V1_ViewportRuntimeExplosionOverlayPlacement *placement)
{
    int aspect;

    if (!explosion || !placement) {
        return 0;
    }
    aspect = dm1_v1_explosion_type_to_aspect(explosion->explosionType);
    if (aspect < 0) {
        placement->sprite_aspect_index = -1;
        placement->sprite_graphic_index = -1;
        placement->sprite_is_smoke = 0;
        placement->sprite_frame = -1;
        placement->sprite_max_frames = -1;
        placement->sprite_attack = -1;
        return 0;
    }
    placement->sprite_aspect_index = aspect;
    placement->sprite_graphic_index =
        dm1_v1_explosion_aspect_to_graphic(aspect);
    placement->sprite_is_smoke =
        dm1_v1_explosion_is_smoke(explosion->explosionType);
    placement->sprite_frame = explosion->currentFrame;
    placement->sprite_max_frames = explosion->maxFrames;
    placement->sprite_attack = explosion->attack;
    return placement->sprite_graphic_index >= 0;
}

int csb_v1_viewport_runtime_explosion_sprite_blit(
    const CSB_V1_ViewportRuntimeExplosionOverlayPlacement *placement,
    CSB_V1_ViewportRuntimeExplosionSpriteBlit *out_blit)
{
    CSB_V1_ViewportRuntimeExplosionSpriteBlit blit;

    memset(&blit, 0, sizeof(blit));
    blit.aspect_index = -1;
    blit.graphic_index = -1;
    blit.frame = -1;
    blit.max_frames = -1;
    blit.attack = -1;
    if (!placement || !out_blit || !placement->visible ||
        placement->sprite_graphic_index < 0 ||
        placement->sprite_aspect_index < 0) {
        if (out_blit) *out_blit = blit;
        return 0;
    }

    /* ReDMCSB DUNVIEW.C F0115 final explosion pass resolves the explosion
     * aspect, GRAPHICS.DAT bitmap, animation frame, attack strength, and
     * source-zone rectangle before the F0114/F0675 draw. M11 consumes this
     * CSB-owned blit record without reinterpreting placement fields. */
    blit.view_cell = placement->view_cell;
    blit.source_zone = placement->source_zone;
    blit.source_zone_row = placement->source_zone_row;
    blit.viewport_x = placement->viewport_x;
    blit.viewport_y = placement->viewport_y;
    blit.x = placement->sprite_x;
    blit.y = placement->sprite_y;
    blit.w = placement->sprite_w;
    blit.h = placement->sprite_h;
    blit.forward = placement->forward;
    blit.aspect_index = placement->sprite_aspect_index;
    blit.graphic_index = placement->sprite_graphic_index;
    blit.is_smoke = placement->sprite_is_smoke;
    blit.frame = placement->sprite_frame;
    blit.max_frames = placement->sprite_max_frames;
    blit.attack = placement->sprite_attack;
    blit.depth_index = placement->depth_index;
    blit.transparent_color = placement->sprite_transparent_color;
    blit.uses_f0791_blit = placement->sprite_uses_f0791_blit;
    *out_blit = blit;
    return 1;
}

int csb_v1_viewport_runtime_explosion_overlay_placement(
    int party_dir,
    int party_x,
    int party_y,
    int explosion_map_x,
    int explosion_map_y,
    int explosion_cell,
    CSB_V1_ViewportRuntimeExplosionOverlayPlacement *out_placement)
{
    CSB_V1_ViewportRuntimeExplosionOverlayPlacement placement;
    const CSB_V1_ViewportExplosionBlitSpec *spec = NULL;
    int x;
    int y;

    memset(&placement, 0, sizeof(placement));
    placement.view_square = -1;
    placement.view_cell = explosion_cell;
    placement.source_zone = -1;
    placement.source_zone_row = -1;
    placement.depth_index = 0;
    placement.sprite_aspect_index = -1;
    placement.sprite_graphic_index = -1;
    placement.sprite_is_smoke = 0;
    placement.sprite_frame = -1;
    placement.sprite_max_frames = -1;
    placement.sprite_attack = -1;
    placement.sprite_transparent_color = CSB_V1_F0115_TRANSPARENT_COLOR;
    placement.sprite_uses_f0791_blit = 1;
    csb_v1_viewport_runtime_relative_position(
        party_dir,
        party_x,
        party_y,
        explosion_map_x,
        explosion_map_y,
        &placement.forward,
        &placement.side);
    if (placement.forward < 0 || placement.forward > 4 ||
        placement.side < -2 || placement.side > 2) {
        if (out_placement) *out_placement = placement;
        return 0;
    }

    if (placement.forward == 3 &&
        (placement.side == -2 || placement.side == 2)) {
        placement.view_square =
            placement.side < 0 ? (int)DM1_VIEW_SQUARE_D3L2
                               : (int)DM1_VIEW_SQUARE_D3R2;
        spec = csb_v1_viewport_get_explosion_blit_spec_for_square(
            placement.view_square);
        if (spec) {
            placement.sprite_transparent_color = spec->transparent_color;
            placement.sprite_uses_f0791_blit = spec->uses_f0791_blit;
        }
        if (explosion_cell == EXPLOSION_CELL_CENTERED) {
            placement.source_zone =
                csb_v1_viewport_explosion_centered_zone(spec);
        } else if (explosion_cell == 0 || explosion_cell == 1) {
            placement.source_zone =
                csb_v1_viewport_explosion_side_zone(
                    spec, (unsigned char)explosion_cell);
        }
        if (placement.source_zone < 0) {
            if (out_placement) *out_placement = placement;
            return 0;
        }
        if (!csb_v1_viewport_runtime_overlay_position(
                party_dir,
                party_x,
                party_y,
                explosion_map_x,
                explosion_map_y,
                &x,
                &y)) {
            if (out_placement) *out_placement = placement;
            return 0;
        }
        placement.visible = 1;
        placement.used_source_zone = 1;
        placement.viewport_x = x;
        placement.viewport_y = y;
        placement.source_zone_row = 0;
        csb_v1_viewport_runtime_explosion_sprite_rect(
            placement.forward,
            placement.source_zone,
            placement.viewport_x,
            placement.viewport_y,
            &placement.depth_index,
            &placement.sprite_x,
            &placement.sprite_y,
            &placement.sprite_w,
            &placement.sprite_h);
        if (out_placement) *out_placement = placement;
        return 1;
    }

    if (!csb_v1_viewport_runtime_overlay_position(
            party_dir,
            party_x,
            party_y,
            explosion_map_x,
            explosion_map_y,
            &x,
            &y)) {
        if (out_placement) *out_placement = placement;
        return 0;
    }
    placement.visible = 1;
    placement.viewport_x = x;
    placement.viewport_y = y;
    csb_v1_viewport_runtime_explosion_sprite_rect(
        placement.forward,
        placement.source_zone,
        placement.viewport_x,
        placement.viewport_y,
        &placement.depth_index,
        &placement.sprite_x,
        &placement.sprite_y,
        &placement.sprite_w,
        &placement.sprite_h);
    if (out_placement) *out_placement = placement;
    return 1;
}

static void csb_v1_viewport_draw_runtime_projectile_overlays(
    CSB_V1_ViewportConfig *cfg,
    int party_dir,
    int party_x,
    int party_y)
{
    int i;

    if (!cfg || !cfg->runtime_projectiles) return;
    cfg->runtime_projectile_sprite_drawn_count = 0;
    cfg->runtime_projectile_material_resolved_count = 0;
    cfg->runtime_projectile_material_icon_drawn_count = 0;
    cfg->runtime_projectile_marker_drawn_count = 0;
    if (cfg->runtime_overlay_source_required &&
        (!cfg->runtime_overlay_source_admitted ||
         cfg->runtime_overlay_source_hash == 0u)) {
        return;
    }
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat *projectile =
            &cfg->runtime_projectiles->entries[i];
        CSB_V1_ViewportRuntimeProjectileOverlayPlacement placement;
        uint8_t color = 0x0Eu;

        if (projectile->reserved3 == 0 || projectile->slotIndex < 0) {
            continue;
        }
        if (!csb_v1_viewport_runtime_projectile_overlay_placement(
                party_dir,
                party_x,
                party_y,
                projectile->mapX,
                projectile->mapY,
                projectile->cell,
                &placement)) {
            continue;
        }
        (void)csb_v1_viewport_runtime_bind_projectile_sprite(
            party_dir,
            projectile,
            &placement);
        (void)csb_v1_viewport_runtime_bind_projectile_material(
            cfg->runtime_profile,
            projectile,
            &placement);
        /* ReDMCSB DUNVIEW.C F0115 lines 5668-5683 map projectiles through
         * G2028 and C2900_ZONE_ + row*4 + ViewCell.  OBJECT.C F0032/F0033
         * resolves the material thing identity used by the fallback marker. */
        if (cfg->projectile_sprite_drawer) {
            CSB_V1_ViewportRuntimeProjectileSpriteBlit blit;
            if (csb_v1_viewport_runtime_projectile_sprite_blit(
                    &placement, &blit) &&
                cfg->projectile_sprite_drawer(
                    cfg->projectile_sprite_user,
                    &blit,
                    cfg->viewport_pixels,
                    cfg->viewport_stride)) {
                ++cfg->runtime_projectile_sprite_drawn_count;
                continue;
            }
        }
        if (placement.material_icon_index >= 0) {
            ++cfg->runtime_projectile_material_resolved_count;
            if (cfg->object_icon_drawer) {
                CSB_V1_ViewportRuntimeObjectIconBlit icon_blit;
                if (csb_v1_viewport_runtime_projectile_material_icon_blit(
                        &placement, &icon_blit) &&
                    cfg->object_icon_drawer(
                        cfg->object_icon_user,
                        &icon_blit,
                        cfg->viewport_pixels,
                        cfg->viewport_stride)) {
                    ++cfg->runtime_projectile_material_icon_drawn_count;
                    continue;
                }
            }
        }
        color = (uint8_t)csb_v1_viewport_projectile_material_overlay_color(
            placement.material_icon_index);
        csb_v1_viewport_draw_runtime_overlay_cross(
            cfg,
            placement.viewport_x,
            placement.viewport_y,
            color,
            placement.source_zone >= 0 ? 2 : 1);
        ++cfg->runtime_projectile_marker_drawn_count;
    }
}

static void csb_v1_viewport_draw_runtime_thing_overlays(
    CSB_V1_ViewportConfig *cfg)
{
    CSB_V1_ViewportRuntimeThingOverlay overlays[80];
    size_t overlay_count;
    size_t overlay_index;
    const int screen_height = 200;

    if (!cfg || !cfg->runtime_profile || !cfg->viewport_pixels) {
        return;
    }

    cfg->runtime_object_sprite_drawn_count = 0;
    cfg->runtime_object_icon_drawn_count = 0;
    cfg->runtime_object_marker_drawn_count = 0;
    cfg->runtime_group_sprite_drawn_count = 0;
    cfg->runtime_group_marker_drawn_count = 0;
    if (cfg->runtime_overlay_source_required &&
        (!cfg->runtime_overlay_source_admitted ||
         cfg->runtime_overlay_source_hash == 0u)) {
        return;
    }

    overlay_count = csb_v1_viewport_runtime_collect_thing_overlays(
        cfg->runtime_profile,
        overlays,
        sizeof(overlays) / sizeof(overlays[0]));
    if (overlay_count > sizeof(overlays) / sizeof(overlays[0])) {
        overlay_count = sizeof(overlays) / sizeof(overlays[0]);
    }

    /* ReDMCSB: DUNVIEW.C F0115 lines 5170-5656 draws floor objects and
     * creature groups before projectiles.  CSB viewport owns the ordered
     * thing-list pass; callers may only provide asset-backed blitters. */
    for (overlay_index = 0; overlay_index < overlay_count; ++overlay_index) {
        const CSB_V1_ViewportRuntimeThingOverlay *overlay =
            &overlays[overlay_index];

        if (overlay->kind == CSB_V1_VIEWPORT_RUNTIME_OVERLAY_OBJECT) {
            const CSB_V1_ViewportRuntimeObjectOverlayPlacement *placement =
                &overlay->object_placement;
            const int icon = placement->icon_index;

            if (placement->sprite_subtype_index >= 0 &&
                cfg->object_sprite_drawer) {
                CSB_V1_ViewportRuntimeObjectSpriteBlit blit;
                if (csb_v1_viewport_runtime_object_sprite_blit(
                        placement, &blit) &&
                    cfg->object_sprite_drawer(
                        cfg->object_sprite_user,
                        &blit,
                        cfg->viewport_pixels,
                        cfg->viewport_stride)) {
                    ++cfg->runtime_object_sprite_drawn_count;
                } else if (icon >= 0 &&
                           cfg->object_icon_drawer) {
                    CSB_V1_ViewportRuntimeObjectIconBlit icon_blit;
                    if (csb_v1_viewport_runtime_object_icon_blit(
                            placement, &icon_blit) &&
                        cfg->object_icon_drawer(
                            cfg->object_icon_user,
                            &icon_blit,
                            cfg->viewport_pixels,
                            cfg->viewport_stride)) {
                        ++cfg->runtime_object_icon_drawn_count;
                    } else if (csb_v1_viewport_draw_runtime_object_marker(
                                   cfg->viewport_pixels,
                                   cfg->viewport_stride,
                                   screen_height,
                                   placement,
                                   icon)) {
                        ++cfg->runtime_object_marker_drawn_count;
                    }
                } else if (csb_v1_viewport_draw_runtime_object_marker(
                               cfg->viewport_pixels,
                               cfg->viewport_stride,
                               screen_height,
                               placement,
                               icon)) {
                    ++cfg->runtime_object_marker_drawn_count;
                }
            } else if (icon >= 0 &&
                       cfg->object_icon_drawer) {
                CSB_V1_ViewportRuntimeObjectIconBlit icon_blit;
                if (csb_v1_viewport_runtime_object_icon_blit(
                        placement, &icon_blit) &&
                    cfg->object_icon_drawer(
                        cfg->object_icon_user,
                        &icon_blit,
                        cfg->viewport_pixels,
                        cfg->viewport_stride)) {
                    ++cfg->runtime_object_icon_drawn_count;
                } else if (csb_v1_viewport_draw_runtime_object_marker(
                               cfg->viewport_pixels,
                               cfg->viewport_stride,
                               screen_height,
                               placement,
                               icon)) {
                    ++cfg->runtime_object_marker_drawn_count;
                }
            } else if (csb_v1_viewport_draw_runtime_object_marker(
                           cfg->viewport_pixels,
                           cfg->viewport_stride,
                           screen_height,
                           placement,
                           icon)) {
                ++cfg->runtime_object_marker_drawn_count;
            }
        } else if (overlay->kind == CSB_V1_VIEWPORT_RUNTIME_OVERLAY_GROUP) {
            const CSB_V1_ViewportRuntimeGroupOverlayPlacement *placement =
                &overlay->group_placement;
            const int creature_type = placement->sprite_creature_type;

            if (creature_type >= 0 &&
                cfg->group_sprite_drawer) {
                CSB_V1_ViewportRuntimeGroupSpriteBlit blit;
                if (csb_v1_viewport_runtime_group_sprite_blit(
                        placement, &blit) &&
                    cfg->group_sprite_drawer(
                        cfg->group_sprite_user,
                        &blit,
                        cfg->viewport_pixels,
                        cfg->viewport_stride)) {
                    ++cfg->runtime_group_sprite_drawn_count;
                } else if (csb_v1_viewport_draw_runtime_group_marker(
                               cfg->viewport_pixels,
                               cfg->viewport_stride,
                               screen_height,
                               placement,
                               creature_type)) {
                    ++cfg->runtime_group_marker_drawn_count;
                }
            } else if (csb_v1_viewport_draw_runtime_group_marker(
                           cfg->viewport_pixels,
                           cfg->viewport_stride,
                           screen_height,
                           placement,
                           creature_type)) {
                ++cfg->runtime_group_marker_drawn_count;
            }
        }
    }
}

static void csb_v1_viewport_draw_runtime_explosion_overlays(
    CSB_V1_ViewportConfig *cfg,
    int party_dir,
    int party_x,
    int party_y)
{
    int i;

    if (!cfg || !cfg->runtime_explosions) return;
    cfg->runtime_explosion_sprite_drawn_count = 0;
    cfg->runtime_explosion_marker_drawn_count = 0;
    if (cfg->runtime_overlay_source_required &&
        (!cfg->runtime_overlay_source_admitted ||
         cfg->runtime_overlay_source_hash == 0u)) {
        return;
    }
    for (i = 0; i < EXPLOSION_LIST_CAPACITY; ++i) {
        const struct ExplosionInstance_Compat *explosion =
            &cfg->runtime_explosions->entries[i];
        CSB_V1_ViewportRuntimeExplosionOverlayPlacement placement;

        if (explosion->reserved0 == 0 || explosion->slotIndex < 0) {
            continue;
        }
        if (!csb_v1_viewport_runtime_explosion_overlay_placement(
                party_dir,
                party_x,
                party_y,
                explosion->mapX,
                explosion->mapY,
                explosion->cell,
                &placement)) {
            continue;
        }
        (void)csb_v1_viewport_runtime_bind_explosion_sprite(
            explosion,
            &placement);
        /* ReDMCSB DUNVIEW.C F0115 lines 5916-6200 restarts the thing list
         * for explosions after all object/creature/projectile cells and maps
         * D3L2/D3R2 through C3014/C3031 via G2034. */
        if (cfg->explosion_sprite_drawer) {
            CSB_V1_ViewportRuntimeExplosionSpriteBlit blit;
            if (csb_v1_viewport_runtime_explosion_sprite_blit(
                    &placement, &blit) &&
                cfg->explosion_sprite_drawer(
                    cfg->explosion_sprite_user,
                    &blit,
                    cfg->viewport_pixels,
                    cfg->viewport_stride)) {
                ++cfg->runtime_explosion_sprite_drawn_count;
                continue;
            }
        }
        csb_v1_viewport_draw_runtime_overlay_cross(
            cfg,
            placement.viewport_x,
            placement.viewport_y,
            0x0Cu,
            placement.used_source_zone ? 3 : 2);
        ++cfg->runtime_explosion_marker_drawn_count;
    }
}

/* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 draws the ordinary
 * floor/ceiling backdrop before the square pass sequence, with F0098 lines
 * 2995-3002 consuming G2109/G2108 and clearing the redraw request.  CSBWin
 * adds the CSB-only CustomBackgrounds pass between that baseline backdrop and
 * cell drawing: Viewport.cpp lines 5317-5325 supply room relative positions,
 * 6567-6615 resolves/apply skin bitmaps, and 6919-7140 inserts room slots. */
static const CSB_V1_ViewportCustomBackgroundSlotSpec s_custom_background_slots[] = {
    { 0, 3, -2, 0, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6919 room 0 before F3L1 draw path." },
    { 2, 3, -1, 1, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6920 room 2 before F3L1 draw path." },
    { 1, 3, 2, 2, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6940 room 1 before F3R1 draw path." },
    { 3, 3, 1, 3, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6941 room 3 before F3R1 draw path." },
    { 4, 3, 0, 4, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6961 room 4 before F3 draw path." },
    { 5, 2, -2, 5, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6981 room 5 before F2L1 draw path." },
    { 7, 2, -1, 6, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6982 room 7 before F2L1 draw path." },
    { 6, 2, 2, 7, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7002 room 6 before F2R1 draw path." },
    { 8, 2, 1, 8, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7003 room 8 before F2R1 draw path." },
    { 9, 2, 0, 9, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7023 room 9 before F2 draw path." },
    { 10, 1, -1, 10, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7043 room 10 before F1L1 draw path." },
    { 11, 1, 1, 11, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7063 room 11 before F1R1 draw path." },
    { 12, 1, 0, 12, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7081 room 12 before F1 draw path." },
    { 13, 0, -1, 13, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7102 room 13 before F0L1 draw path." },
    { 14, 0, 1, 14, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7122 room 14 before F0R1 draw path." },
    { 15, 0, 0, 15, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7140 room 15 before F0 draw path." }
};

/* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 and 8443 draw the base
 * floor/ceiling pass through F0098 lines 2962-3002 before cell drawing.
 * CSBWin extends that baseline with CSD/CSD-I34 background-library bitmaps:
 * Viewport.cpp lines 5402-5412 load the skin-selected bitmap, 6444-6470
 * applies the mask composite, and 6567-6615 gates each runtime layer. */
static const char s_custom_background_application_source[] =
    "ReDMCSB DUNVIEW.C:8337-8339,8443 F0128 floor/ceiling baseline; "
    "2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 "
    "relposSid/relposFwd; 5402-5412 GetBitmap selects CSD/CSD-I34 "
    "background-library bitmaps instead of ReDMCSB base bitmaps; 6444-6470 "
    "ApplyBackground masked composite; 6567-6615 CustomBackgrounds runtime "
    "skin/default/mask/bitmap application.";

#define CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(slot_index, applies_near) \
    { \
        &s_custom_background_slots[(slot_index)], \
        1, 1, 1, \
        CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_GRAPHIC_ID, \
        CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_MIN_BYTES, \
        (applies_near), \
        CSB_V1_CUSTOM_BACKGROUND_NEAR_ROOM_LIMIT, \
        1, 0, 1, 1, \
        "CustomBackgrounds", \
        s_custom_background_application_source \
    }

static const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec
    s_custom_background_application_specs[] = {
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(0, 1),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(1, 1),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(2, 1),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(3, 1),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(4, 1),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(5, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(6, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(7, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(8, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(9, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(10, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(11, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(12, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(13, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(14, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(15, 0)
    };

#undef CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC

typedef struct {
    int bitmap_skin_def_index;
    int mask_skin_def_index;
    int bitmap_min_bytes;
    int byte_width;
    int height;
} CSB_V1_CustomBackgroundLayerSelection;

static uint16_t csb_v1_read_le16(const uint8_t *bytes)
{
    return (uint16_t)(bytes[0] | ((uint16_t)bytes[1] << 8));
}

static int csb_v1_viewport_custom_background_layer_for_view(
    CSB_V1_ViewportCustomBackgroundViewIndex view_index,
    CSB_V1_CustomBackgroundLayerSelection *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    switch (view_index) {
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L2:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D3C:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D3R:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D3R2:
            out->bitmap_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_SKIN_DEF_INDEX;
            out->mask_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_SKIN_DEF_INDEX;
            out->bitmap_min_bytes = CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_MIN_BYTES;
            out->byte_width = 112;
            out->height = 70;
            return 1;
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D2L2:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D2L:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D2C:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D2R:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D2R2:
            out->bitmap_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_SKIN_DEF_INDEX;
            out->mask_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_SKIN_DEF_INDEX;
            out->bitmap_min_bytes = CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_MIN_BYTES;
            out->byte_width = 56;
            out->height = 58;
            return 1;
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D1L:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D1C:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D1R:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D0L:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D0C:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D0R:
            out->bitmap_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_SKIN_DEF_INDEX;
            out->mask_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_SKIN_DEF_INDEX;
            out->bitmap_min_bytes = CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_MIN_BYTES;
            out->byte_width = 56;
            out->height = 74;
            return 1;
        default:
            return 0;
    }
}

static const CSB_V1_ViewportWallOrnamentRouteSpec s_wall_ornament_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        DM1_PC34_ZONE_WALL_D3L2,
        1,
        CSB_V1_ORNAMENT_SLOT_RIGHT,
        CSB_V1_VIEW_WALL_D3L2_RIGHT,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6254-6263 wall panel then F0107(M551_RIGHT_WALL_ORNAMENT_ORDINAL, C00_VIEW_WALL_D3L2_RIGHT); DEFS.H:2696"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        DM1_PC34_ZONE_WALL_D3R2,
        1,
        CSB_V1_ORNAMENT_SLOT_LEFT,
        CSB_V1_VIEW_WALL_D3R2_LEFT,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6321-6330 wall panel then F0107(M553_LEFT_WALL_ORNAMENT_ORDINAL, C01_VIEW_WALL_D3R2_LEFT); DEFS.H:2697"
    },
    {
        (int)DM1_VIEW_SQUARE_D2L2,
        DM1_PC34_ZONE_WALL_D2L2,
        0,
        CSB_V1_NO_ORNAMENT_SLOT,
        CSB_V1_NO_VIEW_WALL,
        "F0678_DrawD2L2",
        "DUNVIEW.C:6848-6865 wall case returns without F0107; teleporter field is the only non-wall draw"
    },
    {
        (int)DM1_VIEW_SQUARE_D2R2,
        DM1_PC34_ZONE_WALL_D2R2,
        0,
        CSB_V1_NO_ORNAMENT_SLOT,
        CSB_V1_NO_VIEW_WALL,
        "F0679_DrawD2R2",
        "DUNVIEW.C:6877-6896 wall case returns without F0107; teleporter field is the only non-wall draw"
    },
};

/* ReDMCSB: DUNVIEW.C F0107 lines 3502-3590, 3817-3829, and
 * 3921-3923; F0676/F0677 lines 6263 and 6330.  The CSB/I34 far-side
 * wall-ornament views return early for ordinal 0, pre-decrement to the
 * current-map wall ornament index, build C1004 + CoordinateSet * 15 +
 * ViewWallIndex, scale through F0675 at C30/C14 with the D3 palette table,
 * flip only C01_VIEW_WALL_D3R2_LEFT, and dispatch the pixels through F0791
 * using C10 transparency. */
static const CSB_V1_ViewportWallOrnamentBlitSpec s_wall_ornament_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_WALL_D3L2_RIGHT,
        1,
        CSB_V1_WALL_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_WALL_ORNAMENT_D3L2_BITMAP_INCREMENT,
        CSB_V1_WALL_ORNAMENT_ZONE_BASE,
        CSB_V1_WALL_ORNAMENT_COORD_STRIDE,
        CSB_V1_WALL_ORNAMENT_SCALE_X_D3,
        CSB_V1_WALL_ORNAMENT_SCALE_Y_D3,
        CSB_V1_FLIP_NONE,
        CSB_V1_WALL_ORNAMENT_TRANSPARENT_COLOR,
        1,
        1,
        "M551_RIGHT_WALL_ORNAMENT_ORDINAL",
        "F0676_DrawD3L2",
        "DUNVIEW.C:6263 F0107(M551, C00_VIEW_WALL_D3L2_RIGHT); F0107:3571 skips ordinal 0; 3575 ordinal--; 3576 reads native bitmap; 3587 zone C1004 + CoordinateSet*15 + ViewWall; 3817-3819 leaves D3L2 unflipped; 3824-3829 F0675 C30/C14 with G0198 D3 palette; 3921-3923 F0791 C10 blit. DEFS.H:2696,4222; DUNVIEW.C:805-819 G0190 MEDIA720; COORD.C:921-1025 C1000..C1107 layout records."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_WALL_D3R2_LEFT,
        1,
        CSB_V1_WALL_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_WALL_ORNAMENT_D3R2_BITMAP_INCREMENT,
        CSB_V1_WALL_ORNAMENT_ZONE_BASE,
        CSB_V1_WALL_ORNAMENT_COORD_STRIDE,
        CSB_V1_WALL_ORNAMENT_SCALE_X_D3,
        CSB_V1_WALL_ORNAMENT_SCALE_Y_D3,
        CSB_V1_FLIP_HORIZONTAL,
        CSB_V1_WALL_ORNAMENT_TRANSPARENT_COLOR,
        1,
        1,
        "M553_LEFT_WALL_ORNAMENT_ORDINAL",
        "F0677_DrawD3R2",
        "DUNVIEW.C:6330 F0107(M553, C01_VIEW_WALL_D3R2_LEFT); F0107:3571 skips ordinal 0; 3575 ordinal--; 3576 reads native bitmap; 3587 zone C1004 + CoordinateSet*15 + ViewWall; 3817-3819 flips C01_VIEW_WALL_D3R2_LEFT; 3824-3829 F0675 C30/C14 with G0198 D3 palette; 3921-3923 F0791 C10 blit. DEFS.H:2697,4222; DUNVIEW.C:805-819 G0190 MEDIA720; COORD.C:921-1025 C1000..C1107 layout records."
    },
};

/* ReDMCSB: DUNVIEW.C F0107 lines 3589, 3608-3753, 3817-3829,
 * and 3921-3928; DUNGEON.C F0149 lines 1330-1347.  The CSB/I34
 * D3L2/D3R2 wall-ornament views still evaluate whether the ornament is an
 * alcove, but because view-wall indices 0/1 are below M585_VIEW_WALL_D1L
 * and are not M587_VIEW_WALL_D1C_FRONT, they stay on the D3 scaled-bitmap
 * path and do not update the D1-front interaction state or champion
 * portrait overlay. */
static const CSB_V1_ViewportWallOrnamentSideEffectSpec s_wall_ornament_side_effects[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_WALL_D3L2_RIGHT,
        1,
        0,
        0,
        0,
        0,
        0,
        1,
        CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6263 calls F0107(C00_VIEW_WALL_D3L2_RIGHT). F0107:3589 evaluates F0149_DUNGEON_IsWallOrnamentAnAlcove; 3608 gates D1-only facing/clickbox state; 3726-3744 updates facing alcove/Vi altar/fountain only inside that D1 branch; 3817-3829 routes C00/C01 through I34 D3 scaled bitmap with CM1_DERIVED_BITMAP_NONE; 3923-3928 champion portrait overlay is only M587_VIEW_WALL_D1C_FRONT. DUNGEON.C:1330-1347 F0149 alcove predicate. DEFS.H:2696/2708-2710."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_WALL_D3R2_LEFT,
        1,
        0,
        0,
        0,
        0,
        0,
        1,
        CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6330 calls F0107(C01_VIEW_WALL_D3R2_LEFT). F0107:3589 evaluates F0149_DUNGEON_IsWallOrnamentAnAlcove; 3608 gates D1-only facing/clickbox state; 3726-3744 updates facing alcove/Vi altar/fountain only inside that D1 branch; 3817-3829 routes C00/C01 through I34 D3 scaled bitmap with CM1_DERIVED_BITMAP_NONE; 3923-3928 champion portrait overlay is only M587_VIEW_WALL_D1C_FRONT. DUNGEON.C:1330-1347 F0149 alcove predicate. DEFS.H:2697/2708-2710."
    },
};

/* ReDMCSB: DUNVIEW.C F0107 lines 3571-3589, 3608-3753,
 * 3817-3860, and 3921-3928; F0119/F0120/F0121/F0122/F0123/F0124
 * wall branches at lines 6968-6969, 7119-7120, 7308, 7459, 7627,
 * and 7842.  These D1/D2 calls are distinct from the CSB-only
 * D3L2/D3R2 path: D2 uses the derived scaled-bitmap route and only
 * front D2 ornaments can return an alcove cell order, while D1 side
 * ornaments use the native bitmap path without updating the D1-front
 * interaction state.  D1C front alone owns the facing/clickbox/portrait
 * side effects. */
#define CSB_D1D2_WALL_ORNAMENT_PATH(square_, view_wall_, slot_, returns_, d2_, d1_, native_inc_, derived_inc_, scale_, flip_, state_, clickbox_, portrait_, fn_, source_) \
    { \
        (int)(square_), \
        (view_wall_), \
        (slot_), \
        (returns_), \
        (d2_), \
        (d1_), \
        (native_inc_), \
        (derived_inc_), \
        (scale_), \
        (flip_), \
        (state_), \
        (clickbox_), \
        (portrait_), \
        CSB_V1_WALL_ORNAMENT_ZONE_BASE, \
        CSB_V1_WALL_ORNAMENT_COORD_STRIDE, \
        (fn_), \
        (source_) \
    }

static const CSB_V1_ViewportWallOrnamentD1D2PathSpec s_wall_ornament_d1d2_paths[] = {
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D2L,
        CSB_V1_VIEW_WALL_D2L_RIGHT,
        CSB_V1_ORNAMENT_SLOT_RIGHT,
        0, 1, 0, 0, CSB_V1_WALL_ORNAMENT_D2_SIDE_DERIVED_INCREMENT,
        CSB_V1_WALL_ORNAMENT_SCALE_D2, CSB_V1_FLIP_NONE, 0, 0, 0,
        "F0119_DrawSquareD2L",
        "DUNVIEW.C:6968 side F0107(M551, M580_VIEW_WALL_D2L_RIGHT) ignores return; F0107:3571-3589 ordinal/index/zone/alcove; 3817-3860 D2 C21 scaled derived-bitmap path with G0190 increment 2 and G0199 D2 palette; 3921-3923 F0791 C10. DEFS.H:2703,4222; DUNVIEW.C:805-819 G0190; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D2L,
        CSB_V1_VIEW_WALL_D2L_FRONT,
        CSB_V1_ORNAMENT_SLOT_FRONT,
        1, 1, 0, 1, CSB_V1_WALL_ORNAMENT_D2_FRONT_DERIVED_INCREMENT,
        CSB_V1_WALL_ORNAMENT_SCALE_D2, CSB_V1_FLIP_NONE, 0, 0, 0,
        "F0119_DrawSquareD2L",
        "DUNVIEW.C:6969 front F0107(M552, M582_VIEW_WALL_D2L_FRONT) controls C0x0000 alcove order; F0107:3571-3589 ordinal/index/zone/alcove; 3800-3804 D2L front X adjustment; 3817-3860 D2 C21 scaled derived-bitmap path with G0190 increment 3 and native bitmap +1; 3921-3923 F0791 C10. DEFS.H:2705,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D2R,
        CSB_V1_VIEW_WALL_D2R_LEFT,
        CSB_V1_ORNAMENT_SLOT_LEFT,
        0, 1, 0, 0, CSB_V1_WALL_ORNAMENT_D2_SIDE_DERIVED_INCREMENT,
        CSB_V1_WALL_ORNAMENT_SCALE_D2, CSB_V1_FLIP_HORIZONTAL, 0, 0, 0,
        "F0120_DrawSquareD2R",
        "DUNVIEW.C:7119 side F0107(M553, M581_VIEW_WALL_D2R_LEFT) ignores return; F0107:3571-3589 ordinal/index/zone/alcove; 3817-3819 sets horizontal flip; 3817-3860 D2 C21 scaled derived-bitmap path with G0190 increment 2 and G0199 D2 palette; 3921-3923 F0791 C10. DEFS.H:2704,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D2R,
        CSB_V1_VIEW_WALL_D2R_FRONT,
        CSB_V1_ORNAMENT_SLOT_FRONT,
        1, 1, 0, 1, CSB_V1_WALL_ORNAMENT_D2_FRONT_DERIVED_INCREMENT,
        CSB_V1_WALL_ORNAMENT_SCALE_D2, CSB_V1_FLIP_NONE, 0, 0, 0,
        "F0120_DrawSquareD2R",
        "DUNVIEW.C:7120 front F0107(M552, M584_VIEW_WALL_D2R_FRONT) controls C0x0000 alcove order; F0107:3571-3589 ordinal/index/zone/alcove; 3782-3784 D2R front offset; 3817-3860 D2 C21 scaled derived-bitmap path with G0190 increment 3 and native bitmap +1; 3921-3923 F0791 C10. DEFS.H:2707,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D2C,
        CSB_V1_VIEW_WALL_D2C_FRONT,
        CSB_V1_ORNAMENT_SLOT_FRONT,
        1, 1, 0, 1, CSB_V1_WALL_ORNAMENT_D2_FRONT_DERIVED_INCREMENT,
        CSB_V1_WALL_ORNAMENT_SCALE_D2, CSB_V1_FLIP_NONE, 0, 0, 0,
        "F0121_DrawSquareD2C",
        "DUNVIEW.C:7308 front F0107(M552, M583_VIEW_WALL_D2C_FRONT) controls C0x0000 alcove order; F0107:3571-3589 ordinal/index/zone/alcove; 3817-3860 D2 C21 scaled derived-bitmap path with G0190 increment 3 and native bitmap +1; 3921-3923 F0791 C10. DEFS.H:2706,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D1L,
        CSB_V1_VIEW_WALL_D1L_RIGHT,
        CSB_V1_ORNAMENT_SLOT_RIGHT,
        0, 0, 1, 0, CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE,
        0, CSB_V1_FLIP_NONE, 0, 0, 0,
        "F0122_DrawSquareD1L",
        "DUNVIEW.C:7459 side F0107(M551, M585_VIEW_WALL_D1L_RIGHT) ignores return; F0107:3571-3589 ordinal/index/zone/alcove; 3608 enters D1 branch, 3755-3760 uses native/CM1_DERIVED_BITMAP_NONE path, 3921-3923 F0791 C10; no 3726-3744 facing state because it is not M587. DEFS.H:2708,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D1R,
        CSB_V1_VIEW_WALL_D1R_LEFT,
        CSB_V1_ORNAMENT_SLOT_LEFT,
        0, 0, 1, 0, CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE,
        0, CSB_V1_FLIP_HORIZONTAL, 0, 0, 0,
        "F0123_DrawSquareD1R",
        "DUNVIEW.C:7627 side F0107(M553, M586_VIEW_WALL_D1R_LEFT) ignores return; F0107:3571-3589 ordinal/index/zone/alcove; 3608 enters D1 branch, 3751-3752 sets horizontal flip, 3755-3760 uses native/CM1_DERIVED_BITMAP_NONE path, 3921-3923 F0791 C10; no 3726-3744 facing state because it is not M587. DEFS.H:2709,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D1C,
        CSB_V1_VIEW_WALL_D1C_FRONT,
        CSB_V1_ORNAMENT_SLOT_FRONT,
        1, 0, 1, 1, CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE,
        0, CSB_V1_FLIP_NONE, 1, 1, 1,
        "F0124_DrawSquareD1C",
        "DUNVIEW.C:7842 front F0107(M552, M587_VIEW_WALL_D1C_FRONT) controls C0x0000 alcove F0115; F0107:3571-3589 ordinal/index/zone/alcove; 3608-3744 D1-front branch updates facing alcove/Vi altar/fountain, 3722 native bitmap +1, 3923-3928 copies clickbox and draws champion portrait overlay when present. DEFS.H:2710,4222; COORD.C:921-1025.")
};

#undef CSB_D1D2_WALL_ORNAMENT_PATH

/* ReDMCSB: DUNVIEW.C F0676/F0677 lines 6270-6286 and 6337-6353.
 * The CSB-only D3L2/D3R2 routes call F0108 before the rear F0115 pass,
 * call F0111 for door-front panels, then finish with the front F0115 pass.
 * Pit cases fall through to the same F0108 call, preserving BUG0_64. */
static const CSB_V1_ViewportFloorOrnamentRouteSpec s_floor_ornament_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        1,
        1,
        1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS2,
        CSB_V1_ZONE_DOOR_D3L2,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108 door-front floor ornament; 6271 F0115 pass1; 6272 F0111 C3700_ZONE_DOOR_D3L2; 6282-6286 pit/corridor F0108 then F0115"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        1,
        1,
        1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS2,
        CSB_V1_ZONE_DOOR_D3R2,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108 door-front floor ornament; 6338 F0115 pass1; 6339 F0111 C3710_ZONE_DOOR_D3R2; 6349-6353 pit/corridor F0108 then F0115"
    },
};

/* ReDMCSB: DUNVIEW.C F0108 lines 3940-4008 and F0676/F0677 lines
 * 6270/6284 and 6337/6351.  F0108 draws nothing for ordinal 0, converts
 * ordinal to map floor-ornament index by pre-decrement, adds the per-view
 * G0191 native bitmap increment, and blits to
 * C1500_ZONE_FLOOR_ORNAMENT + CoordinateSet * 11 + ViewFloorIndex using
 * F0791 with C10 transparency.  The CSB/I34 coordinate-set table is all 0,
 * and the CSB/I34 path flips D3R2 horizontally. */
static const CSB_V1_ViewportFloorOrnamentBlitSpec s_floor_ornament_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        1,
        CSB_V1_FLOOR_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_FLOOR_ORNAMENT_D3L2_BITMAP_INCREMENT,
        CSB_V1_FLOOR_ORNAMENT_COORD_SET,
        CSB_V1_FLOOR_ORNAMENT_ZONE_BASE,
        CSB_V1_FLOOR_ORNAMENT_COORD_STRIDE,
        CSB_V1_FLIP_NONE,
        CSB_V1_FLOOR_ORNAMENT_TRANSPARENT_COLOR,
        "M552_FRONT_WALL_ORNAMENT_ORDINAL",
        "M558_FLOOR_ORNAMENT_ORDINAL",
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108 uses M552 for door-front D3L2; 6284 uses M558 for pit/corridor. F0108:3959 skips ordinal 0; 3965 ordinal-- plus G0191[0]=0; 3998 F0791 zone C1500 + CoordinateSet*11 + C00_VIEW_FLOOR_D3L2; G0195:1008-1017 gives CSB/I34 CoordinateSet 0; flip 0; C10. DEFS.H:2750, 3522-3523, 4223; COORD.C:903-913."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        1,
        CSB_V1_FLOOR_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_FLOOR_ORNAMENT_D3R2_BITMAP_INCREMENT,
        CSB_V1_FLOOR_ORNAMENT_COORD_SET,
        CSB_V1_FLOOR_ORNAMENT_ZONE_BASE,
        CSB_V1_FLOOR_ORNAMENT_COORD_STRIDE,
        CSB_V1_FLIP_HORIZONTAL,
        CSB_V1_FLOOR_ORNAMENT_TRANSPARENT_COLOR,
        "M558_FLOOR_ORNAMENT_ORDINAL",
        "M558_FLOOR_ORNAMENT_ORDINAL",
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108 uses M558 for door-front D3R2; 6351 uses M558 for pit/corridor. F0108:3959 skips ordinal 0; 3965 ordinal-- plus G0191[1]=0; G0195:1008-1017 gives CSB/I34 CoordinateSet 0; 3980-3983 sets MASK0x0001_FLIP_HORIZONTAL for C01_VIEW_FLOOR_D3R2; 3998 F0791 zone C1500 + CoordinateSet*11 + C01_VIEW_FLOOR_D3R2; C10. DEFS.H:2751, 3522-3523, 4223; COORD.C:903-913."
    },
};

/* ReDMCSB: DUNVIEW.C F0676 lines 6270-6286 and F0677 lines 6337-6353.
 * CSB's extra D3L2/D3R2 squares route the same F0115 stack as DM1: objects
 * first, a creature after object cells, projectiles after creatures, and
 * explosions after all cells.  Projectiles restart from the first thing for
 * the current cell and use C2900_ZONE_ + G2028[ViewSquare] * 4 + ViewCell.
 * Door-front squares split F0115 around F0111: rear cells before the door
 * panel/frame, front cells after it. */
static const CSB_V1_ViewportThingPassOrderSpec s_thing_pass_order_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        0,
        1,
        2,
        3,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS2,
        CSB_V1_CELL_ORDER_D3L2_CORRIDOR,
        CSB_V1_CELL_ORDER_D3L2_SIDE,
        0,
        1,
        2,
        CSB_V1_PROJECTILE_ROW_D3L2,
        CSB_V1_PROJECTILE_ZONE_BASE,
        CSB_V1_PROJECTILE_ZONE_STRIDE,
        1,
        1,
        1,
        3,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108; 6271 F0115 door rear cells 0x0218; 6272 F0111 door; 6284 F0108 pit/corridor BUG0_64; 6286 F0115 final/corridor/side; F0115:4567-4581 objects/creatures/projectiles, 5668 G2028 row, 5672 D3 front-cell skip, 5679 restart projectile list, 5681 cell match, 5683 C2900 zone, 5881-5883 blit, 5915-5933 explosions"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        0,
        1,
        2,
        3,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS2,
        CSB_V1_CELL_ORDER_D3R2_CORRIDOR,
        CSB_V1_CELL_ORDER_D3R2_SIDE,
        0,
        1,
        2,
        CSB_V1_PROJECTILE_ROW_D3R2,
        CSB_V1_PROJECTILE_ZONE_BASE,
        CSB_V1_PROJECTILE_ZONE_STRIDE,
        1,
        1,
        1,
        3,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108; 6338 F0115 door rear cells 0x0128; 6339 F0111 door; 6351 F0108 pit/corridor BUG0_64; 6353 F0115 final/corridor/side; F0115:4567-4581 objects/creatures/projectiles, 5668 G2028 row, 5672 D3 front-cell skip, 5679 restart projectile list, 5681 cell match, 5683 C2900 zone, 5881-5883 blit, 5915-5933 explosions"
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 4806-4811 and 4923.
 * For the PC34/I34E path, D3L2/D3R2 map through G2027/G2028 to depth 3
 * rows 3/4.  The object predicate accepts only weapon..junk things on the
 * processed cell; depth 3 squares suppress front-left/front-right cells by
 * requiring AL0126_i_ViewCell > C01_VIEW_CELL_FRONT_RIGHT. */
static const CSB_V1_ViewportObjectVisibilitySpec s_object_visibility_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3L2,
        1,
        1,
        1,
        0,
        CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL,
        CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL,
        "F0676_DrawD3L2",
        "DUNVIEW.C:371-373 G2026/G2027/G2028; 4806-4811 loads lane/depth/object row; 4923 F0115 weapon..junk, L2476>=0, cell match, depth3 front-cell suppression"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3R2,
        1,
        1,
        1,
        0,
        CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL,
        CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL,
        "F0677_DrawD3R2",
        "DUNVIEW.C:371-373 G2026/G2027/G2028; 4806-4811 loads lane/depth/object row; 4923 F0115 weapon..junk, L2476>=0, cell match, depth3 front-cell suppression"
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 4923-5110, DEFS.H lines 3517 and
 * 4228, and COORD.C lines 1129-1193.  For the PC34/I34 route,
 * visible weapon..junk objects are drawn through
 * C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES plus the G2028
 * visibility row and view cell.  The shift mask makes COORD.C apply the
 * per-pile G0223/G0217 shift before F0791 blits with C10 transparency. */
static const CSB_V1_ViewportObjectBlitSpec s_object_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3L2,
        CSB_V1_OBJECT_ZONE_BASE,
        CSB_V1_OBJECT_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        CSB_V1_OBJECT_SHIFT_SET_D3_FRONT,
        1,
        10,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:4923 F0115 weapon..junk visible-cell predicate; 5030-5039 D3 object scale/shift set; 5071-5082 C2500_ZONE_ | MASK0x8000 + G2028 row*4 + ViewCell plus pile shift; 5109 F0791 C10 blit. DEFS.H:3517,4228; COORD.C:1129-1193 layout range 2500..2560."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3R2,
        CSB_V1_OBJECT_ZONE_BASE,
        CSB_V1_OBJECT_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        CSB_V1_OBJECT_SHIFT_SET_D3_FRONT,
        1,
        10,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:4923 F0115 weapon..junk visible-cell predicate; 5030-5039 D3 object scale/shift set; 5071-5082 C2500_ZONE_ | MASK0x8000 + G2028 row*4 + ViewCell plus pile shift; 5109 F0791 C10 blit. DEFS.H:3517,4228; COORD.C:1129-1193 layout range 2500..2560."
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 5668-5683 and 5710-5885,
 * DEFS.H line 4230, and COORD.C lines 1194-1239.  The MEDIA709
 * PC34/I34 path restarts from the first thing for the active cell, accepts
 * only projectile things whose stored cell matches, maps D3L2/D3R2 through
 * G2028 rows 3/4, rejects D3 front cells, and sends the scaled projectile
 * bitmap through F0791 with the computed C2900 zone, dynamic flip flags,
 * CM1_DERIVED_BITMAP_NONE for uncached scaled paths, and C10 transparency. */
static const CSB_V1_ViewportProjectileBlitSpec s_projectile_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_PROJECTILE_ROW_D3L2,
        CSB_V1_PROJECTILE_ZONE_BASE,
        CSB_V1_PROJECTILE_ZONE_STRIDE,
        1,
        1,
        1,
        1,
        0,
        CSB_V1_PROJECTILE_DERIVED_BITMAP_NONE,
        CSB_V1_PROJECTILE_TRANSPARENT_COLOR,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:5668-5683 F0115 maps C14_VIEW_SQUARE_D3L2 through G2028 row 3, suppresses D3 front cells, restarts the thing list, requires C14_THING_TYPE_PROJECTILE and matching cell, then computes C2900_ZONE_ + row*4 + ViewCell. 5710-5722 scales by depth/cell/kinetic energy; 5859 CM1_DERIVED_BITMAP_NONE for uncached scaled path; 5881-5882 F0791 uses L2474 zone, dynamic flip flags, and C10 transparency. DEFS.H:4230; COORD.C:1194-1239 projectile zone records."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_PROJECTILE_ROW_D3R2,
        CSB_V1_PROJECTILE_ZONE_BASE,
        CSB_V1_PROJECTILE_ZONE_STRIDE,
        1,
        1,
        1,
        1,
        0,
        CSB_V1_PROJECTILE_DERIVED_BITMAP_NONE,
        CSB_V1_PROJECTILE_TRANSPARENT_COLOR,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:5668-5683 F0115 maps C15_VIEW_SQUARE_D3R2 through G2028 row 4, suppresses D3 front cells, restarts the thing list, requires C14_THING_TYPE_PROJECTILE and matching cell, then computes C2900_ZONE_ + row*4 + ViewCell. 5710-5722 scales by depth/cell/kinetic energy; 5859 CM1_DERIVED_BITMAP_NONE for uncached scaled path; 5881-5882 F0791 uses L2474 zone, dynamic flip flags, and C10 transparency. DEFS.H:4230; COORD.C:1194-1239 projectile zone records."
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 4840-4842, 5201-5214, and 5615-5627.
 * The MEDIA720 PC34/I34E path records one group thing while scanning each
 * cell, rejects view squares where G2033 maps to -1, then draws creatures
 * through C3200_ZONE_ + CreatureAspectCoordinateSet * 65 + G2033 row * 5
 * + ViewCell with MASK0x8000_SHIFT_OBJECTS_AND_CREATURES applied so COORD.C
 * adds the object/creature shifts before the F0791 blit. */
static const CSB_V1_ViewportCreatureVisibilitySpec s_creature_visibility_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_CREATURE_ROW_D3L2,
        1,
        1,
        CSB_V1_CREATURE_ZONE_BASE,
        CSB_V1_CREATURE_COORDINATE_SET_STRIDE,
        CSB_V1_CREATURE_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        "F0676_DrawD3L2",
        "DUNVIEW.C:375 G2033 row; 4840-4842 records C04_THING_TYPE_GROUP; 5201-5214 F0115 creature gate rejects G2033<0; 5615-5627 C3200_ZONE_ | MASK0x8000 + CoordinateSet*65 + G2033*5 + ViewCell then F0791; COORD.C:1248-1251 C3200 layout range 3200..3364; 2074-2075 clears shift mask"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_CREATURE_ROW_D3R2,
        1,
        1,
        CSB_V1_CREATURE_ZONE_BASE,
        CSB_V1_CREATURE_COORDINATE_SET_STRIDE,
        CSB_V1_CREATURE_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        "F0677_DrawD3R2",
        "DUNVIEW.C:375 G2033 row; 4840-4842 records C04_THING_TYPE_GROUP; 5201-5214 F0115 creature gate rejects G2033<0; 5615-5627 C3200_ZONE_ | MASK0x8000 + CoordinateSet*65 + G2033*5 + ViewCell then F0791; COORD.C:1248-1251 C3200 layout range 3200..3364; 2074-2075 clears shift mask"
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 5915-6219, DEFS.H lines 4232-4235
 * and 4042-4043, COORD.C lines 1058-1123 and 1194-1238.  Explosions
 * restart from the first thing after all requested cells are processed.
 * MEDIA720 maps D3L2/D3R2 through G2034/G2035, uses C3000/C3007 for
 * rebirth, C3014 for centered explosions, C3031 + row*2 + cell for side
 * explosions, and defers fluxcage to F0113 as a field overlay. */
static const CSB_V1_ViewportExplosionBlitSpec s_explosion_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_EXPLOSION_ROW_D3L2,
        CSB_V1_FIELD_ASPECT_D3L2,
        1,
        1,
        1,
        CSB_V1_FIELD_ZONE_D3L2,
        CSB_V1_EXPLOSION_REBIRTH_STEP1_ZONE_BASE,
        CSB_V1_EXPLOSION_REBIRTH_STEP2_ZONE_BASE,
        CSB_V1_EXPLOSION_CENTERED_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_CELL_STRIDE,
        10,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:5915-5933 restarts explosion pass; 5920-5924 G2034/G2035 visibility rows; 5948 rebirth requires visible row and matching cell; 5998-5999 C3000 + row; 6094-6096 C3007 + row; 6106-6107 C3014 + row; 6121-6122 C3031 + row*2 + ViewCell; 6192-6193 F0791 C10 blit; 6202-6219 fluxcage field C702 + field aspect. DEFS.H:4042,4232-4235; COORD.C:1058-1123,1194-1238."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_EXPLOSION_ROW_D3R2,
        CSB_V1_FIELD_ASPECT_D3R2,
        1,
        1,
        1,
        CSB_V1_FIELD_ZONE_D3R2,
        CSB_V1_EXPLOSION_REBIRTH_STEP1_ZONE_BASE,
        CSB_V1_EXPLOSION_REBIRTH_STEP2_ZONE_BASE,
        CSB_V1_EXPLOSION_CENTERED_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_CELL_STRIDE,
        10,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:5915-5933 restarts explosion pass; 5920-5924 G2034/G2035 visibility rows; 5948 rebirth requires visible row and matching cell; 5998-5999 C3000 + row; 6094-6096 C3007 + row; 6106-6107 C3014 + row; 6121-6122 C3031 + row*2 + ViewCell; 6192-6193 F0791 C10 blit; 6202-6219 fluxcage field C702 + field aspect. DEFS.H:4043,4232-4235; COORD.C:1058-1123,1194-1238."
    },
};

/* ReDMCSB: DUNVIEW.C F0676/F0677 lines 6288-6290 and 6355-6357,
 * F0678/F0679 lines 6863-6865 and 6894-6896, G2035 at line 377, and
 * DEFS.H lines 4042-4048.  CSB/I34 D3L2/D3R2 teleporters finish their
 * F0108/F0115 path before F0113; D2L2/D2R2 are near-wall teleporter-only
 * routes with no F0108, F0115, or thing pass. */
static const CSB_V1_ViewportTeleporterFieldSpec s_teleporter_fields[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        1,
        1,
        CSB_V1_FIELD_ASPECT_D3L2,
        CSB_V1_FIELD_ZONE_D3L2,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6288-6290 teleporter draws F0113(G0188[G2035[C14_VIEW_SQUARE_D3L2]], C702_ZONE_WALL_D3L2) after 6284 F0108 and 6286 F0115. DUNVIEW.C:377 G2035 maps C14 to field aspect 0; 4382-4409 F0113 clips by zone. DEFS.H:4042 C702_ZONE_WALL_D3L2."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        1,
        1,
        CSB_V1_FIELD_ASPECT_D3R2,
        CSB_V1_FIELD_ZONE_D3R2,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6355-6357 teleporter draws F0113(G0188[G2035[C15_VIEW_SQUARE_D3R2]], C703_ZONE_WALL_D3R2) after 6351 F0108 and 6353 F0115. DUNVIEW.C:377 G2035 maps C15 to field aspect 1; 4382-4409 F0113 clips by zone. DEFS.H:4043 C703_ZONE_WALL_D3R2."
    },
    {
        (int)DM1_VIEW_SQUARE_D2L2,
        9,
        1,
        0,
        CSB_V1_FIELD_ASPECT_D2L2,
        CSB_V1_FIELD_ZONE_D2L2,
        1,
        "F0678_DrawD2L2",
        "DUNVIEW.C:6863-6865 teleporter draws F0113(G0188[G2035[C09_VIEW_SQUARE_D2L2]], C707_ZONE_WALL_D2L2) without F0108 and without F0115. DUNVIEW.C:377 G2035 maps C09 to field aspect 5; 4382-4409 F0113 clips by zone. DEFS.H:2605 C09_VIEW_SQUARE_D2L2; DEFS.H:4047 C707_ZONE_WALL_D2L2."
    },
    {
        (int)DM1_VIEW_SQUARE_D2R2,
        10,
        1,
        0,
        CSB_V1_FIELD_ASPECT_D2R2,
        CSB_V1_FIELD_ZONE_D2R2,
        1,
        "F0679_DrawD2R2",
        "DUNVIEW.C:6894-6896 teleporter draws F0113(G0188[G2035[C10_VIEW_SQUARE_D2R2]], C708_ZONE_WALL_D2R2) without F0108 and without F0115. DUNVIEW.C:377 G2035 maps C10 to field aspect 6; 4382-4409 F0113 clips by zone. DEFS.H:2606 C10_VIEW_SQUARE_D2R2; DEFS.H:4048 C708_ZONE_WALL_D2R2."
    },
};

/* ReDMCSB: DUNVIEW.C F0111 lines 4218-4337, F0676/F0677 lines
 * 6271-6273 and 6338-6340, DEFS.H lines 4250-4251, COORD.C lines
 * 1545-1565 and 781-807.  The CSB-only far door panels reuse the
 * D3 native 48x41 door bitmap but clip it through COORD.C record 126's
 * 48x40 viewport sub-zone.  F0111 skips state 0, shifts zone ids by
 * door state for partially-open doors, applies the destroyed-door mask for
 * state 5, and blits with C10 transparency. */
static const CSB_V1_ViewportDoorPanelBlitSpec s_door_panel_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_ZONE_DOOR_D3L2,
        CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED,
        CSB_V1_DOOR_PANEL_PARENT_D3L2,
        CSB_V1_DOOR_PANEL_D3L2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_PANEL_CLIP_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_NATIVE_H_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_CLIPPED_H_D3,
        CSB_V1_DOOR_PANEL_D3L2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_ORNAMENT_D3LCR,
        1,
        1,
        6,
        3,
        CSB_V1_DOOR_STATE_DESTROYED,
        CSB_V1_DOOR_ORNAMENT_DESTROYED_MASK,
        1,
        CSB_V1_DOOR_TRANSPARENT_COLOR,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6271 F0115 rear pass; 6272 F0111(... C3700_ZONE_DOOR_D3L2); 6273-6286 F0115 front pass. F0111:4248 skips C0 open, 4301-4302 applies C15 destroyed mask to P0128 view ornament index, 4298-4321 shifts zone by state/horizontal halves, 4334 F0791 blits with C10. DEFS.H:1044,2466,4250; COORD.C:1548-1565 records 120/126/129 and 788-797 zone 3700..3709."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_ZONE_DOOR_D3R2,
        CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED,
        CSB_V1_DOOR_PANEL_PARENT_D3R2,
        CSB_V1_DOOR_PANEL_D3R2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_PANEL_CLIP_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_NATIVE_H_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_CLIPPED_H_D3,
        CSB_V1_DOOR_PANEL_D3R2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_ORNAMENT_D3LCR,
        1,
        1,
        6,
        3,
        CSB_V1_DOOR_STATE_DESTROYED,
        CSB_V1_DOOR_ORNAMENT_DESTROYED_MASK,
        1,
        CSB_V1_DOOR_TRANSPARENT_COLOR,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6338 F0115 rear pass; 6339 F0111(... C3710_ZONE_DOOR_D3R2); 6340-6353 F0115 front pass. F0111:4248 skips C0 open, 4301-4302 applies C15 destroyed mask to P0128 view ornament index, 4298-4321 shifts zone by state/horizontal halves, 4334 F0791 blits with C10. DEFS.H:1044,2466,4251; COORD.C:1548-1565 records 120/126/130 and 798-807 zone 3710..3719."
    },
};

void csb_v1_viewport_init(CSB_V1_ViewportConfig *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->ambient_color = 0xFF000000; /* black ambient */
    cfg->viewport_pixels = NULL;
    cfg->viewport_stride = 320;
    cfg->dungeon_grid = NULL;
    cfg->dungeon_width = 0;
    cfg->dungeon_height = 0;
    cfg->custom_background_loaded_level = -1;
    cfg->custom_background_room_num = -1;
}

void csb_v1_viewport_apply_runtime_drawer_binding(
    CSB_V1_ViewportConfig *cfg,
    const CSB_V1_ViewportRuntimeDrawerBinding *binding)
{
    if (!cfg) {
        return;
    }
    if (!binding) {
        cfg->object_sprite_drawer = NULL;
        cfg->object_sprite_user = NULL;
        cfg->object_icon_drawer = NULL;
        cfg->object_icon_user = NULL;
        cfg->group_sprite_drawer = NULL;
        cfg->group_sprite_user = NULL;
        cfg->projectile_sprite_drawer = NULL;
        cfg->projectile_sprite_user = NULL;
        cfg->explosion_sprite_drawer = NULL;
        cfg->explosion_sprite_user = NULL;
        cfg->runtime_overlay_source_required = 0;
        cfg->runtime_overlay_source_admitted = 0;
        cfg->runtime_overlay_source_hash = 0u;
        cfg->real_graphics_session = 0;
        cfg->first_frame_material_proof = NULL;
        cfg->first_frame_material_bytes = NULL;
        cfg->first_frame_material_source_path = NULL;
        cfg->first_frame_material_source_md5 = NULL;
        cfg->first_frame_material_consumed_count = 0;
        cfg->first_frame_material_blocked_count = 0;
        cfg->first_frame_material_hash = 0u;
        cfg->first_frame_draw_plan_consumed_count = 0;
        cfg->first_frame_draw_plan_blocked_count = 0;
        cfg->first_frame_draw_plan_command_count = 0;
        cfg->first_frame_draw_plan_hash = 0u;
        cfg->first_frame_draw_plan_palette_hash = 0u;
        cfg->first_frame_material_raster_consumed_count = 0;
        cfg->first_frame_material_raster_blocked_count = 0;
        cfg->first_frame_material_raster_hash = 0u;
        cfg->graphic_provider_callback = NULL;
        cfg->graphic_provider_user_data = NULL;
        return;
    }
    cfg->object_sprite_drawer = binding->object_sprite_drawer;
    cfg->object_sprite_user = binding->object_sprite_user;
    cfg->object_icon_drawer = binding->object_icon_drawer;
    cfg->object_icon_user = binding->object_icon_user;
    cfg->group_sprite_drawer = binding->group_sprite_drawer;
    cfg->group_sprite_user = binding->group_sprite_user;
    cfg->projectile_sprite_drawer = binding->projectile_sprite_drawer;
    cfg->projectile_sprite_user = binding->projectile_sprite_user;
    cfg->explosion_sprite_drawer = binding->explosion_sprite_drawer;
    cfg->explosion_sprite_user = binding->explosion_sprite_user;
    cfg->runtime_overlay_source_required =
        binding->runtime_overlay_source_required ? 1 : 0;
    cfg->runtime_overlay_source_admitted =
        binding->runtime_overlay_source_admitted ? 1 : 0;
    cfg->runtime_overlay_source_hash = binding->runtime_overlay_source_hash;
    cfg->real_graphics_session = binding->real_graphics_session ? 1 : 0;
    cfg->first_frame_material_proof = binding->first_frame_material_proof;
    cfg->first_frame_material_bytes = binding->first_frame_material_bytes;
    cfg->first_frame_material_source_path = binding->first_frame_material_source_path;
    cfg->first_frame_material_source_md5 = binding->first_frame_material_source_md5;
    cfg->first_frame_material_consumed_count = 0;
    cfg->first_frame_material_blocked_count = 0;
    cfg->first_frame_material_hash = 0u;
    cfg->first_frame_draw_plan_consumed_count = 0;
    cfg->first_frame_draw_plan_blocked_count = 0;
    cfg->first_frame_draw_plan_command_count = 0;
    cfg->first_frame_draw_plan_hash = 0u;
    cfg->first_frame_draw_plan_palette_hash = 0u;
    cfg->first_frame_material_raster_consumed_count = 0;
    cfg->first_frame_material_raster_blocked_count = 0;
    cfg->first_frame_material_raster_hash = 0u;
    cfg->graphic_provider_callback = binding->graphic_provider_callback;
    cfg->graphic_provider_user_data = binding->graphic_provider_user_data;
}

void csb_v1_viewport_runtime_draw_counts_reset(
    CSB_V1_ViewportRuntimeDrawCounts *counts)
{
    if (counts) {
        memset(counts, 0, sizeof(*counts));
    }
}

void csb_v1_viewport_runtime_draw_counts_from_config(
    const CSB_V1_ViewportConfig *cfg,
    CSB_V1_ViewportRuntimeDrawCounts *counts)
{
    if (!counts) {
        return;
    }
    csb_v1_viewport_runtime_draw_counts_reset(counts);
    if (!cfg) {
        return;
    }
    counts->object_sprite_drawn_count =
        cfg->runtime_object_sprite_drawn_count;
    counts->object_icon_drawn_count =
        cfg->runtime_object_icon_drawn_count;
    counts->object_marker_drawn_count =
        cfg->runtime_object_marker_drawn_count;
    counts->group_sprite_drawn_count =
        cfg->runtime_group_sprite_drawn_count;
    counts->group_marker_drawn_count =
        cfg->runtime_group_marker_drawn_count;
    counts->projectile_sprite_drawn_count =
        cfg->runtime_projectile_sprite_drawn_count;
    counts->projectile_material_resolved_count =
        cfg->runtime_projectile_material_resolved_count;
    counts->projectile_material_icon_drawn_count =
        cfg->runtime_projectile_material_icon_drawn_count;
    counts->projectile_marker_drawn_count =
        cfg->runtime_projectile_marker_drawn_count;
    counts->explosion_sprite_drawn_count =
        cfg->runtime_explosion_sprite_drawn_count;
    counts->explosion_marker_drawn_count =
        cfg->runtime_explosion_marker_drawn_count;
    counts->first_frame_material_consumed_count =
        cfg->first_frame_material_consumed_count;
    counts->first_frame_material_blocked_count =
        cfg->first_frame_material_blocked_count;
    counts->first_frame_material_hash =
        cfg->first_frame_material_hash;
    counts->first_frame_draw_plan_consumed_count =
        cfg->first_frame_draw_plan_consumed_count;
    counts->first_frame_draw_plan_blocked_count =
        cfg->first_frame_draw_plan_blocked_count;
    counts->first_frame_draw_plan_command_count =
        cfg->first_frame_draw_plan_command_count;
    counts->first_frame_draw_plan_hash =
        cfg->first_frame_draw_plan_hash;
    counts->first_frame_draw_plan_palette_hash =
        cfg->first_frame_draw_plan_palette_hash;
    counts->first_frame_material_raster_consumed_count =
        cfg->first_frame_material_raster_consumed_count;
    counts->first_frame_material_raster_blocked_count =
        cfg->first_frame_material_raster_blocked_count;
    counts->first_frame_material_raster_hash =
        cfg->first_frame_material_raster_hash;
}

void csb_v1_viewport_set_first_frame_material_proof_pc34(
    CSB_V1_ViewportConfig *cfg,
    const CSB_V1_ViewportFirstFrameMaterialProof *proof)
{
    if (!cfg) {
        return;
    }
    cfg->first_frame_material_proof = proof;
    cfg->first_frame_material_consumed_count = 0;
    cfg->first_frame_material_blocked_count = 0;
    cfg->first_frame_material_hash = 0u;
    cfg->first_frame_draw_plan_consumed_count = 0;
    cfg->first_frame_draw_plan_blocked_count = 0;
    cfg->first_frame_draw_plan_command_count = 0;
    cfg->first_frame_draw_plan_hash = 0u;
    cfg->first_frame_draw_plan_palette_hash = 0u;
}

void csb_v1_viewport_set_wall_set(CSB_V1_ViewportConfig *cfg, int set) {
    if (cfg) cfg->wall_set_index = set;
}

void csb_v1_viewport_set_custom_background(CSB_V1_ViewportConfig *cfg, int bg_id) {
    if (cfg) cfg->custom_background = bg_id;
}

size_t csb_v1_viewport_custom_background_slot_spec_count(void)
{
    return sizeof(s_custom_background_slots) / sizeof(s_custom_background_slots[0]);
}

const CSB_V1_ViewportCustomBackgroundSlotSpec *
csb_v1_viewport_get_custom_background_slot_spec(size_t index)
{
    if (index >= csb_v1_viewport_custom_background_slot_spec_count()) return NULL;
    return &s_custom_background_slots[index];
}

const CSB_V1_ViewportCustomBackgroundSlotSpec *
csb_v1_viewport_get_custom_background_slot_spec_for_room(int room_num)
{
    for (size_t i = 0; i < csb_v1_viewport_custom_background_slot_spec_count(); ++i) {
        if (s_custom_background_slots[i].room_num == room_num) {
            return &s_custom_background_slots[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_custom_background_bitmap_application_spec_count(void)
{
    return sizeof(s_custom_background_application_specs) /
           sizeof(s_custom_background_application_specs[0]);
}

const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *
csb_v1_viewport_get_custom_background_bitmap_application_spec(size_t index)
{
    if (index >= csb_v1_viewport_custom_background_bitmap_application_spec_count()) return NULL;
    return &s_custom_background_application_specs[index];
}

const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *
csb_v1_viewport_get_custom_background_bitmap_application_spec_for_room(int room_num)
{
    for (size_t i = 0;
         i < csb_v1_viewport_custom_background_bitmap_application_spec_count();
         ++i) {
        const CSB_V1_ViewportCustomBackgroundSlotSpec *slot =
            s_custom_background_application_specs[i].room_slot;
        if (slot && slot->room_num == room_num) {
            return &s_custom_background_application_specs[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_custom_background_translate_cell(
    const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *spec,
    int party_x,
    int party_y,
    int facing,
    int *out_x,
    int *out_y)
{
    static const int dx_forward[4] = { 0, 1, 0, -1 };
    static const int dy_forward[4] = { -1, 0, 1, 0 };
    static const int dx_side[4] = { 1, 0, -1, 0 };
    static const int dy_side[4] = { 0, 1, 0, -1 };
    const CSB_V1_ViewportCustomBackgroundSlotSpec *slot;

    if (!spec || !spec->room_slot || facing < 0 || facing >= 4 || !out_x || !out_y) {
        return 0;
    }

    slot = spec->room_slot;
    *out_x = party_x + dx_side[facing] * slot->relative_side +
             dx_forward[facing] * slot->relative_forward;
    *out_y = party_y + dy_side[facing] * slot->relative_side +
             dy_forward[facing] * slot->relative_forward;
    return 1;
}

CSB_V1_ViewportCustomBackgroundSelection
csb_v1_viewport_custom_background_load_and_select_pc34(
    const uint8_t *skin_def,
    size_t skin_def_size,
    CSB_V1_ViewportCustomBackgroundViewIndex view_index)
{
    CSB_V1_ViewportCustomBackgroundSelection result;
    CSB_V1_CustomBackgroundLayerSelection layer;
    size_t bitmap_offset;
    size_t mask_offset;

    memset(&result, 0, sizeof(result));

    /* ReDMCSB DRAWVIEW.C has no custom/background/skin-def references; it is
     * only the shared viewport transfer anchor.  The CSB-only path is the
     * CSBWin extension: CSBCode.cpp:26 declares CustomBackgrounds for the
     * CSB display lane rooted at CSBCode.cpp:9196, while Viewport.cpp
     * 6567-6615 loads a skin definition, selects pSkinDef[0/2/1] bitmaps
     * and pSkinDef[4/6/5] masks, then calls ApplyBackground (6444-6470). */
    if (!skin_def ||
        skin_def_size < CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES ||
        !csb_v1_viewport_custom_background_layer_for_view(view_index, &layer)) {
        return result;
    }

    bitmap_offset = (size_t)layer.bitmap_skin_def_index * 2u;
    mask_offset = (size_t)layer.mask_skin_def_index * 2u;
    if (bitmap_offset + 2u > skin_def_size || mask_offset + 2u > skin_def_size) {
        return result;
    }

    if (csb_v1_read_le16(skin_def) !=
        (uint16_t)CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_GRAPHIC_ID) {
        return result;
    }
    if (csb_v1_read_le16(skin_def + bitmap_offset) == 0 ||
        csb_v1_read_le16(skin_def + mask_offset) == 0) {
        return result;
    }

    result.bitmap = skin_def + bitmap_offset;
    result.mask = skin_def + mask_offset;
    result.byte_width = layer.byte_width;
    result.height = layer.height;
    result.is_valid = 1;
    return result;
}

size_t csb_v1_viewport_custom_background_layer_plan_pc34(
    int room_num,
    CSB_V1_ViewportCustomBackgroundLayerPlan *out_layers,
    size_t out_capacity)
{
    CSB_V1_ViewportCustomBackgroundLayerPlan layers[3];
    size_t count = 0;

    /* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 only establishes the
     * floor/ceiling baseline via F0098 lines 2962-3002. CSBWin extends
     * that baseline at Viewport.cpp lines 6593-6612 by applying large,
     * middle, then near CustomBackgrounds masks, with the near layer gated
     * by roomNum < 5. */
    if (room_num < 0 || room_num >= (int)csb_v1_viewport_custom_background_slot_spec_count()) {
        return 0;
    }

    layers[count++] = (CSB_V1_ViewportCustomBackgroundLayerPlan) {
        CSB_V1_CUSTOM_BACKGROUND_LAYER_LARGE,
        CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_SKIN_DEF_INDEX,
        CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_SKIN_DEF_INDEX,
        CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_MIN_BYTES,
        CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_MIN_BYTES,
        1
    };
    layers[count++] = (CSB_V1_ViewportCustomBackgroundLayerPlan) {
        CSB_V1_CUSTOM_BACKGROUND_LAYER_MIDDLE,
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_SKIN_DEF_INDEX,
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_SKIN_DEF_INDEX,
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_MIN_BYTES,
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_MIN_BYTES,
        1
    };
    if (room_num < CSB_V1_CUSTOM_BACKGROUND_NEAR_ROOM_LIMIT) {
        layers[count++] = (CSB_V1_ViewportCustomBackgroundLayerPlan) {
            CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR,
            CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_SKIN_DEF_INDEX,
            CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_SKIN_DEF_INDEX,
            CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_MIN_BYTES,
            CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_MIN_BYTES,
            1
        };
    }

    if (out_layers && out_capacity > 0) {
        size_t copy_count = count < out_capacity ? count : out_capacity;
        memcpy(out_layers, layers, copy_count * sizeof(layers[0]));
    }

    return count;
}

void csb_v1_viewport_set_dungeon_grid(CSB_V1_ViewportConfig *cfg,
                                       const uint8_t *grid,
                                       int width, int height) {
    if (!cfg) return;
    cfg->dungeon_grid = grid;
    cfg->dungeon_width = width;
    cfg->dungeon_height = height;
}

int csb_v1_viewport_build_dungeon_grid(
    const CSB_V1_DungeonData *dungeon,
    int level,
    uint8_t out_grid[CSB_V1_MAX_SQUARE_SIZE * CSB_V1_MAX_SQUARE_SIZE])
{
    int width;
    int height;
    int max_w;
    int max_h;
    int x;
    int y;

    if (!out_grid) {
        return 0;
    }
    memset(out_grid, 0, CSB_V1_MAX_SQUARE_SIZE * CSB_V1_MAX_SQUARE_SIZE);
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) {
        return 0;
    }
    if (level < 0 || level >= dungeon->level_count) {
        level = 0;
    }

    width = dungeon->level_widths[level];
    height = dungeon->level_heights[level];
    if (width <= 0 || height <= 0) {
        return 0;
    }
    max_w = width < CSB_V1_MAX_SQUARE_SIZE
        ? width
        : CSB_V1_MAX_SQUARE_SIZE;
    max_h = height < CSB_V1_MAX_SQUARE_SIZE
        ? height
        : CSB_V1_MAX_SQUARE_SIZE;

    /* ReDMCSB: DUNGEON.C F0151 stores CSB/DM1 squares column-major.
     * Keep viewport grid materialization in CSB so callers do not inspect
     * raw dungeon map descriptors before entering DUNVIEW.C F0128. */
    for (y = 0; y < max_h; ++y) {
        for (x = 0; x < max_w; ++x) {
            const int square_type =
                csb_v1_dungeon_get_square_type(dungeon, level, x, y);
            out_grid[y * CSB_V1_MAX_SQUARE_SIZE + x] =
                (square_type >= 0) ? (uint8_t)square_type : 0U;
        }
    }
    return 1;
}

int csb_v1_viewport_bind_live_dungeon_grid(
    CSB_V1_ViewportConfig *cfg,
    const CSB_V1_DungeonData *dungeon,
    int level,
    uint8_t out_grid[CSB_V1_MAX_SQUARE_SIZE * CSB_V1_MAX_SQUARE_SIZE])
{
    if (!cfg || !csb_v1_viewport_build_dungeon_grid(dungeon, level, out_grid)) {
        if (out_grid) {
            memset(out_grid, 0,
                   CSB_V1_MAX_SQUARE_SIZE * CSB_V1_MAX_SQUARE_SIZE);
        }
        if (cfg) {
            cfg->dungeon_grid = NULL;
            cfg->dungeon_width = 0;
            cfg->dungeon_height = 0;
        }
        return 0;
    }

    csb_v1_viewport_set_dungeon_grid(cfg, out_grid,
                                     CSB_V1_MAX_SQUARE_SIZE,
                                     CSB_V1_MAX_SQUARE_SIZE);
    return 1;
}

static uint32_t csb_v1_viewport_pack_4bpp_word(const uint8_t *row)
{
    uint32_t word = 0u;
    int pair;

    for (pair = 0; pair < 4; ++pair) {
        uint8_t packed = (uint8_t)(((row[pair * 2] & 0x0fu) << 4) |
                                   (row[pair * 2 + 1] & 0x0fu));
        word |= (uint32_t)packed << (pair * 8);
    }
    return word;
}

static void csb_v1_viewport_unpack_4bpp_word(uint32_t word, uint8_t *row)
{
    int pair;

    for (pair = 0; pair < 4; ++pair) {
        uint8_t packed = (uint8_t)((word >> (pair * 8)) & 0xffu);
        row[pair * 2] = (uint8_t)((packed >> 4) & 0x0fu);
        row[pair * 2 + 1] = (uint8_t)(packed & 0x0fu);
    }
}

static int csb_v1_viewport_apply_configured_custom_backgrounds(
    CSB_V1_ViewportConfig *cfg,
    int party_dir,
    int party_x,
    int party_y,
    int preserve_existing)
{
    const int viewport_word_stride = DM1_VIEWPORT_WIDTH / 8;
    const size_t viewport_word_count =
        (size_t)viewport_word_stride * (size_t)DM1_VIEWPORT_HEIGHT;
    const CSB_V1_CSBGraphicsRuntimePlan *plan;
    const CSB_V1_CSBGraphicsDatRealCache *cache;
    uint32_t *viewport_words;
    uint16_t selected_skin_def_words[CSB_V1_CSBGRAPHICS_RUNTIME_SKIN_DEF_MAX_WORDS];
    const uint16_t *skin_def_words;
    size_t skin_def_word_count;
    const CSB_V1_CustomBackgroundsRoomSlotContract *room_contract;
    int first_room;
    int last_room;
    int room_index;
    int layer;
    int applied = 0;

    if (!cfg || !cfg->viewport_pixels ||
        !cfg->csbgraphics_plan || !cfg->csbgraphics_cache) {
        if (cfg) {
            cfg->custom_background_applied_count = 0;
            cfg->custom_background_applied_room_mask = 0u;
            cfg->custom_background_last_room_num = -1;
        }
        return 0;
    }
    plan = (const CSB_V1_CSBGraphicsRuntimePlan *)cfg->csbgraphics_plan;
    cache = (const CSB_V1_CSBGraphicsDatRealCache *)cfg->csbgraphics_cache;
    if (!preserve_existing) {
        cfg->custom_background_selected_skin_num = 0;
        cfg->custom_background_used_default_skin = 0;
        cfg->custom_background_applied_room_mask = 0u;
        cfg->custom_background_last_room_num = -1;
        cfg->custom_background_applied_count = 0;
    }

    room_contract = csb_v1_viewport_custom_backgrounds_room_slot_contract_pc34();
    if (cfg->custom_background_room_num >= 0) {
        first_room = cfg->custom_background_room_num;
        last_room = cfg->custom_background_room_num;
    } else if (((cfg->custom_background_cell_skins &&
                 cfg->custom_background_cell_skin_width > 0 &&
                 cfg->custom_background_cell_skin_height > 0) ||
                cfg->custom_background_default_skin > 0) &&
               room_contract && room_contract->room_slot_count > 0) {
        first_room = 0;
        last_room = room_contract->room_slot_count - 1;
    } else {
        cfg->custom_background_applied_count = 0;
        return 0;
    }

    for (room_index = first_room; room_index <= last_room; ++room_index) {
        int room_num = room_index;
        int room_applied = 0;
        skin_def_words = cfg->custom_background_skin_def_words;
        skin_def_word_count = cfg->custom_background_skin_def_word_count;
        if (cfg->custom_background_room_num < 0) {
            const CSB_V1_CustomBackgroundsRoomSlotSpec *slot =
                csb_v1_viewport_custom_backgrounds_room_slot_spec_pc34(
                    (size_t)room_index);
            if (!slot) {
                continue;
            }
            room_num = slot->room_num;
        }
        if (preserve_existing && room_num >= 0 && room_num < 32 &&
            (cfg->custom_background_applied_room_mask &
             ((uint32_t)1u << (uint32_t)room_num)) != 0u) {
            continue;
        }

        CSB_V1_CustomBackgroundsRoomSlotSelection selection;
        if (((cfg->custom_background_cell_skins &&
              cfg->custom_background_cell_skin_width > 0 &&
              cfg->custom_background_cell_skin_height > 0) ||
             cfg->custom_background_default_skin > 0) &&
            csb_v1_viewport_custom_backgrounds_room_slot_select_pc34(
                cfg->custom_background_cell_skins,
                cfg->custom_background_cell_skin_width,
                cfg->custom_background_cell_skin_height,
                cfg->custom_background_loaded_level,
                party_x,
                party_y,
                party_dir,
                room_num,
                cfg->custom_background_default_skin,
                &selection) &&
            selection.has_custom_background_entry) {
            size_t selected_word_count = 0u;
            int rc = csb_v1_csbgraphics_runtime_plan_decode_custom_background_skin_def_for_skin(
                cache,
                (uint32_t)selection.selected_skin,
                selected_skin_def_words,
                CSB_V1_CSBGRAPHICS_RUNTIME_SKIN_DEF_MAX_WORDS,
                &selected_word_count);
            cfg->custom_background_selected_skin_num = selection.selected_skin;
            cfg->custom_background_used_default_skin = selection.used_default_skin;
            if (rc == CSB_V1_CSBGRAPHICS_RUNTIME_PLAN_OK &&
                selected_word_count > 0u) {
                skin_def_words = selected_skin_def_words;
                skin_def_word_count = selected_word_count;
            }
        }
        if (!skin_def_words || skin_def_word_count == 0u) {
            continue;
        }

        viewport_words = (uint32_t *)calloc(viewport_word_count,
                                            sizeof(*viewport_words));
        if (!viewport_words) {
            cfg->custom_background_applied_count = applied;
            return applied;
        }

        for (int y = 0; y < DM1_VIEWPORT_HEIGHT; ++y) {
            uint8_t *row = cfg->viewport_pixels +
                (DM1_VIEWPORT_SCREEN_Y + y) *
                    (cfg->viewport_stride > 0 ? cfg->viewport_stride : 320) +
                DM1_VIEWPORT_SCREEN_X;
            for (int x = 0; x < DM1_VIEWPORT_WIDTH; x += 8) {
                viewport_words[(size_t)y * (size_t)viewport_word_stride +
                               (size_t)(x / 8)] =
                    csb_v1_viewport_pack_4bpp_word(row + x);
            }
        }

        /* CSB-lineage Viewport.cpp:5367-5381 and 6599-6619 apply room
         * pSkinDef layers in large, middle, then near order. Prefer caller-
         * supplied geometry for synthetic gates; otherwise decode the CSBWin
         * BACKGROUND_MASK for this room from the mask graphic entry itself. */
        for (layer = 0; layer < 3; ++layer) {
            int rc;
            if (cfg->custom_background_layer_mask_valid[layer]) {
                rc = csb_v1_csbgraphics_runtime_plan_apply_custom_background_room_layer(
                    plan,
                    cache,
                    room_num,
                    (CSB_V1_CSBGraphicsRuntimeCustomBackgroundLayer)layer,
                    skin_def_words,
                    skin_def_word_count,
                    &cfg->custom_background_layer_masks[layer],
                    viewport_words,
                    viewport_word_count,
                    DM1_VIEWPORT_WIDTH);
            } else {
                rc = csb_v1_csbgraphics_runtime_plan_apply_custom_background_room_layer_auto_mask(
                    plan,
                    cache,
                    room_num,
                    (CSB_V1_CSBGraphicsRuntimeCustomBackgroundLayer)layer,
                    skin_def_words,
                    skin_def_word_count,
                    viewport_words,
                    viewport_word_count,
                    DM1_VIEWPORT_WIDTH);
            }
            if (rc == CSB_V1_CSBGRAPHICS_RUNTIME_PLAN_OK) {
                ++applied;
                ++room_applied;
            }
        }

        if (room_applied > 0) {
            cfg->custom_background_last_room_num = room_num;
            if (room_num >= 0 && room_num < 32) {
                cfg->custom_background_applied_room_mask |=
                    (uint32_t)1u << (uint32_t)room_num;
            }
            for (int y = 0; y < DM1_VIEWPORT_HEIGHT; ++y) {
                uint8_t *row = cfg->viewport_pixels +
                    (DM1_VIEWPORT_SCREEN_Y + y) *
                        (cfg->viewport_stride > 0 ? cfg->viewport_stride : 320) +
                    DM1_VIEWPORT_SCREEN_X;
                for (int x = 0; x < DM1_VIEWPORT_WIDTH; x += 8) {
                    csb_v1_viewport_unpack_4bpp_word(
                        viewport_words[(size_t)y * (size_t)viewport_word_stride +
                                       (size_t)(x / 8)],
                        row + x);
                }
            }
        }
        free(viewport_words);
    }
    if (preserve_existing) {
        cfg->custom_background_applied_count += applied;
    } else {
        cfg->custom_background_applied_count = applied;
    }
    return applied;
}

typedef struct {
    CSB_V1_ViewportConfig *cfg;
    int party_dir;
    int party_x;
    int party_y;
} CSB_V1_CustomBackgroundsPreSquareDrawContext;

static int csb_v1_viewport_custom_background_room_for_square(
    DM1_ViewSquareIndex square)
{
    if (square == DM1_VIEW_SQUARE_D3L2) return 0;
    if (square == DM1_VIEW_SQUARE_D3R2) return 1;
    if (square == DM1_VIEW_SQUARE_D3L) return 2;
    if (square == DM1_VIEW_SQUARE_D3R) return 3;
    if (square == DM1_VIEW_SQUARE_D3C) return 4;
    if (square == DM1_VIEW_SQUARE_D2L2) return 5;
    if (square == DM1_VIEW_SQUARE_D2R2) return 6;
    if (square == DM1_VIEW_SQUARE_D2L) return 7;
    if (square == DM1_VIEW_SQUARE_D2R) return 8;
    if (square == DM1_VIEW_SQUARE_D2C) return 9;
    if (square == DM1_VIEW_SQUARE_D1L) return 10;
    if (square == DM1_VIEW_SQUARE_D1R) return 11;
    if (square == DM1_VIEW_SQUARE_D1C) return 12;
    if (square == DM1_VIEW_SQUARE_D0L) return 13;
    if (square == DM1_VIEW_SQUARE_D0R) return 14;
    if (square == DM1_VIEW_SQUARE_D0C) return 15;
    return -1;
}

static void csb_v1_viewport_apply_custom_backgrounds_before_square(
    void *user_data,
    DM1_ViewSquareIndex square,
    int relative_forward,
    int relative_side)
{
    CSB_V1_CustomBackgroundsPreSquareDrawContext *ctx =
        (CSB_V1_CustomBackgroundsPreSquareDrawContext *)user_data;
    CSB_V1_ViewportConfig *cfg;
    int room_num;
    int original_room_num;

    (void)relative_forward;
    (void)relative_side;
    if (!ctx || !ctx->cfg) {
        return;
    }
    cfg = ctx->cfg;
    room_num = csb_v1_viewport_custom_background_room_for_square(square);
    if (room_num < 0) {
        return;
    }
    if (cfg->custom_background_room_num >= 0 &&
        cfg->custom_background_room_num != room_num) {
        return;
    }

    original_room_num = cfg->custom_background_room_num;
    cfg->custom_background_room_num = room_num;
    (void)csb_v1_viewport_apply_configured_custom_backgrounds(
        cfg, ctx->party_dir, ctx->party_x, ctx->party_y, 1);
    cfg->custom_background_room_num = original_room_num;
}

/* csb_v1_viewport_render_frame — integration entry point
 *
 * Renders the CSB dungeon view by delegating to the shared DM1 V1 viewport
 * engine (dm1_viewport_3d_draw_frame).  CSB config provides:
 *   - viewport_pixels + stride: the pixel buffer to draw into
 *   - dungeon_grid/width/height: square type data for element routing
 *   - wall_set_index: selects which GRAPHICS.DAT wall set to use
 *
 * When viewport_pixels is NULL, this is a no-op (allows staged integration).
 *
 * Source: CSBWin/Viewport.cpp F0128 passthrough; ReDMCSB DUNVIEW.C F0128
 */
void csb_v1_viewport_render_frame(CSB_V1_ViewportConfig *cfg,
                                   int party_dir,
                                   int party_x,
                                   int party_y)
{
    if (!cfg) return;

    /* Guard: no viewport buffer means not yet initialised */
    if (!cfg->viewport_pixels) return;

    /* Set up a DM1 viewport state backed by our CSB pixel buffer.
     * We share the exact same pixel format (320×200 indexed, viewport
     * sub-region 224×136 at screen row 33) so the DM1 draw primitives
     * work without modification. */
    DM1_Viewport3DState vp;
    CSB_V1_CustomBackgroundsPreSquareDrawContext custom_bg_ctx;
    memset(&vp, 0, sizeof(vp));
    vp.viewport_pixels = cfg->viewport_pixels;
    vp.viewport_stride = cfg->viewport_stride > 0 ? cfg->viewport_stride : 320;
    vp.floor_area = cfg->viewport_pixels +
                    DM1_VIEWPORT_FLOOR_Y * vp.viewport_stride;
    vp.floor_ceiling_dirty = true;
    vp.graphic_provider_callback = cfg->graphic_provider_callback;
    vp.graphic_provider_user_data = cfg->graphic_provider_user_data;

    /* Wire dungeon grid for CSB back-wall rendering (D3L2/D3R2/D2L2/D2R2).
     * The dungeon grid enables element-specific routing for CSB four-sided
     * wall decoration: walls, doors, stairs, pits, teleporters, corridors.
     * When dungeon_grid is NULL, CSB back-wall rendering falls back to
     * the generic wall-drawing path (same as DM1).
     *
     * Grid layout: dungeon_grid[y * dungeon_width + x] = raw dungeon cell.
     * Raw cell: low 5 bits = element type (0=WALL, 1=CORRIDOR, 2=PIT,
     * 3=STAIRS, 4=DOOR, 5=TELEPORTER, 6=FAKEWALL).
     *
     * Source: ReDMCSB DUNVIEW.C:6226-6353 F0676/F0677; 6837-6896 F0678/F0679 */
    vp.dungeon_grid   = cfg->dungeon_grid;
    vp.dungeon_width  = cfg->dungeon_width;
    vp.dungeon_height = cfg->dungeon_height;

    cfg->custom_background_selected_skin_num = 0;
    cfg->custom_background_used_default_skin = 0;
    cfg->custom_background_applied_room_mask = 0u;
    cfg->custom_background_last_room_num = -1;
    cfg->custom_background_applied_count = 0;

    memset(&custom_bg_ctx, 0, sizeof(custom_bg_ctx));
    custom_bg_ctx.cfg = cfg;
    custom_bg_ctx.party_dir = party_dir;
    custom_bg_ctx.party_x = party_x;
    custom_bg_ctx.party_y = party_y;
    vp.pre_square_draw_callback =
        csb_v1_viewport_apply_custom_backgrounds_before_square;
    vp.pre_square_draw_user_data = &custom_bg_ctx;

    /* Wall set index — CSB may use a different wall set than DM1.
     * ReDMCSB DUNVIEW.C F0096 loads wall set based on current map index.
     * Here we accept the index from config; 0 = default CSB wall set. */
    dm1_viewport_3d_load_wall_set(&vp, cfg->wall_set_index, 0);

    /* Main draw call — mirrors ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF.
     * Draws wall frames for D4 far objects, then D3L/R/C → D2 → D1 → D0
     * back-to-front with correct depth occlusion and parity flip.
     *
     * The CSB-specific elements (back-walls D3L2/D3R2, near-walls D2L2/D2R2)
     * are handled by dm1_viewport_3d_draw_csb_back_wall and
     * dm1_viewport_3d_draw_csb_near_wall, which are invoked from the
     * dm1_viewport_3d_draw_frame wall loop for D3L2/D3R2/D2L2/D2R2 positions. */
    dm1_viewport_3d_draw_frame(&vp, party_dir, party_x, party_y);
    (void)csb_v1_viewport_apply_configured_custom_backgrounds(
        cfg, party_dir, party_x, party_y, 1);
    if (cfg->runtime_profile) {
        csb_v1_viewport_draw_runtime_thing_overlays(cfg);
    }
    if (cfg->first_frame_material_proof) {
        CSB_V1_ViewportFirstFrameMaterializationReceipt receipt;
        CSB_V1_ViewportRuntimeDrawPlanPc34 draw_plan;
        if (csb_v1_viewport_admit_first_frame_materialization_pc34(
                cfg->first_frame_material_proof,
                cfg->real_graphics_session,
                &receipt) &&
            csb_v1_viewport_build_first_frame_runtime_draw_plan_pc34(
                cfg->first_frame_material_proof,
                cfg->real_graphics_session,
                party_dir,
                party_x,
                party_y,
                &draw_plan)) {
            cfg->first_frame_material_consumed_count = 1;
            cfg->first_frame_material_blocked_count = 0;
            cfg->first_frame_material_hash = receipt.combined_material_hash;
            cfg->first_frame_draw_plan_consumed_count = 1;
            cfg->first_frame_draw_plan_blocked_count = 0;
            cfg->first_frame_draw_plan_command_count = draw_plan.command_count;
            cfg->first_frame_draw_plan_hash = draw_plan.plan_hash;
            cfg->first_frame_draw_plan_palette_hash =
                draw_plan.shared_palette_hash;
            if (cfg->first_frame_material_bytes &&
                csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                    cfg->first_frame_material_proof, &draw_plan,
                    cfg->first_frame_material_bytes, &receipt) &&
                csb_v1_viewport_consume_first_frame_material_raster_pc34(
                    &receipt, &draw_plan, cfg->first_frame_material_bytes,
                    cfg->first_frame_material_source_path,
                    cfg->first_frame_material_source_md5,
                    cfg->viewport_pixels, vp.viewport_stride, 200,
                    NULL)) {
                cfg->first_frame_material_raster_consumed_count = 1;
                cfg->first_frame_material_raster_blocked_count = 0;
                cfg->first_frame_material_raster_hash =
                    csb_v1_viewport_fnv1a_bytes_pc34(
                        cfg->viewport_pixels,
                        (size_t)vp.viewport_stride * 200u);
            } else {
                cfg->first_frame_material_raster_consumed_count = 0;
                cfg->first_frame_material_raster_blocked_count = 1;
                cfg->first_frame_material_raster_hash = 0u;
            }
        } else {
            cfg->first_frame_material_consumed_count = 0;
            cfg->first_frame_material_blocked_count = 1;
            cfg->first_frame_material_hash = 0u;
            cfg->first_frame_draw_plan_consumed_count = 0;
            cfg->first_frame_draw_plan_blocked_count = 1;
            cfg->first_frame_draw_plan_command_count = 0;
            cfg->first_frame_draw_plan_hash = 0u;
            cfg->first_frame_draw_plan_palette_hash = 0u;
            cfg->first_frame_material_raster_consumed_count = 0;
            cfg->first_frame_material_raster_blocked_count = 1;
            cfg->first_frame_material_raster_hash = 0u;
        }
    } else {
        cfg->first_frame_material_consumed_count = 0;
        cfg->first_frame_material_blocked_count = 0;
        cfg->first_frame_material_hash = 0u;
        cfg->first_frame_draw_plan_consumed_count = 0;
        cfg->first_frame_draw_plan_blocked_count = 0;
        cfg->first_frame_draw_plan_command_count = 0;
        cfg->first_frame_draw_plan_hash = 0u;
        cfg->first_frame_draw_plan_palette_hash = 0u;
        cfg->first_frame_material_raster_consumed_count = 0;
        cfg->first_frame_material_raster_blocked_count = 0;
        cfg->first_frame_material_raster_hash = 0u;
    }
    csb_v1_viewport_draw_runtime_projectile_overlays(
        cfg, party_dir, party_x, party_y);
    csb_v1_viewport_draw_runtime_explosion_overlays(
        cfg, party_dir, party_x, party_y);
}

/* ReDMCSB: DUNVIEW.C F0678 lines 6848-6862 and F0679 lines
 * 6879-6893.  The CSB/I34 D2L2/D2R2 wall case draws only the wall panel:
 * normal rendering uses its own G2107 wall-set bitmap (C06 for D2L2,
 * C05 for D2R2), while G0076_B_UseFlippedWallAndFootprintsBitmaps swaps
 * to the opposite bitmap and calls the flipped blitter.  The wall branch
 * returns before the teleporter-only F0113 path at lines 6863-6865 and
 * 6894-6896. */
int csb_v1_viewport_near_wall_d2_wall_bitmap_index(int view_square,
                                                   int use_flipped_wall_bitmaps)
{
    if (view_square == (int)DM1_VIEW_SQUARE_D2L2) {
        return use_flipped_wall_bitmaps ? CSB_V1_WALL_BITMAP_D2R2 :
                                          CSB_V1_WALL_BITMAP_D2L2;
    }
    if (view_square == (int)DM1_VIEW_SQUARE_D2R2) {
        return use_flipped_wall_bitmaps ? CSB_V1_WALL_BITMAP_D2L2 :
                                          CSB_V1_WALL_BITMAP_D2R2;
    }
    return -1;
}

int csb_v1_viewport_near_wall_d2_wall_zone(int view_square)
{
    if (view_square == (int)DM1_VIEW_SQUARE_D2L2) return CSB_V1_FIELD_ZONE_D2L2;
    if (view_square == (int)DM1_VIEW_SQUARE_D2R2) return CSB_V1_FIELD_ZONE_D2R2;
    return -1;
}

int csb_v1_viewport_near_wall_d2_wall_uses_flipped_blit(
    int view_square,
    int use_flipped_wall_bitmaps)
{
    if (view_square != (int)DM1_VIEW_SQUARE_D2L2 &&
        view_square != (int)DM1_VIEW_SQUARE_D2R2) {
        return -1;
    }
    return use_flipped_wall_bitmaps ? 1 : 0;
}

size_t csb_v1_viewport_wall_ornament_route_spec_count(void)
{
    return sizeof(s_wall_ornament_routes) / sizeof(s_wall_ornament_routes[0]);
}

const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_route_spec_count()) return NULL;
    return &s_wall_ornament_routes[index];
}

const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_wall_ornament_route_spec_count(); ++i) {
        if (s_wall_ornament_routes[i].view_square == view_square) {
            return &s_wall_ornament_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_wall_ornament_blit_spec_count(void)
{
    return sizeof(s_wall_ornament_blits) / sizeof(s_wall_ornament_blits[0]);
}

const CSB_V1_ViewportWallOrnamentBlitSpec *csb_v1_viewport_get_wall_ornament_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_blit_spec_count()) return NULL;
    return &s_wall_ornament_blits[index];
}

const CSB_V1_ViewportWallOrnamentBlitSpec *csb_v1_viewport_get_wall_ornament_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_wall_ornament_blit_spec_count(); ++i) {
        if (s_wall_ornament_blits[i].view_square == view_square) {
            return &s_wall_ornament_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_wall_ornament_blit_zone(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                            int coordinate_set)
{
    if (!spec || coordinate_set < 0) return -1;
    return spec->zone_base + (coordinate_set * spec->coordinate_set_stride) +
           spec->view_wall_index;
}

int csb_v1_viewport_wall_ornament_native_bitmap_index(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                                      int base_native_bitmap_index)
{
    if (!spec || base_native_bitmap_index < 0) return -1;
    return base_native_bitmap_index + spec->native_bitmap_index_increment;
}

int csb_v1_viewport_wall_ornament_blit_pixels(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                              const uint8_t *source,
                                              int source_stride,
                                              uint8_t *destination,
                                              int destination_stride,
                                              int width,
                                              int height)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride <= 0 || destination_stride <= 0 ||
        width <= 0 || height <= 0) {
        return -1;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sx = spec->horizontal_flip ? (width - 1 - x) : x;
            uint8_t pixel = source[(y * source_stride) + sx];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

size_t csb_v1_viewport_wall_ornament_side_effect_spec_count(void)
{
    return sizeof(s_wall_ornament_side_effects) / sizeof(s_wall_ornament_side_effects[0]);
}

const CSB_V1_ViewportWallOrnamentSideEffectSpec *csb_v1_viewport_get_wall_ornament_side_effect_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_side_effect_spec_count()) return NULL;
    return &s_wall_ornament_side_effects[index];
}

const CSB_V1_ViewportWallOrnamentSideEffectSpec *csb_v1_viewport_get_wall_ornament_side_effect_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_wall_ornament_side_effect_spec_count(); ++i) {
        if (s_wall_ornament_side_effects[i].view_square == view_square) {
            return &s_wall_ornament_side_effects[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_wall_ornament_d1d2_path_spec_count(void)
{
    return sizeof(s_wall_ornament_d1d2_paths) / sizeof(s_wall_ornament_d1d2_paths[0]);
}

const CSB_V1_ViewportWallOrnamentD1D2PathSpec *
csb_v1_viewport_get_wall_ornament_d1d2_path_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_d1d2_path_spec_count()) return NULL;
    return &s_wall_ornament_d1d2_paths[index];
}

int csb_v1_viewport_wall_ornament_d1d2_path_zone(
    const CSB_V1_ViewportWallOrnamentD1D2PathSpec *spec,
    int coordinate_set)
{
    if (!spec || coordinate_set < 0) return -1;
    return spec->zone_base + (coordinate_set * spec->coordinate_set_stride) +
           spec->view_wall_index;
}

size_t csb_v1_viewport_floor_ornament_route_spec_count(void)
{
    return sizeof(s_floor_ornament_routes) / sizeof(s_floor_ornament_routes[0]);
}

const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec(size_t index)
{
    if (index >= csb_v1_viewport_floor_ornament_route_spec_count()) return NULL;
    return &s_floor_ornament_routes[index];
}

const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_floor_ornament_route_spec_count(); ++i) {
        if (s_floor_ornament_routes[i].view_square == view_square) {
            return &s_floor_ornament_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_floor_ornament_blit_spec_count(void)
{
    return sizeof(s_floor_ornament_blits) / sizeof(s_floor_ornament_blits[0]);
}

const CSB_V1_ViewportFloorOrnamentBlitSpec *csb_v1_viewport_get_floor_ornament_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_floor_ornament_blit_spec_count()) return NULL;
    return &s_floor_ornament_blits[index];
}

const CSB_V1_ViewportFloorOrnamentBlitSpec *csb_v1_viewport_get_floor_ornament_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_floor_ornament_blit_spec_count(); ++i) {
        if (s_floor_ornament_blits[i].view_square == view_square) {
            return &s_floor_ornament_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_floor_ornament_blit_zone(const CSB_V1_ViewportFloorOrnamentBlitSpec *spec,
                                             int coordinate_set)
{
    if (!spec || coordinate_set < 0) return -1;
    return spec->zone_base + (coordinate_set * spec->coordinate_set_stride) + spec->floor_view_index;
}

int csb_v1_viewport_floor_ornament_native_bitmap_index(const CSB_V1_ViewportFloorOrnamentBlitSpec *spec,
                                                       int base_native_bitmap_index)
{
    if (!spec || base_native_bitmap_index < 0) return -1;
    return base_native_bitmap_index + spec->native_bitmap_index_increment;
}

size_t csb_v1_viewport_thing_pass_order_spec_count(void)
{
    return sizeof(s_thing_pass_order_routes) / sizeof(s_thing_pass_order_routes[0]);
}

const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec(size_t index)
{
    if (index >= csb_v1_viewport_thing_pass_order_spec_count()) return NULL;
    return &s_thing_pass_order_routes[index];
}

const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_thing_pass_order_spec_count(); ++i) {
        if (s_thing_pass_order_routes[i].view_square == view_square) {
            return &s_thing_pass_order_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_object_visibility_spec_count(void)
{
    return sizeof(s_object_visibility_routes) / sizeof(s_object_visibility_routes[0]);
}

const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec(size_t index)
{
    if (index >= csb_v1_viewport_object_visibility_spec_count()) return NULL;
    return &s_object_visibility_routes[index];
}

const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_object_visibility_spec_count(); ++i) {
        if (s_object_visibility_routes[i].view_square == view_square) {
            return &s_object_visibility_routes[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_object_visibility_allows_cell(const CSB_V1_ViewportObjectVisibilitySpec *spec,
                                                  unsigned char cell_ordinal)
{
    if (!spec) return 0;
    if (cell_ordinal < spec->first_visible_cell_ordinal) return 0;
    if (cell_ordinal > spec->last_visible_cell_ordinal) return 0;
    return 1;
}

size_t csb_v1_viewport_object_blit_spec_count(void)
{
    return sizeof(s_object_blits) / sizeof(s_object_blits[0]);
}

const CSB_V1_ViewportObjectBlitSpec *csb_v1_viewport_get_object_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_object_blit_spec_count()) return NULL;
    return &s_object_blits[index];
}

const CSB_V1_ViewportObjectBlitSpec *csb_v1_viewport_get_object_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_object_blit_spec_count(); ++i) {
        if (s_object_blits[i].view_square == view_square ||
            s_object_blits[i].redmcsb_view_square_index == view_square) {
            return &s_object_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_object_blit_layout_zone(const CSB_V1_ViewportObjectBlitSpec *spec,
                                            unsigned char view_cell)
{
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *route;
    if (!spec || view_cell > 4 || spec->object_visibility_row < 0) return -1;
    route = csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
        spec->redmcsb_view_square_index,
        CSB_V1_D3L2_D3R2_F0115_ROUTE_ITEM_PC34);
    if (route) {
        const int routed_zone = csb_v1_viewport_d3l2_d3r2_f0115_item_layout_zone_pc34(
            route, (int)view_cell);
        if (routed_zone >= 0) return routed_zone;
    }
    return spec->object_zone_base +
           (spec->object_visibility_row * spec->object_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_object_blit_zone(const CSB_V1_ViewportObjectBlitSpec *spec,
                                     unsigned char view_cell)
{
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *route;
    int zone = csb_v1_viewport_object_blit_layout_zone(spec, view_cell);
    if (zone < 0) return -1;
    route = csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
        spec->redmcsb_view_square_index,
        CSB_V1_D3L2_D3R2_F0115_ROUTE_ITEM_PC34);
    if (route) {
        const int routed_zone = csb_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(
            route, (int)view_cell);
        if (routed_zone >= 0) return routed_zone;
    }
    return zone | spec->shifts_objects_and_creatures;
}

size_t csb_v1_viewport_projectile_blit_spec_count(void)
{
    return sizeof(s_projectile_blits) / sizeof(s_projectile_blits[0]);
}

const CSB_V1_ViewportProjectileBlitSpec *csb_v1_viewport_get_projectile_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_projectile_blit_spec_count()) return NULL;
    return &s_projectile_blits[index];
}

const CSB_V1_ViewportProjectileBlitSpec *csb_v1_viewport_get_projectile_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_projectile_blit_spec_count(); ++i) {
        if (s_projectile_blits[i].view_square == view_square) {
            return &s_projectile_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_projectile_blit_zone(const CSB_V1_ViewportProjectileBlitSpec *spec,
                                         unsigned char view_cell)
{
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *route;
    if (!spec || view_cell > 4 || spec->projectile_visibility_row < 0) return -1;
    route = csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
        spec->redmcsb_view_square_index,
        CSB_V1_D3L2_D3R2_F0115_ROUTE_PROJECTILE_PC34);
    if (route) {
        const int routed_zone = csb_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(
            route, (int)view_cell);
        if (routed_zone >= 0) return routed_zone;
    }
    if (spec->view_depth == 3 && spec->suppresses_depth3_front_cells && view_cell <= 1) {
        return -1;
    }
    if (spec->view_depth == 0 && spec->suppresses_depth0_back_cells && view_cell >= 2) {
        return -1;
    }
    return spec->projectile_zone_base +
           (spec->projectile_visibility_row * spec->projectile_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_projectile_blit_pixels(const CSB_V1_ViewportProjectileBlitSpec *spec,
                                           int flip_flags,
                                           const uint8_t *source,
                                           int source_stride,
                                           uint8_t *destination,
                                           int destination_stride,
                                           int width,
                                           int height)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride < width || destination_stride < width ||
        width <= 0 || height <= 0) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C F0115 lines 5755-5762/5791-5802 build
     * MASK0x0001/MASK0x0002 flip flags dynamically, then lines 5881-5882
     * send the scaled bitmap through F0791 with C10 transparency. */
    for (int y = 0; y < height; ++y) {
        int sy = (flip_flags & CSB_V1_FLIP_VERTICAL) ? (height - 1 - y) : y;
        for (int x = 0; x < width; ++x) {
            int sx = (flip_flags & CSB_V1_FLIP_HORIZONTAL) ? (width - 1 - x) : x;
            uint8_t pixel = source[(sy * source_stride) + sx];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

size_t csb_v1_viewport_creature_visibility_spec_count(void)
{
    return sizeof(s_creature_visibility_routes) / sizeof(s_creature_visibility_routes[0]);
}

const CSB_V1_ViewportCreatureVisibilitySpec *csb_v1_viewport_get_creature_visibility_spec(size_t index)
{
    if (index >= csb_v1_viewport_creature_visibility_spec_count()) return NULL;
    return &s_creature_visibility_routes[index];
}

const CSB_V1_ViewportCreatureVisibilitySpec *csb_v1_viewport_get_creature_visibility_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_creature_visibility_spec_count(); ++i) {
        if (s_creature_visibility_routes[i].view_square == view_square ||
            s_creature_visibility_routes[i].redmcsb_view_square_index ==
                view_square) {
            return &s_creature_visibility_routes[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_creature_visibility_zone(const CSB_V1_ViewportCreatureVisibilitySpec *spec,
                                             int coordinate_set,
                                             unsigned char view_cell)
{
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *route;
    if (!spec || coordinate_set < 0 || view_cell > 4 || spec->creature_visibility_row < 0) {
        return -1;
    }
    route = csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
        spec->redmcsb_view_square_index,
        CSB_V1_D3L2_D3R2_F0115_ROUTE_CREATURE_PC34);
    if (route) {
        const int routed_zone = csb_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(
            route, coordinate_set, (int)view_cell);
        if (routed_zone >= 0) return routed_zone;
    }
    return spec->creature_zone_base +
           (coordinate_set * spec->creature_coordinate_set_stride) +
           (spec->creature_visibility_row * spec->creature_zone_cell_stride) +
           view_cell;
}

size_t csb_v1_viewport_explosion_blit_spec_count(void)
{
    return sizeof(s_explosion_blits) / sizeof(s_explosion_blits[0]);
}

const CSB_V1_ViewportExplosionBlitSpec *csb_v1_viewport_get_explosion_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_explosion_blit_spec_count()) return NULL;
    return &s_explosion_blits[index];
}

const CSB_V1_ViewportExplosionBlitSpec *csb_v1_viewport_get_explosion_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_explosion_blit_spec_count(); ++i) {
        if (s_explosion_blits[i].view_square == view_square) {
            return &s_explosion_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_explosion_rebirth_step1_zone(const CSB_V1_ViewportExplosionBlitSpec *spec)
{
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *route;
    if (!spec || spec->explosion_row < 0) return -1;
    route = csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
        spec->redmcsb_view_square_index,
        CSB_V1_D3L2_D3R2_F0115_ROUTE_EXPLOSION_PC34);
    if (route) {
        return csb_v1_viewport_d3l2_d3r2_f0115_explosion_rebirth_step1_zone_pc34(route);
    }
    return spec->rebirth_step1_zone_base + spec->explosion_row;
}

int csb_v1_viewport_explosion_rebirth_step2_zone(const CSB_V1_ViewportExplosionBlitSpec *spec)
{
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *route;
    if (!spec || spec->explosion_row < 0) return -1;
    route = csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
        spec->redmcsb_view_square_index,
        CSB_V1_D3L2_D3R2_F0115_ROUTE_EXPLOSION_PC34);
    if (route) {
        return csb_v1_viewport_d3l2_d3r2_f0115_explosion_rebirth_step2_zone_pc34(route);
    }
    return spec->rebirth_step2_zone_base + spec->explosion_row;
}

int csb_v1_viewport_explosion_centered_zone(const CSB_V1_ViewportExplosionBlitSpec *spec)
{
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *route;
    if (!spec || spec->explosion_row < 0) return -1;
    route = csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
        spec->redmcsb_view_square_index,
        CSB_V1_D3L2_D3R2_F0115_ROUTE_EXPLOSION_PC34);
    if (route) {
        return csb_v1_viewport_d3l2_d3r2_f0115_explosion_centered_zone_pc34(route);
    }
    return spec->centered_zone_base + spec->explosion_row;
}

int csb_v1_viewport_explosion_side_zone(const CSB_V1_ViewportExplosionBlitSpec *spec,
                                        unsigned char view_cell)
{
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *route;
    if (!spec || spec->explosion_row < 0 || view_cell > 1) return -1;
    route = csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
        spec->redmcsb_view_square_index,
        CSB_V1_D3L2_D3R2_F0115_ROUTE_EXPLOSION_PC34);
    if (route) {
        return csb_v1_viewport_d3l2_d3r2_f0115_explosion_side_zone_pc34(
            route, (int)view_cell);
    }
    return spec->side_zone_base +
           (spec->explosion_row * spec->side_zone_cell_stride) +
           view_cell;
}

size_t csb_v1_viewport_teleporter_field_spec_count(void)
{
    return sizeof(s_teleporter_fields) / sizeof(s_teleporter_fields[0]);
}

const CSB_V1_ViewportTeleporterFieldSpec *csb_v1_viewport_get_teleporter_field_spec(size_t index)
{
    if (index >= csb_v1_viewport_teleporter_field_spec_count()) return NULL;
    return &s_teleporter_fields[index];
}

const CSB_V1_ViewportTeleporterFieldSpec *csb_v1_viewport_get_teleporter_field_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_teleporter_field_spec_count(); ++i) {
        if (s_teleporter_fields[i].view_square == view_square) {
            return &s_teleporter_fields[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_door_panel_blit_spec_count(void)
{
    return sizeof(s_door_panel_blits) / sizeof(s_door_panel_blits[0]);
}

const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_door_panel_blit_spec_count()) return NULL;
    return &s_door_panel_blits[index];
}

const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_door_panel_blit_spec_count(); ++i) {
        if (s_door_panel_blits[i].view_square == view_square) {
            return &s_door_panel_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_door_panel_first_half_zone(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                               int door_state,
                                               int horizontal_door)
{
    if (!spec || door_state < 0) return -1;
    if (spec->skips_open_state && door_state == 0) return -1;
    if (!horizontal_door) return -1;
    if (door_state == 4 || door_state == spec->destroyed_state) return -1;

    /* ReDMCSB: DUNVIEW.C F0111 lines 4298-4311.  Partially-open
     * horizontal PC34/I34 doors blit their first half through
     * P2084_i_ZoneIndex + DoorState + C6_UNKNOWN before the final half. */
    return spec->door_zone_base + door_state + spec->horizontal_first_half_zone_offset;
}

int csb_v1_viewport_door_panel_final_zone(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                          int door_state,
                                          int horizontal_door)
{
    if (!spec || door_state < 0) return -1;
    if (spec->skips_open_state && door_state == 0) return -1;
    if (door_state == 4 || door_state == spec->destroyed_state) {
        return spec->door_zone_base;
    }

    /* ReDMCSB: DUNVIEW.C F0111 lines 4298-4321 shifts the zone by
     * DoorState for partially-open panels.  Horizontal doors add the second
     * half offset and MASK0x4000 before the final F0791 at line 4334. */
    int zone = spec->door_zone_base + door_state;
    if (horizontal_door) {
        zone += spec->horizontal_second_half_zone_offset;
        zone |= CSB_V1_DOOR_HORIZONTAL_FINAL_SHIFT_MASK;
    }
    return zone;
}

int csb_v1_viewport_door_panel_blit_pixels(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                           int door_state,
                                           const uint8_t *source,
                                           int source_stride,
                                           uint8_t *destination,
                                           int destination_stride)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride < spec->native_bitmap_width ||
        destination_stride < spec->clipped_width ||
        spec->clipped_width <= 0 || spec->clipped_height <= 0 ||
        spec->clipped_width > spec->native_bitmap_width ||
        spec->clipped_height > spec->native_bitmap_height) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C F0111 lines 4248 and 4334 skip open doors, then
     * blit G0074 through F0791 with C10 transparency; COORD.C 1556-1560
     * clips the native D3 48x41 door bitmap to the 48x40 panel record. */
    if (spec->skips_open_state && door_state == 0) {
        return 0;
    }

    for (int y = 0; y < spec->clipped_height; ++y) {
        for (int x = 0; x < spec->clipped_width; ++x) {
            uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    return copied;
}

const char *csb_v1_viewport_source_evidence(void) {
    return
        "ReDMCSB WIP20210206 Toolchains/Common/Source/DUNVIEW.C:\n"
        "  6226-6353 F0676/F0677 back-wall D3L2/D3R2 element routing\n"
        "  6254-6263 F0676 D3L2 wall panel then F0107 right-wall ornament route\n"
        "  6270-6286 F0676 D3L2 F0108 floor ornament, F0115 pass1/pass2, F0111 door panel route\n"
        "  6321-6330 F0677 D3R2 wall panel then F0107 left-wall ornament route\n"
        "  6337-6353 F0677 D3R2 F0108 floor ornament, F0115 pass1/pass2, F0111 door panel route\n"
        "  4567-4581 F0115 draws objects, then creatures, then projectiles per processed view cell\n"
        "  5668-5683 F0115 restarts for projectile things and uses C2900_ZONE_ + G2028 row * 4 + ViewCell\n"
        "  5881-5883 F0115 blits PC34/I34 projectile sprites through the computed C2900 zone\n"
        "  5710-5722 and 5755-5802 F0115 computes PC34/I34 projectile scale and dynamic MASK0x0001/MASK0x0002 flip flags before the C10 F0791 blit\n"
        "  4806-4811 F0115 maps PC34 view square to lane/depth/object visibility rows\n"
        "  4923 F0115 filters weapon..junk objects by visible row, matching cell, and D3/D0 cell gates\n"
        "  5030-5039 F0115 selects PC34/I34 object scale and shift set from depth/cell\n"
        "  5071-5110 F0115 blits objects through C2500_ZONE_ | MASK0x8000 plus G2028 row/cell and pile shifts\n"
        "  375, 5201-5214, 5615-5627 F0115 maps creatures through G2033 and C3200_ZONE_ with MASK0x8000 shifts\n"
        "  5915-5933 F0115 restarts for explosions after all processed view cells\n"
        "  5920-6219 F0115 maps PC34/I34 explosions through G2034/G2035, C3000/C3007/C3014/C3031 zones, F0791 C10 blits, and fluxcage field deferral\n"
        "  6288-6290 and 6355-6357 F0676/F0677 draw teleporter fields through G2035, F0113, and C702/C703 after the F0108/F0115 path\n"
        "  6863-6865 and 6894-6896 F0678/F0679 draw D2L2/D2R2 teleporter fields through G2035, F0113, and C707/C708 without F0108/F0115\n"
        "  6848-6862 and 6879-6893 F0678/F0679 D2L2/D2R2 wall branches swap C06/C05 wall bitmaps under G0076 and return before the teleporter field path\n"
        "  3502-3590, 3817-3829, 3921-3923 F0107 maps CSB/I34 far wall ornaments through C1004 + CoordinateSet*15 + ViewWall, C30/C14 scaling, D3 palette changes, optional D3R2 flip, and F0791 C10 blits\n"
        "  3589, 3608-3753, 3817-3829, 3923-3928 F0107 evaluates F0149 alcove status for C00/C01 but skips D1-only facing state, clickbox copy, and champion portrait overlay while using the I34 D3 CM1_DERIVED_BITMAP_NONE scaled path\n"
        "  6968-6969,7119-7120,7308,7459,7627,7842 F0119-F0124 call F0107 for D2/D1 wall ornaments; D2 uses C21/G0199 derived scaled bitmaps, D1 side uses native CM1_DERIVED_BITMAP_NONE, and only D1C front updates facing/clickbox/portrait state\n"
        "  3940-4008 F0108 floor ornament ordinal/index, G0191 native bitmap increment, C1500 zone, flip, C10 blit dispatch\n"
        "  4218-4337 F0111 door bitmap, ornament, state, zone shift, and C10 transparent blit dispatch\n"
        "  4301-4302 F0111 applies C15_DOOR_ORNAMENT_DESTROYED_MASK for C5_DOOR_STATE_DESTROYED\n"
        "  6837-6896 F0678/F0679 near-wall D2L2/D2R2 element routing\n"
        "  6848-6865 F0678 and 6877-6896 F0679 return for walls without F0107\n"
        "  8337-8339 F0128 draws the standard floor/ceiling baseline before square draws; F0098 2962-3002 draws G2109/G2108 and clears the request\n"
        "  8318-8542 F0128 shared viewport draw sequence\n"
        "  1008-1017 G0195 CSB/I34 floor ornament coordinate-set indices are all 0\n"
        "  DEFS.H:2750-2751 C00_VIEW_FLOOR_D3L2 / C01_VIEW_FLOOR_D3R2\n"
        "  DEFS.H:4250-4251 C3700_ZONE_DOOR_D3L2 / C3710_ZONE_DOOR_D3R2\n"
        "  DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT; COORD.C:903-913 floor ornament zone records\n"
        "  DEFS.H:4222 C1004_ZONE_WALL_ORNAMENT; COORD.C:921-1025 wall ornament zone records\n"
        "  DEFS.H:2703-2710 M580..M587 D2/D1 view-wall indices\n"
        "  DEFS.H:4228 C2500_ZONE_; COORD.C:1129-1193 object zone records\n"
        "  DEFS.H:3517 MASK0x8000_SHIFT_OBJECTS_AND_CREATURES; 4236 C3200_ZONE_; COORD.C:1248-1251,2074-2075 creature zones\n"
        "  DEFS.H:4042-4048 C702/C703/C707/C708 field zones; 4232-4235 explosion zone bases; COORD.C:1058-1123,1194-1238 explosion zone records\n"
        "  DEFS.H:3428-3429 C05_WALL_D2R2 / C06_WALL_D2L2\n"
        "  COORD.C:1548-1565 D3 48x41 native door bitmap and 48x40 clip records; 788-807 far door zones\n"
        "  G0711/G0712 back-wall frame descriptors (lines 579-580)\n"
        "  G2107 WallSet bitmap indices (lines ~183)\n"
        "  G3048 WallSetFlipped (lines 277-295)\n"
        "ReDMCSB DEFS.H:2696-2697 C00_VIEW_WALL_D3L2_RIGHT / C01_VIEW_WALL_D3R2_LEFT\n"
        "ReDMCSB DUNGEON.C:1330-1347 F0149_DUNGEON_IsWallOrnamentAnAlcove scans C003_ALCOVE_ORNAMENT_COUNT\n"
        "CSBWin/Viewport.cpp: 7290 lines viewport rendering\n"
        "CSBWin/Viewport.cpp:5317-5325 relposSid/relposFwd; 5402-5412 GetBitmap CSD/CSD-I34 bitmap selection; 6444-6470 ApplyBackground masked composite; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6919-7140 sixteen background room slots before cell draws\n"
        "CSBWin/Graphics.cpp: 3186 lines asset cache\n"
        "CSBWin/CSBCode.cpp:26 CustomBackgrounds\n"
        "CSBWin/CSBCode.cpp:9196 _DisplayChaosStrikesBack (prison door)\n";
}
