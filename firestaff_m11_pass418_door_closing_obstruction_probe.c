/*
 * Pass 418 — DM1 V1 door closing obstruction parity.
 *
 * Source audit anchors:
 *   ReDMCSB PC 3.4 TIMELINE.C:749-754 reads the door square/state and
 *   pre-increments Map_Time for C01_EVENT_DOOR_ANIMATION.
 *   TIMELINE.C:759-774 handles a closing door on the party square: force
 *   the door open, damage all champions for 5, increment Map_Time again,
 *   and reschedule.
 *   TIMELINE.C:779-797 handles material creatures: compare doorState to
 *   vertical ? creatureHeight : 1, damage the group for 5, step one state
 *   toward open, and reschedule after the single pre-increment.
 *   TIMELINE.C:803-817 is the normal +/-1 animation path.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_door_action_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

#define MAP_W 3
#define MAP_H 3
static int g_pass, g_fail;

static unsigned char sqb(int elementType, int attribs) {
    return (unsigned char)(((elementType & 7) << 5) | (attribs & 0x1F));
}
static void rec(const char* id, int ok, const char* msg) {
    if (ok) { ++g_pass; printf("PASS %s %s\n", id, msg); }
    else { ++g_fail; printf("FAIL %s %s\n", id, msg); }
}
static int square_state(const struct DungeonDatState_Compat* d, int x, int y) {
    return d->tiles[0].squareData[x * d->maps[0].height + y] & 0x07;
}
static void build_fixture(struct DungeonDatState_Compat* dungeon, unsigned char* squares) {
    int i;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(squares, 0, MAP_W * MAP_H);
    dungeon->header.mapCount = 1;
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(*dungeon->maps));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(*dungeon->tiles));
    dungeon->maps[0].width = MAP_W;
    dungeon->maps[0].height = MAP_H;
    dungeon->tiles[0].squareData = squares;
    dungeon->tiles[0].squareCount = MAP_W * MAP_H;
    dungeon->tilesLoaded = 1;
    dungeon->loaded = 1;
    for (i = 0; i < MAP_W * MAP_H; ++i) squares[i] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);
    squares[1 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 2); /* closing mid-state */
}
static void free_fixture(struct DungeonDatState_Compat* dungeon) {
    free(dungeon->maps);
    free(dungeon->tiles);
}
static int has_emit(const struct TickResult_Compat* r, int kind) {
    int i;
    for (i = 0; i < r->emissionCount; ++i) if (r->emissions[i].kind == kind) return 1;
    return 0;
}

int main(void) {
    struct DoorClosingObstruction_Compat o;
    struct DungeonDatState_Compat dungeon;
    unsigned char squares[MAP_W * MAP_H];
    struct GameWorld_Compat world;
    struct TimelineEvent_Compat ev;
    struct TickResult_Compat result;

    memset(&o, 0, sizeof(o));
    rec("P418_PARTY_OBSTRUCTION_PURE",
        F0717_DOOR_ResolveClosingObstruction_Compat(2, 0, 1, 4, 0, 0, &o) &&
        o.kind == DOOR_OBSTRUCTION_PARTY && o.newDoorState == 0 &&
        o.rescheduleDelayTicks == 2 && o.damageAmount == 5 &&
        o.woundMask == DOOR_OBSTRUCTION_WOUND_HEAD,
        "party on a non-open closing door forces OPEN, damage=5, delay=2, preserves PC34 BUG0_78 head wound precedence");

    memset(&o, 0, sizeof(o));
    rec("P418_CREATURE_HORIZONTAL_PURE",
        F0717_DOOR_ResolveClosingObstruction_Compat(2, 0, 0, 0, 1, 4, &o) &&
        o.kind == DOOR_OBSTRUCTION_CREATURE && o.newDoorState == 1 &&
        o.rescheduleDelayTicks == 1 && o.damageAmount == 5,
        "horizontal material creature obstruction uses threshold 1 and steps door one state toward open");

    memset(&o, 0, sizeof(o));
    rec("P418_CREATURE_VERTICAL_HEIGHT_GATE",
        F0717_DOOR_ResolveClosingObstruction_Compat(2, 1, 0, 0, 1, 4, &o) &&
        o.kind == DOOR_OBSTRUCTION_NONE && o.newDoorState == 2,
        "vertical material creature obstruction requires doorState >= creatureHeight");

    memset(&o, 0, sizeof(o));
    rec("P418_NO_PARTY_ON_OPEN",
        F0717_DOOR_ResolveClosingObstruction_Compat(0, 0, 1, 4, 0, 0, &o) &&
        o.kind == DOOR_OBSTRUCTION_NONE,
        "open door under party does not take the obstruction branch");

    build_fixture(&dungeon, squares);
    memset(&world, 0, sizeof(world));
    F0881_WORLD_InitDefault_Compat(&world, 1234u);
    world.dungeon = &dungeon;
    world.ownsDungeon = 0;
    world.party.mapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.championCount = 4;
    world.party.champions[0].present = 1;
    world.gameTick = 10;

    memset(&ev, 0, sizeof(ev));
    ev.kind = TIMELINE_EVENT_DOOR_ANIMATE;
    ev.fireAtTick = 10;
    ev.mapIndex = 0;
    ev.mapX = 1;
    ev.mapY = 1;
    ev.aux1 = DOOR_EFFECT_CLEAR;
    F0721_TIMELINE_Schedule_Compat(&world.timeline, &ev);

    memset(&result, 0, sizeof(result));
    F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    rec("P418_ORCH_PARTY_FORCES_OPEN",
        square_state(&dungeon, 1, 1) == 0 &&
        has_emit(&result, EMIT_DOOR_STATE) &&
        has_emit(&result, EMIT_DAMAGE_DEALT) &&
        has_emit(&result, EMIT_SOUND_REQUEST),
        "orchestrator CLEAR event on occupied door forces square open and emits door/damage/sound markers");

    rec("P418_ORCH_RESCHEDULES_PLUS_TWO",
        F0722_TIMELINE_Peek_Compat(&world.timeline, &ev) == 1 &&
        ev.kind == TIMELINE_EVENT_DOOR_ANIMATE && ev.fireAtTick == 12 &&
        ev.aux1 == DOOR_EFFECT_CLEAR,
        "occupied-door CLEAR event is rescheduled two ticks later, matching TIMELINE.C party branch");

    free_fixture(&dungeon);
    printf("# summary: %d/%d passed\n", g_pass, g_pass + g_fail);
    return g_fail ? 1 : 0;
}
