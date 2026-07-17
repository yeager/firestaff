#include "dm2_v1_gdat_draw_picst_surface_address_trace.h"

#include <string.h>

static uint32_t dm2_v1_gdat_draw_picst_surface_address_trace_mix(uint32_t hash,
                                                                   uint32_t value)
{
    return (hash ^ value) * 16777619u;
}

int dm2_v1_gdat_draw_picst_surface_address_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectGlobalIntersectionTraceReceipt *intersection,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    int destination_is_gfxsys_screen,
    uint8_t source_resolution,
    int palette_absent,
    int16_t alpha_mask,
    uint32_t draw_picst_image_hash,
    DM2_V1_GdatDrawPicstSurfaceAddressTraceReceipt *out)
{
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint32_t source_row_offset;
    uint32_t destination_row_offset;
    uint32_t source_last;
    uint32_t destination_last;
    uint32_t hash = 2166136261u;

    if (out == NULL)
        return 0;
    memset(out, 0, sizeof(*out));

    /* SKULLWIN/c_image.cpp:293-335, c_gfx_blit.cpp:604-656. */
    if (intersection == NULL || material_handoff == NULL || owner == NULL ||
        !intersection->valid || !intersection->no_draw ||
        intersection->identity_hash == 0 ||
        !material_handoff->valid || !material_handoff->no_draw ||
        material_handoff->identity_hash == 0 ||
        material_handoff->material_bytes == NULL ||
        intersection->material_handoff_hash != material_handoff->identity_hash ||
        material_handoff->stride != material_handoff->width ||
        material_handoff->width == 0 || material_handoff->height == 0 ||
        !destination_is_gfxsys_screen || source_resolution != 8u ||
        !palette_absent || alpha_mask != -1 || draw_picst_image_hash == 0 ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface) ||
        surface.generation != intersection->surface_generation ||
        surface.generation != material_handoff->surface_generation ||
        surface.framebuffer == NULL || surface.width != DM2_VP_WIDTH ||
        surface.height != DM2_VP_HEIGHT || surface.stride != DM2_VP_WIDTH ||
        surface.resolution != 8u ||
        intersection->source_x >= material_handoff->width ||
        intersection->source_y >= material_handoff->height ||
        intersection->destination_x >= surface.width ||
        intersection->destination_y >= surface.height ||
        intersection->destination_width == 0 ||
        intersection->destination_height == 0 ||
        intersection->source_x + intersection->destination_width > material_handoff->width ||
        intersection->source_y + intersection->destination_height > material_handoff->height ||
        intersection->destination_x + intersection->destination_width > surface.width ||
        intersection->destination_y + intersection->destination_height > surface.height)
        return 0;

    source_row_offset = (uint32_t)intersection->source_y * material_handoff->stride +
                        intersection->source_x;
    destination_row_offset = (uint32_t)intersection->destination_y * surface.stride +
                             intersection->destination_x;
    source_last = source_row_offset +
        (uint32_t)(intersection->destination_height - 1u) * material_handoff->stride +
        intersection->destination_width;
    destination_last = destination_row_offset +
        (uint32_t)(intersection->destination_height - 1u) * surface.stride +
        intersection->destination_width;
    if (source_last > (uint32_t)material_handoff->stride * material_handoff->height ||
        destination_last > (uint32_t)surface.stride * surface.height)
        return 0;

    out->valid = 1;
    out->no_draw = 1;
    out->source_bytes = material_handoff->material_bytes;
    out->destination_bytes = surface.framebuffer;
    out->source_stride = material_handoff->stride;
    out->destination_stride = surface.stride;
    out->source_row_offset = source_row_offset;
    out->destination_row_offset = destination_row_offset;
    out->width = intersection->destination_width;
    out->height = intersection->destination_height;
    out->intersection_hash = intersection->identity_hash;
    out->material_handoff_hash = material_handoff->identity_hash;
    out->draw_picst_image_hash = draw_picst_image_hash;
    out->surface_generation = surface.generation;
    hash = dm2_v1_gdat_draw_picst_surface_address_trace_mix(hash,
                                                             intersection->identity_hash);
    hash = dm2_v1_gdat_draw_picst_surface_address_trace_mix(hash,
                                                             material_handoff->identity_hash);
    hash = dm2_v1_gdat_draw_picst_surface_address_trace_mix(hash,
                                                             draw_picst_image_hash);
    hash = dm2_v1_gdat_draw_picst_surface_address_trace_mix(hash,
                                                             surface.generation);
    hash = dm2_v1_gdat_draw_picst_surface_address_trace_mix(hash,
                                                             source_row_offset);
    hash = dm2_v1_gdat_draw_picst_surface_address_trace_mix(hash,
                                                             destination_row_offset);
    out->identity_hash = hash == 0 ? 1 : hash;
    return 1;
}
