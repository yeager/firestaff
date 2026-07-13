/*
 * Runtime receipt for the F0407/F0231 ordering boundary.
 *
 * ReDMCSB MENU.C F0407:1613-1628 resolves F0402/F0231 before its
 * F0325 action-stamina tail.  PROJEXPL.C F0231 reads F0312 strength, whose
 * low-stamina adjustment must therefore observe the pre-action stamina.
 */

#include "m11_game_view.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", (message)); \
        ++g_failures; \
    } \
} while (0)

typedef struct F0407F0231ReceiptPc34 {
    int actionIndex;
    int actionCost;
    int damage;
    int finalStamina;
} F0407F0231ReceiptPc34;

static unsigned short make_thing(int type, int index) {
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static int is_melee_action(unsigned char actionIndex) {
    return actionIndex != DM1_ACTION_BLOCK &&
           dm1_v1_graphic560_action_damage_factor_get_pc34(actionIndex) > 0;
}

static int run_live_f0407_f0231_receipt(
    unsigned short stamina,
    F0407F0231ReceiptPc34* outReceipt) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonGroup_Compat groups[1];
    unsigned char actions[3];
    DM1_ActionF0407BeginInputPc34 beginIn;
    DM1_ActionF0407BeginPlanPc34 beginPlan;
    int row = -1;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&state, 0, sizeof(state));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(groups, 0, sizeof(groups));

    M11_GameView_Init(&state);
    state.active = 1;
    state.world.gameTick = 7;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].stamina.current = stamina;
    state.world.party.champions[0].stamina.maximum = 100;
    state.world.party.champions[0].food = 2048;
    state.world.party.champions[0].water = 2048;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    state.world.party.champions[0].direction = 2; /* South. */
    state.world.party.champions[0].name[0] = 'H';
    state.world.party.champions[0].name[1] = 'A';
    state.world.party.champions[0].name[2] = 'L';
    state.world.party.champions[0].name[3] = 'K';
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
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1; /* East: the party front is empty. */

    weapons[0].type = 8; /* Dagger action set contains a live melee row. */
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    groups[0].cells = 0xff;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;

    if (!M11_GameView_SetActingChampion(&state, 0) ||
        !M11_GameView_GetActingActionIndices(&state, actions)) {
        return 0;
    }
    for (i = 0; i < 3; ++i) {
        if (is_melee_action(actions[i])) {
            row = i;
            break;
        }
    }
    if (row < 0) return 0;

    memset(&beginIn, 0, sizeof(beginIn));
    beginIn.actionIndex = actions[row];
    beginIn.championIndex = 0;
    beginIn.gameTick = state.world.gameTick;
    beginIn.currentStamina = stamina;
    beginIn.maximumStamina = 100;
    beginIn.currentHealth = 100;
    if (!dm1_v1_action_begin_plan_f0407_pc34(&beginIn, &beginPlan) ||
        !beginPlan.valid) {
        return 0;
    }
    if (!M11_GameView_TriggerActionRow(&state, row)) return 0;

    outReceipt->actionIndex = actions[row];
    outReceipt->actionCost = beginPlan.staminaCost;
    outReceipt->damage = 200 - groups[0].health[0];
    outReceipt->finalStamina = state.world.party.champions[0].stamina.current;
    return 1;
}

int main(void) {
    F0407F0231ReceiptPc34 atHalf;
    F0407F0231ReceiptPc34 aboveHalf;

    CHECK(run_live_f0407_f0231_receipt(50, &atHalf),
          "50-stamina F0407/F0231 live receipt is available");
    CHECK(run_live_f0407_f0231_receipt(64, &aboveHalf),
          "64-stamina F0407/F0231 live receipt is available");
    CHECK(atHalf.actionIndex == aboveHalf.actionIndex,
          "both receipts use the same real PC34 action row");
    CHECK(atHalf.actionCost > 0 && atHalf.actionCost == aboveHalf.actionCost,
          "F0407 cost is one stable G0494/M005 receipt");
    CHECK(atHalf.damage > 0 && aboveHalf.damage > 0,
          "both live F0231 routes hit the authentic south-facing target");
    /* F0312 scales only below half stamina.  Both pre-action values are at
     * or above half, so MENU.C's F0407 tail order requires equal F0231
     * damage under the same deterministic M10 RNG receipt. */
    CHECK(atHalf.damage == aboveHalf.damage,
          "F0231 strength precedes F0407 F0325 action stamina tail");

    if (g_failures != 0) {
        fprintf(stderr, "%d F0407/F0231 stamina-order check(s) failed\\n",
                g_failures);
        return 1;
    }
    printf("F0407/F0231 stamina-order runtime receipt passed\\n");
    return 0;
}
