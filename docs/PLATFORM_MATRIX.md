# Firestaff — Platform & Version Support Matrix

The canonical "what game versions does Firestaff support, and which
ones *could* it support with more data" reference. Derived from
the dmweb/greatstone survey (`docs/DMWEB_REFERENCE.md`) and
Firestaff's actual implementation state.

## Status legend

| Symbol | Meaning |
|---|---|
| ✅ | **Verified working** — hash-verified game data exists locally (`docs/VERIFIED_HASHES.md`), M-series tests pass, runtime proof captured in `parity-evidence/` |
| 🟡 | **Source-locked** — ReDMCSB/CSBwin/SKWIN source reverse-engineered, but no local data files (yet) for runtime proof |
| 🔵 | **Greatstone-extracted** — Pierre Monnot's sck tool has decoded this version, data is publicly browsable at `http://greatstone.free.fr/dm/db_data/`, but Firestaff doesn't have a local probe for it yet |
| ⚪ | **Publicly documented only** — exists in the wild, no extraction tool covers it, would require new reverse-engineering |

## DM1 (Dungeon Master) — 22+ known versions

| Platform | Version | Lang | Status | Greatstone DB | Local hash | Notes |
|---|---|---|---|---|---|---|
| Atari ST | 1.0 | EN | 🟡 | `c_dm_atari_st_v1_0/` | — | ReDMCSB source-locked; first commercial release |
| Atari ST | 1.1 | EN | 🟡 | `c_dm_atari_st_v1_1/` | — | ReDMCSB |
| Atari ST | 1.2 | EN | 🟡 | `c_dm_atari_st_v1_2/` | — | ReDMCSB |
| Atari ST | 1.2 | DE | 🟡 | `c_dm_atari_st_v1_2/` | — | ReDMCSB |
| Atari ST | 1.3 | FR | 🟡 | `c_dm_atari_st_v1_3/` | — | ReDMCSB |
| Atari ST | Teaser demo | EN | 🔵 | `c_dm_teaser_atari/` | — | demo.dat only |
| Amiga | 1.0 | EN | 🔵 | `c_dm_amiga_v1_0/` | — | earliest Amiga port |
| Amiga | 2.0 | EN | 🔵 | `c_dm_amiga_v2_0/` | — | LZW-compressed graphics |
| Amiga | 2.0 | FR | 🔵 | `c_dm_amiga_v2_0/` | — | |
| Amiga | 2.0 | GE | 🔵 | `c_dm_amiga_v2_0/` | — | cracked Rainbow |
| Amiga | 2.1 | EN | 🔵 | `c_dm_amiga_v2_1/` | — | |
| Amiga | 2.2 | EN | 🔵 | `c_dm_amiga_v2_2/` | — | + DUNGEONB.DAT ("kid" dungeon) |
| Amiga | 2.2 | GE | 🔵 | `c_dm_amiga_v2_2/` | — | |
| Amiga | 3.6 | EN/FR/GE | 🔵 | `c_dm_amiga_v3_6/` | — | 749-item GRAPHICS.DAT, multilingual |
| Apple IIGS | 1.0 | EN | 🔵 | `c_dm_iigs/` | — | IMG2 format |
| FM-Towns | 2.0 | EN/JP | 🔵 | `c_dm_fmtowns/` | — | |
| PC | 3.4 | EN | ✅ | `c_dm_pc_eng/` | yes (363,417 B) | our canonical "PC" target |
| PC | 3.4 | EN/FR/GE | ✅ | `c_dm_pc_multilingual/` | yes (398,925 B) | multilingual, 748 items |
| PC-98 | 2.0 | JP | 🔵 | `c_dm_pc98/` | — | |
| SNES | 1.0 NTSC | EN | 🔵 | `c_dm_snes/` | — | ROM .smc, per-tile palettes |
| SNES | 1.0 PAL | EN | 🔵 | `c_dm_snes/` | — | |
| SNES | 1.0 NTSC | JP | 🔵 | `c_dm_snes/` | — | |
| SNES | 1.1 NTSC | JP | 🔵 | `c_dm_snes/` | — | |
| X68000 | 3.0 | JP | 🔵 | `c_dm_x68k/` | — | 562-item GRAPHICS.DAT |

**Missing data on disk that would unlock 2️⃣ more:** any one of
the Amiga 1.0, Atari 1.0, FM-Towns 2.0, or PC-98 2.0 data files.

## CSB (Chaos Strikes Back) — 11 known versions

| Platform | Version | Lang | Status | Greatstone DB | Local hash | Notes |
|---|---|---|---|---|---|---|
| Atari ST | 2.0 | EN | 🟡 | `c_csb_atari_st_v2_0/` | yes (319,080 B graphics + 2,098 B dungeon) | ReDMCSB source-locked (incl CSB engine mods); local Atari ST v2.0 GRAPHICS/DUNGEON hashes are recorded, but full Atari-runtime parity is not yet promoted |
| Atari ST | 2.1 | EN | 🟡 | `c_csb_atari_st_v2_1/` | — | ReDMCSB |
| Amiga | 3.1 | EN | 🔵 | `c_csb_amiga_v3_1_en/` | — | |
| Amiga | 3.1 | EN/FR/GE | 🔵 | `c_csb_amiga_v3_1_ml/` | — | 749-item GRAPHICS.DAT, multilingual |
| Amiga | 3.3 | EN/FR/GE | 🔵 | `c_csb_amiga_v3_3_ml/` | — | |
| FM-Towns | 3.1 | EN | 🔵 | `c_csb_fmtowns_en/` | — | 728-item GRAPHICS.DAT |
| FM-Towns | 3.1 | JP | 🔵 | `c_csb_fmtowns_jp/` | — | |
| PC | 3.4 | EN | ✅ | — | yes (435,076 B) | Paul Stevens' unofficial Windows port (`CSBwin`) is what our `src/csb/` is based on; the underlying game is Atari ST 2.0. The PC "version" is a port, not a separate FTL release. |
| PC-98 | 3.1 | JP | 🔵 | `c_csb_pc98/` | — | |
| X68000 | 3.1 | JP | 🔵 | `c_csb_x68k/` | — | 732-item GRAPHICS.DAT |
| Apple IIGS | — | — | ❌ | not released | — | FTL never ported CSB to IIGS |

## DM2 (Dungeon Master II: The Legend of Skullkeep) — 15 tracked DMWeb/Greatstone rows

| Platform | Version | Lang | Status | Greatstone DB | Local hash | Notes |
|---|---|---|---|---|---|---|
| PC | 0.9 Beta | EN | 🔵 | `c_dm2_pc_beta/` | — | DMWeb PC page: early DOS beta line with split `GRAPHICS.DAT` + `GRAPHIC2.DAT`; no final DOS music path |
| PC | 1.0 | EN | ✅ | `c_dm2_pc/` | yes (8,639,757 B) | DMWeb PC page: Interplay 256-color DOS line, Europe/USA redump BIN/CUE CD images, USA CD-content archive, HMP music in `GRAPHICS.DAT`, VGA/SVGA MVE videos, Alt-S / Shift-arrow / keypad command table |
| PC | 1.0 | DE | 🔵 | `c_dm2_pc_de/` | — | DMWeb PC page: Germany CD and Bestseller CD lines, redump BIN/CUE and CD-content archive coverage |
| PC | 1.0 | FR | 🔵 | `c_dm2_pc_fr/` | — | DMWeb PC page: France floppy/CD/Bestseller lines and CD-content archives |
| PC | 1.0 | Demo | 🔵 | `c_dm2_pc_demo/` | — | DMWeb PC page: five known 1995 demo builds; earliest `FIRE.EXE`/LZ91 build has no music or save/load, later `SKULL.EXE`/Watcom builds vary sound, logo, title, and order assets; 3332 items in Greatstone row |
| Amiga | 1.0 | EN/FR/GE | 🔵 | `c_dm2_amiga/` | — | DMWeb Amiga edition page: Europe-only release with Germany/UK edition pages, six ADF/IPF floppy images but hard-disk install required, MOD music keyed by `CD.DAT`, WinUAE/Smacker video captures, 68020+ with OCS/ECS support, and Ctrl-S / Del-Help / keypad wall-ornate input table; 4630 items, 16-color |
| Macintosh | 1.0 | EN | 🔵 | `c_dm2_mac/` | — | DMWeb edition page: USA redump BIN/CUE CD image plus CD-content archives; upgraded 256-color graphics; QuickTime/MooV, MIDI/SoundMusicSys resources, Mac menu/balloon help, Command-key input table |
| Macintosh | 1.0 | JP | 🔵 | `c_dm2_mac_jp/` | — | DMWeb edition page: Japan redump BIN/CUE CD image plus DMFiles archive; older 16-color graphics; intro animation also present on Sega CD; CD-audio tracks; `Skullkeep` resource-fork protection notes |
| Macintosh | 1.0 | EN (demo) | 🔵 | `c_dm2_mac_demo/` | — | DMWeb edition page: USA demo redump BIN/CUE CD image plus HQX/CD-content handoff |
| PC-9801 | 1.0 | JP | 🔵 | `c_dm2_pc98/` | — | four FDI disk images; no music; PC-98 keypad / Alt-S input table |
| PC-9821 | 1.0 | JP | 🔵 | `c_dm2_pc9821/` | — | DMWeb edition page: Victor JP v1.0 BIN/CUE CD image, six CD.DAT music tracks, PC-98 keypad / Alt-S input table, LZEXE `FIRE.EXE` CD-ROM protection notes |
| IBM PS/V | 1.0 | JP | 🔵 | `c_dm2_ibmpsv/` | — | DMWeb edition page: Victor JP v1.0 three-floppy/WinImage media; no music; IBM PS/V keypad / Alt-S / Shift-arrow input table; LZEXE `FIRE.EXE` protection notes |
| Sega CD / Mega CD | 1.0 | EN | 🔵 | `c_dm2_segacd_en/` | — | DMWeb edition page: Europe + USA redump BIN/CUE CD images; USA also has DMFiles CD-content archive plus split data-track ISO and audio-track MP3 archives |
| Sega CD / Mega CD | 1.0 | JP | 🔵 | `c_dm2_segacd_jp/` | — | DMWeb edition page: Japan redump BIN/CUE CD image plus DMFiles CD-content archive; same CD.DAT trigger table, but track 7 is a 15-second silent track |
| FM-Towns | 1.0 | EN/JP | 🔵 | `c_dm2_fmtowns/` | — | DMWeb JP edition page: redump BIN/CUE CD image; 3407 items, signature 8004h; CD-audio variant with quieter tracks 2-6, silent track 8, and Ctrl-Shift-S disk menu |

**Notes:** The PC English version is the only one that uses
Interplay MVE animations and 256-color graphics. All other
versions use IMG2-style compressed bitmaps and 16-color palettes.
DMWeb's DM2 page also tracks the inventory-eye mouse-cursor bug:
all listed non-Mac platforms show the cursor/cross graphical glitch,
with Amiga retaining the cursor-loss bug after button release; Japanese
and English Macintosh versions are explicitly exempt.

## Theron's Quest — 1 known version

| Platform | Version | Lang | Status | Greatstone DB | Local hash | Notes |
|---|---|---|---|---|---|---|
| TurboGrafx-16 / PC Engine | 1.0 (CD) | JP | ✅ | `c_therons_quest_jp/` | yes (Track 02) | JP canonical + JP extras Track 02 launch to the TQR level-load milestone; screenshot readiness records metadata/hash receipts only |
| TurboGrafx-16 / PC Engine | 1.0 (CD) | EN | ✅ | `c_therons_quest_en/` | yes (Track 02) | US extras Track 02 launch to the TQR level-load milestone; full semantic dungeon-bank parity and public screenshot promotion remain separate work |

## DM Nexus — 1 known version

| Platform | Version | Lang | Status | Greatstone DB | Local hash | Notes |
|---|---|---|---|---|---|---|
| Sega Saturn | 1.0 | JP | 🔵 | `c_dm_nexus_saturn/` | yes (DM.BIN 555,144 B) | true 3D, 15 levels; hash-verified Track 1 can expose `DM.BIN`, but full Saturn V1 runtime handoff remains a Nexus gap |

**Notes:** DM Nexus uses the proprietary DMDF (Dungeon Master
Data File) format, completely different from the FTL
DUNGEON.DAT format used by DM1/CSB/DM2. Firestaff's
`src/nexus/nexus_v1_iso_reader.c` and `nexus_v1_engine.c` are
the start of the parser.

## Total coverage

| Game | Verified working | Source-locked | Greatstone-extracted | Publicly documented | Total known |
|---|---|---|---|---|---|
| DM1 | 2 (PC 3.4 EN + Multilingual) | 5 (Atari ST) | 22 (Amiga/PC-98/Apple IIGS/FM-Towns/X68000/SNES) | 0 | 22+ |
| CSB | 1 (PC 3.4 via CSBwin) | 2 (Atari ST, with local v2.0 hashes) | 11 (Amiga/PC-98/FM-Towns/X68000) | 0 | 11 |
| DM2 | 1 (PC EN) | 0 | 13 (Amiga/Mac/PC-98/PS/V/PC-9821/Sega-CD/FM-Towns/Demo/Beta) | 0 | 13+ |
| TQ | 2 (JP + EN Track 02 launch/readiness) | 0 | 0 | 0 | 2 |
| Nexus | 0 | 0 | 1 (Saturn JP) | 0 | 1 |
| **Total** | **6** | **7** | **47** | **0** | **49+** |

## What we'd need to unlock more 1️⃣/2️⃣ coverage

In rough priority order, the cheapest route to bump Firestaff
support is to add any one of these data files:

| Effort | File | Unlocks |
|---|---|---|
| Easy | DM Amiga 3.6 EN/FR/GE GRAPHICS.DAT | 1 new language (FR+GE) for DM, with all 749 items extracted (greatstone already did the work) |
| Easy | CSB Amiga 3.1 EN/FR/GE GRAPHICS.DAT | FR/GE for CSB, 749 items |
| Easy | DM Atari ST 1.0 GRAPHICS.DAT | canonical first-release DM, 532 items |
| Easy | DM Amiga 2.0 EN GRAPHICS.DAT | Amiga-specific IMG1 format, 532 items |
| Medium | DM FM-Towns 2.0 GRAPHICS.DAT | IMG2 format, 532 items |
| Medium | CSB Atari ST 2.1 GRAPHICS.DAT | second Atari CSB line; v2.0 graphics/dungeon hashes are already locally recorded |
| Medium | DM PC-9801 2.0 GRAPHICS.DAT | PC-98-specific LZW+IMG2, 575 items |
| Medium | DM2 PC-9801 1.0 FDI set | Japanese four-disk FDI media, no-music behavior, PC-98-specific keyboard bridge |
| Medium | DM2 PC-9821 1.0 BIN/CUE CD image | Japanese CD media, CD.DAT music triggers, PC-98 keyboard bridge, `FIRE.EXE` CD-ROM protection behavior evidence |
| Medium | DM2 IBM PS/V 1.0 floppy/WinImage set | Japanese three-floppy media, no-music behavior, IBM PS/V keyboard bridge, `FIRE.EXE` protection behavior evidence |
| Medium | DM2 Macintosh 1.0 BIN/CUE/CD-content set | Japanese + USA CD media, StuffIt/HQX/resource-fork handling, QuickTime/MooV animations, MIDI/SoundMusicSys resources, Mac keyboard/menu bridge |
| Medium | DM2 PC demo/build matrix | five 1995 DOS demo builds, `FIRE.EXE`/LZ91 versus `SKULL.EXE`/Watcom split, save/load and music differences, distinct `GRAPHICS.DAT`/`DUNGEON.DAT` evidence |
| Medium | DM2 Amiga 1.0 ADF/IPF hard-disk install set | Europe EN/FR/GE six-disk media, installed-hard-disk layout, MOD/CD.DAT music triggers, Amiga keyboard bridge, WinUAE/Smacker video evidence |
| Medium | DM2 FM Towns 1.0 BIN/CUE CD image | Japanese CD media, CD.DAT music triggers, quieter Red Book tracks 2-6, silent track 8, FM Towns-specific keyboard bridge |
| Medium | DM2 Sega CD / Mega CD 1.0 BIN/CUE CD image | Europe/USA/Japan CD media, split data-track ISO + audio-track archive evidence, track-7 silence variant, Sega-CD-specific runtime/input bridge |
| Hard | CSB X68000 3.1 GRAPHICS.DAT | X68000-specific, 732 items |
| Hard | DM2 Amiga 1.0 GRAPHICS.DAT | 4630 items, 16-color |
| Hard | Real Theron `.srm` save artifact | unlocks Track 02 save import/export evidence beyond the current launch/readiness gates |
| Very hard | Nexus Saturn NEXUS.BIN | already have, but the .MNS, .AVI companions are large |

## Adding a new version to Firestaff

When new data becomes available, here's the typical
checklist (covered in `docs/DMWEB_REFERENCE.md`):

1. **Hash the new data** — `sha256sum` it, add to `docs/VERIFIED_HASHES.md`
2. **Run sck extraction** — download Pierre Monnot's sck JAR
   from `http://greatstone.free.fr/dm/t_download.html`,
   extract the new file, compare the sck's output to Firestaff's
   asset loader output
3. **Add a probe** — if there's a new format version (e.g., the
   FM-Towns DM uses IMG2 + LZW), add an integration test under
   `probes/` that exercises the parser against the new data
4. **Update i18n** — if the new data has new in-game text,
   extract it via `tools/extract_dm1_strings.py` (or equivalent)
   and add to `po/dm1.pot`
5. **Document** — add a row to this matrix and a news entry to
   `RELEASE_NOTES.md` if it's a public-facing change

## See also

- `docs/DMWEB_REFERENCE.md` — the dmweb + greatstone survey
- `docs/VERIFIED_HASHES.md` — the 148 SHA256 checksums for our
  local data files
- `docs/REDMCSB_REFERENCE.md` — Meynaf's decompiled C source
- `docs/dm2_platform_*.md` and `docs/nexus_platform.md` — the
  per-game deep-dive platform audits
- `src/shared/asset_find_by_hash.c` — the runtime asset
  discovery probe (uses MD5; mismatch with VERIFIED_HASHES.md
  SHA256 is a known issue, see `docs/PLATFORM_MATRIX.md` and
  `docs/VERIFIED_HASHES.md` for context)
