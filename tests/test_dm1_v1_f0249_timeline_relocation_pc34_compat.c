#include "dm1_v1_f0249_timeline_relocation_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    struct TimelineEvent_Compat events[4];

    memset(events, 0, sizeof(events));
    events[0].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    events[0].aux0 = 3;
    events[1].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    events[1].aux0 = 4;
    events[2].kind = TIMELINE_EVENT_EXPLOSION_ADVANCE;
    events[2].aux0 = 7;
    events[3].kind = TIMELINE_EVENT_REMOVE_FLUXCAGE;
    events[3].aux0 = 7;
    events[3].mapX = 1;
    events[3].mapY = 2;

    assert(DM1_V1_F0249_RelocateTimelineForMovedThingPc34Compat(
               events, 4, DM1_F0249_THING_PROJECTILE_PC34, 3,
               2, 9, 10, 1) == 1);
    assert(events[0].mapIndex == 2 && events[0].mapX == 9 &&
           events[0].mapY == 10 && events[0].cell == 1);
    assert(events[1].mapIndex == 0 && events[1].mapX == 0);

    assert(DM1_V1_F0249_RelocateTimelineForMovedThingPc34Compat(
               events, 4, DM1_F0249_THING_EXPLOSION_PC34, 7,
               1, 4, 5, 2) == 1);
    assert(events[2].mapIndex == 1 && events[2].mapX == 4 &&
           events[2].mapY == 5 && events[2].cell == 2);
    assert(events[3].mapX == 1 && events[3].mapY == 2);
    assert(DM1_V1_F0249_RelocateTimelineForMovedThingPc34Compat(
               events, 4, DM1_F0249_THING_PROJECTILE_PC34, -1,
               0, 0, 0, 0) == -1);
    return 0;
}
