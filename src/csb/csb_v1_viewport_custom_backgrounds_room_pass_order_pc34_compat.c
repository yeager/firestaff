#include "csb_v1_viewport_custom_backgrounds_room_pass_order_pc34_compat.h"

#include <string.h>

static const char s_redmcsb_f0128_anchor[] =
    "ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542 orders "
    "F0098, D4 F0115, D3L/D3R/D3C, then D2L/D2R/D2C room passes.";

static const char s_redmcsb_f0098_anchor[] =
    "ReDMCSB DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002 "
    "draws G2109_Ceiling/G2108_Floor and clears G0297 before the room pass.";

static const char s_redmcsb_f0115_anchor[] =
    "ReDMCSB DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4547 "
    "is invoked by D3/D2 room draw functions after their room slot is selected.";

static const char s_csb_applybackground_anchor[] =
    "CSB-lineage Viewport.cpp:6451-6505 ApplyBackground masked composite.";

static const char s_csb_custombackgrounds_anchor[] =
    "CSB-lineage Viewport.cpp:6574-6622 CustomBackgrounds resolves the "
    "roomNum skin and applies large, middle, then near background bitmaps.";

static const char s_csb_room_dispatch_anchor[] =
    "CSB-lineage Viewport.cpp:6926-7045 dispatches CustomBackgrounds "
    "rooms 0/2, 1/3, 4, 5/7, 6/8, and 9 before the matching D3/D2 draw.";

static const char s_source_summary[] =
    "contract_only=1; ReDMCSB DUNVIEW.C F0128:8318-8542, F0098:2962-3002, "
    "F0115:4547; D3L F0115 lines 6444/6480; D3R 6580/6622; "
    "D3C 6723/6816; D2L 6989/7031; D2R 7182/7224; D2C 7315/7368; "
    "CSB-lineage Viewport.cpp:6451-6505 ApplyBackground; "
    "Viewport.cpp:6574-6622 CustomBackgrounds; Viewport.cpp:6926-7045 "
    "room dispatch; CustomBackgrounds precedes room draw and F0115 thing pass.";

static const CSB_V1_CustomBackgroundsRoomPassEvent s_events[] = {
    { 0, CSB_V1_ROOM_PASS_EVENT_F0098_BASELINE, -1, -1, -1, -1,
      8337, 0, "F0098 floor/ceiling baseline", s_redmcsb_f0098_anchor },
    { 1, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 0, 14, 3, -2,
      8481, 6926, "CustomBackgrounds D3L2", s_csb_room_dispatch_anchor },
    { 2, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 2, 12, 3, -1,
      8490, 6927, "CustomBackgrounds D3L", s_csb_room_dispatch_anchor },
    { 3, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, 2, 12, 3, -1,
      8491, 6942, "Draw D3L room", s_redmcsb_f0128_anchor },
    { 4, CSB_V1_ROOM_PASS_EVENT_F0115_THING_PASS, 2, 12, 3, -1,
      6444, 6942, "F0115 D3L things", s_redmcsb_f0115_anchor },
    { 5, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 1, 15, 3, 2,
      8485, 6947, "CustomBackgrounds D3R2", s_csb_room_dispatch_anchor },
    { 6, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 3, 13, 3, 1,
      8494, 6948, "CustomBackgrounds D3R", s_csb_room_dispatch_anchor },
    { 7, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, 3, 13, 3, 1,
      8495, 6963, "Draw D3R room", s_redmcsb_f0128_anchor },
    { 8, CSB_V1_ROOM_PASS_EVENT_F0115_THING_PASS, 3, 13, 3, 1,
      6580, 6963, "F0115 D3R things", s_redmcsb_f0115_anchor },
    { 9, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 4, 11, 3, 0,
      8498, 6968, "CustomBackgrounds D3C", s_csb_room_dispatch_anchor },
    { 10, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, 4, 11, 3, 0,
      8499, 6983, "Draw D3C room", s_redmcsb_f0128_anchor },
    { 11, CSB_V1_ROOM_PASS_EVENT_F0115_THING_PASS, 4, 11, 3, 0,
      6723, 6983, "F0115 D3C things", s_redmcsb_f0115_anchor },
    { 12, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 5, 9, 2, -2,
      8503, 6988, "CustomBackgrounds D2L2", s_csb_room_dispatch_anchor },
    { 13, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 7, 7, 2, -1,
      8512, 6989, "CustomBackgrounds D2L", s_csb_room_dispatch_anchor },
    { 14, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, 7, 7, 2, -1,
      8513, 7004, "Draw D2L room", s_redmcsb_f0128_anchor },
    { 15, CSB_V1_ROOM_PASS_EVENT_F0115_THING_PASS, 7, 7, 2, -1,
      6989, 7004, "F0115 D2L things", s_redmcsb_f0115_anchor },
    { 16, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 6, 10, 2, 2,
      8507, 7009, "CustomBackgrounds D2R2", s_csb_room_dispatch_anchor },
    { 17, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 8, 8, 2, 1,
      8516, 7010, "CustomBackgrounds D2R", s_csb_room_dispatch_anchor },
    { 18, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, 8, 8, 2, 1,
      8517, 7025, "Draw D2R room", s_redmcsb_f0128_anchor },
    { 19, CSB_V1_ROOM_PASS_EVENT_F0115_THING_PASS, 8, 8, 2, 1,
      7182, 7025, "F0115 D2R things", s_redmcsb_f0115_anchor },
    { 20, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 9, 6, 2, 0,
      8520, 7030, "CustomBackgrounds D2C", s_csb_room_dispatch_anchor },
    { 21, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, 9, 6, 2, 0,
      8521, 7045, "Draw D2C room", s_redmcsb_f0128_anchor },
    { 22, CSB_V1_ROOM_PASS_EVENT_F0115_THING_PASS, 9, 6, 2, 0,
      7315, 7045, "F0115 D2C things", s_redmcsb_f0115_anchor }
};

static const CSB_V1_CustomBackgroundsRoomPassOrderContract s_contract = {
    1,
    (int)(sizeof(s_events) / sizeof(s_events[0])),
    1,
    1,
    1,
    1,
    2,
    3,
    4,
    7,
    8,
    9,
    0x7a89e668u,
    s_redmcsb_f0128_anchor,
    s_redmcsb_f0098_anchor,
    s_redmcsb_f0115_anchor,
    s_csb_applybackground_anchor,
    s_csb_custombackgrounds_anchor,
    s_csb_room_dispatch_anchor,
    s_source_summary
};

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int shift;

    for (shift = 0; shift < 32; shift += 8) {
        hash ^= (value >> shift) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

const CSB_V1_CustomBackgroundsRoomPassOrderContract *
csb_v1_viewport_custom_backgrounds_room_pass_order_contract_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 gives D3/D2 room order,
     * F0098 lines 2962-3002 gives the baseline reset, and F0115 line 4547
     * anchors thing drawing inside those room passes. CSB-lineage:
     * Viewport.cpp lines 6926-7045 puts CustomBackgrounds before each
     * matching D3/D2 draw block. */
    return &s_contract;
}

size_t csb_v1_viewport_custom_backgrounds_room_pass_order_trace_pc34(
    CSB_V1_CustomBackgroundsRoomPassEvent *out_events,
    size_t out_capacity)
{
    const size_t count = sizeof(s_events) / sizeof(s_events[0]);

    if (out_events && out_capacity > 0) {
        const size_t copy_count = out_capacity < count ? out_capacity : count;
        memcpy(out_events, s_events, copy_count * sizeof(s_events[0]));
    }
    return count;
}

uint32_t csb_v1_viewport_custom_backgrounds_room_pass_order_hash_pc34(
    const CSB_V1_CustomBackgroundsRoomPassEvent *events,
    size_t event_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!events) {
        return 0;
    }

    for (i = 0; i < event_count; ++i) {
        hash = fnv1a_u32(hash, (uint32_t)events[i].ordinal);
        hash = fnv1a_u32(hash, (uint32_t)events[i].kind);
        hash = fnv1a_u32(hash, (uint32_t)events[i].room_num);
        hash = fnv1a_u32(hash, (uint32_t)events[i].redmcsb_view_square);
        hash = fnv1a_u32(hash, (uint32_t)events[i].relative_forward);
        hash = fnv1a_u32(hash, (uint32_t)events[i].relative_side);
        hash = fnv1a_u32(hash, (uint32_t)events[i].redmcsb_line);
        hash = fnv1a_u32(hash, (uint32_t)events[i].csb_line);
    }
    return hash;
}

int csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
    const CSB_V1_CustomBackgroundsRoomPassEvent *events,
    size_t event_count,
    CSB_V1_CustomBackgroundsRoomPassEventKind kind,
    int room_num)
{
    size_t i;

    if (!events) {
        return -1;
    }

    for (i = 0; i < event_count; ++i) {
        if (events[i].kind == kind && events[i].room_num == room_num) {
            return (int)i;
        }
    }
    return -1;
}

const char *
csb_v1_viewport_custom_backgrounds_room_pass_order_source_evidence_pc34(void)
{
    return s_source_summary;
}
