# Game and platform status

**Last full review: 2026-10-12; Theron rows updated: 2026-10-03.** This is the
operational status matrix. It separates media recognition, verified runtime
slices and end-to-end playability. A row marked as supported does not mean
that every menu, save format or visual parity gate is complete.

## Status terms

| Status | Meaning |
|---|---|
| **Playable** | Original data reaches normal gameplay through a maintained runtime gate. |
| **Verified route** | Authentic media is read and a bounded startup, New Game or gameplay route is tested. |
| **Data path** | The format or media is admitted and source-locked, but end-to-end gameplay is not proven. |
| **Preservation** | The edition is documented or retained as reference only. It cannot select a normal game route. |
| **Unsupported** | The platform is outside the game's Firestaff support matrix. |

On 2026-10-08, the current `main` commit `5126347d` passed a focused local
startup/menu revalidation against supplied original media: DM1 Amiga v2.0
(`dm1_v1_amiga_v20_archive_cli_boot`), CSB Atari ST
(`csb_v1_atari_original_archive_cli_boot`), and DM2 FM Towns
(`dm2_v1_fmtowns_native_cli_boot`). The real-media return-to-menu regression
(`return_to_menu_rescans_dm1_csb_dm2_real_media`) passed all four configured
routes and rediscovered DM1, CSB and DM2 after each return. The empty-data menu
flow (`m12_startup_menu`) also passed its DM1/CSB/DM2 missing-media checks.
These tests ran on Mac16,10 with headless SDL; they do not verify native M5
HiDPI presentation or input.

On 2026-10-10, `dm2_v1_fmtowns_native_cli_boot` was run directly against the
installed original FM Towns ZIP after CTest's configured archive path skipped
the case. Bare `--game dm2` and M12 both selected FM Towns; its New Game menu
matched the source RGB digest, and ordinary main-loop input reached the first
party. This remains headless startup evidence, not physical M5 parity.

On 2026-10-12, the complete `dm2_v1_fmtowns_native_cli_boot` test passed
against the installed original FM Towns package on Mac16,10 in 346 seconds.
It checked bare CLI and M12 AUTO selection from the full data root, completed
all 225 TWANIM frames by 40 seconds, compared the presented New Game menu
pixels with the decoded original `TITLE/0/4` RGB digest, and drove New Game to
the first champion on map 0 through both direct CLI and M12. The normal-loop
runtime frame used the original graphics with zero core fallback draws. These
checks use SDL's dummy video and audio drivers; they do not verify a native
window, HiDPI input or audible playback.

The same day, `csb_v1_fmtowns_native_cli_boot` passed against the original
CSB FM Towns package in 110 seconds. It covered automatic and explicit F31
selection, keyboard and mouse start-menu routes, the original
`TITLE.ANM → SWITCHTW → CHTWE/CHTWJ → C004 Prison → MINI.DAT` chain, and the
authentic first party at map 4, (22,18), direction 2. Captures checked the
closed-door C28 palette composition in Original, Modern and Custom modes; the
1920×1080 FIT route also reached the same party. This uses SDL dummy output
and does not prove native HiDPI or audio parity.

Also on 2026-10-12, `dm1_v1_fmtowns_archive_cli_boot` passed against the
original DM1 FM Towns ZIP in 264 seconds. It exercised bare and explicit CLI
launch, English EDM and Japanese JDM selection through the start menu,
language-specific runtime admission and movement inputs, and the source-owned
Hall route through mirror selection and champion resurrection. SDL dummy
output establishes route and input behavior only; this run did not assess
visual, audible or native HiDPI parity.

`dm1_v1_m12_auto_multi_platform_menu` also passed against the installed
English FM Towns, DOS 3.4, Atari ST 1.2 and Amiga 2.0 archives. The mixed-data
M12 AUTO route selected FM Towns and reached its original Hall-of-Champions
runtime; the menu and direct CLI routes also selected each explicitly chosen
edition and reached its authenticated first runtime frame. This is startup
and route evidence with SDL dummy output, not native-display parity.

On 2026-10-12, both `dm1_v1_pc34_native_cli_boot` and
`dm1_v1_m12_auto_multi_platform_menu` passed again from commit `62793e3` against
the installed original media. The PC 3.4 test reached its first source-owned
runtime frame from the authentic retail ZIP and loose files extracted from
that ZIP in the test scratch area. The mixed-platform menu test discovered the
authentic FM Towns, DOS 3.4, Atari ST 1.2 and Amiga 2.0 editions, selected FM
Towns through AUTO, and verified bare CLI and explicit platform routes. The
tests took 255.74 and 88.10 seconds respectively. SDL dummy output verifies
startup routing and source identities, not native Retina presentation or
input.

`csb_v1_atari_stx_native_cli_boot` passed the same day against the original
Atari STX and campaign archive. It covered the retained Atari title, menu
keyboard and pointer paths, the original map-0 party pose and movement input,
the authentic two-champion recruitment route, and the normal M12-to-entrance
handoff. The runtime probes retained a nonzero source viewport receipt. SDL
dummy output does not establish native display or audio parity.

`csb_v1_amiga_native_cli_boot` passed against the original A31 ZIP in 67
seconds. It exercised both the A31M title/program route and A31E C03_GAME
handoff, Original and Modern startup/input paths, the full first-movement
input matrix, M12 launch and AUTO discovery of the original FTL Amiga package.
The runtime receipt retained a nonzero viewport hash. SDL dummy output does
not establish native display or audio parity.

`dm2_v1_dos_native_cli_boot` passed against the installed English DOS retail
ZIP in 73 seconds. It reached the source New Game menu through the normal M12
route and loaded the first party at map 0, (1,8), facing north. A presented
menu capture was checked for a populated multicolour source surface; the run
used SDL dummy video and audio and does not establish native display or audio
parity.

`dm2_v1_amiga_native_cli_boot` passed in 103 seconds against the original
Amiga ZIP→ADF data. The M12 keyboard and pointer paths reached the source
SWSH/TITL New Game screen and loaded the first map-0 party with zero fallback
draws; the presented runtime frame was nonblank, and the authentic movement
and action input matrix passed. SDL dummy output does not establish native
display or audio parity.

`dm2_v1_mac_native_cli_boot` passed in 168 seconds against the authentic
Macintosh retail ZIP. The normal loop completed the original `Title.MooV`,
opened New Game and mirror selection, reached runtime and accepted two moves;
the M12 route checked a scaled 1920×1080 host surface, and direct `--game`
Back exited the process instead of reopening the start menu. Its screenshot
checks are headless smoke checks, not native HiDPI or pixel-parity evidence.

The 2026-10-10 original-media return-to-menu checks also cover DM1 Atari ST
1.1 and Amiga 3.6. Each route reached its source-owned runtime, returned to
the launcher, and rediscovered DM1, CSB and DM2 from an isolated collection
root. Combined with existing checks, this closes headless return/rescan
coverage for DM1 DOS/Atari ST/Amiga/FM Towns, CSB Atari ST/Amiga/FM Towns, and
DM2 DOS/Macintosh/Amiga/FM Towns; it does not establish physical desktop
input or platform visual parity.

On 2026-10-12, commit `0b8f1f347` passed the complete local
`return_to_menu_rescans_dm1_csb_dm2_real_media` matrix against the installed
original-media collection in 1,188 seconds. The regression now also verifies
that SDL3 window-close requests cancel synchronous return scans, without
redrawing the launcher, for keyboard and mouse return paths. The focused
`asset_scan_access_denied` test passed with the authentic DM1 FM Towns archive
staged; it covers reporting an unreadable folder beside valid game data, a
same-process rescan after access is restored, and discovery on a fresh startup.
That permission test uses filesystem mode changes, not the native macOS TCC
privacy dialog. GitHub Actions run `38238388059` passed its full Linux, macOS
and Windows build/test matrix, including the SDL3 close-request regression on
all three operating systems. These results establish scan and launcher
behavior, not physical M5 HiDPI rendering, input, or game-specific visual/audio
parity.

## Latest real-media revalidation

Recent reports of DM1/CSB input and presentation faults and DM2/FM Towns
palette/menu/dungeon faults remain open parity work until they are reproduced
against matching original media and resolved with source or capture evidence.
A green launch test below is therefore not a substitute for visual, audio,
input or campaign parity.

On 2026-08-30 the local, non-CI DM2 regression set passed against supplied
archives without extracting or modifying them. It
covered `dm2_v1_dos_native_cli_boot`, `dm2_v1_dos_fr_native_cli_boot`,
`dm2_v1_dos_sksave_archive_menu_resume`,
`dm2_v1_dos_sksave_archive_menu_resume_matrix`,
`dm2_v1_amiga_native_cli_boot`, `dm2_v1_fmtowns_native_cli_boot`, and
`dm2_v1_mac_native_cli_boot`. The matrix proves the stated bounded launch,
menu, New Game/first-movement and DOS read-only resume routes only; it does
not promote untraced combat, cross-platform save writing, complete GUI or
pixel/audio parity to playable status.

The same local revalidation passed the DM1 and CSB native launch matrices
against their supplied original media.  DM1 covered PC/DOS, Atari ST, Amiga
and FM Towns archive routes, including nested original containers.  CSB
covered Atari ST, Amiga ZIP-to-ADF and FM Towns English/Japanese routes.  The
FM Towns CSB evidence now explicitly drives the original F31 chain
`TITLE.ANM → SWITCHTW → CHTWE/CHTWJ → C004 Prison → MINI.DAT`, then checks the
published live party pose and a source-owned turn.  These are bounded
real-media launch and first-gameplay checks only; they do not imply full
campaign, save, audio or pixel-capture parity.

On 2026-09-01 a follow-up run repeated the maintained real-media startup
routes after DM1 viewport ordering work. DM2 passed DOS, Amiga and FM Towns;
CSB passed Atari ST, Amiga and FM Towns. The checked tests are
`dm2_v1_{dos,amiga,fmtowns}_native_cli_boot` and
`csb_v1_{atari_stx,amiga,fmtowns}_native_cli_boot`. These results reaffirm
native, in-memory startup and bounded first-runtime behavior only.

On 2026-09-05 the CSB Atari ST, Amiga and FM Towns maintained real-media
routes were run again on the current tree: `csb_v1_atari_stx_native_cli_boot`,
`csb_v1_amiga_native_cli_boot`, and `csb_v1_fmtowns_native_cli_boot` passed.
The FM Towns Japanese selection was also run explicitly against its supplied
archive, traversing the same title, switch, prison and `MINI.DAT` route. This
is fresh evidence for in-memory startup and the first source-owned movement;
it is not a claim of complete campaign, save, audio or pixel parity.

The same 2026-09-05 revalidation passed DM2's maintained native launch matrix:
`dm2_v1_dos_native_cli_boot`, `dm2_v1_amiga_native_cli_boot`,
`dm2_v1_fmtowns_native_cli_boot`, and `dm2_v1_mac_native_cli_boot`. The
Macintosh route specifically covers the retail ZIP's native title, mirror
selection and observed input matrix. As elsewhere in this document, this is
bounded startup/runtime evidence rather than a complete parity assertion.

On 2026-09-09 the current native build was revalidated against only supplied,
unmodified game containers. `dm1_v1_fmtowns_archive_cli_boot` reached the
FM Towns `TMENU` → `EDM`/`JDM` handoff, entrance, English/Japanese input
matrix and a captured F20 frame. CSB passed
`csb_v1_atari_stx_native_cli_boot`, `csb_v1_amiga_adf_archive_cli_boot` and
`csb_v1_fmtowns_native_cli_boot`; the latter checks the C28 palette with the
C002/C003 closed-prison composition and the `MINI.DAT` map 4, party 22/18/2
bootstrap pose. DM2's real FM Towns media passed its title, gameplay and
corpus gates. Nexus passed real `TITLE.CG`/`MAPD` title admission and its
startup handoff gates. Theron passed Japanese raw track-02 intake, level-block
and later-dungeon gates, plus the Japanese CUE boot route. These are bounded
route receipts, not a claim of complete campaign, save, audio or pixel parity.

On 2026-09-29 the authentic startup smoke tests passed for DM1 PC 3.4 DOS,
DM1 Atari ST, CSB Atari ST and DM2 Macintosh. The DM1 PC 3.4 normal start-menu
route now also records that it selected the source-owned `GRAPHICS.DAT` C001
intro and completed all 23 `TITLE.C` F0437 steps before entering runtime.
`dm2_unsupported_platform_cli` passed its PC-9821 alias-rejection checks and,
with the supplied Japanese archive present, confirmed that automatic discovery
and the start menu do not launch it. These are bounded startup results; they
do not establish full presentation, audio or campaign parity.

The focused DM2 Macintosh movie-runtime test also passed against the supplied
retail ZIP with the full `Title.MooV` running at its per-sample QuickTime
deadlines. It checks the source-derived total duration and that the startup
menu is ready at EOF, closing the prior test gap that fast-forwarded the title
after checking only its opening frames. SDL dummy output does not prove native
window pacing or audible playback.

The authentic FM Towns CSB EN/JA handoff tests also ran with SDL's dummy audio
driver. Both verified that C0_MUSIC_ENTRANCE resolves to physical CD-DA track
02 and that its selected CUE/IMG PCM reaches an active, unpaused audio stream
after the original Game-program handoff. This verifies source selection and
queue acceptance, not audible hardware or retail sound parity.

## Dungeon Master

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| PC DOS 3.4 | **Playable** | The supplied English distribution ZIP is consumed in memory: direct CLI, start-menu handoff and a native movement step all bind `dungeon-master/dmaster/DATA/GRAPHICS.DAT` (MD5 `fa6b1aa29e191418713bf2cda93d962e`) through the admitted IMG3 decoder. V1 startup, menus, viewport, HUD, input, combat and saves use matching original data. | Broader original-vs-Firestaff capture and V2 finished material. |
| PC DOS French SFX | **Playable from ZIP or extracted files** | The authenticated repack `Dungeon-Master_DOS_FR_EUDATA.zip` reaches the first runtime frame through direct CLI and M12 start-menu launch, admits the original `IMG3` handoff and advances one native movement step. The same authentic `EUDATA` also works after manual extraction. Its supplied `DMSAVE.DAT`/`.BAK` pair completes a source-backed F0435 → F0433 → F0435 round trip against the original `DUNGEON.DAT`, preserving party, C03/C04 timeline and active-group state. | The original RAR 2.0 wrapper remains unsupported and must be repacked or manually extracted. |
| Atari ST | **Verified route** | Native ST/STX media is read in RAM. The supplied English 1.0a, English 1.2, German 1.2 and French 1.3 archive routes reach the bounded runtime through direct CLI and start menu; each selected `GRAPHICS.DAT` is hash-locked. The supplied preservation collection is read as `ZIP → ZIP → STX` and admits only its `[!]` original member, including its release-specific `DUNGEON.DAT` identity; cracked siblings remain excluded. The menu receipt reports the admitted `DMCSB1` Atari decoder only after its original graphics and dungeon pair bind. | Complete gameplay parity and original-media capture. |
| Amiga 2.0 English | **Verified route** | The supplied preservation ZIP → original ZIP → ADF is read in RAM, hash-locked at `GRAPHICS.DAT` MD5 `6a2f135b53c2220f0251fa103e2a6e7e`; both the original 2.0 package and the supplied HD package (`Dungeon Master (1988)(FTL)[HD].zip` → ADF) reach the bounded native runtime through direct CLI and start-menu handoff. The menu receipt reports the admitted big-endian IMG2 decoder only after its original graphics and dungeon pair bind. The original save-disk `DMGAMEG.DAT`/`.BAK` is also authenticated entirely in memory through its Amiga-specific F0435 parts and F0434 dungeon checksum; it is deliberately kept separate from PC34 data. | Format-specific save-world materialization, broader gameplay, Amiga-specific presentation/input ownership and capture parity. |
| Amiga 2.1/2.2 English | **Verified route** | The original Amiga software archive's ADF members are hash-checked and repacked byte-for-byte as ZIP for transport. Both editions reach `dm1-runtime` through direct CLI and M12 start-menu launch after their native F0437 title and F0441 entrance sequences; the paired original dungeon binds to the authenticated owner. | Broader gameplay and complete presentation/input parity. |
| Amiga 2.2 German | **Verified route** | The original 910,058-byte ADF member is hash-locked at SHA-256 `5abc4f549941eed4e6af48bd52d348212b38b0a17fd3e83609c8aaabc2e4cd73`, then exercised through ZIP transport and direct disk selection. CLI and M12 routes complete native F0437/F0441 startup and reach the first runtime pose. | Original emulator frame alignment, broader gameplay, audio and complete presentation parity. |
| Amiga 3.6 English/French/German startup | **Verified route** | The authentic original ZIP is hash-locked at `GRAPHICS.DAT` MD5 `7f9458e4a3972d06e649a6fa85a7f34b`. English direct CLI archive and discovered-directory routes, plus M12 start-menu handoff, reach the first runtime pose through the authenticated A36 entrance path. The original-media regression also starts French and German through direct CLI `--lang`, selecting the exact `DUNGEONF.DAT` or `DUNGEONG.DAT`, and through M11 runtime/M12 handoffs from loose files and ZIP; wrong-language files are rejected. Tests cover the native C696 zones, all 64 two-pixel door steps and fifteen driver palette steps. | Source-language graphics selection via G2004, original emulator frame alignment, audible hardware output, wider gameplay and complete presentation parity. |
| Remaining Amiga editions | **Data path** | Version fingerprints and media/protection references are classified. | Authentic media plus per-edition native runtime proof. |
| FM Towns | **Verified route** | The supplied JA/EN original ZIP reaches `dm1-runtime` through direct CLI and start menu only after the native `TMENU.INF` selection and selected `EDM.EXP`/`JDM.EXP` receipt validate. The boot receipt publishes that selected program and its MD5 alongside the authenticated FM Towns graphics receipt. | Wider gameplay, executable/CD-audio parity and capture proof. |
| Apple IIGS 2.0/2.1 English | **Verified route** | Original ProDOS media binds `GRAPHICS.GAME`, `DUNGEON.GAME`, `DUNGEON.MASTER` and `SYSTEM/START` to one disk. `test_firestaff_iigs_cli_real_media.sh` reaches the first world through direct CLI and M12 start-menu launch for both editions, with native keyboard-turn input in the CLI route. The test uses SDL dummy output. | Original title/entrance, DOC playback, broader interactions and campaign; native display and visual parity are unverified. |
| PC-9801 | **Preservation** | Japanese floppy provenance is documented. | No Firestaff runtime route. |
| X68000 | **Unsupported** | Not part of the DM1 support matrix. | None planned in the current target. |

## Chaos Strikes Back

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| Atari ST | **Verified route** | Native STX title, 50 Hz `ANIMATE.SCR`/`ANIMATE.DAT` title path, FTLCODE handoff, first runtime HUD/viewport frame and start-menu CLI route are tested against supplied campaign media. The Atari presenter emits a nonzero FNV-1a receipt for its completed source-owned 224×136 indexed viewport; the full start-menu path reaches live gameplay at map 0, (9,0), facing south. ReDMCSB confirms the authentic new-game party is empty; an original-media CLI probe verifies one northward step to (9,1), but not a normal paced route to a mirror or successful champion recruitment. The supplied Atari save disk has no campaign save files. This remains separate from the PC F0128 session gate. The supplied nested retail archive is read directly as ZIP → ZIP → STX in memory; its `GRAPHICS.DAT` is hash-locked. The French preservation ZIP route was previously verified, but the archive is currently absent from the configured user data directory, so its CTest is skipped. | Normal paced campaign navigation, Champion Hall/party interaction, restored French-media verification, and capture parity. |
| Amiga | **Verified route** | Native A31E, A31M and A33M ZIP → ADF routes read graphics, dungeon and required launcher/language programs in RAM. A31E verifies `APPB.FTL` / `BJELoad_R` and reaches `csb-entrance-0`; A31M/A33M bind `TITL.DAT` beside the selected graphics file, whether the receipt is a virtual ADF member or an installed AmigaDOS directory, then use the source language-selection route to `APPB.FTL` / version-specific `KAOS.FTL`. A33M's distinct C03_GAME digest is hash-locked; original-media tests cover CLI ZIP → ADF, loose-folder CLI, and the ZIP-backed M12-to-M11 route through language choice and runtime. | Full campaign, save and visual parity. |
| FM Towns | **Verified route** | English/Japanese native packages have separate data, startup, Utility Disk and input paths. The authentic F31 title runs through `SWITCHTW`, the selected `CHTWE`/`CHTWJ` Game owner and the visible C004 Prison transition before `MINI.DAT` publishes its live party pose; the first source-owned turn is tested in both languages. Start-menu launch reports the source-bound `TITLE.ANM` palette/frame handoff and nonzero frame hash; an explicit F0435 save instead reports its verified C03 executable handoff. | Wider gameplay, Champion Editor and CD-audio parity. |
| PC-9801 | **Unsupported** | Preservation reference only. | None. |
| X68000 | **Unsupported** | Preservation reference only. | None. |

## Dungeon Master II: The Legend of Skullkeep

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| DOS | **Verified route** | English and French retail ZIPs have separate authenticated GDAT identities and native launch paths. English reaches a presented first runtime frame through M12 → MVE → New Game; French now reaches its initial party through that same ordinary start-menu route, with its source movement checked in a separate direct probe. All eight supplied `data/sksave0..3.{dat,bak}` members resume through the native start-menu route directly from the English distribution ZIP in memory. | Complete V1 renderer/mechanics parity, French presentation/input parity and full SKSAVE ownership. |
| Amiga | **Verified route** | Original Amiga archive reaches title, New Game, bounded runtime and the clipped native CHARSHEET inventory frame through verified GDAT/RAW4 material. | Wider gameplay, save and full pixel/audio parity. |
| FM Towns | **Verified route** | Authentic Towns data and platform-specific startup, input, CD-audio and gameplay slices are tested. The HUD portrait proof reads `GRAPHICS.DAT` through the supplied ZIP → original IMG → ISO chain in RAM and verifies all 16 original `CHAMPIONS` records; it never relies on an extracted `DATA/` directory. | Complete native runtime ownership and full parity. |
| Macintosh large retail | **Verified route** | Authentic English retail ZIP is read in RAM. HFS, big-endian dungeon data, New Game, bounded wall input, and an accepted real-GDAT frame with zero fallback draws pass. The normal Mac CLI loop now plays the complete authentic Title.MooV and reaches loaded runtime. Its 256-row `PalIRGB`/`dtPalette16` pair remains distinct from the Amiga physical 16-colour palette route. Native QuickTime code admits original `moov`/`mdat` sample spans and decodes all four retail films: Cinepak/`twos` Title, Animation RLE/`raw ` Swoosh, and Cinepak/`raw ` Credits/Ending. The normal-loop regression uses SDL dummy video/audio and does not verify a native window, HiDPI input or audible playback. No FFmpeg, BIOS, extraction, or host codec is used. | Complete Mac GAME_LOAD/Resume, dynamic pointer/drag owner, MIDI timing and full pixel/audio parity. No authentic Mac save is present. |
| Macintosh Japanese/French | **Preservation** | Authentic media is retained or classified as preservation input. | Separate graphics/dungeon/runtime ownership and language-specific proof. |
| PC-9821 Japanese retail | **Unsupported** | Removed from Firestaff discovery, launcher choices, CLI aliases and native media/runtime admission at the user's request. A regression test rejects explicit aliases and the authentic archive through both direct CLI and launcher discovery. Reference facts remain documentation only. | None; do not add a Firestaff route. |
| PC-9801 demo / IBM PS/V | **Preservation** | Separate Japanese demo and floppy media remain classified independently. | Native startup, game-view and input are not implemented. |
| X68000 | **Unsupported** | Not part of the DM2 support matrix. | None planned in the current target. |

## DM Nexus

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| Sega Saturn Japanese | **Verified bounded title route** | Authentic CUE/BIN media is retained as the sole source owner. Firestaff decodes the retail `TITLE.BIN`/`TITLE.CG` MAPD planes, selector sequence and BGR555 palettes in memory and presents that title sequence natively through both CLI and start-menu selection. Disc parsing, CDDA ownership, DMDF/DGN/MNS/PRS3 decoding and capture receipts remain separately source-bound. | A same-revision consumer capture for the menu/face compositor, HUD and dungeon display; native event/audio playback and full gameplay. |
| Saturn demo or fan translations | **Preservation** | Classified separately from the canonical Japanese retail route. | Independent media and runtime proof. |

## Theron's Quest

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| PC Engine/TurboGrafx-16 Japanese | **Verified bounded route** | Authentic Rev 1 CUE reaches native title, stage, Soul Room and Akutuba runtime (`party=1,0,0`). LOAD GAME → FILE_1 → YES returned to the selection map after emulator restart, but the 2-KiB BRAM is byte-identical to the known empty menu-only JP image; native campaign-save persistence is therefore unverified. The source-only Track 02 loader binds all seven campaign dungeons from raw BIN and the byte-matched CUE-projected ISO (34 maps, 2,269 linked source-object records; Drator: 8 maps/291 objects); the separate world ledger retains 2,266 ground-reference occurrences. Identity and level framing are verified; the generic first-floor/North party pose is still provisional and the fresh Akutuba RAM capture is not joined to an active map. | Complete campaign transitions, initial pose/map join, bitmap/palette and HUD binding, native save-body semantics and end-to-end gameplay. |
| PC Engine/TurboGrafx-16 US | **Verified bounded routes** | Authentic raw Track 02 BIN reaches native startup from direct launch and mouse-selected M12 game/platform/presentation cards. Authentic CloneCD ZIP (`.ccd` + bounded `.img` Track 02 slice) starts natively from memory through CLI and keyboard start-menu routes. The authentic CloneCD-derived raw CUE/BIN reaches native Track 02 startup using its compact no-pregap offsets. With the authentic Akutuba-complete Backup RAM, Continue restores the source-verified Theron attributes and skill experience, skips the completed first chapter, and reaches loaded dungeon 2 level 0 through its Track 02 roster and Soul Room route; three successive native movement inputs traverse a floor-only path in that actual loaded map. The MODE1/2048 ISO independently reaches Akutuba's source-backed forcefield handoff; all seven dungeon sources match the raw Track 02 user-data after its 225-sector pregap (34 maps, 2,269 source-object records). | Original-emulator parity for the Continue transition, JP Continue, later campaign progression, original gameplay presentation, broader save behavior, movement/mechanics parity beyond the bounded native route, combat/item/AI/audio parity and complete end-to-end gameplay. |

## Data and preservation rules

Firestaff reads the original file or container in its supplied form. It may
use temporary in-memory decoding during development, but production does not
extract game data to a Firestaff-owned directory. A DOSBox save is never
treated as a Macintosh save, and a parser or synthetic fixture never upgrades
a row to playable. See [game-data setup](DATA_SETUP.md),
[preservation](wiki/Preservation.md) and [verified hashes](VERIFIED_HASHES.md).
