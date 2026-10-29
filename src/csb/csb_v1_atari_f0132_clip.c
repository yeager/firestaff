#include "csb_v1_atari_f0132_clip.h"

#include <limits.h>

int csb_v1_atari_f0132_visible_span(int16_t source_x,
                                    uint16_t source_width,
                                    uint16_t box_width,
                                    CSB_V1_AtariF0132VisibleSpan *out_span)
{
    int32_t source_start;
    int32_t destination_start;
    int32_t available_source;
    int32_t available_destination;
    int32_t count;

    if (!out_span) return 0;
    out_span->destination_offset = 0u;
    out_span->source_offset = 0u;
    out_span->pixel_count = 0u;
    if (source_width == 0u || box_width == 0u) return 1;

    source_start = source_x;
    destination_start = 0;
    if (source_start < 0) {
        destination_start = -source_start;
        source_start = 0;
    }
    if (source_start >= source_width || destination_start >= box_width)
        return 1;

    available_source = (int32_t)source_width - source_start;
    available_destination = (int32_t)box_width - destination_start;
    count = available_source < available_destination
        ? available_source : available_destination;
    if (count <= 0 || source_start > UINT16_MAX ||
        destination_start > UINT16_MAX || count > UINT16_MAX) return 0;

    out_span->destination_offset = (uint16_t)destination_start;
    out_span->source_offset = (uint16_t)source_start;
    out_span->pixel_count = (uint16_t)count;
    return 1;
}
