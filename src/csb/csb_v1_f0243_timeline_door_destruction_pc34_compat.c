/* ReDMCSB TIMELINE.C F0243:872-883. */
#include "csb_v1_f0243_timeline_door_destruction_pc34_compat.h"

void F0243_TIMELINE_ProcessEvent2_DoorDestruction(uint8_t *door_square)
{
    *door_square = (uint8_t)((*door_square & (uint8_t)~0x07u) | 0x05u);
}
