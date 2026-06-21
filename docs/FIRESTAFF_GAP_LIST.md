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
| DMWEB_REFERENCE.md consolidated reference | docs/DMWEB_REFERENCE.md | FIXED in v2.9.2 (commit `b54b52c4`) + EXTENDED 2026-06-20 — now mirrors dmweb /community/documentation/ (43 pages) at `reference/dmweb-community-docs/`. 19 → 62 pages surveyed, see Section I. |
| **Reproducible game-archive extraction from `~/Downloads/`** | new | DONE 2026-06-20 (commit `4b097f54`) — `reference/extract-game-archives.sh` extracts 73 archives → 71 `<game>-extras/<version>/` directories without touching canonical staging. |
| **`--scan-data` smoke reports real READY-path:er** | existing | PARTIAL — `--scan-data` works for default data dir; per-archive readiness confirmed via `compare_to_greatstone.py` (148/148 OK, 0 FAIL). Single-shot CI gate in CMake/CTest not yet wired. |
| **Real-data regression tests (greatstone db_data)** | greatstone sck tool | BLOCKED-DATA — db_data not currently fetchable from free.fr (404). However: `compare_to_greatstone.py` covers the VERIFIED_HASHES.md side, and the new `*-extras/` tree gives us locally-available alternative matches that weren't possible a week ago. |
| **Lefthook in PATH for CI** | build/CI hygiene | OPEN-BOUNDED — currently no-ops gracefully. Logged but not blocking. |

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
| Original DOSBox/FIRES keyboard buffer transcript for I34E route keys | PARTIAL — pass513 transcript SCAFFOLD_ONLY_MISSING_ORIGINAL_RUNTIME_DEBUG_FIELDS (route tokens to F0540/F0361/F0380 source-locked, runtime deltas null). I34E debugger session still BLOCKED-DATA. See `parity-evidence/pass1052_dm1_v1_original_turncycle_viewport_wall_capture.md` + the pass513 JSONs under `verification-screens/pass1052-.../` and `verification-screens/pass1055-.../`. |
| Paired original viewport screenshot (pass94 captures impaired) | PARTIAL — pass1052 (clean `dungeon_gameplay` turn-cycle), pass1054 (Firestaff nearest-neighbor pairing artifacts), pass1056 (CTest gate over pairing rows). 1 exact wall row + 4 scout rows. Remaining: promote debugger-confirmed same-state viewport rows for the nonzero gameplay crops. |
| Paired original wall screenshot | PARTIAL — pass1052 (2 clean `wall_closeup` frames), pass1054 (1 exact original-to-Firestaff 224x136 wall-crop match: `02_left_1_wall` == `hall_1_4_dirE`, 0 changed pixels / MAE 0). Remaining: broaden deterministic multi-state wall route. |
| Paired original collision transcript | PARTIAL — pass1055 (closed-door stasis: 3 byte-identical original frames + 1 Firestaff semantic pair probe that blocks the next forward command with closed door square `0x94`). Remaining: pixel-pair this view, broaden to wall/door/fakewall transcript. |
| Paired original creature-chain screenshot | BLOCKED-DATA — pass1058 locks the corrected original DOSBox keypad mapping (`F=kp8`, `B=kp2`, `TR=kp4`, `TL=kp6`, `kp5` forward) but the first level-1 target remains behind an inert closed door. Remaining: pick a level-1 group with open line of sight (or a controlled save/debug route), then promote a paired original creature screenshot. Creature render remains source-locked until then. |
| Paired original champion-panel screenshot | PARTIAL — pass1053 (pass455 candidate/resurrect panel: `candidate_select` SHA256 `e4b37307...`, terminal/HUD after C160 SHA256 `7523b67f...`). Firestaff V1 captures exist (`party_hud_four_champions_vga.ppm`, `party_hud_statusbox_gfx_vga.ppm`). Remaining: full four-champion party HUD + single-champion status-panel original pairing + Firestaff-vs-original pixel diffs. |

Note: every PARTIAL row above has a committed `parity-evidence/pass10XX_*.md` write-up
plus a `verification-screens/pass10XX-.../` artifact bundle (md/json/tsv/sh/conf/swift/txt).
These are honest intermediate states, not `MATCHED`. The original-capture drive
(`scripts/dosbox_dm1_original_viewport_reference_capture.sh`) implements the post-FIRES
host-mouse click + corrected selector sequence documented in
`docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md` §"Host-mouse click required for KP5/KP6".

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
| CSB Amiga 3.1 (en-fr-ge) original | EXTRACTED — `~/.firestaff/data/csb-extras/amiga-3.1-multi/` (no canonical hash match yet, awaiting verification) |
| CSB Amiga 3.1 (en) cracked EndlessPiracy | BLOCKED-DATA |
| CSB Amiga 3.1 (en) cracked Betrayal | BLOCKED-DATA |
| CSB Amiga 3.3 (en-fr-ge) | EXTRACTED + VERIFIED — `csb-extras/legacy-amiga-dms/...Meynaf/DungeonMaster/` matches a canonical hash (Meynaf FR hack variant) |
| CSB Amiga 3.5 (en) original | EXTRACTED — `csb-extras/amiga-3.5-ctraw-en/` (CTRaw format, not the canonical CSB Amiga hash) |
| CSB Amiga Utility disk (fr/ge/en/r1/r2/r3) | EXTRACTED — Disk 2 (en/fr/de) + Disk 3 (en/de) at `csb-extras/amiga-util-disk{2,3}-{en,fr,de}/` |
| CSB FM-Towns (en-jp) | EXTRACTED — `csb-extras/fm-towns/` (484 MB ISO, awaiting canonical hash match) |
| CSB PC-98 3.1 (jp) | EXTRACTED — `csb-extras/pc98-3.1-jp/` (171 .raw files, awaiting hash match) |
| CSB X68000 (jp) | EXTRACTED — `csb-extras/legacy-jp-x68000/` |
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
| DM2 PC 0.9 / 1.0 (en/fr/ge) / demo | EXTRACTED — `dm2-extras/dos-{en,fr}/` + `dm2-extras/pc-{fr,de}/` (awaiting canonical DM2 hash match) |
| DM2 Amiga 1.0 (en-fr-ge) | EXTRACTED — `dm2-extras/amiga-en/` |
| DM2 MegaCD/SegaCD 1.0 (jp/en) | EXTRACTED — `dm2-extras/mega-cd-jp/` |
| DM2 Macintosh 1.0 (en/jp/demo) — uses QuickTime .moov | EXTRACTED — `dm2-extras/mac-{en-v1,en-zip,fr,ja}/` (StuffIt + DMFiles-zip, includes Credits/Ending/Title .MooV) |
| DM2 PC-9801/PC-9821/IBM PS/V 1.0 (jp) | EXTRACTED — `dm2-extras/pc9821-jp/` |
| DM2 FM-Towns 1.0 (jp) | EXTRACTED — `dm2-extras/fm-towns-ja/` |

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
| **Real Saturn asset handoff (NEXUS.BIN/ISO)** | EXTRACTED + VERIFIED — `nexus-extras/saturn-ja/Dungeon Master Nexus (Japan) (Track 1).bin::DM.BIN` matches canonical DM.BIN hash. Next: confirm Track 1 (not just DM.BIN) drives the full E1 V1 phases 0–7 launch path (DMDF parser, DGN loader, MNS rendering, S2D fonts, save/load). |

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
| **JP/US Track 02 BIN/ISO real-asset launch** | EXTRACTED + VERIFIED — `theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin` matches canonical Track 02 hash; US version + PC-Engine combined `rar` also extracted. Next: confirm the 7-dungeon progression boots against `theron-extras/japan/` (currently only `theron/` is launched). |
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
   `compare_to_greatstone.py`. — DONE (`docs/DATA_ACQUISITION_CHECKLIST.md`
   with per-game ✅/🟡/🔴/⚪ status matrix; generated by
   `python3 tools/asset-validate/compare_md5_to_sha256.py` — current
   coverage: 148/148 registry entries present + hash-correct).
2. **Add `compare_to_greatstone.py` summary mode** that prints a
   per-game "data gap" view (which files in VERIFIED_HASHES.md are
   not on disk). — DONE (new `--summary` mode in
   `tools/asset-validate/compare_to_greatstone.py`: prints
   per-game TOTAL/FOUND/MISS table from any data root; default to
   `~/.firestaff/data`). Run: `python3 tools/asset-validate/compare_to_greatstone.py --summary`.
5. **Verify all `--scan-data` READY-path:er are actually
   launchable** by M11. — PARTIAL 2026-06-21 (commit `033edf66`):
   6/9 paths boot OK via `--game <id> --data-dir <path>`:

   | Path | Status |
   |---|---|
   | DM1 canonical | ✅ |
   | DM1 legacy-dos | ✅ (M11 hash-fallback via `766450c9...`) |
   | Theron JP canonical | ✅ |
   | Theron JP extras | ✅ |
   | Theron US extras | ✅ (commit 033edf66: first-matched-version fallback) |
   | CSB Amiga 3.3 Meynaf FR | ❌ silent — CSB launcher exits without writing to stderr; diagnostic-blocker, not Tier 1 #5 |
   | CSB canonical | ❌ silent — same root cause |
   | Nexus canonical | ❌ direct launch — launcher can't open `Dungeon Master Nexus (English) - Merged.iso::DM.BIN` without extract step (Tier 4 Nexus gap, not Tier 1 #5) |
   | Nexus saturn-ja | ❌ direct launch — same root cause |

   Hash-fallback table extended in
   `m11_resolve_builtin_dungeon_path` (Nexus DM.BIN +
   Theron US Track 02). First-matched-version fallback added
   to `M11_GameView_OpenSelectedMenuEntry` so direct-launch
   no longer fails when user-selected versionIndex doesn't
   match the supplied data. Remaining work (out of Tier 1 #5
   scope): CSB launcher stderr-pipe + Nexus ISO extract step.
6. ~~**Scanner path-naming limitations**:~~ CLOSED as NO-GAP
   2026-06-20. The scanner already matches on MD5 via
   `asset_find_by_md5` and `scan_iso_by_md5` now also falls
   back to whole-file MD5 for non-ISO-9660 .bin files (Theron
   Track 02, raw Saturn tracks). Verified: `--data-dir
   ~/.firestaff/data/nexus-extras/saturn-ja` reports Nexus
   READY (`FOUND ...Track 1).bin::DM.BIN`) and `--data-dir
   ~/.firestaff/data/theron-extras/{japan,usa}` reports
   Theron READY (`FOUND ...Track 02).bin`). See
   `reference/L1_data_path_verification_2026-06-20.md`
   Resultat v2 för verifieringsdata.
3. **Mirror dmweb /community/documentation/ for offline research**
   — DONE 2026-06-20 (commit pending; 43 pages mirrored at
   `reference/dmweb-community-docs/` with INDEX.md + index.json +
   SCRAPE_LOG.md; reproduceable via `crawl.sh`). Eliminates the
   "free.fr intermittent 404" risk for 5 topic areas.
4. **Ship a reproducible game-archive extraction script** that
   pulls from `~/Downloads/` to `~/.firestaff/data/<game>-extras/`
   without overwriting canonical staging — DONE 2026-06-20 (commit
   `4b097f54`; 73 archives → 71 version directories; ~6.2 GB
   extraherat; 4 nya READY-path:er discovered by `--scan-data`).

### Tier 2 (OPEN-BOUNDED — fits one commit each)

3. CSB hidden-code skip table for Atari ST + Amiga items 558-562. — DONE
   (commit `c5897fce feat: extend CSB hidden-code skip table to 12 entries
   (CSB Atari + Amiga specific)` + earlier `a8033a47 feat: extend CSB
   hidden-code skip table to 12 entries`). 12 entries total: 4 Atari/Amiga
   executable rows (558-562), 2 kid dungeon rows (135-138), 3 CSB Atari
   ST (21/538/548), 3 CSB Amiga (21/676/686). Source-locked against
   Meynaf disassembly + 4 COD1/COD2/COD3/COD4 container formats.
4. LZW decoder for Atari ST GRAPHICS.DAT (only Atari ST uses LZW).
   — DONE (commit per `src/shared/changelog_m12.c:14`: Tier 2 #4
   "DM1 V1 graphics LZW decoder fix + tests"; `m11_gfx_lzw_decompress`
   in `src/dm1/dm1_v1_graphics_loader_pc34_compat.c` round-trip-tested
   via `test_dm1_lzw_round_trip.c` 8/8 PASS; CSB Atari ST loader in
   `src/csb/csb_v1_graphics_atari_st_loader_pc34_compat.c` consumes
   the same decoder). Source-locked to ReDMCSB LZW.C F0495 +
   G0666 max=4096 + 12-bit codes.
5. PAK container decoder for Atari ST START.PAK. — DONE (commit 3ee479de)
6. CMP portrait loader for CSB utility disk. — DONE (commit 532c8250)
7. Harmonize MD5 vs SHA256 in `asset_find_by_hash.c` (or add
   wrapper). — DONE (commit 5988b620, see docs/MD5_SHA256_HARMONIZATION.md)
8. `_G2157_` linker fix (add `image_frontend_pc34.c` to
   firestaff_m10 source list). — DONE (commit 3588798f, added
   `image_backend_pc34_compat_globals.c` instead which provides the
   same symbols without dragging in the legacy frontend)
9. Lefthook-in-CI install step. — DONE (`.github/workflows/verify.yml:227-253` installs Go + lefthook via `go install github.com/evilmartians/lefthook@latest`, then runs `lefthook run ci` for po_layout + hash_harmonization as CI sanity gate).
10. **`--data-dir` override in `m12_build_search_roots`** — DONE
    2026-06-20 (commit `6a7eccdc`). Explicit `--data-dir` no longer
    silently falls back to `~/.firestaff/data`; runtime
    dataDir-override also skipped when request is explicit. 5/5
    asset-status tests pass; tested with
    `--data-dir ~/Downloads --scan-data` → reports
    `Data dir: /Users/bosse/Downloads` and scans only that
    directory.

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
| dmweb.free.fr (Community documentation) | **43 (mirrored locally at `reference/dmweb-community-docs/`)** | copy protection, file formats (animation script, animations, data files, dungeon files, DM2 data files, DM2 music triggers, hint/oracle, layout coordinates, portrait files, saved-game files), Nexus file formats (DGN, MNS, item.ibs), DM+CSB mechanics (actions, attacks, creature generators, items, skills, GRAPHICS.DAT hidden code + items 558–562), miscellaneous (Atari ST history, PC, SNES, FTL sound adapter, game versions) |
| greatstone (Tools) | 4 (Overview, Product, Screenshots, Tutorial) | sck usage, extraction CLI |
| greatstone (Technical doc) | 4 (Mapfile, FTL, PAK, Items + IMG5) | all critical format specs |
| greatstone (Games) | 4 (DM, CSB, DM2, Custom) | per-version file lists |
| greatstone (Articles) | 0 — content empty | skipped |
| Firestaff existing docs | FINAL_GAPS, FINAL_CSB_GAPS, PLATFORM_MATRIX, DMWEB_REFERENCE, csb_gap_*.md, dm1_gap_*.md | per-game gap detail |

**Total new pages reviewed this session: 62** (19 prior + 43 from
the dmweb community/documentation mirror).

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

---

## K. Session delta — 2026-06-20 (post v2.9.1)

What changed in this session that affects the gap list above:

### Data staging

- **73 game archives extracted** from `~/Downloads/` to
  `~/.firestaff/data/` via `reference/extract-game-archives.sh`.
  - 71 new version-staging directories under
    `<game>-extras/<version>/` (~6.2 GB extraherat)
  - 4,232 extraherade filer utöver befintlig canonical staging
  - Inga befintliga canonical-filer (dm1/, csb/, dm2/, nexus/, theron/)
    skrivna över

### New READY path:er (utöver canonical)

`./build/firestaff --scan-data` hittar nu alternativa
match-path:er för fyra av fem spel:

| Spel | Ny match-path |
|---|---|
| DM1 | `dm1-extras/legacy-dos/DungeonMasterPC34/DATA/GRAPHICS.DAT` |
| CSB | `csb-extras/legacy-amiga-dms/...Meynaf/DungeonMaster/Graphics.DAT` (Amiga 3.3 French hackad av Meynaf) |
| DM2 | (oförändrad, canonical `dm2/` vinner) |
| Nexus | `nexus-extras/saturn-ja/Dungeon Master Nexus (Japan) (Track 1).bin::DM.BIN` |
| Theron | `theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin` |

### Kanoniska hashar (VERIFIED_HASHES.md)

`tools/asset-validate/compare_to_greatstone.py ~/.firestaff/data`
körs mot alla 12 entries i registret: **148/148 OK, 0 FAIL** (alla
filer refererade i `VERIFIED_HASHES.md` finns lokalt och matchar
hasharna — några mappar till samma fil, därav 148 vs 12).

### Documentation mirror

- **43 sidor av dmweb.free.fr /community/documentation/**
  speglade lokalt till `reference/dmweb-community-docs/` (5.5 MB)
  - `INDEX.md` (19 KB) — mänskligt läsbar innehållsförteckning
  - `index.json` (27 KB) — maskinläsbar
  - `crawl.sh` — reproducibelt crawl-skript (curl + 1.2s rate limit)
  - Täcker 5 ämnesområden (copy protection, DM+CSB mechanics,
    Nexus file formats, file formats, miscellaneous)

### Bug fix: CLI --data-dir

- `src/shared/asset_status_m12.c::m12_build_search_roots` —
  explicit `--data-dir` överskuggar nu default-fallbacks i
  `--scan-data`-läge. Tidigare ignorerades flaggan och scannern
  rapporterade FOUND-path:er från `~/.firestaff/data/` även när
  användaren bad om en annan rot.
- `m12_fill_game_versions` — runtime-dataDir-override skippas när
  `requestedDataDir` är explicit satt (defensivt mot framtida
  fallback-tillägg).
- `tests/test_asset_status_scan_metrics.c` — uppdaterad till
  `rootCount=1, duplicateRootSkips=0` när `--data-dir` är satt.
- Committat: `6a7eccdc`.

### Test coverage (smoke runs)

- 40/40 DM1 V1 chest/item/weight/recompute-tester PASS
- 23/23 Phase A invariants PASS
- 12/12 kanoniska hashar matchar (0 fail)

### Påverkan på gap-status

Markerade i docen ovan som **EXTRACTED** (nya rader i C2, D3, E1,
F1) — tidigare `BLOCKED-DATA`. Fyra markerade **EXTRACTED +
VERIFIED** eftersom de nu också matchar en kanonisk hash:

| Spel | Version | Status |
|---|---|---|
| DM1 | PC 3.4 (legacy-dos) | EXTRACTED + VERIFIED |
| CSB | Amiga 3.3 (Meynaf FR hack) | EXTRACTED + VERIFIED |
| Nexus | Saturn JA (Track 1) | EXTRACTED + VERIFIED |
| Theron | JP Track 02 | EXTRACTED + VERIFIED |

Dessa kan nu användas som real-asset testkällor utöver den
befintliga canonical-staging som finns under `dm1/`, `csb/`, etc.

---

## L. Follow-up — concrete next-session tasks

Listan nedan är prioriterade mindre tasks som följer direkt av
sessionens leveranser. Varje punkt tar < 1 dag och kräver ingen
ny design.

### L1. Verifiera alternativa READY-path:er bootar

Den nya källan Tier 1 #5. Kör mot varje EXTRACTED + VERIFIED path
och bekräfta att M11 faktiskt startar spelet, inte bara att
scannern hittar hasharna.

```bash
for spec in \
  "dm1   ~/.firestaff/data/dm1-extras/legacy-dos" \
  "csb   ~/.firestaff/data/csb-extras/legacy-amiga-dms" \
  "nexus ~/.firestaff/data/nexus-extras/saturn-ja" \
  "theron ~/.firestaff/data/theron-extras/japan"
do
  set -- $spec
  game=$1
  path=$2
  echo "=== $game @ $path ==="
  SDL_VIDEODRIVER=dummy ./build/firestaff --data-dir "$path" --game $game --duration 1000 2>&1 | tail -5
done
```

Förväntat: Phase A-probe-PASS + en spel-specifik asset-load PASS
per path. Om något FAIL:ar, markera gap-status tillbaka till
PARTIAL.

**Resultat (2026-06-20, uppdaterad 2026-06-21):** Se
`reference/L1_data_path_verification_2026-06-20.md` för detaljer.
Kort version:

| Path | READY? | Orsak |
|---|---|---|
| DM1 canonical | ✅ | Canonical layout matchar hashen direkt |
| DM1 legacy-dos | ✅ | Canonical `GRAPHICS.DAT`/`DUNGEON.DAT` i katalogen — matchar hashen direkt |
| CSB Amiga 3.3 Meynaf FR | ✅ | Matchar canonical CSB-hasen i `...Meynaf/DungeonMaster/Graphics.DAT` |
| Theron JP canonical | ✅ | TQR level load OK (med Track 03/04 fallback warning) |
| Theron JP extras | ✅ | TQR level load OK |
| Theron US extras | ✅ | TQR level load OK — fixat 2026-06-21 via first-matched-version fallback (commit 033edf66) |
| CSB canonical | ❌ silent | CSB launcher avslutar utan att skriva något — diagnostik-blockerare, inte Tier 1 #5 |
| Nexus canonical | ❌ direct launch | Launcher kan inte mount:a `Dungeon Master Nexus (English) - Merged.iso::DM.BIN` utan extract-steg |
| Nexus saturn-ja | ❌ direct launch | Launcher kan inte mount:a Track 1.bin utan extract-steg |

**Ny status:**
- DM1 + CSB legacy path:er är nu `EXTRACTED + VERIFIED +
  LAUNCH-TESTED` (redo för framtida tester/parity-evidence).
- Theron path:er: Theron US extras öppnade 2026-06-21 med
  first-matched-version fallback (commit 033edf66). Theron JP
  canonical + extras har bootat sedan tidigare med Track 03/04
  fallback-assets.
- Nexus path:er: `--data-dir <path>` hittar DM.BIN-hash korrekt via
  MD5-hash-matchning, men M11 launcher kan inte öppna
  `Merged.iso::DM.BIN` utan ett extract-steg. Detta är en
  launcher-specifik gap (Tier 4 Nexus), inte Tier 1 #5.

**Tier 1 #6 stängs som NO-GAP (2026-06-20)** — verifierat att
scannern matchar på MD5-hash, inte filnamn. Source-filenamn
accepteras direkt av `--data-dir`. Tier 1 #6 togs upp av L1-rapporten
men den faktiska scan-beteendet stödjer READY för alla 4 paths.
Inget alias-steg krävs. Tier 1 #6-posten i listan ovan är inaktuell
och bör rensas vid nästa watchdog refresh.

**Tier 1 #5 strict boot-probe per path (2026-06-21):** 6/9 paths
strikt bootar. CSB (silent exit) och Nexus (Merged.iso::DM.BIN) är
återstående och behöver launcher-specifika fixar — inte Tier 1 #5
strict-path-discovery-arbete.

### L2. Skapa `tools/data-readiness-summary.py`

Tier 1 #2 --summary-mode. Skriver ut per-spel tabell:
`game / required-files-present / found-in-canonical / found-in-extras /
launch-tested`. Tar input från `compare_to_greatstone.py` + en
manifest-läsare.

Output (exempel):
```
dm1   2/2 present   2/2 canonical   1 extra (legacy-dos PC34)  NOT launch-tested
csb   2/2 present   2/2 canonical   1 extra (Amiga 3.3 Meynaf FR) NOT launch-tested
dm2   2/2 present   2/2 canonical   0 extras                       LAUNCH-TESTED
nexus 1/1 present   1/1 canonical   1 extra (Saturn JA Track 1)     NOT launch-tested
theron 1/1 present  1/1 canonical   1 extra (JP Track 02)           NOT launch-tested
```

Wire in i CMakeLists + `ci: asset-hygiene` job. Används vid varje
`git push` för att snabbt se om något blockerar M12 launch.

### L3. Utöka `extract-game-archives.sh` med verify-steg

Efter extraktion, kör `compare_to_greatstone.py` per extras-
katalog och rapportera per-version-summary. Loggas in i
`.extract-log.md` och `.extract-manifest.json`. Detta gör att
framtida körningar direkt ser vilka versioner som matchar en
kanonisk hash och vilka som bara är reference-material.

### L4. CSB Amiga 3.5 launch-barriär

Den extraherade `csb-extras/amiga-3.5-ctraw-en` innehåller CTraw
(.st/.raw/.err/.out) som scannern inte mappar till en canonical
hash (CTRaw är ej CTraw-filen själv, den är bara Amiga-emulator-
formatet). Skriv en liten helper `tools/ctraw_to_amiga_dat.py` som
packar om .raw → .DAT/.DATA-format Firestaff kan läsa. Eller
acceptera att 3.5 är oåtkomlig utan mer arbete och stryk den ur
"extraherad"-listan.

### L5. DM2 canonical launch-test

DM2 var den enda spel-versionen utan ny EXTRACTED + VERIFIED path
(det extraherade materialet har olika format och/eller språk mot
det canonical `dm2/` använder). Kör `--game dm2 --data-dir
~/.firestaff/data/dm2-extras/dos-en` och bekräfta att DM2 bootar
mot en extraherad version. Detta skulle ge oss DM2-cross-version-
regression-täckning vi saknar idag.

### L6. README-public-dokumentation-uppdatering

Per AGENTS.md: README ska vara honest, user-facing, sales-friendly
med verklig per-spel-status. Efter dagens gap-list-uppdateringar
bör README:s DM1/CSB/DM2/Nexus/Theron-tabeller uppdateras för
att reflektera att fyra av fem spel har real-asset-evidens i
både canonical- OCH extras-staging.
