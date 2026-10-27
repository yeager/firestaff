# Dungeon Master Nexus — startup status

This is an evidence report for startup, not a specification for a completed
Nexus startup screen. Updated 2026-10-27: an authentic Japanese retail cold
boot reaches the native `PRESS START BUTTON` title prompt and, with scheduled
retail controller input, the native three-choice title menu in Ymir. The menu
was navigated into its Configuration page from a saved diagnostic state.
Firestaff startup and presentation, the New Game route to stable gameplay, real
save loading through Continue, and HUD/viewport parity remain unverified.

## Verified initialization order

`nexus_v1_init()` first selects a hash-verified extracted corpus when `DM.BIN`
and the authentic DGN corpus are available as real loose files. `LEV00.DGN` is
a title/entry resource; the first playable level is `LEV01.DGN`. A valid
co-located retail ISO can then be read as a supplementary source for exact ISO
members. If only ISO data is available, the ISO reader is used directly.

After source selection, the engine attempts to read real `RLOWFIX.BIN`, startup
surfaces, `FACE.BIN`, `FONT256.S2D`, sound metadata, and other source files.
Missing or unbound consumers must not be replaced with synthetic surfaces.
Title startup must therefore not report a game position until Saturn's authentic
`LEV01` position and direction have been captured and bound to the same runtime
source.

## Verified startup resources

| Resource | Status |
|---|---|
| `TITLE.CG` + `TITLE.BIN` | MAPD/TIBG decoding and five retail images verified for the canonical profile and documented English revision; VDP2 placement is missing |
| `LOGOBG.DG2` | PP pixels and a 256-entry BGR555 palette are verified; layer/timing is missing |
| `WARNING.BIN`, `GAMEOVER.BIN`, `STABG.BIN` | real surfaces are decoded and receive byte provenance; presentation is missing |
| `FACE.BIN` | The JP retail member is verified as 45,104 bytes (`bd9ca16ea68043984e2804067b6cd66f`): 20 variable-length, 56×56 PRS3 portraits with source palettes. The PLRD roster-to-portrait ordinal and the VDP1 destination/order are still unobserved. |
| `FONT256.S2D` | 242 CG tiles are retained in production source; Saturn page/attribute/glyph mapping is missing |
| `MENU.BPK` | 162 PRS3 surfaces decode to source-bound indexed bytes; the exact title-menu draw ownership, ordering, CLUT, and VDP1/VDP2 composition are not established |
| Saturn Backup RAM | `nexus_v1_saturn_bkr_parse` reads hash-pinned 32 KiB card directories and block chains, returning exact opaque payload bytes; native save selection, payload interpretation, and runtime loading remain missing |
| Mednafen visual baseline | A clean J-BIOS capture against authentic retail shows black boot followed by the original intro FMV without input; menu/LEV01 identity remains missing |

## Deterministic title-state replay (2026-10-26)

The system-wide Ymir 0.4.2-dev capture build on TRV2 was run with `--debug`,
the authenticated Japanese BIOS and retail cue, and the retained title-prompt
state in CLI slot 2. The isolated profile mapped that state to
`savestates/5EA76F972566FE6EB667D092CD494BCF/1.savestate`; it was removed
after the run. One frame after the successful load, the VDP2 outputs matched
the retained title-state reference exactly:

| Output | SHA-256 |
|---|---|
| VDP2 VRAM | `de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c` |
| VDP2 CRAM | `db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de` |
| VDP2 registers | `79efa616c2db727749aa9018864d1ad32c1583935652fafb16534ba69f8e1f2d` |

The capture build also emitted big-endian VDP1 and VDP2 register files (32 and
512 bytes respectively) and a register snapshot. That tooling is proposed in
[Ymir PR #966](https://github.com/ymir-emu/Ymir/pull/966). This confirms a
repeatable authentic VDP2 title-state checkpoint, not the separate VDP1
title-menu command bundle: the one-frame VDP1 outputs did not match that
bundle's frame-497 hashes. It therefore does not admit the menu compositor,
input dispatch, title-to-menu transition, or Firestaff presentation.

The same system build was subsequently checked after its VDP1 trace was
reattached following save-state loading. The single captured frame produced a
complete `ymir-vdp1-capture-bundle-v1`: capture frame 0 contained 168 command
rows, its snapshot cursor was command 168 at address `0x00000cc0`, and the
bundle reported zero dropped rows. This proves that the Ymir CLI can now retain
frame-aligned VDP1 command evidence across this authentic state load. It still
does not identify the displayed menu, establish source ownership or palette
composition, or authorize a Firestaff renderer.

Ymir PR #966 also contains a bounded VDP2 VRAM/CRAM write tracer and an
export receipt. Its system build and the existing title-state VDP1/VDP2
checkpoint were regression-checked on TRV2. The VDP2 write tracer has not yet
produced a retained source-to-write witness for this menu, so it is tooling for
the next capture rather than an admission signal.

An input-driven retail probe sent the observed Start key route while retaining
the VDP2 trace. It produced 92,288 VRAM-write rows with no trace drops, but its
captured VDP1 cursor had only seven commands rather than the authenticated
title-menu bundle's 178. It is therefore negative routing evidence, not a
title-menu capture; its untracked raw dump was removed after inspection. Do not
use those writes to label a menu resource or relax a presentation gate.

`scripts/verify_nexus_ymir_title_state_checkpoint.py` verifies this narrow
checkpoint from caller-supplied retail CUE, BIOS, title-prompt state, and local
Ymir dump files. It pins every CUE-referenced retail track and validates the
register snapshot schema. The checkpoint pins VDP2 register bytes; it does not
use the emitted VDP1 register file as evidence because this capture record has
no independent VDP1-register digest. Nothing in this receipt establishes menu
or gameplay presentation.

## Deterministic Start-to-menu checkpoint (2026-10-26)

Ymir PR #966 adds a frame-aligned Control Pad 1 Start route for the existing
state/frame/dump capture path. On TRV2, a fresh temporary profile used the
authenticated Japanese retail CUE, BIOS, and title-prompt state in CLI slot 2.
It held Start during capture frames 0 through 59 and captured 120 frames with
`--debug --load-state 2 --capture-frames 120 --dump-memory
--capture-start-frame 0 --capture-start-hold 60`. The profile and raw dumps
were removed after inspection; the displayed title-menu image and VDP1 receipt
are retained in
[`nexus_v1_ymir_deterministic_start_menu_20261026`](../parity-evidence/verification/nexus_v1_ymir_deterministic_start_menu_20261026/README.md).

The capture completed at VDP1 frame 59 with command cursor 178 and 178 rows
for that frame. Its complete trace had 10,668 rows from frames 0 through 59,
with zero dropped entries. This is the same command count as the retained
retail three-choice title-menu capture, so it is reproducible emulator evidence
for the observed Start route rather than host-window timing evidence. The VDP2
title checkpoint remained byte-identical to the retained title-state reference:

| Output | SHA-256 |
|---|---|
| VDP2 VRAM | `de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c` |
| VDP2 CRAM | `db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de` |
| VDP2 registers | `79efa616c2db727749aa9018864d1ad32c1583935652fafb16534ba69f8e1f2d` |
| VDP1 capture receipt | `0e5cfa33070e33f24ae0e25ead7ce4fac561ae7c329ef7c59ff12382d3ea519e` |
| VDP1 trace | `b80c7a118e7f9df4f941804cdc045f4024336080504c512bbdf5c4e0df19d8d3` |
| Retained title-menu image | `0ad6bbc433936cd853fef476c8eb14be2c9532efcfcc218a43ca1053500ca55c` |

This makes title-menu capture reproducible without X11 keyboard injection. It
does not bind a retail menu resource to a Saturn consumer, prove the final
displayed pixels, or permit Firestaff to present a host-built menu. Those
production gates remain closed.

An independent 2,200-frame no-present replay on 2026-10-26 used the same
authenticated Japanese IPL, retail CUE, retained title-prompt state, and the
hash-verified four-champion BKR (`87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035`).
It scheduled Start, Down, A, and a second A at emulator-frame boundaries rather
than through X11. The BKR digest was unchanged after the run. Its final native
frame was the Dungeon Master Nexus title image, SHA-256
`1c224b148ec5c107569c1c9b30c5dfab24d8110d3e391b0bcb1089cd582dfb13`.
This is a timing boundary: those inputs occur before the title is receptive and
must not be interpreted as a verified menu or Continue route. The temporary
profile, dump, and screenshot were removed after inspection.

A later 3,400-frame replay moved the same Start/Down/A/A sequence after the
title boundary, but held every control for only eight emulated frames. Its
retail endpoint remained the title image (SHA-256
`8d495ffa4e8a76ab38a48c3c84a6fcc437db1bc4682aec6ac897609ed2307d95`), while
the BKR digest again remained unchanged. This is negative timing evidence: an
eight-frame post-title hold is not sufficient to establish a retail input
transition. It does not authorize a host menu, save route, HUD, or viewport.

A separate 3,400-frame debug replay kept the same post-title frame schedule
but extended each Start, Down, A, and second-A press to 60 emulated frames.
Its endpoint was still the native title screen, with SHA-256
`8d495ffa4e8a76ab38a48c3c84a6fcc437db1bc4682aec6ac897609ed2307d95`; the
four-champion BKR digest remained
`87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035`.
Both runs ended with the same final title-screen image hash. Extending the
post-title hold from eight to 60 frames did not change the final endpoint; no
claim is made here about intermediate menu or loading transitions. The
temporary profile, X display, memory dump, and inspection copy were removed
after verification. This remains negative emulator evidence only: it does not
establish menu, save, HUD, or viewport parity.

Ymir PR #966 additionally provides `--capture-screenshot` for the same
capture-owned route. It queues a profile-local framebuffer PNG only after the
requested frames and memory receipt have completed; it requires both a frame
capture and `--dump-memory`. A 60-frame retail boot check produced both the
VDP1 receipt and that PNG. The generated media was discarded after verification
and is not a Firestaff asset or parity admission signal.

Ymir PR #966 now also provides `--capture-no-present` for deterministic runs
where an X11/GLX compositor cannot swap GUI frames reliably. A fresh temporary
profile on TRV2 loaded the same authenticated Japanese BIOS, retail CUE, and
title-prompt state, then completed `--load-state 2 --capture-frames 1
--dump-memory --capture-no-present` under Xvfb. It wrote a complete memory dump
and exited with status 0 in under three seconds. The profile and dump were
removed after the check. This proves the capture transport no longer depends on
a successful GUI presentation for this state; it does not supply a framebuffer,
prove visual parity, or open any Firestaff presentation gate.

The same state was then captured with `--capture-screenshot` in no-present
mode. The completed dump was followed by a 640×448 RGBA PNG with SHA-256
`4b19474395c7192e7e68faf82efe0c240c9da213c4cd36c1ad91f31e68c17d82` before
Ymir exited. The temporary image was removed after its dimensions and digest
were checked. This validates non-present screenshot delivery for future visual
comparisons; it is not itself a pixel-parity admission or a Firestaff asset.

## Boot VDP2 writer anchor (2026-10-26)

Ymir PR #966 was extended with a bounded direct SH-2 VDP2-write trace and a
normal-disc boot capture route. A 60-frame `--debug --capture-boot-frames 60
--dump-memory --trace-vdp2-writes` run against the authenticated Japanese
retail CUE and BIOS recorded one direct master-SH-2 write: PC `0x0602e12e`
wrote the 16-bit value `0x0000` to VDP2 VRAM bus address `0x05e7fffe` at VDP1
frame 21. The current call-site was return address `0x06032c24`, targeting the
generic word-copy routine at `0x0602e120`. The trace had zero dropped entries.
Its CSV and receipt hashes were respectively
`47b98193f1b7615aad2326f5fc8386b502a214c6a244fece30e3a6580a0efc46` and
`7efd6c70e83aea4178fa013ce59dfe7149e05727c26e81b64f736897356646fd`.

This is a real SH-2 routine anchor for subsequent disassembly of a retail VDP2
writer. It does not identify the source resource, establish a complete upload
sequence, or prove title-menu ownership. The temporary memory dump was removed
after the PC, address, width, value, and receipts were checked.

Big-endian SH-2 disassembly of the captured high WRAM identifies the inner
call at `0x06032c20`: the wrapper masks `r4` with `0x0007ffff`, maps it into
the uncached Saturn bus (`0x25e00000 | r4`), and calls `0x0602e120` with a
one-word count. A static cross-reference at `0x060282d4` calls that wrapper
with destination `0x25e7fffe`. This explains the recorded address, but its
source is a stack temporary. It therefore remains an initialization anchor,
not source-resource provenance or permission to draw a host menu.

## Delayed title-menu confirm observation (2026-10-26)

An authenticated Japanese retail title-prompt state was replayed in Ymir with
the deterministic capture route for 360 frames. Start was held for frames
`0..59`; Control Pad 1 A was held for frames `240..249`. The captured final
frame was VDP1 frame 231 with 112 command records, rather than the earlier
title-menu capture's 139 records. A window capture made immediately after the
dump showed the retail door-opening transition; its SHA-256 was
`abdd1e9f65c0999d967fced4dad456856751cd9afcb46080616f617470ffc55a`.

This is an observed response to a delayed input schedule, not sufficient
proof that A semantically confirms New Game. A matching no-A control was then
run from the same retained state with Start held for `0..59` and no other
scheduled input through the same requested 360-frame capture. Its final
profile-local framebuffer was the native three-choice title menu, with
SHA-256 `20801648c86a5cbbe2a0e6c9cc90e625579fb52ba41f0b2234a51d3b0b17ecfe`.
The control capture produced an undropped VDP1 trace (32,028 rows), but its
bounded trace buffer ended at frame 179; it therefore cannot be used to
compare a final VDP1 command stream to the delayed-A capture. The framebuffer
is nevertheless emitted after the requested capture-owned frame loop, so this
is evidence that the observed door transition depends on the delayed A
schedule rather than on the Start-only title timeline.

This still does not establish which retail menu item A confirms, the next
stable game state, source-resource joins, or palette/compositor evidence. Do
not enable a host menu, gameplay, HUD, or synthetic confirmation rule from this
observation. The temporary screenshots, dumps, profile, and emulator process
were removed after their hashes and receipt limits were checked.

The same real state and input schedule was also captured for 600 requested
frames. Its final framebuffer showed the retail `Now Loading` screen (SHA-256
`07a6f3a612ca75209cd6308852a7cf25b3d1f35943ce2f5774e720376ef7c61c`), and
the VDP1 receipt retained 48,380 undropped trace rows through frame 391. This
extends the observed route from the door transition to the native loading
state. It does not yet reach or bind a stable playable level, the party pose,
viewport, HUD, level source, or palette/compositor route; it must not be used
to synthesize any of those Firestaff states.

After the no-present capture path was added to Ymir PR #966, the same
authenticated state and input schedule was repeated under Xvfb without GUI
presentation. The 600-frame run exited with status 0, wrote its memory dump,
and saved a 640×448 RGBA screenshot with the same `Now Loading` SHA-256
`07a6f3a612ca75209cd6308852a7cf25b3d1f35943ce2f5774e720376ef7c61c`.
Its VDP1 trace and receipt digests were respectively
`d529f883b686b7c3fc83db929e3e1b476527799ca3fbda35ba41a4ef48a1079e` and
`45d384f8b6d8c4a5ab48941bcea462cbe1624ea57f8679ebfe4f8c8c1baf3a22`.
The temporary profile, dump, screenshot, process, and result receipt were
removed after verification. This independently reproduces the observed loading
checkpoint but does not extend its presentation admission.

Ymir PR #966 also adds `--capture-save-state N`. A real title-prompt capture
saved slot 3 after its dump, and a second isolated Ymir process loaded slot 3
and completed a further one-frame dump. This makes long retail transitions
inspectable through short, repeatable follow-up captures. The temporary state
and both profiles were removed after the round trip; a saved emulator state is
diagnostic transport, not a Firestaff asset or an admission signal.

An authenticated 900-frame Start/A capture saved such a checkpoint and reached
a 640×448 first-person dungeon viewport with SHA-256
`8ede95f1fd777eda69ebcfc91d0268194e501f145306eb1fc2971ed0113a2274`.
Reloading slot 3 in a fresh isolated process and capturing one frame reproduced
the same digest exactly. The image has no party HUD, so it is evidence for this
narrow rendered viewport checkpoint only. It does not identify the level or
room source, establish input semantics, authorize a host compositor, or prove
the HUD route.

On 2026-10-26, the system-wide Ymir build on TRV2 reproduced that route from
the retained Japanese title-prompt state: Start held for capture frames `0..59`,
then A for `240..249`, over 900 `--debug` frames. The framebuffer again hashed
to `8ede95f1fd777eda69ebcfc91d0268194e501f145306eb1fc2971ed0113a2274`; the
isolated four-champion BKR remained
`87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035`.
Saving that endpoint in CLI slot 3 and reloading it in a second isolated
profile reproduced the framebuffer hash after one frame. Both temporary
profiles, dumps, screenshots, and X displays were removed. This verifies the
current Ymir capture tooling and the bounded retail viewport route, not a
Firestaff renderer, menu consumer, or HUD admission.

## Bounded candidate-state-window SH-2 read witness (2026-10-26)

Using the same hash-pinned Japanese retail CUE, BIOS, title-prompt state, and
four-champion BKR, an isolated system-wide Ymir run advanced the observed
Start/Down/A route, saved a fresh slot-3 checkpoint, and replayed one debug
frame from that checkpoint. The source BKR remained
`87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035`
before and after both captures. A bounded SH-2 data-read trace over
`0x060476d0..0x06048000` recorded twelve master-SH-2 16-bit reads, all in the
`0x06047ca0..0x06047cb6` range, without dropped rows. The captured checkpoint
has SHA-256 `48ca0feb12432dde6748124a9e639ad4e466efc307520268356a16c00fd5b2ce`;
the retained CSV has SHA-256
`e45ad64fa89ce263b51c0579167a35fcfacdce840a438854b48b8df5100dd9ee`.

Disassembly against the retained high-WRAM image identifies two real 16-bit
field reads at offsets `+4` and `+6` from the observed eight-byte records, and
the caller dereferences a table entry in the same address range. For example,
the trace reads the `0x0124` and `0x00b0` fields from the record beginning at
`0x06047cb0`; the next observed records have the same packed layout. This is
evidence of structured runtime state, not evidence of a particular UI role.
The high-WRAM image used for this disassembly has SHA-256
`344b6a15bdac95a8e698c9a9ccc336f6f225fb3a4c60eec9788102c57b2e197d`.

The raw CSV, its completed Ymir bundle, VDP1 command trace, VDP register
snapshot, VDP2 registers, high-WRAM image, and checkpoint are retained in the
TRV2 verification store as `nexus_v1_ymir_sh2_read_trace_hud_20261026`. This
is an authentic execution witness for the specified RAM window. It is not an
ownership join from a Firestaff source resource to a HUD element, does not
label those reads semantically, and does not permit a host HUD, viewport
compositor, save route, or input implementation.

## SH-2 to VDP1 command handoff (2026-10-26)

Ymir PR #966 now exposes a bounded SH-2 data-write trace independently of the
older VDP2-only trace. A two-frame replay from the same authentic checkpoint
traced physical VDP1 RAM at `0x05c00000..0x05c80000`. It retained 176
master-SH-2 writes with no drops. In frame 0, the master SH-2 completed the
command at physical address `0x05c000a0` by writing `0x5000` to offset zero
and `0x0018` to offset two. In frame 1, the VDP1 command trace executed the
command at VDP1 address `0x00a0` with word 0 `0x5000` and word 1 `0x0018`.

This is a same-checkpoint, temporal CPU-to-VDP1 handoff: it proves that the
retail SH-2 authored the VDP1 command consumed on the next captured frame. The
two-frame SH-2 write trace and VDP1 command trace have SHA-256
`58c9d41740a75063543c70b6cb11e336268cfb47486df1ffdcbe19718f12bc66` and
`c93f3a8f86263ded3b79d081f15771bce4331ab31ec6a0ce37d3b4691b628f32`
respectively. The corresponding completed receipts are retained with the
checkpoint. This does not identify the source resource, texture, palette, or
semantic role of that command, so it does not by itself admit a Firestaff HUD,
viewport, or renderer.

## Title-menu VDP1 source-span join (2026-10-19 capture; analyzed 2026-10-26)

A read-only source-join analysis on TRV2 used the retained Ymir v0.4.0
Japanese title-menu VDP1 capture (VRAM SHA-256
`0f484877f5c5ef300fcbcaec6948b50c8b7aed7945e448a9f8281b2da46a2f21`) and
read the Japanese retail CUE/ISO members directly from the authentic Nexus
media root. The analysis identified 137 active texture draws: 124 indexed
image/CLUT pairs exactly matched to `LEV00.DGN`, eight direct-color spans
matched to `LEV00.DGN`, and five spans matched to `TITLE.BIN`. No active draw
was left byte-unbound. Of the 124 indexed DGN draws, 108 had a matching
static FACE selector; 16 did not.

This narrows source-byte provenance for that single Ymir menu frame. It does
not establish SH-2 upload ownership, the semantic use of the 16 draws without
static FACE selectors, menu input progression, complete pixel composition,
or production renderer permission. It is a different capture from the 2026-10-25
VDP command bundle above. Keep Nexus startup gated until a same-session source,
input, command, palette, and displayed-frame route is bound; do not infer that
`MENU.BPK` owns these observed draws or that it is unused in other frames.

The five matched `TITLE.BIN` source spans fall within the verified `TITL`
resource-directory entry with class-local id 3 (record beginning at file
offset `0x0d068`, before the `MAPD` record at `0x0e278`). This adds record-level
ownership only. The exact subresource role, upload path, and menu label/pixel
association remain unbound. The 16 `LEV00.DGN` draws without static FACE
selectors likewise have byte and material matches, but no evidenced semantic
owner; do not label them from dimensions or geometry alone.

The BKR parser has been checked against all four hash-pinned images in the
private corpus, which are reported as retail-written. It reproduces each
20,480-byte payload hash from the existing read-only Python reader. These
images use 64-byte card blocks (512 blocks per 32 KiB image); each game entry
claims 354 blocks. The parser deliberately returns payload bytes without
interpreting game fields. This supports container-level Continue work but is
not yet a save-load path into the Nexus engine. The opaque Saturn capture
receipt remains a separate API and is not a BKR parser. The BUP used in the
Ymir Continue run has no retained transfer chain, so its retail provenance is
reported rather than independently established.

## Retail title-menu evidence (2026-10-25)

An isolated Ymir `--debug` run with the authenticated Japanese BIOS and retail
disc reaches the native Nexus title menu. The retained 2026-10-25 screenshot
shows the three actual choices: `NEW GAME`, `CONTINUE`, and `CONFIGURATION`,
with New Game selected. Its SHA-256 is
`d1ec79c6b1e3abcd16837781a1212eedb3cb157562356593ab3ce069b18f2ab4`.
The 2026-10-24 keyboard run records Start opening the menu and confirming
`NEW GAME` entering the dungeon. In a separate fresh-profile attempt, selecting
Continue left the screen unchanged because no save was present; that attempt
did not verify save loading.
Together, these are retail startup/input observations, not Firestaff parity.
The capture environment, key mapping, and per-action limitations are recorded
in [`parity-evidence/verification/keyboard-mouse-debug-20261024/README.md`](../parity-evidence/verification/keyboard-mouse-debug-20261024/README.md).

## Retail Continue with an authentic save (2026-10-25)

A second isolated Ymir `--debug` run used a reported retail-written 32 KiB
Saturn backup image (SHA-256
`87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035`) with the
same hash-verified Japanese retail disc and BIOS. The title menu showed
`NEW GAME`, `CONTINUE`, and `CONFIGURATION`. Selecting `CONTINUE` opened the
retail `LOAD` panel, which listed the internal-memory entry `B01 0:06 NORMAL`.
Confirming it displayed `Now Loading`, then entered the dungeon with the
four-champion HUD. The BUP image retained its original SHA-256 after loading.

The isolated profile was `/dev/shm/ymir-nexus-real-bup-menu-20261025`; the
source/transfer chain was not retained, so the save's retail provenance is
reported rather than independently established. The source BUP was copied into
that profile and was not edited directly. Ymir 0.4.1
ran with `--debug` on TRV2 against the Japanese retail cue (SHA-256
`878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`). The
retained screenshots and their hashes are in
[`parity-evidence/verification/nexus-continue-real-bup-20261025/README.md`](../parity-evidence/verification/nexus-continue-real-bup-20261025/README.md).
This demonstrates the retail save-load route under Ymir with a BUP reported as
retail-written. Because its transfer chain was not retained, the save's retail
provenance is not independently established. It does not prove Firestaff
implements that title menu or save selection; the production startup route
remains blocked by the consumer-evidence gates described below.

On 2026-10-26, a fresh TRV2 Ymir profile copied the same hash-verified
four-champion BKR and replayed the retained Japanese title-prompt state with
Start at frames `0..59`, Down at `180..189`, and A at `240..249`. At frame 900,
the authentic framebuffer showed the native Japanese `LOAD` panel and its
`B01 0:06 NORMAL` internal-memory row (SHA-256
`b95cdc24ae30705648533dba1e1a1a304196a8fbb449bd069df513be85c0928b`). A
saved checkpoint was loaded into a second isolated profile; A at frames
`0..9` then reached a four-character HUD plus first-person viewport at frame
900 (SHA-256
`68de1c7898d79d70d7f3b9fd8f220485fd5ffa75b8aefad9f491eeb3d16a2a76`). Both
profiles retained the BKR SHA-256
`87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035`.
This authentic two-stage route binds a real UI checkpoint and HUD witness for
future capture consumers; it does not yet admit a Firestaff menu compositor,
HUD compositor, or save-format implementation. The profiles, dumps,
screenshots, and X displays were removed after verification.

Firestaff's source-level startup helpers do not implement the retail title
menu. `nexus_v1_startup_boot_handle_input` uses the synthetic warning/title
schedule and selects a host save or champion action from the available slot
mask. That helper logic is not proof of a reachable production menu: the
authentic launch route remains blocked by its menu-surface and consumer
evidence gates, as described below. The retail title instead opens its three-choice menu
before New Game or Continue proceeds. Configuration behavior and the Saturn
consumer that draws the retail menu have not yet been bound to a same-session
VDP1/VDP2 capture. Do not treat the existing host save/champion panels as
equivalent to the retail title menu, and do not synthesize the missing choices
or Saturn presentation from decoded MENU.BPK surfaces alone.

## Runtime boundary

`firestaff_nexus` must not present a host-built title, champion menu,
portrait placement or a text footer as though it were Saturn output. M11 handoff
and viewport remain fail-closed until authenticated Saturn captures bind the
relevant resource, palette, destination, input action, and timing. Startup has
separate gates for title presentation, title-to-menu dispatch, menu surface
presentation, New Game/Continue selection, and start-pose/level handoff. Passing
a decoder or later handoff gate does not prove the earlier transitions.

Champion portraits are a later champion-select consumer question, not evidence
that the title menu is ready. A future champion-select capture must bind a PLRD
row to its FACE ordinal, VDP1 command destination, palette/CRAM state, and
displayed frame timing. Presence or hash of `FACE.BIN` alone cannot relax that
gate.

## Title-menu evidence (2026-10-25)

An isolated Ymir 0.4.1 `--debug` run with the authenticated Japanese BIOS and
retail disc shows the native title menu after holding Start. The screen contains
`NEW GAME`, `CONTINUE`, and `CONFIGURATION`; New Game is initially selected.
Down selects Continue, and A confirms New Game into the dungeon. A fresh profile
with no game save leaves Continue on the same menu and does not prove a save-load
route. This establishes retail behavior only; it is not Firestaff parity.
Capture details and screenshots are recorded in
[`parity-evidence/verification/keyboard-mouse-debug-20261024/README.md`](../parity-evidence/verification/keyboard-mouse-debug-20261024/README.md).

The historical direct-CLI probe in
[`parity-evidence/verification/nexus_cli_direct_start_block_20261024/README.md`](../parity-evidence/verification/nexus_cli_direct_start_block_20261024/README.md)
reached the ready title frame but reported `menu=0` and `levelLoaded=0` after
one scripted Return. It did not start Nexus. That receipt names a MENU.BPK PRS3
capture blocker for that build; it should not be read as proof that decoding
MENU.BPK alone implements the title-menu transition.

An inspection of the hash-verified Japanese retail `MENU.BPK` confirms that its
162 PRS3 surfaces decode, and visually identifies save/load panels, Japanese
configuration panels, labels, and small button glyphs. The inspected contact
sheets did not identify the three English title-menu choices shown by Ymir.
This is a content observation, not proof those assets are absent from the game
or that MENU.BPK is unrelated to the title menu. Surface ordering, CLUT,
consumer, placement, and input-to-action dispatch still require a same-session
Saturn trace.

## Native title-menu command/source observation (2026-10-27; startup provenance limited)

A retained Ymir menu-state capture has a frame-aligned VDP1 VRAM snapshot and
a complete command trace. The available record does not establish that its
three-choice screen followed a fresh cold boot in the same run; treat it as a
native menu-state reference, not cold-boot-to-menu proof. In that captured
menu frame, five textured VDP1 commands reference byte spans that each match
exactly once in the authentic Japanese `TITLE.BIN`: two 24×24 spans at file
offset `0x15f6` and three 4bpp text-sized spans at offsets `0x0446`, `0x0576`,
and `0x07d6`. The screenshot shows paired selector graphics and three menu
labels, making these strong candidate source spans for those visible elements.
This observation does not establish each source span's semantic identity, the
Saturn upload route, or the palette's source owner.

The same capture's VDP1 VRAM was checked against every decoded surface in the
hash-pinned Japanese `MENU.BPK` (SHA-256
`740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636`). Although
162 PRS3 surfaces decode, none of the captured textured command source starts
falls within an exact matched MENU.BPK surface range. This narrows the current
menu asset lead toward `TITLE.BIN`; it does not prove MENU.BPK is unrelated or
that Firestaff can yet compose the menu. The captured data and trace remain
external at
`/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/`.

The four distinct 32-byte VDP1 CLUT blocks used by these commands were also
compared directly and word-swapped against all 126 hash-accepted Japanese
retail files in the local corpus. None matched; five files with unaccepted
hashes were excluded. A full VDP1-window SH-2 write trace hit its 250,000-row
limit and dropped 45,183,915 master-SH-2 rows, so it cannot establish upload
ownership. A second cold boot narrowed tracing to `0x05c29000..0x05c48000` and
completed 205,300 rows with zero drops. The final VDP1 VRAM hash matched the
visual capture, but the trace values do not reconstruct the final texture
spans and the trace has no per-frame markers. Thus the SH-2 writer path and
CLUT source remain open.

The narrowed trace provides disassembly anchors in authentic Japanese
`DM.BIN` (SHA-256
`3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`) at load
base `0x06010040`: the recorded call target is `0x0602623c` (file offset
`0x161fc`), with writes observed at `0x0602625c` and `0x0602626c` (offsets
`0x1621c` and `0x1622c`); the trace's caller-return field is `0x06027064`
(offset `0x17024`). These instructions are concrete reverse-engineering
targets, not yet an authenticated asset-upload routine. The four captured
32-byte CLUTs also have no direct or word-swapped exact match in the 126
hash-accepted Japanese retail files, so their palette source remains unbound.

FONT256's authenticated Character Generator bytes now remain in the engine's
source object for the future Saturn consumer. This does not admit glyph coding,
page/PND attributes, layer placement, or text writing; `font_loaded` remains
closed until those parts are bound by the same runtime witness.

## Debug boot memory receipt (2026-10-26)

A system-built Ymir 0.4.2-dev run on TRV2 booted the hash-verified Japanese
retail cue for 600 deterministic frames with `--debug`. It used the verified
Japanese BIOS (SHA-256
`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`) and
recorded bounded SH-2 writes to `0x05c00000..0x05c80000`. The resulting VDP1
VRAM dump is SHA-256
`0e0a67c1eec3c0bf61e7358ded889d3df343a5cb7bd1154b6422636d57bf1d74`.

The capture used Ymir's explicit no-render mode so that it could retain real
emulated memory and CPU-write evidence without producing a synthetic host
frame. It must not be used for visual parity, title/menu placement, or HUD
claims. The raw dump and write trace are retained outside the repository on
the external evidence volume; only their reproducible receipt is recorded
here.

## Long boot memory trace (2026-10-26)

A separate Ymir `--debug --capture-boot-frames 13000` run retained a full
memory snapshot after normal disc boot. The final VDP1 counter was 12947; its
complete command trace had four terminal records and no VDP1 trace drops. The
bounded source scanner found no supported textured draw commands in that
final frame. VDP2 had NBG0, NBG1, and NBG2 enabled (`BGON=0x0007`,
`CHCTLA=0x1212`). The SH-2 write trace reached its 250,000-row cap and reports
420,006 dropped master-SH-2 rows, so it cannot establish complete writer
provenance.

Both NBG0 and NBG1 were 8bpp bitmap layers at VRAM offsets `0x00000` and
`0x20000`. Comparison with the authentic retail `TITLE.BIN` and `MENU.BPK`
data found no exact pixel-span or palette joins. The largest partial byte
matches were 118 bytes to `TITLE.BIN` map 4, 99 bytes to a repeated
`MENU.BPK` surface sequence, and 35 bytes to `TITLE.BIN` maps 1 and 3; these
do not prove asset ownership. A diagnostic decode of the captured NBG0 bytes
and CRAM shows Japanese date/time settings, the date 2026-10-26, and Cancel
/ Confirm labels. This looks like a system date/time page and is not evidence
that Nexus reached its title menu.

This capture used `--capture-no-render`: both VDP1 framebuffers are zeroed and
there is no screenshot. It is useful as a register and memory checkpoint only;
it does not identify what appeared on screen or prove that Nexus reached a
particular game menu. The raw files, SHA-256 list, and clearly labelled layer
diagnostic previews are retained off-repository.

## Captured title-menu frame reconstruction (2026-10-26)

The atomic frame-59 Ymir capture loaded a previously saved Japanese retail
title-prompt state, held Start for 60 of 120 capture frames, and dumped memory
and the VDP1 trace in the same run. It was not a cold boot. A bounded C replay
combines the retained VDP1 display framebuffer, NBG0 8bpp bitmap, CRAM,
registers, and verified zero back-screen table. Its output matches all 286,720
pixels of the hash-pinned 640×448 Ymir screenshot exactly. This confirms static
composition for the captured menu frame; the screenshot was saved after the
memory dump, so exact frame identity is still unproven.

The replay remains capture-only and does not feed the Firestaff viewport or
establish cold-boot-to-menu behavior, title-to-menu input handling, or gameplay
HUD. Separately, a 13,000-loop cold boot with `--capture-no-render` stopped on
a Japanese date/time settings page. That run has no screenshot and incomplete
SH-2 write tracing; it does not prove that Nexus reached the game title. See
the boot-dump receipt on the external evidence disk and
[full capture receipt and SHA-256 identities](NEXUS_MENU_BPK_VDP1_SNAPSHOT_JOIN.md#full-ymir-title-frame-capture-replay)
for the menu-state reproduction command and exact scope.

## Deterministic cold boot reaches the native title prompt (2026-10-27)

Ymir 0.4.2-dev was built on TRV2 with the repeatable
`--capture-pad-input FRAME:BUTTON:HOLD` option (submitted to the Yeager Ymir
fork in [PR #1](https://github.com/yeager/Ymir/pull/1)). A fresh isolated Ymir
profile used the authenticated Japanese Saturn BIOS
(SHA-256
`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`)
and the real Japanese retail cue
`/home/trv2/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue`. The
isolated profile was marked onboarded to bypass Ymir's first-run UI; no game
state or saved game was loaded.

The deterministic boot schedule pressed Right at frames 700, 720, 740, 760,
780, and 800, then A at frame 830 to confirm the Japanese BIOS date/time
screen. It sent Start for 2 frames at frame 7000 to advance past the opening
intro, then held Start for 60 frames at frame 9000 on the title prompt. The
9,120-frame rendered capture shows the native `PRESS START BUTTON` title
prompt. It does not show the three-choice `NEW GAME` / `CONTINUE` /
`CONFIGURATION` menu. The run loaded no emulator state and no game save, so it
verifies cold boot to the title prompt only.

The emulator was Ymir 0.4.2-dev with `--debug`, built on TRV2 from the branch
that adds repeatable `--capture-pad-input FRAME:BUTTON:HOLD` scheduling. The
authentic Japanese cue SHA-256 is
`878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`. The
controller-scheduler change is submitted to the Yeager Ymir fork in
[PR #1](https://github.com/yeager/Ymir/pull/1).

The rendered title-prompt screenshot, complete raw memory dump, source-join
report, and bounded SH-2 write traces are retained on the external evidence
volume. `SHA256SUMS.txt` covers the screenshot, all 30 dump files, the Japanese
`MENU.BPK`, the source-join report, and both write-trace captures. The separate
three-choice menu screenshot is a retained menu-state reference whose clean
cold-boot provenance is not established.

| Artifact | Location | SHA-256 |
|---|---|---|
| Native three-choice menu-state reference (not cold-boot evidence) | `/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/title-menu.png` | `8721cce3354bc20b1cd367fd44be3d4ecc60ca65fa89c7d00654e83f5ad7db23` |
| Source-join report | `/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/vdp1-source-join.txt` | `5df73daf4a37f187e0f3e4805d68d7f4870703839e5fb9b2d4c26f8cf12dc3a3` |
| Complete narrowed SH-2 write trace | `/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/sh2-write-narrow/sh2-memory-write-trace.csv` | `53502102c1dcdc0c511983bfaf0ce7e017fe711f26a393e353eea162a82f001d` |
| Trace receipt | `/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/sh2-write-narrow/sh2-memory-write-trace-bundle.txt` | `c463f5d77a57c7883fd2e22ae3bf69aa41823264ad25f804f0535b634b560166` |
| Dump and screenshot manifest | `/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/SHA256SUMS.txt` | `30d4c019f5dccf090553a1e9328b185c1e90305d5c89f50a2f554fef87bc403b` |

The exact command was:

```sh
ymir-sdl3 --debug --capture-boot-frames 9120 --dump-memory --capture-screenshot \
  --capture-pad-input 700:right:2 --capture-pad-input 720:right:2 \
  --capture-pad-input 740:right:2 --capture-pad-input 760:right:2 \
  --capture-pad-input 780:right:2 --capture-pad-input 800:right:2 \
  --capture-pad-input 830:a:2 --capture-pad-input 7000:start:2 \
  --capture-pad-input 9000:start:60 \
  "/home/trv2/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue"
```

This closes the Ymir cold-boot-to-title-prompt evidence gap only. It does not
establish cold-boot entry to the three-choice menu, Firestaff menu rendering or
control, a Firestaff `NEW GAME` route to a playable level, real-save loading
through Continue, or HUD parity. Those remain separate production-parity
gates.

## Cold-boot follow-up: exact retail FMV frame match (2026-10-27)

A rendered 19,000-frame Ymir run used the authentic Japanese retail disc and
Japanese IPL in the isolated TRV2 profile. It loaded no emulator save state or
game save; the profile retained its BIOS setup configuration. With `--debug
--fast-forward`, Start was held for two frames at capture-loop frames 9,000,
13,000, and 15,000.

The endpoint screenshot is an exact match to zero-based frame 626 of the
authentic Japanese retail `DMV0.AVI` video. The AVI SHA-256 is
`eafb4c638db9ffada9e929d82299562e0a9d3eb73fb2a47858c0de0dc0fa57bb`; it is
RIFF AVI using TrueMotion 1.0 at 320×180 and approximately 15 fps. The native
Ymir screenshot is 640×448. Its centered crop `image[42:402:2, ::2]` matches
all 57,600 decoded frame pixels after Saturn RGB5 quantization (`& 0xF8`),
with zero mismatches.

The final VDP1 capture is frame 15,907, with four command rows and 785,986
retained trace rows with zero drops. The video draw at command offset `0x40`
uses PMOD `0x0028` (colour mode 5), SRCA `0x8f80` (VRAM byte offset
`0x47c00`), and SIZE `0x28b4` (320×180), placing the image at y=21–200.
Capture-loop frame 19,000 and VDP1 frame 15,907 are separate counters. The
SCU DMA trace retained 16,384 rows but dropped 4,485
(`trace_data_complete=0`), so it cannot provide complete transfer provenance.

This confirms authentic full-screen video output, likely the opening FMV in
this boot sequence. It does not prove which input selected or started the
video, the three-choice menu transition, a stable playable level, gameplay,
or HUD.

The Ymir executable SHA-256 is
`f5d94ee84762d81e97f950d6cfc4253b4f72c61dc5c4f376b4296235ed46dcc3`. At
inspection, its source worktree was based on commit
`534ed10c6f7f13a3ba752476eea191be3ab553b4` with tracked-diff SHA-256
`2442944df961bf0079f61a032b2ca09f3252be4416a28471aee72573b49e4d5d`. These
identify the observed executable and source worktree state; they do not
independently prove the binary was built from that exact source snapshot.

The screenshot, raw Ymir dumps, run log, reproducible RGB5 comparison, and
retail TM.BIN disassembly receipts are retained at
`/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_rendered_20261027/rendered_menu_candidate_19000/`.
The screenshot SHA-256 is
`903e710b455df66a1df79adcdf1e1a9a6c8fdc257833f0eed9e80def840d541a`; the run
log SHA-256 is
`caf636e378d9888b4c97e01b12ddd864b0564e292fceb6f5ffc97b14f442d282`; the
`DMV0_FRAME_MATCH.txt` SHA-256 is
`51081a26e28878ac6f90a102fe79c76a51c519d3aba5c23f7e9b4612364d298f`; the
`TM_BIN_AVI_DISPATCH.txt` SHA-256 is
`884a62d68879edec2239fcafb0addec581c3e27ae252ddc606c4240303658d40`;
`README.md` SHA-256 is
`42ee176d4a06c539864fbcd4e68a176ea1ea579627be31ea0699faa5d0ade67a`; and
`SHA256SUMS.txt` SHA-256 is
`6d831e2f30bbf32031ae0586f152a5ee0aef80d67cec0183a76fba8a7ed27ae1`. All 37
listed bundle files passed checksum verification. This is diagnostic evidence;
it is not a Firestaff asset or a parity admission.

## Retail TM.BIN video dispatcher disassembly (2026-10-27)

The authentic Japanese retail `TM.BIN` is 160,044 bytes with SHA-256
`d87485fe6eba1f6e9fbbf487f5fcdd994911136905e6172e5bb5bc0122407eb6`.
With load base `0x06010000`, the three embedded video-name strings map to:
`DMV0.AVI` at file offset `0x1968c` / runtime `0x0602968c`,
`DMV1.AVI` at `0x19698` / `0x06029698`, and `DMV2.AVI` at `0x196a4` /
`0x060296a4`.

The module entry routine at file offset `0x00` reads the selector at SH-2
address `0x00200000` at PC `0x0601006c`. It branches on values 0, 1, or 2 and
loads the corresponding AVI pointer from its literal pool at offsets
`0x260`, `0x264`, and `0x268`. It calls the shared playback routine at
`0x06010eec`; the DMV0 branch passes `0x000a0000` in `r5`. After the shared
playback call returns, the routine makes further calls and reads the word at
`0x06029978`. If it is not 2, control branches to `0x060100e0`. If it is 2,
the code tests the word addressed by `r14`: when nonzero, it clears byte
`0x0020000b` and branches to `0x060100e0`; when zero, it loads a call target
from `0x0600026c` and calls through that pointer. These are static control-flow
observations, not proof of the values used during cold boot or the following
menu transition.

The retained excerpt was produced with GNU objdump's `-m sh4` decoder setting.
The retail Saturn CPU is SH-2; this evidence claims only the displayed opcode
bytes and their compatible control flow, not an SH-4 execution target. The
reproducible command and raw output are retained at
`/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_rendered_20261027/rendered_menu_candidate_19000/TM_BIN_AVI_DISPATCH.txt`.

This is high-confidence static evidence of a retail three-video dispatcher and
shared playback call. Together with the exact DMV0 frame-626 pixel match above,
it connects the rendered screenshot to the authentic video path. It does not
show which caller sets selector `0x00200000` during cold boot, establish
selector 0 as the startup default, identify the Start skip behavior, or prove
how the three-choice menu is entered. The load base is supported by the AVI
pointer arithmetic and the documented TM overlay mapping at file offset
`0x4500` to runtime `0x06014500`; it remains a disassembly load hypothesis.

`DM.BIN` contains resource-string anchors for `MENU.BPK`, `yam\menu.c`,
`FONT256.S2D`, and `STABG.BIN` at offsets `0x373b4–0x373d8`, and
`TITLE.CG` / `TITLE.BIN` at `0x38e6c` / `0x38e78`. These strings do not
identify the title-menu entry routine. The next useful dynamic trace should
capture SH-2 call/branch edges and selector/return globals around cold-boot
loop frames 6,900–7,100 and 8,900–9,120, then correlate them with Start reads
and frame-indexed VDP1 commands.

## Complete cold-boot SCU DMA trace (2026-10-27)

A follow-up run used Ymir's explicit SCU DMA trace route with the same
hash-verified Japanese retail cue, BIOS, and deterministic cold-boot pad
schedule described above. Ymir was run with `--debug --capture-boot-frames
9120 --dump-memory --capture-no-present --capture-no-render --trace-scu-dma`.
The emulator loaded no save state or game save. Input timing was recorded in
the retained run log. The full capture retained 8,399 SCU DMA transfers in an
8,400-line CSV including its header; the manifest reports a 16,384-row
capacity, zero dropped rows, no overflow, and complete trace data.

The retained SCU DMA table records transfer metadata; it does not contain the
transferred payload bytes. The listed source addresses were compared with the
final captured high-WRAM image, final VDP1 VRAM, and the authentic Japanese
`TITLE.BIN` (SHA-256
`51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3`,
112,216 bytes). Several final high-WRAM spans at the recorded source addresses
match exactly:

| Transfer | Captured WRAM source | VDP1 destination | Size | Exact `TITLE.BIN` offset |
|---:|---:|---:|---:|---:|
| 3777 | `0x060ab376` | `0x05c44060` | 288 bytes | `0x15f6` |
| 3764 | `0x060aa1c6` | `0x05c47de0` | 256 bytes | `0x0446` |
| 3765 | `0x060aa2f6` | `0x05c479e0` | 256 bytes | `0x0576` |
| 3767 | `0x060aa556` | `0x05c46f60` | 416 bytes | `0x07d6` |

The 288-byte source address of transfer 3778 (`0x060ab4c6`) corresponds to
VDP1 VRAM address `0x05c43be0` in the trace, but its final WRAM bytes do not
match the cited `TITLE.BIN` span; its source ownership remains unassigned. The
SCU trace records transfer order, source/destination addresses, and counts,
not payload bytes or a per-transfer frame number. These comparisons are
final-state byte/address evidence from one deterministic run. They do not
prove that a DMA payload matched `TITLE.BIN` or timestamp a transfer to the
VDP1 menu frame.

The external evidence bundle is retained at
`/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/scu_dma_complete_20261027/`.
Its manifest SHA-256 is
`5b2ede0748d3a4b009f4d6d781e1fbc9897c114be270e69a567daf277cefed36`; the
trace CSV SHA-256 is
`718f10d71fcae26437224362abf245f594fdb07d15cb3732ddebbd4d7322039c`. The
bundle's `SHA256SUMS.txt` was checked against every retained artifact. The
same trace functionality is proposed in [Yeager Ymir PR
#2](https://github.com/yeager/Ymir/pull/2), stacked on its deterministic input
capture PR #1.

The retained README does not record the Ymir source revision used to build this
capture. The TRV2 worktree inspected during this follow-up has base commit
`bf68a2fdb13e533e7b6c6c9a511701ec52609220` and an uncommitted tracked diff
whose SHA-256 is
`ec59cfa728cb73f032836eaa5d8782bdfdf1bc1702d906af6b3ef627a2e95082`. This
identifies the inspected worktree state, but the retained capture does not
bind that state to the executable; the exact build revision is therefore
unverified.

This adds reproducible evidence linking recorded DMA source addresses to
matching bytes in the final WRAM snapshot for the authentic cold boot. It does
not prove the payload bytes transferred, label the exact semantic role of
each texture, bind the palette source, prove transfer-to-frame timing, or
admit a Firestaff title-menu renderer. The existing production gates remain in
force.

## SCU DMA transfers indexed by deterministic capture frame (2026-10-27)

A follow-up authentic cold-boot run added a zero-based CLI capture-loop frame
to every SCU DMA transfer row. It used the same hash-verified Japanese retail
disc and BIOS, with no save state or game save. The capture ran 9,120 frames
with `--debug --capture-no-present --capture-no-render --trace-scu-dma` and the
same deterministic date/time, intro-skip, and title-prompt inputs documented
above. The no-render run has no screenshot and cannot establish displayed
pixels.

The SCU manifest reports 7,986 retained transfers, zero dropped rows, and
complete trace data. The CSV has 7,987 lines including its header. Transfer
rows 3347, 3348, 3350, and 3360 are each indexed to capture-loop frame 7564.
At each listed source address in the final high-WRAM snapshot and destination
range in the final VDP1 VRAM snapshot, the bytes independently match the
corresponding authentic Japanese `TITLE.BIN` span. The metadata row counts and
addresses align these snapshots; the trace itself does not contain transfer
payload bytes:

| Transfer | Capture-loop frame | WRAM source | VDP1 destination | Bytes | `TITLE.BIN` offset |
|---:|---:|---:|---:|---:|---:|
| 3347 | 7564 | `0x060aa1c6` | `0x05c47de0` | 1,024 | `0x0446` |
| 3348 | 7564 | `0x060aa2f6` | `0x05c479e0` | 1,024 | `0x0576` |
| 3350 | 7564 | `0x060aa556` | `0x05c46f60` | 1,664 | `0x07d6` |
| 3360 | 7564 | `0x060ab376` | `0x05c44060` | 1,152 | `0x15f6` |

These values are capture-loop frames, not VDP1 frame numbers. The VDP1 receipt
for this dump ends at VDP1 frame 8037 with 178 commands for that frame; the
run does not provide an exact mapping between the two counters. The data
therefore strengthens byte provenance and deterministic timing, but does not
assign semantic labels to texture spans, identify palette ownership, or prove
a transfer-to-displayed-frame relation.

The raw dump bundle and reproducible receipt are retained at
`/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/scu_dma_frame_index_20261027/`.
Its README SHA-256 is
`65c2fe206b937c3dc2a4e09cb1626389db458c9b9f5ca89a01b678a044a07c7d`, its
checksum-list SHA-256 is
`e9716f9f444789cba25613cab128e8a92a1400a49888f0948e3ac315e359f849`, and the
SCU CSV SHA-256 is
`77e3e2e1e667956092e1fd75b183e5e0929f1dd698d22364171be90561e4a4bb`. All 31
dump files were verified after transfer to the external evidence disk.

This capture uses the frame-indexed exporter proposed in [Yeager Ymir PR
#2](https://github.com/yeager/Ymir/pull/2). It remains diagnostic evidence;
the Firestaff title-menu rendering and interaction gates stay closed.

## SCU DMA to VDP1 frame correlation (2026-10-27)

A subsequent run used Ymir's trace-bundle v4, which records the VDP1 frame
index immediately before and after each deterministic capture-loop frame. Four
SCU DMA transfers at capture-loop frame 7564 all have VDP1 frame 6890 both
before and after the emulated frame. This pins the transfers to VDP1 frame
6890, rather than estimating their VDP1 time from the overall capture length.

The final high-WRAM source spans and final VDP1 VRAM destination spans for
transfers 3347, 3348, 3350, and 3360 independently match authentic Japanese
`TITLE.BIN` offsets `0x0446`, `0x0576`, `0x07d6`, and `0x15f6`. The trace records
addresses, counts, and timing metadata, not the bytes on the DMA bus.

The same run's final VDP1 frame is 8037. Its 178-command trace has five
textured draws at indices 171–175 reading from those four VDP1 destinations:
two 192×3 draws at `0x44060`, then 64×8 at `0x47de0`, 64×8 at `0x479e0`,
and 64×13 at `0x46f60`. The source byte counts and addresses match the
transfer destinations and `TITLE.BIN` joins. A check of the SCU trace found
no later SCU DMA writes overlapping these four destination ranges. This correlates the final snapshot bytes with VDP1 command reads in one
cold-boot run. It does not prove that the SCU DMA bus transferred those exact
bytes, because the trace is metadata-only and later CPU writes are not
excluded. CLUT ownership also remains unresolved.

The five corresponding command records in the earlier rendered native-menu
capture at VDP1 frame 8050 match these draw words, geometry, and source
addresses; only their VDP1 link word differs. That separate rendered capture
has the retained menu screenshot at
`/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/title-menu.png`,
SHA-256
`8721cce3354bc20b1cd367fd44be3d4ecc60ca65fa89c7d00654e83f5ad7db23`.
The current trace-v4 run used no-render mode and has no screenshot, so it
cannot establish its own displayed pixels.

The complete raw capture and receipt are retained at
`/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/scu_dma_vdp1_frame_map_20261027/`.
Its README SHA-256 is
`5fb18214931b5837b9e7fcaf02d933cc91f7f741d45d59c4fffce6a0033875bb`, the
checksum-list SHA-256 is
`ccb38a615a5ae0f5d1dbf514431a2ad17e77f0b21877852e213140b30ac21a18`, and
the SCU trace CSV SHA-256 is
`2b008715262da43fb9d675a87cc92695a36eb64a26a3e014af3dda89a6e7b6e8`.
All 32 dump files were verified after copying to the external evidence disk.
The supporting exporter update is in [Yeager Ymir PR
#2](https://github.com/yeager/Ymir/pull/2).

This is stronger source-to-command evidence for the native menu path, but it
still does not provide CLUT source ownership or admit a Firestaff renderer.

## Sources

- DMWeb Nexus file formats and DMN Data File Decoder.
- `src/nexus/nexus_v1_engine.c` and `src/nexus/nexus_v1_title.c`.
- `docs/NEXUS_RUNTIME_CAPTURE.md`.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md).

## Cold-boot title menu and retail menu navigation (2026-10-27)

A rendered Ymir 0.4.2-dev run on TRV2 used the authentic Japanese retail
disc and BIOS in an isolated profile. It loaded no emulator state or game
save. After confirming the BIOS date/time screen, the deterministic schedule
sent Start for two frames at capture-loop frames 9,000 and 13,000, then one
frame at 15,000. At 16,000 capture-loop frames, the rendered retail screen is
the native three-choice menu: `NEW GAME`, `CONTINUE`, and
`CONFIGURATION`. This extends the clean-boot evidence beyond the title
prompt; it remains an emulator capture, not a Firestaff menu implementation.

The endpoint was saved as diagnostic emulator slot 3. Reloading that state
and advancing two frames with no input leaves the main menu visible. From the
same state, one frame of Down moves the selection from `NEW GAME` to
`CONTINUE`. Two one-frame Down pulses followed by a one-frame A pulse open
the native `CONFIGURATION` page, which displays Music, Sound, Game level,
Monster gauge, and Exit. These short replay captures establish that Ymir's
frame-indexed input route drives these visible menu transitions from this
authentic retail state. The saved emulator state contains no game save.

The final capture receipt reports VDP1 frame 13,108 and command cursor 168.
These are distinct from the 16,000-loop-frame capture counter. The evidence
bundle, screenshots, run log, VDP dumps, and saved state are retained on the
external evidence disk at
`/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_rendered_20261027/`.
The cold-boot menu screenshot SHA-256 is
`2b64c0baeb8f006f68176eb1f70bc7e4c626db9ec77f2157a1d20daef89387ec`;
the no-input replay is
`4476cc68722372b9f57b237374801a12bcfdac6d2e8aa068f575aa8bc87960c4`;
the Down replay is
`70ef5517ba73309fb63722a2af3c553e7863e02ab10ebf1414bc19a1cf9548a1`;
and the Configuration replay is
`dffac2f786b92b543ddfed7b6eb969d50b5f2b7e115a5e1a2423537c01e18427`.
All 24 retained files in the new bundle pass its SHA-256 manifest.

The original run used `--debug --capture-boot-frames 16000
--dump-memory --capture-screenshot --capture-save-state 3` with deterministic
Start pulses at frames 9,000, 13,000, and 15,000. The navigation replays used
`--load-state 3 --capture-frames` with no input, Down, or Down/Down/A
respectively. The full commands and hashes are recorded in the external
bundle README.

This establishes a native retail menu and a narrow menu-navigation route in
Ymir after a no-save cold boot. It does not prove the semantic purpose of
every input across regions, a stable New Game transition to a playable level,
Continue behavior with real save data, Firestaff menu rendering, or gameplay
HUD/viewport parity. Those remain separate parity gates.
