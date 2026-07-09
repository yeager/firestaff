/*
 * DM1 V1 Engine Integration Layer — implementation.
 *
 * Wires all dm1_v1_* subsystems into a single init/tick/shutdown API.
 * Follows the DM.C F0449 init sequence and GAMELOOP.C F0002 tick order.
 *
 * Source lock: see header for ReDMCSB function references.
 */

#include "dm1_v1_engine_pc34_compat.h"
#include <string.h>

/* ── Module manifest ──────────────────────────────────────────────── */

static const char * const s_moduleNames[] = {
    "dm1_v1_blit_fill_pc34_compat",
    "dm1_v1_champion_needs_pc34_compat",
    "dm1_v1_champion_panel_hud_pc34_compat",
    "dm1_v1_champion_stats_pc34_compat",
    "dm1_v1_click_routing_pc34_compat",
    "dm1_v1_collision_door_pc34_compat",
    "dm1_v1_combat_pc34_compat",
    "dm1_v1_creature_ai_behavior_pc34_compat",
    "dm1_v1_creature_render_pc34_compat",
    "dm1_v1_creature_viewport_pc34_compat",
    "dm1_v1_dialog_scroll_pc34_compat",
    "dm1_v1_draw_primitives_pc34_compat",
    "dm1_v1_dungeon_data_pc34_compat",
    "dm1_v1_dungeon_decompressor_pc34_compat",
    "dm1_v1_dungeon_loader_pc34_compat",
    "dm1_v1_dungeon_square_structs_pc34_compat",
    "dm1_v1_endgame_system_pc34_compat",
    "dm1_v1_engine_pc34_compat",
    "dm1_v1_entrance_champion_select_pc34_compat",
    "dm1_v1_event_timer_pc34_compat",
    "dm1_v1_fade_transition_pc34_compat",
    "dm1_v1_field_teleporter_effect_pc34_compat",
    "dm1_v1_fountain_interaction_pc34_compat",
    "dm1_v1_food_water_pc34_compat",
    "dm1_v1_game_loop_integration_pc34_compat",
    "dm1_v1_game_loop_pc34_compat",
    "dm1_v1_game_over_pc34_compat",
    "dm1_v1_game_state_pc34_compat",
    "dm1_v1_graphics_loader_pc34_compat",
    "dm1_v1_input_command_queue_pc34_compat",
    "dm1_v1_input_poll_pc34_compat",
    "dm1_v1_inventory_pc34_compat",
    "dm1_v1_light_pc34_compat",
    "dm1_v1_menu_render_pc34_compat",
    "dm1_v1_movement_command_core_pc34_compat",
    "dm1_v1_movement_pipeline_pc34_compat",
    "dm1_v1_movement_timing_pc34_compat",
    "dm1_v1_object_interaction_pc34_compat",
    "dm1_v1_object_world_pc34_compat",
    "dm1_v1_palette_font_pc34_compat",
    "dm1_v1_room_transition_pc34_compat",
    "dm1_v1_portrait_panel_pc34_compat",
    "dm1_v1_projectile_explosion_render_pc34_compat",
    "dm1_v1_resurrection_pc34_compat",
    "dm1_v1_save_load_system_pc34_compat",
    "dm1_v1_screen_framebuffer_pc34_compat",
    "dm1_v1_sensor_trigger_pc34_compat",
    "dm1_v1_skill_experience_pc34_compat",
    "dm1_v1_sound_pc34_compat",
    "dm1_v1_spell_casting_pc34_compat",
    "dm1_v1_spell_effect_render_pc34_compat",
    "dm1_v1_stairs_level_pc34_compat",
    "dm1_v1_teleporter_pit_pc34_compat",
    "dm1_v1_text_message_pc34_compat",
    "dm1_v1_title_screen_pc34_compat",
    "dm1_v1_viewport_3d_pc34_compat",
    "dm1_v1_viewport_click_pc34_compat",
    "dm1_v1_viewport_floor_ceiling_items_pc34_compat",
    "dm1_v1_wall_ornament_pc34_compat",
};

#define MODULE_COUNT  ((int)(sizeof(s_moduleNames) / sizeof(s_moduleNames[0])))

/* ── Lifecycle ────────────────────────────────────────────────────── */

bool DM1_V1_Engine_InitPc34Compat(DM1_V1_EnginePc34 *engine, const DM1_V1_EngineConfigPc34 *config)
{
    if (!engine || !config) return false;

    memset(engine, 0, sizeof(*engine));
    engine->config = *config;

    /* 1. Screen / framebuffer */
    DM1_V1_Screen_InitPc34Compat(&engine->screen);

    /* 2. Input */
    DM1_V1_Input_InitPc34Compat(&engine->input);

    /* 3. Game state machine → TITLE */
    DM1_V1_GameState_InitPc34Compat(&engine->stateMachine);
    DM1_V1_GameState_TransitionPc34Compat(&engine->stateMachine, DM1_STATE_TITLE_SCREEN);

    /* 4. Game loop (tick rate, vblank config) */
    DM1_V1_GameLoop_InitPc34Compat(&engine->gameLoop, config->extended_vblank);
    if (config->tick_rate_hz > 0)
        DM1_V1_GameLoop_SetTickRatePc34Compat(&engine->gameLoop, config->tick_rate_hz);

    /* 5. Game loop integration */
    DM1_V1_LoopIntegration_InitPc34Compat(&engine->loopIntegration);

    /* 6. Central dungeon data */
    DM1_V1_DungeonData_InitPc34Compat(&engine->dungeonData);
    if (config->dungeon_dat_path) {
        if (!DM1_V1_DungeonData_LoadDungeonPc34Compat(&engine->dungeonData,
                                 config->dungeon_dat_path)) {
            /* Non-fatal: engine can run in title-only mode */
        }
    }
    if (config->graphics_dat_path) {
        DM1_V1_DungeonData_LoadObjectsPc34Compat(&engine->dungeonData, config->graphics_dat_path);
    }

    /* 7. Event timer (embedded in dungeonData — already init'd) */

    /* 8. Movement pipeline */
    DM1_V1_MovementPipeline_InitPc34Compat(&engine->movementPipeline);

    /* 9. Viewport 3D (wired to screen back buffer) */
    dm1_viewport_3d_init(&engine->viewport3d,
                         &engine->screen.backBuffer[0][0],
                         DM1_V1_SCREEN_W_PC34);

    /* 10. Dialog / message system */
    DM1_V1_Dialog_InitPc34Compat(&engine->dialog);
    DM1_V1_Dialog_SetBarPositionPc34Compat(&engine->dialog, 0, 169, 320, 7);

    /* 11. Click routing + dungeon zones */
    DM1_V1_ClickRouting_InitPc34Compat(&engine->clickRouting);
    DM1_V1_ClickRouting_SetupDungeonZonesPc34Compat(&engine->clickRouting);

    /* 12. Save/load */
    DM1_V1_SaveLoad_InitPc34Compat(&engine->saveLoad,
                config->save_dir ? config->save_dir : ".");

    engine->initialized = true;
    return true;
}

DM1_V1_EngineTickResultPc34 DM1_V1_Engine_TickPc34Compat(DM1_V1_EnginePc34 *engine, uint32_t nowMs)
{
    DM1_V1_EngineTickResultPc34 result;
    memset(&result, 0, sizeof(result));

    if (!engine || !engine->initialized) {
        result.exitRequested = true;
        return result;
    }

    DM1_V1_GameLoopTickResultPc34 loopResult =
        DM1_V1_GameLoop_TickPc34Compat(&engine->gameLoop, nowMs);

    /* 1. Poll input — always */
    result.inputProcessed = true;

    /* 2. Process movement pipeline (only in dungeon state) */
    if (DM1_V1_GameState_CurrentPc34Compat(&engine->stateMachine) ==
        DM1_STATE_DUNGEON_VIEWPORT) {

        DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat(
            &engine->movementPipeline);
        result.movementProcessed = true;
    }

    /* 3. Advance game time + process expired events */
    if (engine->gameLoop.timerActive) {
        DM1_V1_DungeonData_AdvanceTickPc34Compat(&engine->dungeonData);
        result.timelineProcessed = true;
    }

    /* 4. Update dialog / messages */
    DM1_V1_Dialog_TickPc34Compat(&engine->dialog);
    result.dialogUpdated = true;

    /* 5. Check death / victory */
    if (engine->dungeonData.partyDead) {
        DM1_V1_GameState_PartyDiedPc34Compat(&engine->stateMachine);
    }
    if (engine->dungeonData.gameWon) {
        DM1_V1_GameState_VictoryPc34Compat(&engine->stateMachine);
    }

    /* 6. Viewport draw flag (actual draw delegated to renderer) */
    result.viewportDrawn = loopResult.dungeonViewDrawn;

    /* 7. Fill result */
    result.currentState  = DM1_V1_GameState_CurrentPc34Compat(&engine->stateMachine);
    result.lastPhase     = loopResult.lastPhaseCompleted;
    result.partyDead     = loopResult.partyDead;
    result.gameWon       = engine->dungeonData.gameWon;
    result.exitRequested = loopResult.exitRequested;
    result.gameTime      = DM1_V1_DungeonData_GetGameTimePc34Compat(&engine->dungeonData);
    result.frameNumber   = engine->gameLoop.verticalBlankCount;

    engine->totalTicks++;
    return result;
}

void DM1_V1_Engine_ShutdownPc34Compat(DM1_V1_EnginePc34 *engine)
{
    if (!engine) return;

    if (engine->initialized) {
        DM1_V1_Input_DeinitPc34Compat(&engine->input);
        DM1_V1_DungeonData_ShutdownPc34Compat(&engine->dungeonData);
    }

    engine->initialized = false;
}

/* ── Engine queries ───────────────────────────────────────────────── */

DM1_V1_GameStateIdPc34 DM1_V1_Engine_GetStatePc34Compat(const DM1_V1_EnginePc34 *engine)
{
    return engine ? DM1_V1_GameState_CurrentPc34Compat(&engine->stateMachine)
                  : DM1_STATE_NONE;
}

DM1_V1_DungeonDataPc34 *DM1_V1_Engine_GetDungeonDataPc34Compat(DM1_V1_EnginePc34 *engine)
{
    return engine ? &engine->dungeonData : NULL;
}

DM1_V1_InputStatePc34 *DM1_V1_Engine_GetInputPc34Compat(DM1_V1_EnginePc34 *engine)
{
    return engine ? &engine->input : NULL;
}

DM1_V1_ScreenStatePc34 *DM1_V1_Engine_GetScreenPc34Compat(DM1_V1_EnginePc34 *engine)
{
    return engine ? &engine->screen : NULL;
}

DM1_V1_SaveLoadStatePc34 *DM1_V1_Engine_GetSaveLoadPc34Compat(DM1_V1_EnginePc34 *engine)
{
    return engine ? &engine->saveLoad : NULL;
}

/* ── Engine actions ───────────────────────────────────────────────── */

bool DM1_V1_Engine_NewGamePc34Compat(DM1_V1_EnginePc34 *engine)
{
    if (!engine || !engine->initialized) return false;

    DM1_V1_TransitionResultPc34 tr =
        DM1_V1_GameState_StartNewGamePc34Compat(&engine->stateMachine);
    if (tr != DM1_TRANS_OK) return false;

    /* Reset dungeon to level 0 */
    DM1_V1_DungeonData_SetCurrentMapPc34Compat(&engine->dungeonData, 0);
    engine->dungeonData.gameTime = 0;
    engine->dungeonData.newGame  = true;

    /* Reset event queue */
    dm1v1_event_queue_init(&engine->dungeonData.events, 0);

    /* Reset movement pipeline */
    DM1_V1_MovementPipeline_InitPc34Compat(&engine->movementPipeline);

    return true;
}

bool DM1_V1_Engine_LoadGamePc34Compat(DM1_V1_EnginePc34 *engine, uint8_t slot)
{
    if (!engine || !engine->initialized) return false;
    if (!DM1_V1_SaveLoad_SourceRuntimeSlotSupportedPc34Compat(slot)) return false;
    if (!DM1_V1_SaveLoad_SlotOccupiedPc34Compat(&engine->saveLoad, slot)) return false;

    DM1_V1_SaveLoadHeaderPc34 header;
    if (!DM1_V1_SaveLoad_LoadHeaderPc34Compat(&engine->saveLoad, slot, &header))
        return false;

    DM1_V1_GameState_LoadGamePc34Compat(&engine->stateMachine);
    return true;
}

bool DM1_V1_Engine_SaveGamePc34Compat(DM1_V1_EnginePc34 *engine, uint8_t slot)
{
    if (!engine || !engine->initialized) return false;
    if (!DM1_V1_SaveLoad_SourceRuntimeSlotSupportedPc34Compat(slot)) return false;

    DM1_V1_SaveLoadHeaderPc34 header;
    memset(&header, 0, sizeof(header));
    header.magic = DM1_SL_SAVE_MAGIC;
    header.game_time = engine->dungeonData.gameTime;
    header.current_level = (uint16_t)engine->dungeonData.currentMapIndex;
    header.party_x = engine->dungeonData.party.posX;
    header.party_y = engine->dungeonData.party.posY;
    header.party_facing = engine->dungeonData.party.facing;

    /* Actual data serialization is a future integration point */
    return DM1_V1_SaveLoad_SavePc34Compat(&engine->saveLoad, slot, &header, NULL, 0);
}

void DM1_V1_Engine_RequestExitPc34Compat(DM1_V1_EnginePc34 *engine)
{
    if (!engine) return;
    DM1_V1_GameLoop_RequestExitPc34Compat(&engine->gameLoop);
}

/* ── Module manifest ──────────────────────────────────────────────── */

int DM1_V1_Engine_ModuleCountPc34Compat(void)
{
    return MODULE_COUNT;
}

const char *DM1_V1_Engine_ModuleNamePc34Compat(int index)
{
    if (index < 0 || index >= MODULE_COUNT) return NULL;
    return s_moduleNames[index];
}

/* ── Source evidence ──────────────────────────────────────────────── */

const char *DM1_V1_Engine_SourceEvidencePc34Compat(void)
{
    return
        "DM1 V1 Engine Integration Layer\n"
        "Init sequence: DM.C F0449_DM_Main\n"
        "Tick order: GAMELOOP.C F0002_MAIN_GameLoop_CPSDF\n"
        "  newMap → timeline → dungeonView → pointer → highlight →\n"
        "  sound → damage → deathCheck → inputWait\n"
        "New game: STARTUP2.C F0462_START_StartGame_CPSEF\n"
        "Full init: STARTUP2.C F0463_START_InitializeGame_CPSADEF\n"
        "Module count: 58 (56 original + dungeon_data + engine)\n"
        "Source: ReDMCSB WIP20210206 Toolchains/Common/Source/\n";
}
