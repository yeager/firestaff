# Firestaff TODO — Nexus

Reviewed 2026-11-02. The items below track open work and include completed
diagnostics as context. Verified evidence is also recorded in the Nexus capture
and reverse-engineering records and in [DONE-nexus.md](DONE-nexus.md).

## 2026-11-02 — gate champion diagnostic output

Champion recruitment and automatic leader-promotion messages now follow the
Nexus engine's `--debug` setting. The flag is copied into the runtime-only
champion-pool state after retail roster initialization and when an initialized
launcher receives an updated debug setting; it is not part of save data. The
Vi Altar model helper has no runtime diagnostic context, so its unconditional
stdout message was removed. The champion, engine and launcher translation
units compile with AppleClang. This is source/compile evidence only; authentic
runtime output for recruitment and leader promotion remains unverified.

## 2026-11-01 — Frame-300 VDP pixel-to-command ownership

The authentic Japanese gameplay frame-300 VDP2 composition now links each of
its 65,991 visible top-layer VDP1 pixels to the last-writing VDP1 command and
that row's source receipt. The 65,500 RGB pixels map to indexed retail
`LEV01.DGN` rows. The 491 CRAM pixels map to captured polyline row 79 (60
pixels) and mode-4 sprite row 80 (431 pixels). The combined real-data
regression verifies those counts together with the complete VDP2 RGB match and
the DGN joins. This proves pixel-to-command/source-class ownership for this
captured state, not a selected Structure3 face per pixel. Active face
selection, transform/culling, other gameplay states, and production renderer
admission remain unresolved. See
[`docs/nexus_vdp2_gameplay_frame300_composition.md`](docs/nexus_vdp2_gameplay_frame300_composition.md)
and PR [#252](https://github.com/yeager/firestaff/pull/252).

## 2026-10-31 — NBG0 live-row replay

The capture-only compositor now accepts Ymir live per-row scroll and
coordinate increments, demonstrated against the authentic Japanese intro
capture at rows 4–116. Source transfer timing remains unbound in this capture;
do not enable this lane in the production renderer. Continue gathering
same-frame producer-to-VRAM evidence and verify row-state coverage across
animation frames. See
[`docs/nexus_vdp2_nbg0_live_rows.md`](docs/nexus_vdp2_nbg0_live_rows.md).

## 2026-10-31 — Authentic VDP2 register order and composition rerun

The VDP2 color-calculation analyzer now detects register word byte order from
the Saturn display-enable value `TVMD=0x8000`, with a score-based fallback and
an explicit `--byte-order` override. This matters because an authentic baseline
capture has `BGON=0x0200`; relying on BGON alone had selected the wrong order.
The corrected analyzer identified big-endian order and decoded `TVMD=0x8000`
in 92 authentic Ymir register captures. It reports the expected `CCCTL` and
`SFCCMD` values for the hash-pinned gameplay frame 2173 and title frame 8037.

The Debug build at PR commit `c376f0c` re-ran the authenticated composition
matrix: gameplay frames 239, 397, and four-champion Continue frame 2173 match
all 286,720 doubled screenshot samples exactly; frame 300 matches all 71,680
pixels in its same-run Ymir RGB reference. Separate authentic frame-7486
NBG0 live-row/color-calculation and frame-8037 title-menu composition tests
also pass against their retained Ymir outputs. The NBG1 active per-character
color-calculation path remains unverified because the available eligible BKR
captures contain no visible NBG1 PND with CCE set. These capture-only results
do not enable production rendering or establish arbitrary-frame parity.
Outputs are retained at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/pr239-c376f0c4-validation/`.

## 2026-10-31 — Frame-59 title-prompt composition

The exact Japanese Ymir title-prompt frame now has a hash-pinned, capture-only
NBG0-over-VDP1 replay and a real-data regression against the raw RGB reference
and screenshot. The visible 921-byte TITL span is joined, but the complete
NBG0 source transform and source owner remain unresolved. This lane does not
cover the later title menu, configuration/Load panels, arbitrary display
register profiles, or production rendering. See
[`docs/nexus_vdp2_title_frame59_composition.md`](docs/nexus_vdp2_title_frame59_composition.md).

## 2026-10-31 — NBG1 4bpp MAPD startup plane

The authentic Japanese MAPD startup pass (VDP1 frame 891 / VDP2 serial
1280) now has a hash-pinned capture-only NBG1 4bpp decoder. It applies Saturn
pattern palette selection, rejects N1TPON and unsupported two-word CRAM color
modes, and records the decoded layer color for later composition. The NBG1
priority-stack path now consumes that palette-resolved RGBA color instead of
treating the raw dot index as a CRAM address. The opt-in authentic MAPD layer
test also rejects each unsupported NBG1 line-state flag in SCRCTL bits 8–11
individually. The authentic capture has `SCRCTL=0`. The VDP priority-stack
comparisons for frames 239, 397, 300, and 8037 passed against PR code commit
`c1d2664`. A new hash-pinned regression for the four-champion Continue frame
2173 was added and passed against commit `8bfbdd9`.
Frames 239, 397, and 2173 each match all 286,720 doubled screenshot RGB samples
with zero channel error; frame 300 matches Ymir's same-run RGB reference, and
frame 8037 matches both Ymir pass buffers. These results verify only the
captured states and do not enable production rendering. The later full CI run
also found unrelated stale localization catalogs and a Theron source-boundary
failure in the merged PR check. See
[the MAPD 4bpp capture report](docs/nexus_vdp2_nbg1_mapd_4bpp.md).

## 2026-10-30 — NBG1 8bpp character stride

The generic source-bound NBG1 compositor now follows Saturn's 32-byte PND
character-number stride in 8bpp mode and wraps full 512 KiB VRAM reads. A
hash-pinned Ymir gameplay capture at frame 239 checks the 320x200 NBG1 index
plane's opaque/transparent counts and a known pixel using Ymir's 32-byte
character-address rule. This fixes the byte fetch for that layer path; NBG1
retail source ownership and whole-screen VDP2/HUD composition remain open. See
[`docs/nexus_vdp2_nbg1_8bpp_stride.md`](docs/nexus_vdp2_nbg1_8bpp_stride.md).

## 2026-10-30 — NBG2 capture plane and palette handoff

A hash-pinned Ymir frame-239 test decodes the authentic NBG2 8bpp map page at
byte offset `0x60000`, covering 40×28 visible cells. It yields 11,232
nonzero pixels; 9,258 CRAM colors match the native screenshot sample within
eight levels. The decoder and priority stack now apply the NBG2 PND palette
number and RGB555 CRAM wrap. Rerun the authentic frame-239 plane and full-stack
comparisons against this revision before treating the palette handoff as
verified. This remains a capture-only measurement; full VDP2 color calculation
and named retail source ownership are unresolved. See
[`docs/nexus_nbg2_capture_plane.md`](docs/nexus_nbg2_capture_plane.md).

The same frame also verifies the existing NBG0 8bpp bitmap lane at VRAM byte
offset zero: 3,954 of 3,968 nontransparent CRAM samples match within eight
levels. This remains capture-only and does not establish cross-layer ordering
or color calculation. See
[`docs/nexus_nbg0_capture_plane.md`](docs/nexus_nbg0_capture_plane.md).

## 2026-10-30 — VDP1 production gaps after frame-13162 replay

The capture-only frame-13162 stepper and its exact Ymir FBS match are recorded
in [DONE-nexus.md](DONE-nexus.md). Production work remains: bind each VDP1 draw
to its selected retail scene face, derive camera transform and culling from
retail execution, and validate the renderer across gameplay states. One exact
Ymir frame replay does not establish original-Saturn or arbitrary-frame parity.
See [`docs/nexus_vdp1_frame13162_dgn_window.md`](docs/nexus_vdp1_frame13162_dgn_window.md).

## 2026-10-30 — Japanese title capture follow-up

Authentic TRV2/Ymir VDP1 frame 8037 now has two complete VDP2 pass buffers
that agree pixel-for-pixel with both Firestaff's capture-only replay and the
same-run frontend screenshot after 2× downscaling. A hash-pinned real-data
CTest verifies all 71,680 native RGB pixels in both passes. This is
emulator-internal frame composition, not original-Saturn or production-renderer
parity. The replay consumes the captured FBS, NBG0 bitmap, CRAM, and registers;
pixel-to-retail-command/source ownership remains incomplete. The capture shows
the retail title and three menu choices with `BGON=0x0001` and ratio register
`0x100=0x1000`; 2,990 VDP1 palette/special pixels still lack complete
per-command CLUT/CRAM provenance to retail source data. Keep the production
title gate closed. Evidence details are in
[`docs/nexus/nexus_title.md`](docs/nexus/nexus_title.md#2026-10-30--authentic-japanese-title-frame-capture).
The test and its external-data reproduction instructions are in
[`docs/nexus_vdp1_frame8037_ymir_composition.md`](docs/nexus_vdp1_frame8037_ymir_composition.md).

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
mana. Payload byte `+0x09` projects to runtime `+0x35`, which follow-up static
analysis places on an ITEM.IBS action-selector path; its domain meaning
remains unresolved. The BKR receipt exposes that byte. It is `2` in every
champion record across the four hash-verified retail BKR
images and is not yet connected to Firestaff's spell-input state. Other
champion-record fields remain opaque.
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
four records. These offsets remain mechanically verified and unnamed. A
read-only Ghidra scan found a second runtime `+0x1a` access in
`FUN_060373d0`, which uses the same champion-row base and `0x13c` stride as
the BKR importer and updates the halfword on an event path. This narrows the
next source investigation but does not establish the field's domain meaning.
`FUN_060295b8` also reads both fields from that same indexed row, adds `0x0400`
and passes them to `FUN_060236ba`, which selects a clipped/sliced render path
for positive values. Its caller invokes it in one mode path. This establishes
a presentation use, but not the fields' interpretation or a coordinate
system. The complete consumer set and semantics for both fields, the rest of
the record, the source-owned post-load consumer, and Firestaff runtime
integration remain open. Follow-up static analysis places `FUN_0603a472` on a
four-index ITEM.IBS eligibility path; its helper reads runtime `+0x18` for
some 40-byte records and its related selector state is `+0x35..+0x38`. The
earlier four-entry Magic-validator interpretation conflated that selector with
the composer's separate `+0x3d/+0x3e` buffer. No call from the composer or
connection to a cast has been established. Identify the saved field's meaning
and any relationship to the displayed Magic window independently.
Reproduction and retained evidence are documented in
[`nexus_sh2_startup_decompilation.md`](docs/nexus/nexus_sh2_startup_decompilation.md#continue-payload-transfer-2026-10-29).
The Ghidra field-consumer scan remains in the external evidence bundle; its
relevant decompilation and caller logs are preserved in
`parity-evidence/verification/nexus-bkr-item-action-eligibility-20261029/`.

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

The production gate is also source-audited: `nexus_v1_launcher_fill_startup_assets_receipt`
requires `engine->startup_title_vdp_capture_verified`, and the production
`nexus_title_load` path resets that field to zero. No production assignment to
one exists; only synthetic launcher-test setup does so. Do not clear the gate
by assigning the flag. The next implementation step is a narrow receipt that
binds an authentic same-session displayed title frame, VDP1/VDP2 state, source
spans, palette and placement to the production composition consumer. The real
New Game input and `LEV01.DGN` start-position handoff remain separate gates.

## 2026-10-29 — Keep extracted Nexus resources and supplemental media in one region

Resolved the shared-`DM.BIN` misidentification with authentic regional
fingerprints. M12 now identifies extracted JP, EN, and FR resources from the
canonical `RLOWFIX.BIN` hash plus at least two matching `TITLE.BIN`,
`MENU.BPK`, and `FONT256.S2D` hashes; every present fingerprint member must
agree. The shared executable alone is no longer treated as Japanese proof.
When an extracted directory is selected, the Nexus engine attaches a
supplemental CUE/ISO only when its canonical shared `DM.BIN`, `RLOWFIX.BIN`,
and at least two localized UI members match that extracted region. A
conflicting or unknown neighboring disc is left unattached.

The updated `tests/test_nexus_v1_native_menu_mouse_title.sh` passed on TRV2
with authentic JP data and an authentic staged EN data set. Both menu pointer
routes reached the existing title compositor capture gate. Direct `DM.BIN`
launches retained their extracted JP/EN profile and selected matching CUEs;
the EN menu route with its canonical full image retained the EN ISO identity,
and the authentic Japanese CUE was rejected as supplemental media for the EN
extraction.
These checks do not establish a playable title/menu or open the compositor
gate.

## 2026-10-30 — Admit source-bound Saturn CUE/BIN media

Resolved the CUE-only CLI/menu admission gap. M12 now authenticates the opened
CUE's data track as Nexus from canonical `DM.BIN`, regional `RLOWFIX.BIN`, and
at least two agreeing localized `TITLE.BIN`, `MENU.BPK`, and `FONT256.S2D`
fingerprints. It checks that every payload declared by the CUE exists and
keeps the exact CUE path as the runtime owner. Existing extracted and full
image owners retain precedence; an explicitly selected unverified CUE fails
closed instead of selecting a sibling disc.

On TRV2, `nexus_v1_native_menu_mouse_title` passed with authentic Japanese
retail media. The regression isolated the CUE and its original tracks by
symlink and verified both direct CLI and mouse-menu handoff, then selected an
authentic DM2 CUE beside the valid Nexus disc and verified that Nexus refused
the mismatched selection. Both valid routes still stop at the existing
capture-gated Nexus title; this change does not make the game playable.

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
- The retained Ymir Japanese title-menu capture at frame 119 independently
  binds five ordered VDP1 draws (two selection markers and three labels) to
  exact `TITLE.BIN` texture/palette records and screenshot samples. The
  capture-only Ymir menu input now supports a bounded ordered list of
  source-joined command offsets, applies them in trace order, and reports the
  aggregate draw count and pixel count. A hash-pinned frame-119 C integration
  case now checks all five commands against retail `TITLE.BIN` texture spans
  and compares the resulting 320x224 RGB plane with the retained Ymir output;
  the focused test passes. The replay remains capture-only. Keep production
  renderer admission blocked until runtime ownership, complete menu input,
  and broader frame behavior are verified.
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
  inventory, and spell semantics remain open. An authentic 2026-10-29 Ymir
  `--debug` replay verified retail Control Pad Y opens the Magic window from a
  real BKR-loaded gameplay state; Firestaff still does not render or route
  that panel. A second authentic replay from a saved Y-open state confirms
  Down/Left/Right change the screenshot and reorder VDP1 references to nine
  exact `MENU.BPK` surfaces (entries 99–106 and 144); Up matches the no-input
  control. All nine commands use 8bpp bank mode, and all 31 nonzero palette
  indices in those joined surfaces match both Japanese `MENU.BPK` PALT and
  same-run captured CRAM. Rune names, final spell acceptance, full dispatch,
  and casting remain unverified. Static analysis found generic
  `MENU.BPK` record/draw helpers but no direct relation from entries 99–106/144
  to rune names. Decompilation identifies `FUN_0602a234` as a sequence-composer
  callback: it computes cost, gates on current mana, writes
  `0xDA + cursor*6 + selection` at `+0x3e+cursor`, then advances the cursor
  stored at `+0x3d`. The four action slots are `+0x3e..+0x41`.
  Authentic GUI captures confirm all six cursor-0 values `0xDA..0xDF`, with
  costs 1..6, using cardinal and simultaneous diagonal input schedules. Four
  A actions fill the composer buffer as `DA E0 E6 EC`, cursor index 3, and
  observed mana changes from `0x12` to `0x09` across the four-A capture; this
  is the total balance change, not a per-action cost claim for cursor positions
  1–3. The fourth append returns its unchanged cursor value
  3 before the common UI update; recovered static caller sites discard it. A
  separate command-8 branch recalculates aggregate mana, but no spell-execution
  meaning is established. The direction handler maps logical events 0/1/2/3 to
  Up/Right/Down/Left, and the retail packet's active-high masks are traced
  through debounce to event `0x28`; physical Saturn electrical signaling is
  not established. In the authentic Y-open state, menu `+0x24` is null and
  `FUN_060328c6` performs no mapping; Down changes the generic menu item index
  0→4 without proving that index 4 is a rune. The Nexus coordinate-to-selection
  table and glyph meanings,
  final spell validator, and cast behavior remain open. Follow-up decompilation
  identifies `FUN_0603a472` as a four-index ITEM.IBS eligibility helper that
  reads runtime `+0x18` for some records; its selector uses row `+0x35..+0x38`,
  separate from composer `+0x3d/+0x3e`. Its relationship to the displayed Magic
  window remains unproven. Paired authentic captures now bind the six values to selected-
  state Japanese `MENU.BPK` entries 107–112 and command-88 VDP1 source addresses
  and coordinates, without naming glyphs. Next, recover the retail
  coordinate-to-index table and trace full-buffer submission to any actual
  validator or cast consumer.
  Preserve the capture receipt under
  `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-magic-down-input-trace-20261029/`.
  The hash-verified callback decompilation is under
  `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-magic-input-callback-static-20261029/`.
  Direction-event decompilation is under
  `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-dpad-menu-selection-20261029/`.
  The selected-surface join is at
  `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-menu-action-surface-bindings-20261029.md`,
  and the fourth-slot caller audit is at
  `/Volumes/Extern-disk/firestaff-evidence/nexus/magic-fourth-slot-status-20261029/`.
  The packet-to-event decompilation is at
  `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-saturn-pad-event-path-20261029/`.
  The Y-open null-table trace is at
  `/Volumes/Extern-disk/firestaff-evidence/nexus/magic-menu-coordinate-map-20261029/`.
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
  its generic `bad event data` diagnostic. Static disassembly now verifies
  relative cursor updates, both mouse descriptor-table alternatives, and the
  event-46 hit-test dispatch; matching live hitbox rows enqueue their own
  event/argument pairs afterward. No retained capture supplies a populated
  runtime table or scheduled ShuttleMouse report, so the physical `0x0400`
  trigger and live click-to-action binding remain unknown. Continue tracing
  the held-item helper and capture a controlled click's guest RAM and visible
  result before enabling pointer actions. See
  `parity-evidence/verification/nexus-shuttle-mouse-event-path-20261029/README.md`.
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
