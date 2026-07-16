#include "dm1_v1_f0249_timeline_relocation_pc34_compat.h"

int DM1_V1_F0249_RelocateTimelineForMovedThingPc34Compat(
    struct TimelineEvent_Compat *events,
    int eventCount,
    int thingType,
    int thingIndex,
    int destinationMapIndex,
    int destinationMapX,
    int destinationMapY,
    int destinationCell)
{
    int i;
    int relocated = 0;

    if (!events || eventCount < 0 || thingIndex < 0 ||
        destinationMapIndex < 0 || destinationCell < 0 ||
        destinationCell > 3) {
        return -1;
    }

    for (i = 0; i < eventCount; ++i) {
        struct TimelineEvent_Compat *event = &events[i];
        int matches = 0;

        /* ReDMCSB TIMELINE.C F0249:1420-1432 reads the moved C14
         * PROJECTILE's EventIndex and rewrites that C48/C49 record. */
        if (thingType == DM1_F0249_THING_PROJECTILE_PC34) {
            matches = event->kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
                      event->aux0 == thingIndex;
        /* F0249:1434-1452 scans only C25 explosion events whose Slot is
         * the moved C15 thing. C24 REMOVE_FLUXCAGE is intentionally not
         * repaired here, matching the documented original PC behavior. */
        } else if (thingType == DM1_F0249_THING_EXPLOSION_PC34) {
            matches = event->kind == TIMELINE_EVENT_EXPLOSION_ADVANCE &&
                      event->aux0 == thingIndex;
        } else {
            return 0;
        }

        if (!matches) continue;
        event->mapIndex = destinationMapIndex;
        event->mapX = destinationMapX;
        event->mapY = destinationMapY;
        event->cell = destinationCell;
        ++relocated;
    }
    return relocated;
}

const char *DM1_V1_F0249_TimelineRelocationSourceEvidencePc34Compat(void)
{
    return "ReDMCSB TIMELINE.C F0249:1420-1452: moved PROJECTILE EventIndex "
           "updates C48/C49; moved EXPLOSION updates C25 only; C24 "
           "REMOVE_FLUXCAGE remains unchanged.";
}
