#include "csb_v1_viewport_custom_backgrounds_both_backdrops_pc34_compat.h"

static const char s_f0128_anchor[] =
    "ReDMCSB DUNVIEW.C F0128:8318-8542 viewport order; first then second "
    "CSB CustomBackgrounds composition follows room 0 before room 2 after "
    "the F0098 first base pass and before near cell drawing.";

static const char s_f0098_anchor[] =
    "ReDMCSB DUNVIEW.C F0098:2962-3002 draws/copies the base floor and "
    "ceiling pixels first and clears G0297_B_DrawFloorAndCeilingRequested "
    "before either CustomBackgrounds backdrop.";

static const char s_f0128_keep_out_anchor[] =
    "ReDMCSB DUNVIEW.C F0128:8337-8339 invokes the backdrop keep-out through "
    "F0098; the room 0 side=-2 and room 2 side=-1 CSB backdrops remain "
    "outside F0107/F0108/F0111/F0115 cell routes.";

static const char s_defs_c10_anchor[] =
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH with DEFS.H:2595-2611 I34E "
    "view-square constants; C10 transparency is preserved across both "
    "contract-only CustomBackgrounds backdrops.";

static const char s_csb_lineage_first_dispatch_anchor[] =
    "CSB-lineage Viewport.cpp:6840 first dispatch CustomBackgrounds room 0; "
    "Viewport.cpp:6503-6551 CustomBackgrounds and Viewport.cpp:6521-6524 "
    "default-skin fallback define the source route.";

static const char s_csb_lineage_second_dispatch_anchor[] =
    "CSB-lineage Viewport.cpp:6841 second dispatch CustomBackgrounds room 2; "
    "Viewport.cpp:6503-6551 CustomBackgrounds and Viewport.cpp:6521-6524 "
    "default-skin fallback define the source route.";

static const char s_non_overlap_note[] =
    "contract_only=1; room 0 first backdrop rel fwd=3 side=-2 keep-out 0; "
    "room 2 second backdrop rel fwd=3 side=-1 keep-out 2; "
    "non_overlap_with_second_backdrop=1 because rooms, sides, and keep-out "
    "ordinals are distinct; DUNVIEW.C F0128; DUNVIEW.C F0098; "
    "DUNVIEW.C F0128:8337-8339; DEFS.H:2088 C10_COLOR_FLESH; "
    "DEFS.H:2595-2611; Viewport.cpp:6840; Viewport.cpp:6841; "
    "Viewport.cpp:6503-6551 CustomBackgrounds; no F0107/F0108/F0111/F0115; "
    "first then second; F0098 first.";

static const char s_source_summary[] =
    "contract_only=1; room 0 then room 2 compose side=-2 then side=-1; "
    "non_overlap_with_second_backdrop=1; DUNVIEW.C F0128:8318-8542 orders "
    "the viewport; DUNVIEW.C F0098:2962-3002 provides F0098 first base "
    "floor/ceiling; DUNVIEW.C F0128:8337-8339 records the backdrop keep-out; "
    "DEFS.H:2088 C10_COLOR_FLESH and DEFS.H:2595-2611 anchor transparency "
    "and I34E view squares; Viewport.cpp:6840 and Viewport.cpp:6841 dispatch "
    "first then second; Viewport.cpp:6503-6551 CustomBackgrounds plus "
    "Viewport.cpp:6521-6524 default-skin fallback provide the CSB-lineage "
    "route; no F0107/F0108/F0111/F0115; first_routes=0/0/0/0; "
    "second_routes=0/0/0/0.";

static const CSB_V1_CustomBackgroundsBothBackdropsContract s_contract = {
    1,
    0,
    3,
    -2,
    0,
    2,
    3,
    -1,
    2,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    s_f0128_anchor,
    s_f0098_anchor,
    s_f0128_keep_out_anchor,
    s_defs_c10_anchor,
    s_csb_lineage_first_dispatch_anchor,
    s_csb_lineage_second_dispatch_anchor,
    s_non_overlap_note,
    s_source_summary
};

const CSB_V1_CustomBackgroundsBothBackdropsContract *
csb_v1_viewport_custom_backgrounds_both_backdrops_contract_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 establishes viewport order,
     * DUNVIEW.C F0098 lines 2962-3002 draws the base first, and
     * DUNVIEW.C F0128 lines 8337-8339 anchors the backdrop keep-out. */
    return &s_contract;
}
