#ifndef DM1_V1_F0111_DOOR_ORNAMENT_DISPATCH_PC34_COMPAT_H
#define DM1_V1_F0111_DOOR_ORNAMENT_DISPATCH_PC34_COMPAT_H

/*
 * ReDMCSB DUNVIEW.C F0111_DUNGEONVIEW_DrawDoor (4218-4337) composites a
 * door ornament into each visible temporary door-panel segment before the
 * state-specific door frame is copied to the viewport.  This is deliberately
 * a draw receipt: asset decode and blitting remain host responsibilities.
 */

#include <stddef.h>

#define DM1_V1_F0111_DOOR_ORNAMENT_MAX_PANELS_PC34 2

typedef enum DM1_V1_F0111DoorOrnamentRejectPc34 {
    DM1_V1_F0111_DOOR_ORNAMENT_ACCEPT_PC34 = 0,
    DM1_V1_F0111_DOOR_ORNAMENT_REJECT_NOT_DOOR_PC34,
    DM1_V1_F0111_DOOR_ORNAMENT_REJECT_OPEN_PC34,
    DM1_V1_F0111_DOOR_ORNAMENT_REJECT_NO_ORNAMENT_PC34,
    DM1_V1_F0111_DOOR_ORNAMENT_REJECT_INVALID_DEPTH_PC34,
    DM1_V1_F0111_DOOR_ORNAMENT_REJECT_INVALID_PANELS_PC34
} DM1_V1_F0111DoorOrnamentRejectPc34;

typedef struct DM1_V1_F0111DoorPanelSegmentPc34 {
    int src_y;
    int dst_x;
    int dst_y;
    int width;
    int height;
} DM1_V1_F0111DoorPanelSegmentPc34;

typedef struct DM1_V1_F0111DoorOrnamentInputPc34 {
    int is_door;
    int is_open;
    int ornament_ordinal;
    int depth_index;
    size_t panel_count;
    const DM1_V1_F0111DoorPanelSegmentPc34 *panels;
} DM1_V1_F0111DoorOrnamentInputPc34;

typedef struct DM1_V1_F0111DoorOrnamentDispatchPc34 {
    DM1_V1_F0111DoorOrnamentRejectPc34 rejection;
    int ornament_ordinal;
    int depth_index;
    size_t panel_count;
    DM1_V1_F0111DoorPanelSegmentPc34 panels[DM1_V1_F0111_DOOR_ORNAMENT_MAX_PANELS_PC34];
} DM1_V1_F0111DoorOrnamentDispatchPc34;

/* Builds the F0111 ornament-before-frame dispatch receipt. Returns nonzero
 * only when every source panel is eligible for ornament composition. */
int dm1_v1_f0111_door_ornament_dispatch_pc34(
    const DM1_V1_F0111DoorOrnamentInputPc34 *input,
    DM1_V1_F0111DoorOrnamentDispatchPc34 *out_dispatch);

#endif
