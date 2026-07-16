#include "dm1_v1_melee_action_f0402_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *label)
{
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", label);
    }
}

static void check_eq_int(int actual, int expected, const char *label)
{
    if (actual != expected) {
        ++failures;
        fprintf(stderr, "FAIL: %s (got %d expected %d)\n",
                label, actual, expected);
    }
}

static void check_eq_ushort(unsigned short actual, unsigned short expected,
                            const char *label)
{
    if (actual != expected) {
        ++failures;
        fprintf(stderr, "FAIL: %s (got 0x%04x expected 0x%04x)\n",
                label, actual, expected);
    }
}

static void check_symbol(const char *actual, const char *expected,
                         const char *label)
{
    check(actual && strcmp(actual, expected) == 0, label);
}

int main(void)
{
    struct RngState_Compat rng;
    unsigned char moving_cells[3];
    DM1_MeleeF0186FixedPossessionReceiptPc34 fixed_receipt;
    DM1_MeleeF0187MovingFixedPossessionReceiptPc34 moving_receipt;
    DM1_MeleeF0188GroupSlotDropInputPc34 slot_in;
    DM1_MeleeF0188GroupPossessionReceiptPc34 slot_receipt;
    DM1_MeleeF0190KilledAllStateInputPc34 delete_in;
    DM1_MeleeF0190KilledAllStatePlanPc34 delete_plan;
    DM1_MeleeF0189DeleteGroupReceiptPc34 delete_receipt;

    rng.seed = 1u;
    check(dm1_v1_melee_drop_creature_fixed_possessions_receipt_f0186_pc34(
              DM1_CREATURE_TYPE_ANIMATED_ARMOUR, 2, &rng,
              &fixed_receipt) == 1,
          "F0186 fixed possession receipt builds");
    check_eq_int(fixed_receipt.valid, 1, "F0186 receipt valid");
    check_symbol(fixed_receipt.sourceSymbol,
                 "F0186_GROUP_DropCreatureFixedPossessions",
                 "F0186 receipt is source named");
    check_eq_int(fixed_receipt.sourceLineStart, 580,
                 "F0186 line start");
    check_eq_int(fixed_receipt.sourceLineEnd, 645,
                 "F0186 line end");
    check_eq_int(fixed_receipt.creatureType,
                 DM1_CREATURE_TYPE_ANIMATED_ARMOUR,
                 "F0186 keeps raw C04 creature type");
    check_eq_int(fixed_receipt.sourceCell, 2,
                 "F0186 keeps raw creature cell");
    check_eq_int(fixed_receipt.dropCount, 6,
                 "F0186 animated armour fixed drop count");
    check_eq_int(fixed_receipt.weaponDropped, 1,
                 "F0186 records weapon thud");
    check_eq_int(fixed_receipt.drops[0].thingType,
                 DM1_DROP_THING_TYPE_ARMOUR,
                 "F0186 first drop thing type");
    check_eq_int(fixed_receipt.drops[0].itemType, 41,
                 "F0186 first drop item type");
    check_eq_int(fixed_receipt.drops[3].thingType,
                 DM1_DROP_THING_TYPE_WEAPON,
                 "F0186 fourth drop is weapon");
    check_eq_int(fixed_receipt.drops[5].sourceOrdinal, 6,
                 "F0186 preserves source table ordinal");

    moving_cells[0] = 1u;
    moving_cells[1] = 2u;
    moving_cells[2] = 3u;
    check(dm1_v1_melee_drop_moving_creature_fixed_possessions_receipt_f0187_pc34(
              moving_cells, 3, &moving_receipt) == 1,
          "F0187 moving fixed possession receipt builds");
    check_symbol(moving_receipt.sourceSymbol,
                 "F0187_GROUP_DropMovingCreatureFixedPossessions",
                 "F0187 receipt is source named");
    check_eq_int(moving_receipt.dropCellCount, 3,
                 "F0187 moving stack count");
    check_eq_int(moving_receipt.dropCells[0], 3,
                 "F0187 consumes moving cells as stack first");
    check_eq_int(moving_receipt.dropCells[2], 1,
                 "F0187 consumes moving cells as stack last");

    memset(&slot_in, 0, sizeof(slot_in));
    slot_in.slotHead = (unsigned short)((THING_TYPE_WEAPON << 10) | 7);
    slot_in.chainEntryCount = 3;
    slot_in.chain[0].thing =
        (unsigned short)((THING_TYPE_WEAPON << 10) | 7);
    slot_in.chain[0].nextThing =
        (unsigned short)((THING_TYPE_JUNK << 10) | 2);
    slot_in.chain[1].thing =
        (unsigned short)((THING_TYPE_JUNK << 10) | 2);
    slot_in.chain[1].nextThing =
        (unsigned short)((THING_TYPE_ARMOUR << 10) | 5);
    slot_in.chain[2].thing =
        (unsigned short)((THING_TYPE_ARMOUR << 10) | 5);
    slot_in.chain[2].nextThing = THING_ENDOFLIST;
    slot_in.randomCellCount = 3;
    slot_in.randomCells[0] = 3u;
    slot_in.randomCells[1] = 1u;
    slot_in.randomCells[2] = 2u;
    check(dm1_v1_melee_drop_group_possessions_receipt_f0188_pc34(
              &slot_in, &slot_receipt) == 1,
          "F0188 group possession receipt builds");
    check_symbol(slot_receipt.sourceSymbol,
                 "F0188_GROUP_DropGroupPossessions",
                 "F0188 receipt is source named");
    check_eq_int(slot_receipt.shouldDrop, 1,
                 "F0188 drops non-empty group slot");
    check_eq_int(slot_receipt.stepCount, 3,
                 "F0188 walks group slot chain");
    check_eq_int(slot_receipt.weaponDropped, 1,
                 "F0188 records weapon in chain");
    check_eq_int(slot_receipt.soundId, 0,
                 "F0188 chooses metallic drop sound");
    check_eq_ushort(slot_receipt.steps[0].sourceThing,
                    slot_in.chain[0].thing,
                    "F0188 first source thing");
    check_eq_int((int)THING_GET_CELL(slot_receipt.steps[0].droppedThing),
                 3, "F0188 first random drop cell");
    check_eq_ushort(slot_receipt.steps[1].nextThing,
                    slot_in.chain[2].thing,
                    "F0188 snapshots next thing before relink");
    check_eq_int((int)THING_GET_CELL(slot_receipt.steps[2].droppedThing),
                 2, "F0188 third random drop cell");

    memset(&delete_in, 0, sizeof(delete_in));
    delete_in.outcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    delete_in.groupIndex = 9;
    delete_in.targetMapIndex = 4;
    delete_in.targetMapX = 12;
    delete_in.targetMapY = 6;
    check(dm1_v1_melee_killed_all_state_plan_f0190_pc34(
              &delete_in, &delete_plan) == 1,
          "F0190 killed-all plan builds for F0189 receipt");
    check(dm1_v1_melee_delete_group_receipt_f0189_pc34(
              &delete_plan, &delete_receipt) == 1,
          "F0189 delete receipt builds");
    check_symbol(delete_receipt.sourceSymbol, "F0189_GROUP_Delete",
                 "F0189 receipt is source named");
    check_eq_int(delete_receipt.shouldUnlinkGroupFromSquare, 1,
                 "F0189 unlinks group thing");
    check_eq_int(delete_receipt.shouldClearGroupNext, 1,
                 "F0189 clears raw C04 next link");
    check_eq_int(delete_receipt.shouldRemoveActiveGroupState, 1,
                 "F0189 retires active state");
    check_eq_int(delete_receipt.shouldDeleteGroupEvents, 1,
                 "F0189 deletes pending C29-C41 events");
    check_eq_ushort(delete_receipt.groupThing,
                    (unsigned short)((THING_TYPE_GROUP << 10) | 9),
                    "F0189 carries group Thing");
    check_eq_ushort(delete_receipt.clearedNextThing, THING_NONE,
                    "F0189 clear value is THING_NONE");

    if (failures != 0) return 1;
    puts("PASS: DM1 F0186-F0189 group drop/delete receipts");
    return 0;
}
