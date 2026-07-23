#include "dm1_v1_chest_admission_f0333_f0334_pc34_compat.h"

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
    struct DungeonContainer_Compat container;
    struct DungeonWeapon_Compat weapons[2];
    unsigned char rawContainer[8] = { 0xfe, 0xff, 0x00, 0x14, 0, 0, 0, 0 };
    unsigned char rawWeapons[8] = {
        0x01, 0x14, 8, 0, 0xfe, 0xff, 9, 0
    };
    unsigned short slots[DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34];
    DM1_ChestAdmissionReceiptF0333F0334Pc34 receipt;
    const unsigned short chest = (unsigned short)((THING_TYPE_CONTAINER << 10));
    const unsigned short weapon0 = (unsigned short)((THING_TYPE_WEAPON << 10));
    const unsigned short weapon1 = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    int ok = 1;
    int i;

    memset(&things, 0, sizeof(things));
    memset(&container, 0, sizeof(container));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.containers = &container;
    things.containerCount = 1;
    things.weapons = weapons;
    things.weaponCount = 2;
    things.thingCounts[THING_TYPE_CONTAINER] = 1;
    things.thingCounts[THING_TYPE_WEAPON] = 2;
    things.rawThingData[THING_TYPE_CONTAINER] = rawContainer;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapons;
    container.next = THING_ENDOFLIST;
    container.slot = weapon0;
    weapons[0].next = weapon1;
    weapons[1].next = THING_ENDOFLIST;

    ok &= check(dm1_v1_chest_open_admit_f0333_pc34(&things, chest, &receipt) &&
                    receipt.valid && receipt.slotCount == 2 &&
                    receipt.slots[0] == weapon0 && receipt.slots[1] == weapon1,
                "F0333 admits an intact raw C09 chain");
    for (i = 0; i < DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34; ++i)
        slots[i] = THING_NONE;
    slots[0] = weapon1;
    slots[1] = weapon0;
    ok &= check(dm1_v1_chest_close_admit_f0334_pc34(
                    &things, chest, slots, &receipt) && receipt.valid &&
                    receipt.slotCount == 2,
                "F0334 admits reordered visible raw Things");
    ok &= check(dm1_v1_chest_close_commit_f0334_pc34(
                    &things, chest, slots, &receipt) && receipt.valid &&
                    container.slot == weapon1 && weapons[1].next == weapon0 &&
                    weapons[0].next == THING_ENDOFLIST &&
                    rawContainer[2] == 0x01 && rawContainer[3] == 0x14 &&
                    rawWeapons[0] == 0xfe && rawWeapons[1] == 0xff &&
                    rawWeapons[4] == 0x00 && rawWeapons[5] == 0x14,
                "F0334 commits decoded and raw C09/generic next bytes together");
    slots[2] = weapon0;
    ok &= check(dm1_v1_chest_close_admit_f0334_pc34(
                    &things, chest, slots, &receipt) && !receipt.valid,
                "duplicate visible Thing fails closed");
    slots[2] = THING_NONE;
    rawContainer[2] = 0xfe;
    rawContainer[3] = 0xff;
    ok &= check(dm1_v1_chest_open_admit_f0333_pc34(
                    &things, chest, &receipt) && !receipt.valid,
                "raw C09 slot drift fails closed");
    if (!ok) return 1;
    puts("PASS: DM1 F0333/F0334 raw chest admission");
    return 0;
}
