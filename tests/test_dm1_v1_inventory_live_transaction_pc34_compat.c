#include "dm1_v1_inventory_live_transaction_pc34_compat.h"

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
    unsigned char rawWeapons[8] = { 0x01, 0x14, 0, 0, 0xfe, 0xff, 1, 0 };
    unsigned char rawPotion[4] = { 0xfe, 0xff, 80, 6 };
    unsigned char rawScroll[4] = { 0xfe, 0xff, 0, 0x14 };
    unsigned char rawJunk[4] = { 0xfe, 0xff, 29, 0 };
    uint16_t slots[DM1_PC34_INVENTORY_SLOT_COUNT];
    const uint16_t chest = (uint16_t)(THING_TYPE_CONTAINER << 10);
    const uint16_t weapon0 = (uint16_t)(THING_TYPE_WEAPON << 10);
    const uint16_t weapon1 = (uint16_t)((THING_TYPE_WEAPON << 10) | 1);
    const uint16_t potion = (uint16_t)(THING_TYPE_POTION << 10);
    const uint16_t scroll = (uint16_t)(THING_TYPE_SCROLL << 10);
    const uint16_t junk = (uint16_t)(THING_TYPE_JUNK << 10);
    DM1_V1_InventoryLiveTransactionPc34 state;
    DM1_ChestAdmissionReceiptF0333F0334Pc34 chestReceipt;
    DM1_V1_InventoryLiveUseReceiptPc34 useReceipt;
    DM1ConsumableChampionPc34 champion;
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(&container, 0, sizeof(container));
    memset(weapons, 0, sizeof(weapons));
    memset(&champion, 0, sizeof(champion));
    things.loaded = 1;
    things.containers = &container;
    things.containerCount = 1;
    things.weapons = weapons;
    things.weaponCount = 2;
    things.thingCounts[THING_TYPE_CONTAINER] = 1;
    things.thingCounts[THING_TYPE_WEAPON] = 2;
    things.thingCounts[THING_TYPE_POTION] = 1;
    things.thingCounts[THING_TYPE_SCROLL] = 1;
    things.thingCounts[THING_TYPE_JUNK] = 1;
    things.rawThingData[THING_TYPE_CONTAINER] = rawContainer;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapons;
    things.rawThingData[THING_TYPE_POTION] = rawPotion;
    things.rawThingData[THING_TYPE_SCROLL] = rawScroll;
    things.rawThingData[THING_TYPE_JUNK] = rawJunk;
    container.next = THING_ENDOFLIST;
    container.slot = weapon0;
    weapons[0].next = weapon1;
    weapons[1].next = THING_ENDOFLIST;

    memset(slots, 0xff, sizeof(slots));
    slots[DM1_PC34_SLOT_READY_HAND] = chest;
    slots[DM1_PC34_SLOT_ACTION_HAND] = potion;
    slots[DM1_PC34_SLOT_BACKPACK_LINE1_1] = scroll;
    ok &= check(dm1_v1_inventory_live_begin_pc34(&state, &things, slots),
                "only authenticated raw C05-C10 Things enter the live panel");
    ok &= check(dm1_v1_inventory_live_click_slot_pc34(&state, DM1_PC34_SLOT_READY_HAND) &&
                state.mouseThing == chest && state.slots[DM1_PC34_SLOT_READY_HAND] == THING_NONE,
                "F0300 removes a real container into the leader hand transactionally");
    ok &= check(!dm1_v1_inventory_live_click_slot_pc34(&state, DM1_PC34_SLOT_HEAD) &&
                state.mouseThing == chest,
                "raw F0141 slot masks reject a chest in the head slot without mutation");
    ok &= check(dm1_v1_inventory_live_click_slot_pc34(&state, DM1_PC34_SLOT_READY_HAND),
                "container returns to a compatible real hand slot");

    ok &= check(dm1_v1_inventory_live_click_slot_pc34(&state, DM1_PC34_SLOT_READY_HAND) &&
                dm1_v1_inventory_live_click_slot_pc34(&state, DM1_PC34_SLOT_ACTION_HAND),
                "hand swap keeps raw Thing identity");
    ok &= check(state.slots[DM1_PC34_SLOT_ACTION_HAND] == chest && state.mouseThing == potion,
                "container reaches action hand while potion stays carried");
    ok &= check(dm1_v1_inventory_live_click_slot_pc34(&state, DM1_PC34_SLOT_READY_HAND),
                "potion can be returned before opening chest");
    ok &= check(dm1_v1_inventory_live_use_action_hand_pc34(&state, &champion, NULL, 0,
                                                             &useReceipt) &&
                useReceipt.kind == DM1_V1_INVENTORY_LIVE_USE_CHEST_PC34 &&
                state.openChestThing == chest,
                "F0349 container dispatch opens only a raw F0333 chest chain");
    ok &= check(dm1_v1_inventory_live_click_slot_pc34(&state, DM1_PC34_SLOT_CHEST_1) &&
                state.mouseThing == weapon0 &&
                dm1_v1_inventory_live_click_slot_pc34(&state, DM1_PC34_SLOT_BACKPACK_LINE2_2),
                "chest-to-backpack drag carries the original weapon Thing, not a copy");
    ok &= check(dm1_v1_inventory_live_close_chest_pc34(&state, &chestReceipt) && chestReceipt.valid &&
                container.slot == weapon1 && rawContainer[2] == 1 && rawContainer[3] == 0x14,
                "F0334 atomically rewrites raw and decoded remaining chest chain");

    memset(slots, 0xff, sizeof(slots));
    slots[DM1_PC34_SLOT_ACTION_HAND] = potion;
    champion.maximumHealth = champion.currentHealth = 100;
    champion.maximumStamina = champion.currentStamina = 100;
    ok &= check(dm1_v1_inventory_live_begin_pc34(&state, &things, slots) &&
                dm1_v1_inventory_live_use_action_hand_pc34(&state, &champion, NULL, 0, &useReceipt) &&
                useReceipt.kind == DM1_V1_INVENTORY_LIVE_USE_CONSUMABLE_PC34 &&
                (rawPotion[3] & 0x7f) == DM1_CONSUMABLE_POTION_EMPTY_FLASK_PC34,
                "potion use commits its real C08 type byte only after effects succeed");

    memset(slots, 0xff, sizeof(slots));
    slots[DM1_PC34_SLOT_ACTION_HAND] = scroll;
    ok &= check(dm1_v1_inventory_live_begin_pc34(&state, &things, slots) &&
                dm1_v1_inventory_live_use_action_hand_pc34(&state, &champion, NULL, 0, &useReceipt) &&
                useReceipt.kind == DM1_V1_INVENTORY_LIVE_USE_SCROLL_PC34 &&
                useReceipt.scrollTextIndex == 5 && useReceipt.iconIndex >= 0,
                "scroll read exposes only real C07 text/index material");

    memset(slots, 0xff, sizeof(slots));
    slots[DM1_PC34_SLOT_ACTION_HAND] = junk;
    champion.food = 0;
    ok &= check(dm1_v1_inventory_live_begin_pc34(&state, &things, slots) &&
                dm1_v1_inventory_live_use_action_hand_pc34(&state, &champion, NULL, 0, &useReceipt) &&
                champion.food > 0 && state.slots[DM1_PC34_SLOT_ACTION_HAND] == THING_NONE,
                "food consumption uses real C10 icon data and removes the Thing");

    if (!ok) return 1;
    puts("PASS: DM1 live C05-C10 inventory transaction");
    return 0;
}
