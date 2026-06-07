#include "dm1_v1_viewport_d3c_center_field_pc34_compat.h"

/*
 * Contract-only source-lock gate; this file does not claim real-asset pixels.
 */

static const char s_f0098_anchor[] =
    "ReDMCSB DUNVIEW.C F0098:2962-3003 owns floor/ceiling before F0128 square dispatch";

static const char s_f0128_anchor[] =
    "ReDMCSB DUNVIEW.C F0128:8488-8499 dispatches depth 3 lane 0 to F0118_DUNGEONVIEW_DrawSquareD3C_CPSF";

static const char s_f0124_anchor[] =
    "ReDMCSB DUNVIEW.C F0118:6811-6831 D3C no-wall center-field path; analogous no-wall reference DUNVIEW.C F0124:7922-7955";

static const char s_f0113_anchor[] =
    "ReDMCSB DUNVIEW.C F0113:6213-6219 field draw helper and DUNVIEW.C F0118:6825-6831 D3C teleporter field call";

static const char s_defs_view_square_anchor[] =
    "ReDMCSB DEFS.H:2595-2611 I34E view-square constants, M600_VIEW_SQUARE_D3C=11 and C14/C15 side indices";

static const char s_defs_zone_anchor[] =
    "ReDMCSB DEFS.H:3432-3437 wall ordinals, DEFS.H:4030-4049 zone family, DEFS.H:4044 C704_ZONE_WALL_D3C";

static const char s_defs_c10_anchor[] =
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH transparency color used by field/bitmap blits";

static const char s_non_overlap_note[] =
    "contract_only=1 DUNVIEW.C F0128 D3C center-field gate is separate from "
    "DUNVIEW.C F0118 wall return metadata and DUNVIEW.C F0124 adjacent "
    "no-wall reference; DUNVIEW.C F0113 teleporter field stays after the "
    "D3C no-wall thing handoff. DEFS.H:2595-2611 binds M600 D3C, "
    "DEFS.H:2088 C10_COLOR_FLESH preserves transparency, and "
    "DEFS.H:3432/4030/4049 anchors the wall/zone families. This slice has "
    "no F0100/F0105/F0107/F0111 center-field routing and records the "
    "F0115 thing-pass only after F0113 guard phrase without claiming pixels.";

static const char s_source_summary[] =
    "contract_only=1 source-lock for DUNVIEW.C F0128:8488-8499 dispatch of "
    "D3C depth 3 lane 0 into F0118; DUNVIEW.C F0124:7922-7955 is cited as "
    "the adjacent no-wall center-field reference while DUNVIEW.C F0118:6811-6831 "
    "is the actual D3C path. DUNVIEW.C F0113:6213-6219 and F0118:6825-6831 "
    "own the field overlay. DEFS.H:2595-2611 gives M600_VIEW_SQUARE_D3C=11, "
    "G2035 at DUNVIEW.C:377 maps that view square to field aspect 2, "
    "DEFS.H:2088 C10_COLOR_FLESH keeps transparent pixels transparent, and "
    "DEFS.H:3432/4030/4049 plus DEFS.H:4044 bind the wall/zone families. "
    "The center-field branch has no F0100/F0105/F0107/F0111 route; F0115 "
    "thing-pass only after F0113 is preserved as a non-overlap guard phrase.";

static const Dm1V1ViewportD3cCenterFieldContractPc34Compat s_contract = {
    1,
    DM1_V1_D3C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX_D3C,
    DM1_V1_D3C_CENTER_FIELD_PC34_LANE,
    DM1_V1_D3C_CENTER_FIELD_PC34_DEPTH,
    DM1_V1_D3C_CENTER_FIELD_PC34_FIELD_ASPECT,
    1,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    s_f0128_anchor,
    s_f0124_anchor,
    s_f0113_anchor,
    s_defs_view_square_anchor,
    s_defs_zone_anchor,
    s_defs_c10_anchor,
    s_non_overlap_note,
    s_source_summary
};

static const Dm1V1ViewportD3cCenterFieldStepPc34Compat s_steps[] = {
    {
        DM1_V1_D3C_CENTER_FIELD_STEP_PC34_F0098_FLOOR_CEILING,
        "F0098 floor/ceiling ownership",
        s_f0098_anchor
    },
    {
        DM1_V1_D3C_CENTER_FIELD_STEP_PC34_F0128_DISPATCH,
        "F0128 depth 3 lane 0 dispatch",
        s_f0128_anchor
    },
    {
        DM1_V1_D3C_CENTER_FIELD_STEP_PC34_F0118_NO_WALL,
        "F0118 D3C no-wall center-field route",
        s_f0124_anchor
    },
    {
        DM1_V1_D3C_CENTER_FIELD_STEP_PC34_F0115_THINGS,
        "F0115 D3C thing pass",
        "ReDMCSB DUNVIEW.C F0118:6811-6816 routes D3C no-wall cells into F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"
    },
    {
        DM1_V1_D3C_CENTER_FIELD_STEP_PC34_F0113_FIELD,
        "F0113 D3C field overlay",
        s_f0113_anchor
    }
};

const Dm1V1ViewportD3cCenterFieldContractPc34Compat *
dm1_v1_viewport_d3c_center_field_contract_pc34_compat(void)
{
    return &s_contract;
}

size_t dm1_v1_viewport_d3c_center_field_steps_pc34_compat(
    Dm1V1ViewportD3cCenterFieldStepPc34Compat *out,
    size_t cap)
{
    size_t i;
    const size_t count = sizeof(s_steps) / sizeof(s_steps[0]);
    const size_t n = cap < count ? cap : count;

    if (out) {
        for (i = 0; i < n; ++i) {
            out[i] = s_steps[i];
        }
    }

    return count;
}
