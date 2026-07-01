# Pass162 C080 address gate

Classification: `ready/address-candidates-and-break-start-bp-bpm-confirmed-no-runtime-hook`

## Function breakpoints

- `F0359_COMMAND_ProcessClick_CPSC` -> `22F4:030D`
- `F0380_COMMAND_ProcessQueue_CPSC` -> `22F4:0699`
- `F0377_COMMAND_ProcessType80_ClickInDungeonView` -> `1E44:02FE`
- `F0275_SENSOR_IsTriggeredByClickOnWall` -> `1859:1405`
- `F0280_CHAMPION_AddCandidateChampionToParty` -> `1782:0031`

## Watchpoints

- `G0432_as_CommandQueue` -> `2C20:3E7A`
- `G0433_i_CommandQueueFirstIndex` -> `2C20:3EC8`
- `G0434_i_CommandQueueLastIndex` -> `2C20:1F08`
- `G0435_B_CommandQueueLocked` -> `2C20:1F0A`
- `G0436_B_PendingClickPresent` -> `2C20:3EC6`
- `G0306_i_PartyMapX` -> `2C20:3C94`
- `G0307_i_PartyMapY` -> `2C20:3CE0`
- `G0308_i_PartyDirection` -> `2C20:3C92`
- `G0305_ui_PartyChampionCount` -> `2C20:3D52`

## DOSBox-debug commands

Commands: `parity-evidence/verification/pass162_c080_queue_trace/pass162_c080_dosbox_debug_commands.txt`

F0372 is static in this public map; use F0275 as the addressable front-wall sensor proxy.
A transcript can promote the gate only if the hit order is observed after debugger run control, not from setup echoes.

## BP acceptance diagnostic

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/dosbox_debug_bp_acceptance/manifest.json`
- status: `FAIL_BREAKPOINT_COMMANDS_NOT_CONFIRMED`
- accepted all: `False`
- interpretation: this is a debugger-control diagnostic only; it does not invalidate the FIRES.MAP addresses.

## DOSBox-X break-start acceptance

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/dosbox_x_break_start_bp_acceptance/manifest.json`
- status: `PASS162_DOSBOX_X_BREAK_START_BP_BPM_ACCEPTED`
- ok: `True`
- interpretation: debugger control and numeric BP/BPM command parsing are proven for an empty-autoexec DOSBox-X break-start session; stock-original runtime hits remain unproven.

## Live HoC break-start diagnostic

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/live_hoc_break_start_probe/manifest.json`
- status: `BLOCKED_PASS162_LIVE_HOC_NO_C080_STOPS`
- route driver: `swift-cgevent`
- mouse post mode: `hid`
- mouse warp: `None`
- DOSBox mouse config: `None`
- DOSBox log: `None`
- DOSBox mouse log summary: `None`
- memory stop count: `34`
- engine ready seen: `True`
- runtime ready seen: `True`
- route window found: `True`
- route control ok: `True`
- first missing expected symbol: `F0359_COMMAND_ProcessClick_CPSC`
- reached F0280: `False`
- interpretation: original runtime stop control and native route input are proven; the current HoC route now blocks before F0359/C080.

## Live movement-click control diagnostic

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_click_control_break_start_probe/manifest.json`
- status: `BLOCKED_PASS162_LIVE_MOVEMENT_CLICK_CONTROL_NO_C080_STOPS`
- route driver: `swift-cgevent`
- mouse post mode: `hid`
- mouse warp: `true`
- DOSBox mouse config: `None`
- DOSBox log: `None`
- DOSBox mouse log summary: `None`
- memory stop count: `34`
- engine ready seen: `True`
- runtime ready seen: `True`
- route window found: `True`
- route control ok: `True`
- first missing expected symbol: `F0359_COMMAND_ProcessClick_CPSC`
- reached F0280: `False`
- interpretation: a simple movement-area click follows the same host input path and still blocks before F0359/C080, so the next pass should inspect DOSBox SDL mouse ingestion/capture rather than more HoC coordinate guesses.

## Live movement double-click control diagnostic

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_double_click_control_break_start_probe/manifest.json`
- status: `BLOCKED_PASS162_LIVE_MOVEMENT_DOUBLE_CLICK_CONTROL_NO_C080_STOPS`
- route driver: `swift-cgevent`
- mouse post mode: `hid`
- mouse warp: `None`
- DOSBox mouse config: `None`
- DOSBox log: `None`
- DOSBox mouse log summary: `None`
- memory stop count: `34`
- engine ready seen: `True`
- runtime ready seen: `True`
- route window found: `True`
- route control ok: `True`
- first missing expected symbol: `F0359_COMMAND_ProcessClick_CPSC`
- reached F0280: `False`
- interpretation: if this still blocks before F0359, a first-click-only focus/capture explanation is not enough.

## Live movement autolock control diagnostic

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_autolock_control_break_start_probe/manifest.json`
- status: `BLOCKED_PASS162_LIVE_MOVEMENT_AUTOLOCK_CONTROL_NO_C080_STOPS`
- route driver: `swift-cgevent`
- mouse post mode: `hid`
- mouse warp: `true`
- DOSBox mouse config: `{'autolock': 'true', 'clip_mouse_button': 'none', 'mouse_emulation': 'always', 'mouse_log': 'debug', 'usesystemcursor': 'false'}`
- DOSBox log: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_autolock_control_break_start_probe/dosbox_runtime.log`
- DOSBox mouse log summary: `{'path': 'parity-evidence/verification/pass162_c080_queue_trace/live_movement_autolock_control_break_start_probe/dosbox_runtime.log', 'exists': True, 'mouse_line_count': 104, 'route_motion_button_line_count': 0, 'first_mouse_lines': ["Logging: opened logfile '/Users/bosse/Documents/Firestaff/parity-evidence/verification/pass162_c080_queue_trace/live_movement_autolock_control_break_start_probe/dosbox_runtime.log' successfully. All further logging will go to this file.", '         0 DEBUG GUI:SDL 1.2.14 hack: SDL_DISABLE_LOCK_KEYS=1', 'Prevent capture: 0', '         0       GUI:Press Ctrl-F10 to capture/release mouse, Alt-F10 for configuration.', '         0 DEBUG MISC:Initializing screenshot and A/V capture system', '         0 DEBUG MISC:Mixer: sample_accurate=0 blocksize=1024 sdl_rate=166053024Hz mixer_rate=48000Hz channels=0 samples=42440 min/max/need=0/0/0 per_ms=48 0/1000 samples prebuffer=1200 dcadj=0.0000208020(en=1)', '         0 DEBUG MOUSE:Initializing mouse interface emulation', "         0 DEBUG MISC:Calling event Reset handler (0x104c71520) 'MOUSE_OnReset'"], 'first_route_motion_button_lines': []}`
- memory stop count: `34`
- engine ready seen: `True`
- runtime ready seen: `True`
- route window found: `True`
- route control ok: `True`
- first missing expected symbol: `F0359_COMMAND_ProcessClick_CPSC`
- reached F0280: `False`
- interpretation: if this still blocks before F0359, the default autolock/mouse_emulation setting is not the missing original input boundary.

## Live movement keyboard-capture control diagnostic

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_keyboard_capture_control_break_start_probe/manifest.json`
- status: `BLOCKED_PASS162_LIVE_MOVEMENT_KEYBOARD_CAPTURE_CONTROL_NO_C080_STOPS`
- route driver: `swift-cgevent`
- mouse post mode: `hid`
- mouse warp: `true`
- DOSBox mouse config: `{'autolock': 'true', 'clip_mouse_button': 'none', 'mouse_emulation': 'always', 'mouse_log': 'debug', 'usesystemcursor': 'false'}`
- DOSBox log: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_keyboard_capture_control_break_start_probe/dosbox_runtime.log`
- DOSBox mouse log summary: `{'path': 'parity-evidence/verification/pass162_c080_queue_trace/live_movement_keyboard_capture_control_break_start_probe/dosbox_runtime.log', 'exists': True, 'mouse_line_count': 104, 'route_motion_button_line_count': 0, 'first_mouse_lines': ["Logging: opened logfile '/Users/bosse/Documents/Firestaff/parity-evidence/verification/pass162_c080_queue_trace/live_movement_keyboard_capture_control_break_start_probe/dosbox_runtime.log' successfully. All further logging will go to this file.", '         0 DEBUG GUI:SDL 1.2.14 hack: SDL_DISABLE_LOCK_KEYS=1', 'Prevent capture: 0', '         0       GUI:Press Ctrl-F10 to capture/release mouse, Alt-F10 for configuration.', '         0 DEBUG MISC:Initializing screenshot and A/V capture system', '         0 DEBUG MISC:Mixer: sample_accurate=0 blocksize=1024 sdl_rate=1661077504Hz mixer_rate=48000Hz channels=0 samples=9688 min/max/need=0/0/0 per_ms=48 0/1000 samples prebuffer=1200 dcadj=0.0000208020(en=1)', '         0 DEBUG MOUSE:Initializing mouse interface emulation', "         0 DEBUG MISC:Calling event Reset handler (0x104db9520) 'MOUSE_OnReset'"], 'first_route_motion_button_lines': []}`
- memory stop count: `34`
- engine ready seen: `True`
- runtime ready seen: `True`
- route window found: `True`
- route control ok: `True`
- first missing expected symbol: `F0359_COMMAND_ProcessClick_CPSC`
- reached F0280: `False`
- interpretation: if this still blocks before F0359 and the log lacks motion/button lines, keyboard capture toggling did not make the injected mouse events enter DOSBox-X.

## Live movement cliclick control diagnostic

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_cliclick_control_break_start_probe/manifest.json`
- status: `BLOCKED_PASS162_LIVE_MOVEMENT_CLICLICK_CONTROL_NO_C080_STOPS`
- route driver: `swift-cgevent`
- mouse post mode: `cliclick`
- mouse warp: `true`
- DOSBox mouse config: `{'autolock': 'true', 'clip_mouse_button': 'none', 'mouse_emulation': 'always', 'mouse_log': 'debug', 'usesystemcursor': 'false'}`
- DOSBox log: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_cliclick_control_break_start_probe/dosbox_runtime.log`
- DOSBox mouse log summary: `{'path': 'parity-evidence/verification/pass162_c080_queue_trace/live_movement_cliclick_control_break_start_probe/dosbox_runtime.log', 'exists': True, 'mouse_line_count': 103, 'route_motion_button_line_count': 0, 'first_mouse_lines': ['         0 DEBUG GUI:SDL 1.2.14 hack: SDL_DISABLE_LOCK_KEYS=1', 'Prevent capture: 0', '         0       GUI:Press Ctrl-F10 to capture/release mouse, Alt-F10 for configuration.', '         0 DEBUG MISC:Initializing screenshot and A/V capture system', '         0 DEBUG MISC:Mixer: sample_accurate=0 blocksize=1024 sdl_rate=2764162272Hz mixer_rate=48000Hz channels=0 samples=26056 min/max/need=0/0/0 per_ms=48 0/1000 samples prebuffer=1200 dcadj=0.0000208020(en=1)', '         0 DEBUG MOUSE:Initializing mouse interface emulation', "         0 DEBUG MISC:Calling event Reset handler (0x1025b5520) 'MOUSE_OnReset'", '         3       MOUSE:INT 15H PS/2 emulation enabled'], 'first_route_motion_button_lines': []}`
- memory stop count: `34`
- engine ready seen: `True`
- runtime ready seen: `True`
- route window found: `True`
- route control ok: `True`
- first missing expected symbol: `F0359_COMMAND_ProcessClick_CPSC`
- reached F0280: `False`
- interpretation: if this still blocks before F0359, the missing boundary is broader than the Swift/CGEvent helper and should be investigated in DOSBox-X SDL/Cocoa event ingestion or the debugger event pump.

## Live movement System Events control diagnostic

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_systemevents_control_break_start_probe/manifest.json`
- status: `BLOCKED_PASS162_LIVE_MOVEMENT_SYSTEMEVENTS_CONTROL_NO_C080_STOPS`
- route driver: `swift-cgevent`
- mouse post mode: `systemevents`
- mouse warp: `true`
- DOSBox mouse config: `{'autolock': 'true', 'clip_mouse_button': 'none', 'mouse_emulation': 'always', 'mouse_log': 'debug', 'usesystemcursor': 'false'}`
- DOSBox log: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_systemevents_control_break_start_probe/dosbox_runtime.log`
- DOSBox mouse log summary: `{'path': 'parity-evidence/verification/pass162_c080_queue_trace/live_movement_systemevents_control_break_start_probe/dosbox_runtime.log', 'exists': True, 'mouse_line_count': 103, 'route_motion_button_line_count': 0, 'first_mouse_lines': ['         0 DEBUG GUI:SDL 1.2.14 hack: SDL_DISABLE_LOCK_KEYS=1', 'Prevent capture: 0', '         0       GUI:Press Ctrl-F10 to capture/release mouse, Alt-F10 for configuration.', '         0 DEBUG MISC:Initializing screenshot and A/V capture system', '         0 DEBUG MISC:Mixer: sample_accurate=0 blocksize=1024 sdl_rate=1627652480Hz mixer_rate=48000Hz channels=0 samples=42440 min/max/need=0/0/0 per_ms=48 0/1000 samples prebuffer=1200 dcadj=0.0000208020(en=1)', '         0 DEBUG MOUSE:Initializing mouse interface emulation', "         0 DEBUG MISC:Calling event Reset handler (0x1045d1520) 'MOUSE_OnReset'", '         3       MOUSE:INT 15H PS/2 emulation enabled'], 'first_route_motion_button_lines': []}`
- memory stop count: `34`
- engine ready seen: `True`
- runtime ready seen: `True`
- route window found: `True`
- route control ok: `True`
- first missing expected symbol: `F0359_COMMAND_ProcessClick_CPSC`
- reached F0280: `False`
- interpretation: if this still blocks before F0359, macOS Accessibility/System Events delivery is not enough either, so the next boundary stays inside DOSBox-X SDL/Cocoa event ingestion or the debugger event pump.

## Live movement PID-post control diagnostic

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_pid_control_break_start_probe/manifest.json`
- status: `BLOCKED_PASS162_LIVE_MOVEMENT_PID_CONTROL_NO_C080_STOPS`
- route driver: `swift-cgevent`
- mouse post mode: `pid`
- mouse warp: `true`
- DOSBox mouse config: `{'autolock': 'true', 'clip_mouse_button': 'none', 'mouse_emulation': 'always', 'mouse_log': 'debug', 'usesystemcursor': 'false'}`
- DOSBox log: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_pid_control_break_start_probe/dosbox_runtime.log`
- DOSBox mouse log summary: `{'path': 'parity-evidence/verification/pass162_c080_queue_trace/live_movement_pid_control_break_start_probe/dosbox_runtime.log', 'exists': True, 'mouse_line_count': 103, 'route_motion_button_line_count': 0, 'first_mouse_lines': ['         0 DEBUG GUI:SDL 1.2.14 hack: SDL_DISABLE_LOCK_KEYS=1', 'Prevent capture: 0', '         0       GUI:Press Ctrl-F10 to capture/release mouse, Alt-F10 for configuration.', '         0 DEBUG MISC:Initializing screenshot and A/V capture system', '         0 DEBUG MISC:Mixer: sample_accurate=0 blocksize=1024 sdl_rate=4173508768Hz mixer_rate=48000Hz channels=0 samples=9672 min/max/need=0/0/0 per_ms=48 0/1000 samples prebuffer=1200 dcadj=0.0000208020(en=1)', '         0 DEBUG MOUSE:Initializing mouse interface emulation', "         0 DEBUG MISC:Calling event Reset handler (0x102c79520) 'MOUSE_OnReset'", '         3       MOUSE:INT 15H PS/2 emulation enabled'], 'first_route_motion_button_lines': []}`
- memory stop count: `34`
- engine ready seen: `True`
- runtime ready seen: `True`
- route window found: `True`
- route control ok: `True`
- first missing expected symbol: `F0359_COMMAND_ProcessClick_CPSC`
- reached F0280: `False`
- interpretation: if this still blocks before F0359, direct postToPid delivery is not enough either, so the next boundary stays inside DOSBox-X SDL/Cocoa event ingestion or the debugger event pump.

## Live movement OpenGL-output control diagnostic

- manifest: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_opengl_control_break_start_probe/manifest.json`
- status: `BLOCKED_PASS162_LIVE_MOVEMENT_OPENGL_CONTROL_DEBUGGER_PACKET_NOT_RETAINED`
- route driver: `swift-cgevent`
- mouse post mode: `hid`
- mouse warp: `true`
- DOSBox mouse config: `{'autolock': 'true', 'clip_mouse_button': 'none', 'mouse_emulation': 'always', 'mouse_log': 'debug', 'output': 'opengl', 'usesystemcursor': 'false'}`
- DOSBox log: `parity-evidence/verification/pass162_c080_queue_trace/live_movement_opengl_control_break_start_probe/dosbox_runtime.log`
- DOSBox mouse log summary: `{'path': 'parity-evidence/verification/pass162_c080_queue_trace/live_movement_opengl_control_break_start_probe/dosbox_runtime.log', 'exists': True, 'mouse_line_count': 97, 'route_motion_button_line_count': 0, 'first_mouse_lines': ['         0 DEBUG GUI:SDL 1.2.14 hack: SDL_DISABLE_LOCK_KEYS=1', 'Prevent capture: 0', '         0       GUI:Press Ctrl-F10 to capture/release mouse, Alt-F10 for configuration.', '         0 DEBUG MISC:Initializing screenshot and A/V capture system', '         0 DEBUG MISC:Mixer: sample_accurate=0 blocksize=1024 sdl_rate=1895974912Hz mixer_rate=48000Hz channels=0 samples=26008 min/max/need=0/0/0 per_ms=48 0/1000 samples prebuffer=1200 dcadj=0.0000208020(en=1)', '         0 DEBUG MOUSE:Initializing mouse interface emulation', "         0 DEBUG MISC:Calling event Reset handler (0x102205520) 'MOUSE_OnReset'", '         3       MOUSE:INT 15H PS/2 emulation enabled'], 'first_route_motion_button_lines': []}`
- memory stop count: `17`
- engine ready seen: `False`
- runtime ready seen: `False`
- route window found: `False`
- route control ok: `True`
- first missing expected symbol: `F0359_COMMAND_ProcessClick_CPSC`
- reached F0280: `False`
- interpretation: if this run does not retain the debugger packet and reach FIRES/DUNGEON readiness, treat it as an OpenGL-backend harness/readiness blocker rather than as F0359 mouse-ingestion evidence.
