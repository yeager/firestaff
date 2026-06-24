# DMWeb Encyclopaedia & Greatstone Reference

The **Dungeon Master Encyclopaedia** at <http://dmweb.free.fr/>
and **Greatstone's Swoosh Construction Kit** site at
<http://greatstone.free.fr/dm/> are the two highest-quality
community references for the original FTL Games data. This
document indexes every page we have reviewed so far, with notes
on what each contributes to Firestaff.

## Why these two sites matter

| Site | What it gives us |
|---|---|
| **dmweb.free.fr** | Whole-game encyclopaedia: per-platform release matrix, magazine scans, awards, custom dungeon galleries, FAQ for each platform's quirks, and — most importantly — **byte-level file format specs** for DUNGEON.DAT, GRAPHICS.DAT, animations, data files, save games, and DMII variants. |
| **greatstone.free.fr/dm/** | Pierre Monnot's "Swoosh Construction Kit" (sck) project: a Java tool that has **already extracted every asset from 26+ commercial versions** of DM and CSB (Amiga 1.0/1.1/1.2/1.3/2.0/2.1/2.2/3.6, Atari, Apple IIGS, FM-Towns, PC-98, PC 3.4, SNES, X68000, CSB Amiga, CSB Atari, CSB FM-Towns, CSB PC-98, CSB X68000, DMII Amiga/Mac/PC/Sega-CD/FM-Towns/PC-98/IBM PSV/PC-9821, Theron's Quest, DM Nexus, Black Crypt, R-Type III GBA). Specs for the FTL container format, the PAK compression format, and the IMG5 4bpp chunked-image format that underlies most of the engine. |

Combined, these are the definitive "ground truth" for any
Dungeon Master implementation. Where ReDMCSB gives us
decompiled C source for DM/CSB Atari ST, dmweb/greatstone give
us the platform matrix and the file-level wire format for
*every* version, including the ones we don't have local assets
for (DMII, Nexus, Theron's Quest).

---

## dmweb.free.fr — pages reviewed

### Game pages

| URL | What's there | What we use it for |
|---|---|---|
| `http://dmweb.free.fr/games/dungeon-master/` | DM: 8 platforms, 4 languages, 30+ year magazine article archive (1986-1996), 200+ award scans, FTL team credits, original DM Atari ST dev timeline (Feb 1986 → Dec 15 1987), Doug Bell D&D-comments interview, **per-version data file spec summaries** | Confirms which data files we need to support per platform; lists Atari ST as canonical first release (PC 3.4 English is what the "PC" platform data files map to in our compatibility layer) |
| `http://dmweb.free.fr/games/dungeon-master/editions/atari-st/` | DM Atari ST edition page: documents the first commercial line in detail. It lists the teaser demo, USA early box/USA releases, French Mirrorsoft first/second editions, United Kingdom Mirrorsoft second edition, DM+CSB bundles, and later Psygnosis bundles. The page pins the US first release date to 1987-12-15 and identifies known runtime versions: English 1.0 builds dated 1987-12-08 (no version number in dialogs) and 1987-12-11 (shows `1.0`), English 1.1, English 1.2, German 1.2, and French 1.3 with two variants. Downloads include preserved STX disk images for the six listed versions, cracked ST/MSA images, hard-disk hacks, RamDisk hacks, custom-dungeon RamDisk workflows, and the sector `#F7` floppy copy-protection boundary. The keyboard table is Atari-specific: F1-F4 champion inventory, Escape freeze/unfreeze, Return wake while resting, Ctrl-S disk menu, Insert/Clr Home turn left/right, and arrows for forward/side/back movement. | Pin DM1 Atari ST as the canonical source-era family, not just a generic "ReDMCSB" ancestor. Future work needs STX/ST/MSA media ingestion, `START.PAK` and LZW real-asset gates, per-version hash classification for 1.0/1.1/1.2/1.3, copy-protection-aware extraction notes, and an Atari ST input bridge for Insert/Clr Home/Ctrl-S before promoting any native Atari runtime claim. |
| `http://dmweb.free.fr/games/dungeon-master/editions/amiga/` | DM Amiga edition page: documents demo, USA 1988, France/Germany/United Kingdom Mirrorsoft releases, AmiRAM/TecnoPlus bundles, USA 1992, Psygnosis bundles, Australia, and Poland. It lists screenshots for 2.0 and 3.6 in English/French/German and dates the Amiga line as November 1988 (2.0), February 1989 (2.1), September 1989 (2.2), and June 1992 (3.6). Known versions are 2.0 demo/EN/FR/DE, 2.1 EN, 2.2 EN/DE, and 3.6 EN/FR/DE. The page calls out Amiga blitter speed, 1 MB RAM requirement, Kickstart 1.2/1.3 expectations for 2.x, later-model workarounds, and the cancelled 512 KB path. Downloads distinguish official SPS IPF 3.6 (not copy protected), unofficial IPFs for 2.0/2.2, ADF originals that may not boot because of copy protection, cracked FR/DE images, a partly cracked hard-disk ADF, WHDLoad/Aminet hard-disk patches, and Meynaf's Atari ST 1.2-to-Amiga port. Keyboard commands are Amiga-specific: F1-F4 champion inventory, Escape freeze/unfreeze, Return wake, Ctrl-S disk menu, keypad/Del/Help/arrows for 2.x movement, a different 3.6 keypad layout, entrance Return, Ctrl-Q/Ctrl-A quit, plus a demo-only Shift-arrow swap warning. The crack notes identify protection reads, checksums, save-function checks, `swoosh` master-disk validation, disk-presence checks, and hidden protection value `0C91` stored in `graphics.dat` item 558. | Pin DM1 Amiga as its own protected-floppy/import family rather than a PC or Atari alias. Future work needs IPF/ADF ingestion, original-vs-cracked-vs-hard-disk classification, real `GRAPHICS.DAT`/`DUNGEON.DAT` extraction by version, Amiga IMG/hidden-item coverage, Amiga keyboard gates for 2.x and 3.6, copy-protection provenance notes without committing patched executables, and hardware assumptions for Kickstart/RAM before any native Amiga runtime claim. |
| `http://dmweb.free.fr/games/dungeon-master/editions/apple-iigs/` | DM Apple IIGS edition page: documents the 1989 Apple IIGS line with known English versions 2.0 and 2.1. It states the 1 MB RAM requirement and records a ROM03-specific 2.0 failure: Kid dungeon loads, but the main dungeon can fail with `Dungeon Master fatal error ->002C`; version 2.1 or expansion memory fixes it. The download set is PO disk images: original 2.0/2.1 copies that cannot boot because the copy-protection sector is absent, Computist-cracked 2.0/2.1 images with Dungeon Master Cheat 1.1 enabled, an ACS-cracked 2.0 image, an ACS + Cheat CDA + hard-disk-install image started through `DM.HARD.DISK/DM20/DM.START`, and an empty save disk. The keyboard table is Apple IIGS-specific: number-row 1-4 champion inventory, Space leader inventory, Escape freeze/unfreeze, Return wake, Ctrl-S disk menu, keypad 4/5/6 turn/forward/turn, keypad 1/2/3 side/back/side, keypad `*` speaker toggle, and keypad `+`/`-` volume. Crack notes identify fuzzy-bit block `$17`, `SYSTEM/START` `PDosInt`, `DUNGEON.MASTER` `F0003_MAIN_ProcessNewPartyMap_CPSE`, and routine `F2150_` as protection-relevant areas. | Pin DM1 Apple IIGS as a PO-disk-image target with 2.0/2.1, ROM01/ROM03 memory behavior, IIGS-specific keyboard/audio hotkeys, and copy-protection provenance. Future work needs PO/ProDOS ingestion, original-vs-cracked-vs-hard-disk classification, file extraction/hash coverage, emulator/hardware assumptions, IIGS input gates, and protection notes without committing patched binaries or treating cracked media as canonical originals. |
| `http://dmweb.free.fr/games/chaos-strikes-back/` | CSB: full encyclopedia, Atari ST Amiga X68000 PC-9801 FM-Towns (no PC, no Apple IIGS — Don Jordan who did the GS port had left FTL by then), full Hint Oracle, plus Paul Stevens' unofficial PC port of CSB that he made in 2002 by disassembling the Atari ST binary. **Per-version differences** (endgame animation, X68000 has FM Music, PC-9801 has 3 light levels, Atari ST can't drink from potion) | The CSB platforms we support; the per-platform "what's different" list is invaluable for our compatibility layer; **Paul Stevens is the same author as CSBwin** which is the basis of our `src/csb/` work |
| `http://dmweb.free.fr/games/dungeon-master-ii/` | DM2: release/platform matrix for PC-9801, FM-Towns, Mega CD/Sega CD, PC-9821, IBM PS/V, Macintosh, PC, and Amiga, plus edition pages for PC beta/demo/regional CD/floppy variants and the version-comparison table with **per-version movement / graphics / music / savegame differences**. CRITICAL: PC English and Mac are Interplay ports with 256-color graphics, while all other versions are 16-color. Mac uses Apple QuickTime .moov for animations, PC uses Interplay MVE format. PC 0.9 Beta and PC demo use older GRAPHICS.DAT with simpler compression. The page also documents the inventory-eye mouse-cursor bug: all listed non-Mac platforms show the cursor/cross glitch, and Amiga has the severe cursor-not-restored variant; Japanese and English Macintosh versions are exempt. | Use for DM2 S0/S1/S2 platform scoping, edition/hash backlog, animation/media split, and a future DM2 inventory mouse-route regression gate. The 256-color Interplay port is what the "PC" data files most players will have; non-PC families need separate asset/runtime paths. |
| `http://dmweb.free.fr/games/dungeon-master-ii/editions/pc/` | DM2 PC edition page: tracks PC 0.9 beta, USA compact-disc demo, USA compact-disc / RSAC / jewel-case lines, Australia/France/Germany/Poland/United Kingdom CD or floppy editions, Czechia magazine demo, and a Spanish fan translation. The page documents final DOS music as 29 HMP files embedded in `GRAPHICS.DAT` with MIDI conversions, PC/Mac track-index comparisons, VGA/SVGA MVE video assets, and a DOS command table: number-row champion inventory, Space leader inventory, Escape freeze/unfreeze, Return wake, Alt-S disk menu, keypad/arrows/Shift-arrows movement, keypad 7/8/9 wall-ornate/button hotkeys, entrance Return, and Ctrl-Q/Ctrl-A quit. It also records official sound patches, HMI/GUS driver updates, and five known 1995 demo builds, including the early `FIRE.EXE`/LZ91 demo with no music or save/load and later `SKULL.EXE`/Watcom demos with different `GRAPHICS.DAT`, `DUNGEON.DAT`, sound, logo, title, and ordering assets. | Pin the DOS line as the current DM2 runtime family while keeping regional CD/floppy/demo/media differences explicit. Future work needs per-edition hash coverage, demo-version classification, HMP/MVE evidence, HMI patch provenance, and DOS command-table gates without treating fan translations or patched drivers as bundled game data. |
| `http://dmweb.free.fr/games/dungeon-master-ii/editions/amiga/` | DM2 Amiga edition page: states Dungeon Master II for Amiga was released only in Europe, with Germany and United Kingdom edition pages and a single Europe v1.0 game line for English/French/German. Downloads are six ADF floppy images and six IPF floppy images, but the page notes the game cannot be played from floppy disks and must be installed to hard disk. It documents MOD music (`SK00.MOD` through `SK09.MOD`) keyed by the `CD.DAT` track index, WinUAE-captured Smacker videos, hardware reality notes (68020+ required, AGA not required, OCS/ECS works, music works with 2 MB RAM), and an undocumented Amiga keyboard table: F1-F4 champion inventory, Space leader inventory, Escape freeze/unfreeze, Return wake, Ctrl-S disk menu, Del/Up/Help/Left/Down/Right movement, numeric-pad `(`/`)`/`/` wall-ornate or button hotkeys, entrance Return, and Ctrl-Q/Ctrl-A quit. | Pin DM2 Amiga as a hard-disk-installed Amiga floppy-image target with MOD music and Amiga-specific keyboard/video behavior, not a DOS/Mac/Sega-CD media alias. Future support needs ADF/IPF ingestion, installed-hard-disk layout evidence, MOD/CD.DAT trigger proof, Smacker/video provenance, Amiga keyboard gates, and 68020/OCS/ECS hardware assumptions before any playable claim. |
| `http://dmweb.free.fr/games/dungeon-master-ii/editions/pc-9801/` | DM2 PC-9801 edition page: Japanese v1.0 by Victor Entertainment, released 1993-12-23; the full game download is four PC-9801 disk images in FDI format. The page also lists separate Japan demo media (`.hdm` setup floppy + `.hdi` bootable hard-disk image), states there is no music in this version, and gives the PC-9801 command table: number-row 1-4 champion inventory, Space leader inventory, Escape freeze/unfreeze, Return/Enter wake while resting, Alt-S disk menu, and numeric keypad 4/5/6 + 1/2/3 for turn/forward/turn and side/back/side movement. | Pin DM2 PC-9801 as a disk-image import/emulation target, not a PC/Interplay data-path alias. Future PC-9801 support needs FDI/HDM/HDI media handling, no-music behavior, and a PC-98-specific input bridge. |
| `http://dmweb.free.fr/games/dungeon-master-ii/editions/fm-towns/` | DM2 FM Towns edition page: Japanese v1.0 by Victor Entertainment, released 1994-01-28; the page points at a redump.org BIN/CUE Compact Disc image. It documents the FM Towns CD-audio profile: same six music tracks as PC-9821 and Japanese Macintosh, tracks 2-6 are slightly quieter, and there is an extra silent track 8. The command table is also distinct: number-row 1-4 champion inventory, Space leader inventory, Escape freeze/unfreeze, Return wakes while resting, Ctrl-Shift-S opens the disk menu, arrows turn/forward/back, Shift+Left/Right move sideways, Return starts a new game on the entrance screen, Ctrl-Q quits, and Return/Enter closes credits. | Pin DM2 FM Towns as a CD-image plus audio-track/import target, not a PC/Interplay data-path alias. Future support needs BIN/CUE handling, CD.DAT/music-trigger evidence, the FM Towns keyboard bridge, and runtime proof before any playable claim. |
| `http://dmweb.free.fr/games/dungeon-master-ii/editions/sega-cd-mega-cd/` | DM2 Sega CD / Mega CD edition page: Europe and USA are English v1.0 releases; Japan is Japanese v1.0. The page points each edition at redump.org BIN/CUE Compact Disc images, also lists DMFiles CD-content archives for USA English and Japanese, and splits the Sega CD data/audio handoff into a data-track ISO archive plus an audio-track MP3 archive. Its music note says Sega CD / Mega CD uses the same six-track CD.DAT trigger table as PC-9821/Japanese Macintosh/FM Towns except track 7 is replaced by a 15-second silent track. | Pin DM2 Sega CD / Mega CD as a BIN/CUE plus mixed data/audio-track target, not an Interplay PC MVE path. Future support needs CD image ingestion, data-track file extraction, audio-track/track-7 silence evidence, and a Sega-CD-specific runtime/input proof before any playable claim. |
| `http://dmweb.free.fr/games/dungeon-master-ii/editions/pc-9821/` | DM2 PC-9821 edition page: Japanese v1.0 by Victor Entertainment, released 1994-09-22 at ¥12800. The page points at a BIN/CUE Compact Disc image and a DMFiles archive, documents the same six CD.DAT-triggered music tracks as Japanese Macintosh, and lists the PC-98 keyboard table: number-row 1-4 champion inventory, Space leader inventory, Escape freeze/unfreeze, Return/Enter wake while resting, Alt-S disk menu, and numeric keypad 4/5/6 plus 1/2/3 for movement. The crack notes identify `FIRE.EXE` LZEXE compression and CD-ROM protection checks during load/save. | Pin DM2 PC-9821 as a PC-98 CD-image target distinct from the PC-9801 floppy line. Future support needs BIN/CUE ingestion, CD.DAT/audio-trigger evidence, PC-98 keyboard handling shared with PC-9801 where valid, and copy-protection behavior/provenance notes without committing patched executables. |
| `http://dmweb.free.fr/games/dungeon-master-ii/editions/ibm-psv/` | DM2 IBM PS/V edition page: Japanese v1.0 by Victor Entertainment, released 1994-09-22 at ¥12800. The page states this version has no music, and its download archive contains the files from the original three floppy disks plus WinImage disk images. The command table lists number-row 1-4 champion inventory, Space leader inventory, Escape freeze/unfreeze, Return wake while resting, Alt-S disk menu, keypad 4/5/6 + arrows for turn/forward/turn, keypad 1/2/3 plus Shift+Left/Right for side/back/side movement, and entrance-screen Return/Ctrl-Q behavior. Crack notes identify `FIRE.EXE` LZEXE compression and protection checks at offsets `0x21989` (new game) and `0x1FB77` (save). | Pin DM2 IBM PS/V as a Japanese floppy/WinImage target, not a PC-9821 CD or Interplay PC path. Future support needs WinImage/floppy ingestion, no-music behavior, IBM PS/V keyboard handling, and copy-protection behavior/provenance notes without committing patched executables. |
| `http://dmweb.free.fr/games/dungeon-master-ii/editions/macintosh/` | DM2 Macintosh edition page: tracks Japanese v1.0, USA English v1.0, and USA English demo media as redump BIN/CUE CD images plus DMFiles/CD-content archives. It documents the JP/US split: JP was released in August 1994 with old 16-color graphics, an intro animation also present on Sega CD, CD-audio tracks, 320x200 and 640x400 layouts, and a Japanese crack note for the `Skullkeep` executable resource fork at offsets `0x37FC9`, `0x3809D`, and `0x3653F`; US was released in October 1995 with upgraded 256-color graphics, an extra layout, Mac menu new/open/save behavior, balloon help, and 28 MIDI files plus SoundMusicSys/SND/INST/SONG/MIDI resources instead of CD-audio. The page also lists separate Japanese and English keyboard tables using Command-S/Command-Q and, in English, Command-O plus wall-ornate/button hotkeys. | Pin DM2 Macintosh as a Mac CD-image/resource-fork target, not a DOS MVE path. Future support needs BIN/CUE/CD-content ingestion, StuffIt/HQX/resource-fork handling, QuickTime/MooV animation extraction, Mac MIDI/SoundMusicSys evidence, JP CD-audio behavior, and Mac-specific keyboard/menu bridges before any runtime claim. |
| `http://dmweb.free.fr/games/therons-quest/` | TQ: TurboGrafx-16/PC Engine, only platform. 7 mini-dungeons instead of 1 big. 1 player (Theron) + 3 NPCs who lose items between dungeons. Game only saves between dungeons. "Light version" of DM. | Confirms our V1 phase plan for Theron: `src/theron/` already mirrors this. |
| `http://dmweb.free.fr/games/dungeon-master-nexus/` | Nexus overview page: Sega Saturn-only Dungeon Master sequel, released only in Japan and only in Japanese, with unofficial English and French fan translations available. It confirms the true 3D engine, 15-level structure, Japanese title `ダンジョン・マスターネクサス`, and preservation of Masaaki Shibata map material for levels 2-12. The page also links the Saturn platform page and magazine evidence from Saturn Fan (1996 announcement/news and 1998 interview) plus Sega Saturn Magazine (1997 news); DMWeb notes Nexus was announced in 1996 with a DM2 screenshot before it had its final name. | Confirms our V0/V1 plan for Nexus: `src/nexus/`. DMDF is a separate Saturn data path, not FTL `DUNGEON.DAT`; the 15-level/source-map boundary should drive DGN/world-loader coverage and public docs should keep fan translations separate from the official Japanese release. |
| `http://dmweb.free.fr/games/dungeon-master-nexus/editions/sega-saturn/` | Nexus Sega Saturn edition page: tracks Japan demo, Japan retail, English fan translation, and French fan translation edition pages. It lists Saturn CD audio tracks 02-09, Victor web-site screenshots, redump.org and CloneCD Japanese disc-image sources, a DMFiles archive containing the original Japanese BIN/CUE plus English version 2 and French fan-translation ISO/BIN/CUE or merged ISO/CUE images, and a Japanese demo BIN/CUE. The retail Japan page gives release date 1998-03-26 and price ¥6800, with box/manual/disc scans and Japanese videos. The fan pages date English version 1 to 2023-09-20, English version 2 to 2024-01-02, and French to 2024-01-02. The page also documents a light-overflow bug where repeated `Ful`/`Oh Ir Ra` can push the dungeon into darkness. | Pin Nexus as a Saturn BIN/CUE plus multi-track-audio target with optional demo/fan-translation variants. Future work needs Track 1 full-disc launch handoff, CD-audio track receipt handling, demo/fan-translation classification, manual/video/screenshot provenance for public proof, and a gameplay regression for the light-overflow boundary before calling the Saturn runtime complete. |

### File format specs (the **most important** pages)

| URL | What's there | What we use it for |
|---|---|---|
| `http://dmweb.free.fr/community/documentation/file-formats/data-files/` | **Complete spec for GRAPHICS.DAT, SONG.DAT, HCSB.DAT, NAKED.AMG** across **38 different game/version combos** with endian, format version (DMCSB1 / DMCSB2 / DMII), and per-file item-type breakdown (IMG1, IMG2, IMG3, IMG4, SND1-SND8, MUS1-MUS2, TXT1-TXT2, FNT1, LAY1, LAY2, COD1-COD4, P4B1, SEQ1-SEQ2, RAW1-RAW2, NULL). Includes the **LZW-compressed** version info (only DM Atari ST and CSB Atari ST use LZW). Includes **endianness and signature per file** (8001h big-endian for DMCSB2; 8005h big-endian for DMII, 8004h for DMII FM-Towns). Documents the **32 KB max item size** in the DMCSB format. Documents the **3 byte local palette** for IMG3/IMG4. | This is the master reference for `src/shared/firestaff_po_loader.c`'s per-game asset loading. We need to implement: header parsing for all 3 formats, item decompression (LZW only for Atari ST), item decoding for IMG1-IMG4, SND1-SND8, etc. The 32 KB item cap is important: any item > 32 KB uses the "expanded portraits" trick (8 lines × 8 portraits per item, instead of 6×4). |
| `http://dmweb.free.fr/community/documentation/file-formats/animations/` | **Complete spec for animation files** (FTL, INTRO, END, CREDITS, TITL.DAT, ANIMATE.DAT, ANIMATE.SCR, ENDA.DAT, STORY.DAT, etc.). Documents all 18 item types: AN (animation def), BN/BR/CU/DL/EN/FO/FS/GD/MD/MF/MI/NE/P8/PL/SD/SF/SO/TD/TR/WA. Documents the 3-byte signature FF81h that starts every DL/EN item. Documents AN-specific per-file values (width × height × depth × unknown per game/version). Documents **DMII-for-PC exception**: those files have 100206 bytes of MVE player, then the actual MVE data. | The animation engine spec for the intro/outro movies. Our V22 modern-asset pipeline replaces these with MP4/H.264, so this is mostly for "find the animation item, skip it, replace it". The `FF81h` signature is the marker we look for. |
| `http://dmweb.free.fr/community/documentation/file-formats/animation-script/` | **Complete spec for ANIMATE.SCR** in CSB Atari ST. Bytecode with 30 instructions (load item, unload, expand graphic, blit, set palette, fade, wait for vertical blank, FOR/NEXT loops, copy image, set display coordinates, etc.). Includes **the entire disassembled ANIMATE.SCR file from CSB Atari ST** with line-by-line comments — this is gold for anyone who needs to understand how the engine drives the animation. | We don't need to implement this (modern V2.2 assets replace the animations), but the data format is documented for reference. |
| `http://dmweb.free.fr/community/documentation/file-formats/graphics-dat/` | (404 at time of fetch — may have moved) | (See `data-files/` page above for the full GRAPHICS.DAT spec which is complete) |
| `http://dmweb.free.fr/community/documentation/file-formats/dungeon-dat/` | (404 at time of fetch — may have moved) | (The ReDMCSB WIP 2021-02-06 source has the complete DUNGEON.DAT layout; see `docs/REDMCSB_REFERENCE.md` and `Toolchains/Common/Source/DUNGEON.C`.) |

### Community / Clones

| URL | What's there | What we use it for |
|---|---|---|
| `http://dmweb.free.fr/community/clones/chaos-strikes-back-for-windows-and-linux-raspbian-macos-x-pocket-pc/` | **CSBwin** by Paul R. Stevens. 12.100 source, available for Windows x86-32/x64, Linux x86-32/x64, Raspbian ARM-32, MacOS X, Pocket PC. **Open source on GitHub via zelurker's fork** (https://github.com/zelurker/CSB). Includes DM and CSB dungeons from Atari ST. Supports custom modules. | Our `src/csb/` is closely modeled on CSBwin's behavior. The **zelurker CSB GitHub repo** is a valuable second source for CSB disassembly in addition to Meynaf's ReDMCSB. **The 'CSB lineage' link in AGENTS.md is the same project**: <https://github.com/zelurker/CSB>. |
| `http://dmweb.free.fr/community/clones/skwin-dungeon-master-ii-for-windows/` | **SKWIN** by kentaro.k-21 + Sphenx. Port of DM2 to Windows. The latest version is the `skproject` on GitHub: <https://github.com/gbsphenx/skproject/releases>. | Our `src/dm2/` work is informed by skproject's reverse-engineering of DM2's data formats. **Sphenx is also the author of several custom Theron's Quest save games** that the greatstone site documents. |
| `http://dmweb.free.fr/community/clones/return-to-chaos/` | **Return to Chaos** (RTC) by George Gilbert. Recreation of DM/CSB/DM2 with custom graphics/sounds support. | The "how to swap graphics and sounds" feature is a useful reference for our V22 modern-asset pipeline. |
| `http://dmweb.free.fr/community/clones/dungeon-strikes-back/` | **Dungeon Strikes Back** — another DM/CSB clone. | (Lower priority — CSBwin is the canonical reference) |
| `http://dmweb.free.fr/community/clones/` | Index of all DM clones: CSBwin, Return to Chaos, Dungeon Strikes Back, SKWIN, plus 30+ "truthful gameplay" / "similar gameplay" / "other games" inspired by DM | Useful to see what other implementations exist; the dmweb "Clones" page has 30+ entries |

### FAQ

`http://dmweb.free.fr/community/faq/` — 50+ FAQ items split into:
- **How to play with emulation** (12 platform-specific how-tos: WinUAE, Apple IIGS, Atari ST, FM-Towns, IBM PS/V, Macintosh, PC, PC-9801/9821, Sega CD, Sega Saturn, Super NES, TurboGrafx-16, X68000) — each has a "how to install + run" guide for the platform.
- **Gameplay questions** (creating Ful Bombs, importing DM champions into CSB, potion power, alternate DM ending, etc.)
- **General questions** (decompressing archives, FTL history, DM Plus, where to buy/download)
- **Technical questions** (CSB Amiga crashes on save, WinUAE issues, DM PC disk-in-A: bug, etc.)

Particularly useful: **"Where can I download the games?"** (which lists every legally-available source for each game) and **"What is the use of the Green Gem, Magnifier, Rabbit's Foot, Ekkhard Cross and ?"** (which gives us a definitive list of magical-item effects that we should encode in our i18n).

### Custom Dungeons

`http://dmweb.free.fr/community/custom-dungeons/` — 50+ custom dungeons for DM/CSB/CSBWin with maps and credits. Useful as **reference implementations** of what a working custom dungeon.dat looks like, especially the engine-extended ones (Conflux III uses CSBWin's extra actuator types; Imprisoned Again uses a non-standard floor decoration type).

The key takeaway: **any "real" custom dungeon for DM/CSB/CSBWin must use the engine's existing actuator/item types** — there is no modding API in the original binary. Our CSB V1 phase plan correctly identifies this constraint.

### Other community pages

- `http://dmweb.free.fr/community/redmcsb/` (referenced from existing `docs/REDMCSB_REFERENCE.md` — Meynaf's decompilation)
- `http://dmweb.free.fr/community/tools/` — the actual page content seems to redirect to a "Magazines" link; the tools are listed elsewhere
- `http://dmweb.free.fr/ftl-games/` — FTL Games company overview, staff credits, magazine scans

---

## greatstone.free.fr/dm/ — pages reviewed

### Overview

`http://greatstone.free.fr/dm/overview.html` — **the page that explains what greatstone's "Swoosh Construction Kit" (sck) tool is and what it does**. The sck is a Java tool (Maven project) that has decoded every major file format for DM/CSB/DM2 and provides both command-line and GUI extraction. As of the last news (2011 milestone), the sck can extract:

- **graphics.dat** — all item types (IMG1-IMG9, SND1-SND9, MUS1-MUS2, TXT1-TXT2, FNT1, LAY1-LAY2, COD1-COD4, P4B1, SEQ1-SEQ2, RAW1-RAW2, plus Amiga Extra Halfbrite IMGEHB)
- **dungeon.dat** — fully decodes the format, exports to XML, and an XSLT stylesheet converts the XML to HTML dungeon maps (this is the "Swoosh" — a "Swoosh" is a swish of a champion's hand in a magic gesture)
- **save game files** — original FTL format + in-game player saves + CSBWin saves; handles all portrait formats inside save games (CMP, IMH6, IMG6LH)
- **ftl files** — animation files (DM, CSB)
- **hint oracle files** — for CSB
- **sound files** — for DM2
- **portraits** — for CSB

The sck can also **recompress** dungeons. It supports **SNDA SPR1** format for DM2 PC-9821 sounds and **SND9 SPR1** for DM2 PC Beta.

The sck has 30+ animation files analyzed (see the animations format page on dmweb above).

### Game-version coverage by sck

| Game | Versions extracted |
|---|---|
| **DM** | Atari 1.0/1.1/1.2/1.3, Amiga 1.0/2.0/2.1/2.2/3.6, Apple IIGS, FM-Towns 2.0, PC 3.4 (English + Multilingual), PC-9801 2.0, X68000, SNES, **Teaser demo** |
| **CSB** | Atari 2.0/2.1, Amiga 3.1/3.3 (EN/FR/GE), FM-Towns 3.1 (EN/JP), PC-9801 3.1 (JP), X68000 |
| **DM2** | Amiga 1.0, FM-Towns 1.0 (EN/JP), IBM PS/V 1.0 (JP), Mac 1.0 (EN/JP), PC 0.9 Beta, PC English, PC German, PC French, PC Demo, PC-9801 1.0 (JP), PC-9821 1.0 (JP), Sega CD 1.0 (EN/JP) |
| **Theron's Quest** | PC Engine (CD) JP/US |
| **DM Nexus** | Sega Saturn JP |
| **Other** | Black Crypt (Amiga), R-Type III (GBA) — both for fun, not DM/CSB |

### File format specs

| URL | What's there | What we use it for |
|---|---|---|
| `http://greatstone.free.fr/dm/d_ftl.html` | **FTL container format spec** — Amiga hunk-based binary, with 2 compression algorithms (none + RLE/LZW), 3 checksums, decoder/encoder logic in pseudocode. Used by Amiga animations and several other files. | The FTL format is used for many Amiga asset files. We already handle some of it in `src/shared/asset_status_m12.c`; this spec is the definitive reference. |
| `http://greatstone.free.fr/dm/d_mapfile.html` | **mapfile format spec** — a YAML-like format that describes the structure of a binary file, used by the sck to identify item boundaries without the original game's header. Each line declares: `type name offset size` (e.g., `IMG1 image00 0 256`). | We could use the same approach in our asset verification probe: instead of relying on the engine's hardcoded item offsets, parse a mapfile to find them. This would make our probes more robust against version differences. |
| `http://greatstone.free.fr/dm/d_pak.html` | **PAK format spec** — Atari ST START.PAK compression format, similar to ZIP. Used by DM/CSB Atari ST to compress the main executable. The sck can decompress PAK and dump the raw binary. | The Atari ST PAK is the equivalent of an ELF wrapper around the game binary. We don't need to decompress it (the ReDMCSB source is already disassembly), but the PAK format is documented for completeness. |
| `http://greatstone.free.fr/dm/d_items.html` | **Items format spec** — covers IMG5 (4bpp chunked image), most common DM image format, plus item 558 (Amiga executable code) and item 559 (Amiga sprite table). | The **IMG5 4bpp format** is what most DM images use. We should have an IMG5 decoder in `src/shared/`. The sck has a working decoder that we can port (Java source is on the greatstone site). |
| `http://greatstone.free.fr/dm/d_articles.html` | **Articles index** — links to several technical articles by greatstone, including: <br>• Mac QuickTime conversion (DM2 Mac .moov → MP4)<br>• DM SNES multi-palettes<br>• Several others | The Mac QuickTime article is relevant if we want to extract DM2 Mac animations. The SNES multi-palettes article explains why DM SNES palettes are per-tile-group. |

### Tool

- **`http://greatstone.free.fr/dm/t_product.html`** — sck overview, screenshots
- **`http://greatstone.free.fr/dm/t_screenshots.html`** — sck GUI screenshots, looks like a 2008-era Java Swing app
- **`http://greatstone.free.fr/dm/t_download.html`** — sck download (Java JAR + source)
- **`http://greatstone.free.fr/dm/t_tutorial.html`** — how to use the sck (write a mapfile, extract a file)

### Game pages (extracted data)

| URL | What's there | What we use it for |
|---|---|---|
| `http://greatstone.free.fr/dm/g_dm.html` | **DM: per-version extraction reports** for Atari 1.0/1.1/1.2/1.3, Amiga 1.0/2.0/2.1/2.2/3.6, Apple IIGS, FM-Towns, PC-98, X68000, PC 3.4, SNES. Each has: extracted item counts, sample images, detected file signature, checksum, whether the dungeon has LZW compression, etc. | This is **the canonical "which file is which" reference**. When a user puts a DM data file in `~/.firestaff/data/dm1/`, we can identify which version it is by comparing to this table. We can also see exactly which items are unique to which version. |
| `http://greatstone.free.fr/dm/g_csb.html` | **CSB: per-version extraction** for Atari 2.0/2.1, Amiga 3.1/3.3 (EN/FR/GE), FM-Towns 3.1 (EN/JP), PC-9801 3.1 (JP), X68000. | Same as above for CSB. The Amiga German (GE) is the version we don't currently have local assets for; this gives us the item count to expect (749 items in GRAPHICS.DAT). |
| `http://greatstone.free.fr/dm/g_dm2.html` | **DM2: per-version extraction** for all 11 versions. | We don't have DM2 data yet but when we do, this is the reference. |
| `http://greatstone.free.fr/dm/g_cd.html` | **Custom dungeons gallery** — 50+ custom dungeons with maps, item lists, and notes about which engine extensions they use. | The "engine extensions" notes are the most useful: tells us which items are CSBWin-only, which are CSB-Atari-only, etc. |
| `http://greatstone.free.fr/dm/g_other.html` | **Other games** — Black Crypt, R-Type III GBA. | (Not relevant to Firestaff, but cool reading) |

### dm_data/ — the actual extracted data

The greatstone site has directories under `db_data/` for every game and every version, with the extracted HTML, XML, and PNG files. e.g., `http://greatstone.free.fr/dm/db_data/c_dm1_amiga_v2/graphics.dat/` has the extracted items for DM Amiga v2.0. **These are public, browse-able, and we can reference them for asset validation.**

The most relevant for Firestaff:
- `db_data/c_dm_atari_st_v1_0/graphics.dat/` — the canonical first release
- `db_data/c_dm_pc_eng/` — DM PC 3.4 English
- `db_data/c_dm_pc_multilingual/` — DM PC 3.4 Multilingual
- `db_data/c_csb_atari_st_v2_0/` — CSB Atari ST 2.0
- `db_data/c_csb_amiga_v3_1_ml/` — CSB Amiga 3.1 Multilanguage (EN/FR/GE)

These can be used as **golden references** for asset validation: when a user puts their DM data in `~/.firestaff/data/dm1/`, we can compare hash-sums of their graphics items against greatstone's extracted data to verify the data is from a known version. (Greatstone has the only such exhaustive dataset.)

---

## What we still need to fetch (dmweb pages we couldn't reach)

The dmweb.free.fr site is a Drupal site with pretty URLs. Some pages returned 404 when accessed directly. The ones we know exist but couldn't fetch cleanly:

- Specific per-edition pages under `/games/dungeon-master/editions/atari-st/`, `.../pc/`, etc. (each has screenshots, manual scans, etc.)
- Per-platform "How to play" FAQ items (12 of them) under `/community/faq/`
- Specific tools pages under `/community/tools/` (CSBWin CSBuild, SpliceCSB, etc.)
- Specific custom dungeon deep-dive pages (50+ of them) under `/community/custom-dungeons/`
- ReDMCSB page (already covered in our `docs/REDMCSB_REFERENCE.md`)

These can all be reached by browsing from the home page, but the direct URLs are not predictable (Drupal uses node numbers internally, not slugs).

---

## Recommended additions to Firestaff

Based on the surveyed material, here are concrete additions that would benefit Firestaff:

### 1. `src/shared/firestaff_image_lzw.c` (decoder for Atari ST LZW-compressed graphics)

dmweb's "Data Files" spec explicitly notes that **only DM Atari ST and CSB Atari ST use LZW compression** on their graphics items. The other versions use no compression. We currently rely on the engine's own decompression code; a clean implementation of the LZW variant used by Atari ST (per dmweb "Data Files" page) would let us extract assets directly without running the game.

### 2. `src/shared/firestaff_img5_decode.c` (IMG5 4bpp chunked image decoder)

Most DM images use the IMG5 format. The greatstone site has working Java reference. Adding an IMG5 decoder would let us read DM item data without the engine.

### 3. `tools/asset-validate/compare_to_greatstone.py`

A probe that downloads greatstone's hash-sums of known-good assets and compares them to what the user has provided. This catches:
- Wrong version of game (e.g., user provided PC demo instead of full PC 3.4)
- Corrupted assets
- Modified/pirated assets (this is a side effect, not a goal)

### 4. `docs/PLATFORM_MATRIX.md` (already partially in our internal docs)

A canonical "Firestaff supports these game versions" matrix, derived from dmweb/greatstone's coverage. Currently our AGENTS.md says "DM1 PC 3.4 English, CSB PC 3.4 English, DM2 Amiga, Theron's Quest, DM Nexus" but doesn't list all the variants we *could* support if we had the data.

### 5. i18n-friendly item names

dmweb's "What is the use of the Green Gem, Magnifier, Rabbit's Foot, Ekkhard Cross?" FAQ gives a definitive list of magical-item effects. Our i18n work already covers most of these (`T%u: %s CASTS %s` and similar), but a few game-specific items (Rabbit's Foot for hunger, etc.) could be added.

### 6. Theron's Quest savegame format

greatstone has a section on Theron's Quest save games. TQ's save format is **completely different** from DM's (it uses gzipped custom format with a header). We should look at this when implementing `src/theron/` V1 save/load.

### 7. Nexus DMDF/DGN formats

The Saturn's DMDF (data) and DGN (dungeon geometry) formats are documented elsewhere but dmweb/greatstone both reference them. Our `src/nexus/` V1 needs to parse these from the Sega Saturn NEXUS.BIN file.

---

## External download sources referenced

| Source | URL | What you get |
|---|---|---|
| **ReDMCSB** | <http://dmweb.free.fr/Stuff/ReDMCSB/ReDMCSB_WIP20210206.7z> (Meynaf's 2021 archive) | Decompiled DM + CSB Atari ST C source |
| **CSBwin** | <https://github.com/zelurker/CSB> | Paul Stevens' CSB port to C++ (open source) |
| **SKWIN** | <https://github.com/gbsphenx/skproject/releases> | DM2 Windows port |
| **sck** | <http://greatstone.free.fr/dm/t_download.html> | Pierre Monnot's asset extraction tool (Java) |
| **DMFiles Shared OneDrive** | <https://1drv.ms/f/s!AsBu7boYHQokbYK3rjKY0b5_ra8> | Community-maintained OneDrive with custom dungeons, clones, games, tools |
| **DM Forums** | <https://www.dungeon-master.com/forum/> | Active community forum (now ~25 years old) |
| **DM Wiki** | <http://dmwiki.atomas.com/> | CSBwin-focused wiki |

### Lokalt spegel av dmweb.free.fr

En komplett offline-kopia av `community/documentation/` finns nu
under `reference/dmweb-community-docs/`. 43 sidor, 5.5 MB,
genererad 2026-06-20.

- `reference/dmweb-community-docs/INDEX.md` — mänskligt läsbar
  innehållsförteckning med titlar, URL:er, breadcrumbs,
  filstorlekar och meta-beskrivningar.
- `reference/dmweb-community-docs/index.json` — maskinläsbar
  version av samma data.
- `reference/dmweb-community-docs/SCRAPE_LOG.md` — hämtningslogg
  (timestamp, HTTP-status, storlek per sida).
- `reference/dmweb-community-docs/crawl.sh` + `build_index.py` —
  reproducibelt crawl-skript (1.2s fördröjning per request,
  identifierande User-Agent).
- `reference/dmweb-community-docs/html/` — raw HTML.

Täcker fem ämnesområden:

| Sektion | Antal sidor |
|---|---|
| Copy Protection (Apple IIGS, Atari ST, generell + data-lagring på floppy) | 5 |
| Dungeon Master & Chaos Strikes Back (actions, attacks, creature generators, items, skills, GRAPHICS.DAT hidden code, GRAPHICS.DAT items 558–562) | 11 |
| Dungeon Master: Nexus (DGN files, MNS files, item.ibs, file formats) | 5 |
| File Formats (animation script, animations, data files, dungeon files, DM2 data files, DM2 music triggers, hint/oracle, layout coordinates, portrait files, saved-game files) | 11 |
| Miscellaneous (DM Atari ST history, DM PC, DM SNES/Super Famicom, FTL sound adapter, game versions) | 5 |

Använd den lokala kopian som första källa vid research; hämta
om sajten är nere eller långsam.

---

## License / Copyright notes

dmweb.free.fr materials are © to their respective authors (mostly
Pierre Monnot for the technical specs). The site itself is a
community resource; data files, graphics, and manuals are ©
FTL Games / Software Heaven, Inc. We use the specs as
**reference documentation** for our own implementation, not as
source data — every byte of game data Firestaff ships comes
from the user's own legally-acquired game files in
`~/.firestaff/data/`.

---

## TODO: fetch these next

When we have time, the following pages from dmweb should be
fetched in detail and incorporated into this reference:

- [x] ~~Fetch the full `/community/documentation/` section from dmweb~~ — klar 2026-06-20, se `reference/dmweb-community-docs/` (43 sidor speglade)
- [ ] All 12 "How to play" FAQ items
- [ ] Per-edition pages for the 5 games (atari-st, amiga, pc, etc.)
- [ ] Custom dungeons deep-dive pages (esp. Conflux III — uses CSBWin extensions)
- [ ] The greatstone "Articles" link (Mac QuickTime, SNES multi-palettes, etc.)
- [ ] greatstone's `db_data/c_dm_pc_eng/graphics.dat/` for an exhaustive item list
- [ ] greatstone's CSB switch.dat support (item type 0x07 in CSB)
- [ ] greatstone's sck source code (Java, on the t_download.html page)

When a new external resource is added to this list, append a
note to the bottom of this file rather than rewriting it.
