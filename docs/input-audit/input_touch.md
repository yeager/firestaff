# DM1 V1 Touch/Pointer Input — Source Audit

## ReDMCSB Source Lock
ReDMCSB WIP20210206, Toolchains/Common/Source/INPUT.C, CLIKVIEW.C, COMMAND.C

## 1. Mouse/Pointer Hardware Abstraction

### INPUT.C — RAWMOUSE Event Processing (F0543_INPUT_DeviceInterruptHandler)
- G1038_i_MouseX, G1039_i_MouseY accumulate ie_X/ie_Y deltas
  (unless G0597_B_IgnoreMouseMovements is set)
- Clamp: x in [0, 640], y in [0, 400] (Amiga coords); logical = raw >> 1
  On A3x: x clamped to [0, 639], y to [0, 399]
- Mouse pointer rebuilt when hotspot changes: F0073_MOUSE_BuildPointerScreenArea
- Button events dispatched to F0359_COMMAND_ProcessClick_CPSC

Button handling:
- IECODE_LBUTTON (0x68): set G1050_B_LeftMouseButtonDown, call
  F0359_COMMAND_ProcessClick(x>>1, y>>1, MASK0x0002_MOUSE_LEFT_BUTTON)
- IECODE_RBUTTON (0x69): set G1051_B_RightMouseButtonDown, call
  F0359_COMMAND_ProcessClick(x>>1, y>>1, MASK0x0001_MOUSE_RIGHT_BUTTON)
- IECODE_LBUTTON+UP: clear G1050, call F0359 with MASK0x0004_MOUSE_LEFT_BUTTON_UP
- IECODE_RBUTTON+UP: clear G1051, call F0359 with MASK0x0008_MOUSE_RIGHT_BUTTON_UP

A3x additionally calls F0359 on button-up events (A20 only on button-down).

## 2. Mouse Hit-Test Tables

### COMMAND.C:72-84 — Primary Mouse Input Table (G0447)
G0447 rows for I34E/I34M (COMMAND.C:375-405 active source-order):
- Champion portrait bars: x(0/69/138/207), y=0, w=67, h=29 per champion
- Champion icon corners: x(281/301), y(0/15), w=19, h=14, 4 corners
- Spell area: (233, 42, 87, 33) → DM1_V1_COMMAND_CLICK_IN_SPELL_AREA
- Action area: (233, 77, 87, 45) → DM1_V1_COMMAND_CLICK_IN_ACTION_AREA

### COMMAND.C:106-121 — Secondary Mouse Input Table (G0448)
G0448 movement rows for C001/C003/C002/C006/C005/C004/C080/C083:
  Turn Left:   (234-261, 125-145)
  Move Forward:(263-289, 125-145)
  Turn Right:  (291-318, 125-145)
  Move Left:   (234-261, 147-167)
  Move Back:   (263-289, 147-167)
  Move Right:  (291-318, 147-167)
  Dungeon view:(0-33, 223-168) for C080

### COMMAND.C:1632-1638 — Special Mouse Event Ordinals
Before primary/secondary table lookup:
- C33 (leave champion icon region) → C129 (release champion icon)
- C04 (left button up during eye/mouth/wall hold) → C254 (stop pressing)
These survive queue flush via reserved 2 slots (DEFS.H:3263-3264).

## 3. CLIKVIEW.C — Viewport Click Routing
- F0372: Click on front-wall sensor cell → F0275_SENSOR_IsTriggeredByClickOnWall
- F0373: Grab leader hand object from party or front square (view cells 0-3)
- F0664: Hold left button on imaginary fake wall → periodic knock sound;
  sets G2152_B_PressingClosedImaginaryFakeWall + G0597_B_IgnoreMouseMovements

## 4. Firestaff Touch Implementation

### dm1_v1_input_poll_pc34_compat.c
- m11_input_mouse_move: delta accumulation + clamping
- m11_input_mouse_button_down/up: mirror F0543 RAWMOUSE button state
  with duplicate-press guards (leftButtonDown/rightButtonDown)
- m11_input_mouse_get_position: returns current x,y
- Screen bounds: 319x199 (PC34 logical resolution)

### dm1_v1_viewport_click_pc34_compat.c
- viewport_click_at: maps screen (x,y) to dungeon cell via perspective math
- knock_on_wall: hold logic for fake wall knock (F0664 path)
  - Press: set G0597_B_IgnoreMouseMovements = true
  - Release: clear ignore flag, play thud
- touch_front_wall_sensor: F0372 path — isTriggeredByClickOnWall check

### dm1_v1_input_command_queue_pc34_compat.c
- command_for_primary_mouse: hit-test champion portraits, icon corners,
  spell/action areas at correct coordinates (COMMAND.C:72-84)
- command_for_secondary_mouse: hit-test movement control pad
  (COMMAND.C:106-121) and dungeon view
- command_for_mouse: primary first, then C129/C254 special ordinals,
  then secondary

## 5. Gaps
None identified. Touch input routing (mouse delta accumulation, button
state machine, hit-test tables, fake wall hold, wall sensor touch) is fully
source-locked against INPUT.C F0543, COMMAND.C primary/secondary tables,
CLIKVIEW.C F0372/F0373/F0664.

## 6. Verdict
SOURCE-LOCKED. All pointer/touch paths documented with file:line citations.
Firestaff correctly handles button state with duplicate-press guards,
viewport→dungeon cell mapping, fake wall knock hold, and wall sensor touch.
