#include "dm2_v1_gdat_query_blit_rect_global_intersection_trace.h"

#include <string.h>

static uint32_t dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
    uint32_t hash, uint32_t value)
{
    return (hash ^ value) * 16777619u;
}

int dm2_v1_gdat_query_blit_rect_global_intersection_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectGlobalClipTraceReceipt *global_clip,
    const DM2_V1_GdatQueryBlitRectMode1TraceReceipt *mode1_trace,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatQueryBlitRectGlobalIntersectionTraceReceipt *out)
{
    DM2_V1_ViewportSurfaceSnapshot surface;
    int32_t dx;
    int32_t dy;
    int32_t destination_width;
    int32_t destination_height;
    int32_t source_x;
    int32_t source_y;
    int32_t destination_x;
    int32_t destination_y;
    uint32_t hash = 2166136261u;

    if (out == NULL)
        return 0;
    memset(out, 0, sizeof(*out));

    /* SKULLWIN/c_xrect.cpp:446-470; rc is the active dm2rect1 override. */
    if (global_clip == NULL || mode1_trace == NULL || material_handoff == NULL ||
        owner == NULL || !global_clip->valid || !global_clip->no_draw ||
        global_clip->identity_hash == 0 || !mode1_trace->valid ||
        !mode1_trace->no_draw || mode1_trace->identity_hash == 0 ||
        !material_handoff->valid || !material_handoff->no_draw ||
        material_handoff->identity_hash == 0 ||
        material_handoff->material_bytes == NULL ||
        global_clip->mode1_trace_hash != mode1_trace->identity_hash ||
        global_clip->material_handoff_hash != material_handoff->identity_hash ||
        material_handoff->identity_hash != mode1_trace->material_handoff_hash ||
        material_handoff->width != mode1_trace->bitmap_width ||
        material_handoff->height != mode1_trace->bitmap_height ||
        material_handoff->stride < material_handoff->width ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface) ||
        surface.generation != global_clip->surface_generation ||
        surface.generation != mode1_trace->surface_generation ||
        surface.generation != material_handoff->surface_generation)
        return 0;

    dx = (int32_t)global_clip->clip_x - mode1_trace->decoded_x;
    dy = (int32_t)global_clip->clip_y - mode1_trace->decoded_y;
    if (dx > 0) {
        source_x = dx;
        destination_x = global_clip->clip_x;
        destination_width = material_handoff->width - dx;
        if (destination_width > global_clip->clip_width)
            destination_width = global_clip->clip_width;
    } else {
        source_x = 0;
        destination_x = mode1_trace->decoded_x;
        destination_width = dx + global_clip->clip_width;
        if (destination_width > material_handoff->width)
            destination_width = material_handoff->width;
    }
    if (dy > 0) {
        source_y = dy;
        destination_y = global_clip->clip_y;
        destination_height = material_handoff->height - dy;
        if (destination_height > global_clip->clip_height)
            destination_height = global_clip->clip_height;
    } else {
        source_y = 0;
        destination_y = mode1_trace->decoded_y;
        destination_height = dy + global_clip->clip_height;
        if (destination_height > material_handoff->height)
            destination_height = material_handoff->height;
    }
    if (source_x < 0 || source_y < 0 || source_x > UINT16_MAX ||
        source_y > UINT16_MAX || destination_x < 0 || destination_y < 0 ||
        destination_x > UINT16_MAX || destination_y > UINT16_MAX ||
        destination_width <= 0 || destination_height <= 0 ||
        destination_width > UINT16_MAX || destination_height > UINT16_MAX)
        return 0;

    out->valid = 1;
    out->no_draw = 1;
    out->source_x = (uint16_t)source_x;
    out->source_y = (uint16_t)source_y;
    out->destination_x = (uint16_t)destination_x;
    out->destination_y = (uint16_t)destination_y;
    out->destination_width = (uint16_t)destination_width;
    out->destination_height = (uint16_t)destination_height;
    out->global_clip_hash = global_clip->identity_hash;
    out->mode1_trace_hash = mode1_trace->identity_hash;
    out->material_handoff_hash = material_handoff->identity_hash;
    out->surface_generation = surface.generation;
    hash = dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
        hash, global_clip->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
        hash, mode1_trace->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
        hash, material_handoff->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
        hash, surface.generation);
    hash = dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
        hash, out->source_x);
    hash = dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
        hash, out->source_y);
    hash = dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
        hash, out->destination_x);
    hash = dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
        hash, out->destination_y);
    hash = dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
        hash, out->destination_width);
    hash = dm2_v1_gdat_query_blit_rect_global_intersection_trace_mix(
        hash, out->destination_height);
    out->identity_hash = hash == 0 ? 1 : hash;
    return 1;
}
