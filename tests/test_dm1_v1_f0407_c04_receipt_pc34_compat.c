/* ReDMCSB MENU.C F0407:1308-1319 -> SOUND.C F0064 delayed C20 receipt. */
#include "m11_game_view.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "dm1_v1_sound_pc34_compat.h"

#include <assert.h>
#include <string.h>

static unsigned short make_thing(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static unsigned char square_for_test(int elementType, int attributes)
{
    return (unsigned char)(((elementType & 0x07) << 5) | (attributes & 0x1f));
}

int main(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonDoor_Compat doors[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    unsigned char actions[3];
    int swingRow = -1;
    (void)swingRow;
    int i;
    int sawNativeC20 = 0;
    (void)sawNativeC20;
    int sawHistoricC11 = 0;
    (void)sawHistoricC11;
    int sawSoundAfterStaleC11 = 0;
    (void)sawSoundAfterStaleC11;
    int sawActionEnabledAfterPartyCellMove = 0;
    (void)sawActionEnabledAfterPartyCellMove;
    int sawSoundAfterPartyCellMove = 0;
    (void)sawSoundAfterPartyCellMove;
    uint32_t historicC11FireAtTick = 0u;
    struct TickResult_Compat dispatchResult;
    struct ChampionState_Compat reRecruitedChampion;
    struct TimelineEvent_Compat forgedOwnerC04;
    struct TimelineEvent_Compat movedOwnerC04;

    memset(&state, 0, sizeof(state));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(doors, 0, sizeof(doors));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.gameTick = 7u;
    state.world.party.championCount = 1;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1; /* Party faces east. */
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].stamina.current = 100;
    state.world.party.champions[0].stamina.maximum = 100;
    state.world.party.champions[0].food = 2048;
    state.world.party.champions[0].water = 2048;
    state.world.party.champions[0].actionIndex = 0xffu;
    state.world.party.champions[0].name[0] = 'H';
    state.world.party.champions[0].name[1] = 'A';
    state.world.party.champions[0].name[2] = 'L';
    state.world.party.champions[0].name[3] = 'K';
    state.world.party.champions[0].direction = 2; /* Champion faces south. */
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
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].doorSet0 = 1;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(1 * 3) + 2] = square_for_test(
        DUNGEON_ELEMENT_DOOR, DUNGEON_SQUARE_MASK_THING_LIST | 4);
    squareFirstThings[0] = make_thing(THING_TYPE_DOOR, 0);
    weapons[0].type = 2; /* ActionSet 5 includes SWING. */
    doors[0].next = THING_ENDOFLIST;
    doors[0].type = 0;
    doors[0].meleeDestructible = 1;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.doors = doors;
    things.doorCount = 1;
    state.world.dungeon = &dungeon;
    state.world.things = &things;

    assert(M11_GameView_SetActingChampion(&state, 0) == 1);
    assert(M11_GameView_GetActingActionIndices(&state, actions) == 1);
    for (i = 0; i < 3; ++i) {
        if (actions[i] == DM1_ACTION_SWING) {
            swingRow = i;
            break;
        }
    }
    assert(swingRow >= 0);
    assert(M11_GameView_TriggerActionRow(&state, swingRow) == 1);

    for (i = 0; i < state.world.timeline.count; ++i) {
        const struct TimelineEvent_Compat *event =
            &state.world.timeline.events[i];
        if (event->kind == TIMELINE_EVENT_PLAY_SOUND &&
            event->mapIndex == 0 && event->mapX == 1 && event->mapY == 2 &&
            event->fireAtTick == 8u) {
            assert(event->aux0 == DM1_SND_WOODEN_THUD);
            assert(event->aux1 == 0);
            assert(event->aux2 == DM1_EVENT_PLAY_SOUND);
            assert(event->aux3 == 0);
            assert(event->aux4 == 70);
            assert(event->cell == 0);
            sawNativeC20 = 1;
        }
        if (event->kind == TIMELINE_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux0 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux2 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux4 == 0) {
            sawHistoricC11 = 1;
            historicC11FireAtTick = event->fireAtTick;
        }
    }
    assert(sawNativeC20 == 1);
    assert(sawHistoricC11 == 1);
    assert(historicC11FireAtTick > state.world.gameTick);
    assert(state.actionDisabledTicks[0] == 6u);
    /* MENU.C F0407:1312-1317 and :1620-1622 create this C04/C11 pair from
     * one valid action.  SOUND.C F0064 stores C04 as location/sound only,
     * while CHAMPION.C F0330 stores the champion in C11 Priority.  Losing
     * the original owner after C11's due tick consumes only C11 in
     * TIMELINE.C; the action's already-due C04 must still play.  Use the
     * real F0643 party-slot rollback rather than mutating count directly. */
    /* A second C04 due beside C11 cannot carry a champion owner: C20's
     * aux4 is SOUND_DATA.Priority.  This forged receipt substitutes owner
     * zero for F0407's source priority 70, and must be consumed silently. */
    memset(&forgedOwnerC04, 0, sizeof(forgedOwnerC04));
    forgedOwnerC04.kind = TIMELINE_EVENT_PLAY_SOUND;
    forgedOwnerC04.fireAtTick = historicC11FireAtTick;
    forgedOwnerC04.mapIndex = state.world.party.mapIndex;
    forgedOwnerC04.mapX = 1;
    forgedOwnerC04.mapY = 2;
    forgedOwnerC04.aux0 = DM1_SND_WOODEN_THUD;
    forgedOwnerC04.aux2 = DM1_EVENT_PLAY_SOUND;
    forgedOwnerC04.aux4 = 0; /* Claimed C11 owner, not C20 sound priority. */
    assert(F0721_TIMELINE_Schedule_Compat(
               &state.world.timeline, &forgedOwnerC04) == 1);
    /* C11 Priority is a champion ordinal, while C49 owns projectile slots.
     * Reset and reuse owner zero in two live projectile generations with
     * distinct launch/impact ticks: neither F0407 receipt may resolve
     * through either new projectile instance. */
    memset(&state.world.projectiles, 0, sizeof(state.world.projectiles));
    state.world.projectiles.count = 2;
    state.world.projectiles.entries[0].slotIndex = 0;
    state.world.projectiles.entries[0].reserved3 = 1;
    state.world.projectiles.entries[0].ownerKind = PROJECTILE_OWNER_CHAMPION;
    state.world.projectiles.entries[0].ownerIndex = 0;
    state.world.projectiles.entries[0].mapIndex = 0;
    state.world.projectiles.entries[0].mapX = 1;
    state.world.projectiles.entries[0].mapY = 2;
    state.world.projectiles.entries[0].launchedAtTick =
        (int)historicC11FireAtTick - 3;
    state.world.projectiles.entries[0].scheduledAtTick =
        (int)historicC11FireAtTick;
    state.world.projectiles.entries[1].slotIndex = 1;
    state.world.projectiles.entries[1].reserved3 = 1;
    state.world.projectiles.entries[1].ownerKind = PROJECTILE_OWNER_CHAMPION;
    state.world.projectiles.entries[1].ownerIndex = 0;
    state.world.projectiles.entries[1].mapIndex = 0;
    state.world.projectiles.entries[1].mapX = 2;
    state.world.projectiles.entries[1].mapY = 2;
    state.world.projectiles.entries[1].launchedAtTick =
        (int)historicC11FireAtTick - 1;
    state.world.projectiles.entries[1].scheduledAtTick =
        (int)historicC11FireAtTick + 2;
    reRecruitedChampion = state.world.party.champions[0];
    assert(F0643_PARTY_ClearChampionSlot_Compat(&state.world.party, 0) == 1);
    assert(state.world.party.championCount == 0);
    assert(state.world.party.champions[0].present == 0);
    assert(state.world.party.activeChampionIndex == -1);
    /* REVIVE.C F0282 C162 removes the candidate before a later mirror can
     * reuse the ordinal. TIMELINE.C C11 dispatches only its stored Priority,
     * so preserve no C11 across that reuse boundary. C04 remains C20 sound
     * data and must still dispatch at its source location. */
    DM1_V1_F0407_ClearRemovedChampionActionReceiptsPc34Compat(
        &state.world, 0);
    state.world.party.champions[0] = reRecruitedChampion;
    state.world.party.champions[0].present = 1;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    memset(&dispatchResult, 0, sizeof(dispatchResult));
    state.world.gameTick = historicC11FireAtTick + 1u;
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(
               &state.world, &dispatchResult) == 3);
    assert(dispatchResult.emissionCount == 2);
    for (i = 0; i < dispatchResult.emissionCount; ++i) {
        const struct TickEmission_Compat *emission =
            &dispatchResult.emissions[i];
        if (emission->kind == EMIT_SOUND_REQUEST &&
            emission->payload[0] == DM1_SND_WOODEN_THUD &&
            emission->payload[1] == 1 && emission->payload[2] == 2 &&
            emission->payload[3] == 0) {
            sawSoundAfterStaleC11 = 1;
        }
        assert(emission->kind != EMIT_ACTION_ENABLED);
    }
    assert(sawSoundAfterStaleC11 == 1);
    assert(state.world.timeline.count == 0);
    assert(state.world.projectiles.count == 2);
    assert(state.world.projectiles.entries[0].slotIndex == 0);
    assert(state.world.projectiles.entries[0].ownerKind ==
           PROJECTILE_OWNER_CHAMPION);
    assert(state.world.projectiles.entries[0].ownerIndex == 0);
    assert(state.world.projectiles.entries[0].launchedAtTick ==
           (int)historicC11FireAtTick - 3);
    assert(state.world.projectiles.entries[0].scheduledAtTick ==
           (int)historicC11FireAtTick);
    assert(state.world.projectiles.entries[1].slotIndex == 1);
    assert(state.world.projectiles.entries[1].ownerKind ==
           PROJECTILE_OWNER_CHAMPION);
    assert(state.world.projectiles.entries[1].ownerIndex == 0);
    assert(state.world.projectiles.entries[1].launchedAtTick ==
           (int)historicC11FireAtTick - 1);
    assert(state.world.projectiles.entries[1].scheduledAtTick ==
           (int)historicC11FireAtTick + 2);
    assert((squareData[(1 * 3) + 2] & 0x07) == 5);
    assert(F0643_PARTY_ClearChampionSlot_Compat(&state.world.party, 0) == 1);
    assert(DM1_V1_F0330_ScheduleEnableChampionActionPc34Compat(
               &state.world, 0, 6) == 0);
    assert(DM1_V1_F0407_MarkPendingThrowActionHandPc34Compat(
               &state.world, 0) == 0);

    /* MENU.C F0407:1256-1259 and CHAMPION.C F0330:2233-2251 retain the
     * Champion[] index, not the party cell.  A projectile owned by champion
     * zero may remain live while zero and one exchange physical party slots;
     * the later C11 must still enable champion zero. */
    state.world.party.championCount = 2;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].cell = 0;
    state.world.party.champions[1] = state.world.party.champions[0];
    state.world.party.champions[1].cell = 1;
    memset(&state.world.projectiles, 0, sizeof(state.world.projectiles));
    state.world.projectiles.count = 1;
    state.world.projectiles.entries[0].slotIndex = 0;
    state.world.projectiles.entries[0].reserved3 = 1;
    state.world.projectiles.entries[0].ownerKind = PROJECTILE_OWNER_CHAMPION;
    state.world.projectiles.entries[0].ownerIndex = 0;
    state.world.projectiles.entries[0].launchedAtTick =
        (int)state.world.gameTick;
    state.world.projectiles.entries[0].scheduledAtTick =
        (int)state.world.gameTick + 1;
    assert(DM1_V1_F0330_ScheduleEnableChampionActionPc34Compat(
               &state.world, 0, 1) == 1);
    memset(&movedOwnerC04, 0, sizeof(movedOwnerC04));
    movedOwnerC04.kind = TIMELINE_EVENT_PLAY_SOUND;
    movedOwnerC04.fireAtTick = state.world.gameTick + 1u;
    movedOwnerC04.mapIndex = state.world.party.mapIndex;
    movedOwnerC04.mapX = 1;
    movedOwnerC04.mapY = 2;
    movedOwnerC04.aux0 = DM1_SND_WOODEN_THUD;
    movedOwnerC04.aux2 = DM1_EVENT_PLAY_SOUND;
    movedOwnerC04.aux4 = 70;
    assert(F0721_TIMELINE_Schedule_Compat(
               &state.world.timeline, &movedOwnerC04) == 1);
    state.world.party.champions[0].cell = 1;
    state.world.party.champions[1].cell = 0;
    state.world.gameTick++;
    memset(&dispatchResult, 0, sizeof(dispatchResult));
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(
               &state.world, &dispatchResult) == 2);
    assert(dispatchResult.emissionCount == 2);
    for (i = 0; i < dispatchResult.emissionCount; ++i) {
        const struct TickEmission_Compat *emission =
            &dispatchResult.emissions[i];
        if (emission->kind == EMIT_ACTION_ENABLED &&
            emission->payload[0] == 0) {
            sawActionEnabledAfterPartyCellMove = 1;
        }
        if (emission->kind == EMIT_SOUND_REQUEST &&
            emission->payload[0] == DM1_SND_WOODEN_THUD &&
            emission->payload[1] == 1 && emission->payload[2] == 2 &&
            emission->payload[3] == 0) {
            sawSoundAfterPartyCellMove = 1;
        }
    }
    assert(sawActionEnabledAfterPartyCellMove == 1);
    assert(sawSoundAfterPartyCellMove == 1);
    assert(state.world.party.champions[0].cell == 1);
    assert(state.world.party.champions[1].cell == 0);
    assert(state.world.projectiles.entries[0].ownerIndex == 0);
    assert(state.world.timeline.count == 0);
    return 0;
}
