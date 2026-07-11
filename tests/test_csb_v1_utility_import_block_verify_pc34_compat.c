/*
 * Focused CSB V1 utility-import regression.
 *
 * Source lock:
 *   ReDMCSB SAVEGAME.C F0100-F0120 keeps champion validation before the
 *   store-party step, so a malformed converted champion block must leave the
 *   import UI in ERROR rather than presenting a partially imported party.
 */

#include "csb_v1_utility_import_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(got, want, label) do { \
    if ((got) == (want)) { \
        passed++; printf("  PASS: %s == %d\n", label, (int)(want)); \
    } else { \
        failed++; printf("  FAIL: %s got=%d want=%d\n", label, (int)(got), (int)(want)); \
    } \
} while (0)

static void put_le16(uint8_t *p, int value)
{
    p[0] = (uint8_t)(value & 0xff);
    p[1] = (uint8_t)((value >> 8) & 0xff);
}

static void seed_empty_slots(uint8_t *record)
{
    int i;
    for (i = 0; i < 30; i++) {
        put_le16(record + 40 + i * 2, 0xffff);
    }
}

static void build_blank_name_dm1_save(uint8_t *buf, size_t size)
{
    uint8_t *record;

    memset(buf, 0, size);
    buf[0] = 1u;

    record = buf + DM1_SAVE_HEADER_SIZE;
    memset(record, ' ', 8);
    put_le16(record + 8, 80);
    put_le16(record + 10, 100);
    put_le16(record + 12, 70);
    put_le16(record + 14, 100);
    put_le16(record + 16, 30);
    put_le16(record + 18, 50);
    record[20] = 60;
    record[21] = 61;
    record[22] = 62;
    record[23] = 63;
    seed_empty_slots(record);
}

static void build_valid_dm1_record(uint8_t *record, const char *name)
{
    memset(record, 0, DM1_CHAMPION_RECORD_SIZE);
    snprintf((char *)record, 8, "%s", name);
    put_le16(record + 8, 80);
    put_le16(record + 10, 100);
    put_le16(record + 12, 70);
    put_le16(record + 14, 100);
    put_le16(record + 16, 30);
    put_le16(record + 18, 50);
    record[20] = 60;
    record[21] = 61;
    record[22] = 62;
    record[23] = 63;
    memset(record + 40, 0, 60);
}

static void seed_existing_party(CSB_V1_PartyState *party)
{
    memset(party, 0x7a, sizeof(*party));
    party->ChampionCount = 3;
    party->LeaderIndex = 2;
    memcpy(party->Champions[0].Name, "LIVE", 5);
}

static void test_blank_name_block_is_rejected_before_party_store(void)
{
    uint8_t buf[DM1_SAVE_HEADER_SIZE + DM1_CHAMPION_RECORD_SIZE];
    CSB_V1_PartyState party;
    CSB_V1_PartyState party_before;
    CSB_V1_ImportResult result;
    int imported;

    build_blank_name_dm1_save(buf, sizeof(buf));
    seed_existing_party(&party);
    party_before = party;
    memset(&result, 0, sizeof(result));

    imported = csb_v1_import_from_dm1_save_buffer(&party, buf, (int)sizeof(buf), &result);

    CHECK_EQ(imported, -1, "blank-name import return");
    CHECK_EQ(result.state, CSB_V1_IMPORT_STATE_ERROR, "blank-name import terminal state");
    CHECK_EQ(result.error_code, CSB_V1_IMPORT_ERR_CHECKSUM, "blank-name import error code");
    CHECK_EQ(result.byte_offset, DM1_SAVE_HEADER_SIZE, "blank-name import error offset");
    CHECK_EQ(result.champion_count, 0, "blank-name result champion_count");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "blank-name leaves live party untouched");
}

static void test_later_invalid_record_leaves_live_party_untouched(void)
{
    uint8_t buf[DM1_SAVE_HEADER_SIZE + 2 * DM1_CHAMPION_RECORD_SIZE];
    CSB_V1_PartyState party;
    CSB_V1_PartyState party_before;
    CSB_V1_ImportResult result;
    int imported;

    memset(buf, 0, sizeof(buf));
    buf[0] = 2u;
    build_valid_dm1_record(buf + DM1_SAVE_HEADER_SIZE, "FIRST");
    build_valid_dm1_record(buf + DM1_SAVE_HEADER_SIZE + DM1_CHAMPION_RECORD_SIZE,
                           "SECOND");
    memset(buf + DM1_SAVE_HEADER_SIZE + DM1_CHAMPION_RECORD_SIZE, ' ', 8);
    seed_existing_party(&party);
    party_before = party;

    imported = csb_v1_import_from_dm1_save_buffer(&party, buf, (int)sizeof(buf), &result);

    CHECK_EQ(imported, -1, "later-invalid import return");
    CHECK_EQ(result.state, CSB_V1_IMPORT_STATE_ERROR, "later-invalid terminal state");
    CHECK_EQ(result.error_code, CSB_V1_IMPORT_ERR_CHECKSUM, "later-invalid error code");
    CHECK_EQ(result.byte_offset, DM1_SAVE_HEADER_SIZE + DM1_CHAMPION_RECORD_SIZE,
             "later-invalid error offset");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "later-invalid leaves live party untouched");
}

static void test_truncated_later_record_leaves_live_party_untouched(void)
{
    uint8_t buf[DM1_SAVE_HEADER_SIZE + DM1_CHAMPION_RECORD_SIZE];
    CSB_V1_PartyState party;
    CSB_V1_PartyState party_before;
    CSB_V1_ImportResult result;
    int imported;

    memset(buf, 0, sizeof(buf));
    buf[0] = 2u;
    build_valid_dm1_record(buf + DM1_SAVE_HEADER_SIZE, "FIRST");
    seed_existing_party(&party);
    party_before = party;

    imported = csb_v1_import_from_dm1_save_buffer(&party, buf, (int)sizeof(buf), &result);

    CHECK_EQ(imported, -1, "truncated import return");
    CHECK_EQ(result.state, CSB_V1_IMPORT_STATE_ERROR, "truncated terminal state");
    CHECK_EQ(result.error_code, CSB_V1_IMPORT_ERR_PARTIAL, "truncated error code");
    CHECK_EQ(result.byte_offset, DM1_SAVE_HEADER_SIZE + DM1_CHAMPION_RECORD_SIZE,
             "truncated error offset");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "truncated import leaves live party untouched");
}

int main(void)
{
    printf("=== CSB V1 Utility Import Block Verification Regression ===\n\n");

    test_blank_name_block_is_rejected_before_party_store();
    test_later_invalid_record_leaves_live_party_untouched();
    test_truncated_later_record_leaves_live_party_untouched();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
