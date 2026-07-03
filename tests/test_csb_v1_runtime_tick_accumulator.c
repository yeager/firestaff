/*
 * test_csb_v1_runtime_tick_accumulator.c
 *
 * Narrow CSB V1 post-handoff runtime tick regression.  This does not claim
 * broad CSB playability; it proves the runtime clock boundary can advance
 * source-locked 55ms game-time quanta from accumulated frame deltas.
 *
 * Source-lock:
 *   ReDMCSB TIMELINE.C F0235 lines 702-708 compares event time with
 *   G0313_ul_GameTime.
 *   ReDMCSB COMMAND.C F0380 lines 2383-2429 toggles
 *   G0301_B_GameTimeTicking for freeze/unfreeze commands.
 */

#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_creature_ai_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static void test_subquantum_frame_slices_fire_one_tick(void)
{
    CSB_V1_RuntimeProfile profile;

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;

    csb_v1_runtime_tick(&profile, 16U);
    csb_v1_runtime_tick(&profile, 16U);
    CHECK(profile.total_play_ms == 32U,
          "two subquantum frame slices accumulate wall time");
    CHECK(profile.tick_count == 0U,
          "no V1 tick fires before the 55ms quantum");
    CHECK(csb_v1_runtime_tick_due(&profile, 0U) == 0,
          "tick_due reports false below the 55ms boundary");

    csb_v1_runtime_tick(&profile, 23U);
    CHECK(profile.total_play_ms == 55U,
          "16+16+23ms reaches one exact V1 quantum");
    CHECK(profile.tick_count == 1U,
          "one V1 tick fires from accumulated subquantum slices");
    CHECK(profile.game_time == 1U,
          "game_time advances once with the fired V1 tick");
    CHECK(profile.game_ticks == CSB_V1_TICK_MS_NOMINAL,
          "game_ticks records one nominal 55ms tick");
    CHECK(profile.chaos_magic.spell_grid_version == 1U,
          "chaos spell grid advances with the fired tick");
    CHECK(csb_v1_runtime_tick_due(&profile, 0U) == 0,
          "tick_due is false immediately after the due tick is consumed");
}

static void test_multi_quantum_tick_and_due_probe(void)
{
    CSB_V1_RuntimeProfile profile;

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;

    csb_v1_runtime_tick(&profile, CSB_V1_TICK_MS_NOMINAL * 3U + 10U);
    CHECK(profile.total_play_ms == 175U,
          "multi-quantum dt preserves the residual wall time");
    CHECK(profile.tick_count == 3U,
          "multi-quantum dt fires three V1 ticks");
    CHECK(profile.game_time == 3U,
          "game_time matches the three fired ticks");
    CHECK(profile.game_ticks == CSB_V1_TICK_MS_NOMINAL * 3U,
          "game_ticks records three nominal quanta");
    CHECK(profile.chaos_magic.spell_grid_version == 3U,
          "chaos spell grid advances once per fired tick");
    CHECK(csb_v1_runtime_tick_due(&profile, 219U) == 0,
          "tick_due(now_ms) is false until the next 55ms boundary");
    CHECK(csb_v1_runtime_tick_due(&profile, 220U) == 1,
          "tick_due(now_ms) detects the next unconsumed boundary");
}

static void test_tick_v1_steps_exactly_once_and_honors_stop_states(void)
{
    CSB_V1_RuntimeProfile profile;

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "tick_v1 fires one deterministic V1 tick from a fresh runtime");
    CHECK(profile.total_play_ms == CSB_V1_TICK_MS_NOMINAL,
          "tick_v1 advances wall time by exactly one nominal quantum");
    CHECK(profile.tick_count == 1U,
          "tick_v1 increments tick_count exactly once");
    CHECK(profile.game_time == 1U,
          "tick_v1 increments game_time exactly once");

    profile.paused = 1;
    CHECK(csb_v1_runtime_tick_v1(&profile) == 0,
          "tick_v1 is blocked while the runtime is paused");
    CHECK(profile.tick_count == 1U,
          "paused tick_v1 does not advance tick_count");
    CHECK(profile.total_play_ms == CSB_V1_TICK_MS_NOMINAL,
          "paused tick_v1 does not accumulate wall time");

    profile.paused = 0;
    profile.game_over = 1;
    csb_v1_runtime_tick(&profile, CSB_V1_TICK_MS_NOMINAL);
    CHECK(profile.tick_count == 1U,
          "game_over runtime_tick does not advance tick_count");
    CHECK(profile.total_play_ms == CSB_V1_TICK_MS_NOMINAL,
          "game_over runtime_tick does not accumulate wall time");
}

static void test_timeline_events_fire_before_game_time_increment(void)
{
    CSB_V1_RuntimeProfile profile;
    struct DM1_Event_V1 ev;
    struct DM1_TickDispatchResult_V1 dispatch;

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_PLAY_SOUND;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 1);
    ev.b_mapX = 7;
    ev.b_mapY = 8;
    CHECK(csb_v1_runtime_add_timeline_event(&profile, &ev) >= 0,
          "CSB runtime accepts a queued V1 timeline event");

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_DOOR_ANIMATION;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 2);
    ev.b_mapX = 9;
    ev.b_mapY = 10;
    CHECK(csb_v1_runtime_add_timeline_event(&profile, &ev) >= 0,
          "CSB runtime accepts a second boundary event");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "first tick fires while game_time is still zero");
    CHECK(profile.game_time == 1U,
          "first tick increments game_time after timeline processing");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 0,
          "event scheduled for time 1 does not fire at pre-increment time 0");
    CHECK(profile.timeline_queue.eventCount == 2,
          "both boundary events remain queued after tick zero processing");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "second tick processes pre-increment game_time 1");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 1,
          "event scheduled for time 1 fires before game_time advances to 2");
    CHECK(dispatch.records[0].dispatchKind == DM1_DISPATCH_SOUND,
          "time 1 event dispatches through the sound event boundary");
    CHECK(dispatch.records[0].mapX == 7 && dispatch.records[0].mapY == 8,
          "time 1 dispatch preserves event coordinates");
    CHECK(profile.game_time == 2U,
          "second tick increments game_time after event dispatch");
    CHECK(profile.timeline_queue.eventCount == 1,
          "future time 2 event remains queued after the time 1 boundary");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "third tick processes pre-increment game_time 2");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 1,
          "event scheduled for time 2 fires on the next boundary");
    CHECK(dispatch.records[0].dispatchKind == DM1_DISPATCH_DOOR_ANIMATION,
          "time 2 event dispatches through the door-animation boundary");
    CHECK(profile.timeline_dispatch_count == 2U,
          "CSB runtime records the cumulative timeline dispatch count");
    CHECK(profile.timeline_queue.eventCount == 0,
          "timeline queue is empty after both boundary events fire");
}

static void make_real_format_square_event_dungeon(CSB_V1_DungeonData *dungeon,
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
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;
    for (i = 0; i < raw_size; ++i) {
        raw[i] = (uint8_t)(1u << 5); /* C01_ELEMENT_CORRIDOR */
    }
}

static void test_put_le16(uint8_t *buf, int offset, uint16_t value)
{
    buf[offset + 0] = (uint8_t)(value & 0xffu);
    buf[offset + 1] = (uint8_t)((value >> 8) & 0xffu);
}

static uint16_t test_get_le16(const uint8_t *buf, int offset)
{
    return (uint16_t)(buf[offset + 0] | ((uint16_t)buf[offset + 1] << 8));
}

static int real_format_square_offset(int x, int y)
{
    return x * 3 + y;
}

static void make_c38_giggler_steal_fixture(CSB_V1_RuntimeProfile *profile,
                                           CSB_V1_DungeonData *dungeon,
                                           uint8_t *raw,
                                           size_t raw_size,
                                           uint32_t game_time)
{
    struct DM1_Event_V1 ev;
    int i;

    make_real_format_square_event_dungeon(dungeon, raw, raw_size);
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[4] = 82;
    dungeon->thing_type_counts[4] = 1;
    dungeon->thing_data_bases[5] = 98;
    dungeon->thing_type_counts[5] = 2;
    raw[real_format_square_offset(1, 1)] = (uint8_t)((1u << 5) | 0x10u);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 66, (uint16_t)(4u << 10));
    test_put_le16(raw, 82, 0xfffeu);
    test_put_le16(raw, 84, 0xfffeu);
    raw[86] = 2u;     /* C02 Giggler */
    raw[87] = 0xffu;  /* single centered creature */
    test_put_le16(raw, 88, 39u);
    test_put_le16(raw, 96, 6u); /* C6 attack, one creature */
    test_put_le16(raw, 98, 0xfffeu);
    test_put_le16(raw, 102, 0xfffeu);

    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_seed = 0xC5B10742u;
    profile->dungeon_handle = dungeon;
    profile->current_level = 0;
    profile->party_x = 1;
    profile->party_y = 2;
    profile->game_time = game_time;
    profile->timeline_queue.gameTick = game_time;
    profile->champion_count = 1;
    profile->party_state_valid = 1;
    profile->party_state.ChampionCount = 1;
    profile->party_state.LeaderIndex = 0;
    profile->leader_index = 0;
    profile->party_state.Champions[0].CurrentHealth = 500;
    profile->party_state.Champions[0].MaximumHealth = 500;
    profile->party_state.Champions[0].Attributes = 0;
    profile->party_state.Champions[0].Cell = 0;
    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        profile->party_state.Champions[0].Statistics[i][CSB_V1_STAT_MIN] = 30;
        profile->party_state.Champions[0].Statistics[i][CSB_V1_STAT_CUR] = 30;
        profile->party_state.Champions[0].Statistics[i][CSB_V1_STAT_MAX] = 30;
    }
    profile->party_state.Champions[0]
        .Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_CUR] = 1;
    for (i = 0; i < CSB_V1_SLOT_COUNT; ++i) {
        profile->party_state.Champions[0].Slots[i] = 0xffffu;
    }
    profile->party_state.Champions[0].Slots[CSB_V1_SLOT_READY_HAND] =
        (uint16_t)(5u << 10);
    profile->party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
        (uint16_t)((5u << 10) | 1u);

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    ev.map_time = DM1_MAP_TIME_MAKE(0, game_time);
    ev.b_mapX = 1;
    ev.b_mapY = 1;
    (void)csb_v1_runtime_add_timeline_event(profile, &ev);
}

static void queue_square_event(CSB_V1_RuntimeProfile *profile,
                               int event_type,
                               int effect,
                               int x,
                               int y)
{
    struct DM1_Event_V1 ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = (uint8_t)event_type;
    ev.map_time = DM1_MAP_TIME_MAKE(0, profile ? profile->game_time : 0);
    ev.b_mapX = (uint8_t)x;
    ev.b_mapY = (uint8_t)y;
    ev.c_effect = (uint8_t)effect;
    CHECK(csb_v1_runtime_add_timeline_event(profile, &ev) >= 0,
          "CSB runtime accepts a queued square event");
}

static void queue_square_cell_event(CSB_V1_RuntimeProfile *profile,
                                    int event_type,
                                    int effect,
                                    int x,
                                    int y,
                                    int cell)
{
    struct DM1_Event_V1 ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = (uint8_t)event_type;
    ev.map_time = DM1_MAP_TIME_MAKE(0, profile ? profile->game_time : 0);
    ev.b_mapX = (uint8_t)x;
    ev.b_mapY = (uint8_t)y;
    ev.c_cell = (uint8_t)cell;
    ev.c_effect = (uint8_t)effect;
    CHECK(csb_v1_runtime_add_timeline_event(profile, &ev) >= 0,
          "CSB runtime accepts a queued square event");
}

static void queue_explosion_advance_event(CSB_V1_RuntimeProfile *profile,
                                          const struct TimelineEvent_Compat *timeline_event)
{
    struct DM1_Event_V1 ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_EXPLOSION;
    ev.map_time = DM1_MAP_TIME_MAKE(
        timeline_event ? timeline_event->mapIndex : 0,
        timeline_event ? timeline_event->fireAtTick : 0);
    ev.priority = (uint8_t)(timeline_event ? timeline_event->aux0 : 0);
    ev.b_mapX = (uint8_t)(timeline_event ? timeline_event->mapX : 0);
    ev.b_mapY = (uint8_t)(timeline_event ? timeline_event->mapY : 0);
    ev.c_cell = (uint8_t)(timeline_event ? timeline_event->cell : 0);
    ev.c_effect = (uint8_t)(timeline_event ? timeline_event->aux1 : 0);
    CHECK(csb_v1_runtime_add_timeline_event(profile, &ev) >= 0,
          "CSB runtime accepts a queued C25 explosion event");
}

static void queue_projectile_move_event(CSB_V1_RuntimeProfile *profile,
                                        const struct TimelineEvent_Compat *timeline_event)
{
    struct DM1_Event_V1 ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_MOVE_PROJECTILE;
    ev.map_time = DM1_MAP_TIME_MAKE(
        timeline_event ? timeline_event->mapIndex : 0,
        timeline_event ? timeline_event->fireAtTick : 0);
    ev.priority = (uint8_t)(timeline_event ? timeline_event->aux0 : 0);
    ev.b_mapX = (uint8_t)(timeline_event ? timeline_event->mapX : 0);
    ev.b_mapY = (uint8_t)(timeline_event ? timeline_event->mapY : 0);
    ev.c_cell = (uint8_t)(timeline_event ? timeline_event->cell : 0);
    ev.c_effect = (uint8_t)(timeline_event ? timeline_event->aux3 : 0);
    CHECK(csb_v1_runtime_add_timeline_event(profile, &ev) >= 0,
          "CSB runtime accepts a queued C49 projectile event");
}

static void queue_future_creature_event(CSB_V1_RuntimeProfile *profile,
                                        int event_type,
                                        int x,
                                        int y,
                                        uint32_t fire_at_tick)
{
    struct DM1_Event_V1 ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = (uint8_t)event_type;
    ev.map_time = DM1_MAP_TIME_MAKE(0, fire_at_tick);
    ev.priority = 17u;
    ev.b_mapX = (uint8_t)x;
    ev.b_mapY = (uint8_t)y;
    CHECK(csb_v1_runtime_add_timeline_event(profile, &ev) >= 0,
          "CSB runtime accepts a queued future creature event");
}

static int find_queued_event_type(const CSB_V1_RuntimeProfile *profile,
                                  int event_type)
{
    int i;

    if (!profile) return -1;
    for (i = 0; i < profile->timeline_queue.eventCount; ++i) {
        int event_index = profile->timeline_queue.timeline[i];
        if (event_index >= 0 &&
            event_index < DM1_EVENT_MAX_COUNT &&
            profile->timeline_queue.events[event_index].type ==
                (uint8_t)event_type) {
            return event_index;
        }
    }
    return -1;
}

static int count_queued_event_type(const CSB_V1_RuntimeProfile *profile,
                                   int event_type)
{
    int i;
    int count = 0;

    if (!profile) return 0;
    for (i = 0; i < profile->timeline_queue.eventCount; ++i) {
        int event_index = profile->timeline_queue.timeline[i];
        if (event_index >= 0 &&
            event_index < DM1_EVENT_MAX_COUNT &&
            profile->timeline_queue.events[event_index].type ==
                (uint8_t)event_type) {
            count++;
        }
    }
    return count;
}

static int find_live_explosion_type(const CSB_V1_RuntimeProfile *profile,
                                    int explosion_type)
{
    int i;

    if (!profile) return -1;
    for (i = 0; i < EXPLOSION_LIST_CAPACITY; ++i) {
        if (profile->explosions.entries[i].reserved0 != 0 &&
            profile->explosions.entries[i].explosionType == explosion_type) {
            return i;
        }
    }
    return -1;
}

static uint32_t expected_c38_seed(uint32_t game_time,
                                  int map_x,
                                  int map_y,
                                  int creature_type,
                                  int creature_index,
                                  int champion_index)
{
    uint32_t seed;

    seed = 0xC5B1C038u ^
           (game_time * 1103515245u) ^
           ((uint32_t)(map_x & 0xFF) << 24) ^
           ((uint32_t)(map_y & 0xFF) << 16) ^
           ((uint32_t)(creature_type & 0xFF) << 8) ^
           (uint32_t)((creature_index & 0x03) |
                      ((champion_index & 0x03) << 2));
    return (seed != 0u) ? seed : 1u;
}

static int expected_c38_shared_combat_damage(
    const CSB_V1_Champion *champion,
    int champion_index,
    uint32_t game_time,
    int map_x,
    int map_y,
    int creature_type,
    int creature_index,
    int *out_wounds,
    int *out_poison)
{
    const struct CreatureBehaviorProfile_Compat *profile;
    struct CombatantCreatureSnapshot_Compat attacker;
    struct CombatantChampionSnapshot_Compat defender;
    struct CombatResult_Compat result;
    struct RngState_Compat rng;
    int parry_level;

    profile = CREATURE_GetProfile_Compat(creature_type);
    if (out_wounds) *out_wounds = 0;
    if (out_poison) *out_poison = 0;
    if (!champion || !profile) return -1;

    memset(&attacker, 0, sizeof(attacker));
    attacker.creatureType = creature_type;
    attacker.attack = profile->baseAttack;
    attacker.defense = profile->baseDefense;
    attacker.dexterity = profile->dexterity;
    attacker.baseHealth = profile->baseHealth;
    attacker.poisonAttack = profile->poisonAttack;
    attacker.attackType = profile->attackType;
    attacker.attributes = profile->attributes;
    attacker.woundProbabilities = profile->woundProbabilities;
    attacker.properties = profile->properties;
    attacker.creatureIndex = creature_index;

    memset(&defender, 0, sizeof(defender));
    defender.championIndex = champion_index;
    defender.currentHealth = champion->CurrentHealth;
    defender.dexterity =
        (int)champion->Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_CUR];
    defender.statisticVitality =
        (int)champion->Statistics[CSB_V1_STAT_VIT][CSB_V1_STAT_CUR];
    defender.statisticAntifire =
        (int)champion->Statistics[CSB_V1_STAT_ANTIFIRE][CSB_V1_STAT_CUR];
    defender.statisticAntimagic =
        (int)champion->Statistics[CSB_V1_STAT_ANTIMAGIC][CSB_V1_STAT_CUR];
    defender.statisticWisdom =
        (int)champion->Statistics[CSB_V1_STAT_WIS][CSB_V1_STAT_CUR];
    defender.statisticLuck =
        (int)champion->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_CUR];
    defender.statisticLuckMax =
        (int)champion->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_MAX];
    defender.statisticLuckMin =
        (int)champion->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_MIN];
    parry_level = (int)champion->Skills[7];
    if (parry_level < 1) parry_level = 1;
    if (parry_level > 16) parry_level = 16;
    defender.skillLevelParry = parry_level;

    (void)F0730_COMBAT_RngInit_Compat(
        &rng,
        expected_c38_seed(
            game_time,
            map_x,
            map_y,
            creature_type,
            creature_index,
            champion_index));
    if (!F0736_COMBAT_ResolveCreatureMelee_Compat(
            &attacker,
            &defender,
            &rng,
            &result)) {
        return -1;
    }
    if (out_wounds) *out_wounds = result.woundMaskAdded;
    if (out_poison) *out_poison = result.poisonAttackPending;
    return (result.outcome == COMBAT_OUTCOME_HIT_DAMAGE &&
            result.damageApplied > 0)
        ? result.damageApplied
        : 0;
}

static void test_timeline_square_events_mutate_real_format_map_bytes(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[9];
    struct DM1_TickDispatchResult_V1 dispatch;

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    raw[real_format_square_offset(1, 0)] = (uint8_t)((4u << 5) | 4u);

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;

    /* ReDMCSB TIMELINE.C F0261 dispatches C10 doors to F0244, which routes
     * into F0241 door animation; F0241 lines 754-809 steps the door-state
     * nibble and requeues until fully open/closed. */
    queue_square_event(&profile, DM1_EVENT_DOOR, DM1_EFFECT_SET, 1, 0);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "door square event fires on the current pre-increment game time");
    CHECK((raw[real_format_square_offset(1, 0)] & 0x07u) == 3u,
          "SET door event steps a closed door state 4 -> 3");
    CHECK(profile.timeline_queue.eventCount == 1,
          "partly opened door schedules a follow-up animation event");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 1,
          "door event remains visible in last timeline dispatch result");
    CHECK(dispatch.records[0].dispatchKind == DM1_DISPATCH_SQUARE_EFFECT &&
              dispatch.records[0].eventType == DM1_EVENT_DOOR,
          "door square event dispatches through the C10 square-effect boundary");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "first follow-up door animation tick fires");
    CHECK((raw[real_format_square_offset(1, 0)] & 0x07u) == 2u,
          "door animation follow-up steps state 3 -> 2");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "second follow-up door animation tick fires");
    CHECK((raw[real_format_square_offset(1, 0)] & 0x07u) == 1u,
          "door animation follow-up steps state 2 -> 1");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "third follow-up door animation tick fires");
    CHECK((raw[real_format_square_offset(1, 0)] & 0x07u) == 0u,
          "door animation follow-up reaches fully open state 0");
    CHECK(profile.timeline_queue.eventCount == 0,
          "fully opened door leaves no pending door-animation event");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    raw[real_format_square_offset(0, 1)] = (uint8_t)(6u << 5);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_event(&profile, DM1_EVENT_FAKEWALL, DM1_EFFECT_SET, 0, 1);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "fakewall square event fires on the current tick");
    CHECK((raw[real_format_square_offset(0, 1)] & 0x04u) == 0x04u,
          "fakewall SET toggles the open/active square flag");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    raw[real_format_square_offset(1, 1)] = (uint8_t)((5u << 5) | 0x08u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_event(&profile, DM1_EVENT_TELEPORTER, DM1_EFFECT_TOGGLE, 1, 1);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "teleporter square event fires on the current tick");
    CHECK((raw[real_format_square_offset(1, 1)] & 0x08u) == 0u,
          "teleporter TOGGLE clears an already-open square flag");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    raw[real_format_square_offset(2, 1)] = (uint8_t)(2u << 5);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_event(&profile, DM1_EVENT_PIT, DM1_EFFECT_TOGGLE, 2, 1);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "pit square event fires on the current tick");
    CHECK((raw[real_format_square_offset(2, 1)] & 0x08u) == 0x08u,
          "pit TOGGLE sets a closed square's open flag");
}

static void make_real_format_sensor_dungeon(CSB_V1_DungeonData *dungeon,
                                            uint8_t *raw,
                                            size_t raw_size,
                                            int sensor_x,
                                            int sensor_y,
                                            uint8_t sensor_square,
                                            uint16_t sensor_type_data,
                                            uint16_t sensor_flags,
                                            uint16_t sensor_target)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->level_offsets[0] = 0;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = 68;
    dungeon->thing_type_counts[3] = 1;

    raw[real_format_square_offset(sensor_x, sensor_y)] =
        (uint8_t)(sensor_square | 0x10u);
    test_put_le16(raw, 60 + sensor_x * 2, 0);
    test_put_le16(raw, 66, (uint16_t)(3u << 10));
    test_put_le16(raw, 68, 0xfffeu);
    test_put_le16(raw, 70, sensor_type_data);
    test_put_le16(raw, 72, sensor_flags);
    test_put_le16(raw, 74, sensor_target);
}

static uint16_t make_sensor_target(int x, int y, int cell)
{
    return (uint16_t)(((cell & 3) << 4) |
                      ((x & 0x1f) << 6) |
                      ((y & 0x1f) << 11));
}

static void make_real_format_wall_text_dungeon(CSB_V1_DungeonData *dungeon,
                                               uint8_t *raw,
                                               size_t raw_size,
                                               int text_cell,
                                               uint16_t text_word)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->level_offsets[0] = 0;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[2] = 68;
    dungeon->thing_type_counts[2] = 1;

    raw[real_format_square_offset(0, 0)] = (uint8_t)((0u << 5) | 0x10u);
    test_put_le16(raw, 60, 0);
    test_put_le16(raw, 66,
                  (uint16_t)(((text_cell & 3) << 14) | (2u << 10)));
    test_put_le16(raw, 68, 0xfffeu);
    test_put_le16(raw, 70, text_word);
}

static void make_real_format_corridor_text_generator_dungeon(
    CSB_V1_DungeonData *dungeon,
    uint8_t *raw,
    size_t raw_size)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->level_offsets[0] = 0;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 2;
    dungeon->thing_data_bases[2] = 70;
    dungeon->thing_type_counts[2] = 1;
    dungeon->thing_data_bases[3] = 74;
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[4] = 82;
    dungeon->thing_type_counts[4] = 1;

    raw[real_format_square_offset(1, 0)] = (uint8_t)((1u << 5) | 0x10u);
    raw[real_format_square_offset(1, 1)] = (uint8_t)((1u << 5) | 0x10u);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 66, (uint16_t)(2u << 10));
    test_put_le16(raw, 68, 0xfffeu);
    test_put_le16(raw, 70, (uint16_t)(3u << 10));
    test_put_le16(raw, 72, 0x0000u);
    test_put_le16(raw, 74, 0xfffeu);
    test_put_le16(raw, 76, (uint16_t)((9u << 7) | 6u));
    test_put_le16(raw, 78, (uint16_t)((1u << 7) | (1u << 6)));
    test_put_le16(raw, 80, (uint16_t)((3u << 4) | 5u));
    test_put_le16(raw, 82, 0xffffu);
}

static void test_timeline_corridor_text_and_generator_mutations(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[192];
    struct DM1_TickDispatchResult_V1 dispatch;
    uint16_t text_word;
    uint16_t type_data;
    uint16_t group_flags;
    uint32_t c38_dispatch_time;
    int c33_event_index;
    int c38_event_index;
    int expected_c38_damage;
    int expected_c38_wounds;
    int expected_c38_poison;
    int i;

    make_real_format_corridor_text_generator_dungeon(
        &dungeon,
        raw,
        sizeof(raw));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    profile.party_x = 1;
    profile.party_y = 2;
    profile.champion_count = 2;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 2;
    profile.party_state.LeaderIndex = 0;
    profile.leader_index = 0;
    profile.party_state.Champions[0].CurrentHealth = 20;
    profile.party_state.Champions[0].MaximumHealth = 20;
    profile.party_state.Champions[0].Attributes = 0;
    profile.party_state.Champions[0].Cell = 2;
    profile.party_state.Champions[1].CurrentHealth = 200;
    profile.party_state.Champions[1].MaximumHealth = 200;
    profile.party_state.Champions[1].Attributes = 0;
    profile.party_state.Champions[1].Cell = 0;
    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        profile.party_state.Champions[0].Statistics[i][CSB_V1_STAT_MIN] = 30;
        profile.party_state.Champions[0].Statistics[i][CSB_V1_STAT_CUR] = 30;
        profile.party_state.Champions[0].Statistics[i][CSB_V1_STAT_MAX] = 30;
        profile.party_state.Champions[1].Statistics[i][CSB_V1_STAT_MIN] = 30;
        profile.party_state.Champions[1].Statistics[i][CSB_V1_STAT_CUR] = 30;
        profile.party_state.Champions[1].Statistics[i][CSB_V1_STAT_MAX] = 30;
    }
    profile.party_state.Champions[1].Skills[7] = 8;

    queue_square_event(
        &profile,
        DM1_EVENT_CORRIDOR,
        DM1_EFFECT_SET,
        1,
        0);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C05 corridor event fires on the current tick");
    text_word = (uint16_t)(raw[72] | ((uint16_t)raw[73] << 8));
    CHECK((text_word & 0x0001u) == 0x0001u,
          "C05 corridor SET marks textstring visible");
    type_data = (uint16_t)(raw[76] | ((uint16_t)raw[77] << 8));
    CHECK((type_data & 0x007fu) == 0u && (type_data >> 7) == 9u,
          "C05 C006 generator disables sensor while preserving creature data");
    CHECK((uint16_t)(raw[66] | ((uint16_t)raw[67] << 8)) == (uint16_t)(4u << 10),
          "C05 C006 generator links the materialized group at the square head");
    CHECK((uint16_t)(raw[82] | ((uint16_t)raw[83] << 8)) == (uint16_t)(2u << 10),
          "materialized group preserves the previous textstring chain as Next");
    CHECK(raw[86] == 9u,
          "materialized group writes creature type from sensor data");
    CHECK(raw[87] == 0xffu,
          "single generated creature uses the source centered group cell marker");
    CHECK((uint16_t)(raw[88] | ((uint16_t)raw[89] << 8)) > 0u,
          "materialized group writes generated creature health");
    group_flags = (uint16_t)(raw[96] | ((uint16_t)raw[97] << 8));
    CHECK(((group_flags >> 5) & 0x03u) == 0u,
          "materialized single-creature group stores source 0-based count");
    CHECK(profile.timeline_queue.eventCount == 2,
          "C05 C006 generator schedules C37 wander plus delayed C65 re-enable");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "first post-generator tick dispatches the generated group's C37 event");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 1 &&
              dispatch.count == 1 &&
              dispatch.records[0].eventType == DM1_EVENT_UPDATE_BEHAVIOR_GROUP &&
              dispatch.records[0].dispatchKind == DM1_DISPATCH_CREATURE_AI &&
              dispatch.records[0].mapX == 1 &&
              dispatch.records[0].mapY == 0,
          "generated group C37 dispatch keeps source square and creature-AI kind");
    CHECK(dispatch.records[0].aux0 == (255 - 21),
          "generated group C37 priority follows 255 - creature movement ticks");
    group_flags = (uint16_t)(raw[96] | ((uint16_t)raw[97] << 8));
    CHECK((group_flags & 0x000fu) == 7u,
          "generated group C37 mutates visible wander behavior to approach");
    CHECK(profile.timeline_queue.eventCount == 2,
          "C37 approach behavior queues the next group behavior tick beside C65");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "second post-generator tick dispatches the queued approach C37");
    CHECK((uint16_t)(raw[66] | ((uint16_t)raw[67] << 8)) == (uint16_t)(2u << 10),
          "approach C37 unlinks the group from the source corridor square");
    CHECK((uint16_t)(raw[68] | ((uint16_t)raw[69] << 8)) == (uint16_t)(4u << 10),
          "approach C37 links the group at the next square toward the party");
    CHECK((uint16_t)(raw[82] | ((uint16_t)raw[83] << 8)) == 0xfffeu,
          "moved group preserves the destination's previous thing chain");
    CHECK(profile.timeline_queue.eventCount == 2,
          "C65 and the future approach C37 remain queued after movement");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "third post-generator tick reaches the delayed C65 boundary");
    type_data = (uint16_t)(raw[76] | ((uint16_t)raw[77] << 8));
    CHECK((type_data & 0x007fu) == 6u && (type_data >> 7) == 9u,
          "delayed C65 re-enables the generator sensor and preserves data");
    CHECK(profile.timeline_queue.eventCount == 1,
          "delayed C65 consumes itself while the future C37 remains queued");
    for (i = 0; i < 20; ++i) {
        CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
              "advance toward the future adjacent approach C37 boundary");
    }
    group_flags = (uint16_t)(raw[96] | ((uint16_t)raw[97] << 8));
    CHECK((group_flags & 0x000fu) == 6u,
          "adjacent C7 approach C37 mutates the generated group to attack");
    CHECK((uint16_t)(raw[68] | ((uint16_t)raw[69] << 8)) == (uint16_t)(4u << 10),
          "adjacent attack transition does not move the group onto the party square");
    CHECK(profile.timeline_queue.eventCount == 1,
          "adjacent attack transition queues one C38 creature attack event");
    c38_dispatch_time = profile.game_time;
    expected_c38_damage = expected_c38_shared_combat_damage(
        &profile.party_state.Champions[1],
        1,
        c38_dispatch_time,
        1,
        1,
        9,
        0,
        &expected_c38_wounds,
        &expected_c38_poison);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "next tick dispatches the bounded C38 attack event");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 1 &&
              dispatch.count == 1 &&
              dispatch.records[0].eventType == DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
              dispatch.records[0].dispatchKind == DM1_DISPATCH_CREATURE_AI &&
              dispatch.records[0].mapX == 1 &&
              dispatch.records[0].mapY == 1,
          "bounded C38 attack event keeps the adjacent group square");
    CHECK(dispatch.records[0].aux0 == (255 - 21),
          "bounded C38 attack event priority follows 255 - creature movement ticks");
    CHECK(expected_c38_damage > 0 &&
              profile.party_state.Champions[0].CurrentHealth == 20 &&
              profile.party_state.Champions[1].CurrentHealth ==
                  200 - expected_c38_damage,
          "bounded C38 attack event uses shared combat damage with imported PARRY level");
    CHECK(expected_c38_wounds != 0 &&
              profile.party_state.Champions[1].Wounds ==
              (uint16_t)expected_c38_wounds,
          "bounded C38 attack event applies shared combat wound mask");
    CHECK(expected_c38_poison == 0 &&
              profile.party_state.Champions[1].PoisonEventCount == 0,
          "bounded non-poison C38 attack leaves poison state clear");
    CHECK(profile.timeline_queue.eventCount == 1,
          "bounded C38 attack event queues the source C33 aspect wrapper");
    c33_event_index =
        find_queued_event_type(&profile, DM1_EVENT_UPDATE_ASPECT_CREATURE_0);
    CHECK(c33_event_index >= 0 &&
              DM1_MAP_TIME_TIME(
                  profile.timeline_queue.events[c33_event_index].map_time) ==
                  c38_dispatch_time + 1U &&
              profile.timeline_queue.events[c33_event_index].priority ==
                  (uint8_t)(255 - 21) &&
              profile.timeline_queue.events[c33_event_index].c_effect == 13u,
          "bounded C38 schedules C33 aspect update with F0208 remaining ticks");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "next tick dispatches the bounded C33 aspect wrapper");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 1 &&
              dispatch.count == 1 &&
              dispatch.records[0].eventType ==
                  DM1_EVENT_UPDATE_ASPECT_CREATURE_0 &&
              dispatch.records[0].effect == 13,
          "bounded C33 aspect dispatch carries remaining C38 ticks");
    CHECK(profile.timeline_queue.eventCount == 1,
          "bounded C33 aspect dispatch expands into the next C38 cadence");
    c38_event_index =
        find_queued_event_type(&profile, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0);
    CHECK(profile.timeline_queue.events[c38_event_index].type ==
              DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
              DM1_MAP_TIME_TIME(
                  profile.timeline_queue.events[c38_event_index].map_time) ==
                  c38_dispatch_time + 14U &&
              profile.timeline_queue.events[c38_event_index].priority ==
                  (uint8_t)(255 - 21),
          "bounded C38 requeue uses C09 AttackTicks and movement priority");
    profile.party_state.Champions[0].Cell = 0;
    profile.party_state.Champions[1].Cell = 2;
    profile.party_state.Champions[0].CurrentHealth = 2;
    profile.party_state.Champions[1].CurrentHealth = 20;
    queue_square_event(
        &profile,
        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0,
        DM1_EFFECT_SET,
        1,
        1);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "bounded C38 lethal follow-up dispatches on the adjacent group square");
    CHECK(profile.party_state.Champions[0].CurrentHealth == 0 &&
              (profile.party_state.Champions[0].Attributes &
               CSB_V1_CHAMPION_ATTRIBUTE_DEAD),
          "bounded lethal C38 marks the damaged champion dead");
    CHECK(profile.party_state.LeaderIndex == 1 && profile.leader_index == 1,
          "bounded lethal C38 moves leadership to the next living champion");
    CHECK(profile.party_state.Champions[1].CurrentHealth == 20,
          "bounded lethal C38 leaves the next living champion undamaged");
    profile.party_state.Champions[1].CurrentHealth = 2;
    queue_square_event(
        &profile,
        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0,
        DM1_EFFECT_SET,
        1,
        1);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "bounded C38 final knockout dispatches before game-over gating");
    CHECK(profile.party_state.Champions[1].CurrentHealth == 0 &&
              (profile.party_state.Champions[1].Attributes &
               CSB_V1_CHAMPION_ATTRIBUTE_DEAD),
          "bounded final C38 marks the last living champion dead");
    CHECK(profile.party_state.LeaderIndex == -1 && profile.leader_index == -1 &&
              profile.game_over == 1,
          "bounded final C38 clears leadership and marks CSB runtime game over");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 0,
          "game-over CSB runtime blocks further V1 ticks after final knockout");
}

static void test_c38_poison_followup_and_c75_tick(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[192];
    struct DM1_TickDispatchResult_V1 dispatch;
    int expected_damage;
    int expected_wounds;
    int expected_poison;
    int poison_event_index;
    int poison_damage;
    uint32_t poison_c38_time;
    int i;

    printf("\n-- CSB C38 poison follow-up and C75 tick --\n");

    make_real_format_corridor_text_generator_dungeon(
        &dungeon,
        raw,
        sizeof(raw));
    raw[real_format_square_offset(1, 1)] = (uint8_t)((1u << 5) | 0x10u);
    test_put_le16(raw, 68, (uint16_t)(4u << 10));
    test_put_le16(raw, 82, 0xfffeu);
    raw[86] = 13u;       /* C13 Couatl: poisonAttack=100 */
    raw[87] = 0xffu;
    test_put_le16(raw, 88, 39u);
    test_put_le16(raw, 96, 6u);  /* C6 attack, one creature */

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    profile.party_x = 1;
    profile.party_y = 2;
    profile.champion_count = 1;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 1;
    profile.party_state.LeaderIndex = 0;
    profile.leader_index = 0;
    profile.party_state.Champions[0].CurrentHealth = 500;
    profile.party_state.Champions[0].MaximumHealth = 500;
    profile.party_state.Champions[0].Attributes = 0;
    profile.party_state.Champions[0].Cell = 0;
    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        profile.party_state.Champions[0].Statistics[i][CSB_V1_STAT_MIN] = 30;
        profile.party_state.Champions[0].Statistics[i][CSB_V1_STAT_CUR] = 30;
        profile.party_state.Champions[0].Statistics[i][CSB_V1_STAT_MAX] = 30;
    }
    profile.party_state.Champions[0].Skills[7] = 16;

    poison_c38_time = 0;
    expected_damage = -1;
    expected_wounds = 0;
    expected_poison = 0;
    for (i = 0; i < 256; ++i) {
        expected_damage = expected_c38_shared_combat_damage(
            &profile.party_state.Champions[0],
            0,
            (uint32_t)i,
            1,
            1,
            13,
            0,
            &expected_wounds,
            &expected_poison);
        if (expected_damage > 0 && expected_poison > 1) {
            poison_c38_time = (uint32_t)i;
            break;
        }
    }
    CHECK(expected_damage > 0 && expected_poison > 1,
          "poison C38 fixture produces damage and a pending poison attack");
    profile.game_time = poison_c38_time;
    profile.timeline_queue.gameTick = poison_c38_time;
    poison_damage = expected_poison >> 6;
    if (poison_damage < 1) poison_damage = 1;

    queue_square_event(
        &profile,
        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0,
        DM1_EFFECT_SET,
        1,
        1);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "poison C38 attack dispatches on the adjacent group square");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 1 &&
              dispatch.count == 1 &&
              dispatch.records[0].eventType ==
                  DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0,
          "poison C38 dispatch remains a creature-AI event");
    CHECK(profile.party_state.Champions[0].CurrentHealth ==
              500 - expected_damage - poison_damage,
          "poison C38 applies melee damage plus immediate F0322 poison damage");
    CHECK(profile.party_state.Champions[0].PoisonDose ==
              (uint16_t)expected_poison &&
              profile.party_state.Champions[0].PoisonEventCount == 1u,
          "poison C38 stores poison dose and one pending C75 event");
    poison_event_index = find_queued_event_type(
        &profile,
        DM1_EVENT_POISON_CHAMPION);
    CHECK(poison_event_index >= 0 &&
              DM1_MAP_TIME_TIME(
                  profile.timeline_queue.events[poison_event_index].map_time) ==
                  poison_c38_time + 36u &&
              profile.timeline_queue.events[poison_event_index].priority == 0u &&
              profile.timeline_queue.events[poison_event_index].c_effect ==
                  (uint8_t)(expected_poison - 1),
          "poison C38 schedules C75 with Attack-1 after 36 ticks");

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.party_state_valid = 1;
    profile.champion_count = 1;
    profile.party_state.ChampionCount = 1;
    profile.party_state.LeaderIndex = 0;
    profile.leader_index = 0;
    profile.party_state.Champions[0].CurrentHealth = 50;
    profile.party_state.Champions[0].MaximumHealth = 50;
    profile.party_state.Champions[0].PoisonDose = 100;
    profile.party_state.Champions[0].PoisonEventCount = 1;
    queue_square_event(
        &profile,
        DM1_EVENT_POISON_CHAMPION,
        5,
        1,
        2);
    profile.timeline_queue.events[
        find_queued_event_type(&profile, DM1_EVENT_POISON_CHAMPION)]
        .priority = 0;
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C75 poison event dispatches on the current tick");
    CHECK(profile.party_state.Champions[0].CurrentHealth == 49,
          "C75 poison tick applies max(1, Attack >> 6) damage");
    CHECK(profile.party_state.Champions[0].PoisonDose == 105,
          "C75 poison tick accumulates the carried attack into poison dose");
    CHECK(profile.party_state.Champions[0].PoisonEventCount == 1u &&
              find_queued_event_type(&profile, DM1_EVENT_POISON_CHAMPION) >= 0,
          "C75 decrements the consumed event then reschedules Attack-1");
}

static int test_csb_giggler_group_slot_chain_contains(const uint8_t *raw,
                                                      uint16_t wanted_thing)
{
    uint16_t thing = test_get_le16(raw, 84);
    int guard;

    for (guard = 0; guard < 4 && thing != 0xfffeu && thing != 0xffffu;
         ++guard) {
        int type = (int)((thing & 0x3c00u) >> 10);
        int index = (int)(thing & 0x03ffu);

        if ((thing & 0x3fffu) == wanted_thing) return 1;
        if (type != 5 || index < 0 || index >= 2) break;
        thing = test_get_le16(raw, 98 + index * 4);
    }
    return 0;
}

static void test_c38_giggler_steals_hand_slots_into_group_slot_chain(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[160];
    const uint16_t ready_thing = (uint16_t)(5u << 10);
    const uint16_t action_thing = (uint16_t)((5u << 10) | 1u);
    int found = 0;
    uint32_t found_time = 0;
    uint32_t t;

    printf("\n-- CSB C38 Giggler hand-slot theft --\n");

    for (t = 0; t < 512u && !found; ++t) {
        make_c38_giggler_steal_fixture(
            &profile,
            &dungeon,
            raw,
            sizeof(raw),
            t);
        (void)csb_v1_runtime_tick_v1(&profile);
        if (profile.party_state.Champions[0]
                    .Slots[CSB_V1_SLOT_READY_HAND] == 0xffffu &&
            profile.party_state.Champions[0]
                    .Slots[CSB_V1_SLOT_ACTION_HAND] == 0xffffu &&
            test_csb_giggler_group_slot_chain_contains(raw, ready_thing) &&
            test_csb_giggler_group_slot_chain_contains(raw, action_thing)) {
            found = 1;
            found_time = t;
        }
    }

    CHECK(found,
          "C38 Giggler fixture finds a deterministic seed that steals both hand slots");
    CHECK(profile.party_state.Champions[0].CurrentHealth == 500,
          "C38 Giggler theft does not also apply normal melee damage");
    CHECK(test_get_le16(raw, 84) != 0xfffeu &&
              test_get_le16(raw, 84) != 0xffffu,
          "C38 Giggler theft writes a carried object into GROUP.Slot");
    CHECK(test_csb_giggler_group_slot_chain_contains(raw, ready_thing) &&
              test_csb_giggler_group_slot_chain_contains(raw, action_thing),
          "C38 Giggler theft links both stolen hand objects into the group slot chain");
    CHECK(found_time < 512u,
          "C38 Giggler theft search stays within the bounded regression window");
}

static void test_c37_group_approach_teleporter_rotation(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[180];
    struct DM1_Event_V1 ev;
    int event_index;
    uint16_t flags;

    printf("\n-- CSB C37 group chained teleporter movement --\n");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 4;
    dungeon.thing_data_bases[1] = 82;
    dungeon.thing_type_counts[1] = 2;
    dungeon.thing_data_bases[4] = 100;
    dungeon.thing_type_counts[4] = 1;
    raw[real_format_square_offset(0, 0)] =
        (uint8_t)((1u << 5) | 0x10u);
    raw[real_format_square_offset(0, 1)] =
        (uint8_t)((5u << 5) | 0x10u | 0x08u);
    raw[real_format_square_offset(1, 1)] =
        (uint8_t)((5u << 5) | 0x10u | 0x08u);
    raw[real_format_square_offset(2, 1)] =
        (uint8_t)((1u << 5) | 0x10u);
    test_put_le16(raw, 60 + 0 * 2, 0);
    test_put_le16(raw, 60 + 1 * 2, 2);
    test_put_le16(raw, 60 + 2 * 2, 3);
    test_put_le16(raw, 66, (uint16_t)(4u << 10));
    test_put_le16(raw, 68, (uint16_t)(1u << 10));
    test_put_le16(raw, 70, (uint16_t)((1u << 10) | 1u));
    test_put_le16(raw, 72, 0xfffeu);
    test_put_le16(raw, 82, 0xfffeu);
    test_put_le16(raw, 84,
                  (uint16_t)(1u | (1u << 5) | (1u << 10) |
                             (1u << 13)));
    test_put_le16(raw, 86, 0u);
    test_put_le16(raw, 88, 0xfffeu);
    test_put_le16(raw, 90,
                  (uint16_t)(2u | (1u << 5) | (1u << 10) |
                             (1u << 13)));
    test_put_le16(raw, 92, 0u);
    test_put_le16(raw, 100, 0xfffeu);
    test_put_le16(raw, 102, 0u);
    raw[104] = 9u;
    raw[105] = 0u;
    test_put_le16(raw, 106, 40u);
    test_put_le16(raw, 108, 0u);
    test_put_le16(raw, 110, 0u);
    test_put_le16(raw, 112, 0u);
    test_put_le16(raw, 114, 7u);

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    profile.party_x = 0;
    profile.party_y = 2;
    profile.champion_count = 1;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 1;
    profile.party_state.LeaderIndex = 0;
    profile.leader_index = 0;
    profile.party_state.Champions[0].CurrentHealth = 100;
    profile.party_state.Champions[0].MaximumHealth = 100;
    profile.party_state.Champions[0].Attributes = 0;
    profile.party_state.Champions[0].Cell = 0;

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    ev.map_time = DM1_MAP_TIME_MAKE(0, profile.game_time);
    ev.priority = 234u;
    ev.b_mapX = 0;
    ev.b_mapY = 0;
    CHECK(csb_v1_runtime_add_timeline_event(&profile, &ev) >= 0,
          "C37 group teleporter fixture queues the approach event");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C37 group chained teleporter fixture dispatches the approach event");
    CHECK(test_get_le16(raw, 66) == 0xfffeu,
          "C37 group chained teleporter removes the group from the source square");
    CHECK(test_get_le16(raw, 68) == (uint16_t)(1u << 10),
          "C37 group chained teleporter leaves the first C05 thing in place");
    CHECK(test_get_le16(raw, 70) == (uint16_t)((1u << 10) | 1u),
          "C37 group chained teleporter leaves the second C05 thing in place");
    CHECK(test_get_le16(raw, 72) == (uint16_t)(4u << 10),
          "C37 group chained teleporter links the group at the final target");
    CHECK(test_get_le16(raw, 100) == 0xfffeu,
          "C37 group chained teleporter terminates the moved group chain");
    flags = test_get_le16(raw, 114);
    CHECK(raw[105] == 2u && ((flags >> 8) & 0x03u) == 2u,
          "C37 group chained teleporter applies two F0262 relative rotations");
    event_index = find_queued_event_type(&profile,
                                         DM1_EVENT_UPDATE_BEHAVIOR_GROUP);
    CHECK(event_index >= 0 &&
              profile.timeline_queue.events[event_index].b_mapX == 2 &&
              profile.timeline_queue.events[event_index].b_mapY == 1,
          "C37 group chained teleporter requeues behavior from the final target");
}

static void test_explosion_c25_persistent_smoke_requeues_until_depleted(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[96];
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first_advance;
    int slot = -1;

    printf("\n-- CSB C25 persistent smoke requeue --\n");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;

    memset(&input, 0, sizeof(input));
    input.explosionType = C040_EXPLOSION_SMOKE;
    input.attack = 96;
    input.mapIndex = 0;
    input.mapX = 1;
    input.mapY = 1;
    input.cell = EXPLOSION_CELL_CENTERED;
    input.centered = 1;
    input.currentTick = 0;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    CHECK(F0821_EXPLOSION_Create_Compat(
              &input,
              &profile.explosions,
              &slot,
              &first_advance) == 1 &&
              slot == 0 &&
              first_advance.kind == TIMELINE_EVENT_EXPLOSION_ADVANCE,
          "CSB C25 smoke fixture creates a live persistent explosion");
    queue_explosion_advance_event(&profile, &first_advance);

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 smoke first boundary tick reaches the scheduled C25 time");
    CHECK(profile.explosions.count == 1 &&
              profile.explosions.entries[slot].attack == 96 &&
              count_queued_event_type(&profile, DM1_EVENT_EXPLOSION) == 1,
          "C25 smoke first boundary tick leaves the future event queued");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 smoke first advance dispatches at the scheduled boundary");
    CHECK(profile.explosions.count == 1 &&
              profile.explosions.entries[slot].attack == 56 &&
              profile.explosions.entries[slot].currentFrame == 1,
          "C25 smoke first advance decrements attack and writes back frame state");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_EXPLOSION) == 1,
          "C25 smoke first advance requeues the next C25 event");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 smoke second advance dispatches on the following boundary");
    CHECK(profile.explosions.count == 1 &&
              profile.explosions.entries[slot].attack == 16 &&
              profile.explosions.entries[slot].currentFrame == 2,
          "C25 smoke second advance decrements attack and keeps the slot live");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_EXPLOSION) == 1,
          "C25 smoke second advance requeues the final C25 event");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 smoke final advance dispatches after attack depletion");
    CHECK(profile.explosions.count == 0,
          "C25 smoke final advance despawns the depleted explosion");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_EXPLOSION) == 0,
          "C25 smoke final advance leaves no stale C25 event");
}

static void test_explosion_c25_party_damage_and_group_hp_writeback(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[192];
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first_advance;
    int slot = -1;
    uint16_t group_hp_before;
    uint16_t group_hp_after;
    uint16_t group_flags;
    int i;

    printf("\n-- CSB C25 party/group damage writeback --\n");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    profile.party_x = 1;
    profile.party_y = 1;
    profile.party_dir = 0;
    profile.champion_count = 2;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 2;
    profile.party_state.LeaderIndex = 0;
    profile.leader_index = 0;
    profile.party_state.Champions[0].CurrentHealth = 1;
    profile.party_state.Champions[0].MaximumHealth = 20;
    profile.party_state.Champions[0].Attributes = 0;
    profile.party_state.Champions[1].CurrentHealth = 500;
    profile.party_state.Champions[1].MaximumHealth = 500;
    profile.party_state.Champions[1].Attributes = 0;
    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        profile.party_state.Champions[0].Statistics[i][CSB_V1_STAT_MIN] = 30;
        profile.party_state.Champions[0].Statistics[i][CSB_V1_STAT_CUR] = 30;
        profile.party_state.Champions[0].Statistics[i][CSB_V1_STAT_MAX] = 30;
        profile.party_state.Champions[1].Statistics[i][CSB_V1_STAT_MIN] = 30;
        profile.party_state.Champions[1].Statistics[i][CSB_V1_STAT_CUR] = 30;
        profile.party_state.Champions[1].Statistics[i][CSB_V1_STAT_MAX] = 30;
    }

    memset(&input, 0, sizeof(input));
    input.explosionType = C000_EXPLOSION_FIREBALL;
    input.attack = 255;
    input.mapIndex = 0;
    input.mapX = 1;
    input.mapY = 1;
    input.cell = EXPLOSION_CELL_CENTERED;
    input.centered = 1;
    input.currentTick = 0;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    CHECK(F0821_EXPLOSION_Create_Compat(
              &input,
              &profile.explosions,
              &slot,
              &first_advance) == 1 &&
              slot == 0,
          "CSB C25 party fireball fixture creates an explosion slot");
    queue_explosion_advance_event(&profile, &first_advance);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 party fireball reaches the scheduled boundary");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 party fireball dispatches through the explosion handler");
    CHECK(profile.explosions.count == 0,
          "C25 party fireball despawns after one-shot advance");
    CHECK(profile.party_state.Champions[0].CurrentHealth == 0 &&
              (profile.party_state.Champions[0].Attributes &
               CSB_V1_CHAMPION_ATTRIBUTE_DEAD),
          "C25 party fireball applies lethal F0324-style damage");
    CHECK(profile.party_state.LeaderIndex == 1 && profile.leader_index == 1,
          "C25 party fireball moves leadership to the next living champion");
    CHECK(profile.party_state.Champions[1].CurrentHealth < 500 &&
              profile.party_state.Champions[1].CurrentHealth > 0,
          "C25 party fireball damages the other living champion");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[4] = 82;
    dungeon.thing_type_counts[4] = 1;
    raw[real_format_square_offset(1, 1)] = (uint8_t)((1u << 5) | 0x10u);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 66, (uint16_t)(4u << 10));
    test_put_le16(raw, 82, 0xfffeu);
    test_put_le16(raw, 84, 0xfffeu);
    raw[86] = 9u;
    raw[87] = 0xffu;
    test_put_le16(raw, 88, 300u);
    test_put_le16(raw, 96, 0u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    profile.party_x = 0;
    profile.party_y = 0;

    memset(&input, 0, sizeof(input));
    input.explosionType = C000_EXPLOSION_FIREBALL;
    input.attack = 160;
    input.mapIndex = 0;
    input.mapX = 1;
    input.mapY = 1;
    input.cell = EXPLOSION_CELL_CENTERED;
    input.centered = 1;
    input.currentTick = 0;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    slot = -1;
    group_hp_before = (uint16_t)(raw[88] | ((uint16_t)raw[89] << 8));
    CHECK(F0821_EXPLOSION_Create_Compat(
              &input,
              &profile.explosions,
              &slot,
              &first_advance) == 1 &&
              slot == 0,
          "CSB C25 group fireball fixture creates an explosion slot");
    queue_explosion_advance_event(&profile, &first_advance);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 group fireball reaches the scheduled boundary");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 group fireball dispatches through the explosion handler");
    group_hp_after = (uint16_t)(raw[88] | ((uint16_t)raw[89] << 8));
    CHECK(profile.explosions.count == 0,
          "C25 group fireball despawns after one-shot advance");
    CHECK(group_hp_after < group_hp_before,
          "C25 group fireball writes damage into real-format GROUP.Health");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[4] = 82;
    dungeon.thing_type_counts[4] = 1;
    dungeon.thing_data_bases[6] = 98;
    dungeon.thing_type_counts[6] = 4;
    dungeon.thing_data_bases[5] = 114;
    dungeon.thing_type_counts[5] = 3;
    raw[real_format_square_offset(1, 1)] = (uint8_t)((1u << 5) | 0x10u);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 66, (uint16_t)(4u << 10));
    test_put_le16(raw, 82, 0xfffeu);
    test_put_le16(raw, 84, 0xfffeu);
    raw[86] = 18u;      /* Animated Armour: six fixed possession drops */
    raw[87] = 0x04u; /* slot0 cell0, slot1 cell1 */
    test_put_le16(raw, 88, 1u);
    test_put_le16(raw, 90, 500u);
    test_put_le16(raw, 96, (uint16_t)((1u << 5) | 6u)); /* two C6 creatures */
    for (i = 0; i < 4; ++i) {
        test_put_le16(raw, 98 + i * 4, 0xffffu);
    }
    for (i = 0; i < 3; ++i) {
        test_put_le16(raw, 114 + i * 4, 0xffffu);
    }
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_seed = 0xC5B10740u;
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    queue_future_creature_event(
        &profile,
        DM1_EVENT_UPDATE_ASPECT_CREATURE_0,
        1,
        1,
        100u);
    queue_future_creature_event(
        &profile,
        DM1_EVENT_UPDATE_ASPECT_CREATURE_1,
        1,
        1,
        100u);
    queue_future_creature_event(
        &profile,
        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0,
        1,
        1,
        100u);
    queue_future_creature_event(
        &profile,
        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_1,
        1,
        1,
        100u);

    memset(&input, 0, sizeof(input));
    input.explosionType = C000_EXPLOSION_FIREBALL;
    input.attack = 80;
    input.mapIndex = 0;
    input.mapX = 1;
    input.mapY = 1;
    input.cell = EXPLOSION_CELL_CENTERED;
    input.centered = 1;
    input.currentTick = 0;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    slot = -1;
    CHECK(F0821_EXPLOSION_Create_Compat(
              &input,
              &profile.explosions,
              &slot,
              &first_advance) == 1 &&
              slot == 0,
          "CSB C25 two-creature group fixture creates an explosion slot");
    queue_explosion_advance_event(&profile, &first_advance);
    CHECK(count_queued_event_type(&profile, DM1_EVENT_UPDATE_ASPECT_CREATURE_0) == 1 &&
              count_queued_event_type(&profile, DM1_EVENT_UPDATE_ASPECT_CREATURE_1) == 1 &&
              count_queued_event_type(&profile, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0) == 1 &&
              count_queued_event_type(&profile, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_1) == 1,
          "C25 two-creature fixture starts with queued aspect and attack events");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 two-creature group reaches the scheduled boundary");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 two-creature group dispatches through the explosion handler");
    group_flags = (uint16_t)(raw[96] | ((uint16_t)raw[97] << 8));
    group_hp_after = (uint16_t)(raw[88] | ((uint16_t)raw[89] << 8));
    CHECK(((group_flags >> 5) & 0x03u) == 0u,
          "C25 group kill compacts real-format Count from two creatures to one");
    CHECK(group_hp_after > 0u && group_hp_after < 500u &&
              (uint16_t)(raw[90] | ((uint16_t)raw[91] << 8)) == 0u,
          "C25 group kill packs surviving Health down to slot 0");
    CHECK((raw[87] & 0x03u) == 1u,
          "C25 group kill packs surviving cell down to slot 0");
    CHECK((uint16_t)(raw[82] | ((uint16_t)raw[83] << 8)) != 0xfffeu &&
              ((uint16_t)(raw[82] | ((uint16_t)raw[83] << 8)) & 0x3c00u) ==
              (uint16_t)(6u << 10),
          "C25 group partial kill appends first fixed armour drop after group");
    CHECK((uint16_t)(raw[100] | ((uint16_t)raw[101] << 8)) == 0x0129u,
          "C25 group partial kill materializes cursed animated-armour foot plate");
    CHECK((uint16_t)(raw[116] | ((uint16_t)raw[117] << 8)) == 0x010au,
          "C25 group partial kill materializes cursed animated-armour sword");
    slot = find_live_explosion_type(&profile, C040_EXPLOSION_SMOKE);
    CHECK(slot >= 0 &&
              profile.explosions.entries[slot].attack == 110 &&
              profile.explosions.entries[slot].mapX == 1 &&
              profile.explosions.entries[slot].mapY == 1 &&
              profile.explosions.entries[slot].cell == 0,
          "C25 group kill creates F0190 death smoke at killed creature cell");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_EXPLOSION) == 1,
          "C25 group kill schedules the F0190 smoke advance event");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_UPDATE_ASPECT_CREATURE_0) == 1 &&
              count_queued_event_type(&profile, DM1_EVENT_UPDATE_ASPECT_CREATURE_1) == 0 &&
              count_queued_event_type(&profile, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0) == 1 &&
              count_queued_event_type(&profile, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_1) == 0,
          "C25 group kill deletes killed-slot events and reindexes survivor events");
    CHECK(find_queued_event_type(&profile, DM1_EVENT_UPDATE_ASPECT_CREATURE_0) >= 0 &&
              DM1_MAP_TIME_TIME(
                  profile.timeline_queue.events[
                      find_queued_event_type(
                          &profile,
                          DM1_EVENT_UPDATE_ASPECT_CREATURE_0)].map_time) == 100u &&
              find_queued_event_type(&profile, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0) >= 0 &&
              DM1_MAP_TIME_TIME(
                  profile.timeline_queue.events[
                      find_queued_event_type(
                          &profile,
                          DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0)].map_time) == 100u,
          "C25 group kill preserves future timing for reindexed survivor events");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[4] = 82;
    dungeon.thing_type_counts[4] = 1;
    dungeon.thing_data_bases[5] = 98;
    dungeon.thing_type_counts[5] = 1;
    raw[real_format_square_offset(1, 1)] = (uint8_t)((1u << 5) | 0x10u);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 66, (uint16_t)(4u << 10));
    test_put_le16(raw, 82, 0xfffeu);
    test_put_le16(raw, 84, (uint16_t)(5u << 10)); /* carried weapon slot */
    raw[86] = 9u;
    raw[87] = 0xffu;
    test_put_le16(raw, 88, 1u);
    test_put_le16(raw, 96, 0u);
    test_put_le16(raw, 98, 0xfffeu);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_seed = 0xC5B10738u;
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;

    memset(&input, 0, sizeof(input));
    input.explosionType = C000_EXPLOSION_FIREBALL;
    input.attack = 160;
    input.mapIndex = 0;
    input.mapX = 1;
    input.mapY = 1;
    input.cell = EXPLOSION_CELL_CENTERED;
    input.centered = 1;
    input.currentTick = 0;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    slot = -1;
    CHECK(F0821_EXPLOSION_Create_Compat(
              &input,
              &profile.explosions,
              &slot,
              &first_advance) == 1 &&
              slot == 0,
          "CSB C25 final-creature group fixture creates an explosion slot");
    queue_explosion_advance_event(&profile, &first_advance);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 final-creature group reaches the scheduled boundary");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 final-creature group dispatches through the explosion handler");
    CHECK(((uint16_t)(raw[66] | ((uint16_t)raw[67] << 8)) & 0x3fffu) ==
              (uint16_t)(5u << 10),
          "C25 final group kill drops carried slot thing onto the square head");
    CHECK((uint16_t)(raw[82] | ((uint16_t)raw[83] << 8)) == 0xffffu,
          "C25 final group kill marks the real-format group record unused");
    CHECK((uint16_t)(raw[84] | ((uint16_t)raw[85] << 8)) == 0xfffeu,
          "C25 final group kill clears the real-format group Slot chain");
    CHECK((uint16_t)(raw[98] | ((uint16_t)raw[99] << 8)) == 0xfffeu,
          "C25 final group kill terminates the dropped carried thing chain");
    slot = find_live_explosion_type(&profile, C040_EXPLOSION_SMOKE);
    CHECK(slot >= 0 &&
              profile.explosions.entries[slot].attack == 255 &&
              profile.explosions.entries[slot].cell == EXPLOSION_CELL_CENTERED,
          "C25 final group kill creates centered F0190 death smoke");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_EXPLOSION) == 1,
          "C25 final group kill schedules the centered death-smoke advance");
}

static void test_explosion_c25_door_destruction_writeback(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[9];
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first_advance;
    int slot = -1;

    printf("\n-- CSB C25 door destruction writeback --\n");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    raw[real_format_square_offset(1, 1)] = (uint8_t)((4u << 5) | 4u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    profile.party_x = 0;
    profile.party_y = 0;

    memset(&input, 0, sizeof(input));
    input.explosionType = C000_EXPLOSION_FIREBALL;
    input.attack = 200;
    input.mapIndex = 0;
    input.mapX = 1;
    input.mapY = 1;
    input.cell = EXPLOSION_CELL_CENTERED;
    input.centered = 1;
    input.currentTick = 0;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    CHECK(F0821_EXPLOSION_Create_Compat(
              &input,
              &profile.explosions,
              &slot,
              &first_advance) == 1 &&
              slot == 0,
          "CSB C25 door fireball fixture creates an explosion slot");
    queue_explosion_advance_event(&profile, &first_advance);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 door fireball reaches the scheduled boundary");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 door fireball dispatches through the explosion handler");
    CHECK(profile.explosions.count == 0,
          "C25 door fireball despawns after one-shot advance");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_DOOR_DESTRUCTION) == 1,
          "C25 door fireball queues a C02 door destruction event");
    CHECK((raw[real_format_square_offset(1, 1)] & 0x07u) == 4u,
          "C25 door fireball leaves the closed door unchanged before C02");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C02 door destruction event dispatches on the next tick");
    CHECK((raw[real_format_square_offset(1, 1)] & 0x07u) == 5u,
          "C02 door destruction mutates the real-format door state to destroyed");
}

static void test_runtime_save_roundtrips_projectiles_and_explosions(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_RuntimeProfile loaded;
    struct ProjectileCreateInput_Compat projectile_input;
    struct TimelineEvent_Compat first_move;
    struct ExplosionCreateInput_Compat explosion_input;
    struct TimelineEvent_Compat first_advance;
    const char *path =
        "/tmp/firestaff_csb_runtime_projectile_explosion_roundtrip.fsav";
    int projectile_slot = -1;
    int explosion_slot = -1;

    printf("\n-- CSB runtime save projectile/explosion persistence --\n");

    remove(path);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_game_id = 0x731u;
    profile.dungeon_seed = 0xC5B10731u;
    profile.party_x = 2;
    profile.party_y = 3;
    profile.party_dir = CSB_V1_DIR_EAST;
    profile.current_level = 0;
    profile.game_time = 42u;
    profile.timeline_queue.gameTick = profile.game_time;

    memset(&projectile_input, 0, sizeof(projectile_input));
    projectile_input.category = PROJECTILE_CATEGORY_MAGICAL;
    projectile_input.subtype = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    projectile_input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    projectile_input.ownerIndex = 7;
    projectile_input.mapIndex = 0;
    projectile_input.mapX = 1;
    projectile_input.mapY = 2;
    projectile_input.cell = 3;
    projectile_input.direction = 1;
    projectile_input.kineticEnergy = 22;
    projectile_input.attack = 88;
    projectile_input.launcherStrength = 99;
    projectile_input.stepEnergy = 5;
    projectile_input.currentTick = (int)profile.game_time;
    projectile_input.associatedThing = C002_EXPLOSION_LIGHTNING_BOLT;
    projectile_input.firstMoveGraceFlag = 0;
    CHECK(F0810_PROJECTILE_Create_Compat(
              &projectile_input,
              &profile.projectiles,
              &projectile_slot,
              &first_move) == 1 &&
              projectile_slot == 0,
          "CSB runtime save fixture creates a live projectile");
    (void)first_move;

    memset(&explosion_input, 0, sizeof(explosion_input));
    explosion_input.explosionType = C040_EXPLOSION_SMOKE;
    explosion_input.attack = 96;
    explosion_input.mapIndex = 0;
    explosion_input.mapX = 1;
    explosion_input.mapY = 1;
    explosion_input.cell = EXPLOSION_CELL_CENTERED;
    explosion_input.centered = 1;
    explosion_input.currentTick = (int)profile.game_time;
    explosion_input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    explosion_input.ownerIndex = 7;
    explosion_input.creatorProjectileSlot = projectile_slot;
    CHECK(F0821_EXPLOSION_Create_Compat(
              &explosion_input,
              &profile.explosions,
              &explosion_slot,
              &first_advance) == 1 &&
              explosion_slot == 0,
          "CSB runtime save fixture creates a live explosion");
    queue_explosion_advance_event(&profile, &first_advance);

    CHECK(csb_v1_runtime_save_game_to_path(&profile, path) == 0,
          "CSB runtime save writes projectile/explosion state");
    csb_v1_runtime_init(&loaded, NULL);
    CHECK(csb_v1_runtime_load_game_from_path(&loaded, path) == 0,
          "CSB runtime load accepts projectile/explosion save state");
    CHECK(loaded.projectiles.count == 1 &&
              loaded.projectiles.entries[projectile_slot].reserved3 != 0 &&
              loaded.projectiles.entries[projectile_slot].projectileSubtype ==
                  PROJECTILE_SUBTYPE_LIGHTNING_BOLT &&
              loaded.projectiles.entries[projectile_slot].mapX == 1 &&
              loaded.projectiles.entries[projectile_slot].mapY == 2 &&
              loaded.projectiles.entries[projectile_slot].attack == 88,
          "CSB runtime load restores live projectile slot state");
    CHECK(loaded.explosions.count == 1 &&
              loaded.explosions.entries[explosion_slot].reserved0 != 0 &&
              loaded.explosions.entries[explosion_slot].explosionType ==
                  C040_EXPLOSION_SMOKE &&
              loaded.explosions.entries[explosion_slot].attack == 96 &&
              loaded.explosions.entries[explosion_slot].creatorProjectileSlot ==
                  projectile_slot,
          "CSB runtime load restores live explosion slot state");
    CHECK(count_queued_event_type(&loaded, DM1_EVENT_EXPLOSION) == 1,
          "CSB runtime load preserves queued C25 explosion event");
    remove(path);
}

static void test_timeline_wall_gate_and_generator_sensor_mutations(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t raw[128];
    uint16_t type_data;

    make_real_format_sensor_dungeon(
        &dungeon,
        raw,
        sizeof(raw),
        1,
        0,
        (uint8_t)(1u << 5),
        (uint16_t)(12u << 7), /* disabled sensor preserving generator data */
        0,
        0);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_event(
        &profile,
        DM1_EVENT_ENABLE_GROUP_GENERATOR,
        DM1_EFFECT_SET,
        1,
        0);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C65 enable-generator event fires on the current tick");
    type_data = (uint16_t)(raw[70] | ((uint16_t)raw[71] << 8));
    CHECK((type_data & 0x007fu) == 6u && (type_data >> 7) == 12u,
          "C65 mutates the first disabled sensor back to C006 and preserves data");

    make_real_format_sensor_dungeon(
        &dungeon,
        raw,
        sizeof(raw),
        0,
        0,
        (uint8_t)(0u << 5),
        (uint16_t)((0x10u << 7) | 5u), /* C005 gate, ref mask bit 0 */
        (uint16_t)(DM1_EFFECT_HOLD << 3),
        make_sensor_target(1, 0, 0));
    raw[real_format_square_offset(1, 0)] = (uint8_t)((4u << 5) | 4u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_cell_event(
        &profile,
        DM1_EVENT_WALL,
        DM1_EFFECT_SET,
        0,
        0,
        0);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C06 wall gate event fires and mutates the sensor record");
    type_data = (uint16_t)(raw[70] | ((uint16_t)raw[71] << 8));
    CHECK((type_data >> 7) == 0x11u,
          "C005 wall gate SET records the current mask bit");
    CHECK(profile.timeline_queue.eventCount == 1,
          "matching C005 HOLD gate queues one remote square effect");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "remote wall-gate square effect fires on the next tick");
    CHECK((raw[real_format_square_offset(1, 0)] & 0x07u) == 3u,
          "remote wall-gate effect reaches the target door and starts opening");

    make_real_format_wall_text_dungeon(
        &dungeon,
        raw,
        sizeof(raw),
        2,
        0x0000u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_cell_event(
        &profile,
        DM1_EVENT_WALL,
        DM1_EFFECT_SET,
        0,
        0,
        2);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C06 wall text SET event fires on the current tick");
    type_data = (uint16_t)(raw[70] | ((uint16_t)raw[71] << 8));
    CHECK((type_data & 0x0001u) == 0x0001u,
          "C06 wall text SET marks same-cell textstring visible");

    queue_square_cell_event(
        &profile,
        DM1_EVENT_WALL,
        DM1_EFFECT_CLEAR,
        0,
        0,
        2);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C06 wall text CLEAR event fires on the current tick");
    type_data = (uint16_t)(raw[70] | ((uint16_t)raw[71] << 8));
    CHECK((type_data & 0x0001u) == 0u,
          "C06 wall text CLEAR hides same-cell textstring");

    queue_square_cell_event(
        &profile,
        DM1_EVENT_WALL,
        DM1_EFFECT_TOGGLE,
        0,
        0,
        1);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C06 wall text wrong-cell TOGGLE event fires on the current tick");
    type_data = (uint16_t)(raw[70] | ((uint16_t)raw[71] << 8));
    CHECK((type_data & 0x0001u) == 0u,
          "C06 wall text ignores textstrings on other wall cells");

    queue_square_cell_event(
        &profile,
        DM1_EVENT_WALL,
        DM1_EFFECT_TOGGLE,
        0,
        0,
        2);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C06 wall text same-cell TOGGLE event fires on the current tick");
    type_data = (uint16_t)(raw[70] | ((uint16_t)raw[71] << 8));
    CHECK((type_data & 0x0001u) == 0x0001u,
          "C06 wall text TOGGLE flips same-cell visibility");

    make_real_format_sensor_dungeon(
        &dungeon,
        raw,
        sizeof(raw),
        0,
        1,
        (uint8_t)(0u << 5),
        (uint16_t)((2u << 7) |
                   DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION),
        (uint16_t)(1u << 2),
        (uint16_t)(7u | (9u << 8)));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_cell_event(
        &profile,
        DM1_EVENT_WALL,
        DM1_EFFECT_SET,
        0,
        1,
        0);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C06 wall C010 launcher event fires on the current tick");
    type_data = (uint16_t)(raw[70] | ((uint16_t)raw[71] << 8));
    CHECK((type_data & 0x007fu) == 0u,
          "C06 once-only launcher disables the source sensor type");
    CHECK(profile.projectiles.count == 2,
          "C06 double explosion launcher creates two CSB runtime projectiles");
    CHECK(profile.projectiles.entries[0].projectileSubtype ==
              PROJECTILE_SUBTYPE_LIGHTNING_BOLT &&
              profile.projectiles.entries[1].projectileSubtype ==
              PROJECTILE_SUBTYPE_LIGHTNING_BOLT,
          "C06 launcher maps explosion thing data to lightning projectile subtype");
    CHECK(profile.projectiles.entries[0].ownerKind == PROJECTILE_OWNER_LAUNCHER &&
              profile.projectiles.entries[1].ownerKind == PROJECTILE_OWNER_LAUNCHER,
          "C06 launcher projectiles are owned by the launcher boundary");
    CHECK(profile.projectiles.entries[0].mapX == 0 &&
              profile.projectiles.entries[0].mapY == 0 &&
              profile.projectiles.entries[0].direction == 0,
          "C06 launcher projectile starts one square in front of the wall cell");
    CHECK(profile.projectiles.entries[0].cell == 2 &&
              profile.projectiles.entries[1].cell == 3,
          "C06 double launcher uses opposite and next projectile cells");
    CHECK(profile.projectiles.entries[0].kineticEnergy == 7 &&
              profile.projectiles.entries[0].stepEnergy == 9 &&
              profile.projectiles.entries[0].attack == 100,
          "C06 launcher carries kinetic, step, and attack values into F0810 state");
    CHECK(profile.projectiles.entries[0].firstMoveGraceFlag == 0 &&
              profile.projectiles.entries[1].firstMoveGraceFlag == 0,
          "C06 launcher projectiles use C49 immediate-impact movement");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_MOVE_PROJECTILE) == 2,
          "C06 launcher schedules one C49 movement event per projectile");

    make_real_format_sensor_dungeon(
        &dungeon,
        raw,
        sizeof(raw),
        1,
        0,
        (uint8_t)(1u << 5),
        (uint16_t)((51u << 7) |
                   DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ),
        (uint16_t)(1u << 2),
        (uint16_t)(6u | (8u << 8)));
    dungeon.thing_data_bases[5] = 82;
    dungeon.thing_type_counts[5] = 2;
    test_put_le16(raw, 66, (uint16_t)((1u << 14) | (3u << 10)));
    test_put_le16(raw, 82, 0xffffu);
    test_put_le16(raw, 86, 0xffffu);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_cell_event(
        &profile,
        DM1_EVENT_WALL,
        DM1_EFFECT_SET,
        1,
        0,
        1);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C06 wall C009 new-object launcher event fires on the current tick");
    type_data = (uint16_t)(raw[70] | ((uint16_t)raw[71] << 8));
    CHECK((type_data & 0x007fu) == 0u,
          "C06 new-object once-only launcher disables the source sensor type");
    CHECK(profile.projectiles.count == 2,
          "C06 new-object launcher creates two CSB runtime projectiles");
    CHECK(profile.projectiles.entries[0].projectileCategory ==
              PROJECTILE_CATEGORY_KINETIC &&
              profile.projectiles.entries[1].projectileCategory ==
                  PROJECTILE_CATEGORY_KINETIC,
          "C06 new-object launcher creates kinetic projectiles");
    CHECK((uint16_t)profile.projectiles.entries[0].reserved1 ==
              (uint16_t)(5u << 10) &&
              (uint16_t)profile.projectiles.entries[1].reserved1 ==
                  (uint16_t)((5u << 10) | 1u),
          "C06 new-object launcher preserves allocated associated things");
    CHECK(test_get_le16(raw, 82) == 0xfffeu &&
              test_get_le16(raw, 84) == 27u &&
              test_get_le16(raw, 86) == 0xfffeu &&
              test_get_le16(raw, 88) == 27u,
          "C06 new-object launcher materializes two arrow weapon records");
    CHECK(profile.projectiles.entries[0].mapX == 2 &&
              profile.projectiles.entries[0].mapY == 0 &&
              profile.projectiles.entries[0].cell == 3 &&
              profile.projectiles.entries[1].cell == 0,
          "C06 new-object launcher starts one square ahead with opposite/next cells");
    CHECK(profile.projectiles.entries[0].kineticEnergy == 6 &&
              profile.projectiles.entries[0].stepEnergy == 8 &&
              profile.projectiles.entries[0].attack == 100,
          "C06 new-object launcher carries kinetic, step, and source attack values");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_MOVE_PROJECTILE) == 2,
          "C06 new-object launcher schedules one C49 movement event per projectile");

    make_real_format_sensor_dungeon(
        &dungeon,
        raw,
        sizeof(raw),
        0,
        1,
        (uint8_t)(0u << 5),
        (uint16_t)DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ,
        (uint16_t)(1u << 2),
        (uint16_t)(7u | (9u << 8)));
    dungeon.thing_data_bases[5] = 82;
    dungeon.thing_type_counts[5] = 2;
    test_put_le16(raw, 68, (uint16_t)(5u << 10));
    test_put_le16(raw, 82, (uint16_t)((1u << 14) | (5u << 10) | 1u));
    test_put_le16(raw, 86, 0xfffeu);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_cell_event(
        &profile,
        DM1_EVENT_WALL,
        DM1_EFFECT_SET,
        0,
        1,
        0);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C06 wall C015 square-object launcher event fires on the current tick");
    type_data = (uint16_t)(raw[70] | ((uint16_t)raw[71] << 8));
    CHECK((type_data & 0x007fu) == 0u,
          "C06 square-object once-only launcher disables the source sensor type");
    CHECK(profile.projectiles.count == 2,
          "C06 square-object launcher creates two CSB runtime projectiles");
    CHECK(profile.projectiles.entries[0].projectileCategory ==
              PROJECTILE_CATEGORY_KINETIC &&
              profile.projectiles.entries[0].projectileSubtype ==
                  PROJECTILE_SUBTYPE_KINETIC_ARROW &&
              profile.projectiles.entries[1].projectileCategory ==
                  PROJECTILE_CATEGORY_KINETIC,
          "C06 square-object launcher creates kinetic object projectiles");
    CHECK((uint16_t)profile.projectiles.entries[0].reserved1 ==
              (uint16_t)(5u << 10) &&
              (uint16_t)profile.projectiles.entries[1].reserved1 ==
                  (uint16_t)((1u << 14) | (5u << 10) | 1u),
          "C06 square-object launcher preserves the unlinked associated things");
    CHECK(profile.projectiles.entries[0].mapX == 0 &&
              profile.projectiles.entries[0].mapY == 0 &&
              profile.projectiles.entries[0].cell == 2 &&
              profile.projectiles.entries[1].cell == 3,
          "C06 square-object launcher starts one square ahead with opposite/next cells");
    CHECK(profile.projectiles.entries[0].kineticEnergy == 7 &&
              profile.projectiles.entries[0].stepEnergy == 9 &&
              profile.projectiles.entries[0].attack == 100,
          "C06 square-object launcher carries kinetic, step, and source attack values");
    CHECK(test_get_le16(raw, 68) == 0xfffeu &&
              test_get_le16(raw, 82) == 0xfffeu &&
              test_get_le16(raw, 86) == 0xfffeu,
          "C06 square-object launcher unlinks both objects from the source square chain");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_MOVE_PROJECTILE) == 2,
          "C06 square-object launcher schedules one C49 movement event per object projectile");

    make_real_format_sensor_dungeon(
        &dungeon,
        raw,
        sizeof(raw),
        1,
        1,
        (uint8_t)(0u << 5),
        (uint16_t)((2u << 7) |
                   DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION),
        (uint16_t)(1u << 2),
        (uint16_t)(20u | (2u << 8)));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_cell_event(
        &profile,
        DM1_EVENT_WALL,
        DM1_EFFECT_SET,
        1,
        1,
        0);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C06 high-energy launcher event fires on the current tick");
    CHECK(profile.projectiles.count == 2 &&
              count_queued_event_type(&profile, DM1_EVENT_MOVE_PROJECTILE) == 2,
          "C06 high-energy launcher creates and queues two projectile moves");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 launcher projectile move events dispatch on the next tick");
    CHECK(profile.projectiles.count == 2,
          "C49 open-square projectile movement keeps live projectiles active");
    CHECK(profile.projectiles.entries[0].cell == 1 &&
              profile.projectiles.entries[1].cell == 0,
          "C49 movement applies the ReDMCSB parity cell step to launcher projectiles");
    CHECK(profile.projectiles.entries[0].kineticEnergy == 18 &&
              profile.projectiles.entries[0].attack == 98 &&
              profile.projectiles.entries[0].firstMoveGraceFlag == 0,
          "C49 movement decrements kinetic/attack energy without first-move grace");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_MOVE_PROJECTILE) == 2,
          "C49 movement requeues live launcher projectiles for the following tick");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 launcher projectile wall-impact events dispatch on the next tick");
    CHECK(profile.projectiles.count == 0,
          "C49 wall impact despawns launcher projectiles");
    CHECK(profile.explosions.count == 2,
          "C49 wall impact materializes launcher projectile explosions");
    CHECK(profile.explosions.entries[0].explosionType ==
              C002_EXPLOSION_LIGHTNING_BOLT &&
              profile.explosions.entries[1].explosionType ==
              C002_EXPLOSION_LIGHTNING_BOLT,
          "C49 wall impact preserves launcher lightning explosion type");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_EXPLOSION) == 2,
          "C49 wall impact schedules C25 explosion advance events");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 launcher impact explosion events dispatch on the next tick");
    CHECK(profile.explosions.count == 0,
          "C25 one-shot launcher impact explosions despawn after advance");
    CHECK(count_queued_event_type(&profile, DM1_EVENT_EXPLOSION) == 0,
          "C25 one-shot launcher impact explosions do not requeue");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[5] = 82;
    dungeon.thing_type_counts[5] = 1;
    raw[real_format_square_offset(1, 0)] =
        (uint8_t)((1u << 5) | 0x10u);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 66, 0xfffeu);
    test_put_le16(raw, 82, 0xfffeu);
    test_put_le16(raw, 84, 27u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    {
        struct ProjectileCreateInput_Compat projectile_input;
        struct TimelineEvent_Compat first_move;
        int projectile_slot = -1;
        memset(&projectile_input, 0, sizeof(projectile_input));
        memset(&first_move, 0, sizeof(first_move));
        projectile_input.category = PROJECTILE_CATEGORY_KINETIC;
        projectile_input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
        projectile_input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
        projectile_input.ownerIndex = 3;
        projectile_input.mapIndex = 0;
        projectile_input.mapX = 1;
        projectile_input.mapY = 0;
        projectile_input.cell = 0;
        projectile_input.direction = 0;
        projectile_input.kineticEnergy = 10;
        projectile_input.attack = 20;
        projectile_input.launcherStrength = 20;
        projectile_input.stepEnergy = 1;
        projectile_input.currentTick = (int)profile.game_time;
        projectile_input.associatedThing = (int)(5u << 10);
        CHECK(F0810_PROJECTILE_Create_Compat(
                  &projectile_input,
                  &profile.projectiles,
                  &projectile_slot,
                  &first_move) == 1 &&
                  projectile_slot == 0,
              "C49 associated-object fixture creates a kinetic projectile");
        queue_projectile_move_event(&profile, &first_move);
    }
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 associated-object fixture advances to first move tick");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 associated-object wall-impact event dispatches");
    CHECK(profile.projectiles.count == 0,
          "C49 kinetic wall impact despawns the object projectile");
    CHECK(test_get_le16(raw, 66) == (uint16_t)(5u << 10) &&
              test_get_le16(raw, 82) == 0xfffeu,
          "C49 kinetic wall impact materializes the associated object on the source square");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 2;
    dungeon.thing_data_bases[1] = 72;
    dungeon.thing_type_counts[1] = 1;
    dungeon.thing_data_bases[5] = 80;
    dungeon.thing_type_counts[5] = 1;
    raw[real_format_square_offset(1, 0)] =
        (uint8_t)((5u << 5) | 0x10u | 0x08u);
    raw[real_format_square_offset(2, 0)] =
        (uint8_t)((1u << 5) | 0x10u);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 60 + 2 * 2, 1);
    test_put_le16(raw, 66, (uint16_t)(1u << 10));
    test_put_le16(raw, 68, 0xfffeu);
    test_put_le16(raw, 72, 0xfffeu);
    test_put_le16(raw, 74,
                  (uint16_t)(2u | (0u << 5) | (1u << 10) |
                             (2u << 13)));
    test_put_le16(raw, 76, 0u);
    test_put_le16(raw, 80, 0xfffeu);
    test_put_le16(raw, 82, 27u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    {
        struct ProjectileCreateInput_Compat projectile_input;
        struct TimelineEvent_Compat first_move;
        int projectile_slot = -1;
        memset(&projectile_input, 0, sizeof(projectile_input));
        memset(&first_move, 0, sizeof(first_move));
        projectile_input.category = PROJECTILE_CATEGORY_KINETIC;
        projectile_input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
        projectile_input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
        projectile_input.ownerIndex = 4;
        projectile_input.mapIndex = 0;
        projectile_input.mapX = 1;
        projectile_input.mapY = 0;
        projectile_input.cell = 0;
        projectile_input.direction = 0;
        projectile_input.kineticEnergy = 10;
        projectile_input.attack = 20;
        projectile_input.launcherStrength = 20;
        projectile_input.stepEnergy = 1;
        projectile_input.currentTick = (int)profile.game_time;
        projectile_input.associatedThing = (int)(5u << 10);
        CHECK(F0810_PROJECTILE_Create_Compat(
                  &projectile_input,
                  &profile.projectiles,
                  &projectile_slot,
                  &first_move) == 1 &&
                  projectile_slot == 0,
              "C49 associated-object teleporter fixture creates a kinetic projectile");
        queue_projectile_move_event(&profile, &first_move);
    }
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 associated-object teleporter fixture advances to first move tick");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 associated-object teleporter wall-impact event dispatches");
    CHECK(profile.projectiles.count == 0,
          "C49 associated-object teleporter impact despawns the projectile");
    CHECK(test_get_le16(raw, 66) == (uint16_t)(1u << 10) &&
              test_get_le16(raw, 72) == 0xfffeu,
          "C49 associated-object teleporter leaves the C05 thing in place");
    CHECK(test_get_le16(raw, 68) == (uint16_t)(5u << 10) &&
              test_get_le16(raw, 80) == 0xfffeu,
          "C49 associated-object teleporter moves the object to the target square without CM2 cell rotation");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 2;
    dungeon.thing_data_bases[5] = 82;
    dungeon.thing_type_counts[5] = 1;
    raw[real_format_square_offset(1, 0)] = (uint8_t)(1u << 5);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 66, 0xffffu);
    test_put_le16(raw, 68, 0xffffu);
    test_put_le16(raw, 82, 0xfffeu);
    test_put_le16(raw, 84, 27u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    {
        struct ProjectileCreateInput_Compat projectile_input;
        struct TimelineEvent_Compat first_move;
        int projectile_slot = -1;
        memset(&projectile_input, 0, sizeof(projectile_input));
        memset(&first_move, 0, sizeof(first_move));
        projectile_input.category = PROJECTILE_CATEGORY_KINETIC;
        projectile_input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
        projectile_input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
        projectile_input.ownerIndex = 5;
        projectile_input.mapIndex = 0;
        projectile_input.mapX = 1;
        projectile_input.mapY = 0;
        projectile_input.cell = 0;
        projectile_input.direction = 0;
        projectile_input.kineticEnergy = 10;
        projectile_input.attack = 20;
        projectile_input.launcherStrength = 20;
        projectile_input.stepEnergy = 1;
        projectile_input.currentTick = (int)profile.game_time;
        projectile_input.associatedThing = (int)(5u << 10);
        CHECK(F0810_PROJECTILE_Create_Compat(
                  &projectile_input,
                  &profile.projectiles,
                  &projectile_slot,
                  &first_move) == 1 &&
                  projectile_slot == 0,
              "C49 empty-square drop fixture creates a kinetic projectile");
        queue_projectile_move_event(&profile, &first_move);
    }
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 empty-square drop fixture advances to first move tick");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 empty-square wall-impact event dispatches");
    CHECK(profile.projectiles.count == 0,
          "C49 empty-square wall impact despawns the projectile");
    CHECK((raw[real_format_square_offset(1, 0)] & 0x10u) == 0x10u,
          "C49 empty-square drop creates a square thing-list flag");
    CHECK(test_get_le16(raw, 66) == (uint16_t)(5u << 10) &&
              test_get_le16(raw, 68) == 0xffffu,
          "C49 empty-square drop inserts the associated object into the reserved first-thing slot");

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[1] = 68;
    dungeon.thing_type_counts[1] = 1;
    raw[real_format_square_offset(1, 0)] =
        (uint8_t)((5u << 5) | 0x10u | 0x08u);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 66, (uint16_t)(1u << 10));
    test_put_le16(raw, 68, 0xfffeu);
    test_put_le16(raw, 70,
                  (uint16_t)(2u | (2u << 5) | (1u << 10) |
                             (2u << 13)));
    test_put_le16(raw, 72, 0u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    {
        struct ProjectileCreateInput_Compat projectile_input;
        struct TimelineEvent_Compat first_move;
        int projectile_slot = -1;
        memset(&projectile_input, 0, sizeof(projectile_input));
        memset(&first_move, 0, sizeof(first_move));
        projectile_input.category = PROJECTILE_CATEGORY_KINETIC;
        projectile_input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
        projectile_input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
        projectile_input.ownerIndex = 6;
        projectile_input.mapIndex = 0;
        projectile_input.mapX = 1;
        projectile_input.mapY = 1;
        projectile_input.cell = 0;
        projectile_input.direction = 0;
        projectile_input.kineticEnergy = 20;
        projectile_input.attack = 20;
        projectile_input.launcherStrength = 20;
        projectile_input.stepEnergy = 1;
        projectile_input.currentTick = (int)profile.game_time;
        projectile_input.associatedThing = (int)(5u << 10);
        CHECK(F0810_PROJECTILE_Create_Compat(
                  &projectile_input,
                  &profile.projectiles,
                  &projectile_slot,
                  &first_move) == 1 &&
                  projectile_slot == 0,
              "C49 projectile teleporter fixture creates a kinetic projectile");
        queue_projectile_move_event(&profile, &first_move);
    }
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 projectile teleporter fixture advances to first move tick");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 projectile teleporter event dispatches");
    CHECK(profile.projectiles.count == 1,
          "C49 projectile teleporter keeps the projectile alive");
    CHECK(profile.projectiles.entries[0].mapX == 2 &&
              profile.projectiles.entries[0].mapY == 2 &&
              profile.projectiles.entries[0].direction == 1 &&
              profile.projectiles.entries[0].cell == 0,
          "C49 projectile teleporter applies target, relative direction, and relative cell rotation");
    {
        int event_index = find_queued_event_type(&profile,
                                                 DM1_EVENT_MOVE_PROJECTILE);
        CHECK(event_index >= 0 &&
                  profile.timeline_queue.events[event_index].b_mapX == 2 &&
                  profile.timeline_queue.events[event_index].b_mapY == 2 &&
                  profile.timeline_queue.events[event_index].c_cell == 0,
              "C49 projectile teleporter requeues movement from the teleported state");
    }

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 2;
    dungeon.thing_data_bases[1] = 70;
    dungeon.thing_type_counts[1] = 2;
    raw[real_format_square_offset(0, 2)] =
        (uint8_t)((5u << 5) | 0x10u | 0x08u);
    raw[real_format_square_offset(1, 0)] =
        (uint8_t)((5u << 5) | 0x10u | 0x08u);
    test_put_le16(raw, 60 + 0 * 2, 0);
    test_put_le16(raw, 60 + 1 * 2, 1);
    test_put_le16(raw, 66, (uint16_t)((1u << 10) | 0u));
    test_put_le16(raw, 68, (uint16_t)((1u << 10) | 1u));
    test_put_le16(raw, 70, 0xfffeu);
    test_put_le16(raw, 72,
                  (uint16_t)(2u | (2u << 5) | (1u << 10) |
                             (2u << 13)));
    test_put_le16(raw, 74, 0u);
    test_put_le16(raw, 76, 0xfffeu);
    test_put_le16(raw, 78,
                  (uint16_t)(0u | (2u << 5) | (1u << 10) |
                             (2u << 13)));
    test_put_le16(raw, 80, 0u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    {
        struct ProjectileCreateInput_Compat projectile_input;
        struct TimelineEvent_Compat first_move;
        int projectile_slot = -1;
        memset(&projectile_input, 0, sizeof(projectile_input));
        memset(&first_move, 0, sizeof(first_move));
        projectile_input.category = PROJECTILE_CATEGORY_KINETIC;
        projectile_input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
        projectile_input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
        projectile_input.ownerIndex = 7;
        projectile_input.mapIndex = 0;
        projectile_input.mapX = 1;
        projectile_input.mapY = 1;
        projectile_input.cell = 0;
        projectile_input.direction = 0;
        projectile_input.kineticEnergy = 20;
        projectile_input.attack = 20;
        projectile_input.launcherStrength = 20;
        projectile_input.stepEnergy = 1;
        projectile_input.currentTick = (int)profile.game_time;
        projectile_input.associatedThing = (int)(5u << 10);
        CHECK(F0810_PROJECTILE_Create_Compat(
                  &projectile_input,
                  &profile.projectiles,
                  &projectile_slot,
                  &first_move) == 1 &&
                  projectile_slot == 0,
              "C49 chained projectile teleporter fixture creates a kinetic projectile");
        queue_projectile_move_event(&profile, &first_move);
    }
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 chained projectile teleporter fixture advances to first move tick");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 chained projectile teleporter event dispatches");
    CHECK(profile.projectiles.count == 1,
          "C49 chained projectile teleporter keeps the projectile alive");
    CHECK(profile.projectiles.entries[0].mapX == 2 &&
              profile.projectiles.entries[0].mapY == 2 &&
              profile.projectiles.entries[0].direction == 2 &&
              profile.projectiles.entries[0].cell == 1,
          "C49 chained projectile teleporter applies both relative rotations");
    {
        int event_index = find_queued_event_type(&profile,
                                                 DM1_EVENT_MOVE_PROJECTILE);
        CHECK(event_index >= 0 &&
                  profile.timeline_queue.events[event_index].b_mapX == 2 &&
                  profile.timeline_queue.events[event_index].b_mapY == 2 &&
                  profile.timeline_queue.events[event_index].c_cell == 1,
              "C49 chained projectile teleporter requeues from the chain target");
    }

    make_real_format_square_event_dungeon(&dungeon, raw, sizeof(raw));
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[4] = 82;
    dungeon.thing_type_counts[4] = 1;
    dungeon.thing_data_bases[5] = 98;
    dungeon.thing_type_counts[5] = 1;
    raw[real_format_square_offset(1, 0)] =
        (uint8_t)((1u << 5) | 0x10u);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 66, (uint16_t)(4u << 10));
    test_put_le16(raw, 82, 0xfffeu);
    test_put_le16(raw, 84, 0xfffeu);
    raw[86] = 3u;     /* C03: keeps thrown sharp weapons in G0243. */
    raw[87] = 0xffu;  /* single centered creature. */
    test_put_le16(raw, 88, 500u);
    test_put_le16(raw, 96, 0u);
    test_put_le16(raw, 98, 0xfffeu);
    test_put_le16(raw, 100, 27u);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    {
        struct ProjectileCreateInput_Compat projectile_input;
        struct TimelineEvent_Compat first_move;
        int projectile_slot = -1;
        memset(&projectile_input, 0, sizeof(projectile_input));
        memset(&first_move, 0, sizeof(first_move));
        projectile_input.category = PROJECTILE_CATEGORY_KINETIC;
        projectile_input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
        projectile_input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
        projectile_input.ownerIndex = 4;
        projectile_input.mapIndex = 0;
        projectile_input.mapX = 1;
        projectile_input.mapY = 1;
        projectile_input.cell = 0;
        projectile_input.direction = 0;
        projectile_input.kineticEnergy = 20;
        projectile_input.attack = 20;
        projectile_input.launcherStrength = 20;
        projectile_input.stepEnergy = 1;
        projectile_input.currentTick = (int)profile.game_time;
        projectile_input.associatedThing = (int)(5u << 10);
        CHECK(F0810_PROJECTILE_Create_Compat(
                  &projectile_input,
                  &profile.projectiles,
                  &projectile_slot,
                  &first_move) == 1 &&
                  projectile_slot == 0,
              "C49 creature-hit fixture creates a kinetic arrow projectile");
        queue_projectile_move_event(&profile, &first_move);
    }
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 creature-hit fixture advances to first move tick");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 creature-hit event dispatches");
    CHECK(profile.projectiles.count == 0,
          "C49 creature hit despawns the object projectile");
    CHECK(test_get_le16(raw, 84) == (uint16_t)(5u << 10) &&
              test_get_le16(raw, 98) == 0xfffeu,
          "C49 kept sharp arrow links into GROUP.Slot");
    CHECK(test_get_le16(raw, 66) == (uint16_t)(4u << 10),
          "C49 kept sharp arrow does not replace the square group chain");
    CHECK(test_get_le16(raw, 88) < 500u,
          "C49 creature hit writes projectile damage into GROUP.Health");

    make_real_format_sensor_dungeon(
        &dungeon,
        raw,
        sizeof(raw),
        0,
        0,
        (uint8_t)(0u << 5),
        (uint16_t)DM1_SENSOR_WALL_END_GAME,
        (uint16_t)(3u << 7),
        0);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    queue_square_cell_event(
        &profile,
        DM1_EVENT_WALL,
        DM1_EFFECT_CLEAR,
        0,
        0,
        3);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C06 wall C018 endgame event fires on the current tick");
    CHECK(profile.victory == 1,
          "C06 wall C018 marks the CSB runtime as victorious");
    CHECK(profile.game_over == 0,
          "C06 wall C018 victory does not mark party-death game_over");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 0,
          "victorious CSB runtime blocks later V1 ticks");
}

static void seed_two_champion_party(CSB_V1_PartyState *party)
{
    int i;

    csb_v1_character_init_default(party);
    party->ChampionCount = 2;
    party->ImportedFromDM1 = 1;
    party->PartyDirection = CSB_V1_DIR_NORTH;
    party->LeaderIndex = 0;
    for (i = 0; i < party->ChampionCount; i++) {
        CSB_V1_Champion *champion = &party->Champions[i];
        champion->CurrentHealth = (int16_t)(80 + i);
        champion->MaximumHealth = (int16_t)(100 + i);
        champion->Cell = (uint8_t)i;
        champion->Direction = (uint8_t)((i + 2) & 3);
    }
}

static void test_input_command_queue_turn_reaches_runtime_party_state(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    CSB_V1_PartyState after;
    struct Dm1V1InputQueueProcessResultPc34Compat dispatch;

    /* This is a focused CSB V1 command-boundary gate, not a full
     * movement/playability claim.  ReDMCSB COMMAND.C F0380 lines
     * 2075-2127 dequeues one source command and lines 2150-2156
     * dispatch C001/C002 turns to CLIKMENU.C F0365; F0365 lines
     * 156-173 maps TURN_RIGHT to party_dir+1 and calls CHAMPION.C F0284
     * lines 117-130 to rotate champion Cell/Direction. */
    csb_v1_runtime_init(&profile, NULL);
    seed_two_champion_party(&party);
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "runtime accepts a seeded imported party for input binding");

    CHECK(csb_v1_runtime_enqueue_input_command(
              &profile, DM1_V1_COMMAND_TURN_RIGHT, 291, 125) == 1,
          "CSB runtime queues one source TURN_RIGHT command");
    CHECK(profile.input_command_queue.count == 1U,
          "input command queue contains one queued command before dispatch");
    CHECK(csb_v1_runtime_process_one_input_command(&profile, 0, 0, 0) == 1,
          "CSB runtime processes one queued input command");
    CHECK(csb_v1_runtime_get_last_input_dispatch(&profile, &dispatch) == 1,
          "last input dispatch reports one dequeued command");
    CHECK(dispatch.command == DM1_V1_COMMAND_TURN_RIGHT,
          "last input dispatch preserves the TURN_RIGHT source command id");
    CHECK(dispatch.dispatchedTurn == 1 && dispatch.dispatchedMove == 0,
          "queue boundary classifies TURN_RIGHT as a turn dispatch");
    CHECK(profile.input_dispatch_count == 1U,
          "CSB runtime records the command dispatch count");
    CHECK(profile.input_command_queue.count == 0U,
          "input command queue is empty after the turn dispatch");
    CHECK(profile.party_dir == CSB_V1_DIR_EAST,
          "queued TURN_RIGHT reaches CSB runtime party_dir NORTH->EAST");
    CHECK(csb_v1_runtime_get_party_state(&profile, &after) == 2,
          "runtime party snapshot remains visible after queued turn");
    CHECK(after.PartyDirection == CSB_V1_DIR_EAST,
          "party snapshot direction follows the queued turn");
    CHECK(after.Champions[0].Cell == 1 &&
              after.Champions[1].Cell == 2,
          "queued turn rotates champion cells by +1 mod 4");
    CHECK(after.Champions[0].Direction == 3 &&
              after.Champions[1].Direction == 0,
          "queued turn rotates champion directions by +1 mod 4");
}

static void test_input_command_queue_move_boundary_does_not_claim_movement(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputQueueProcessResultPc34Compat dispatch;

    csb_v1_runtime_init(&profile, NULL);
    seed_two_champion_party(&party);
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "runtime accepts a seeded imported party for move-boundary guard");
    CHECK(csb_v1_runtime_enqueue_input_command(
              &profile, DM1_V1_COMMAND_MOVE_FORWARD, 263, 125) == 1,
          "CSB runtime queues one source MOVE_FORWARD command");
    CHECK(csb_v1_runtime_process_one_input_command(&profile, 0, 0, 0) == 1,
          "CSB runtime dequeues one MOVE_FORWARD command at the command boundary");
    CHECK(csb_v1_runtime_get_last_input_dispatch(&profile, &dispatch) == 1,
          "last input dispatch reports MOVE_FORWARD was dequeued");
    CHECK(dispatch.command == DM1_V1_COMMAND_MOVE_FORWARD &&
              dispatch.dispatchedMove == 1,
          "queue boundary classifies MOVE_FORWARD as a move dispatch");
    CHECK(profile.party_dir == CSB_V1_DIR_NORTH,
          "MOVE_FORWARD boundary does not mutate party_dir before CSB movement is bound");
    CHECK(profile.party_x == CSB_V1_START_PARTY_X &&
              profile.party_y == CSB_V1_START_PARTY_Y - 1,
          "MOVE_FORWARD boundary reaches the bounded open-step runtime movement");
}

int main(void)
{
    printf("=== CSB V1 Runtime Tick Accumulator Follow-up ===\n\n");
    test_subquantum_frame_slices_fire_one_tick();
    test_multi_quantum_tick_and_due_probe();
    test_tick_v1_steps_exactly_once_and_honors_stop_states();
    test_timeline_events_fire_before_game_time_increment();
    test_timeline_square_events_mutate_real_format_map_bytes();
    test_timeline_corridor_text_and_generator_mutations();
    test_c38_poison_followup_and_c75_tick();
    test_c38_giggler_steals_hand_slots_into_group_slot_chain();
    test_c37_group_approach_teleporter_rotation();
    test_explosion_c25_persistent_smoke_requeues_until_depleted();
    test_runtime_save_roundtrips_projectiles_and_explosions();
    test_explosion_c25_party_damage_and_group_hp_writeback();
    test_explosion_c25_door_destruction_writeback();
    test_timeline_wall_gate_and_generator_sensor_mutations();
    test_input_command_queue_turn_reaches_runtime_party_state();
    test_input_command_queue_move_boundary_does_not_claim_movement();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: CSB V1 runtime tick boundary accumulates sub-55ms frame slices, fires source-locked V1 quanta, and dispatches timeline events before game_time increments");
        puts("sourceEvidence=ReDMCSB TIMELINE.C F0235/F0240/F0261 lines 702-708,1833-1850; GAMELOOP.C F0002 lines 69-124; COMMAND.C F0380 lines 2383-2429");
        puts("ok: CSB V1 runtime input queue processes one source TURN_RIGHT into party_dir and champion Cell/Direction state without claiming full movement/playability");
        puts("sourceEvidence=ReDMCSB COMMAND.C F0380 lines 2075-2127,2150-2156; CLIKMENU.C F0365 lines 156-173; CHAMPION.C F0284 lines 117-130");
    }
    return failed == 0 ? 0 : 1;
}
