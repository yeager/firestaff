#include "csb_v1_viewport_d0l2_d0r2_f0111_f0115_route_receipt_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static void check_side(int side,
                       int expected_square,
                       unsigned int expected_thing_order,
                       unsigned int expected_rear_order,
                       unsigned int expected_front_order)
{
    CSB_V1_D0L2D0R2F0111F0115RouteReceiptPc34 receipt;

    CHECK(csb_v1_viewport_d0l2_d0r2_f0111_f0115_route_receipt_pc34(
              side, &receipt) == 1);
    CHECK(receipt.valid == 1);
    CHECK(receipt.side == side);
    CHECK(receipt.view_square_index == expected_square);
    CHECK(receipt.thing_pass_cell_order == expected_thing_order);
    CHECK(receipt.door_rear_cell_order == expected_rear_order);
    CHECK(receipt.door_front_cell_order == expected_front_order);
    CHECK(receipt.thing_pass_disabled_items_and_projectiles == 1);
    CHECK(receipt.door_draw_does_not_mutate_lists == 1);
    CHECK(receipt.c10_transparency_preserved == 1);
    CHECK(receipt.no_game_data_load == 1);
    CHECK(receipt.no_real_asset_bitmap_parity == 1);
    CHECK(receipt.no_synthetic_viewport_pixels == 1);
    CHECK(receipt.no_legacy_viewport_wrapper == 1);
    check_contains(receipt.source_evidence, "F0115 thing-pass order");
    check_contains(receipt.source_evidence, "F0111 door-front draw");
}

static void test_accepts_both_d0_side_routes(void)
{
    check_side(CSB_V1_D0L2_D0R2_F0115_SIDE_D0L2_PC34,
               1,
               0x0002u,
               0x0218u,
               0x0349u);
    check_side(CSB_V1_D0L2_D0R2_F0115_SIDE_D0R2_PC34,
               2,
               0x0001u,
               0x0128u,
               0x0439u);
}

static void test_rejects_invalid_side_and_preserves_null_output(void)
{
    CSB_V1_D0L2D0R2F0111F0115RouteReceiptPc34 receipt;

    memset(&receipt, 0x7f, sizeof(receipt));
    CHECK(csb_v1_viewport_d0l2_d0r2_f0111_f0115_route_receipt_pc34(
              0, &receipt) == 0);
    CHECK(receipt.valid == 0);
    CHECK(csb_v1_viewport_d0l2_d0r2_f0111_f0115_route_receipt_pc34(
              99, NULL) == 0);
}

static void test_underlying_contracts_remain_source_bounded(void)
{
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *left_thing =
        csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(
            CSB_V1_D0L2_D0R2_F0115_SIDE_D0L2_PC34);
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *left_door =
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_for_side_pc34(
            CSB_V1_D0L2_D0R2_F0111_SIDE_D0L2_PC34);

    CHECK(left_thing != NULL);
    CHECK(left_door != NULL);
    CHECK(csb_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(
              left_thing, left_thing->f0115_first_cell) == -1);
    CHECK(csb_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(
              left_thing, left_thing->f0115_first_cell) == -1);
    CHECK(csb_v1_viewport_d0l2_d0r2_f0111_door_front_is_draw_mutating_pc34(
              left_door) == 0);

    CHECK(left_thing->source_locked_contract_only == 1);
    CHECK(left_thing->no_real_asset_bitmap_parity == 1);
    CHECK(left_thing->no_game_data_load == 1);
}

static void test_evidence_string(void)
{
    const char *evidence =
        csb_v1_viewport_d0l2_d0r2_f0111_f0115_route_receipt_source_evidence_pc34();

    check_contains(evidence, "F0125/F0126/F0128");
    check_contains(evidence, "C10 transparency");
    check_contains(evidence, "creates no synthetic viewport pixels");
}

int main(void)
{
    test_accepts_both_d0_side_routes();
    test_rejects_invalid_side_and_preserves_null_output();
    test_underlying_contracts_remain_source_bounded();
    test_evidence_string();
    return 0;
}
