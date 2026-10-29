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

Continue from the retained authentic gameplay checkpoint in the receipt above.
Capture Y to open Magic, then vary one Control Pad input at a time while
recording the on-screen result and bounded SH-2 reads/writes. Join the accepted
rune sequence to its retail state updates and the `+0x18` consumer before
assigning field names or implementing the Firestaff route. Only then trace the
retail cast path through mana changes, target/effect state, and visible output.

Raw emulator states, screenshots, logs, and memory traces remain outside Git
under `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028/nexus-magic-window-input-20261029/`.
