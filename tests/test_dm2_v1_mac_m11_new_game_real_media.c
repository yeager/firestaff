/* Opt-in real-media Mac M11 startup -> NEW GAME -> active-session regression. */

#include "m11_game_view.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_spell.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int find_mac_creature_material(
    const DM2_V1_DungeonData *dungeon, int map, int16_t creature_record,
    DM2_V1_G1CreatureMapChipMaterial *out);

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
        return 0;
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
    /* The retail source keeps the title/menu boundary alive while GAME_LOAD
     * prepares the mirror view. A viewport click is the source confirmation
     * that commits the selected champion and only then closes the menu. */
    if (M11_GameView_HandlePointerButton(
            &view, 100, 60, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        view.dm2State.startup_menu_active || !view.dm2State.level_loaded) {
        fprintf(stderr, "FAIL: Mac M11 source mirror click did not commit runtime\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    memset(&runtime, 0, sizeof(runtime));
    if (!dm2_v1_boot_runtime_capture(
            (DM2_V1_BootProfile *)view.dm2BootProfile, &runtime) ||
        !runtime.runtime_ready) {
        fprintf(stderr, "FAIL: Mac M11 active session did not capture runtime\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_TURN_RIGHT) !=
            M11_GAME_INPUT_REDRAW ||
        M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) !=
            M11_GAME_INPUT_REDRAW) {
        fprintf(stderr, "FAIL: Mac M11 active session rejected movement\n");
        M11_GameView_Shutdown(&view);
        return 1;
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
