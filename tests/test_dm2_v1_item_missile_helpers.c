#include "dm2_v1_item_missile_helpers.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static void test_item_hand_activable(void)
{
    DM2_V1_ItemMissileReceipt receipt;

    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(0x1234u, 0x0001u,
                                              &receipt) == 1,
                "IS_ITEM_HAND_ACTIVABLE accepts dbspec mask");
    expect_true(receipt.handled && receipt.source_locked && receipt.valid &&
                    receipt.result == 1 && !receipt.blocked &&
                    strcmp(receipt.symbol, "IS_ITEM_HAND_ACTIVABLE") == 0,
                "hand activable receipt records positive predicate");

    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(0x1234u, 0x0000u,
                                              &receipt) == 0,
                "IS_ITEM_HAND_ACTIVABLE rejects clear dbspec mask");
    expect_true(receipt.valid && receipt.result == 0 && !receipt.blocked,
                "clear dbspec mask is a bounded negative result");

    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(
                    DM2_V1_ITEM_MISSILE_NULL_REF, 0x0001u, &receipt) == 0,
                "IS_ITEM_HAND_ACTIVABLE blocks null object");
    expect_true(receipt.handled && receipt.blocked && !receipt.valid,
                "null hand object is fail-closed");
}

static void test_retrieve_item_bonus(void)
{
    DM2_V1_ItemBonusFacts facts = {0x2001u, 0x0001u, 12, -5};
    DM2_V1_ItemMissileReceipt receipt;

    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == 7,
                "RETRIEVE_ITEM_BONUS combines base and runtime delta");
    expect_true(receipt.valid && receipt.result == 7 &&
                    strcmp(receipt.symbol, "RETRIEVE_ITEM_BONUS") == 0,
                "item bonus receipt records source symbol");

    facts.base_bonus = 32760;
    facts.runtime_delta = 99;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == 32767,
                "RETRIEVE_ITEM_BONUS clamps positive overflow");

    facts.item_ref = DM2_V1_ITEM_MISSILE_NULL_REF;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == 0,
                "RETRIEVE_ITEM_BONUS blocks null object");
    expect_true(receipt.blocked && !receipt.valid,
                "null bonus facts are fail-closed");
}

static void test_get_missile_ref_of_minion(void)
{
    uint16_t refs[] = {0x1111u, 0x2222u, DM2_V1_ITEM_MISSILE_NULL_REF};
    DM2_V1_ItemMissileReceipt receipt;

    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(refs, 3, 1,
                                                 &receipt) == 0x2222u,
                "GET_MISSILE_REF_OF_MINION returns selected slot");
    expect_true(receipt.valid && receipt.result == 0x2222 &&
                    strcmp(receipt.symbol,
                           "GET_MISSILE_REF_OF_MINION") == 0,
                "minion missile receipt records selected ref");

    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(refs, 3, 2, &receipt) ==
                    DM2_V1_ITEM_MISSILE_NULL_REF,
                "GET_MISSILE_REF_OF_MINION preserves null missile ref");
    expect_true(receipt.valid && receipt.blocked,
                "null minion missile is a blocked receipt");

    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(refs, 3, 9, &receipt) ==
                    DM2_V1_ITEM_MISSILE_NULL_REF,
                "GET_MISSILE_REF_OF_MINION blocks out-of-range slot");
    expect_true(receipt.blocked && !receipt.valid,
                "out-of-range minion slot is fail-closed");
}

static void test_missile_valid_to_launcher(void)
{
    DM2_V1_MissileLauncherFacts facts = {0x3001u, 0x4001u, 1, 4, 4};
    DM2_V1_ItemMissileReceipt receipt;

    expect_true(dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(&facts, &receipt) == 1,
                "IS_MISSILE_VALID_TO_LAUNCHER accepts matching class");
    expect_true(receipt.valid && receipt.result == 1 &&
                    strcmp(receipt.symbol,
                           "IS_MISSILE_VALID_TO_LAUNCHER") == 0,
                "launcher predicate receipt records source symbol");

    facts.missile_class = 5;
    expect_true(dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(&facts, &receipt) == 0,
                "IS_MISSILE_VALID_TO_LAUNCHER rejects mismatched class");
    expect_true(receipt.valid && receipt.blocked,
                "mismatched missile class blocks launch");

    facts.missile_class = 4;
    facts.launcher_ref = DM2_V1_ITEM_MISSILE_NULL_REF;
    expect_true(dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(&facts, &receipt) == 0,
                "IS_MISSILE_VALID_TO_LAUNCHER blocks null launcher");
    expect_true(receipt.blocked && !receipt.valid,
                "null launcher is fail-closed");
}

int main(void)
{
    test_item_hand_activable();
    test_retrieve_item_bonus();
    test_get_missile_ref_of_minion();
    test_missile_valid_to_launcher();
    expect_true(strstr(dm2_v1_item_missile_helpers_source_evidence(),
                       "IS_ITEM_HAND_ACTIVABLE:8423") != 0,
                "source evidence includes hand activable symbol");
    expect_true(strstr(dm2_v1_item_missile_helpers_source_evidence(),
                       "GET_MISSILE_REF_OF_MINION:8205") != 0,
                "source evidence includes minion missile symbol");
    if (failures) {
        return 1;
    }
    puts("DM2 item/missile helpers: ok");
    return 0;
}
