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
- A follow-up used the same hash-identified Y-open state with GUI presentation
  enabled and traced SH-2 reads/writes around the champion rows. Holding Down
  for 90 frames changed 3,704 screenshot pixels and VDP1 rows 78–82 and 87–88
  relative to no input. The one-frame A screenshot differs by 4,632 pixels;
  its trace ends at frame 88 versus control's frame 89, so that count includes
  a one-frame endpoint offset. At common frame 88, A changes VDP1 rows 25,
  79–84, and 88. These captures show a rendered response to both inputs, but
  do not label the ring sprites or prove a selected rune or accepted cast.
  All three traces read `FUN_0603a472` and the row `+0x35` bytes through
  `FUN_0603a55c`; none writes row offsets `+0x35..+0x38`. The corresponding
  `wram-hi.bin` snapshots remain `02 00 78 00` for rows 0 and 1 and zero for
  rows 2 and 3. The A capture ends at frame 88 with a complete trace and no
  dropped records. Full data and hashes are retained at
  `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-magic-down-input-trace-20261029/`.
- Static Ghidra analysis of the hash-verified Japanese `DM.BIN` finds the
  `MENU.BPK` resource string in `FUN_060286a4`, called indirectly by
  `FUN_06010f44`. Generic draw helpers include `FUN_06029188` and
  `FUN_060292e0`; the analysis does not connect these or decoded entries
  99–106/144 to the observed Magic-window command block or D-pad state.
  `FUN_0603a472` validates a four-entry sequence and is called by mutators
  `FUN_0603a55c` and `FUN_0603a602`, which update row offsets `+0x35`, `+0x36`,
  and `+0x38`. No direct relation to the captured commands or menu surfaces is
  established. The bounded raw-byte scan found no direct table of the matching
  entry IDs; it cannot rule out indirect encodings. See the [static audit
  report](</Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028/magic-window-menu-bpk-static-semantic-audit-20261029/README.md>).

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

The directional probe has a hash-identified Y-open source state reused across
the same-state runs; that save remains distinct from the separately retained
Y-entry receipt. Nine VDP1 command sources and their 31 active palette indices
are bound to authentic `MENU.BPK` surfaces, PALT, and same-run CRAM. The
follow-up confirms that Down and A alter the visible ring while the candidate
sequence bytes remain unchanged, even as the retail validator and sequence
helper are read. Next, trace the input dispatch and the producer of the changed
VDP1 ring commands to distinguish cursor movement from selection and commit.
Then follow a confirmed commit through sequence writes and validator result,
and continue casting through mana changes, target/effect state, random inputs,
animation, and visible output. Do not assign rune field names or implement the
Firestaff route from sprite appearance alone. The `+0x18` consumer remains a
separate work item.

Raw emulator states, screenshots, logs, and memory traces remain outside Git
in `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028/nexus-magic-window-input-20261029/`,
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028/magic-window-dpad-inputs-20261029/`, and
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-magic-down-input-trace-20261029/`.
