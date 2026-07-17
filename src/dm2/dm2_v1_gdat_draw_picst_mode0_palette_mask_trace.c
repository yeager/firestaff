#include "dm2_v1_gdat_draw_picst_mode0_palette_mask_trace.h"

#include <string.h>

static uint32_t mix(uint32_t h, uint32_t v) { return (h ^ v) * 16777619u; }

int dm2_v1_gdat_draw_picst_mode0_palette_mask_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectGlobalIntersectionTraceReceipt *i,
    const DM2_V1_GdatMaterializationHandoff *m,
    const DM2_V1_ViewportState *owner, uint8_t mode, int16_t alpha,
    uint16_t colors, uint32_t palette_hash,
    DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt *out)
{
    DM2_V1_ViewportSurfaceSnapshot s;
    uint32_t sf, df, sl, dl, se, de, h = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* SKULLWIN/c_image.cpp:311-335; c_gfx_blit.cpp:655-760. */
    if (!i || !m || !owner || !i->valid || !i->no_draw || !i->identity_hash ||
        !m->valid || !m->no_draw || !m->identity_hash || !m->material_bytes ||
        !m->palette_bytes || i->material_handoff_hash != m->identity_hash ||
        m->stride != m->width || !m->width || !m->height ||
        m->material_byte_count != (uint32_t)m->stride * m->height ||
        m->palette_byte_count < 256u || mode != 0u || alpha < 0 || alpha > 255 ||
        !colors || colors > 256u || !palette_hash ||
        !dm2_v1_viewport_surface_snapshot(owner, &s) ||
        s.generation != i->surface_generation || s.generation != m->surface_generation ||
        !s.framebuffer || s.width != DM2_VP_WIDTH || s.height != DM2_VP_HEIGHT ||
        s.stride != DM2_VP_WIDTH || s.resolution != 8u ||
        i->source_x + i->destination_width > m->width ||
        i->source_y + i->destination_height > m->height ||
        i->destination_x + i->destination_width > s.width ||
        i->destination_y + i->destination_height > s.height ||
        !i->destination_width || !i->destination_height) return 0;
    sf = (uint32_t)i->source_y * m->stride + i->source_x;
    df = (uint32_t)i->destination_y * s.stride + i->destination_x;
    sl = sf + (uint32_t)(i->destination_height - 1u) * m->stride;
    dl = df + (uint32_t)(i->destination_height - 1u) * s.stride;
    se = sl + i->destination_width;
    de = dl + i->destination_width;
    if (se > m->material_byte_count || de > (uint32_t)s.stride * s.height) return 0;
    out->valid = out->no_draw = 1;
    out->source_bytes = m->material_bytes; out->palette_bytes = m->palette_bytes;
    out->destination_bytes = s.framebuffer; out->alpha_index = (uint8_t)alpha;
    out->row_count = i->destination_height; out->bytes_per_row = i->destination_width;
    out->source_first_row = sf; out->source_last_row = sl; out->source_end_exclusive = se;
    out->destination_first_row = df; out->destination_last_row = dl; out->destination_end_exclusive = de;
    out->palette_hash = palette_hash; out->intersection_hash = i->identity_hash;
    out->material_handoff_hash = m->identity_hash; out->surface_generation = s.generation;
    h=mix(h,i->identity_hash); h=mix(h,m->identity_hash); h=mix(h,palette_hash); h=mix(h,s.generation);
    h=mix(h,(uint16_t)alpha); h=mix(h,se); h=mix(h,de); out->identity_hash=h?h:1;
    return 1;
}
