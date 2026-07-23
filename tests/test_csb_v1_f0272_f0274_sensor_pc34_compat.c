#include "csb_v1_f0272_f0274_sensor_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\\n", message); } \
    else { ++failed; printf("  FAIL: %s\\n", message); } \
} while (0)

enum { kObjectType = 27, kSensorOffset = 68, kWeaponOffset = 80 };

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static unsigned short sensor_target(int x, int y, int cell)
{
    return (unsigned short)(((cell & 3) << 4) | ((x & 0x1f) << 6) |
                            ((y & 0x1f) << 11));
}

static void make_fixture(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_DungeonData *dungeon,
                         unsigned char raw[128])
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, (unsigned char)(1u << 5), 128u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 128;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = kSensorOffset;
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[THING_TYPE_WEAPON] = kWeaponOffset;
    dungeon->thing_type_counts[THING_TYPE_WEAPON] = 1;
    raw[1] |= 0x10u;
    raw[6] = (unsigned char)(6u << 5); /* target fakewall at (2,0). */
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, kSensorOffset, 0xfffeu);
    put_le16(raw, kSensorOffset + 2, (unsigned short)((kObjectType << 7) | 8u));
    put_le16(raw, kSensorOffset + 4, (unsigned short)((3u << 7) | (DM1_EFFECT_SET << 3)));
    put_le16(raw, kSensorOffset + 6, sensor_target(2, 0, 3));
    put_le16(raw, kWeaponOffset, 0xfffeu);
    put_le16(raw, kWeaponOffset + 2, kObjectType);
    csb_v1_runtime_init(profile, NULL);
    profile->dungeon_handle = dungeon;
    profile->current_level = 0;
    profile->game_time = 11;
    profile->party_state_valid = 1;
    profile->party_state.ChampionCount = 1;
    profile->party_state.Champions[0].CurrentHealth = 100;
    profile->party_state.LeaderHandThing = (unsigned short)(THING_TYPE_WEAPON << 10);
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0272F0274ReceiptPc34 receipt;
    unsigned char raw[128];

    make_fixture(&profile, &dungeon, raw);
    CHECK(csb_v1_f0274_party_possession_receipt_pc34(&profile, kObjectType, &receipt) == 1 &&
              receipt.valid && receipt.possession_found,
          "F0274 admits the loaded PC34 leader-hand object");
    CHECK(csb_v1_f0274_party_possession_receipt_pc34(&profile, kObjectType + 1, &receipt) == 1 &&
              receipt.valid && !receipt.possession_found,
          "F0274 rejects an absent object type without a fallback");
    CHECK(csb_v1_f0272_trigger_remote_effect_pc34(
              &profile, (unsigned short)(3u << 10), DM1_EFFECT_SET, 0, 1, &receipt) == 1 &&
              receipt.valid && receipt.target_x == 2 && receipt.target_y == 0 &&
              receipt.target_cell == 0 && receipt.event_type == DM1_EVENT_FAKEWALL &&
              receipt.event_time == 14 && profile.timeline_queue.eventCount == 1,
          "F0272 reads raw C03 target, wall/fakewall cell rule, and Value delay");
    put_le16(raw, kSensorOffset + 4, (unsigned short)(0x0800u | (DM1_EFFECT_SET << 3)));
    CHECK(csb_v1_f0272_trigger_remote_effect_pc34(
              &profile, (unsigned short)(3u << 10), DM1_EFFECT_SET, 0, 1, &receipt) == 0 &&
              profile.timeline_queue.eventCount == 1,
          "F0272 leaves LocalEffect to the separate F0270/F0271 owner");
    printf("PASSED: %d\\nFAILED: %d\\n", passed, failed);
    return failed ? 1 : 0;
}
