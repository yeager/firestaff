#include "dm1_v1_chest_pickup_at_capacity_pc34_compat.h"

enum {
    DM1_V1_CHEST_PICKUP_AT_CAPACITY_VISIBLE_CAPACITY = 8,
    DM1_V1_CHEST_PICKUP_AT_CAPACITY_LEADER_HAND_SENTINEL = 0x6A51,
    DM1_V1_CHEST_PICKUP_AT_CAPACITY_LEADER_HAND_WEIGHT = 17
};

static const char s_redmcsb_f0333_anchor[] =
    "ReDMCSB CHEST.C F0333:31-67 caps open-chest materialization at "
    "G0425_aT_ChestSlots[0..7]; slot index 7 is the last writable visible "
    "chest slot.";

static const char s_redmcsb_f0334_anchor[] =
    "ReDMCSB CHEST.C F0334:113-132 rewrites only non-empty visible "
    "G0425_aT_ChestSlots[0..7] when a chest close reaches the relink path.";

static const char s_redmcsb_f0297_anchor[] =
    "ReDMCSB CHAMPION.C F0297:243-268 puts a thing in the leader hand and "
    "adds its object weight to the leader load.";

static const char s_redmcsb_f0298_anchor[] =
    "ReDMCSB CHAMPION.C F0298:270-298 removes a thing from the leader hand "
    "and subtracts its object weight; F0302:688-710 performs the slot swap "
    "only after the selected slot is valid.";

static const char s_redmcsb_defs_c08_anchor[] =
    "ReDMCSB DEFS.H C30_SLOT_CHEST_1..C37_SLOT_CHEST_8:810-817, "
    "C08_SLOT_BOX_INVENTORY_FIRST_SLOT/C09_SLOT_BOX_INVENTORY_ACTION_HAND:"
    "1874-1875, C537_ZONE_SLOT_BOX_38_CHEST_1..C544_ZONE_SLOT_BOX_45_CHEST_8:"
    "3906-3913, and G0425_aT_ChestSlots[8]:5878 define the eight visible "
    "chest slots.";

static const char s_capacity_note[] =
    "contract_only=1; CHEST.C F0333:31-67 visible capacity=8; "
    "CHEST.C F0334:113-132 no F0334 rewrite; "
    "CHAMPION.C F0297/F0298:243-285 pickup rejected; "
    "leader hand unchanged; chest slots unchanged.";

static const char s_source_summary[] =
    "contract_only=1 source-locks the DM1 V1 pickup-into-chest-at-capacity "
    "case: CHEST.C F0333:31-67 exposes only eight visible slots, "
    "CHAMPION.C F0297/F0298:243-285 keeps the leader-hand thing/weight when "
    "the rejected pickup cannot enter a full chest, CHEST.C F0334:113-132 is "
    "not reached for a close/rewrite, visible capacity=8, pickup rejected, "
    "leader hand unchanged, chest slots unchanged, no F0334 rewrite.";

static const Dm1V1ChestPickupAtCapacityContractPc34Compat s_contract = {
    1,
    DM1_V1_CHEST_PICKUP_AT_CAPACITY_VISIBLE_CAPACITY,
    DM1_V1_CHEST_PICKUP_AT_CAPACITY_VISIBLE_CAPACITY,
    DM1_V1_CHEST_PICKUP_AT_CAPACITY_LEADER_HAND_SENTINEL,
    DM1_V1_CHEST_PICKUP_AT_CAPACITY_LEADER_HAND_WEIGHT,
    1,
    0,
    DM1_V1_CHEST_PICKUP_AT_CAPACITY_LEADER_HAND_SENTINEL,
    DM1_V1_CHEST_PICKUP_AT_CAPACITY_LEADER_HAND_WEIGHT,
    1,
    DM1_V1_CHEST_PICKUP_AT_CAPACITY_VISIBLE_CAPACITY,
    1,
    1,
    1,
    s_redmcsb_f0333_anchor,
    s_redmcsb_f0334_anchor,
    s_redmcsb_f0297_anchor,
    s_redmcsb_f0298_anchor,
    s_redmcsb_defs_c08_anchor,
    s_capacity_note,
    s_source_summary
};

const Dm1V1ChestPickupAtCapacityContractPc34Compat *
dm1_v1_chest_pickup_at_capacity_contract_pc34_compat(void)
{
    return &s_contract;
}
