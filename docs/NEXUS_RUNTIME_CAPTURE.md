# Nexus runtime capture contract

This document describes the boundary between decoded Saturn source files and
runtime evidence. `MENU.BPK`, `TITLE.BIN`/`TITLE.CG`, `STABG.BIN`, `FACE.BIN`,
`DM.BIN`, `LEV##.DGN`, `SLEV##.BIN`, `SNDLEV##.SAL/.MAP` and `SDDRVS.TSK`
may be parsed and hash-verified from user-supplied data, but those facts do
not identify the Saturn VDP1/VDP2 destination, CLUT owner, command order,
runtime selector, or SH-2 consumer by themselves.

## Required producer

The capture launchers under `probes/nexus/` are operator-only. A capture-capable
instrumented Saturn emulator must create the binary artifact itself. Firestaff
does not copy BIOS, disc, source, or guessed trace bytes into an artifact.

The launchers now check that the selected Mednafen binary advertises the
corresponding `FIRESTAFF_NEXUS_*_OUTPUT` hook before writing a manifest or
starting the emulator. Stock Mednafen is therefore rejected early (exit 78):
its normal video snapshots or movie recording are useful visual evidence, but
they do not prove VDP register/VRAM ownership or SLEV/SAL event dispatch.
The raw Saturn launcher uses a non-quiet string scan for this check because
`set -o pipefail` makes `strings | grep -q` report a false failure when grep
closes the pipe after the first match.

For startup/input experiments the launcher can request an operator-owned
controller window with `--press-start-frame N --press-start-length N` and
choose the active-low Saturn button mask with `--press-button-mask`. The
default `0x10` is START, `0x20` is A, and `0x30` tests both. The external
Mednafen SMPC hook drives that real gamepad bit window and releases it on the
following frame. This is input provenance only: it does not write SH-2 state,
VDP memory, host pixels, or a guessed menu. A resulting menu/HUD/viewport claim
still requires the corresponding source identity and VDP1/VDP2 consumer
artifact.

For long operator-only scans, the capture patch also accepts the inherited
`FIRESTAFF_NEXUS_NO_WAITING=1` environment flag. It requests Mednafen's
no-wait scheduler path only; it does not alter the Saturn input, VDP or SCSP
capture payload.

The raw witness can be inspected with
`scripts/analyze_nexus_saturn_runtime_capture.py`. It reports SHA-256 values
for each captured VDP1/VDP2 region and can require a region to differ between
adjacent frames. This proves observed runtime change in the producer without
identifying a menu, HUD, viewport, CLUT, or consumer; the tool always keeps
semantic admission blocked.

## Authenticated VDP1 DGN material join

The external European run
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-menu-window-20260809/runtime-vdp12.raw`
has SHA-256
`685ae90896cf72deeac0a98de5fb1fb4f0bd90e89ace73c5e7631daf44b8faa7`.
In frame 0, the real command at `0x0e180` is a VDP1 colour-mode-1 draw with
`COLR=0x32a4`, source offset `0x4c580`, and 1,952 source bytes. The
word-swapped source is an exact match for the hash-verified
`LEV00.DGN` Structure2 descriptor 72 (`64x61`). The captured 16-word VDP1
CLUT at `((COLR & ~3) << 2) = 0xca90` also exactly matches that descriptor's
16-entry big-endian BGR555 palette after the capture's word order is restored.

This is the first source-plus-CLUT material receipt for the live Nexus
viewport. Reproduce it with
`scripts/analyze_nexus_vdp1_dgn_material_join.py`. The same receipt now also
matches the canonical raw `fill_selector` face rows in `LEV00.DGN` Structure3:
Structure2=72 is owned by entry 15 face 1. This is source-level face/mesh
provenance, not proof of runtime face selection, command ordering, transform,
culling, or complete scene assembly. The script therefore reports
`semantic_admission=blocked` and the production renderer remains fail-closed.

The same frame also contains four additional type-2 draws. Their exact
word-swapped sources and CLUTs bind to `LEV00.DGN` Structure2=60, 64, 68 and
71, plus Structure2=36 for the fifth draw. The observed VDP1 quadrilaterals
are reported as eight signed Saturn coordinates per command. Their source
materials now also have canonical `LEV00.DGN` Structure3 raw selector owners:
60 → entries 11/13 faces 5/9, 64 → faces 6/10, 68 → faces 7/11, 71 → entry 14
face 0, and 36 → entries 8/9 faces 0..6. This closes the source-level
material-to-face-selector join and proves hardware destination coordinates for
the captured commands, but not runtime face selection, command ordering,
transform/culling, or complete scene assembly.

The code now contains a separate capture-only VDP1 mode-1 compositor in
`nexus_v1_vdp1_capture_compositor.c`. It accepts one command only when its
texture span and 32-byte palette state exactly match the canonical DGN image
and palette after Saturn word-order restoration, and when the caller supplies
an authenticated original-capture receipt plus the display origin. It applies
the documented high-nibble-first texel order and transparent/end-code rules.
This is a bounded replay consumer, not permission to use the ordinary DGN
mesh route: complete command-list order, local/system clipping, dynamic face
selection and VDP2 composition remain separate gates.

An input-free pre-Start run at runtime frame 10000 independently captured
active VDP2 NBG1 bitmap mode (`BGON=0x0002`, `CHCTLA=0x1211`) with stable VRAM
and CRAM. Its decoded `MENU.BPK`, `FONT256.S2D`, `TITLE`, and `STABG` source
join is negative, so it is hardware-layer evidence only and is not called a
menu capture.

The validator also accepts `--require-vdp1-activity` for a V2 witness. This
requires a non-idle `PTMR`/`EDSR` state and a nonzero VDP1 VRAM or framebuffer
payload. It is only an active-engine observation. It does not prove which
`MENU.BPK`, DGN, ITEM, HUD or viewport record owns the bytes.

The first external European gameplay witness is retained at
`/Volumes/Extern-disk/nexus-saturn-capture/run-french-gameplay-skip18000-2/runtime-vdp12.raw`.
It has SHA-256
`549e03856163899381d4b6a03f65ef989fadbeccb338579eb87876e00f30e362`, with
two non-idle frames at `PTMR=02`, `EDSR=03`, `COPR=00000c`. The VDP1 command
window is stable across both frames: system clip, local coordinate, one type-2
textured command (`PMOD=0x0028`, `SRCa=0xc7c0`, `SIZE=0x28b4`) and END. The
draw-buffer selector and VDP1 VRAM/framebuffer payload change between frames,
which proves a real gameplay redraw. The command source is not yet joined to
an authenticated DGN/ITEM/MNS source span, so no menu, HUD, CLUT, source-asset
or production viewport binding is admitted from it yet.

An additional negative startup-input run is retained outside the repository at
`/Volumes/Extern-disk/nexus-saturn-capture/run-french-start100x60-skip6000-next/runtime-vdp12.raw`.
It uses the authenticated E-BIOS/French media, drives START+A for frames
100–159, then captures eight active VDP1 frames beginning at runtime frame
6000 (`ce800662…`). VDP2 registers, VRAM and CRAM remain byte-stable and the
observed framebuffer is still intro/dungeon imagery. This narrows neither the
menu transition nor its consumer; semantic admission remains blocked.

An operator-only VDP1 write trace was also run against the same European image
with an SH-2 PC attached to each VRAM write. The first bounded source probe was
dominated by framebuffer/colour fills at `PC0=0x06026260`/`0x06026270`; this is
not a DGN texture-owner join. `scripts/analyze_nexus_vdp1_write_trace.py`
records that negative result and remains fail-closed. The reproducible
Mednafen instrumentation is kept in
`scripts/mednafen_1.32.1_nexus_saturn_vdp1_pc_trace.patch`; it is diagnostic
only and does not authorize production drawing.

A second source-span-specific run captured 4,601 writes from runtime PC
`0x06013098` into the beginning of the type-2 command's source window
`0x47c00..0x49ffe`; the same bounded trace separately identifies the colour/
framebuffer writers at `0x06026260`, `0x06026270`, `0x060262c4` and
`0x060262d4`. The enhanced analyzer can require both a PC and an address
range. This is a positive runtime writer corridor, not yet a DM.BIN/TM.BIN
source join or a proof that the bytes are `DMV`, TITLE, MENU, HUD or DGN
material, so semantic admission remains blocked.

The authenticated eight-frame E-BIOS/French startup run now also joins the
bounded command window to this writer corridor. `COPR=0x00000c` exposes four
records at `0x00000`, `0x00020`, `0x00040` and `0x00060`: system records
`0x09`/`0x0a`, one type-2 bitmap command, and END. In frame 7 the draw command
is `PMOD=0x0028`, `SRCa=0x8f80`, `SIZE=0x28b4`; the Saturn encoding maps
`SRCa` to VDP1 byte offset `0x47c00`, the same destination selected by the
runtime writer PC `0x06013098`. The captured source span is
`0x47c00..0x4fe00` (33,280 bytes). This is a real command-to-VRAM join, but
the source span is still not joined to a decoded MENU/DGN/ITEM record or a
VDP2 CLUT/tile owner, so production menu/HUD/viewport drawing remains gated.

The reproducible `scripts/analyze_nexus_vdp1_source_join.py` comparator now
checks that source span against every bounded TEXT surface in the real MNS
corpus. The two-frame gameplay witness has one 16bpp draw command and 815
retail MNS surfaces in the supplied data root; neither native-byte nor
word-swapped comparison produces an exact owner. This is a negative source
join, not permission to use a host texture: the source may still belong to a
relocated/decompressed runtime buffer or another retail asset class, and the
CLUT/placement/command-order gates remain open.

The same comparator now also checks the raw image spans of all hash-verified
`LEV00.DGN` through `LEV15.DGN` Structure2 descriptors. The European frame-1
witness contains 1,678 bounded Structure2 candidates; its 33,280-byte 16bpp
source span has no native or word-swapped exact match. Frame 7 of the separate
eight-frame witness has the same negative result. These are source-byte joins
only: an exact match would still need the original command order, CLUT and
placement relation before any DGN material could be admitted.

The comparator also scans the extracted corpus as whole files, but admits a
file to this scan only when its SHA-256 matches the selected retail manifest
or the authenticated European startup-asset identity. Frames 1 and 7 each
scan 126 verified files (five variant identities are rejected rather than
silently treated as canonical); neither native nor word-swapped bytes contain
the captured source span. This closes another false-positive route without
claiming that relocated or decompressed runtime data has been recovered.

A second authenticated E-BIOS/French run with START+A held at frames 18000–18119
and capture beginning at frame 18500 is retained outside the repository at
`/Volumes/Extern-disk/nexus-saturn-capture/run-french-start18000x120-skip18500-live2/runtime-vdp12.raw`.
Its SHA-256 is `d648bd88…`, and the eight-frame validator reports active VDP1
draws in every frame. Frames 0–1 retain a 16bpp type-2 source at `0x63e00`
whose 33280-byte span hashes `5cca9793…`; frames 3–7 retain the same command
shape and span address but hash `58afb9c9…`. A bounded exact-byte scan of the
authenticated Nexus data directory finds no owning file for either span.
VDP2 registers, VRAM and CRAM remain byte-stable with the earlier witness, so
this run adds VDP1 source-span evidence only; it does not authorize a DGN,
MENU, ITEM, HUD or viewport interpretation.

The same producer run also emitted a bounded VDP2 write witness. The trace
contains 15,365 register writes in `0x180000..0x18011e`, 183,355 VRAM writes
in `0x000000..0x0743fe`, and 1,280 CRAM writes in `0x100000..0x1007fe`.
Nonzero SH-2 PCs are now captured with the writes; the dominant VRAM writers
are `0x06011924` (65,538), `0x060118fc` (40,448) and `0x06002fc4` (22,914),
and the dominant register writer is `0x0600231c` (14,400). This binds the
VDP2 activity to executing Saturn code, but not yet to a decoded tilemap,
CLUT bank, menu asset or final layer placement; semantic admission remains
blocked.

The producer also records 64 unique VDP2 writer code windows. The primary
window at `PC=0x06011924` contains the runtime words
`25fe 0000 25fe 007c ...`, while the initial register setup at
`PC=0x06001416` contains the expected VDP-register literal pairs. The exact
48-word windows do not occur verbatim in the hash-verified `TM.BIN` or
`DM.BIN` files, so the current result is an execution/code receipt with
source identity still unbound; no decompression or relocation interpretation
is promoted to production semantics. `scripts/analyze_nexus_vdp2_writer_candidates.py`
also reports the longest aligned partial matches: the primary window reaches
only four words in `DM.BIN` and three in `TM.BIN`; the longest observed
ten-word match belongs to another runtime PC and is shared by both retail
files. These are review leads only, not source ownership or tilemap/CLUT
consumer proof.

The authentic raw witness can now be decoded at the VDP2 register-field level
with `scripts/analyze_nexus_vdp2_composition.py`. In both the one-frame and
eight-frame European samples, `TVMD=0x0080` and `BGON=0x0002` show display
enabled with only `NBG1` enabled. `CHCTLA=0x1211` selects NBG1 bitmap mode,
colour code `1` (the 256-colour mode in Mednafen's VDP2 renderer), bitmap-size
code `0`, and `BMPNA=0x0000` selects bitmap palette `0`; `PNCN1=0x00c0` and
`MPOFN=0x3000` are present but are not consumed as NBG1 tilemap selectors in
this bitmap-mode frame. The remaining observed values are
`CRAOFA=0x1000` and `PRINA=0x0503`. This is a bounded hardware composition
observation, not a retail asset join: the bitmap VRAM source, CLUT owner and
text/menu/HUD meaning remain blocked.

`scripts/analyze_nexus_vdp2_bitmap_source.py` binds that geometry to the
hash-verified local corpus and compares the 131072-byte NBG1 span against 162
decoded `MENU.BPK` PRS3 surfaces, 242 real `FONT256.S2D` character-generator
tiles, and five authentic `TITLE.BIN` MAPD/TIBG maps expanded through
`TITLE.CG`, plus the first authentic 40×21 `STABG.BIN` map expanded through its
791-tile pixel region. It also compares the authenticated 256-entry `MENU.BPK`
PALT record. The one-frame and frame-7 eight-frame samples both produce zero
non-zero exact matches; MENU, title and STABG palettes have no exact byte or
word-swapped position in captured VDP2 CRAM. This is bounded negative evidence
for the captured gameplay/intro state, not proof of another source owner.
Dungeon bitmap and CLUT joins remain open.

The VDP2 bitmap comparator now also validates every nonzero 32-byte palette
anchor in the canonical LEV00–LEV15 Structure2 descriptors. The real corpus
contains 1,266 such anchors. Neither the European frame-1 witness nor the
independent eight-frame frame-7 witness contains a native or word-swapped
exact CRAM match for any of them; the previously checked MENU, TITLE and
STABG palettes remain negative as well. This is a bounded CLUT-source result,
not permission to select a palette bank or compose a layer.

Both VDP2 analyzers accept `--capture-frames N` and can now inspect any frame
inside an authenticated multi-frame witness. The eight-frame European
gameplay witness was rechecked at frame 7: it retains the same `NBG1` bitmap
composition (`BGON=0x0002`, `CHCTLA=0x1211`, `BMPNA=0x0000`) and still has zero
exact retail-source joins. This extends animation-state observation only; it
does not turn a frame into menu, HUD or viewport evidence.

The same producer has an independent SCSP trace patch for the audio lane.
Against the European gameplay window, the authenticated 68K task observed
nonzero mailbox writes at `0x100400` from PCs inside `SDDRVS.TSK` when loaded at
`0x1000`; the main SH-2 trace observed `0x06001652 -> 0x100400 = 0x02`.
`scripts/analyze_nexus_scsp_write_trace.py` verifies the trace envelope and
driver SHA-256 (`68890ee4…`). This is an authentic runtime handoff corridor,
but it does not identify the SLEV event selector, MAP row, SAL sample, or
SCSP voice ABI, so semantic admission and host playback remain blocked.

The observed 68K PC `0x3224` is now checked by
`scripts/analyze_nexus_scsp_driver_owner.py` against the authenticated driver
window at `SDDRVS.TSK+0x2220`. The source bytes read a command byte, limit it
against `0x12`, update driver state and write the SCSP per-channel register
family at offset `$17` from `a5=0x100000`. This proves a runtime
command-to-driver corridor, but does not bind the SLEV selector, MAP row or SAL
sample; playback remains blocked.

`scripts/analyze_nexus_slev_sal_runtime_corridor.py` now joins that runtime
corridor to the complete real European sound corpus without promoting any
meaning. It verifies all 16 `SLEV##.BIN`, `SNDLEV##.MAP`, `SNDLEV##.SAL` files
and `SDDRVS.TSK`, then applies the same DMWeb eight-byte MAP record grammar as
the C loader. The corpus contains 154 terminated MAP rows. The observed
mailbox trace contains four non-zero 68K writes, all raw value `0x02`, from
`SDDRVS.TSK` PCs `0x3224`, `0x1090`, `0x34aa` and `0x108e`; the main trace has
five mailbox records (`0x0002` and `0x0200`).
The analyzer also preserves the chronological raw sequence: sound-CPU writes
`0x02` to `0x100400`, `0x0f` to `0x100401` and `0x0118` to `0x100402`, repeated
at the observed PCs; the main CPU writes `0x02` and `0x0200` to
`0x00100400`. These are address/value/PC observations only and are not
renamed to event IDs, MAP rows or samples.

Fifty-four parsed MAP windows extend beyond the extracted SAL file length when
treated as direct file intervals. The source code already identifies these
fields as an opaque sound-driver memory area, so the audit records this as a
boundary fact rather than an error or a guessed relocation. It does not prove
which event selects a row, how SAL bytes are decoded, or which SCSP voice owns
the data. The receipt therefore reports
`event_selector_semantics=unproven`, `sal_codec=unproven` and
`host_playback=blocked`.

The external capture inventory is reproducible with
`scripts/analyze_nexus_capture_inventory.py`. The current operator corpus has
34 valid raw witnesses and 136 frame observations: 8 reset/no-layer frames,
14 RBG0-only frames from the Saturn CD-player state, 100 NBG1-only frames from
the dungeon witness, and 14 other active VDP2 states. The inventory does not
call any of these states a menu, HUD or viewport. The latest frame-0 startup
attempt is a one-frame reset witness (`TVMD=0`, `BGON=0`) and supplies no
startup asset-consumer evidence.

The producer also has a bounded SCSP-read trace with an optional sound-CPU PC
filter. In the retained 100-record European gameplay window, reads were
observed from shared sound RAM and driver setup tables, but none from the
`0x100400..0x100401` mailbox range and none while filtered to `0x3224`.
`scripts/analyze_nexus_scsp_read_trace.py` records this negative result. It
does not authorize an inferred event, selector, sample or playback route.

The capture patch now emits `FIRESTAFF_NEXUS_SATURN_VDP1_RAW_V2` with an
explicit VDP1 state line (`TVMR`, `FBCR`, `PTMR`, `EDSR`, `LOPR`, `COPR`, the
return pointer and framebuffer selector) before the unchanged raw VRAM/FB
payload. V1 witnesses remain readable. The state line is an observation of
the emulator's VDP1 model, not by itself proof that a particular MENU.BPK,
DGN, HUD or viewport record owns the command stream.

`scripts/analyze_nexus_vdp1_command_window.py` can then inspect the bounded
record window ending at the captured `COPR`. It reports raw command words and
requires an observed END record when requested. Its output is still a state
receipt: command type `0x09`, `0x0A`, or END does not establish a game-asset
owner or authorize a host draw.

The runtime writer/source join is reproducible with
`scripts/analyze_nexus_vdp1_runtime_writer_join.py`. On the authenticated
European `DM.BIN`/`TM.BIN` pair, the captured writer window beginning at
`PC=0x06013058` and writing `0x47c00` has no whole native owner. With the
explicit `0x06010000` load-base hypothesis, its direct `DM.BIN` offset
`0x3058` is a byte mismatch; the best retail-file overlap is one SH-2 word.
The tool records this as negative evidence and leaves relocation,
decompression and runtime-code ownership unbound. It does not treat a short
word overlap as a source join or unlock menu, DGN, HUD or viewport drawing.

`scripts/analyze_nexus_tm_bin_vdp_owner.py` records a separate static source
receipt. On the authenticated retail `TM.BIN`, its SH-2 PC-relative literal
loads reach the VDP1 register window (`0x25d00000` through the observed
register offsets) and VDP2 register space. This narrows code ownership, but a
literal corridor is not an execution trace and does not join `TM.BIN` to any
captured command or source span.

`scripts/analyze_nexus_slev_sh2_owner.py` provides the corresponding static
receipt for all 16 hash-authenticated `SLEV##.BIN` tasks. It scans the
big-endian SH-2 word stream, binds the shared `0x2fe6` entry and reports the
exact PC-relative literal rows that touch the observed `0x25/0x26` address
corridors. The scan is source evidence only; the task body remains opaque and
cannot authorize event dispatch, selector order, callback writes or sound
playback.

When a later frame contains a texture command, the same tool reports its
bounded `SRCa`-derived VRAM byte span and SHA-256. That span is the join key
for a future source-owned capture; it is not permission to reinterpret the
bytes as PRS3, TITLE.CG, MENU.BPK, STABG, FACE, or DGN material.

## Artifact families

| Route | Magic | Evidence still required |
|---|---|---|
| PRS3/menu replay | `NXSPRS3M` | source stream, SH-2 execution trace, PALT/VDP1 consumer and frame identity |
| VDP1 material | `NXSVDP1C` | DGN/face/descriptor/image/palette identity, command words, VRAM window and destination |
| SLEV/SAL runtime | `NXSLSC01` | task body, event selector, MAP row, SAL window and SDDRVS consumer |
| Structure3 topology | `NXS3TOP1` | face/vertex ownership, transform/culling route and frame order |

Each verifier checks the artifact header, source hashes, bounded payload and
identity lanes. A manifest alone is not a capture; a source decoder alone is
not a presentation or gameplay proof. Until the corresponding artifact is
admitted, production routes remain no-draw or playback-gated.

## Source references

The byte-format baseline is the [DMWeb Nexus file-format documentation](http://dmweb.free.fr/community/documentation/dungeon-master-nexus/file-formats/),
including the [ITEM.IBS description](http://dmweb.free.fr/community/documentation/dungeon-master-nexus/item.ibs-file/)
and [DGN documentation](http://dmweb.free.fr/community/documentation/dungeon-master-nexus/dgn-files/).
Disassembly and runtime ownership must be established from the Saturn program
and an execution capture; inferred offsets or host screenshots must not be
promoted to production semantics.
