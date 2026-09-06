# Firestaff TODO — active cross-game work

- Extend fresh-game G0236 pool admission to the separately authenticated
  Atari/Amiga/FM Towns paths and CSB. Audit runtime capacities against source
  pool/event counts (including 50 fresh C15 slots), pool recycling and all
  resource-exhaustion behavior. Save/import expansion must remain disabled.
- Extend ordinary C15 runtime captures to all visible lanes, explosion
  families and original-emulator timing/pixels. Five D1C spells in three modes
  does not prove complete explosions or kinetic-energy scaling parity.
  Verify raw C006 unlink timing and RNG against an original emulator trace.

- Extend the complete projectile-orientation query to CSB's live material
  owner with original edition-specific weapon aspects and view lanes;
  its legacy sprite query still uses subtype and direction-only bitmap
  selection. Verify all authentic projectile-art families, side/deep lanes,
  both map parities and kinetic-energy scaling through full runtime captures.

- Extend DM1 thrown-object capture coverage to the separate Hall of Champions
  renderer, all visible flight cells and natural-play emulator comparisons.
  The DOS PC34 D0C real-media regression uses a RAM-configured party/camera
  and mastery threshold; it does not establish complete flight/render parity.

- Extend the [DM1 world XP transaction](docs/parity/DM1_ORIGINAL_XP_TRANSACTION.md)
  evidence to original-emulator level crossings for melee, parry, sensors,
  throw and actions; finish original recently-upgraded flags and localized
  message-area timing. Audit XP saturation and maximum-stat edge semantics.
- Bind edition-specific level-up antimagic behavior for remaining early
  DM1 editions (modulo 3 versus PC34/late-Amiga two bits) and compare complete
  level transitions against original emulator traces, including source UI.

- Extend authentic combat coverage to multi-creature selection, reach
  blocking and exact damage/XP comparisons against original emulator traces.
  The relocated-party original-group tests do not prove a natural play route.

- Complete CSB's source-owned cast execution. M11 currently preserves the
  rune line and reports the unavailable CSB cast owner rather than executing
  DM1 effects against CSB state. Spell-panel raster/input verification does
  not close this gap. Verify Japanese caster names through their authentic
  whole-string glyph path and compare spell-panel timing with an emulator.
  Follow [the original cast contract](docs/parity/CSB_ORIGINAL_CAST_CONTRACT.md)
  for edition tables and complete effect/XP/timer transactions; CSBWin-only
  parser or abort-path tests are not original-game casting evidence.
  Connect F31's admitted 29-record table to the complete transaction and
  admit Atari/Amiga tables.
  Implement original F0304 practice/level-up mutation and bind resting state
  end-to-end; verify Firestaff mastery bonuses with authentic object evidence.
  Bind G0361 at the live creature-attack boundary and preserve original
  level-up RNG ordering, recently-upgraded flags and localized messages.

- Complete CSB FM Towns action-menu pixel parity and all row boundaries.
  Source C696 now owns language-specific action/Pass pointer geometry;
  remaining Japanese glyph and visual-composition gaps are independent.
  Replace the generic HUD's PC-only material admission: original F31 C011
  is 14x39, not 14x26; Japanese C010 is 96x72 and C013 is 96x41.
  Extend the corrected C007 viewport origin to emulator pixel comparisons
  and authentic C080 edge/pickup/throw/sensor interaction sequences.
  Do not enable the whole generic HUD merely by accepting its asset sizes.
  Verify all idle-cell click edges/gaps and source cooldown transitions.
  Verify empty-hand
  cooldown expiry against original emulator pixels, beyond predicate tests.
  Extend source C013 movement-control verification beyond closed-inventory
  panel pixels and rotation to traversal, exact edges/gaps and inventory mode.

- Complete Japanese FM Towns active-menu text and remaining row boundaries.
  Reconcile the old JDM mixed-font adapter with TEXT2.C:75–105: the original
  chooses the Japanese path for a whole string, then F0952 uses 8x16 or 16x16
  system glyphs per unit. Its ASCII/M653-per-byte mixture is not equivalent.
  JAPANESE.C:242–269 calls EGB_sjisString; do not add a runtime BIOS dependency.
  Extend full C010 panel tests to Japanese labels; English one/two/three
  action panels are covered using existing dungeon weapon records.
  Retire obsolete solid-fill shim/tests and misleading colour-selector
  names; destination region IDs11/77/79 are not palette values.

- Extend DM1 FM Towns Japanese movement pointer coverage to traversal and
  exact boundaries, and action-cell coverage to outside edges. Verify
  hatching against the original JDM registry. Verify Japanese message
  wrapping and glyph metrics within its224x33 container. The unsupported
  Japanese menu text adapter is no longer called by the live renderer.
  Extend Japanese spell input verification to casting and outside-parent
  boundaries; verify Japanese text separately from the ASCII/rune oracle.

- Complete DM1 inventory-owner regression for Modern composed-HUD input
  and remaining consumers; the source-layout owner/leader separation is
  implemented and covered by original-media tests.

- Verify DM1 death/resurrection leader ownership with original-media
  runtime sequences, beyond isolated leader-selection admission checks.

- Extend DM1 carried-load verification to Modern composed-HUD leader switching, cross-champion
  exchanges and open-chest mutations; see TODO-dm1.md for remaining scope.

- Compare Atari CSB's full chest composition with a same-state original
  runtime capture; source-material composition tests are not emulator parity.

- Extend Atari CSB's original-object inventory corpus to equipment/chest
  drag destinations and native chest-panel interactions.

- Extend original-media chest interaction verification to CSB Atari and
  Amiga, including panel geometry, same-owner refresh and owner switching;
  successful startup alone does not establish inventory interaction parity.

Reviewed 2026-08-25. This file contains only work that is still open. Game
details and acceptance evidence belong in `TODO-<game>.md`; completed work is
recorded in `DONE-<game>.md`. Historical mixed logs are retained as
`HISTORY-archived-2026-08-08.md` and in Git history, not as active work.

- Bind all production paths to user-supplied original media under
  `~/.firestaff/data/<game>`; do not substitute generated gameplay assets.
- Keep every platform claim at the strongest real-media evidence level in
  `docs/PLATFORM_STATUS.md`; a parser or fixture is never a playable-route
  claim.
- For each open behavior: obtain original-media evidence, bind it to an
  original consumer, add a native regression, then record the result in DONE.
- Keep external emulators and disassemblers development-only. Firestaff must
  run the games natively and may not depend on them at runtime.

Current authoritative status: `docs/PROJECT_STATUS.md`,
`docs/PLATFORM_STATUS.md`, and `docs/MISSING_FUNCTIONS_BY_GAME.md`.
