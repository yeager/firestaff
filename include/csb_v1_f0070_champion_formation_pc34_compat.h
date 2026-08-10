/*
 * ReDMCSB IO.C F0070 champion-icon formation transaction (PC 3.4).
 *
 * C125..C128 are deliberately not inventory commands. COMMAND.C F0380
 * dispatches them straight to F0070; the latter picks up an occupied party
 * icon, then moves or swaps party cells when it is released/clicked again.
 * This boundary owns only the durable champion state. Pointer bitmap and
 * screen blits stay with the platform UI owner.
 */
#ifndef FIRESTAFF_CSB_V1_F0070_CHAMPION_FORMATION_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0070_CHAMPION_FORMATION_PC34_COMPAT_H

#include <stdint.h>

enum {
    CSB_V1_F0070_CHAMPION_CAPACITY_PC34 = 4,
    CSB_V1_F0070_ATTRIBUTE_ICON_DIRTY_PC34 = 0x0400
};

typedef struct CsbV1F0070ChampionFormationStatePc34 {
    int champion_count;
    int party_direction;
    /* G0599: zero is no icon in the pointer; otherwise icon index + 1. */
    unsigned int held_icon_ordinal;
    /* M516_CHAMPIONS[*].Cell and .Direction. */
    uint8_t cell[CSB_V1_F0070_CHAMPION_CAPACITY_PC34];
    uint8_t direction[CSB_V1_F0070_CHAMPION_CAPACITY_PC34];
    uint16_t attributes[CSB_V1_F0070_CHAMPION_CAPACITY_PC34];
} CsbV1F0070ChampionFormationStatePc34;

typedef struct CsbV1F0070ChampionFormationReceiptPc34 {
    int accepted;
    int picked_up;
    int released;
    int moved_to_empty_cell;
    int swapped_with_occupant;
    int source_icon_suppressed;
    int source_icon_cleared;
    int source_icon_index;
    int target_icon_index;
    int source_champion_index;
    int target_champion_index;
    const char *source_evidence;
} CsbV1F0070ChampionFormationReceiptPc34;

/* Applies one C125..C128 click. Invalid/incomplete party state is rejected
 * without modification. The function makes no inventory or raster changes. */
int csb_v1_f0070_champion_formation_click_pc34(
    CsbV1F0070ChampionFormationStatePc34 *state,
    int target_icon_index,
    CsbV1F0070ChampionFormationReceiptPc34 *out_receipt);

const char *csb_v1_f0070_champion_formation_source_evidence_pc34(void);

#endif
