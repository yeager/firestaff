#include "csb_v1_f0276_sensor_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;
#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\n", message); } \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)
enum { SENSOR_BASE = 68 };
static void write_u16(uint8_t *bytes, int offset, uint16_t value) { bytes[offset] = (uint8_t)value; bytes[offset + 1] = (uint8_t)(value >> 8); }
static uint16_t sensor_thing(int index) { return (uint16_t)((CSB_V1_THING_TYPE_ACTUATOR << 10) | index); }
static void fixture(CSB_V1_RuntimeProfile *profile, CSB_V1_DungeonData *dungeon, uint8_t raw[160], int sensor_type, int sensor_data)
{
    memset(dungeon, 0, sizeof(*dungeon)); memset(raw, 0, 160);
    dungeon->level_count = 1; dungeon->level_widths[0] = 3; dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1; dungeon->raw_data = raw; dungeon->raw_size = 160;
    dungeon->square_first_thing_base = 66; dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = SENSOR_BASE;
    dungeon->thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    raw[1] = 0x30; write_u16(raw, 60, 0); write_u16(raw, 66, sensor_thing(0));
    write_u16(raw, SENSOR_BASE, 0xfffeu);
    write_u16(raw, SENSOR_BASE + 2, (uint16_t)((sensor_data << 7) | sensor_type));
    write_u16(raw, SENSOR_BASE + 4, 0);
    csb_v1_runtime_init(profile, NULL); profile->dungeon_handle = dungeon;
    profile->current_level = 0; profile->party_dir = 0; profile->champion_count = 1;
}
int main(void)
{
    CSB_V1_RuntimeProfile profile; CSB_V1_DungeonData dungeon;
    CSB_V1_F0276ReceiptPc34 receipt; uint8_t raw[160]; uint8_t raw_before[160];
    fixture(&profile, &dungeon, raw, 3, 0); memcpy(raw_before, raw, sizeof(raw));
    CHECK(csb_v1_f0276_sensor_receipt_pc34(&profile, 0, 1, 0xffffu, 0, 1, &receipt) == 1 && receipt.valid && receipt.sensor_type == 3 && receipt.would_trigger && receipt.effect == 0, "C003 floor-party receipt is source-qualified");
    CHECK(memcmp(raw, raw_before, sizeof(raw)) == 0 && profile.timeline_queue.eventCount == 0, "receipt does not mutate raw Things or publish a timeline event");
    fixture(&profile, &dungeon, raw, 1, 0);
    CHECK(csb_v1_f0276_sensor_receipt_pc34(&profile, 0, 1, 0xffffu, 1, 1, &receipt) == 1 && receipt.valid && !receipt.would_trigger && receipt.sensor_thing == 0xffffu, "C001 party-square exclusion has no fallback candidate");
    fixture(&profile, &dungeon, raw, 9, 35);
    CHECK(csb_v1_f0276_sensor_receipt_pc34(&profile, 0, 1, 0xffffu, 0, 1, &receipt) == 1 && receipt.valid && !receipt.would_trigger, "C009 rejects an unsupported engine-version sensor");
    fixture(&profile, &dungeon, raw, 3, 0); write_u16(raw, SENSOR_BASE, sensor_thing(3));
    CHECK(csb_v1_f0276_sensor_receipt_pc34(&profile, 0, 1, 0xffffu, 0, 1, &receipt) == 0, "malformed raw chain fails closed");
    printf("PASSED: %d\nFAILED: %d\n", passed, failed); return failed ? 1 : 0;
}
