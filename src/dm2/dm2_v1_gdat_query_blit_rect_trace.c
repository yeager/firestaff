#include "dm2_v1_gdat_query_blit_rect_trace.h"

#include <string.h>

static uint32_t dm2_v1_gdat_query_blit_rect_trace_mix(uint32_t hash,
                                                        uint32_t value)
{
    return (hash ^ value) * 16777619u;
}

int dm2_v1_gdat_query_blit_rect_trace_receipt_build(
    const DM2_V1_GdatDrawPicstRectTrace *source_rect,
    int16_t query1,
    int16_t query2,
    uint8_t mode1,
    uint16_t mode2,
    int16_t datax,
    int16_t datay,
    uint32_t rectangle_node_hash,
    int bitmap_present,
    DM2_V1_GdatQueryBlitRectTraceReceipt *out)
{
    uint32_t hash = 2166136261u;

    if (out == NULL)
        return 0;
    memset(out, 0, sizeof(*out));

    /* SKULLWIN/c_xrect.cpp:217-280.  No signed, override or chained node. */
    if (source_rect == NULL || !source_rect->valid || !source_rect->no_draw ||
        source_rect->identity_hash == 0 || query1 < 0 || query1 == -1 ||
        query2 != -1 || mode1 > 8 || mode2 != 0 || datax < 0 || datay < 0 ||
        rectangle_node_hash == 0 || !bitmap_present)
        return 0;

    out->valid = 1;
    out->no_draw = 1;
    out->rectangle_node = (uint16_t)query1;
    out->mode1 = mode1;
    out->initial_x = (uint16_t)datax;
    out->initial_y = (uint16_t)datay;
    out->source_rect_hash = source_rect->identity_hash;
    out->rectangle_node_hash = rectangle_node_hash;
    hash = dm2_v1_gdat_query_blit_rect_trace_mix(hash, source_rect->identity_hash);
    hash = dm2_v1_gdat_query_blit_rect_trace_mix(hash, (uint16_t)query1);
    hash = dm2_v1_gdat_query_blit_rect_trace_mix(hash, mode1);
    hash = dm2_v1_gdat_query_blit_rect_trace_mix(hash, (uint16_t)datax);
    hash = dm2_v1_gdat_query_blit_rect_trace_mix(hash, (uint16_t)datay);
    hash = dm2_v1_gdat_query_blit_rect_trace_mix(hash, rectangle_node_hash);
    out->identity_hash = hash == 0 ? 1 : hash;
    return 1;
}
