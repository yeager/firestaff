#include "dm1_v1_leader_hand_throw_admission_f0329_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct PartyState_Compat party;
    unsigned char rawWeapon[4] = { 0xfe, 0xff, 8, 0 };
    DM1_LeaderHandThrowAdmissionInputF0329Pc34 input;
    DM1_LeaderHandThrowAdmissionReceiptF0329Pc34 receipt;
    uint32_t firstRawFingerprint;
    const unsigned short weaponThing =
        (unsigned short)(THING_TYPE_WEAPON << 10);
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(&weapon, 0, sizeof(weapon));
    memset(&party, 0, sizeof(party));
    memset(&input, 0, sizeof(input));
    things.loaded = 1;
    things.weapons = &weapon;
    things.weaponCount = 1;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapon;
    party.championCount = 1;
    party.activeChampionIndex = 0;
    party.champions[0].present = 1;
    party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = THING_NONE;
    input.things = &things;
    input.party = &party;
    input.leaderIndex = 0;
    input.leaderHandThing = weaponThing;
    input.throwSide = 1;

    ok &= check(dm1_v1_leader_hand_throw_admit_f0329_pc34(
                    &input, &receipt) && receipt.valid &&
                    receipt.leaderIndex == 0 && receipt.throwSide == 1 &&
                    receipt.leaderHandThing == weaponThing &&
                    receipt.rawLeaderHandFNV1a != 0u &&
                    receipt.partyFNV1a != 0u,
                "F0329 admits a loaded raw leader-hand Thing");
    firstRawFingerprint = receipt.rawLeaderHandFNV1a;
    rawWeapon[2] = 9;
    ok &= check(dm1_v1_leader_hand_throw_admit_f0329_pc34(
                    &input, &receipt) && receipt.valid &&
                    receipt.rawLeaderHandFNV1a != firstRawFingerprint,
                "F0329 receipt follows the current raw Thing bytes");
    things.rawThingData[THING_TYPE_WEAPON] = NULL;
    ok &= check(dm1_v1_leader_hand_throw_admit_f0329_pc34(
                    &input, &receipt) && !receipt.valid,
                "missing raw leader-hand Thing fails closed");
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapon;
    input.leaderIndex = 1;
    ok &= check(dm1_v1_leader_hand_throw_admit_f0329_pc34(
                    &input, &receipt) && !receipt.valid,
                "nonleader F0329 caller fails closed");

    if (!ok) return 1;
    puts("PASS: DM1 F0329 source-bound leader-hand throw admission");
    return 0;
}
