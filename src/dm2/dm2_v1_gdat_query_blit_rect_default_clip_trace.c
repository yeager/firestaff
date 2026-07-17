#include "dm2_v1_gdat_query_blit_rect_default_clip_trace.h"

#include <string.h>

static uint32_t dm2_v1_gdat_query_blit_rect_default_clip_trace_mix(uint32_t hash,
                                                                     uint32_t value)
{
    return (hash ^ value) * 16777619u;
}

int dm2_v1_gdat_query_blit_rect_default_clip_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectMode1TraceReceipt *mode1_trace,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    int global_clip_override,
    uint16_t terminal_query_node,
    DM2_V1_GdatQueryBlitRectDefaultClipTraceReceipt *out)
{
    DM2_V1_ViewportSurfaceSnapshot surface;
    int32_t right;
    int32_t bottom;
    uint32_t hash = 2166136261u;

    if (out == NULL)
        return 0;
    memset(out, 0, sizeof(*out));

    /* SKULLWIN/c_xrect.cpp:239,438-470; default rc is [-10000, 10000). */
    if (mode1_trace == NULL || material_handoff == NULL || owner == NULL ||
        !mode1_trace->valid || !mode1_trace->no_draw ||
        mode1_trace->identity_hash == 0 ||
        !material_handoff->valid || !material_handoff->no_draw ||
        material_handoff->identity_hash == 0 ||
        material_handoff->material_bytes == NULL ||
        material_handoff->identity_hash != mode1_trace->material_handoff_hash ||
        material_handoff->width != mode1_trace->bitmap_width ||
        material_handoff->height != mode1_trace->bitmap_height ||
        material_handoff->stride < material_handoff->width ||
        global_clip_override || terminal_query_node != 0 ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface) ||
        surface.generation != mode1_trace->surface_generation ||
        surface.generation != material_handoff->surface_generation)
        return 0;

    right = (int32_t)mode1_trace->decoded_x + material_handoff->width;
    bottom = (int32_t)mode1_trace->decoded_y + material_handoff->height;
    if (right > 10000 || bottom > 10000)
        return 0;

    out->valid = 1;
    out->no_draw = 1;
    out->rect_x = (int16_t)mode1_trace->decoded_x;
    out->rect_y = (int16_t)mode1_trace->decoded_y;
    out->rect_width = material_handoff->width;
    out->rect_height = material_handoff->height;
    out->mode1_trace_hash = mode1_trace->identity_hash;
    out->material_handoff_hash = material_handoff->identity_hash;
    out->surface_generation = surface.generation;
    hash = dm2_v1_gdat_query_blit_rect_default_clip_trace_mix(hash,
                                                               mode1_trace->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_default_clip_trace_mix(hash,
                                                               material_handoff->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_default_clip_trace_mix(hash,
                                                               surface.generation);
    hash = dm2_v1_gdat_query_blit_rect_default_clip_trace_mix(hash,
                                                               (uint16_t)out->rect_x);
    hash = dm2_v1_gdat_query_blit_rect_default_clip_trace_mix(hash,
                                                               (uint16_t)out->rect_y);
    out->identity_hash = hash == 0 ? 1 : hash;
    return 1;
}
