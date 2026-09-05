#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\n", message); } \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static void make_dungeon(CSB_V1_DungeonData *dungeon,
                         unsigned char *raw, int visible)
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
    dungeon->thing_data_bases[2] = 68;
    dungeon->thing_type_counts[2] = 1;
    dungeon->text_data_base = 96;
    dungeon->text_word_count = 2;
    raw[1] |= 0x10u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, (unsigned short)(2u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, (unsigned short)(visible ? 1u : 0u));
    put_le16(raw, 96, (unsigned short)((7u << 10) | (4u << 5) | 11u));
    put_le16(raw, 98, (unsigned short)(31u << 10));
}

static void prepare_party(CSB_V1_RuntimeProfile *profile,
                          CSB_V1_DungeonData *dungeon)
{
    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    profile->current_level = 0;
    profile->party_x = 0;
    profile->party_y = 0;
    profile->party_dir = CSB_V1_DIR_SOUTH;
    profile->champion_count = 1;
    profile->party_state_valid = 1;
    profile->party_state.ChampionCount = 1;
    profile->party_state.Champions[0].CurrentHealth = 100;
}

static int move_forward(CSB_V1_RuntimeProfile *profile)
{
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    if (!DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
            &queue, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0)) return 0;
    return csb_v1_runtime_process_input_queue(profile, &queue, 0, 0, 0,
                                              &result) == 1 &&
           result.movement_step_applied == 1;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];

    make_dungeon(&dungeon, raw, 1);
    prepare_party(&profile, &dungeon);
    CHECK(move_forward(&profile), "F0267 moves party onto the loaded C02 square");
    CHECK(profile.csbwin_text_message_receipt.valid &&
              strcmp(profile.csbwin_text_message_receipt.text, "HEL") == 0,
          "F0276 uses F0168 bytes for the visible party-arrival message");
    profile.game_time = profile.csbwin_text_message_receipt.source_game_time + 69u;
    CHECK(csb_v1_runtime_text_message_active_pc34(&profile),
          "F0046 retains the authentic C015 message through tick 69");
    profile.game_time = profile.csbwin_text_message_receipt.source_game_time + 70u;
    CHECK(!csb_v1_runtime_text_message_active_pc34(&profile),
          "F0046 expires the authentic C015 message exactly at tick 70");

    make_dungeon(&dungeon, raw, 0);
    prepare_party(&profile, &dungeon);
    CHECK(move_forward(&profile), "F0267 still moves onto an invisible C02 square");
    CHECK(!profile.csbwin_text_message_receipt.valid,
          "invisible or malformed C02 text remains fail-closed");

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
