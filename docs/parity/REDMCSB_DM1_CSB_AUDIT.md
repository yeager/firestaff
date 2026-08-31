# ReDMCSB DM1/CSB Native Parity Audit

Audit date: 2026-08-31
Reference: local ReDMCSB WIP20210206, `Toolchains/Common/Source/`
Scope: Firestaff's native DM1 and CSB V1 routes. ReDMCSB establishes control
flow and data ownership; it is not treated as a substitute for original-media
captures.

## Corrections made from source review

| Area | ReDMCSB evidence | Firestaff correction | Verification |
|---|---|---|---|
| DM1 psychic defence | `CHAMPION.C:F0321` calls `BASE.C:F0030`; the scaled product truncates | Removed a host half-up rounding step from `F0762_MAGIC_GetDefenderPsychicAdjustedAttack_Compat` | `dm1_v1_combat_damage_source_lock` |
| DM1/CSB low-stamina arithmetic | `CHAMPION.C:F0306` documents package/compiler operand-order behaviour | Native handoff chooses first-operand semantics only for Megamax Atari ST and High C FM Towns; PC 3.4 and DM1 Amiga 2.x retain second-operand semantics | `dm1_v1_f0306_stamina_pc34_compat` |
| DM1 PC 3.4 screen palette | `DATA.C:G0347` and `PALETTE.C:G3088` give the top/bottom RGB4 values | Replaced a hand-written Atari-looking DAC palette in the PC3.4 compatibility screen with the exact `G0347` values, losslessly expanded into the host DAC6 representation | `dm1_v1_screen_framebuffer_pc34_compat`, `dm1_v1_pc34_native_cli_boot`, `dm1_v1_dos_en_archive_cli_boot` |
| DM1 floor-object relocation | `DUNGEON.C:F0163` (line 1769) appends one THING only after clearing its next link; `F0164` (line 1840) removes the current list owner before a move. `MOVESENS.C:804→870` demonstrates that order in a source movement path. | Made the compact floor-cell drop transaction unlink → capacity-check → link, including rollback on a full destination; it can no longer leave duplicated floor entries or move coordinates without a list entry. The live M11 regression now also follows a retail C2548 alcove object through `F0302` inventory→mouse-hand and `F0374` mouse-hand→floor, then reads the compact source chain to require exactly one owner. | `dm1_v1_object_interaction_source_lock`; `m11_dm1_real_alcove_item_runtime_pc34` (retail ZIP, in-memory) |
| DM1 real archive object transfer | `CLIKVIEW.C:F0373` moves the painted pile-top THING to `G4055`; `PANEL.C:F0302` then owns placement in a legal inventory slot | The real alcove-item regression now accepts the untouched PC34 ZIP through the production M12→M11 handoff instead of requiring an extracted `DATA` directory. It proves a rendered original C2548 item can be picked up and placed in a legal source slot while retaining its original M564 name | `m11_dm1_real_alcove_item_runtime_pc34` (PC34 ZIP, in-memory) |
| DM1 PC 3.4 audio from selected archive | `SOUND.C:F0740-F0743` consumes `SONG.DAT`; `SWSH.C` drives the title PSG program, while `GRAPHICS.DAT:SND3` owns in-game effects | The audio locator now recognizes a selected retail ZIP as a data root and passes `ZIP::DATA/SONG.DAT` and `ZIP::DATA/GRAPHICS.DAT` to the shared bounded reader. No media file is extracted or written. The source music gate and live M11 audio initialization both exercise those virtual paths. | `dm1_v1_f0740_f0743_music_source_gate`; `dm1_v1_swsh_psg_audio_pc34_compat` (PC34 ZIP, in-memory) |
| CLI selected-game ingress | ReDMCSB's `STARTUP1.C` → `TITLE.C:F0437` → `ENTRANCE.C:F0441` path is reachable only after the caller has selected the game; `--menu` is an explicit alternative path | Initialised `M11_PhaseA_Options.menuRequested`. The field had been added to the public options type without a default, so stack bytes could make ordinary `--game dm1` silently retain M12 instead of entering the selected-game transaction | `m11_direct_launch_prepare_all_games`; debugger-backed native CLI option check (`directLaunch=1`, `menuRequested=0`) |
| DM1 open fake-wall floor ornaments | `DUNGEON.C:F0172` converts an open fake wall to a corridor, then calls `F0170` for `M558`; a closed fake wall instead follows the wall route | Preserve that effective-element conversion when calculating the M11 floor-ornament ordinal, so an open fake wall may expose its source-owned random ornament while a closed one cannot | `test_m11_overlay_command_queue_block` |
| DM1 closed fake-wall wall sensors | `DUNGEON.C:F0172` converts a closed fake wall to `C00` before walking wall `TEXTSTRING`/`SENSOR` things | Use the effective wall class for the M11 sensor and ornament draw paths, so visible source ornaments on closed or imaginary fake walls are no longer discarded as raw type `C06` | `test_m11_overlay_command_queue_block` |
| CSB teleporter eligibility | CSB's Lord Chaos/Grey Lord/Materializer rule differs from the DM1 comparison path | CSB helper defaults to the CSB rule and exposes DM1 only as an explicit comparison mode | `csb_v1_graphics_extras_pc34_compat` |
| CSB F0093 creature palette | `DUNVIEW.C:2805-2815` restores colour slots 9/10 through replacement sets 8/12 before walking `CurrentMapAllowedCreatureTypes`; Atari and version-3 `G2025`/`F0695` tables have distinct D3 targets | The live M11 adapter resets both slots, applies the current map's final ordered owner, and selects the source palette family for Atari, Amiga, and FM Towns rather than applying PC targets | `csb_v1_f0093_replacement_palette_pc34_compat`, real Atari/Amiga/FM Towns startup and M12→M11 handoff tests |
| CSB Atari MSA intake | Atari preservation media must be decoded in memory before the FAT-owned game files are read | Removed an impossible host-size overflow branch after retaining the actual MSA bounds (`<=64` sectors/track, `<=2` sides); the remaining multiplication guards cover the decoded image | `csb_v1_atari_msa` |
| CSB FM Towns F0115 group blit | `DUNVIEW.C:F0093` applies the current map's ordered allowed-creature palette owners before `F0115` draws a group | The F31 runtime stores its admitted dungeon outside M11's legacy DM1 mirror. The palette bridge now reads that real map's metadata list directly from the owned dungeon payload, so F0093 can complete and the IMG2-backed Prison group blits rather than failing closed | `csb_v1_fmtowns_en_m11_real_media_handoff`, `csb_v1_fmtowns_ja_m11_real_media_handoff` |
| CSB M11 input bridge scope | `COMMAND.C:F0361/F0380` queues and dispatches C001..C006; mouse hits reach the same queue through `F0358/F0359` | Corrected stale wording that described the narrow event translator as an active playability TODO. M11 owns the live keyboard, pointer, and gamepad consumers and invokes the bridge only for the source command queue transition | `csb_v1_input_command_bridge_pc34_compat`, `csb_v1_command_input_geometry_pc34_compat`, `m11_gamepad_csb_input_bridge` |
| CSB launcher-gate regression fixture | M12's current interaction is game card -> platform card -> presentation card -> launch; it must not silently reinterpret a platform-selection action as a launch | Marked the fixture as already past the platform-card step before it sends `ACCEPT` to the canonical launch gate. The test again verifies the intended invariant: hash-shaped launcher metadata can form an intent but cannot manufacture the absent original files needed by the native CSB boot profile | `csb_v1_required_complete_launches` |
| DM1 FM Towns picture-library loader test | The shipped Towns runtime owns `DATA/GRAPHICS.DAT`; ReDMCSB's FM Towns source maps this through `INIT_PIC_LIB` / `READ_ASSET` rather than a generated cache | Removed the test-only `/tmp` fixtures. Fixtures now stay in CTest's build working directory and are removed after each assertion, matching Firestaff's no-extraction storage rule | Rebuilt `test_dm1_v1_fmtowns_pic_library_loader`; direct CTest pass |

## Validated boundaries

- DM1 supplied DOS, Atari ST, Amiga and FM Towns media boot through the native
  CLI/start-menu paths. This is start and handoff coverage, not a full
  campaign or pixel-parity claim.
- On 2026-08-31, an independent native CLI boot-probe repeated this result
  against the current user data root, without an emulator or extracted game
  directory. Each route reached `phase=dm1-runtime` and loaded map 0 at party
  position `(1,3)`:

  | Selected platform | Preserved source form | Native decoder handoff |
  |---|---|---|
  | PC DOS 3.4 | `Dungeon-Master_DOS_EN_Version-34.zip::DATA/GRAPHICS.DAT` | `pc-img3` |
  | Amiga 2.0 | nested ZIP → ADF → `GRAPHICS.DAT` virtual member | `amiga-img2` |
  | Atari ST | nested ZIP → STX → `GRAPHICS.DAT` virtual member | `atari-st-dmcsb1` |
  | FM Towns EN/JP disc archive | ZIP-resident disc members | `fmtowns-tmenu-edm`; verified `EDM.EXP` and CDDA track 2 |

  The virtual-member notation records in-memory reads only. It is not an
  extraction step and does not grant campaign or original-frame parity.
- On 2026-08-31 the rebuilt targeted DM1 real-media suite passed every
  runnable route: PC 3.4, DOS archive, Atari ST (English/German/French),
  Amiga HD/v2.0, and FM Towns archive boot; the loader, CD/audio, menu,
  graphics, and Amiga data readers also passed. The sole skipped gate requires
  a separately supplied authentic F0134/F0135 material capture and remains
  correctly non-promoted.
- The direct 11-route CLI matrix was also rerun on 2026-08-31 after the
  floor-object ownership repair. It passed PC 3.4 (plain and nested archive),
  English and French DOS, English/German/French Atari ST, two Amiga archive
  layouts, and the FM Towns archive. Every assertion reads the supplied media
  as an in-memory member and requires the native start-menu and runtime
  handoff; none accepts an extracted directory, emulator process, or
  generated game-data fallback.
- CSB supplied Atari ST, Amiga and FM Towns media boot through the native
  routes. The CSB map loader, teleporter chain, projectile cadence, end-game
  raw receipt and version gate have focused regression coverage.
- On 2026-08-31 the rebuilt focused CSB real-media suite passed its Atari ST,
  French Atari, Amiga, PC and FM Towns (English/Japanese) routes, including
  M12-to-M11 handoff, source palette selection, teleporter rotation, Lord
  Chaos direction, projectile post-teleport consumers, and real graphics
  readers. After the FM Towns F0093 repair, the complete labelled CSB CTest
  suite (161 tests on the current tree) passed without failures. This closes no capture-only item
  below; it confirms that the implemented source rules admit the supplied
  media without an emulator.
- The directly exercised native CLI matrix additionally passed against the
  supplied Atari STX, nested Atari ZIP, Amiga archive, and FM Towns EN/JP
  archives. It verifies the native source title phase, explicit source input
  into the initial Prison runtime, the bounded first-command matrix, and the
  mouse/keyboard start-menu route. The Atari route is intentionally expected
  to remain in `ANIMATE.SCR` until `Enter`; merely reaching that title is not
  reported as a loaded dungeon.
- All reviewed code consumes game archives in memory. No emulator, BIOS or
  extracted-game-data directory is required at Firestaff runtime.
- DM1 PC 3.4's selected ZIP is also a first-class audio root: `SONG.DAT` and
  `GRAPHICS.DAT:SND3` are read as individual virtual members, and the
  source-owned F0740-F0743/music plus SWSH/PSG tests pass against that form.
- The FM Towns loader regression is rebuilt before interpreting its result:
  a stale executable compiled against an older layout can produce an invalid
  free and is not evidence of a runtime media defect.

## Evidence still required

The following are deliberately not marked complete without original media
captures:

- DM1 original-save interoperability over a corpus of real campaigns. The
  distributed archives do not contain a save corpus.
- CSB long-route captures covering a Grey Lord encounter, ZOKATHRA cast,
  teleporter traversal, cold save/resume and end-game presentation.
- CSB Atari creature captures on a BUG7_01 map. The source reset/order is now
  runtime-bound, but a real frame comparison is still needed for pixel-level
  confirmation.

Synthetic fixtures may isolate these rules in tests, but they do not satisfy
any of the capture requirements above.
