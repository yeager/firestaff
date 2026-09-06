# Firestaff TODO — active cross-game work

- Extend authentic combat coverage to multi-creature selection, reach
  blocking and exact damage/XP comparisons against original emulator traces.
  The relocated-party original-group tests do not prove a natural play route.

- Complete CSB FM Towns action-menu pixel parity and all row boundaries.
  Source C696 now owns language-specific action/Pass pointer geometry;
  remaining Japanese glyph and visual-composition gaps are independent.
  Replace the generic HUD's PC-only material admission: original F31 C011
  is 14x39, not 14x26; Japanese C010 is 96x72 and C013 is 96x41.
  Extend the corrected C007 viewport origin to emulator pixel comparisons
  and authentic C080 edge/pickup/throw/sensor interaction sequences.
  Do not enable the whole generic HUD merely by accepting its asset sizes.

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
