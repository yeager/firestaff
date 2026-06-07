#ifndef DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_C007_PC34_COMPAT 7
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_C011_PC34_COMPAT 11
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_CLOSE_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_NONE_PC34_COMPAT (-1)

typedef enum Dm1V1MirrorCandidateInventoryToggleRoutePc34Compat {
    DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_NOT_IN_INVENTORY_TOGGLE_RANGE_PC34_COMPAT = 0,
    DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_DISPATCHED_CLOSED_INVENTORY_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_DISPATCHED_PARTY_CHAMPION_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_BLOCKED_BY_G0299_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_BLOCKED_OUT_OF_PARTY_PC34_COMPAT
} Dm1V1MirrorCandidateInventoryToggleRoutePc34Compat;

typedef struct Dm1V1MirrorCandidateInventoryToggleInputPc34Compat {
    int command;
    unsigned int candidate_champion_ordinal;
    unsigned int party_champion_count;
    int current_inventory_champion_ordinal;
} Dm1V1MirrorCandidateInventoryToggleInputPc34Compat;

typedef struct Dm1V1MirrorCandidateInventoryToggleEvidencePc34Compat {
    const char *commandGateAnchor;
    const char *defsCommandAnchor;
    const char *defsChampionAnchor;
    const char *defsGlobalsAnchor;
    const char *defsInventoryOrdinalAnchor;
    const char *toggleEntrypointAnchor;
    const char *spellActionGateAnchor;
    const char *contractScope;
    const char *disjointFunctions;
} Dm1V1MirrorCandidateInventoryToggleEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat {
    int should_dispatch_toggle;
    int target_champion_index;
    Dm1V1MirrorCandidateInventoryToggleRoutePc34Compat route_taken;
    const Dm1V1MirrorCandidateInventoryToggleEvidencePc34Compat *evidence;
    int command_in_inventory_toggle_range;
    int command_range_low;
    int command_range_high;
    int computed_champion_index;
    int close_inventory_index;
    int is_close_inventory_command;
    int champion_index_inside_party;
    int party_gate_passed;
    int candidate_gate_passed;
    int would_call_f0355;
    int inventory_ordinal_before;
    int inventory_ordinal_after;
    int contract_only;
} Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat;

int dm1_v1_mirror_candidate_inventory_toggle_pc34_compat_probe(
    const Dm1V1MirrorCandidateInventoryToggleInputPc34Compat *input,
    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat *output);

const Dm1V1MirrorCandidateInventoryToggleEvidencePc34Compat *
dm1_v1_mirror_candidate_inventory_toggle_pc34_compat_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
