# Pass 1115 — DM2 mouse/touch input subsystem (c_tmouse.cpp)

## Source

skproject `SKULLWIN/c_tmouse.cpp` (598 lines, 26 functions across 5 classes).

## Ported structures

| skproject class/struct | Firestaff type |
|---|---|
| `c_servercommand` | `DM2_TmouseServerCommand` |
| `c_commandqueue` | `DM2_TmouseCommandQueue` |
| `c_evententry` (mouse) | `DM2_TmouseEventEntry` |
| `c_mousequeue` | `DM2_TmouseMouseQueue` |
| `c_xmouserect` | `DM2_TmouseXMouseRect` |
| `c_Tmouse` | `DM2_TmouseState` |
| `c_clickrectnode` | `DM2_TmouseClickRectNode` |
| `c_clickrectdata` | `DM2_TmouseClickRectData` |
| `s_mcursor` | `DM2_TmouseCursorDesc` |
| `e_cursoridx` | `DM2_TmouseCursorIdx` |

## Ported functions

| skproject function | Firestaff function | Lines |
|---|---|---|
| `c_commandqueue::push` | `dm2_v1_tmouse_command_queue_push` | 22-32 |
| `c_commandqueue::pop` | `dm2_v1_tmouse_command_queue_pop` | 34-44 |
| `c_mousequeue::init` | `dm2_v1_tmouse_mouse_queue_init` | 174-181 |
| `c_mousequeue::push` | `dm2_v1_tmouse_mouse_queue_push` | 183-204 |
| `c_mousequeue::pop` | `dm2_v1_tmouse_mouse_queue_pop` | 206-234 |
| `c_xmouserect::init` | `dm2_v1_tmouse_xmouserect_init` | 16-20 |
| `c_Tmouse::init` | `dm2_v1_tmouse_init` | 141-172 |
| `c_Tmouse::is_visible` | `dm2_v1_tmouse_is_visible` | header |
| `c_Tmouse::hide` | `dm2_v1_tmouse_hide` | header |
| `c_Tmouse::driver_blockmouseinput` | `dm2_v1_tmouse_block_mouse_input` | 522-525 |
| `c_Tmouse::driver_unblockmouseinput` | `dm2_v1_tmouse_unblock_mouse_input` | 528-532 |
| `send_command` | `dm2_v1_tmouse_send_command` | 48-53 |
| `c_Tmouse::command_interpreter` | `dm2_v1_tmouse_command_interpreter` | 59-139 |
| `c_Tmouse::T1_queue_0x20` | `dm2_v1_tmouse_queue_0x20` | 240-323 |
| `c_Tmouse::T1_queue_event` | `dm2_v1_tmouse_queue_event` | 326-344 |
| `c_Tmouse::T1_driver_mouseint` | `dm2_v1_tmouse_driver_mouseint` | 419-432 |
| `DM2_HIDE_MOUSE` | `dm2_v1_tmouse_hide_mouse` | 538-541 |
| `DM2_SHOW_MOUSE` | `dm2_v1_tmouse_show_mouse` | 544-549 |
| `DM2_MOUSE_SET_CAPTURE` | `dm2_v1_tmouse_set_capture` | 552-555 |
| `DM2_MOUSE_RELEASE_CAPTURE` | `dm2_v1_tmouse_release_capture` | 558-562 |
| `DM2_REFRESHMOUSE` | `dm2_v1_tmouse_refresh_mouse` | 565-570 |
| `DM2_CHOOSE_CURSOR3` | `dm2_v1_tmouse_choose_cursor3` | 573-576 |
| `DM2_RELEASE_MOUSE_CAPTURES` | `dm2_v1_tmouse_release_mouse_captures` | 579-588 |
| `DM2_GET_MOUSE_ENTRY_DATA` | `dm2_v1_tmouse_get_mouse_entry_data` | 591-598 |

## Not ported (graphics-only, no logic parity needed)

- `c_Tmouse::T1_drawmouse` — cursor compositing into framebuffer (SDL3 handles this)
- `c_mblitter::blit` / `blit_hline` / `blit_hline_masked` / `blit_hline_stretched` — pixel blitting

## Test coverage

39 tests in `tests/test_dm2_v1_tmouse_pc34_compat.c`:

- Command queue: init, push/pop, empty pop, full, FIFO order (5)
- Mouse queue: init, push/pop, empty pop, full (4)
- XMouseRect: init (1)
- Tmouse state: init (1)
- Visibility: show/hide (1)
- Block/unblock: increment/decrement/floor-at-zero (1)
- send_command: block-push-unblock cycle (1)
- Command interpreter: all 7 command cases + mouse queue left/right press (7)
- T1_queue_event: below 0x20, 0x20 with no rectlist (2)
- T1_driver_mouseint: accepted, blocked, fetch_busy, captured (4)
- Public API: hide, show (with transition trigger), set/release capture, refresh, choose_cursor3, release_mouse_captures (active/none), get_entry_data (9)
- Click-rect: no rectlist, hit with cursor type 1, cursor type 2 -> CURSOR3 (3)

## Verification

```
39 passed, 0 failed
```
