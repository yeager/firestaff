#ifndef FIRESTAFF_DM1_V1_CHEST_TELEPORTER_SURVIVAL_OPEN_G0426_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_TELEPORTER_SURVIVAL_OPEN_G0426_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int assertions;
    int failures;
    uint64_t deterministic_hash;
    int g0426_kept_open;
    int leader_hand_preserved;
    int chest_slot_count_preserved;
    int teleporter_activations;
    int mutation_rejections;

    int contract_only;
    int command_dispatches;
    int audible_buzzes;
    int creature_only_transition_blocked;
    int stacked_pair_preserved;
    int visible_chain_pixels_preserved;
    int initial_chest_slot_count;
    int final_chest_slot_count;
    int initial_stack_count;
    int final_stack_count;
    int initial_visible_chain_pixels;
    int final_visible_chain_pixels;
    int initial_leader_hand_thing;
    int final_leader_hand_thing;
    int final_map_index;
    int final_map_x;
    int final_map_y;
    const char* source_anchors;
} DM1_V1_ChestTeleporterSurvivalOpenG0426SelfTestResultPc34;

int run_dm1_v1_chest_teleporter_survival_open_g0426_self_test(void);

const DM1_V1_ChestTeleporterSurvivalOpenG0426SelfTestResultPc34*
dm1_v1_chest_teleporter_survival_open_g0426_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
