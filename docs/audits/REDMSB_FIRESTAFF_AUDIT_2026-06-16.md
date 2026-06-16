# ReDMCSB → Firestaff Code Audit Report

**Date:** 2026-06-16
**Auditor:** Firestaff Watchdog (main session)
**Scope:** Source-level comparison of ReDMCSB WIP20210206
(200,402 LOC across 289 .C + 58 .H files in
`~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`)
against the Firestaff codebase (536,365 LOC across 878 .C + 846 .H files).

---

## 1. Headline Numbers

| Metric | ReDMCSB | Firestaff |
|--------|---------|-----------|
| `.C` source files | 289 | 878 |
| `.H` header files | 58 | 846 |
| Total LOC | 200,402 | 536,365 |
| Unique `F0xxx_*` functions | 595 | 1886 (refs), many via `firestaff_pc34_core_amalgam.c` (not built) |
| DM1 modules | (1 monolithic source) | 394 pc34-compat modules |
| CSB modules | (sister source dir) | 126 modules |
| Theron modules | n/a | 22 modules |
| Nexus modules | n/a | 38 modules |
| Dead code (unbuilt files) | 0 | 3 amalgam files × ~16K LOC |

**Bottom line:** Firestaff is significantly **larger** than ReDMCSB (5.3× raw, 2.7× on unique F0xxx). The DM1 V1 has been broken into 394 contract-only pc34-compat gates. The 4 other games (CSB, DM2, Theron, Nexus) have varying coverage.

---

## 2. Missing ReDMCSB Functions (F0xxx)

**71 F0xxx functions referenced in ReDMCSB have no Firestaff pc34 equivalent.**

Most are platform-legacy that should NOT be ported:

### 2a. Platform-legacy (do not port)

- **FLOPPY_* (F0449, F0450, F0451, F0518-F0532, F0926):** Floppy disk I/O, irrelevant for modern SDL engine.
- **EMM_* (F0747, F0749):** Expanded memory manager, irrelevant.
- **AMIGA_* (F0506, F0511):** Amiga-specific hardware glue.
- **JAPANESE_* (F0949-F0951):** Language-specific helper.
- **VIDEO_* (F0550, F0551):** VGA/CRTC hardware glue.
- **SCROLLER_* (F0557-F0563):** Copy-protection scroller.
- **COPYPROA/B_* (F0567-F0573):** Copy-protection subroutines.
- **INPUT_* (F0537, F0541, F0544):** Atari ST/PC input glue.
- **BASE_D (F0552), FILE_* (F0770-F0779):** Boot/loader helpers.
- **PRIM_* (F0917-F0929):** Video primitives.
- **DIALOG_* (F0428, F0600):** UI dialog helpers, scoped to copy-protection flow.
- **STARTEND_* (F0434, F0436, F0439, F0458, F0512):** Start/end game flow.
- **F0211_CPSDF_GetSubroutineAbsoluteAddress:** Copy-protection.

### 2b. Gameplay-relevant (REVIEW)

- **F0018_MAIN_S, F0022_MAIN_S:** Save/load summary helpers.
- **F0037_OBJECT_D:** Object description render.
- **F0060_SOUND_P:** Sound play (in ANIMSND.C).
- **F0073_MOUSE_B:** Mouse button state.
- **F0106_DUNGEONVIEW_T:** Dungeon view text render.
- **F0211_CPSDF_G:** Copy-protection (ignore).
- **F0260_TIMELINE_R:** Timeline read (in TIMELINE.C).
- **F0362_COMMAND_H, F0369_COMMAND_P, F0376_COMMAND_I:** Command processing.
- **F0421_SAVEUTIL_I:** Save utility (see Section 3).
- **F0496_LZW_O:** LZW (graphics decompression).

**Prioritet:** F0376 (click hit-test) and F0421 (save checksum) are gameplay-relevant and may have an unverified gap. See Section 3.

---

## 3. Suspected Gaps (Gameplay-relevant, not just platform glue)

### Gap A: F0376_COMMAND_IsPointInBox may be missing

ReDMCSB CLIKVIEW.C:290 defines a static helper used 8+ times for
door-button / wall-ornament / object-pile click hit-testing in
the dungeon view:

```c
return (x <= right) && (x >= left) && (y <= bottom) && (y >= top);
```

Firestaff's pc34_compat modules do **not** appear to define an
equivalent helper. The amalgam
(`firestaff_pc34_core_amalgam.c`) does, but is not built.
Confirm: the modern firestaff view code path uses an inline
implementation; this is fine if it matches the contract.

**Status:** TODO verify

### Gap B: F0421_SAVEUTIL_IsReadBytesWithChecksumSuccessful has modern replacement

ReDMCSB READWRIT.C:265 defines save-checksum. Firestaff uses
F0422 + CRC32 in `src/dm1/dm1_v1_save_load.c:356` and
`src/csb/csb_v1_save_load_pc34_compat.c:135` (F0429/F0430 + word-checksum).
Theron uses an in-house uint16_t sum
(`src/theron/theron_v1_save_load.c:70`).

**No gap** — modern equivalent, but **undocumented which path
corresponds to which media in ReDMCSB**. The Atari/PC checksum
used here differs from the Amiga path.

### Gap C: 3 dead amalgam files (16K LOC × 3 = 48K LOC wasted)

`src/shared/firestaff_pc34_core_amalgam.c`,
`src/shared/firestaff_pc34_flattened_amalgam.c`,
`src/shared/firestaff_pc34_sanitized_amalgam.c`

Each is 16K LOC, last touched 2026-05-26, only one reference in
git log (`8fd30dfb refactor: restructure repo`). Not built by
CMakeLists. Comments in `m11_game_view.c:2915`,
`include/dm1_v1_champion_panel_hud_pc34_compat.h:50`, and a probe
reference them.

**Status:** dead code, but keeping them in tree risks confusion
about which is canonical. Recommend: delete the 3 amalgam files,
keep pc34-compat modules as the canonical source.

### Gap D: P1 visual bugs from mail (Daniel Nylander 2026-06-16)

From `memory/2026-06-16.md`:
1. Resurrected champion dies within seconds — **fixed on origin/main**
   (commit `6bac7fcc fix: m11 resurrect sets HP, clears wounds, asserts leader`).
2. Champion graphical assets rendered floating in the air (Hall of
   Champions, first 4 steps forward) — **still open**.
3. Wall inscriptions are unreadable / blurry — **still open**.
4. Items on floor can only pick up one (resurrection-state-coupled
   pickup?) — **still open**.
5. CSB does not start (BOOT - FAILED TO LOAD DUNGEON.DAT) — **likely
   data discovery**, not engine. Verify with a fresh DM1+CSB
   user-data dir.

**Status:** 4 of 5 P1 bugs still open. TODO.md already lists
these as 🐛.

---

## 4. Module-Coverage Cross-Check

### 4a. DM1 V1 (394 modules)

DM1 has the densest coverage. Most are chest-mirror / panel
contract tests (F0333/F0334 chains, F0296-F0302 champion-panel,
F0280-F0286 revive, etc.). The pattern is sound: each
behavioural path in ReDMCSB has at least one contract-only test
pinning the F0xxx identifier, line range, expected state shape.

### 4b. CSB V1 (126 modules)

Solid core (boot, dungeon, combat, movement, magic, save-load).
V2 filter-config + presentation-mode + texture-upscale probes
cover CSB V2.0/V2.1/V2.2. **No gap identified.**

### 4c. DM2 (34 modules)

Skullkeep. Mostly the V2 smooth-movement + runtime binding
(43/43 binding seam). **No gap identified** — DM2 is a separate
game with its own source tree (SKULL.ASM).

### 4d. Theron V1 (22 modules)

PC Engine CD. 8 slots save, dungeon progression, viewport
renderer, tile renderer, shop, mechanics, world. V2 has
presentation mode + texture upscale + V2.2 shapes + filter
config. Phase 0/1 (V1-compat lock + launch/profile separation)
just landed. **No gap identified** in V1; V2 is missing Phase
2-7 (no separate modules for V2.2 lighting/particles/UI overlays
yet — they would be new modules if pursued).

### 4e. Nexus V2 (38 modules)

Saturn DM Nexus. V2 has V1-compat gate (Phase 0/1), HUD overlay
probe, lighting probe, smooth movement, touch controller, upscaler,
render pipeline, particles, atmosphere, verification suite
(Phase 7). All 8 phases covered. **No gap identified.**

---

## 5. Bug Bounty — Round 1 (high-leverage first)

Listed in priority order. Each is a single, self-contained fix
that can be committed and pushed independently.

### Bug 1: Confirm `F0376_COMMAND_IsPointInBox` coverage — **RESOLVED, partial**

- Read ReDMCSB CLIKVIEW.C:285-310.
- Confirmed F0376's contract: `(x <= right) && (x >= left) && (y <= bottom) && (y >= top)`.
- Grep Firestaff for any helper with that contract.
- Found `m11_point_in_source_box(px, py, const int box[4])` in
  `src/engine/m11_game_view.c:10833` with **identical contract**
  (different parameter order, packaged as a 4-element int box).
- Call sites confirmed at lines 10995, 11007 (door button),
  11092 (object pile), 11100 (wall ornament) — exactly the same
  clickable zones ReDMCSB uses.
- **No gap in modern engine code** — the contract is preserved
  through the helper.
- **Gap in test coverage**: no test pins the F0376 contract. Tried
  to add `tests/test_m11_point_in_source_box_pc34_compat.c` but
  hit a pre-existing build issue: `firestaff_m10` lib has
  `_G2157_` as unresolved symbol (image_backend_pc34_compat.c
  uses it but image_frontend_pc34.c, which defines it, is NOT
  in M10 lib's source list). Linking any M11 test against
  firestaff_m10 fails with undefined symbol error.
- **Action**: documented in Bug B below. Don't fix in this lane —
  scope creep. The F0376 contract IS preserved in modern code,
  the regression-risk comes from missing test, not missing impl.
- **Recommendation**: file separate pre-existing-build-issue ticket
  to add `image_frontend_pc34.c` to `firestaff_m10` lib source list
  OR move G2157_ definition into image_backend_pc34_compat.c
  (or a shared header). After that, the test gates below become
  buildable.

- Read ReDMCSB CLIKVIEW.C:285-310.
- Grep Firestaff for any helper that returns
  `x <= right && x >= left && y <= bottom && y >= top`.
- If found, document the equivalent pc34 function.
- If not found, write a test that proves the modern M11
  click pipeline uses the same contract (any D0L/D0R/D0C door
  button test should suffice).

### Bug B: Pre-existing build issue — `_G2157_` unresolved in `firestaff_m10` lib

`firestaff_m10` lib has unresolved symbol `_G2157_` from
`image_backend_pc34_compat.c` (used by F0687/F0688/IMG3_*).
The variable is defined in `src/shared/image_frontend_pc34.c`
(line 19: `unsigned int16_t G2157_;`), but this file is **not**
in `firestaff_m10` lib's source list (CMakeLists.txt:56 only
lists `M10_SOURCES` + `dungeon_decompressor_ftl.c` +
`m11_game_text_latin_extended_glyphs.c`).

Effect: any test that links against `firestaff_m10` lib fails
with `Undefined symbols for architecture arm64: _G2157_` if it
references any M10 object file that imports it.  The existing
`firestaff_m11_phase_a_probe` happens not to trigger this because
its invariant sweep doesn't call into IMG3 paths.

**Fix:** add `src/shared/image_frontend_pc34.c` to
`firestaff_m10` lib source list in CMakeLists.txt, OR move
G2157_ definition into `image_backend_pc34_compat.c` (or a
shared header that both files include).

This bug was discovered while writing the F0376 regression test
(Bug 1) and is a blocker for any new M11+M10 unit test.

### Bug 2: Delete dead amalgam files (48K LOC)

- `git rm src/shared/firestaff_pc34_core_amalgam.c` (990K)
- `git rm src/shared/firestaff_pc34_flattened_amalgam.c` (889K)
- `git rm src/shared/firestaff_pc34_sanitized_amalgam.c` (898K)
- Update the 3 reference sites (`m11_game_view.c:2915`,
  `include/dm1_v1_champion_panel_hud_pc34_compat.h:50`,
  `probes/dm2/firestaff_dm2_v1_asset_loader_probe.c`) to point
  to the canonical pc34-compat module.

### Bug 3: Hall-of-Champions champion Z-order

- Read TODO.md "P1 visual bugs" entry.
- Find the first-4-steps-forward test scenario.
- Pin the champion Z-order via a unit test that exercises the
  draw-list builder.

### Bug 4: Wall-inscription blur

- Read TODO.md "P1 visual bugs" entry.
- Find the inscription-render path in `src/dm1v2/` and the
  font scaler in `src/engine/`.
- Add a contract test that pins the inscription-render output
  shape (font, scale, kerning).

### Bug 5: Pickup-1-at-a-time-after-resurrection

- Read TODO.md "P1 visual bugs" entry.
- Find the pickup-route in `src/dm1/dm1_v1_*pc34_compat.c`
  (chest or floor pickup).
- Add a contract test that exercises pickup-while-resurrected
  and pins the multi-item pickup path.

---

## 6. Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Dead amalgam files in tree confuse new contributors | Medium | Low | Delete (Bug 2) |
| F0376 IsPointInBox has unverified modern replacement | Low | Medium | Add test (Bug 1) |
| Champion Z-order regression in Hall of Champions | Medium | High (user-visible) | Pin via test (Bug 3) |
| Inscription blur in wall decor | Medium | High (user-visible) | Pin via test (Bug 4) |
| Pickup race after resurrection | Low | Medium | Pin via test (Bug 5) |

---

## 7. Excluded from this audit

- **Performance benchmarks** (probes exist; raw ctest speed
  not measured here).
- **CI matrix** (5-platform build matrix is in verify.yml;
  status: green per v2.8.0 release).
- **Public docs** (README.md, RELEASE_NOTES.md, AGENTS.md are
  not source-code).
- **i18n** (17-language PO catalog is via firestaff_po_loader;
  coverage check is a separate audit).
- **Mod metadata** (cloning, image decompression, save format
  compatibility — would need a separate decomp audit with real
  game data).

---

## 8. Recommended next action

Execute Bug 1 (confirm F0376) first — it is the smallest, has
the clearest pass/fail criterion, and validates the modern
click pipeline. Then Bug 2 (delete dead code) to reduce
audit-noise. Then Bug 3/4/5 (P1 visual bugs) one at a time.
