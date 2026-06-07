#include "dm1_v1_viewport_f0111_door_panel_pc34_compat.h"

#include <string.h>

static const char s_non_overlap_note[] =
    "non-overlap: existing DM1 door-front occlusion gates cover square draw "
    "order around F0111; this gate covers only the F0111 door-panel state "
    "machine, temporary composition, partly-open zone math, and C10 blit "
    "contract without claiming real-asset pixel parity.";

static const DM1_V1_F0111DoorPanelSpecPc34 s_spec = {
    true,
    true,
    DM1_V1_F0111_DOOR_STATE_OPEN_PC34,
    DM1_V1_F0111_DOOR_STATE_CLOSED_PC34,
    DM1_V1_F0111_DOOR_STATE_DESTROYED_PC34,
    DM1_V1_F0111_DOOR_PANEL_C10_COLOR_FLESH_PC34,
    DM1_V1_F0111_DOOR_PANEL_D1C_ZONE_PC34,
    DM1_V1_F0111_DOOR_PANEL_MASK0X4000_PC34,
    NULL,
    s_non_overlap_note
};

const DM1_V1_F0111DoorPanelSpecPc34 *
dm1_v1_viewport_f0111_door_panel_spec_pc34(void)
{
    static DM1_V1_F0111DoorPanelSpecPc34 spec;
    spec = s_spec;
    spec.source_lines = dm1_v1_viewport_f0111_door_panel_source_evidence_pc34();
    return &spec;
}

bool dm1_v1_viewport_f0111_door_panel_resolve_pc34(
    const DM1_V1_F0111DoorPanelInputPc34 *input,
    DM1_V1_F0111DoorPanelTracePc34 *out)
{
    int state;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = *dm1_v1_viewport_f0111_door_panel_spec_pc34();
    if (!input) return false;

    state = input->door_state;
    if (state < DM1_V1_F0111_DOOR_STATE_OPEN_PC34 ||
        state > DM1_V1_F0111_DOOR_STATE_DESTROYED_PC34 ||
        input->zone_index < 0 ||
        input->temporary_bitmap_width < 0) {
        return false;
    }

    out->valid = true;
    out->final_zone_index = input->zone_index;
    out->front_half_zone_index = -1;
    out->zone_shift_x = 0;
    out->zone_shift_y = 0;

    if (state == DM1_V1_F0111_DOOR_STATE_OPEN_PC34) {
        out->branch = DM1_V1_F0111_DOOR_BRANCH_OPEN_NO_DRAW_PC34;
        return true;
    }

    out->copied_native_panel_to_temporary = true;
    out->drew_base_ornament_to_temporary = true;
    out->applied_animated_flip = input->animated;
    out->flip_flags = input->animated ? (input->random_flip & 0x03) : 0;
    out->applied_thieves_eye_mask =
        input->zone_index == DM1_V1_F0111_DOOR_PANEL_D1C_ZONE_PC34 &&
        input->thieves_eye_event_count > 0;

    if (state == DM1_V1_F0111_DOOR_STATE_CLOSED_PC34) {
        out->branch = DM1_V1_F0111_DOOR_BRANCH_CLOSED_PC34;
        out->drew_closed_or_destroyed_frame = true;
        out->final_viewport_blit = true;
        return true;
    }

    if (state == DM1_V1_F0111_DOOR_STATE_DESTROYED_PC34) {
        out->branch = DM1_V1_F0111_DOOR_BRANCH_DESTROYED_PC34;
        out->applied_destroyed_mask = true;
        out->drew_closed_or_destroyed_frame = true;
        out->final_viewport_blit = true;
        return true;
    }

    out->drew_partly_open_frame = true;
    out->final_zone_index = input->zone_index + state;
    if (input->vertical) {
        out->branch = DM1_V1_F0111_DOOR_BRANCH_PARTLY_VERTICAL_PC34;
    } else {
        out->branch = DM1_V1_F0111_DOOR_BRANCH_PARTLY_HORIZONTAL_PC34;
        out->drew_horizontal_front_half = true;
        out->front_half_zone_index = input->zone_index + state + 6;
        out->zone_shift_x = input->temporary_bitmap_width >> 1;
        out->final_zone_index += 3 | DM1_V1_F0111_DOOR_PANEL_MASK0X4000_PC34;
    }
    out->final_viewport_blit = true;
    return true;
}

uint8_t dm1_v1_viewport_f0111_door_panel_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

const char *dm1_v1_viewport_f0111_door_panel_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; no real-asset pixel parity is "
        "claimed. ReDMCSB DUNVIEW.C:4218-4339 F0111_DUNGEONVIEW_DrawDoor "
        "guards C0_DOOR_STATE_OPEN at line 4248, copies the native door panel "
        "into G0074_puc_Bitmap_Temporary at line 4260, draws the base ornament "
        "into the temporary bitmap at line 4262, resolves animated flip flags "
        "at lines 4273-4285, applies the D1C Thieves Eye mask at lines "
        "4292-4294, draws the closed frame at lines 4297-4298, applies the "
        "destroyed-door mask at lines 4301-4304, shifts partly-open PC34 zones "
        "at lines 4317-4325, then blits the temporary door bitmap to the "
        "viewport with C10_COLOR_FLESH at line 4334. DEFS.H:1039-1044 defines "
        "C0..C5 door states; DEFS.H:2088 defines C10_COLOR_FLESH; "
        "DEFS.H:3516 defines MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR; "
        "DEFS.H:4250-4260 defines MEDIA720 door zones including "
        "M631_ZONE_DOOR_D1C=3790. non-overlap: existing door-front occlusion "
        "gates cover surrounding F0128 square order; this gate covers the "
        "internal F0111 door-panel state and viewport blit contract.";
}
