#ifndef FIRESTAFF_DM1_V1_CHEST_INVENTORY_C545_DROP_TO_LEADER_HAND_ALREADY_OCCUPIED_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_INVENTORY_C545_DROP_TO_LEADER_HAND_ALREADY_OCCUPIED_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_PC34_C545_LEADER_HAND_OCCUPIED_SLOT_COUNT DM1_PC34_CHEST_SLOT_COUNT
#define DM1_PC34_C545_LEADER_HAND_OCCUPIED_SURFACE_BYTES 64

enum {
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_NONE = 0,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_COMMAND = 545,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_C040_PANEL = 40,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_C038_SCROLL_ICON = 38,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_C537_FIRST = 537,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_C544_LAST = 544,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_C30_FIRST = 30,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_C37_LAST = 37,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_ACTIVE_INDEX = 0,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_SCROLL_THING = 0x7038,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_OPEN_CHEST_THING = 0x6c45,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_LEADER_COUNT = 1,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_CHEST_COUNT = 1,
    DM1_PC34_C545_LEADER_HAND_OCCUPIED_EXPECTED_COUNT = 2
};

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    const char *chestOpenInitAnchor;
    const char *chestSlotDropSinkAnchor;
    const char *leaderHandStateAnchor;
    const char *championHandPutGetAnchor;
    const char *c30SlotAnchor;
    const char *c30OwnershipAnchor;
    const char *partyChampionPanelAnchor;
    const char *c545PanelInputAnchor;
    const char *queueDispatchAnchor;
    const char *c040PanelRedrawAnchor;
    const char *mouthRouteRedrawAnchor;
    const char *objectIdentityAnchor;
    const char *redrawIdentityAnchor;
    const char *defsAnchor;
    const char *scope;
} Dm1V1ChestInventoryC545DropToLeaderHandAlreadyOccupiedEvidencePc34Compat;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
} Dm1V1ChestInventoryC545DropToLeaderHandAlreadyOccupiedStatsPc34Compat;

const Dm1V1ChestInventoryC545DropToLeaderHandAlreadyOccupiedEvidencePc34Compat *
dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_evidence_pc34_compat(
    void);

const char *
dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_source_evidence_pc34_compat(
    void);

int run_dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat_self_test(
    void);

Dm1V1ChestInventoryC545DropToLeaderHandAlreadyOccupiedStatsPc34Compat
dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_last_stats_pc34_compat(
    void);

#ifdef __cplusplus
}
#endif

#endif
