#include "dm2_v1_skproject_core.h"

#include <string.h>

static uint32_t dm2_v1_skproject_rect_hash(
    uint8_t slot,
    uint8_t next_slot,
    const DM2_V1_SkprojectRect *rect)
{
    uint32_t hash = 2166136261u;

    hash = (hash ^ slot) * 16777619u;
    hash = (hash ^ next_slot) * 16777619u;
    hash = (hash ^ (uint16_t)rect->x) * 16777619u;
    hash = (hash ^ (uint16_t)rect->y) * 16777619u;
    hash = (hash ^ (uint16_t)rect->w) * 16777619u;
    hash = (hash ^ (uint16_t)rect->h) * 16777619u;
    return hash ? hash : 1u;
}

int16_t dm2_v1_skproject_between_value(int16_t minv,
                                       int16_t newv,
                                       int16_t maxv)
{
    if (newv < minv) return minv;
    if (newv > maxv) return maxv;
    return newv;
}

int16_t dm2_v1_skproject_dm2_between_value(int16_t minv,
                                           int16_t maxv,
                                           int16_t value)
{
    return dm2_v1_skproject_between_value(minv, value, maxv);
}

void dm2_v1_skproject_temp_rect_ring_init(
    DM2_V1_SkprojectTempRectRing *ring)
{
    if (!ring) return;
    memset(ring, 0, sizeof(*ring));
}

int dm2_v1_skproject_alloc_temp_rect(
    DM2_V1_SkprojectTempRectRing *ring,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    DM2_V1_SkprojectTempRectReceipt *out_receipt)
{
    uint8_t slot;
    DM2_V1_SkprojectRect *rect;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!ring || !out_receipt || ring->next_index >= 4u) return 0;

    slot = ring->next_index;
    rect = &ring->rects[slot];
    ring->next_index = (uint8_t)((slot + 1u) & 3u);

    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;

    out_receipt->valid = 1;
    out_receipt->slot = slot;
    out_receipt->next_slot = ring->next_index;
    out_receipt->rect = *rect;
    out_receipt->receipt_hash =
        dm2_v1_skproject_rect_hash(slot, ring->next_index, rect);
    return 1;
}

int dm2_v1_skproject_alloc_temp_origin_rect(
    DM2_V1_SkprojectTempRectRing *ring,
    int16_t w,
    int16_t h,
    DM2_V1_SkprojectTempRectReceipt *out_receipt)
{
    return dm2_v1_skproject_alloc_temp_rect(ring, 0, 0, w, h, out_receipt);
}

const char *dm2_v1_skproject_core_source_evidence(void)
{
    return "skproject SKWINSPX/src/v4/skcore.cpp "
           "ALLOC_TEMP_RECT/ALLOC_TEMP_ORIGIN_RECT/BETWEEN_VALUE; "
           "SKWINSPX/src/v5/skrect.cpp alloc_tmprect/alloc_origin_tmprect; "
           "SKWINSPX/src/v5/util.cpp DM2_BETWEEN_VALUE";
}
