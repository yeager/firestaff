#ifndef FIRESTAFF_CSB_V1_RUNTIME_CHAMPION_INVENTORY_HANDOFF_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_RUNTIME_CHAMPION_INVENTORY_HANDOFF_PC34_COMPAT_H

#include <stdint.h>
#include "csb_v1_runtime_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int champion_index;
    int slot_index;
    int leader_index_before;
    int leader_index_after;
    uint16_t slot_thing_before;
    uint16_t slot_thing_after;
    uint16_t leader_hand_before;
    uint16_t leader_hand_after;
    uint16_t attributes_before;
    uint16_t attributes_after;
    uint16_t load_before;
    uint16_t load_after;
    int imported_from_dm1;
    int party_state_valid;
    uint64_t state_hash;
} CSB_V1_RuntimeChampionInventoryHandoffResultPc34Compat;

const char *
csb_v1_runtime_champion_inventory_handoff_source_evidence_pc34_compat(void);

int csb_v1_runtime_champion_inventory_handoff_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int slot_index,
    uint16_t leader_hand_thing,
    CSB_V1_RuntimeChampionInventoryHandoffResultPc34Compat *out_result);

uint64_t csb_v1_runtime_champion_inventory_handoff_hash_pc34_compat(
    const CSB_V1_RuntimeProfile *profile);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_RUNTIME_CHAMPION_INVENTORY_HANDOFF_PC34_COMPAT_H */
