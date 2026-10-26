# Dungeon Master Nexus — startup status

This is an evidence report for startup, not a specification for a completed
Nexus startup screen. Reviewed 2026-10-25: the NBG1 tile-fetch address fix in
merged PR #19 passes against an authentic Japanese startup frame, but remains a
capture-only decoder result and does not prove the title-to-menu transition.

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
[Ymir PR #965](https://github.com/ymir-emu/Ymir/pull/965). This confirms a
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

Ymir PR #965 also now contains a bounded VDP2 VRAM/CRAM write tracer and an
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
Ymir dump files. It requires the emitted VDP register artifacts but does not
accept them as menu or gameplay evidence.

## Deterministic Start-to-menu checkpoint (2026-10-26)

Ymir PR #965 adds a frame-aligned Control Pad 1 Start route for the existing
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

## Boot VDP2 writer anchor (2026-10-26)

Ymir PR #965 was extended with a bounded direct SH-2 VDP2-write trace and a
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
proof that A semantically confirms New Game: an equally timed no-A control,
the next stable screen, source-resource joins, and palette/compositor evidence
are still missing. Do not enable a host menu, gameplay, HUD, or synthetic
confirmation rule from this observation.

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

FONT256's authenticated Character Generator bytes now remain in the engine's
source object for the future Saturn consumer. This does not admit glyph coding,
page/PND attributes, layer placement, or text writing; `font_loaded` remains
closed until those parts are bound by the same runtime witness.

## Sources

- DMWeb Nexus file formats and DMN Data File Decoder.
- `src/nexus/nexus_v1_engine.c` and `src/nexus/nexus_v1_title.c`.
- `docs/NEXUS_RUNTIME_CAPTURE.md`.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md).
