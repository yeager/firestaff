#ifndef FIRESTAFF_DM1_V1_ROOM_TRANSITION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ROOM_TRANSITION_PC34_COMPAT_H

/*
 * DM1 V1 room/party-square transition planner.
 *
 * This is deliberately a presentation plan, not a camera tween system.
 * ReDMCSB DM1 V1 does not fade, wipe, or walk the party sprite out of sight
 * for ordinary dungeon movement.  The game mutates party/map state, then the
 * main loop redraws the viewport from the new square.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   GAMELOOP.C:58-64  deferred new-party-map processing, discard input
 *   GAMELOOP.C:90     redraw dungeon view each visible loop
 *   MOVESENS.C:441-451 party X/Y and direction carried into F0267
 *   MOVESENS.C:492-517 teleporter map switch and party rotation
 *   MOVESENS.C:538-606 pit fall chain, map switch, fall damage/rope reset
 *   MOVESENS.C:817-822 same-map sensor enter vs deferred new-party-map
 *   DUNGEON.C:1508-1558 map lookup after level change
 *   DUNGEON.C:2724-2740 SetCurrentMap: map data pointer/door info/columns
 *   DUNGEON.C:2742-2762 SetCurrentMapAndPartyMap: party map metadata arrays
 *   ENTRANCE.C:323-360 entrance door reveal loop, vblank cadence
 */

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_ROOM_TRANSITION_PRESENTATION_ORIGINAL 1
#define DM1_V1_ROOM_TRANSITION_ENTRANCE_FRAMES 32
#define DM1_V1_ROOM_TRANSITION_ENTRANCE_VBLANKS_PC34 3
#define DM1_V1_ROOM_TRANSITION_CHAIN_LIMIT_PC34 100

/* What caused the transition plan. */
enum Dm1V1RoomTransitionTriggerPc34Compat {
    DM1_V1_ROOM_TRANSITION_TRIGGER_NONE = 0,
    DM1_V1_ROOM_TRANSITION_TRIGGER_STEP = 1,
    DM1_V1_ROOM_TRANSITION_TRIGGER_TELEPORTER = 2,
    DM1_V1_ROOM_TRANSITION_TRIGGER_PIT = 3,
    DM1_V1_ROOM_TRANSITION_TRIGGER_STAIRS = 4,
    DM1_V1_ROOM_TRANSITION_TRIGGER_NEW_PARTY_MAP = 5,
    DM1_V1_ROOM_TRANSITION_TRIGGER_ENTRANCE = 6
};

/* The only visual transition kinds source-locked for this pass. */
enum Dm1V1RoomTransitionVisualPc34Compat {
    DM1_V1_ROOM_TRANSITION_VISUAL_NONE = 0,
    DM1_V1_ROOM_TRANSITION_VISUAL_VIEWPORT_REDRAW = 1,
    DM1_V1_ROOM_TRANSITION_VISUAL_ENTRANCE_DOORS = 2
};

struct Dm1V1RoomTransitionPosePc34Compat {
    int mapIndex;
    int mapX;
    int mapY;
    int direction;
};

struct Dm1V1RoomTransitionInputPc34Compat {
    int presentationMode;
    enum Dm1V1RoomTransitionTriggerPc34Compat trigger;
    struct Dm1V1RoomTransitionPosePc34Compat before;
    struct Dm1V1RoomTransitionPosePc34Compat after;
    int partyChampionCount;
    int postMoveChainCount;
    int postMovePitCount;
    int postMoveTeleporterCount;
};

struct Dm1V1RoomTransitionPlanPc34Compat {
    int active;
    enum Dm1V1RoomTransitionTriggerPc34Compat trigger;
    enum Dm1V1RoomTransitionVisualPc34Compat visual;

    /* Ordinary dungeon transitions: redraw only. */
    int requestViewportRedraw;
    int fadeFrames;
    int wipeFrames;
    int walkOutFrames;
    int doorTransitionFrames;

    /* Entrance special-case animation. */
    int entranceDoorFrames;
    int vblankWaitPerFrame;

    /* Dungeon-state/data consequences. */
    int mapChanged;
    int requestSetCurrentMap;
    int requestSetCurrentMapAndPartyMap;
    int requestInputDiscard;
    int maxChainCount;

    /* State preservation facts from ReDMCSB F0267. */
    int preserveChampionInventories;
    int preserveChampionStats;
    int preserveLeaderHandObject;
    int preservePartyDirectionUnlessTeleporterRotates;

    struct Dm1V1RoomTransitionPosePc34Compat before;
    struct Dm1V1RoomTransitionPosePc34Compat after;
};

void DM1_V1_RoomTransition_InitPlanPc34Compat(
    struct Dm1V1RoomTransitionPlanPc34Compat* plan);

int DM1_V1_RoomTransition_BuildPlanPc34Compat(
    const struct Dm1V1RoomTransitionInputPc34Compat* input,
    struct Dm1V1RoomTransitionPlanPc34Compat* outPlan);

const char* DM1_V1_RoomTransition_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_ROOM_TRANSITION_PC34_COMPAT_H */
