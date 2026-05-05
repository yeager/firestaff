#include "dm1_v1_room_transition_pc34_compat.h"

#include <string.h>

void DM1_V1_RoomTransition_InitPlanPc34Compat(
    struct Dm1V1RoomTransitionPlanPc34Compat* plan)
{
    if (!plan) return;
    memset(plan, 0, sizeof(*plan));
    plan->visual = DM1_V1_ROOM_TRANSITION_VISUAL_NONE;
    plan->maxChainCount = DM1_V1_ROOM_TRANSITION_CHAIN_LIMIT_PC34;
}

static int dm1_v1_pose_changed(
    const struct Dm1V1RoomTransitionPosePc34Compat* before,
    const struct Dm1V1RoomTransitionPosePc34Compat* after)
{
    return before->mapIndex != after->mapIndex ||
           before->mapX != after->mapX ||
           before->mapY != after->mapY ||
           before->direction != after->direction;
}

static int dm1_v1_trigger_is_room_entry(
    enum Dm1V1RoomTransitionTriggerPc34Compat trigger)
{
    return trigger == DM1_V1_ROOM_TRANSITION_TRIGGER_STEP ||
           trigger == DM1_V1_ROOM_TRANSITION_TRIGGER_TELEPORTER ||
           trigger == DM1_V1_ROOM_TRANSITION_TRIGGER_PIT ||
           trigger == DM1_V1_ROOM_TRANSITION_TRIGGER_STAIRS ||
           trigger == DM1_V1_ROOM_TRANSITION_TRIGGER_NEW_PARTY_MAP;
}

int DM1_V1_RoomTransition_BuildPlanPc34Compat(
    const struct Dm1V1RoomTransitionInputPc34Compat* input,
    struct Dm1V1RoomTransitionPlanPc34Compat* outPlan)
{
    int changed;

    if (!input || !outPlan) return 0;
    DM1_V1_RoomTransition_InitPlanPc34Compat(outPlan);

    /* V1 presentation mode guard: this module must not affect non-original
     * presentation modes.  The M11 launcher may select several render modes;
     * V1 parity begins only after selecting the original DM1 V1 presentation. */
    if (input->presentationMode != DM1_V1_ROOM_TRANSITION_PRESENTATION_ORIGINAL) {
        return 1;
    }

    outPlan->trigger = input->trigger;
    outPlan->before = input->before;
    outPlan->after = input->after;
    outPlan->mapChanged = (input->before.mapIndex != input->after.mapIndex);

    if (input->trigger == DM1_V1_ROOM_TRANSITION_TRIGGER_ENTRANCE) {
        /* Source: ENTRANCE.C:323-360 loops until both entrance-door zones have
         * zero width; ENTRANCE.C:323-324 increments by two graphics per step;
         * ENTRANCE.C:352-359 waits for vblank cadence before drawing each step.
         * PC34/F20/F31 entrance doors are a special start-game reveal, not a
         * reusable per-room fade/wipe. */
        outPlan->active = 1;
        outPlan->visual = DM1_V1_ROOM_TRANSITION_VISUAL_ENTRANCE_DOORS;
        outPlan->entranceDoorFrames = DM1_V1_ROOM_TRANSITION_ENTRANCE_FRAMES;
        outPlan->vblankWaitPerFrame = DM1_V1_ROOM_TRANSITION_ENTRANCE_VBLANKS_PC34;
        outPlan->requestViewportRedraw = 1; /* ENTRANCE.C:367 after reveal. */
        return 1;
    }

    changed = dm1_v1_pose_changed(&input->before, &input->after);
    if (!changed || !dm1_v1_trigger_is_room_entry(input->trigger)) {
        return 1;
    }

    /* Ordinary dungeon room/square entry is redraw-only.
     * Source: GAMELOOP.C:90 calls F0128_DUNGEONVIEW_Draw_CPSF with the current
     * party direction/X/Y.  MOVESENS.C:441-443 updates party X/Y before the
     * post-move chain; MOVESENS.C:817-822 either processes same-map enter
     * sensors or defers a new party map.  There is no ReDMCSB fade, wipe,
     * per-room door reveal, or party walk-out animation in this path. */
    outPlan->active = 1;
    outPlan->visual = DM1_V1_ROOM_TRANSITION_VISUAL_VIEWPORT_REDRAW;
    outPlan->requestViewportRedraw = 1;
    outPlan->fadeFrames = 0;
    outPlan->wipeFrames = 0;
    outPlan->walkOutFrames = 0;
    outPlan->doorTransitionFrames = 0;

    /* DUNGEON.DAT / current-map data consequences.
     * Source: DUNGEON.C:1508-1558 converts pit/stairs global coordinates to
     * target map index.  DUNGEON.C:2724-2740 changes current map data pointers,
     * dimensions, door-info sets, and cumulative thing columns.  DUNGEON.C:
     * 2742-2762 additionally marks party map and copies per-map creature,
     * wall/floor/door ornament metadata.  GAMELOOP.C:58-64 calls the new-map
     * processor and discards queued input after the deferred map switch. */
    if (outPlan->mapChanged) {
        outPlan->requestSetCurrentMap = 1;
        outPlan->requestSetCurrentMapAndPartyMap = 1;
        outPlan->requestInputDiscard = 1;
    }

    /* State preservation.
     * Source: MOVESENS.C:441-451 mutates party map X/Y and carries direction;
     * teleporter rotation only changes direction at MOVESENS.C:505-517.  Pit
     * fall damage/stamina/rope reset happens at MOVESENS.C:578-606.  Champion
     * inventories, champion stats other than damage/stamina consequences, and
     * leader hand object are not reloaded by DUNGEON.C:2724-2762 map pointer
     * switches; they remain in champion/hand globals outside current-map data. */
    outPlan->preserveChampionInventories = 1;
    outPlan->preserveChampionStats = 1;
    outPlan->preserveLeaderHandObject = 1;
    outPlan->preservePartyDirectionUnlessTeleporterRotates = 1;

    if (input->postMoveChainCount > DM1_V1_ROOM_TRANSITION_CHAIN_LIMIT_PC34 ||
        input->postMovePitCount < 0 || input->postMoveTeleporterCount < 0 ||
        input->partyChampionCount < 0) {
        return 0;
    }

    return 1;
}

const char* DM1_V1_RoomTransition_SourceEvidencePc34Compat(void)
{
    return "GAMELOOP.C:58-64,90; MOVESENS.C:441-451,492-517,538-606,817-822; "
           "DUNGEON.C:1508-1558,2724-2740,2742-2762; ENTRANCE.C:323-360,367";
}
