#include "csb_v1_boot.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_creature_ai_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int count_source_groups_on_level(const CSB_V1_DungeonData *dungeon,
                                        int level)
{
    int x;
    int y;
    int groups = 0;

    for (x = 0; x < dungeon->level_widths[level]; ++x) {
        for (y = 0; y < dungeon->level_heights[level]; ++y) {
            int thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
            int guard;
            for (guard = 0; guard < 128 && thing >= 0 &&
                 thing != THING_NONE && thing != THING_ENDOFLIST; ++guard) {
                int type = -1;
                int record_size = 0;
                const uint8_t *record = csb_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)thing, &type, NULL, &record_size);
                if (!record || record_size < 2) return -1;
                if (type == CSB_V1_THING_TYPE_GROUP) {
                    ++groups;
                    break;
                }
                thing = (int)((uint16_t)record[0] |
                              ((uint16_t)record[1] << 8));
            }
            if (guard == 128 && thing != THING_NONE &&
                thing != THING_ENDOFLIST) return -1;
        }
    }
    return groups;
}

static int source_group_types_match_all_map_metadata(
    const CSB_V1_DungeonData *dungeon, int *out_maps, int *out_groups)
{
    int level;
    int checked_maps = 0;
    int checked_groups = 0;
    for (level = 0; level < dungeon->level_count; ++level) {
        const int metadata = dungeon->level_offsets[level] +
            dungeon->level_widths[level] * dungeon->level_heights[level];
        int x;
        ++checked_maps;
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int thing = csb_v1_dungeon_get_first_thing(
                    dungeon, level, x, y);
                int guard;
                for (guard = 0; guard < 128 && thing >= 0 &&
                     thing != THING_NONE && thing != THING_ENDOFLIST; ++guard) {
                    int type = -1;
                    int size = 0;
                    const uint8_t *record = csb_v1_dungeon_get_thing_record(
                        dungeon, (uint16_t)thing, &type, NULL, &size);
                    if (!record || size < 2) return 0;
                    if (type == CSB_V1_THING_TYPE_GROUP) {
                        int allowed;
                        int found = 0;
                        if (size < 6) return 0;
                        for (allowed = 0;
                             allowed < dungeon->map_creature_type_count[level];
                             ++allowed) {
                            if (dungeon->raw_data[metadata + allowed] ==
                                record[4]) {
                                found = 1;
                                break;
                            }
                        }
                        if (!found) return 0;
                        ++checked_groups;
                        break;
                    }
                    thing = (int)((uint16_t)record[0] |
                                  ((uint16_t)record[1] << 8));
                }
                if (guard == 128 && thing != THING_NONE &&
                    thing != THING_ENDOFLIST) return 0;
            }
        }
    }
    if (out_maps) *out_maps = checked_maps;
    if (out_groups) *out_groups = checked_groups;
    return 1;
}

static int count_events(const CSB_V1_RuntimeProfile *runtime,
                        int event_type,
                        int map_index)
{
    int i;
    int count = 0;
    for (i = 0; i < runtime->timeline_queue.eventCount; ++i) {
        const struct DM1_Event_V1 *event = &runtime->timeline_queue.events[i];
        if (event->type == event_type &&
            DM1_MAP_TIME_MAP(event->map_time) == map_index) {
            ++count;
        }
    }
    return count;
}

static int find_adjacent_attackable_group(
    const CSB_V1_DungeonData *dungeon, int level,
    uint16_t *out_group_thing, int *out_group_x, int *out_group_y,
    int *out_party_x, int *out_party_y)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    int x;
    int y;

    if (!dungeon || !out_group_thing || !out_group_x || !out_group_y ||
        !out_party_x || !out_party_y) return 0;
    for (x = 0; x < dungeon->level_widths[level]; ++x) {
        for (y = 0; y < dungeon->level_heights[level]; ++y) {
            int thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
            int guard;
            for (guard = 0; guard < 128 && thing >= 0 &&
                 thing != THING_NONE && thing != THING_ENDOFLIST; ++guard) {
                int type = -1;
                int size = 0;
                const uint8_t *record = csb_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)thing, &type, NULL, &size);
                if (!record || size < 2) return 0;
                if (type == CSB_V1_THING_TYPE_GROUP && size >= 16) {
                    const int behavior = record[14] & 0x0f;
                    int direction;
                    /* F0195 resets attack/dead/approach groups before their
                     * initial C37.  A wandering/approaching original C04 is
                     * therefore suitable for a source-authentic attack probe. */
                    if ((behavior == 0 || behavior == 2 || behavior == 3 ||
                         behavior >= 4) &&
                        CREATURE_GetProfile_Compat((int)record[4]) &&
                        CREATURE_GetProfile_Compat((int)record[4])->attackTicks != 255 &&
                        CREATURE_GetProfile_Compat((int)record[4])->baseAttack > 0) {
                        for (direction = 0; direction < 4; ++direction) {
                            const int party_x = x + dx[direction];
                            const int party_y = y + dy[direction];
                            if (csb_v1_dungeon_get_square_type(
                                    dungeon, level, party_x, party_y) != 1)
                                continue;
                            *out_group_thing = (uint16_t)thing;
                            *out_group_x = x;
                            *out_group_y = y;
                            *out_party_x = party_x;
                            *out_party_y = party_y;
                            return 1;
                        }
                    }
                    break;
                }
                thing = (int)((uint16_t)record[0] |
                              ((uint16_t)record[1] << 8));
            }
            if (guard == 128) return 0;
        }
    }
    return 0;
}

static int run_real_mini_ranged_projectile_probe(const char *save_path)
{
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_DungeonData *dungeon;
    int chosen_level = -1;
    int chosen_x = -1;
    int chosen_y = -1;
    int party_x = -1;
    int party_y = -1;
    uint16_t chosen_thing = THING_NONE;
    int chosen_type = -1;
    int level;
    int i;

    if (!save_path || !save_path[0]) return 77;
    csb_v1_runtime_init(&runtime, NULL);
    if (csb_v1_runtime_load_game_from_path(&runtime, save_path) !=
            CSB_V1_LOAD_OK || !runtime.dungeon_handle ||
        runtime.level_count != 11) {
        fprintf(stderr,
                "FAIL: original Atari MINI.DAT did not load its 11-level source dungeon\n");
        csb_v1_runtime_cleanup(&runtime);
        return 1;
    }
    dungeon = runtime.dungeon_handle;
    for (level = 0; level < dungeon->level_count && chosen_thing == THING_NONE;
         ++level) {
        int x;
        for (x = 0; x < dungeon->level_widths[level] &&
             chosen_thing == THING_NONE; ++x) {
            int y;
            for (y = 0; y < dungeon->level_heights[level] &&
                 chosen_thing == THING_NONE; ++y) {
                int thing = csb_v1_dungeon_get_first_thing(
                    dungeon, level, x, y);
                int guard;
                for (guard = 0; guard < 128 && thing >= 0 &&
                     thing != THING_NONE && thing != THING_ENDOFLIST; ++guard) {
                    int type = -1;
                    int size = 0;
                    const uint8_t *record = csb_v1_dungeon_get_thing_record(
                        dungeon, (uint16_t)thing, &type, NULL, &size);
                    if (!record || size < 2) break;
                    if (type == CSB_V1_THING_TYPE_GROUP && size >= 16 &&
                        (record[4] == CREATURE_TYPE_SWAMP_SLIME ||
                         record[4] == CREATURE_TYPE_WIZARD_EYE)) {
                        static const int dx[4] = {0, 1, 0, -1};
                        static const int dy[4] = {-1, 0, 1, 0};
                        int direction;
                        for (direction = 0; direction < 4; ++direction) {
                            const int candidate_x = x + dx[direction] * 2;
                            const int candidate_y = y + dy[direction] * 2;
                            if (csb_v1_dungeon_get_square_type(
                                    dungeon, level, x + dx[direction],
                                    y + dy[direction]) != 1 ||
                                csb_v1_dungeon_get_square_type(
                                    dungeon, level, candidate_x,
                                    candidate_y) != 1) continue;
                            chosen_thing = (uint16_t)thing;
                            chosen_type = record[4];
                            chosen_level = level;
                            chosen_x = x;
                            chosen_y = y;
                            party_x = candidate_x;
                            party_y = candidate_y;
                            break;
                        }
                    }
                    thing = (int)((uint16_t)record[0] |
                                  ((uint16_t)record[1] << 8));
                }
            }
        }
    }
    if (chosen_thing == THING_NONE) {
        fprintf(stderr,
                "FAIL: original MINI.DAT has no ranged C04 with two source corridor squares\n");
        csb_v1_runtime_cleanup(&runtime);
        return 1;
    }

    runtime.current_level = chosen_level;
    runtime.party_x = party_x;
    runtime.party_y = party_y;
    runtime.party_state.PartyMapX = party_x;
    runtime.party_state.PartyMapY = party_y;
    csb_v1_dungeon_set_current_level(chosen_level);
    if (csb_v1_runtime_f0195_group_add_all_active_groups(&runtime) < 1) {
        fprintf(stderr,
                "FAIL: source F0195 did not activate ranged C04 at level %d (%d,%d)\n",
                chosen_level, chosen_x, chosen_y);
        csb_v1_runtime_cleanup(&runtime);
        return 1;
    }
    for (i = 0; i < 512 && runtime.projectiles.count == 0; ++i) {
        if (csb_v1_runtime_tick_v1(&runtime) != 1) {
            fprintf(stderr,
                    "FAIL: source runtime stopped during ranged-monster probe\n");
            csb_v1_runtime_cleanup(&runtime);
            return 1;
        }
    }
    if (runtime.projectiles.count == 0) {
        fprintf(stderr,
                "FAIL: original creature type %d at level %d (%d,%d) did not create C14\n",
                chosen_type, chosen_level, chosen_x, chosen_y);
        csb_v1_runtime_cleanup(&runtime);
        return 1;
    }
    printf("PASS: original MINI.DAT creature type %d at level %d (%d,%d) "
           "created %u source C14 projectile(s) while party was at %d,%d\n",
           chosen_type, chosen_level, chosen_x, chosen_y,
           (unsigned int)runtime.projectiles.count, party_x, party_y);
    csb_v1_runtime_cleanup(&runtime);
    return 0;
}

static int append_authentic_c127_champion(
    const CSB_V1_DungeonData *dungeon, CSB_V1_RuntimeProfile *runtime)
{
    struct GameWorld_Compat query_world;
    struct ChampionMirrorCatalog_Compat catalog;
    struct ChampionState_Compat champion;
    uint8_t *snapshot;
    int i;
    int appended = 0;

    if (!dungeon || !runtime || !dungeon->raw_data ||
        dungeon->raw_size < 2 || dungeon->raw_size > INT_MAX) return 0;
    snapshot = (uint8_t *)malloc((size_t)dungeon->raw_size);
    if (!snapshot) return 0;
    memcpy(snapshot, dungeon->raw_data, (size_t)dungeon->raw_size);
    /* Native Atari data is normalized by the campaign loader but preserves
     * the final checksum trailer in source big-endian order. Match M11's
     * private query snapshot before building the source C127 catalogue. */
    {
        uint8_t checksum_byte = snapshot[dungeon->raw_size - 2];
        snapshot[dungeon->raw_size - 2] = snapshot[dungeon->raw_size - 1];
        snapshot[dungeon->raw_size - 1] = checksum_byte;
    }
    memset(&query_world, 0, sizeof(query_world));
    if (!F0882_WORLD_InitFromDungeonDatBuffer_Compat(
            snapshot, dungeon->raw_size,
            (uint32_t)dungeon->ornament_random_seed, &query_world)) {
        free(snapshot);
        return 0;
    }
    free(snapshot);
    memset(&catalog, 0, sizeof(catalog));
    if (!query_world.things ||
        F0652_CHAMPION_BuildMirrorCatalog_Compat(
            query_world.things, &catalog) <= 0) {
        F0883_WORLD_Free_Compat(&query_world);
        return 0;
    }
    for (i = 0; i < catalog.count; ++i) {
        const int ordinal = catalog.records[i].mirrorOrdinal;
        memset(&champion, 0, sizeof(champion));
        if (F0675_CHAMPION_MirrorCatalogCopyRecord_Compat(
                &catalog, ordinal, &champion) &&
            F0634_CHAMPION_HasValidEncodedMirrorFields_Compat(&champion) &&
            csb_v1_runtime_append_mirror_candidate_source_compat(
                runtime, &champion) == 0 &&
            runtime->party_state_valid &&
            runtime->party_state.ChampionCount == 1 &&
            runtime->party_state.Champions[0].CurrentHealth > 0) {
            appended = 1;
            break;
        }
    }
    F0883_WORLD_Free_Compat(&query_world);
    return appended;
}

static int verify_authentic_player_melee_damage(
    CSB_V1_RuntimeProfile *runtime,
    int group_x, int group_y,
    int party_x, int party_y)
{
    static const int melee_actions[] = {
        DM1_ACTION_PUNCH, DM1_ACTION_KICK, DM1_ACTION_STAB_NINJA,
        DM1_ACTION_STAB_FIGHTER, DM1_ACTION_THRUST, DM1_ACTION_JAB,
        DM1_ACTION_MELEE, DM1_ACTION_SLASH, DM1_ACTION_CLEAVE,
        DM1_ACTION_STUN
    };
    CSB_V1_RuntimeMeleeActionResult result;
    int dx = group_x - party_x;
    int dy = group_y - party_y;
    int i;

    if (!runtime || !runtime->party_state_valid ||
        runtime->party_state.ChampionCount < 1) return 0;
    if (dx == 0 && dy == -1) runtime->party_dir = 0;
    else if (dx == 1 && dy == 0) runtime->party_dir = 1;
    else if (dx == 0 && dy == 1) runtime->party_dir = 2;
    else if (dx == -1 && dy == 0) runtime->party_dir = 3;
    else {
        fprintf(stderr,
                "FAIL: authentic Atari combat probe party %d,%d is not adjacent to C04 %d,%d\n",
                party_x, party_y, group_x, group_y);
        return 0;
    }

    runtime->party_x = party_x;
    runtime->party_y = party_y;
    runtime->party_state.PartyMapX = party_x;
    runtime->party_state.PartyMapY = party_y;
    runtime->party_state.PartyDirection = (uint8_t)runtime->party_dir;

    for (i = 0; i < (int)(sizeof(melee_actions) / sizeof(melee_actions[0]));
         ++i) {
        if (!csb_v1_runtime_perform_melee_action(
                runtime, 0, melee_actions[i], &result)) {
            fprintf(stderr,
                    "FAIL: authentic C127 champion rejected CSB melee action %d\n",
                    melee_actions[i]);
            return 0;
        }
        if (result.hit_group && result.damage > 0 &&
            result.creature_hp_after < result.creature_hp_before) {
            printf("PASS: authentic Atari champion action %d reduced C04 0x%04x creature %d from %d to %d HP (damage=%d)\n",
                   melee_actions[i], (unsigned int)result.group_thing,
                   result.creature_index, result.creature_hp_before,
                   result.creature_hp_after, result.damage);
            return 1;
        }
    }

    fprintf(stderr,
            "FAIL: authentic Atari C127 champion's melee actions did not reduce adjacent C04 HP (last-action=%d hit=%d damage=%d HP=%d->%d)\n",
            result.action_index, result.hit_group, result.damage,
            result.creature_hp_before, result.creature_hp_after);
    return 0;
}

int main(void)
{
    const char *source = getenv("FIRESTAFF_CSB_ATARI_STX");
    CSB_V1_BootProfile boot;
    CSB_V1_DungeonData *dungeon;
    CSB_V1_RuntimeProfile *runtime;
    int expected_groups;
    int initial_groups;
    int added_groups;
    int initial_level;
    int validated_maps = 0;
    int validated_groups = 0;
    int c37_dispatches = 0;
    uint16_t adjacent_group = 0u;
    int adjacent_group_x = 0;
    int adjacent_group_y = 0;
    int adjacent_party_x = 0;
    int adjacent_party_y = 0;
    int c38_damage_seen = 0;
    int c38_group_dispatches = 0;
    int last_c38_x = -1;
    int last_c38_y = -1;
    int i;

    if (!source || !source[0]) {
        puts("SKIP: FIRESTAFF_CSB_ATARI_STX not set");
        return 77;
    }

    csb_v1_boot_profile_init(&boot);
    if (csb_v1_boot_scan_assets(&boot, source) != 0 ||
        (boot.variant_id != CSB_V1_VARIANT_ST20_EN &&
         boot.variant_id != CSB_V1_VARIANT_ST21_EN) ||
        csb_v1_boot_enter_game(&boot) != 0) {
        fprintf(stderr, "FAIL: supplied original Atari campaign did not boot\n");
        csb_v1_boot_cleanup(&boot);
        return 1;
    }

    runtime = &boot.runtime;
    dungeon = (CSB_V1_DungeonData *)runtime->dungeon_handle;
    if (!dungeon || dungeon->level_count < 2 || dungeon->square_bytes != 1) {
        fprintf(stderr, "FAIL: original Atari campaign has no source map 1\n");
        csb_v1_boot_cleanup(&boot);
        return 1;
    }

    /* Production boot must run MAIN.C F003's initial F0195 handoff itself.
     * Verify the authentic start map before this test deliberately moves the
     * party-map owner to map 1 below. */
    initial_level = runtime->current_level;
    initial_groups = count_source_groups_on_level(dungeon, initial_level);
    if (initial_groups < 0 ||
        runtime->active_group_state_count != (uint16_t)initial_groups ||
        count_events(runtime, DM1_EVENT_UPDATE_BEHAVIOR_GROUP,
                     initial_level) != initial_groups) {
        fprintf(stderr,
                "FAIL: production Atari boot map %d group handoff expected=%d active=%u C37=%d\n",
                initial_level, initial_groups,
                (unsigned int)runtime->active_group_state_count,
                count_events(runtime, DM1_EVENT_UPDATE_BEHAVIOR_GROUP,
                             initial_level));
        csb_v1_boot_cleanup(&boot);
        return 1;
    }
    if (csb_v1_runtime_tick_v1(runtime) != 1 ||
        csb_v1_runtime_tick_v1(runtime) != 1) {
        fprintf(stderr,
                "FAIL: production Atari boot map %d did not advance two ticks\n",
                initial_level);
        csb_v1_boot_cleanup(&boot);
        return 1;
    }
    {
        int dispatched = 0;
        for (i = 0; i < runtime->last_timeline_dispatch.count; ++i) {
            const struct DM1_DispatchRecord_V1 *record =
                &runtime->last_timeline_dispatch.records[i];
            if (record->eventType == DM1_EVENT_UPDATE_BEHAVIOR_GROUP &&
                record->mapIndex == initial_level) ++dispatched;
        }
        if (dispatched != initial_groups) {
            fprintf(stderr,
                    "FAIL: production Atari boot map %d dispatched %d/%d initial C37 events\n",
                    initial_level, dispatched, initial_groups);
            csb_v1_boot_cleanup(&boot);
            return 1;
        }
    }

    /* This test enters the authentic second map directly to exercise the
     * source F0195 map-load boundary without fabricating a dungeon, group,
     * creature, party, or timeline event. */
    runtime->current_level = 1;
    csb_v1_dungeon_set_current_level(1);
    expected_groups = count_source_groups_on_level(dungeon, 1);
    if (expected_groups <= 0) {
        fprintf(stderr,
                "FAIL: original Atari campaign map 1 has no readable C04 groups\n");
        csb_v1_boot_cleanup(&boot);
        return 1;
    }
    if (!find_adjacent_attackable_group(
            dungeon, 1, &adjacent_group, &adjacent_group_x,
            &adjacent_group_y, &adjacent_party_x, &adjacent_party_y)) {
        fprintf(stderr,
                "FAIL: original Atari map 1 has no C04 adjacent to an open square for an AI probe\n");
        csb_v1_boot_cleanup(&boot);
        return 1;
    }
    /* Use the authentic map and C04 records, adding only the minimum live
     * party presence needed for F0209 to perceive the party during C37. */
    runtime->party_x = adjacent_party_x;
    runtime->party_y = adjacent_party_y;
    if (!append_authentic_c127_champion(dungeon, runtime)) {
        fprintf(stderr,
                "FAIL: original Atari DUNGEON.DAT C127 catalogue did not provide a live champion\n");
        csb_v1_boot_cleanup(&boot);
        return 1;
    }
    if (!source_group_types_match_all_map_metadata(
            dungeon, &validated_maps, &validated_groups)) {
        fprintf(stderr,
                "FAIL: an Atari C04 Type field does not match its map's allowed-creature table\n");
        csb_v1_boot_cleanup(&boot);
        return 1;
    }

    added_groups = csb_v1_runtime_f0195_group_add_all_active_groups(runtime);
    if (added_groups != expected_groups ||
        runtime->active_group_state_count != (uint16_t)expected_groups ||
        count_events(runtime, DM1_EVENT_UPDATE_BEHAVIOR_GROUP, 1) !=
            expected_groups) {
        fprintf(stderr,
                "FAIL: authentic Atari map 1 group handoff expected=%d added=%d active=%u C37=%d\n",
                expected_groups, added_groups,
                (unsigned int)runtime->active_group_state_count,
                count_events(runtime, DM1_EVENT_UPDATE_BEHAVIOR_GROUP, 1));
        csb_v1_boot_cleanup(&boot);
        return 1;
    }

    /* F0180 schedules each group's first C37 at GameTime + 1. F0002
     * processes the source event heap before incrementing GameTime. */
    if (csb_v1_runtime_tick_v1(runtime) != 1 ||
        csb_v1_runtime_tick_v1(runtime) != 1) {
        fprintf(stderr, "FAIL: source runtime did not advance two game ticks\n");
        csb_v1_boot_cleanup(&boot);
        return 1;
    }
    for (i = 0; i < runtime->last_timeline_dispatch.count; ++i) {
        const struct DM1_DispatchRecord_V1 *record =
            &runtime->last_timeline_dispatch.records[i];
        if (record->eventType == DM1_EVENT_UPDATE_BEHAVIOR_GROUP &&
            record->mapIndex == 1) {
            ++c37_dispatches;
        }
    }
    if (c37_dispatches != expected_groups) {
        fprintf(stderr,
                "FAIL: authentic Atari map 1 dispatched %d/%d initial C37 events\n",
                c37_dispatches, expected_groups);
        csb_v1_boot_cleanup(&boot);
        return 1;
    }

    {
        int group_type = -1;
        int group_size = 0;
        const uint8_t *group_record = csb_v1_dungeon_get_thing_record(
            dungeon, adjacent_group, &group_type, NULL, &group_size);
        int attack_events = count_events(
            runtime, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0, 1) +
            count_events(runtime, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_1, 1) +
            count_events(runtime, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_2, 1) +
            count_events(runtime, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3, 1);
        if (!group_record || group_type != CSB_V1_THING_TYPE_GROUP ||
            group_size < 16 || (group_record[14] & 0x0fu) != 6 ||
            attack_events == 0) {
            fprintf(stderr,
                    "FAIL: authentic Atari C04 at %d,%d adjacent to party %d,%d did not enter C6 and queue C38/C39 attacks (behavior=%d events=%d)\n",
                    adjacent_group_x, adjacent_group_y,
                    adjacent_party_x, adjacent_party_y,
                    group_record && group_size >= 16
                        ? (int)(group_record[14] & 0x0fu) : -1,
                    attack_events);
            csb_v1_boot_cleanup(&boot);
            return 1;
        }
    }

    for (i = 0; i < 512 && !c38_damage_seen; ++i) {
        int dispatch_index;
        if (csb_v1_runtime_tick_v1(runtime) != 1) {
            fprintf(stderr,
                    "FAIL: authentic Atari C38 attack timeline stopped before damage\n");
            csb_v1_boot_cleanup(&boot);
            return 1;
        }
        for (dispatch_index = 0;
             dispatch_index < runtime->last_timeline_dispatch.count;
             ++dispatch_index) {
            const struct DM1_DispatchRecord_V1 *record =
                &runtime->last_timeline_dispatch.records[dispatch_index];
            if (record->eventType == DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
                record->mapIndex == 1) {
                ++c38_group_dispatches;
                last_c38_x = record->mapX;
                last_c38_y = record->mapY;
                if (record->mapX == adjacent_group_x &&
                    record->mapY == adjacent_group_y &&
                    runtime->party_state.Champions[0].CurrentHealth <
                        runtime->party_state.Champions[0].MaximumHealth) {
                    c38_damage_seen = 1;
                    break;
                }
            }
        }
    }
    if (!c38_damage_seen) {
        fprintf(stderr,
                "FAIL: authentic Atari C04 0x%04x entered C6 but no C38 damage reached the authentic C127 champion within 512 source ticks (health=%d maximum=%d c38-dispatches=%d last=%d,%d game-time=%u)\n",
                (unsigned int)adjacent_group,
                runtime->party_state.Champions[0].CurrentHealth,
                runtime->party_state.Champions[0].MaximumHealth,
                c38_group_dispatches, last_c38_x, last_c38_y,
                runtime->game_time);
        csb_v1_boot_cleanup(&boot);
        return 1;
    }

    /* ReDMCSB MENU.C F0402 selects the front C04 and PROJEXPL.C F0231
     * applies champion melee damage. Use the authentic map/group records and
     * C127 champion already loaded above; this specifically covers the
     * player-to-monster direction missing from the campaign attack check. */
    if (!verify_authentic_player_melee_damage(
            runtime, adjacent_group_x, adjacent_group_y,
            adjacent_party_x, adjacent_party_y)) {
        csb_v1_boot_cleanup(&boot);
        return 1;
    }

    printf("PASS: %d authentic Atari maps contain %d C04 groups whose Type fields match their allowed-creature tables; production boot map %d admitted and dispatched %d initial C37 events; map 1 admitted %d groups and dispatched all initial C37 events; authentic C04 0x%04x at %d,%d entered C6 from adjacent test party pose %d,%d, queued C38/C39, and damaged authentic C127 champion %s to %d/%d HP\n",
           validated_maps, validated_groups, initial_level, initial_groups,
           expected_groups, (unsigned int)adjacent_group, adjacent_group_x,
           adjacent_group_y, adjacent_party_x, adjacent_party_y,
           runtime->party_state.Champions[0].Name,
           runtime->party_state.Champions[0].CurrentHealth,
           runtime->party_state.Champions[0].MaximumHealth);
    csb_v1_boot_cleanup(&boot);
    {
        const char *mini_path = getenv("FIRESTAFF_CSB_ATARI_MINI");
        if (mini_path && mini_path[0]) {
            return run_real_mini_ranged_projectile_probe(mini_path);
        }
    }
    return 0;
}
