#include "dm1/dm1_v1_viewport_d1c_f0111_door_pc34_compat.h"

#include <string.h>

enum {
    DM1_PRESENT = 1,
    DM1_VIEW_SQUARE_D1C = 3,      /* ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C. */
    DM1_ELEMENT_DOOR_FRONT = 17,  /* ReDMCSB DUNVIEW.C:7873 C17_ELEMENT_DOOR_FRONT. */
    DM1_F0128_D1C_ORDER = 13,     /* ReDMCSB DUNVIEW.C:8533 after D1L/D1R. */
    DM1_F0124_FUNCTION = 124,     /* ReDMCSB DUNVIEW.C:7727 F0124. */
    DM1_WALL_ZONE_D1C = 712,      /* ReDMCSB DEFS.H:4052 C712_ZONE_WALL_D1C. */
    DM1_DOOR_ZONE_D1C = 3790,     /* ReDMCSB DEFS.H:4259 M631_ZONE_DOOR_D1C. */
    DM1_FRAME_TOP_D1C = 733,      /* ReDMCSB DEFS.H:4094 C733_ZONE_DOOR_FRAME_TOP_D1C. */
    DM1_FRAME_LEFT_D1C = 726,     /* ReDMCSB DEFS.H:4087 C726_ZONE_DOOR_FRAME_LEFT_D1C. */
    DM1_FRAME_RIGHT_D1C = 727,    /* ReDMCSB DEFS.H:4088 C727_ZONE_DOOR_FRAME_RIGHT_D1C. */
    DM1_FLOOR_VIEW_D1C = 9,       /* ReDMCSB DEFS.H:2759 M595_VIEW_FLOOR_D1C. */
    DM1_D1C_DOOR_W = 96,          /* ReDMCSB DUNVIEW.C:7905 M075_BITMAP_BYTE_COUNT(96,88). */
    DM1_D1C_DOOR_H = 88,
    DM1_D1C_DOOR_BYTES = 4224,
    DM1_D1C_ORNAMENT_VIEW = 2,    /* ReDMCSB DEFS.H:2791 C2_VIEW_DOOR_ORNAMENT_D1LCR. */
    DM1_DOORPASS1_ORDER = 0x0218,
    DM1_DOORPASS2_ORDER = 0x0349,
    DM1_DOOR_STATE_OPEN = 0,
    DM1_DOOR_STATE_CLOSED_ONE_FOURTH = 1,
    DM1_DOOR_STATE_CLOSED_HALF = 2,
    DM1_DOOR_STATE_CLOSED_THREE_FOURTH = 3,
    DM1_DOOR_STATE_CLOSED = 4,
    DM1_DOOR_STATE_DESTROYED = 5,
    DM1_FIRST_DOOR_SET_GRAPHIC = 246, /* ReDMCSB DEFS.H:2375 PC34 M633. */
    DM1_DOOR_SET_GRAPHIC_COUNT = 3,
    DM1_D1C_DOOR_BITMAP_OFFSET = 2
};

static const char s_source_evidence[] =
    "DM1 V1 D1C F0111 door transparency source-lock; contract-only, "
    "asset-free, no game-data load, and no original DOS pixel parity claim. "
    "ReDMCSB DUNVIEW.C F0111:4218-4337 anchors the partly-open horizontal "
    "path used by D1C: open state C0 skips drawing, closed state C4 draws "
    "ClosedOrDestroyed, partly-open states C1..C3 decrement at 4308, select "
    "G0186_s_Graphic558_Frames_Door_D1C.LeftHorizontal[state-1] and "
    "RightHorizontal[state-1] at 4311-4313, perform the first horizontal "
    "C10_COLOR_FLESH transparent half-blit through C6_UNKNOWN at 4317-4324, "
    "then apply 3|MASK0x4000 before the second C10 blit at 4325-4334. "
    "DUNVIEW.C F0124:7727-7937 is the actual D1C body in this source; its "
    "C17_ELEMENT_DOOR_FRONT branch at 7873-7911 draws F0108 floor ornament, "
    "rear F0115 order 0x0218, D1C top/left/right door frames, optional D1C "
    "door button, F0111 with G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, "
    "C2_VIEW_DOOR_ORNAMENT_D1LCR, M631_ZONE_DOOR_D1C, and finally front "
    "F0115 order 0x0349. DUNVIEW.C F0128:8318-8542 dispatches D1L, D1R, "
    "then D1C at line 8533. DUNVIEW.C F0118:6642-6763 is D3C and can only "
    "name F0124 through the copy-protection teleporter branch around "
    "6781-6784, not the normal D1C door route. DUNVIEW.C F0121:7244-7389 "
    "is the D2C sibling door-front body and is recorded here only to keep "
    "this D1C gate non-duplicative with D2C/D2L2/D2R2 work. DEFS.H:1039-1044 "
    "defines C0..C5 door states, DEFS.H:2088 defines C10_COLOR_FLESH, "
    "DEFS.H:2599 defines M606_VIEW_SQUARE_D1C, DEFS.H:2759/2791/4052/"
    "4087-4094/4259 define D1C floor/ornament/wall/frame/door zones, and "
    "DEFS.H:2375/2431 plus DUNVIEW.C:2651-2658 define the G0695 D1C native "
    "bitmap index progression through per-door-set ordinals.";

static const DM1_V1_D1CF0111DoorSpecPc34 s_specs[] = {
    {
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_VIEW_SQUARE_D1C,
        DM1_ELEMENT_DOOR_FRONT,
        DM1_F0128_D1C_ORDER,
        1,
        0,
        DM1_F0124_FUNCTION,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_WALL_ZONE_D1C,
        DM1_DOOR_ZONE_D1C,
        DM1_FRAME_TOP_D1C,
        DM1_FRAME_LEFT_D1C,
        DM1_FRAME_RIGHT_D1C,
        DM1_FLOOR_VIEW_D1C,
        DM1_D1C_DOOR_W,
        DM1_D1C_DOOR_H,
        DM1_D1C_DOOR_BYTES,
        DM1_D1C_ORNAMENT_VIEW,
        DM1_DOORPASS1_ORDER,
        DM1_DOORPASS2_ORDER,
        DM1_DOOR_STATE_OPEN,
        DM1_DOOR_STATE_CLOSED_ONE_FOURTH,
        DM1_DOOR_STATE_CLOSED_HALF,
        DM1_DOOR_STATE_CLOSED_THREE_FOURTH,
        DM1_DOOR_STATE_CLOSED,
        DM1_DOOR_STATE_DESTROYED,
        DM1_FIRST_DOOR_SET_GRAPHIC,
        DM1_DOOR_SET_GRAPHIC_COUNT,
        DM1_D1C_DOOR_BITMAP_OFFSET,
        "D1C front-wall F0111 partly-open horizontal door transparency route",
        "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor",
        "ReDMCSB DUNVIEW.C:7727-7937 F0124_DUNGEONVIEW_DrawSquareD1C; "
            "C17 branch 7873-7911",
        "ReDMCSB DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF; "
            "D1C dispatch at 8533",
        "ReDMCSB DUNVIEW.C:6642-6763 F0118 D3C; copy-protection can name "
            "F0124 near 6781-6784",
        "ReDMCSB DUNVIEW.C:7244-7389 F0121 is D2C sibling, not D1C",
        "ReDMCSB DEFS.H:1039-1044,2088,2375,2431,2599,2759,2791,4052,"
            "4087-4094,4259",
        "ReDMCSB DUNVIEW.C:2651-2658 G0693/G0694/G0695 door bitmap index "
            "progression",
        "CSB-lineage Viewport.cpp:1903-1915 StdDrawF1DoorFacing"
    }
};

static const DM1_V1_D1CF0111DoorFramePc34 s_closed = {
    64, 159, 17, 102, 48, 88, 0, 0
};

static const DM1_V1_D1CF0111DoorFramePc34 s_left_horizontal[3] = {
    { 64, 75, 17, 102, 48, 88, 36, 0 },
    { 64, 87, 17, 102, 48, 88, 24, 0 },
    { 64, 99, 17, 102, 48, 88, 12, 0 }
};

static const DM1_V1_D1CF0111DoorFramePc34 s_right_horizontal[3] = {
    { 148, 159, 17, 102, 48, 88, 48, 0 },
    { 136, 159, 17, 102, 48, 88, 48, 0 },
    { 124, 159, 17, 102, 48, 88, 48, 0 }
};

size_t dm1_v1_viewport_d1c_f0111_door_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D1CF0111DoorSpecPc34 *
dm1_v1_viewport_d1c_f0111_door_spec_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d1c_f0111_door_spec_count_pc34()) return 0;
    return &s_specs[index];
}

const DM1_V1_D1CF0111DoorSpecPc34 *
dm1_v1_viewport_d1c_f0111_door_spec_for_square_pc34(int view_square)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d1c_f0111_door_spec_count_pc34(); ++i) {
        if (s_specs[i].view_square_d1c == view_square) return &s_specs[i];
    }
    return 0;
}

DM1_V1_D1CF0111DoorBranchPc34
dm1_v1_viewport_d1c_f0111_door_branch_pc34(
    const DM1_V1_D1CF0111DoorSpecPc34 *spec,
    int door_state)
{
    if (!spec) return DM1_V1_D1C_F0111_DOOR_BRANCH_INVALID_PC34;
    if (door_state == spec->door_state_open) {
        return DM1_V1_D1C_F0111_DOOR_BRANCH_OPEN_PC34;
    }
    if (door_state >= spec->door_state_closed_one_fourth &&
        door_state <= spec->door_state_closed_three_fourth) {
        return DM1_V1_D1C_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34;
    }
    if (door_state == spec->door_state_closed) {
        return DM1_V1_D1C_F0111_DOOR_BRANCH_CLOSED_PC34;
    }
    if (door_state == spec->door_state_destroyed) {
        return DM1_V1_D1C_F0111_DOOR_BRANCH_DESTROYED_PC34;
    }
    return DM1_V1_D1C_F0111_DOOR_BRANCH_INVALID_PC34;
}

int dm1_v1_viewport_d1c_f0111_door_native_bitmap_index_pc34(
    const DM1_V1_D1CF0111DoorSpecPc34 *spec,
    int door_set0,
    int door_set1,
    int door_type)
{
    int door_set;

    if (!spec || door_set0 < 0 || door_set1 < 0) return -1;
    if (door_type == 0) {
        door_set = door_set0;
    } else if (door_type == 1) {
        door_set = door_set1;
    } else {
        return -1;
    }
    return spec->first_door_set_graphic +
           (door_set * spec->door_set_graphic_count) +
           spec->d1c_door_bitmap_offset;
}

const char *dm1_v1_viewport_d1c_f0111_door_frame_name_pc34(
    const DM1_V1_D1CF0111DoorSpecPc34 *spec,
    int door_state,
    int right_half)
{
    if (dm1_v1_viewport_d1c_f0111_door_branch_pc34(spec, door_state) !=
        DM1_V1_D1C_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return 0;
    }
    return right_half ?
        "G0186_s_Graphic558_Frames_Door_D1C.RightHorizontal[state-1]" :
        "G0186_s_Graphic558_Frames_Door_D1C.LeftHorizontal[state-1]";
}

int dm1_v1_viewport_d1c_f0111_door_state_trace_pc34(
    const DM1_V1_D1CF0111DoorSpecPc34 *spec,
    int door_state,
    int horizontal_door,
    int door_set0,
    int door_set1,
    int door_type,
    DM1_V1_D1CF0111DoorStateTracePc34 *out)
{
    DM1_V1_D1CF0111DoorBranchPc34 branch;
    int decremented;

    if (!spec || !out) return 0;
    memset(out, 0, sizeof(*out));
    branch = dm1_v1_viewport_d1c_f0111_door_branch_pc34(spec, door_state);
    out->input_state = door_state;
    out->branch = (int)branch;
    out->decremented_state = -1;
    out->first_half_zone = spec->door_zone_d1c;
    out->first_half_clip_zone = -1;
    out->second_half_zone = spec->door_zone_d1c;
    out->first_half_transparent_color = DM1_V1_D1C_F0111_C10_COLOR_FLESH_PC34;
    out->second_half_transparent_color = DM1_V1_D1C_F0111_C10_COLOR_FLESH_PC34;
    out->native_bitmap_index =
        dm1_v1_viewport_d1c_f0111_door_native_bitmap_index_pc34(
            spec, door_set0, door_set1, door_type);
    if (out->native_bitmap_index < 0) return 0;

    if (branch == DM1_V1_D1C_F0111_DOOR_BRANCH_OPEN_PC34) {
        return 1;
    }
    if (branch == DM1_V1_D1C_F0111_DOOR_BRANCH_CLOSED_PC34 ||
        branch == DM1_V1_D1C_F0111_DOOR_BRANCH_DESTROYED_PC34) {
        out->draws_any_bitmap = 1;
        out->closed_frame_selected = 1;
        out->closed_or_destroyed = s_closed;
        out->selected_frame_state_index = door_state;
        return 1;
    }
    if (branch != DM1_V1_D1C_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) return 0;

    decremented = door_state - 1;
    out->draws_any_bitmap = 1;
    out->decremented_state = decremented;
    out->selected_frame_state_index = decremented;
    out->first_half_zone = spec->door_zone_d1c + decremented;
    out->second_half_zone = out->first_half_zone;
    if (!horizontal_door) return 1;

    out->left_horizontal_selected = 1;
    out->right_horizontal_selected = 1;
    out->horizontal_mask_applied = 1;
    out->first_half_clip_zone =
        out->first_half_zone + DM1_V1_D1C_F0111_C6_UNKNOWN_PC34;
    out->second_half_zone =
        out->first_half_zone + 3 + DM1_V1_D1C_F0111_MASK0X4000_PC34;
    out->selected_left_horizontal = s_left_horizontal[decremented];
    out->selected_right_horizontal = s_right_horizontal[decremented];
    return 1;
}

int dm1_v1_viewport_d1c_f0111_door_synthetic_blit_pc34(
    const uint8_t *source,
    uint8_t *destination,
    size_t pixel_count,
    int *out_c10_skipped)
{
    size_t i;
    int copied = 0;
    int skipped = 0;

    if (!source || !destination) return -1;
    for (i = 0; i < pixel_count; ++i) {
        if (source[i] == DM1_V1_D1C_F0111_C10_COLOR_FLESH_PC34) {
            ++skipped;
            continue;
        }
        destination[i] = source[i];
        ++copied;
    }
    if (out_c10_skipped) *out_c10_skipped = skipped;
    return copied;
}

const char *dm1_v1_viewport_d1c_f0111_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
