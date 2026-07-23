#include "dm1_v1_inventory_panel_action_hand_f0347_pc34_compat.h"

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
    struct DungeonWeapon_Compat weapon;
    unsigned char raw[4] = { 0xfe, 0xff, 0x83, 0x82 };
    DM1_V1_InventoryPanelActionHandReceiptF0347Pc34 receipt;
    const unsigned short sword = (unsigned short)(THING_TYPE_WEAPON << 10);
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(&weapon, 0, sizeof(weapon));
    things.loaded = 1;
    things.weapons = &weapon;
    things.weaponCount = 1;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    things.rawThingData[THING_TYPE_WEAPON] = raw;
    weapon.next = THING_ENDOFLIST;
    weapon.type = 3u;
    weapon.doNotDiscard = 1u;
    weapon.poisoned = 1u;
    weapon.lit = 1u;

    ok &= check(dm1_v1_inventory_panel_action_hand_admit_f0347_pc34(
                    &things, sword, &receipt) && receipt.valid &&
                    receipt.actionHandThing == sword && receipt.weaponType == 3u &&
                    receipt.rawFingerprint != 0u,
                "F0347 accepts only a coherent raw C05 action-hand weapon");
    raw[3] ^= 0x80u;
    ok &= check(dm1_v1_inventory_panel_action_hand_admit_f0347_pc34(
                    &things, sword, &receipt) && !receipt.valid,
                "F0347 rejects decoded/raw C05 drift");
    raw[3] ^= 0x80u;
    ok &= check(dm1_v1_inventory_panel_action_hand_admit_f0347_pc34(
                    &things, THING_NONE, &receipt) && !receipt.valid,
                "F0347 rejects no action-hand Thing rather than inventing a panel route");
    if (!ok) return 1;
    puts("PASS: DM1 F0347 raw C05 action-hand admission");
    return 0;
}
