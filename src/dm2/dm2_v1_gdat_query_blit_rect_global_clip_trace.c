#include "dm2_v1_gdat_query_blit_rect_global_clip_trace.h"

#include <string.h>

static uint32_t dm2_v1_gdat_query_blit_rect_global_clip_trace_mix(uint32_t hash,
                                                                    uint32_t value)
{
    return (hash ^ value) * 16777619u;
}

int dm2_v1_gdat_query_blit_rect_global_clip_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectMode1TraceReceipt *mode1_trace,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    int global_clip_enabled,
    int16_t trim_x,
    int16_t trim_y,
    int16_t right_trim,
    int16_t bottom_trim,
    uint32_t trim_call_hash,
    DM2_V1_GdatQueryBlitRectGlobalClipTraceReceipt *out)
{
    DM2_V1_ViewportSurfaceSnapshot surface;
    int32_t clip_width;
    int32_t clip_height;
    uint32_t hash = 2166136261u;

    if (out == NULL)
        return 0;
    memset(out, 0, sizeof(*out));

    /* SKULLWIN/c_gui_vp.cpp:570-573; c_xrect.cpp:438-439. */
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
        !global_clip_enabled || trim_x < 0 || trim_y < 0 || right_trim < 0 ||
        bottom_trim < 0 || trim_call_hash == 0 ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface) ||
        surface.generation != mode1_trace->surface_generation ||
        surface.generation != material_handoff->surface_generation)
        return 0;

    clip_width = (int32_t)surface.width - ((int32_t)trim_x + right_trim);
    clip_height = (int32_t)surface.height - ((int32_t)trim_y + bottom_trim);
    if (clip_width <= 0 || clip_height <= 0 || clip_width > UINT16_MAX ||
        clip_height > UINT16_MAX)
        return 0;

    out->valid = 1;
    out->no_draw = 1;
    out->clip_x = (uint16_t)trim_x;
    out->clip_y = (uint16_t)trim_y;
    out->clip_width = (uint16_t)clip_width;
    out->clip_height = (uint16_t)clip_height;
    out->trim_call_hash = trim_call_hash;
    out->mode1_trace_hash = mode1_trace->identity_hash;
    out->material_handoff_hash = material_handoff->identity_hash;
    out->surface_generation = surface.generation;
    hash = dm2_v1_gdat_query_blit_rect_global_clip_trace_mix(hash,
                                                              mode1_trace->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_global_clip_trace_mix(hash,
                                                              material_handoff->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_global_clip_trace_mix(hash,
                                                              trim_call_hash);
    hash = dm2_v1_gdat_query_blit_rect_global_clip_trace_mix(hash,
                                                              surface.generation);
    hash = dm2_v1_gdat_query_blit_rect_global_clip_trace_mix(hash,
                                                              out->clip_x);
    hash = dm2_v1_gdat_query_blit_rect_global_clip_trace_mix(hash,
                                                              out->clip_y);
    hash = dm2_v1_gdat_query_blit_rect_global_clip_trace_mix(hash,
                                                              out->clip_width);
    hash = dm2_v1_gdat_query_blit_rect_global_clip_trace_mix(hash,
                                                              out->clip_height);
    out->identity_hash = hash == 0 ? 1 : hash;
    return 1;
}
