#include "dm2_v1_gdat_draw_picst_mode0_executor.h"

int dm2_v1_gdat_draw_picst_mode0_execute(
    const DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt *m,
    const DM2_V1_GdatDrawPicstPaletteIndexTraceReceipt *i,
    const DM2_V1_GdatDrawPicstPaletteWriteTraceReceipt *w,
    const DM2_V1_GdatDrawPicstMaskedConsumeTraceReceipt *c,
    DM2_V1_ViewportState *owner)
{
    DM2_V1_ViewportSurfaceSnapshot s;
    uint32_t row;
    if (!m || !i || !w || !c || !owner || !m->valid || !m->no_draw ||
        !i->valid || !i->no_draw || !w->valid || !w->no_draw ||
        !c->valid || !c->no_draw || !m->identity_hash || !i->identity_hash ||
        !w->identity_hash || !c->identity_hash ||
        i->masked_palette_trace_hash != m->identity_hash ||
        w->masked_palette_trace_hash != m->identity_hash ||
        c->palette_write_trace_hash != w->identity_hash ||
        i->palette_hash != m->palette_hash || w->alpha_index != m->alpha_index ||
        c->alpha_index != m->alpha_index || i->palette_index_domain != 256u ||
        w->palette_entry_bytes != 1u || w->palette_entry_count != 256u ||
        c->source_step != 1u || c->destination_step != 1u ||
        !c->conditional_write_before_destination_step ||
        c->row_count != m->row_count || c->pixels_per_row != m->bytes_per_row ||
        !m->source_bytes || !m->palette_bytes || !m->destination_bytes ||
        !dm2_v1_viewport_surface_snapshot(owner, &s) ||
        s.framebuffer != m->destination_bytes || s.generation != m->surface_generation ||
        s.stride != DM2_VP_WIDTH || s.resolution != 8u ||
        m->destination_end_exclusive > (uint32_t)s.stride * s.height)
        return 0;

    for (row = 0; row < m->row_count; ++row) {
        uint32_t source_stride = m->row_count > 1u ?
            (m->source_last_row - m->source_first_row) / (m->row_count - 1u) : 0u;
        uint32_t destination_stride = m->row_count > 1u ?
            (m->destination_last_row - m->destination_first_row) /
                (m->row_count - 1u) : 0u;
        const uint8_t *source = m->source_bytes + m->source_first_row +
            row * source_stride;
        uint8_t *destination = m->destination_bytes + m->destination_first_row +
            row * destination_stride;
        uint32_t pixel;
        for (pixel = 0; pixel < m->bytes_per_row; ++pixel) {
            uint8_t index = source[pixel];
            if (index != m->alpha_index)
                destination[pixel] = m->palette_bytes[index];
        }
    }
    return 1;
}
