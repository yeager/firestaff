/* DM1 V1 chest close object stack-merge source-locked contract gate. */
#ifndef DM1_V1_CHEST_CLOSE_STACK_MERGE_PC34_COMPAT_H
#define DM1_V1_CHEST_CLOSE_STACK_MERGE_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE 0xFFFF
#define DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_END  0xFFFE

#define DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_COUNT 6

typedef struct {
    const char *f0333_open;
    const char *f0334_close;
    const char *f0163_append;
    const char *f0159_get_next;
    const char *f0140_container_weight;
    const char *f0297_put_leader_hand;
    const char *f0302_slot;
    const char *sentinel_chain;
} DM1_V1_ChestCloseStackMergeAnchorsPc34;

typedef struct {
    const char *contract_marker;
    int contract_only;
    int chest_slot_count;
    int thing_none;
    int thing_end;
    int sparse_visible_count;
    int expected_f0163_call_count;
    int expected_f0163_walk_hops;
    int sparse_visible_types[DM1_PC34_CHEST_SLOT_COUNT];
    int sparse_last_visible_ordinal;
    int rewire_expected_types[DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_COUNT];
    int reopen_expected_types[DM1_PC34_CHEST_SLOT_COUNT];
    int leader_hand_stackable_item_type;
    int leader_hand_stackable_charges;
    int leader_hand_stackable_weight;
    int rewire_container_base_weight;
    DM1_V1_ChestCloseStackMergeAnchorsPc34 anchors;
} DM1_V1_ChestCloseStackMergeSpecPc34;

typedef struct {
    int sparse_open_result;
    int sparse_visible_count;
    int sparse_visible_types[DM1_PC34_CHEST_SLOT_COUNT];

    int leader_hand_item_type_before;
    int leader_hand_charges_before;
    int leader_hand_item_type_after;
    int leader_hand_charges_after;
    int leader_hand_weight_after;

    int rewire_head_ordinal;
    int rewire_chain_count;
    int rewire_chain_terminator;
    int rewire_chain_leaked_none;
    int rewire_chain_types[DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_COUNT];
    int rewire_chain_next[DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_COUNT];
    int rewire_f0163_call_count;
    int rewire_f0163_walk_hops;
    int rewire_f0163_first_thing_ordinal;
    int rewire_f0163_first_previous_ordinal;
    int rewire_f0163_second_thing_ordinal;
    int rewire_f0163_second_previous_ordinal;
    int rewire_container_base_weight;

    int post_close_chest_slot_item_types[DM1_PC34_CHEST_SLOT_COUNT];
    int post_close_open_chest_thing;

    int reopen_result;
    int reopen_visible_count;
    int reopen_types[DM1_PC34_CHEST_SLOT_COUNT];
    int reopen_leader_hand_item_type;
    int reopen_leader_hand_charges;
} DM1_V1_ChestCloseStackMergeProbePc34;

const char *dm1_v1_chest_close_stack_merge_source_evidence_pc34(void);

const DM1_V1_ChestCloseStackMergeSpecPc34 *
dm1_v1_chest_close_stack_merge_spec_pc34(void);

int dm1_v1_chest_close_stack_merge_run_pc34(
    DM1_V1_ChestCloseStackMergeProbePc34 *out);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_CHEST_CLOSE_STACK_MERGE_PC34_COMPAT_H */
