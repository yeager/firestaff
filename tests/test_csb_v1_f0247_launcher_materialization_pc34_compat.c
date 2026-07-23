/* ReDMCSB TIMELINE.C F0247 -> PROJEXPL.C F0212 CSB21 overflow fallback. */
#include "csb_v1_f0247_launcher_materialization_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { ++failures; printf("FAIL: %s\\n", message); } \
} while (0)

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static unsigned short get_le16(const unsigned char *bytes, int offset)
{
    return (unsigned short)(bytes[offset] |
                            ((unsigned short)bytes[offset + 1] << 8));
}

static int square_offset(int x, int y)
{
    return x * 3 + y;
}

static void make_fixture(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_DungeonData *dungeon,
                         unsigned char raw[128])
{
    int slot;

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 128u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 128u;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 2;
    dungeon->thing_data_bases[3] = 70;
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[5] = 82;
    dungeon->thing_type_counts[5] = 1;

    /* The destination (1,0) owns first-thing slot zero; the C007 source at
     * (1,1) is the next marked square in that column and owns slot one. */
    raw[square_offset(1, 0)] = 0x30u;
    raw[square_offset(1, 1)] = 0x10u;
    put_le16(raw, 62, 0u);
    put_le16(raw, 66, 0xfffeu);
    put_le16(raw, 68, (unsigned short)(3u << 10));
    put_le16(raw, 70, 0xfffeu);
    put_le16(raw, 72, (unsigned short)((51u << 7) |
                                        DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ));
    put_le16(raw, 74, (unsigned short)(1u << 2));
    put_le16(raw, 76, (unsigned short)(5u | (7u << 8)));
    put_le16(raw, 82, 0xffffu);

    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    /* F028's first bit from zero is zero: M18_OPPOSITE(0) stays cell two. */
    profile->csbwin_random_seed = 0u;
    for (slot = 0; slot < PROJECTILE_LIST_CAPACITY; ++slot) {
        profile->projectiles.entries[slot].reserved3 = 1;
    }
    profile->projectiles.count = PROJECTILE_LIST_CAPACITY;
}

static int queue_wall_event(CSB_V1_RuntimeProfile *profile)
{
    struct DM1_Event_V1 event;

    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_WALL;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile->game_time);
    event.b_mapX = 1;
    event.b_mapY = 1;
    event.c_cell = 0;
    event.c_effect = DM1_EFFECT_SET;
    return csb_v1_runtime_add_timeline_event(profile, &event) >= 0;
}

static void test_f0212_overflow_receipt(void)
{
    CSB_V1_F0247LauncherMaterializationReceipt_PC34 receipt;

    CHECK(csb_v1_f0247_launcher_create_failure_materialization_pc34_compat(
              (uint16_t)(5u << 10), 1, 0, 2, &receipt) &&
              receipt.valid && receipt.thing == (uint16_t)((5u << 10) | (2u << 14)),
          "F0212 overflow receipt moves a kinetic object to its launch cell");
    CHECK(!csb_v1_f0247_launcher_create_failure_materialization_pc34_compat(
               0xff82u, 1, 0, 2, &receipt),
          "F0212 overflow receipt rejects C15 explosion pseudo-things");
}

static void test_c007_full_pool_places_real_object(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];

    make_fixture(&profile, &dungeon, raw);
    CHECK(queue_wall_event(&profile), "C007 source-shaped event queues");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "F0248 dispatches C007 with an exhausted C14 pool");
    CHECK(profile.projectiles.count == PROJECTILE_LIST_CAPACITY,
          "F0212 overflow does not invent another projectile slot");
    CHECK(get_le16(raw, 66) == (unsigned short)((5u << 10) | (2u << 14)) &&
              get_le16(raw, 82) == 0xfffeu && get_le16(raw, 84) == 27u,
          "C007 F0212 overflow moves the allocated arrow to the real launch square");
    CHECK((get_le16(raw, 72) & 0x007fu) == 0u,
          "F0248 still disables the once-only C007 source after F0212 recovery");
}

int main(void)
{
    test_f0212_overflow_receipt();
    test_c007_full_pool_places_real_object();
    if (failures != 0) {
        printf("test_csb_v1_f0247_launcher_materialization_pc34_compat: FAIL %d\\n", failures);
        return 1;
    }
    puts("ok: F0247 C007 preserves F0212 CSB21 overflow materialization");
    return 0;
}
