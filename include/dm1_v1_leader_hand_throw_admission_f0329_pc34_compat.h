#ifndef FIRESTAFF_DM1_V1_LEADER_HAND_THROW_ADMISSION_F0329_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_LEADER_HAND_THROW_ADMISSION_F0329_PC34_COMPAT_H

#include <stdint.h>

#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CHAMPION.C F0329 has no independent projectile formula: it selects the
 * current leader, temporarily maps G4055 leader-hand Thing through C01, then
 * delegates to F0328. This gate owns only that source transaction. */
typedef struct {
    const struct DungeonThings_Compat *things;
    const struct PartyState_Compat *party;
    int leaderIndex;
    unsigned short leaderHandThing;
    int throwSide;
} DM1_LeaderHandThrowAdmissionInputF0329Pc34;

typedef struct {
    int valid;
    int leaderIndex;
    int throwSide;
    unsigned short leaderHandThing;
    unsigned short savedActionHandThing;
    uint32_t rawLeaderHandFNV1a;
    uint32_t partyFNV1a;
    const char *sourceAnchor;
} DM1_LeaderHandThrowAdmissionReceiptF0329Pc34;

int dm1_v1_leader_hand_throw_admit_f0329_pc34(
    const DM1_LeaderHandThrowAdmissionInputF0329Pc34 *input,
    DM1_LeaderHandThrowAdmissionReceiptF0329Pc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif
