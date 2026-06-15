#include "csb/csb_v1_viewport_d2c_f0111_partly_open_door_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_VIEW_SQUARE_D2C = 6,       /* ReDMCSB: DEFS.H:2602 M603_VIEW_SQUARE_D2C. */
    CSB_F0121_FUNCTION = 121,       /* ReDMCSB: DUNVIEW.C:7244 F0121. */
    CSB_F0128_D2C_ORDER = 10,      /* ReDMCSB: DUNVIEW.C F0128 line 8521. */
    CSB_F0128_D2_DEPTH = 2,        /* ReDMCSB: DUNVIEW.C:8512/8516/8520. */
    CSB_D2C_LANE = 0,              /* ReDMCSB: DUNVIEW.C:8520 (2, 0). */
    CSB_C709_ZONE_WALL_D2C = 709,   /* ReDMCSB: DEFS.H:4049 PC34 form. */
    CSB_C707_ZONE_WALL_D2L2 = 707,  /* ReDMCSB: DEFS.H:4047. */
    CSB_C708_ZONE_WALL_D2R2 = 708,  /* ReDMCSB: DEFS.H:4048. */
    CSB_M628_ZONE_DOOR_D2C = 3760,  /* ReDMCSB: DEFS.H:4256. */
    CSB_C3700_ZONE_DOOR_D2L2 = 3700,/* ReDMCSB: DEFS.H:4242/4250. */
    CSB_C3710_ZONE_DOOR_D2R2 = 3710,/* ReDMCSB: DEFS.H:4244/4252. */
    CSB_D2C_DOOR_WIDTH = 64,        /* ReDMCSB: DUNVIEW.C:7336 64x61. */
    CSB_D2C_DOOR_HEIGHT = 61,       /* ReDMCSB: DUNVIEW.C:7336 64x61. */
    CSB_D2C_DOOR_BYTE_COUNT = 1952, /* (64/2)*61; ReDMCSB: M075_BITMAP_BYTE_COUNT(64, 61). */
    CSB_DOOR_STATE_OPEN = 0,        /* ReDMCSB: DEFS.H:1039 C0_DOOR_STATE_OPEN. */
    CSB_DOOR_STATE_PARTLY_ONE = 1,
    CSB_DOOR_STATE_PARTLY_TWO = 2,
    CSB_DOOR_STATE_PARTLY_THREE = 3,
    CSB_DOOR_STATE_CLOSED = 4,      /* ReDMCSB: DEFS.H:1043. */
    CSB_DOOR_STATE_DESTROYED = 5,   /* ReDMCSB: DEFS.H:1044. */
    CSB_C6_UNKNOWN = 6,             /* ReDMCSB: DEFS.H:3508. */
    CSB_SECOND_HALF_OFFSET = 3,     /* ReDMCSB: DUNVIEW.C:4326 +3. */
    CSB_MASK0X4000 = 0x4000,        /* ReDMCSB: DEFS.H:3516. */
    CSB_C10_COLOR_FLESH = 10,       /* ReDMCSB: DEFS.H:2088. */
    CSB_DOORPASS1_ORDER = 0x0218,   /* ReDMCSB: DUNVIEW.C:7315. */
    CSB_DOORPASS2_ORDER = 0x0349    /* ReDMCSB: DUNVIEW.C:7341. */
};

static const char s_source_evidence[] =
    "PASS659 contract-only synthetic source-lock; no real-asset pixel "
    "parity and no CSB game-data load. ReDMCSB DUNVIEW.C:4218-4337 "
    "F0111_DUNGEONVIEW_DrawDoor anchors the partly-open D2C horizontal "
    "path: 4311-4313 selects D2C.LeftHorizontal[state-1] and "
    "D2C.RightHorizontal[state-1] after the state decrement at the top "
    "of the else branch, 4317-4318 increments P2084_i_ZoneIndex by state, "
    "4320-4324 performs the F0635 zone clip and F0654 C10_COLOR_FLESH "
    "blit for the first horizontal half through zone + C6_UNKNOWN, and "
    "4325-4334 shifts the second half by 3 | MASK0x4000 then blits with "
    "C10_COLOR_FLESH in F0791_DUNGEONVIEW_DrawBitmapXX. DUNVIEW.C:7244-7389 "
    "F0121_DUNGEONVIEW_DrawSquareD2C is the D2C body; the "
    "C17_ELEMENT_DOOR_FRONT branch at 7313-7341 dispatches F0111 with "
    "G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, "
    "M075_BITMAP_BYTE_COUNT(64, 61), C1_VIEW_DOOR_ORNAMENT_D2LCR, and "
    "G0183_s_Graphic558_Frames_Door_D2C at zone M628_ZONE_DOOR_D2C, after "
    "F0108 floor ornament, F0115 order 0x0218, F0100/F0104 top/left/right "
    "door frames at C730/C724/C725, optional F0110 door button at "
    "C2_VIEW_DOOR_BUTTON_D2C, and before the F0115 order 0x0349 front "
    "pass. DUNVIEW.C:8508-8533 F0128_DUNGEONVIEW_Draw_CPSF dispatches "
    "D2L2/D2R2 at 8504/8508 (MEDIA720 guard), D2L/D2R at 8513/8517, D2C "
    "F0121 at 8521, then D1L/D1R/D1C at 8525/8529/8533 and D0L/D0R/D0C "
    "follow; D0L/D0R-style F0100/F0105/F0107 fallback is NOT used for the "
    "D2C center. DUNVIEW.C:6837-6865 F0678_DrawD2L2 and 6868-6896 "
    "F0679_DrawD2R2 are cited as the D2-side wall anchors, and they "
    "return wall cases before any F0111 door-front route, making this "
    "D2C F0111 partly-open gate non-duplicative with the existing D2L2/"
    "D2R2 partly-open/front-clipped/wall gates and the existing D1L2/"
    "D1R2/D2L2/D2R2 F0111 partly-open gates. DEFS.H:2088 C10_COLOR_FLESH, "
    "2602 M603_VIEW_SQUARE_D2C, 2605-2606 C09/C10 D2L2/D2R2, 3508 "
    "C6_UNKNOWN, 3516 MASK0x4000, 4029-4031 C707/C708/C709 D2C/D2L/D2R "
    "wall zones (and 4047-4048 C707/C708 D2L2/D2R2), 4250-4256/4259-4262 "
    "M628 D2C door zone and D2L2/D2R2 door zone bases, 4228-4230 C2500_/"
    "C2900_ thing/projectile bases outside this F0111 gate. CSB-lineage "
    "Viewport.cpp:1903-1915 is the requested center-door dispatch anchor "
    "and binds StdDrawF2DoorFacing locally at 1865-1879. Requested "
    "C2600_DOOR_PARTLY_OPEN_BITMAP is absent from available ReDMCSB "
    "Common/Source; F0111:4311-4313 is the cited D2C partly-open "
    "bitmap-selection source anchor.";

static const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 s_specs[] = {
    {
        /* identity */
        CSB_PRESENT,                                  /* source_locked_contract_only */
        CSB_PRESENT,                                  /* no_real_asset_pixel_parity */
        CSB_PRESENT,                                  /* no_game_data_load */
        /* F0128 D2C dispatch */
        CSB_VIEW_SQUARE_D2C,                          /* view_square_d2c */
        CSB_F0128_D2C_ORDER,                          /* f0128_dispatch_order */
        CSB_F0128_D2_DEPTH,                           /* f0128_relative_depth */
        CSB_D2C_LANE,                                 /* f0128_relative_lane */
        /* F0121 D2C body */
        CSB_F0121_FUNCTION,                           /* f0121_function_number */
        /* D2-side wall zones */
        CSB_C709_ZONE_WALL_D2C,                       /* wall_zone_d2c_binding */
        CSB_C707_ZONE_WALL_D2L2,                      /* wall_zone_d2l2_binding */
        CSB_C708_ZONE_WALL_D2R2,                      /* wall_zone_d2r2_binding */
        /* D2L2/D2R2 F0111 direct route is NOT used by D2C center */
        CSB_ABSENT,                                   /* f0678_f0679_d2l2_d2r2_direct_f0111_route_present */
        CSB_PRESENT,                                  /* wall_case_returns_before_f0111 */
        /* non-duplication markers */
        CSB_PRESENT,                                  /* excludes_existing_front_clipped_gate */
        CSB_PRESENT,                                  /* excludes_existing_d2l2_d2r2_partly_open_gate */
        CSB_PRESENT,                                  /* excludes_existing_d2l2_d2r2_wall_gate */
        CSB_PRESENT,                                  /* excludes_existing_d1l2_d1r2_partly_open_gate */
        CSB_PRESENT,                                  /* excludes_existing_closed_d2c_gate */
        CSB_PRESENT,                                  /* excludes_d0l_d0r_f0100_f0105_f0107_fallback */
        /* door zone bindings */
        CSB_M628_ZONE_DOOR_D2C,                       /* door_zone_d2c */
        CSB_C3700_ZONE_DOOR_D2L2,                     /* door_zone_d2l2 */
        CSB_C3710_ZONE_DOOR_D2R2,                     /* door_zone_d2r2 */
        /* door native dimensions */
        CSB_D2C_DOOR_WIDTH,                           /* door_native_width */
        CSB_D2C_DOOR_HEIGHT,                          /* door_native_height */
        CSB_D2C_DOOR_BYTE_COUNT,                      /* door_native_byte_count */
        /* door states */
        CSB_DOOR_STATE_OPEN,                          /* open_state */
        CSB_DOOR_STATE_PARTLY_ONE,                    /* partly_open_state_one */
        CSB_DOOR_STATE_PARTLY_TWO,                    /* partly_open_state_two */
        CSB_DOOR_STATE_PARTLY_THREE,                  /* partly_open_state_three */
        CSB_DOOR_STATE_CLOSED,                        /* closed_state */
        CSB_DOOR_STATE_DESTROYED,                     /* destroyed_state */
        CSB_PRESENT,                                  /* decrements_state_before_frame_select */
        /* first half zone math */
        CSB_C6_UNKNOWN,                               /* first_half_zone_offset */
        CSB_PRESENT,                                  /* first_half_uses_f0635_zone_clip */
        CSB_PRESENT,                                  /* first_half_uses_f0654_blit */
        CSB_PRESENT,                                  /* first_half_zone_shift_x_is_half_bitmap_width */
        CSB_C10_COLOR_FLESH,                          /* first_half_transparent_color */
        /* second half zone math */
        CSB_SECOND_HALF_OFFSET,                       /* second_half_zone_offset */
        CSB_MASK0X4000,                               /* second_half_zone_mask */
        CSB_PRESENT,                                  /* second_half_uses_f0791_drawbitmapxx */
        CSB_C10_COLOR_FLESH,                          /* second_half_transparent_color */
        /* c2600 marker */
        CSB_ABSENT,                                   /* c2600_literal_symbol_present */
        /* F0121 order constants */
        CSB_DOORPASS1_ORDER,                          /* doorpass1_order */
        CSB_DOORPASS2_ORDER,                          /* doorpass2_order */
        /* route name + anchors */
        "D2C F0111 partly-open horizontal door via F0121 D2C body",
        "G0183_s_Graphic558_Frames_Door_D2C.LeftHorizontal[state-1]",
        "G0183_s_Graphic558_Frames_Door_D2C.RightHorizontal[state-1]",
        "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; "
            "partly-open D2C horizontal 4311-4334",
        "ReDMCSB DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C; "
            "C17_ELEMENT_DOOR_FRONT branch 7313-7341",
        "ReDMCSB DUNVIEW.C:8508-8533 F0128_DUNGEONVIEW_Draw_CPSF; "
            "D2C F0121 dispatch at line 8521",
        "ReDMCSB DUNVIEW.C:6837-6896 F0678_DrawD2L2 and F0679_DrawD2R2 "
            "wall anchors; D2C F0111 is NOT reached through F0678/F0679",
        "ReDMCSB DEFS.H:2088,2602,2605-2606,3508,3516,4029-4031,4047-4049,"
            "4228-4230,4250-4256",
        "CSB-lineage Viewport.cpp:1903-1915 requested; local "
            "StdDrawF2DoorFacing lines 1865-1879",
        "C2600_DOOR_PARTLY_OPEN_BITMAP absent; use ReDMCSB "
            "DUNVIEW.C:4311-4313"
    }
};

size_t csb_v1_viewport_d2c_f0111_partly_open_door_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d2c_f0111_partly_open_door_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d2c_f0111_partly_open_door_spec_count_pc34()) {
        return 0;
    }
    return &s_specs[index];
}

const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(int view_square)
{
    for (size_t i = 0;
         i < csb_v1_viewport_d2c_f0111_partly_open_door_spec_count_pc34();
         ++i) {
        if (s_specs[i].view_square_d2c == view_square) return &s_specs[i];
    }
    return 0;
}

int csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec,
    int door_state)
{
    if (!spec) return CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_INVALID_PC34;
    if (door_state == spec->open_state) {
        return CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_OPEN_PC34;
    }
    if (door_state == spec->closed_state) {
        return CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_CLOSED_PC34;
    }
    if (door_state == spec->destroyed_state) {
        return CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_DESTROYED_PC34;
    }
    if (door_state >= spec->partly_open_state_one &&
        door_state <= spec->partly_open_state_three) {
        return CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_PARTLY_OPEN_PC34;
    }
    return CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_INVALID_PC34;
}

int csb_v1_viewport_d2c_f0111_partly_open_door_first_half_zone_pc34(
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    if (!spec) return -1;
    if (!horizontal_door) return -1;
    if (csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return -1;
    }
    /* ReDMCSB: DUNVIEW.C F0111 lines 4317-4324
     *   P2084_i_ZoneIndex += state; then zone + C6_UNKNOWN
     *   is the first-half F0635/F0654 blit zone for horizontal doors. */
    return spec->door_zone_d2c + door_state + spec->first_half_zone_offset;
}

int csb_v1_viewport_d2c_f0111_partly_open_door_second_half_zone_pc34(
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    const int branch =
        csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(
            spec, door_state);

    if (!spec) return -1;
    if (branch == CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_OPEN_PC34 ||
        branch == CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_INVALID_PC34) {
        return -1;
    }
    if (branch == CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_CLOSED_PC34 ||
        branch == CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_DESTROYED_PC34) {
        /* Closed / destroyed use the base D2C door zone directly without
         * the second-half shift. */
        return spec->door_zone_d2c;
    }
    if (!horizontal_door) {
        return spec->door_zone_d2c + door_state;
    }
    /* ReDMCSB: DUNVIEW.C F0111 lines 4325-4326
     *   P2084_i_ZoneIndex += 3 | MASK0x4000
     *   is the second-half horizontal partly-open zone. */
    return spec->door_zone_d2c + door_state +
           (spec->second_half_zone_offset | spec->second_half_zone_mask);
}

const char *
csb_v1_viewport_d2c_f0111_partly_open_door_frame_bitmap_pc34(
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int right_half)
{
    if (csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return 0;
    }
    /* ReDMCSB: DUNVIEW.C F0111 lines 4311-4313 decrements state and then
     * selects D2C.LeftHorizontal[state-1] and D2C.RightHorizontal[state-1]
     * from G0183_s_Graphic558_Frames_Door_D2C. */
    return right_half ? spec->right_horizontal_frame_bitmap :
                        spec->left_horizontal_frame_bitmap;
}

int csb_v1_viewport_d2c_f0111_partly_open_door_synthetic_blit_pc34(
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    const uint8_t *source,
    int source_width,
    int source_height,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int destination_stride,
    int *out_c10_skipped)
{
    int copied = 0;
    int skipped = 0;

    if (!spec || !source || !destination) return -1;
    if (source_width <= 0 || source_height <= 0) return -1;
    if (source_stride < source_width || destination_stride < destination_width) {
        return -1;
    }
    if (destination_width < source_width || destination_height < source_height) {
        return -1;
    }
    if (csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return 0;
    }

    /* ReDMCSB: DUNVIEW.C F0111 line 4322-4334 keep C10_COLOR_FLESH
     * transparent when drawing the D2C door bitmap; this helper is
     * synthetic and only proves the C10 skip contract. */
    for (int y = 0; y < source_height; ++y) {
        for (int x = 0; x < source_width; ++x) {
            const uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)spec->second_half_transparent_color) {
                ++skipped;
                continue;
            }
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    if (out_c10_skipped) *out_c10_skipped = skipped;
    return copied;
}

int csb_v1_viewport_d2c_f0111_partly_open_door_probe_pc34_compat(
    CSB_V1_D2CF0111PartlyOpenDoorProbePc34 *out_probe)
{
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(
            CSB_VIEW_SQUARE_D2C);
    const uint8_t source[6] = { 10, 1, 2, 10, 3, 4 };
    uint8_t destination[6] = { 0, 0, 0, 0, 0, 0 };
    int skipped = 0;
    int copied;

    if (!out_probe || !spec) return -1;
    copied = csb_v1_viewport_d2c_f0111_partly_open_door_synthetic_blit_pc34(
        spec, CSB_DOOR_STATE_PARTLY_TWO, source, 3, 2, 3, destination, 3, 2, 3,
        &skipped);

    out_probe->route_count =
        (int)csb_v1_viewport_d2c_f0111_partly_open_door_spec_count_pc34();
    out_probe->assertions = 0;
    out_probe->failures = 0;
    out_probe->dispatch_order_ok =
        spec->f0128_dispatch_order == CSB_F0128_D2C_ORDER &&
        spec->f0128_relative_depth == CSB_F0128_D2_DEPTH &&
        spec->f0128_relative_lane == CSB_D2C_LANE;
    out_probe->branch_state_ok =
        csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(
            spec, CSB_DOOR_STATE_PARTLY_TWO) ==
        CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_PARTLY_OPEN_PC34;
    out_probe->frame_selection_ok =
        csb_v1_viewport_d2c_f0111_partly_open_door_frame_bitmap_pc34(
            spec, CSB_DOOR_STATE_PARTLY_TWO, 0) != 0 &&
        csb_v1_viewport_d2c_f0111_partly_open_door_frame_bitmap_pc34(
            spec, CSB_DOOR_STATE_PARTLY_TWO, 1) != 0;
    out_probe->first_half_zone =
        csb_v1_viewport_d2c_f0111_partly_open_door_first_half_zone_pc34(
            spec, CSB_DOOR_STATE_PARTLY_TWO, CSB_PRESENT);
    out_probe->second_half_zone =
        csb_v1_viewport_d2c_f0111_partly_open_door_second_half_zone_pc34(
            spec, CSB_DOOR_STATE_PARTLY_TWO, CSB_PRESENT);
    out_probe->horizontal_mask_ok =
        spec->second_half_zone_mask == CSB_MASK0X4000 &&
        spec->second_half_zone_offset == CSB_SECOND_HALF_OFFSET;
    out_probe->c10_transparency_ok =
        spec->second_half_transparent_color == CSB_C10_COLOR_FLESH &&
        spec->first_half_transparent_color == CSB_C10_COLOR_FLESH;
    out_probe->copied_pixels = copied;
    out_probe->c10_skipped_pixels = skipped;
    out_probe->no_real_asset_pixel_parity = CSB_PRESENT;

    if (copied != 4 || skipped != 2) out_probe->failures = 1;
    return out_probe->failures ? -1 : 0;
}

const char *
csb_v1_viewport_d2c_f0111_partly_open_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
