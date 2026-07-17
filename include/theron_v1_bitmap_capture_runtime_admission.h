#ifndef THERON_V1_BITMAP_CAPTURE_RUNTIME_ADMISSION_H
#define THERON_V1_BITMAP_CAPTURE_RUNTIME_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_raw_loader_trace.h"

/* Joins the already authenticated CD capture with the source-owned Soul Room
 * runtime surface. It is intentionally a byte-provenance admission only. */
typedef struct {
    int valid;
    int startup_media_capture_consumed;
    int raw_loader_trace_consumed;
    int runtime_surface_consumed;
    int source_to_runtime_verified;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    unsigned int route_bit;
    size_t first_raw_offset;
    size_t last_raw_offset;
    size_t first_user_data_offset;
    uint32_t bitmap_checksum;
    uint32_t bitmap_atlas_checksum;
    uint32_t dynamic_cd_read_record;
    int palette_descriptor_relation_verified;
    int pixel_decode_verified;
    int render_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1BitmapCaptureRuntimeAdmissionReceipt;

/* Admits the one Soul Room surface whose raw span is already proven disjoint
 * from the authentic dynamic CD_READ span. No palette, pixel, level, object,
 * or drawing semantics are inferred. */
int theron_v1_bitmap_capture_admit_soul_room_runtime(
    const Theron_StartupMediaStateReceipt *media_capture,
    const Theron_V1RawLoaderTraceReceipt *loader_trace,
    const Theron_V1_World *world,
    Theron_V1BitmapCaptureRuntimeAdmissionReceipt *out);

#endif
