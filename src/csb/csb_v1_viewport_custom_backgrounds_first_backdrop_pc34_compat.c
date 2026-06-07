#include "csb_v1_viewport_custom_backgrounds_first_backdrop_pc34_compat.h"

#include <string.h>

static const char s_f0128_viewport_anchor[] =
    "ReDMCSB DUNVIEW.C F0128:8318-8542 viewport order; CSB "
    "CustomBackgrounds first backdrop is the first CSB-only backdrop pass "
    "after the F0098 base floor/ceiling copy and before near cell drawing.";

static const char s_f0098_base_anchor[] =
    "ReDMCSB DUNVIEW.C F0098:2962-3002 copies base ceiling/floor pixels "
    "and clears G0297_B_DrawFloorAndCeilingRequested before CSB backdrop "
    "overlays.";

static const char s_f0128_keep_out_anchor[] =
    "ReDMCSB DUNVIEW.C F0128:8337-8339 calls F0098 for the backdrop "
    "keep-out; first CustomBackgrounds remains outside F0107/F0108/F0111/"
    "F0115 cell routes.";

static const char s_defs_c10_anchor[] =
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH; first backdrop must preserve "
    "C10 transparency for later cell blits.";

static const char s_defs_view_square_anchor[] =
    "ReDMCSB DEFS.H:2595-2611 I34E viewport square constants; "
    "C14_VIEW_SQUARE_D3L2 and C15_VIEW_SQUARE_D3R2 are distinct from "
    "the first backdrop room ordinal.";

static const char s_csb_lineage_first_dispatch_anchor[] =
    "ReDMCSB CSB-lineage Viewport.cpp:6503-6551 CustomBackgrounds; 6840 "
    "room 0 dispatch is first, 6841 room 2 is the fallback/second backdrop.";

static const char s_csb_lineage_required_anchors[] =
    "ReDMCSB CSB-lineage Viewport.cpp:1853-1862,1881-1888,1895,1899,"
    "1922,1926 door-facing draw-code anchors; 2614-2620 draw-order/decor "
    "test; 6503-6551 CustomBackgrounds; 6840-6841 first/second dispatch.";

static const char s_csb_lineage_default_skin_anchor[] =
    "ReDMCSB CSB-lineage Viewport.cpp:322 NUMCELL=16; CSB.h:141-154 "
    "SKIN_CACHE defaultSkins/GetDefaultSkin; Viewport.cpp:6521-6524 "
    "default skin is the fallback when the cell skin is zero.";

static const char s_non_overlap_note[] =
    "ReDMCSB non_overlap_with_second_backdrop=1; first backdrop room 0 "
    "uses rel fwd=3 side=-2 and keep-out ordinal 0; second backdrop room "
    "2 uses rel fwd=3 side=-1 and keep-out ordinal 2.";

static const char s_source_evidence[] =
    "contract_only=1; ReDMCSB DUNVIEW.C F0128:8318-8542 viewport order; "
    "F0098:2962-3002 base pixels first; F0128:8337-8339 backdrop keep-out; "
    "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2595-2611 view-square constants; "
    "CSB-lineage Viewport.cpp:322 NUMCELL=16, 6503-6551 CustomBackgrounds, "
    "6521-6524 default skin fallback, 6840 room 0 before 6841 room 2; "
    "required anchors Viewport.cpp:1853-1862,1881-1888,1895,1899,1922,1926,"
    "2614-2620; no F0107/F0108/F0111/F0115 route.";

static const CSB_V1_CustomBackgroundsFirstBackdropContract s_contract = {
    1,
    0,
    0,
    2,
    1,
    1,
    3,
    -2,
    3,
    -1,
    0,
    2,
    1,
    1,
    1,
    1,
    10,
    1,
    0,
    0,
    0,
    0,
    16,
    1,
    18,
    0,
    4,
    7840,
    64,
    2,
    6,
    3248,
    64,
    1,
    5,
    4144,
    20,
    5,
    s_f0128_viewport_anchor,
    s_f0098_base_anchor,
    s_f0128_keep_out_anchor,
    s_defs_c10_anchor,
    s_defs_view_square_anchor,
    s_csb_lineage_first_dispatch_anchor,
    s_csb_lineage_required_anchors,
    s_csb_lineage_default_skin_anchor,
    s_non_overlap_note,
    s_source_evidence
};

static const CSB_V1_CustomBackgroundsFirstBackdropStep s_order[] = {
    CSB_V1_FIRST_BACKDROP_STEP_F0098_BASE_PIXELS,
    CSB_V1_FIRST_BACKDROP_STEP_FIRST_CUSTOM_BACKGROUND,
    CSB_V1_FIRST_BACKDROP_STEP_SECOND_CUSTOM_BACKGROUND_FALLBACK,
    CSB_V1_FIRST_BACKDROP_STEP_F0128_BACKDROP_KEEP_OUT
};

const CSB_V1_CustomBackgroundsFirstBackdropContract *
csb_v1_viewport_custom_backgrounds_first_backdrop_contract_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 supplies the shared
     * viewport ordering anchor. CSB-lineage Viewport.cpp lines 6840-6841
     * dispatch CustomBackgrounds room 0 before room 2; lines 6503-6551
     * define skin/default/masked backdrop application. */
    return &s_contract;
}

size_t csb_v1_viewport_custom_backgrounds_first_backdrop_order_pc34(
    CSB_V1_CustomBackgroundsFirstBackdropStep *out_steps,
    size_t out_capacity)
{
    const size_t count = sizeof(s_order) / sizeof(s_order[0]);

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 writes base pixels first,
     * while F0128 lines 8337-8339 keeps that backdrop pass separate from
     * CSB-only CustomBackgrounds. CSB-lineage Viewport.cpp lines 6840-6841
     * fixes first room 0 before second room 2. */
    if (out_steps && out_capacity > 0) {
        const size_t copy_count = out_capacity < count ? out_capacity : count;
        memcpy(out_steps, s_order, copy_count * sizeof(s_order[0]));
    }

    return count;
}

const char *
csb_v1_viewport_custom_backgrounds_first_backdrop_source_evidence_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 and F0098 lines 2962-3002
     * anchor the baseline; DEFS.H line 2088 anchors C10 preservation;
     * CSB-lineage Viewport.cpp lines 6503-6551 and 6840-6841 anchor the
     * first-backdrop dispatch. */
    return s_source_evidence;
}

int csb_v1_viewport_custom_backgrounds_first_backdrop_preserve_c10_pc34(
    const unsigned char *source,
    unsigned char *destination,
    size_t count)
{
    size_t i;
    int copied = 0;

    /* ReDMCSB: DEFS.H line 2088 names C10_COLOR_FLESH. This synthetic
     * helper proves the first-backdrop gate keeps C10 transparent pixels
     * available for later F0107/F0108/F0111/F0115 routes without entering
     * those routes. */
    if (!source || !destination) {
        return -1;
    }
    for (i = 0; i < count; ++i) {
        if (source[i] == (unsigned char)s_contract.c10_transparent_color) {
            continue;
        }
        destination[i] = source[i];
        ++copied;
    }
    return copied;
}
