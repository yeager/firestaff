#include "dm2_v1_gdat_draw_picst_palette_index_trace.h"

#include <string.h>

int dm2_v1_gdat_draw_picst_palette_index_trace_receipt_build(
    const DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt *m,
    DM2_V1_GdatDrawPicstPaletteIndexTraceReceipt *out)
{
    uint32_t h = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* SKULLWIN/c_gfx_blit.cpp:39-42,675-682; do not dereference bytes here. */
    if (!m || !m->valid || !m->no_draw || !m->identity_hash ||
        !m->source_bytes || !m->palette_bytes || !m->destination_bytes ||
        !m->palette_hash || !m->row_count || !m->bytes_per_row ||
        m->source_end_exclusive <= m->source_last_row ||
        m->destination_end_exclusive <= m->destination_last_row)
        return 0;
    out->valid = out->no_draw = 1;
    out->alpha_index = m->alpha_index;
    out->palette_index_domain = 256u;
    out->row_count = m->row_count;
    out->pixels_per_row = m->bytes_per_row;
    out->source_first_row = m->source_first_row;
    out->source_last_row = m->source_last_row;
    out->palette_hash = m->palette_hash;
    out->masked_palette_trace_hash = m->identity_hash;
    h = (h ^ m->identity_hash) * 16777619u;
    h = (h ^ m->palette_hash) * 16777619u;
    h = (h ^ m->alpha_index) * 16777619u;
    h = (h ^ m->source_first_row) * 16777619u;
    h = (h ^ m->source_last_row) * 16777619u;
    out->identity_hash = h ? h : 1u;
    return 1;
}
