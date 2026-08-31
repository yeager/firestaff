# Native platform and startup verification

**Reviewed 2026-08-31.** This page records what the native Firestaff runtime
has actually admitted from authentic player-supplied media. It is deliberately
not a compatibility wish-list: a parser, a disassembly reference, or an
emulator capture does not by itself prove a public start route.

## Terms

| Term | Meaning |
| --- | --- |
| **Playable** | Original media reaches ordinary gameplay through a maintained Firestaff runtime route. |
| **Verified start route** | Original media is selected, read in memory, and reaches a bounded title/New Game/runtime handoff in both direct CLI and start-menu selection. |
| **Data path** | The native media format is recognised, but an end-to-end start has not been proven. |
| **Not released** | The original game had no release on that platform; Firestaff must not manufacture one from another port's files. |
| **Outside target** | The platform is not an original-game target in the current matrix. |

All entries use original data in its supplied form. ZIP/CUE/disc members are
read in memory and are never extracted by the runtime. Development captures
may use tools such as a debugger or emulator, but no such program, BIOS or
firmware is a Firestaff runtime dependency.

On 2026-08-31 the complete DM1 real-media selection passed all 42 registered
routes: archive identity, CLI and start-menu admission, original saves, Atari
ST, FM Towns, DOS and Amiga paths, plus source-material gates. A separate
real-media CLI/start-menu matrix passed for DM1 and CSB across their available
DOS, Atari ST, Amiga and FM Towns media. On 2026-08-31 the CSB selection passed
12 maintained routes: standalone and nested Atari ST media (including the
French preservation archive), Amiga ZIP→ADF media, both FM Towns language
lanes, and M12→M11 handoff boundaries. The focused
native suite also passed all 73 registered DM2 real-media routes across DOS,
Amiga, FM Towns and Macintosh media. These are source-owner, launch, selected
runtime, save and format receipts; they do not claim un-captured gameplay or
pixel parity.
Theron's US CloneCD ZIP and JP Rev 1 CUE routes passed after inclusion in the
native label. These are start/menu and bounded-runtime receipts, not a claim
that campaign, capture or pixel parity is complete; the remaining evidence
gaps below stay open.

## Verification matrix

| Game | Atari ST | FM Towns | DOS | Amiga | PC Engine CD | Saturn |
| --- | --- | --- | --- | --- | --- | --- |
| Dungeon Master | **Verified start route** — EN/DE/FR ST/STX variants, direct CLI and menu handoff | **Verified start route** — JA/EN package, CLI, menu and TMENU/EDM handoff | **Playable** — PC 3.4 and manually unpacked French media | **Verified start route** — HD and v2.0 ZIP→ADF in memory, CLI and menu | Outside target | Outside target |
| Chaos Strikes Back | **Verified start route** — STX title→FTLCODE, CLI and menu | **Verified start route** — EN/JP F31 package | **Not released** | **Verified start route** — A31E/A31M ZIP→ADF, CLI and menu | Outside target | Outside target |
| Dungeon Master II: Skullkeep | Not released | **Verified start route** — authenticated Towns startup/input/CD-audio slices | **Verified start route** — hash-verified GDAT/G1 startup/runtime | **Verified start route** — original installer media, title/New Game/runtime | Outside target | Outside target |
| Theron's Quest | Outside target | Outside target | Outside target | Outside target | **Verified start route** — JP and US Track 02 CUE/BIN, CLI and menu | Outside target |
| Dungeon Master Nexus | Outside target | Outside target | Outside target | Outside target | Outside target | **Data path / title blocked** — retail JP media is native and hash-checked, but the public title is fail-closed pending the complete same-revision VDP state binding |

`ZIP→ADF` means nested archives are traversed in memory, not unpacked to
disk. CSB's PC-shaped CSBWin files are reverse-engineering evidence, never
evidence of an original DOS edition. Similarly, a Theron PC Engine capture or
Nexus Saturn capture never adds an emulator dependency to the released game.

## What each verified route proves

The direct CLI and start-menu tests prove the same selected edition reaches
the same native admission boundary. They do not imply complete campaign,
save, audio or pixel parity.

| Game/platform | Source identity and bound handoff | Explicitly still open |
| --- | --- | --- |
| DM1 Atari ST | English 1.0a/1.2, German 1.2 and French 1.3 ST/STX archive routes reach the bounded runtime through direct CLI and start-menu selection. | Full campaign and platform presentation parity. |
| DM1 FM Towns | The supplied JA/EN package reaches `dm1-runtime` through direct CLI and start-menu selection after native TMENU/EDM admission. | Wider executable/CD-audio and capture parity. |
| DM1 Amiga | The supplied HD and v2.0 preservation ZIP→ZIP→ADF routes reach bounded native runtime through direct CLI and start-menu selection. The v2.0 `dm` executable is examined entirely in memory: it programs OCS COLOR00--COLOR31 through four Copper-list builders rooted at `$00dff180`, not immediate color-register writes. The proved builder receives COLOR00--15 as a caller-owned 16-word RGB4 table (`12(A5)`); COLOR16--31 are read from a distinct global table. The native RGB4 producer now mirrors the original 68000 fade routine: it copies the source table into a working table, adjusts each component by one or two toward the target, and presents exactly eight Copper updates. It accepts only verified original Amiga tables and supplies no fallback colors. The supplied original save disk is read as ZIP→ZIP→ADF in memory and its `DMGAMEG.DAT`/`.BAK` files authenticate the save header, five F0435 encrypted parts, four portraits and the F0434 big-endian dungeon checksum. Its authenticated `ZIP→ZIP→ADF→DMGAMEG.DAT` route is also accepted by Continue and direct CLI `--save`, with the original save adopted in memory. | A route-specific gameplay palette/capture that binds the active source and target tables to the composed renderer, plus wider gameplay parity. |
| DM1 DOS | PC 3.4 reaches native movement through direct CLI and start-menu selection; manually unpacked authentic French `EUDATA` does the same after the RAR2 launcher diagnostic. | Broader original-vs-Firestaff capture parity; Firestaff does not decode RAR2 itself. |
| CSB Atari ST | The supplied standalone STX plus English and French preservation ZIP→ZIP→STX packages each reach the original `ANIMATE.SCR`/`ANIMATE.DAT` title, FTLCODE, first HUD/viewport frame and runtime movement through direct CLI and start-menu selection. Nested routes remain entirely in RAM. The Atari resume/HUD regression pairs the game STX with `Utility.stx::MINI.DAT` through the virtual-member reader; it proves the original F0435 resume pose, ANIM.C→FTLCODE handoff, C232 HUD composition and the V1/V2.0/V2.1 presentation paths without extracting either disk. `csb_v1_atari_st_animation_assets_real` additionally reads the original Utility STX in RAM and verifies title pages, both source SND1 cues, VBlank ordering and FTLCODE handoff. The four Hint Oracle real-media gates read `Utility.stx::HCSB.HTC` directly in RAM, verify its hash, parse its authored pages, classify the revision, and bind the decoded text to the Firestaff-facing panel API. | Broader campaign, original pixel/cadence captures, and live Hint Oracle click routing. |
| CSB FM Towns | The supplied EN/JA F31 package selects its language-private program and Utility chain independently. Both language variants admit the original ZIP in RAM, bind the matching `CDATA`/`CJDATA` `MINI.DAT` provenance, load the retail seed state, and decode startup surfaces. The native boot profile retains and admits all 24 original C06 `PORTRAIT/*.CMP` members directly from the ZIP/CD image; the portrait regression decodes their planar pixels from those RAM views and does not use a loose asset cache. | Champion Editor, broader CD-audio and campaign parity. |
| CSB Amiga | The supplied A31E/A31M ZIP→ADF routes select original program, graphics and dungeon assets and reach `csb-entrance-0` through direct CLI and start-menu selection without unpacking the game disk. The title, dungeon and APPB language-selection regressions now each consume their hash-verified ZIP→ADF member directly in RAM; the dungeon loader accepts the same virtual member path in production as the test boundary. `csb_v1_amiga_runtime_graphics_real` decodes C001–C004 title/entrance, C017/C040 HUD, pits, fields, stairs, wall/floor ornaments and door families from original big-endian DMCSB2 records. `csb_v1_amiga_amg_real_media` reads the five original SND2 effects directly from the utility ADF in RAM and verifies their source sample/trailer layout. | Full campaign, saves, runtime playback and visual parity. |
| DM2 DOS | The supplied retail ZIP reaches GDAT/G1 loading and native New Game. Its original dungeon data now completes a regression-locked pit, stair, door and active-creature runtime chain directly from the archive in RAM. The full-map DRAW_ITEM and static-object pixel probes also read the archive members directly; the admitted File_header corpus contains no legacy DB5/DB9 static-object roots, so that legacy blit route remains explicitly no-draw rather than using fabricated objects. The read-only `SKSAVE1` WIELD diagnostic derives the hit threshold from the selected map descriptor and party skill records: the inspected `0x140c` → `0x1116` route has map difficulty 8, party power 11 and effective dexterity 21, and correctly remains a miss. | Full renderer/mechanics parity and complete SKSAVE ownership; an authentic input/timing trace is still required before this diagnostic can prove a kill/drop. |
| DM2 FM Towns | Authentic Towns startup, input, CD-audio and gameplay slices are covered. | Complete runtime ownership and full parity. |
| DM2 Amiga | Authentic installer data reaches title, New Game, runtime and the clipped native CHARSHEET frame. | Wider game/save/pixel/audio parity. |
| Theron PC Engine | JP reaches title, stage, Soul Room and Akutuba; US CloneCD CUE/IMG reaches title and scripted Soul Room startup. | Authentic CD→RAM→consumer transition capture, game-owned level publication, saves and full gameplay. |
| Nexus Saturn | All 11 selected `nexus;real-media` tests pass against the authentic Japanese retail package. The source route reads the existing ZIP/CUE/BIN members in memory: DM.BIN HUD/champion tables, the full LEV00–LEV15 face/material corpus, and engine-level DGN receipts no longer require a mounted or extracted ISO. This proves source admission and bounded no-draw parsing, not a rendered Saturn scene. | One same-revision title capture joining `TITLE.CG`, active MAPD, palette, VDP registers/layers/timing and consumer; until then title/menu/gameplay remain closed. |

## Reproduction and evidence

The status is maintained with the operational matrix and source-locked test
receipts, not screenshots. Start with:

- [platform status](../PLATFORM_STATUS.md)
- [game-data format reference](../GAME_DATA_FORMATS.md)
- [verified media hashes](../VERIFIED_HASHES.md)
- [Nexus title evidence](../nexus_title.md)
- [Theron capture readiness](../THERON_CAPTURE_READINESS.md)

The path roots are `$HOME/.firestaff/data/dm1`, `dm2`, `csb`, `nexus` and
`theron`. No game data, captures, BIOS images or generated artefacts belong in
the source repository.
