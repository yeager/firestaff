# Pass 70 follow-up — original DM1 route/crop blocker probe

Date: 2026-04-26

## Goal

Try to produce original DM1 PC 3.4 `224x136` viewport reference crops for the same six-state route as Firestaff's deterministic capture, or reduce the blocker with exact evidence.

## Inventory

In this worktree (`/Users/bosse/.openclaw/workspace-main/tmp/firestaff-parallel-original-route-crops`, commit `2ee8100`):

- `original-games/` is absent.
- `verification-screens/dm1-dosbox-capture/` is absent.
- `scripts/dosbox_dm1_capture.sh` exists, but cannot stage because the tracked archive is absent:
  - expected: `original-games/Game,Dungeon_Master,DOS,Software.7z`
  - observed exit: `ERROR: DM1 archive missing: .../original-games/Game,Dungeon_Master,DOS,Software.7z`
- Existing relevant evidence:
  - `parity-evidence/pass62_v1_title_dosbox_capture_blocker.md`
  - `parity-evidence/pass63_v1_title_dosbox_input_automation.md`
  - `parity-evidence/pass67_v1_native_raw_title_capture.md`
  - `parity-evidence/pass70_original_dm1_viewport_reference_pipeline.md`

Workspace-wide search found only partial ReDMCSB original reference trees, not the expected `original-games/` archive:

```text
/Users/bosse/.openclaw/workspace-main/ReDMCSB_WIP20210206/Reference/Original/I34E/DM.EXE
/Users/bosse/.openclaw/workspace-main/ReDMCSB_WIP20210206/Reference/Original/I34M/DM.EXE
```

The I34E tree contains small readonly original files (`DM.EXE`, `VGA`, `IBMIO`, `FIRES`, etc.) but no `DUNGEON.DAT` / full staged `DungeonMasterPC34` tree.

## Script changes

Updated `scripts/dosbox_dm1_original_viewport_reference_capture.sh` to make the blocker easier to eliminate without fabricating evidence:

1. Added `DM1_ORIGINAL_STAGE_DIR=/path/to/tree` override so an existing full DM1 PC 3.4 tree can be used without copying/moving large directories.
2. Added a Pillow fallback for normalization when ImageMagick (`magick`/`convert`) is unavailable. This host has Python Pillow 11.3.0, but no `magick`/`convert` on PATH.
3. Added route-shape validation before `--run`:
   - rejects unknown route tokens,
   - requires exactly six `shot` tokens.

This is only syntactic validation. It does **not** claim the route is semantically equivalent to Firestaff's state.

## Commands and results

Syntax check:

```sh
bash -n scripts/dosbox_dm1_original_viewport_reference_capture.sh
```

Result: OK.

Synthetic normalization gate for the new Pillow fallback (scratch `/tmp`, not reference evidence):

```sh
OUT_DIR=/tmp/firestaff-normalize-synthetic \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --normalize-only
```

Result: six synthetic `320x200` PNGs normalized to six `224x136` PPM crops and a manifest with all crop dimensions `224x136`.

Default dry-run, with no staged tree and no route:

```sh
scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run
```

Result:

```text
[blocked] staged DM1 tree missing: /Users/bosse/.openclaw/workspace-main/tmp/firestaff-parallel-original-route-crops/verification-screens/dm1-dosbox-capture/DungeonMasterPC34
          next: scripts/dosbox_dm1_capture.sh
[blocked] DM1_ORIGINAL_ROUTE_EVENTS is not set. Do not guess; validate the exact original keystroke route first.
```

Stage attempt:

```sh
scripts/dosbox_dm1_capture.sh
```

Result:

```text
ERROR: DM1 archive missing: /Users/bosse/.openclaw/workspace-main/tmp/firestaff-parallel-original-route-crops/original-games/Game,Dungeon_Master,DOS,Software.7z
```

Dry-run against the partial ReDMCSB I34E tree to validate the override and route-shape gate:

```sh
DM1_ORIGINAL_STAGE_DIR=/Users/bosse/.openclaw/workspace-main/ReDMCSB_WIP20210206/Reference/Original/I34E \
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 shot right wait:300 shot up wait:300 shot wait:300 shot wait:300 shot wait:300 shot' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run
```

Result:

```text
[pass-70] route shape OK: 16 tokens, 6 shots
```

Probe run against the same partial I34E tree (scratch output only under `/tmp/firestaff-original-route-probe`, not promoted):

```sh
OUT_DIR=/tmp/firestaff-original-route-probe \
DM1_ORIGINAL_STAGE_DIR=/Users/bosse/.openclaw/workspace-main/ReDMCSB_WIP20210206/Reference/Original/I34E \
DM1_ORIGINAL_ROUTE_EVENTS='wait:9000 enter wait:2000 shot right wait:500 shot up wait:700 shot wait:500 shot wait:500 shot wait:500 shot' \
WAIT_BEFORE_INPUT_MS=2500 \
NEW_FILE_TIMEOUT_MS=4000 \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Result:

```text
[pass-70] route shape OK: 16 tokens, 6 shots
ERROR: expected raw screenshot 320x200, got 720x400 for /tmp/firestaff-original-route-probe/image0001-raw.png
```

DOSBox log evidence from that scratch run:

```text
DISPLAY: VGA 320x200 256-colour graphics mode 13h ...
DISPLAY: VGA 720x400 16-colour text mode 03h ...
CAPTURE: Capturing raw image to '/tmp/firestaff-original-route-probe/image0001-raw.png'
```

Visual inspection of `image0001-raw.png` shows a DOS text prompt (`C:\>`), not the Dungeon Master game viewport. Therefore the partial ReDMCSB I34E tree is **not** a valid substitute for the missing full staged original game for this route/crop task.

## Route status

`DM1_ORIGINAL_ROUTE_EVENTS` is still **not validated**.

The Firestaff route to mirror is known from `verification-screens/capture_firestaff_ingame_series.c`:

1. initial game view after `M11_GameView_OpenSelectedMenuEntry`,
2. turn right,
3. move forward,
4. spell rune 1 panel,
5. spell cast,
6. clear spell panel + inventory toggle.

The exact original DM1 PC 3.4 keystroke/mouse route is still blocked because the full runnable original game tree is missing. Keyboard route shape alone is insufficient; spell/inventory states may require original UI mouse coordinates and must be validated visually/raw against the actual original runtime.

## Crop status

No original viewport crops were committed.

Reason: raw screenshots from the only available substitute tree were `720x400` DOS prompt frames, not `320x200` in-game DM1 framebuffer frames. Promoting crops from those would create false reference evidence.

## Exact remaining blocker

Supply or restore a full original DM1 PC 3.4 staged tree/archive in one of these forms:

```text
verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DM.EXE
```

or:

```text
original-games/Game,Dungeon_Master,DOS,Software.7z
```

or run with:

```sh
DM1_ORIGINAL_STAGE_DIR=/path/to/full/DungeonMasterPC34
```

Then validate a six-shot `DM1_ORIGINAL_ROUTE_EVENTS` against the real original runtime before running:

```sh
DM1_ORIGINAL_ROUTE_EVENTS='<validated route with exactly six shot tokens>' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```
