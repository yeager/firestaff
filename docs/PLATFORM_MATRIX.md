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
| 🔵 | **Greatstone-extracted** — Pierre Monnot's sck tool has decoded this version, data is publicly browsable at `http://greatstone.free.fr/dm/db_data/`, and `tools/verify_greatstone_db_data_paths.py` (CTest `greatstone_db_data_paths_probe`, OFFLINE by default with a metadata-only fixture) keeps the curated current-path set + the obsolete `c_dm_*` / `c_csb_*` / guessed-DM2 404-regression set in sync; per-row evidence lives at `parity-evidence/verification/greatstone_db_data_paths_probe/manifest.json` |
| ⚪ | **Publicly documented only** — exists in the wild, no extraction tool covers it, would require new reverse-engineering |

## DM1 (Dungeon Master) — 22+ known versions

| Platform | Version | Lang | Status | Greatstone DB | Local hash | Notes |
|---|---|---|---|---|---|---|
| Atari ST | 1.0 | EN | 🟡 | `c_dm_atari_st_v1_0/` | — | ReDMCSB source-locked; DMWeb page: first commercial release, 1987-12-15 US date, two known 1.0 STX builds dated 1987-12-08 and 1987-12-11 |
| Atari ST | 1.1 | EN | 🟡 | `c_dm_atari_st_v1_1/` | — | ReDMCSB; DMWeb page lists preserved STX plus cracked MSA and hard-disk/RamDisk hack derivatives |
| Atari ST | 1.2 | EN | 🟡 | `c_dm_atari_st_v1_2/` | — | ReDMCSB; DMWeb page lists preserved STX; Atari input uses Insert/Clr Home turns and Ctrl-S disk menu |
| Atari ST | 1.2 | DE | 🟡 | `c_dm_atari_st_v1_2/` | — | ReDMCSB; DMWeb page lists preserved German 1.2 STX and German/Psygnosis bundle provenance |
| Atari ST | 1.3 | FR | 🟡 | `c_dm_atari_st_v1_3/` | — | ReDMCSB; DMWeb page lists French 1.3 with two variants and French Mirrorsoft edition/bundle provenance |
| Atari ST | Teaser demo | EN | 🔵 | `c_dm_teaser_atari/` | — | demo.dat only; DMWeb page keeps teaser as a separate Atari ST edition |
| Amiga | 1.0 | EN | 🔵 | `c_dm_amiga_v1_0/` | — | earliest Amiga port in Greatstone corpus; DMWeb edition page starts the public Amiga release notes at 2.0-era 1988 media |
| Amiga | 2.0 | EN | 🔵 | `c_dm_amiga_v2_0/` | — | DMWeb page: November 1988 line, USA/UK import provenance, fastest blitter-backed port, ADF/IPF media, 1 MB RAM, Kickstart 1.2/1.3 assumptions |
| Amiga | 2.0 | FR | 🔵 | `c_dm_amiga_v2_0/` | — | DMWeb page: France Mirrorsoft edition, protected original ADF plus Meynaf cracked ADF/protection notes |
| Amiga | 2.0 | GE | 🔵 | `c_dm_amiga_v2_0/` | — | DMWeb page: German Mirrorsoft editions, unofficial IPF/cracked ADF evidence; cracked images stay non-canonical |
| Amiga | 2.1 | EN | 🔵 | `c_dm_amiga_v2_1/` | — | DMWeb page: February 1989 line, original ADF does not boot because of copy protection |
| Amiga | 2.2 | EN | 🔵 | `c_dm_amiga_v2_2/` | — | DMWeb page: September 1989 line; + DUNGEONB.DAT ("kid" dungeon); unofficial IPF and protected original ADF evidence |
| Amiga | 2.2 | GE | 🔵 | `c_dm_amiga_v2_2/` | — | DMWeb page: protected original ADF evidence and German media provenance |
| Amiga | 3.6 | EN/FR/GE | 🔵 | `c_dm_amiga_v3_6/` | — | DMWeb page: June 1992 line, official SPS IPF not copy protected, 749-item multilingual GRAPHICS.DAT, changed title/menu/perspective versus 2.0, distinct 3.6 keypad layout |
| Apple IIGS | 2.0 | EN | 🔵 | `c_dm_iigs/` | — | DMWeb page: 1989 line, PO disk images, 1 MB RAM, ROM03 main-dungeon `->002C` failure unless expansion memory is present, original images lack copy-protection sector |
| Apple IIGS | 2.1 | EN | 🔵 | `c_dm_iigs/` | — | DMWeb page: fixes the ROM03 2.0 main-dungeon memory failure; separate original/cracked PO images and IIGS keypad/speaker/volume hotkeys |
| FM-Towns | 2.0 | EN/JP | 🔵 | `c_dm_fmtowns/` | — | DMWeb page: Japan CD line, November 1989, ¥8800, redump BIN/CUE masters T1/T2 plus ISO/CUE archive, English/Japanese screenshots, CD audio tracks 02-20, and FM Towns Shift-S / shifted-arrow command table. Bounded redump-layout classifier (`firestaff_fmtowns_cd_classify`, CTest `firestaff_fmtowns_cd_classify_unit`) parses redump CUE sheets and scores candidate disc images against the DMWeb-documented 17..19-audio-track table; no game data vendored |
| PC | 3.4 | EN | ✅ | `c_dm_pc_eng/` | yes (363,417 B) | canonical PC target; DMWeb page: 1992 DOS line, USA English media, VGA/EGA screenshots, PC-only ending animation, entrance music excerpt, and DOS keypad / Alt-S command table |
| PC | 3.4 | EN/FR/GE | ✅ | `c_dm_pc_multilingual/` | yes (398,925 B) | multilingual, 748 items; DMWeb page: Europe English/French/German package plus French/German VGA screenshots and Spanish fan-translation provenance kept separate |
| PC-9801 | 2.0a/2.0b | JP | 🔵 | `c_dm_pc98/` | — | DMWeb page: Japan 3.5-inch and 5.25-inch HDM/floppy line, 1990-02-09, ¥9800, 8-bit/16-bit screenshots, `2.0a` copy-protected original plus cracked image, and newer non-copy-protected `2.0b` original |
| SNES | 1.0 NTSC | EN | 🔵 | `c_dm_snes/` | — | DMWeb page: SMC cartridge ROM, English screenshots/videos, SPC music, and English-only "master" skill-prefix bug |
| SNES | 1.0 PAL | EN | 🔵 | `c_dm_snes/` | — | DMWeb page: Europe cartridge line; same English-version bug family, PAL timing needs separate runtime proof |
| SNES | 1.0 NTSC | JP | 🔵 | `c_dm_snes/` | — | DMWeb page: Super Famicom ROM, Japanese screenshots/videos, alternate-ending save-corruption bug, Hissssa redraw bug, and See Through Walls freeze |
| SNES | 1.1 NTSC | JP | 🔵 | `c_dm_snes/` | — | DMWeb page: fixes Japanese 1.0 alternate-ending save corruption and Hissssa redraw, but still has the Japanese See Through Walls freeze |
| X68000 | 3.0 | JP | 🔵 | `c_dm_x68k/` | — | DMWeb page: Japan HDM/floppy line, 1990-01-26, ¥9800, 31 kHz screenshots, original image missing copy-protection sectors, cracked image, save disk, and X68000 keypad / Ctrl-S command table. **2026-06-25 bounded classifier:** `firestaff_x68k_media_classify_unit` PASS 12/12 covers 2DHD geometry (1232 KB), `HPR-0007` sentinel detection, blank save-disk shape, and FTL-magic handoff; `firestaff_x68k_ftl_handoff_unit` PASS 5/5 covers FTL `data_area1_memory_size` size-fit against the HDM.

**DMWeb overview boundary:** the DM overview page is the umbrella
source for the cross-port differences that sit above the individual
edition pages. It pins Atari ST as the original development/release
family, PC 3.4 as the site's reference-data basis, and the other ports
as distinct runtime families with their own presentation and behavior:
two dungeon-view perspective groups, mono/stereo and missing-effect
sound splits, Kid Dungeon only on Apple IIGS 2.0/2.1 and Amiga 2.2 EN,
PC-only entrance music and ending animation, Atari ST 1.0/v1.2 item and
regeneration differences, X68000 larger view, PC-9801 reduced palette
with three light levels, SNES redrawn graphics/music/introduction, and
FM Towns CD audio.

**Missing data on disk that would unlock 2️⃣ more:** any one of
the Amiga 1.0, Atari 1.0, FM-Towns 2.0, PC-98 2.0, or X68000 3.0
data files.

## CSB (Chaos Strikes Back) — 12 known versions

| Platform | Version | Lang | Status | Greatstone DB | Local hash | Notes |
|---|---|---|---|---|---|---|
| Atari ST | 2.0 | EN | 🟡 | `c_csb_atari_st_v2_0/` | yes (319,080 B graphics + 2,098 B dungeon) | ReDMCSB source-locked (incl CSB engine mods); DMWeb Atari ST page: December 1989 line, original STX game+utility disks, cracked ST/MSA media, save disk, hard-disk/RamDisk hacks, intro video, no ending animation, and Atari Insert/Clr Home/Ctrl-S command table; local v2.0 game-disk hashes are recorded, but full Atari-runtime parity is not yet promoted |
| Atari ST | 2.1 | EN | 🟡 | `c_csb_atari_st_v2_1/` | — | ReDMCSB; DMWeb page lists original STX game+utility disks plus Meynaf's v2.1 assembler-source disassembly, so v2.1 needs separate media/hash and utility-disk classification |
| Amiga | 3.1 | EN | 🔵 | `c_csb_amiga_v3_1_en/` | — | DMWeb Amiga page: December 1990 line, separate English media, v3.1 screenshots, unofficial IPF/protected ADF/cracked ADF boundaries, WinUAE/Smacker intro+ending, and Amiga keypad/Del/Help/Ctrl-S input |
| Amiga | 3.1 | EN/FR/GE | 🔵 | `c_csb_amiga_v3_1_ml/` | — | 749-item GRAPHICS.DAT, multilingual; DMWeb separates unofficial IPF, protected original ADF, imperfect cracked ADF, and utility disks by language/release |
| Amiga | 3.3 | EN/FR/GE | 🔵 | `c_csb_amiga_v3_3_ml/` | — | DMWeb lists multilingual v3.3 media plus Meynaf's French hard-disk/accelerator-card hack; Firestaff's verified extra path is a hack variant, not a canonical original |
| Amiga | 3.5 | EN | ✅ | — | yes (435,076 B graphics + 2,098 B dungeon) | canonical CSB hash set; DMWeb lists protected original ADF/IPF media, WinUAE/Smacker intro+ending evidence, and Amiga input table |
| Amiga | 3.5 | EN/FR/GE | ⚪ | — | — | DMWeb lists multilingual v3.5 media, v3.5 language-choice/entrance screenshots with Quit button, and v3.5 multilingual Ctrl-Q/Ctrl-A entrance quit behavior; needs separate canonical hash classification |
| FM-Towns | 3.1 | EN | 🔵 | `c_csb_fmtowns_en/` | — | 728-item GRAPHICS.DAT; DMWeb page: Japan BIN/CUE CD line, English screenshots, CD audio tracks 02-31, and FM Towns Ctrl-S/Shift-S plus shifted-arrow input |
| FM-Towns | 3.1 | JP | 🔵 | `c_csb_fmtowns_jp/` | — | DMWeb page: Japan v3.1, release 1990-12-14, ¥9800, Japanese screenshots, Champion Editor/portrait-loading screens, redump.org and DMFiles BIN/CUE sources |
| PC | 3.4 | EN | ✅ | — | yes (435,076 B) | Paul Stevens' unofficial Windows port (`CSBwin`) is what our `src/csb/` is based on; the underlying game is Atari ST 2.0. The PC "version" is a port, not a separate FTL release. |
| PC-98 | 3.1 | JP | 🔵 | `c_csb_pc98/` | — | DMWeb PC-9801 page: Japan HDM/floppy line, 1990-12-21, ¥9800, original image missing copy-protection sectors, cracked image, 8-bit/16-bit screenshots, Champion Editor/portrait-loading screens, and PC-98 keypad / Alt-S input |
| X68000 | 3.1 | JP | 🔵 | `c_csb_x68k/` | — | 732-item GRAPHICS.DAT; DMWeb X68000 page: Japan HDM/floppy line, 1990-12-21, ¥9800, original image missing copy-protection sectors, `CK.R` cracked image, save disk, 24/31 kHz screenshots, Champion Editor/portrait-loading screens, and X68000 Ctrl-S / Opt.1 / Opt.2 input |
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
| TurboGrafx-16 / PC Engine | 1.0 (CD) | JP | ✅ | `c_therons_quest_jp/` | yes (Track 02) | DMWeb overview: Japan release 1992-09-18, PC Engine CD adaptation with seven small dungeons; edition pages list bar code `4 988002 247950`, ¥8200 retail price, 142x125x10 mm jewel-case package, booklet/back-card/spine-card/CD scans, JP redump BIN/CUE, CloneCD, DMFiles ISO/OGG, intro/outro video, and JP audio tracks 01/03-18 including the Track 17 static-noise defect; JP canonical + JP extras Track 02 launch to the TQR level-load milestone, while screenshot readiness records metadata/hash receipts only |
| TurboGrafx-16 / PC Engine | 1.0 (CD) | EN | ✅ | `c_therons_quest_en/` | yes (Track 02) | DMWeb overview: USA release in 1993 and reference data based on English version; edition pages list bar code `0 92218 00178 8`, 142x125x10 mm jewel-case package, box hook-back, booklet/back-card/CD scans, USA redump BIN/CUE, CloneCD, DMFiles ISO/OGG, intro/outro video, and fixed Track 17 audio; US extras Track 02 launch to the TQR level-load milestone, with full semantic dungeon-bank/audio parity and public screenshot promotion kept separate |

## DM Nexus — 1 known version

| Platform | Version | Lang | Status | Greatstone DB | Local hash | Notes |
|---|---|---|---|---|---|---|
| Sega Saturn | 1.0 | JP | 🔵 | `c_dm_nexus_saturn/` | yes (DM.BIN 555,144 B) | DMWeb page: Japan-only Saturn release by Victor Interactive Software, 1998-03-26, ¥6800, catalog `T-9111G`, true 3D, 15 levels, CD audio tracks 02-09, Japanese screenshots/manual scans/playthrough, Japanese demo plus unofficial EN/FR fan translations. **2026-06-25:** hash-verified Track 1 is now the source the engine actually opens (`nexus_iso_open_cue` parses the first CUE FILE entry), and `firestaff_nexus_v1_track1_phase_launch_probe` is CTest-gated as `nexus_v1_track1_phase_launch_{synthetic,extracted_root,saturn_ja_iso}` (PASS 3/3) so the E1 V1 phase-launch path (engine init, DGN load, DMDF MNS load, S2D font parse, BPK archive) is provably driven by Track 1; demo and EN/FR fan-translation media lines remain classified separately. Full Nexus V1 runtime/playability + public screenshot promotion remain tracked gaps |

**Notes:** DM Nexus uses the proprietary DMDF (Dungeon Master
Data File) format, completely different from the FTL
DUNGEON.DAT format used by DM1/CSB/DM2. Firestaff's
`src/nexus/nexus_v1_iso_reader.c` and `nexus_v1_engine.c` are
the start of the parser. DMWeb keeps the official Japanese release,
the Japanese demo, and the 2023-2024 English/French fan translations
as separate media lines; Firestaff should do the same when adding
future hashes or launch profiles.

## Total coverage

| Game | Verified working | Source-locked | Greatstone-extracted | Publicly documented | Total known |
|---|---|---|---|---|---|
| DM1 | 2 (PC 3.4 EN + Multilingual) | 5 (Atari ST) | 22 (Amiga/PC-98/Apple IIGS/FM-Towns/X68000/SNES) | 0 | 22+ |
| CSB | 2 (CSBwin PC port + Amiga 3.5 EN hash set) | 2 (Atari ST, with local v2.0 hashes) | 7 (Amiga 3.1/3.3, PC-98, FM-Towns, X68000) | 1 (Amiga 3.5 multilingual) | 12 |
| DM2 | 1 (PC EN) | 0 | 13 (Amiga/Mac/PC-98/PS/V/PC-9821/Sega-CD/FM-Towns/Demo/Beta) | 0 | 13+ |
| TQ | 2 (JP + EN Track 02 launch/readiness) | 0 | 0 | 0 | 2 |
| Nexus | 0 | 0 | 1 (Saturn JP) | 0 | 1 |
| **Total** | **7** | **7** | **43** | **1** | **50+** |

## What we'd need to unlock more 1️⃣/2️⃣ coverage

In rough priority order, the cheapest route to bump Firestaff
support is to add any one of these data files:

| Effort | File | Unlocks |
|---|---|---|
| Easy | DM Amiga 3.6 EN/FR/GE GRAPHICS.DAT | 1 new language (FR+GE) for DM, with all 749 items extracted; DMWeb identifies official SPS IPF media that is not copy protected |
| Easy | CSB Amiga 3.1 EN/FR/GE GRAPHICS.DAT | FR/GE for CSB, 749 items |
| Easy | DM Atari ST 1.0 GRAPHICS.DAT | canonical first-release DM, 532 items |
| Easy | DM Amiga 2.0 EN GRAPHICS.DAT | Amiga-specific IMG1 format, 532 items; DMWeb page adds ADF/IPF provenance, 1 MB/Kickstart boundary, and Amiga 2.x keypad/Del/Help input table |
| Medium | DM Apple IIGS 2.0/2.1 PO disk images | Apple IIGS ProDOS/PO import, 2.0 ROM03 memory-failure boundary, 2.1 fix evidence, IIGS keyboard/audio hotkeys, copy-protection/crack classification |
| Medium | DM FM-Towns 2.0 BIN/CUE or ISO/CUE CD image | English/Japanese v2.0 hash classification, IMG2 real-asset evidence, CD audio tracks 02-20, and FM Towns keyboard bridge |
| Medium | CSB Atari ST 2.1 GRAPHICS.DAT | second Atari CSB line; v2.0 graphics/dungeon hashes are already locally recorded |
| Medium | CSB FM Towns 3.1 BIN/CUE CD image | English/Japanese v3.1 hash classification, CD audio tracks 02-31, Champion Editor/portrait-loading receipts, and FM Towns keyboard bridge |
| Medium | DM PC-9801 2.0a/2.0b HDM/floppy images | Japanese v2.0 hash classification, 8-bit/16-bit presentation evidence, PC-98 keypad / Alt-S input gates, and copy-protection/crack separation |
| Medium | DM X68000 3.0 HDM/floppy images | Japanese v3.0 hash classification, 31 kHz presentation evidence, X68000 keypad / arrow input gates, and copy-protection/crack separation. **2026-06-25 bounded classifier added:** `firestaff_x68k_media_classify_unit` PASS 12/12 + `firestaff_x68k_ftl_handoff_unit` PASS 5/5 lock down the DMWeb 2DHD geometry (1232 KB), the `HPR-0007` Track 1 Side 1 Sector 9 sentinel, the blank save-disk detection, and the FTL `data_area1_memory_size` handoff boundary. Public-DMFiles HDM without the protection sentinel stays honest as a documented expected state. |
| Medium | DM PC 3.4 VGA/EGA and PC-only presentation receipts | Existing hash-verified PC runtime target; add focused receipts for DMWeb's VGA/EGA split, PC ending animation, entrance music excerpt, DOS keypad / Alt-S / analog-joystick / mouse-simulation input table, and Spanish fan-translation non-canonical boundary |
| Medium | DM1 overview version-comparison receipts | Cross-port proof from DMWeb's overview page: two perspective families, sound/stereo/missing-effect behavior, fountain/wall-click differences, Kid Dungeon gates, Atari ST-vs-PC dungeon deltas, PC-only ending/music, PC-9801 light-level boundary, SNES presentation differences, and FM Towns CD-audio receipts |
| Medium | DM2 PC-9801 1.0 FDI set | Japanese four-disk FDI media, no-music behavior, PC-98-specific keyboard bridge |
| Medium | DM2 PC-9821 1.0 BIN/CUE CD image | Japanese CD media, CD.DAT music triggers, PC-98 keyboard bridge, `FIRE.EXE` CD-ROM protection behavior evidence |
| Medium | DM2 IBM PS/V 1.0 floppy/WinImage set | Japanese three-floppy media, no-music behavior, IBM PS/V keyboard bridge, `FIRE.EXE` protection behavior evidence |
| Medium | DM2 Macintosh 1.0 BIN/CUE/CD-content set | Japanese + USA CD media, StuffIt/HQX/resource-fork handling, QuickTime/MooV animations, MIDI/SoundMusicSys resources, Mac keyboard/menu bridge |
| Medium | DM2 PC demo/build matrix | five 1995 DOS demo builds, `FIRE.EXE`/LZ91 versus `SKULL.EXE`/Watcom split, save/load and music differences, distinct `GRAPHICS.DAT`/`DUNGEON.DAT` evidence |
| Medium | DM2 Amiga 1.0 ADF/IPF hard-disk install set | Europe EN/FR/GE six-disk media, installed-hard-disk layout, MOD/CD.DAT music triggers, Amiga keyboard bridge, WinUAE/Smacker video evidence |
| Medium | DM2 FM Towns 1.0 BIN/CUE CD image | Japanese CD media, CD.DAT music triggers, quieter Red Book tracks 2-6, silent track 8, FM Towns-specific keyboard bridge |
| Medium | DM2 Sega CD / Mega CD 1.0 BIN/CUE CD image | Europe/USA/Japan CD media, split data-track ISO + audio-track archive evidence, track-7 silence variant, Sega-CD-specific runtime/input bridge |
| Hard | CSB X68000 3.1 HDM/floppy images | X68000-specific 732-item GRAPHICS.DAT, 24/31 kHz presentation receipts, Champion Editor/portrait-loading proof, X68000 Ctrl-S / Opt.1 / Opt.2 input gates, and original/cracked/save-disk classification |
| Hard | DM SNES / Super Famicom SMC ROM set | four cartridge versions, per-tile palettes, SPC music, console input, and SNES-specific bug gates |
| Hard | DM2 Amiga 1.0 GRAPHICS.DAT | 4630 items, 16-color |
| Hard | Real Theron `.srm` save artifact | unlocks Track 02 save import/export evidence for DMWeb's between-dungeon save model beyond the current launch/readiness gates |
| Very hard | Nexus Saturn full Track 1 / fan-translation matrix | original JP Track 1 is present, but full-disc runtime handoff, CD audio tracks 02-09, demo classification, and EN/FR fan-translation hashes still need proof |

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
- `tools/verify_greatstone_db_data_paths.py` +
  `parity-evidence/verification/greatstone_db_data_paths_probe/manifest.json` —
  the bounded regression gate for the current reachable
  `db_data/` paths and the obsolete `c_dm_*` / `c_csb_*` /
  guessed-DM2 404 paths. See FIRESTAFF_GAP_LIST.md A5.
- `docs/REDMCSB_REFERENCE.md` — Meynaf's decompiled C source
- `docs/dm2_platform_*.md` and `docs/nexus_platform.md` — the
  per-game deep-dive platform audits
- `src/shared/asset_find_by_hash.c` — the runtime asset
  discovery probe (uses MD5; mismatch with VERIFIED_HASHES.md
  SHA256 is a known issue, see `docs/PLATFORM_MATRIX.md` and
  `docs/VERIFIED_HASHES.md` for context)
