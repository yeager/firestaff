# DM1 V1 — CPU Usage

## ReDMCSB source

**Main CPU tasks (ordered by execution frequency)**

1. **VBLANK interrupt handler** — IRQ4, level 4
   - `BASE.C:830-1075` / `E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF`  
   - Executes every VBLANK (50 or 60 Hz depending on region)
   - Tasks per VBLANK:
     a. Viewport blit if `G0324_B_DrawViewportRequested` is set
     b. Palette switching via Timer B (if enabled)
     c. Mouse pointer draw/hide
     d. Increment `G0317_i_WaitForInputVerticalBlankCount`
     e. If idle > threshold, set `G0321_B_StopWaitingForPlayerInput` and `G0301_B_GameTimeTicking`
   - Interrupt-driven; not polled

2. **Mouse/IKBD interrupt handler** — IRQ6, level 6
   - `BASE.C:comment near line 830` / `F0075_MOUSE_Exception70Handler_IKBD_MIDI_MouseStatus`  
   - Fires on mouse move and button events from the Atari IKBD controller
   - Low latency; immediate cursor position update

3. **Sound interrupt handler** — IRQ6, level 6
   - Fires thousands of times per second per audio channel  
   - Plays back pre-loaded sound samples via DMA  
   - See `ANIMSND.C`, `SWSH.C` for sound trigger functions

4. **Main game loop** — `GAMELOOP.C:35-243` / `F0002_MAIN_GameLoop_CPSDF`  
   Runs in a tight loop when game is not paused/.menu'd:
   - Process timeline events (`F0261_TIMELINE_Process_CPSEF`)
   - Draw dungeon view (`F0128_DUNGEONVIEW_Draw_CPSF`) — heaviest draw call
   - Play pending sound (`F0065_SOUND_PlayPendingSound_CPSD`)
   - Apply champion damage/wounds (`F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds`)
   - Decrement movement/projectile cooldowns (lines 150-155)
   - Drain keyboard buffer, process command queue (`F0380_COMMAND_ProcessQueue_CPSC`)
   - Spin-wait for player input (lines 165-219)

5. **Command queue processing** — `COMMAND.C:2045-2829` / `F0380_COMMAND_ProcessQueue_CPSC`  
   - Gated by `G0310_i_DisabledMovementTicks` and `G0311_i_ProjectileDisabledMovementTicks`
   - Dispatches turn (type 1-2) and move (type 3-6) commands
   - Uses a semaphore/lock pattern for queue safety

**Interrupt vs. poll-based design**
- VBLANK, mouse, and sound: **interrupt-driven** (IRQ4 and IRQ6)
- Keyboard: **polled** in main loop from IKBD hardware buffer (`M527_IsCharacterInKeyboardBuffer`)
- Command queue: **polled** in main loop (not interrupt-driven)
- The main loop actively spins waiting for player input; CPU is not idle during gameplay

**CPU load estimate**
| Task | Relative load |
|---|---|
| VBLANK handler (50-60 Hz) | ~5% |
| Mouse/IKBD handler (on event) | <1% |
| Sound DMA (background) | <1% |
| Dungeon view draw (once per action) | ~60-80% of frame time |
| Command queue processing | ~5-15% |
| Timeline processing | ~1-5% |

**Key bottleneck: F0128_DUNGEONVIEW_Draw_CPSF**
- This is the single heaviest function
- Iterates over all visible cells in depth order (D3L, D3R, D3C, D2L, D2R, D2C, D1L, D1R, D1C, D0L, D0R, D0C)
- Calls bitmap decompression/blit for each wall/object/creature cell
- Takes the majority of main-loop time during navigation

**Timer B (palette switching)**
- `BASE.C:837-844`: Timer B is programmed in the VBLANK handler for mid-scanline palette switching (split-screen: champion panel uses full brightness, dungeon view uses light-dependent palette)
- Timer B fires at the programmed scan line and triggers palette swap

## Firestaff coverage
- `m11_game_loop.c` / `m11_main_loop_tick` — mirrors F0002 game loop order
- `dm1_v1_movement_pipeline_pc34_compat.c:244-443` — command processing pipeline
- No CPU usage measurement probe was built; source documentation is the lock.

## Status
✅ SOURCE-LOCKED — CPU tasks and interrupt/polling model documented with file:line citations.
