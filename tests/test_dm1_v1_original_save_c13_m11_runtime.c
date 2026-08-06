#include "m11_game_view.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_resurrection_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n", message); \
            ++failures; \
        } \
    } while (0)

static int advance_movement_tick(M11_GameViewState *state)
{
    return M11_GameView_HandleInput(state, M12_MENU_INPUT_TURN_LEFT) ==
           M11_GAME_INPUT_REDRAW;
}

static int pending_rebirth_event_count(const struct TimelineQueue_Compat *timeline)
{
    int count = 0;
    int i;

    if (!timeline) {
        return 0;
    }
    for (i = 0; i < timeline->count; ++i) {
        if (timeline->events[i].kind == TIMELINE_EVENT_VI_ALTAR_REBIRTH) {
            ++count;
        }
    }
    return count;
}

int main(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junk;
    struct TimelineEvent_Compat event;
    unsigned char squareData[1];
    unsigned short firstThing[1];
    unsigned char rawJunk[4];
    unsigned short columnSftBases[1];
    int tick;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(&junk, 0, sizeof(junk));
    memset(&event, 0, sizeof(event));
    memset(squareData, 0, sizeof(squareData));
    memset(rawJunk, 0, sizeof(rawJunk));

    map.width = 1;
    map.height = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    /* F0160 resolves the flagged square through the G0280 per-column
     * compact-SFT base loaded from DUNGEON.DAT (db1e5846e).  This
     * single-column map keeps its only flagged square at entry zero. */
    columnSftBases[0] = 0u;
    dungeon.columnsCumulativeSquareFirstThingCount = columnSftBases;
    dungeon.dungeonColumnCount = 1;
    tiles.squareData = squareData;
    tiles.squareCount = 1;

    squareData[0] = DUNGEON_SQUARE_MASK_THING_LIST;
    firstThing[0] = (unsigned short)((THING_TYPE_JUNK << 10) | (1u << 14));
    junk.next = THING_ENDOFLIST;
    junk.type = DM1_JUNK_TYPE_BONES;
    junk.doNotDiscard = 1u;
    rawJunk[0] = 0xfeu;
    rawJunk[1] = 0xffu;
    rawJunk[2] = (unsigned char)(DM1_JUNK_TYPE_BONES | 0x80u);
    things.squareFirstThings = firstThing;
    things.squareFirstThingCount = 1;
    things.junks = &junk;
    things.junkCount = 1;
    things.rawThingData[THING_TYPE_JUNK] = rawJunk;
    things.thingCounts[THING_TYPE_JUNK] = 1;
    things.loaded = 1;

    M11_GameView_Init(&state);
    state.active = 1;
    state.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    strcpy(state.sourceId, "dm1");
    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].cell = 1;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].inventory[0] = 0x1234u;
    CHECK(F0720_TIMELINE_Init_Compat(&state.world.timeline, 0u),
          "tail-backed C13 M11 timeline initializes");

    event.kind = TIMELINE_EVENT_VI_ALTAR_REBIRTH;
    event.fireAtTick = 1u;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.cell = 1;
    event.aux0 = DM1_EVENT_VI_ALTAR_REBIRTH;
    event.aux1 = 2;
    event.aux4 = 0;
    CHECK(F0721_TIMELINE_Schedule_Compat(&state.world.timeline, &event),
          "tail-backed C13 step 2 reaches live M11 timeline");

    for (tick = 0; tick < 7; ++tick) {
        CHECK(advance_movement_tick(&state),
              "M11 movement pipeline advances an admitted C13 tick");
    }

    CHECK(state.world.gameTick == 7u,
          "M11 movement path advances the source timeline clock");
    CHECK(firstThing[0] == THING_ENDOFLIST,
          "M11 C13 step 1 unlinks the authenticated bones record");
    CHECK(pending_rebirth_event_count(&state.world.timeline) == 0,
          "M11 C13 sequence consumes all source-backed transition steps");
    CHECK(state.world.party.champions[0].hp.maximum == 98u &&
              state.world.party.champions[0].hp.current == 49u,
          "M11 C13 terminal step applies F0283 health mutation");
    CHECK(state.world.party.champions[0].direction ==
              state.world.party.direction &&
              state.world.party.champions[0].inventory[0] == THING_NONE,
          "M11 C13 terminal step applies F0283 direction and inventory state");
    {
        uint32_t liveWorldHash = 0u;
        CHECK(F0891_ORCH_WorldHash_Compat(&state.world, &liveWorldHash) &&
                  state.lastWorldHash == liveWorldHash && liveWorldHash != 0u,
              "M11 republishes the post-C13 F0887 world hash for live save and HoC state");
    }

    M11_GameView_Shutdown(&state);
    if (failures != 0) {
        fprintf(stderr, "DM1 C13 M11 runtime: %d failure(s)\n", failures);
        return 1;
    }
    puts("PASS dm1_v1_original_save_c13_m11_runtime");
    return 0;
}
