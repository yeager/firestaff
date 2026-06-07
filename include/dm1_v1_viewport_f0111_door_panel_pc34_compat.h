/*
 * ReDMCSB evidence:
 * - DUNVIEW.C:4218-4339 F0111_DUNGEONVIEW_DrawDoor: open-door guard,
 *   temporary door copy, ornament/mask composition, partly-open zone math,
 *   and final F0791 C10-transparent viewport blit.
 * - DEFS.H:1039-1044 C0..C5 door states; DEFS.H:2088 C10_COLOR_FLESH;
 *   DEFS.H:3516 MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR;
 *   DEFS.H:4250-4260 MEDIA720 door zones.
 */
#ifndef FIRESTAFF_DM1_V1_VIEWPORT_F0111_DOOR_PANEL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_F0111_DOOR_PANEL_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_F0111_DOOR_PANEL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_F0111_DOOR_PANEL_MASK0X4000_PC34 0x4000
#define DM1_V1_F0111_DOOR_PANEL_D1C_ZONE_PC34 3790

typedef enum {
    DM1_V1_F0111_DOOR_STATE_OPEN_PC34 = 0,
    DM1_V1_F0111_DOOR_STATE_CLOSED_ONE_FOURTH_PC34 = 1,
    DM1_V1_F0111_DOOR_STATE_CLOSED_HALF_PC34 = 2,
    DM1_V1_F0111_DOOR_STATE_CLOSED_THREE_FOURTH_PC34 = 3,
    DM1_V1_F0111_DOOR_STATE_CLOSED_PC34 = 4,
    DM1_V1_F0111_DOOR_STATE_DESTROYED_PC34 = 5
} DM1_V1_F0111DoorStatePc34;

typedef enum {
    DM1_V1_F0111_DOOR_BRANCH_OPEN_NO_DRAW_PC34 = 0,
    DM1_V1_F0111_DOOR_BRANCH_CLOSED_PC34 = 1,
    DM1_V1_F0111_DOOR_BRANCH_DESTROYED_PC34 = 2,
    DM1_V1_F0111_DOOR_BRANCH_PARTLY_VERTICAL_PC34 = 3,
    DM1_V1_F0111_DOOR_BRANCH_PARTLY_HORIZONTAL_PC34 = 4
} DM1_V1_F0111DoorBranchPc34;

typedef struct {
    int door_state;
    bool vertical;
    bool animated;
    int random_flip;
    int zone_index;
    int temporary_bitmap_width;
    int thieves_eye_event_count;
} DM1_V1_F0111DoorPanelInputPc34;

typedef struct {
    bool source_locked_contract_only;
    bool no_real_asset_pixel_parity;
    int open_state;
    int closed_state;
    int destroyed_state;
    int transparent_color;
    int d1c_zone_index;
    int half_zone_shift_mask;
    const char *source_lines;
    const char *non_overlap_note;
} DM1_V1_F0111DoorPanelSpecPc34;

typedef struct {
    DM1_V1_F0111DoorPanelSpecPc34 spec;
    DM1_V1_F0111DoorBranchPc34 branch;
    bool valid;
    bool copied_native_panel_to_temporary;
    bool drew_base_ornament_to_temporary;
    bool applied_animated_flip;
    int flip_flags;
    bool applied_thieves_eye_mask;
    bool applied_destroyed_mask;
    bool drew_closed_or_destroyed_frame;
    bool drew_partly_open_frame;
    bool drew_horizontal_front_half;
    bool final_viewport_blit;
    int front_half_zone_index;
    int final_zone_index;
    int zone_shift_x;
    int zone_shift_y;
} DM1_V1_F0111DoorPanelTracePc34;

const DM1_V1_F0111DoorPanelSpecPc34 *
dm1_v1_viewport_f0111_door_panel_spec_pc34(void);

bool dm1_v1_viewport_f0111_door_panel_resolve_pc34(
    const DM1_V1_F0111DoorPanelInputPc34 *input,
    DM1_V1_F0111DoorPanelTracePc34 *out);

uint8_t dm1_v1_viewport_f0111_door_panel_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_f0111_door_panel_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_F0111_DOOR_PANEL_PC34_COMPAT_H */
