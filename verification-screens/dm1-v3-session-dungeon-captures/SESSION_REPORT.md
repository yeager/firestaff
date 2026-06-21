# DM1 V1 Original DOSBox Capture Session — 2026-06-21 19:32 GMT+2

Status: **dungeon_gameplay + inventory frames captured; creature-chain still requires a longer route**

This directory contains the artifacts from an attempt to close the
DM1 V1 creature-chain original-capture gap by running the
`scripts/dm1_v1_original_capture.py --pair all` harness in this
session against the canonical DM1 PC 3.4 English runtime staged at
`/Users/bosse/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34`.

## Selector-to-dungeon hand-off (verified)

The harness's selector sequence (`GRAPHICS=1` VGA → `SOUND=1` No Sound
→ `CONTROL=4` Keyboard Sim Digital Joystick → `Return` on entrance
wall → wait for FIRES window title) consistently reached the dungeon
during this session. The selector sequence is the same one the runbook
documents and that `scripts/dosbox_dm1_original_viewport_reference_capture.sh`
used for the pass1052 and pass1055 captures.

## pass80 classifier verdicts

| Pair | Captures | dungeon_gameplay | inventory | non_graphics_blocker |
|---|---|---|---|---|
| `01_viewport` | 3 (320x200 + 1024x800 + 224x136 variants) | **2** | 1 | 0 |
| `02_wall` | 3 | 0 | 3 | 0 |
| `03_collision` | 5 | 0 | 5 | 0 |
| `04_creature` | 7 | 0 | 7 | 0 |
| `05_champion` | 2 | 1 | 1 | 0 |

`01_viewport/image0001-raw.png` and `01_viewport/image0002-raw.png`
are **real dungeon_gameplay** captures (selector → entrance wall →
forward step into the dungeon corridor). pass80 classifier reason
"viewport content with mostly dark in-game right column".

## What we did NOT capture

A pair of frames where a Trolin creature is in D2C / D1C. The
04_creature route (7 forward steps south from the entrance hall)
hits the inventory panel at step 1 because the entrance area in DM1
PC 3.4 has the inventory overlay pop up the first time the party
moves. To reach a creature in D2C / D1C the next live session must
extend the route past the inventory interaction (close the inventory
by pressing `Escape`/`kp2`, then navigate through the closed door
in the entrance hall via the runbook's host-mouse click + kp4/kp6
corrected keypad sequence documented in
`docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`).

## Captures not committed but preserved on disk

The full set of captures (320x200 + 224x136 viewport crops + 1024x800
DOSBox window crops for all 5 pairs) is at:

- `/tmp/dm1_v3_capture/01_viewport/`
- `/tmp/dm1_v3_capture/02_wall/`
- `/tmp/dm1_v3_capture/03_collision/`
- `/tmp/dm1_v3_capture/04_creature/`
- `/tmp/dm1_v3_capture/05_champion/`

Each contains:
- `<label>.png` (320x200 canonical DM1 framebuffer)
- `<label>_viewport.png` (224x136 dungeon-viewport crop)
- `<label>_dosbox_window.png` (1024x800 DOSBox window content)
- `pass80_original_frame_classifier.{json,md}` (classifier audit)

## Honest non-claims

- This session did **not** capture the creature-chain pixel-pair
  (Trolin in D2C / D1C) that the
  `docs/parity/DM1_V1_CREATURE_CHAIN_ORIGINAL_CAPTURE_CONTRACT.json`
  contract requires. The route extension required to reach those
  squares is out of scope for a single 90-second capture run.
- The dungeon_gameplay / inventory / wall-blocked captures here are
  Firestaff-side verification material only. They are not promoted
  to the contract's `creature_chain_d2c_trolin_front` /
  `creature_chain_d1c_trolin_front` rows.
- The `verify_dm1_v1_creature_chain_original_capture_gate.py`
  status remains `BLOCKED_DM1_V1_CREATURE_CHAIN_ORIGINAL_CAPTURE_GATE_LOCKED`
  because the contract requires creature-in-viewport captures that
  this session did not produce.

## File listing

- `01_viewport/01_viewport_start.{png,_viewport.png,_dosbox_window.png}` —
  entrance wall, sha256 `8f62254638b4…`, classified dungeon_gameplay.
- `01_viewport/01_viewport_after_step.{png,_viewport.png,_dosbox_window.png}` —
  after KP5, sha256 `0e6eeb39bbbb…`, classified dungeon_gameplay.
- `01_viewport/01_viewport_after_turn.{png,_viewport.png,_dosbox_window.png}` —
  after KP6, sha256 `8c43b5795765…`, classified inventory (right-column
  HUD lit by the turn).

The 02_wall / 03_collision / 04_creature / 05_champion captures
share SHA `9164394fd354…` (party stuck against entrance-area wall)
or `0e6eeb39bbbb…` (back to dungeon corridor), neither of which
contains a creature in viewport.
