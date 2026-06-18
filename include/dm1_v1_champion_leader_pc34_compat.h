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

typedef struct Dm1V1ChampionLeaderEntryPc34Compat {
    int currentHealth;
    int direction;
    int load;
    unsigned int attributes;
} Dm1V1ChampionLeaderEntryPc34Compat;

typedef struct Dm1V1ChampionLeaderStatePc34Compat {
    int leaderIndex;
    int partyDirection;
    int leaderHandWeight;
    unsigned int candidateChampionOrdinal;
    Dm1V1ChampionLeaderEntryPc34Compat champions[DM1_V1_CHAMPION_LEADER_COUNT_PC34_COMPAT];
} Dm1V1ChampionLeaderStatePc34Compat;

typedef struct Dm1V1ChampionLeaderSetResultPc34Compat {
    int previousLeaderIndex;
    int newLeaderIndex;
    int ignoredSameLeader;
    int ignoredDeadTarget;
    int oldLeaderDetached;
    int newLeaderAttached;
    int oldLeaderDrawStateCount;
    int newLeaderDrawStateCount;
} Dm1V1ChampionLeaderSetResultPc34Compat;

void DM1_V1_ChampionLeader_InitPc34Compat(
    Dm1V1ChampionLeaderStatePc34Compat *state);

int DM1_V1_ChampionLeader_SetPc34Compat(
    Dm1V1ChampionLeaderStatePc34Compat *state,
    int championIndex,
    Dm1V1ChampionLeaderSetResultPc34Compat *outResult);

const char *DM1_V1_ChampionLeader_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
