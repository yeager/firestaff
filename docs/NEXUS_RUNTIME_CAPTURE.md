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
enabled with only `NBG1` enabled; the captured values are `CHCTLA=0x1211`,
`PNCN1=0x00c0`, `MPOFN=0x3000`, `CRAOFA=0x1000`, and `PRINA=0x0503`.
This is the first bounded layer/map/priority/CRAM-offset observation from the
real frame. It still does not identify which retail asset filled the NBG1
map/character data, so the text/menu/HUD consumer and host composition remain
blocked.

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
