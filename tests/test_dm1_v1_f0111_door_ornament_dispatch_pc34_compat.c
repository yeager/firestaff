#include "dm1_v1_f0111_door_ornament_dispatch_pc34_compat.h"

#include <stdio.h>

static int s_failures;

static void expect(int condition, const char *name)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++s_failures;
    }
}

int main(void)
{
    DM1_V1_F0111DoorPanelSegmentPc34 panels[2] = {
        {0, 32, 13, 32, 29},
        {29, 32, 42, 32, 18}
    };
    DM1_V1_F0111DoorOrnamentInputPc34 input = {1, 0, 7, 1, 2, panels};
    DM1_V1_F0111DoorOrnamentDispatchPc34 dispatch;

    expect(dm1_v1_f0111_door_ornament_dispatch_pc34(&input, &dispatch),
           "closed ornamented front door dispatches");
    expect(dispatch.rejection == DM1_V1_F0111_DOOR_ORNAMENT_ACCEPT_PC34,
           "accepted receipt");
    expect(dispatch.ornament_ordinal == 7 && dispatch.depth_index == 1,
           "source ornament and depth retained");
    expect(dispatch.panel_count == 2 && dispatch.panels[1].src_y == 29 &&
               dispatch.panels[1].height == 18,
           "all temporary panel segments replay in source order");

    input.is_open = 1;
    expect(!dm1_v1_f0111_door_ornament_dispatch_pc34(&input, &dispatch) &&
               dispatch.rejection == DM1_V1_F0111_DOOR_ORNAMENT_REJECT_OPEN_PC34,
           "open door does not compose ornament");
    input.is_open = 0;
    input.is_door = 0;
    expect(!dm1_v1_f0111_door_ornament_dispatch_pc34(&input, &dispatch) &&
               dispatch.rejection == DM1_V1_F0111_DOOR_ORNAMENT_REJECT_NOT_DOOR_PC34,
           "non-door caller does not enter F0111 ornament path");
    input.is_door = 1;
    input.ornament_ordinal = 0;
    expect(!dm1_v1_f0111_door_ornament_dispatch_pc34(&input, &dispatch) &&
               dispatch.rejection == DM1_V1_F0111_DOOR_ORNAMENT_REJECT_NO_ORNAMENT_PC34,
           "empty ornament ordinal is a no-draw receipt");
    input.ornament_ordinal = 1;
    input.depth_index = 3;
    expect(!dm1_v1_f0111_door_ornament_dispatch_pc34(&input, &dispatch) &&
               dispatch.rejection == DM1_V1_F0111_DOOR_ORNAMENT_REJECT_INVALID_DEPTH_PC34,
           "D4 is outside F0111 ornament coordinate sets");
    input.depth_index = 0;
    input.panel_count = 0;
    expect(!dm1_v1_f0111_door_ornament_dispatch_pc34(&input, &dispatch) &&
               dispatch.rejection == DM1_V1_F0111_DOOR_ORNAMENT_REJECT_INVALID_PANELS_PC34,
           "missing temporary panel rejects");
    input.panel_count = 1;
    panels[0].height = 0;
    expect(!dm1_v1_f0111_door_ornament_dispatch_pc34(&input, &dispatch) &&
               dispatch.rejection == DM1_V1_F0111_DOOR_ORNAMENT_REJECT_INVALID_PANELS_PC34,
           "zero-height clipped panel rejects");

    if (s_failures) {
        return 1;
    }
    printf("PASS: dm1 F0111 door ornament dispatch (10 checks)\n");
    return 0;
}
