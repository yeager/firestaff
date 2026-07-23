#include "dm1_v1_dungeon_light_admission_f0337_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int value, const char *message)
{
    if (!value) fprintf(stderr, "FAIL: %s\n", message);
    return value;
}

int main(void)
{
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    unsigned char raw[8] = { 0xfe, 0xff, 2, 0x98, 0xfe, 0xff, 3, 0x1c };
    unsigned short hands[DM1_V1_F0337_HAND_COUNT_PC34];
    DM1_V1_DungeonLightReceiptF0337Pc34 receipt;
    const unsigned short torch = (unsigned short)(THING_TYPE_WEAPON << 10);
    const unsigned short unlit = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    int i;
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    for (i = 0; i < DM1_V1_F0337_HAND_COUNT_PC34; ++i) hands[i] = THING_NONE;
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 2;
    things.thingCounts[THING_TYPE_WEAPON] = 2;
    things.rawThingData[THING_TYPE_WEAPON] = raw;
    weapons[0].next = THING_ENDOFLIST;
    weapons[0].type = 2;
    weapons[0].chargeCount = 6;
    weapons[0].lit = 1;
    weapons[1].next = THING_ENDOFLIST;
    weapons[1].type = 3;
    weapons[1].chargeCount = 7;
    weapons[1].lit = 0;
    hands[0] = torch;
    hands[1] = unlit;

    ok &= check(dm1_v1_dungeon_light_admit_f0337_pc34(
                    &things, hands, &receipt) && receipt.valid &&
                    receipt.torchLightPower[0] == 6 &&
                    receipt.torchLightPower[1] == 0,
                "F0337 reads light only from raw-authenticated lit C05 torches");
    raw[3] ^= 0x80u;
    ok &= check(dm1_v1_dungeon_light_admit_f0337_pc34(
                    &things, hands, &receipt) && !receipt.valid,
                "C05 lit-bit drift fails closed");
    raw[3] ^= 0x80u;
    hands[1] = torch;
    ok &= check(dm1_v1_dungeon_light_admit_f0337_pc34(
                    &things, hands, &receipt) && !receipt.valid,
                "duplicate hand Thing fails closed");
    if (!ok) return 1;
    puts("PASS: DM1 F0337 raw dungeon-light admission");
    return 0;
}
