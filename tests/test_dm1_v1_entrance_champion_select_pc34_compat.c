#include "dm1_v1_entrance_champion_select_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_assertions = 0;

static void expect_int(const char *label, int got, int want)
{
    g_assertions++;
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        g_failures++;
    }
}

static void expect_true(const char *label, int condition)
{
    expect_int(label, condition ? 1 : 0, 1);
}

static void test_door_animation(void)
{
    DM1_V1_EntranceCtxPc34 ctx;
    DM1_V1_EntranceFullStartRenderReceiptPc34 receipt;

    DM1_V1_Entrance_InitPc34Compat(&ctx);
    expect_int("init.state", ctx.state, DM1_ENTRANCE_IDLE);
    expect_int("init.selected", ctx.selectedMirrorIndex, -1);
    expect_int("init.door.steps", ctx.doorAnim.totalSteps, 10);
    expect_int("init.door.delay", ctx.doorAnim.frameDelayMs, 100);

    DM1_V1_Entrance_StartDoorAnimationPc34Compat(&ctx, 1000u);
    expect_int("door.start.state", ctx.state, DM1_ENTRANCE_DOOR_OPENING);
    expect_int("door.start.step", ctx.doorAnim.animationStep, 0);
    expect_int("door.too_early", DM1_V1_Entrance_TickDoorAnimationPc34Compat(&ctx, 1099u), 0);
    expect_int("door.first_frame", DM1_V1_Entrance_TickDoorAnimationPc34Compat(&ctx, 1100u), 1);
    expect_int("door.first_frame.step", ctx.doorAnim.animationStep, 1);

    for (int i = 2; i <= 10; i++) {
        expect_int("door.frame", DM1_V1_Entrance_TickDoorAnimationPc34Compat(&ctx, 1000u + (uint32_t)i * 100u), 1);
    }
    expect_int("door.complete", ctx.doorAnim.complete, 1);
    expect_int("door.complete.state", ctx.state, DM1_ENTRANCE_VIEWING);
    expect_int("door.after_complete", DM1_V1_Entrance_TickDoorAnimationPc34Compat(&ctx, 2200u), 0);

    expect_int("fullstart.receipt", DM1_V1_Entrance_BuildFullStartRenderReceiptPc34Compat(&ctx, &receipt), 1);
    expect_int("fullstart.map", receipt.mapIndex, DM1_V1_ENTRANCE_MAP_INDEX_PC34);
    expect_int("fullstart.width", receipt.width, 5);
    expect_int("fullstart.height", receipt.height, 5);
    expect_int("fullstart.party.x", receipt.partyX, 2);
    expect_int("fullstart.party.y", receipt.partyY, 0);
    expect_int("fullstart.party.dir", receipt.partyDirection, DM1_V1_ENTRANCE_DIRECTION_SOUTH_PC34);
    expect_int("fullstart.corridors", receipt.corridorCount, 6);
    for (int i = 0; i < DM1_V1_ENTRANCE_MICRO_DUNGEON_SIZE_PC34; ++i) {
        int want = (i >= 10 && i <= 14) || i == 7
                   ? DM1_V1_ENTRANCE_ELEMENT_CORRIDOR_PC34
                   : DM1_V1_ENTRANCE_ELEMENT_WALL_PC34;
        expect_int("fullstart.square", receipt.squares[i], want);
    }
    expect_int("fullstart.complete.frame", receipt.doorFrameIndex, 9);
    expect_int("fullstart.music", receipt.entranceMusicRequested, 1);

    DM1_V1_Entrance_StartDoorAnimationPc34Compat(&ctx, 3000u);
    expect_int("fullstart.start.receipt", DM1_V1_Entrance_BuildFullStartRenderReceiptPc34Compat(&ctx, &receipt), 1);
    expect_int("fullstart.start.frame", receipt.doorFrameIndex, 0);
    expect_int("fullstart.start.rattle", receipt.playDoorRattleSound, 0);
    expect_int("door.step1", DM1_V1_Entrance_TickDoorAnimationPc34Compat(&ctx, 3100u), 1);
    expect_int("fullstart.step1.receipt", DM1_V1_Entrance_BuildFullStartRenderReceiptPc34Compat(&ctx, &receipt), 1);
    expect_int("fullstart.step1.frame", receipt.doorFrameIndex, 1);
    expect_int("fullstart.step1.rattle", receipt.playDoorRattleSound, 1);
}

static void test_mirror_recruit_and_finalize(void)
{
    DM1_V1_EntranceCtxPc34 ctx;
    DM1_V1_EntranceTickResultPc34 result;

    DM1_V1_Entrance_InitPc34Compat(&ctx);
    ctx.state = DM1_ENTRANCE_VIEWING;

    expect_int("mirror.add.0", DM1_V1_Entrance_AddMirrorPc34Compat(&ctx, 7, 1, 2, 3, 0), 0);
    expect_int("mirror.add.1", DM1_V1_Entrance_AddMirrorPc34Compat(&ctx, 8, 2, 2, 1, 1), 1);
    expect_int("mirror.count", ctx.mirrorCount, 2);
    expect_int("mirror.0.champion", ctx.mirrors[0].championIndex, 7);
    expect_int("mirror.1.dead", ctx.mirrors[1].dead, 1);

    result = DM1_V1_Entrance_ClickMirrorPc34Compat(&ctx, 0, 5000u);
    expect_true("click.live.selected", result.mirrorSelected);
    expect_int("click.live.state", ctx.state, DM1_ENTRANCE_SELECTING);
    expect_int("click.live.new_state", result.newState, DM1_ENTRANCE_SELECTING);
    expect_true("click.live.redraw", result.needsRedraw);

    expect_int("recruit.live", DM1_V1_Entrance_RecruitChampionPc34Compat(&ctx), 1);
    expect_int("recruit.party.count", DM1_V1_Entrance_GetPartyCountPc34Compat(&ctx), 1);
    expect_int("recruit.party.index", ctx.partyChampionIndices[0], 7);
    expect_int("recruit.state", ctx.state, DM1_ENTRANCE_VIEWING);
    expect_int("recruit.same_again", DM1_V1_Entrance_ClickMirrorPc34Compat(&ctx, 0, 5100u).mirrorSelected, 0);

    result = DM1_V1_Entrance_FinalizePc34Compat(&ctx);
    expect_true("finalize.complete", result.entranceComplete);
    expect_int("finalize.state", ctx.state, DM1_ENTRANCE_DONE);
    expect_int("is_complete", DM1_V1_Entrance_IsCompletePc34Compat(&ctx), 1);
}

static void test_dead_mirror_paths(void)
{
    DM1_V1_EntranceCtxPc34 ctx;
    DM1_V1_EntranceTickResultPc34 result;

    DM1_V1_Entrance_InitPc34Compat(&ctx);
    ctx.state = DM1_ENTRANCE_VIEWING;
    (void)DM1_V1_Entrance_AddMirrorPc34Compat(&ctx, 12, 3, 4, 0, 1);

    result = DM1_V1_Entrance_ClickMirrorPc34Compat(&ctx, 0, 7000u);
    expect_true("click.dead.selected", result.mirrorSelected);
    expect_int("click.dead.state", ctx.state, DM1_ENTRANCE_RESURRECTING);
    expect_int("dead.recruit.reject", DM1_V1_Entrance_RecruitChampionPc34Compat(&ctx), 0);
    expect_int("dead.resurrect", DM1_V1_Entrance_ResurrectPc34Compat(&ctx), 1);
    expect_int("dead.resurrect.state", ctx.state, DM1_ENTRANCE_SELECTING);
    expect_int("dead.resurrect.flag", ctx.mirrors[0].dead, 0);
    expect_int("dead.recruit.after", DM1_V1_Entrance_RecruitChampionPc34Compat(&ctx), 1);

    DM1_V1_Entrance_InitPc34Compat(&ctx);
    ctx.state = DM1_ENTRANCE_VIEWING;
    (void)DM1_V1_Entrance_AddMirrorPc34Compat(&ctx, 13, 4, 4, 2, 1);
    (void)DM1_V1_Entrance_ClickMirrorPc34Compat(&ctx, 0, 8000u);
    expect_int("dead.reincarnate", DM1_V1_Entrance_ReincarnatePc34Compat(&ctx), 1);
    expect_int("dead.reincarnate.state", ctx.state, DM1_ENTRANCE_SELECTING);
    DM1_V1_Entrance_CancelSelectionPc34Compat(&ctx);
    expect_int("cancel.state", ctx.state, DM1_ENTRANCE_VIEWING);
    expect_int("cancel.selected", ctx.selectedMirrorIndex, -1);
}

static void test_capacity(void)
{
    DM1_V1_EntranceCtxPc34 ctx;

    DM1_V1_Entrance_InitPc34Compat(&ctx);
    for (int i = 0; i < DM1_V1_MAX_MIRROR_SLOTS_PC34; i++) {
        expect_int("capacity.add", DM1_V1_Entrance_AddMirrorPc34Compat(&ctx, i, i, 0, 0, 0), i);
    }
    expect_int("capacity.full", DM1_V1_Entrance_AddMirrorPc34Compat(&ctx, 99, 0, 0, 0, 0), -1);

    ctx.state = DM1_ENTRANCE_VIEWING;
    for (int i = 0; i < M11_MAX_CHAMPIONS; i++) {
        (void)DM1_V1_Entrance_ClickMirrorPc34Compat(&ctx, i, 9000u + (uint32_t)i);
        expect_int("capacity.recruit", DM1_V1_Entrance_RecruitChampionPc34Compat(&ctx), 1);
    }
    (void)DM1_V1_Entrance_ClickMirrorPc34Compat(&ctx, M11_MAX_CHAMPIONS, 9100u);
    expect_int("capacity.party_full", DM1_V1_Entrance_RecruitChampionPc34Compat(&ctx), 0);
}

int main(void)
{
    printf("probe=dm1_v1_entrance_champion_select_pc34_compat\n");
    printf("source=%s\n", DM1_V1_Entrance_SourceEvidencePc34Compat());

    test_door_animation();
    test_mirror_recruit_and_finalize();
    test_dead_mirror_paths();
    test_capacity();

    if (g_failures) {
        printf("FAIL dm1_v1_entrance_champion_select_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_entrance_champion_select_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
