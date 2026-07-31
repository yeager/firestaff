#include "theron_v1_media_inventory.h"

#include <string.h>

int main(void) {
    Theron_V1_TrackMediaAvailabilityReceipt end_variant = {
        THERON_V1_TRACK_MEDIA_END_VARIANT,
        0
    };
    Theron_V1_TrackMediaAvailabilityReceipt raw = {
        THERON_V1_TRACK_MEDIA_RAW_READY,
        1
    };
    Theron_V1Track19InventoryReceipt track19 = {
        .valid = 1,
        .sector_aligned = 1,
        .container_format_unproven = 1,
        .variant = "us"
    };
    Theron_V1MediaInventoryReceipt a =
        theron_v1_media_inventory(&end_variant, &track19);
    Theron_V1MediaInventoryReceipt b =
        theron_v1_media_inventory(&raw, &track19);

    return !a.startup_eligible &&
        !a.visual_fallback_allowed &&
        !a.bitmap_route_usable &&
        !a.level_route_usable &&
        !a.object_route_usable &&
        a.track19_usable == 0 &&
        strcmp(a.diagnostic, "raw_track02_iso_end_variant") == 0 &&
        b.startup_eligible &&
        !b.bitmap_route_usable &&
        !b.level_route_usable &&
        !b.object_route_usable ? 0 : 1;
}
