/* ReDMCSB TIMELINE.C F0252 + MOVE.C F0266 atomic C04 relocation. */
#include "csb_v1_runtime_pc34_compat.h"

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

static unsigned short read_le16(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

static void seed_fixture(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_DungeonData *dungeon,
                         unsigned char *raw)
{
    const unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 160u);
    dungeon->raw_data = raw;
    dungeon->raw_size = 160;
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 2;
    dungeon->level_heights[0] = 1;
    dungeon->square_bytes = 1;
    dungeon->square_first_thing_base = 64;
    dungeon->square_first_thing_count = 2;
    dungeon->thing_data_bases[THING_TYPE_GROUP] = 80;
    dungeon->thing_type_counts[THING_TYPE_GROUP] = 1;
    raw[0] = 0x30u; /* Source C01 corridor. */
    raw[1] = 0x50u; /* Open C02 destination with an explicit Thing slot. */
    put_le16(raw, 60, 0u);
    put_le16(raw, 62, 1u);
    put_le16(raw, 64, group);
    put_le16(raw, 66, THING_ENDOFLIST);
    put_le16(raw, 80, THING_ENDOFLIST);
    raw[84] = 3u;  /* Existing source CreatureInfo fixture. */
    raw[85] = 0xffu;  /* Single creature occupies all F0176 cell probes. */
    put_le16(raw, 86, 40u);
    put_le16(raw, 94, 0u); /* one creature, direction north */
    csb_v1_runtime_init(profile, NULL);
    profile->dungeon_handle = dungeon;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0252F0266GroupMoveTransactionReceiptPc34 receipt;
    struct DM1_DispatchRecord_V1 record;
    CSB_V1_F0252GroupMoveReceiptPc34 move;
    CSB_V1_F0266GroupMoveProjectileReceiptPc34 projectile;
    CSB_V1_F0176CreatureOrdinalReceiptPc34 ordinal;
    unsigned char raw[160];
    unsigned char before[160];
    const unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);

    seed_fixture(&profile, &dungeon, raw);
    memset(&record, 0, sizeof(record));
    record.eventType = DM1_EVENT_MOVE_GROUP_SILENT;
    record.mapIndex = 0;
    record.mapX = 1;
    record.mapY = 0;
    record.cell = group & 0xffu;
    record.effect = group >> 8;

    CHECK(csb_v1_runtime_f0252_group_move_receipt_pc34(&profile, &record, &move) == 1,
          "fixture exposes a live C60 C04 timer receipt");
    CHECK(csb_v1_runtime_f0176_creature_ordinal_receipt_pc34(
              &dungeon, group, 0, 0, 0, 0, &ordinal) == 1,
          "fixture exposes F0176 raw C04 cell ownership");
    CHECK(csb_v1_runtime_f0266_group_move_projectile_receipt_pc34(
              &dungeon, group, 0, 0, 0, 1, 0, &projectile) == 1,
          "fixture exposes F0266 C04/C14 movement material");

    CHECK(csb_v1_runtime_f0252_f0266_group_move_transaction_pc34(
              &profile, &record, &receipt) == 1,
          "F0252/F0266 admits a linked C04 plus its adjacent C14 census");
    CHECK(receipt.valid && receipt.committed && !receipt.retry_scheduled &&
              receipt.move.valid && receipt.projectile.valid &&
              receipt.raw_dungeon_fnv1a_before != 0u &&
              receipt.raw_dungeon_fnv1a_after != 0u,
          "transaction retains raw C04, C14 and timeline ownership receipts");
    CHECK(read_le16(raw + 64) == THING_ENDOFLIST &&
              read_le16(raw + 66) == group,
          "committed transaction moves the group through source Thing chains");

    seed_fixture(&profile, &dungeon, raw);
    memcpy(before, raw, sizeof(raw));
    raw[66] = 0u; /* Corrupt only the target square-first record. */
    CHECK(csb_v1_runtime_f0252_f0266_group_move_transaction_pc34(
              &profile, &record, &receipt) == 0 && !receipt.valid,
          "malformed destination rejects before any source C04 mutation");
    CHECK(read_le16(raw + 64) == group &&
              memcmp(raw, before, 64u) == 0,
          "rejected transaction preserves the original source square chain");

    printf("csb F0252/F0266 group-move transaction: %s\n",
           failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
