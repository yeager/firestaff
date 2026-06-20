# Firestaff Gap List — Meta-Analysis (2026-06-20)

Honest, source-cited inventory of what remains between Firestaff
HEAD (`216b0b67`) and full cross-game coverage of all five
supported games (DM1, CSB, DM2, Nexus, Theron), assembled from
the dmweb-free-fr + greatstone surveys, the existing per-game
gap docs, and the stale `docs/parity/COMPLETION_MATRIX.md`.

This doc does NOT replace the per-game FINAL_GAPS files. It
sits above them as a cross-game index, prioritized for action.

Classification:
- **FIXED** — exists in main HEAD, source-locked.
- **PARTIAL** — partially implemented; some sub-items in main, others not.
- **OPEN-BOUNDED** — tractable, fits in a focused commit.
- **OPEN-LARGE** — would need a separate milestone (weeks, not days).
- **BLOCKED-DATA** — cannot close without real game data we don't have.
- **OUT-OF-SCOPE** — explicitly out of Firestaff (e.g. modding tools).

---

## A. Cross-cutting gaps

### A1. Asset format coverage

| Gap | Source | Status |
|---|---|---|
| IMG1/IMG2 RLE 16-color image decoder | ReDMCSB, sck | FIXED (`image_backend_pc34_compat.c` `IMG3_Compat_ExpandFromSource`) |
| IMG3/IMG4 4bpp local-palette image | dmweb Data Files | FIXED (same code path) |
| **IMG5 4bpp chunked image (Amiga, SNES)** | greatstone d_items.html | **FIXED in v2.9.2** (`firestaff_img5_decode.c`, commit `216b0b67`) |
| LZW-compressed items (DM Atari ST, CSB Atari ST) | dmweb Data Files | OPEN-BOUNDED — only Atari ST uses LZW; need decoder |
| **FTL container format (Amiga, X68000, MegaCD)** | greatstone d_ftl.html | OPEN-LARGE — 3-hunk Amiga-hunks structure, 4 checksums, two compression algorithms |
| **PAK container format (Atari ST)** | greatstone d_pak.html | OPEN-BOUNDED — 28-byte Atari ST executable header + LZ77-like compression |
| **HTC hint oracle text format (CSB)** | sck tutorial | OPEN-LARGE — text+layout format used by CSB Hint Oracle |
| **CMP portrait image format** | sck tutorial | OPEN-BOUNDED — portrait compression used by CSB Amiga |
| **AMG sound format (CSB utility disk)** | sck tutorial | OPEN-BOUNDED — sound effects storage |
| **MVE (Interplay, DM2 PC)** | dmweb Animations | OPEN-LARGE — DOS-stub + Interplay MVE binary |
| **QuickTime .moov (DM2 Macintosh)** | dmweb Animations | OPEN-LARGE — Apple QuickTime container |
| **DMDF/DGN (Nexus Saturn)** | AGENTS.md / ReDMCSB | PARTIAL — DMDF parser exists (`src/nexus/`), DGN partially |
| **MNS (Nexus monster/spell files)** | locally verified | PARTIAL — handled in launcher/profile detection, runtime sparse |
| **S2D (Nexus font files)** | locally verified | PARTIAL — parser exists, font rendering incomplete |
| **TAI/SAL/MAP (Nexus level data)** | locally verified | PARTIAL — loaders exist; TLINK metadata and rendering sparse |
| **BPX/BPK (Nexus compressed archives)** | locally verified | OPEN-BOUNDED |
| **Theron's Quest Track 02 BIN/ISO** | locally verified | OPEN-BOUNDED — loader exists, real-asset launch evidence missing |

### A2. Mapfile system

| Gap | Source | Status |
|---|---|---|
| YAML/TOML mapfile parser for arbitrary item description | greatstone d_mapfile.html | OPEN-BOUNDED — would let us reuse sck's 26+ game/version maps |
| Mapfile-to-Firestaff-asset-loader bridge | greatstone d_mapfile.html | OPEN-LARGE — Firestaff uses hard-coded table lookups in `image_backend_pc34_compat_globals.c` |

### A3. Engine behaviour parity

| Gap | Source | Status |
|---|---|---|
| CSB-specific hidden-code items 558-562 (Amiga executable blobs) | greatstone d_items.html | OPEN-BOUNDED — need skip table in graphics loader |
| Atari ST hidden code skip | greatstone d_items.html | OPEN-BOUNDED — same mechanism, separate items |
| Champion panel portrait loading from CSB utility disk | CSB docs | PARTIAL — DM1 path exists, CSB-portrait-source selection missing |
| Savegame format (DM1, CSB) | ReDMCSB + dmweb | FIXED for DM1 (`dm1_v1_save_load.c`), PARTIAL for CSB |
| Savegame format (DM2) | skproject source | PARTIAL |
| Savegame format (Nexus .sav) | locally verified | PARTIAL |
| Savegame format (Theron .SRM) | locally verified | PARTIAL |
| Custom dungeon import (DM1 dungeon.dat, CSBWin dmsave/csbgame) | dmweb Custom Dungeons page | OPEN-BOUNDED — synthetic loader exists, real-asset path missing |

### A4. i18n / l10n (post v2.9.1)

| Gap | Source | Status |
|---|---|---|
| 19-launcher-locale cycle | po/validate_po_layout.sh | FIXED in v2.9.2 |
| DM1 native translations (17 non-Swedish locales) | po/dm1_translations_complete.py | PARTIAL — 90 strings seeded, rest = English fallback |
| CSB native translations | po/csb_translations | PARTIAL — same pattern |
| DM2 native translations | po/dm2_translations | OUT-OF-SCOPE — DM2 slice not implemented |
| Nexus native translations | po/nexus_translations | PARTIAL |
| Theron native translations | po/theron_translations | OPEN-LARGE — 0 translations, Theron slice thin |
| **Native-vs-fallback separation in validator** | po/validate_po_layout.sh | OPEN-BOUNDED — currently masks native translations |
| Native QA on terminology / runtime rendering | po/ | OPEN-LARGE — needs native speakers |

### A5. Tooling

| Gap | Source | Status |
|---|---|---|
| compare_to_greatstone.py SHA256 probe | tools/asset-validate/ | FIXED in v2.9.2 (commit `0d89adc6`) |
| PLATFORM_MATRIX.md version support map | docs/PLATFORM_MATRIX.md | FIXED in v2.9.2 (commit `32dcf76c`) |
| DMWEB_REFERENCE.md consolidated reference | docs/DMWEB_REFERENCE.md | FIXED in v2.9.2 (commit `b54b52c4`) |
| **Real-data regression tests (greatstone db_data)** | greatstone sck tool | BLOCKED-DATA — db_data not currently fetchable from free.fr (404) |
| **Lefthook in PATH for CI** | build/CI hygiene | OPEN-BOUNDED — currently no-ops gracefully |

### A6. Build / CI

| Gap | Source | Status |
|---|---|---|
| Strict warnings `-Wall -Wextra -Werror` | CI verify.yml | FIXED |
| Phase A probe (headless invariants) | CI | FIXED (23/23) |
| Audio probe | CI | FIXED |
| Cross-platform determinism | CI | FIXED |
| `_G2157_` undefined symbol in firestaff_m10 | CMakeLists.txt | FIXED — `image_backend_pc34_compat_globals.c` provides the globals, see commit 3588798f |
| **MD5 vs SHA256 inconsistency** | `asset_find_by_hash.c` vs `VERIFIED_HASHES.md` | FIXED — see docs/MD5_SHA256_HARMONIZATION.md and `tools/asset-validate/compare_md5_to_sha256.py` |

---

## B. DM1 gaps (specific to game 1)

Source: `docs/FINAL_GAPS.md` (188 lines), `docs/DM1_V1_BUG_AUDIT.md`,
dmweb Game Page for Dungeon Master, ReDMCSB decompilation.

### B1. Capture-gap pairs (from prior TODO audit)

| Gap | Status |
|---|---|
| Original DOSBox/FIRES keyboard buffer transcript for I34E route keys | BLOCKED-DATA — needs Atari ST or DOSBox session |
| Paired original viewport screenshot (pass94 captures impaired) | BLOCKED-DATA |
| Paired original wall screenshot | BLOCKED-DATA |
| Paired original collision transcript | BLOCKED-DATA |
| Paired original creature-chain screenshot | BLOCKED-DATA |
| Paired original champion-panel screenshot | BLOCKED-DATA |

### B2. Per-domain DM1 gaps

| Gap | Doc reference | Status |
|---|---|---|
| Champion stats F0308, F0202, F0229 | FINAL_GAPS §Group 1 | FIXED |
| Magic-map C80-83 | FINAL_GAPS | FIXED |
| Teleporter rotation | FINAL_GAPS | FIXED |
| Kinetic pass-through F0816 | FINAL_GAPS | FIXED |
| Fire/spell shield subtraction F0321 | FINAL_GAPS | FIXED |
| C6 wisdom factor | FINAL_GAPS | FIXED |
| Trolin anti-mage palette F0823 | FINAL_GAPS | FIXED |
| DM_SAVE_HEADER noise/keys/checksums | FINAL_GAPS | FIXED |
| Hall of Champions 4-mirror + wall-mirror zones | FINAL_GAPS | FIXED |
| M12 launcher extras (3/5 wired) | FINAL_GAPS | FIXED (3 of 5) |
| **M12 launcher extras (2 remaining)** | FINAL_GAPS | OPEN-BOUNDED |
| Chest runtime detail coverage | TODO.md | OPEN-BOUNDED |
| Inventory route parity for all item types | `docs/dm1_gap_inventory_items.md` | PARTIAL |
| Champion portrait sensor parity | `docs/dm1_gap_portrait_sensor.md` | PARTIAL |
| Per-champion C01-C24 stats | `docs/dm1_gap_c01_c24_stats.md` | PARTIAL |
| C25-C26 anti-magic/spell items | `docs/dm1_gap_c25_c26.md` | PARTIAL |
| Touch zones for inventory | `docs/dm1_touch_inventory.md` | PARTIAL |
| Touch zones for champion panel | `docs/dm1_touch_champion.md` | PARTIAL |
| Touch zones for menu | `docs/dm1_touch_menu.md` | PARTIAL |
| AI pathfinding | `docs/ai_pathfinding.md` | PARTIAL |
| AI champion, creature, grouping, aggro | `docs/ai_*.md` | PARTIAL |

### B3. DM1 V2

| Gap | Status |
|---|---|
| V2.0/V2.1/V2.2 runtime pipeline | FIXED |
| V2.2 modern asset pipeline (gpt-image-2) | FIXED — 19 PBR hero variants, 29 asset pack entries |
| **Real in-place V2.2 drawing via m11_draw_dm1_\* passes** | OPEN-LARGE — overlay placeholder only |
| V2 modern UI overlay polish (inventory/champion/rune/action) | OPEN-LARGE |
| Enhanced lighting/shadows/field/projectile VFX | OPEN-LARGE |
| Smooth movement interpolation coverage | OPEN-BOUNDED |
| Full V1/V2 deterministic input scripts + screenshot/pixel gates | OPEN-BOUNDED |
| Per-mode pixel/material verification gates | OPEN-BOUNDED |

---

## C. CSB gaps

Source: `docs/FINAL_CSB_GAPS.md` (135 lines), 5 csb_gap_*.md files,
greatstone g_csb.html (14 versions documented), dmweb CSB Game Page.

### C1. Champions/mechanics/dungeon/graphics

| Gap | Doc | Status |
|---|---|---|
| Champions per-stat parity | csb_gap_champions.md | PARTIAL |
| Combat mechanics | csb_gap_combat.md | PARTIAL |
| Dungeon model/mechanics | csb_gap_dungeon.md | PARTIAL |
| Graphics + ornament blits (F0108, F0115, F0111, CustomBackgrounds) | csb_gap_graphics.md | OPEN-LARGE |
| Full mechanics parity | csb_gap_mechanics.md | OPEN-LARGE |

### C2. Per-version CSB asset coverage

| Version | Status |
|---|---|
| CSB Atari ST 2.0 (en) — original | BLOCKED-DATA |
| CSB Atari ST 2.0 (en) — cracked Replicants | BLOCKED-DATA |
| CSB Atari ST 2.1 (en) | BLOCKED-DATA |
| CSB Amiga 3.1 (en-fr-ge) original | BLOCKED-DATA |
| CSB Amiga 3.1 (en) cracked EndlessPiracy | BLOCKED-DATA |
| CSB Amiga 3.1 (en) cracked Betrayal | BLOCKED-DATA |
| CSB Amiga 3.3 (en-fr-ge) | BLOCKED-DATA |
| CSB Amiga 3.5 (en) original | BLOCKED-DATA — only M12 launch intent exists locally |
| CSB Amiga Utility disk (fr/ge/en/r1/r2/r3) | BLOCKED-DATA — champion editor, hint oracle |
| CSB FM-Towns (en-jp) | BLOCKED-DATA |
| CSB PC-98 3.1 (jp) | BLOCKED-DATA |
| CSB X68000 (jp) | BLOCKED-DATA |
| CSBWin (PC port by Paul Stevens) | PARTIAL — synthetic loader exists; real-asset test missing |

### C3. CSB hidden-code items

| Gap | Status |
|---|---|
| Atari ST hidden executable-code items (skip table) | OPEN-BOUNDED |
| Amiga 558-562 items (skip table) | OPEN-BOUNDED |
| CSBWin custom resource handling (csbgraphics.dat + dmsave + csbgame) | OPEN-LARGE |

### C4. CSB V2

| Gap | Status |
|---|---|
| V2.1/V2.2 dispatch + csb_v22_modern_assets_pc34 | FIXED |
| **Real PBR hero art for CSB** | OPEN-LARGE |
| **DM1 shared V2.2 in-place drawing pipeline** | OPEN-LARGE — same as DM1 B3 |
| Per-cell modern-art swap in CSB 9-square viewport | OPEN-LARGE |
| Phase 3 enhanced UI overlays | PARTIAL |
| Phase 5 smooth movement deterministic pixel gates | OPEN-BOUNDED |

---

## D. DM2 gaps

Source: `docs/NEXUS_PLAN.md` (similar scope), greatstone g_dm2.html
(11 versions), skproject source (DM2 Windows port).

### D1. DM2 V1 mechanics

| Gap | Status |
|---|---|
| Data model | FIXED |
| Boot/profile | FIXED |
| Rendering pipeline | FIXED |
| Combat resolver | FIXED |
| Spell module | FIXED |
| Tech/magic module | FIXED |
| **Shops/NPCs** | OPEN-BOUNDED |
| **Pressure plates** | OPEN-BOUNDED |
| **Triggers** | OPEN-BOUNDED |
| **Timeline wiring** | OPEN-BOUNDED |
| **Advanced CCM (DM2_PROCEED_CCM)** | OPEN-LARGE |
| **Projectile-list drain back into M11 renderer** | OPEN-LARGE |
| **Original-overlay proof** | OPEN-BOUNDED |
| **Launch-smoke gate** | OPEN-BOUNDED |

### D2. DM2 V2

| Gap | Status |
|---|---|
| Phase 2 asset pipeline + dm2_v22_modern_assets_pc34 | FIXED |
| **Real PBR hero art for DM2** | OPEN-LARGE |
| **DM1 shared V2.2 in-place drawing pipeline** | OPEN-LARGE |
| Per-cell modern-art swap in DM2 V1 (T560 indoor, T600 outdoor) | OPEN-LARGE |
| Phase 3 HUD runtime | PARTIAL |
| Phase 3 HUD bitmap assets + widgets (inventory quick-view, action prompt) | OPEN-BOUNDED |

### D3. DM2 per-version coverage

| Version | Status |
|---|---|
| DM2 PC 0.9 / 1.0 (en/fr/ge) / demo | BLOCKED-DATA — only M12 launch intent |
| DM2 Amiga 1.0 (en-fr-ge) | BLOCKED-DATA |
| DM2 MegaCD/SegaCD 1.0 (jp/en) | BLOCKED-DATA |
| DM2 Macintosh 1.0 (en/jp/demo) — uses QuickTime .moov | BLOCKED-DATA |
| DM2 PC-9801/PC-9821/IBM PS/V 1.0 (jp) | BLOCKED-DATA |
| DM2 FM-Towns 1.0 (jp) | BLOCKED-DATA |

---

## E. Nexus gaps (Saturn)

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`, `docs/NEXUS_PLAN.md`,
Nexus locally verified files in `~/.firestaff/data/nexus/`.

### E1. V1 phases 0-7

| Gap | Status |
|---|---|
| DMDF parser | PARTIAL |
| DGN world loader | PARTIAL |
| MNS creature/spell rendering | PARTIAL |
| S2D font loader | PARTIAL |
| TLINK/TAI/SAL/MAP runtime | PARTIAL |
| Save/load (.sav) | PARTIAL |
| V1 mechanics | PARTIAL |
| **Real Saturn asset handoff (NEXUS.BIN/ISO)** | BLOCKED-DATA — only DM.BIN + components locally |

### E2. V2 phases

| Gap | Status |
|---|---|
| Phase 0/1/2 | FIXED |
| Phase 5 smooth movement runtime | PARTIAL |
| Phase 7 verification (deterministic input + screenshot gates) | OPEN-BOUNDED |
| **Real PBR hero art for Nexus** | OPEN-LARGE |
| **Per-cell modern-art swap in Nexus V1 draw pipeline** | OPEN-LARGE |

---

## F. Theron gaps (PC Engine / TurboGrafx-16)

Source: `docs/NEXUS_PLAN.md` (similar shape), Theron local probes.

### F1. V1

| Gap | Status |
|---|---|
| V1 parser | FIXED |
| Rendering pipeline | FIXED |
| Mechanics | FIXED |
| Save/load (.SRM) | PARTIAL |
| Track02 bank routing | FIXED |
| Dungeon progression (7 dungeons) | FIXED |
| **JP/US Track 02 BIN/ISO real-asset launch** | BLOCKED-DATA — only synthetic data locally |
| Cross-slot import/export against real Track 02 saves | OPEN-BOUNDED |
| Cross-route mechanics runtime evidence | OPEN-BOUNDED |

### F2. V2

| Gap | Status |
|---|---|
| Phase 0/1 | FIXED |
| Phase 2 (presentation selection, EPX upscaler) | FIXED |
| theron_v22_modern_assets_pc34 | FIXED |
| **Real PBR hero art for Theron** | OPEN-LARGE |
| **Per-cell modern-art swap in T400/T600** | OPEN-LARGE |
| **Phase 3 enhanced UI overlays** | OPEN-LARGE — not started per TODO |
| **Phase 4 enhanced lighting/effects** | OPEN-LARGE — not started |
| **Phase 5 smooth movement** | OPEN-LARGE — not started |
| **Phase 6 touch/controller ergonomics** | OPEN-LARGE — not started |
| **Phase 7 V2 verification suite** | OPEN-LARGE — not started |

---

## G. Launcher / Settings / Accessibility (cross-cutting)

### G1. M12 launcher

| Gap | Status |
|---|---|
| 19-locale UI cycle | FIXED in v2.9.2 |
| Persistence for many options + 5 per-game slots | FIXED |
| **Polished UI flow** | OPEN-BOUNDED |
| Runtime handoff for every option | PARTIAL |
| **Save export/import** | OPEN-BOUNDED |
| **Session timer** | OPEN-BOUNDED |
| **Manual/docs launcher** | OPEN-BOUNDED |
| **Cloud sync** | OPEN-LARGE |
| Custom/V2 smooth-turn-pan toggles | OPEN-BOUNDED |

### G2. Touch / controller

| Gap | Status |
|---|---|
| Gesture navigation for runtime movement/turning | OPEN-LARGE |
| UI scaling + touch-target audit across launcher/game views | OPEN-LARGE |

### G3. Accessibility

| Gap | Status |
|---|---|
| Screen reader integration | OPEN-LARGE |
| High-contrast launcher remap | FIXED |
| In-game overlay coverage | PARTIAL |
| Launcher fontScale affects M12 text | FIXED |
| In-game overlays + UI-fit coverage | OPEN-BOUNDED |

---

## H. Cross-spiel prioritized work order

For the next sprint (post v2.9.1, pre v2.10.0), I propose this
order:

### Tier 1 (BLOCKED-DATA — surface via tooling, not code)

1. **Document real-data acquisition checklist**: which Atari ST /
   Amiga / SNES / MegaCD / Saturn / PC Engine binaries need to be
   sourced per game, with hashes that will gate future runs of
   `compare_to_greatstone.py`.
2. **Add `compare_to_greatstone.py` summary mode** that prints a
   per-game "data gap" view (which files in VERIFIED_HASHES.md are
   not on disk).

### Tier 2 (OPEN-BOUNDED — fits one commit each)

3. CSB hidden-code skip table for Atari ST + Amiga items 558-562.
4. LZW decoder for Atari ST GRAPHICS.DAT (only Atari ST uses LZW).
5. PAK container decoder for Atari ST START.PAK. — DONE (commit 3ee479de)
6. CMP portrait loader for CSB utility disk. — DONE (commit 532c8250)
7. Harmonize MD5 vs SHA256 in `asset_find_by_hash.c` (or add
   wrapper). — DONE (commit 5988b620, see docs/MD5_SHA256_HARMONIZATION.md)
8. `_G2157_` linker fix (add `image_frontend_pc34.c` to
   firestaff_m10 source list). — DONE (commit 3588798f, added
   `image_backend_pc34_compat_globals.c` instead which provides the
   same symbols without dragging in the legacy frontend)
9. Lefthook-in-CI install step.

### Tier 3 (OPEN-LARGE — separate milestones)

10. FTL container decoder (Amiga, X68000, MegaCD).
11. CSBWin custom resource path (csbgraphics.dat + dmsave).
12. DM2 advanced CCM + projectile-list drain.
13. DM2 per-cell modern-art swap (T560/T600).
14. Nexus per-cell modern-art swap (Saturn draw pipeline).
15. Theron V2 phases 3-7 (UI overlays, lighting, smooth movement,
    touch, verification).
16. CSB V1 graphics ornament blits (F0108, F0115, F0111,
    CustomBackgrounds).

### Tier 4 (per-game polish, partially started)

17. CSB mechanics parity (combat, dungeon, champion, inventory).
18. DM2 mechanics (shops, NPCs, triggers, timeline).
19. Nexus runtime/probe coverage beyond compile/save-load.
20. Theron cross-slot import/export + cross-route evidence.

### Tier 5 (i18n follow-up)

21. Native-vs-fallback separation in `validate_po_layout.sh`.
22. Fill DM1/nexus/csb/theron/firestaff missing native translations
    with native speakers (out of scope for AI).

### Tier 6 (launcher/accessibility)

23. Save export/import, session timer, manual launcher.
24. Touch gesture navigation.
25. Screen reader integration.

---

## I. Sources surveyed (this round)

| Source | Pages reviewed | Used for |
|---|---|---|
| dmweb.free.fr (Game Pages) | 5 (DM, CSB, DM2, TQ, Nexus) | per-game history, awards, ports |
| dmweb.free.fr (File formats) | 3 (Data Files, Animations, Animation Script) | IMG/IMG5/ANIMATE.SCR specs |
| dmweb.free.fr (Clones) | 3 (CSBwin, SKWIN, Return to Chaos) | clone source references |
| dmweb.free.fr (Custom dungeons) | 1 (g_csb.html) | 60+ custom dungeon index |
| dmweb.free.fr (FAQ) | 1 index | category map only — Drupal URLs 404 |
| dmweb.free.fr (ReDMCSB) | 0 — page not fetchable | fallback to local ReDMCSB |
| greatstone (Tools) | 4 (Overview, Product, Screenshots, Tutorial) | sck usage, extraction CLI |
| greatstone (Technical doc) | 4 (Mapfile, FTL, PAK, Items + IMG5) | all critical format specs |
| greatstone (Games) | 4 (DM, CSB, DM2, Custom) | per-version file lists |
| greatstone (Articles) | 0 — content empty | skipped |
| Firestaff existing docs | FINAL_GAPS, FINAL_CSB_GAPS, PLATFORM_MATRIX, DMWEB_REFERENCE, csb_gap_*.md, dm1_gap_*.md | per-game gap detail |

**Total new pages reviewed this session: 19** (beyond what was
already covered in DMWEB_REFERENCE.md).

---

## J. Update cadence

This document should be regenerated after every milestone commit.
Each major section (B-J) is regenerated from its per-domain
source-of-truth:

- A → `po/validate_po_layout.sh` + `tools/asset-validate/compare_to_greatstone.py --list`
- B → `docs/FINAL_GAPS.md` + `docs/dm1_gap_*.md` + `docs/ai_*.md`
- C → `docs/FINAL_CSB_GAPS.md` + `docs/csb_gap_*.md`
- D → `docs/NEXUS_PLAN.md` (DM2 sections) + `src/dm2/` headers
- E → `docs/NEXUS_FILE_CLASSIFICATION.md` + `src/nexus/` headers
- F → Theron TODO sections + `src/theron/` headers
- G → `TODO.md` cross-cutting sections
- H → manual prioritization after the above regenerates

Avoid duplicating content; this doc is an index, not a source.
