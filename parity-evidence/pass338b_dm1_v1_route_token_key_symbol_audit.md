# Pass338b — DM1 V1 route token / key-symbol audit

Status: `BLOCKED_PASS338B_KEYPAD_NUMERIC_TOKENS_ABSENT_USE_ARROW_ROUTE_TOKENS`

## ReDMCSB source audit

Source tree: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `INPUT.C:531-568` normalizes raw movement input before `F1097_StoreKeyInBuffer`: CapsLock affects movement keys at `531-533`; `L2623_l_NormalizedKeyCode` is built at `539-546`; numeric pad keys are replaced with movement key codes at `548-567`; the result is buffered at `568`. The replacement constants shown there are:
  - numpad 7 (`0x3D37`) -> `0x4600` DEL / turn-left (`INPUT.C:549-550`)
  - numpad 9 (`0x3F39`) -> `0x5F00` Help / turn-right (`INPUT.C:552-553`)
  - numpad 8 (`0x3E38`) -> `0x4C00` Arrow Up / move-forward (`INPUT.C:555-556`)
  - numpad 5 or 2 (`0x2E35`, `0x1E32`) -> `0x4D00` Arrow Down / move-backward (`INPUT.C:558-560`)
  - numpad 4 (`0x2D34`) -> `0x4F00` Arrow Left / move-left (`INPUT.C:562-563`)
  - numpad 6 (`0x2F36`) -> `0x4E00` Arrow Right / move-right (`INPUT.C:565-566`)
- `COMMAND.C:1709-1813` is `F0361_COMMAND_ProcessKeyPress`. It requires `G0443_ps_PrimaryKeyboardInput` at `1734-1735`, scans the primary keyboard table at `1754-1778`, then scans `G0444_ps_SecondaryKeyboardInput` at `1779-1806`, writing matched commands into `G0432_as_CommandQueue` at `1761-1769` / `1788-1793` and unlocking at `1810-1812`.
- `COMMAND.C:669-676` is the A36/A31/A33/A35 arrow-code movement table matching the `INPUT.C` normalized arrow constants (`0x4600`, `0x4C00`, `0x5F00`, `0x4F00`, `0x4D00`, `0x4E00`).
- `COMMAND.C:677-683` is the I34E/I34M movement keyboard table cited by pass335: turn-left `0x004B`, move-forward `0x004C`, turn-right `0x004D`, move-left `0x004F`, move-backward `0x0050`, move-right `0x0051`.
- `COMMAND.C:2146-2158` is the movement dispatch in `F0380_COMMAND_ProcessQueue_CPSC`: commands 1-2 go to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and commands 3-6 go to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.

## Firestaff route parser / scripted-key audit

- `main_loop_m11.c:740-818` maps plain script tokens directly to `M12_MenuInput` values. Supported route/movement tokens include `up`/`u`, `down`/`d`, `left`/`l`, `right`/`r`, `strafe-left`/`sl`, `strafe-right`/`sr`, plus `enter`/`return`, `space`/`act`, `tab`/`champ`, `esc`/`escape`/`back`, `rest`, `stairs`/`descend`, `grab`/`pickup`/`g`, `drop`/`put`/`p`, `rune1`..`rune6`, `cast`/`spell`, `clear`, and `use`/`drink`/`eat`.
- `main_loop_m11.c:821-865` is the scripted SDL key-name path for `key:<name>`. Supported `key:` names are `up`, `down`, `left`, `right`, `enter`, `return`, `kp-enter`, `space`, `tab`, `esc`, `escape`, `f5`, `f9`, `f10`, `f11`, one-letter names `a c d e g i m p q r s u v w x`, and digits `1`..`6`.
- `main_loop_m11.c:867-918` injects SDL events for `key:<name>`, `click:x:y`, and `move:x:y`. Unknown `key:` names return `0x7fffffff` from `m11_script_keycode_from_name` but are still pushed as key events, so they are not reliable supported route symbols.
- `main_loop_m11.c:921-943` tokenizes a comma-separated script, first tries event injection, and otherwise falls back to the plain-token `M12_MenuInput` mapper.
- `main_loop_m11.c:1051-1184` maps SDL key-down events to `M12_MenuInput`: arrow keys drive up/down/left/right; `Q/E` also turn left/right; WASD maps only when `wasdMovementEnabled`; Enter/KP Enter accepts; Escape, Space, Tab, F5/F9/F10/F11, R/X/G/P, digits 1-6, C/V/U/M/I are handled.
- `m11_game_view.c:5227-5250` consumes `M12_MENU_INPUT_UP/DOWN/LEFT/RIGHT/STRAFE_LEFT/STRAFE_RIGHT` as actual game commands: forward, backstep, turn-left, turn-right, strafe-left, strafe-right.
- `input_remap_m12.c:23-29` default bindings are Up/W forward, Down/S backward, Left/Q turn-left, Right/E turn-right, A strafe-left, D strafe-right, Return/KP Enter accept.
- `input_remap_m12.c:125-196` persistent key-name config supports arrows, `return`, `kp_enter` (underscore, not `kp-enter`), space/escape/tab/navigation/F1-F12, letters `a`..`z`, digits `0`..`9`, and modifiers. This is the config/remap layer, not the script parser.

## Supported route-token answer

Supported plain route tokens: `up`, `down`, `left`, `right`, `u`, `d`, `l`, `r`, `strafe-left`, `strafe-right`, `sl`, `sr`, `enter`, `return`, `space`, `act`, `tab`, `champ`, `esc`, `escape`, `back`, `rest`, `stairs`, `descend`, `grab`, `pickup`, `g`, `drop`, `put`, `p`, `rune1`..`rune6`, `cast`, `spell`, `clear`, `use`, `drink`, `eat`.

Supported scripted `key:` symbols: `key:up`, `key:down`, `key:left`, `key:right`, `key:enter`, `key:return`, `key:kp-enter`, `key:space`, `key:tab`, `key:esc`, `key:escape`, `key:f5`, `key:f9`, `key:f10`, `key:f11`, `key:a`, `key:c`, `key:d`, `key:e`, `key:g`, `key:i`, `key:m`, `key:p`, `key:q`, `key:r`, `key:s`, `key:u`, `key:v`, `key:w`, `key:x`, and `key:1`..`key:6`.

Absent from Firestaff `main_loop_m11.c` script/parser support: `kp4`, `kp5`, `kp6`, `kp7`, `kp8`, `kp9`, `kp-4`, `kp-5`, `kp-6`, `kp_4`, `kp_5`, `kp_6`, `numlock`, and any numeric-keypad movement symbol other than `key:kp-enter`. Pass333/pass335 used custom Python `xdotool` route helpers for `numlock`, `kp4`, `kp5`, and `kp6`; those names are not native `main_loop_m11.c` script route tokens.

## Exact token to use after pass335

After pass335 (`BLOCKED_PASS335_TABLE_READY_BUT_KEYPAD_CODES_WRONG`), forward/turn tests should stop using `numlock kp5 kp4 kp6`. Use plain high-level route tokens instead:

- forward: `up`
- turn-left: `left`
- turn-right: `right`
- optional backward/strafe checks: `down`, `strafe-left`, `strafe-right`

If a test specifically needs to exercise SDL event injection rather than direct `M12_MenuInput`, use `key:up`, `key:left`, and `key:right`. Do not use `kp5`/`kp4`/`kp6`; those keypad numeric tokens are absent in the native route parser and were the wrong key-symbol family for the pass335-ready route.

Final blocker status: `BLOCKED_PASS338B_KEYPAD_NUMERIC_TOKENS_ABSENT_USE_ARROW_ROUTE_TOKENS`.
