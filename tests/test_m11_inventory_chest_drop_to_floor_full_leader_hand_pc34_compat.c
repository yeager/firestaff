/*
 * DM1 V1 M11 chest full leader-hand drop-to-floor runtime gate.
 *
 * This intentionally sits beyond the C537..C544-only chest gates:
 * an occupied C539 click with a full leader hand is followed through
 * the C071 eye route, which must preserve the leader hand while the
 * open chest is closed from visible slots only.  The resulting hand item
 * is then dropped through the real viewport floor-drop helper so the
 * square-first-thing floor list records the handoff.
 *
 * Source anchors:
 *   ReDMCSB CHEST.C F0333:30-32 same-open guard.
 *   ReDMCSB CHEST.C F0334:117-132 visible C537..C544 close rewrite.
 *   ReDMCSB CHAMPION.C F0302:688-710 C30..C37 slot click order.
 *   ReDMCSB CLIKVIEW.C F0374:170-171 / DUNGEON.C F0163:1800-1837
 *   leader-hand object drop to the dungeon floor list.
 */
#include "m11_inventory_chest_drop_to_floor_full_leader_hand_pc34_compat.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass;
static int g_fail;

#define ACTION_C539_THEN_C071_THEN_FLOOR_DROP 1
#define ACTION_C071_EMPTY_HAND_NEGATIVE 2

static const char* A_F0333 =
    "ReDMCSB CHEST.C F0333:30-32 same G0426 open chest returns";
static const char* A_F0334 =
    "ReDMCSB CHEST.C F0334:117-132 relinks visible C537..C544 only";
static const char* A_F0302 =
    "ReDMCSB CHAMPION.C F0302:688-710 C30..C37 leader-hand swap order";
static const char* A_FLOOR =
    "ReDMCSB CLIKVIEW.C F0374:170-171 and DUNGEON.C F0163:1800-1837 floor-link drop";

static int expect_ulong(const char* label,
                        unsigned long got,
                        unsigned long want,
                        const char* anchor)
{
    if (!anchor || anchor[0] == '\0') {
        ++g_fail;
        fprintf(stderr, "FAIL: %s missing source anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_fail;
        fprintf(stderr, "FAIL: %s got=%lu want=%lu anchor=%s\n",
                label, got, want, anchor);
        return 0;
    }
    ++g_pass;
    return 1;
}

static unsigned short thing_ref(int type, int index)
{
    return (unsigned short)(((type & 0x0F) << 10) | (index & 0x03FF));
}

static void raw_set_next(unsigned char* raw, int index, unsigned short next)
{
    raw[index * 4] = (unsigned char)(next & 0xFFu);
    raw[index * 4 + 1] = (unsigned char)((next >> 8) & 0xFFu);
}

static unsigned short raw_get_next(const unsigned char* raw, int index)
{
    return (unsigned short)(raw[index * 4] |
                            ((unsigned short)raw[index * 4 + 1] << 8));
}

static void seed_runtime_view(M11_GameViewState* state,
                              struct DungeonDatState_Compat* dungeon,
                              struct DungeonMapDesc_Compat* maps,
                              struct DungeonMapTiles_Compat* tiles,
                              unsigned char* squareData,
                              unsigned short* squareFirstThings,
                              struct DungeonThings_Compat* things,
                              struct DungeonWeapon_Compat* weapons,
                              unsigned char* weaponRaw,
                              int weaponCount,
                              struct DungeonContainer_Compat* containers,
                              int containerCount)
{
    int i;

    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(*maps));
    memset(tiles, 0, sizeof(*tiles));
    memset(squareData, 0, 4);
    memset(squareFirstThings, 0, sizeof(unsigned short) * 4);
    memset(things, 0, sizeof(*things));
    memset(weapons, 0, sizeof(*weapons) * (size_t)weaponCount);
    memset(weaponRaw, 0, (size_t)weaponCount * 4);
    memset(containers, 0, sizeof(*containers) * (size_t)containerCount);

    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->inventoryPanelActive = 1;

    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    dungeon->header.mapCount = 1;
    dungeon->header.squareFirstThingCount = 4;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    maps[0].width = 2;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 4;
    for (i = 0; i < 4; ++i) {
        squareData[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
        squareFirstThings[i] = THING_ENDOFLIST;
    }

    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 4;
    things->weapons = weapons;
    things->weaponCount = weaponCount;
    things->containers = containers;
    things->containerCount = containerCount;
    things->rawThingData[THING_TYPE_WEAPON] = weaponRaw;
    things->thingCounts[THING_TYPE_WEAPON] = weaponCount;

    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 1;
    state->world.party.direction = DIR_NORTH;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        state->world.party.champions[0].inventory[i] = THING_NONE;
    }
}

static void seed_overfull_chest(struct DungeonWeapon_Compat* weapons,
                                unsigned char* weaponRaw,
                                int weaponCount,
                                struct DungeonContainer_Compat* containers,
                                unsigned short* weaponThings)
{
    int i;

    for (i = 0; i < weaponCount; ++i) {
        weaponThings[i] = thing_ref(THING_TYPE_WEAPON, i);
        weapons[i].type = 8; /* DUNGEON.C G0237 dagger-like, container-allowed. */
        weapons[i].next = THING_ENDOFLIST;
        raw_set_next(weaponRaw, i, THING_ENDOFLIST);
    }
    for (i = 0; i < 8; ++i) {
        weapons[i].next = weaponThings[i + 1];
        raw_set_next(weaponRaw, i, weaponThings[i + 1]);
    }
    containers[0].slot = weaponThings[0];
}

static int weapon_chain_count(const struct DungeonWeapon_Compat* weapons,
                              int weaponCount,
                              unsigned short firstThing,
                              int maxWalk)
{
    unsigned short thing = firstThing;
    int count = 0;

    while (thing != THING_NONE && thing != THING_ENDOFLIST && count < maxWalk) {
        int index;
        if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON) {
            break;
        }
        index = (int)THING_GET_INDEX(thing);
        if (index < 0 || index >= weaponCount) {
            break;
        }
        ++count;
        thing = weapons[index].next;
    }
    return count;
}

static int weapon_chain_contains(const struct DungeonWeapon_Compat* weapons,
                                 int weaponCount,
                                 unsigned short firstThing,
                                 unsigned short targetThing,
                                 int maxWalk)
{
    unsigned short thing = firstThing;
    int count = 0;

    while (thing != THING_NONE && thing != THING_ENDOFLIST && count < maxWalk) {
        int index;
        if (thing == targetThing) {
            return 1;
        }
        if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON) {
            break;
        }
        index = (int)THING_GET_INDEX(thing);
        if (index < 0 || index >= weaponCount) {
            break;
        }
        ++count;
        thing = weapons[index].next;
    }
    return 0;
}

static int floor_chain_count(const unsigned char* weaponRaw,
                             const unsigned short* squareFirstThings,
                             int squareCount,
                             int maxWalk)
{
    int count = 0;
    int square;

    for (square = 0; square < squareCount; ++square) {
        unsigned short thing = squareFirstThings[square];
        int walked = 0;

        while (thing != THING_NONE && thing != THING_ENDOFLIST &&
               walked < maxWalk) {
            int index;
            if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON) {
                break;
            }
            ++count;
            index = (int)THING_GET_INDEX(thing);
            thing = raw_get_next(weaponRaw, index);
            ++walked;
        }
    }
    return count;
}

static int click_chest_slot(M11_GameViewState* state, int chestOrdinal)
{
    int x, y, w, h;

    if (!M11_GameView_GetV1ChestSlotBoxZone(chestOrdinal, &x, &y, &w, &h)) {
        return 0;
    }
    return M11_GameView_HandlePointer(state,
                                      x + w / 2,
                                      33 + y + h / 2,
                                      1) == M11_GAME_INPUT_REDRAW;
}

static int click_eye(M11_GameViewState* state)
{
    return M11_GameView_HandlePointer(state, 12 + 8, 33 + 13 + 8, 1) ==
           M11_GAME_INPUT_REDRAW;
}

static int click_front_floor_drop(M11_GameViewState* state)
{
    return M11_GameView_HandlePointer(state,
                                      120,
                                      33 + 100,
                                      1) == M11_GAME_INPUT_REDRAW;
}

int m11_inventory_chest_drop_to_floor_full_leader_hand_pc34_compat_run(
    M11_InventoryChestDropToFloorFullLeaderHandProbePc34* out)
{
    enum {
        WEAPON_COUNT = 10,
        CHEST_INDEX = 0,
        C539_ORDINAL = 2
    };
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[4];
    unsigned short squareFirstThings[4];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[WEAPON_COUNT];
    unsigned char weaponRaw[WEAPON_COUNT * 4];
    struct DungeonContainer_Compat containers[1];
    unsigned short weaponThings[WEAPON_COUNT];
    unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, CHEST_INDEX);
    unsigned short leaderThing;
    unsigned short c539Thing;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->anchor = A_FLOOR;
    out->actionType = ACTION_C539_THEN_C071_THEN_FLOOR_DROP;
    out->expectedFloorState = 1;
    out->expectedChestVisibleSlotCount = 8;

    seed_runtime_view(&state, &dungeon, maps, tiles, squareData,
                      squareFirstThings, &things, weapons, weaponRaw,
                      WEAPON_COUNT, containers, 1);
    seed_overfull_chest(weapons, weaponRaw, WEAPON_COUNT, containers,
                        weaponThings);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chestThing;
    leaderThing = weaponThings[9];
    c539Thing = weaponThings[C539_ORDINAL];

    if (!M11_GameView_OpenV1ActionHandChest(&state)) {
        return 0;
    }
    out->leaderHandBefore = leaderThing;
    for (i = 0; i < 8; ++i) {
        out->chestContentsBefore[i] = weaponThings[i];
    }
    if (!M11_GameView_SetV1LeaderHandObject(&state, leaderThing)) {
        return 0;
    }

    if (!click_chest_slot(&state, C539_ORDINAL)) {
        return 0;
    }
    out->leaderHandBeforeEye = M11_GameView_GetV1LeaderHandThing(&state);

    /* ReDMCSB CHAMPION.C F0302:688-710 swaps the full hand with C539.
     * C071 then exercises PANEL.C/F0333/F0334 panel replacement while
     * preserving the post-swap leader hand. */
    if (!click_eye(&state)) {
        return 0;
    }
    out->leaderHandAfterEye = M11_GameView_GetV1LeaderHandThing(&state);
    out->expectedLeaderHandAfter = c539Thing;
    out->chestVisibleSlotCountAfterClose =
        weapon_chain_count(weapons, WEAPON_COUNT, containers[0].slot, 12);
    out->hiddenNinthTailPresentAfterClose =
        weapon_chain_contains(weapons, WEAPON_COUNT, containers[0].slot,
                              weaponThings[8], 12);

    (void)M11_GameView_DismissDialogOverlay(&state);
    state.inventoryPanelActive = 0;
    if (!click_front_floor_drop(&state)) {
        return 0;
    }
    out->floorSlotCountAfterDrop =
        floor_chain_count(weaponRaw, squareFirstThings, 4, 8);

    return 1;
}

static void test_full_leader_hand_c539_eye_then_floor_drop(void)
{
    M11_InventoryChestDropToFloorFullLeaderHandProbePc34 probe;

    memset(&probe, 0, sizeof(probe));
    expect_ulong("probe run",
                 (unsigned long)m11_inventory_chest_drop_to_floor_full_leader_hand_pc34_compat_run(&probe),
                 1, A_F0333);
    expect_ulong("action type", (unsigned long)probe.actionType,
                 ACTION_C539_THEN_C071_THEN_FLOOR_DROP, A_F0302);
    expect_ulong("leader hand before non-empty",
                 (unsigned long)(probe.leaderHandBefore != THING_NONE),
                 1, A_F0302);
    expect_ulong("C071 preserves post-C539 leader hand",
                 probe.leaderHandAfterEye, probe.expectedLeaderHandAfter,
                 A_F0333);
    expect_ulong("leader hand unchanged across C071",
                 probe.leaderHandAfterEye, probe.leaderHandBeforeEye,
                 A_F0333);
    expect_ulong("closed chest visible count",
                 (unsigned long)probe.chestVisibleSlotCountAfterClose,
                 (unsigned long)probe.expectedChestVisibleSlotCount, A_F0334);
    expect_ulong("hidden ninth absent after close",
                 (unsigned long)probe.hiddenNinthTailPresentAfterClose, 0,
                 A_F0334);
    expect_ulong("floor slot count after helper drop",
                 (unsigned long)probe.floorSlotCountAfterDrop,
                 (unsigned long)probe.expectedFloorState, A_FLOOR);
}

static void test_empty_hand_eye_does_not_populate_floor(void)
{
    enum {
        WEAPON_COUNT = 10,
        CHEST_INDEX = 0
    };
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[4];
    unsigned short squareFirstThings[4];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[WEAPON_COUNT];
    unsigned char weaponRaw[WEAPON_COUNT * 4];
    struct DungeonContainer_Compat containers[1];
    unsigned short weaponThings[WEAPON_COUNT];
    unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, CHEST_INDEX);

    seed_runtime_view(&state, &dungeon, maps, tiles, squareData,
                      squareFirstThings, &things, weapons, weaponRaw,
                      WEAPON_COUNT, containers, 1);
    seed_overfull_chest(weapons, weaponRaw, WEAPON_COUNT, containers,
                        weaponThings);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chestThing;

    expect_ulong("negative open chest",
                 (unsigned long)M11_GameView_OpenV1ActionHandChest(&state),
                 1, A_F0333);
    expect_ulong("negative leader hand starts empty",
                 (unsigned long)(M11_GameView_GetV1LeaderHandThing(&state) ==
                                 THING_NONE),
                 1, A_F0302);
    expect_ulong("negative empty-hand C071 closes chest",
                 (unsigned long)click_eye(&state), 1, A_F0334);
    expect_ulong("negative floor remains empty",
                 (unsigned long)floor_chain_count(weaponRaw, squareFirstThings,
                                                  4, 8),
                 0, A_FLOOR);
}

int main(void)
{
    printf("=== M11 DM1 V1 Chest Full Leader-Hand Floor-Drop Gate ===\n");
    printf("ReDMCSB: CHEST.C F0333:30-32; F0334:117-132; ");
    printf("CHAMPION.C F0302:688-710; CLIKVIEW.C F0374:170-171\n");

    test_full_leader_hand_c539_eye_then_floor_drop();
    test_empty_hand_eye_does_not_populate_floor();

    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
