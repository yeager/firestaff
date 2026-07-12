#ifndef FIRESTAFF_DM1_V1_F0249_TIMELINE_RELOCATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0249_TIMELINE_RELOCATION_PC34_COMPAT_H

#include "memory_timeline_pc34_compat.h"

enum {
    DM1_F0249_THING_PROJECTILE_PC34 = 14,
    DM1_F0249_THING_EXPLOSION_PC34 = 15
};

/* ReDMCSB TIMELINE.C F0249 updates the C48/C49 event belonging to a moved
 * projectile, and every C25 event belonging to a moved explosion. C24
 * fluxcage-removal events are deliberately not changed by the original.
 * Returns the number of relocated events, or -1 for invalid arguments. */
int DM1_V1_F0249_RelocateTimelineForMovedThingPc34Compat(
    struct TimelineEvent_Compat *events,
    int eventCount,
    int thingType,
    int thingIndex,
    int destinationMapIndex,
    int destinationMapX,
    int destinationMapY,
    int destinationCell);

const char *DM1_V1_F0249_TimelineRelocationSourceEvidencePc34Compat(void);

#endif
