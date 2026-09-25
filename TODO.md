# Firestaff TODO — active cross-game work

- Extend real-media start-menu launch coverage for DM1, CSB, DM2, Nexus and
  Theron's Quest. `m11_direct_launch_prepare_all_games` now drives the game-card,
  verified-platform and custom-options screens with installed original media,
  checks that the launch intent retains the authenticated edition selected on
  the platform card, and reaches M11 through that intent. It still needs to
  advance each remaining game from this same menu handoff to its first
  source-owned runtime frame; the separate boot-probe cases do not prove that
  menu path. Theron's authentic Japanese edition now reaches a source-owned
  runtime receipt through the same M12-selected M11 menu handoff; verify that
  path through its first presented runtime frame.
  Authentic DM1 PC 3.4 and CSB Amiga A31M now have this runtime-frame
  assertion. The CSB route waits through TITL.DAT and uses the native AppB
  language choice. Authentic DM1 Atari ST v1.2 and DM2 DOS English now also
  follow normal M12 selection and source-owned startup input to a presented
  first runtime frame; authentic DM2 Amiga now does the same through its
  original SWSH/TITL and GDAT New Game pointer route. DM2 French DOS now also
  reaches its authenticated initial party through the normal M12 → MVE →
  New-Game route, with direct-probe movement checked separately. The optional
  authentic CSB Atari ST route now completes ordinary M12 selection and
  ANIMATE.SCR/FTLCODE startup through the source-owned entrance view. Reaching
  gameplay from that Atari entrance remains unverified; the authentic C200
  primary-mouse command must be tested through the normal CLI route. Its
  initial state has zero champions, so this does not prove a playable campaign
  party. The optional
  French DOS original-save regression's M12 leg now uses normal Quick Resume
  and a runtime receipt (not the rejected `--menu --boot-probe` pair), but this
  local checkout lacks the authentic unpacked French EUDATA needed to execute
  that route. German Atari ST 1.2 and French Atari ST 1.3 also reach DM1 runtime
  from M12 with no selected champions. German Atari ST 1.2 now has verified
  direct CLI movement to the adjacent tile for C127 ordinal 14. A direct M11
  API harness selects it, but the CLI Enter route does not yet invoke selection;
  verify player-input recruitment through CLI and M12, then repeat on French
  Atari media.
  Atari now uses a distinct STARTUP1.C/F0437/F0441 media receipt, skips PC
  SWSH and PC34 special palettes, and routes entrance input through Atari's
  source mouse command. Authentic English, German and French Atari ST M12
  regressions now require the applied startup handoff and HoC first-frame
  receipt before accepting the live runtime state. Source review of ReDMCSB
  STARTUP1.C:160-173 shows the Atari path continues after F0441 through the
  F0435 load loop and F0462_StartGame before runtime. Firestaff's M12 handoff
  still begins with zero champions. The direct CLI route reaches C127, and a
  direct M11 API harness recruits from authentic Hall data; player-input
  recruitment still needs proof. `DUNGEON.FTL` is
  only an optional custom-dungeon path in ReDMCSB LOADSAVE.C; its absence from
  clean retail STX disks does not block built-in new-game startup. Verify the
  source-owned F0435/F0462 transition and recruitment through M12. The Atari
  receipt covers title/entrance; its F0437 presentation/palette remains open.
  Headless receipts do not claim host capture, and visual parity remains
  deferred.
  Other DM1 editions, remaining DM2 platforms, Nexus
  and Theron still need equivalent evidence. Keep this separate from visual
  parity.

- Extend F0219 current-cell collision coverage to original-media encounters,
  half-square creature footprints, nonmaterial/Black Flame cases and inactive
  group destinations. The source-cell tests cover centered and quarter-cell
  admission plus party priority; destination admission still uses active AI.

- Extend F0213 initial-burst ownership to CSB/legacy ownerless creation
  consumers; their previous delayed burst remains isolated from the fixed
  source-bound DM1 transaction. Verify packed source/target MapXCombo damage
  coordinates, group kill cleanup and original emulator RNG/damage traces.
- Extend source C14 retirement verification to original-media GROUP.Slot
  transfers, thrown-potion consumption, projectile collisions and allocation failures;
  audit all remaining host unlink paths against F0164. Verify complete
  original-emulator flight timing beyond the authentic DOS regression lane.

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

- Complete CSB's source-owned cast execution. For authenticated FM Towns
  media, M11 now performs F0409's G0487 lookup and F0408's meaningless-spell
  clear without touching RNG, effects, XP or the timeline; a valid source
  spell remains fail-closed rather than executing DM1 effects against CSB
  state. Spell-panel raster/input verification does not close this gap.
  Verify Japanese caster names through their authentic
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

Reviewed 2026-09-23. This file contains only work that is still open. Game
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
