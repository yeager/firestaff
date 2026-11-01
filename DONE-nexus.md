# Firestaff DONE — Nexus

## 2026-11-01 — Verify frame-300 visible pixel source classes

- The authentic Japanese frame-300 composition joins all 65,991 visible VDP1
  pixels to their last-writing command and verified source receipt. Of these,
  65,500 belong to indexed retail `LEV01.DGN` rows; the 15 direct-color DGN
  draws own no final visible pixels in this capture. The remaining 491 CRAM
  pixels map to captured polyline row 79 (60) and mode-4 sprite row 80 (431).
- The combined real-data regression now asserts this exact source-class
  distribution alongside the full VDP2 RGB match and command/source joins.
  It does not establish selected Structure3 face ownership or production
  renderer admission. Details are in
  [`docs/nexus_vdp2_gameplay_frame300_composition.md`](docs/nexus_vdp2_gameplay_frame300_composition.md).

## 2026-11-01 — Match an authentic gameplay VDP pass in Firestaff

- Added an optional real-media test case with SHA-256-pinned VDP1 FBS, VDP2
  VRAM/CRAM/registers and Ymir final RGBX input for Japanese retail gameplay
  save-state frame 0.
- On TRV2, the capture-only priority compositor reproduced every one of the
  71,680 RGB pixels in the Ymir 320×224 final software-rendered framebuffer.
  The resulting layer counts are 64,804 VDP1, 3,297 NBG0, 74 NBG1 and 3,505
  NBG2 pixels, with no empty pixels or unsupported color-calculation/shadow
  cases.
- Ymir's NBG1 8bpp trace had 71,680 valid fetches; all dot bytes agreed with
  the same-pass raw VDP2 VRAM snapshot. The existing test also passes without
  external capture variables.
- This is one state on Ymir and capture-only host composition. It does not
  establish all-frame/hardware parity or authorize the live renderer. Details
  and reproduction steps are in
  [`docs/nexus_vdp2_gameplay_pass0_composition.md`](docs/nexus_vdp2_gameplay_pass0_composition.md);
  raw evidence remains on the external disk.

## 2026-10-31 — Replay authentic NBG0 live-row scrolling

- Added a capture-only NBG0 bitmap sampler that consumes verified Ymir
  per-row scroll and coordinate increments, falling back to the captured
  register snapshot on rows without live state.
- The retained authentic Japanese intro capture reproduces all 71,680
  screenshot samples exactly when the live `0x1f79/0x0bcd` scroll state is
  applied to rows 4–116. The prior final-register replay matched 70,454.
- The real-data CTest pins the VDP1/VDP2 dumps, screenshot, and retained
  baseline replay. The replay receipt reports 21,455 NBG0-selected pixels and
  50,225 VDP1-selected pixels; the prior replay matched 70,454 screenshot
  upper-left samples, while the scanline replay matches all 286,720 samples
  across the 2× screenshot blocks exactly.
- This remains capture-only; a screenshot match does not by itself prove a
  source-to-VRAM timing join or authorize production renderer admission.
  Evidence and reproduction are in
  [`docs/nexus_vdp2_nbg0_live_rows.md`](docs/nexus_vdp2_nbg0_live_rows.md).

## 2026-10-30 — Exact Ymir VDP1 replay for one captured frame

- The capture-only integer stepper replays 124 indexed mode-1 distorted
  sprites and 28 flat polygons from the authenticated frame-13162 Ymir trace.
- Its real-data CTest pins the complete 168-row trace, VDP1 VRAM/FBS/registers,
  VDP2 registers, and frame receipt. The 320x224 output matches Ymir FBS buffer
  0 exactly across all 71,680 RGB555 pixels and opacity bits.
- The companion frame-window test verifies that the other eight mode-5 draws
  are transparent, accounting for all 160 draws in the captured window.
- The stepper claims no DGN ownership and retains `renderer_permitted=false`.
  This single captured-state replay is not Saturn hardware parity, arbitrary-
  frame parity, or production renderer admission. See
  [`docs/nexus_vdp1_frame13162_dgn_window.md`](docs/nexus_vdp1_frame13162_dgn_window.md).

## 2026-10-31 — Replay the authentic frame-59 title prompt

- The capture-only NBG0-over-VDP1 compositor accepts the hash-pinned Ymir
  frame-59 zero-scroll, zero-palette-bank register profile. It writes 18,513
  NBG0 pixels and 53,167 captured VDP1 pixels and matches all 71,680 RGB
  pixels in the retained native-size Ymir reference.
- The real-data CTest also compares the replay with the authentic 640×448
  screenshot: all 286,720 doubled samples are within eight RGB levels, with
  maximum channel error eight. The test pins the raw capture, complete VDP1
  command bundle/trace, register receipt, reference and screenshot hashes.
- The two VDP1 framebuffer buffers are byte-identical. The raw VDP2 register
  word at offset `0xf8` is `0x0004`; an older evidence note's `0x01f4` value
  does not match the hash-pinned register dump. The compositor stays
  capture-only and does not claim full bitmap source ownership or production
  renderer admission.
- Evidence and reproduction steps are in
  [`docs/nexus_vdp2_title_frame59_composition.md`](docs/nexus_vdp2_title_frame59_composition.md).

## 2026-10-30 — Admit authentic CUE-only Nexus media

- M12 now binds a Saturn CUE/BIN source to its own ISO data track and admits
  its region only when canonical `DM.BIN`, regional `RLOWFIX.BIN`, and at
  least two localized title/menu/font hashes agree. Every CUE-declared media
  file must be present.
- Existing extracted or recognized full-image owners keep precedence. A
  directly selected unverified CUE is terminal and cannot fall through to a
  sibling Nexus disc.
- The authentic-media `nexus_v1_native_menu_mouse_title` regression passed
  on TRV2 for direct CLI and pointer-menu routes from an isolated Japanese
  CUE/BIN set and for rejection of an authentic non-Nexus CUE beside a valid
  Nexus sibling.
- The engine reaches the established title compositor gate. Nexus still
  does not have a verified playable viewport/HUD path.

## 2026-10-29 — Verify regional ownership for extracted Saturn resources

- M12 now distinguishes extracted Japanese, English, and French Nexus data by
  authentic `RLOWFIX.BIN` plus at least two matching localized
  `TITLE.BIN`/`MENU.BPK`/`FONT256.S2D` fingerprints. The byte-identical
  `DM.BIN` no longer proves a Japanese edition by itself.
- A supplemental Saturn CUE/ISO is admitted to an extracted runtime only when
  its canonical shared `DM.BIN`, `RLOWFIX.BIN`, and at least two present
  localized resources agree with the extracted region. The authentic Japanese
  CUE in the staged English test remained unattached.
- On TRV2, the real JP and staged real EN menu tests passed through the
  capture-gated title route. Direct `DM.BIN` launches retained the matching
  extracted region and supplemental CUE; the canonical EN ISO menu route
  retained its full-image edition. No game-media bytes were modified.
- This does not open the native title/menu compositor gate. A standalone CUE
  with its referenced tracks is now recognized, but the game still does not
  have a verified playable viewport/HUD path.

## 2026-10-29 — Identify the authentic BKR health pair

- The BKR source halfwords at champion offsets `+0x38/+0x3a` map to runtime
  `+0/+4`. `FUN_0603b014` initializes those same runtime cells from the PLRD
  health value. The pair is therefore current/maximum health, not an unnamed
  recovery pair. Health updates after Continue have not yet been bound.
- The BKR receipt and authentic-media regression now expose and check the
  values as `champion_health_current` and `champion_max_health` across all four
  hash-pinned retail saves. The engine still does not apply a complete Saturn
  Continue payload.

## 2026-10-29 — Identify the authentic BKR mana pair

- `FUN_0602eb4c` maps BKR halfwords at `+0x40/+0x42` to runtime `+0x10/+0x14`.
  The New Game initializer fills those cells from the PLRD mana value at
  `+0x0a`; `FUN_060361a8` increments the first toward the second. This binds
  the pair as current and maximum mana.
- The BKR receipt and authentic-media regression now expose the fields as
  `champion_mana_current` and `champion_max_mana`. The four authentic saves
  contain `18/20`; the retained retail update capture shows `19/20`.
- The complete payload mapping and Firestaff Continue application remain
  incomplete.

## Superseded — Initial conservative label for the health pair

- `FUN_0602eb4c` projects signed BKR halfwords at champion offsets `+0x38/+0x3a`
  to runtime longwords `+0/+4`. `FUN_060361a8` caps increases at the gap
  between the two values. These were initially described as recovery fields
  because the domain had not yet been identified. Follow-up evidence in the
  section above maps them to current and maximum health; the receipt and real
  media check now use health names. Full BKR application to the engine and
  playable Continue remain open.

## Superseded — Initial conservative label for the mana pair

- `FUN_0602eb4c` projects signed source halfwords at champion-record offsets
  `+0x40/+0x42` to runtime longwords `+0x10/+0x14`. `FUN_060361a8` adjusts
  the first value within the second value's ceiling. The retained Ymir trace
  shows retail changing `18/20` to `19/20`.
- This pair was initially left as a generic bounded value and limit while its
  game meaning was unknown. Follow-up evidence above identifies it as current
  and maximum mana; the receipt and real-media check now use those names.
- This does not complete the champion schema or apply a Saturn save to the
  Firestaff engine.

## 2026-10-29 — Bind the authentic BKR stamina pair

- `FUN_0602eb4c` projects signed save halfwords at champion-record offsets
  `+0x3c/+0x3e` to runtime longwords `+0x08/+0x0c`. The retail updater
  `FUN_060360fc` subtracts from `+0x08`, handles values below one as damage,
  and clamps against `+0x0c`, identifying current and maximum stamina.
- The bounded BKR probe returns this pair for each champion. The regression
  checks all four hash-pinned authentic saves: `66/67` for each champion in
  the two-, three-, and ordinary four-champion samples, and `67/67` in the
  leader-index-3 sample. No synthetic game data was added.
- All remaining champion fields stay opaque, and applying Saturn Continue
  state to Firestaff's engine remains open.

## 2026-10-29 — Bind the authentic BKR party leader index

- pcrecomp/Ghidra decompilation of `FUN_0602eb4c` shows payload byte `+0x61`
  sign-extended into runtime context `+0x0c`. All four authentic BKR payloads
  match same-run Ymir WorkRAM at `0x0608009c`, including the leader-index-3
  save (`0, 0, 0, 3` across the two-, three-, and four-champion samples).
- The bounded BKR probe now returns this index and rejects negative or
  out-of-roster values. The real-data test uses all four hash-pinned saves and
  derives malformed cases from authentic payload bytes; it introduces no
  synthetic save fixture.
- This identifies one context field only. Complete payload mapping, engine
  state application, and playable Continue remain open.

## 2026-10-29 — Join authentic BKR name prefix to Japanese PLRD roster

- Extracted the Japanese `RLOWFIX.BIN` from the authentic Track 1 image in
  memory and verified its 72,332-byte SHA-256 against the pinned retail
  identity. Compared its PLRD/TABL entries against all four hash-verified BKR
  payloads without adding original media to Git.
- Every champion record in these saves starts with `a3 88 96 80 80 00`.
  The first three bytes uniquely match PLRD roster row 2 and TABL codes
  `0x88`, `0x65`, `0x6d`; the loader copies six bytes to runtime `+0x2c`.
- Documented the unresolved two-byte suffix, the missing runtime text
  consumer, and the need for an authentic save with a different roster member.
  This is a partial source join, not a BKR import or text-rendering feature.

## 2026-10-29 — Expand authentic BKR roster projection checks

- Extended the pcrecomp/Ghidra-directed analyzer to verify 83 loader
  operations per champion across authentic two-, three-, and four-champion
  captures. The checks cover byte spans, byte-to-halfword conversions,
  sentinel handling, repeated tables, and signed/unsigned halfword fields.
- Added validation of the retained leader-3 write trace for post-load changes
  to runtime offsets `+0x18` and `+0x1a`. The expanded capture now includes all
  four runtime records and explains all 16 final-value differences across
  offsets `+0x08`, `+0x10`, `+0x18`, and `+0x1a`.
- Kept save-field semantics unassigned and champion records opaque in
  Firestaff. This evidence advances the mechanical roster projection only; it
  does not implement Saturn BKR loading or playable Continue.
- Re-ran the analyzer against all four authentic captured payloads and the
  complete leader-3 trace. The two-, three-, four-, and leader-3 runs report
  166, 249, 332, and 332 checks, respectively. The full leader-3 update
  capture explains all 16 final-value differences, with none unresolved in
  the four tracked runtime fields.

## 2026-10-28 — Source-bind Japanese Configuration foreground and mouse route

- Enabled the Configuration foreground for the hash-verified Japanese retail
  `TITLE.BIN`. The 14 UI text sprites and active/inactive palettes are read from
  real retail offsets, whose 71 text, palette and marker spans match the
  English retail source byte-for-byte.
- Opened Configuration with Accept and routed pointer row/value/Exit input for
  both admitted retail regions. The M11 integration test covers Japanese
  Configuration mouse selection and exit.
- Added `scripts/verify_nexus_japanese_configuration_page.py`. It compiles the
  production renderer, checks seven menu states against decoded Japanese retail
  bytes, and matches all 12,518 visible text pixels against an authentic Ymir
  screenshot. Scene background, Japanese interaction-state captures, and exact
  marker-animation pixels remain unverified. No synthetic game assets were
  added.

## 2026-10-28 — Correct English Configuration scene match accounting

- Re-ran both source joiners against the authentic English Ymir frame-119
  capture and hash-verified local Nexus media. The source scan examined 148
  draws: 121 informative spans match DGN Structure2; 16 other informative
  spans match other admitted retail resources but not DGN Structure2, and 11
  spans are zero-filled. The DGN command join confirms 113 indexed image/CLUT
  matches and 8 direct-color matches; 105 have candidate Structure3 owners.
- Added explicit DGN-only counts and VDP1/DGN encoding checks so source-span
  matches cannot cross indexed and direct-color formats. The report remains
  candidate evidence only; active face, camera, placement and composition are
  unbound.
- Reproduction details and limitations: [English Configuration DGN join](parity-evidence/verification/nexus_english_configuration_scene_dgn_join_20261028/README.md).

## 2026-10-28 — Join Japanese Configuration scene commands to retail DGN

- Added a Ymir trace-prefix reader for complete capture files whose current
  frame has not reached VDP1 END. It validates the capture receipt, command
  indices and addresses, current-command cursor, zero dropped rows, and VRAM
  size without claiming that the observed display list is complete.
- Applied it to the authentic Japanese Configuration-page capture at frame
  599. Of 132 textured draws, 11 are all-zero indexed textures with
  transparency enabled and no visible texels. All remaining 121 informative
  draws join to authentic DGN Structure2 bytes and matching indexed CLUT or
  direct-colour data; 105 have at least one candidate Structure3 face owner.
  Sixteen matched draws have no candidate face-owner row.
- The joins still leave shared material ownership, active level instance,
  selected faces, camera transform, culling, display origin, and VDP2
  composition unresolved. Renderer admission remains blocked. The full report
  and capture are retained on the external evidence volume; no retail data was
  added to Git.
- The 16 matched draws without face-owner rows are early distorted-sprite
  quads using five 48x48 materials from `LEV00.DGN`. Their vertices form a
  receding tiled plane at the bottom of the scene, consistent with a floor or
  menu backdrop; the active level and map selector remain unproven. Ymir source
  confirms the 11 all-zero indexed draws are transparent with the captured
  PMOD. No rendering permission follows from these candidate roles.
- Receipt: [Japanese Configuration DGN trace prefix](parity-evidence/verification/nexus_japanese_configuration_dgn_trace_prefix_20261028/README.md).

## 2026-10-28 — Bind a partial Japanese title source span to NBG0 writes

- Added a bounded verifier for the complete authentic Ymir VDP2 write trace.
  It authenticates the Japanese CUE and `TITLE.BIN`, checks the matching
  frame-8,060 snapshot receipts, proves a frame-6,869 clear and replays the
  frame-7,729 byte writes to reconstruct a 921-byte `TITL` record 2 span at
  NBG0 offset `0xd404`.
- The span at `TITLE.BIN+0xb8a2` contains 295 nonzero bytes. Full bitmap decode
  forming the native one-row title divider at `x=13..307, y=106`. Full bitmap
  decode and display consumer work remain open; no presentation admission or
  synthetic data was added.
- Verification used the retained external capture bundle and local authentic
  Japanese CUE. Raw media and emulator dumps remain outside Git.

## 2026-10-28 — Render verified regional title-menu foreground sprites

- Added bounded title-menu rendering for English New Game/Continue and
  Japanese New Game/Continue/Configuration selections. Each layout is selected
  only for a matching
  full-size regional `TITLE.BIN` fingerprint; sprite spans, placements and
  palette offsets come from authentic Ymir captures and retail data.
- The Japanese Continue markers use the palette at `0x1b16`; Japanese New Game
  markers use `0x15d6`. Text palette bytes match seven identical retail copies.
  The renderer uses the captured 31/32 color calculation over the black
  back-screen. It does not draw the unbound background or full Configuration
  page.
- The source verifier matches all visible sprite texels against the retained
  authentic screenshots: English New Game 1,451/1,451, English Continue
  1,451/1,451, Japanese New Game 1,495/1,495, Japanese Continue 1,441/1,441,
  and Japanese Configuration 1,453/1,453.
  Screenshot-to-command-frame identity remains unproven.
- The four English/Japanese New Game and Continue checks and the separate
  Japanese Configuration capture verifier pass against the authentic CUE and
  retained Ymir screenshots. No game media or emulator dumps were added to Git.

## 2026-10-27 — Keep MAPD decoding separate from Saturn capture admission

- Corrected `nexus_title_load`: successfully decoding the five real
  `TITLE.BIN` MAPD planes into the bounded asset renderer no longer sets
  `startup_title_vdp_capture_verified`. That field remains fail-closed until
  a capture-backed Saturn display-consumer receipt admits VDP1/VDP2 ownership
  and composition.
- Rebuilt `firestaff` and ran
  `tests/test_nexus_v1_native_menu_mouse_title.sh` against the installed
  authentic Japanese Saturn media. Keyboard/mouse launcher selection reached
  the title route and correctly stopped with
  `title-vdp-owner-compositor-capture-required`; it did not claim a Nexus menu
  or playable runtime.
- The broader
  `tests/test_firestaff_cli_startup_diagnostics_real_media.sh` matrix also
  passed its installed-data checks: direct CLI media selection for all five
  games and Nexus's blocked-start assertion. Its DM2 Macintosh diagnostic was
  skipped because that authentic archive is not installed.

## 2026-10-27 — Document English-version Ymir menu reference

- Recorded the authenticated English-version no-save Ymir cold boot to the
  native three-choice menu alongside the Japanese retail checkpoints.
- A replay of the captured same-frame state reproduces the English screenshot
  pixel-for-pixel. The source join has 126 informative matches among 137
  textured draws; 11 zero-filled spans are excluded. This remains partial
  provenance and does not identify the visible menu asset/CLUT owner.
- Updated the README, status matrices, startup gap analysis, Nexus reference
  pages and wiki summaries. They distinguish the English-version disc image
  from the Japanese retail release and state that Nexus is not yet playable
  through Firestaff. Capture hashes and reproduction details remain in
  `docs/nexus/nexus_startup.md`; the raw capture stays outside Git.

## 2026-10-25 — Desktop input build and authentic menu-route verification

A clean Linux Release build of `firestaff` completed with the unified desktop
movement-pad routing and reverse leader selection. The focused desktop-input
and leader-switch regressions passed. The authentic Japanese Saturn pointer-only
menu regression also passed against that binary: it reached the title capture
gate and rejected a playable-runtime claim. Run this regression with Bash,
including from CTest, because its output checks use here-strings.

The same binary passed the authentic Theron JP raw-BIN startup regression,
including keyboard and mouse pose/tick checks. These results do not establish
Nexus menu/gameplay parity or full Theron parity.

## 2026-10-25 — Parse authentic Saturn Backup RAM containers in C

- Added a bounded, read-only Saturn BKR parser for 64-byte and 512-byte block
  geometries. It validates directory references and continuation blocks,
  rejects overlap/cycles, and copies game payload bytes without interpreting
  their contents.
- Added a real-data regression pinned to the SHA-256 of all four 32 KiB Nexus
  BKR images reported as retail-written. On TRV2, the C parser returned the expected
  `DMNEXUS__01` entry and 20,480-byte payload for each image; all payload
  SHA-256 values matched the existing Python parser byte-for-byte.
- The retail card images use 64-byte blocks, 512 blocks per image and 354
  allocated blocks per entry. This does not establish payload field meaning,
  BUP selection UI, or loading into the game engine. Continue remains
  incomplete. The retail provenance of the BUP used in the Ymir run was not
  independently established because its original transfer chain was not
  retained.

## 2026-10-29 — Bound the authentic Nexus Continue payload roster

- Added a C probe for the decompilation-bound `DMNEXUS__01` source-relative
  pointers, roster count, `0x11c` source table, `0xe0` stride, and
  count-dependent payload span. Champion record contents remain opaque.
- Extended the real-data regression across all four SHA-256-pinned retail BKR
  images and verified that mutations to the source pointer or roster count are
  rejected. The CTest uses the private authentic corpus and is labeled
  `real_data;no_synthetic`.
- The probe does not map these records into Firestaff's champion structure or
  implement Continue; the engine handoff remains open.

## 2026-10-29 — Probe one explicitly selected Saturn card file

- Added a path-based, read-only BKR file probe with a 4 MiB input bound. It
  requires exactly one `DMNEXUS__01` entry and the existing verified payload
  receipt, then returns only card and roster metadata; payload bytes are freed
  and no gameplay state is applied.
- Exercised the path probe against all four authentic, hash-pinned retail card
  images. It does not scan the game-data directory or decide which card the
  player selected.
- The launcher and Load-panel flow still need explicit card-selection state,
  full save mapping, and a verified engine handoff before Continue is playable.

## 2026-10-24 — Verified title and media diagnostics

## Available local retail media

2026-10-23 direct-start diagnostic recheck: the installed English Saturn CUE
and original media root selected the authenticated `nexus-saturn-en-fan-v2`
image, loaded `TITLE.CG`, `STABG.BIN`, `LOGOBG.DG2` and all 20 `FACE.BIN`
records, then completed the title animation. The startup receipt remains
`status=blocked`, `levelLoaded=0`, `menu=0`, with
`menu-bpk-prs3-capture-required`. The old `runtime-entered` phase label was
therefore false for this title-only state; the pending source change reports
`title-entered`, and the updated original-media CLI checks require the blocked
receipt and reject a runtime claim. This diagnostic correction does not make the
Nexus menu or playable level available. The 2026-10-25 Linux build and authentic
Japanese pointer-route regression above now verify the title-only boundary;
the earlier English diagnostic remains separate platform/media evidence.

The current `nexus_v1_bpk_archive_runtime_upload_plan` already decodes PRS3
payloads for bounded evidence, but explicitly emits no drawable pixels and
keeps each row blocked. A successful PRS3 decode alone does not establish
Saturn PALT/CRAM ownership, VDP1 upload framing or destination semantics. The
startup gate separately requires the menu text consumer and its VDP2 placement.
Next evidence must bind the post-title menu's input transition and these
source-owned consumers from the same authentic Saturn session; do not bypass
the gate by drawing decoded surfaces or substitute generated menu material.

The staged JP retail set supplies the CUE, nine BIN tracks, CDDA WAV tracks,
and the original ZIP package.  The disc itself does **not** contain a Saturn
VDP1/VDP2 VRAM dump, CRAM dump, register snapshot, or frame/timing capture.
Firestaff reads the CUE/ISO members directly in memory; it must not manufacture
either a capture or a presentation claim from those disc files.

The native title renderer consumes the retail `TITLE.BIN`/`TITLE.CG` MAPD
planes, selector sequence and BGR555 palettes directly from the selected CUE/
BIN media. `test_nexus_v1_title_mapd_real`, `test_m11_nexus_startup_gate` and
`test_m11_nexus_startup_runtime_handoff` cover this bounded path. The renderer
does not authorise the separate menu, face, HUD or dungeon compositors.

The French `TITLE.BIN` now has a separate exact SHA-256 corpus profile and a
real-CUE regression for its 60 RES* directory entries. The receipt preserves
observed IDs, offsets, record heads and chain bounds only; regional menu
rendering remains blocked.

Four authentic 32 KiB Saturn Backup RAM images with a `DMNEXUS__01` entry
are available in the private real-data corpus. The savegame editor now reads
their block chains and exposes each exact 20,480-byte payload as read-only
hex and can compare same-name entries from another image using payload-relative
byte offsets; it does not assign payload fields or allow rewriting. The BKR
self-test accepts only these four filename/SHA-256-pinned captures, while
`nexus_v1_saturn_bkr_real_data` validates their container structure and skips
when the private sample corpus is absent. Native Saturn save/load compatibility
remains blocked until the payload schema and retail consumer are source-bound.
Across the authentic two-, three-, and four-champion samples, the big-endian
value at payload offset `0x0a` is `0x0274 + champion_count * 0x00e0`, and the
payload tail from that value is zero in all four images. This narrows the
observed count-correlated boundary but does not establish record or field
semantics; trace the retail BUP save/load consumer before decoding or writing it.

An end-to-end check against the installed English Saturn CUE on 2026-09-27
confirms that `--game nexus --platform saturn` opens the original disc and
reaches `phase=nexus-title` with `titleReady=1` after 140 frames. Sending Enter
after 500 title frames still leaves the process at `nexus-title`, with no menu
or level loaded; the startup receipt reports `blocker=faces`. The separate
retail `FACE.BIN` decoder passes for all 20 authenticated portraits, so this
is not missing user media or failed portrait decoding. The remaining join is
PLRD roster row to FACE.BIN ordinal: `nexus_v1_champions.c` deliberately leaves
each `portrait_index` unknown (`-1`) because no Saturn consumer trace proves
the ordinal mapping. CLI launch to the title is therefore verified, but the
interactive Nexus start-menu handoff remains blocked until that real-media
mapping and the corresponding menu consumer are captured.

A same-revision, media-immutable title-session receipt now joins retail CD
FIFO records for LBA 6063--6089 to SH-2 RAM source writes for the same range,
cached SH-2 reads, and frame-stamped VDP2 writes/registers for frames
13294--13455. This closes title CD-to-RAM-to-VDP2 provenance only; it is not
menu, HUD, dungeon, audio or input evidence and does not widen the native
presentation boundary.


## 2026-09-29 — Keep unproven floor decorations out of the altar registry

- Removed the production conversion of every Structure1Fb floor-decoration
  record into a Vi altar. The DGN decoder continues to retain the authentic raw
  decoration records, but neither their model/aspect bytes nor the Saturn event
  consumer proves altar ownership.
- Extended the authentic LEV01–LEV15 production-load regression to require an
  empty altar registry after each level load. This is a fail-closed semantic
  boundary, not proof that the retail levels contain no altars.
- Built the real-data regression on TRV2 and passed it in three consecutive
  serial loops against `/home/trv2/.firestaff/data/nexus`.

## 2026-09-29 — Verify the production floor-item handoff across playable levels

- Extended the real-media boot regression to check every playable retail level
  LEV01–LEV15 after loading it through the production engine. Each Structure1Fa
  item must match the exact floor-registry position, declaration ID, quantity,
  raw attributes, and source-entry index; extra floor items fail the check.
- Extended the production semantic-boundary regression to exercise the real
  mechanics tick with an ISO source identity and a queued forward command. The
  tick must leave party pose, tick count, and queued command unchanged while the
  Saturn action owner remains unbound.
- Configured and built both test targets on TRV2 against
  `/home/trv2/.firestaff/data/nexus`. Both CTests passed in three consecutive
  serial loops; the all-level path used the hash-admitted retail DGN files and
  source-bound ITEM.IBS bank. This verifies source handoff and the fail-closed
  production boundary, not item action semantics or full Nexus playability.

## 2026-09-29 — Keep unproven Structure1B door candidates out of runtime

- The retail DGN parser may retain type-8 Structure1B cells as geometry
  candidates, but no authenticated Structure1E-to-cell join or Saturn state
  consumer is established. The production mechanics loader now resets the
  legacy door registry without populating it from those candidates.
- The hash-bound real-media boot test confirmed LEV01 contains type-8
  candidates, then loaded every playable retail level LEV01–LEV15 twice and
  verified the runtime door registry stayed empty. The separate 16-level
  Structure1E corpus test also passed twice against TRV2's authentic files.
  This preserves the source boundary; it does not claim that retail doors are
  implemented.

## 2026-09-29 — French retail title corpus receipt

- Pinned the authentic French `TITLE.BIN` SHA-256
  `3bd33594cb952ea9be9e398b85bb8c9a112483088eb6c93cb628bba14c57262d`,
  matched to the already recorded French MD5 identity. The streamed French
  CUE member is 112,216 bytes and has the same 60-entry RES* table, class
  counts, IDs, offsets, record-head tags, and contiguous source-tail chain as
  the documented Japanese/English profile. Added a real-media CTest for the
  French CUE and a raw-file fallback SHA gate. This is structural admission
  only; it proves no image semantics or regional presentation.

## 2026-09-29 — Pin the authentic BKR payload boundary observation

- Extended the hash-pinned real-corpus self-test to verify that the value at
  payload offset `0x0a` follows `0x0274 + 0x00e0 * champion_count` for the
  authentic two-, three-, and four-champion images, and that the payload tail
  after that value is zero in all four samples.
- Re-ran the updated self-test on TRV2 against all four authentic 32 KiB BKR
  images. Their pinned identities, `DMNEXUS__01` entries, and 20,480-byte
  payloads passed. This records a count-correlated boundary only; no record
  size, field semantics, or Saturn save import is inferred or enabled.

## 2026-09-28 — Verify English and French retail PLRD rows

- Extended the regional CUE real-media regression to compare all 20 parsed
  PLRD rows against each authenticated member's own bytes: current/max
  statistics, raw class levels and portrait type, six TABL indices and codes,
  ten row-local equipment words, the unbound eleventh slot, and empty
  inventory. It also asserts that unsupported labels/provisions/portrait
  identity remain unset. The check verifies byte preservation, not gameplay
  meanings for regional values.
- The English retail `RLOWFIX.BIN` (MD5
  `14c3a7e6fed2dc9e53a727640d4c9348`) and French retail `RLOWFIX.BIN` (MD5
  `ecbecff383d6ee8330e68e38417be9c8`) both passed all 20 rows from their
  original CUEs. `nexus_v1_title_res` passed with `fail=0`; no synthetic game
  data or emulation was used.

## 2026-09-28 — Round-trip authentic ITEM.IBS floor images

- Extended the ITEM.IBS real-media test to render every decoded floor image
  with an admitted pixel hash and compare the materialized RGBA pixels against
  the source decoder's hash, including all 109 descriptors in the installed
  retail bank. The authentic `ITEM.IBS` SHA-256 on TRV2 is
  `fc32ca5875906e6e0dc69e0b5edfa5d00cb1f4401b7d497397c699be7c4530c1`.
- The targeted ITEM.IBS CTest passed on TRV2 and reported 109/109 source
  floor images round-tripped. This validates source decoding/materialization
  only; it does not infer item semantics or open the Saturn item-use route.

## 2026-09-28 — Bound PLRD equipment reads to authentic rows

- Corrected the RLOWFIX champion parser so equipment reads stay within the
  40-byte tail of each 64-byte PLRD row. The unsupported eleventh slot and
  backpack offsets remain empty instead of inheriting bytes from the next row
  or `CRET` resource.
- Added a regression against the authenticated Japanese RLOWFIX SHA-256 that
  compares all 20 rows' ten retained equipment words against their own source
  bytes and checks the unsupported eleventh slot and inventory remain empty.
  The test passed against the real Japanese RLOWFIX corpus on TRV2. A
  temporary out-of-row mutation made it fail, and after restoring the exact
  test source hash it passed again. The authentic bytes place the final PLRD
  row directly before `CRET`; no BIOS or emulator was needed for this source
  boundary check.

## 2026-09-28 — Japanese retail title corpus gate

- Added a dedicated CTest entry for the authentic Japanese `RLOWFIX.BIN` in
  the retail CUE. It reports CTest skip (return code 77) when the Japanese
  CUE is absent, and fails on identity, engine-open, or RES decode errors;
  English/French media cannot accidentally satisfy this Japanese gate.
- Built and ran `nexus_v1_title_res_real_japan` on TRV2 against the installed
  Japanese retail CUE. The engine opened its 137-file disc and verified
  `RLOWFIX.BIN` MD5 `bb650a4e6f7b6374ba8aa86a61f8f523`; all 14 RES entries
  decoded and the authentic champion roster seeded. CTest passed. No game
  data was copied into the checkout or Git.

## 2026-09-28 — Authentic Saturn card metadata inspection

- Extended the read-only BKR overview to show the raw language and timestamp
  bytes plus every allocated block ID. The authentic-corpus self-test checks
  those displayed values against the original header bytes, validates unique
  in-range block IDs, and keeps the payload read-only and uninterpreted.
- On TRV2, rebuilt the editor script in an isolated temporary directory and
  ran `--self-test-bkr` against authentic two- and three-champion retail-written
  BKR images. Both images are 32 KiB, each contains the 20,480-byte
  `DMNEXUS__01` payload using 354 blocks, and both passed. Their SHA-256 values
  are `3d60856119df55ffa4937d13b5dc7487157c7c6b131a999a2e61683316679634` and
  `d9c86de3b0c668f9961a16529b1d5fd72f3e095823f8f0021cf8db8bfba48e9b`. The
  authentic images remain in TRV2 capture storage; no card or game data was
  copied into Git. This does not decode Saturn save fields or enable import.

## 2026-09-28 — Regional real-media test discovery

- The regional title/RES tests now share the normal media-root lookup and fall
  back to `~/.firestaff/data/nexus` when `FIRESTAFF_NEXUS_DATA_DIR` is unset.
  On TRV2, the title/RES test was rebuilt and run with that variable explicitly
  unset; it opened the authentic Japanese CUE, verified the pinned
  `RLOWFIX.BIN` identity and real PLRD rows, and reported absent English and
  French CUEs as skips. No synthetic media was used.

## 2026-09-28 — Absolute-frame-bounded VDP1 trace generation

- Added inclusive absolute-runtime-frame filters for the external Mednafen
  VDP1 write, writer-code, writer-register and snapshot diagnostics. The raw
  capture's VDP1 markers remain relative to its selected window; the launcher
  passes and records the absolute bounds. A source-copy build on TRV2 compiled
  successfully, and the launcher regression passes with an explicitly bounded
  interval. This is capture tooling only; it proves no Nexus asset owner,
  consumer, or presentation behavior.

## 2026-09-28 — Bounded VDP1 pre-capture writer corridor

- The hash-bound 701-frame JP capture beginning at runtime frame 13197 has a
  VDP1 write trace capped at ten million records before captured frame 0.
  In that distinct pre-capture prefix, the exact starts `0x58b58` and
  `0x58c58` are each written once by master-SH-2 PC `0x0601307c`; later rows
  at the same addresses come from PC `0x06026260`. The trace is bound to raw
  SHA-256 `43b8979b79fb69ebe2bad08ae1090e42b784f0a3b700fbdf2bb99a6af130b80c`
  and trace SHA-256
  `2d169758e5fad0ca1783e68224f56a0a2aa18d92c861a8a0aa17da6bd50ce73f`.
  This is a writer-PC observation only; the prefix cannot be joined to the
  701 captured frames, and it does not prove an asset or display consumer.
- The same-session code receipt at PC `0x0601307c` is manifest-hash-bound, but
  its 96-byte instruction window has no exact match in either byte order in
  the 156 authentic files under `.firestaff/data/nexus`. The supplied
  `DM.BIN` is SHA-256
  `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`;
  the code window also does not match that file at the documented
  `0x06010040` static code mapping. A later raw WorkRAMH snapshot confirms
  DM.BIN image bytes at `0x06010000`, so neither value is a universal runtime
  mapping for every capture. Do not infer a `DM.BIN` routine or source owner
  from the runtime PC alone. Traces and all game data remain on TRV2.

## 2026-09-28 — Fresh JP VDP1 window did not reproduce the retained sources

- Captured a no-input, hash-bound JP retail window on TRV2 with
  `skip_frames=13197` and 701 frames. The raw SHA-256 is
  `43b8979b79fb69ebe2bad08ae1090e42b784f0a3b700fbdf2bb99a6af130b80c`; the
  VDP1 write-trace SHA-256 is
  `2d169758e5fad0ca1783e68224f56a0a2aa18d92c861a8a0aa17da6bd50ce73f`.
  Scanning every captured frame found neither recurring source offset
  `0x58b58` nor `0x58c58`; frame 700 instead has one 1,008-byte mode-0 source
  at `0x415a0`. The ten-million-record VDP1 trace budget was reached before
  frame 0, so its writes are a pre-capture prefix and cannot be joined to
  this frame window. This fresh cold-start capture does not reproduce or
  replace the earlier retained window and does not identify a retail owner.
  Keep the existing source-ownership and presentation blockers open. The raw
  capture, traces, BIOS, disc and private game data remain on TRV2.

## 2026-09-28 — Regional MENU.BPK source-scan identity

- Corrected the VDP1 retail source-join scanner to accept the independently
  verified Japanese, English and French `MENU.BPK` SHA-256 identities. This
  prevents the Japanese retail member from being rejected merely because the
  scanner previously preferred the English alternate hash. Added a regression
  test and a dedicated CI check; unknown filenames still grant no identity.
  This improves source investigation only and does not identify either
  recurring VDP1 span, authorize a menu compositor or widen runtime output.
- The analyzer now authenticates and prepares retail members once per run,
  rather than rereading and word-swapping the full corpus for every draw. On
  TRV2, the 701-frame authentic Japanese capture completed its frame-700 join
  against 137 hash-verified retail members with zero rejected files. The
  recurring `0x58c58` and `0x58b58` source spans still have no exact or
  word-swapped retail join; other DGN surfaces did join. Menu ownership and
  semantic admission therefore remain blocked. The capture and game data stay
  on TRV2.
- The source-join summary now reports per-draw coverage instead of labeling
  the whole frame `verified` after any single hit. Re-running the authentic
  frame-700 Japanese capture reports `source_joined_draws=225/227` and
  `source_join=partial`; the two recurring spans remain the unmatched draws.

## 2026-09-28 — VDP1 write-trace pre-capture prefix

- Both VDP1 write-trace analyzers now accept valid V2 records emitted before
  the first frame marker and keep them separate from frame-bounded writes. The
  source join reports pre-capture coverage independently; the summary tool
  selects it only with `--pre-capture`. Parser tests cover the observed prefix
  format and retain V1 behavior. On the hash-bound TRV2 Start/input and control
  captures at frame 10500, both had 20,000 pre-capture rows, no writes in the
  selected frame, and the same mode-5 source span at `0x63e00..0x6c000`; the
  prefix did not cover that span. This is transport evidence only and neither
  identifies the menu asset nor admits presentation. The authentic raw dumps
  and traces remain on TRV2.

## 2026-09-28 — Retail MAPD CD-to-RAM capture integration

- Captured a fresh, hash-bound 162-frame JP retail receipt on TRV2. The real
  verifier accepts all 27,441 CD FIFO words against Track 1 and 13,312
  contiguous loader writes from LBAs 6063--6088 to WorkRAMH, locating the
  authenticated MAPD record and palette. Updated the real-media CTest to
  discover both flat legacy traces and current `traces/`-nested receipts;
  the integrated test passes on the captured retail files. This proves only
  CD-to-RAM transport; RAM-to-VDP2 ownership and semantic admission remain
  blocked. No game media or captures were added to Git.

## 2026-09-28 — Paired Start table-reader observation

- A same-scope cold-start control/Start pair on TRV2 captured frames
  10500--10507 with identical JP BIOS, retail CUE, raw VDP bytes and rendered
  frame 10507. The broadened SH-2 trace reached the same table read at
  `0x0602c940` / PC `0x0601462c`; `R4` was `0` in control and `0x10` in the
  one-frame Start-input run. The analyzer now reports this register state
  rather than assuming a button value as a chain requirement. It labels the
  chain observed, not semantically verified; no rendered/menu transition was
  observed and semantic admission remains blocked. Artifacts remain on TRV2.

## 2026-09-28 — SH-2 RAM read receipt V2 analysis support

- The fail-closed controller-buffer receipt analyzer accepts both V1 and the
  current V2 register-owner format, rejects slave-SH-2 register ownership, and
  can select one frame from a bounded multi-frame trace. Parser regression
  cases cover V1 compatibility, V2 selection and owner rejection. The actual
  TRV2 Start-only V2 receipts parse at frame 10507; their bounded address
  filter omits downstream linked-buffer/table reads, so full consumer-chain
  verification and all menu/action semantics remain blocked.

## 2026-09-03 — Native CUE-media regression audit

- Re-ran the native production boundaries for combat, magic, light, rest,
  experience and world actions, plus CUE data-track and CDDA binding. The
  retail CUE route and native title mouse route passed without BIOS or
  emulator runtime dependencies. ISO-only probes remained skip-safe because
  the staged retail set is CUE/BIN/WAV media, not a standalone ISO.

Reviewed 2026-08-26. Completed work only.

- Native CUE-media corpus checks read the supplied Saturn title resources,
  SAL, MAP, SLEV and raw data without a runtime emulator dependency.
- The full `LEV00.DGN`–`LEV15.DGN` Structure3 mesh corpus is read directly
  from the supplied retail Track-1 CUE/BIN into RAM and checked against its
  canonical per-level MD5 before parsing.  This is source verification only:
  it does not authorize an unbound Saturn mesh renderer.
- The native sound owner now binds retail CUE-declared audio Tracks 02–09 to
  their original BIN payloads and rejects host WAV/OGG/MP3/FLAC substitutes.
  This is source selection only; playback remains closed without a verified
  Saturn decoder and dispatcher.
- An authenticated Saturn capture verifies NBG1 hardware state: enabled
  bitmap mode, 256-colour code, BMPNA palette bank 0 and scroll origin
  `(0,0)`. It does not identify the bitmap/CLUT source or authorize drawing.
- The same authentic frame's full 512×256 indexed NBG1 span and 256-entry
  CRAM decode in native code using the capture's recorded Saturn byte order.
  This is capture-only evidence: it does not identify an asset owner or
  authorize production presentation.
- The real Japanese CUE independently proves Track-1 `STABG.BIN` reaches the
  native STMP/DMWeb first-map consumer (320×168 with retained source palette);
  it remains `no_draw` without an exact VDP source join.
- Development-only VDP tracing has deterministic emulation-frame filters and
  retains fail-closed source correlation for unbound rendering writes.
- A same-session Japanese retail receipt verifies one SH-2 RAM-to-VDP1 copy:
  PC `0x060135e8` transfers 2 KiB from `0x06027874..0x06028074` to
  VDP1 `0x10a00..0x11200` with Saturn word byte order. It is a completed
  transport observation, not an asset, palette, command or title-rendering
  admission.
- A post-intro Japanese retail title receipt binds the full `TITLE.CG` payload
  to word-swapped VDP2 VRAM `0x24020` and the MAPD palette to word-swapped
  CRAM `0x400`. The raw MAPD planes, tilemap transform, layer placement and
  timing remain unbound, so this does not authorize title rendering.
- The post-intro input receipt is completed as a negative observation: title
  frames 13000–13039 are bit-identical with and without verified Start/A
  pulses, so that window is not an interactive start-menu transition.
- The Nexus gameplay Tab action cycles the loaded party's leader slot. An
  authentic Ymir `--debug` run on Japanese retail media visibly confirmed L/R
  changes the leader marker and portrait; the guest RAM owner remains unknown.
  See `docs/nexus/nexus_input.md` and
  `parity-evidence/verification/nexus-leader-cycle-debug-20261025/`.
- The hash-pinned Japanese retail `DM.BIN` event-44 route now has checked
  control flow through its context resolver and slot-pair read/clear/write
  helpers. It uses a `0x13c` context stride, a `+0x110` slot-pair array with
  two 16-bit words per slot, and 40-byte item metadata records; slot 7 takes a
  separate path. This does not bind its context index or opaque auxiliary word
  to Firestaff's champion/inventory model. The reproducible checker is
  `scripts/analyze_nexus_mouse_event_crossrefs.py`.

## 2026-10-28 — Japanese retail Configuration title selection

- Firestaff now renders the selected Japanese Configuration title row from
  authentic regional `TITLE.BIN` sprite and palette spans. The mirrored right
  marker follows the captured VDP1 horizontal-flip flag.
- Mouse selection accepts the Japanese Configuration row when the authenticated
  Japanese retail title source is active.
- The real Ymir capture verifier binds five VDP1 commands to retail texture and
  palette bytes and checks all 1,453 visible screenshot texels. The production
  C renderer's output was independently compared against the same authentic
  capture and matched all 1,453 texels.
- English Configuration support, full title background and composition,
  destination execution, Continue/save loading, and gameplay/HUD parity remain
  open. See `docs/nexus/nexus_startup.md` and
  `docs/nexus/nexus_title.md` for the evidence boundary.

## 2026-10-28 — English retail Configuration title selection

- A Ymir `--debug` menu-state replay on authentic English retail media selected
  Configuration with one scheduled Control Pad 1 Up pulse.
- Firestaff renders the five English Configuration-selected foreground
  sprites from hash-verified `TITLE.BIN` texture and palette bytes, including
  the flipped right marker. Mouse selection is enabled for this authenticated
  English region.
- The capture verifier binds all five VDP1 commands, source spans, CRAM
  palettes, placement, priority, color-calculation ratio and 1,069 screenshot
  text pixels. Marker screenshot RGB depends on the still-unbound NBG0
  background.
- The production C renderer's framebuffer index map matches all 1,459 source
  texels from the captured commands with zero differing pixels.
- Full title background/composition, Configuration page controls, destination
  execution, Continue/save loading, and gameplay/HUD parity remain open. See
  `docs/nexus/nexus_startup.md` and `docs/nexus/nexus_title.md` for the evidence
  boundary.

## 2026-10-28 — English retail Configuration page foreground

- The English-region title runtime now opens and renders the authentic retail Configuration page foreground, including value sprites and the animated focus marker from hash-verified `TITLE.BIN`.
- Ymir 0.4.2-dev `--debug` captures establish defaults (Music ON, Sound STEREO, Game Level NORMAL, Monster Gauge ON), keyboard row/value navigation, and Exit/B return to the title menu. Mouse row/value routing is covered by the runtime tests.
- The production C renderer matches the captured 320x224 indexed foreground and all 256 palette entries for seven real capture states. Reproduce with `scripts/verify_nexus_english_configuration_page.py` and the captured evidence directory documented in `docs/nexus/nexus_startup.md`.
- The 3D/DGN page background, cold-boot setting persistence, audio/difficulty/HUD effects, Japanese Configuration page, and New Game/Continue destination execution remain open. No synthetic game art was introduced.
