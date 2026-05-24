# DM1 V1 Startup Sequence — Source Lock

## Entry Point: firestaff_main_m11.c

main() in src/shared/firestaff_main_m11.c:
1. Parse CLI arguments: --duration, --width, --height, --script, --data-dir
2. Populate M11_PhaseA_Options struct
3. Call M11_PhaseA_Run(&opts)

## Phase A Run Loop (main_loop_m11.c M11_PhaseA_Run)

1. Render Init: M11_Render_Init(windowWidth, windowHeight, scaleMode)
   - Creates SDL window, renderer, internal framebuffer textures

2. Launcher Framebuffer Allocation
   - launcherFramebuffer = calloc(480, 270)  // indexed 8-bit
   - modernRgba = calloc(960, 540 * 4)        // RGBA for modern menu

3. Startup Menu Init: M12_StartupMenu_InitWithDataDir(&menuState, dataDir)
   - Calls M12_AssetStatus_Scan() to locate game data directories
   - Menu state carries asset status forward for TITLE path resolution

4. PO Translation Loading
   - Based on M12_Config_GetAutoLanguageIndex(), loads po/startup-menu.<lang>.po

5. Accessibility Enable: fs_ax_set_enabled(1)  (always on for now)

6. Game View Init: M11_GameView_Init(&gameView)
   - Initializes DM1 V1 game view subsystem (viewport, input, state machine)

7. Event Loop (Launcher Phase)
   SDL_PollEvent -> M12_MenuInput mapping -> menuState transition -> draw -> present
   On game launch (menu input -> ENTER DUNGEON):
     Transitions from launcher to game view
     Calls m11_engine_init() with config

8. Game View Main Loop
   When game launched: M11_GameView_Tick()
   dm1v1_game_loop_tick() follows DM.C F0002 tick order:
     Input poll -> Command queue -> Sensor/trigger -> Movement -> Combat -> AI
     -> Dungeon events -> Viewport render
   Tick rate: 200ms interval (DM1 V1 authentic PAL: 10 VBlanks x 20ms = 200ms,
   per VBLANK.C:F0577 and GAMELOOP.C:F0002)

9. TITLE Intro Sequence (V1 original mode)
   When M11_GameView_IsV1OriginalMode() and TITLE file found:
   - m11_find_title_dat_for_intro() locates the TITLE file
   - 53-frame TITLE bank decoded from packed 4bpp format
   - Each frame presented with M11_Render_PresentIndexedWithSpecialPalette()
     using VGA_PALETTE_PC34_SPECIAL_TITLE palette
   - Frame timing follows ReDMCSB TITLE.C F0449 VBlank gates

10. Entrance Sequence
    After TITLE intro: draws 3D entrance scene
    - Wall/floor rendering from loaded dungeon data
    - Champion selection UI (Hall of Champions)
    - Mouse/click routing for entrance interactions

11. Shutdown
    M11_GameView_Shutdown() -> M11_Render_Shutdown() -> free framebuffers -> return 0

## M11_Engine Init Sequence (dm1_v1_engine_pc34_compat.c m11_engine_init)

Follows DM.C F0449 init order and STARTUP2.C F0463:

| Step | Subsystem | Function |
|------|-----------|----------|
| 1 | Screen/framebuffer | m11_screen_init() |
| 2 | Input | m11_input_init() |
| 3 | Game state machine -> TITLE | m11_game_state_init() -> transition to DM1_STATE_TITLE_SCREEN |
| 4 | Game loop (tick rate, vblank) | m11_game_loop_init() |
| 5 | Loop integration | m11_gl_init() |
| 6 | Dungeon data | m11_dd_init() |
| 7 | Event timer | (embedded in dungeonData, already initd) |
| 8 | Viewport 3D | dm1_viewport_3d_init() |
| 9 | Dialog | m11_dg_init() |
| 10 | Click routing | m11_ck_init() |
| 11 | Save/load | m11_sl_init() |

## Boot Sequence Compatibility (boot_sequence_runtime_pc34_compat.h)

F9006_RUNTIME_RunBootSequence_Compat():
- Input: graphicsDatPath, outputPrefix, dialogGraphicIndex, viewportGraphicIndex,
  firstFrameNumber, frameCount
- Output: BootSequenceRuntimeResult_Compat with phase, step counts, frame counts

Phases: NOT_STARTED -> RUNTIME_BOOTSTRAP -> RUNTIME_STEADY -> BOOT_SEQUENCE_COMPLETE

## Startup Data Dir Guarantee

fs_startup_ensure_data_dirs() called during startup check:
- Ensures data/dm1/, data/csb/, data/dm2/, data/dm1-multilingual/, data/nexus/ exist
- Writes data/README.txt with placement instructions

## ReDMCSB Reference

| ReDMCSB Function | Firestaff Equivalent |
|---|---|
| STARTUP2.C F0463_START_InitializeGame_CPSADEF | m11_engine_init() |
| DM.C F0449_DM_Main | M11_PhaseA_Run() |
| GAMELOOP.C F0002_MAIN_GameLoop_CPSDF | dm1v1_game_loop_tick() |
| TITLE.C F0449 (intro sequence) | m11_draw_title_intro_frames() |
| VBLANK.C F0577 (200ms tick) | gameTickInterval = 200 |
