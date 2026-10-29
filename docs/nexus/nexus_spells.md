# Nexus spell system evidence and implementation status

**Updated:** 2026-10-29

This page separates verified Nexus retail behavior from older Firestaff design
notes that reused DM1 spell data. DM1 references, enum names, formulas, and
fixture behavior do not establish Nexus retail semantics.

## Verified retail evidence

- The Japanese retail `DM.BIN` is SHA-256
  `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`.
  Its 32 big-endian halfwords at file offset `0x38368` match the table copied
  into the diagnostic implementation in `src/nexus/nexus_v1_magic.c`. This
  byte match verifies the table contents only; it does not establish the
  meaning of every index or spell effect.
- Static Ghidra analysis found that retail `FUN_06039f76` reads runtime
  champion-row offset `+0x18` while computing a threshold used by
  `FUN_0603a472`. The latter accepts or rejects the next rune in a four-entry
  sequence. The field's domain meaning and dynamic input path remain unknown.
  See `docs/nexus/nexus_sh2_startup_decompilation.md`.
- The Japanese retail manual documents Control Pad Y as opening the Magic
  window. A 2026-10-29 authentic Ymir `--debug` capture scheduled Y for one
  frame from a gameplay checkpoint loaded from the real BKR and showed the
  native Magic window. This verifies the entry action for that state. It does
  not verify rune selection, sequence acceptance, casting, or spell effects.
  See
  [`nexus_magic_window_y_debug_20261029`](../../parity-evidence/verification/nexus_magic_window_y_debug_20261029/README.md).
- A second authentic Ymir `--debug` probe replayed one Y-open profile state
  for 90 frames with no input and with one-frame Up, Down, Left, and Right
  inputs. Up was pixel-identical to control; Down, Left, and Right changed
  3,704, 3,740, and 3,664 pixels respectively. Their final-frame VDP1 command
  lists also differ (7, 9, and 5 rows respectively), while Up's VDP1 trace is
  identical to control. This confirms direction-sensitive retail output from
  that checkpoint, but does not identify rune symbols, sequence meaning, or
  casting. This probe's source savestate is documented separately and is not
  byte-identical to the state in the Y-entry receipt. Raw evidence is retained
  at `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028/magic-window-dpad-inputs-20261029/`.
- The frame-89 VDP1 snapshots also join nine command source starts to decoded
  entries 99–106 and 144 in hash-verified Japanese `MENU.BPK`. Down, Left, and
  Right reorder references to entries 99–106 at command indices 79–88; Up
  keeps the control command list. This binds those command source bytes to
  authentic Japanese MENU.BPK surfaces. All nine commands use VDP1 8bpp bank
  mode; for all 31 nonzero indices in those joined surfaces, the PALT word
  matches the same-run CRAM word. This does not establish rune names, surface
  meaning, full dispatch ownership, complete screen composition, or renderer
  permission. Reproducible palette-join receipts are in the external capture
  directory above.

## Firestaff production boundary

`src/nexus/nexus_v1_magic.c` is a source-study implementation. The production
`firestaff_nexus` target excludes it and links
`src/nexus/nexus_v1_magic_runtime_noop.c`, whose lookup, mana-cost, cast, and
damage operations fail closed. The diagnostic table, inferred cost formula,
skill thresholds, magnitude values, and effect examples in that study file
are not production behavior and are not yet established as retail behavior.

Firestaff does not yet open or render the Nexus Magic window from its desktop
input route. Its rune-selection state, input-to-rune mapping, retail sequence
acceptance, mana commit, target selection, effect dispatch, random inputs,
animation, and save-state changes remain unverified. Do not enable the study
implementation or substitute DM1 tables for these missing retail joins.

## Next evidence needed

Continue from a reproducible authentic Magic-window checkpoint. The first
directional replay is retained, but its exact source savestate differs from
the Y-entry receipt. Establish one checkpoint's provenance, repeat the
directional inputs while joining changed VDP1 commands to retail menu
surfaces, then trace rune acceptance and the `+0x18` consumer before assigning
field names or implementing the Firestaff route. Only then trace casting
through mana changes, target/effect state, random inputs, animation, and
visible output.

Raw emulator states, screenshots, logs, and memory traces remain outside Git
in `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028/nexus-magic-window-input-20261029/` and
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028/magic-window-dpad-inputs-20261029/`.
