/* ReDMCSB MENU.C F0407:1620-1622 live SWING -> F0330 C11 receipt. */
#include "m11_game_view.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define REQUIRE(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static unsigned short make_thing(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static unsigned char square_for_test(int elementType, int attributes)
{
    return (unsigned char)(((elementType & 7) << 5) | (attributes & 31));
}

static const struct TimelineEvent_Compat* find_pending_c11(
    const struct GameWorld_Compat* world)
{
    int i;

    if (!world) return NULL;
    for (i = 0; i < world->timeline.count; ++i) {
        const struct TimelineEvent_Compat* event = &world->timeline.events[i];
        if (event->kind == TIMELINE_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux0 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux2 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux4 == 0) {
            return event;
        }
    }
    return NULL;
}

int main(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonDoor_Compat door;
    struct TimelineEvent_Compat earlierC11;
    unsigned char squareData[9];
    unsigned short squareFirstThing[1];
    unsigned char weaponRaw[4];
    unsigned char doorRaw[4];
    unsigned char actions[3];
    const struct TimelineEvent_Compat* event;
    int swingRow = -1;
    int actionDefenseAfterBegin;
    (void)actionDefenseAfterBegin;
    unsigned int c11Tick;
    unsigned int localLockExpiryTick;
    int i;

    memset(&state, 0, sizeof(state));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(&weapon, 0, sizeof(weapon));
    memset(&door, 0, sizeof(door));
    memset(squareData, 0, sizeof(squareData));
    memset(weaponRaw, 0, sizeof(weaponRaw));
    memset(doorRaw, 0, sizeof(doorRaw));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.gameTick = 7u;
    state.world.party.championCount = 1;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].stamina.current = 100;
    state.world.party.champions[0].stamina.maximum = 100;
    state.world.party.champions[0].food = 2048;
    state.world.party.champions[0].water = 2048;
    state.world.party.champions[0].actionIndex = 0xFFu;
    memcpy(state.world.party.champions[0].name, "HALK", 4u);
    state.world.party.champions[0].direction = 2;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;

    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    map.width = 3;
    map.height = 3;
    map.doorSet0 = 1;
    tiles.squareData = squareData;
    tiles.squareCount = 9;
    squareData[7] = square_for_test(
        DUNGEON_ELEMENT_DOOR, DUNGEON_SQUARE_MASK_THING_LIST | 4);
    squareFirstThing[0] = make_thing(THING_TYPE_DOOR, 0);
    weapon.type = 2; /* PC34 ActionSet 5 exposes SWING. */
    weaponRaw[0] = 0xfe;
    weaponRaw[1] = 0xff;
    weaponRaw[2] = weapon.type;
    door.next = THING_ENDOFLIST;
    doorRaw[0] = 0xfe;
    doorRaw[1] = 0xff;
    door.type = 0;
    door.meleeDestructible = 1;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThing;
    things.squareFirstThingCount = 1;
    things.weapons = &weapon;
    things.weaponCount = 1;
    things.rawThingData[THING_TYPE_WEAPON] = weaponRaw;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    things.doors = &door;
    things.doorCount = 1;
    things.rawThingData[THING_TYPE_DOOR] = doorRaw;
    things.thingCounts[THING_TYPE_DOOR] = 1;
    state.world.dungeon = &dungeon;
    state.world.things = &things;

    /* A prior source-shaped C11 makes F0330 apply its exact existing-owner
     * delay rule.  The SWING lock's ordinary duration now expires before
     * the replacement C11; only that replacement receipt may run F0253. */
    memset(&earlierC11, 0, sizeof(earlierC11));
    earlierC11.kind = TIMELINE_EVENT_ENABLE_CHAMPION_ACTION;
    earlierC11.fireAtTick = state.world.gameTick + 20u;
    earlierC11.aux0 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    earlierC11.aux2 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    earlierC11.aux4 = 0;
    REQUIRE(F0721_TIMELINE_Schedule_Compat(
        &state.world.timeline, &earlierC11));

    REQUIRE(M11_GameView_SetActingChampion(&state, 0));
    REQUIRE(M11_GameView_GetActingActionIndices(&state, actions));
    for (i = 0; i < 3; ++i) {
        if (actions[i] == DM1_ACTION_SWING) swingRow = i;
    }
    REQUIRE(swingRow >= 0);
    (void)M11_GameView_TriggerActionRow(&state, swingRow);
    event = find_pending_c11(&state.world);
    REQUIRE(event != NULL);
    REQUIRE(state.actionDisabledTicks[0] > 0);
    REQUIRE(event->aux1 == 0);
    actionDefenseAfterBegin = state.world.party.champions[0].actionDefense;
    c11Tick = event->fireAtTick;
    localLockExpiryTick = state.world.gameTick + state.actionDisabledTicks[0];
    REQUIRE(actionDefenseAfterBegin != 0);
    REQUIRE(c11Tick > localLockExpiryTick);

    while (state.world.gameTick < localLockExpiryTick) {
        REQUIRE(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    }
    REQUIRE(state.world.party.champions[0].actionDefense == actionDefenseAfterBegin);
    REQUIRE(state.world.party.champions[0].actionIndex == DM1_ACTION_SWING);
    REQUIRE(state.actionDisabledTicks[0] > 0u);
    REQUIRE(state.actionDisabledIndex[0] == DM1_ACTION_SWING);

    /* The real F0330 C11 owner reaches TIMELINE.C F0253 through the normal
     * M11 idle tick.  Its ordinal-zero SWING receipt must restore the action
     * state once and retire the host cooldown mirror, not let that mirror
     * replay F0253 after the emission. */
    /* F0884 dispatches due timeline entries at the next tick boundary, so
     * advance through the recorded C11 time before observing F0253 state. */
    while (state.world.gameTick <= c11Tick) {
        REQUIRE(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    }
    REQUIRE(state.world.party.champions[0].actionDefense == 0);
    REQUIRE(state.world.party.champions[0].actionIndex == 0xFFu);
    REQUIRE(state.actionDisabledTicks[0] == 0u);
    REQUIRE(state.actionDisabledIndex[0] == 0xFFu);
    REQUIRE(state.actionEnableSlotOrdinal[0] == 0xFFu);
    REQUIRE(state.dm1LiveActionEffects.count == 0);
    return 0;
}
