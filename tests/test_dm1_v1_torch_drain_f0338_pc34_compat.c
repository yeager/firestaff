#include "dm1_v1_torch_drain_f0338_pc34_compat.h"

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
    unsigned char raw[8] = { 0xfe, 0xff, 0x82, 0x94, 0xfe, 0xff, 2, 0x18 };
    unsigned short hands[DM1_V1_F0338_HAND_COUNT_PC34];
    DM1_V1_TorchDrainReceiptF0338Pc34 receipt;
    const unsigned short torch0 = (unsigned short)(THING_TYPE_WEAPON << 10);
    const unsigned short torch1 = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    int i;
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    for (i = 0; i < DM1_V1_F0338_HAND_COUNT_PC34; ++i) hands[i] = THING_NONE;
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 2;
    things.thingCounts[THING_TYPE_WEAPON] = 2;
    things.rawThingData[THING_TYPE_WEAPON] = raw;
    weapons[0].next = THING_ENDOFLIST;
    weapons[0].type = DM1_V1_F0338_TORCH_WEAPON_TYPE_PC34;
    weapons[0].doNotDiscard = 1;
    weapons[0].chargeCount = 5;
    weapons[0].lit = 1;
    weapons[1].next = THING_ENDOFLIST;
    weapons[1].type = DM1_V1_F0338_TORCH_WEAPON_TYPE_PC34;
    weapons[1].doNotDiscard = 0;
    weapons[1].chargeCount = 6;
    weapons[1].lit = 0;
    hands[0] = torch0;
    hands[1] = torch1;

    ok &= check(dm1_v1_torch_drain_f0338_pc34(&things, hands, 1, &receipt) &&
                    receipt.valid && receipt.changedCount == 1 &&
                    receipt.drainedThings[0] == torch0 &&
                    weapons[0].chargeCount == 4 &&
                    ((raw[2] | ((unsigned short)raw[3] << 8)) >> 10 & 0x0fu) == 4 &&
                    weapons[1].chargeCount == 6,
                "F0338 drains only a raw-authenticated lit torch");
    raw[3] ^= 0x80u;
    ok &= check(dm1_v1_torch_drain_f0338_pc34(&things, hands, 1, &receipt) &&
                    !receipt.valid && weapons[0].chargeCount == 4,
                "raw C05 drift fails closed before charge mutation");
    raw[3] ^= 0x80u;
    hands[1] = torch0;
    ok &= check(dm1_v1_torch_drain_f0338_pc34(&things, hands, 1, &receipt) &&
                    !receipt.valid && weapons[0].chargeCount == 4,
                "duplicate lit torch identity fails closed");
    if (!ok) return 1;
    puts("PASS: DM1 F0338 raw torch drain");
    return 0;
}
