#ifndef THERON_V1_TRACK02_LIVE_LOADER_ROUTE_ADMISSION_H
#define THERON_V1_TRACK02_LIVE_LOADER_ROUTE_ADMISSION_H

#include "theron_v1_track02_capture_trace_runtime_admission.h"
#include "theron_v1_track02_dynamic_cd_read_ownership.h"
#include "theron_v1_track02_huc6280_capture_event_log.h"
#include "theron_v1_track02_mednafen_trace_converter.h"

typedef enum {
    THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM = 1,
    THERON_V1_TRACK02_LIVE_ROUTE_DUNGEON_HANDOFF = 2
} Theron_V1Track02LiveRouteKind;

/* Joins the register-normalized dynamic loader record to the independently
 * observed HuC6280 consumer rows. It publishes only the pre-existing opaque
 * runtime route; no record bytes or gameplay/visual semantics are exposed. */
typedef struct {
    int valid;
    int dynamic_cd_read_ownership_consumed;
    int huc6280_event_log_consumed;
    int manifest_bound;
    int opaque_runtime_route_ready;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    char source_trace_md5[33];
    char huc6280_event_log_md5[33];
    uint32_t loader_record;
    uint16_t loader_destination;
    uint32_t consumer_trace_checksum;
    int level_object_semantics_allowed;
    int bitmap_palette_admission_allowed;
    int pixel_decode_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02LiveLoaderRouteAdmissionReceipt;

int theron_v1_track02_live_loader_route_admit(
    const Theron_V1Track02DynamicCdReadOwnershipReceipt *ownership,
    const Theron_V1Track02Huc6280CaptureEventLog *event_log,
    const Theron_V1Track02MednafenTraceConvertReceipt *trace_identity,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out_runtime,
    Theron_V1Track02LiveLoaderRouteAdmissionReceipt *out);

#endif
