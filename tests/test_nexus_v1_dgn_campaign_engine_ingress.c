#include "nexus_v1_engine.h"
#include "nexus_v1_dgn_multi_level_capture_adjudicator.h"
#include <string.h>
#include <stdio.h>
int main(void) {
    Nexus_V1_Engine e; Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt r;
    memset(&e, 0, sizeof(e)); memset(&r, 0, sizeof(r));
    e.level_loaded = 1; e.game.current_level = 3;
    e.current_level_structure2_source.loaded_dgn_fnv1a64 = 0x123;
    e.external_prs3_placement_valid = 1; e.external_prs3_placement_level = 3;
    e.external_prs3_placement_dgn_fnv1a64 = 0x123;
    e.external_prs3_placement_trace_fnv1a64 = 0x456;
    e.external_prs3_placement_trace_size = 0x789;
    e.external_prs3_placement_frame_sequence = 1;
    e.external_prs3_placement_command_sequence = 2;
    r.valid = r.opaque_original_capture_only = 1;
    r.levels[3].valid = r.levels[3].opaque_original_capture_covered = 1;
    r.levels[3].dgn_fnv1a64 = 0x123; r.levels[3].trace_fnv1a64 = 0x456;
    r.levels[3].trace_size = 0x789; r.levels[3].frame_sequence = 1;
    r.levels[3].command_sequence = 2;
    if (!nexus_v1_engine_set_dgn_multi_level_capture_adjudication(&e, &r) ||
        !nexus_v1_engine_current_level_dgn_capture_ready(&e)) return 1;
    e.external_prs3_placement_trace_fnv1a64++;
    if (nexus_v1_engine_current_level_dgn_capture_ready(&e)) return 1;
    e.external_prs3_placement_trace_fnv1a64 = 0x456;
    e.external_prs3_placement_trace_size++;
    if (nexus_v1_engine_current_level_dgn_capture_ready(&e)) return 1;
    e.external_prs3_placement_trace_size = 0x789; e.game.current_level = 4;
    if (nexus_v1_engine_current_level_dgn_capture_ready(&e)) return 1;
    e.game.current_level = 3; e.current_level_structure2_source.loaded_dgn_fnv1a64 = 0x124;
    if (nexus_v1_engine_current_level_dgn_capture_ready(&e)) return 1;
    puts("dgn campaign engine ingress: PASS"); return 0;
}
