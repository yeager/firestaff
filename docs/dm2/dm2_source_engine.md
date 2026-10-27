# DM2 V1 Engine

## Main Loop

GAME_LOOP() is the core per-frame loop (SkWinCore.h/cpp). The loop processes:
1. Timer tick (int 08h handler: _INT08_HANDLER) — movement, AI ticks
2. Mouse input (IBMIO_MOUSE_HANDLER, int 33h)
3. Keyboard input (IBMIO_KBOARD_HANDLER, int 09h)
4. Event queue (c_eventqueue) — delayed/timed actions
5. Process timers (PROCEED_TIMERS)
6. Render pass — blit from backbuffer to screen

Unlike DM1 which used a fixed 10Hz or 15Hz tick, DM2 has more granular timer handling:
- PROCESS_TIMER_0C — per-champion light/torch timers
- PROCESS_TIMER_RESURRECTION — death/resurrection countdown
- CONTINUE_ORNATE_ANIMATOR — special animations
- CONTINUE_TICK_GENERATOR — game event ticker

## Rendering Pipeline

Step 1: Clear/composite backbuffer
- DM2_FILL_BACKBUFF_RECT or DM2_FILL_ENTIRE_PICT with background color
- pictbuff holds pre-loaded UI graphics

Step 2: Draw dungeon view
- c_map: renders 3D dungeon view into backbuffer
- c_cloud: applies fog-of-war per tile
- c_light: calculates per-tile lighting

Step 3: Draw UI overlay
- c_gui_draw: draws panels, borders, text
- c_gui_vp: viewport management
- c_buttongroup: interactive button regions

Step 4: Text/strings
- c_gfx_str: text rendering using font from gdatfile

Step 5: Double-buffer blit
- blit_toscreen(): copy backbuffer rect to dm2screen
- Uses BLITMODE0, NOALPHA, 8bpp source to 8bpp dest
- DM2_DRAWINGS_COMPLETED() signals end of frame

## Input System

CSkWin (SkWin.h):
- xMiceInput[MAXMICEIN] — mouse events (btn, x, y)
- xKeybInput[MAXKEYBIN] — keyboard events (raw scancodes)
- DequeueKinput() / allocMinput() — allocate from ring buffers
- Double-step movement flag (enableDoubleStepMove)

Input routing:
- c_input: high-level input processing
- c_keybd: keyboard scancode to game-key mapping
- c_mcursor: mouse cursor display
- c_tmouse: tracked mouse state

## AI System

c_ai (creature AI):
- Per-creature state machine
- Engagement logic via c_engage
- c_creature: entity definition (hit points, type, position)

Timer-driven AI ticks:
- PROCESS_TIMER_59 — creature AI tick
- _4976_0cba[12] — attack/combat handlers

## Audio System

MIDI music:
- c_midi — MIDI file playback via SkWinMIDI (Windows MCI)
- PROCESS_SOUND — ambient sound processing

Sound effects:
- c_sound / c_sfx — WAV sound effects
- c_music_wav — additional WAV audio

## Level Loading

c_loadlevel: dungeon level data loading
- Reads from Graphics .DAT file (img1.dat)
- Tile data decompressed via c_gfx_decode
- c_map: renders loaded dungeon view

## Timer/Event System

c_timer + c_tim_proc:
- Timed procedures scheduled for future execution
- PROCEED_TIMERS() processes all active timers
- Used for: movement animation, combat, light decay, resurrection

c_eventqueue + c_events:
- Event-driven architecture for game actions
- Queued events processed each frame
