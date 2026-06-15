#ifndef FIRESTAFF_DM1_V1_CHEST_PICKUP_AT_CAPACITY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PICKUP_AT_CAPACITY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int contract_only;
    int chest_visible_capacity;
    int chest_filled_slots_at_start;
    int leader_hand_thing_at_start;
    int leader_hand_weight_at_start;
    int pickup_invoked_with_full_chest;
    int pickup_accepted;
    int leader_hand_thing_after_pickup;
    int leader_hand_weight_after_pickup;
    int chest_visible_slots_unchanged;
    int chest_unchanged_count;
    int hidden_tail_unchanged;
    int no_recompaction_required;
    int no_error_path_to_f0334;
    const char *redmcsb_f0333_anchor;
    const char *redmcsb_f0334_anchor;
    const char *redmcsb_f0297_anchor;
    const char *redmcsb_f0298_anchor;
    const char *redmcsb_defs_c08_anchor;
    const char *capacity_note;
    const char *source_summary;
} Dm1V1ChestPickupAtCapacityContractPc34Compat;

const Dm1V1ChestPickupAtCapacityContractPc34Compat *
dm1_v1_chest_pickup_at_capacity_contract_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_PICKUP_AT_CAPACITY_PC34_COMPAT_H */
