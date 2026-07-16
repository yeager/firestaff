#include "dm2_v1_object_transfer_helpers.h"

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

static void test_remove_possession(void)
{
    DM2_V1_ObjectTransferLink links[] = {
        {0x1001u, 0x1002u},
        {0x1002u, 0x1003u},
        {0x1003u, DM2_V1_OBJECT_TRANSFER_NULL}
    };
    DM2_V1_ObjectTransferReceipt receipt;
    uint16_t new_head;
    uint16_t removed_next;

    expect_true(dm2_v1_REMOVE_POSSESSION(links, 3u, 0x1001u, 0x1002u,
                                         &new_head, &removed_next,
                                         &receipt) == 1,
                "REMOVE_POSSESSION removes middle chain object");
    expect_true(new_head == 0x1001u && removed_next == 0x1003u &&
                    receipt.valid && receipt.mutated &&
                    receipt.previous_ref == 0x1001u &&
                    strcmp(receipt.symbol, "REMOVE_POSSESSION") == 0,
                "REMOVE_POSSESSION receipt records relink facts");

    expect_true(dm2_v1_REMOVE_POSSESSION(links, 3u, 0x1001u, 0x1001u,
                                         &new_head, &removed_next,
                                         &receipt) == 1,
                "REMOVE_POSSESSION removes head object");
    expect_true(new_head == 0x1002u && removed_next == 0x1002u,
                "REMOVE_POSSESSION advances head to next object");

    expect_true(dm2_v1_REMOVE_POSSESSION(links, 3u, 0x1001u, 0x9999u,
                                         &new_head, &removed_next,
                                         &receipt) == 0,
                "REMOVE_POSSESSION blocks absent object");
    expect_true(receipt.blocked && !receipt.valid,
                "absent possession removal is fail-closed");
}

static void test_put_object_into_container(void)
{
    DM2_V1_ObjectTransferLink links[] = {
        {0x2001u, 0x2002u},
        {0x2002u, DM2_V1_OBJECT_TRANSFER_NULL}
    };
    DM2_V1_ObjectTransferReceipt receipt;
    uint16_t new_head;
    uint16_t previous_tail;

    expect_true(dm2_v1_PUT_OBJECT_INTO_CONTAINER(
                    links, 2u, 0x3000u, 0x2001u, 0x2222u, &new_head,
                    &previous_tail, &receipt) == 1,
                "PUT_OBJECT_INTO_CONTAINER appends to non-empty chain");
    expect_true(new_head == 0x2001u && previous_tail == 0x2002u &&
                    receipt.valid && receipt.previous_ref == 0x2002u &&
                    strcmp(receipt.symbol,
                           "PUT_OBJECT_INTO_CONTAINER") == 0,
                "container append receipt records tail and head");

    expect_true(dm2_v1_PUT_OBJECT_INTO_CONTAINER(
                    links, 0u, 0x3000u, DM2_V1_OBJECT_TRANSFER_NULL, 0x2222u,
                    &new_head, &previous_tail, &receipt) == 1,
                "PUT_OBJECT_INTO_CONTAINER inserts into empty container");
    expect_true(new_head == 0x2222u &&
                    previous_tail == DM2_V1_OBJECT_TRANSFER_NULL,
                "empty container head becomes inserted object");

    expect_true(dm2_v1_PUT_OBJECT_INTO_CONTAINER(
                    links, 2u, 0x3000u, 0x2001u, 0x2002u, &new_head,
                    &previous_tail, &receipt) == 0,
                "PUT_OBJECT_INTO_CONTAINER blocks duplicate chain object");
    expect_true(receipt.blocked && !receipt.valid,
                "duplicate container insertion is fail-closed");
}

static void test_load_projectile_to_hand(void)
{
    DM2_V1_LoadProjectileToHandInput input = {
        0x4400u,
        DM2_V1_OBJECT_TRANSFER_NULL,
        0x4400u,
        0x4401u
    };
    DM2_V1_ObjectTransferReceipt receipt;
    uint16_t hand_ref;
    uint16_t source_head;

    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &hand_ref,
                                               &source_head,
                                               &receipt) == 1,
                "LOAD_PROJECTILE_TO_HAND moves projectile into empty hand");
    expect_true(hand_ref == 0x4400u && source_head == 0x4401u &&
                    receipt.valid && receipt.hand_ref == 0x4400u &&
                    strcmp(receipt.symbol, "LOAD_PROJECTILE_TO_HAND") == 0,
                "projectile hand receipt records source head update");

    input.hand_ref = 0x7777u;
    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &hand_ref,
                                               &source_head,
                                               &receipt) == 0,
                "LOAD_PROJECTILE_TO_HAND blocks occupied hand");
    expect_true(receipt.blocked && !receipt.valid,
                "occupied hand load is fail-closed");
}

int main(void)
{
    test_remove_possession();
    test_put_object_into_container();
    test_load_projectile_to_hand();
    expect_true(strstr(dm2_v1_object_transfer_helpers_source_evidence(),
                       "REMOVE_POSSESSION:5430") != 0,
                "source evidence includes removal symbol");
    expect_true(strstr(dm2_v1_object_transfer_helpers_source_evidence(),
                       "LOAD_PROJECTILE_TO_HAND:5543") != 0,
                "source evidence includes projectile symbol");
    if (failures) {
        return 1;
    }
    puts("DM2 object transfer helpers: ok");
    return 0;
}
