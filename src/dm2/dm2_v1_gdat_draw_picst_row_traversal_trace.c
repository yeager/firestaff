#include "dm2_v1_gdat_draw_picst_row_traversal_trace.h"

#include <string.h>

static uint32_t dm2_v1_gdat_draw_picst_row_traversal_trace_mix(uint32_t hash,
                                                                 uint32_t value)
{
    return (hash ^ value) * 16777619u;
}

int dm2_v1_gdat_draw_picst_row_traversal_trace_receipt_build(
    const DM2_V1_GdatDrawPicstSurfaceAddressTraceReceipt *surface_address,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    uint8_t blit_mode,
    DM2_V1_GdatDrawPicstRowTraversalTraceReceipt *out)
{
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint32_t source_last_row;
    uint32_t destination_last_row;
    uint32_t source_end_exclusive;
    uint32_t destination_end_exclusive;
    uint32_t addressed_pixel_count;
    uint32_t hash = 2166136261u;

    if (out == NULL)
        return 0;
    memset(out, 0, sizeof(*out));

    /* SKULLWIN/c_gfx_blit.cpp:604-656 default branch: forward rows only. */
    if (surface_address == NULL || material_handoff == NULL || owner == NULL ||
        !surface_address->valid || !surface_address->no_draw ||
        surface_address->identity_hash == 0 ||
        !material_handoff->valid || !material_handoff->no_draw ||
        material_handoff->identity_hash == 0 ||
        material_handoff->material_bytes == NULL ||
        surface_address->source_bytes != material_handoff->material_bytes ||
        surface_address->material_handoff_hash != material_handoff->identity_hash ||
        material_handoff->material_byte_count !=
            (uint32_t)material_handoff->stride * material_handoff->height ||
        material_handoff->stride != surface_address->source_stride ||
        surface_address->width == 0 || surface_address->height == 0 ||
        blit_mode != 0u || !dm2_v1_viewport_surface_snapshot(owner, &surface) ||
        surface.generation != surface_address->surface_generation ||
        surface.generation != material_handoff->surface_generation ||
        surface.framebuffer != surface_address->destination_bytes ||
        surface.stride != surface_address->destination_stride ||
        surface.stride != DM2_VP_WIDTH || surface.resolution != 8u)
        return 0;

    source_last_row = surface_address->source_row_offset +
        (uint32_t)(surface_address->height - 1u) * surface_address->source_stride;
    destination_last_row = surface_address->destination_row_offset +
        (uint32_t)(surface_address->height - 1u) * surface_address->destination_stride;
    source_end_exclusive = source_last_row + surface_address->width;
    destination_end_exclusive = destination_last_row + surface_address->width;
    addressed_pixel_count = (uint32_t)surface_address->width * surface_address->height;
    if (source_end_exclusive > material_handoff->material_byte_count ||
        destination_end_exclusive > (uint32_t)surface.stride * surface.height)
        return 0;

    out->valid = 1;
    out->no_draw = 1;
    out->row_count = surface_address->height;
    out->bytes_per_row = surface_address->width;
    out->source_first_row = surface_address->source_row_offset;
    out->source_last_row = source_last_row;
    out->source_end_exclusive = source_end_exclusive;
    out->destination_first_row = surface_address->destination_row_offset;
    out->destination_last_row = destination_last_row;
    out->destination_end_exclusive = destination_end_exclusive;
    out->addressed_pixel_count = addressed_pixel_count;
    out->surface_address_hash = surface_address->identity_hash;
    out->material_handoff_hash = material_handoff->identity_hash;
    out->surface_generation = surface.generation;
    hash = dm2_v1_gdat_draw_picst_row_traversal_trace_mix(hash,
                                                           surface_address->identity_hash);
    hash = dm2_v1_gdat_draw_picst_row_traversal_trace_mix(hash,
                                                           material_handoff->identity_hash);
    hash = dm2_v1_gdat_draw_picst_row_traversal_trace_mix(hash,
                                                           surface.generation);
    hash = dm2_v1_gdat_draw_picst_row_traversal_trace_mix(hash, source_last_row);
    hash = dm2_v1_gdat_draw_picst_row_traversal_trace_mix(hash,
                                                           destination_last_row);
    hash = dm2_v1_gdat_draw_picst_row_traversal_trace_mix(hash,
                                                           addressed_pixel_count);
    out->identity_hash = hash == 0 ? 1 : hash;
    return 1;
}
