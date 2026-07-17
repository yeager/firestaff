#include "dm2_v1_gdat_query_blit_rect_signed_trace.h"

#include <string.h>

static uint32_t dm2_v1_gdat_query_blit_rect_signed_trace_mix(uint32_t hash,
                                                               uint32_t value)
{
    return (hash ^ value) * 16777619u;
}

int dm2_v1_gdat_query_blit_rect_signed_trace_receipt_build(
    const DM2_V1_GdatDrawPicstRectTrace *source_rect,
    int16_t query1,
    int16_t query2,
    uint8_t mode1,
    uint16_t mode2,
    int16_t datax,
    int16_t datay,
    int16_t input_x,
    int16_t input_y,
    uint32_t rectangle_node_hash,
    int bitmap_present,
    DM2_V1_GdatQueryBlitRectSignedTraceReceipt *out)
{
    int32_t offset_x;
    int32_t offset_y;
    uint32_t hash = 2166136261u;

    if (out == NULL)
        return 0;
    memset(out, 0, sizeof(*out));

    /* SKULLWIN/c_xrect.cpp:228-276; stop before crdecode and clipping. */
    if (source_rect == NULL || !source_rect->valid || !source_rect->no_draw ||
        source_rect->identity_hash == 0 || query1 >= -2 || query2 != -1 ||
        mode1 > 8 || mode2 != 0 || datax < 0 || datay < 0 || input_x < 0 ||
        input_y < 0 || rectangle_node_hash == 0 || !bitmap_present)
        return 0;

    offset_x = (int32_t)datax + input_x;
    offset_y = (int32_t)datay + input_y;
    if (offset_x > INT16_MAX || offset_y > INT16_MAX)
        return 0;

    out->valid = 1;
    out->no_draw = 1;
    out->rectangle_node = (uint16_t)(query1 & 0x7fff);
    out->mode1 = mode1;
    out->offset_x = (uint16_t)offset_x;
    out->offset_y = (uint16_t)offset_y;
    out->source_rect_hash = source_rect->identity_hash;
    out->rectangle_node_hash = rectangle_node_hash;
    hash = dm2_v1_gdat_query_blit_rect_signed_trace_mix(hash, source_rect->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_signed_trace_mix(hash, out->rectangle_node);
    hash = dm2_v1_gdat_query_blit_rect_signed_trace_mix(hash, mode1);
    hash = dm2_v1_gdat_query_blit_rect_signed_trace_mix(hash, out->offset_x);
    hash = dm2_v1_gdat_query_blit_rect_signed_trace_mix(hash, out->offset_y);
    hash = dm2_v1_gdat_query_blit_rect_signed_trace_mix(hash, rectangle_node_hash);
    out->identity_hash = hash == 0 ? 1 : hash;
    return 1;
}
