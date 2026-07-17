#include "nexus_v1_dgn_multi_level_capture_adjudicator.h"
#include <string.h>

int nexus_v1_dgn_multi_level_capture_adjudicate(
    const Nexus_V1_DgnMultiLevelCaptureAdjudicationInput *in,
    Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt *out)
{
    Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt receipt;
    uint32_t i, j;
    memset(&receipt, 0, sizeof(receipt));
    receipt.opaque_original_capture_only = 1;
    if (!out || !in) { if (out) *out = receipt; return 0; }
    for (i = 0; i < NEXUS_V1_DGN_MULTI_LEVEL_CAPTURE_LEVEL_COUNT; ++i) {
        const Nexus_V1_Structure1FCorpusTraceTarget *a = in->levels[i].structure1f;
        const Nexus_V1_Prs3DgnPlacementAdapterReceipt *b = in->levels[i].structure2;
        const Nexus_V1_Structure3FaceTexturingCaptureTarget *c = in->levels[i].structure3;
        if (!a || !b || !c || !a->valid || !a->original_saturn_trace_required ||
            !a->no_draw_only || a->fallback_visuals_permitted || !b->valid ||
            !b->trace_fnv1a64 || !b->trace_size || !b->dgn_placement_observed ||
            !b->no_draw_only || b->fallback_visuals_permitted || !c->valid ||
            !c->original_saturn_capture_required || !c->capture_only ||
            c->pixel_semantics_permitted || c->geometry_semantics_permitted ||
            c->draw_permitted || a->level_index != i || c->level_index != i ||
            a->dgn_fnv1a64 != b->dgn_fnv1a64 || a->dgn_fnv1a64 != c->dgn_fnv1a64 ||
            a->descriptor_index != b->descriptor_index || a->descriptor_index != c->descriptor_index ||
            a->descriptor_fnv1a64 != b->descriptor_fnv1a64 ||
            a->descriptor_fnv1a64 != c->descriptor_fnv1a64 || !c->frame_sequence ||
            !c->command_sequence || c->frame_sequence != b->frame_sequence ||
            c->command_sequence != b->command_sequence) { *out = receipt; return 0; }
        for (j = 0; j < i; ++j) {
            if (receipt.levels[j].dgn_fnv1a64 == a->dgn_fnv1a64 ||
                (receipt.levels[j].frame_sequence == c->frame_sequence &&
                 receipt.levels[j].command_sequence == c->command_sequence)) {
                *out = receipt; return 0;
            }
        }
        receipt.levels[i].valid = receipt.levels[i].opaque_original_capture_covered = 1;
        receipt.levels[i].level_index = i;
        receipt.levels[i].dgn_fnv1a64 = a->dgn_fnv1a64;
        receipt.levels[i].descriptor_index = a->descriptor_index;
        receipt.levels[i].trace_fnv1a64 = b->trace_fnv1a64;
        receipt.levels[i].trace_size = b->trace_size;
        receipt.levels[i].frame_sequence = c->frame_sequence;
        receipt.levels[i].command_sequence = c->command_sequence;
    }
    receipt.valid = 1; *out = receipt; return 1;
}
