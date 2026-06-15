# DM1 V1 original DOS capture — dungeon entry reached

Status: REACHED_DM1_V1_ORIGINAL_DUNGEON_GAMEPLAY

This evidence note records the first reproducible, capture-backed run of the
DM1 PC 3.4 original-DOS route that actually arrives at the in-game dungeon
viewport (`dungeon_gameplay`) and writes capture artifacts, via
`docs/parity/tools/dosbox_capture_session.py --live`.

No proprietary game frames are committed here: the DM1 PC 3.4 framebuffer
images stay in the operator-local capture root
(`~/firestaff-captures/`). This note records the route facts and the
hash/density receipt so the run is auditable and reproducible without
shipping user-supplied game data.

## Host / runtime

- DOSBox Staging 0.82.2 (5e2ba), macOS 15 (Darwin 25.5.0, arm64)
- cliclick + osascript (System Events) for host input
- DM1 PC 3.4 runtime: `DM.EXE` + `DATA/` (GRAPHICS.DAT/DUNGEON.DAT canonical
  SHA256 verified by `dosbox_capture_preflight.py`)
- Capture backend: `dosbox-rawshot` (DOSBox internal rendered screenshot)

## Verified live route (DEFAULT_PLAN)

1. `graphics_select` — type `1` + Return at the SELECTOR graphics menu
   (VGA). settle-only (text menu, not a dungeon framebuffer).
2. `sound_select` — type `1` + Return (No Sound). settle-only.
3. `input_select` — type `1` + Return (Mouse). settle-only.
4. `entrance_wall` — the Dungeon Master title art then the
   ENTER/RESUME/QUIT entrance wall renders. settle-only.
5. `enter_dungeon` — click the ENTER target on the right-hand stone wall
   (framebuffer-fractional `(696/799, 160/599)`); the dungeon corridor
   viewport renders. Gated on the density classifier.
6. `dungeon_gameplay` — 3 stable frames at the dungeon target.

## Capture receipt (two reproduced runs, identical)

- `dungeon_gameplay` stable samples: 6
- viewport_nonblack = 0.9268, rightcol_nonblack = 0.1154 (< 0.135 threshold),
  full_nonblack = 0.4647 → matches the runbook's `dungeon_gameplay` envelope
  (v ≥ 0.135 AND r < 0.135)
- normalized RGB sha256 (dungeon sample): `3a5ab1a8edd2e5a8…`
- saved originals sha256:
  `afafc1fe80aa132889d518af2391525a03fba2d40b7aea7654b9d5456687c036`

## Root causes fixed to reach dungeon entry

The route stack was complete in tooling but never reached the dungeon on a
real host because of three macOS host-input facts, each isolated with an
executable probe:

1. **Window focus** — launching the inner `Contents/MacOS/dosbox` binary
   left the window behind Terminal; AppleScript `set frontmost to true`
   alone did not raise it. `open -a "DOSBox Staging"` reliably activates the
   running instance (no duplicate process). (`_activate_dosbox` /
   `_open_activate_dosbox`.)
2. **Screenshot key** — DOSBox's default raw-screenshot Ctrl+F5 (and Cmd+F5)
   are swallowed by macOS keyboard navigation and never reach DOSBox, so the
   internal capture wrote nothing. Alt+F5 (rendered screenshot) is delivered
   and, with `glshader=none`, produces a clean chrome-free framebuffer the
   loader decodes. (`_trigger_dosbox_internal_screenshot`,
   `_write_live_conf`.)
3. **Key delivery** — cliclick `kp:return` / `t:1` were silently dropped by
   DOSBox even with the window frontmost (the SELECTOR echoed the typed
   digit but never consumed the Return). AppleScript `key code N`
   (System Events) is the only keystroke path that actually reaches DOSBox,
   so SELECTOR navigation now uses osascript key codes per keystroke with a
   re-raise before each. (`OSASCRIPT_KEY_CODES`, `_osascript_key_code`,
   `_press_key`.)

SELECTOR text pages do not fit the 4-state dungeon density classifier
(graphics page ≈ `entrance_menu`, sound page ≈ `title_screen`), so those
steps are `settle_only`: send keys, dwell, capture, proceed. Only the
dungeon-crossing click and the final dungeon hold are classifier-gated.

## Non-claims

- This run is host-side capture proof that the original-DOS route reaches the
  dungeon viewport; it does NOT promote original-vs-Firestaff pixel parity.
- No game logic or rendering behaviour is changed.
- No proprietary game frames are committed; only hashes/densities.
- Forward movement inside the dungeon uses DM PC's on-screen mouse controls,
  not the arrow keys; the post-entry "step forward" capture is a known
  follow-up and is not claimed as movement parity here.
