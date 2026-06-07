#ifndef FIRESTAFF_DM1_V1_CHEST_LINK_CORRUPTION_RECOVERY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_LINK_CORRUPTION_RECOVERY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT = 8,
    DM1_V1_CHEST_LINK_CORRUPTION_CHAIN_CAPACITY = 8,
    DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST = 0xFFFE,
    DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE = 0xFFFF
};

typedef struct {
    int input_slots[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT];
    int close_count;
    int close_order[DM1_V1_CHEST_LINK_CORRUPTION_CHAIN_CAPACITY];
    int reopen_count;
    int reopen_slots[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT];
    int pre_sentinel_count_if_regressed;
    int container_slot_after_end_reset;
    int container_slot_after_head_write;
    int last_valid_thing;
    int last_valid_next_after_close;
    int chain_terminator;
    int chain_contains_none;
} DM1_V1_ChestLinkCorruptionRecoveryCasePc34;

typedef struct {
    int contract_only;
    int visible_slot_count;
    int thing_none;
    int thing_end_of_list;
    int close_loop_bound;
    int close_loop_skips_none;
    int reopen_loop_walks_rewritten_links;
    int reopen_loop_stops_at_end;
    int open_empty_tail_fills_none;
    DM1_V1_ChestLinkCorruptionRecoveryCasePc34 mid_array_none;
    DM1_V1_ChestLinkCorruptionRecoveryCasePc34 leading_none;
    const char *redmcsb_f0334_anchor;
    const char *redmcsb_f0333_anchor;
    const char *sentinel_anchor;
    const char *contract_note;
    const char *source_summary;
} DM1_V1_ChestLinkCorruptionRecoveryContractPc34;

const DM1_V1_ChestLinkCorruptionRecoveryContractPc34 *
dm1_v1_chest_link_corruption_recovery_contract_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
