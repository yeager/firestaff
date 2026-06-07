#include "csb_v1_viewport_custom_backgrounds_second_backdrop_pc34_compat.h"

#include <string.h>

static const char s_f0128_viewport_anchor[] =
    "ReDMCSB DUNVIEW.C F0128:8318-8542 viewport pass order; CSB "
    "CustomBackgrounds second backdrop is a contract-only unmasked baseline "
    "extension before near cell drawing.";

static const char s_f0098_base_anchor[] =
    "ReDMCSB DUNVIEW.C F0098:2962-3002 copies base ceiling/floor pixels "
    "and clears G0297_B_DrawFloorAndCeilingRequested before later overlays.";

static const char s_f0128_keep_out_anchor[] =
    "ReDMCSB DUNVIEW.C F0128:8337-8339 calls F0098 for the backdrop "
    "keep-out; this is the only masked-overlay keep-out path in this "
    "contract and has no CSB near-layer substitute.";

static const char s_f0128_second_backdrop_anchor[] =
    "ReDMCSB DUNVIEW.C F0128:8318-8542 second/backing backdrop keep-out; "
    "the CSB second CustomBackgrounds enumeration remains baseline-backed "
    "and does not replace F0128:8337-8339 with a near-layer path.";

static const char s_csb_lineage_index_path[] =
    "CSB-lineage CustomBackgrounds index path: CSBWin Viewport.cpp:6919 "
    "CustomBackgrounds(..., room 0) then 6920 CustomBackgrounds(..., room 2) "
    "before the F3L1 draw path; Viewport.cpp:6567-6615 applies the runtime "
    "large/middle/near background masks.";

static const char s_source_evidence[] =
    "contract_only=1; ReDMCSB DUNVIEW.C F0128:8318-8542 viewport order; "
    "F0098:2962-3002 base pixels first; F0128:8337-8339 backdrop keep-out; "
    "CSBWin Viewport.cpp:6919-6920 second CustomBackgrounds enumeration "
    "and 6567-6615 CustomBackgrounds mask application.";

static const CSB_V1_CustomBackgroundsSecondBackdropContract s_contract = {
    1,
    1,
    2,
    1,
    1,
    1,
    1,
    0,
    s_f0128_viewport_anchor,
    s_f0098_base_anchor,
    s_f0128_keep_out_anchor,
    s_f0128_second_backdrop_anchor,
    s_csb_lineage_index_path,
    s_source_evidence
};

static const CSB_V1_CustomBackgroundsSecondBackdropStep s_order[] = {
    CSB_V1_SECOND_BACKDROP_STEP_F0098_BASE_PIXELS,
    CSB_V1_SECOND_BACKDROP_STEP_FIRST_CUSTOM_BACKGROUND,
    CSB_V1_SECOND_BACKDROP_STEP_SECOND_CUSTOM_BACKGROUND,
    CSB_V1_SECOND_BACKDROP_STEP_F0128_BACKDROP_KEEP_OUT
};

const CSB_V1_CustomBackgroundsSecondBackdropContract *
csb_v1_viewport_custom_backgrounds_second_backdrop_contract_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 supplies the viewport
     * ordering anchor. CSBWin Viewport.cpp lines 6919-6920 are the
     * CSB-lineage CustomBackgrounds index path for the second/backing
     * backdrop; this slice records the contract only. */
    return &s_contract;
}

size_t csb_v1_viewport_custom_backgrounds_second_backdrop_order_pc34(
    CSB_V1_CustomBackgroundsSecondBackdropStep *out_steps,
    size_t out_capacity)
{
    const size_t count = sizeof(s_order) / sizeof(s_order[0]);

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 writes base pixels first,
     * while F0128 lines 8337-8339 keep the backdrop pass separate from
     * CSB-only masked CustomBackgrounds overlays. */
    if (out_steps && out_capacity > 0) {
        const size_t copy_count = out_capacity < count ? out_capacity : count;
        memcpy(out_steps, s_order, copy_count * sizeof(s_order[0]));
    }

    return count;
}

const char *
csb_v1_viewport_custom_backgrounds_second_backdrop_source_evidence_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 and F0098 lines 2962-3002
     * are the source-lock anchors; CSBWin Viewport.cpp lines 6919-6920 cite
     * the CSB-lineage second CustomBackgrounds enumeration order. */
    return s_source_evidence;
}
