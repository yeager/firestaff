#include "dm1_v1_dungeon_data_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void)
{
    DM1_V1_DungeonDataPc34 dd;
    DM1_V1_DungeonData_InitPc34Compat(&dd);
    assert(dd.loaded == false);
    assert(dd.partyDead == false);
    assert(dd.newGame == false);
    assert(dd.gameWon == false);
}

static void test_set_party_pos(void)
{
    DM1_V1_DungeonDataPc34 dd;
    const DM1_V1_DungeonDataPartyPosPc34 *pos;
    DM1_V1_DungeonData_InitPc34Compat(&dd);
    DM1_V1_DungeonData_SetPartyPosPc34Compat(&dd, 1, 5, 7, 2);
    pos = DM1_V1_DungeonData_GetPartyPosPc34Compat(&dd);
    assert(pos->mapIndex == 1);
    assert(pos->posX == 5);
    assert(pos->posY == 7);
    assert(pos->facing == 2);
    (void)pos;
}

static void test_set_champion_count(void)
{
    DM1_V1_DungeonDataPc34 dd;
    DM1_V1_DungeonData_InitPc34Compat(&dd);
    DM1_V1_DungeonData_SetChampionCountPc34Compat(&dd, 3);
    assert(dd.championCount == 3);
}

static void test_advance_tick(void)
{
    DM1_V1_DungeonDataPc34 dd;
    uint32_t t;
    DM1_V1_DungeonData_InitPc34Compat(&dd);
    t = DM1_V1_DungeonData_GetGameTimePc34Compat(&dd);
    assert(t == 0);
    DM1_V1_DungeonData_AdvanceTickPc34Compat(&dd);
    t = DM1_V1_DungeonData_GetGameTimePc34Compat(&dd);
    assert(t == 1);
    (void)t;
}

static void test_shutdown(void)
{
    DM1_V1_DungeonDataPc34 dd;
    DM1_V1_DungeonData_InitPc34Compat(&dd);
    DM1_V1_DungeonData_ShutdownPc34Compat(&dd);
    assert(dd.loaded == false);
}

static void test_source_evidence(void)
{
    const char *ev = DM1_V1_DungeonData_SourceEvidencePc34Compat();
    assert(ev != NULL);
    assert(ev[0] != '\0');
    (void)ev;
}

int main(void)
{
    test_init();
    test_set_party_pos();
    test_set_champion_count();
    test_advance_tick();
    test_shutdown();
    test_source_evidence();
    puts("ok: DM1 dungeon data (Q-DM1-08) 6 tests passed");
    return 0;
}
