# DM1 V1 original DOS capture — in-dungeon movement unblocked

Status: DM1_V1_ORIGINAL_IN_DUNGEON_MOVEMENT_CAPTURE_UNBLOCKED

This note continues the capture lane after
`dm1_v1_original_capture_dungeon_entry_reached.md`. It does not redo the
already-proven `dungeon_gameplay` entry route. The live harness now reaches
the dungeon, preserves the C070 mouse probe as a diagnostic, then uses the
source-locked keyboard-simulation movement path to produce a changed
original-DOS viewport.

No proprietary game frames are committed here. Frames and JSON receipts remain
under the operator-local capture root.

## Host / runtime

- DOSBox Staging 0.82.2 (5e2ba), macOS host
- Capture backend: `dosbox-rawshot` (DOSBox internal rendered screenshot)
- Live capture root:
  `~/firestaff-captures/dm1-v1-fix-keypad5-20260613-123226/`
- Job-local conf:
  `~/firestaff-captures/dm1-v1-fix-keypad5-20260613-123226/dosbox_capture.live.conf`
- Conf sha256:
  `4b4f380cfcd07e80ccf2d9358c93d16c40df0fa2efff4805a85c3e9da74a60e0`
- Movement receipt sha256:
  `09e6131a6359a94dd8044ff8fcd28ccf5efb0c7f7f68d6595dfd569f79db0fa4`

## Conf and route facts

Single `-conf` file, no layered confs:

- `[sdl]` `output=opengl`, `windowresolution=1024x768`,
  `viewport_resolution=1024x768`, job-local `mapperfile=.../dosbox_capture.mapper.map`
- `[dosbox]` `machine=svga_s3`, `memsize=16`
- `[render]` `frameskip=0`, `aspect=auto`, `glshader=none`
- `[cpu]` `core=dynamic`, `cycles=max`
- `[capture]` `capture_dir=.../dosbox-capture`,
  `default_image_capture_formats=rendered raw`
- `[mouse]` `mouse_capture=onclick`, `mouse_sensitivity=100`,
  `mouse_raw_input=true`, `dos_mouse_driver=true`,
  `dos_mouse_immediate=true`
- `[autoexec]` mounts the DM1 PC 3.4 runtime dir as `C:`, then runs `DM.EXE`

Video mode selector option `1` remains VGA. Sound option `1` remains No Sound.
The control selector now uses option `4`, Keyboard Simulation of Digital
Joystick, because selector option `1` Mouse reaches `dungeon_gameplay` but
ignores the post-entry keyboard movement table.

## Movement attempt

Source targets:

- Mouse probe: ReDMCSB `COMMAND.C:396-405`, line 398 maps
  `C003_COMMAND_MOVE_FORWARD` to `C070_ZONE_MOVE_FORWARD`.
- PC screen-relative C070 box: `x=263..289`, `y=125..145`.
- Harness click target: center `(276, 135)` in the normalized 320x200
  framebuffer, fraction `(0.8625, 0.675)`.
- Keyboard fallback: ReDMCSB `COMMAND.C:275-281` binds
  `C003_COMMAND_MOVE_FORWARD` to numeric keypad 5 and Up Arrow in the PC
  movement keyboard table. The live route sends macOS key code `87`
  (`Keypad-5`) after the C070 diagnostic click remains unchanged.

Live result:

- `enter_dungeon` reaches `dungeon_gameplay`.
- The C070 `dungeon_move_forward_click` diagnostic dispatch is logged as
  `mouse:left:c070_move_forward`, `ok=true`, but does not change the viewport.
- The `Keypad-5` fallback dispatch is logged as `mapped=keypad-5`, `ok=true`,
  and does change the original-DOS viewport.
- Before state: `dungeon_gameplay`, viewport sha256:
  `d19a2c8e1fe69e399acf24fb0ce196d8080576b6249238bac7df7d8df3e5e345`
- After state: `dungeon_gameplay`, viewport sha256:
  `d37c77ee27bec57ba2dcef0e3f998a52dd781b6120080678b58997277d6f2e60`
- Before normalized full-frame sha256:
  `3a5ab1a8edd2e5a84eb91fa94907bb87dc9a8213db5e2bcaf1c4b2a18187e345`
- After normalized full-frame sha256:
  `6a6d48a7ce98efc754728fb01298766752204c959e5d70a110803399ec8bd09e`
- The first post-`Keypad-5` rawshot sample changed the viewport hash and was
  saved locally as `original/02_ingame_step_forward.png`.

## Resolution

The earlier blocker was not the dungeon-entry classifier or the capture
backend. It was the input-mode combination: selector option `1` Mouse plus a
post-entry movement action left both the C070 host mouse click and the keyboard
movement key with no viewport mutation. Selecting option `4` Keyboard
Simulation of Digital Joystick before dungeon entry, then activating ENTER via
Return and moving via `Keypad-5`, gives a reproducible source-locked original
movement row.

The C070 mouse path is still kept as a diagnostic and remains non-promoted.
The unblocked path for the original capture lane is the keyboard-simulation
movement route.

## Non-claims

- This is not original-vs-Firestaff pixel parity.
- This does not claim C070 mouse delivery is fixed.
- No proprietary frame data is committed.
