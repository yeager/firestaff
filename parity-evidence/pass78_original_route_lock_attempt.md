# Pass 78 — original DM1 route-lock attempt

Date: 2026-04-26

## Goal

Target the original DOS runtime to the now-locked Firestaff fixture route from pass 77:

```text
start → turn right → move west → spell panel → actual Ful Ir cast → inventory
```

This pass is deliberately blocker-safe: it refuses to normalize text-mode or menu-prompt captures as gameplay viewport references.

## Script hardening

Updated `scripts/dosbox_dm1_original_viewport_reference_capture.sh`:

- added `DM1_ORIGINAL_PROGRAM` override for staged trees where `DM VGA` stays in selector/text mode;
- kept the explicit `DM1_ORIGINAL_ROUTE_EVENTS` requirement;
- route shape still requires exactly six `shot` tokens.

Added `tools/pass78_original_route_attempt_audit.py`:

- audits raw DOSBox screenshots before normalization;
- accepts only `320×200` raw graphics frames as gameplay candidates;
- classifies `720×400` text-mode selector/prompt captures as blockers.

## Attempts

### Attempt A — keypad selectors through `DM VGA`

```sh
OUT_DIR=$PWD/verification-screens/pass78-original-route-target3 \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:3000 kp1 enter wait:800 kp1 enter wait:800 kp1 enter wait:10000 enter wait:18000 shot right wait:1200 shot up wait:1200 shot kp1 wait:800 shot kp4 wait:200 kp4 wait:200 enter wait:1500 shot i wait:1200 shot' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Audit:

```text
captures=6, dimensions_seen={"720x400": 6}, all_gameplay_320x200=false
```

Evidence: `parity-evidence/overlays/pass78/pass78_selector_blocker_target3.json`

Result: **blocked** — still text-mode selector prompt, not gameplay.

### Attempt B — letter selectors through `DM VGA`

```sh
OUT_DIR=$PWD/verification-screens/pass78-original-route-v-key \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:3000 v enter wait:1000 a enter wait:1000 m enter wait:15000 shot enter wait:15000 shot right wait:1000 shot up wait:1000 shot i wait:1000 shot esc wait:1000 shot' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Audit:

```text
captures=6, dimensions_seen={"720x400": 6}, all_gameplay_320x200=false
```

Evidence: `parity-evidence/overlays/pass78/pass78_selector_blocker_v_key.json`

Result: **blocked** — still text-mode selector prompt, not gameplay.

### Attempt C — direct `VGA` program override

```sh
OUT_DIR=$PWD/verification-screens/pass78-original-route-vga \
DM1_ORIGINAL_PROGRAM=VGA \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:10000 enter wait:18000 shot right wait:1200 shot up wait:1200 shot one wait:800 shot four wait:200 four wait:200 enter wait:1500 shot i wait:1200 shot' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Audit:

```text
captures=6, dimensions_seen={"720x400": 6}, all_gameplay_320x200=false
```

Evidence: `parity-evidence/overlays/pass78/pass78_program_vga_prompt_blocker.json`

Result: **blocked** — DOS prompt reports `Illegal command: VGA`; the extensionless staged `VGA` file cannot be launched as a DOS command in this path.

### Attempt D — temporary `VGA.EXE` staging copy

```sh
cp -R verification-screens/dm1-dosbox-capture/DungeonMasterPC34 verification-screens/pass78-stage-vga-exe
cp verification-screens/pass78-stage-vga-exe/VGA verification-screens/pass78-stage-vga-exe/VGA.EXE
OUT_DIR=$PWD/verification-screens/pass78-original-route-vga-exe \
DM1_ORIGINAL_STAGE_DIR=$PWD/verification-screens/pass78-stage-vga-exe \
DM1_ORIGINAL_PROGRAM=VGA \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:10000 enter wait:18000 shot right wait:1200 shot up wait:1200 shot one wait:800 shot four wait:200 four wait:200 enter wait:1500 shot i wait:1200 shot' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Audit:

```text
captures=6, dimensions_seen={"720x400": 6}, all_gameplay_320x200=false
```

Evidence: `parity-evidence/overlays/pass78/pass78_program_vga_exe_prompt_blocker.json`

Result: **blocked** — even with a temporary `VGA.EXE` copy, the automation remained at the DOS prompt/text mode. No 320×200 gameplay frames were produced.

## Current blocker

The original route cannot yet be locked because the staged original runtime remains in text-mode selector/prompt state under automation. We have not produced valid original gameplay frames for the pass-77 Firestaff route.

This is useful negative evidence: pass 78 prevents polluted original references by proving the current attempts are `720×400` text-mode frames, not `320×200` DM gameplay frames.

## Next viable routes

1. Identify the exact original launcher invocation / selector input path that reaches 320×200 gameplay from this staged PC 3.4 tree.
2. Investigate why the staged `VGA`/`VGA.EXE` launcher returns to prompt under DOSBox automation despite an MZ/LZEXE header; likely needs the selector/loader environment created by `DM.EXE` or a different working-directory/argument path.
3. Once raw screenshots are `320×200`, rerun normalization and pass74 comparison against the pass-77 Firestaff fixture route.
