#ifndef THERON_V1_MEDIA_INVENTORY_H
#define THERON_V1_MEDIA_INVENTORY_H

#include "theron_v1_track19_inventory.h"
#include "theron_v1_track_media_availability.h"

typedef struct {
    int raw_track02_usable;
    int end_variant_usable;
    int track19_usable;
    /* Track 19 metadata is authenticated independently of runtime use. */
    int track19_metadata_verified;
    int startup_eligible;
    int visual_fallback_allowed;
    int bitmap_route_usable;
    int level_route_usable;
    int object_route_usable;
    const char *diagnostic;
} Theron_V1MediaInventoryReceipt;

Theron_V1MediaInventoryReceipt theron_v1_media_inventory(
    const Theron_V1_TrackMediaAvailabilityReceipt *raw,
    const Theron_V1Track19InventoryReceipt *track19);

#endif
