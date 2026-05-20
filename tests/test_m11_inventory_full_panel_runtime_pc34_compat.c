/*
 * Source-lock gate for the M11 normal V1 inventory full backpack panel.
 *
 * ReDMCSB evidence:
 *   DEFS.H lines 778-817: C00..C37 source slot namespace
 *   DATA.C lines 1049-1087: 30 champion inventory masks + 8 chest masks
 *   CHAMPION.C F0302 lines 677-699: inventory slot-box click resolves
 *     source slot index, reads champion slot/chest slot, no-ops empty/empty,
 *     and gates leader-hand placement through AllowedSlots & SlotMasks
 *   CHAMPION.C F0302 lines 700-712: leader hand/slot swap and redraw
 *   PANEL.C F0347 lines 1651-1691: inventory panel redraw is driven by
 *     the active inventory champion action hand and panel content route
 *   PANEL.C F0352 lines 1250-1254: weapon descriptions expose Cursed,
 *     Poisoned, Broken and ChargeCount-derived state from WEAPON data
 *   PANEL.C F0351 lines 2026-2108: eye click with empty leader hand draws
 *     champion base skills plus all six current/max statistic families
 */

#include "m11_game_view.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_TRUE(expr, msg) do { \
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

static void seed_inventory_view(M11_GameViewState* state,
                                struct DungeonThings_Compat* things,
                                struct DungeonWeapon_Compat* weapon) {
    int i;
    memset(things, 0, sizeof(*things));
    memset(weapon, 0, sizeof(*weapon));
    weapon->type = 8; /* dagger: source AllowedSlots includes backpack/container */
    things->weapons = weapon;
    things->weaponCount = 1;

    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->inventoryPanelActive = 1;
    state->world.things = things;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        state->world.party.champions[0].inventory[i] = THING_NONE;
    }
}

static void test_extended_backpack_source_mapping(void) {
    ASSERT_EQ(CHAMPION_SLOT_COUNT, 30,
              "champion runtime storage keeps 30 source inventory slots");
    ASSERT_EQ(CHAMPION_SLOT_ACTION_HAND, CHAMPION_SLOT_HAND_RIGHT,
              "action hand aliases the real C01 hand storage slot");
    ASSERT_EQ(M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot(CHAMPION_SLOT_BACKPACK_9), 29,
              "BACKPACK_9 maps to source slot box C528");
    ASSERT_EQ(M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot(CHAMPION_SLOT_BACKPACK_17), 37,
              "BACKPACK_17 maps to source slot box C536");
    ASSERT_EQ(M11_GameView_GetV1ChampionSlotForInventorySourceSlotBox(29), CHAMPION_SLOT_BACKPACK_9,
              "source slot box C528 maps back to BACKPACK_9");
    ASSERT_EQ(M11_GameView_GetV1ChampionSlotForInventorySourceSlotBox(37), CHAMPION_SLOT_BACKPACK_17,
              "source slot box C536 maps back to BACKPACK_17");
    ASSERT_EQ(M11_GameView_GetV1InventorySourceSlotBoxZoneCount(), 30,
              "source inventory exposes all C507..C536 slot-box zones");
}

static void test_extended_backpack_runtime_clicks(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int space = 0, zone = 0;

    seed_inventory_view(&state, &things, &weapon);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_9] = daggerThing;
    ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(29, &sx, &sy, &sw, &sh),
                "C528 zone exists");
    ASSERT_EQ(M11_GameView_GetV1MouseCommandForPoint(M11_DM1_MOUSE_LIST_INVENTORY,
                                                     sx + sw / 2,
                                                     33 + sy + sh / 2,
                                                     M11_DM1_MOUSE_MASK_LEFT,
                                                     &space,
                                                     &zone),
              49,
              "C528 resolves to source command C049");
    ASSERT_EQ(zone, 528, "C528 resolves to source zone id 528");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C528 picks the extended backpack item");
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_9], THING_NONE,
              "C528 pickup clears BACKPACK_9 storage");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), daggerThing,
              "C528 pickup moves item to leader hand");

    ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(37, &sx, &sy, &sw, &sh),
                "C536 zone exists");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C536 places leader hand into BACKPACK_17");
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_17], daggerThing,
              "C536 placement fills BACKPACK_17 storage");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "C536 placement clears leader hand");
}

static void test_open_chest_runtime_routes_and_clicks(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[3];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short axeThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    unsigned short arrowThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int space = 0, zone = 0;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 3;
    things.containers = containers;
    things.containerCount = 1;
    weapons[0].type = 2; /* ReDMCSB object-info index 25: container-compatible. */
    weapons[0].next = axeThing;
    weapons[1].type = 2;
    weapons[1].next = THING_ENDOFLIST;
    weapons[2].type = 4; /* ReDMCSB object-info index 27: quiver-only, not container. */
    weapons[2].next = THING_ENDOFLIST;
    containers[0].type = 0;
    containers[0].slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand container opens source chest panel state");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "open chest thing mirrors the action-hand container");
    ASSERT_EQ(M11_GameView_GetV1ChestSlotBoxZoneCount(), 8,
              "source chest panel exposes eight C537..C544 slots");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &sx, &sy, &sw, &sh),
                "C537 chest slot zone exists");
    ASSERT_EQ(M11_GameView_GetV1MouseCommandForPoint(M11_DM1_MOUSE_LIST_INVENTORY,
                                                     sx + sw / 2,
                                                     33 + sy + sh / 2,
                                                     M11_DM1_MOUSE_MASK_LEFT,
                                                     &space,
                                                     &zone),
              58,
              "C537 resolves to source command C058");
    ASSERT_EQ(zone, 537, "C537 route returns the source zone id");

    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C537 picks the first visible chest item");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), daggerThing,
              "chest pickup moves the slot object to the leader hand");
    ASSERT_EQ(containers[0].slot, axeThing,
              "open chest writeback compacts remaining visible slot objects");
    ASSERT_EQ(weapons[0].next, THING_ENDOFLIST,
              "picked chest object is detached from the container list");

    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(1, &sx, &sy, &sw, &sh),
                "C538 chest slot zone exists");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C538 places the leader-hand item into the chest");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "placing into chest clears the leader hand");
    ASSERT_EQ(containers[0].slot, axeThing,
              "chest list keeps the first existing object first");
    ASSERT_EQ(weapons[1].next, daggerThing,
              "placed object is linked after the existing visible item");
    ASSERT_EQ(weapons[0].next, THING_ENDOFLIST,
              "placed object terminates the compacted chest list");

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, arrowThing), 1,
              "leader hand accepts a source weapon thing for rejection test");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_IGNORED,
              "quiver-only object is rejected from the container slot");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), arrowThing,
              "rejected object stays in the leader hand");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "closing the panel clears open chest state");
}

static void test_action_hand_chest_panel_state_follows_slot_clicks(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short axeThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    int sx = 0, sy = 0, sw = 0, sh = 0;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 2;
    things.containers = containers;
    things.containerCount = 1;
    weapons[0].type = 2;
    weapons[0].next = axeThing;
    weapons[1].type = 2;
    weapons[1].next = THING_ENDOFLIST;
    containers[0].slot = daggerThing;

    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "open action-hand chest before action-hand removal");
    ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(9, &sx, &sy, &sw, &sh),
                "C508 action-hand source slot zone exists");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C508 picks up the open action-hand chest");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "removing the open action-hand chest clears panel state");
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND], THING_NONE,
              "action hand is empty after picking up the chest");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), chestThing,
              "removed chest moves to the leader hand");
    ASSERT_EQ(containers[0].slot, daggerThing,
              "closing action-hand panel state preserves container list head");
    ASSERT_EQ(weapons[0].next, axeThing,
              "closing action-hand panel state preserves linked contents");

    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &sx, &sy, &sw, &sh),
                "C537 chest source slot zone exists after close");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_IGNORED,
              "closed chest panel ignores stale C537 clicks");

    ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(9, &sx, &sy, &sw, &sh),
                "C508 action-hand source slot zone still exists");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "placing the leader-hand chest into C508 reopens panel state");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "placing a container in the action hand opens its chest panel");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "placing the chest clears the leader hand");
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND], chestThing,
              "action hand stores the placed chest");
}

static void test_object_description_layout_source_zones(void) {
    int x = -1, y = -1, w = -1, h = -1;
    const char* evidence = M11_GameView_GetV1ObjectDescriptionLayoutEvidence();

    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionPanelGraphicId(), 20,
              "object description uses C020 panel-empty graphic");
    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionCircleGraphicId(), 29,
              "object description uses C029 circle graphic");

    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionCircleZoneId(), 504,
              "object description circle source zone is C504");
    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionCircleZone(&x, &y, &w, &h), 1,
              "C504 circle zone resolves");
    ASSERT_EQ(x, 103, "C504 circle x follows layout-696 F0635");
    ASSERT_EQ(y, 53, "C504 circle y follows layout-696 F0635");
    ASSERT_EQ(w, 32, "C029 circle width resolves to 32 pixels");
    ASSERT_EQ(h, 27, "C029 circle height resolves to 27 pixels");

    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionIconZoneId(), 505,
              "object description icon source zone is C505");
    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionIconZone(&x, &y, &w, &h), 1,
              "C505 icon zone resolves");
    ASSERT_EQ(x, 111, "C505 icon x follows layout-696 F0635");
    ASSERT_EQ(y, 59, "C505 icon y follows layout-696 F0635");
    ASSERT_EQ(w, 16, "C505 icon width is one object-icon cell");
    ASSERT_EQ(h, 16, "C505 icon height is one object-icon cell");

    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionNameZoneId(), 506,
              "object description name source zone is C506");
    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionNameZoneForText(80, 7, &x, &y, &w, &h), 1,
              "C506 text zone resolves for an 80x7 measured name");
    ASSERT_EQ(x, 134, "C506 name x follows TEXT.C/F0635");
    ASSERT_EQ(y, 64, "C506 name top follows type-8 vertical centering");
    ASSERT_EQ(w, 80, "C506 keeps caller-measured text width");
    ASSERT_EQ(h, 7, "C506 keeps caller-measured text height");

    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionContinuationOrigin(&x, &y), 1,
              "C556 continuation text origin resolves");
    ASSERT_EQ(x, 108, "C556 form-feed x includes one-pixel margin");
    ASSERT_EQ(y, 59, "C556 form-feed y includes one-pixel margin");

    ASSERT_TRUE(evidence && strstr(evidence, "PANEL.C:1136-1145") != NULL,
                "layout evidence cites object-description panel blits");
    ASSERT_TRUE(strstr(evidence, "TEXT.C:1937-1950") != NULL,
                "layout evidence cites C506 text zone resolver");
    ASSERT_TRUE(strstr(evidence, "COORD.C:2052-2412") != NULL,
                "layout evidence cites F0635 layout resolver");
}

static void test_eye_panel_weapon_attribute_flags(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);

    seed_inventory_view(&state, &things, &weapon);
    weapon.type = 8;
    weapon.cursed = 1;
    weapon.poisoned = 1;
    weapon.broken = 1;
    weapon.chargeCount = 7;

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, daggerThing), 1,
              "leader hand accepts source weapon thing");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click opens source weapon description");
    ASSERT_TRUE(strstr(state.inspectDetail, "CURSED") != NULL,
                "weapon eye panel reports source cursed flag");
    ASSERT_TRUE(strstr(state.inspectDetail, "POISONED") != NULL,
                "weapon eye panel reports source poisoned flag");
    ASSERT_TRUE(strstr(state.inspectDetail, "BROKEN") != NULL,
                "weapon eye panel reports source broken flag");
    ASSERT_TRUE(strstr(state.inspectDetail, "CHARGE 7") != NULL,
                "weapon eye panel reports source charge count");
}

static void test_eye_panel_potion_power_prefix_runtime(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonPotion_Compat potions[3];
    unsigned short rosPotionThing = (unsigned short)((THING_TYPE_POTION << 10) | 0);
    unsigned short waterFlaskThing = (unsigned short)((THING_TYPE_POTION << 10) | 1);
    unsigned short emptyFlaskThing = (unsigned short)((THING_TYPE_POTION << 10) | 2);

    seed_inventory_view(&state, &things, &weapon);
    memset(potions, 0, sizeof(potions));
    things.potions = potions;
    things.potionCount = 3;
    potions[0].type = 6;   /* ROS POTION: DUNGEON.C object-info index 8, icon C154. */
    potions[0].power = 80; /* PANEL.C:1184 => '_' + 2 == 'a'. */
    potions[1].type = 15;  /* WATER FLASK: DUNGEON.C object-info index 17, icon C163. */
    potions[1].power = 120;
    potions[2].type = 20;  /* EMPTY FLASK: DUNGEON.C object-info index 22, icon C195. */
    potions[2].power = 0;

    state.world.party.champions[0].skillLevels[CHAMPION_SKILL_PRIEST] = 2;
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, rosPotionThing), 1,
              "leader hand accepts source ROS potion thing");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click opens source ROS potion description");
    ASSERT_TRUE(strstr(state.inspectTitle, "POTION: a ROS POTION") != NULL,
                "priest skill > 1 prefixes non-water potion name in eye panel");
    ASSERT_TRUE(strstr(state.inspectDetail, "PANEL a ROS POTION") != NULL,
                "runtime potion detail carries source PANEL.C description text");
    ASSERT_EQ(M11_GameView_DismissDialogOverlay(&state), 1,
              "dismiss ROS potion eye-panel overlay");

    state.world.party.champions[0].skillLevels[CHAMPION_SKILL_PRIEST] = 4;
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, waterFlaskThing), 1,
              "leader hand accepts source water flask thing");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click opens source water flask description");
    ASSERT_TRUE(strstr(state.inspectTitle, "POTION: WATER FLASK") != NULL,
                "water flask is excluded from the priest-skill power prefix");
    ASSERT_EQ(M11_GameView_DismissDialogOverlay(&state), 1,
              "dismiss water flask eye-panel overlay");

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, emptyFlaskThing), 1,
              "leader hand accepts source empty flask thing");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click opens source empty flask description");
    ASSERT_TRUE(strstr(state.inspectTitle, "POTION: _ EMPTY FLASK") != NULL,
                "empty flask keeps the original non-water potion prefix quirk");
}

static void test_eye_panel_champion_stats_and_skills(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct ChampionState_Compat* champ;

    seed_inventory_view(&state, &things, &weapon);
    champ = &state.world.party.champions[0];
    champ->hp.current = 77;
    champ->hp.maximum = 100;
    champ->stamina.current = 66;
    champ->stamina.maximum = 90;
    champ->mana.current = 12;
    champ->mana.maximum = 33;
    champ->attributes[CHAMPION_ATTR_STRENGTH] = 41;
    champ->attributes[CHAMPION_ATTR_DEXTERITY] = 42;
    champ->attributes[CHAMPION_ATTR_WISDOM] = 43;
    champ->attributes[CHAMPION_ATTR_VITALITY] = 44;
    champ->attributes[CHAMPION_ATTR_ANTIMAGIC] = 45;
    champ->attributes[CHAMPION_ATTR_ANTIFIRE] = 46;
    champ->skillLevels[CHAMPION_SKILL_FIGHTER] = 2;
    champ->skillLevels[CHAMPION_SKILL_NINJA] = 3;
    champ->skillLevels[CHAMPION_SKILL_PRIEST] = 4;
    champ->skillLevels[CHAMPION_SKILL_WIZARD] = 5;

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click with empty leader hand opens champion stats");
    ASSERT_TRUE(strstr(state.inspectDetail, "MANA 12/33") != NULL,
                "champion stats panel reports mana");
    ASSERT_TRUE(strstr(state.inspectDetail, "STR 41") != NULL &&
                strstr(state.inspectDetail, "DEX 42") != NULL &&
                strstr(state.inspectDetail, "WIS 43") != NULL &&
                strstr(state.inspectDetail, "VIT 44") != NULL,
                "champion stats panel reports core statistics");
    ASSERT_TRUE(strstr(state.inspectDetail, "AM 45") != NULL &&
                strstr(state.inspectDetail, "AF 46") != NULL,
                "champion stats panel reports anti-magic and anti-fire");
    ASSERT_TRUE(strstr(state.inspectDetail, "FTR 2") != NULL &&
                strstr(state.inspectDetail, "NIN 3") != NULL &&
                strstr(state.inspectDetail, "PRI 4") != NULL &&
                strstr(state.inspectDetail, "WIZ 5") != NULL,
                "champion stats panel reports base skill levels");
}

int main(void) {
    printf("=== M11 Inventory Full Panel Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: DEFS.H 778-817, DATA.C 1049-1087, CHAMPION.C F0302 677-712, PANEL.C F0347 1651-1691\n\n");

    test_extended_backpack_source_mapping();
    test_extended_backpack_runtime_clicks();
    test_open_chest_runtime_routes_and_clicks();
    test_action_hand_chest_panel_state_follows_slot_clicks();
    test_eye_panel_potion_power_prefix_runtime();
    test_object_description_layout_source_zones();
    test_eye_panel_weapon_attribute_flags();
    test_eye_panel_champion_stats_and_skills();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
