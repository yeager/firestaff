# ReDMCSB Firestaff complete audit

Updated: 2026-11-01

This is the DM1/CSB source-review contract for comparing ReDMCSB against
Firestaff. ReDMCSB is the primary functional reference for DM1 and CSB V1.
If Firestaff and ReDMCSB disagree without stronger original-runtime evidence,
Firestaff is treated as wrong. DM2 must be reviewed against skproject; DMWeb
and Greatstone provide supporting reference material for all three games.
A game-level result is complete only when every supported platform lane is
covered independently.

## Scope

The requested parity target covers all three games and every platform lane
Firestaff currently supports. The current documented runtime lanes are:

| Game | Supported platform lanes in Firestaff |
|---|---|
| DM1 | PC DOS, Atari ST, Amiga, FM Towns, Apple IIGS |
| CSB | Atari ST, Amiga, FM Towns |
| DM2 | DOS, Amiga, FM Towns, Macintosh |

These are separate implementation targets: evidence from one platform or
edition does not close another lane. DM1 and CSB behavior uses ReDMCSB as the
primary functional source; DM2 uses skproject. DMWeb and Greatstone are
supporting references for all three games, with claims tied to the relevant
game, edition, and asset identity.

## Platform coverage register

The repository currently documents selected runtime routes for these lanes.
Those routes are evidence of bounded operation, not proof that every source
function is implemented on that platform. A lane remains `PARTIAL` until its
source functions are mapped to Firestaff behavior and platform-specific gates.

| Game | Platform lane | Existing bounded-route evidence | Parity status |
|---|---|---|---|
| DM1 | PC DOS | `README.md` platform summary; `docs/parity/PARITY_MATRIX_DM1_V1.md` (PC DOS target) | PARTIAL |
| DM1 | Atari ST | `README.md` platform summary; `docs/parity/REDMCSB_DM1_CSB_AUDIT.md` | PARTIAL |
| DM1 | Amiga | `README.md` platform summary; `docs/parity/REDMCSB_DM1_CSB_AUDIT.md` | PARTIAL |
| DM1 | FM Towns | `README.md` platform summary; `docs/parity/REDMCSB_DM1_CSB_AUDIT.md` | PARTIAL |
| DM1 | Apple IIGS | `docs/PLATFORM_STATUS.md` Apple IIGS 2.0/2.1 row; `test_firestaff_iigs_cli_real_media.sh` verifies bounded startup only. ReDMCSB `OBJECT.C` F0031 MEDIA060 includes G14ED/G20E/G21E and specifies a 199-name high-bit-terminated M564 stream. Firestaff's native IIGS startup now reads GRAPHICS.GAME record 556 and routes it through the 199-entry high-bit-terminated parser before inventory labels; an authentic IIGS full-table comparison, runtime proof, and broader source-function parity remain open. | PARTIAL |
| CSB | Atari ST | `docs/parity/PARITY_MATRIX_CSB_V1.md`, original-media route table | PARTIAL |
| CSB | Amiga | `docs/parity/PARITY_MATRIX_CSB_V1.md`, original-media route table | PARTIAL |
| CSB | FM Towns | `docs/parity/PARITY_MATRIX_CSB_V1.md`, original-media route table | PARTIAL |
| DM2 | DOS | `docs/parity/DM2_V1_REAL_MEDIA_RUNTIME_AUDIT.md`, DOS English/French rows | PARTIAL |
| DM2 | Amiga | `docs/parity/DM2_V1_REAL_MEDIA_RUNTIME_AUDIT.md`, Amiga row | PARTIAL |
| DM2 | FM Towns | `docs/parity/DM2_V1_REAL_MEDIA_RUNTIME_AUDIT.md`, FM Towns row | PARTIAL |
| DM2 | Macintosh | `README.md` Macintosh English route; Macintosh First Chapter demo remains explicitly unsupported | PARTIAL |

Current inventory snapshot (2026-11-01): the ReDMCSB full callable ledger has 2,133 rows: 2,100 functions (759 `MISSING`, 485 `UNCERTAIN_NUMBERED_EVIDENCE`, 536 `IMPLEMENTED_NARROW`, 95 `VERIFIED_SOURCE_MAPPING`, 70 `SOURCE_NONAPPLICABLE`, 150 `PARTIAL`, and 5 `PC34_SOURCE_IMPLEMENTED`) plus 33 auxiliary rows. Across all 2,133 rows, the current counts are 771 `MISSING`, 505 `UNCERTAIN_NUMBERED_EVIDENCE`, 537 `IMPLEMENTED_NARROW`, 95 `VERIFIED_SOURCE_MAPPING`, 70 `SOURCE_NONAPPLICABLE`, 150 `PARTIAL`, and 5 `PC34_SOURCE_IMPLEMENTED`. The full ledger currently has 1,426 open rows when `MISSING`, `UNCERTAIN_NUMBERED_EVIDENCE`, and `PARTIAL` are counted together; this is a triage count, not a claim that all are applicable game features. ReDMCSB `DUNVIEW.C:F0112/F0118-F0123/F0125` now map to partial ceiling-pit and viewport square dispatchers; full element ordering and platform routes remain open. F0127 D0C and F0128 per-square scheduling are also partial, with material/element rendering and complete overlay/platform parity still open. F0129 has bounded PC3.4, CSB, and FM Towns sampling routes; IIGS title sampling and cross-platform pixel parity remain open. F0130-F0133 map bounded PC34 flip/blit primitives and the DM1 masked-field renderer; generic packed-mask and all-platform draw semantics remain open. F0136 has a bounded checkerboard hatch owner. F0137 is `MISSING`: Firestaff has no hidden-code launcher or executable patcher. ReDMCSB `DUNVIEW.C:F0099/F0116/F0117` now map to partial row-flip and PC34 square-dispatch owners; complete bitmap ownership and platform dispatcher parity remain open. ReDMCSB `DUNVIEW.C:F0095` now maps to partial Firestaff map-wall-set resolution; edition-specific preload/cache ownership remains open. ReDMCSB `STRING.C:F0090/F0091` now maps to bounded host-libc search behavior, with `F0091` partial because the source does not match the terminating NUL. ReDMCSB compiler-runtime symbols `DEFS.H:F0082-F0088` now distinguish Megamax arithmetic/structure helpers from ordinary `strcat`/`strcmp`/`strcpy` host-libc mappings. ReDMCSB `COPYPRO1.C:F0079` and its Atari ST alias are now correctly marked `MISSING`: Firestaff has no equivalent copy-protection code checksum. ReDMCSB `IO.C:F0074-F0078` now map to bounded SDL host-cursor visibility and input event paths as `PARTIAL`; source bitmap save/restore, Atari ST interrupts, and paired redraw visibility timing remain open. ReDMCSB `MOUSESET.C:F0066-F0071/F0073` now map to bounded host-pointer, champion-icon formation pickup/drop, and PC34 screen-area receipt counterparts as `PARTIAL`; source cursor raster, Atari ST driver and cross-platform save/restore behavior remain open. ReDMCSB `SOUND.C:F0061-F0065` now map to bounded PSG amplitude-table, DM1 sound-state/host initialization, sound-event arbitration, and pending-event flush counterparts as `PARTIAL`; Atari ST key-click/PSG/Timer-A ownership and all-edition audio-driver and timing parity remain open. ReDMCSB `TEXT.C:F0042-F0047` now map to bounded PC34 message-area cursor, row, and print counterparts as `PARTIAL`; bitmap/font/pixel and cross-platform integration remain open. ReDMCSB Atari ST aliases `F031-F039` now map to the bounded DM1/CSB name, object/icon and slot-box counterparts as `PARTIAL`; Atari ST runtime/ABI and authentic asset behavior remain open. ReDMCSB `BASE.C:F0022` and `SWSH.C:F0022` now map to source-gated VBlank delay and millisecond-estimated swoosh-hold counterparts as `PARTIAL`; frame-exact all-platform timing remains open. ReDMCSB `BASE.C:F0020/F0021` now map to Firestaff source-named screen/viewport blitters and bounded PC34 admission as `PARTIAL`; live M11 use and complete platform bitmap behavior remain open. ReDMCSB Atari ST aliases `F023-F030` now map to their bounded PC34 math/RNG helpers as `PARTIAL`; the Atari ST runtime owner, ABI, RNG and overflow behavior remain open. ReDMCSB `BASE.C:F0009/F0010` now map to the shared Firestaff spaced byte/word writers for positive counts as `PARTIAL`; some assembly source lanes wrap zero count to 65,536 writes. ReDMCSB `BASE.C:F0006` now maps bounded Firestaff menu/champion-panel highlight overlays as `PARTIAL`; the generic inclusive source-screen rectangle inversion remains unimplemented/unverified. ReDMCSB `CLEARBYT.C:F0008` now maps to the shared Firestaff `memset` wrapper for valid bounded clears as `PARTIAL`; source pointer/count widths and ABI behavior remain open. ReDMCSB `COPYBYTE.C:F0007` now maps to the shared Firestaff `memmove` wrapper for positive-length overlapping copies as `PARTIAL`; platform-specific assembly, pointer/size domains, and ABI behavior remain open. ReDMCSB `GAMELOOP.C:F0002` now maps to Firestaff's bounded tick adapter and live M11 DM1 loop as `PARTIAL`; full edition-specific loop order, platform drivers, and watchdog behavior remain open. ReDMCSB `NEWMAP.C:F0003` now maps to the staged M11 group/map transition as `PARTIAL`; M11 now selects the destination track after its shared movement/teleporter handoff; stairs and post-move transitions have separate track paths, and rendering resolves active-map graphics/palette. Complete caller ordering and ReDMCSB copy-protection event behavior remain unverified across editions/platforms. The largest missing-function families are platform/toolchain contracts (`AES`, `VDI`, `FIO1`, `VDEO`, floppy, and ABI support), so they require per-edition reachability review before being called game-feature gaps. The DM2 active-source inventory has 3,934 definition rows (SPX v4 1,118; SPX v5 1,347; shared SKWINSPX 262; legacy DOS 832; SKWINDOS/src 373; SKULLWIN 2). Applying the active-source review overrides gives 3,282 unreviewed, 581 partial, 5 missing, and 56 implemented-parity definitions. There are 8 architecture-specific source entries and two Bloodwych-only definitions excluded from this three-game audit. Compared DOS `SKW_2066_398a`, SPX v4 `DIALOG_SAVEGAMES_NAMES`, and SPX v5 `DM2_dialog_2066_398a`. Firestaff opens the save panel but does not draw the ten source save-slot names or selection highlight rows; SPX v4 also has extended-mode empty/DM1/non-DM slot colors. All three definitions remain `PARTIAL`. Active-source counts: 3,341 unreviewed, 522 partial, 5 missing and 56 implemented-parity; combined backlog: 3,943 (3,341 unreviewed, 595 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_2066_3820`, SPX v4 `DIALOG_2066_3820`, and SPX v5 `DM2_dialog_2066_3820`. Firestaff has a matching GDAT panel draw plan and renderer, but the host-command path is test-only, there is no production M11 binding, and the renderer does not draw the supplied text. All three definitions remain `PARTIAL`. Active-source counts: 3,345 unreviewed, 518 partial, 5 missing and 56 implemented-parity; combined backlog: 3,943 (3,345 unreviewed, 591 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW__OPEN_DIALOG_PANEL`, SPX v4 `__OPEN_DIALOG_PANEL`, and SPX v5 `DM2_dialog_OPEN_DIALOG_PANEL`. Firestaff has a live M11 save-panel renderer consuming authenticated GDAT image/text/font/palette data, but generic panel inputs, source allocation and teardown, input/mouse transitions and supported platform/edition paths remain open. All three definitions are `PARTIAL`. Active-source counts: 3,348 unreviewed, 515 partial, 5 missing and 56 implemented-parity; combined backlog: 3,943 (3,348 unreviewed, 588 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_2066_1f37`, SPX v4 `_2066_1f37`, and SPX v5 `DM2_map_2066_1f37`. Firestaff matches the forward record-chain scan, DB3/type-0x27 predicate, source packed-field update, counter increment, and found return. The production dungeon-arrangement owner calls the helper on both teleporter chains during staged world loading. All three source definitions are `IMPLEMENTED_PARITY`. Active-source counts: 3,351 unreviewed, 512 partial, 5 missing and 56 implemented-parity; combined backlog: 3,943 (3,351 unreviewed, 585 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_3c30`, SPX v4 `__LOAD_CREATURE_FROM_DUNGEON`, and SPX v5 `DM2_1c9a_3c30`. Firestaff matches the DOS/v5 non-static creature count and +0x64 CAII capacity owner, but SPX v4 uses all live records and +0x4b; source auxiliary-array allocation and cross-platform ownership remain incomplete. All three are `PARTIAL`. Active-source counts: 3,354 unreviewed, 512 partial, 5 missing and 53 implemented-parity; combined backlog: 3,946 (3,354 unreviewed, 585 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_FILL_ORPHAN_CAII`, SPX v4 `FILL_ORPHAN_CAII`, and SPX v5 `DM2_FILL_ORPHAN_CAII`. Firestaff has the map-iteration receipt helper, but no production caller; both compatibility entrypoints for orphan/per-map CAII filling are stubs, so creatures are not activated across maps. All three definitions are `PARTIAL`. Active-source counts: 3,357 unreviewed, 509 partial, 5 missing and 53 implemented-parity; combined backlog: 3,946 (3,357 unreviewed, 582 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_381c`, SPX v4 `CREATURE_THINK_381c`, and SPX v5 `DM2_1c9a_381c`. Firestaff has a source-contract path consumer helper but no production caller; the compatibility wrapper is a stub, while DOS/v4 path-cache tags and live AI/path ownership remain open. All three definitions are `PARTIAL`. Active-source counts: 3,360 unreviewed, 506 partial, 5 missing and 53 implemented-parity; combined backlog: 3,946 (3,360 unreviewed, 579 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW___SET_CURRENT_THINKING_CREATURE_WALK_PATH`, SPX v4 `__SET_CURRENT_THINKING_CREATURE_WALK_PATH`, and SPX v5 `DM2___SET_CURRENT_THINKING_CREATURE_WALK_PATH`. Firestaff has a v5/SKULLWIN source-contract helper, but no production caller; DOS and v4 use different cache tags, and runtime cache ownership remains open. All three definitions are `PARTIAL`. Active-source counts: 3,363 unreviewed, 503 partial, 5 missing and 53 implemented-parity; combined backlog: 3,946 (3,363 unreviewed, 576 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_19d4`, SPX v4 `_1c9a_19d4`, and SPX v5 `DM2_1c9a_19d4`. Firestaff has the command-to-direction translation helper, but it has no production caller; the similarly named 1c9a wrapper is a no-op. Source attack dispatch and platform routes remain open, so all three definitions are `PARTIAL`. Active-source counts: 3,366 unreviewed, 500 partial, 5 missing and 53 implemented-parity; combined backlog: 3,946 (3,366 unreviewed, 573 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_APPLY_CREATURE_POISON_RESISTANCE`, SPX v4 `APPLY_CREATURE_POISON_RESISTANCE`, and SPX v5 `DM2_APPLY_CREATURE_POISON_RESISTANCE`. Firestaff matches the resistance-nibble immunity/formula and has runtime cloud-damage callbacks; exact RNG stream behavior, the complete spell/attack call graph, and supported-platform integration remain open. All three are `PARTIAL`. Active-source counts: 3,368 unreviewed, 498 partial, 5 missing and 53 implemented-parity; combined backlog: 3,946 (3,369 unreviewed, 570 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_17c7`, SPX v4 `_1c9a_17c7`, and SPX v5 `DM2_1c9a_17c7`. Firestaff contains the exact map/state/direction/range predicate as `dm2_v1_skproject_1c9a_17c7`, but it has no production caller; the similarly named 1c9a adapter computes square distance instead. The WOUND_CREATURE gate and platform routes remain incomplete, so all three are `PARTIAL`. Active-source counts: 3,371 unreviewed, 495 partial, 5 missing and 53 implemented-parity; combined backlog: 3,946 (3,371 unreviewed, 568 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_DELETE_CREATURE_RECORD`, SPX v4 `DELETE_CREATURE_RECORD`, and SPX v5 `DM2_DELETE_CREATURE_RECORD`. Firestaff routes lethal creature damage and world/load deletion through a bounded owner covering trigger gates, CAII cleanup, tile-chain removal, possession drops and deallocation. The source 3CE7D move side effects, dballoc/GFX-cache cleanup and full platform caller coverage remain open; all three definitions are `PARTIAL`. Active-source counts: 3,374 unreviewed, 492 partial, 5 missing and 53 implemented-parity; combined backlog: 3,946 (3,374 unreviewed, 565 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_0fcb`, SPX v4 `CREATURE_1c9a_0fcb`, and SPX v5 `DM2_1c9a_0fcb`. Firestaff frees the CAII slot and cancels the timer, but only models the v5 conditional deletion decision head; it lacks the subsequent creature-record deletion, has no production M11 caller for its public free-slot wrapper, and does not implement source allocator recycling. The v4 variant has no conditional-delete tail. All three are `PARTIAL`. Active-source counts: 3,377 unreviewed, 489 partial, 5 missing and 53 implemented-parity; combined backlog: 3,946 (3,377 unreviewed, 562 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_0db0`, SPX v4 `RELEASE_CREATURE_TIMER`, and SPX v5 `DM2_1c9a_0db0`. Firestaff checks DB4, resolves the CAII slot, cancels a live timer ticket, and clears the slot word through its shared deletion helper. Live compositions exist in CAII scheduling/free-slot and creature-loop paths; complete queue ownership and supported-platform caller coverage remain open. All three source variants are `PARTIAL`. Active-source counts: 3,380 unreviewed, 486 partial, 5 missing and 53 implemented-parity; combined backlog: 3,946 (3,380 unreviewed, 559 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS `SKW_1c9a_0cf7` and SPX v5 `DM2_1c9a_0cf7` with Firestaff timer helpers. Firestaff reproduces tile lookup, prior timer replacement, group-link event selection, next-tick scheduling, and CAII ticket storage, with live missile-impact and world/load compositions. The public wrapper lacks a direct M11 caller and SPX v4 has a distinct timer owner; source caller and all-platform coverage remain open. Both definitions are `PARTIAL`. Active-source counts: 3,383 unreviewed, 483 partial, 5 missing, and 53 implemented-parity; combined backlog: 3,946 open rows (3,383 unreviewed, 556 partial source/older-ledger rows, 5 missing, 2 partial source owners).

Compared DOS and SPX v5 `R_1FE74`: the source helper checks coordinate deltas for axis alignment and then the requested direction. No corresponding Firestaff helper or live caller was found; the source marks it unreferenced and SPX v4 has no distinct implementation. Recorded both definitions as `MISSING`. Active-source counts: 3,385 unreviewed, 481 partial, 5 missing, and 53 implemented-parity; the combined DM2 backlog is 3,946 open rows (3,385 unreviewed, 554 partial source/older-ledger rows, 5 missing, 2 partial source owners).

The two empty v0 and the empty SPX v4 About-dialog `BEGIN_MESSAGE_MAP` blocks are optional MFC shell macros and have no gameplay handlers. The generated combined skproject backlog is 3,943 open rows (3,282 unreviewed, 654 partial source or older-ledger rows, 5 missing, 2 partial source owners). The SDL2 `AudioPlaySound` override maps to Firestaff’s verified GDAT sample path and 16-voice SDL3 backend, but its channel recycling, per-call playback rate and stereo panning differ; keep the renderer implementation partial. Its generic no-op virtual fallback is architecture-specific. SDL2 `AudioStop` maps partially to Firestaff’s sound-voice stop helper, but not the separate music queue, and its only source request route is unreachable in skproject; its generic fallback is architecture-specific. The generated combined skproject backlog is 3,943 open rows (3,342 unreviewed, 594 partial source or older-ledger rows, 5 missing, 2 partial source owners). Neither inventory is yet a complete feature-by-platform audit. The DOS `SKW_48ae_05ae`, SPX v4 `_48ae_05ae_CREATURE`, and SPX v5 `DM2_query_48ae_05ae` item-versus-creature queries are now separately reviewed as partial. Firestaff now implements callback-bound DOS/SKULLWIN, SPX v4, and SPX v5 query variants, including source-width arithmetic, variant-specific creature-word and mask indices, max-charge defaulting, and percentage adjustment. The SPX v5 literal-0x64 mask argument remains source TODO behavior. The outer oversee callback accepts an injected query owner, but production runtime wiring/callers and supported-platform coverage remain open. DOS `SKW_1c9a_3bab` is now mapped as partial: Firestaff has slot-level CAII cleanup but does not iterate active slots from its save prepass. DOS and SPX v5 `FSUBSAVE` are now mapped as partial to Firestaff’s DM2 save orchestrator and orphan-CAII walker; file rollback, buffer ownership, and event/UI cleanup remain unbound.  DOS and SPX v4/v5 `LOAD_NEW_DUNGEON` remain partial: Firestaff reloads the verified selected dungeon, but source alternate dungeon/custom filename selection and GAME_LOAD handle routes are not bound. DOS and SPX v5 `GAME_SAVE_MENU` remain partial: Firestaff has a serializer and generic input callback, but no runtime binding for slot/file transaction, backup, admission, or finalization. DOS and SPX v5 top-level `GAME_LOAD` remain partial: the staged SKSAVE core exists, but file selection/fallback, mode branches, prompts, cleanup, and exit/UI ordering are not bound as the complete source entry flow. DOS and SPX v5 `COMPACT_TIMERLIST` remain partial: Firestaff has the compaction primitive but no save-pipeline binding, and the source follows it with sort and `3a15_020f`. DOS and SPX v5 `2066_197c` special-timer record-chain readers are now implemented parity: they read one non-following valueB chain for timer 0x3c/0x3d before map chains and reject missing savegamew7. DOS and SPX v5 `2066_0b44` special-timer record-chain writers are now implemented parity: Firestaff serializes timer valueB chains for types 0x3c/0x3d before map data and stops on writer failure. DOS and SPX v5 `2066_062b` possession-continuation readers are now implemented parity: Firestaff consumes the same 10-bit fields for DB9/DB14 only and publishes them in the staged save/load owner. The current DM2 source pass corrected a concrete dispatcher mismatch: SKULLWIN and SPX v5 use zero-based action case 41 for champion throwing, DOS uses case 41, and SPX v4 names it `CmThrow` command 42 (Firestaff case 41 after its action-id normalization). Each selects relative throw direction 0 or 1 from party facing and champion position. Firestaff had treated its case 41 as a position swap; it now routes that case to a throw callback. Minion release now resolves its DB14 missile and dispatches the source 0x13 CAII command; minion creation remains fail-closed without its map-search and creature-allocation owner. The DOS, Amiga, FM Towns, and Macintosh runtime/data lanes remain open. Other action transaction bindings and platform runtime coverage remain open. DOS, SPX v4 and SPX v5 `_2759_0155` variants were source-compared with Firestaff’s existing query receipt: each scans text commands 8–11 and checks loadability, command field 2, and charge field 8. DOS, SPX v4 and SPX v5 max-charge functions were also compared: all return 15 for DB5/DB6, 3 for DB10 and 0 for null/unsupported records, matching Firestaff’s helper. DOS/SPX v4/v5 container-map predicates were compared with the DB9/type-1 Firestaff helper and authenticated record-pool hand-action callback; other platform caller routes still need evidence. DOS, SPX v4/v5, and SKULLWIN item-DB-from-distinctive-type routines now map to the matching narrow record-ops composition; production call routing remains open. The uncalled MFC-only `AskMe` wrapper and unreachable SDL2 `AudioData` typedef are recorded as architecture-specific and outside gameplay parity. SPX v6 `AudioPlayDirect` maps its external MP3/WAV music route to Firestaff’s verified source-track queues; external file playback and the uncalled v6 request route remain partial. The SDL2 `AudioPlayFile` override is source-compared; the generic renderer no-op is classified as architecture-specific. Firestaff still lacks the live cross-platform charge-stat-bar owner and authenticated class/GDAT/CMDSTR caller bindings.

F0275 has one bounded DM1 PC34 M11 C080 behavior regression in this review:
successful non-head C017 removal and the linked-head exception. ReDMCSB's
`MOVESENS.C:1415-1427` compiles the unlink under MEDIA009/MEDIA529. MEDIA009
covers the supported S10-S21, G14/G20/G21, and A20-A22 editions; MEDIA529
includes I34, A31/A36, F31, and their supported language variants. MEDIA508
is the separate F20/X30/P20 set and does not contain I34. Thus C017 unlink is
source-applicable to the documented DM1/CSB lanes, while the current live M11
integration regression is PC34-only. Each other lane still needs runtime
integration and original-media evidence. CSB's bounded PC34 C011/C012 routes
are tracked separately and do not close its other platform lanes.

Current copy-protection scope correction (2026-11-01): ReDMCSB protects
`F0079` and `F0137` with `!NOCOPYPROTECTION`. `LOADSAVE.C:2091-2103` calls
`COPYPRO1.C:F0079` only inside `!NOCOPYPROTECTION` branches for MEDIA137 and
MEDIA351 editions. When enabled, the DOS/Atari routine sums machine-code
words through its UNLK/RTS marker; Amiga and IIGS variants sum bytes using a
caller skip table. A checksum mismatch can reject the loaded dungeon. Firestaff
explicitly runs its PC34 host save path without this copy-protection contract
and validates its own save/dungeon data. Thus F0079 remains missing as a
conditional protected-edition feature; it is not a missing step in Firestaff's
active PC34 save flow. `COPYPRO3.C:F0137` similarly copies a platform-specific
hidden launcher into a free CODE_PATCH slot, saves overwritten instructions,
and writes a jump into the target function; its PANEL/DUNVIEW callers are
fuzzy-sector/hidden-sector paths. Firestaff has no matching executable patch
and restore owner. The PC34 product route excludes these hidden launchers,
while other supported ReDMCSB platform editions have independent conditional
protection paths. Keep both functions open until each platform's selected
protection mode and runtime policy are resolved; do not silently treat a
source-listed optional mode as already covered by PC34 behavior.

Current DM1 map-transition finding (2026-11-01): ReDMCSB `DUNGEON.C:F0173`
updates the current-map index, map/data pointers, dimensions, door-info cache,
and cumulative thing-column pointer only when the map changes (`2724-2740`).
`F0174` composes that transition with party-map ownership, publishes the
allowed-creature metadata, copies wall/floor/door ornament lists, and writes
the inscription ordinal (`2742-2762`). Firestaff's similarly named descriptor
adapter has no production caller; the dungeon-data F0174 adapter updates the
party-map index and dimensions but does not publish these metadata arrays. The
live M11 renderer reads selected map data through separate paths. The two
callable rows therefore remain `PARTIAL`: adapter tests prove only those
bounded models, not live transition behavior, nor per-platform parity.

The audit below is the DM1/CSB source-function inventory, not a substitute for
the DM2 skproject inventory or the per-platform status register. Completion
claims must be made per game and per supported platform lane.

The source review covers behavior and presentation, not only existing
completion percentages. The following family-level index is a triage map; every
underlying function, caller chain, supported edition and platform still needs
its own disposition:

| Lane | ReDMCSB source roots | Firestaff surface | Status |
|---|---|---|---|
| movement_input | COMMAND.C, CLIKMENU.C, MOVESENS.C, GAMELOOP.C, MOVESENS.C sensor sections, DUNGEON.C | dm1_v1_movement*, dm1_v1_input*, dm1_v1_collision*, dm1_v1_sensor* | active_audit |
| viewport_hud_render | DUNVIEW.C, DRAWVIEW.C, PANEL.C, CHAMDRAW.C, CHAMPION.C, PORTRAIT.C, TEXT.C, DEFS.H | dm1_v1_viewport*, dm1_v1_champion_panel*, dm1_v1_portrait*, dm1_v1_palette*, dm1_v1_screen* | active_audit |
| gameplay_systems | GROUP.C, PROJEXPL.C, CASTER.C, OBJECT.C, CHAMPION.C, TIMELINE.C, LOADSAVE.C, DUNGEON.C | dm1_v1_combat*, dm1_v1_creature*, dm1_v1_projectile*, dm1_v1_spell*, dm1_v1_object*, dm1_v1_inventory*, dm1_v1_save*, dm1_v1_food* | active_audit |
| frontend_route_capture | TITLE.C, ENTRANCE.C, MENU.C, MENUDRAW.C, COMMAND.C, CLIKVIEW.C, GAMELOOP.C, ENDGAME.C | dm1_v1_entrance*, dm1_v1_menu*, boot_*pc34*, V1-sensitive parts of main_loop_m11, m11_game_view, menu_startup_m12 | active_audit |

## Required outcome for every row

Each audited row must end in exactly one of these states:

| State | Meaning |
|---|---|
| MATCHED | Firestaff behavior is source-locked to ReDMCSB and has a passing gate. |
| FIXED | A ReDMCSB-proven deviation was corrected and gated in Firestaff. |
| KNOWN_DIFF | Firestaff intentionally differs, and the difference is unreachable in normal DM1 V1 parity play or otherwise justified by source/runtime evidence. |
| MISSING | ReDMCSB behavior exists, Firestaff does not implement it yet, and the gap is recorded with a focused blocker/gate. |
| BLOCKED | The audit cannot finish without missing original data, missing runtime capture, or a tool/environment issue. |

Rows cannot be counted as complete from internal consistency probes alone.
They need ReDMCSB source anchors and a Firestaff gate or a named blocker.

## Immediate high-risk gaps

These are known from the current source tree and must be retired or explicitly
bounded before DM1 V1 can be called complete:

| Gap | Evidence | Required action |
|---|---|---|
| Creature AI fidelity | `src/memory/memory_creature_ai_pc34_compat.c` lists all 27 DM1 creature profiles at FULL tier, while `F0804_CREATURE_Tick_Compat` retains a tier-0 fallback. | Do not report a broad profile-stub gap: compare each source behavior and platform profile against its gate; separately track any simplified or deferred mechanics and confirm whether any profile actually selects the fallback. |
| Magic/combat fidelity | `memory_magic_pc34_compat` implements a bounded PC 3.4 spell and defence path; its header records the historical stub that it replaced. | Compare the implemented path against ReDMCSB for each supported DM1/CSB platform and edition; list only source-confirmed missing semantics as `MISSING`. |
| Sensor execution unsupported types | Floor C10 group-split Steal-XP and deferred CLEAR/TOGGLE rotation have source-shaped M11 WALK_ON consumers. The F0248 wall-event route awards cell-specific C10 Steal-XP to the active leader and applies OnceOnly before dispatch. A bounded DM1 PC34 object add/remove sensor adapter now runs after successful M11 keyboard and rendered-pile pickup/drop chain changes, carrying the F0276 audible-sensor switch-sound receipt. M11 now also prints visible destination-square TextStrings in source chain order for accepted DM1 PC34 movement and first party placement, matching F0276's C1_TEXT_TYPE_MESSAGE path. A focused startup-placement regression confirms chain order and invisible-string suppression; ordinary movement integration, other mutation routes, wall-cell object branches, local effects, uncovered sensor-event paths, full platform coverage and original-media proof remain open. CSB F0276 now decodes each visible C02 in chain order into source-shaped four-row C015 state, tested by a real movement fixture containing visible HEL, hidden text and visible TWO. Atari/FM Towns rows expire at +70 and Amiga rows at +200 per TEXT.C F0046; complete edition-specific UI and platform coverage remains open. | Continue MOVESENS.C F0270/F0271/F0276 audit across every event path and platform; add original-media comparison and remaining sensor/lifecycle integration coverage, cover remaining local-effect types, and keep original-media claims separate from source-level behavior. |
| Original overlay regression absent | docs/parity/COMPLETION_MATRIX.md gives DM1 V1 original_overlay_regression 0/10. | Keep source fixes separate from pixel claims until representative original overlay frames exist. |
| V1-visible shell leakage | Previous parity docs identify custom M11/M12 surfaces that must not decide original behavior. | Audit normal V1 route and keep invented/debug surfaces unreachable unless source-backed. |

## Cross-game completion blockers

The repository's current completion model remains explicitly partial for each
primary V1 target; its scores are summaries, not function inventories:

| Game | Current repository status | Concrete missing proof |
|---|---|---|
| DM1 V1 | 78%, `playable_verified_partial` in `parity-evidence/verification/firestaff_completion_matrix.json` | Original same-state overlay regression is 0/10; audio timing/overlap and wider per-edition functional coverage remain open. |
| CSB V1 | 43%, `runtime_boundary_verified_partial` in `docs/parity/PARITY_MATRIX_CSB_V1.md` | Broad real-data adventuring, save/reload, original overlays, and audio playback/timing remain open across Atari ST, Amiga, and FM Towns. |
| DM2 V1 | 36%, `runtime_slices_verified_partial` in `parity-evidence/verification/firestaff_completion_matrix.json` | The parity matrix is incomplete (4/10); renderer and original-overlay coverage are 3/20 and 0/10, and runtime wiring remains incomplete. |

These blockers must be decomposed into source-function rows and closed for each
supported platform lane. Do not infer platform parity from aggregate scores.

## Source-symbol inventory findings

The current callable-symbol audits expose concrete incomplete work. These are
triage inputs, not direct counts of missing gameplay features: platform APIs,
ABI aliases, unreferenced routines, and weak numeric matches must be separated
from reachable game behavior before an implementation claim is made.

| Reference inventory | Current row counts | Confirmed interpretation |
|---|---|---|
| ReDMCSB `docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv` | 2,133 total: 777 `MISSING`, 507 `UNCERTAIN_NUMBERED_EVIDENCE`, 538 `IMPLEMENTED_NARROW`, 95 `VERIFIED_SOURCE_MAPPING`, 69 `SOURCE_NONAPPLICABLE`, 5 `PC34_SOURCE_IMPLEMENTED`, 142 `PARTIAL` | The ledger contains 2,100 callable-function rows and 33 auxiliary rows (19 state, 8 record, and 6 exception rows). Across all row kinds, 1,426 rows remain `MISSING`, `UNCERTAIN_NUMBERED_EVIDENCE`, or `PARTIAL`; 1,394 of those are callable-function rows. These are not all gameplay gaps, so reachable functions and OS/ABI-only routines still require per-platform triage. ReDMCSB `DUNVIEW.C:F0112/F0118-F0123/F0125` now map to partial ceiling-pit and viewport square dispatchers; full element ordering and platform routes remain open. F0127 D0C and F0128 per-square scheduling are also partial, with material/element rendering and complete overlay/platform parity still open. F0129 has bounded PC3.4, CSB, and FM Towns sampling routes; IIGS title sampling and cross-platform pixel parity remain open. F0130-F0133 map bounded PC34 flip/blit primitives and the DM1 masked-field renderer; generic packed-mask and all-platform draw semantics remain open. F0136 has a bounded checkerboard hatch owner. F0137 is `MISSING`: Firestaff has no hidden-code launcher or executable patcher. ReDMCSB `DUNVIEW.C:F0099/F0116/F0117` now map to partial row-flip and PC34 square-dispatch owners; complete bitmap ownership and platform dispatcher parity remain open. ReDMCSB `DUNVIEW.C:F0095` now maps to partial Firestaff map-wall-set resolution; edition-specific preload/cache ownership remains open. ReDMCSB `STRING.C:F0090/F0091` now maps to bounded host-libc search behavior, with `F0091` partial because the source does not match the terminating NUL. ReDMCSB compiler-runtime symbols `DEFS.H:F0082-F0088` now distinguish Megamax arithmetic/structure helpers from ordinary `strcat`/`strcmp`/`strcpy` host-libc mappings. ReDMCSB `COPYPRO1.C:F0079` and its Atari ST alias are now correctly marked `MISSING`: Firestaff has no equivalent copy-protection code checksum. ReDMCSB `IO.C:F0074-F0078` now map to bounded SDL host-cursor visibility and input event paths as `PARTIAL`; source bitmap save/restore, Atari ST interrupts, and paired redraw visibility timing remain open. ReDMCSB `MOUSESET.C:F0066-F0071/F0073` now map to bounded host-pointer, champion-icon formation pickup/drop, and PC34 screen-area receipt counterparts as `PARTIAL`; source cursor raster, Atari ST driver and cross-platform save/restore behavior remain open. ReDMCSB `SOUND.C:F0061-F0065` now map to bounded PSG amplitude-table, DM1 sound-state/host initialization, sound-event arbitration, and pending-event flush counterparts as `PARTIAL`; Atari ST key-click/PSG/Timer-A ownership and all-edition audio-driver and timing parity remain open. ReDMCSB `TEXT.C:F0042-F0047` now map to bounded PC34 message-area cursor, row, and print counterparts as `PARTIAL`; bitmap/font/pixel and cross-platform integration remain open. ReDMCSB Atari ST aliases `F031-F039` now map to the bounded DM1/CSB name, object/icon and slot-box counterparts as `PARTIAL`; Atari ST runtime/ABI and authentic asset behavior remain open. ReDMCSB `BASE.C:F0022` and `SWSH.C:F0022` now map to source-gated VBlank delay and millisecond-estimated swoosh-hold counterparts as `PARTIAL`; frame-exact all-platform timing remains open. ReDMCSB `BASE.C:F0020/F0021` now map to Firestaff source-named screen/viewport blitters and bounded PC34 admission as `PARTIAL`; live M11 use and complete platform bitmap behavior remain open. ReDMCSB Atari ST aliases `F023-F030` now map to their bounded PC34 math/RNG helpers as `PARTIAL`; the Atari ST runtime owner, ABI, RNG and overflow behavior remain open. ReDMCSB `BASE.C:F0009/F0010` now map to the shared Firestaff spaced byte/word writers for positive counts as `PARTIAL`; some assembly source lanes wrap zero count to 65,536 writes. ReDMCSB `BASE.C:F0006` now maps bounded Firestaff menu/champion-panel highlight overlays as `PARTIAL`; the generic inclusive source-screen rectangle inversion remains unimplemented/unverified. ReDMCSB `CLEARBYT.C:F0008` now maps to the shared Firestaff `memset` wrapper for valid bounded clears as `PARTIAL`; source pointer/count widths and ABI behavior remain open. ReDMCSB `COPYBYTE.C:F0007` now maps to the shared Firestaff `memmove` wrapper for positive-length overlapping copies as `PARTIAL`; platform-specific assembly, pointer/size domains, and ABI behavior remain open. ReDMCSB `GAMELOOP.C:F0002` now maps to Firestaff's bounded tick adapter and live M11 DM1 loop as `PARTIAL`; full edition-specific loop order, platform drivers, and watchdog behavior remain open. ReDMCSB `NEWMAP.C:F0003` now maps to the staged M11 group/map transition as `PARTIAL`; M11 now selects the destination track after its shared movement/teleporter handoff; stairs and post-move transitions have separate track paths, and rendering resolves active-map graphics/palette. Complete caller ordering and ReDMCSB copy-protection event behavior remain unverified across editions/platforms. ReDMCSB BASE.C F0023-F0030 now have direct narrow PC34 mappings: signed absolute value, minimum, maximum, inclusive clamp, the F0027 LCG and its F0028/F0029 bit extracts, plus bounded F0030 scaled products. F0028/F0029 use HORIZCNT on Apple IIGS; global seed ownership, call order, those hardware branches, general unsigned overflow behavior in F0030, and other edition/platform runtime-owner equivalence remain open. FIO1 F1321 and F1324 now map to their tested PC34 bounded path-substitution and source no-op contracts; Amiga ABI/device semantics remain outside those adapters. ReDMCSB F0906/F0907 now map to an Amiga raw-ADF admission check and boot-sector fallback, including the exact embedded-NUL comparison, exercised against authentic A20E media and the nested-archive CLI startup path. F0099's standalone PC3.4 row-flip adapter is contract-only; M11 flips pixels while blitting in separate live paths, which does not prove the source function's copy ownership or all caller destinations. ReDMCSB uses MEDIA009 F0130 and MEDIA529 F0655/F0616+F1006 branches; per-edition/platform equivalence remains open. F0116/F0117 now record their tested PC3.4 D3L/D3R scheduler, wall/ornament, door, pit/stairs, floor-ornament, thing-order and field slices; right-side door-button coverage is also source-locked. Their full dispatcher behavior across DM1/CSB editions and supported platforms remains open. F0024/F0025/F0026 now expose source-named signed 16-bit minimum, maximum, and inclusive-clamp helpers in both DM1 and CSB. Their bodies match ReDMCSB BASE.C MEDIA201 C and MEDIA247 68k comparison logic over that domain; target ABI/linker bindings and original platform runtime proof remain open. F0348 records bounded DM1/CSB statistic-adjustment routes as `PARTIAL`: the DM1 adapter implements MEDIA001 (S10EA/S10EB: cap 255/no diminishing gains), MEDIA222 (S11E: strict >120/>180 thresholds, cap 250) and MEDIA240 (strict >120/>150 thresholds, cap 170), with graphics hashes selecting the supported Atari ST 1.0/1.1 rule sets. The DM1 live mouth routes pass the selected rule set from the admitted graphics identity; other supported editions still need live-mouth and original-media proof. The CSB live PC3.4 potion path implements the MEDIA240 formula and its boundary regression covers 120/121/150/151/169/170. Its supported Atari, Amiga and FM Towns builds require independent runtime proof; do not apply DM1 S10E/S11E edition rules to CSB without source evidence. F0349 records bounded DM1/CSB mouth-consumption routes as `PARTIAL`: CSB has native PC3.4 effects for C06-C14 plus separate water, waterskin, and food consumers; full RNG/timer and cross-platform proof remains open. F0031 now records the DM1/CSB M564 name loaders as `IMPLEMENTED_NARROW`: DM1 real-media gates compare all 199 DOS, Atari ST, Amiga, and FM Towns Japanese names, while CSB has source-selected Atari/Amiga/FMTowns boot loaders and synthetic parser fixtures. ReDMCSB also includes G14ED/G20E/G21E in F0031's MEDIA060 branch; Firestaff's native IIGS startup now reads raw GRAPHICS.GAME record 556 and routes it through the 199-entry high-bit-terminated object-name parser. IIGS authentic full-table comparison and runtime proof, full CSB original-archive table comparison, and all-edition/platform runtime parity remain open. F0036 remains `PARTIAL`: Firestaff now selects the IIGS strip from authenticated metadata, extracts a 16x16 indexed-pixel destination buffer, and blits it. Authentic IIGS icon comparisons have not been run; other edition/platform evidence only records source-call ordering, so full DM1/CSB parity remains open. F0039 records the tested DM1 PC3.4 slot-box icon-index getter as `IMPLEMENTED_NARROW`; other edition/platform runtime parity remains open. F0358 records the tested PC3.4 mouse-route resolver as `IMPLEMENTED_NARROW`; generated-table/media and other-platform equivalence remain open. F0359 records the tested DM1 PC3.4 click enqueue and locked-queue replay route as `IMPLEMENTED_NARROW`; other platform release semantics and CSB lane remain open. F0365 records the tested DM1 PC3.4 turn route as `IMPLEMENTED_NARROW`. IIGS.H shortened F0373/F0375 tokens are now recorded as macro aliases of the canonical functions, avoiding false uncertain-symbol rows. F0373 now records the partial PC34 C080 floor-pickup path: front-cell items remain blocked by a non-levitating group that occupies the clicked F0176 cell, using the F0145/F0147 active-group cell and direction values. F0372 now records the partial PC34 front-wall sensor route: occupied-hand C05 clicks dispatch the same F0275/F0726 path as empty-hand clicks, and no-result sensor zones do not report a handled action; exact source tick semantics and cross-platform runtime proof remain open. F0376 now records the shared inclusive source-box helper, whose 38-assertion PC34 edge/equivalence gate passes. F0377 now preserves the PC34 F0375 throw lane where it overlaps the D1C door-button box; its boundary regression passes. F0374/F0375 now have bounded PC34 runtime and resolver paths: F0374 object-pile floor drop, alcove drop, and Vi Altar event plus F0375 side-zone throw have bounded PC34 runtime paths and focused admission checks; F0374 now records the remapped destination cell for the Vi Altar event. The MEDIA720 F0377 fountain fill route adjusts carried load by the F0140 weight delta when filling a waterskin or flask; other edition-specific fountain behavior and click-zone parity remain open. Edition-specific click-zone generation and all-platform runtime parity remain open. F0366 records the bounded DM1 PC3.4 step route as `IMPLEMENTED_NARROW`: Firestaff now resolves teleporter records through ReDMCSB’s compact flagged-square SFT index, and the movement integration test passes with source-valid floor-sensor fixtures. CSB V1 compatibility-runtime turn/step cases also cover directional C003 turn sensors, stairs, and an open-corridor step; original-platform and cross-platform verification remain open. |
| skproject `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` | 1,760 older `SKWIN`/`SKULLWIN` disposition rows: 1,074 `IMPLEMENTED_PARITY`, 559 `NOT_APPLICABLE_ARCH`, 73 `NONAPPLICABLE`, 52 `PARTIAL`, and 2 `PARTIAL_SOURCE_OWNER`; expanded inventory covers 3,932 source definitions (SPX v4 1,116; SPX v5 1,347; shared SPX 262; legacy DOS 832; DOS src 373; SKULLWIN 2), with active review overrides of 3,775 `UNREVIEWED`, 149 `PARTIAL`, and 8 `IMPLEMENTED_PARITY` (3,630 names, 302 repeated definitions). The generated open backlog has 3,964 rows (3,775 `UNREVIEWED`, 200 `PARTIAL`, 2 `PARTIAL_SOURCE_OWNER`). | `DM2_ALLOC_NEW_RECORD`, `DM2_ALLOC_NEW_DBITEM`, `DM2_ALLOC_NEW_DBITEM_DEEPLY`, and `DM2_RECYCLE_A_RECORD_FROM_THE_WORLD` now have bounded source-owner paths: the private GAME_LOAD/SKSAVE clone commits direct DB0/ordinary-DB2/DB3/DB7/DB9 returns plus selected DB5/6/8/10/14 routes, while the generated-drop path recycles selected DB5/6/8/10 records and directly clears DB7/DB9 records in place. They remain partial: complete DB0-DB15 cursor and owner traversal, protected maps, DB4/creature-bearing DB14 deletion, timer/CAII interactions, and all-platform runtime behavior are not covered. `DM2_GET_FROM_FREEPOOL` through `DM2_DTOR_MEMORYALLOCATION` remain partial: their matching `dm2_v1_skproject_*_classify` routines are not called by a live Firestaff allocation owner and do not establish source pool-pointer effects. The SPX/DOS allocator definitions are also `PARTIAL`; DOS `DM2_ALLOC_MEMORY_RAM` ignores `clean` and always zeroes memory, unlike SPX. SPX v4 and v5 door query definitions are tracked separately: v4 uses field 0x0f for strength, 0x0e for resistance, and map-header values for door graphics; v5 uses 0x11 strength with a 0x10 fallback and 0x0f/0x0d for resistance/graphics. Firestaff query receipts were corrected to the locked SKULLWIN/v5 field contracts. Firestaff already selects parsed map door sets through header-use flags and door-record type; the SPX v4 0xff no-door caller behavior, complete consumer coverage, and per-platform runtime evidence remain open.  SPX v4 `ACTUATE_DOOR`/`STEP_DOOR` remain partial: the live handler now follows DoorBit09/10 with source byte order and handles active reversal; the live and GAME_LOAD candidate paths derive movement from DoorBit09; the candidate world-owner no longer rejects timer actor bytes outside 0/1. The direct GAME_LOAD world-owner receipt remains structural, and source party/creature collision effects and all-platform runtime parity remain open. `DM2_DEALLOC_RECORD` itself matches the source primitive; remaining gaps belong to its other game-level callers and their owner transactions. `DM2_ARRANGE_DUNGEON` has a bounded mutation owner integrated into New Game; source-by-source mutation and all-edition/platform parity proof remain open. `DM2_ADD_BACKGROUND_LIGHT_FROM_TILE` remains partial; `DRAW_WALL_ORNATE` and `DRAW_ALCOVE_ITEMS` are now explicit `PARTIAL` SKWIN rows; the renderer now retains the bounded `SUMMARIZE_STONE_ROOM` DB4+ wall record root in `DM2_ViewSquare`, while alcove item geometry/cloud/click rendering remains open. `_32cb_0f82_SHOP_GLASS` is `PARTIAL`: Firestaff resolves the DB3 wall/floor actuator pair, selects eligible stock from the CREATURES mask and endian-aware WALL_GFX whitelist, filters evidenced tile/party ownership, mutates actuator state in source byte order, and renders selected merchandise, the conditional sold-out icon, and navigation board with source GDAT palettes, offsets, and rectangle clipping. The DOS `SKW_32cb_0f82`, SPX v4 `_32cb_0f82_SHOP_GLASS`, and SPX v5 `DM2_guivp_32cb_0f82` definitions have now been compared separately; each remains partial because flagged DB14 stock allocation, item names, and price/coin rows are missing. The separate `ddat.v1d67bc` hand-container owner, click-direction state, item name/prices, high-bit DB14 allocation, shop panel text/money rendering, and buy/sell transactions remain open; bounded flags-3 ceiling and creature F8 branches use distinct skproject falloff tables, including the distance-over-eight case where only creature light remains. A bounded class-5 live DBE/DBF tail scan now decodes DBE selector FF80 and DBF zero/2/30 selectors, matching sklight.cpp:398-405 and feeds `v1e0978` with its own distance falloff; focused term/accumulator checks and the FM Towns map-3/map-38 gameplay route pass, but the real-media route has not proven a nonzero retail DBE/DBF contribution. Flags-4 mode-7 now retains ordinary floor `RG6` beyond distance 8 while clearing only weather-scaled `vw_24`, matching sklight.cpp:455-481; distance-9 regressions cover both terms. The live callback also admits the sklight.cpp class-2 no-record flags-3 ceiling branch after complete source-input checks. Class-2 bit-08 tiles whose first record is DB4+ now retain the source summary root after the DB0-DB3 loop and use bounded live-chain evidence for creature and DBE/DBF terms; flags-4 remains a source no-op. A synthetic loader fixture verifies DB4+ root preservation and continued rejection of DB2 prefixes; focused flags-3/flags-4 accumulator regressions cover its light and darkness terms. Class-2 DB0/DB1 prefixes now skip to the first DB4+ summary root, matching c_gui_vp.cpp:2876-2903 and 3082-3086. C02 DB2 now follows skproject static and state-selected frame branches for ext-usages 0/5/10/15/16/17, plus tick-selected animation for ext-usage 2 and flagged 5, matching c_gui_vp.cpp:2971-3048 and 3072-3086. DB3 actuator floor decorations now resolve through the authenticated map-local floor list and apply source frame/tick rules, matching c_record.cpp:1288-1311 and c_gui_vp.cpp:2907-2965; subtype 0x27 consumes the source view direction, retains frame zero when inactive, and queries tick animation only when active and direction-matched. The live mode-8/mode-7 runtime now attempts any authenticated dynamic-light map position and requires complete source-walk receipts; unknown tile/record branches remain fail-closed. The mode-7 flags-3 class-0 no-op at skproject c_light.cpp:233-243 is now covered by a focused helper regression. The class-1 floor-actuator branch now queries authenticated GDAT F8/99 values and retains the distinct source weather term and F8 high-bit gate; focused accumulator regressions cover both. A new FM Towns map-3 (8,4) real-media regression is compiled but has not run locally because original media is unavailable. Mode-7 flags-3 now also summarizes class-3 and direction-matched class-4 tiles and scans up to 256 authenticated record links for DBE/DBF darkness plus creature F8, with focused accumulator tests; flags-4 class-3/4 is a no-op for any accepted record-chain root, class-4 direction-mismatched summaries now accept source-selected adjacent roots after validating class-0..3 record walks (or the source early return for higher classes). `query_0cee_1a46` only changes side-decoration summary words, not the tile/root fields consumed by `c_light.cpp`; ordered DB2 static/animated text decorations now use source GDAT animation timing, and the ordered query feeds DM2 door-button metadata with the current tick and map wall-GFX list; full wall-side decoration integration and source-case runtime coverage remain open. Existing runtime evidence remains FM Towns-only on maps 3 and 38; source record paths and DOS, Amiga, and Macintosh dynamic-light runtime coverage remain open. The DOS New Game start is static-light map 0 at (1,8), so it does not exercise the dynamic path. The DOS `SKW_ADD_COIN_TO_WALLET`/`SKW_TAKE_COIN_FROM_WALLET` and SPX v4/v5/SKULLWIN variants are separately mapped: Firestaff has source-specific add results and the matching take/decrement/allocate/cut behavior, with live platform callback binding still open. These behaviors are not closed by source mapping alone. The active DM2 wall-mecha owner now creates DB14 shooter records and queues source GEN2 throw noise and 0x1E continuation timers for actuator shooter types 7-0x0a and 0x0e-0x0f, including source item cutting and single-shot session-RNG direction. DOS allocation fallback, standalone/creature shooter owners, and DOS/Amiga/FM Towns/Macintosh runtime evidence remain open. DOS and SPX v5 `DM2_ABS`, `DM2_BETWEEN_VALUE`, square distance and vector direction now match Firestaff's signed utility helpers, including source argument order and tie-break selection for an explicit random bit. DOS, SPX v5, and SPX v4 `QUERY_PLAYER_SKILL_LV` variants now match the common bounded Firestaff skill-level helper for override, parent skill bonus, XP threshold and final minimum-one bonus; source-state adapters and supported-platform execution remain separate. DOS, SPX v5, SPX v4, and SKULLWIN `ADJUST_SKILLS` are source-compared: the common Firestaff helper now uses the source skill-level, random-bit, class-specific mana/health/stamina rules; the v4 current-stamina branch and production/platform bindings remain open. DOS, SPX v5, and SPX v4 `ADJUST_HAND_COOLDOWN` are separately source-compared: SPX v4 now has its own helper for its raw-delay/Aura of Speed formula, while DOS/v5 use the base-plus-quarter/save-flag behavior. Live spell casting and the accepted M11 attack/creature-wield hand routes now invoke the DOS/v5 cooldown owner. The hand-action route consumes CMDSTR field 4 plus the source RANDBIT and applies field-3 cooldown/field-4 stamina after ordinary source success or failure; rejected attacks roll back timer/counter state, and fail-closed missing owners receive no gameplay side effects. Skill adjustment, throw and other action callers, source sound/hand-table updates, SPX v4 selection, and platform bindings remain open. `CHANGE_PLAYER_POS` is source-compared across DOS, SPX v4/v5, and SKULLWIN. Production M11 now routes source events 125-129 into the runtime two-stage formation owner, including slots 0..4, facing-relative lookups, occupied swaps, empty-slot moves, direction suppression, and live session-snapshot updates. `ACTIVATE_ACTION_HAND` now triggers the source conditional high-bit recompute when a drag is pending. Both function families remain partial because source cursor-icon feedback, transient UI consumers, and non-M11 platform routes remain open. `SET_SPELLING_CHAMPION` retains its separate rune-panel bridge and now invokes the same conditional high-bit position recompute when a formation drag is pending; source transient UI and non-M11 platform effects remain partial. |

The inventory dispositions are repository evidence and must be checked against
the locked source revision before implementation. The current DM2 open scope is
substantive: skproject `DM2_ALLOC_NEW_RECORD`, `DM2_ALLOC_NEW_DBITEM`,
`DM2_ALLOC_NEW_DBITEM_DEEPLY` and `DM2_RECYCLE_A_RECORD_FROM_THE_WORLD`
still lack the complete DB0-DB15 cursor and owner walk,
protected-map handling, DB4/creature-bearing DB14 deletion, and timer/CAII
effects across the supported DOS, Amiga, FM Towns, and Macintosh lanes.
`DM2_ARRANGE_DUNGEON` has a bounded New Game mutation owner but lacks
source-by-source mutation equivalence across editions and platforms.
`DM2_ADD_BACKGROUND_LIGHT_FROM_TILE` has bounded light/darkness branches and
FM Towns map-3/map-38 runtime evidence only; DOS, Amiga, and Macintosh dynamic
lighting and nonzero retail DBE/DBF media cases remain unverified. The ordered
`DM2_query_0cee_1a46` door-decoration path is also incomplete: actuator
0x18 XOR and 0x46 active-state fields have focused source checks, and the
0x3f shop-panel handle and once-only 0x7e actuator datum are receipted in
summary word 7. DB2 TextMode-1 side-summary writes now preserve the packed ornament/frame
word in source slots 3..6; ext-usage 5 selects animation only when visible.
Ext-usage 14 clears the selected side slot 3..6 and writes the record handle or
visible-text absent marker to summary word 7 only when that slot is 5. The
runtime now selects the visible ornament side from viewport lateral position
and publishes its WALL_GFX index and source field on `DM2_ViewSquare`. A
bounds-checked parser for skproject's GDAT `fmtRect` ranges and four-word
rectangle nodes, plus front/side wall-ornament rect-number selection, is
covered with synthetic fixtures; runtime view squares now carry the selected
source rect number alongside the ornament fields. Source `QUERY_BLIT_RECT`
placement/clipping and animated-frame rendering, full output-array compatibility,
source captures, and all-platform runtime behavior remain open. Do not treat
these bounded receipts as full DM2 parity.

## Gate

Run:

    python3 tools/verify_redmcsb_firestaff_complete_audit.py


2026-11-01 skproject DOS/SPX v4/v5 `PROCEED_GLOBAL_EFFECT_TIMERS` review:
Firestaff stages the DOS/SPX v5 post-load timer effects 0x0e, 0x46, 0x47, 0x48 and 0x4b into the save-load candidate, with rollback if an unsupported effect cannot be applied. The source also recalculates party weight, which the candidate cannot do without active party/eventqueue/hand state. SPX v4 additionally has conditional 0x49 SeeThruWalls handling and a different 0x4b poison effect. Mark all three platform variants partial; full runtime and edition parity remain open.

Applying the active-source review overrides gives 3,182 unreviewed, 682 partial, 5 missing, and 55 implemented-parity definitions. The generated combined skproject backlog is 3,944 open rows (3,182 unreviewed, 755 partial source or older-ledger rows, 5 missing, 2 partial source owners). There are 8 architecture-specific source entries and two Bloodwych-only definitions excluded from this three-game audit. The two empty v0 and the empty SPX v4 About-dialog `BEGIN_MESSAGE_MAP` blocks are optional MFC shell macros and have no gameplay handlers. The SDL2 `AudioPlaySound` override maps to Firestaff’s verified GDAT sample path and 16-voice SDL3 backend, but its channel recycling, per-call playback rate and stereo panning differ; keep the renderer implementation partial. Its generic no-op virtual fallback is architecture-specific. SDL2 `AudioStop` maps partially to Firestaff’s sound-voice stop helper, but not the separate music queue, and its only source request route is unreachable in skproject; its generic fallback is architecture-specific.

DOS and SPX v5 `SUPPRESS_INIT` match Firestaff codec context initialization; its writer is reset once before ordered encoded sections and the retained reader starts at the encoded stream offset. Both source rows are implemented parity. SPX v4 has no matching standalone source definition and remains open.

DOS and SPX v5 SUPPRESS reader/writer/flush primitives now match Firestaff masked bit order, state carry, preserve-versus-zero-fill reads, underflow/capacity errors, and final zero-padded byte emission. All six active source rows are implemented parity; caller-level file/menu/UI routes remain separate open items.

DOS, SPX v4, and SPX v5 one-bit SUPPRESS reader/writer wrappers now map to Firestaff’s source-masked single-bit helpers. The SPX v4 READ_1BIT definition in skcore.cpp was missing from the active-source inventory and has been added; all six indexed DOS/SPX definitions are implemented parity.

DOS, SPX v4, and SPX v5 `SKLOAD_READ`/`SKSAVE_WRITE` wrappers are now source-compared as partial: Firestaff uses complete-file reads and path-level atomic writes, without the source current-file-handle and requested-byte-count adapters.

DOS and SPX v5 `WRITE_POSSESSION_INDICES` match ordered DB9/DB14 10-bit continuation encoding. Firestaff now aborts if an eligible possession index cannot be resolved instead of omitting it. Both rows remain partial because the save orchestrator has no production runtime caller.

The current save/load review compared DOS `SKW_READ_RECORD_CHECKCODE`, SPX v5 `DM2_READ_RECORD_CHECKCODE`, and the separate SPX v4 reader. DOS and v5 chain decoding now map to Firestaff’s staged record-pool decoder for continuation fields, database-specific masks, nested possessions, timers, and fail-closed restore errors. The v4 reader remains partial because it additionally reconstructs temporary minion associations for DB9 containers and DB14 missiles, and its caller/platform routes are not bound. The active inventory now includes that previously omitted v4 definition.

SPX v4 `AJ32` is now mapped to an exact Firestaff helper: subtract 8 under `__LINUX__`, otherwise return the supplied size. The source uses it only to adjust diagnostic output when comparing 32-bit and 64-bit MEMENT buffers; no MEMENT allocator/cache parity is claimed.

DOS, SPX v4, and SPX v5 `ALLOC_CAII_TO_CREATURE` definitions are reviewed as partial against Firestaff’s bounded allocator. Firestaff allocates a free slot and initializes base CAII data, but table-full world-record recycling and several local-variable, thinker, animation, and platform caller effects remain incomplete.


## Scope correction: Bloodwych v6 creature-name helper (2026-11-01)

`SKWINSPX/src/v6/xcorebwy.cpp:100` `BW_GetCreatureName` maps Bloodwych-specific creature IDs and is called only by the Bloodwych import diagnostic at line 1904. It is part of the experimental Bloodwych dungeon reader, outside the requested DM1/CSB/DM2 scope, and is recorded as `NOT_APPLICABLE_GAME`. The DM2 backlog remains 3,950 open rows (3,528 unreviewed, 420 partial source/older-ledger rows, 2 partial source owners); two Bloodwych-only v6 definitions are excluded.


## Latest DM2 collision-wound source pass (2026-11-01)

Compared DOS `SKW_12b4_023f`, SPX v4 `_12b4_023f_WOUND_RUNNING_INTO_CREATURE`, and SPX v5 `DM2_move_12b4_023f` with Firestaff's narrow receipt helper and the SKULLWIN implementation. Firestaff records the two adjacent direction candidates, suppresses duplicate champions, consumes supplied wound outcomes, and emits a noise request only for successful wounds. It does not bind source position lookup, live WOUND_PLAYER mutation, source-version return semantics or production noise playback on these three routes. Recorded each as `PARTIAL`. The DM2 active-source inventory now has 3,525 unreviewed, 350 partial, and 49 implemented-parity definitions, plus 8 architecture-specific and 2 outside-game rows. Combined open backlog: 3,950 (3,525 unreviewed, 423 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 side-offset movement source pass (2026-11-01)

Compared DOS `SKW_12b4_0953`, SPX v4 `CREATURE_12b4_0953`, and SPX v5 `DM2_12b4_0953` with Firestaff's side-offset helper. The helper reproduces facing-relative directions and the 5x5 horizontal-offset predicate, and a staged GAME_LOAD owner uses the source result. The live runtime movement path does not query the creature position or use the C5/C4 classification for side-offset creatures; each platform definition is recorded `PARTIAL`. Current DM2 active-source counts are 3,522 unreviewed, 353 partial, 49 implemented-parity, 8 architecture-specific, and 2 outside-game. The combined backlog remains 3,950 rows (3,522 unreviewed, 426 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 new-map loader source pass (2026-11-01)

Compared DOS `SKW_LOAD_NEWMAP`, SPX v4 `LOAD_NEWMAP`, and SPX v5 `DM2_LOAD_NEWMAP` with Firestaff's `dm2_v1_load_newmap`. The adapter mirrors the top-level callback ordering but is not called in production; its dynamic-level loader intentionally refuses because the live map/record/GDAT transaction is not implemented. All three platform definitions are therefore `PARTIAL`. Current active-source counts: 3,519 unreviewed, 356 partial, 49 implemented-parity, 8 architecture-specific and 2 outside-game. The combined DM2 backlog is 3,950 (3,519 unreviewed, 429 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 local-creature-context source pass (2026-11-01)

Compared DOS `SKW_13e4_0004`, SPX v4 `PREPARE_LOCAL_CREATURE_VAR`, and SPX v5 `DM2_PREPARE_LOCAL_CREATURE_VAR` with Firestaff's creature-AI context adapter. Firestaff documents the state contract and selected branch classifications, while the preparation function returns NULL/fail-closed without creating the runtime s350/DB4/CAII/AI-spec/timer context. All three source definitions remain partial. Current active-source counts: 3,516 unreviewed, 359 partial, 49 implemented-parity, 8 architecture-specific, and 2 outside-game. The combined DM2 backlog is 3,950 (3,516 unreviewed, 432 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 creature-AI initialization source pass (2026-11-01)

Compared DOS `SKW_13e4_01a3`, SPX v4 `_13e4_01a3`, and SPX v5 `DM2_13e4_01a3` with Firestaff's creature-AI initializer. Firestaff has a written contract for s350, AI-spec/GDAT, timing/RNG and allocation-fallback effects, but the function fails closed without mutating or binding them. The three platform definitions are `PARTIAL`. Current active-source counts: 3,513 unreviewed, 362 partial, 49 implemented-parity, 8 architecture-specific, and 2 outside-game. The combined DM2 backlog remains 3,950 (3,513 unreviewed, 435 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 command/AI-turn source pass (2026-11-01)

Compared DOS `SKW_13e4_0360`, SPX v4 `CREATURE_SET_NEW_COMMAND`, and SPX v5 `DM2_ai_13e4_0360`. The v5 AI-turn operation matches Firestaff's CAII helper, including the record/slot guards, byte@0x17 write, byte@0x13 protection, and timer cancellation/requeue tail, and has production runtime callers; recorded `IMPLEMENTED_PARITY`. DOS's CAII layout is not source-mapped to the Firestaff slot offsets, and v4 is a distinct command-queue/sequence-control function without a Firestaff setter, so both remain `PARTIAL`. Current DM2 counts: 3,510 unreviewed, 364 partial, 50 implemented-parity, 8 architecture-specific and 2 outside-game. Combined backlog: 3,949 (3,510 unreviewed, 437 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 creature-animation timing source pass (2026-11-01)

Compared DOS `SKW_13e4_071b`, SPX v4 `CREATURE_13e4_071b`, and SPX v5 `DM2_ai_13e4_071b` with Firestaff's timing receipt and creature-AI loop. The sources compute the animation phase from tick/frame-count data, mark aligned animations complete, or replace the active creature timer. Firestaff's runtime helper fails closed without animation mutation or timer ownership, and the loop has no production timing callback; all three definitions remain `PARTIAL`. Current active-source counts: 3,507 unreviewed, 367 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game. Combined DM2 backlog: 3,949 (3,507 unreviewed, 440 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 creature-animation 0x2000 timing source pass (2026-11-01)

Compared DOS `SKW_13e4_0806`, SPX v4 `CREATURE_13e4_0806`, and SPX v5 `DM2_ai_13e4_0806`. The source paths test the stable-animation state, compute phase modulo frame count, set the completion/timing flags, and replace the active timer when needed. Firestaff classifies that branch but its timing helper fails closed before mutating animation state or timers; no production AI timing callback is bound. All three variants remain `PARTIAL`. Current active-source counts are 3,504 unreviewed, 370 partial, 50 implemented-parity, 8 architecture-specific and 2 outside-game. Combined backlog is 3,949 (3,504 unreviewed, 443 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 AI action-context source pass (2026-11-01)

Compared DOS `SKW_14cd_0276`, SPX v4 `CREATURE_PATH_14cd_0276`, and SPX v5 `DM2_14cd_0276`. Firestaff extracts the behavior-entry fields and notes the path-allocation condition, but does not update live AI context/pointer fields, allocate/reuse path memory, or copy path bytes; the helper is not called by production AI decision code. All three source definitions remain `PARTIAL`. Current active-source counts: 3,501 unreviewed, 373 partial, 50 implemented-parity, 8 architecture-specific, and 2 outside-game. The combined backlog is 3,949 (3,501 unreviewed, 446 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 target-validation source pass (2026-11-01)

Compared DOS `SKW_14cd_0389`, SPX v4 `CREATURE_THINK_0389`, and SPX v5 `DM2_14cd_0389` with Firestaff's target-validation receipts. Each source validates live AI context, dispatches the selected creature action through the version-specific 0f0a handler, checks the resulting plan count, and may update the creature's selected target/plan field. Firestaff records guard and table-byte information but does not execute the live handler or mutate the creature, and has no production AI caller. All three definitions remain `PARTIAL`. Current counts: 3,498 unreviewed, 376 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,498 unreviewed, 449 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 action-flags source pass (2026-11-01)

Compared DOS `SKW_14cd_062e`, SPX v4 `_14cd_062e`, and SPX v5 `DM2_14cd_062e` with Firestaff's action-flag helper. The helper reproduces the table byte@5 mask and cross-map clear rule for supplied table/map inputs, but it has no production caller; the skproject classifier leaves table-derived results unresolved. All three definitions remain `PARTIAL`. Current counts: 3,495 unreviewed, 379 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,495 unreviewed, 452 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 next-action source pass (2026-11-01)

Compared DOS `SKW_DECIDE_NEXT_XACT`, SPX v4 action-table behavior, and SPX v5 `DM2_DECIDE_NEXT_XACT`. Firestaff's table-walker helper covers supplied entries, including 0xf6 register writes, but the source classifier stops before reading table data and no production runtime caller uses the helper. The DOS and SPX definitions remain `PARTIAL`. Current counts: 3,492 unreviewed, 382 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,492 unreviewed, 455 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 post-action transition source pass (2026-11-01)

Compared DOS `SKW_14cd_08f5`, SPX v4 `CREATURE_THINK_08F5`, and SPX v5 `DM2_14cd_08f5`. Firestaff's helper computes the branch transition from supplied action-table rows, while the sources also clear or update live table/row state and return whether the action changed. The helper has no production AI-loop caller and does not mutate the live context. All three definitions remain `PARTIAL`. Current counts: 3,489 unreviewed, 385 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,489 unreviewed, 458 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 action-dispatch source pass (2026-11-01)

Compared DOS `SKW_14cd_0f0a`, SPX v4 `_14cd_0f0a_EXE_17`, and SPX v5 `DM2_14cd_0f0a`. The source dispatchers run one of 17 action bodies that mutate live AI/game state. Firestaff classifies the case but its dispatcher fails closed before invoking any action body. All three definitions remain `PARTIAL`. Current counts: 3,486 unreviewed, 388 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,486 unreviewed, 461 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 action-entry source pass (2026-11-01)

Compared DOS `SKW_14cd_0f3c`, SPX v4 `_14cd_0f3c`, and SPX v5 `DM2_ai_14cd_0f3c`. Firestaff's helper computes the strength and record fields, while the source appends the populated entry to the live AI action array and increments its count. The helper has no production caller and does not mutate that live array. All three definitions remain `PARTIAL`. Current counts: 3,483 unreviewed, 391 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,483 unreviewed, 464 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 hexe action-walk source pass (2026-11-01)

Compared DOS `SKW_14cd_18f2`, SPX v4 `_14cd_18f2`, and SPX v5 `DM2_14cd_18f2`. The sources scan live hexe entries, apply action-condition checks, adjust selected entries, and append derived actions. Firestaff has separate condition-walk and action-entry helpers, but no live list traversal or production binding. All three definitions remain `PARTIAL`. Current counts: 3,480 unreviewed, 394 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,480 unreviewed, 467 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 condition-gated action source pass (2026-11-01)

Compared DOS `SKW_14cd_19c2`, SPX v4 `CREATURE_14cd_19c2`, and SPX v5 `DM2_14cd_19c2`. These wrappers gate on readiness, evaluate a hexe condition, update live action flags, and delegate to the 18f2 action-list walk. Firestaff has separate condition and list-walk receipts, but lacks a production-bound composition and live state mutation. All three definitions remain `PARTIAL`. Current counts: 3,477 unreviewed, 397 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,477 unreviewed, 470 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 condition-table action source pass (2026-11-01)

Compared DOS `SKW_14cd_1a78`, SPX v4 `_14cd_1a78`, and SPX v5 `DM2_14cd_1a78`. Sources resolve action conditions, scan hexe entries, run per-entry predicates, and append derived actions. Firestaff records table matches but does not run the source predicate or mutate the live action array, and has no production caller. All three definitions remain `PARTIAL`. Current counts: 3,474 unreviewed, 400 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,474 unreviewed, 473 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 conditional spell-action source pass (2026-11-01)

Compared DOS `SKW_14cd_1bac`, SPX v4 `_14cd_1bac`, and SPX v5 `DM2_14cd_1bac`. The sources gate on readiness/attack mode, evaluate action conditions, update the live action mask, and delegate to the 18f2 walker. Firestaff records branch decisions but does not mutate the mask or call the walker in a production route. All three definitions remain `PARTIAL`. Current counts: 3,471 unreviewed, 403 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,471 unreviewed, 476 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 target-position action pass (2026-11-01)

Compared DOS `SKW_14cd_1c8d`, SPX v4 `_14cd_1c8d_PFN17_09`, and SPX v5 `DM2_14cd_1c8d`. Firestaff's classifier matches the coordinate/map gate, but the source delegates to the 18f2 action-list walker on nonmatch and Firestaff's action wrapper fails closed before doing so. All three definitions remain `PARTIAL`. Current counts: 3,468 unreviewed, 406 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,468 unreviewed, 479 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 minion-missile action source pass (2026-11-01)

Compared DOS `SKW_14cd_1cec`, SPX v4 `_14cd_1cec_PFN17_10`, and SPX v5 `DM2_14cd_1cec`. Each resolves a minion's missile reference, checks for missile record type 9, reads its action parameter, and delegates to the action-list builder. Firestaff's classifier covers the reference lookup but does not execute the action walk or bind the production route. All three definitions remain `PARTIAL`. Current counts: 3,465 unreviewed, 409 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,465 unreviewed, 482 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 action-table copy source pass (2026-11-01)

Compared DOS `SKW_14cd_1eec`, SPX v4 `_14cd_1eec`, and SPX v5 `DM2_14cd_1eec`. Sources scan matching hexe entries, apply the action predicate, copy/adjust entries, and append derived actions. Firestaff records table-walk shape but does not execute the live predicate/mutation path or bind production callers. All three definitions remain `PARTIAL`. Current counts: 3,462 unreviewed, 412 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,462 unreviewed, 485 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 75 source pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_75`, SPX v4 `PROCEED_XACT_75`, and SPX v5 `DM2_PROCEED_XACT_75`. Firestaff has a source-mapped cache/mode receipt, while the reference paths also mutate live creature/AI flags and invoke the 19f0_0891 transaction. Firestaff's XACT-use-item runtime helper fails closed and has no production platform route. All three definitions remain `PARTIAL`. Current counts: 3,459 unreviewed, 415 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,459 unreviewed, 488 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 random-turn XACT source pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_57`, SPX v4 `PROCEED_XACT_57`, and SPX v5 `DM2_PROCEED_XACT_57`. Firestaff's callback helper models the random first movement, alternate attempt, and turn fallback, but no production AI/XACT route calls it. All three source definitions remain `PARTIAL`. Current counts: 3,456 unreviewed, 418 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,456 unreviewed, 491 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 target-move XACT source pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_59_76`, SPX v4 `PROCEED_XACT_59_76`, and SPX v5 `DM2_PROCEED_XACT_59_76`. Firestaff models item selection and handling checks, but the source paths also issue 19f0_2165 toward the live target; Firestaff's runtime XACT helper fails closed before that command and has no production caller. All three rows remain `PARTIAL`. Current counts: 3,453 unreviewed, 421 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,453 unreviewed, 494 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 69 source correction (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_69`, SPX v4 `PROCEED_XACT_69`, and SPX v5 `DM2_PROCEED_XACT_69`. All set a facing-adjacent target tile and update live creature target/action fields; this operation does not emit sound. Firestaff's helper computes related coordinates/command bytes but is mislabeled as sound and does not mutate live state or bind production callers. All three rows remain `PARTIAL`. Current counts: 3,450 unreviewed, 424 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,450 unreviewed, 497 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 64 source pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_64`, SPX v4 `PROCEED_XACT_64`, and SPX v5 `DM2_PROCEED_XACT_64`. Firestaff mirrors possession, flag, and item checks, but the source paths also issue projectile command 0x81 and restore live flags; Firestaff's handler fails closed before launch and is not production-bound. All three definitions remain `PARTIAL`. Current counts: 3,447 unreviewed, 427 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,447 unreviewed, 500 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 70 source pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_70`, SPX v4 `PROCEED_XACT_70`, and SPX v5 `DM2_PROCEED_XACT_70`. Sources construct target/action fields from party position and facing, check the creature at the adjacent target, and conditionally test item handling. Firestaff has a source-mapped receipt, but its runtime helper fails closed before lookup/state mutation and has no production caller. All three definitions remain `PARTIAL`. Current counts: 3,444 unreviewed, 430 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,444 unreviewed, 503 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 63 source correction (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_63`, SPX v4 `PROCEED_XACT_63`, and SPX v5 `DM2_PROCEED_XACT_63`. These check whether a creature ahead can handle the requested item; they do not cast spells. Firestaff has a matching query helper, but its XACT dispatcher does not call it in production and labels the operation as cast spell. All three rows remain `PARTIAL`. Current counts: 3,441 unreviewed, 433 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,441 unreviewed, 506 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 equipment-assessment callback pass (2026-11-01)

Compared DOS `SKW_14cd_2807`, SPX v4 `_14cd_2807_CREATURE`, and SPX v5 `DM2_14cd_2807` against skproject. Each callback checks whether an item can be handled, derives charge/type inputs, and accumulates the 48ae combat-stat query while the paired 2886 routine walks the creature possession chain. Firestaff contains source-shaped callback and chain helpers, but no production combat/AI caller binds them. All three definitions remain `PARTIAL`. Current counts: 3,438 unreviewed, 436 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,438 unreviewed, 509 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 68 combat-assessment pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_68`, SPX v4 `PROCEED_XACT_68`, and SPX v5 `DM2_PROCEED_XACT_68` with skproject. Each checks the front-handler condition, finds a creature ahead, queries attack and defense through `14cd_2886` and `48ae_0767`, then updates live AI/combat state. Firestaff has a source-shaped receipt helper, but no production dispatcher invokes it or applies the state changes. All three definitions remain `PARTIAL`. Current counts: 3,435 unreviewed, 439 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,435 unreviewed, 512 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 77 walk-path pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_77`, SPX v4 `PROCEED_XACT_77`, and SPX v5 `DM2_PROCEED_XACT_77` with skproject. Each scans live type-17 plan entries, appends walk candidates, and runs `FIND_WALK_PATH` to select a path. Firestaff has a structure-only receipt; it does not traverse/mutate the live plan or bind the path search into production. All three definitions remain `PARTIAL`. Current counts: 3,432 unreviewed, 442 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,432 unreviewed, 515 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 78 map movement pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_78`, SPX v4 `PROCEED_XACT_78`, and SPX v5 `DM2_PROCEED_XACT_78` with skproject. On a matching map, each computes direction toward the party, tests the adjacent tile, and issues `19f0_0559` when passable. Firestaff currently records map equality only; direction, tile query, movement dispatch, and production binding are open. All three definitions remain `PARTIAL`. Current counts: 3,429 unreviewed, 445 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,429 unreviewed, 518 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 80 movement delegation pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_80`, SPX v4 `PROCEED_XACT_80`, and SPX v5 `DM2_PROCEED_XACT_80` with skproject. Each sets temporary movement flags, derives direction/mode, delegates to `CREATURE_GO_THERE`, and restores the saved flags. Firestaff records the arguments but does not invoke the live movement or bind a production caller. All three definitions remain `PARTIAL`. Current counts: 3,426 unreviewed, 448 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,426 unreviewed, 521 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 83 command-selection pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_83`, SPX v4 `PROCEED_XACT_83`, and SPX v5 `DM2_PROCEED_XACT_83` with skproject. Each tests creature bit 7 and the action selector, clamps the selector, writes a `0x23`-series command, and returns variant result codes. Firestaff reproduces the branch in a receipt but has no production caller that applies the command to live creature data. All three definitions remain `PARTIAL`. Current counts: 3,423 unreviewed, 451 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,423 unreviewed, 524 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 XACT 91 possession item-check pass (2026-11-01)

Compared DOS `SKW_PROCEED_XACT_91`, SPX v4 `PROCEED_XACT_91`, and SPX v5 `DM2_PROCEED_XACT_91` with skproject. Each checks two selected item ids against the creature possession and succeeds if either item can be handled. Firestaff has query helpers matching the result logic, but no production XACT dispatcher calls them. All three definitions remain `PARTIAL`. Current counts: 3,420 unreviewed, 454 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,420 unreviewed, 527 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 creature-walk source pass (2026-11-01)

Compared DOS `SKW_CREATURE_WALK_NOW`, SPX v4 `CREATURE_WALK_NOW`, and SPX v5 `DM2_CREATURE_WALK_NOW` with skproject. They perform attack setup, movement planning/walkability checks, optional pit/teleport handling, record movement, landing/map/timer recovery, and pending-action updates. Firestaff contains a similar callback-shaped helper, but no production DM2 caller binds it to the live creature runtime; exact record and platform callback ownership remains open. All three definitions remain `PARTIAL`. Current counts: 3,417 unreviewed, 457 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,417 unreviewed, 530 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 CCM06 direction source pass (2026-11-01)

Compared DOS `SKW_1887_05aa`, SPX v4 `CREATURE_CCM06`, and SPX v5 `DM2_CREATURE_CCM06` with skproject. Firestaff has a corresponding direction helper and live CCM route that updates record byte 0x0f using the session RNG. Exact source RNG sequence and route coverage across all supported platforms are not yet verified, so all three definitions remain `PARTIAL`. Current counts: 3,414 unreviewed, 460 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,414 unreviewed, 533 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 CCM0B path-message pass (2026-11-01)

Compared DOS `SKW_1887_0c75`, SPX v4 `CREATURE_CCM0B`, and SPX v5 `DM2_CREATURE_CCM0B` with skproject. Each calls the 19f0_0d10 path/message gate and invokes message type 2 on the target tile when the creature is at action 0x0b. Firestaff has a callback implementation, but a production route and complete platform message ownership are not established. All three definitions remain `PARTIAL`. Current counts: 3,411 unreviewed, 463 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,411 unreviewed, 536 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 creature shoot-item source pass (2026-11-01)

Compared DOS `SKW_1887_0eca` and SPX v5 `DM2_CREATURE_SHOOT_ITEM` with skproject. Both select/cut a suitable possession item, derive projectile strength, apply two bounded random additions, clamp damage, then launch. Firestaff has a matching callback helper; the v5 row was already partial, and DOS is now recorded as partial because the production CCM/AI route does not compose selection, cut, and launch. Source RNG/platform routes remain open. Current counts: 3,410 unreviewed, 464 partial, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,410 unreviewed, 537 partial source/older-ledger rows, 2 partial source owners).


## Latest DM2 creature transformation source pass (2026-11-01)

Compared DOS `SKW_1887_126d`, SPX v4 `CREATURE_TRANSFORM`, and SPX v5 `DM2_CREATURE_TRANSFORM` with skproject. These transform/explode effects include cloud/noise phases, replacement creature type/AI/HP/animation setup, level-admission checks, and possible record deletion. No corresponding Firestaff transformation implementation exists; all three definitions are `MISSING`. Current counts: 3,407 unreviewed, 464 partial, 3 missing, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,407 unreviewed, 537 partial source/older-ledger rows, 3 missing, 2 partial source owners).


## Latest DM2 explode/summon source pass (2026-11-01)

Compared DOS `SKW_1887_141c`, SPX v4 `CREATURE_EXPLODE_OR_SUMMON`, and SPX v5 `DM2_CREATURE_EXPLODE_OR_SUMMON` with skproject. Variants handle explosion, minion creation, or transform/rebind phases with cloud/noise, creature-state changes, randomized HP/animation, and level validation. Firestaff has a callback implementation in its CCM dispatcher, but exact per-platform fields, RNG and side effects remain open. All three definitions remain `PARTIAL`. Current counts: 3,404 unreviewed, 467 partial, 3 missing, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,404 unreviewed, 540 partial source/older-ledger rows, 3 missing, 2 partial source owners).


## Latest DM2 1BA1B tile-passability pass (2026-11-01)

Compared DOS `R_1BA1B` and SPX v5 `DM2_1BA1B` with skproject. Both classify floor/empty/open tiles, apply the teleporter bit rule, and admit a special door only when its rebirth-altar graphics query returns zero. Firestaff has source-shaped callbacks but no verified production path/LOS caller binding across supported platforms. Both definitions remain `PARTIAL`. Current counts: 3,402 unreviewed, 469 partial, 3 missing, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,402 unreviewed, 542 partial source/older-ledger rows, 3 missing, 2 partial source owners).


## Latest DM2 19f0_045a tile-cache pass (2026-11-01)

Compared DOS `SKW_19f0_045a`, SPX v4 `_19f0_045a`, and SPX v5 `DM2_19f0_045a` with skproject. Each refreshes cached tile and record-search fields when tile coordinates or map change. Firestaff has an equivalent cache helper, but production map/cache caller binding across supported platforms remains unverified. All three definitions remain `PARTIAL`. Current counts: 3,399 unreviewed, 472 partial, 3 missing, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,399 unreviewed, 545 partial source/older-ledger rows, 3 missing, 2 partial source owners).


## Latest DM2 19f0_04bf record-chain pass (2026-11-01)

Compared DOS `SKW_19f0_04bf`, SPX v4 `_19f0_04bf`, and SPX v5 `DM2_19f0_04bf` with skproject. Each returns a cached first eligible tile record or walks past record types 0-3 and caches the result. Firestaff has the equivalent cache helper, but production record-chain caller binding remains unverified. All three definitions remain `PARTIAL`. Current counts: 3,396 unreviewed, 475 partial, 3 missing, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,396 unreviewed, 548 partial source/older-ledger rows, 3 missing, 2 partial source owners).


## Latest DM2 19f0_050f creature-record cache pass (2026-11-01)

Compared DOS `SKW_19f0_050f`, SPX v4 `_19f0_050f`, and SPX v5 `DM2_19f0_050f` with skproject. Each continues the tile-record chain to the first DB4 creature or end marker and caches that candidate. Firestaff has the equivalent cache helper, but production map-record caller binding remains unverified. All three definitions remain `PARTIAL`. Current counts: 3,393 unreviewed, 478 partial, 3 missing, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,393 unreviewed, 551 partial source/older-ledger rows, 3 missing, 2 partial source owners).


## Latest DM2 1c9a_0247 cache-free pass (2026-11-01)

Compared DOS `SKW_1c9a_0247`, SPX v4 `_1c9a_0247_FREE_CACHE_FROM_CREATURE`, and SPX v5 `DM2_1c9a_0247` with skproject. They free two allocator/cache keys; v4 uses selector 0x28000000 where DOS/v5 use 0x30000000. Firestaff has a dballoc helper for DOS/v5 selectors, but its runtime wrapper is a no-op, the v4 selector is absent, and production allocator ownership is unbound. All three definitions remain `PARTIAL`. Current counts: 3,390 unreviewed, 481 partial, 3 missing, 50 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,949 (3,390 unreviewed, 554 partial source/older-ledger rows, 3 missing, 2 partial source owners).


## Latest DM2 GET_CREATURE_AT source pass (2026-11-01)

Compared DOS `SKW_GET_CREATURE_AT`, SPX v4 `GET_CREATURE_AT`, and SPX v5 `DM2_GET_CREATURE_AT` with skproject. All scan the tile record chain to the first DB4 creature or return the null sentinel. Firestaff uses the same DB4 search in a shared helper with active AI, timer and world callers; corrupt chains fail closed under a bounded walk. All three definitions are `IMPLEMENTED_PARITY`. Current counts: 3,385 unreviewed, 481 partial, 5 missing, 53 implemented-parity, 8 architecture-specific, 2 outside-game; combined DM2 backlog 3,946 (3,385 unreviewed, 554 partial source/older-ledger rows, 5 missing, 2 partial source owners).

2026-11-01 skproject DM2 `GET_ITEM_NAME` review: DOS/SPX v4/SPX v5 item-name functions resolve the source record's GDAT category and index, with platform-specific champion-bones ownership checks. Firestaff has a callback helper and authenticated record-pool→GDAT `dtText/0x18` receipt, but the name route is not bound to live DM2 presentation; the leader-hand HUD explicitly suppresses DM2 names. All three source variants are partial.


2026-11-01 skproject DM2 `DRAW_ITEM_ICON` review: DOS, SPX v4 and both SPX v5 definitions select record/slot-specific animated item frames, draw source hand/inventory surfaces and optional action/selection slab overlays, then render into the proper hand group or dialogue surface. Firestaff callback and receipt counterparts omit background/highlight composition and have no live DM2 source-geometry/GDAT inventory/hand rendering owner; the M11 leader-hand icon API still lacks a verified rect. All four source rows are partial.


2026-11-01 skproject DM2 `SHOW_MENU_SCREEN` review: DOS and SPX v5 source loops select the raw TITLE/0/7/4 menu or decoded TITLE/0/4 image, retain title/credit assets, close graphics data, redraw and process cursor/input events, then release resources according to raw/decoded ownership. Firestaff has a live source-backed startup menu and original surface capture, but palette extraction, complete event-loop/mouse/redraw ordering, memory release and all supported-platform lifecycle parity remain open. Both source rows are partial.


2026-11-01 skproject DM2 `BURN_PLAYER_LIGHTING_ITEMS` review: DOS, SPX v4 and SPX v5 burn charged hand-held light items from the source champion range, clear importance on depletion and recalculate party lighting once. Firestaff has the helper algorithm but no production game-tick binding; source cadence and supported-platform ownership remain open. All three source definitions are partial.


2026-11-01 skproject DM2 `_24a5_0732` review: DOS/SPX v5 convert uppercase/extended bytes to source font codes and center by QUERY_STR_METRICS width; SPX v4 preserves MBCS text and prefixes only converted single-byte strings. Firestaff has isolated metrics/conversion and a receipt helper but no integrated production DM2 font-measure/encode/draw route. All three source variants remain partial.


2026-11-01 skproject DM2 `DRAW_SCROLL_TEXT` review: DOS/SPX v5 wrap source message text to the scroll panel width and draw vertically centered lines with source font metrics; SPX v4 additionally changes spacing/encoding for MBCS. Firestaff has only a message-query/image callback shell and a line-count receipt, with no source-faithful layout/pixel renderer or production DM2 caller. All four inventoried definitions are partial.


2026-11-01 skproject DM2 `CHECK_ROOM_FOR_CONTAINER` review: DOS/SPX v4/v5 detach up to eight child objects, null-fill the remaining slots and cache the current container handle. Firestaff's callback helper has the broad behavior but trusts caller capacity rather than enforcing eight and has no production runtime record-chain owner. All three source variants remain partial.


2026-11-01 skproject DM2 `MONEY_BOX_SURVEY` review: DOS/SPX v4/v5 draw counts for ordered currency kinds and denomination-weighted total. Firestaff's GUI routine sums raw counts instead, while its receipt has no survey arithmetic/rendering; the authenticated count helper has no production HUD binding. All four source definitions remain partial.


2026-11-01 skproject DM2 `_24a5_0e82` review: DOS/SPX v5 scale a value interval to a rectangle, select threshold colors, offset the background, and optionally draw a remainder segment. Firestaff has a substantial callback helper but no production DM2 HUD geometry/palette/blit binding; its core receipt models a different power-bar operation. All three definitions remain partial.


2026-11-01 skproject DM2 `24a5_1532` review: DOS/SPX v5 draw champion skill rows and current/previous ability comparisons using source labels, skill rank caps, change colors and rectangle layout. Firestaff has a structural receipt but no DM2 renderer; M11's actual champion stats view is ReDMCSB DM1-owned and is not a DM2 counterpart. All three source definitions are partial.


2026-11-01 skproject `R_2F5DE` review: DOS and SPX v5 copy the source-layout GDAT image block (six-byte header plus bpp-dependent payload) between header-relative pointers. Firestaff exposes decoded image realization and bounded copying but no source-header-aware duplicate wrapper or live caller. Both helper definitions are explicitly unreferenced and remain partial.


2026-11-01 skproject `QUERY_GDAT_IMAGE_METRICS` review: DOS, SPX v4 and SPX v5 select raw 10-bit dimensions for nonresident dtImage entries and decoded image-header dimensions for resident entries. Firestaff currently provides the decoded-buffer metric receipt without the source shelf-state branch or a live cross-platform query owner; all three function variants remain partial.


2026-11-01 skproject `QUERY_CMDSTR_TEXT` review: DOS/SPX v5 repeatedly parse matching key values with optional equals/minus prefixes and sign-flipped decimal accumulation. Firestaff has this behavior inside bounded CMDSTR receipts, but the declared generic query API is unimplemented and DOS 16-bit overflow behavior differs. Both definitions remain partial.


2026-11-01 skproject `QUERY_CMDSTR_ENTRY` review: DOS/SPX v5 enforce the 18-entry key bound, decode the selected GDAT text, return zero when empty, and dispatch through the source key table. Firestaff provides a matching bounded receipt consumed by some runtime owners, but not generic parity across editions/platforms; DOS 16-bit overflow semantics also differ. Both variants remain partial.


2026-11-01 skproject `_2759_01fe` review: DOS/SPX v5 command validity matches the SPX v4 `COMMAND_ON_MAGIC_MAP` and existing source receipt, including container subtype, minion missile/creature type and command gates. Firestaff binds the helper to the M11 hand-action path; other UI/command callers and platform routes remain incomplete. Both source rows remain partial.


2026-11-01 skproject `IS_MISSILE_VALID_TO_LAUNCHER` review: DOS queries one DBSPEC record twice and then requires bit 0x8000 both set and clear, so its variant always rejects. SPX v4/v5 compare distinct held-launcher and candidate-projectile masks; Firestaff has that standalone helper but no live resolver callback. All three variants remain partial.


2026-11-01 skproject `R_30DEA` review: DOS/SPX v5 share charge-decrement and conditional item-removal effects, but their return flags are opposite. Firestaff’s callback helper matches SPX v5 and is not production-bound; DOS requires a separate adapter. Both source rows remain partial.


2026-11-01 skproject `PROCEED_COMMAND_SLOT` review: DOS/v5 use slot charge/chance/RNG and the charge-depletion event, while SPX v4 directly consumes command charge. Firestaff only calls an injected engage handler and refreshes the panel; it lacks source charge/item transactions and a live callback owner. All three variants remain partial.


2026-11-01 skproject `FIND_HAND_WITH_EMPTY_FLASK` review: DOS/SPX v4/v5 scan hand slots 1 then 0 for a potion with cls2 0x14 and return its slot or -1. Firestaff has the matching source receipt, but its generic query API is unimplemented and no live potion cast owner uses the receipt. All three variants remain partial.


2026-11-01 skproject `TRY_CAST_SPELL` review: DOS/SPX v4/v5 all resolve the active hero rune string, cast with rune[0]-0x5f power, dispatch failures, mask failure class, clear rune state except class 0x30 and update the hands panel. Firestaff has a live M11 runtime-cast path, but bounded spell-table, failure UI and rune-state ownership leave parity partial for all three source variants.


2026-11-01 skproject `29ee_000f` arrow-panel review: DOS, SPX v4 and both SPX v5 definitions select arrow icon base 2/14, draw the six source GDAT arrows, restore the panel and dispatch animation group 5/6. Firestaff has the matching callback renderer without a live DM2 GUI caller binding the source panel and assets. All four variants remain partial.


### DM2 SKW_DRAW_HAND_ACTION_ICONS review (2026-11-01)
Compared DOS, SPX v4 and both duplicate SPX v5 definitions. They choose hand/active-state entries from INTERFACE_GENERAL/4, rotate the 0x46/0x4a hand rect by party position/facing, draw centered item-on-wood-panel pixels, black out dead heroes and gray overlay cooldown/sleep. Firestaff has a live source-bound renderer for party hand backdrops, selected state, cooldown/sleep and a selected-hand item graphic, including the Towns source-layout branch. Full source item/wood-panel materialization, dead-slot drawing and parity across every supported platform remain incomplete. All four source rows remain partial. Current active-source counts: 3,278 unreviewed, 585 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,278 unreviewed, 658 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 SKW_29ee_0396 formation icon review (2026-11-01)
Compared DOS `SKW_29ee_0396`, both duplicate SPX v5 `DM2_29ee_0396` helpers and SPX v4's inline `UPDATE_RIGHT_PANEL` formation route. They draw the squad background, crop source hero portraits by facing, suppress a dragged hero, optionally jitter the icon, draw directional/frame-selected aura overlays and apply panel gray state. Firestaff has a partial generic squad-icon crop helper and separate live Amiga/Towns formation renderers, but not the complete DOS/PC formation panel, drag/jitter/aura animation and overlay behavior across supported platforms. All three explicit active definitions remain partial. Current active-source counts: 3,275 unreviewed, 588 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,275 unreviewed, 661 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 command-slot player name review (2026-11-01)
Compared DOS `SKW_DRAW_PLAYER_NAME_AT_CMDSLOT`, SPX v4 `DRAW_PLAYER_NAME_AT_CMDSLOT` and both SPX v5 copies. All draw the command icon pair and active hero name, but DOS highlights using mouse state, v4 uses leader state/orange, and v5 uses event hero. Firestaff has partial callback and structural receipt pieces for the common icon/name route and v5 event color, but no production command-panel render caller or integrated DOS/v4 highlight owner. All four variants remain partial. Current active-source counts: 3,271 unreviewed, 592 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,271 unreviewed, 665 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 DRAW_PLAYER_ATTACK_DIR review (2026-11-01)
Compared DOS `SKW_DRAW_PLAYER_ATTACK_DIR`, SPX v4 `DRAW_PLAYER_ATTACK_DIR` and both SPX v5 copies. The sources draw the F6 directional background, the active hero's facing-cropped squad icon and palette, optional speed-aura jitter, a directional/tick-frame enchantment aura, and the rotate arrows. Firestaff has a callback shell, a structural draw receipt and a separate Towns aura command route, but no complete production right-panel raster owner across supported platforms; DOS also has edition-specific jitter/aura state. All four definitions remain partial. Current active-source counts: 3,267 unreviewed, 596 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,267 unreviewed, 669 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 DRAW_SPELL_TO_BE_CAST review (2026-11-01)
Compared DOS, SPX v4 and both SPX v5 definitions. The sources reset the right panel, optionally draw the cast-frame icon, then draw the selected champion's rune string in source button rectangles with edition palette colors. Firestaff's callback and structural receipt match the bounded frame/rune sequence, but no production DM2 spell-panel caller consumes them; live rune text/font presentation and supported-platform integration remain incomplete. All four definitions remain partial. Current active-source counts: 3,263 unreviewed, 600 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,263 unreviewed, 673 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 DRAW_CMD_SLOT review (2026-11-01)
Compared DOS, SPX v4 and both SPX v5 definitions. Normal mode draws an interface command icon and localized CMDSTR name; magical-map mode draws a container icon selected from command-record fields at a different button rectangle. Firestaff has a structural receipt for the two source branches, but its callback currently draws a fixed icon in either mode and omits the name; there is no production cmdslot record/panel-state owner. All four rows remain partial. Current active-source counts: 3,259 unreviewed, 604 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,259 unreviewed, 677 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 DRAW_CHIP_OF_MAGIC_MAP review (2026-11-01)
Compared DOS, SPX v4 and both SPX v5 definitions. Each clips a map-sized tile to the magic-map viewport and adjusts source crop offsets for mirrored edges before applying the frame atlas offset, alpha and palette. Firestaff's generic callback computes a full destination/frame X offset but omits the magic-map clip and edge-dependent source offsets; the separate scene viewport chip renderers are not a generic magic-map panel implementation. All four variants remain partial. Current active-source counts: 3,255 unreviewed, 608 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,255 unreviewed, 681 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 DRAW_MONEYBOX review (2026-11-01)
Compared DOS, SPX v4 and both SPX v5 definitions. They draw a moneybox frame, count and order ten coin types, resolve each coin's own GDAT image/local palette, then scatter up to 32 coin sprites per denomination using the rotating source position table and panel clipping. Firestaff paints a generic container icon per populated denomination; its receipt only summarizes occupied slots/stack caps and no production panel uses it. All four source definitions remain partial. Current active-source counts: 3,251 unreviewed, 612 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,251 unreviewed, 685 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 DRAW_CONTAINER_PANEL review (2026-11-01)
Compared DOS, SPX v4 and both SPX v5 definitions. They draw the container frame/lid in right-panel mode, inventory-relative frame imagery behind occupied slots, and each contained item's own class icon with slot-specific animation. Firestaff's callback asks for class graphics from the container record for every slot instead of the actual contained item; its source-core receipt only counts occupied slots. There is no production DM2 renderer binding the full path, so all four definitions remain partial. Current active-source counts: 3,247 unreviewed, 616 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,247 unreviewed, 689 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 2c1d_0186 enchantment self-cast review (2026-11-01)
Compared DOS `SKW_2c1d_0186` and SPX v5 `DM2_hero_2c1d_0186`; SPX v4 uses its renamed `CALL_ENCHANTMENT_SELF` path. The source optionally consumes mana, rejects zero mana, halves power when fewer than four points remain, then applies aura type, adjusted power and duration through the enchantment processor. Firestaff has matching cast and aura/timer helpers, but the compatibility engage-command wrapper has no production caller; a live cross-platform gameplay route is unproven. Both inventoried definitions remain partial. Current active-source counts: 3,245 unreviewed, 618 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,245 unreviewed, 691 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 GET_PLAYER_AT_POSITION review (2026-11-01)
Compared DOS, SPX v4 and SPX v5. Each scans party members, matches party position, requires the champion to be alive and returns its index or `-1`. Firestaff's generic QueryDB helper currently always returns `-1`; the structural receipt only indexes a supplied position map without checking live HP. A few separate runtime subsystems scan positions correctly, but they do not provide the general source-query behavior across platforms. All three definitions remain partial. Current active-source counts: 3,242 unreviewed, 621 partial, 5 missing, 56 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,943 open rows (3,242 unreviewed, 694 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 2c1d_0300 / BOOST_ATTRIBUTE review (2026-11-01)
Compared DOS `SKW_2c1d_0300`, SPX v4 `BOOST_ATTRIBUTE` and SPX v5 `DM2_hero_2c1d_0300`. They adjust current ability against maximum, reduce the delta by a quarter for each 20-point gap when moving farther away, and clamp the result to 10–220. Firestaff contains the same arithmetic in party-model and hero-stats helpers, but neither is bound to a production gameplay caller; the item-bonus compatibility call path has no production caller, leaving potion/special-item integration unverified. All three definitions remain partial. Current active-source counts: 3,240 unreviewed, 624 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,240 unreviewed, 697 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 TAKE_OBJECT review (2026-11-01)
Compared DOS `SKW_TAKE_OBJECT`, SPX v4 `TAKE_OBJECT` and SPX v5 `DM2_TAKE_OBJECT`. They set up the held-item state, flags and weight; draw the item/name while managing cursor visibility; choose immediate or deferred processing; apply item bonuses; and update room/moverec state. DOS masks the object reference and uses mouse-selected hero state, v4 uses leader state and a separate pointer/room-placement route, and v5 uses the event hero. Firestaff has a callback helper for the common sequence, but lacks these complete platform branches and production caller wiring. All three definitions remain partial. Current active-source counts: 3,237 unreviewed, 627 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,237 unreviewed, 700 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 REMOVE_POSSESSION review (2026-11-01)
Compared DOS `SKW_REMOVE_POSSESSION`, SPX v4 `REMOVE_POSSESSION` and SPX v5 `DM2_REMOVE_POSSESSION`. They clear a hero or container slot, return the removed object, refresh the active hand panel when the selected hand slot changes, and process item bonuses in removal mode. Firestaff's transfer helper models slot mutation and panel-refresh conditions, but leaves bonus mutation pending for the caller; production inventory wiring is incomplete. All three definitions remain partial. Current active-source counts: 3,234 unreviewed, 630 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,234 unreviewed, 703 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 EQUIP_ITEM_TO_HAND review (2026-11-01)
Compared DOS `SKW_EQUIP_ITEM_TO_HAND`, SPX v4 `EQUIP_ITEM_TO_INVENTORY` and SPX v5 `DM2_EQUIP_ITEM_TO_HAND`. Each ignores null, clears object-direction bits, stores to a hero slot or the hand-container slot at 30+, then applies item bonuses in add mode. Firestaff has the matching callback helper and storage branches, but it is reached only from the object-transfer helper and has no production caller proving the live inventory/bonus path across platforms. All three definitions remain partial. Current active-source counts: 3,231 unreviewed, 633 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,231 unreviewed, 706 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 GET_PLAYER_ABILITY review (2026-11-01)
Compared DOS `SKW_GET_PLAYER_ABILITY`, SPX v4 `GET_PLAYER_ABILITY` and SPX v5 `get_adj_ability1`. They select current/max attribute, add item enhancement, apply a current-only random aura bonus and clamp to 10–220. DOS has a compatibility fix that mutates its stored aura byte by -2 before comparison; v4/v5 compare aura−2 without that mutation. Firestaff has the bounded calculation in hero-stats, party and predicate helpers, but random/state ownership is caller-supplied and no shared cross-platform query is wired to source callsites; the DOS mutation is not preserved. All three definitions remain partial. Current active-source counts: 3,228 unreviewed, 636 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,228 unreviewed, 709 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 USE_LUCK_ATTRIBUTE review (2026-11-01)
Compared DOS `SKW_USE_LUCK_ATTRIBUTE`, SPX v4 `USE_LUCK_ATTRIBUTE` and SPX v5 `use_luck`. They try the random-bit/RAND16(100) shortcut, otherwise roll against twice adjusted current luck, then lower luck by 2 on success or raise by 2 on failure, clamped to 10..min(adjusted max luck,220). Firestaff has a structural helper for branching/mutation, but random values are injected, the helper uses raw current/max luck instead of adjusted aura/item values, and no production caller binds the operation. All three definitions remain partial. Current active-source counts: 3,225 unreviewed, 639 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,225 unreviewed, 712 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 MAX_LOAD review (2026-11-01)
Compared DOS `SKW_MAX_LOAD`, SPX v4 `MAX_LOAD` and SPX v5 `get_max_load`. They compute stamina-adjusted strength-based capacity, apply body-flag reduction and round down to tens. DOS uses a distinct ability selector; v4 literally shifts the load left in its body-flag reduction while DOS/v5 shift right. Firestaff matches the v5 formula and is used in runtime movement/weight paths, but does not cover the DOS selection and v4's literal behavior remains divergent. All three definitions remain partial. Current active-source counts: 3,222 unreviewed, 642 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,222 unreviewed, 715 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 USE_DEXTERITY_ATTRIBUTE review (2026-11-01)
Compared DOS `SKW_USE_DEXTERITY_ATTRIBUTE`, SPX v4 `USE_DEXTERITY_ATTRIBUTE` and SPX v5 `DM2_USE_DEXTERITY_ATTRIBUTE`. They combine adjusted dexterity with random jitter, subtract the carried-weight/max-load penalty, floor at 2, halve when sleeping and clamp against random bounds. Firestaff binds the equivalent weight/load/sleep helper into its attack path and uses session RNG for the three draws, but asks the adjusted-ability helper with rand16=0, making active dexterity-aura bonus fixed at +4 instead of source RNG behavior. All three definitions remain partial. Current active-source counts: 3,219 unreviewed, 645 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,219 unreviewed, 718 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 2c1d_132c armor scaling review (2026-11-01)
Compared DOS `SKW_2c1d_132c`, SPX v5 `DM2_hero_2c1d_132c` and v4 `SkWinCore::_2c1d_132c`. With scaling disabled they return the armor value's low byte; otherwise they extract packed bits 8..10, add four, multiply the low byte and shift right by three. Firestaff's equivalent helper masks those upper bits while computing its scale selector, so nonzero-scale input always uses factor four. The DOS and v5 active rows remain partial. Current active-source counts: 3,217 unreviewed, 647 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,217 unreviewed, 720 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 2c1d_135d shield-defense review (2026-11-01)
Compared DOS `SKW_2c1d_135d`, SPX v4 `CHAMPION_COMPUTE_SOME_SHIELD` and SPX v5 `DM2_hero_2c1d_135d`. They combine armor flags on two hand items, attack/throw skill and facing weights, a vitality roll, shield aura, hand-defense values, optional selected-slot armor, body-part damage penalty and sleep scaling before clamping to 0–100. Firestaff's similarly named helper instead sums six body slots under a body-flag mask and uses dexterity; it does not contain the source hand/skill/facing calculation and has no production caller. All three definitions remain partial. Current active-source counts: 3,214 unreviewed, 650 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,214 unreviewed, 723 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 DROP_PLAYER_ITEMS review (2026-11-01)
Compared DOS `SKW_DROP_PLAYER_ITEMS`, SPX v4 `DROP_PLAYER_ITEMS` and SPX v5 `DM2_DROP_PLAYER_ITEMS`. They walk a fixed 30-slot source order, remove each possession, skip nulls, pack party-position bits with the 14-bit object reference and move each item to the party tile. Firestaff has a callback composition for removal/move/packing, but takes order, count and destination from callers, returns a dropped count, and is only called by tests. Source ordering and live world-state ownership are not connected. All three definitions remain partial. Current active-source counts: 3,211 unreviewed, 653 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,211 unreviewed, 726 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 SHOOT_CHAMPION_MISSILE review (2026-11-01)
Compared DOS, SPX v4 and SPX v5 definitions. They derive launch direction from absolute facing and party position, clamp damage/kinetic energy/power to bytes, invoke SHOOT_ITEM at the party tile, then set throw counter 4 and last facing. Firestaff's helper reproduces the parameter translation and result state, but its only source-shaped caller is an unreferenced engage-command wrapper; it has no production projectile/record-pool binding across platforms. All three definitions remain partial. Current active-source counts: 3,208 unreviewed, 656 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,208 unreviewed, 729 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 CAST_CHAMPION_MISSILE_SPELL review (2026-11-01)
Compared DOS, SPX v4 and SPX v5 definitions. They reject insufficient mana, deduct the cost, set hero flag 0x8, adjust projectile power/accuracy and launch the missile. DOS/v4 derive accuracy from max stamina; v5 derives it from remaining mana. Firestaff's helper matches the v5 formula, but its only caller is an unreferenced engage-command wrapper, and the DOS/v4 rules differ. All three definitions remain partial. Current active-source counts: 3,205 unreviewed, 659 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,205 unreviewed, 732 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 37BEA hero special-force review (2026-11-01)
Compared DOS `R_37BEA`, SPX v5 `DM2_hero_37BEA` and SPX v4 `GET_CHAMPION_SPECIAL_FORCE`. Dead heroes return zero; living heroes return 50 when weight/10 plus the 0x10 hero flag is nonzero, otherwise 40. Firestaff has the matching formula in `dm2_v1_hero_37bea`, but the only callers are its standalone party helper and tests; production party-weight consumers are not bound. The DOS and v5 rows remain partial. Current active-source counts: 3,203 unreviewed, 661 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,203 unreviewed, 734 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 GET_PARTY_SPECIAL_FORCE review (2026-11-01)
Compared DOS, SPX v4 and SPX v5. They sum each active hero's special-force contribution: dead heroes count zero; living heroes contribute 50 or 40 based on carried weight/10 plus flag 0x10. Firestaff has the same per-hero helper and sum, but requires caller-supplied weights and is used only in tests; live party weight ownership is not bound. All three definitions remain partial. Current active-source counts: 3,200 unreviewed, 664 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,200 unreviewed, 737 partial source/older-ledger rows, 5 missing, 2 partial source owners).

### DM2 QUERY_GDAT_SQUAD_ICON review (2026-11-01)
Compared DOS, SPX v4 and both duplicate SPX v5 definitions. They select a facing-relative champion frame, choose normal or gray/invisible strip, load the hero-specific local palette and draw the cropped GDAT icon. Firestaff's callback reproduces frame/palette/blit parameters, but has no production caller binding the renderer. All three active DOS/v5 rows remain partial; the v4 counterpart was inspected as supporting platform evidence. Current active-source counts: 3,197 unreviewed, 667 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,197 unreviewed, 740 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 DRAW_PLAYER_3STAT_PANE review (2026-11-01)
Compared DOS, SPX v4 and both duplicate SPX v5 definitions. They gate on button-group-1 availability, select dead/selected/default GDAT field 1/9/0, draw it at hero+0xa1 and clear group size when xx is zero. Firestaff has matching bounded field selection and source-backed raster validation/rendering in its HUD route, but the generic callback omits the NODATA and group-reset semantics and has no production callsite; full interactive button-group ownership across platforms remains open. All four rows remain partial. Current active-source counts: 3,182 unreviewed, 682 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,193 unreviewed, 744 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 DRAW_PLAYER_3STAT_HEALTH_BAR review (2026-11-01)
Compared DOS, SPX v4 and both duplicate SPX v5 definitions, plus the SPX v5 map copy. They draw HP, stamina and mana fills at hero-relative rectangles using current/max values scaled to 10000; Mana uses max(maxMP, curMP). Each non-empty fill also draws an offset black shadow and a hero-color fill. Firestaff has a source-shaped callback and live HUD three-bar raster paths, but its dedicated receipt helper has no caller; generic HUD bars accept precomputed percentages, clamp to 0–100%, may use zero for unbound input and do not bind identical source color ownership across platforms. Keep all five active definitions partial. Current active-source counts: 3,182 unreviewed, 682 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,189 unreviewed, 748 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 SKW_2e62_0cfa review (2026-11-01)
Compared DOS SKW_2e62_0cfa with SPX v4 UPDATE_GENERAL_CHAMPIONS_STAT_DISP, SPX v5 DM2_events_2e62_0cfa and legacy SKWIN _2e62_0cfa. They refresh each non-selected hero, set the selected hero's 0x1000/0x3000 aura flags from edition UI/held-item state, refresh the selected hero, then update the right panel with the input mode. Firestaff has a callback composition with the same sequence plus separate refresh/panel helpers, but no production caller binds this coordinator to live state; platform-specific aura-flag conditions and end-to-end refresh ownership remain incomplete. DOS source row remains partial; v4/v5/legacy definitions are reviewed as supporting evidence, not additional active-inventory rows. Current active-source counts: 3,182 unreviewed, 682 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,188 unreviewed, 749 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 SKW_GLOBAL_UPDATE_UNKNOW1 review (2026-11-01)
Compared DOS SKW_GLOBAL_UPDATE_UNKNOW1 with SPX v5 DM2_GLOBAL_UPDATE_UNKNOW1 and legacy SKWIN GLOBAL_UPDATE_UNKNOW1; there is no active SPX v4 definition. They gate on a non-null held object and DBSPEC word 0 bit 0x8000, compute its _2405_014a image frame, and only redraw the carried item/cursor and update the remembered frame when that value changes. Firestaff has source-based item frame selection and a carried-item viewport renderer, but no matching guarded dirty-frame cache/poller or cursor refresh operation. Keep the active DOS/v5 source rows partial; legacy SKWIN was supporting evidence. Current active-source counts: 3,182 unreviewed, 682 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,187 unreviewed, 750 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 SKW_BRING_CHAMPION_TO_LIFE review (2026-11-01)
Compared DOS, SPX v4, SPX v5 and legacy SKWIN definitions. DOS repairs party position/direction, clears weight and 30 item refs, reduces maximum HP by maxHP/64+1 with floor 25, sets current HP to half and alive flag 0x4000, clears transient status/poison state, then runs cooldown/timer and mouse updates. SPX v5 instead clears weight/slots and aura/power around the same HP penalty; v4 invokes its champion-reset helper first. Firestaff has the matching HP/flag/aura calculations and live state mutations for restricted floor-mechanism revivals, plus transactional timer resurrection, but does not bind every source entrypoint or fully match DOS status/position repair and redraw behavior. The active DOS, v4 and v5 definitions remain partial; legacy SKWIN is supporting evidence. Current active-source counts: 3,182 unreviewed, 682 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,186 unreviewed, 751 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 ROTATE_CREATURE review (2026-11-01)
Compared DOS SKW_ROTATE_CREATURE, SPX v4 ROTATE_CREATURE and SPX v5 DM2_ROTATE_CREATURE. They set creature facing as relative or absolute and, when AI-spec flag bit 0 is set, rotate direction bits on every possession-chain object by the same delta. Firestaff has two bounded record helpers with matching bit transforms, but both are only referenced by tests; no production creature, timer or teleport route binds the complete source mutation to live records and AI-spec ownership. The active DOS and SPX v5 definitions remain partial; SPX v4 is supporting comparison. Current active-source counts: 3,182 unreviewed, 682 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,184 unreviewed, 753 partial source/older-ledger rows, 5 missing, 2 partial source owners).


### DM2 ROTATE_RECORD_BY_TELEPORTER review (2026-11-01)
Compared DOS, SPX v4 and SPX v5 (including the duplicate map.cpp copy). With teleporter flag 0x10 clear, source adds teleporter orientation to party heading and rotates the carried record direction; with the flag set, it sets party heading to teleporter orientation and leaves the record unchanged. Firestaff's canonical helper instead changes record direction in the set-bit branch and has no mutable party-heading state, reversing the source branch and omitting relative heading accumulation. The helper is test-only and has no production callsite. Active DOS and v5 definitions remain partial. Current active-source counts: 3,182 unreviewed, 682 partial, 5 missing, 55 implemented-parity, 8 architecture-specific and 2 outside-game; combined DM2 backlog: 3,944 open rows (3,182 unreviewed, 755 partial source/older-ledger rows, 5 missing, 2 partial source owners).
