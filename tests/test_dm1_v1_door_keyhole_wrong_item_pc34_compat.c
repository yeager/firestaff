#include <stdio.h>
#include <string.h>

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_movement_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static unsigned char square_type(int elementType, int attrs)
{
    return (unsigned char)((elementType << 5) | (attrs & DUNGEON_SQUARE_MASK_ATTRIBS));
}

static void seed_door_view(M11_GameViewState* state,
                           struct DungeonDatState_Compat* dungeon,
                           struct DungeonMapDesc_Compat* map,
                           struct DungeonMapTiles_Compat* tiles,
                           unsigned char* squares,
                           struct DungeonThings_Compat* things,
                           struct DungeonDoor_Compat* doors,
                           unsigned short* squareFirstThings,
                           struct DungeonWeapon_Compat* weapons)
{
    int x, y;

    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(squares, 0, 9u * sizeof(*squares));
    memset(things, 0, sizeof(*things));
    memset(doors, 0, sizeof(*doors));
    for (x = 0; x < 9; ++x) {
        squareFirstThings[x] = THING_ENDOFLIST;
    }
    memset(weapons, 0, sizeof(*weapons));

    M11_GameView_Init(state);
    state->active = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 2;
    state->world.party.direction = DIR_NORTH;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].inventory[0] = THING_NONE;
    state->world.party.champions[0].inventory[1] = THING_NONE;
    state->world.party.champions[1].present = 1;
    state->world.party.champions[1].inventory[0] = THING_NONE;
    state->world.party.champions[1].inventory[1] = THING_NONE;

    map->width = 3;
    map->height = 3;
    tiles->squareData = squares;
    tiles->squareCount = 9;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    for (x = 0; x < 3; ++x) {
        for (y = 0; y < 3; ++y) {
            squares[x * 3 + y] = square_type(DUNGEON_ELEMENT_CORRIDOR, 0);
        }
    }

    /* Front cell (1,1) is the D1C door face.  The wrong-item click
     * must stay on the door/keyhole box instead of spilling into the
     * generic throw path. */
    squares[1 * 3 + 1] = square_type(DUNGEON_ELEMENT_DOOR,
                                     0x10 | 4);
    squareFirstThings[1 * 3 + 1] = 0; /* type 0 / door, index 0 */
    doors[0].next = THING_ENDOFLIST;
    doors[0].button = 1;
    doors[0].ornamentOrdinal = 1;
    squares[2 * 3 + 2] = square_type(DUNGEON_ELEMENT_DOOR,
                                     0x10 | 4);
    squareFirstThings[2 * 3 + 2] = 0; /* Same source-style door thing for leader/facing churn. */
    things->loaded = 1;
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 9;
    things->doors = doors;
    things->doorCount = 1;
    things->weapons = weapons;
    things->weaponCount = 1;
    weapons[0].type = 2; /* Dagger-like object icon, but not a key. */
    weapons[0].next = THING_ENDOFLIST;
}

int main(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[9];
    struct DungeonThings_Compat things;
    struct DungeonDoor_Compat doors[1];
    unsigned short squareFirstThings[9];
    struct DungeonWeapon_Compat weapons[1];
    unsigned short wrongThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    const int clickX = 168;
    const int clickY = 81;
    const int localClickX = clickX;
    const int localClickY = clickY - 33;
    int baselineMessageCount;
    int baselineProjectileCount;
    int baselineTimelineCount;
    int baselineSoundIndex;
    int baselineDoorButton;
    int baselineDoorOrnamentOrdinal;
    unsigned short baselineDoorNext;
    unsigned short baselineFrontThing;
    int secondBaselineMessageCount;
    int secondBaselineProjectileCount;
    int secondBaselineTimelineCount;
    int secondBaselineSoundIndex;
    unsigned int secondBaselineWorldHash;
    int secondBaselineDoorButton;
    int secondBaselineDoorOrnamentOrdinal;
    unsigned short secondBaselineDoorNext;
    unsigned short secondBaselineFrontThing;
    char secondBaselineAction[sizeof(state.lastAction)];
    char secondBaselineOutcome[sizeof(state.lastOutcome)];
    char secondBaselineInspectTitle[sizeof(state.inspectTitle)];
    char secondBaselineInspectDetail[sizeof(state.inspectDetail)];
    int coordinateSpace = 0;
    int zoneId = 0;
    int command = 0;
    int ok = 1;

    printf("probe=dm1_v1_door_keyhole_wrong_item_pc34_compat\n");
    printf("sourceEvidence=COMMAND.C:2322-2324 C080 dispatch; "
           "CLIKVIEW.C:F0377:365-400 empty-hand door event vs occupied-hand throw branch; "
           "DUNVIEW.C:4210-4212 D1C door-button/keyhole click box\n");
    seed_door_view(&state, &dungeon, &map, &tiles, squares, &things, doors,
                   squareFirstThings, weapons);

    ok &= expect_int("leader hand setup accepted",
                     M11_GameView_SetV1LeaderHandObject(&state, wrongThing), 1);
    ok &= expect_int("leader hand holds wrong item before click",
                     M11_GameView_GetV1LeaderHandThing(&state), wrongThing);

    command = M11_GameView_GetV1MouseCommandForPoint(
        M11_DM1_MOUSE_LIST_MOVEMENT,
        clickX,
        clickY,
        M11_DM1_MOUSE_MASK_LEFT,
        &coordinateSpace,
        &zoneId);
    ok &= expect_int("click resolves through C080 broad viewport command",
                     command, 80);
    ok &= expect_int("click resolves through C007 viewport zone",
                     zoneId, 7);
    ok &= expect_int("click uses screen-space movement route",
                     coordinateSpace, M11_DM1_MOUSE_SPACE_SCREEN);
    ok &= expect_int("click is inside D1C keyhole/button source x",
                     localClickX >= 160 && localClickX <= 175, 1);
    ok &= expect_int("click is inside D1C keyhole/button source y",
                     localClickY >= 44 && localClickY <= 52, 1);

    state.lastWorldHash = 0xBADF00Du;
    snprintf(state.lastAction, sizeof(state.lastAction), "SENTINEL");
    snprintf(state.lastOutcome, sizeof(state.lastOutcome), "UNCHANGED");
    snprintf(state.inspectTitle, sizeof(state.inspectTitle), "SENTINEL INSPECT");
    snprintf(state.inspectDetail, sizeof(state.inspectDetail), "NO NEW DETAIL");
    M11_MessageLog_Push(&state.messageLog, "SENTINEL MESSAGE", 0);
    baselineMessageCount = M11_GameView_GetMessageLogCount(&state);
    baselineProjectileCount = state.world.projectiles.count;
    baselineTimelineCount = state.world.timeline.count;
    baselineDoorButton = doors[0].button;
    baselineDoorOrnamentOrdinal = doors[0].ornamentOrdinal;
    baselineDoorNext = doors[0].next;
    baselineFrontThing = squareFirstThings[1 * 3 + 1];
    state.audioState.lastSoundIndex = 77;
    baselineSoundIndex = state.audioState.lastSoundIndex;

    /* ReDMCSB CLIKVIEW.C F0377 lines 356-401: a door-button/keyhole
     * click schedules EVENT_DOOR only from the empty leader-hand branch
     * (lines 365-389); the occupied leader-hand branch goes to F0375
     * throw handling instead (lines 396-400). Firestaff's D1C keyhole
     * guard claims this source box so the click produces no open, no
     * consume, no throw, and no invented wrong-item message/status. */
    ok &= expect_int("wrong-item door-keyhole click is ignored",
                     M11_GameView_HandlePointerButton(&state, clickX, clickY,
                                                     M11_DM1_MOUSE_MASK_LEFT),
                     M11_GAME_INPUT_IGNORED);
    ok &= expect_int("leader hand survives wrong-item click",
                     M11_GameView_GetV1LeaderHandThing(&state), wrongThing);
    ok &= expect_int("door remains closed after wrong-item click",
                     squares[1 * 3 + 1],
                     square_type(DUNGEON_ELEMENT_DOOR, 0x10 | 4));
    ok &= expect_int("wrong-item click preserves door button flag",
                     doors[0].button, baselineDoorButton);
    ok &= expect_int("wrong-item click preserves door ornament ordinal",
                     doors[0].ornamentOrdinal, baselineDoorOrnamentOrdinal);
    ok &= expect_int("wrong-item click preserves door next link",
                     doors[0].next, baselineDoorNext);
    ok &= expect_int("wrong-item click preserves front door thing",
                     squareFirstThings[1 * 3 + 1], baselineFrontThing);
    ok &= expect_int("wrong-item click does not refresh world hash",
                     (int)state.lastWorldHash, (int)0xBADF00Du);
    ok &= expect_int("wrong-item click does not append a message",
                     M11_GameView_GetMessageLogCount(&state),
                     baselineMessageCount);
    ok &= expect_int("wrong-item click does not spawn projectile",
                     state.world.projectiles.count,
                     baselineProjectileCount);
    ok &= expect_int("wrong-item click does not schedule door event",
                     state.world.timeline.count,
                     baselineTimelineCount);
    ok &= expect_int("wrong-item click does not play switch/door sound",
                     state.audioState.lastSoundIndex,
                     baselineSoundIndex);
    ok &= expect_int("wrong-item click does not report wrong-item status",
                     strcmp(state.lastAction, "SENTINEL") == 0 &&
                     strcmp(state.lastOutcome, "UNCHANGED") == 0, 1);
    ok &= expect_int("wrong-item click does not publish inspect title",
                     strcmp(state.inspectTitle, "SENTINEL INSPECT") == 0, 1);
    ok &= expect_int("wrong-item click does not publish inspect detail",
                     strcmp(state.inspectDetail, "NO NEW DETAIL") == 0, 1);
    ok &= expect_int("wrong-item click preserves prior message text",
                     strcmp(M11_GameView_GetMessageLogEntry(&state, 0),
                            "SENTINEL MESSAGE") == 0, 1);

    state.world.party.championCount = 2;
    state.world.party.activeChampionIndex = 1;
    state.world.party.direction = DIR_EAST;
    secondBaselineWorldHash = state.lastWorldHash;
    snprintf(secondBaselineAction, sizeof(secondBaselineAction), "%s",
             state.lastAction);
    snprintf(secondBaselineOutcome, sizeof(secondBaselineOutcome), "%s",
             state.lastOutcome);
    snprintf(secondBaselineInspectTitle, sizeof(secondBaselineInspectTitle),
             "%s", state.inspectTitle);
    snprintf(secondBaselineInspectDetail, sizeof(secondBaselineInspectDetail),
             "%s", state.inspectDetail);
    secondBaselineMessageCount = M11_GameView_GetMessageLogCount(&state);
    secondBaselineProjectileCount = state.world.projectiles.count;
    secondBaselineTimelineCount = state.world.timeline.count;
    secondBaselineDoorButton = doors[0].button;
    secondBaselineDoorOrnamentOrdinal = doors[0].ornamentOrdinal;
    secondBaselineDoorNext = doors[0].next;
    secondBaselineFrontThing = squareFirstThings[2 * 3 + 2];
    secondBaselineSoundIndex = state.audioState.lastSoundIndex;

    /* Same ReDMCSB F0377 branch after changing the active champion and
     * facing direction: the wrong item still belongs to G4055 leader hand,
     * not to the newly selected champion, and the D1C keyhole/button box
     * must remain a no-op instead of throwing the object. */
    ok &= expect_int("wrong-item click after leader/facing change is ignored",
                     M11_GameView_HandlePointerButton(&state, clickX, clickY,
                                                     M11_DM1_MOUSE_MASK_LEFT),
                     M11_GAME_INPUT_IGNORED);
    ok &= expect_int("active champion survives wrong-item click",
                     state.world.party.activeChampionIndex, 1);
    ok &= expect_int("facing direction survives wrong-item click",
                     state.world.party.direction, DIR_EAST);
    ok &= expect_int("leader hand survives after leader/facing change",
                     M11_GameView_GetV1LeaderHandThing(&state), wrongThing);
    ok &= expect_int("east-facing door remains closed after wrong-item click",
                     squares[2 * 3 + 2],
                     square_type(DUNGEON_ELEMENT_DOOR, 0x10 | 4));
    ok &= expect_int("leader/facing wrong-item click preserves door button flag",
                     doors[0].button, secondBaselineDoorButton);
    ok &= expect_int("leader/facing wrong-item click preserves door ornament ordinal",
                     doors[0].ornamentOrdinal, secondBaselineDoorOrnamentOrdinal);
    ok &= expect_int("leader/facing wrong-item click preserves door next link",
                     doors[0].next, secondBaselineDoorNext);
    ok &= expect_int("leader/facing wrong-item click preserves front door thing",
                     squareFirstThings[2 * 3 + 2], secondBaselineFrontThing);
    ok &= expect_int("leader/facing wrong-item click does not refresh world hash",
                     (int)state.lastWorldHash, (int)secondBaselineWorldHash);
    ok &= expect_int("leader/facing wrong-item click does not append a message",
                     M11_GameView_GetMessageLogCount(&state),
                     secondBaselineMessageCount);
    ok &= expect_int("leader/facing wrong-item click does not spawn projectile",
                     state.world.projectiles.count,
                     secondBaselineProjectileCount);
    ok &= expect_int("leader/facing wrong-item click does not schedule door event",
                     state.world.timeline.count,
                     secondBaselineTimelineCount);
    ok &= expect_int("leader/facing wrong-item click does not play switch/door sound",
                     state.audioState.lastSoundIndex,
                     secondBaselineSoundIndex);
    ok &= expect_int("leader/facing wrong-item click keeps status",
                     strcmp(state.lastAction, secondBaselineAction) == 0 &&
                     strcmp(state.lastOutcome, secondBaselineOutcome) == 0, 1);
    ok &= expect_int("leader/facing wrong-item click keeps inspect text",
                     strcmp(state.inspectTitle, secondBaselineInspectTitle) == 0 &&
                     strcmp(state.inspectDetail, secondBaselineInspectDetail) == 0, 1);

    if (!ok) return 1;
    printf("ok: DM1 V1 door keyhole wrong-item click is ignored without consume, open, or message\n");
    return 0;
}
