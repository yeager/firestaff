#include "csb_v1_f0267_move_result_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failed;

#define CHECK(condition, message) do { \
    if (condition) printf("  PASS: %s\n", message); \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

int main(void)
{
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0267MoveResultReceiptPc34 receipt;
    unsigned char raw[112];
    unsigned short weapon = (unsigned short)(THING_TYPE_WEAPON << 10);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, 0, sizeof(raw));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 2;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 64;
    dungeon.square_first_thing_count = 2;
    dungeon.thing_data_bases[THING_TYPE_WEAPON] = 80;
    dungeon.thing_type_counts[THING_TYPE_WEAPON] = 1;
    raw[0] = 0x30u;
    raw[1] = 0x30u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 62, 1u);
    put_le16(raw, 64, weapon);
    put_le16(raw, 66, 0xFFFEu);
    put_le16(raw, 80, 0xFFFEu);

    CHECK(csb_v1_f0267_move_result_receipt_pc34(
              &dungeon, CSB_V1_F0267_VARIANT_CPSCE_GET_MOVE_RESULT_PC34,
              weapon, 0, 0, 0, 0, 1, 0, &receipt) == 1,
          "CPSCE admits a linked raw Thing moving between loaded squares");
    CHECK(receipt.valid && receipt.source_mode ==
              CSB_V1_F0267_SOURCE_ON_SQUARE_PC34 &&
              receipt.destination_mode == CSB_V1_F0267_DESTINATION_ON_SQUARE_PC34 &&
              receipt.thing_record_fnv1a != 0u,
          "CPSCE receipt preserves record identity and both source/destination modes");

    CHECK(csb_v1_f0267_move_result_receipt_pc34(
              &dungeon, CSB_V1_F0267_VARIANT_CPSCE_GET_MOVE_RESULT_PC34,
              weapon, 0, 0, 0, 0, -1, 0, &receipt) == 1 &&
              receipt.destination_mode == CSB_V1_F0267_DESTINATION_REMOVE_PC34,
          "CPSCE admits a raw linked Thing removal without a synthetic target");
    CHECK(csb_v1_f0267_move_result_receipt_pc34(
              &dungeon, CSB_V1_F0267_VARIANT_CPSCE_GET_MOVE_RESULT_PC34,
              weapon, 0, -2, 0, 0, 1, 0, &receipt) == 1 &&
              receipt.source_mode ==
                  CSB_V1_F0267_SOURCE_PROJECTILE_ASSOCIATED_PC34,
          "CPSCE keeps the projectile-associated placement mode explicit");
    CHECK(csb_v1_f0267_move_result_receipt_pc34(
              &dungeon, CSB_V1_F0267_VARIANT_IIGS_GET_MOVE_RES_PC34,
              weapon, 0, 0, 0, 0, 1, 0, &receipt) == 0 && !receipt.valid,
          "IIGS-only GetMoveRes fails closed without a source body");

    put_le16(raw, 64, 0xFFFEu);
    CHECK(csb_v1_f0267_move_result_receipt_pc34(
              &dungeon, CSB_V1_F0267_VARIANT_CPSCE_GET_MOVE_RESULT_PC34,
              weapon, 0, 0, 0, 0, 1, 0, &receipt) == 0 && !receipt.valid,
          "CPSCE rejects a detached raw Thing");

    printf("csb F0267 move-result receipt: %s\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
