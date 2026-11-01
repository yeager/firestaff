# Game and platform status

**Last status review: 2026-11-01.** This is the
operational status matrix. It separates media recognition, verified runtime
slices and end-to-end playability. A row marked as supported does not mean
that every menu, save format or visual parity gate is complete.

For Theron implementation and capture details, use the concise
[capture-readiness summary](theron/THERON_CAPTURE_READINESS.md); the dated
startup runs below are history and should not be read as complete parity.
The current T900 item-progression evidence boundary and five authentic US/JP
ground-placement capture candidates are documented in the
[T900 evidence report](source-lock/theron_t900_proof_2026-08-08.md) and its
[ground-item index](source-lock/theron-t900-ground-item-index-candidates-20261101.md).

The preceding functional revision `e70c460d` had successful GitHub Actions main CI run
[`40487479502`](https://github.com/yeager/firestaff/actions/runs/40487479502)
and Pages deployment run
[`40487479478`](https://github.com/yeager/firestaff/actions/runs/40487479478)
completed successfully after PR #142 merged. These checks establish that
revision's build, test and documentation deployment status; they do not
establish that all five games are playable. The original-media startup sweep
below is dated history and has not been rerun at that revision.

On 2026-10-20, the current `main` commit `f97398d8` passed another local
original-media startup sweep. DM1 passed the Atari ST 1.2, Amiga 3.6, English
DOS and FM Towns native CLI suites. Its mixed-library M12 test selected
FM Towns automatically and reached authenticated first runtime frames for
Atari ST, Amiga and DOS. CSB passed the Amiga, Atari STX and FM Towns native
CLI suites, including Atari's source C200 entrance event and the FM Towns
English `MINI.DAT` party. The DM1, CSB and DM2 return-to-menu matrix also
passed its configured original-media sessions, including availability
rescans, direct-CLI exit behavior, return during rescans, and discovery after
adding or removing media. `firestaff_cli_game_option_forms` passed its normal
no-game startup guard and CLI spelling checks. These runs use SDL dummy
video/audio and local original archives; they verify bounded startup and
launcher behavior, not native display, sound output or full visual parity.

Also on 2026-10-20, fresh isolated configurations started the application
without `--game` against a shared root containing only links to the installed
original DM1, CSB and DM2 data. The normal M12 main-menu keyboard flow selected
each game and reached its real-data runtime: DM1 selected FM Towns and loaded
HoC map 0 at (1,3); CSB selected FM Towns and loaded its `MINI.DAT` party on
map 4 at (22,18); DM2 selected FM Towns, completed all 225 original title
frames, reached the first champion on map 0 at (1,8), and accepted a turn.
The logs began with `startup game=none mode=menu` and recorded each actual
M12 game handoff. These SDL dummy sessions verify the previously untested
no-CLI-game main-menu selection path and bounded runtime entry, not native
presentation or audio output.

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

On 2026-10-31, the current `main` commit `17f9c2e41` passed a fresh
real-media startup sweep. `firestaff_cli_startup_diagnostics_real_media`
passed for all five games in 160.57 seconds. The three Swedish scan tests
passed for DM1 PC34, CSB FM Towns and DM2 FM Towns, covering explicit and
saved language selection in direct launch and the no-game menu. Both
unsupported-platform CLI gates also passed before renderer startup.
`m12_main_menu_game_selection_real_media` passed in 488.23 seconds against the
installed original-media collection. It verified keyboard selection for all
five game cards, authentic-media handoffs and the Nexus/Theron pointer cases.
The separate priority-game pointer test passed in 34.10 seconds for DM1, CSB
and DM2, and the scan-in-progress pointer test passed in 32.61 seconds. The
menu cases checked source hashes around the selected media. Together these
tests cover keyboard and mouse game-card selection for all five games, plus
the priority games' platform choices and deferred selection during scanning.
GitHub Actions run
[`41214887945`](https://github.com/yeager/firestaff/actions/runs/41214887945)
passed for this commit. These checks establish bounded CLI and menu startup
routes only. They do not establish full gameplay or native display/audio
parity, and Nexus remains gated before playable runtime.

After PR #233 merged, the updated `main` commit `a1f7901f5` passed a fresh
priority-game M12 selection test in 188.82 seconds. The menu pointer routes
for DM1, CSB and DM2 passed in 30.48 seconds, and the corresponding
scan-in-progress selection routes passed in 27.08 seconds. All cases used the
installed original-media collection and verified bounded game handoffs. They
do not establish full gameplay or visual parity. GitHub Actions run
[`41216993783`](https://github.com/yeager/firestaff/actions/runs/41216993783)
is the cross-platform verification for the merged revision.

On 2026-10-31, `firestaff_cli_startup_diagnostics_real_media` passed in
171.94 seconds against the installed original-media root. The test checked
CLI startup diagnostics for DM1, CSB, DM2, Nexus and Theron. The separate
`m12_main_menu_game_selection_real_media` test passed in 435.08 seconds and
checked keyboard and pointer game-card selection and launch handoff for all
five games. These tests cover launcher and bounded startup routes; they do not
establish complete gameplay or visual/audio parity, and Nexus remains
capture-gated at its Saturn title.

The same day's `dm2_v1_mac_extracted_native_cli_boot` passed in 210.37 seconds.
It exercised DM2 Macintosh direct CLI launch, selected-game menu launch and
first-run game discovery from a ZIP made in test scratch from the supplied
authentic extracted Mac files, including their AppleDouble sidecars. The ZIP
retained the original file bytes and was not presented as a reconstructed CD
image. Hash checks confirmed that the supplied files remained unchanged.
These checks prove the tested bounded Macintosh startup routes, not complete
Macintosh sound or gameplay parity.

The same day's primary-platform startup matrix passed for DM1 DOS PC34,
Atari ST 1.1, Amiga 3.6, FM Towns and Apple IIGS; for CSB Atari ST, Amiga 3.5
and FM Towns; and for DM2 English/French DOS, Amiga and FM Towns. The tests
used the installed original-media collection and exercised the maintained CLI
and start-menu routes. The FM Towns DM1 test was repeated after one sampled
door-frame interval measured 61 ms against its 60 ms per-frame ceiling; the
test now allows up to 65 ms of host scheduler jitter while retaining its
45 ms minimum and 1,450–1,800 ms total-door-duration bounds. The repeated test
passed. Authentic Amiga 2.0 German media also exposed a false F0906/F0907
admission check: ReDMCSB's MEDIA441 compiles that check for A20E, A20F and A21E,
not A20G/A22G. After correcting the edition gate, German A20 direct CLI, M12
start-menu and first-Hall checks passed; neighboring A20 English/French, A21/
A22 English, A22 German and Amiga 3.6 startup checks passed as well. These
bounded routes do not establish visual, audio or complete gameplay parity.

On 2026-10-28, `test_m12_main_menu_game_selection_real_media.sh` passed
against the current working tree and the installed original-media root. It
verified keyboard and mouse selection for all five game cards, plus default-
root menu handoff for DM1, CSB, DM2, Nexus and Theron. These card and handoff
receipts do not claim that every selection reaches a playable runtime: Nexus
remains capture-gated at its Saturn title. The 2026-10-27 first-runtime matrix
and reproduction details remain in the [five-game startup
analysis](STARTUP_SUPPORT_GAP_ANALYSIS.md#latest-local-real-media-menu-check-2026-10-27).
The SDL dummy run does not establish native-display, audio or physical-input
parity.

Recent reports of DM1/CSB input and presentation faults and DM2/FM Towns
palette/menu/dungeon faults remain open parity work until they are reproduced
against matching original media and resolved with source or capture evidence.
A green launch test below is therefore not a substitute for visual, audio,
input or campaign parity.

On 2026-10-27, the local startup sweep ran against `main` snapshot
`f6c7ad006` before unrelated Theron commits were merged. DM1 passed
`dm1_v1_pc34_native_cli_boot`,
`dm1_v1_atari_st_archive_cli_boot`,
`dm1_v1_amiga_v20_archive_cli_boot`, and
`dm1_v1_fmtowns_archive_cli_boot` for English and Japanese routes. CSB passed
`csb_v1_atari_stx_native_cli_boot`, `csb_v1_amiga_native_cli_boot`, and
`csb_v1_fmtowns_native_cli_boot`. DM2 passed
`dm2_v1_dos_native_cli_boot`, `dm2_v1_dos_fr_native_cli_boot`,
`dm2_v1_mac_native_cli_boot`, `dm2_v1_amiga_native_cli_boot`, and
`dm2_v1_fmtowns_native_cli_boot`. The initial Towns invocation skipped
because its source-menu RGB helper had not been built in the incremental
build directory; after building that test target, the same real-media test
passed. The original game archives were read in place and left unchanged.
These SDL dummy-driver checks establish the tested CLI/menu startup and
bounded runtime routes only; they do not close native display, HiDPI, physical
input, audible output, visual parity or complete-gameplay gaps.

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
| Amiga 2.0 English/French/German | **Verified route** | The supplied preservation ZIP → original ZIP → ADF route is read in RAM. English is hash-locked at `GRAPHICS.DAT` MD5 `6a2f135b53c2220f0251fa103e2a6e7e`; French and German original ADFs are hash-locked before being repacked byte-for-byte as ZIP for the product's ZIP/ADF readers. All three languages reach the bounded native runtime through direct CLI and M12 start-menu handoff with the selected original title, entrance and paired dungeon. The supplied HD package (`Dungeon Master (1988)(FTL)[HD].zip` → ADF) also reaches runtime. The original save-disk `DMGAMEG.DAT`/`.BAK` is authenticated in memory through its Amiga-specific F0435 parts and F0434 dungeon checksum; it stays separate from PC34 data. | Format-specific save-world materialization, broader gameplay, Amiga-specific presentation/input ownership and capture parity. |
| Amiga 2.1/2.2 English | **Verified route** | The original Amiga software archive's ADF members are hash-checked and repacked byte-for-byte as ZIP for transport. Both editions reach `dm1-runtime` through direct CLI and M12 start-menu launch after their native F0437 title and F0441 entrance sequences; the paired original dungeon binds to the authenticated owner. | Broader gameplay and complete presentation/input parity. |
| Amiga 2.2 German | **Verified route** | The original 910,058-byte ADF member is hash-locked at SHA-256 `5abc4f549941eed4e6af48bd52d348212b38b0a17fd3e83609c8aaabc2e4cd73`, then exercised through ZIP transport and direct disk selection. CLI and M12 routes complete native F0437/F0441 startup and reach the first runtime pose. | Original emulator frame alignment, broader gameplay, audio and complete presentation parity. |
| Amiga English Demo | **Verified route** | The supplied original demo archive reaches the first Hall runtime frame through direct CLI, the M12 no-game picker, and loose-file M12 discovery. The real-media test checks the original graphics and dungeon identities and confirms the supplied files remain unchanged. | The demo's original campaign limit; this route does not establish retail-edition gameplay parity. |
| Amiga 3.6 English/French/German startup | **Verified route** | The authentic original ZIP is hash-locked at `GRAPHICS.DAT` MD5 `7f9458e4a3972d06e649a6fa85a7f34b`. English direct CLI archive and discovered-directory routes, plus M12 start-menu handoff, reach the first runtime pose through the authenticated A36 entrance path. The original-media regression also starts French and German through direct CLI `--lang`, selecting the exact `DUNGEONF.DAT` or `DUNGEONG.DAT`, and through M11 runtime/M12 handoffs from loose files and ZIP; wrong-language files are rejected. Tests cover the native C696 zones, all 64 two-pixel door steps, fifteen driver palette steps and the G2004 English/French/German graphics map. Seven original localized bitmap payloads are verified; three mappings currently have index-level evidence only. | Original emulator frame alignment, audible hardware output, wider gameplay and complete presentation parity. |
| Additional Amiga media formats | **Data path** | The supplied software archive includes the official SPS IPF 3.6 image and unofficial IPF variants. Current end-to-end startup proofs cover the identified ADF/ZIP editions; these protected-track IPF images are not yet admitted by the native Amiga media path. | Bounded IPF decoding, protection-aware media validation and startup proof against the original SPS image. |
| FM Towns | **Verified route** | The supplied JA/EN original ZIP reaches `dm1-runtime` through direct CLI and start menu only after the native `TMENU.INF` selection and selected `EDM.EXP`/`JDM.EXP` receipt validate. The boot receipt publishes that selected program and its MD5 alongside the authenticated FM Towns graphics receipt. | Wider gameplay, executable/CD-audio parity and capture proof. |
| Apple IIGS 2.0/2.1 English | **Verified route** | Original ProDOS media binds `GRAPHICS.GAME`, `DUNGEON.GAME`, `DUNGEON.MASTER` and `SYSTEM/START` to one disk. `test_firestaff_iigs_cli_real_media.sh` reaches the first world through direct CLI and M12 start-menu launch for both editions, with native keyboard-turn input in the CLI route. The test uses SDL dummy output. | Original title/entrance, DOC playback, broader interactions and campaign; native display and visual parity are unverified. |
| PC-9801 | **Preservation** | Japanese floppy provenance is documented. | No Firestaff runtime route. |
| X68000 | **Unsupported** | Not part of the DM1 support matrix. | None planned in the current target. |

## Chaos Strikes Back

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| Atari ST | **Verified route** | Native STX title, 50 Hz `ANIMATE.SCR`/`ANIMATE.DAT` title path, FTLCODE handoff, first runtime HUD/viewport frame and start-menu CLI route are tested against supplied campaign media. The Atari presenter emits a nonzero FNV-1a receipt for its completed source-owned 224×136 indexed viewport; the full start-menu path reaches live gameplay at map 0, (9,0), facing south. ReDMCSB confirms the authentic new-game party is empty; an original-media CLI probe verifies one northward step to (9,1), but not a normal paced route to a mirror or successful champion recruitment. The supplied Atari save disk has no campaign save files. This remains separate from the PC F0128 session gate. The supplied nested retail archive is read directly as ZIP → ZIP → STX in memory; its `GRAPHICS.DAT` is hash-locked. The French preservation ZIP route was previously verified, but the archive is currently absent from the configured user data directory, so its CTest is skipped. | Normal paced campaign navigation, Champion Hall/party interaction, restored French-media verification, and capture parity. |
| Amiga | **Verified route** | Native A31E, A31M and A33M ZIP → ADF routes read graphics, dungeon and required launcher/language programs in RAM. A31E verifies `APPB.FTL` / `BJELoad_R` and reaches `csb-entrance-0`; A31M/A33M bind `TITL.DAT` beside the selected graphics file, whether the receipt is a virtual ADF member or an installed AmigaDOS directory, then use the source language-selection route to `APPB.FTL` / version-specific `KAOS.FTL`. A33M's distinct C03_GAME digest is hash-locked; original-media tests cover CLI ZIP → ADF, loose-folder CLI, and the ZIP-backed M12-to-M11 route through language choice and runtime. The F0107 wall-ornament path uses source MEDIA720 scale selection and type-0/type-7 anchors decoded from authentic Amiga Graphics.DAT item 696; focused checks validate real-data parsing and anchor values, not the final framebuffer. A31M/A33M SWSH parsing also verifies the authentic palette, logo and sample data. | Matched original frame; palette/light, alcove and inscription appearance; floor trapdoor visibility; native intro timing and audible output; full campaign, save and visual parity. |
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
| Macintosh Japanese/French localized editions | **Preservation** | The supplied French-named SIT is hash-identified as the known English retail runtime tuple; its real-media test verifies the English Macintosh CLI route and an accepted source-asset viewport/HUD frame. It is not evidence of a French runtime. No separate Japanese retail runtime set is staged in the configured data root. | Authentic localized runtime files and independent source-owned startup proof. |
| PC-9821 Japanese retail | **Unsupported** | Removed from Firestaff discovery, launcher choices, CLI aliases and native media/runtime admission at the user's request. A regression test rejects explicit aliases and the authentic archive through both direct CLI and launcher discovery. Reference facts remain documentation only. | None; do not add a Firestaff route. |
| PC-9801 demo / IBM PS/V | **Preservation** | Separate Japanese demo and floppy media remain classified independently. | Native startup, game-view and input are not implemented. |
| X68000 | **Unsupported** | Not part of the DM2 support matrix. | None planned in the current target. |

## DM Nexus

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| Sega Saturn | **Verified bounded title-asset route; not playable through Firestaff** | After M12 admits a recognized full image, an authentic Japanese CUE-only disc, or an extracted owner, CUE/BIN members can supply title assets; Firestaff decodes and renders the five retail `TITLE.BIN`/`TITLE.CG` MAPD planes with their palette in a bounded sequence; this is not the complete native Saturn title or menu. The CLI and start-menu paths stop at the capture-gated title/menu handoff. The real-media CUE-only regression verifies direct CLI and pointer-menu admission, retains the selected CUE as the runtime media owner, and rejects a selected non-Nexus CUE even when valid Nexus media is also present. Japanese title-session traces bind the `TITLE.CG` upload, MAPD palette and NBG0 bitmap producer; the display consumer and full composition remain unbound. Ymir references verify Japanese and English no-save cold boots to the native three-choice title menu; a separate Japanese New Game replay reaches a native 3D corridor viewport. The English screenshot replay is pixel-exact with 126 informative retail-byte matches among 137 textured draws; 11 zero-filled spans are excluded. These are emulator checkpoints, not Firestaff native-menu or gameplay evidence. Merged PR #19 adds a capture-only NBG1 tile-fetch address correction tested on authentic startup data. Disc parsing, CDDA ownership, DMDF/DGN/MNS/PRS3 decoding and capture receipts remain source-bound. A real-media regression now authenticates extracted JP/EN/FR ownership using regional `RLOWFIX.BIN` plus at least two localized title/menu/font hashes and admits supplemental disc media only when canonical shared `DM.BIN` and those regional members agree. Authentic JP and EN menu tests passed on TRV2, and the Japanese CUE was rejected for an EN extraction. The native title/menu compositor gate remains closed; these handoffs do not establish a playable route. See [Nexus TODO](../TODO-nexus.md#2026-10-29--keep-extracted-nexus-resources-and-supplemental-media-in-one-region). | Complete Saturn title/menu rendering and input, verified edition ownership for mixed extracted/disc directories, same-revision menu/face/HUD/dungeon consumer evidence, native event/audio playback and full gameplay. |
| Saturn demo or fan translations | **Preservation** | Classified separately from the canonical Japanese retail route. | Independent media and runtime proof. |

## Theron's Quest

**Native source-receipt update (2026-11-01):** the native `--debug` spawn
receipt was verified in three JP and three US gameplay-startup loops using
authentic raw Track 02 BINs, then in three more loops per region using the
authenticated split MODE1/2048 ISO images. All twelve runs reached a loaded
level, emitted one table header, eight pointer rows and five zone rows, and
matched every field to the corresponding media projection with the reusable
verifier. This verifies metadata reporting only; original spawn
selection/semantics and creature behavior remain unverified. See the
[receipt report](source-lock/theron-track02-spawn-debug-receipt-20261101.md).

**JP map-data update (2026-11-01):** an offline scan of an authentic JP
`.mc0` state found the exact Akutuba descriptor and all four map arrays from
Track 02 in its inflated payload. A source-locked Mednafen 1.32.1 inspector
maps the match to the named HuC CD-RAM variable at physical page `$74`
(`$0E891D`); the saved MPRs do not select that page. All four arrays are
resident, so this does not identify the active map, a runtime consumer, or a
rendered result.
See the [source-map report](source-lock/theron-disassembly/theron-jp-akutuba-map-source-sector-20261101.md#offline-save-state-source-match-2026-11-01).

**US saved-state map update (2026-11-01):** a read-only comparison against
authenticated US Track 02 finds all four Akutuba arrays in the authentic
standard-PCE state's `HuC.SysCardRAM`. Saved MPR3 selects page `$74` in the
`$6000–$7fff` window. Two prior collision-trace addresses align with map-0
bytes only conditionally because that trace has no same-event MPR snapshot.
The saved mapping does not identify the active map, tile consumer, movement
semantics or rendered result. See the [US state map report](source-lock/theron-us-authentic-standard-pce-state-map-binding-20261101.md).

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| PC Engine/TurboGrafx-16 Japanese | **Verified bounded native route** | Firestaff runs Theron in its native C/SDL runtime. Native SDL regressions cover DM1-like keyboard and mouse-panel movement-command dispatch, champion selection and wait against authentic JP Track 02; they do not verify retail click hitboxes or original-game movement semantics. A cold JP reference capture confirms four-second Button I holds on NEW GAME and FILE 1 advance into Akutuba; native hold-duration parity and a cold game-owned handoff remain unproven. Native JP fresh-start integration reaches a loaded dungeon. Native M11 rejects the authentic empty JP BRAM; a progressed JP BRAM is not available. Hash-verified US/JP ISO projections bind the same eight creature-pointer and five regular-spawn zone records as their raw BINs. In both regions, pointer records 0–4 resolve in order to the five zone records and 5–7 have zero offsets; runtime selection, JP category semantics and spawn/RNG consumers remain unverified. An earlier authenticated state-only replay consumed five scripted control events but recorded zero spawn consumer/register samples, zero RNG samples, and no dungeon transition; it does not prove movement or spawn semantics. The latest matched JP state-only UP/control pair verifies a guest read at `$44C1` and 256 identical authentic-source-matched data reads per run before the map arrays; no movement-command write was observed, so movement and renderer semantics remain open. The current isolated PCE Fast reference instrumenter has been rebuilt against standalone SDL 2.32.10, but its latest capture is not yet accepted. Production CDDA only routes title Track 01; gameplay event-to-track routing for tracks 03–18 is unverified, and the retail square/material-to-tile and pose-to-frame renderer join is still missing. See the [matched trace report](source-lock/theron-disassembly/theron-jp-state-only-up-control-data-read-20261101.md). A source-locked disassembly maps the `$50D2` reader window and a bounded trace observes reads of a candidate pointer-like list tail, but its relation to an active map or tile consumer remains unproven. Three authentic JP state-only traces recorded 1,677, 1,513 and 1,556 dynamic map-array reads across 17 source-matched byte addresses in Map 0 and Map 3, with no 4,096-row cap reached. The active map and pose-to-render join remain unproven. Hash-verified raw-BIN windows disassemble all four reader paths; the compiled v4 PCE Fast hook adds opcode/register/Page1/MPR context, but no gameplay trace has yet been captured. See the [capture report](source-lock/theron-disassembly/theron-jp-state-only-map-array-read-capture-20261101.md) and [reader report](source-lock/theron-disassembly/theron-jp-akutuba-map-index-reader-20261101.md). Three matched state-only PCE Fast UP/control pairs repeat Button II+UP-conditioned `$C1FC` TII transfers from `$20B4` into candidate `$2040/$2041` bytes; a follow-up records the same 128-instruction context into `$C1FC`. Static source associates this TII with teleporter destination copying, so it does not prove player movement. A separate state-only trace validates 4,096 gated VRAM commits after a guest read in shared event order only. | Map/field meaning, JP category behavior, runtime spawn/RNG consumer, movement versus teleporter role, collision, visible input effect, dynamic viewport and paired retail frame, cold CD-to-RAM handoff, positive JP save/Continue, later dungeons, combat, item-use and full audio/gameplay parity remain open. See [capture readiness](theron/THERON_CAPTURE_READINESS.md), the [spawn-record source lock](source-lock/theron-track02-spawn-record-iso-projection-20261101.md), the [runtime spawn capture](source-lock/theron-disassembly/theron-runtime-spawn-capture.md) and the [TII control-flow report](source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md). |
| PC Engine/TurboGrafx-16 US | **Verified bounded routes** | Native direct launch bypasses Firestaff's Run menu and System Card; the Debug probe itself stopped at `levelLoaded=0`. Authentic US Track 02 plus matching Ak-Tu-Ba-complete BRAM reaches dungeon 2 through Continue. Native `--debug` and SDL-loop regressions cover bounded keyboard and mouse-panel command dispatch, champion cycling, wait, and pickup/reselect/drop of an authentic floor item; they do not establish original-game movement semantics or mouse USE effects. A fresh native macOS Debug event-loop run passed three consecutive executions on hash-verified US Track 02; see the [input run receipt](source-lock/theron-native-sdl-input-real-media-20261101.md). An isolated cold reference replay reached Drator `start`, `left-event` and reentry with 132 `$C852` fetches; its first five 512-byte windows at physical `$0DC852` exactly match US Track 02 raw offset `$A3AD2`. The same-session trace proves writes and reads of generated row offsets 6–8 and a later ten-byte TII copy into a work buffer. Static reader windows at `$CBC6` and `$45C3` now match authenticated US Track 02 bytes; the verifier reports four and three possible raw offsets, respectively. This validates a constructed retail-byte fixture, not the live code occurrence or a unique LBA join. The observer reached its cap after these reads; the route subsequently reached reentry. The streaming verifier processed the explicitly truncated 8,388,608-row BIOS trace at low memory and correctly rejected its marker. Status and callback sidecars passed; the wrapper rejected the full capture package. The later cold run used source-scoped BIOS and `drator-row` filters, but the package remained rejected because of the live `$45C3` mismatch and trace caps. A later authentic JP state-only `$3879` trace reached the shared 8,192-row cap before `$C852`; its separate first-hit sidecar reported zero rows (`not_observed`), and JP `$C85E` was opcode `$A6` at physical `$0D285E`, with no `$5039` execution. A later authentic US cold trace records ordered `$C852` → `$C85E` `JSR $5039` → `$5039` dispatch under MPR2 `$68`; the live target bytes differ from the static stage-two candidate. The wrapper rejected this package because live `$45C3` did not match retail Track 02 and status/consumer traces hit their caps. No contiguous 8 KiB page in raw US Track 02 or Track 19 matched the 102 unique MPR2 `$68` code windows; a separate authentic `$23AD` output matched 0/102 windows, while other decode or runtime-write sources remain possible. C852 schema v3 and the all-writer trace now share a per-run capture ID; the wrapper receipt binds their hashes, full-page range and completeness counts. The join verifier has fixture coverage, but a fresh cold capture is still required before causal attribution. | Live reader source-occurrence/LBA binding; downstream buffer semantics; an untruncated handoff capture; US native title-hold parity capture; later progression, JP Continue, authentic gameplay presentation, broader save behavior, combat, item-use, AI, audio and complete parity remain open. See the [ordered call-window report](source-lock/theron-disassembly/theron-us-c852-runtime-call-capture-20261101.md), [runtime-page source scan](source-lock/theron-disassembly/theron-us-mpr2-68-runtime-page-source-scan-20261101.md) and [Drator capture boundary](source-lock/theron-us-drator-c852-cold-capture-20261031.md). |

The reference-only Mednafen instruction-start trace patch sequence passed
three clean patch-only runs and the instrumented PCE build/install on TRV2.
Controller dry-run and mouse parser checks each passed three loops. A follow-up
cold US run validates three scripted inputs at System Card controller polls
and records one `$E009` dispatch, but observes no game-owned poll or runtime CD
source-read join. This is not evidence of an authenticated handoff or gameplay
and does not alter the native routes above. A later matched three-run JP PCE Fast state-only comparison records repeatable `$20B4->$2040/$2041` TII transfers after Button II+UP guest reads; three Button II-only controls retain the prior candidate bytes. This is input-conditioned transfer evidence, not yet mapped movement or proof of field meaning, collision behavior, visible effect, cold CD handoff, or parity. The earlier F5 wall probe remains a separate no-transfer result. See the [state-only comparison](../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md), [capture-readiness record](theron/THERON_CAPTURE_READINESS.md), and
[source-locked follow-up report](source-lock/theron-disassembly/theron-us-cold-scripted-input-returned-mask-20261031.md).

A separate US saved-state PCE Fast capture loaded its existing state and
recorded 1,354 short-format rows at `$44CA/$44DB`. Standard-PCE and PCE Fast
instrumentation share the output path and row label, so these are not verified
PCE Fast read receipts despite separate scripted input-apply records. RAM,
VRAM, and SAT snapshots differ from control, but the changes are not joined to
received input, party pose, map cell, or a display consumer. This adds no
verified action or renderer mapping; see the
[saved-state capture report](source-lock/theron-disassembly/theron-us-state-scripted-input-boundary-20261031.md).

A separate corrected PCE Fast capture from an authentic Japanese Akutuba
state validates 4,096 bounded VRAM commit rows after the guest read of scripted
UP at `$44C1`. The input read and first commit are adjacent in shared event
order (`58758` and `58761`); the capture uses state-only mode and reports
`transition=missing`. This confirms trace-gate operation and event ordering
only. It does not establish causality, VRAM meaning, a visible response,
game-owned cold handoff, or Firestaff parity. See the
[source-locked input-gated trace report](source-lock/theron-disassembly/theron-jp-pce-fast-input-gated-vram-trace-20261031.md).

## Data and preservation rules

Firestaff reads the original file or container in its supplied form. It may
use temporary in-memory decoding during development, but production does not
extract game data to a Firestaff-owned directory. A DOSBox save is never
treated as a Macintosh save, and a parser or synthetic fixture never upgrades
a row to playable. See [game-data setup](DATA_SETUP.md),
[preservation](wiki/Preservation.md) and [verified hashes](VERIFIED_HASHES.md).
