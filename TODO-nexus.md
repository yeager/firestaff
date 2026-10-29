# Firestaff TODO — Nexus

Reviewed 2026-10-29. The items below track open work and include completed
diagnostics as context. Verified evidence is also recorded in the Nexus capture
and reverse-engineering records and in [DONE-nexus.md](DONE-nexus.md).

The Japanese and English retail Configuration foregrounds now render from
hash-verified `TITLE.BIN` files and share byte-identical UI text/palette spans.
Japanese-region keyboard/mouse state routing is covered by the M11 runtime
test. The retained Japanese Ymir screenshot captures only the initial settings
page; Japanese state changes and marker animation still need authentic captures.

The C Saturn BKR container parser is verified against all four hash-pinned
samples reported as retail-written; see DONE-nexus.md. A bounded C probe now
validates the retail `DMNEXUS__01` payload offsets, champion count, party
leader index, current/maximum health, current/maximum stamina, and current/
maximum mana against decompiled retail code and all four authentic images.
The health pair at source `+0x38/+0x3a` loads as `47/47`; New Game initializes
the same runtime cells from PLRD health. Its post-load update behavior remains
untraced. The mana pair at source `+0x40/+0x42` loads as `18/20` and
updates to `19/20`; New Game initializes the same runtime cells from PLRD
mana. Other champion-record fields remain opaque.
Retail Continue's BKR named-entry lookup,
complete payload transfer,
and checksum gate are source-bound. The remaining work is to identify the
rest of the champion field schema and the post-load consumer that applies the
loaded state.
Keep unbound payload fields opaque; do not infer game state from FNXS or another
Dungeon Master version.

The bounded BKR parser remains outside the production Continue route. M11's
retail title reducer still receives `has_continue = 0`, and
`SHOW_LOAD_PANEL` currently redraws without entering a Load-panel state. The
next integration step is explicit real-card selection and a separate BKR
catalog/load state; do not infer the active card by scanning unrelated
installation data. Keep final load confirmation gated until the remaining
champion fields and retail state consumer are mapped. A hash-verified callback
decompilation and Ymir trace now bind the authentic BKR comment suffix `N` to
the visible `NORMAL` difficulty label. The selected comment prefix is also
copied into the callback's text buffer, and enlarged inspection of the
authentic capture confirms the full row `B01  0:06 NORMAL`. The retail row text
path is mapped from the selected BKR comment through glyph conversion and
drawing. The Load-panel capture still lacks complete NBG0/NBG1 composition and
text/border source joins; see
`docs/nexus/nexus_startup.md` and the saved Ymir `--debug` captures.
The callback's EASY-suffix branch is statically identified but has not been
verified with an authentic EASY BKR card.

The 2026-10-29 authentic Load-panel replay and callback decompilation confirm
the selected card's `B01  0:06N` comment is copied, its `N` suffix selects the
visible `NORMAL` label, and the assembled string is glyph-rendered. The
authentic screenshot shows `B01  0:06 NORMAL`. See the trace and capture
bundle on the external evidence disk and
`docs/nexus/nexus_sh2_startup_decompilation.md`.

A separate Ymir cold boot tested the retail Start control in the New Game
view. Start displayed `PAUSE`; a second Start returned to the byte-identical
unpaused view. The interval produced zero SH-2 writes to the BUP bus and the
fresh internal BUP still contains no entries. Save creation and the in-game
save command remain unresolved; see the authentic input/BUP trace receipt in
the external evidence folder.

A 2026-10-29 Ymir cold boot with the authentic two-champion BKR installed in
internal backup RAM reached the native Load panel, displayed `B01  0:06 NORMAL`,
and accepted the save to reach a dungeon view with two champion HUD panels. The
BUP image retained its source hash. This verifies the retail reference flow
but does not identify payload fields or implement Firestaff Continue. See the
cold-boot receipt in
[`nexus_startup.md`](docs/nexus/nexus_startup.md#two-champion-bkr-cold-boot-2026-10-29).

The 2026-10-29 Continue trace now binds all 20,480 reads at SH-2 PC
`0x06003680` to the exact authentic `DMNEXUS__01` BKR payload address
sequence. Ghidra also binds the caller to the named-entry lookup, WRAM-L
destination `0x00204000`, and checksum gate. This establishes the retail
payload transfer, but not its field schema or the later mapping into
Firestaff's game model. A bounded follow-up write trace identifies the writer
of the four differing bytes: the copy routine writes `00 00 00 cc` at payload
offset `0x58`, then PC `0x0602ec94` replaces it with stack-derived
`05 ef bf 3c` nine frames later. Its meaning and whether this is a required
runtime relocation remain unresolved. The containing function resolves the
fields at structure offsets `+4` and `+8` as base-relative source addresses;
the four authentic payloads contain `0x10` and `0xcc`, pointing from structure
offset `0x50` to offsets `0x60` and `0x11c`. pcrecomp/Ghidra identifies the
containing function as `FUN_0602eb4c`; its code bytes and the named-entry copy
function match the same-run Ymir capture. Identify the substructure schema and
the transformed field's consumer before implementing payload-to-engine state.
See
[`nexus_sh2_startup_decompilation.md`](docs/nexus/nexus_sh2_startup_decompilation.md#continue-payload-transfer-2026-10-29).

The source-record stride (`0xe0`) and runtime-record stride (`0x13c`) are now
verified from the retail code constants. A reproducible analyzer restores the
traced four-byte fixup for payload hash verification and checks 83
decompilation-specified operations per record across authentic two-, three-,
and four-champion captures (166, 249, and 332 checks). The expanded leader-3
Ymir trace now explains all 16 final-value differences across four tracked
fields:
source `+0x3c` / runtime `+0x08`, source `+0x40` / runtime `+0x10`, source
`+0x44` / runtime `+0x18`, and source `+0x46` / runtime `+0x1a`, across all
four records. These offsets remain mechanically verified and unnamed; the
rest of the record, field semantics, source-owned post-load consumer, and
Firestaff runtime integration remain open.
Reproduction and retained evidence are documented in
[`nexus_sh2_startup_decompilation.md`](docs/nexus/nexus_sh2_startup_decompilation.md#continue-payload-transfer-2026-10-29).

The four authentic BKR samples all contain `a3 88 96 80 80 00` at the start
of every champion record. In the authentic Japanese Track 1 `RLOWFIX.BIN`,
the prefix `a3 88 96` uniquely matches the first three PLRD name-table indices
of roster row 2; its TABL entries resolve to raw codes `0x88`, `0x65`, and
`0x6d`. The retail loader copies all six source bytes to runtime offset
`+0x2c`. This binds a real name prefix to a source roster row but leaves the
two `0x80` bytes and the text consumer unresolved. All current save samples
use this same prefix, so they do not establish how other roster names are
stored. Decompilation now identifies runtime `+0x2c` as the champion-name
string slot: the New Game initializer copies a NUL-terminated roster name to
it, and the BKR importer copies the six saved bytes to the same slot. The
sample's final `00` is its terminator, but the `0x80` suffix and glyph mapping
remain unresolved. Trace the runtime string consumer and capture a retail save
with a different roster member before decoding the remaining bytes or loading
BKR names into Firestaff's `name_ascii` or `name_jp` fields. Details are in
[`nexus_save_format.md`](docs/nexus/nexus_save_format.md#name-byte-comparison-against-the-japanese-roster).

The 2026-10-29 Load-panel VDP2 trace now joins the retail SH-2 writer at
`0x06023640` to all 352 VDP2 halfword events on frame 659. The selected NBG1
name table maps 176 writes to visible cells x=1..16, y=6..16; each PND is
`0x2000/0x1100`, and the referenced 32-byte character pattern is zero-filled
in that capture. This establishes NBG1 name-table writes, not rendered menu
content or source-asset ownership. Identify the active NBG1 consumer and bind
the displayed panel and text to authentic source data before admitting a
Firestaff Load-panel renderer. See the Load-panel writer evidence in
[`nexus_startup.md`](docs/nexus/nexus_startup.md#load-panel-vdp2-longword-writer-join-2026-10-29).
An exact-source scan of this frame's NBG0 bitmap found no complete match among
410 decoded authentic menu, font, title, and static-background candidates. A
separate frame-bounded join now binds 57 nonzero pixels from authentic Japanese
`MENU.BPK` surface 55 to both the SH-2 writer and VDP2 bus trace at NBG0 offset
`0xe6f`; Ghidra identifies the caller as the retail menu-image renderer. This
is a partial pixel-index transport join. A same-state screenshot comparison
now verifies direct CRAM output for all 42 visible nonzero NBG0 indices, but
an additional row-wise trace join binds unique `MENU.BPK[60]` (228x100) to
22,800 frame-bounded SH-2 writes at NBG0 `(16,112)`. The retail resource that
supplied 41 of the 42 visible palette values is the same-index `MENU.BPK` PALT
word; the remaining index `0x83` is overwritten by a frame-traced retail
animation. Direct bulk palette transfer, the remaining bitmap producer, and
complete screen composition remain unbound. No exact MENU/STABG or DGN
Structure2 palette asset match was found. Identify the full NBG0 producer,
palette resource owner, and remaining composition before admitting a Firestaff
Load-panel renderer. See the frame-bounded evidence and reproduction command in
[`nexus_startup.md`](docs/nexus/nexus_startup.md#frame-bounded-menu-bpk-to-nbg0-source-span-2026-10-29).

2026-10-28 retail New Game loader route: the M11 retail menu now sends
keyboard/controller Accept and the retail mouse row to the same source-gated
New Game path. Against authentic Japanese retail media, the regression
authenticates `DM.BIN`, parses the level-1 start record, loads hash-verified
`LEV01.DGN`, validates `(29,47,North)`, and records the engine pose for both
input routes. The viewport/HUD receipts still block the runtime handoff and
restore the title; there is no paired Ymir capture of the Firestaff handoff.
This closes the retail start-record/loader route, not the playable-start gap.

An authentic Japanese Configuration-page capture now yields a Ymir VDP1 trace
prefix with all 121 informative draws joined to canonical DGN material bytes,
including indexed CLUTs and direct-colour surfaces. The other 11 draws are
transparent all-zero textures. This is not enough to render the scene: 16
matched draws lack candidate face-owner rows, and multiple levels/faces share
materials. Active level, selected faces, camera transform, culling, screen
origin, VDP2 composition, and Japanese settings interaction states still need
authentic joins. See the [DGN trace-prefix receipt](parity-evidence/verification/nexus_japanese_configuration_dgn_trace_prefix_20261028/README.md).
The 16 draws without Structure3 owners use five 48x48 `LEV00.DGN` materials
and form a receding tiled plane at the bottom of the scene, consistent with a
floor or menu backdrop. Identify the active level/map selector and prove this
geometry's owner before adding a runtime compositor.

## Current evidence boundary

The current workstation's authentic `~/.firestaff/data/nexus/MENU.BPK` has
SHA-256 `f2f78dddfe37a5ff414775ae888f164624e987059934b034ba36299cc769d2ca`,
the authenticated English-version image admitted by
`scripts/analyze_nexus_vdp1_source_join.py`. The deterministic Ymir title-menu
capture below uses the Japanese disc and Japanese BIOS. Its VDP1-to-resource
join therefore cannot establish source ownership for the installed English
menu archive. A separate controlled 2026-10-27 Ymir cold boot used the
authenticated English-version disc image and USA 1.01a BIOS and reached the
native three-choice title menu. Its frame-13106 VDP1 trace is complete, with 178
command rows and no drops. The source join finds informative source
bytes for 126 of 137 textured draws; the other 11 spans are zero padding and
are excluded. This remains partial source evidence, not complete surface,
palette, ordering, or consumer ownership.

Replaying the English screenshot from the captured same-frame state reproduces
every RGB pixel exactly. In that English capture, 86 of 162 PRS3 `MENU.BPK`
surfaces have direct byte matches in VDP1 VRAM, but none of 137 textured command source starts falls
within those ranges. The menu screenshot confirms that Ymir displays the
English-version title menu. A matched retail-menu capture now binds the five
visible English New Game and Continue VDP1 sprites to `TITLE.BIN` offsets,
captured CRAM palette bytes present in authentic `TITLE.BIN` ranges, screen
placement, and all 1,451 visible pixels in each state. The icon palette has one
source match; seven identical copies of the text palette remain candidates.
Firestaff now draws those five authenticated English foreground sprites for
New Game and Continue from the exact hash-pinned `TITLE.BIN`, plus the five
authenticated Japanese New Game and Continue sprites and their source-bound
regional palettes. A Japanese Configuration-selected capture now binds five
foreground sprites, their palettes, positions and 1,453 visible screenshot
pixels to authentic `TITLE.BIN`. Firestaff renders this Japanese selection and
its startup mouse can select that row. An English Configuration-selected Ymir
capture now binds its five foreground commands, regional palette, placement,
and horizontal flip to authentic English `TITLE.BIN`; Firestaff renders the
same row and accepts its mouse selection. The three English text sprites match
1,069 screenshot pixels, while the markers' final RGB depends on the unbound
NBG0 background. Neither regional result binds the shared background or
implements destination execution; full menu composition remains open.
Destination activation and save loading remain open.
This does not bind the NBG0 background or remaining screen composition;
the exact background source and complete display consumer are still unknown.
The English New Game and Continue captures share byte-identical VDP2 VRAM,
CRAM and registers, so their observed selection difference is confined to
VDP1 state; this still leaves the shared NBG0 retail source unbound.
The Japanese New Game and Continue captures also have identical NBG0 VRAM
(SHA-256 `0a3b8b5ba0bd9e9c4b549d136961b44439d2534cbb7060c5bcbdca6da9c53d08`);
scanning all authentic Japanese disc members finds no complete decoded source
match. A complete frame-tagged VDP2 trace now proves that a 921-byte raw span
from `TITL` record 2 in Japanese `TITLE.BIN` (`0xb438 + 0x46a`) is written
into NBG0: frame 6,869 clears the span, then frame 7,729 writes 608 byte lanes
that, with the intervening
captured writes, reconstruct the authentic source bytes exactly. This joins a
partial source span, not the full NBG0 bitmap or its display meaning.
An exact but sparse 1,603-byte `TITLE.BIN`/NBG0/WRAM-high overlap is a trace
target only; without frame-bound VDP2 writes it does not identify a producer.
The Ymir log records `0DMSTRT.BIN` and a CD Block soft reset,
but no `DM.BIN` read event. Separately, an exact 230,669-byte prefix of the
authentic 555,144-byte English `DM.BIN` is present at offset `0x10000` in the
WRAM snapshot. This proves prefix residency, not that the whole file was
loaded or how the logged read path works. The English capture establishes the
English-version emulator cold-boot-to-menu route. Firestaff now renders the
authenticated English New Game/Continue foreground sprites and Japanese New
Game/Continue/Configuration sprites. This does not establish full menu
composition, Continue/save loading, or playable-level parity. Keep the shared
background and remaining regional renderer gates closed. Capture hashes,
output, and reproduction commands are in
[`docs/nexus/nexus_startup.md`](docs/nexus/nexus_startup.md#english-cold-boot-reaches-the-native-title-menu-2026-10-27);
raw capture files remain on the external evidence volume.

2026-10-27 TRV2 Firestaff v3.0.415 direct CLI probe against the authentic
Japanese Saturn corpus selected `DM.BIN` (MD5
`e88d60859f65f08fa622e1992b02280f`). After 110 boot-probe frames and one
scripted Enter, the receipt reported `titleReady=1`, `levelLoaded=0`,
`menu=0`, and `menu-bpk-prs3-capture-required`. The real-data
`test_nexus_v1_native_menu_mouse_title.sh` also passed through the visible M12
mouse flow and stopped at the same title gate. These SDL dummy runs prove
authentic data selection and title handoff only; they do not prove an in-game
Nexus menu, save load, or playable level.

2026-10-28 current-working-tree direct CLI probe against the authentic
Japanese retail `DM.BIN`: `--game nexus --platform saturn --data-dir
~/.firestaff/data/nexus/DM.BIN --boot-probe --boot-probe-frames 0` selected
edition `nexus-saturn-jp` with asset MD5
`e88d60859f65f08fa622e1992b02280f`. The receipt remains
`status=blocked title=1 levelLoaded=0 menu=0` with blocker
`title-vdp-owner-compositor-capture-required`; the frame is `nexus-title` at
48/102. This confirms the current authentic-media CLI source selection and
pinpoints the still-unresolved Firestaff title/menu composition gate. It is
not a playable-level start.

An older 600-frame Mednafen capture on the authentic English merged disc
(`run-authentic-merged-menu-20260811`, disc SHA-256
`5ab2fbd572d454e4a9df024e2bb6b3cd30f84018c3e650bd945bd52742edd83d`) does
not close that gap. The source-join analyzer found one VDP1 direct-colour draw
and no exact match against the 126 hash-verified retail files at each sampled
frame 299, 300, 301, 320, 360, 400, 500 and 599; the source remained unbound.
The separate authentic English-disc capture
(`run-authentic-english-menu-20260811`) records manifest disc SHA-256
`f3575af985cadbecc74edda0c51451ffeea775054ec5fcdd7c4f960dcdc0cc17`. This
also does not identify a retail source for its sampled direct-colour draw.
Against the installed English Nexus data, frames 0, 299, 300, 301, 320, 360,
400, 500 and 599 each produced one unbound draw and zero exact matches across
126 hash-verified files. The manifest skips 1,200 emulation frames, captures
600 frames, and schedules Start at emulation frame 1,500; the input patch's
frame counter is not reset by the capture skip, so the press falls within the
capture window, around capture frame 300. The raw VDP1/source join still does not
identify the retail source or establish whether the input caused a visible
title-to-menu transition. Keep this separate from the English merged-disc
negative result; neither capture authorizes renderer admission.
An earlier cold-start capture requested 400 frames but retained only 48 before
its 45-second timeout, before its scheduled Start input at capture frame 300.
These earlier captures are incomplete or negative provenance observations.
The 2026-10-27 controlled English-version capture establishes that this disc
image cold boots to the three-choice title menu, with a complete VDP1 command
trace and a partial source join. A matched menu-state capture now binds the
five English New Game and Continue sprite draws to authentic `TITLE.BIN`
offsets, captured CRAM colors, placement, and screenshot pixels. The NBG0
background source and full-screen consumer remain unbound; neither capture
identifies which scheduled input caused the cold-boot title transition. Keep
the production menu gate closed until the background and complete display
ownership are joined and Firestaff's own title/menu route is verified against
the same English assets.

PR #19 merged the corrected NBG1 TileFetcher address path and a scroll-offset
regression. Its focused test verifies the bounded name-table fetch behavior;
this does not bind the
NBG1 data to FONT256, MENU.BPK or a retail text/display consumer, and does not
change menu, HUD, viewport or production-readiness gates. Keep the next work
focused on identifying the authentic NBG0 background source and complete
English display consumer, authenticating the title input transition, and
wiring the verified sprite and input routes into Firestaff without opening a
renderer gate prematurely.

The 2026-10-26 Mednafen `.mca` startup capture is not evidence of a loaded
authentic backup card. Mednafen 1.32.1 uses the `mca` suffix for its autosave
state via `MDFNMKF_STATE`/`filesys.path_state`; `filesys.path_savbackup` is a
separate backup-file path. The different capture hash and `Cart: Backup
Memory` line do not identify loaded card bytes. Re-run with the authentic
Mednafen save-memory file under verified `filesys.path_sav`, and retain its
digest, effective settings and a same-session load/read receipt. The local
Firestaff `.bkr` is a different format and cannot substitute.

A retained Ymir v0.4.0 Japanese title-menu frame received a read-only source
join on TRV2 against the authentic Japanese CUE/ISO members. Its hash-pinned
VDP1 VRAM (`0f484877f5c5ef300fcbcaec6948b50c8b7aed7945e448a9f8281b2da46a2f21`)
contained 137 active texture draws: 124 indexed image/CLUT pairs from
`LEV00.DGN`, eight direct-color spans from `LEV00.DGN`, and five `TITLE.BIN`
spans, with zero active draws left byte-unbound. Of the 124 indexed draws, 108
matched a static FACE selector and 16 did not. This narrows authentic source
bytes for one emulator frame; it does not establish SH-2 upload ownership,
semantic use of the 16 selector-free draws, input progression, full display
composition, or production renderer permission. Preserve the current startup
gate. The analysis ran against the authentic media and raw capture retained on
TRV2; the VDP1 dump is intentionally not copied into this repository.

The 2026-10-24 direct-CLI check against authenticated Japanese retail
`DM.BIN` confirms the same remaining start gap: the title reaches its ready
frame, but no menu or level is loaded and the startup receipt remains blocked
on the PRS3 menu capture. See
[`parity-evidence/verification/nexus_cli_direct_start_block_20261024/README.md`](parity-evidence/verification/nexus_cli_direct_start_block_20261024/README.md).

The title asset loader now keeps `startup_title_vdp_capture_verified` closed
when it decodes authentic `TITLE.BIN`/`TITLE.CG` MAPD data. That decode admits
the bounded five-plane asset renderer only; it does not prove Saturn VDP1/VDP2
composition or open the native title/menu handoff. Preserve this separation
until a capture-backed production receipt explicitly admits the display
consumer. A rebuilt real-media pointer-only launcher check now reports
`title-vdp-owner-compositor-capture-required` for the installed Japanese
retail image and rejects a playable-runtime claim. See
[`DONE-nexus.md`](DONE-nexus.md) for the verification record.

## 2026-10-27 — Resolve Nexus edition ownership when extracted files and discs coexist

The authentic shared Nexus data directory contains extracted Japanese-route
files alongside multiple retail Saturn images. The normal M12 pointer route
reported `edition=nexus-saturn-jp` from the extracted `DM.BIN`, but the engine
then opened the English CUE as supplemental media. A working-tree attempt to
prevent this by matching the disc's `DM.BIN` still selected English: the
Japanese and English images share that executable, so its byte identity does
not distinguish their localized resource sets. The real-media regression
`tests/test_nexus_v1_native_menu_mouse_title.sh` failed on this assertion.

Do not treat a matching `DM.BIN` as proof that an ISO belongs to the selected
language/revision. Keep extracted data and disc members under one verified
edition owner, or reject the ambiguous combination. Resolve the selected
M12 version against a stronger identity (for example, a set of authenticated
region-specific members or the complete disc identity), then verify that the
CLI and menu routes open only that owner. Keep the production title/menu gate
closed. The attempted `DM.BIN`-only selection in the current working tree is
not a fix and has not passed this real-media test.

- Implement the observed retail title-menu input flow in Firestaff and identify
  the display consumer that places menu and face material.
  Updated 2026-10-28: the Japanese retail Ymir run establishes a no-save
  cold boot to the native three-choice menu; a separate single Down press
  selects Continue, and Down/Down/A opens Configuration from that captured
  menu state. Firestaff retains a separate keyboard selection state after the
  ready title prompt; F, Enter, keypad Enter, Space, Up/Down, and W/S are wired
  only while the Nexus title is active. An English retail Ymir replay verifies
  Down to Continue and Up back to a pixel-identical New Game frame. Firestaff
  renders English New Game, Continue, and Configuration selected-state
  foreground sprites and Japanese New Game, Continue, and Configuration
  sprites from authentic regional `TITLE.BIN` data and source-bound palettes.
  The shared background and full Configuration page remain unrendered. New
  Game now reaches the authenticated `DM.BIN`/`LEV01.DGN` loader and emits the
  retail level-1 start receipt, but the downstream viewport/HUD gate still
  blocks a playable handoff. Ymir now proves the retail Continue BKR lookup,
  payload read, checksum gate, and 24/36/48 selected roster-field
  projections from authentic two-, three-, and four-champion saves.
  Firestaff still does not load Saturn BKR state, and Configuration
  destination behavior remains unimplemented.
  The Japanese New Game
  and Continue captures each verify five `TITLE.BIN` sprites and 1,441
  screenshot texels; the English states each verify 1,451. The Japanese
  Configuration-selected capture verifies five sprites and 1,453 texels. The
  English Configuration capture verifies five VDP1 commands against retail
  assets, with all 1,069 text pixels matched; marker RGB remains dependent on
  the unbound background. The frame-8047 replay
  reconstructs all 286,720 screenshot pixels from Ymir's VDP1 display buffer
  and VDP2/CRAM snapshots, with 178 VDP1 commands and zero dropped rows.
  A same-run repeat has the same screenshot hash and a complete frame-8047
  VDP1 trace, but the retained PNG is a copy whose path differs from the
  ephemeral path in `run.log`; the `--run-log` verifier rejects that relocated
  copy and exact screenshot-frame identity remains unproven. A frame-tagged
  SH-2 trace records 15,264 writes at frame 7,716; a separate frame-6,893
  source-read pass covers all 31,616 Japanese `TITLE.BIN` bytes but records no
  values and drops 860,856 unrelated reads. The final NBG0 snapshot has a
  30,561-byte storage match under the verified row layout, not a complete
  pixel/source join. See the Japanese and English menu sections in
  `docs/nexus/nexus_startup.md` and the
  [snapshot join](docs/nexus/NEXUS_MENU_BPK_VDP1_SNAPSHOT_JOIN.md).
  These captures strengthen retail evidence; they do not prove full Firestaff
  menu composition, source ownership, or destination execution. Keep those
  gates closed.
  Earlier paired raw-trace experiments below remain valid negative results for
  their sampled windows, but they are no longer the only input evidence. Keep
  full-menu admission closed while binding background and face-material
  ownership and verifying any remaining region-specific layouts.
  The JP window at frames 13000–13039 is bit-identical with and without
  Start/A pulses, so it is non-interactive animation rather than the native
  startup menu. A later same-session control/Start pair establishes that
  Start is read from the live controller register at frames 18020--18023 by
  SH-2 code, but its 64-frame VDP1/VDP2 output remains byte-identical to the
  no-input control. That specific reader is therefore not an admitted menu
  consumer. Later Ymir captures establish title-menu and selection transitions
  and record a frame-tagged source-address read pass, but do not prove the read
  values or bind the full menu/face image to a complete upload or display
  consumer. A separate post-title control/input
  pair at frames 30000--30119 exercised Start, A, B and C in four distinct
  20-frame pulses.  All eight captured video regions (VDP1 state/VRAM/frame
  buffers/draw selector and VDP2 registers/VRAM/CRAM) were byte-identical to
  the no-input control.  The observed active VDP1 state has two stable
  texture sources for frames 30000--30037, but remains an unbound hardware
  observation rather than proof of a menu, HUD or dungeon consumer. This
  pair covers only its sampled noninteractive window. Separate
  interactive Ymir evidence on authentic Japanese retail media shows
  F/Start opening the title menu, S selecting Continue, and J/A confirming
  New Game into the dungeon. A separate ShuttleMouse run shows a button event
  advancing the title into the intro. These establish retail input behavior,
  not Firestaff startup parity or the menu/face display consumer; see
  `parity-evidence/verification/keyboard-mouse-debug-20261024/README.md`.
  A further matched JP control/input pair at frames 40000--40079 scheduled the
  remaining Up/Down/Left/Right/X/Y/Z/L/R masks (each for two frames). The
  80-frame raw VDP1/VDP2 capture matched the no-input control byte-for-byte
  (`627a1055274abbd164bc1bab7a9253ae2760e0a1a039ef0c587748ff10083681`),
  and frames 40000 and 40079 rendered identically; the input-event receipt
  confirms only that the external hook scheduled the masks, not that Saturn
  code consumed them. This later window adds no menu/gameplay semantics; the
  first input-correlated presentation change within these raw capture windows
  remains open.
  A fresh cold-start matched pair then captured frames 0--279 on the
  authenticated Japanese BIOS/disc: Start was requested at 140--199 and A at
  210--239 in the input run; the control forced no buttons in those windows.
  The SMPC `0x10` read at RPC `0x060103e6` returned `0x10` in the input run
  and `0x00` in control, but all ten sampled rendered frames and the complete
  280-frame raw VDP1/VDP2 stream were identical (both raw SHA-256
  `ea2eb96dc56ce9505d67062b2a5f98141d2a456d413c74c19997c04346e27af3`).
  This confirms delivered input and a live reader, not action semantics or a
  presentation change in this matched cold-start pair; startup/menu admission
  remains blocked. A later
  matched frame-10500 WorkRAMH pair and retail CD-to-RAM source receipt now
  bind the relocated reader PCs to an exact 512-byte `TM.BIN` span at file
  offset `0x4500` (destination `0x06014500..0x060146ff`). The authenticated
  instruction stream reads SMPC OREG and a Start pulse changes six bytes in
  the matched input/control memory snapshots. It still does not map those
  bytes to a Start action or menu transition; see the JP controller-reader
  section in `docs/nexus/NEXUS_RUNTIME_CAPTURE.md`.
  A complete 52-frame matched pair then scheduled 13 two-frame button pulses
  and sampled WorkRAMH every four frames. All eight VDP1/VDP2 regions remained
  byte-identical. Two corresponding RAM ranges differed, but their values did
  not uniquely track the 13 masks; this is not a button/action mapping. A later
  195-frame pair used 12-frame holds, three-frame releases and a
  WorkRAMH snapshot every frame. It still produced no VDP1/VDP2 presentation
  change; RAM differences persisted through release frames and did not bind all
  masks to actions. A later independent 60-frame Start-only pair joins the
  mask to transient controller-state bytes (`0x0602c90c`); a frame-stamped
  SH-2 read/write follow-up observes `0x1000f001` versus `0x0000f001` at
  `0x0602c90c` and related byte writes, but the raw VDP streams and all four
  sampled renders remain identical. This is controller-state evidence, not an
  action or menu dispatch; two other RAM bytes remain different through all
  release frames. A widened one-frame pair also reaches TM.BIN state code at
  `0x0601462c`: authentic JP TM.BIN compares the byte read at `0x0602c940`
  with `1`, but that fetched value is identical in Start and control; only a
  sampled register differs, while raw video remains identical. This is not a
  Start-dependent dispatch or menu receipt. Continue tracing the controller
  state into a source-bound event consumer and capture an actually interactive
  title/menu interval before opening menu admission; see
  `docs/nexus/NEXUS_RUNTIME_CAPTURE.md`.
- Resolve the remaining Structure2/VDP1 material, texture, CLUT, raster,
  clipping, animation and composition ownership with real captures. Keep
  unbound bytes and generated fixtures out of production gameplay. In the
  retained authentic 701-frame JP Saturn window, frame 700 has 249 linked
  VDP1 records; the current compositor resolves 225 of 242 draw commands and
  correctly keeps semantic admission blocked for the 17 unowned draws. The two
  nonzero mode-1 spans at VRAM offsets `0x58b58` (344x177) and `0x58c58`
  (88x177) have no exact or word-byte-swapped match in the 156 files under the
  TRV2 user's `.firestaff/data/nexus` directory. The Japanese retail
  `MENU.BPK` there matches its verified SHA-256
  `740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636`; the
  real-media surface-class test passes and the PRS3 decoder succeeds on all
  162 PRS3 surfaces, but a capture-CLUT/palette-color remap and 4-bpp repacking
  still produce no exact join for either span. Thus neither source ownership
  nor palette, placement, or the original Saturn consumer is established.
  A frame-by-frame pass finds the `0x58c58` source in 287 command lists and
  the `0x58b58` source in 289, spanning relative frames 97--700; each pixel
  span has exactly one SHA-256 across the window, and both commands share one
  stable captured CLUT hash. Their raw signed command coordinates are
  `(-3701,-3701)` and `(-3733,-3733)`; local-coordinate transforms and actual
  visible placement remain unverified. This proves recurring stable runtime
  bytes only, not retail provenance or composition.
  A separate hash-bound cold-start capture at runtime frames 13197--13897 did
  not reproduce either source offset in any of its 701 frames; frame 700 has
  only one mode-0 source at `0x415a0`. Its ten-million-row VDP1 write-trace
  budget filled before frame 0, so that run cannot bind writes to the captured
  window. Recover or reproduce the exact retained runtime window before using
  a fresh writer trace to claim source upload provenance. The external
  producer now supports inclusive absolute-frame bounds; set them to
  `skip_frames` through `skip_frames + frame_limit - 1` so earlier startup
  writes do not consume the selected capture window's trace budget.
  The bound pre-capture prefix did record one write at each target start from
  PC `0x0601307c`, but the corresponding 96-byte live code window has no exact
  native- or word-swapped match in the authentic 156-file corpus, including
  `DM.BIN` at its verified load base. Trace the helper's SH-2 RAM reads and
  inspect its runtime caller in a frame-aligned session before assigning that
  writer or either span to a retail resource.
  Keep both spans blocked until upload provenance or an exact transformed
  retail-surface join is captured.
- Implement native Saturn runtime semantics only after each dispatcher,
  material, event, save or audio consumer has a captured, hash-verified
  contract.

## Input parity

- Finish Nexus keyboard and mouse gameplay routing. Retail Ymir captures now
  confirm forward movement, left/right turns, Start-to-pause, and the Z cursor
  mode. Firestaff maps A/D to Nexus turns while preserving DM1's A/D strafe
  behavior. Tab cycles the loaded party leader forward and Shift+Tab cycles
  backward in Firestaff's desktop controls. The reverse direction is a host
  keyboard convenience; the native cursor action contract and attack,
  inventory, and spell semantics remain open.
  An independent 2026-10-25 Ymir `--debug` comparison loaded a hash-verified
  authentic JP gameplay savestate into a private profile: the no-input reload
  changed 0.00175% of pixels, W-forward changed 3.08408%, and A-left changed
  5.08610%. The three real captures and source hashes are recorded in
  `parity-evidence/verification/nexus-savestate-input-20261025/README.md`.
  This validates the retail ControlPad response from a stable gameplay state,
  not Firestaff's desktop route. The isolated Xvfb exposed no nonzero SDL
  mouse for Ymir ShuttleMouse, so mouse gameplay remains unverified there.
  Static SH-2 triage is now reproducible with
  `scripts/analyze_nexus_mouse_event_crossrefs.py`: the hash-verified English
  DM.BIN has no direct PC-relative literal references to the 60 event-name
  strings, while its HUD table's subtable at `0x38040` is referenced. This
  scan is supplemented by a verified SH-2 path that registers the hit-region
  table with a callback and later dispatches indirectly through that callback
  slot. A second retail callback trace verifies object fields 100/104 and
  queues numeric events 13/44 to `0x06024288` in two modes. The retail
  pointer table names these `EV_EXIT_INV` and `EV_INV_EQUIP`. The event-44
  path is now traced through its context resolver, slot-pair reads/clears and
  compatible destination writes: it uses context stride `0x13c`, slot-pair
  base `0x0607fba0 + context*0x13c + 0x110 + slot*4`, and 40-byte item
  metadata records. Slot 7 has a separate path. Firestaff only has a partial
  item-ID fit: it lacks the opaque second word, and context-to-champion mapping
  is unproven. The hash-pinned Japanese RLOWFIX scan finds 20 PLRD rows with
  ten pairs each: 64 nonempty item ordinals, 136 empty pairs, and zero in all
  second words. This does not identify auxiliary data for live items. The
  helper that transfers the held pair through global UI state and the visible
  post-click result also remain unverified. The eight authentic action rectangles at
  `DM.BIN:0x3850c` are verified to queue `EV_INV_EQUIP` with slot arguments
  0--7 through the retail bounds scanner at `0x24ed6`. Bind that event's
  actual retail state owner to the authentic champion/inventory model before
  opening the production equipment-click route. The verified consumer at `0x21c84`
  special-cases only IDs 38, 39, 40, 49, 52, 56, and 57; IDs 13 and 44 reach
  its generic `bad event data` diagnostic. The separate mouse poller filters
  for event 46 before it invokes the hit scanner; it does not separately
  consume event 44. Continue tracing the held-item helper and capture a
  controlled click's guest RAM and visible result before enabling pointer
  actions.
  See `parity-evidence/verification/keyboard-mouse-debug-20261024/README.md`.
  See also `parity-evidence/verification/nexus-mouse-callback-dispatch-20261024/README.md`.
  Firestaff now has a host Z toggle, logical pointer motion through keyboard
  WASD/arrows or mouse movement, including captured diagonal WASD movement,
  and the authentic retail hand sprite joined from
  ITEM.IBS image 65/palette 6 against Ymir VDP1 capture. Pointer click actions
  remain open until their retail source joins are captured. All host
  pointer-button routes now use the same parsed retail movement-pad rectangle
  and six-cell desktop mapping; this does not claim Saturn ShuttleMouse
  parity. The authentic callback's click-action semantics remain unresolved.

- Input parity follow-up: Firestaff routes Tab and Shift+Tab to cycle the
  loaded Nexus party leader forward and backward. Authentic Ymir `--debug`
  captures confirm retail L/R changes the visible leader marker and portrait;
  the desktop reverse binding is a host convenience, and the captures do not
  bind the guest RAM owner.
  Retail PLRD has no live/dead field, so Tab treats an available roster row
  with positive health as selectable even while `alive` is unknown, and skips
  zero-health members.
  Keyboard/mouse movement, pointer motion, and the desktop movement pad remain
  available. The hand pointer now travels across the authentic full-game-screen
  hit rectangle, including the HUD, clipped to Firestaff's 320x200 framebuffer.
  Mouse clicks on the hand pointer and non-movement actions remain blocked
  until the opaque item data, champion context mapping and runtime click effects
  are established. See `docs/nexus/nexus_input.md` and
  `parity-evidence/verification/nexus-leader-cycle-debug-20261025/README.md`.

## English Configuration page remaining parity

The authentic English retail Configuration page now has source-bound foreground
rendering, keyboard/mouse navigation, and Exit/B return behavior. Ymir
`--debug` captures and the production-renderer verifier establish the foreground
and four initial values. Continue with the real 3D/DGN background source and
composition, then trace setting persistence and prove the audio, difficulty,
and HUD effects in retail gameplay. Japanese Configuration-page parity
remains open. The New Game loader receipt is now verified, but its viewport/HUD
handoff remains blocked. Retail Continue payload transfer and a bounded set of
runtime roster-field projections are verified in Ymir. Firestaff can now probe
one explicitly selected BKR file with bounded memory and return verified card
and roster metadata, but the Load UI does not yet select a card or apply its
payload. Complete state mapping and a playable Continue handoff remain open. See
`docs/nexus/nexus_startup.md` and `docs/nexus/nexus_title.md`.

The authentic English Ymir Configuration-page frame 119 source scan examined
148 textured VDP1 draws. It found exact DGN Structure2 image spans for 121
informative draws; 11 additional draws are all-zero transparent textures.
The broader retail-resource join matches 137 informative draws, while 113
image spans also occur in whole-file `LEV00.DGN`. These are source candidates
only: repeated bytes leave instance ownership ambiguous, and the active cell,
face, placement, camera, and complete scene output remain unbound. Continue by
joining command UV/CLUT and geometry state to a specific `LEV00.DGN` cell/face
and the retail scene setup path. See
`parity-evidence/verification/nexus_english_configuration_scene_dgn_join_20261028/README.md`.

The frame-119 DGN sequence join matches all 121 informative draws (113 indexed
image/CLUT pairs and 8 direct-color spans); 11 draws are transparent all-zero
textures. Of the matched draws, 105 have candidate Structure3 face-owner rows,
often with multiple possible owners. Sixteen matched draws have no face-owner
row. Determine the active owner and transform from the Saturn SH-2 draw setup
before using these materials to render the Configuration scene.

The authentic Load-panel NBG0 work now binds an 86-byte `MENU.BPK[55]` source
span and the unique 228x100 `MENU.BPK[60]` panel base to frame-bounded SH-2 and
VDP2 VRAM writes. The captured CRAM palette output is pixel-exact against the
Ymir screenshot for 43,889 visible NBG0 pixels. The PALT table matches the
captured CRAM at all 41 static used indices; a retail animation supplies
visible index `0x83`. Decompilation and the authentic frame-659 WorkRAM-HI
snapshot now join the animation's PALT v0 pointer slot to a live `RES*` record;
its 512-byte payload is an exact match for `RHIFIX.BIN` at offset `0x1140`, and
the live archive differs from that hash-verified file only at version byte
`0x0b`. The table contains the `0xffff` value seen in the frame-655 CRAM write;
nine four-frame-spaced writes match PALT indices 23–31 in order, identifying
index 31 for frame 655. Direct bulk
static-palette-transfer ownership and complete
bitmap composition remain unresolved. Continue source tracing for the CRAM
static-palette upload, full-screen producer, and transparent-pixel composition before
admitting a Load-panel renderer. See
`docs/nexus/nexus_startup.md` and the external trace bundle's
`nbg0-menu-source-trace.txt`, `nbg0-palette-screenshot-join.txt`, and Ghidra
decompilation logs.
