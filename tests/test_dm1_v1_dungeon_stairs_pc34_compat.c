#include "dm1_v1_dungeon_stairs_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_result_struct(void)
{
    struct StairsTransitionResult_Compat r;
    memset(&r, 0, sizeof(r));
    assert(r.transitioned == 0);
    assert(r.stairUp == 0);
    assert(r.fromMapIndex == 0);
    assert(r.toMapIndex == 0);
    assert(r.newMapX == 0);
    assert(r.newMapY == 0);
    assert(r.newDirection == 0);
}

static void test_resolve_null_result(void)
{
    int ok = dm1_v1_dungeon_resolve_stairs_transition_pc34(NULL, NULL, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_resolve_null_dungeon(void)
{
    struct StairsTransitionResult_Compat r;
    int ok = dm1_v1_dungeon_resolve_stairs_transition_pc34(NULL, NULL, &r);
    (void)ok;
    assert(ok == 0);
}

static void test_resolve_null_party(void)
{
    struct DungeonDatState_Compat d;
    struct StairsTransitionResult_Compat r;
    memset(&d, 0, sizeof(d));
    int ok = dm1_v1_dungeon_resolve_stairs_transition_pc34(&d, NULL, &r);
    (void)ok;
    assert(ok == 0);
}

static void test_source_evidence(void)
{
    const char* e = dm1_v1_dungeon_stairs_source_evidence_pc34();
    assert(e != NULL);
    assert(strlen(e) > 0);
}

int main(void)
{
    test_result_struct();
    test_resolve_null_result();
    test_resolve_null_dungeon();
    test_resolve_null_party();
    test_source_evidence();

    puts("ok: DM1 dungeon stairs (Q-DM1-04) 5 tests passed");
    return 0;
}
