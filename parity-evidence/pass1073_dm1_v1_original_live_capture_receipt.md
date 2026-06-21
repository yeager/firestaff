# Pass1073 - DM1 V1 original live-capture receipt

Status: PASS1073_DM1_V1_ORIGINAL_LIVE_CAPTURE_RECEIPT_LOCKED

This gate records a redacted receipt from the macOS DOSBox Staging live runner. The run reached DM1 PC 3.4 `dungeon_gameplay`, saved two operator-local frames, preserved the C070 mouse no-change diagnostic, and recorded a viewport-hash change after `Keypad-5`.

## Evidence

- Live launch mode: `staging_app_binary_dm_exe_path`
- Capture backend: `peekaboo`
- Runtime required files: `DUNGEON.DAT`, `GRAPHICS.DAT`
- Frame rows: `2`
- Start frame: `original/01_ingame_start.png`, 320x200, `dungeon_gameplay`
- Step frame: `original/02_ingame_step_forward.png`, 320x200, `dungeon_gameplay`
- Viewport hashes differ after `Keypad-5`: `307e2dd864df...` -> `5c73694e24af...`
- ReDMCSB anchors: `COMMAND.C:396-405` C070 diagnostic and `COMMAND.C:275-281` numeric keypad 5 movement binding.

## Decision

The live-capture harness is now proven beyond a dry run and locked by a repository CTest gate. The B1 rows stay `PARTIAL`: the frames are not tracked, there is no same-state Firestaff pairing, and there is no live I34E debugger row.

## Non-claims

- This is not original-vs-Firestaff pixel parity.
- The proprietary frames remain operator-local and are not committed.
- This is not an I34E debugger observation.
- This does not close any B1 row.

Manifest: `parity-evidence/verification/pass1073_dm1_v1_original_live_capture_receipt/manifest.json`
