# DM2 V1 — Input System

## Overview

DM2's input system is SDL-driven, split into keyboard and mouse subsystems, both routed through a unified event processing pipeline. The system was extended from DM1's DOS interrupt-based model to a polling/event-loop model under Windows SDL.

## Keyboard Input

### Core Processing
- `DM2_IBMIO_USER_INPUT_CHECK()` — polls keyboard state each frame, writes to event table `v_evtable`
- `v_evtable` — global `const i8*` pointing to keyboard event lookup table
- SDL key events are translated into platform-specific events before being queued

### Key Characteristics vs DM1
- DM1 used BIOS interrupt 0x16 for keyboard polling
- DM2 uses SDL `SDL_keyboard.h` / `SDL_mouse.h` APIs under Windows
- Key repeat, modifier state (Ctrl/Alt/Shift) handled by SDL backend
- Text entry encoded via `glbTextEntryEncoded` flag (Bit16u at `_4976_52de`)

## Mouse Input

### Architecture
- `c_Tmouse` class — main mouse controller (formerly `c_tmouse` in SKULLWIN)
- `c_mousequeue` — FIFO buffer of `MOUSE_QUEUE_LENGTH = 10` events
- `c_evententry` — individual mouse event (x, y, button state)
- Mouse position stored as `glbMouseXPos`/`glbMouseYPos` in SkWinCore globals
- `c_xmouserect` — driver-level mouse rect + button state

### Mouse Queue (c_mousequeue)
- Buffered FIFO with separate in/out indices
- `push()` / `pop()` for queuing mouse events
- Multiple mouse events can accumulate per frame
- Processed in `T1_driver_mouseint()`

### Mouse Capture
- `DM2_MOUSE_SET_CAPTURE()` / `DM2_MOUSE_RELEASE_CAPTURE()`
- `driver_blockmouseinput()` / `driver_unblockmouseinput()` — prevent input during transitions
- `glbMouseVisibility` (i16 at `_4976_4860`) — cursor visibility control
- `mouse_invisible` in `c_Tmouse` — immediate hide/show

### Cursor System
- `e_cursoridx` — cursor shape enumeration
- `DM2_CHOOSE_CURSOR3()` — select appropriate cursor for context
- `T1_drawmouse()` — renders cursor sprite via `c_mblitter`
- `c_mblitter` — blit mouse sprites (cursor bitmap) onto `dm2mscreen` (mouse screen buffer)
- Mouse cursor sprites: `cursor1.cur` and others in SKWIN resources

### Mouse vs DM1 Differences
- DM1: INT 33h mouse driver, BIOS-level cursor
- DM2: SDL mouse, Windows handle, multiple cursor types
- DM1: no queuing (immediate events); DM2: buffered FIFO
- DM2: mouse capture/release API for modal interactions

## Input Event Loop

### Main Loop Functions
- `DM2_EVENT_LOOP()` — top-level event dispatcher
- `event_loop_T1()` — friend function for mouse processing
- `c_Tmouse::command_interpreter()` — routes mouse commands

### Event Data Structure
- `c_eventdata` — contains primary `c_uievent` + `table1e04e0[3]` array + event table pointer
- `c_uievent` — x, y, idx, c_rect (bounding rect for click target)
- Events routed to clickable rects (`c_clickrectnode* rectlist1/rectlist2`)

### Command Queue
- `c_commandqueue` — separate FIFO for server/game commands (length 10)
- `send_command(i16 c)` — enqueue a command
- Commands processed by `c_Tmouse::command_interpreter()`

## Input Differences from DM1 Summary

| Feature | DM1 | DM2 |
|---|---|---|
| Keyboard | BIOS INT 16h polling | SDL keyboard API |
| Mouse | INT 33h, single-event | SDL, buffered queue (10) |
| Text entry | Direct ASCII | Encoded (`glbTextEntryEncoded`) |
| Cursor | Single sprite | Multiple cursor types |
| Capture | None | Full capture/release API |
| Event system | Immediate | FIFO event queue |

## Key Globals (SkWinCore)

- `glbMouseXPos`, `glbMouseYPos` — current mouse position
- `glbMouseButtonState` — button state bits (bit0=left, bit1=right)
- `glbMouseVisibility` — cursor visibility flag
- `glbShowMousePointer` — show/hide cursor
- `sysMousePositionCaptured` — mouse capture state
- `MousePosition glbMousePosition` — struct mouse position
- `_4976_4e02[11]` — mouse state history ring buffer

## Source Files

- `skproject/SKULLWIN/c_input.h` — input class declarations
- `skproject/SKULLWIN/c_input.cpp` — input implementation
- `skproject/SKULLWIN/c_tmouse.h` — mouse controller class
- `skproject/SKULLWIN/c_tmouse.cpp` — mouse implementation
- `skproject/SKWIN/SkWinCore.h` / `.cpp` — global input state
- `skproject/SKWIN/defines.h` — input-related constants
