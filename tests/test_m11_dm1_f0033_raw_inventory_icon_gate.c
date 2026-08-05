#include "m11_game_view.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;
static int assertions = 0;

#define CHECK_EQ(label, actual, expected) do { \
    int got_ = (actual); \
    ++assertions; \
    if (got_ != (expected)) { \
        ++failures; \
        fprintf(stderr, "FAIL %s: got %d want %d\n", (label), got_, (expected)); \
    } \
} while (0)

static unsigned short make_thing(int type, int index)
{
    return (unsigned short)(((unsigned int)type << 10) | (unsigned int)index);
}

int main(void)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat decodedWeapon;
    /* PC34 torch subtype 2, lit, ChargeCount 4: raw byte 3 packs
     * ChargeCount in bits 2..5 and Lit in bit 7. */
    unsigned char rawWeapon[4] = { 0, 0, 2, 0x90u };
    unsigned short torch = make_thing(THING_TYPE_WEAPON, 0);

    memset(&state, 0, sizeof(state));
    memset(&things, 0, sizeof(things));
    memset(&decodedWeapon, 0, sizeof(decodedWeapon));
    things.loaded = 1;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapon;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    /* Deliberately contradictory decoded data: F0033 must select the raw
     * source record's lit-torch charge band, not this M11 mirror. */
    decodedWeapon.type = 0;
    decodedWeapon.lit = 0;
    decodedWeapon.chargeCount = 0;
    things.weapons = &decodedWeapon;
    things.weaponCount = 1;
    state.world.things = &things;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = torch;

    CHECK_EQ("raw F0033 inventory icon",
             DM1_V1_M11Runtime_GetInventorySlotIconIndexPc34Compat(
                 &state, CHAMPION_SLOT_ACTION_HAND),
             6);
    CHECK_EQ("raw F0141 action-hand allowed slots",
             (int)dm1_v1_dungeon_get_object_allowed_slots_pc34(&things, torch),
             0x0400);

    state.leaderHandObjectPresent = 1;
    state.leaderHandThing = torch;
    state.leaderHandIconIndex = -1;
    CHECK_EQ("raw F0033 leader-hand icon",
             DM1_V1_M11Runtime_GetLeaderHandObjectIconIndexPc34Compat(&state),
             6);

    things.rawThingData[THING_TYPE_WEAPON] = NULL;
    CHECK_EQ("missing raw allowed-slots record is rejected",
             (int)dm1_v1_dungeon_get_object_allowed_slots_pc34(&things, torch),
             0);
    CHECK_EQ("missing raw inventory record is no-draw",
             DM1_V1_M11Runtime_GetInventorySlotIconIndexPc34Compat(
                 &state, CHAMPION_SLOT_ACTION_HAND),
             -1);
    CHECK_EQ("missing raw leader-hand record is no-draw",
             DM1_V1_M11Runtime_GetLeaderHandObjectIconIndexPc34Compat(&state),
             -1);

    printf("test_m11_dm1_f0033_raw_inventory_icon_gate: %d/%d assertions passed\n",
           assertions - failures, assertions);
    return failures == 0 ? 0 : 1;
}
