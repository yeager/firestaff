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

The BIOS/media region must also be compatible. The supplied English merged
disc reports `SGAREA=J`; the external capture disk now contains the matching
Japanese BIOS 1.01 (SHA-256
`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`). The
European E-BIOS remains a separate, valid pairing for the French
`SGAREA=E` data-only disc. Stock Mednafen or a mismatched pair is still
rejected; any resulting corrupted desktop frame is not a valid startup/menu
witness and must not be admitted to Firestaff.

The external Mednafen 1.32.1 build also advertises the `NXSLSC01` SLEV/SAL
producer. Its hook records only authenticated SH-2 WorkRAM writes, both SH-2
program counters, and an incremental payload hash. The payload is deliberately
opaque: this producer does not establish the SLEV selector, SAL codec, MAP-row
ownership, or host playback. A real capture must still be imported and joined
to the source-owned runtime corridor before any event or audio behavior opens.

For startup/input experiments the launcher can request an operator-owned
controller window with `--press-start-frame N --press-start-length N` and
choose the active-low Saturn button mask with `--press-button-mask`. The
default `0x10` is START, `0x20` is A, and `0x30` tests both. The external
Mednafen SMPC hook drives that real gamepad bit window and releases it on the
following frame. This is input provenance only: it does not write SH-2 state,
VDP memory, host pixels, or a guessed menu. A resulting menu/HUD/viewport claim
still requires the corresponding source identity and VDP1/VDP2 consumer
artifact.

When the input must be observed inside the raw capture, pass
`--require-input-window`. The launcher then rejects a plan unless the complete
button interval lies between `skip_frames` and
`skip_frames + frame_limit`; this prevents a pre-input witness from being
mistaken for a post-input menu capture.

For a multi-step Saturn startup route, the operator launcher also forwards the
optional environment variable
`FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE`. Its format is a comma-separated list
of `frame:length:mask` entries, for example
`1250:60:0x10,1350:60:0x10,1450:60:0x20`. The sequence is recorded in the
manifest and is applied to the real SMPC input stream in one emulator session;
the masks are the Saturn pad bits exposed by the instrumented SMPC hook; the
hook clears those bits during each interval because the Saturn pad bus is
active-low. The
13-bit path includes the right-shoulder bit (`0x1000`), which is required for
the documented Nexus debug sequence `L, R, X, Up, C, Left, L, Right`.
This sequence is an emulator investigation aid and input-provenance record
only. The resulting frame still requires an exact MENU.BPK/FONT256 or
other source-consumer join before startup or menu presentation can open.

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

The material probe accepts both the newer VDP1+VDP2 frame container and the
earlier VDP1-only witness. For the retained four-frame V1 capture, reproduce
the join with:
`python3 scripts/analyze_nexus_vdp1_dgn_material_join.py \
  /Volumes/Extern-disk/nexus-saturn-capture/run-codex-menu-window-20260809/runtime-vdp12.raw \
  --data-dir /Users/bosse/.firestaff/data/nexus --frame 0 --capture-frames 4 \
  --command-offset 0xe180`.

The same legacy witness also passes the C runtime handoff when linked against
the normal `firestaff_nexus` target: the first resolved mode-1 command reports
`source_join_verified=1`, `palette_join_verified=1`,
`structure3_face_owner_join_verified=1`, and `renderer_permitted=1`. This is
one source-bound capture command only; it does not promote the complete DGN
scene, camera transform, command-list ownership, menu/HUD identity, or VDP2
composition.

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

The capture compositor is atomic at both levels: a single command that is
fully off-screen or otherwise produces no pixel cannot publish its palette or
partial framebuffer, and a failed first VDP1 pass in the VDP2-over-VDP1
composition restores the complete viewport. The retail DGN face/material
receipt likewise keeps `no_draw_only=1` and
`blocks_real_dgn_mesh_render=1` on its successful source-bound state; these
flags describe a validated source boundary and do not authorize normal
rendering.

An input-free pre-Start run at runtime frame 10000 independently captured
active VDP2 NBG1 bitmap mode (`BGON=0x0002`, `CHCTLA=0x1211`) with stable VRAM
and CRAM. Its decoded `MENU.BPK`, `FONT256.S2D`, `TITLE`, and `STABG` source
join is negative, so it is hardware-layer evidence only and is not called a
menu capture.

A time-corrected authenticated E-BIOS/French session at
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-menu-window-20260809-f/runtime-vdp12.raw`
captured 512 consecutive frames with `skip_frames=10000`, so raw frame 500
corresponds to runtime frame 10500, where the real START input is injected.
Its SHA-256 is
`decf7dbd3a327cb5623fe7c12b4820f5037dc0e977c50ec3aac38645fc353d30`.
All 512 frames have VDP1 activity. The first frame retains the active NBG1
bitmap tuple; after the input window VDP2 changes to other register states,
but no `MENU.BPK`/`FONT256`/`TITLE`/`STABG` exact source join is found. This
is positive input/transition evidence, not menu, HUD, or text-consumer
admission, so semantic admission remains blocked.

A parallel 512-frame run with Saturn A (`0x20`) instead of START (`0x10`)
produced the same raw SHA-256 and the same unbound VDP2 state. The button-mask
change therefore did not produce a verifiable menu window; this is not a
reason to open the host text consumer or to infer a menu route.

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

The longer J-BIOS/English-media startup-input witness
`run-codex-start-menu-long-20260810/runtime-vdp12.raw` adds 1,800 frames with
the authenticated Start window at capture frames 300–359. Frame 300 has a
complete VDP1 chain (`User/System Clip → Local Coordinate → type-2 draw →
END`) whose draw consumes `0x63e00..0x6bf80` (33,280 bytes, 16bpp). The
command-sequence, material-resolver and capture-compositor tests accept that
hardware framing, while the source comparator finds no exact native or
word-swapped owner among 126 verified retail files, MNS surfaces or DGN
Structure2 spans. It is therefore stronger startup-input transport evidence,
not a startup/menu asset identity or production-renderer admission.

The isolated rerun on 2026-08-11 keeps the same J-BIOS/English-media pair and
captures 361 frames with `SGAREA=J` and `Region=0x1`. Its bounded VDP1 write
trace observes 18,318 writes in `0x63e00..0x6bf80`; 18,277 carry
`PC=0x06029e84`. This is authenticated runtime-writer evidence for the
destination corridor, not proof of the source asset, CLUT owner or menu
identity. The source-join and production gates therefore remain closed.

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

## Session-bound SCSP/SLEV corridor

The short clean J-region diagnostic run at
`/Volumes/Extern-disk/nexus-capture-20260813/run-attachment-j-short-20260813/`
validated 60 raw frames and 16 active VDP1 observations, but produced no
SH-2 RAM-write or memory-snapshot artifact despite the launcher receiving
those output paths. This is a capture-tooling limitation: the witness remains
transport-only and cannot identify a LEV01 pose, save write, or runtime
consumer.

A separate operator-only J-BIOS/English-media run is retained outside the
repository at
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-scsp-menu-bound-20260810/`.
The manifest binds the Japanese BIOS SHA-256
`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`, the
English merged-disc SHA-256
`5ab2fbd572d454e4a9df024e2bb6b3cd30f84018c3e650bd945bd52742edd83d`, and the
same trace session token `scsp-menu-bound-20260810` into both the sound-CPU
and main-SH-2 traces. The raw VDP1/VDP2 capture is independently validated;
its SHA-256 is
`86bc7cddb23c012edebd1f8ca93fe019737cc8e1fb59a5c68f39fb2a60ffd63a`.

The source-bound corridor analyzer reports all 16 SLEV files, all 16 MAP
files, all 16 SAL files, and the authenticated `SDDRVS.TSK` identity. It
observes mailbox value `0x02` at the expected mailbox and binds the producer
and consumer traces to the same session. It does not observe the verified
SDDRVS command handler at `PC=0x3224`, an event selector, a MAP row, a SAL
codec/sample, or a SCSP voice write. The C runtime join test therefore passes
with `source-bound, playback blocked`; this is a positive provenance receipt,
not an audio playback admission.

Reproduce the source-bound check with:

```text
python3 scripts/analyze_nexus_slev_sal_runtime_corridor.py \
  /Volumes/Extern-disk/nexus-saturn-capture/run-codex-scsp-menu-bound-20260810/scsp-writes.trace \
  --main-trace /Volumes/Extern-disk/nexus-saturn-capture/run-codex-scsp-menu-bound-20260810/main-scsp-writes.trace \
  --data-dir /Users/bosse/.firestaff/data/nexus \
  --driver /Users/bosse/.firestaff/data/nexus/SDDRVS.TSK
```

The missing handler and event-to-SAL relation keep SLEV/SAL/SDDRVS playback
capture-gated.

For the next operator capture, the external Mednafen diagnostic build can
also record sound-CPU reads. Set the trace path and a bounded SCSP range before
launching the existing raw-capture launcher:

```text
FIRESTAFF_NEXUS_TRACE_SCSP_READS=/Volumes/Extern-disk/nexus-saturn-capture/run-scsp-read/scsp-reads.trace
FIRESTAFF_NEXUS_TRACE_SCSP_READ_MIN=0x100000
FIRESTAFF_NEXUS_TRACE_SCSP_READ_MAX=0x101000
FIRESTAFF_NEXUS_TRACE_SCSP_READ_LIMIT=200000
```

The launcher propagates these variables and records the resulting trace hash
in the manifest. This trace is still observational: a read of a SCSP address
does not by itself identify a SLEV selector, SAL codec or host-playback route.
It is intended to provide the missing source-to-voice evidence needed before
those runtime gates can be reconsidered.

The corrected external build produced a valid 300-frame menu-window witness at
`/Volumes/Extern-disk/nexus-saturn-capture/run-scsp-read-menu-20260811c/`.
The raw capture SHA-256 is
`13a7cd31ad1fe1eaa1368e7b8c4403fe43d8f03a6541f25ac86bc0c44a96b9b3`.
The read trace is session-bound and contains 200,000 reads, but only the
mailbox/status addresses `0x100420` and `0x100408`; it contains no SAL-window
or voice-parameter read. The paired write trace reaches the same authenticated
mailbox corridor, while event-selector semantics, SAL codec and playback stay
blocked. This is a negative runtime result, not a decoder admission.

A matching deterministic 300-frame menu witness with the focused sound-RAM
window `0x40000..0x4ffff` recorded zero sound-CPU reads and zero writes in
that window. Its raw SHA-256 is the same
`13a7cd31ad1fe1eaa1368e7b8c4403fe43d8f03a6541f25ac86bc0c44a96b9b3`.
This rules out menu initialization as evidence for a sample-RAM consumer; a
gameplay witness is still required.

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

The source-side STABG work also has a separate capture-only join adapter.
Given an explicitly identified Saturn crop, it compares the DMWeb first-map
decode row by row and all 512 raw STABG palette bytes before writing the
320×168 indexed surface to a framebuffer. This strengthens the source/capture
join for a future HUD witness, but does not reinterpret the negative gameplay
capture above or open VDP2 layer ownership and normal HUD presentation.

The atomic VDP1/VDP2 composition lane now accepts that STABG receipt as its
VDP2 source and records either explicitly attested order (`VDP1 over VDP2` or
`VDP2 over VDP1`). It rejects an ambiguous order and restores the framebuffer
on a failed subroute. This establishes the handoff contract needed for HUD
over viewport, not the missing Saturn frame that would select the order.

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

Firestaff now has a C receipt for the emitted
`FIRESTAFF_NEXUS_SCSP_WRITE_TRACE_V1` schema. It validates the raw trace hash,
mailbox writes, the `SDDRVS.TSK` command-handler PC `0x3224`, and the SCSP
voice-register corridor. The real external trace passes this structural
receipt, while event→MAP semantics, SAL decoding, and playback remain false.
The C receipt also accepts the producer-side
`FIRESTAFF_NEXUS_MAIN_SCSP_WRITE_TRACE_V1` schema as a separate receipt. It
keeps the SH-2 mailbox values `0x02` and `0x0200` distinct from the sound-CPU
trace and requires both on the authenticated external producer trace. This
joins the producer observation to the mailbox corridor only; it still does
not assign a gameplay event, MAP row, SAL sample, SCSP voice, or playback.
Against the European gameplay window, the authenticated 68K task observed
nonzero mailbox writes at `0x100400` from PCs inside `SDDRVS.TSK` when loaded at
`0x1000`; the main SH-2 trace observed `0x06001652 -> 0x100400 = 0x02`.
`scripts/analyze_nexus_scsp_write_trace.py` verifies the trace envelope and
driver SHA-256 (`68890ee4…`). This is an authentic runtime handoff corridor,
but it does not identify the SLEV event selector, MAP row, SAL sample, or
SCSP voice ABI, so semantic admission and host playback remain blocked.

The two trace files are bindable only when the producer writes the same
non-empty `session=` token into both headers. Set
`FIRESTAFF_NEXUS_TRACE_SESSION` to a capture-local token before starting
Mednafen. Older traces without this field remain valid structural receipts,
but are deliberately rejected as a cross-trace runtime join because they do
not prove that the producer and sound-CPU observations came from one session.

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

The C integration test `nexus_v1_slev_scsp_runtime_join` now requires the
same two trace streams, the authenticated `SDDRVS.TSK` corridor, and a real
`SNDLEV00.SAL`/`.MAP` load in one receipt. It passes only with the source
directory present and still requires `blocked-unsupported-decode`; this joins
provenance without turning mailbox values into event or sample identities.

The external capture inventory is reproducible with
`scripts/analyze_nexus_capture_inventory.py`. The current operator corpus has
34 valid raw witnesses and 136 frame observations: 8 reset/no-layer frames,
14 RBG0-only frames from the Saturn CD-player state, 100 NBG1-only frames from
the dungeon witness, and 14 other active VDP2 states. The inventory does not
call any of these states a menu, HUD or viewport. The latest frame-0 startup
attempt is a one-frame reset witness (`TVMD=0`, `BGON=0`) and supplies no
startup asset-consumer evidence.

## Latest J/J startup-to-menu attempt

The external run
`/Volumes/Extern-disk/nexus-saturn-capture/run-authentic-merged-startup-menu-20260811d/`
contains 200 frames from the merged English disc with the authenticated J
BIOS. The bounded active-low `START+A` input window is frames 120–129. The
raw VDP1/VDP2 stream is SHA-256
`1f1b63b7917ecc347eb2e931fefe2881d64faf96338e0fd04312c1e5887a2008` and
contains 156 frames with observed VDP1 activity.

The post-input window changes VDP state and produces a mode-5 VDP1 source
span at `0x10000`, but the span has no exact match in the verified MENU.BPK,
MNS, DGN or other retail source corpus. The VDP2 character lane reports
`font256_vram_span_matches=1/4` and `font256_cram_palette_matches=0/1`, with
the text consumer still unbound. The paired SCSP traces record a producer
mailbox write of `0x02` and the sound-CPU mailbox/voice-register corridor, but
do not identify a menu event, MAP row or SAL sample. This is authenticated
transition evidence only; startup→menu, menu text, HUD and viewport admission
remain blocked.

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

### Generic Mednafen capture handoff

The upstream review candidate on the external disk uses the neutral markers
`MDFN_SS_SATURN_RUNTIME_CAPTURE_V1` and `VDP1_RAW`. Firestaff's transport
reader accepts those markers as well as the historical Firestaff markers. The
payload order is unchanged: VDP1 VRAM, both VDP1 framebuffers, the optional
draw-buffer byte, then VDP2 registers, VRAM and CRAM. This is intentionally a
transport compatibility boundary only; generic Mednafen output still has no
Nexus asset owner and therefore cannot pass semantic admission on its own.
The generic Mednafen container serializes Saturn words explicitly big-endian.
Historical Firestaff V1/V2 containers have no byte-order field, so the reader
resolves their order from a nonzero VDP2 register witness; ambiguous raw
registers remain transport-only. The regression test uses nonzero `TVMD` and
`BGON` words in both forms, and the external Firestaff witness is verified as
big-endian by that receipt.

To reproduce the generic dump, build the review candidate on the external
disk with Saturn enabled and provide user-owned BIOS/media paths at runtime:

```sh
cd /Volumes/Extern-disk/mednafen-nexus-upstream-pr-v1
./configure --enable-ss
make -j2

HOME=/Volumes/Extern-disk/mednafen-nexus-upstream-pr-v1/runtime-home \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
./src/mednafen -sound 0 \
  -ss.bios_jp "/path/to/Sega Saturn BIOS (J) (1.01).bin" \
  -ss.capture.path "/Volumes/Extern-disk/mednafen-nexus-upstream-pr-v1/nexus-test.capture" \
  -ss.capture.frames 1 "/path/to/Dungeon Master Nexus.cue"
```

The command is allowed to stop after the requested frame has been emitted;
the resulting file stays outside the repository. Validate its neutral stream
layout first, then run the Firestaff transport gate:

```sh
python3 scripts/validate_mednafen_saturn_capture.py \
  /Volumes/Extern-disk/mednafen-nexus-upstream-pr-v1/nexus-test.capture
FIRESTAFF_NEXUS_RUNTIME_CAPTURE=\
/Volumes/Extern-disk/mednafen-nexus-upstream-pr-v1/nexus-test.capture \
FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME=0 \
./build/test_nexus_v1_saturn_runtime_capture
```

Both checks prove only that the VDP1/VDP2 byte stream is readable. A frame
with `TVMD=0` or `BGON=0` is a reset/idle observation and must not be counted
as startup, menu, HUD or viewport evidence.

In the 600-frame Japanese-BIOS/English-media witness, frame 350 is an active
NBG1 tilemap frame (`TVMD=0x8000`, `BGON=0x0103`, `CHCTLA=0x1010`). The
capture-only NBG1 tilemap decoder accepts that frame after restoring the
candidate's explicit big-endian VDP2 word order. This proves raw tilemap
decoding only; no Nexus source span, menu identity or production compositor
is admitted by that result.

The source-bound NBG1 bitmap compositor now consumes the same explicit byte
order carried by the raw-frame receipt for both VDP2 registers and CRAM. An
older fixture with no producer order keeps the previous scoring fallback; a
real Firestaff or Mednafen frame does not guess from register values.

### Fresh reset witness

An external reset-only run using the authenticated J BIOS and the merged
English/European disc cue is retained at
`/Volumes/Extern-disk/nexus-saturn-capture/run-reset-title-menu-20260811b/`.
Its 300-frame raw stream has SHA-256
`7756900f79ed3b7c4e680918a1f9604943fb96398b068866c77bc91b37d3f0ad` and
passes the raw-layout validator. VDP2 is reset through the early frames,
becomes display-active around frame 120, and then holds the observed
`TVMD=0x8000`, `BGON=0x000f`, `CHCTLA=0x1010` state. No input was injected.
This is useful reset/runtime evidence, but it does not identify a retail
TITLE, MENU.BPK, FONT256 or host display consumer, so startup admission stays
closed.

On 2026-08-10 a bounded J-BIOS/English-media source-trace attempt reached
500,000 SH-2 RAM-source rows but timed out before producing a complete frame
capture. After rejecting zero-filled RAM and unmapped ISO padding, the trace
has `exact_iso_chunk_matches=0` and `retail_runtime_source_join=missing`.
It is retained as a negative transport/provenance witness, not as menu
evidence.

`scripts/analyze_nexus_vdp1_command_window.py` can then inspect the bounded
record window ending at the captured `COPR`. It reports raw command words and
requires an observed END record when requested. Its output is still a state
receipt: command type `0x08` (User Clip), `0x09` (System Clip), `0x0A`
(Local Coordinate), or END does not establish a game-asset owner or authorize
a host draw.

## Authenticated VDP1 command-list framing

`scripts/analyze_nexus_vdp1_command_sequence.py` follows the captured
`CMDLINK` chain instead of treating the live `COPR` cursor as the end of the
list. It distinguishes User Clip, System Clip and Local Coordinate records,
handles a cursor captured during an active draw, and accepts reset/idle frames
as idle observations rather than pretending they contain a scene.

Against the external 300-frame European witness
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-long-menu-20260809b/runtime-vdp12.raw`
(manifest SHA-256
`6ba86952b3fe1cbf9e2ebbf20f52345e7df15c270a90e00e09867a7fff0c9611`), the
verifier covers all 300 frames: 290 active command chains and 10 idle END
frames. Every active chain reaches an observed END and contains draw records
plus a clip-state record and Local Coordinate state. The witness uses both
VDP1 command buffers; for example, frame 104 resolves `0x0c3c0`→`0x0d6c0`
with 153 records, 147 draws, two User Clip records, no System Clip record and
three Local Coordinate records. This is hardware command-order evidence only;
it does not identify the chain as startup, menu, HUD or a DGN scene.

## Full gameplay-chain DGN join

The longer European E-BIOS/French-data-only witness
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-menu-long-20260809f/runtime-vdp12.raw`
has 900 captured frames and SHA-256
`cd167dabeaedd02f8555a4e1d4eaa1818b51b5fb6ebb3479c9a2869faf6bbdc9`.
At frame 899 the authenticated chain contains 220 records, including 209
colour-mode-1 textured draws. `scripts/analyze_nexus_vdp1_dgn_command_sequence_join.py`
matches 204/209 captured source spans and CLUTs to canonical `LEV00.DGN`
Structure2 materials; 175 of those matches also have Structure3 face-selector
owners. The five unmatched source spans and 34 selector-less material uses are
reported, not replaced by guessed HUD/menu assets. This closes a bounded
gameplay material/order receipt while leaving the unresolved cases visible.
Transform/culling, display origin, HUD/menu ownership and VDP2 composition are
still unproven, so `semantic_admission` remains blocked.

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

The first positive runtime source join is retained outside the repository at
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-vdp1-source-read-20260810`.
The authenticated J-BIOS/English-media capture has 600 raw VDP1/VDP2 frames.
Its bounded SH-2 source-write trace contains a complete 16,384-byte
`TM.BIN` chunk loaded to `0x06027000` by runtime PC `0x00205f18`; the strict
analyzer resolves the chunk to ISO offset `0xd89800`, member offset
`0x17000`, and reports `retail_runtime_source_join=verified`. Reproduce it
with:

```sh
python3 scripts/analyze_nexus_sh2_source_trace.py \
  "/Volumes/Extern-disk/nexus-saturn-capture/media/Dungeon Master Nexus (English) - Merged.iso" \
  "/Volumes/Extern-disk/nexus-saturn-capture/run-codex-vdp1-source-read-20260810/sh2-source-writes.trace"
```

This proves retail `TM.BIN` bytes entered runtime RAM during the capture. The
same witness still has no exact VDP1 command-to-retail-file owner
(`source_join=unbound`), so it does not unlock the startup/menu text consumer,
HUD, viewport or SLEV/SAL/SDDRVS playback.

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

The VDP1 writer join also accepts the frame-scoped V2 trace emitted by the
current capture producer. Use the same frame number for both artifacts; for
example, the startup-source witness can be checked at frame 106 with:

```sh
python3 scripts/analyze_nexus_vdp1_source_write_join.py \
  /Volumes/Extern-disk/nexus-saturn-capture/run-authentic-merged-startup-source-20260811b/runtime-vdp12.raw \
  /Volumes/Extern-disk/nexus-saturn-capture/run-authentic-merged-startup-source-20260811b/vdp1-writes.trace \
  --capture-frames 300 --frame 106
```

This removes the former V1-only parser failure. It still reports
`semantic_admission=blocked` when no exact source owner is found.

## Artifact families

The capture consumer now also has a bounded VDP2 NBG1 tilemap lane. It admits
only an exact name-table/character-generator/full-CRAM join, the observed
NBG1 tilemap register tuple, 8x8 cells and two-word name entries. Mednafen's
verified fields are used directly: `PNDSize=0`, `palno=tmp&0x7f`, flip bits
`0x4000/0x8000`, `charno=pnd[1]&0x7fff`, and the 4/8bpp CG addressing used by
`TileFetcher::Fetch`. This is a source-bound replay primitive, not a general
VDP2 emulator. It does not admit PNDSize=1, 16x16 cells, inferred source-map
crops or a host text layout. A positive FONT256/TEXT4/TABL consumer capture
is still required before the startup/menu text gate can change.

The transport-only tilemap decoder is also exercised against the authenticated
external J-BIOS/English witness at frame 300:

```sh
cmake --build /tmp/firestaff-nexus-build \
  --target test_nexus_v1_vdp2_runtime_tilemap -j2
FIRESTAFF_NEXUS_RUNTIME_CAPTURE=\
  /Volumes/Extern-disk/nexus-saturn-capture/run-codex-vdp1-source-read-20260810/runtime-vdp12.raw \
FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME=300 \
  /tmp/firestaff-nexus-build/test_nexus_v1_vdp2_runtime_tilemap
```

The test passes with NBG1 enabled in character/tilemap mode and keeps
`renderer_permitted=0`. The same witness is not a bitmap-mode frame, so the
separate 512x256/8bpp bitmap decoder correctly rejects it; this prevents a
character map from being mislabeled as a bitmap or a menu surface.

VDP1 replay also has an atomic sequence lane for a complete bounded command
window. Every mode-1 command must pass the existing exact DGN image/CLUT join;
the sequence additionally requires captured clip state (User Clip or System
Clip), local-coordinate state, command order and an observed END record. Replay
is staged into a
temporary framebuffer and published only after all commands pass. This closes
the single-command-to-command-list handoff mechanically, but does not claim
that the current European traces are a complete retail viewport scene.

`nexus_v1_vdp1_command_sequence_frame()` now performs the same bounded
CMDLINK traversal in C. On the authenticated European frame 899 snapshot it
reports 220 records, 215 draw records, two User Clip records, two
Local-Coordinate records and the first display origin `(160,112)`. The origin
is carried into the sequence compositor and every command must match it.
This proves command-list framing and display-origin state only; mesh camera
transform, face selection and culling remain blocked.

## Mednafen generic VDP1 state and bus-order handoff

The generic Mednafen candidate now emits an optional `state=` line after
`VDP1_RAW`. It records TVMR/FBCR/PTMR/EDSR, LOPR/COPR, return address,
framebuffer selector and system-clip bounds. The VDP1 payload remains
big-endian Saturn bus words. Firestaff records that order in the transport
receipt and swaps only a temporary replay copy before feeding the existing
little-endian VDP1 command parser; the raw artifact is never rewritten.

The external candidate run
`/Volumes/Extern-disk/nexus-saturn-capture/run-mednafen-generic-state-20260811i/runtime-vdp12.raw`
contains 120 frames and 76 non-idle VDP1 observations. The transport
validator, VDP1 command-sequence probe and VDP2 tilemap capture-only probe
pass against it. This proves a replayable VDP1 command-state corridor, not
yet a MENU.BPK/FONT256, HUD or DGN scene owner; semantic admission remains
blocked.

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
