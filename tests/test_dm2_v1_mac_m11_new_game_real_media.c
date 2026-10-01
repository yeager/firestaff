/* Opt-in real-media Mac M11 startup -> NEW GAME -> active-session regression. */

#include "m11_game_view.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_gdat_hud_m11_command.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_spell.h"
#include "dm2_v1_weather_gdat.h"
#include "dm2_v1_mac_input.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int find_mac_creature_material(
    const DM2_V1_DungeonData *dungeon, int map, int16_t creature_record,
    DM2_V1_G1CreatureMapChipMaterial *out);

static int mac_non_c4_palette_is_identity(const uint8_t palette[16])
{
    if (!palette) return 0;
    for (int color = 0; color < 16; ++color) {
        if (palette[color] != (uint8_t)color) return 0;
    }
    return 1;
}

static uint32_t mac_dungeon_view_hash(const unsigned char *framebuffer)
{
    uint32_t hash = 2166136261u;
    if (!framebuffer) return 0u;
    for (int y = 40; y < 176; ++y) {
        for (int x = 0; x < 224; ++x) {
            hash ^= framebuffer[(size_t)y * 320u + (size_t)x];
            hash *= 16777619u;
        }
    }
    return hash ? hash : 1u;
}

static size_t mac_nonzero_in_rect(const unsigned char *framebuffer,
                                  int x, int y, int width, int height)
{
    size_t count = 0u;
    if (!framebuffer || x < 0 || y < 0 || width <= 0 || height <= 0 ||
        x + width > 320 || y + height > 200) return 0u;
    for (int row = y; row < y + height; ++row)
        for (int col = x; col < x + width; ++col)
            if (framebuffer[(size_t)row * 320u + (size_t)col] != 0u)
                ++count;
    return count;
}

static int mac_live_map_has_nearby_creature(
    const DM2_V1_DungeonData *dungeon, int map, int party_x, int party_y,
    int *out_x, int *out_y)
{
    if (out_x) *out_x = -1;
    if (out_y) *out_y = -1;
    if (!dungeon || map < 0 || map >= dungeon->level_count) return -1;
    for (int y = 0; y < dungeon->level_heights[map]; ++y) {
        for (int x = 0; x < dungeon->level_widths[map]; ++x) {
            DM2_V1_RuntimeCreatureRecordReceipt record;
            int16_t handle = DM2_V1_RECORD_HANDLE_NULL;
            int dx = x - party_x;
            int dy = y - party_y;
            if (!dm2_v1_runtime_query_creature_at(map, x, y, &handle))
                return -1;
            if (handle == DM2_V1_RECORD_HANDLE_NULL) continue;
            memset(&record, 0, sizeof(record));
            if (!dm2_v1_runtime_creature_record_receipt(handle, &record) ||
                !record.valid) return -1;
            if (!record.kill_flag && dx >= -1 && dx <= 1 &&
                dy >= -1 && dy <= 1) {
                if (out_x) *out_x = x;
                if (out_y) *out_y = y;
                return 1;
            }
        }
    }
    return 0;
}

static int exercise_authentic_active_mac_creature(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon,
    unsigned char *framebuffer, M11_GameViewState *view)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };

    if (!profile || !dungeon || !framebuffer || !view) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        DM2_V1_G1CreatureMapChipRuntimeReceipt materials;
        dm2_v1_runtime_set_outdoor(dm2_v1_dungeon_is_outdoor(dungeon, map));
        dm2_v1_runtime_set_position(map, 1, 1, 0);
        memset(&materials, 0, sizeof(materials));
        if (!dm2_v1_runtime_g1_creature_map_chip_receipt(&materials) ||
            !materials.valid) continue;
        for (int i = 0; i < materials.material_count; ++i) {
            const DM2_V1_G1CreatureMapChipMaterial *material =
                &materials.materials[i];
            const DM2_AIDefinition *ai = NULL;
            DM2_V1_RuntimeCreatureRecordReceipt record_info;
            if (!dm2_v1_creature_ai_spec_def(material->creature_type, &ai) ||
                !ai || ai->ArmorClass == 0xffu ||
                (ai->w0AIFlags & DM2_AIFLAG_STATIC) == 0u)
                continue;
            memset(&record_info, 0, sizeof(record_info));
            if (dm2_v1_runtime_creature_record_receipt(
                    (int16_t)material->object_id, &record_info)) {
                printf("Mac candidate DB4/F9 map %d,%d,%d type %d HP %u kill %d possession %04x drops %d\n",
                       map, material->x, material->y, record_info.creature_type,
                       record_info.hp, record_info.kill_flag,
                       record_info.possession_head,
                       record_info.drop_slots_loaded);
            }
            for (int dir = 0; dir < 4; ++dir) {
                int px = material->x - dx[dir];
                int py = material->y - dy[dir];
                DM2_V1_BootRuntimeRenderReceipt render;
                DM2_V1_RuntimeCreatureRenderReceipt creature;

                if (px < 0 || py < 0 ||
                    px >= dungeon->level_widths[map] ||
                    py >= dungeon->level_heights[map]) continue;
                dm2_v1_runtime_set_outdoor(
                    dm2_v1_dungeon_is_outdoor(dungeon, map));
                dm2_v1_runtime_set_position(map, px, py, dir);
                memset(framebuffer, 0, M11_FB_BYTES);
                memset(&render, 0, sizeof(render));
                memset(&creature, 0, sizeof(creature));
                (void)dm2_v1_boot_runtime_render_frame(
                    profile, framebuffer, M11_FB_WIDTH, M11_FB_WIDTH,
                    M11_FB_HEIGHT, NULL, NULL, &render);
                if (!dm2_v1_runtime_last_creature_render_receipt(&creature) ||
                    render.render_result != 0 || !render.v1_succeeded ||
                    !creature.valid || creature.source_kind != 2 ||
                    creature.thing_handle != material->object_id ||
                    !creature.asset_blit_ready || creature.fallback_drawn ||
                    creature.gdat_index == 0) continue;
                printf("Mac active creature DB4/F9 map %d,%d,%d type %d GDAT %d armor %u\n",
                       map, material->x, material->y,
                       creature.creature_type, creature.gdat_index,
                       ai->ArmorClass);
                dm2_v1_runtime_set_outdoor(
                    dm2_v1_dungeon_is_outdoor(dungeon, map));
                dm2_v1_runtime_set_position(map, px, py, dir);
                /* Put an authentic DB4 one cell ahead of the party and use
                 * the actual M11 spell path.  A successful 0x1e dispatch is
                 * not enough: the destination-cell owner must consume the
                 * DB14 missile instead of endlessly re-queueing it. */
                if (!M11_GameView_OpenSpellPanel(view)) return 0;
                view->spellBuffer.runes[0] = DM2_RUNE_YA;
                view->spellBuffer.runes[1] = DM2_RUNE_FUL;
                view->spellBuffer.runes[2] = DM2_RUNE_IR;
                view->spellBuffer.runeCount = 3;
                if (!M11_GameView_CastSpell(view) || view->spellPanelOpen)
                    return 0;
                {
                    DM2_V1_CreatureScheduleReceipt post_cast_schedule;
                    memset(&post_cast_schedule, 0, sizeof(post_cast_schedule));
                    if (!dm2_v1_runtime_schedule_creature_at(
                            map, material->x, material->y,
                            &post_cast_schedule) || !post_cast_schedule.valid)
                        return 0;
                }
                for (int tick = 0; tick < 4; ++tick)
                    (void)M11_GameView_AdvanceIdleTick(view);
                {
                    DM2_V1_RuntimeMissileImpactReceipt impact;
                    memset(&impact, 0, sizeof(impact));
                    if (!dm2_v1_runtime_last_missile_impact_receipt(&impact) ||
                        !impact.valid || !impact.destination_hit ||
                        !impact.hp_applied ||
                        !impact.missile_consumed || impact.damage_amount <= 0 ||
                        impact.damage_hp_word_after <= 0) {
                        return 0;
                    }
                    printf("Mac Fireball impact creature %d damage %d CAII %d missile consumed %d\n",
                           impact.creature_record, impact.damage_amount,
                           impact.damage_hp_word_after, impact.missile_consumed);
                    {
                        DM2_V1_RuntimeCreatureDamageReceipt damage;
                        DM2_V1_G1CreatureMapChipMaterial current;
                        int wound_seen = 0;
                        for (int tick = 0; tick < 32; ++tick) {
                            memset(&damage, 0, sizeof(damage));
                            if (dm2_v1_runtime_last_creature_damage_receipt(
                                    &damage) && damage.valid &&
                                damage.creature_record == impact.creature_record &&
                                damage.pending_damage > 0 &&
                                damage.wound_applied) {
                                wound_seen = 1;
                                break;
                            }
                            (void)M11_GameView_AdvanceIdleTick(view);
                        }
                        if (!wound_seen) return 0;
                        printf("Mac Fireball WOUND creature %d HP %d->%d damage %d\n",
                               damage.creature_record, damage.hp_before,
                               damage.hp_after, damage.pending_damage);
                        for (int shot = 1; shot < 6; ++shot) {
                            DM2_V1_CreatureScheduleReceipt schedule;
                            DM2_V1_RuntimeMissileImpactReceipt repeat_impact;
                            int repeated = 0;
                            int shot_px = -1;
                            int shot_py = -1;
                            int shot_dir = -1;
                            /* Let the source action cooldown expire before
                             * selecting the creature's new authentic tile. */
                            for (int wait_tick = 0; wait_tick < 32; ++wait_tick)
                                (void)M11_GameView_AdvanceIdleTick(view);
                            if (!find_mac_creature_material(
                                    dungeon, map, impact.creature_record,
                                    &current)) {
                                printf("Mac repeat Lightning %d target record %d no longer in G1 map\n",
                                       shot, impact.creature_record);
                                break;
                            }
                            for (int candidate_dir = 0; candidate_dir < 4;
                                 ++candidate_dir) {
                                int candidate_x = current.x -
                                    dx[candidate_dir];
                                int candidate_y = current.y -
                                    dy[candidate_dir];
                                if (candidate_x >= 0 && candidate_y >= 0 &&
                                    candidate_x < dungeon->level_widths[map] &&
                                    candidate_y < dungeon->level_heights[map] &&
                                    dm2_v1_dungeon_get_square_type(
                                        dungeon, map, candidate_x,
                                        candidate_y) != 0) {
                                    shot_px = candidate_x;
                                    shot_py = candidate_y;
                                    shot_dir = candidate_dir;
                                    break;
                                }
                            }
                            if (shot_dir < 0) break;
                            dm2_v1_runtime_set_outdoor(
                                dm2_v1_dungeon_is_outdoor(dungeon, map));
                            dm2_v1_runtime_set_position(
                                map, shot_px, shot_py, shot_dir);
                            if (!M11_GameView_OpenSpellPanel(view)) {
                                printf("Mac repeat Lightning %d spell panel unavailable\n", shot);
                                break;
                            }
                            /* Source dSpellsTable index 15: Lightning is
                             * YA OH KATH RA (OH KATH RA after the power rune). */
                            view->spellBuffer.runes[0] = DM2_RUNE_YA;
                            view->spellBuffer.runes[1] = DM2_RUNE_OH;
                            view->spellBuffer.runes[2] = DM2_RUNE_KATH;
                            view->spellBuffer.runes[3] = DM2_RUNE_RA;
                            view->spellBuffer.runeCount = 4;
                            if (!M11_GameView_CastSpell(view) ||
                                view->spellPanelOpen) {
                                printf("Mac repeat Lightning %d cast rejected\n", shot);
                                break;
                            }
                            memset(&schedule, 0, sizeof(schedule));
                            if (!dm2_v1_runtime_schedule_creature_at(
                                    map, current.x, current.y,
                                    &schedule) || !schedule.valid) {
                                printf("Mac repeat Lightning %d schedule rejected at %d,%d\n",
                                       shot, current.x, current.y);
                                break;
                            }
                            for (int tick = 0; tick < 4; ++tick)
                                (void)M11_GameView_AdvanceIdleTick(view);
                            memset(&repeat_impact, 0, sizeof(repeat_impact));
                            if (!dm2_v1_runtime_last_missile_impact_receipt(
                                    &repeat_impact) || !repeat_impact.valid ||
                                !repeat_impact.destination_hit ||
                                !repeat_impact.missile_consumed) {
                                printf("Mac repeat Lightning %d impact rejected valid %d hit %d consumed %d missile %d creature %d damage %d hp %d resched %d\n",
                                       shot, repeat_impact.valid,
                                       repeat_impact.destination_hit,
                                       repeat_impact.missile_consumed,
                                       repeat_impact.missile_record,
                                       repeat_impact.creature_record,
                                       repeat_impact.damage_amount,
                                       repeat_impact.damage_hp_word_after,
                                       repeat_impact.damage_rescheduled);
                                break;
                            }
                            (void)M11_GameView_AdvanceIdleTick(view);
                            for (int tick = 0; tick < 32; ++tick) {
                                memset(&damage, 0, sizeof(damage));
                                if (dm2_v1_runtime_last_creature_damage_receipt(
                                        &damage) && damage.valid &&
                                    damage.creature_record ==
                                        repeat_impact.creature_record &&
                                    damage.pending_damage > 0 &&
                                    damage.wound_applied) {
                                    repeated = 1;
                                    printf("Mac repeat Lightning %d HP %d->%d damage %d lethal %d deallocated %d drops %d\n",
                                           shot, damage.hp_before,
                                           damage.hp_after,
                                           damage.pending_damage, damage.lethal,
                                           damage.deallocated,
                                           damage.drops_placed);
                                    if (damage.lethal || damage.deallocated ||
                                        damage.drops_placed > 0)
                                        break;
                                    break;
                                }
                                (void)M11_GameView_AdvanceIdleTick(view);
                            }
                            if (!repeated) break;
                            if (damage.lethal || damage.deallocated ||
                                damage.drops_placed > 0)
                                break;
                        }
                        if (damage.lethal && !damage.deallocated) {
                            int gone = 0;
                            for (int death_tick = 0; death_tick < 96;
                                 ++death_tick) {
                                (void)M11_GameView_AdvanceIdleTick(view);
                                if (!find_mac_creature_material(
                                        dungeon, map,
                                        impact.creature_record, &current)) {
                                    gone = 1;
                                    break;
                                }
                            }
                            printf("Mac post-lethal creature %d deallocated %d drops %d\n",
                                   impact.creature_record, gone,
                                   damage.drops_placed);
                        }
                    }
                }
                printf("Mac dynamic path attempts %d admissions %d\n",
                       dm2_v1_runtime_dynamic_path_attempts(),
                       dm2_v1_runtime_dynamic_path_admissions());
                printf("Mac dynamic path last failure %d\n",
                       dm2_v1_runtime_dynamic_path_last_failure());
                printf("Mac dynamic move queues %d\n",
                       dm2_v1_runtime_dynamic_move_queue_admissions());
                for (int tick = 0; tick < 4; ++tick)
                    dm2_v1_runtime_tick();
                printf("Mac dynamic move timers %d successes %d\n",
                       dm2_v1_runtime_dynamic_move_timer_consumptions(),
                       dm2_v1_runtime_dynamic_move_successes());
                printf("Mac dynamic move last failure %d\n",
                       dm2_v1_runtime_dynamic_move_last_failure());
                if (dm2_v1_runtime_dynamic_path_admissions() <= 0 ||
                    dm2_v1_runtime_dynamic_move_queue_admissions() <= 0 ||
                    dm2_v1_runtime_dynamic_move_timer_consumptions() <= 0 ||
                    dm2_v1_runtime_dynamic_move_successes() <= 0)
                    return 0;
                return dm2_v1_runtime_get_projectile_drain(NULL) == 0;
            }
        }
    }
    return 0;
}

static int find_mac_creature_material(
    const DM2_V1_DungeonData *dungeon, int map, int16_t creature_record,
    DM2_V1_G1CreatureMapChipMaterial *out)
{
    DM2_V1_G1CreatureMapChipRuntimeReceipt materials;

    if (!dungeon || !out || creature_record < 0) return 0;
    dm2_v1_runtime_set_outdoor(dm2_v1_dungeon_is_outdoor(dungeon, map));
    dm2_v1_runtime_set_position(map, 1, 1, 0);
    memset(&materials, 0, sizeof(materials));
    if (!dm2_v1_runtime_g1_creature_map_chip_receipt(&materials) ||
        !materials.valid || materials.map != map)
        return 0;
    for (int i = 0; i < materials.material_count; ++i) {
        if ((int16_t)materials.materials[i].object_id == creature_record) {
            *out = materials.materials[i];
            return 1;
        }
    }
    return 0;
}

static int exercise_authentic_mac_stairs(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!profile || !dungeon || !dungeon->record_graph_complete) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
        int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                if (raw < 0 || dm2_v1_dungeon_get_square_type(
                        dungeon, map, x, y) != 3)
                    continue;
                for (int dir = 0; dir < 4; ++dir) {
                    int px = x - dx[dir], py = y - dy[dir];
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
                    dm2_v1_runtime_tick();
                    memset(&receipt, 0, sizeof(receipt));
                    if (dm2_v1_runtime_move(dir) == 0 &&
                        dm2_v1_boot_runtime_capture(profile, &receipt) &&
                        receipt.current_level != map) {
                        printf("Mac stairs transition map %d,%d,%d -> %d,%d,%d\n",
                               map, x, y, receipt.current_level,
                               receipt.party_x, receipt.party_y);
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
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    DM2_V1_StartupMenuPointerLayout layout;
    DM2_V1_BootRuntimeReceipt runtime;
    unsigned char framebuffer[320u * 200u];
    int frame;

    if (!zip || !zip[0]) {
        puts("SKIP: FIRESTAFF_DM2_MAC_EN_ZIP is not set");
        return 77;
    }
    memset(&view, 0, sizeof(view));
    memset(&spec, 0, sizeof(spec));
    spec.title = "Dungeon Master II Macintosh";
    spec.gameId = "dm2";
    spec.dataDir = zip;
    spec.sourceId = "mac-en-retail";
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.launcherOptionsBound = 1;
    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec) || !view.dm2BootProfile) {
        fprintf(stderr, "FAIL: Mac M11 start did not bind the boot profile\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    /* This is a bounded source-frame regression, not a wall-clock playback
     * test. Use the explicit M11 boot-probe fast-forward mode so each draw
     * consumes one authentic QuickTime frame without making CI wait for the
     * complete retail title movie. */
    M11_GameView_SetBootProbeMode(&view, 1);
    memset(framebuffer, 0, sizeof(framebuffer));
    for (frame = 0; view.dm2MacMovieActive && frame < 4000; ++frame)
        M11_GameView_Draw(&view, framebuffer, 320, 200);
    if (view.dm2MacMovieActive || !view.dm2State.startup_menu_active) {
        fprintf(stderr, "FAIL: Mac M11 did not reach the source menu\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    memset(&layout, 0, sizeof(layout));
    if (!dm2_v1_boot_startup_menu_pointer_layout(
            (DM2_V1_BootProfile *)view.dm2BootProfile, &layout) ||
        !layout.valid || layout.new_game.w <= 0 || layout.new_game.h <= 0 ||
        M11_GameView_HandlePointerButton(
            &view, layout.new_game.x + layout.new_game.w / 2,
            layout.new_game.y + layout.new_game.h / 2,
            DM1_V1_MOUSE_MASK_LEFT_PC34) != M11_GAME_INPUT_REDRAW ||
        !view.dm2State.startup_menu_active || view.dm2State.level_loaded) {
        fprintf(stderr, "FAIL: Mac M11 NEW GAME did not enter source preselection\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    /* Resolve Mac's source mirror selection through the same native viewport
     * input used by the player. Never reuse a previous test's global runtime. */
    view.dm2State.music_elapsed_us = 123456u;
    if (M11_GameView_HandlePointerButton(
            &view, 112, 130, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        !((DM2_V1_BootProfile *)view.dm2BootProfile)
             ->source_game_load_session_ready ||
        view.dm2State.music_elapsed_us != 0u ||
        !dm2_v1_boot_runtime_capture(
            (DM2_V1_BootProfile *)view.dm2BootProfile, &runtime) ||
        !runtime.runtime_ready) {
        fprintf(stderr,
                "FAIL: Mac source mirror selection did not complete GAME_LOAD and reset the music clock\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    /* Check the session pose immediately after the real New Game/mirror
     * input. Later portions of this test deliberately teleport the runtime
     * to exercise render assets, so they cannot guard the spawn location. */
    if (runtime.current_level != 0 || runtime.party_x != 1 ||
        runtime.party_y != 8 || runtime.party_dir != 0) {
        fprintf(stderr,
                "FAIL: Mac New Game spawned at map %d (%d,%d) facing %d; expected source start (0,1,8) facing north\n",
                runtime.current_level, runtime.party_x, runtime.party_y,
                runtime.party_dir);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    {
        DM2_V1_G1CreatureMapChipRuntimeReceipt creatures;
        memset(&creatures, 0, sizeof(creatures));
        if (!dm2_v1_runtime_g1_creature_map_chip_receipt(&creatures) ||
            !creatures.valid || creatures.map != runtime.current_level) {
            fprintf(stderr,
                    "FAIL: Mac New Game could not inspect authentic creatures on the spawn map\n");
            M11_GameView_Shutdown(&view);
            return 1;
        }
        for (int i = 0; i < creatures.material_count; ++i) {
            const DM2_V1_G1CreatureMapChipMaterial *creature =
                &creatures.materials[i];
            int dx = creature->x - runtime.party_x;
            int dy = creature->y - runtime.party_y;
            if (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1 &&
                (dx != 0 || dy != 0)) {
                fprintf(stderr,
                        "FAIL: Mac New Game spawned within one tile of source creature type %u at (%d,%d); party at (%d,%d)\n",
                        creature->creature_type, creature->x, creature->y,
                        runtime.party_x, runtime.party_y);
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
    }
    view.dm2State.startup_menu_active = 0;
    view.dm2State.level_loaded = 1;
    /* The retail map-chip census above catches a creature already occupying
     * a neighboring tile. Also let the real spawn session run long enough for
     * its source creature scheduler to act, then query the live DB4 chains.
     * This matches the reported "monster beside me" symptom more closely
     * than inspecting the immutable dungeon bytes alone. */
    for (int tick = 0; tick < 16; ++tick)
        (void)M11_GameView_AdvanceIdleTick(&view);
    {
        const DM2_V1_DungeonData *spawn_dungeon =
            (const DM2_V1_DungeonData *)
                ((DM2_V1_BootProfile *)view.dm2BootProfile)->dungeon_data;
        DM2_V1_BootRuntimeReceipt spawn_after_ticks;
        int nearby;
        int nearby_x;
        int nearby_y;
        memset(&spawn_after_ticks, 0, sizeof(spawn_after_ticks));
        if (!dm2_v1_boot_runtime_capture(
                (DM2_V1_BootProfile *)view.dm2BootProfile,
                &spawn_after_ticks) || !spawn_after_ticks.runtime_ready ||
            spawn_after_ticks.current_level != 0 ||
            spawn_after_ticks.party_x != 1 || spawn_after_ticks.party_y != 8 ||
            spawn_after_ticks.party_dir != 0) {
            fprintf(stderr,
                    "FAIL: Mac spawn pose changed during 16 source ticks "
                    "(ready=%d map=%d party=%d,%d,%d)\n",
                    spawn_after_ticks.runtime_ready,
                    spawn_after_ticks.current_level,
                    spawn_after_ticks.party_x, spawn_after_ticks.party_y,
                    spawn_after_ticks.party_dir);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        nearby = mac_live_map_has_nearby_creature(
            spawn_dungeon, spawn_after_ticks.current_level,
            spawn_after_ticks.party_x, spawn_after_ticks.party_y,
            &nearby_x, &nearby_y);
        if (nearby != 0) {
            fprintf(stderr,
                    "FAIL: live Mac DB4 creature is within one tile of "
                    "New Game spawn after 16 source ticks "
                    "(nearby=%d creature=%d,%d party=%d,%d)\n",
                    nearby, nearby_x, nearby_y,
                    spawn_after_ticks.party_x, spawn_after_ticks.party_y);
            M11_GameView_Shutdown(&view);
            return 1;
        }
    }
    {
        const DM2_V1_DungeonData *dungeon =
            (const DM2_V1_DungeonData *)
                ((DM2_V1_BootProfile *)view.dm2BootProfile)->dungeon_data;
        int raw = -1;
        int middle_raw = -1;
        if (!dungeon || dungeon->level_count < 1 ||
            !dungeon->initial_party_pose_valid ||
            dungeon->initial_party_x != 1 || dungeon->initial_party_y != 8 ||
            dungeon->initial_party_dir != 0 ||
            (middle_raw = dm2_v1_dungeon_c_map_get_tile_value(
                 dungeon, 0, dungeon->initial_party_x,
                 dungeon->initial_party_y - 1)) < 0 ||
            (raw = dm2_v1_dungeon_c_map_get_tile_value(
                 dungeon, 0, dungeon->initial_party_x,
                 dungeon->initial_party_y - 2)) < 0 ||
            dm2_v1_viewport_g1_tile_class_to_square_type(
                (uint8_t)((unsigned int)middle_raw >> 5)) != DM2_SQUARE_FLOOR ||
            dm2_v1_viewport_g1_tile_class_to_square_type(
                (uint8_t)((unsigned int)raw >> 5)) != DM2_SQUARE_WALL) {
            fprintf(stderr,
                    "FAIL: Mac retail start corridor no longer has its source "
                    "floor at distance 1 and wall at distance 2 "
                    "(middle=%d far=%d)\n", middle_raw, raw);
            M11_GameView_Shutdown(&view);
            return 1;
        }
    }
    {
        DM2_V1_BootProfile *profile =
            (DM2_V1_BootProfile *)view.dm2BootProfile;
        DM2_V1_DungeonData *dungeon = profile
            ? (DM2_V1_DungeonData *)profile->dungeon_data : NULL;
        DM2_V1_GdatSceneM11CommandPlan scene_plan;
        DM2_V1_GdatWallM11CommandPlan wall_plan;
        DM2_V1_GdatWallM11CommandPlan expected_wall_plan;
        static const int16_t mac_arrow_positions[6][2] = {
            { 229, 129 }, { 291, 129 }, { 260, 129 },
            { 291, 153 }, { 260, 153 }, { 229, 153 }
        };
        for (uint16_t rect_id = 40; rect_id <= 45; ++rect_id) {
            DM2_V1_InterfaceRect rect;
            int slot = (int)(rect_id - 40u);
            if (!dm2_v1_boot_query_blit_rect_for_dimensions(
                    profile, rect_id, 29, 23, &rect) ||
                rect.x != mac_arrow_positions[slot][0] ||
                rect.y != mac_arrow_positions[slot][1] ||
                rect.w != 29 || rect.h != 23) {
                fprintf(stderr,
                        "FAIL: Mac movement arrow RECT_%03u did not match its retail RAW4 placement\n",
                        rect_id);
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
        {
            DM2_V1_GdatHudM11CommandPlan mac_hud;
            memset(&mac_hud, 0, sizeof(mac_hud));
            if (!dm2_v1_boot_gdat_hud_static_m11_command_plan(
                    profile, 0, &mac_hud) || !mac_hud.mac_native_layout ||
                mac_hud.command_count != 6) {
                fprintf(stderr,
                        "FAIL: Mac HUD did not bind its six retail movement images\n");
                dm2_v1_gdat_hud_m11_command_plan_free(&mac_hud);
                M11_GameView_Shutdown(&view);
                return 1;
            }
            for (int arrow = 0; arrow < 6; ++arrow) {
                const DM2_V1_GdatHudM11Command *command =
                    &mac_hud.commands[arrow];
                uint8_t source_palette[16];
                uint32_t source_palette_hash = 0u;
                if (command->kind != DM2_V1_GDAT_HUD_M11_COMMAND_MOVE_ARROW ||
                    command->gdat_category !=
                        DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
                    command->gdat_index != 3 ||
                    command->gdat_field != 2 + arrow * 2 ||
                    command->destination_rect_id != (uint16_t)(40 + arrow) ||
                    command->destination.x != mac_arrow_positions[arrow][0] ||
                    command->destination.y != mac_arrow_positions[arrow][1] ||
                    command->width != 29 || command->height != 23 ||
                    !command->pixels || command->decoded_hash == 0u ||
                    command->palette_hash == 0u ||
                    !dm2_v1_asset_load_image_local_palette(
                        dm2_v1_boot_asset_loader(profile),
                        DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 3,
                        2 + arrow * 2, source_palette,
                        &source_palette_hash) ||
                    memcmp(command->palette16, source_palette,
                           sizeof(source_palette)) != 0 ||
                    command->palette_hash != source_palette_hash) {
                    fprintf(stderr,
                            "FAIL: Mac HUD arrow %d lost its retail image or destination binding\n",
                            arrow);
                    dm2_v1_gdat_hud_m11_command_plan_free(&mac_hud);
                    M11_GameView_Shutdown(&view);
                    return 1;
                }
            }
            dm2_v1_gdat_hud_m11_command_plan_free(&mac_hud);
        }
        int graphicsset = dungeon
            ? dm2_v1_dungeon_get_map_graphics_style(dungeon, 0) : -1;
        memset(&scene_plan, 0, sizeof(scene_plan));
        memset(&wall_plan, 0, sizeof(wall_plan));
        memset(&expected_wall_plan, 0, sizeof(expected_wall_plan));
        if (!profile || graphicsset < 0 ||
            !dm2_v1_boot_gdat_scene_m11_command_plan(
                profile, (uint8_t)graphicsset, &scene_plan) ||
            !dm2_v1_boot_gdat_wall_m11_command_plan(
                profile, (uint8_t)graphicsset, &wall_plan) ||
            !dm2_v1_boot_gdat_wall_m11_command_plan_for_scene(
                profile, graphicsset, 0,
                (dungeon->initial_party_x + dungeon->initial_party_y +
                 dungeon->initial_party_dir + dungeon->map_offset_x[0] +
                 dungeon->map_offset_y[0] +
                 dungeon->map_graphics_flip_seed[0]) & 1,
                &expected_wall_plan)) {
            fprintf(stderr, "FAIL: Mac source-backed scene plans did not bind\n");
            dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
            dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
            dm2_v1_gdat_wall_m11_command_plan_free(&expected_wall_plan);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        if (wall_plan.command_count != DM2_V1_GDAT_WALL_M11_COMMAND_MAX) {
            fprintf(stderr,
                    "FAIL: Mac wall plan omitted source cells (%u/%d)\n",
                    wall_plan.command_count,
                    DM2_V1_GDAT_WALL_M11_COMMAND_MAX);
            dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
            dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
            dm2_v1_gdat_wall_m11_command_plan_free(&expected_wall_plan);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        {
            const DM2_V1_GdatWallM11Command *center_far_wall = NULL;
            for (int i = 0; i < expected_wall_plan.command_count; ++i) {
                const DM2_V1_GdatWallM11Command *command =
                    &expected_wall_plan.commands[i];
                if (command->skproject_cell == 6u) {
                    center_far_wall = command;
                    break;
                }
            }
            if (!center_far_wall || center_far_wall->field != 0x28u ||
                center_far_wall->rect_number != 0x2c4u ||
                center_far_wall->view_square != DM2_SQ_D2C ||
                !center_far_wall->decoded_hash ||
                !center_far_wall->geometry_hash ||
                !center_far_wall->destination_width ||
                !center_far_wall->destination_height ||
                center_far_wall->destination_x +
                    center_far_wall->destination_width > DM2_VP_WIDTH ||
                center_far_wall->destination_y +
                    center_far_wall->destination_height > DM2_VP_HEIGHT) {
                fprintf(stderr,
                        "FAIL: Mac map-0 wall at distance two lacks its exact "
                        "source image and in-viewport RAW4 placement\n");
                dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
                dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
                dm2_v1_gdat_wall_m11_command_plan_free(&expected_wall_plan);
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
        {
            static const uint8_t mirror_cells[16] = {
                0x00u, 0x02u, 0x01u, 0x03u, 0x05u, 0x04u, 0x06u, 0x08u,
                0x07u, 0x0au, 0x09u, 0x0bu, 0x0du, 0x0cu, 0x0fu, 0x0eu
            };
            DM2_V1_GdatWallM11CommandPlan flipped_plan;
            const DM2_V1_AssetLoader *loader =
                dm2_v1_boot_asset_loader(profile);
            memset(&flipped_plan, 0, sizeof(flipped_plan));
            if (!loader || !dm2_v1_boot_gdat_wall_m11_command_plan_for_scene(
                    profile, graphicsset, 0, 1, &flipped_plan) ||
                flipped_plan.graphics_flip_parity != 1u ||
                flipped_plan.command_count != wall_plan.command_count) {
                fprintf(stderr,
                        "FAIL: Mac source-backed parity-one wall plan did not bind\n");
                dm2_v1_gdat_wall_m11_command_plan_free(&flipped_plan);
                dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
                dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
                M11_GameView_Shutdown(&view);
                return 1;
            }
            for (int i = 0; i < flipped_plan.command_count; ++i) {
                const DM2_V1_GdatWallM11Command *command =
                    &flipped_plan.commands[i];
                int expected_field = 0x32;
                if (command->skproject_cell < 16u) {
                    int cell = command->skproject_cell;
                    int width = 0, height = 0;
                    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
                    uint8_t *mirrored = dm2_v1_asset_load_image_field(
                        loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
                        0xb0 + mirror_cells[cell], &width, &height, &format);
                    expected_field = mirrored && width > 0 && height > 0
                        ? 0xb0 + mirror_cells[cell] : 0x22 + cell;
                    dm2_v1_asset_free_pixels(mirrored);
                }
                if (command->field != expected_field) {
                    fprintf(stderr,
                            "FAIL: Mac wall parity chose field %02x for cell %02x; expected %02x\n",
                            command->field, command->skproject_cell,
                            expected_field);
                    dm2_v1_gdat_wall_m11_command_plan_free(&flipped_plan);
                    dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
                    dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
                    dm2_v1_gdat_wall_m11_command_plan_free(&expected_wall_plan);
                    M11_GameView_Shutdown(&view);
                    return 1;
                }
            }
            dm2_v1_gdat_wall_m11_command_plan_free(&flipped_plan);
        }
        /* The Macintosh retail set uses an 8-bit IMG9 floor and a 4-bit
         * IMG3 ceiling. Plausible dimensions alone can admit a wrong asset. */
        if (scene_plan.commands[0].format != DM2_IMG_FMT_IMG9 ||
            scene_plan.commands[1].format != DM2_IMG_FMT_IMG3) {
            fprintf(stderr,
                    "FAIL: Mac scene selected unexpected plane formats "
                    "(floor=%d ceiling=%d)\n",
                    scene_plan.commands[0].format,
                    scene_plan.commands[1].format);
            dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
            dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        if (scene_plan.rects[1].y != 0 ||
            scene_plan.rects[0].y + scene_plan.rects[0].height != 136 ||
            scene_plan.rects[0].width != 224u ||
            scene_plan.rects[1].width != 224u) {
            fprintf(stderr,
                    "FAIL: Mac RECT_700/701 anchors do not place the ceiling "
                    "at the top and floor at the bottom\n");
            dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
            dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        for (int i = 0; i < 2; ++i) {
            if (scene_plan.commands[i].format != DM2_IMG_FMT_IMG3 &&
                scene_plan.commands[i].format != DM2_IMG_FMT_U4 &&
                !mac_non_c4_palette_is_identity(
                    scene_plan.commands[i].palette16)) {
                fprintf(stderr,
                        "FAIL: Mac 8-bit scene plane used a C4 local palette\n");
                dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
                dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
        if (!dm2_v1_boot_gdat_scene_m11_apply_light_palette(
                profile, 0, 1u, 0x4d41434cu, &scene_plan)) {
            fprintf(stderr,
                    "FAIL: Mac source-backed light pass rejected the scene plan\n");
            dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
            dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        for (int i = 0; i < 2; ++i) {
            const DM2_V1_GdatSceneM11Command *command =
                &scene_plan.commands[i];
            int local_palette = command->format == DM2_IMG_FMT_IMG3 ||
                                command->format == DM2_IMG_FMT_U4;
            if (command->decoded_hash !=
                    dm2_v1_gdat_scene_m11_command_pixel_hash(command) ||
                (!local_palette &&
                 (!mac_non_c4_palette_is_identity(command->palette16) ||
                  command->palette_light_receipt_hash != 0u ||
                  command->palette_transform_hash != 0u)) ||
                (local_palette &&
                 (command->palette_light_receipt_hash == 0u ||
                  command->palette_transform_hash == 0u))) {
                fprintf(stderr,
                        "FAIL: Mac light pass changed global-index pixels via a "
                        "16-color local palette (field=%02x format=%d)\n",
                        command->field, command->format);
                dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
                dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
        {
            const DM2_V1_AssetLoader *loader =
                dm2_v1_boot_asset_loader(profile);
            for (int i = 0; loader && i < wall_plan.command_count; ++i) {
                const DM2_V1_GdatWallM11Command *command =
                    &wall_plan.commands[i];
                DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
                int width = 0;
                int height = 0;
                uint8_t *pixels = dm2_v1_asset_load_image_field(
                    loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
                    command->field, &width, &height, &format);
                int local_palette = format == DM2_IMG_FMT_IMG3 ||
                                    format == DM2_IMG_FMT_U4;
                dm2_v1_asset_free_pixels(pixels);
                if (!local_palette &&
                    !mac_non_c4_palette_is_identity(command->palette16)) {
                    fprintf(stderr,
                            "FAIL: Mac 8-bit wall used a C4 local palette "
                            "(field=%02x format=%d %dx%d)\n",
                            command->field, format, width, height);
                    dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
                    dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
                    M11_GameView_Shutdown(&view);
                    return 1;
                }
            }
        }
        dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
        dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
        dm2_v1_gdat_wall_m11_command_plan_free(&expected_wall_plan);
    }
    {
        DM2_V1_RuntimeFrameOwnershipReceipt frame_ownership;
        DM2_V1_ViewportM11FrameReceipt hud_receipt;
        size_t viewport_nonblack = 0u;
        size_t mac_hud_nonblack = 0u;
        memset(&frame_ownership, 0, sizeof(frame_ownership));
        memset(&hud_receipt, 0, sizeof(hud_receipt));
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        for (int y = 40; y < 176; ++y) {
            for (int x = 0; x < 224; ++x) {
                if (framebuffer[(size_t)y * 320u + (size_t)x] != 0u)
                    ++viewport_nonblack;
            }
        }
        if (mac_nonzero_in_rect(framebuffer, 56, 62, 112, 48) < 1500u) {
            fprintf(stderr,
                    "FAIL: Mac spawn's authentic center wall is absent from "
                    "the expected close-wall region\n");
            M11_GameView_Shutdown(&view);
            return 1;
        }
        for (int y = 0; y < 28; ++y)
            for (int x = 0; x < 320; ++x)
                if (framebuffer[(size_t)y * 320u + (size_t)x] != 0u)
                    ++mac_hud_nonblack;
        (void)dm2_v1_runtime_last_m11_frame_receipt(&hud_receipt);
        if (viewport_nonblack < 22000u ||
            mac_hud_nonblack < 100u ||
            !hud_receipt.valid || !hud_receipt.hud_material_plan_required ||
            !hud_receipt.hud_material_plan_consumed ||
            hud_receipt.hud_material_plan_command_count != 8 ||
            !dm2_v1_runtime_last_frame_ownership(&frame_ownership) ||
            !frame_ownership.full_gdat_frame_valid ||
            !frame_ownership.floor_ceiling_materials_complete ||
            (frame_ownership.wall_source_cell_required_mask & (1u << 6)) == 0u ||
            (frame_ownership.wall_source_cell_required_mask & (1u << 3)) != 0u ||
            frame_ownership.wall_source_cell_required_mask !=
                frame_ownership.wall_source_cell_consumed_mask ||
            frame_ownership.total_runtime_fallback_draws != 0) {
            fprintf(stderr,
                    "FAIL: Mac New Game did not render its source-backed RECT_7/HUD "
                    "(view=%zu hud=%zu hudReceipt=%d/%d/%d frame=%d planes=%d fallbacks=%d)\n",
                    viewport_nonblack, mac_hud_nonblack,
                    hud_receipt.hud_material_plan_required,
                    hud_receipt.hud_material_plan_consumed,
                    hud_receipt.hud_material_plan_command_count,
                    frame_ownership.full_gdat_frame_valid,
                    frame_ownership.floor_ceiling_materials_complete,
                    frame_ownership.total_runtime_fallback_draws);
            M11_GameView_Shutdown(&view);
            return 1;
        }
    }
    {
        uint32_t direction_hashes[4] = { 0u, 0u, 0u, 0u };
        for (int direction = 0; direction < 4; ++direction) {
            DM2_V1_RuntimeFrameOwnershipReceipt ownership;
            DM2_V1_ViewportM11FrameReceipt m11_receipt;
            size_t nonblack = 0u;
            memset(&ownership, 0, sizeof(ownership));
            memset(&m11_receipt, 0, sizeof(m11_receipt));
            dm2_v1_runtime_set_position(0, 1, 8, direction);
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            for (int y = 40; y < 176; ++y) {
                for (int x = 0; x < 224; ++x) {
                    if (framebuffer[(size_t)y * 320u + (size_t)x] != 0u)
                        ++nonblack;
                }
            }
            direction_hashes[direction] = mac_dungeon_view_hash(framebuffer);
            if (!dm2_v1_runtime_last_frame_ownership(&ownership) ||
                !dm2_v1_runtime_last_m11_frame_receipt(&m11_receipt) ||
                ownership.is_outdoor != 0 ||
                !ownership.full_gdat_frame_valid ||
                !ownership.floor_ceiling_materials_complete ||
                ownership.creature_gdat_blits != 0 ||
                ownership.total_runtime_fallback_draws != 0 ||
                !m11_receipt.valid || !m11_receipt.m11_consume_frame ||
                nonblack < 22000u ||
                (direction == 1 && ownership.wall_gdat_blits == 0)) {
                fprintf(stderr,
                        "FAIL: Mac real-media viewport rejected direction %d "
                        "(frame=%d planes=%d m11=%d consume=%d pixels=%zu fallbacks=%d)\n",
                        direction, ownership.full_gdat_frame_valid,
                        ownership.floor_ceiling_materials_complete,
                        m11_receipt.valid, m11_receipt.m11_consume_frame,
                        nonblack, ownership.total_runtime_fallback_draws);
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
        for (int direction = 0; direction < 4; ++direction) {
            for (int previous = 0; previous < direction; ++previous) {
                if (direction_hashes[direction] == direction_hashes[previous]) {
                    fprintf(stderr,
                            "FAIL: Mac dungeon view did not change between directions %d and %d\n",
                            previous, direction);
                    M11_GameView_Shutdown(&view);
                    return 1;
                }
            }
        }
        dm2_v1_runtime_set_position(0, 1, 8, 0);
    }
    /* Retail Mac movement arrows are outside the dungeon C080 viewport.
     * Clicking RECT_42 (top-center/forward) must still enter the ordinary
     * source movement pipeline. */
    if (M11_GameView_HandlePointerButton(
            &view, 274, 140, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW) {
        fprintf(stderr,
                "FAIL: Mac retail forward-arrow click was not routed to movement\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    (void)M11_GameView_AdvanceIdleTick(&view);
    if (dm2_v1_runtime_get_party_x() != 1 ||
        dm2_v1_runtime_get_party_y() != 7 ||
        dm2_v1_runtime_get_party_dir() != 0) {
        fprintf(stderr,
                "FAIL: Mac forward-arrow click did not move north to (1,7) (party=%d,%d,%d)\n",
                dm2_v1_runtime_get_party_x(), dm2_v1_runtime_get_party_y(),
                dm2_v1_runtime_get_party_dir());
        M11_GameView_Shutdown(&view);
        return 1;
    }
    if (view.dm2State.music_events_due == 0u) {
        fprintf(stderr,
                "FAIL: Mac New Game did not advance the selected MIDI cue on its first gameplay tick\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    {
        const DM2_V1_DungeonData *live_dungeon =
            (const DM2_V1_DungeonData *)
                ((DM2_V1_BootProfile *)view.dm2BootProfile)->dungeon_data;
        int nearby_x;
        int nearby_y;
        int nearby = mac_live_map_has_nearby_creature(
            live_dungeon, 0, dm2_v1_runtime_get_party_x(),
            dm2_v1_runtime_get_party_y(), &nearby_x, &nearby_y);
        printf("Mac first forward step from New Game: party=(%d,%d) nearby-live-DB4=%d creature=(%d,%d)\n",
               dm2_v1_runtime_get_party_x(), dm2_v1_runtime_get_party_y(),
               nearby, nearby_x, nearby_y);
        if (nearby != 0) {
            fprintf(stderr,
                    "FAIL: Mac first forward step left a live DB4 creature "
                    "within one tile (nearby=%d creature=%d,%d party=%d,%d)\n",
                    nearby, nearby_x, nearby_y,
                    dm2_v1_runtime_get_party_x(),
                    dm2_v1_runtime_get_party_y());
            M11_GameView_Shutdown(&view);
            return 1;
        }
    }
    /* Continue the exact route exercised by the post-menu M12 smoke test:
     * keep the authentic first forward step, turn east, then walk twice.
     * This checks the reported endpoint without teleporting the party. */
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_TURN_RIGHT) !=
            M11_GAME_INPUT_REDRAW) {
        fprintf(stderr, "FAIL: Mac M11 active session rejected turn input\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    (void)M11_GameView_AdvanceIdleTick(&view);
    {
        DM2_V1_RuntimeMusicMapReceipt music;
        memset(&music, 0, sizeof(music));
        if (!dm2_v1_runtime_last_music_map_receipt(&music) ||
            music.blocked_no_session || music.selected_track < 0 ||
            !music.source_stream_resolved ||
            (music.queue_result != DM2_V1_MUSIC_QUEUE_READY &&
             music.queue_result !=
                 DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE)) {
            fprintf(stderr,
                    "FAIL: Mac post-move source music route unavailable "
                    "(valid=%d track=%d blocked=%d resolved=%d result=%d due=%u)\n",
                    music.valid, music.selected_track, music.blocked_no_session,
                    music.source_stream_resolved, music.queue_result,
                    view.dm2State.music_events_due);
            M11_GameView_Shutdown(&view);
            return 1;
        }
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) !=
            M11_GAME_INPUT_REDRAW) {
        fprintf(stderr, "FAIL: Mac M11 active session rejected move input\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    (void)M11_GameView_AdvanceIdleTick(&view);
    if (dm2_v1_runtime_get_party_x() != 2 ||
        dm2_v1_runtime_get_party_y() != 7 ||
        dm2_v1_runtime_get_party_dir() != 1) {
        fprintf(stderr,
                "FAIL: Mac movement did not apply to the authentic map "
                "(party=%d,%d,%d)\n",
                dm2_v1_runtime_get_party_x(), dm2_v1_runtime_get_party_y(),
                dm2_v1_runtime_get_party_dir());
        M11_GameView_Shutdown(&view);
        return 1;
    }
    /* Exercise the retail Mac forward key after a genuine New Game and let
     * ordinary source ticks run. The earlier spawn-only check could miss a
     * bad move destination or a nearby DB4 that appears after timer updates. */
    {
        DM2_V1_MacInputReceipt key_receipt;
        DM2_V1_BootRuntimeReceipt after_ticks;
        if (!dm2_v1_mac_input_resolve('k', 0,
                                      DM2_V1_MAC_INPUT_GAMEPLAY,
                                      &key_receipt) ||
            key_receipt.action != DM2_V1_MAC_ACTION_MOVE_FORWARD ||
            M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) !=
                M11_GAME_INPUT_REDRAW) {
            fprintf(stderr,
                    "FAIL: retail Mac forward key did not reach the active movement route\n");
            M11_GameView_Shutdown(&view);
            return 1;
        }
        (void)M11_GameView_AdvanceIdleTick(&view);
        if (dm2_v1_runtime_get_party_x() != 3 ||
            dm2_v1_runtime_get_party_y() != 7 ||
            dm2_v1_runtime_get_party_dir() != 1) {
            fprintf(stderr,
                    "FAIL: second Mac forward move did not advance to (3,7) facing east (party=%d,%d,%d)\n",
                    dm2_v1_runtime_get_party_x(),
                    dm2_v1_runtime_get_party_y(),
                    dm2_v1_runtime_get_party_dir());
            M11_GameView_Shutdown(&view);
            return 1;
        }
        for (int tick = 0; tick < 16; ++tick)
            (void)M11_GameView_AdvanceIdleTick(&view);
        memset(&after_ticks, 0, sizeof(after_ticks));
        if (!dm2_v1_boot_runtime_capture(
                (DM2_V1_BootProfile *)view.dm2BootProfile, &after_ticks) ||
            !after_ticks.runtime_ready || after_ticks.current_level != 0 ||
            after_ticks.party_x != 3 || after_ticks.party_y != 7 ||
            after_ticks.party_dir != 1) {
            fprintf(stderr,
                    "FAIL: Mac source ticks changed movement state unexpectedly (ready=%d map=%d party=%d,%d,%d)\n",
                    after_ticks.runtime_ready, after_ticks.current_level,
                    after_ticks.party_x, after_ticks.party_y,
                    after_ticks.party_dir);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        {
            const DM2_V1_DungeonData *live_dungeon =
                (const DM2_V1_DungeonData *)
                    ((DM2_V1_BootProfile *)view.dm2BootProfile)->dungeon_data;
            int nearby = mac_live_map_has_nearby_creature(
                live_dungeon, after_ticks.current_level, after_ticks.party_x,
                after_ticks.party_y, NULL, NULL);
            if (nearby != 0) {
                fprintf(stderr,
                        "FAIL: live Mac DB4 creature is within one tile after movement and 16 source ticks (nearby=%d party=%d,%d)\n",
                        nearby, after_ticks.party_x, after_ticks.party_y);
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
    }
    {
        if (!M11_GameView_OpenSpellPanel(&view)) {
            fprintf(stderr, "FAIL: Mac M11 did not open the DM2 spell panel\n");
            M11_GameView_Shutdown(&view);
            return 1;
        }
        view.spellBuffer.runes[0] = DM2_RUNE_YA;
        view.spellBuffer.runes[1] = DM2_RUNE_FUL;
        view.spellBuffer.runes[2] = DM2_RUNE_IR;
        view.spellBuffer.runeCount = 3;
        if (!M11_GameView_CastSpell(&view) || view.spellPanelOpen) {
            fprintf(stderr, "FAIL: Mac M11 DM2 spell panel did not enqueue fireball (active %d dead %d hand %d panel %d title %s detail %s)\n",
                    view.active, view.partyDead, dm2_v1_runtime_get_active_hand(),
                    view.spellPanelOpen,
                    view.inspectTitle, view.inspectDetail);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        {
            int step_seen = 0;
            for (int i = 0; i < 4; ++i) {
                DM2_V1_ProceedTimersReceipt timers;
                dm2_v1_runtime_tick();
                memset(&timers, 0, sizeof(timers));
                if (dm2_v1_runtime_last_proceed_timers_receipt(&timers) &&
                    timers.type_tally[0x1e] > 0) {
                    step_seen = 1;
                    break;
                }
            }
            if (!step_seen) {
                fprintf(stderr, "FAIL: Mac M11 Fireball never reached DM2_STEP_MISSILE\n");
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_INVENTORY_TOGGLE) !=
            M11_GAME_INPUT_REDRAW || !view.inventoryPanelActive) {
        fprintf(stderr, "FAIL: Mac M11 did not open the native CHARSHEET inventory\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    {
        size_t nonzero = 0u;
        for (size_t i = 0u; i < sizeof(framebuffer); ++i)
            if (framebuffer[i] != 0u) ++nonzero;
        if (nonzero == 0u) {
            fprintf(stderr, "FAIL: Mac M11 inventory did not draw CHARSHEET pixels\n");
            M11_GameView_Shutdown(&view);
            return 1;
        }
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK) !=
            M11_GAME_INPUT_REDRAW || view.inventoryPanelActive) {
        fprintf(stderr, "FAIL: Mac M11 did not close the native inventory\n");
        M11_GameView_Shutdown(&view);
            return 1;
    }
    if (!exercise_authentic_mac_stairs(
            (DM2_V1_BootProfile *)view.dm2BootProfile,
            (const DM2_V1_DungeonData *)
                ((DM2_V1_BootProfile *)view.dm2BootProfile)->dungeon_data)) {
        fprintf(stderr, "FAIL: Mac M11 did not reach an authentic stairs transition\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    if (!exercise_authentic_active_mac_creature(
            (DM2_V1_BootProfile *)view.dm2BootProfile,
            (const DM2_V1_DungeonData *)
                ((DM2_V1_BootProfile *)view.dm2BootProfile)->dungeon_data,
            framebuffer, &view)) {
        fprintf(stderr, "FAIL: Mac M11 did not reach an active DB4/F9 creature\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    puts("PASS: authentic Mac M11 NEW GAME reaches active runtime");
    M11_GameView_Shutdown(&view);
    return 0;
}
