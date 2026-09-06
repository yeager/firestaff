# Firestaff TODO — active cross-game work

- Resolve DM1 FM Towns Japanese action cells C089–C092, icon anchors
  C093–C096 and hatching from the original JDM registry. The idle clear
  correction alone does not fix these PC-coordinate consumers. The active
  Japanese menu also still calls the English-only backdrop resolver.
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
