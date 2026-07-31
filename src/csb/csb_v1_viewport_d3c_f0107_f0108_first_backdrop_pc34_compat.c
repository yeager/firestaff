#include "csb_v1_viewport_d3c_f0107_f0108_first_backdrop_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_VIEW_SQUARE_D3C = 11,
    CSB_VIEW_WALL_D3C_FRONT = 5,
    CSB_VIEW_FLOOR_D3C = 3,
    CSB_ZONE_WALL_ORNAMENT = 1004,
    CSB_WALL_COORDINATE_SET_STRIDE = 15,
    CSB_ZONE_FLOOR_ORNAMENT = 1500,
    CSB_FLOOR_COORDINATE_SET_STRIDE = 11,
    CSB_TRANSPARENT_COLOR = 10
};

static const char s_source_evidence[] =
    "Contract-only routing and geometry gate; no pixel composition or real-asset "
    "bitmap parity is claimed. ReDMCSB DRAWVIEW.C:709-722 F0097 commits the "
    "viewport. DUNVIEW.C F0098:2962-3002 and F0128:8337-8339 place base "
    "backdrop before D3C routes. F0107:3502-3938 calculates wall-ornament "
    "zones at 3587 and uses C10 transparency; F0108:3940-4011 calculates "
    "floor zones at 3998 and preserves C10 transparency. F0118:6642-6763 "
    "calls F0107 at 6716 and F0108 at 6722/6814. CSBWin Viewport.cpp "
    "1192-1209,1903-1915,6507-6548,6800-6840 cross-checks the draw order.";

static const CSB_V1_ViewportD3cF0107F0108FirstBackdropPc34Contract s_contract = {
    CSB_PRESENT, CSB_PRESENT,
    CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_WIDTH_PC34,
    CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_HEIGHT_PC34,
    CSB_VIEW_SQUARE_D3C, CSB_VIEW_WALL_D3C_FRONT, CSB_VIEW_FLOOR_D3C,
    CSB_ZONE_WALL_ORNAMENT, CSB_WALL_COORDINATE_SET_STRIDE,
    CSB_ZONE_FLOOR_ORNAMENT, CSB_FLOOR_COORDINATE_SET_STRIDE,
    CSB_TRANSPARENT_COLOR, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
    { 74, 25, 149, 75 }, { 88, 35, 135, 64 }, { 96, 57, 127, 80 },
    "ReDMCSB DRAWVIEW.C:709-722 F0097_DUNGEONVIEW_DrawViewport",
    "ReDMCSB DUNVIEW.C F0098:2962-3002; F0128:8337-8339",
    "ReDMCSB DUNVIEW.C F0107:3502-3938; zone 3587; blit 3922",
    "ReDMCSB DUNVIEW.C F0108:3940-4011; zone 3998; mask recursion 4007-4008",
    "ReDMCSB DUNVIEW.C F0118:6642-6763; F0107 at 6716; F0108 at 6722/6814",
    "ReDMCSB DUNVIEW.C F0115:4547-4581",
    "ReDMCSB DUNVIEW.C F0127:8294 post-D3C wall/thing follow-up anchor",
    "ReDMCSB DUNVIEW.C F0128:8478-8508 D3C dispatch and view commit sequence",
    "ReDMCSB DEFS.H:4222-4223 zones; 2749-2760 floor views",
    "CSBWin Viewport.cpp:1192-1209,1903-1915,6507-6548,6800-6840",
    s_source_evidence
};

const CSB_V1_ViewportD3cF0107F0108FirstBackdropPc34Contract *
csb_v1_viewport_d3c_f0107_f0108_first_backdrop_contract_pc34(void)
{
    return &s_contract;
}

const char *
csb_v1_viewport_d3c_f0107_f0108_first_backdrop_source_evidence_pc34(void)
{
    return s_source_evidence;
}
