#include "dm2_v1_gdat_draw_picst_palette_write_trace.h"

#include <string.h>

int dm2_v1_gdat_draw_picst_palette_write_trace_receipt_build(
    const DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt *m,
    const DM2_V1_GdatDrawPicstPaletteIndexTraceReceipt *i,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatDrawPicstPaletteWriteTraceReceipt *out)
{
    DM2_V1_ViewportSurfaceSnapshot s;
    uint32_t h = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* t_palette has one c_pixel256 (one ui8); xlat writes only non-alpha pixels. */
    if (!m || !i || !owner || !m->valid || !m->no_draw || !m->identity_hash ||
        !i->valid || !i->no_draw || !i->identity_hash ||
        i->masked_palette_trace_hash != m->identity_hash ||
        i->palette_hash != m->palette_hash || i->alpha_index != m->alpha_index ||
        i->palette_index_domain != 256u || i->row_count != m->row_count ||
        i->pixels_per_row != m->bytes_per_row || !m->palette_bytes ||
        !m->destination_bytes || m->palette_hash == 0 ||
        !dm2_v1_viewport_surface_snapshot(owner, &s) ||
        s.framebuffer != m->destination_bytes ||
        s.generation != m->surface_generation || s.stride != DM2_VP_WIDTH ||
        s.resolution != 8u || m->destination_end_exclusive >
            (uint32_t)s.stride * s.height)
        return 0;
    out->valid = out->no_draw = 1;
    out->palette_entry_bytes = 1u;
    out->palette_entry_count = 256u;
    out->alpha_index = m->alpha_index;
    out->row_count = m->row_count;
    out->writes_per_row = m->bytes_per_row;
    out->destination_first_row = m->destination_first_row;
    out->destination_last_row = m->destination_last_row;
    out->destination_end_exclusive = m->destination_end_exclusive;
    out->destination_surface_generation = s.generation;
    out->palette_index_trace_hash = i->identity_hash;
    out->masked_palette_trace_hash = m->identity_hash;
    h=(h^m->identity_hash)*16777619u; h=(h^i->identity_hash)*16777619u;
    h=(h^m->palette_hash)*16777619u; h=(h^s.generation)*16777619u;
    h=(h^m->destination_first_row)*16777619u;
    h=(h^m->destination_end_exclusive)*16777619u;
    out->identity_hash=h?h:1u;
    return 1;
}
