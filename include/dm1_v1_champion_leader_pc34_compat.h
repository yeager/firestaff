#ifndef DM1_V1_CHAMPION_LEADER_PC34_COMPAT_H
#define DM1_V1_CHAMPION_LEADER_PC34_COMPAT_H

/*
 * pass795 - DM1 V1 champion leader state contract test
 * (CLIKCHAM.C F0367 status-box dispatch + F0368 set-leader state
 * transition with old-leader dirty marking, leader-hand weight
 * remove/add, G0411 set/clear, candidate leader redraw).
 * Source-locked against CLIKCHAM.C F0367:24-35 + F0368:51-72 +
 * DEFS.H:8090-8100 (C00..C03 champion indices, G0411, G0455) +
 * DEFS.H:1874-1878 (CM1_CHAMPION_NONE).
 */

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_LEADER_NONE_PC34_COMPAT (-1)
#define DM1_V1_CHAMPION_LEADER_COUNT_PC34_COMPAT 4

#define DM1_V1_CHAMPION_ATTR_NAME_TITLE_PC34_COMPAT 0x0080u
#define DM1_V1_CHAMPION_ATTR_LOAD_PC34_COMPAT       0x0200u
#define DM1_V1_CHAMPION_ATTR_ICON_PC34_COMPAT       0x0400u

typedef struct DM1_V1_ChampionLeaderEntryPc34 {
    int currentHealth;
    int direction;
    int load;
    unsigned int attributes;
} DM1_V1_ChampionLeaderEntryPc34;

typedef struct DM1_V1_ChampionLeaderStatePc34 {
    int leaderIndex;
    int partyDirection;
    int leaderHandWeight;
    unsigned int candidateChampionOrdinal;
    DM1_V1_ChampionLeaderEntryPc34 champions[DM1_V1_CHAMPION_LEADER_COUNT_PC34_COMPAT];
} DM1_V1_ChampionLeaderStatePc34;

typedef struct DM1_V1_ChampionLeaderSetResultPc34 {
    int previousLeaderIndex;
    int newLeaderIndex;
    int ignoredSameLeader;
    int ignoredDeadTarget;
    int oldLeaderDetached;
    int newLeaderAttached;
    int oldLeaderDrawStateCount;
    int newLeaderDrawStateCount;
} DM1_V1_ChampionLeaderSetResultPc34;

void DM1_V1_ChampionLeader_InitPc34(
    DM1_V1_ChampionLeaderStatePc34 *state);

int DM1_V1_ChampionLeader_SetPc34(
    DM1_V1_ChampionLeaderStatePc34 *state,
    int championIndex,
    DM1_V1_ChampionLeaderSetResultPc34 *outResult);

const char *DM1_V1_ChampionLeader_SourceEvidencePc34(void);

typedef DM1_V1_ChampionLeaderEntryPc34
    Dm1V1ChampionLeaderEntryPc34Compat;
typedef DM1_V1_ChampionLeaderStatePc34
    Dm1V1ChampionLeaderStatePc34Compat;
typedef DM1_V1_ChampionLeaderSetResultPc34
    Dm1V1ChampionLeaderSetResultPc34Compat;

#define DM1_V1_ChampionLeader_InitPc34Compat \
    DM1_V1_ChampionLeader_InitPc34
#define DM1_V1_ChampionLeader_SetPc34Compat \
    DM1_V1_ChampionLeader_SetPc34
#define DM1_V1_ChampionLeader_SourceEvidencePc34Compat \
    DM1_V1_ChampionLeader_SourceEvidencePc34

#ifdef __cplusplus
}
#endif

#endif
