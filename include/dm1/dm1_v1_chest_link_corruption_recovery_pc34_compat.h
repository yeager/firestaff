#ifndef FIRESTAFF_DM1_V1_CHEST_LINK_CORRUPTION_RECOVERY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_LINK_CORRUPTION_RECOVERY_PC34_COMPAT_H

/*
 * Contract-only DM1 V1 chest link corruption recovery gate.
 *
 * ReDMCSB source anchors used by the implementation and printed by the test:
 * - CHEST.C F0333:30-67 opens a chest, materializes Container->Slot into
 *   G0425_aT_ChestSlots, stops at C0xFFFE_THING_ENDOFLIST, and caps at eight.
 * - CHEST.C F0334:113-132 closes a chest by resetting Container->Slot to
 *   C0xFFFE_THING_ENDOFLIST, scanning eight G0425_aT_ChestSlots, skipping
 *   C0xFFFF_THING_NONE, clearing visible slots, and relinking with F0163.
 * - DUNGEON.C F0163:1769-1838 sets ThingToLink->Next to
 *   C0xFFFE_THING_ENDOFLIST, returns for END input, and appends to the list.
 * - DUNGEON.C F0164:1840-1905 returns for END input, stops cleanup when the
 *   searched link reaches C0xFFFE_THING_ENDOFLIST or C0xFFFF_THING_NONE, and
 *   isolates the unlinked thing by setting its Next to END.
 * - DUNGEON.C F0140:1114-1120 contributes container base weight 50 plus the
 *   recursively linked contents until C0xFFFE_THING_ENDOFLIST.
 * - CHAMPION.C F0297:243-268 and F0298:270-298 preserve leader-hand thing
 *   identity while adding/removing F0140 weight and refreshing load state.
 * - CHAMPION.C F0300:511-515 and F0301:606-614 clear/write C30+ chest slots
 *   through G0425_aT_ChestSlots instead of champion body slots.
 * - OBJECT.C F0033:147-212 derives icon identity from the THING without
 *   rewriting the THING ordinal.
 * - BLITMASK.C F0133:30-33 describes partial-mask bitmap dispatch; this gate
 *   asserts no real-asset or bitmap parity claim for that presentation path.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34 = 8,
    DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34 = 0xFFFE,
    DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34 = 0xFFFF,
    DM1_V1_CHEST_LINK_CORRUPTION_THING_DANGLING_PC34 = 0x10000,
    DM1_V1_CHEST_LINK_CORRUPTION_C30_SLOT_PC34 = 30,
    DM1_V1_CHEST_LINK_CORRUPTION_CHEST_THING_PC34 = 0x0901,
    DM1_V1_CHEST_LINK_CORRUPTION_ICON_CHEST_OPEN_PC34 = 145
};

typedef struct {
    const char *f0333_open_materialization;
    const char *f0334_close_rewrite;
    const char *f0163_link_append;
    const char *f0164_square_cleanup;
    const char *f0140_container_weight;
    const char *f0297_put_leader_hand;
    const char *f0298_remove_leader_hand;
    const char *f0300_clear_c30_slot;
    const char *f0301_write_c30_slot;
    const char *f0033_icon_identity;
    const char *f0133_partial_mask_dispatch;
    const char *sentinel_chain;
    const char *contract_scope;
} DM1_V1_ChestLinkCorruptionRecoveryAnchorsPc34;

typedef struct {
    int contract_only;
    int no_real_asset_parity_claim;
    int slot_count;
    int thing_none;
    int thing_end;
    int thing_dangling;

    int initial_visible_slots[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int initial_next_by_item[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int initial_container_slot;
    int initial_visible_count;
    int initial_chain_terminates_at_end;

    int corrupted_visible_slots[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int corrupted_next_by_item[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int corrupt_null_next_item;
    int corrupt_duplicate_item;
    int corrupt_duplicate_slot;
    int corrupt_dangling_next_item;
    int corruptions_present;

    int post_close_visible_slots[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int post_close_next_by_item[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int close_order[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int close_count;
    int close_container_slot;
    int close_cleared_visible_slots;
    int close_duplicate_ordinal_count;
    int close_has_dangling_next;
    int close_has_none_inside_visible_window;
    int close_dropped_null_next_item;
    int close_dropped_duplicate_shadow_item;
    int close_dropped_replaced_item;

    int cleanup_next_by_item[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int cleanup_isolated_null_next_item;
    int cleanup_isolated_duplicate_shadow_item;
    int cleanup_isolated_replaced_item;
    int cleanup_stopped_at_none_or_end;

    int reopen_slots[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int reopen_count;
    int reopen_tail_none_count;
    int reopen_duplicate_ordinal_count;
    int reopen_has_dangling_next;
    int reopen_has_end_inside_visible_window;
    int reopen_first_slot;
    int reopen_last_visible_slot;

    int item_weights[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int initial_container_weight;
    int recovered_container_weight;
    int container_base_weight;
    int leader_hand_initial_thing;
    int leader_hand_icon_before;
    int leader_hand_removed_thing;
    int leader_hand_final_thing;
    int leader_load_before_put;
    int leader_load_after_put;
    int leader_load_after_remove;
    int leader_load_after_restore;
    int leader_hand_valid_after_cycle;

    int c30_slot_index;
    int c30_clear_before;
    int c30_clear_after;
    int c30_write_thing;
    int c30_write_after;

    DM1_V1_ChestLinkCorruptionRecoveryAnchorsPc34 anchors;
} DM1_V1_ChestLinkCorruptionRecoverySpecPc34;

const DM1_V1_ChestLinkCorruptionRecoverySpecPc34 *
dm1_v1_chest_link_corruption_recovery_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
