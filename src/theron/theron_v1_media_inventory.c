#include "theron_v1_media_inventory.h"

Theron_V1MediaInventoryReceipt theron_v1_media_inventory(
    const Theron_V1_TrackMediaAvailabilityReceipt *raw,
    const Theron_V1Track19InventoryReceipt *track19) {
    Theron_V1MediaInventoryReceipt receipt = {
        .diagnostic = "raw_track02_missing"
    };

    if (raw && raw->availability == THERON_V1_TRACK_MEDIA_RAW_READY &&
        raw->loader_usable) {
        receipt.raw_track02_usable = 1;
        receipt.startup_eligible = 1;
        /* Raw Track 02 proves media ownership and startup admission only.
         * DMWeb documents the seven-dungeon game, but it does not provide a
         * byte-level Track 02 dungeon grammar here. Keep the three downstream
         * routes closed until a real loader consumer and decoder are proven. */
        receipt.bitmap_route_usable = 0;
        receipt.level_route_usable = 0;
        receipt.object_route_usable = 0;
        receipt.diagnostic = "raw_track02_ready";
    } else if (raw &&
               raw->availability == THERON_V1_TRACK_MEDIA_END_VARIANT) {
        receipt.end_variant_usable = 0;
        receipt.visual_fallback_allowed = 0;
        receipt.diagnostic = "raw_track02_iso_end_variant";
    }

    if (track19 && track19->valid) {
        receipt.track19_usable = 0;
        receipt.track19_metadata_verified =
            track19->item_name_table_verified &&
            track19->level_label_table_verified &&
            track19->item_property_table_verified &&
            track19->opaque_record_window_verified &&
            track19->startup_level_envelope_verified &&
            track19->source_md5[0] != '\0';
    }
    return receipt;
}
