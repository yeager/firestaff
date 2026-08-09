#include "m11_game_view.h"
#include "dm1_v1_creature_render_pc34_compat.h"
#include "dm1_v1_sound_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

typedef struct {
    int total;
    int passed;
} ProbeTally;

static void record(ProbeTally* tally, const char* id, int ok, const char* message) {
    tally->total += 1;
    if (ok) {
        tally->passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

static unsigned short make_thing(int type, int index) {
    return (unsigned short)(((type & 0x0F) << 10) | (index & 0x03FF));
}

static void set_next(unsigned char* raw, unsigned short nextThing) {
    if (!raw) return;
    raw[0] = (unsigned char)(nextThing & 0xFFu);
    raw[1] = (unsigned char)((nextThing >> 8) & 0xFFu);
}

static int map_square_base(const struct DungeonDatState_Compat* dungeon, int mapIndex) {
    int base = 0;
    int i;
    if (!dungeon || mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return -1;
    for (i = 0; i < mapIndex; ++i) {
        base += (int)dungeon->maps[i].width * (int)dungeon->maps[i].height;
    }
    return base;
}

static int setup_creature_runtime_view(M11_GameViewState* state,
                                       int creatureType,
                                       int groupX,
                                       int groupY,
                                       int partyX,
                                       int partyY,
                                       int resting) {
    struct DungeonDatState_Compat* dungeon;
    struct DungeonThings_Compat* things;
    const int mapCount = 2;
    const int squaresPerMap = 25;
    const int squareCount = mapCount * squaresPerMap;
    const int activeMapIndex = 1;
    int i;
    int m;

    if (!state) return 0;
    M11_GameView_Init(state);
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;
    snprintf(state->title, sizeof(state->title), "SYNTHETIC CREATURE AUDIO");
    snprintf(state->sourceId, sizeof(state->sourceId), "probe");

    dungeon = (struct DungeonDatState_Compat*)calloc(1, sizeof(*dungeon));
    things = (struct DungeonThings_Compat*)calloc(1, sizeof(*things));
    if (!dungeon || !things) return 0;

    dungeon->header.mapCount = mapCount;
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc((size_t)mapCount, sizeof(*dungeon->maps));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc((size_t)mapCount, sizeof(*dungeon->tiles));
    if (!dungeon->maps || !dungeon->tiles) return 0;

    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    for (m = 0; m < mapCount; ++m) {
        dungeon->maps[m].width = 5;
        dungeon->maps[m].height = 5;
        dungeon->maps[m].difficulty = 0;
        dungeon->tiles[m].squareCount = squaresPerMap;
        dungeon->tiles[m].squareData = (unsigned char*)calloc((size_t)squaresPerMap, sizeof(unsigned char));
        if (!dungeon->tiles[m].squareData) return 0;
        for (i = 0; i < squaresPerMap; ++i) {
            dungeon->tiles[m].squareData[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
        }
    }

    /* Compact PC34 first-thing layout.  Only squares that actually hold a
     * thing carry DUNGEON_SQUARE_MASK_THING_LIST; F0514 relies on that
     * invariant, appending to an existing chain on a flagged square and
     * inserting a fresh slot (memmove + cumulative-count fixup) for an
     * unflagged one.  Flagging every square instead leaves flagged-but-empty
     * squares, which is not a state the source can produce and which F0514
     * rejects outright — the group could then never link into its
     * destination.  Spare slots must read THING_NONE so F0514's insert path
     * has room to shift into. */
    dungeon->dungeonColumnCount = mapCount * 5;
    dungeon->columnsCumulativeSquareFirstThingCount =
        (uint16_t*)calloc((size_t)(dungeon->dungeonColumnCount + 1),
                          sizeof(uint16_t));
    if (!dungeon->columnsCumulativeSquareFirstThingCount) return 0;
    dungeon->header.squareFirstThingCount = (uint16_t)squareCount;

    things->squareFirstThings = (unsigned short*)calloc((size_t)squareCount, sizeof(unsigned short));
    things->groups = (struct DungeonGroup_Compat*)calloc(1, sizeof(*things->groups));
    things->rawThingData[THING_TYPE_GROUP] = (unsigned char*)calloc(16, sizeof(unsigned char));
    if (!things->squareFirstThings || !things->groups ||
        !things->rawThingData[THING_TYPE_GROUP]) {
        return 0;
    }

    for (i = 0; i < squareCount; ++i) {
        things->squareFirstThings[i] = THING_NONE;
    }

    things->loaded = 1;
    things->squareFirstThingCount = squareCount;
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    set_next(things->rawThingData[THING_TYPE_GROUP], THING_ENDOFLIST);

    things->groups[0].next = THING_ENDOFLIST;
    things->groups[0].slot = THING_NONE;
    things->groups[0].creatureType = (unsigned char)creatureType;
    things->groups[0].cells = 0;
    things->groups[0].health[0] = 200;
    things->groups[0].count = 0;
    things->groups[0].direction = 0;

    /* Authenticate the C04 record.  m11_group_live_position_matches_source
     * validates the decoded group against these raw bytes, and a record
     * carrying only Next left every non-zero creatureType (red dragon C24)
     * unauthenticated, so the tick loop skipped the group entirely and it
     * never moved or emitted a sound.  Layout per DUNGEON.C C04:
     * [0..1]=Next, [2..3]=Slot, [4]=Type, [5]=Cells, [6..7]=Health[0],
     * [14]=Behavior|Count<<5, [15]=Direction. */
    {
        unsigned char* raw = things->rawThingData[THING_TYPE_GROUP];
        raw[2] = (unsigned char)(things->groups[0].slot & 0xffu);
        raw[3] = (unsigned char)(things->groups[0].slot >> 8);
        raw[4] = things->groups[0].creatureType;
        raw[5] = things->groups[0].cells;
        raw[6] = (unsigned char)(things->groups[0].health[0] & 0xffu);
        raw[7] = (unsigned char)(things->groups[0].health[0] >> 8);
        raw[14] = (unsigned char)((things->groups[0].behavior & 0x0fu) |
                                  ((things->groups[0].count & 0x03u) << 5));
        raw[15] = (unsigned char)(things->groups[0].direction & 0x03u);
    }

    /* The group's square is the only occupied one, so it is the only flagged
     * square and therefore compact slot 0; every column at or after it is
     * preceded by that single flagged square. */
    dungeon->tiles[activeMapIndex]
        .squareData[groupX * dungeon->maps[activeMapIndex].height + groupY] |=
        DUNGEON_SQUARE_MASK_THING_LIST;
    things->squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    {
        int firstCol = activeMapIndex * 5;
        int c;
        for (c = 0; c <= dungeon->dungeonColumnCount; ++c) {
            dungeon->columnsCumulativeSquareFirstThingCount[c] =
                (uint16_t)((c > firstCol + groupX) ? 1 : 0);
        }
    }

    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.partyMapIndex = activeMapIndex;
    state->world.newPartyMapIndex = -1;
    state->world.party.mapIndex = activeMapIndex;
    state->world.party.mapX = partyX;
    state->world.party.mapY = partyY;
    state->world.party.direction = 0;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    memcpy(state->world.party.champions[0].name, "TIGGY", 5);
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    state->world.party.champions[0].stamina.current = 100;
    state->world.party.champions[0].stamina.maximum = 100;
    state->world.party.champions[0].food = 200;
    state->world.party.champions[0].water = 200;
    state->world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 0;
    state->world.magic.magicalLightAmount = 150;
    state->world.partyIsResting = resting ? 1 : 0;
    state->resting = resting ? 1 : 0;
    return 1;
}

static int group_is_at(const M11_GameViewState* state, int x, int y) {
    if (!state || !state->world.dungeon || !state->world.things) return 0;
    /* Resolve through the source accessor rather than a flat array index:
     * the fixture uses the compact PC34 first-thing layout, where a square's
     * slot is derived from the per-column cumulative counts over flagged
     * squares and is NOT the flat (x * height + y) offset. */
    return F0511_DUNGEON_GetSquareFirstThing_Compat(
               state->world.dungeon, state->world.things,
               state->world.party.mapIndex, x, y) ==
           make_thing(THING_TYPE_GROUP, 0);
}

static void run_attack_runtime_probe(ProbeTally* tally) {
    M11_GameViewState state;
    memset(&state, 0, sizeof(state));
    record(tally, "INV_CREATURE_AUDIO_ATTACK_SETUP",
           setup_creature_runtime_view(&state, DM1_CREATURE_GIANT_SCORPION, 2, 2, 2, 2, 1),
           "synthetic M11 scorpion-on-party state initializes");
    state.world.gameTick = 19;
    record(tally, "INV_CREATURE_AUDIO_ATTACK_RUNTIME",
           M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW,
           "idle tick runs through M11 creature attack runtime path");
    record(tally, "INV_CREATURE_AUDIO_ATTACK_INDEX",
           state.audioState.lastSoundIndex == DM1_SND_ATTACK_SCORPION,
           "scorpion attack emits source sound index 20");
    /* DM1 deliberately emits NO generated fallback marker.
     * m11_audio_emit_source_sound_with_volume routes every DM1 source kind
     * through M11_Audio_EmitSourceSoundIndex, which carries the SND3 event
     * index alone: "A missing or malformed source record is silent; a
     * generated marker would make an unverified effect sound plausible while
     * being wrong" (ReDMCSB SOUND.C F0060). Asserting a CREATURE marker here
     * would demand exactly the synthetic cue that rule forbids, so the gate
     * now pins the real contract — the source index fires and no procedural
     * marker is substituted. */
    record(tally, "INV_CREATURE_AUDIO_ATTACK_MARKER",
           state.audioState.lastMarker == M11_AUDIO_MARKER_NONE,
           "attack emits the source SND3 index with no generated marker cue");
    M11_GameView_Shutdown(&state);
}

static void run_movement_runtime_probe(ProbeTally* tally) {
    M11_GameViewState state;
    memset(&state, 0, sizeof(state));
    record(tally, "INV_CREATURE_AUDIO_MOVE_SETUP",
           setup_creature_runtime_view(&state, DM1_CREATURE_RED_DRAGON, 2, 3, 2, 1, 0),
           "synthetic M11 red-dragon movement state initializes");
    /* C24 Red Dragon carries movementTicks 13 (DUNGEON.C G0243[24]), so the
     * M11 movement cadence (gameTick % movementTicks == 0) admits tick 13,
     * not tick 12.  Start at 12 so AdvanceIdleTick lands on it. */
    state.world.gameTick = 12;
    record(tally, "INV_CREATURE_AUDIO_MOVE_RUNTIME",
           M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW && group_is_at(&state, 2, 2),
           "idle tick moves creature through M11 runtime path");
    record(tally, "INV_CREATURE_AUDIO_MOVE_INDEX",
           state.audioState.lastSoundIndex == DM1_SND_MOVE_RED_DRAGON,
           "red dragon movement emits source sound index 33");
    /* Same DM1 no-synthetic-marker rule as the attack gate above. */
    record(tally, "INV_CREATURE_AUDIO_MOVE_MARKER",
           state.audioState.lastMarker == M11_AUDIO_MARKER_NONE,
           "movement emits the source SND3 index with no generated marker cue");
    M11_GameView_Shutdown(&state);
}

static void run_resting_gate_probe(ProbeTally* tally) {
    M11_GameViewState state;
    memset(&state, 0, sizeof(state));
    record(tally, "INV_CREATURE_AUDIO_REST_SETUP",
           setup_creature_runtime_view(&state, DM1_CREATURE_RED_DRAGON, 2, 3, 2, 1, 1),
           "synthetic resting movement state initializes");
    /* C24 Red Dragon carries movementTicks 13 (DUNGEON.C G0243[24]), so the
     * M11 movement cadence (gameTick % movementTicks == 0) admits tick 13,
     * not tick 12.  Start at 12 so AdvanceIdleTick lands on it. */
    state.world.gameTick = 12;
    record(tally, "INV_CREATURE_AUDIO_REST_RUNTIME",
           M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW && group_is_at(&state, 2, 2),
           "creature still moves while party-resting sound gate is tested");
    record(tally, "INV_CREATURE_AUDIO_REST_NO_SOUND",
           state.audioState.lastSoundIndex == -1 && state.audioState.lastMarker == M11_AUDIO_MARKER_NONE,
           "ReDMCSB F0514 resting gate emits no movement SFX");
    M11_GameView_Shutdown(&state);
}

int main(void) {
    ProbeTally tally = {0, 0};
    run_attack_runtime_probe(&tally);
    run_movement_runtime_probe(&tally);
    run_resting_gate_probe(&tally);
    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return tally.passed == tally.total ? 0 : 1;
}
