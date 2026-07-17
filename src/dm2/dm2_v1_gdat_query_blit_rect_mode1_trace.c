#include "dm2_v1_gdat_query_blit_rect_mode1_trace.h"

#include <string.h>

static uint32_t dm2_v1_gdat_query_blit_rect_mode1_trace_mix(uint32_t hash,
                                                              uint32_t value)
{
    return (hash ^ value) * 16777619u;
}

int dm2_v1_gdat_query_blit_rect_mode1_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectSignedTraceReceipt *signed_trace,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatQueryBlitRectMode1TraceReceipt *out)
{
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint32_t hash = 2166136261u;

    if (out == NULL)
        return 0;
    memset(out, 0, sizeof(*out));

    /* SKULLWIN/c_xrect.cpp:162-211, 426-436: mode 1 is x=x0, y=y0. */
    if (signed_trace == NULL || material_handoff == NULL || owner == NULL ||
        !signed_trace->valid || !signed_trace->no_draw ||
        signed_trace->mode1 != 1 || signed_trace->identity_hash == 0 ||
        !material_handoff->valid || !material_handoff->no_draw ||
        material_handoff->identity_hash == 0 ||
        material_handoff->material_bytes == NULL ||
        material_handoff->width == 0 || material_handoff->height == 0 ||
        material_handoff->stride < material_handoff->width ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface) ||
        surface.generation != material_handoff->surface_generation)
        return 0;

    out->valid = 1;
    out->no_draw = 1;
    out->decoded_x = signed_trace->offset_x;
    out->decoded_y = signed_trace->offset_y;
    out->bitmap_width = material_handoff->width;
    out->bitmap_height = material_handoff->height;
    out->signed_trace_hash = signed_trace->identity_hash;
    out->material_handoff_hash = material_handoff->identity_hash;
    out->surface_generation = surface.generation;
    hash = dm2_v1_gdat_query_blit_rect_mode1_trace_mix(hash,
                                                        signed_trace->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_mode1_trace_mix(hash,
                                                        material_handoff->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_mode1_trace_mix(hash,
                                                        surface.generation);
    hash = dm2_v1_gdat_query_blit_rect_mode1_trace_mix(hash, out->decoded_x);
    hash = dm2_v1_gdat_query_blit_rect_mode1_trace_mix(hash, out->decoded_y);
    out->identity_hash = hash == 0 ? 1 : hash;
    return 1;
}
