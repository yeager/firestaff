#include "m11_game_view.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"

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

int main(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct TimelineEvent_Compat event;
    DM1_ActionDefenseInputPc34 defenseInput;
    DM1_ActionDefensePlanPc34 defensePlan;
    unsigned char squareData[1];

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&event, 0, sizeof(event));
    memset(&defenseInput, 0, sizeof(defenseInput));
    memset(&defensePlan, 0, sizeof(defensePlan));
    memset(squareData, 0, sizeof(squareData));

    map.width = 1;
    map.height = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    tiles.squareData = squareData;
    tiles.squareCount = 1;

    M11_GameView_Init(&state);
    state.active = 1;
    state.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    strcpy(state.sourceId, "dm1");
    state.world.dungeon = &dungeon;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].actionIndex = DM1_ACTION_SWING;
    defenseInput.actionIndex = DM1_ACTION_SWING;
    CHECK(dm1_v1_action_defense_apply_plan_f0407_pc34(
              &defenseInput, &defensePlan) && defensePlan.valid,
          "C11 fixture obtains the source action-defense owner");
    state.world.party.champions[0].actionDefense = defensePlan.defenseDelta;
    state.actionDisabledTicks[0] = 3u;
    state.actionDisabledIndex[0] = DM1_ACTION_SWING;
    state.actionEnableSlotOrdinal[0] = 0u;
    CHECK(F0720_TIMELINE_Init_Compat(&state.world.timeline, 0u),
          "C11 M11 timeline initializes");

    event.kind = TIMELINE_EVENT_ENABLE_CHAMPION_ACTION;
    event.fireAtTick = 1u;
    event.aux0 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    event.aux1 = 0;
    event.aux2 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    event.aux4 = 0;
    CHECK(F0721_TIMELINE_Schedule_Compat(&state.world.timeline, &event),
          "tail-backed C11 reaches the live M11 timeline");

    CHECK(M11_GameView_HandleInput(&state, M12_MENU_INPUT_TURN_LEFT) ==
              M11_GAME_INPUT_REDRAW,
          "M11 movement pipeline advances the admitted C11 tick");
    CHECK(state.world.gameTick == 1u,
          "C11 movement tick advances the source timeline clock");
    CHECK(state.world.timeline.count == 0,
          "C11 is consumed from the live source timeline");
    CHECK(state.actionDisabledTicks[0] == 0u &&
              state.actionDisabledIndex[0] == 0xffu &&
              state.actionEnableSlotOrdinal[0] == 0xffu,
          "C11 reaches the matching M11 action-lock owner once");
    CHECK(state.world.party.champions[0].actionDefense == 0 &&
              state.world.party.champions[0].actionIndex == 0xffu,
          "C11 applies F0253 action-defense completion in live M11 state");
    {
        uint32_t liveWorldHash = 0u;
        CHECK(F0891_ORCH_WorldHash_Compat(&state.world, &liveWorldHash) &&
                  state.lastWorldHash == liveWorldHash && liveWorldHash != 0u,
              "M11 republishes the post-C11 F0891 world hash");
    }

    M11_GameView_Shutdown(&state);
    if (failures != 0) {
        fprintf(stderr, "DM1 C11 M11 runtime: %d failure(s)\n", failures);
        return 1;
    }
    puts("PASS dm1_v1_original_save_c11_m11_runtime");
    return 0;
}
