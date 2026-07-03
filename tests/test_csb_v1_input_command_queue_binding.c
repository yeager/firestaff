/*
 * test_csb_v1_input_command_queue_binding.c
 *
 * Focused CSB V1 input-command runtime binding gate.  This proves queued
 * C002 turn and C003 movement commands are dequeued through the shared V1
 * queue and applied to the bounded CSB runtime state: F0284 direction
 * rotation for turns and one-cell F0366 movement for open movement.  It
 * now also covers bounded movement consequences for real-format stairs,
 * pits, teleporters, doors/fake walls, and party floor sensors. It does not
 * claim wall-click sensors, object/group/projectile sensors, inventory,
 * action-area, or broader playability.
 *
 * Source-lock:
 *   ReDMCSB COMMAND.C F0380 lines 2045-2156 dispatches C001/C002 to
 *   F0365_COMMAND_ProcessTypes1To2_TurnParty.
 *   ReDMCSB CHAMPION.C F0284 lines 117-130 applies the turn delta to every
 *   champion Cell and Direction before writing G0308_i_PartyDirection.
 *   ReDMCSB CLIKMENU.C F0366 lines 224-351 plus DUNGEON.C F0150 lines
 *   1389-1391 apply one movement coordinate step.
 *   ReDMCSB MOVESENS.C F0267 lines 538-603 applies open-pit falls and
 *   CHAMPION.C F0324 lines 1991-2022 applies attack-20 fall damage to
 *   every living champion with legs/feet wound eligibility.
 */

#include "csb_v1_runtime_pc34_compat.h"
#include "memory_combat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static void put_le16(uint8_t *buf, int offset, uint16_t value)
{
    buf[offset + 0] = (uint8_t)(value & 0xffu);
    buf[offset + 1] = (uint8_t)(value >> 8);
}

static void make_party(CSB_V1_PartyState *party)
{
    int i;
    csb_v1_character_init_default(party);
    party->ChampionCount = 2;
    party->PartyDirection = CSB_V1_DIR_NORTH;
    party->LeaderIndex = 0;
    party->MagicCasterIndex = -1;
    party->PartyMapX = CSB_V1_START_PARTY_X;
    party->PartyMapY = CSB_V1_START_PARTY_Y;
    for (i = 0; i < party->ChampionCount; i++) {
        party->Champions[i].CurrentHealth = 100;
        party->Champions[i].MaximumHealth = 100;
        party->Champions[i].Cell = (uint8_t)i;
        party->Champions[i].Direction = CSB_V1_DIR_NORTH;
    }
}

static void test_turn_right_command_reaches_csb_runtime_state(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    csb_v1_runtime_init(&profile, NULL);
    make_party(&party);
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "fixture party snapshot enters the CSB runtime profile");

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
              &queue,
              (struct Dm1V1InputEventPc34Compat){
                  DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }) == 1,
          "PC-34 right-turn key queues one V1 command");

    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes one queued input command");
    CHECK(result.queue_result.command == DM1_V1_COMMAND_TURN_RIGHT,
          "dequeued command is C002 turn-right");
    CHECK(result.queue_result.dispatchedTurn == 1,
          "shared V1 queue marks the command as a turn dispatch");
    CHECK(result.unsupported_runtime_command == 0,
          "turn-right is a supported CSB runtime binding");
    CHECK(result.runtime_state_changed == 1,
          "CSB runtime reports a party state mutation");
    CHECK(result.old_party_dir == CSB_V1_DIR_NORTH &&
          result.new_party_dir == CSB_V1_DIR_EAST,
          "runtime direction changes north to east");
    CHECK(profile.party_dir == CSB_V1_DIR_EAST,
          "CSB runtime party_dir reaches the intended state");
    CHECK(profile.party_state.PartyDirection == CSB_V1_DIR_EAST,
          "CSB party snapshot direction reaches the intended state");
    CHECK(profile.party_state.Champions[0].Cell == 1 &&
          profile.party_state.Champions[1].Cell == 2,
          "F0284 cell delta applies to every loaded champion");
    CHECK(profile.party_state.Champions[0].Direction == CSB_V1_DIR_EAST &&
          profile.party_state.Champions[1].Direction == CSB_V1_DIR_EAST,
          "F0284 direction delta applies to every loaded champion");
    CHECK(profile.party_x == CSB_V1_START_PARTY_X &&
          profile.party_y == CSB_V1_START_PARTY_Y,
          "turn binding does not claim movement or change map coordinates");
    CHECK(queue.count == 0u,
          "input queue is empty after the consumed command");
}

static void test_movement_gate_keeps_command_queued(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    csb_v1_runtime_init(&profile, NULL);
    make_party(&party);
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "movement-gate fixture party enters runtime profile");

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
              &queue,
              (struct Dm1V1InputEventPc34Compat){
                  DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }) == 1,
          "PC-34 forward key queues one V1 movement command");

    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 1, 0, 0, &result) == 0,
          "CSB runtime leaves movement queued while movement is disabled");
    CHECK(result.queue_result.movementDisabledGate == 1,
          "shared V1 queue reports the movement-disabled gate");
    CHECK(result.queue_result.dequeued == 0,
          "movement command is not dequeued behind the gate");
    CHECK(result.runtime_state_changed == 0,
          "movement gate does not mutate CSB runtime state");
    CHECK(profile.party_dir == CSB_V1_DIR_NORTH,
          "movement gate leaves party direction unchanged");
    CHECK(queue.count == 1u,
          "movement command remains queued for a later runtime boundary");
}

static void test_forward_command_applies_open_runtime_step(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    csb_v1_runtime_init(&profile, NULL);
    make_party(&party);
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "open-step fixture party enters runtime profile");

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
              &queue,
              (struct Dm1V1InputEventPc34Compat){
                  DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }) == 1,
          "PC-34 forward key queues one open-step movement command");

    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes one open-step movement command");
    CHECK(result.queue_result.command == DM1_V1_COMMAND_MOVE_FORWARD,
          "open-step command is C003 move-forward");
    CHECK(result.queue_result.dispatchedMove == 1,
          "shared V1 queue marks the command as a move dispatch");
    CHECK(result.unsupported_runtime_command == 0,
          "move-forward is a supported bounded CSB runtime binding");
    CHECK(result.movement_command_handled == 1,
          "runtime result reports handled movement command");
    CHECK(result.movement_step_attempted == 1,
          "runtime result reports attempted movement step");
    CHECK(result.movement_step_applied == 1,
          "runtime result reports applied movement step");
    CHECK(result.movement_blocked_by_wall == 0,
          "runtime result reports no wall block on open step");
    CHECK(result.movement_destination_x == CSB_V1_START_PARTY_X &&
          result.movement_destination_y == CSB_V1_START_PARTY_Y - 1,
          "runtime result exposes open-step destination");
    CHECK(result.disabled_movement_ticks_after == 1,
          "runtime result exposes movement cooldown after open step");
    CHECK(result.runtime_state_changed == 1,
          "CSB runtime reports a coordinate state mutation");
    CHECK(profile.party_x == CSB_V1_START_PARTY_X,
          "northward open step leaves x unchanged");
    CHECK(profile.party_y == CSB_V1_START_PARTY_Y - 1,
          "northward open step decrements y");
    CHECK(profile.party_dir == CSB_V1_DIR_NORTH,
          "open step preserves party direction");
    CHECK(profile.party_state.PartyMapY == CSB_V1_START_PARTY_Y - 1,
          "party snapshot mirrors the open step");
    CHECK(profile.party_state.Champions[0].Cell == 0 &&
          profile.party_state.Champions[1].Cell == 1,
          "open step leaves champion cells unchanged");
    CHECK(queue.count == 0u,
          "input queue is empty after the consumed open step");
}

static void make_legacy_wall_dungeon(CSB_V1_DungeonData *dungeon,
                                     uint8_t *raw,
                                     size_t raw_size)
{
    size_t i;

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->level_offsets[0] = 0;
    dungeon->square_bytes = 2;
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;

    for (i = 0; i < 9; ++i) {
        raw[i * 2] = 2;      /* legacy fixture floor */
        raw[i * 2 + 1] = 0;
    }
    raw[(1 * 3 + 0) * 2] = 1; /* north of center: legacy fixture wall */
}

static void make_real_format_stair_dungeon(CSB_V1_DungeonData *dungeon,
                                           uint8_t *raw,
                                           size_t raw_size)
{
    size_t i;

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->level_count = 2;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->level_offsets[0] = 0;
    dungeon->level_widths[1] = 3;
    dungeon->level_heights[1] = 3;
    dungeon->level_offsets[1] = 9;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;

    for (i = 0; i < raw_size; ++i) {
        raw[i] = (uint8_t)(1u << 5); /* C01_ELEMENT_CORRIDOR */
    }
    raw[1 * 3 + 0] = (uint8_t)(3u << 5);       /* level 0: stairs down */
    raw[9 + 1 * 3 + 0] = (uint8_t)((3u << 5) | 0x04u); /* level 1: up */
}

static void make_real_format_consequence_dungeon(CSB_V1_DungeonData *dungeon,
                                                 uint8_t *raw,
                                                 size_t raw_size)
{
    size_t i;

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->level_count = 3;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->level_offsets[0] = 0;
    dungeon->level_widths[1] = 3;
    dungeon->level_heights[1] = 3;
    dungeon->level_offsets[1] = 9;
    dungeon->level_widths[2] = 3;
    dungeon->level_heights[2] = 3;
    dungeon->level_offsets[2] = 18;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;

    for (i = 0; i < raw_size; ++i) {
        raw[i] = (uint8_t)(1u << 5); /* C01_ELEMENT_CORRIDOR */
    }
}

static int load_real_format_teleporter_dungeon(CSB_V1_DungeonData *dungeon)
{
    uint8_t buf[114];
    const int level_count = 2;
    const int map0_desc = 44;
    const int map1_desc = 60;
    const int column_counts = 76;
    const int square_first_things = 88;
    const int teleporter_record = 90;
    const int raw_map = 96;
    const uint16_t map0_bits =
        (uint16_t)(0u | ((3u - 1u) << 6) | ((3u - 1u) << 11));
    const uint16_t map1_bits =
        (uint16_t)(1u | ((3u - 1u) << 6) | ((3u - 1u) << 11));
    uint16_t teleporter_word;

    memset(buf, 0, sizeof(buf));
    buf[4] = (uint8_t)level_count;
    put_le16(buf, 10, 1);      /* square-first-thing entries */
    put_le16(buf, 12 + 1 * 2, 1); /* one C01_THING_TYPE_TELEPORTER */
    put_le16(buf, map0_desc + 0, 0);
    put_le16(buf, map0_desc + 8, map0_bits);
    put_le16(buf, map1_desc + 0, 9);
    put_le16(buf, map1_desc + 8, map1_bits);

    put_le16(buf, column_counts + 2, 0); /* level 0, x=1, y=0 uses entry 0 */
    put_le16(buf, square_first_things, (uint16_t)(1u << 10));

    put_le16(buf, teleporter_record + 0, 0xffffu);
    teleporter_word =
        (uint16_t)(2u | (2u << 5) | (1u << 10) | (2u << 13) | 0x8000u);
    put_le16(buf, teleporter_record + 2, teleporter_word);
    put_le16(buf, teleporter_record + 4, (uint16_t)(1u << 8));

    memset(buf + raw_map, (uint8_t)(1u << 5), 18);
    buf[raw_map + 1 * 3 + 0] = (uint8_t)((5u << 5) | 0x10u | 0x08u);
    return csb_v1_dungeon_load(dungeon, buf, (int)sizeof(buf));
}

static int load_real_format_chained_teleporter_dungeon(
    CSB_V1_DungeonData *dungeon)
{
    uint8_t buf[153];
    const int level_count = 3;
    const int map0_desc = 44;
    const int map1_desc = 60;
    const int map2_desc = 76;
    const int column_counts = 92;
    const int square_first_things = 110;
    const int teleporter0_record = 114;
    const int teleporter1_record = 120;
    const int raw_map = 126;
    const uint16_t map0_bits =
        (uint16_t)(0u | ((3u - 1u) << 6) | ((3u - 1u) << 11));
    const uint16_t map1_bits =
        (uint16_t)(1u | ((3u - 1u) << 6) | ((3u - 1u) << 11));
    const uint16_t map2_bits =
        (uint16_t)(2u | ((3u - 1u) << 6) | ((3u - 1u) << 11));
    uint16_t teleporter_word;

    memset(buf, 0, sizeof(buf));
    buf[4] = (uint8_t)level_count;
    put_le16(buf, 10, 2);          /* square-first-thing entries */
    put_le16(buf, 12 + 1 * 2, 2);  /* two C01_THING_TYPE_TELEPORTER records */
    put_le16(buf, map0_desc + 0, 0);
    put_le16(buf, map0_desc + 8, map0_bits);
    put_le16(buf, map1_desc + 0, 9);
    put_le16(buf, map1_desc + 8, map1_bits);
    put_le16(buf, map2_desc + 0, 18);
    put_le16(buf, map2_desc + 8, map2_bits);

    put_le16(buf, column_counts + 2, 0);  /* level 0, x=1 -> SFT 0 */
    put_le16(buf, column_counts + 8, 1);  /* level 1, x=1 -> SFT 1 */
    put_le16(buf, square_first_things + 0, (uint16_t)(1u << 10));
    put_le16(buf, square_first_things + 2, (uint16_t)((1u << 10) | 1u));

    put_le16(buf, teleporter0_record + 0, 0xffffu);
    teleporter_word =
        (uint16_t)(1u | (1u << 5) | (1u << 10) | (2u << 13) | 0x8000u);
    put_le16(buf, teleporter0_record + 2, teleporter_word);
    put_le16(buf, teleporter0_record + 4, (uint16_t)(1u << 8));

    put_le16(buf, teleporter1_record + 0, 0xffffu);
    teleporter_word =
        (uint16_t)(2u | (2u << 5) | (3u << 10) |
                   (1u << 12) | (2u << 13));
    put_le16(buf, teleporter1_record + 2, teleporter_word);
    put_le16(buf, teleporter1_record + 4, (uint16_t)(2u << 8));

    memset(buf + raw_map, (uint8_t)(1u << 5), 27);
    buf[raw_map + 1 * 3 + 0] = (uint8_t)((5u << 5) | 0x10u | 0x08u);
    buf[raw_map + 9 + 1 * 3 + 1] =
        (uint8_t)((5u << 5) | 0x10u | 0x08u);
    return csb_v1_dungeon_load(dungeon, buf, (int)sizeof(buf));
}

static int load_real_format_floor_sensor_dungeon(CSB_V1_DungeonData *dungeon)
{
    uint8_t buf[85];
    const int level_count = 1;
    const int map0_desc = 44;
    const int column_counts = 60;
    const int square_first_things = 66;
    const int sensor_record = 68;
    const int raw_map = 76;
    const uint16_t map0_bits =
        (uint16_t)(0u | ((3u - 1u) << 6) | ((3u - 1u) << 11));
    const uint16_t sensor_type_data =
        (uint16_t)3u; /* C003_SENSOR_FLOOR_PARTY, data=0 */
    const uint16_t sensor_flags =
        (uint16_t)((3u << 3) | (1u << 6)); /* HOLD + audible */
    const uint16_t sensor_target =
        (uint16_t)((1u << 6) | (0u << 11)); /* target x=1,y=0,cell=0 */

    memset(buf, 0, sizeof(buf));
    buf[4] = (uint8_t)level_count;
    put_le16(buf, 10, 1);          /* square-first-thing entries */
    put_le16(buf, 12 + 3 * 2, 1);  /* one C03_THING_TYPE_SENSOR record */
    put_le16(buf, map0_desc + 0, 0);
    put_le16(buf, map0_desc + 8, map0_bits);

    put_le16(buf, column_counts + 2, 0); /* level 0, x=1 -> SFT 0 */
    put_le16(buf, square_first_things, (uint16_t)(3u << 10));

    put_le16(buf, sensor_record + 0, 0xfffeu);
    put_le16(buf, sensor_record + 2, sensor_type_data);
    put_le16(buf, sensor_record + 4, sensor_flags);
    put_le16(buf, sensor_record + 6, sensor_target);

    memset(buf + raw_map, (uint8_t)(1u << 5), 9);
    buf[raw_map + 1 * 3 + 0] = (uint8_t)((1u << 5) | 0x10u);
    return csb_v1_dungeon_load(dungeon, buf, (int)sizeof(buf));
}

static void test_forward_command_blocks_legacy_wall_destination(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[18];
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    csb_v1_runtime_init(&profile, NULL);
    make_party(&party);
    party.PartyMapX = 1;
    party.PartyMapY = 1;
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "legacy-wall fixture party enters runtime profile");
    profile.party_x = 1;
    profile.party_y = 1;
    profile.party_state.PartyMapX = 1;
    profile.party_state.PartyMapY = 1;
    make_legacy_wall_dungeon(&dungeon, raw, sizeof(raw));
    profile.dungeon_handle = &dungeon;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
              &queue,
              (struct Dm1V1InputEventPc34Compat){
                  DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }) == 1,
          "PC-34 forward key queues a legacy-wall movement command");

    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes the wall-blocked command");
    CHECK(result.queue_result.command == DM1_V1_COMMAND_MOVE_FORWARD,
          "legacy-wall command is C003 move-forward");
    CHECK(result.unsupported_runtime_command == 0,
          "legacy-wall command is handled by bounded runtime movement");
    CHECK(result.movement_command_handled == 1,
          "legacy-wall result reports handled movement command");
    CHECK(result.movement_step_attempted == 1,
          "legacy-wall result reports attempted movement step");
    CHECK(result.movement_step_applied == 0,
          "legacy-wall result reports no applied movement step");
    CHECK(result.movement_blocked_by_wall == 1,
          "legacy-wall result reports wall block");
    CHECK(result.movement_destination_x == 1 &&
          result.movement_destination_y == 0,
          "legacy-wall result exposes blocked destination");
    CHECK(result.runtime_state_changed == 0,
          "legacy-wall block reports no coordinate mutation");
    CHECK(profile.party_x == 1 && profile.party_y == 1,
          "legacy wall destination blocks the runtime party step");
    CHECK(profile.party_state.PartyMapX == 1 &&
          profile.party_state.PartyMapY == 1,
          "legacy wall block keeps party snapshot coordinates");
    CHECK(queue.count == 0u,
          "input queue is empty after the consumed wall-blocked command");
}

static void test_forward_command_applies_real_format_stairs(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[18];
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    csb_v1_runtime_init(&profile, NULL);
    make_party(&party);
    party.PartyMapX = 1;
    party.PartyMapY = 1;
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "stair fixture party enters runtime profile");
    profile.party_x = 1;
    profile.party_y = 1;
    profile.party_state.PartyMapX = 1;
    profile.party_state.PartyMapY = 1;
    make_real_format_stair_dungeon(&dungeon, raw, sizeof(raw));
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    csb_v1_dungeon_set_current_level(0);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
              &queue,
              (struct Dm1V1InputEventPc34Compat){
                  DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }) == 1,
          "PC-34 forward key queues a stair-down movement command");
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes the stair-down movement command");
    CHECK(result.movement_step_applied == 1,
          "stair-down movement still applies coordinate step");
    CHECK(result.movement_destination_square_type == 3,
          "stair-down result exposes destination stair square type");
    CHECK(result.movement_destination_raw_square == (3 << 5),
          "stair-down result exposes raw real-format square byte");
    CHECK(result.stair_transition_applied == 1,
          "stair-down result reports level transition");
    CHECK(result.stair_up == 0,
          "stair-down result reports downward direction");
    CHECK(result.old_party_level == 0 && result.new_party_level == 1,
          "stair-down movement changes runtime level 0 to 1");
    CHECK(profile.current_level == 1 && csb_v1_dungeon_get_current_level() == 1,
          "stair-down movement updates profile and dungeon current level");

    profile.party_x = 1;
    profile.party_y = 1;
    profile.party_state.PartyMapX = 1;
    profile.party_state.PartyMapY = 1;
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
              &queue,
              (struct Dm1V1InputEventPc34Compat){
                  DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }) == 1,
          "PC-34 forward key queues a stair-up movement command");
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes the stair-up movement command");
    CHECK(result.movement_destination_square_type == 3,
          "stair-up result exposes destination stair square type");
    CHECK(result.stair_transition_applied == 1,
          "stair-up result reports level transition");
    CHECK(result.stair_up == 1,
          "stair-up result reports upward direction");
    CHECK(result.old_party_level == 1 && result.new_party_level == 0,
          "stair-up movement changes runtime level 1 to 0");
    CHECK(profile.current_level == 0 && csb_v1_dungeon_get_current_level() == 0,
          "stair-up movement updates profile and dungeon current level");
}

static void queue_forward_or_fail(struct Dm1V1InputCommandQueuePc34Compat *queue,
                                  const char *message)
{
    DM1_V1_InputCommandQueue_InitPc34Compat(queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
              queue,
              (struct Dm1V1InputEventPc34Compat){
                  DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }) == 1,
          message);
}

static void seed_center_party(CSB_V1_RuntimeProfile *profile)
{
    CSB_V1_PartyState party;

    make_party(&party);
    party.PartyMapX = 1;
    party.PartyMapY = 1;
    CHECK(csb_v1_runtime_set_party_state(profile, &party) == 0,
          "movement consequence fixture party enters runtime profile");
    profile->party_x = 1;
    profile->party_y = 1;
    profile->party_state.PartyMapX = 1;
    profile->party_state.PartyMapY = 1;
    profile->current_level = 0;
    csb_v1_dungeon_set_current_level(0);
}

static void test_forward_command_handles_real_format_door_fakewall_and_pit(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[27];
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;
    const int north_of_center = 1 * 3 + 0;

    csb_v1_runtime_init(&profile, NULL);
    make_real_format_consequence_dungeon(&dungeon, raw, sizeof(raw));
    profile.dungeon_handle = &dungeon;

    raw[north_of_center] = (uint8_t)((4u << 5) | 4u); /* closed door */
    seed_center_party(&profile);
    queue_forward_or_fail(&queue, "PC-34 forward key queues closed-door move");
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes the closed-door movement command");
    CHECK(result.movement_step_applied == 0,
          "closed-door movement does not apply coordinate step");
    CHECK(result.movement_blocked_by_wall == 1,
          "closed-door movement is blocked by the movement probe");
    CHECK(result.movement_blocked_by_door == 1,
          "closed-door movement reports door block cause");
    CHECK(result.movement_destination_square_type == 4,
          "closed-door movement exposes destination door type");
    CHECK(result.movement_destination_door_state == 4,
          "closed-door movement exposes C4 closed state");
    CHECK(profile.party_x == 1 && profile.party_y == 1,
          "closed-door block preserves party coordinates");

    raw[north_of_center] = (uint8_t)(6u << 5); /* closed real fakewall */
    seed_center_party(&profile);
    queue_forward_or_fail(&queue, "PC-34 forward key queues closed-fakewall move");
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes the closed-fakewall movement command");
    CHECK(result.movement_step_applied == 0,
          "closed-fakewall movement does not apply coordinate step");
    CHECK(result.movement_blocked_by_fakewall == 1,
          "closed-fakewall movement reports fakewall block cause");
    CHECK(result.movement_destination_square_type == 6,
          "closed-fakewall movement exposes destination fakewall type");
    CHECK(profile.party_x == 1 && profile.party_y == 1,
          "closed-fakewall block preserves party coordinates");

    raw[north_of_center] = (uint8_t)((2u << 5) | 0x08u); /* open pit */
    seed_center_party(&profile);
    queue_forward_or_fail(&queue, "PC-34 forward key queues open-pit move");
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes the open-pit movement command");
    CHECK(result.movement_step_applied == 1,
          "open-pit movement applies coordinate step before falling");
    CHECK(result.movement_destination_square_type == 2,
          "open-pit movement exposes destination pit type");
    CHECK(result.pit_open == 1,
          "open-pit movement reports open pit bit");
    CHECK(result.pit_fall_applied == 1,
          "open-pit movement applies bounded level fall");
    CHECK(result.chained_move_count == 1 && result.pit_chain_count == 1,
          "open-pit movement reports one chained pit move");
    CHECK(result.pit_fall_damaged_champion_count == 2,
          "open-pit movement applies F0324 fall damage to both champions");
    CHECK(result.pit_fall_total_damage > 0 &&
          profile.party_state.Champions[0].CurrentHealth < 100 &&
          profile.party_state.Champions[1].CurrentHealth < 100,
          "open-pit movement mutates champion health through fall damage");
    CHECK((result.pit_fall_wound_mask &
           ~(COMBAT_WOUND_LEGS | COMBAT_WOUND_FEET)) == 0,
          "open-pit movement restricts fall wounds to legs/feet");
    CHECK(result.old_party_level == 0 && result.new_party_level == 1,
          "open-pit movement changes runtime level 0 to 1");
    CHECK(profile.current_level == 1 && csb_v1_dungeon_get_current_level() == 1,
          "open-pit movement updates profile and dungeon current level");
    CHECK(profile.party_x == 1 && profile.party_y == 0,
          "open-pit movement keeps destination coordinates on new level");
}

static void test_forward_command_applies_real_format_teleporter(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    memset(&dungeon, 0, sizeof(dungeon));
    CHECK(load_real_format_teleporter_dungeon(&dungeon) == 0,
          "real-format teleporter fixture loads through CSB dungeon loader");
    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    seed_center_party(&profile);
    queue_forward_or_fail(&queue, "PC-34 forward key queues open-teleporter move");

    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes the open-teleporter movement command");
    CHECK(result.movement_step_applied == 1,
          "open-teleporter movement first applies destination coordinate step");
    CHECK(result.movement_destination_square_type == 5,
          "open-teleporter movement exposes destination teleporter type");
    CHECK(result.teleporter_open == 1,
          "open-teleporter movement reports open bit");
    CHECK(result.teleporter_scope == 2,
          "open-teleporter movement reports objects-or-party scope");
    CHECK(result.teleporter_transition_applied == 1,
          "open-teleporter movement applies party teleporter transition");
    CHECK(result.chained_move_count == 1 &&
          result.teleporter_chain_count == 1,
          "open-teleporter movement reports one chained teleporter move");
    CHECK(result.teleporter_target_x == 2 && result.teleporter_target_y == 2,
          "open-teleporter movement exposes target coordinates");
    CHECK(result.teleporter_target_level == 1,
          "open-teleporter movement exposes target map index");
    CHECK(result.teleporter_rotation == 1 &&
          result.teleporter_absolute_rotation == 0 &&
          result.teleporter_audible == 1,
          "open-teleporter movement exposes relative audible rotation");
    CHECK(result.old_party_level == 0 && result.new_party_level == 1,
          "open-teleporter movement changes runtime level 0 to 1");
    CHECK(profile.party_x == 2 && profile.party_y == 2,
          "open-teleporter movement updates party coordinates to target");
    CHECK(profile.current_level == 1 && csb_v1_dungeon_get_current_level() == 1,
          "open-teleporter movement updates profile and dungeon current level");
    CHECK(profile.party_dir == CSB_V1_DIR_EAST,
          "relative teleporter rotation turns north to east through F0284");
    CHECK(profile.party_state.Champions[0].Cell == 1 &&
          profile.party_state.Champions[1].Cell == 2,
          "relative teleporter rotation updates champion cells through F0284");
    csb_v1_dungeon_free(&dungeon);
}

static void test_forward_command_chains_real_format_pits(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[27];
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;
    const int north_of_center = 1 * 3 + 0;

    csb_v1_runtime_init(&profile, NULL);
    make_real_format_consequence_dungeon(&dungeon, raw, sizeof(raw));
    profile.dungeon_handle = &dungeon;
    raw[north_of_center] = (uint8_t)((2u << 5) | 0x08u);
    raw[9 + north_of_center] = (uint8_t)((2u << 5) | 0x08u);
    seed_center_party(&profile);
    queue_forward_or_fail(&queue, "PC-34 forward key queues chained-pit move");

    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes the chained-pit movement command");
    CHECK(result.movement_step_applied == 1,
          "chained-pit movement applies the initial coordinate step");
    CHECK(result.pit_fall_applied == 1,
          "chained-pit movement applies at least one pit fall");
    CHECK(result.chained_move_count == 2 && result.pit_chain_count == 2,
          "chained-pit movement follows both open pits");
    CHECK(result.pit_fall_damaged_champion_count == 4,
          "chained-pit movement applies F0324 damage once per champion per pit");
    CHECK(result.pit_fall_total_damage > 0 &&
          profile.party_state.Champions[0].CurrentHealth < 100 &&
          profile.party_state.Champions[1].CurrentHealth < 100,
          "chained-pit movement accumulates champion fall damage");
    CHECK(result.chained_move_limit_hit == 0,
          "chained-pit movement stops before the F0267 chain limit");
    CHECK(result.old_party_level == 0 && result.new_party_level == 2,
          "chained-pit movement changes runtime level 0 to 2");
    CHECK(profile.current_level == 2 && csb_v1_dungeon_get_current_level() == 2,
          "chained-pit movement updates profile and dungeon to final level");
    CHECK(profile.party_x == 1 && profile.party_y == 0,
          "chained-pit movement keeps the falling coordinates");
}

static void test_forward_command_chains_real_format_teleporters(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    memset(&dungeon, 0, sizeof(dungeon));
    CHECK(load_real_format_chained_teleporter_dungeon(&dungeon) == 0,
          "real-format chained-teleporter fixture loads through CSB loader");
    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    seed_center_party(&profile);
    queue_forward_or_fail(&queue,
                          "PC-34 forward key queues chained-teleporter move");

    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes the chained-teleporter movement command");
    CHECK(result.movement_step_applied == 1,
          "chained-teleporter movement applies initial coordinate step");
    CHECK(result.teleporter_transition_applied == 1,
          "chained-teleporter movement applies teleporter transition");
    CHECK(result.chained_move_count == 2 &&
          result.teleporter_chain_count == 2,
          "chained-teleporter movement follows both teleporters");
    CHECK(result.chained_move_limit_hit == 0,
          "chained-teleporter movement stops before the F0267 chain limit");
    CHECK(result.teleporter_target_x == 1 &&
          result.teleporter_target_y == 1 &&
          result.teleporter_target_level == 1,
          "chained-teleporter result preserves first teleporter metadata");
    CHECK(result.old_party_level == 0 && result.new_party_level == 2,
          "chained-teleporter movement changes runtime level 0 to 2");
    CHECK(profile.party_x == 2 && profile.party_y == 2,
          "chained-teleporter movement reaches final target coordinates");
    CHECK(profile.current_level == 2 && csb_v1_dungeon_get_current_level() == 2,
          "chained-teleporter movement updates profile and dungeon level");
    CHECK(profile.party_dir == CSB_V1_DIR_WEST,
          "chained-teleporter absolute rotation from second teleporter wins");
    csb_v1_dungeon_free(&dungeon);
}

static void test_forward_command_triggers_real_format_party_floor_sensor(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    memset(&dungeon, 0, sizeof(dungeon));
    CHECK(load_real_format_floor_sensor_dungeon(&dungeon) == 0,
          "real-format floor-sensor fixture loads through CSB loader");
    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    seed_center_party(&profile);
    queue_forward_or_fail(&queue, "PC-34 forward key queues floor-sensor move");

    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes the floor-sensor movement command");
    CHECK(result.movement_step_applied == 1,
          "floor-sensor movement applies the coordinate step");
    CHECK(result.sensor_source_remove_checked == 1,
          "floor-sensor movement checks source-square removal sensors");
    CHECK(result.sensor_destination_add_checked == 1,
          "floor-sensor movement checks destination-square addition sensors");
    CHECK(result.sensor_trigger_count == 1,
          "destination C003 floor-party sensor triggers once");
    CHECK(result.sensor_event_count == 1,
          "floor-party sensor queues one F0268-style square event");
    CHECK(result.sensor_audible_count == 1,
          "floor-party sensor preserves the audible switch flag");
    CHECK(result.sensor_last_type == 3 && result.sensor_last_data == 0,
          "floor-party sensor exposes C003/data=0 metadata");
    CHECK(result.sensor_last_effect == DM1_EFFECT_SET,
          "HOLD floor-party add resolves to SET");
    CHECK(result.sensor_last_target_x == 1 &&
          result.sensor_last_target_y == 0 &&
          result.sensor_last_target_cell == 0,
          "floor-party sensor exposes remote target");
    CHECK(result.sensor_last_event_type == DM1_EVENT_CORRIDOR,
          "floor-party sensor maps corridor target to C05 event");
    CHECK(profile.timeline_queue.eventCount == 1,
          "CSB runtime timeline owns the queued sensor event");
    CHECK(profile.timeline_queue.events[0].type == DM1_EVENT_CORRIDOR &&
          profile.timeline_queue.events[0].b_mapX == 1 &&
          profile.timeline_queue.events[0].b_mapY == 0 &&
          profile.timeline_queue.events[0].c_effect == DM1_EFFECT_SET,
          "queued sensor event carries target square and SET effect");
    CHECK(profile.party_x == 1 && profile.party_y == 0,
          "floor-sensor movement leaves party on the destination square");
    csb_v1_dungeon_free(&dungeon);
}

int main(void)
{
    printf("=== CSB V1 Input Command Queue Binding Gate ===\n\n");
    test_turn_right_command_reaches_csb_runtime_state();
    test_movement_gate_keeps_command_queued();
    test_forward_command_applies_open_runtime_step();
    test_forward_command_blocks_legacy_wall_destination();
    test_forward_command_applies_real_format_stairs();
    test_forward_command_handles_real_format_door_fakewall_and_pit();
    test_forward_command_applies_real_format_teleporter();
    test_forward_command_chains_real_format_pits();
    test_forward_command_chains_real_format_teleporters();
    test_forward_command_triggers_real_format_party_floor_sensor();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: queued CSB V1 turn and bounded movement commands reach CSB runtime party state");
        puts("sourceEvidence=ReDMCSB COMMAND.C F0380 lines 2045-2156; CLIKMENU.C F0366 lines 224-351; DUNGEON.C F0150 lines 1389-1391; CHAMPION.C F0284 lines 117-130");
    }
    return failed == 0 ? 0 : 1;
}
