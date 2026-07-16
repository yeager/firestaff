#include "theron_v1_stage2_runtime_handoff.h"
#include "theron_v1_startup_media.h"
#include "theron_v1_track02.h"

int theron_v1_stage2_runtime_handoff_from_original_media(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_V1Stage2RuntimeHandoff *out_handoff) {
    (void)track02_data;
    (void)track02_size;
    (void)md5_hex;
    (void)out_handoff;
    return 0;
}

int theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
    const Theron_StartupMediaStateReceipt *receipt) {
    (void)receipt;
    return 0;
}

Theron_Track02Variant theron_v1_track02_variant_for_md5(const char *md5_hex) {
    (void)md5_hex;
    return THERON_TRACK02_VARIANT_UNKNOWN;
}

Theron_Track02SignalStatus theron_v1_track02_inspect_stage2_dynamic_payload(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2DynamicPayloadReceipt *out_receipt) {
    (void)track02_data;
    (void)track02_size;
    (void)md5_hex;
    (void)out_receipt;
    return THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus theron_v1_track02_catalog_startup_bitmap_samples(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02StartupBitmapCatalog *out_catalog) {
    (void)track02_data;
    (void)track02_size;
    (void)md5_hex;
    (void)out_catalog;
    return THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus theron_v1_track02_build_startup_bitmap_atlas_wide(
    const Theron_Track02StartupBitmapCatalog *catalog,
    Theron_Track02StartupBitmapAtlas *out_atlas) {
    (void)catalog;
    (void)out_atlas;
    return THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus theron_v1_track02_inspect_4bpp_palette_window(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t raw_offset,
    Theron_Track02PaletteWindowEvidence *out_evidence) {
    (void)track02_data;
    (void)track02_size;
    (void)md5_hex;
    (void)raw_offset;
    (void)out_evidence;
    return THERON_TRACK02_SIGNAL_NOT_FOUND;
}

int theron_v1_track02_palette_window_evidence_can_promote(
    const Theron_Track02PaletteWindowEvidence *evidence) {
    (void)evidence;
    return 0;
}

int theron_v1_track02_capture_level_route_receipt(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02LevelRouteReceipt *out_receipt) {
    (void)track02_data;
    (void)track02_size;
    (void)md5_hex;
    (void)out_receipt;
    return 0;
}

int theron_v1_track02_capture_object_table_route_receipt(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02ObjectTableRouteReceipt *out_receipt) {
    (void)track02_data;
    (void)track02_size;
    (void)md5_hex;
    (void)out_receipt;
    return 0;
}

Theron_Track02SignalStatus theron_v1_track02_capture_nonstartup_sector_receipt(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02NonstartupSectorReceipt *out_receipt) {
    (void)track02_data;
    (void)track02_size;
    (void)md5_hex;
    (void)out_receipt;
    return THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus
theron_v1_track02_capture_initial_level_object_boundary(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02InitialLevelObjectBoundaryReceipt *out_receipt) {
    (void)track02_data;
    (void)track02_size;
    (void)md5_hex;
    (void)out_receipt;
    return THERON_TRACK02_SIGNAL_NOT_FOUND;
}
