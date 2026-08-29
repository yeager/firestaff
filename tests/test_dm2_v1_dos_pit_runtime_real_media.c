/* Native real-media DOS DM2 open-pit runtime regression. */

#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_dbitem_alloc_pc34_compat.h"
#include "dm2_v1_runtime.h"
#include <string.h>

static int advance_authentic_dos_tick(DM2_V1_BootProfile *profile)
{
    DM2_V1_BootRuntimeReceipt receipt;
    int ok;
    memset(&receipt, 0, sizeof(receipt));
    ok = dm2_v1_boot_runtime_tick(profile, &receipt);
    if (ok && receipt.runtime_ready && receipt.tick_count == 0)
        printf("  DOS boot tick returned zero\n");
    return ok && receipt.runtime_ready;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void __attribute__((unused)) census_authentic_db4_records(
    const DM2_V1_DungeonData *dungeon)
{
    unsigned char seen[65536] = {0};
    int count = 0;
    if (!dungeon) return;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int cursor = dm2_v1_dungeon_get_first_thing(
                    dungeon, map, x, y);
                for (int step = 0; cursor >= 0 && cursor != 0xfffe &&
                         step < 256; ++step) {
                    int type = -1;
                    int size = 0;
                    const uint8_t *record = dm2_v1_dungeon_get_thing_record(
                        dungeon, (uint16_t)cursor, &type, NULL, &size);
                    int next;
                    if (!record || size < 2) break;
                    next = dm2_v1_dungeon_read_record_u16(dungeon, record);
                    if (type == 4 && !seen[(uint16_t)cursor]) {
                        DM2_V1_RuntimeCreatureRecordReceipt info;
                        seen[(uint16_t)cursor] = 1;
                        memset(&info, 0, sizeof(info));
                        if (dm2_v1_runtime_creature_record_receipt(
                                (int16_t)(uint16_t)cursor, &info)) {
                            printf("  DOS DB4 census handle %04x map %d,%d,%d type %d HP %u kill %d possession %04x drops %d\n",
                                   (unsigned)cursor, map, x, y,
                                   info.creature_type, info.hp,
                                   info.kill_flag, info.possession_head,
                                   info.drop_slots_loaded);
                            ++count;
                        }
                    }
                    cursor = next;
                }
            }
        }
    }
    printf("  DOS DB4 census records %d\n", count);
}

static int find_authentic_kill_candidate(
    const DM2_V1_DungeonData *dungeon, int *out_map, int *out_x, int *out_y,
    DM2_V1_RuntimeCreatureRecordReceipt *out_info)
{
    if (!dungeon || !out_map || !out_x || !out_y || !out_info) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int cursor = dm2_v1_dungeon_get_first_thing(
                    dungeon, map, x, y);
                for (int step = 0; cursor >= 0 && cursor != 0xfffe &&
                         step < 256; ++step) {
                    int type = -1;
                    int size = 0;
                    const uint8_t *record = dm2_v1_dungeon_get_thing_record(
                        dungeon, (uint16_t)cursor, &type, NULL, &size);
                    int next;
                    DM2_V1_RuntimeCreatureRecordReceipt info;
                    if (!record || size < 2) break;
                    next = dm2_v1_dungeon_read_record_u16(dungeon, record);
                    memset(&info, 0, sizeof(info));
                    if (type == 4 &&
                        dm2_v1_runtime_creature_record_receipt(
                            (int16_t)(uint16_t)cursor, &info) &&
                        info.kill_flag) {
                        *out_map = map; *out_x = x; *out_y = y;
                        *out_info = info;
                        return 1;
                    }
                    cursor = next;
                }
            }
        }
    }
    return 0;
}

static int find_authentic_record_tile(
    const DM2_V1_DungeonData *dungeon, int16_t handle,
    int *out_map, int *out_x, int *out_y)
{
    if (!dungeon || !out_map || !out_x || !out_y) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int cursor = dm2_v1_dungeon_get_first_thing(
                    dungeon, map, x, y);
                for (int step = 0; cursor >= 0 && cursor != 0xfffe &&
                         step < 256; ++step) {
                    int type = -1;
                    int size = 0;
                    const uint8_t *record = dm2_v1_dungeon_get_thing_record(
                        dungeon, (uint16_t)cursor, &type, NULL, &size);
                    int next;
                    if (!record || size < 2) break;
                    next = dm2_v1_dungeon_read_record_u16(dungeon, record);
                    if ((int16_t)(uint16_t)cursor == handle) {
                        *out_map = map; *out_x = x; *out_y = y;
                        return 1;
                    }
                    cursor = next;
                }
            }
        }
    }
    return 0;
}

static int __attribute__((unused)) exercise_authentic_kill_drop(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    DM2_V1_RuntimeCreatureRecordReceipt target;
    DM2_V1_RuntimeSpellCastReceipt cast;
    DM2_V1_RuntimeCreatureDamageReceipt damage;
    int map, x, y, px = -1, py = -1, dir = -1;
    int hero, hand;
    uint8_t runes[4] = {DM2_RUNE_YA, DM2_RUNE_OH,
                         DM2_RUNE_KATH, DM2_RUNE_RA};

    memset(&target, 0, sizeof(target));
    if (!find_authentic_kill_candidate(dungeon, &map, &x, &y, &target)) {
        printf("  DOS kill candidate not found\n");
        return 77;
    }
    printf("  DOS kill target %04x map %d,%d,%d type %d HP %u armor %u flags %04x drops %d\n",
           (unsigned)(uint16_t)target.record_handle, map, x, y,
           target.creature_type, target.hp,
           dm2_v1_creature_ai_spec(target.creature_type)->ArmorClass,
           dm2_v1_creature_ai_spec(target.creature_type)->w0AIFlags,
           target.drop_slots_loaded);
    for (int candidate_dir = 0; candidate_dir < 4; ++candidate_dir) {
        int candidate_x = x - dx[candidate_dir];
        int candidate_y = y - dy[candidate_dir];
        if (candidate_x >= 0 && candidate_y >= 0 &&
            candidate_x < dungeon->level_widths[map] &&
            candidate_y < dungeon->level_heights[map] &&
            dm2_v1_dungeon_get_square_type(
                dungeon, map, candidate_x, candidate_y) != 0) {
            px = candidate_x; py = candidate_y; dir = candidate_dir;
            break;
        }
    }
    if (dir < 0) { printf("  DOS kill no adjacent passage\n"); return 0; }
    hero = dm2_v1_runtime_get_active_champion_index();
    hand = dm2_v1_runtime_get_active_hand();
    if (hero < 0) {
        DM2_V1_RuntimeSourceHeroStateReceipt hero_state;
        for (int candidate = 0; candidate < DM2_MAX_HEROES; ++candidate) {
            memset(&hero_state, 0, sizeof(hero_state));
            if (dm2_v1_runtime_get_source_hero_state(
                    (uint8_t)candidate, &hero_state) && hero_state.cur_hp > 0) {
                hero = candidate;
                hand = 0;
                (void)dm2_v1_runtime_activate_action_hand(hero, hand);
                break;
            }
        }
    }
    if (hero < 0 || hand < 0) {
        printf("  DOS kill no active hero/hand %d/%d\n", hero, hand);
        return 0;
    }
    dm2_v1_runtime_set_position(map, px, py, dir);
    dm2_v1_runtime_set_outdoor(dm2_v1_dungeon_is_outdoor(dungeon, map));
    if (!dm2_v1_runtime_set_spell_runes(hero, runes, 4)) {
        printf("  DOS kill rune setup rejected\n"); return 0;
    }
    memset(&cast, 0, sizeof(cast));
    if (!dm2_v1_runtime_cast_spell_player(hero, hand, &cast) ||
        !cast.applied) {
        printf("  DOS kill cast rejected valid %d applied %d success %d cost %d mana %d->%d failure %d\n",
               cast.valid, cast.applied, cast.cast.cast_success,
               cast.cast.mana_cost, cast.mana_before, cast.mana_after,
               cast.missile_failure_stage);
        return 0;
    }
    {
        DM2_V1_CreatureScheduleReceipt schedule;
        memset(&schedule, 0, sizeof(schedule));
        if (!dm2_v1_runtime_schedule_creature_at(
                map, x, y, &schedule) || !schedule.valid) {
            printf("  DOS kill schedule rejected at %d,%d\n", x, y);
            return 0;
        }
    }
    memset(&damage, 0, sizeof(damage));
    for (int tick = 0; tick < 64; ++tick) {
        if (dm2_v1_runtime_last_creature_damage_receipt(&damage) &&
            damage.valid && damage.creature_record == target.record_handle &&
            damage.wound_applied && damage.lethal)
            break;
        {
            int live_map, live_x, live_y;
            DM2_V1_CreatureScheduleReceipt schedule;
            if (find_authentic_record_tile(
                    dungeon, target.record_handle,
                    &live_map, &live_x, &live_y)) {
                memset(&schedule, 0, sizeof(schedule));
                int scheduled = dm2_v1_runtime_schedule_creature_at(
                    live_map, live_x, live_y, &schedule);
                if (tick == 0)
                    printf("  DOS kill reschedule tick %d map %d,%d,%d rc %d valid %d resolved %d type %d due %lu ticket %u\n",
                           dm2_v1_runtime_get_tick_count(),
                           live_map, live_x, live_y, scheduled,
                           schedule.valid, schedule.resolved, schedule.timer_type,
                           schedule.due_tick, schedule.timer_ticket);
            }
        }
        if (tick == 0)
            printf("  DOS kill boot tick rc %d\n",
                   advance_authentic_dos_tick(profile));
        else
            advance_authentic_dos_tick(profile);
    }
    if (!damage.valid || !damage.lethal) {
        DM2_V1_RuntimeMissileImpactReceipt impact;
        memset(&impact, 0, sizeof(impact));
        dm2_v1_runtime_last_missile_impact_receipt(&impact);
        {
            DM2_V1_ThinkCreatureReceipt think;
            DM2_V1_RuntimeCreatureRecordReceipt after;
            memset(&think, 0, sizeof(think));
            memset(&after, 0, sizeof(after));
            dm2_v1_runtime_think_creature_receipt(&think);
            dm2_v1_runtime_creature_record_receipt(
                target.record_handle, &after);
            printf("  DOS kill think timers %d resolved %d last %d body %d rejected %d\n",
                   think.think_timers, think.resolved, think.last_record,
                   think.body_consumed, think.body_rejected);
        printf("  DOS kill target after valid %d hp %u word %04x possession %04x caii %u pending %u\n",
                   after.valid, after.hp, after.record_word,
                   after.possession_head, after.caii_slot,
                   after.pending_damage);
        }
        printf("  DOS kill impact valid %d hit %d consumed %d missile %d creature %d damage %d hp %d resched %d\n",
               impact.valid, impact.destination_hit, impact.missile_consumed,
               impact.missile_record, impact.creature_record,
               impact.damage_amount, impact.damage_hp_word_after,
               impact.damage_rescheduled);
        printf("  DOS kill no lethal receipt valid %d record %d HP %d->%d pending %d wound %d\n",
               damage.valid, damage.creature_record, damage.hp_before,
               damage.hp_after, damage.pending_damage, damage.wound_applied);
        return 0;
    }
    printf("  DOS kill candidate %04x map %d,%d,%d type %d HP %d->%d damage %d deallocated %d drops %d\n",
           (unsigned)(uint16_t)target.record_handle, map, x, y,
           target.creature_type, damage.hp_before, damage.hp_after,
           damage.pending_damage, damage.deallocated, damage.drops_placed);
    return damage.deallocated;
}

static int exercise_authentic_pit(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!profile || !dungeon || !dungeon->record_graph_complete) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                if (raw < 0 || dm2_v1_dungeon_get_square_type(dungeon, map, x, y) != 2 ||
                    (raw & 0x08) == 0 || (raw & 0x01) != 0)
                    continue;
                for (int dir = 0; dir < 4; ++dir) {
                    int px = x - dx[dir];
                    int py = y - dy[dir];
                    DM2_V1_BootRuntimeReceipt receipt;
                    if (px < 0 || py < 0 || px >= dungeon->level_widths[map] ||
                        py >= dungeon->level_heights[map] ||
                        dm2_v1_dungeon_get_square_type(dungeon, map, px, py) == 0)
                        continue;
                    dm2_v1_runtime_set_position(map, px, py, dir);
                    dm2_v1_runtime_set_outdoor(
                        dm2_v1_dungeon_is_outdoor(dungeon, map));
                    advance_authentic_dos_tick(profile);
                    memset(&receipt, 0, sizeof(receipt));
                    if (dm2_v1_runtime_move(dir) != 0 ||
                        !dm2_v1_boot_runtime_capture(profile, &receipt) ||
                        receipt.current_level == map)
                        continue;
                    printf("  authentic DOS pit transition map %d,%d,%d -> %d,%d,%d\n",
                           map, px, py, receipt.current_level,
                           receipt.party_x, receipt.party_y);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int census_authentic_stairs(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    unsigned candidates = 0;
    if (!profile || !dungeon || !dungeon->record_graph_complete) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                if (raw < 0 || dm2_v1_dungeon_get_square_type(
                        dungeon, map, x, y) != 3)
                    continue;
                ++candidates;
                printf("  DOS stairs candidate map %d,%d,%d raw=%02x offset=%d,%d\n",
                       map, x, y, raw & 0xff,
                       dungeon->map_offset_x[map], dungeon->map_offset_y[map]);
                for (int dir = 0; dir < 4; ++dir) {
                    int px = x - dx[dir];
                    int py = y - dy[dir];
                    DM2_V1_BootRuntimeReceipt receipt;
                    if (px < 0 || py < 0 ||
                        px >= dungeon->level_widths[map] ||
                        py >= dungeon->level_heights[map] ||
                        dm2_v1_dungeon_get_square_type(
                            dungeon, map, px, py) == 0)
                        continue;
                    dm2_v1_runtime_set_position(map, px, py, dir);
                    dm2_v1_runtime_set_outdoor(
                        dm2_v1_dungeon_is_outdoor(dungeon, map));
                    advance_authentic_dos_tick(profile);
                    memset(&receipt, 0, sizeof(receipt));
                    if (dm2_v1_runtime_move(dir) == 0 &&
                        dm2_v1_boot_runtime_capture(profile, &receipt) &&
                        receipt.current_level != map) {
                        printf("  authentic DOS stairs transition map %d,%d,%d -> %d,%d,%d\n",
                               map, x, y, receipt.current_level,
                               receipt.party_x, receipt.party_y);
                        return 1;
                    }
                }
            }
        }
    }
    printf("  DOS stairs candidates=%u (no positive runtime route)\n",
           candidates);
    return candidates ? -1 : 0;
}

static int exercise_authentic_door(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!profile || !dungeon || !dungeon->record_graph_complete) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                int type = -1;
                int first;
                if (raw < 0 || ((unsigned)raw >> 5) != 4u) continue;
                first = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
                if (first < 0 || !dm2_v1_dungeon_get_thing_record(
                        dungeon, (uint16_t)first, &type, NULL, NULL) ||
                    type != 0)
                    continue;
                for (int dir = 0; dir < 4; ++dir) {
                    int px = x + dx[dir];
                    int py = y + dy[dir];
                    int neighbor = dm2_v1_dungeon_get_tile_raw(
                        dungeon, map, px, py);
                    if (neighbor < 0 || dm2_v1_dungeon_get_square_type(
                            dungeon, map, px, py) == 0)
                        continue;
                    dm2_v1_runtime_set_position(map, px, py, 0);
                    if (dm2_v1_runtime_door_action(map, x, y, 0, 0) != 0)
                        continue;
                    {
                        int before = dm2_v1_runtime_get_door_state(map, x, y);
                        DM2_V1_RuntimeActuatorTileReceipt actuator;
                        DM2_V1_RuntimeDoorStepReceipt step;
                        for (int tick = 0; tick < 6; ++tick)
                            advance_authentic_dos_tick(profile);
                        memset(&actuator, 0, sizeof(actuator));
                        memset(&step, 0, sizeof(step));
                        dm2_v1_runtime_actuator_tile_receipt(&actuator);
                        dm2_v1_runtime_door_step_receipt(&step);
                        if (actuator.door <= 0 || step.mutations <= 0 ||
                            dm2_v1_runtime_get_door_state(map, x, y) != 0)
                            continue;
                        printf("  authentic DOS door action map %d,%d,%d state %d->%d\n",
                               map, x, y, before,
                               dm2_v1_runtime_get_door_state(map, x, y));
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

static int exercise_authentic_active_creature(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    unsigned char framebuffer[320 * 200];
    if (!profile || !dungeon) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        DM2_V1_G1CreatureMapChipRuntimeReceipt materials;
        int base_x = dungeon->level_widths[map] > 1 ? 1 : 0;
        int base_y = dungeon->level_heights[map] > 1 ? 1 : 0;
        dm2_v1_runtime_set_outdoor(dm2_v1_dungeon_is_outdoor(dungeon, map));
        dm2_v1_runtime_set_position(map, base_x, base_y, 0);
        memset(&materials, 0, sizeof(materials));
        if (!dm2_v1_runtime_g1_creature_map_chip_receipt(&materials) ||
            !materials.valid) continue;
        for (int i = 0; i < materials.material_count; ++i) {
            const DM2_V1_G1CreatureMapChipMaterial *material =
                &materials.materials[i];
            DM2_V1_RuntimeCreatureRecordReceipt record_info;
            memset(&record_info, 0, sizeof(record_info));
            if (dm2_v1_runtime_creature_record_receipt(
                    (int16_t)material->object_id, &record_info)) {
                printf("  DOS candidate DB4/F9 map %d,%d,%d type %d HP %u armor %u kill %d possession %04x drops %d\n",
                       map, material->x, material->y, record_info.creature_type,
                       record_info.hp,
                       dm2_v1_creature_ai_spec(record_info.creature_type)->ArmorClass,
                       record_info.kill_flag,
                       record_info.possession_head,
                       record_info.drop_slots_loaded);
            }
            for (int dir = 0; dir < 4; ++dir) {
                int px = material->x - dx[dir];
                int py = material->y - dy[dir];
                DM2_V1_BootRuntimeRenderReceipt render;
                DM2_V1_RuntimeCreatureRenderReceipt creature;
                if (px < 0 || py < 0 || px >= dungeon->level_widths[map] ||
                    py >= dungeon->level_heights[map]) continue;
                dm2_v1_runtime_set_outdoor(dm2_v1_dungeon_is_outdoor(dungeon, map));
                dm2_v1_runtime_set_position(map, px, py, dir);
                memset(framebuffer, 0, sizeof(framebuffer));
                memset(&render, 0, sizeof(render));
                memset(&creature, 0, sizeof(creature));
                (void)dm2_v1_boot_runtime_render_frame(
                    profile, framebuffer, 320, 320, 200, NULL, NULL, &render);
                if (!dm2_v1_runtime_last_creature_render_receipt(&creature))
                    continue;
                if (render.render_result == 0 && render.v1_succeeded &&
                    creature.valid && creature.source_kind == 2 &&
                    creature.thing_handle == material->object_id &&
                    creature.asset_blit_ready && !creature.fallback_drawn &&
                    creature.gdat_index != 0 &&
                    dm2_v1_creature_drop_slots_loaded(creature.creature_type)) {
                    printf("  active DOS creature DB4/F9 map %d,%d,%d type %d GDAT %d\n",
                           map, material->x, material->y,
                           creature.creature_type, creature.gdat_index);
                    /* A source-owned viewport proof is sufficient here.
                     * Positioning a restored party beside arbitrary DB4
                     * records and repeatedly issuing WIELD is not an
                     * authentic player trajectory; it can neither prove a
                     * combat hit nor a drop.  Keep WIELD/drop verification
                     * in its trace-required diagnostic until original input
                     * timing and weapon selection are available. */
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int exercise_authentic_db1(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!profile || !dungeon || !dungeon->record_graph_complete) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                int first;
                int type = -1;
                const uint8_t *record;
                int w2, w4, dest_map, dest_x, dest_y;
                int rotation, rotation_type, expected_dir;
                if (raw < 0 || dm2_v1_dungeon_get_square_type(dungeon, map, x, y) != 5 ||
                    (raw & 0x08) == 0)
                    continue;
                first = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
                if (first < 0 || (((unsigned)first >> 10) & 0xfu) != 1u)
                    continue;
                record = dm2_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)first, &type, NULL, NULL);
                if (!record || type != 1) continue;
                w2 = dm2_v1_dungeon_read_record_u16(dungeon, record + 2);
                w4 = dm2_v1_dungeon_read_record_u16(dungeon, record + 4);
                dest_x = w2 & 0x1f;
                dest_y = (w2 >> 5) & 0x1f;
                dest_map = (w4 >> 8) & 0xff;
                rotation = (w2 >> 10) & 3;
                rotation_type = (w2 >> 12) & 1;
                if (dest_map < 0 || dest_map >= dungeon->level_count ||
                    dest_x >= dungeon->level_widths[dest_map] ||
                    dest_y >= dungeon->level_heights[dest_map] ||
                    (w2 & 0x6000) != 0x4000)
                    continue;
                for (int dir = 0; dir < 4; ++dir) {
                    int px = x - dx[dir];
                    int py = y - dy[dir];
                    DM2_V1_BootRuntimeReceipt receipt;
                    if (px < 0 || py < 0 || px >= dungeon->level_widths[map] ||
                        py >= dungeon->level_heights[map] ||
                        dm2_v1_dungeon_get_square_type(dungeon, map, px, py) == 0)
                        continue;
                    dm2_v1_runtime_set_position(map, px, py, dir);
                    dm2_v1_runtime_set_outdoor(
                        dm2_v1_dungeon_is_outdoor(dungeon, map));
                    advance_authentic_dos_tick(profile);
                    expected_dir = rotation_type ? rotation : ((dir + rotation) & 3);
                    memset(&receipt, 0, sizeof(receipt));
                    if (dm2_v1_runtime_move(dir) != 0 ||
                        !dm2_v1_boot_runtime_capture(profile, &receipt))
                        continue;
                    if (receipt.current_level == dest_map &&
                        receipt.party_x == dest_x && receipt.party_y == dest_y &&
                        receipt.party_dir == expected_dir) {
                        printf("  authentic DOS DB1 transition map %d,%d,%d -> %d,%d,%d\n",
                               map, x, y, dest_map, dest_x, dest_y);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    DM2_V1_BootStartupLaunch launch;
    DM2_V1_BootProfile *profile;
    DM2_V1_BootRuntimeReceipt runtime;
    int ok;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is not set");
        return 77;
    }
    memset(&launch, 0, sizeof(launch));
    /* The real DOS distribution stays mounted as a ZIP.  The boot owner
     * resolves its data members through virtual archive paths in RAM. */
    if (!dm2_v1_boot_startup_launch_alloc(archive, &launch) ||
        !launch.profile || !launch.profile->assets_verified) {
        fprintf(stderr, "FAIL: native DOS DM2 archive was not admitted\n");
        dm2_v1_boot_startup_launch_cleanup(&launch);
        return 1;
    }
    profile = launch.profile;
    if (!dm2_v1_boot_prepare_new_game_world(profile) ||
        !dm2_v1_boot_commit_new_game_session(profile)) {
        fprintf(stderr, "FAIL: DOS NEW GAME did not commit\n");
        dm2_v1_boot_startup_launch_cleanup(&launch);
        return 1;
    }
    memset(&runtime, 0, sizeof(runtime));
    ok = dm2_v1_boot_runtime_capture(profile, &runtime) && runtime.runtime_ready &&
         exercise_authentic_pit(
             profile, (const DM2_V1_DungeonData *)profile->dungeon_data) &&
         census_authentic_stairs(
             profile, (const DM2_V1_DungeonData *)profile->dungeon_data) == 1 &&
         exercise_authentic_door(
             profile, (const DM2_V1_DungeonData *)profile->dungeon_data) &&
         exercise_authentic_active_creature(
             profile, (const DM2_V1_DungeonData *)profile->dungeon_data) &&
         exercise_authentic_db1(
             profile, (const DM2_V1_DungeonData *)profile->dungeon_data);
    dm2_v1_boot_startup_launch_cleanup(&launch);
    if (!ok) {
        fprintf(stderr, "FAIL: DOS did not commit an authentic open-pit transition\n");
        return 1;
    }
    puts("PASS: authentic DOS DM2 pit reaches active runtime");
    return 0;
}
