/*
 * Dedicated runtime regression for ReDMCSB CHAMPION.C F0318:1527-1550
 * and F0319:1552-1687.  This drives the production M11 death route, not
 * the older isolated resurrection helper gate.
 */
#include "m11_game_view.h"
#include "dm1_v1_resurrection_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", (message)); \
        ++g_failures; \
    } \
} while (0)

static unsigned short thing_id(int type, int index, int cell)
{
    return (unsigned short)(((cell & 3) << 14) | ((type & 15) << 10) |
                            (index & 0x3ff));
}

static void seed_world(M11_GameViewState* state,
                       struct DungeonDatState_Compat* dungeon,
                       struct DungeonThings_Compat* things,
                       struct DungeonWeapon_Compat weapons[2],
                       struct DungeonJunk_Compat junks[2],
                       struct DungeonMapDesc_Compat maps[1],
                       unsigned short square_first_things[1],
                       unsigned char weapon_raw[8],
                       unsigned char junk_raw[8])
{
    struct ChampionState_Compat* dead;
    struct ChampionState_Compat* survivor;
    int slot;

    memset(dungeon, 0, sizeof(*dungeon));
    memset(things, 0, sizeof(*things));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat));
    memset(weapons, 0, sizeof(struct DungeonWeapon_Compat) * 2u);
    memset(junks, 0, sizeof(struct DungeonJunk_Compat) * 2u);
    memset(square_first_things, 0xff, sizeof(unsigned short));
    memset(weapon_raw, 0xff, 8u);
    memset(junk_raw, 0xff, 8u);

    dungeon->header.mapCount = 1;
    dungeon->maps = maps;
    maps[0].width = 1;
    maps[0].height = 1;
    things->squareFirstThings = square_first_things;
    things->squareFirstThingCount = 1;
    things->weapons = weapons;
    things->weaponCount = 2;
    things->junks = junks;
    things->junkCount = 2;
    things->rawThingData[THING_TYPE_WEAPON] = weapon_raw;
    things->thingCounts[THING_TYPE_WEAPON] = 2;
    things->rawThingData[THING_TYPE_JUNK] = junk_raw;
    things->thingCounts[THING_TYPE_JUNK] = 2;

    /* Index 0 is live inventory; index 1 is the F0319 unused bones slot. */
    weapons[0].next = THING_ENDOFLIST;
    weapons[0].type = 7;
    weapons[1].next = THING_NONE;
    junks[0].next = THING_ENDOFLIST;
    junks[0].type = 4;
    junks[1].next = THING_NONE;

    state->active = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.party.championCount = 2;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 0;
    state->world.party.mapY = 0;
    state->world.party.activeChampionIndex = 0;
    state->inventoryPanelActive = 1;
    state->spellPanelOpen = 1;

    dead = &state->world.party.champions[0];
    dead->present = 1;
    dead->cell = 2;
    dead->hp.current = 0;
    dead->hp.maximum = 100;
    dead->wounds = 0xffffu;
    dead->poisonDose = 12;
    for (slot = 0; slot < CHAMPION_SLOT_COUNT; ++slot) {
        dead->inventory[slot] = THING_NONE;
    }
    dead->inventory[CHAMPION_SLOT_ACTION_HAND] =
        thing_id(THING_TYPE_WEAPON, 0, 0);

    survivor = &state->world.party.champions[1];
    survivor->present = 1;
    survivor->hp.current = 100;
    survivor->hp.maximum = 100;
}

static void test_surviving_party_death_route(void)
{
    static M11_GameViewState state;
    static struct DungeonDatState_Compat dungeon;
    static struct DungeonThings_Compat things;
    static struct DungeonWeapon_Compat weapons[2];
    static struct DungeonJunk_Compat junks[2];
    static struct DungeonMapDesc_Compat maps[1];
    static unsigned short square_first_things[1];
    static unsigned char weapon_raw[8];
    static unsigned char junk_raw[8];
    unsigned short bones;
    unsigned short dropped_weapon;

    M11_GameView_Init(&state);
    seed_world(&state, &dungeon, &things, weapons, junks, maps,
               square_first_things, weapon_raw, junk_raw);
    M11_GameView_ProbeCheckPartyDeath(&state);

    bones = thing_id(THING_TYPE_JUNK, 1, 2);
    dropped_weapon = thing_id(THING_TYPE_WEAPON, 0, 2);
    CHECK(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
              THING_NONE,
          "F0318 removes the dead champion's action-hand object");
    CHECK(square_first_things[0] == bones,
          "F0319 prepends champion bones after dropped inventory");
    CHECK(junks[1].type == DM1_JUNK_TYPE_BONES && junks[1].doNotDiscard == 1 &&
              junks[1].chargeCount == 0,
          "F0319 initializes bones type, persistence, and champion index");
    CHECK(junks[1].next == dropped_weapon,
          "F0318 drop remains chained below F0319 bones");
    CHECK(state.world.party.champions[0].wounds == 0 &&
              state.world.party.champions[0].poisonDose == 0,
          "F0319 clears wounds and poison");
    CHECK(state.inventoryPanelActive == 0 && state.spellPanelOpen == 0,
          "F0319 clears dead champion UI ownership");
    CHECK(state.world.party.activeChampionIndex == 1,
          "F0319 selects the first surviving champion as leader");
    CHECK(state.partyDead == 0 && state.world.partyDead == 0,
          "a surviving champion prevents the party-dead flag");
}

static void test_last_champion_sets_party_dead(void)
{
    static M11_GameViewState state;
    static struct DungeonDatState_Compat dungeon;
    static struct DungeonThings_Compat things;
    static struct DungeonWeapon_Compat weapons[2];
    static struct DungeonJunk_Compat junks[2];
    static struct DungeonMapDesc_Compat maps[1];
    static unsigned short square_first_things[1];
    static unsigned char weapon_raw[8];
    static unsigned char junk_raw[8];

    M11_GameView_Init(&state);
    seed_world(&state, &dungeon, &things, weapons, junks, maps,
               square_first_things, weapon_raw, junk_raw);
    state.world.party.champions[1].hp.current = 0;
    M11_GameView_ProbeCheckPartyDeath(&state);

    CHECK(state.partyDead == 1 && state.world.partyDead == 1,
          "F0319 marks the party dead after the final champion falls");
}

int main(void)
{
    test_surviving_party_death_route();
    test_last_champion_sets_party_dead();
    if (g_failures != 0) {
        fprintf(stderr, "DM1-002 failed with %d assertion(s)\n", g_failures);
        return 1;
    }
    printf("DM1-002 CHAMPION.C F0318/F0319 runtime regression passed\n");
    return 0;
}
