#include <stdio.h>
#include <string.h>

#include "dm1_v1_room_transition_pc34_compat.h"

static int g_pass;
static int g_fail;

#define EXPECT(label, cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s: %s\n", label, #cond); \
        ++g_fail; \
    } else { \
        ++g_pass; \
    } \
} while (0)

#define EXPECT_INT(label, got, want) do { \
    int _g = (got); \
    int _w = (want); \
    if (_g != _w) { \
        fprintf(stderr, "FAIL %s: got=%d want=%d\n", label, _g, _w); \
        ++g_fail; \
    } else { \
        ++g_pass; \
    } \
} while (0)

static struct Dm1V1RoomTransitionPosePc34Compat pose(int mapIndex, int x, int y, int dir)
{
    struct Dm1V1RoomTransitionPosePc34Compat p;
    p.mapIndex = mapIndex;
    p.mapX = x;
    p.mapY = y;
    p.direction = dir;
    return p;
}

static void test_non_v1_guard_disables_plan(void)
{
    struct Dm1V1RoomTransitionInputPc34Compat in;
    struct Dm1V1RoomTransitionPlanPc34Compat plan;
    memset(&in, 0, sizeof(in));
    in.presentationMode = 0;
    in.trigger = DM1_V1_ROOM_TRANSITION_TRIGGER_STEP;
    in.before = pose(0, 5, 5, 0);
    in.after = pose(0, 5, 4, 0);
    EXPECT("guard_rc", DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan) == 1);
    EXPECT_INT("guard_inactive", plan.active, 0);
    EXPECT_INT("guard_visual_none", plan.visual, DM1_V1_ROOM_TRANSITION_VISUAL_NONE);
    EXPECT_INT("guard_no_redraw", plan.requestViewportRedraw, 0);
}

static void test_ordinary_step_is_redraw_only(void)
{
    struct Dm1V1RoomTransitionInputPc34Compat in;
    struct Dm1V1RoomTransitionPlanPc34Compat plan;
    memset(&in, 0, sizeof(in));
    in.presentationMode = DM1_V1_ROOM_TRANSITION_PRESENTATION_ORIGINAL;
    in.trigger = DM1_V1_ROOM_TRANSITION_TRIGGER_STEP;
    in.before = pose(0, 5, 5, 0);
    in.after = pose(0, 5, 4, 0);
    in.partyChampionCount = 4;
    EXPECT("step_rc", DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan) == 1);
    EXPECT_INT("step_active", plan.active, 1);
    EXPECT_INT("step_visual", plan.visual, DM1_V1_ROOM_TRANSITION_VISUAL_VIEWPORT_REDRAW);
    EXPECT_INT("step_redraw", plan.requestViewportRedraw, 1);
    EXPECT_INT("step_no_fade", plan.fadeFrames, 0);
    EXPECT_INT("step_no_wipe", plan.wipeFrames, 0);
    EXPECT_INT("step_no_walkout", plan.walkOutFrames, 0);
    EXPECT_INT("step_no_door_anim", plan.doorTransitionFrames, 0);
    EXPECT_INT("step_same_map_no_loader", plan.requestSetCurrentMapAndPartyMap, 0);
    EXPECT_INT("step_preserve_inventory", plan.preserveChampionInventories, 1);
    EXPECT_INT("step_preserve_stats", plan.preserveChampionStats, 1);
    EXPECT_INT("step_preserve_leader_hand", plan.preserveLeaderHandObject, 1);
}

static void test_map_change_requests_map_metadata_and_input_discard(void)
{
    struct Dm1V1RoomTransitionInputPc34Compat in;
    struct Dm1V1RoomTransitionPlanPc34Compat plan;
    memset(&in, 0, sizeof(in));
    in.presentationMode = DM1_V1_ROOM_TRANSITION_PRESENTATION_ORIGINAL;
    in.trigger = DM1_V1_ROOM_TRANSITION_TRIGGER_PIT;
    in.before = pose(0, 7, 8, 2);
    in.after = pose(1, 7, 8, 2);
    in.partyChampionCount = 4;
    in.postMoveChainCount = 1;
    in.postMovePitCount = 1;
    EXPECT("mapchange_rc", DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan) == 1);
    EXPECT_INT("mapchange_active", plan.active, 1);
    EXPECT_INT("mapchange_redraw", plan.requestViewportRedraw, 1);
    EXPECT_INT("mapchange_changed", plan.mapChanged, 1);
    EXPECT_INT("mapchange_set_current", plan.requestSetCurrentMap, 1);
    EXPECT_INT("mapchange_set_party", plan.requestSetCurrentMapAndPartyMap, 1);
    EXPECT_INT("mapchange_discard_input", plan.requestInputDiscard, 1);
    EXPECT_INT("mapchange_chain_limit", plan.maxChainCount, DM1_V1_ROOM_TRANSITION_CHAIN_LIMIT_PC34);
}

static void test_entrance_door_special_case(void)
{
    struct Dm1V1RoomTransitionInputPc34Compat in;
    struct Dm1V1RoomTransitionPlanPc34Compat plan;
    memset(&in, 0, sizeof(in));
    in.presentationMode = DM1_V1_ROOM_TRANSITION_PRESENTATION_ORIGINAL;
    in.trigger = DM1_V1_ROOM_TRANSITION_TRIGGER_ENTRANCE;
    in.before = pose(-1, 0, 0, 0);
    in.after = pose(0, 0, 0, 0);
    EXPECT("entrance_rc", DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan) == 1);
    EXPECT_INT("entrance_active", plan.active, 1);
    EXPECT_INT("entrance_visual", plan.visual, DM1_V1_ROOM_TRANSITION_VISUAL_ENTRANCE_DOORS);
    EXPECT_INT("entrance_frames", plan.entranceDoorFrames, DM1_V1_ROOM_TRANSITION_ENTRANCE_FRAMES);
    EXPECT_INT("entrance_vblank", plan.vblankWaitPerFrame, DM1_V1_ROOM_TRANSITION_ENTRANCE_VBLANKS_PC34);
    EXPECT_INT("entrance_redraw", plan.requestViewportRedraw, 1);
    EXPECT_INT("entrance_no_fade", plan.fadeFrames, 0);
}

static void test_source_evidence_mentions_required_files(void)
{
    const char* ev = DM1_V1_RoomTransition_SourceEvidencePc34Compat();
    EXPECT("evidence_gameloop", strstr(ev, "GAMELOOP.C:58-64,90") != NULL);
    EXPECT("evidence_movesens", strstr(ev, "MOVESENS.C:441-451") != NULL);
    EXPECT("evidence_dungeon", strstr(ev, "DUNGEON.C:1508-1558") != NULL);
    EXPECT("evidence_entrance", strstr(ev, "ENTRANCE.C:323-360") != NULL);
}

int main(void)
{
    test_non_v1_guard_disables_plan();
    test_ordinary_step_is_redraw_only();
    test_map_change_requests_map_metadata_and_input_discard();
    test_entrance_door_special_case();
    test_source_evidence_mentions_required_files();
    printf("dm1_v1_room_transition_pc34_compat: pass=%d fail=%d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
