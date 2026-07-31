/* ReDMCSB MAIN.C F028 + TIMELINE.C F0247 launcher RNG ownership. */
#include "csb_v1_f0247_launcher_rng_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

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

static int square_offset(int x, int y)
{
    return x * 3 + y;
}

static void make_fixture(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_DungeonData *dungeon,
                         unsigned char raw[128],
                         unsigned int random_state)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 128u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 128u;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = 68;
    dungeon->thing_type_counts[3] = 1;

    /* C008 source wall at (0,1), cell zero, shoots north into (0,0). */
    raw[square_offset(0, 1)] = 0x10u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, (unsigned short)((2u << 7) |
                                        DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION));
    put_le16(raw, 72, 0u);
    put_le16(raw, 74, (unsigned short)(7u | (9u << 8)));

    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    profile->csbwin_gameblock2_summary_valid = 1;
    profile->csbwin_random_seed = random_state;
}

static int queue_wall_event(CSB_V1_RuntimeProfile *profile, int cell)
{
    struct DM1_Event_V1 event;

    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_WALL;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile->game_time);
    event.b_mapX = 0;
    event.b_mapY = 1;
    event.c_cell = (unsigned char)cell;
    event.c_effect = DM1_EFFECT_SET;
    return csb_v1_runtime_add_timeline_event(profile, &event) >= 0;
}

static void test_f028_word_exact_stream(void)
{
    uint32_t state = UINT32_C(0x12345678);
    int bit = -1;
    int masked = -1;

    CHECK(csb_v1_f0247_launcher_next_random_bit_pc34_compat(&state, &bit),
          "F028 accepts a source random-state word");
    CHECK(state == UINT32_C(0x7ee30323) && bit == 1,
          "F028 advances G349 with the ReDMCSB multiplier and extracts bit zero");
    CHECK(csb_v1_f0247_launcher_next_random_masked_pc34_compat(
              &state, 31u, &masked),
          "F027 accepts the same persisted source random state");
    CHECK(state == UINT32_C(0x9c78ff32) && masked == 31,
          "F027 advances G349 once and applies the F0113 M003_RANDOM(32) mask");
    CHECK(!csb_v1_f0247_launcher_next_random_bit_pc34_compat(NULL, &bit),
          "F028 rejects a missing source state");
}

static void test_c008_consumes_one_bit_only_after_cell_match(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];
    const uint32_t before = UINT32_C(0x12345678);
    const uint32_t after = UINT32_C(0x7ee30323);

    make_fixture(&profile, &dungeon, raw, before);
    CHECK(queue_wall_event(&profile, 1), "wrong-cell C008 event queues");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "wrong-cell C008 event reaches F0248");
    CHECK(profile.csbwin_random_seed == before && profile.projectiles.count == 0,
          "wrong-cell C008 does not consume M005_RANDOM(2)");

    CHECK(queue_wall_event(&profile, 0), "matching C008 event queues");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "matching C008 event reaches F0247");
    CHECK(profile.csbwin_random_seed == after && profile.projectiles.count == 1,
          "C008 consumes exactly one persisted F028 random bit after selecting its explosion");
    CHECK(profile.projectiles.entries[0].cell == 3,
          "C008 applies the source random bit to the opposite launch cell");
}

int main(void)
{
    test_f028_word_exact_stream();
    test_c008_consumes_one_bit_only_after_cell_match();
    if (failures != 0) {
        printf("test_csb_v1_f0247_launcher_rng_pc34_compat: FAIL %d\\n", failures);
        return 1;
    }
    puts("ok: F0247 uses ReDMCSB F028 launcher random-bit timing");
    return 0;
}
