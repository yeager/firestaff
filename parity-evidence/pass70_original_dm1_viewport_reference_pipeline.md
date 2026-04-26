# Pass 70 — original DM1 viewport reference-capture pipeline

Date: 2026-04-26

## Scope

Build a concrete, runnable path for producing **original DM1 PC 3.4** `224x136` viewport crops that can be compared against Firestaff's deterministic `run_capture_screenshots.sh` route.

This pass does **not** claim original pixel parity. It adds the missing automation bridge and records the remaining hard blockers instead of fabricating reference crops.

## Inventory findings

Relevant existing in-tree material:

- `run_capture_screenshots.sh`
  - Generates 6 deterministic Firestaff full-frame `320x200` PPMs.
  - Generates 6 Firestaff viewport crops at the DM1 source aperture `(0,33,224,136)`.
  - Writes `verification-screens/capture_manifest_sha256.tsv`.
- `verification-screens/capture_firestaff_ingame_series.c`
  - Firestaff route is:
    1. initial DM1 game view after `M11_GameView_OpenSelectedMenuEntry`,
    2. turn right,
    3. move forward,
    4. spell rune 1 panel,
    5. spell cast,
    6. close spell panel + inventory toggle.
  - The capture harness injects a deterministic synthetic champion/ready-hand dagger for right-panel fixture stability. The viewport crop ignores most of that HUD work, but the original whole-frame state is therefore **not** equivalent yet.
- `scripts/dosbox_dm1_capture.sh`
  - Stages original DM1 PC 3.4 from `original-games/Game,Dungeon_Master,DOS,Software.7z` into `verification-screens/dm1-dosbox-capture/DungeonMasterPC34`.
  - Writes deterministic DOSBox config.
  - Current worktree does **not** have `original-games/` nor a staged `DungeonMasterPC34` tree.
- Pass 62/63/67 evidence:
  - DOS redirection is not a clean selector path.
  - Host/CGEvent selector input works: original setup choices are `1`/Return repeated 3 times.
  - DOSBox Staging native raw screenshots work and produce raw `320x200` PNGs via the Cmd+F5 mapper when `default_image_capture_formats=raw` is set.

## New tool

Added:

```sh
scripts/dosbox_dm1_original_viewport_reference_capture.sh
```

Modes:

```sh
scripts/dosbox_dm1_original_viewport_reference_capture.sh --prepare
scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
scripts/dosbox_dm1_original_viewport_reference_capture.sh --normalize-only
```

What it automates:

1. Uses the existing staged original DM1 tree at `verification-screens/dm1-dosbox-capture/DungeonMasterPC34`.
2. Writes a DOSBox Staging config with:
   - `machine=svga_paradise`,
   - `core=normal`,
   - `cpu_cycles=3000`,
   - `default_image_capture_formats=raw`,
   - capture dir under `verification-screens/pass70-original-dm1-viewports/`,
   - `DM VGA` in autoexec.
3. Writes a Swift CGEvent helper that:
   - sends the known original selector sequence (`1`, Return) x3,
   - executes an explicit caller-supplied `DM1_ORIGINAL_ROUTE_EVENTS` token route,
   - triggers raw DOSBox screenshots with Cmd+F5 on each `shot` or `shot:<label>` token,
   - records optional route labels in `original_viewport_shot_labels.tsv` so semantic checkpoints such as `party_hud`, `spell_panel`, and `inventory_panel` are visible to overlay-readiness tooling without renaming legacy crop files.
4. Normalizes the first six `image*.png` raw screenshots into:
   - `verification-screens/pass70-original-dm1-viewports/viewport_224x136/*.ppm`,
   - matching PNG previews when ImageMagick is available,
   - `verification-screens/pass70-original-dm1-viewports/original_viewport_224x136_manifest.tsv`,
   - `verification-screens/pass70-original-dm1-viewports/original_viewport_shot_labels.tsv` when a route is supplied.
5. Fails hard if crop geometry is not exactly `224x136` or if exactly six crops are not present.

Output artifacts are intentionally ignored; original/raw captures must not be accidentally committed.

## Current blocker

The pipeline is runnable, but **reference generation is blocked** in this worktree by two missing inputs:

1. Missing original game staging:

```text
verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DM.EXE
```

The checked worktree also has no `original-games/` directory, so `scripts/dosbox_dm1_capture.sh` cannot stage the archive here until `original-games/Game,Dungeon_Master,DOS,Software.7z` is supplied.

2. Missing validated original route events:

```text
DM1_ORIGINAL_ROUTE_EVENTS
```

The script deliberately refuses to guess these. The exact keystroke sequence must be validated in original DM1 PC 3.4 so the six raw screenshots correspond to the same route/state as Firestaff's route above. A shape-only example is:

```sh
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 shot:party_hud right wait:300 shot up wait:300 shot:spell_panel wait:300 shot wait:300 shot:inventory_panel'
```

That example is **not validated evidence**. The labels are route metadata only; they do not prove the original runtime reached those semantic states and must not be cited as pixel parity.

## Verification performed

```sh
bash -n scripts/dosbox_dm1_original_viewport_reference_capture.sh
scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run
```

Observed dry-run blockers:

```text
[blocked] staged DM1 tree missing: .../verification-screens/dm1-dosbox-capture/DungeonMasterPC34
[blocked] DM1_ORIGINAL_ROUTE_EVENTS is not set. Do not guess; validate the exact original keystroke route first.
```

## Exact next commands

First stage original DM1 PC 3.4:

```sh
scripts/dosbox_dm1_capture.sh
```

Then run the original reference pipeline after validating the original route string:

```sh
DM1_ORIGINAL_ROUTE_EVENTS='<validated route with exactly six shot or shot:<label> tokens>' \
  scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

If raw `image*.png` screenshots already exist under `verification-screens/pass70-original-dm1-viewports/`, normalize only:

```sh
scripts/dosbox_dm1_original_viewport_reference_capture.sh --normalize-only
```

Then generate a first viewport overlay/diff against Firestaff crop 01:

```sh
python3 tools/redmcsb_overlay_diff.py \
  --firestaff verification-screens/01_ingame_start_latest_viewport_224x136.png \
  --reference verification-screens/pass70-original-dm1-viewports/viewport_224x136/01_ingame_start_original_viewport_224x136.png \
  --region-xywh 0,0,224,136 \
  --out parity-evidence/overlays/pass70/01_ingame_start_viewport_original_vs_firestaff
```

Run the same command for rows `02..06` once the original manifest exists.
