#include "dm1_v1_stairs_level_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    assert(DM1_STAIR_UP == 0);
    assert(DM1_STAIR_DOWN == 1);
    assert(DM1_V1_MAX_STAIRS_PC34 == 32);
    assert(DM1_V1_MAX_LEVELS_PC34 == 16);
}

static void test_struct_layout(void)
{
    DM1_V1_StairDefPc34 sd;
    memset(&sd, 0, sizeof(sd));
    sd.x = 5; sd.y = 10; sd.direction = DM1_STAIR_DOWN;
    sd.destLevel = 3; sd.destX = 7; sd.destY = 12; sd.destFacing = 2;
    assert(sd.x == 5);
    assert(sd.destLevel == 3);
    assert(sd.destFacing == 2);
}

static void test_init(void)
{
    DM1_V1_StairLevelStatePc34 s;
    DM1_V1_Stairs_InitPc34Compat(&s);
    assert(s.stairCount == 0);
    assert(s.levelCount == 0);
    assert(s.currentLevel == 0);
    assert(s.transitionActive == 0);
}

static void test_add_stair(void)
{
    DM1_V1_StairLevelStatePc34 s;
    DM1_V1_Stairs_InitPc34Compat(&s);
    int r = DM1_V1_Stairs_AddPc34Compat(&s, 5, 10, DM1_STAIR_DOWN, 2, 3, 4, 1);
    (void)r;
    assert(r == 1);
    assert(s.stairCount == 1);
    assert(s.stairs[0].x == 5);
    assert(s.stairs[0].destLevel == 2);
}

static void test_check_stair(void)
{
    DM1_V1_StairLevelStatePc34 s;
    DM1_V1_Stairs_InitPc34Compat(&s);
    DM1_V1_Stairs_AddPc34Compat(&s, 5, 10, DM1_STAIR_DOWN, 2, 3, 4, 1);
    DM1_V1_StairDefPc34 out;
    int found = DM1_V1_Stairs_CheckPc34Compat(&s, 5, 10, &out);
    (void)found;
    assert(found == 1);
    assert(out.destLevel == 2);
    int notfound = DM1_V1_Stairs_CheckPc34Compat(&s, 99, 99, &out);
    (void)notfound;
    assert(notfound == 0);
}

static void test_use_stair(void)
{
    DM1_V1_StairLevelStatePc34 s;
    DM1_V1_Stairs_InitPc34Compat(&s);
    DM1_V1_Stairs_AddPc34Compat(&s, 5, 10, DM1_STAIR_DOWN, 2, 3, 4, 1);
    int nx, ny, nf;
    int r = DM1_V1_Stairs_UsePc34Compat(&s, 5, 10, &nx, &ny, &nf);
    (void)r; (void)nx; (void)ny; (void)nf;
    assert(r == 1);
    assert(nx == 3);
    assert(ny == 4);
    assert(nf == 1);
}

static void test_add_level(void)
{
    DM1_V1_StairLevelStatePc34 s;
    DM1_V1_Stairs_InitPc34Compat(&s);
    DM1_V1_Stairs_AddLevelPc34Compat(&s, 32, 32);
    assert(s.levelCount == 1);
    assert(s.levels[0].width == 32);
    assert(s.levels[0].height == 32);
}

static void test_transition(void)
{
    DM1_V1_StairLevelStatePc34 s;
    DM1_V1_Stairs_InitPc34Compat(&s);
    int t = DM1_V1_Stairs_IsTransitioningPc34Compat(&s);
    (void)t;
    assert(t == 0);
}

static void test_tick(void)
{
    DM1_V1_StairLevelStatePc34 s;
    DM1_V1_Stairs_InitPc34Compat(&s);
    DM1_V1_Stairs_TickPc34Compat(&s, 16);
    assert(s.transitionActive == 0);
}

int main(void)
{
    test_constants();
    test_struct_layout();
    test_init();
    test_add_stair();
    test_check_stair();
    test_use_stair();
    test_add_level();
    test_transition();
    test_tick();

    puts("ok: DM1 stairs level (Q-DM1-04) 9 tests passed");
    return 0;
}
