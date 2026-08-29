# Nexus: verified completion status

## Local Japanese retail-media verification — 2026-08-26

The supplied Japanese retail corpus was opened directly both as its original
nine-track CUE/BIN set and as the original ZIP container.  In both cases the
native launch probe found the same 137 ISO members, loaded the real title,
warning, game-over, STMP and LOGOBG surfaces, and completed five engine ticks
without materializing game data.  The CUE binding check also verified the
original raw BIN bindings for CD tracks 2–9.

This is a media and native-launch result only.  It deliberately does **not**
promote CDDA playback, the Saturn title/menu handoff, or a playable start:
the real CDDA consumer and the source-owned LEV01 level/x/y/facing witness
remain unbound.  `LEV00` remains a title-only asset and the engine therefore
fails closed before it can invent a start pose.

## Japanese held-START SMPC witness — 2026-08-27

An external, instrumented Mednafen session used the same JP 1.01 BIOS and
the hash-verified nine-track CUE/BIN disc.  `START` was held for all 90
captured frames after 30,000 boot frames.  The producer wrote the current
Firestaff V2 envelope, which the native transport validator now accepts:

- raw VDP1/VDP2 capture: 90 frames, SHA-256
  `163fcd0a39c6a408fd1e76b9818dc419e67f39ece1c7b65b5634a12f3092ce93`;
- input receipt: 180 injected-frame rows, SHA-256
  `9da3b51f01407a0872fa0e72d19506a2bfaf47a74c022acab9b967302d650f25`;
- SMPC receipt: 1,029 source-PC-bound reads over capture frames 38--89,
  SHA-256
  `6d4c2369439b8a69cd91b5dd2119d05c7fba22bd0b471cc6f1b81455380f13ed`.

The observed master SH-2 PCs all map into the authenticated `DM.BIN` range,
but the trace does not bind a returned controller byte to a named menu action,
new-game state transition, `LEV01` load, or party pose.  It is therefore
transport and source-range evidence only; it does not authorize a native
menu/start consumer or substitute a start coordinate.  BIOS and capture
artifacts remain outside the repository and Firestaff's runtime.

## Japanese START/control VDP comparison — 2026-08-27

A clean V2 producer build was run twice against the same JP BIOS and original
CUE/BIN media over emulation frames 1800--1810: once with START held over the
eight captured frames and once with no injected input. Both raw VDP1/VDP2
captures pass the native V2 transport validator and have the identical SHA-256
`995929ef9e06700bdf333f278979c30f9b59872c14d4060a8ef8f59dae95f45a`.
The respective SMPC receipts differ (`11546931ede2b625b147fc41848830763de4fde388b522238265726c890bf8b0`
with START; `1ae296ff7c420d61f143333608193f5face85867021626b6af6a645f2a8ee218`
without), proving delivery changed while the observed VDP state did not.

The V2 producer also exposes that its normal Mednafen path has no usable
`PC_ID` value, so the analyzer reports `pipeline_unavailable` rather than
pretending to have a retired-instruction identity. This is negative visual and
instruction-binding evidence only. It does not authorize a Saturn menu,
LEV01 load, party pose, or a synthetic substitute.

## Japanese title-window control recheck — 2026-08-29

The JP 1.01 BIOS and the same hash-verified Japanese CUE/BIN media were run
again with the instrumented capture producer over frames 12590--12709, this
time with no injected input. The resulting 120-frame raw VDP1/VDP2 witness
passed the native transport validator and has SHA-256
`7e00dc4a3512e21d3662a9f85db3f139db18113bd5f926ac8489be4acbb7d1a8`.
It is byte-identical to the prior witness taken over the identical frame
window with a START pulse at frame 12596, across VDP1 state, VRAM and both
framebuffers plus VDP2 registers, VRAM and CRAM.

This is a controlled negative result: delivery of START does not make this
window the interactive title menu. The capture is external development
evidence, not a Firestaff runtime dependency, and the title/menu gate remains
fail-closed pending a source-owned consumer and transition witness.

## Japanese late START VDP comparison — 2026-08-27

The same clean V2 producer then held START for sixty frames after 30,000 boot
frames. The direct CUE/BIN run produced 60 validated VDP frames (SHA-256
`2506793d61d50420c10f30cef4b01da976ca9d301ab157463118a09a8f6a574c`),
120 injected-input rows (SHA-256
`055f4e14e9a8880742768f8ae8dc85a0fd928c6454f855cff65cdd6cb823ab2f`), and
525 SMPC reads over frames 30038--30065 (SHA-256
`991d29494c20170c1463570d5216849179b76cb89b77fb9a8a0aece8a82dc4c1`).

The first 60 raw frames are byte-identical to both the matching no-input
control and a previously captured START/A/B/C sequence. Thus a real controller
transport change still has no observed VDP consumer in this late title window.
The result remains a fail-closed block on menu semantics, level selection and
start pose; the raw receipts remain external to the repository.

## Japanese START high-RAM state comparison — 2026-08-27

Two V2 high-RAM snapshots were taken from direct JP CUE/BIN sessions at the
same frame 1810, one with START held from frame 1800 and one without input.
Both contain the exact 1 MiB `0x06000000`--`0x060fffff` WorkRAMH image. Their
payloads differ in six bytes: `0x0602c8f9`--`fa`, `0x0602c900`--`01`,
`0x0602c90c`, and `0x0602c910`; the latter two are `0x10` in the held run and
`0x00` in that control.

This is not a state-owner receipt. A repeat no-input snapshot at frame 1900
also differs from the first no-input run, including the same two long-lived
addresses. Their values therefore have an uncontrolled timing or session
component, and the held/control difference cannot be attributed to START.
The state owner and consumer remain capture-gated; Firestaff must not promote
these bytes into synthetic gameplay state.

A direct byte-write follow-up used a frame-windowed SH-2 WorkRAMH tracer for
`0x0602c8f0`--`0x0602c91f` over frames 1798--1804.  The START run and two
no-input runs have the same 154-write layout and the same producing PCs
(`0x0601438e`--`0x06014ade`), but values vary across no-input sessions too:
32 value differences occur between the two controls, versus 42 between START
and the first control.  This rules out the trace as an input attribution
receipt.  In particular, `0x06014518`, `0x06014588`, and `0x0601459a` are
observed writers, not proven title-menu or start-pose consumers.

## Japanese SMPC relocated-instruction receipt — 2026-08-27

A same-session JP capture over frames 1798--1802 pairs 105 master-SH-2 SMPC
reads with five exact 1 MiB WorkRAMH images. The debug capture loop provides
nonzero `PC_ID`/`PC_IF`; every captured `PID` word matches the corresponding
address in its same-frame runtime snapshot. The verifier also reads and hashes
the original `DM.BIN` in place, reporting
`master_sh2_instruction_identity=runtime_snapshot_verified`.

This resolves instruction identity after retail code has been relocated to
WorkRAMH, not controller semantics. The receipt neither proves a menu action
nor a new-game/`LEV01` transition, so native pose and title admission remain
closed.

## Japanese SMPC-to-WorkRAMH transport — 2026-08-27

A one-frame debug receipt at frame 1798 records the seven initial pad-bus
reads from `0x20100021` (SMPC byte registers `0x11`--`0x17`) with the live
destination base `R3=0x0602c900`. The verified relocated loop at
`0x06014510`--`0x06014518` reads each byte and stores it into that WorkRAMH
buffer before incrementing its index. Subsequent reads of `0x20100021` through
`0x20100025` feed nearby fields such as `0x0602c90c`, `0x0602c90e`, and
`0x0602c940`; the runtime code masks and packs nibbles while doing so.

This proves an authored controller transport and runtime buffering path. It
does not name the fields, identify a title selection, or establish a gameplay
action, so it is not a basis for an inferred START route or pose.

A one-frame START pulse at the same capture point changes SMPC register
`0x10` from `0x00` to `0x10` in that verified read chain. The copy source and
WorkRAMH destination remain the same. Other pad bytes differ across separate
sessions, so only this explicitly injected bit is attributed to START. This
is an input-transport receipt, not a title-menu or new-game receipt.

The V3 WorkRAMH-reader receipt records the live master-SH-2 register bank for
each bounded read. In the same JP START capture, `PC=0x06014388` reads the
buffer through `R4=0x0602c908` and `R3=0x0602c8f8`; the next reader at
`PC=0x0601439a` follows the linked words at `0x0602c90c` and `0x0602c910`.
This establishes a real input-buffer consumer chain. It still does not expose
the target's menu state or a level-load call, therefore it does not admit a
native new-game pose.

The same capture includes an exact frame-1798 1 MiB WorkRAMH snapshot. Runtime
disassembly shows the SMPC copy loop at `0x06014500`--`0x0601451a`, followed by
the nibble packing path at `0x06014568`--`0x060145c6` and the later branch/table
reader beginning at `0x06014620`. The 36-row receipt passes
`analyze_nexus_sh2_ram_read_trace.py` with
`workram_input_consumer_chain=verified`. These are input transport and
normalization instructions only: no observed branch target is identified as a
title selection, save action, `LEV01` load, coordinate, or facing assignment.

An independent 60-frame START hold at frame 1800 was compared against an
otherwise identical no-input JP run. Render receipts at frames 1750, 1800,
1850, 1900, 1950, and 2000 are byte-identical. This eliminates title-animation
changes as a false positive and proves no observed presentation transition for
that input interval. It does not prove that START is ignored globally, and it
cannot authorize a menu or gameplay route.

The same bounded A-button (`0x20`) hold produces the same byte-identical six
frame receipts against the no-input control. Both common confirmation inputs
therefore have negative evidence in this particular intro window. Further
capture work must move to a distinct observed input-ready state rather than
repeating the 1800--1860 interval.

The later A-button hold at frame 3000 likewise matches the no-input control
byte-for-byte at frames 2250, 2500, 2750, and 3000. The long intro window is
therefore not a valid substitute for a title-menu receipt; the next capture
must target a distinct input-ready state.

The no-input timeline remains title-composed at frames 8000 and 10000, then
becomes a completely black 704x480 PPM at frame 12000 (one RGB value across
all 337,920 pixels). This observed blanking boundary is not a menu or gameplay
transition, so it cannot supply a native startup route or a pose witness.

## Japanese retail start-pose disassembly audit — 2026-08-27

The Japanese CUE data track was read in place and the authenticated `DM.BIN`
member was statically disassembled in memory.  The resulting identities are:

| Member | ISO LBA | Size | MD5 | SHA-256 |
|---|---:|---:|---|---|
| `DM.BIN` | 44 | 555,144 | `e88d60859f65f08fa622e1992b02280f` | `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124` |
| `LEV01.DGN` | 500 | 280,576 | `751e1442bf7dccbd41bf146b5be144ab` | `b359a89ffa344439b6d0d15de223f3211caa7725b13dfdb4461b579c48f06723` |

`DM.BIN+0x366b8` and `+0x36990` contain the generic `LEV%02d.DGN`
format; the nearby `+0x36744`/`+0x36788` strings identify debug level
selection/loading, and `+0x36a18` names `SLEV%02d.BIN`.  There is no literal
`LEV01` reference apart from unrelated `SNDLEV01` text, and no static table or
call chain proves a new-game event, a `LEV01` consumer, or an `(x, y, facing)`
tuple.  This is negative semantic evidence, not a reason to substitute a
walkable cell or another inferred pose.  The native resolver therefore remains
capture-gated until an authenticated execution trace or save-consumer record
binds all four values.

## LEV01 door and entrance-source boundaries — 2026-08-27

DMWeb's Nexus DGN specification, checked against the hash-verified JP
`LEV01.DGN`, identifies Structure1E as 16-byte door records.  It documents
the grid coordinates, orientation, model index, initial closed/open state at
byte `0x08`, and movable-wall type `0x04` at byte `0x0d`.  The same source
identifies four LEV01 floor sensors: entrance face animation, entrance music,
an exit-door pressure plate, and the stairs to level 2.  This is sufficient
to preserve and validate the authored data records; it is not evidence of the
Saturn input/trigger dispatcher, state-write timing, audio consumer, or VDP
consumer.  Firestaff therefore must not enable the DM1-shaped door or sensor
runtime merely from these fields.

## Local Japanese post-render title witness — 2026-08-26

A separate, temporary developer capture used the hash-verified Japanese
Saturn 1.01 BIOS and the same original nine-track Japanese CUE/BIN set.  The
capture producer is outside Firestaff's runtime and the BIOS, raw capture and
its build artifacts were not retained in the repository.  Its ten
post-render frames pass the Firestaff raw-layout validator; frames 6--9 have
the measured active title NBG0 state exactly:

- NBG0 VRAM `0x00000`--`0x1ffff` SHA-256
  `ad10d99f00c3eecdf9577b15af1a7b86870a4ba83299dc50a09881dc569ad5e8`;
- little-endian VDP2 registers `TVMD=0x8000`, `BGON=0x0003`,
  `CHCTLA=0x0013`, `BMPNA=0x0000`.

This is a region-matched post-render observation and confirms the title-frame
boundary used by the fail-closed NBG0 tools.  The same capture session's
frame-12596 VDP2 trace contains exactly 32,850 clear writes tagged
`0x060230ac` and 31,616 byte-lane writes tagged `0x0602312c`; replaying all
64,466 writes
reconstructs the frame-6 NBG0 bytes and hash exactly.  A same-frame SH-2
instruction trace records the 31,616 preceding WorkRAM byte loads in order;
their values match the VDP2 byte lanes exactly.  The same JP session also
captures the 17,408 CDB FIFO payload words for `TITLE.BIN` LBAs 6039--6055,
checks every word against raw Track 1, and records the CDB data-port and FIFO
word position for the WorkRAM receipt.  The title/menu display consumer is
still unbound, so this does not authorize a playable start.

## Static SH-2 disassembly correction — 2026-08-26

The hash-verified retail `DM.BIN` was fully disassembled directly from the
original CUE member in memory with its verified load base `0x06010040`.
The frame-12596 PC tag `0x0602312c` is the loop checkpoint immediately after
the relevant instructions: `mov.b @r5+,r1` at `0x06023120` and `mov.b r1,@r4`
at `0x06023128`. The tag itself decodes as `cmp/pl r14`; its following branch
and delay slot advance the destination pointer. This resolves the observed
byte-copy instruction instead of guessing it from the PC tag. A full-member
scan also identifies an outer direct `BSR` from `0x06022772` to `0x060230c0`.
The authenticated frame-12596 trace instead records the copy helper's live
`PR=0x06023176` on all 31,616 rows. Those are not interchangeable: `PR` is a
live SH-2 link register and may be replaced by nested work before the sampled
store. The verifier reports the static edge and requires one consistent
observed PR, without falsely attributing it to the outer BSR. It does not
identify the transform or Saturn display consumer, so native title/menu
admission remains closed.

## Current external-data verification — 2026-08-13

## Authentic archive-source verification — 2026-08-14

Archive-only directory launch is now source-complete at the media handoff:
M12 admits the real English ISO member from the supplied `.7z`, and Nexus
directory discovery uses the same memory-backed ISO reader as direct archive
launch. A real copy of the supplied archive was tested without extracting or
writing game data. The remaining result is the existing authentic Saturn
presentation blocker, not a missing-data error.

The supplied authentic Nexus `.7z` can now be passed directly as the data
source. Firestaff selects the real English ISO member and opens its ISO 9660
tree in memory (`137` files); it does not rewrite the archive or create game
data files. The boot probe reaches the authentic title route and reports
`levelLoaded=0` with the existing Saturn-capture blocker, so the remaining
failure is presentation/start-pose evidence rather than media discovery.

## Verification-count correction — 2026-08-14

The current configured external-data CTest selection contains 184 Nexus tests:
173 pass and 11 are intentional capture-gated skips. The older 304/14 count
below belongs to a broader registration set and must not be presented as the
current configured run. Neither count opens the still-missing Saturn semantic
gates.

## Authenticated virtual-source read correction — 2026-08-14

Hash-based discovery may identify a real Nexus asset as a virtual source path,
for example `disc.iso::DM.BIN` or an archive member. The runtime now reads ISO
members with the sector-aware reader and other supported archive members with
the bounded in-memory reader. It no longer treats the virtual path as a host
filename, and it never writes a materialized game-data file. Real ISO, launch,
manifest, and hash-scan regressions pass.

## Media discovery correction — 2026-08-14

Nexus discovery now validates each CUE/BIN/ISO candidate before selecting it.
This is required for authentic data directories that contain more than one
regional image or unrelated media: directory enumeration order is not a source
identity. A candidate is admitted only when its ISO tree contains the real
`DM.BIN` and `LEV01.DGN` files. Firestaff reads the selected original image in
place; it does not unpack or rewrite game data. The focused ISO/CUE, launch,
manifest, and external-data tests pass.

When discovery first identifies a physical Track 1 payload, launch resolves
its CUE ownership strictly: a sibling CUE is accepted only when exactly one
sheet opens that exact payload as its Nexus ISO track. The native engine then
receives the CUE, preserving authored CDDA declarations without extraction.
Zero or multiple owners fail closed rather than selecting a filename guess.

The identical no-extraction provenance rule now covers the original ZIP:
when its internal CUE declares Tracks 2--9, each selection resolves to an
explicit `archive.zip::Track N.bin` member only after that member has been
validated in the archive. This is a CDDA source receipt, not host playback or
a replacement for the still-unbound Saturn CDDA selector/decoder.

The external checkout was tested against
`/Volumes/Extern-disk/FirestaffUserData/data/nexus`: all 304 registered Nexus
CTest cases completed successfully. Fourteen tests remain intentionally
capture-gated and are reported as skips, not as passing semantic evidence.
The real English `MENU.BPK` decoder independently decodes all 162 PRS3
surfaces (`test_nexus_v1_bppk`), and the engine exposes the resulting
`READY_DECODED` source route. This does not authorize Saturn presentation:
the menu still requires an authenticated PALT/VDP1 consumer join, and the
startup/menu, LEV01 pose, HUD/viewport, SLEV/SAL playback, and Saturn-save
production gates remain closed where their source-owned runtime witnesses are
missing.

## Correction — long SCSP traces remain structurally admitted only

The parser ceiling is now 64 MiB, so the authenticated gameplay traces of
about 33 MiB are no longer rejected before parsing. The two retained
`run-slev-scsp-gameplay-20260811j/k` pairs pass structural trace validation.
Their runtime join is still fail-closed: neither trace contains the
disassembly-bound SDDRVS handler PC `0x3224`. They therefore do not open the
producer-to-SDDRVS-to-SCSP production gate, and no event, MAP, SAL or playback
semantics are claimed.

No meaningful percentage can be based on the number of C files or tests. That
would count parser code and no-op gates as completed runtime. This document
therefore distinguishes implementation coverage from production readiness.
Implementation coverage measures built format, analysis, and capture support.
Production readiness measures what may actually be used in a playable,
region-matched Saturn chain.

## Current evidence correction — 2026-08-13

The CLI boot-probe path now selects SDL's dummy video driver by default when
no driver was supplied. This makes headless Nexus receipts reproducible on CI
and display-less hosts; it does not alter interactive rendering. With the
authentic external Nexus corpus, the probe exits cleanly and still reports
the source-owned title VDP-capture blocker rather than claiming a playable
menu.

The local Nexus boot-profile hardening is now verified: nested asset checks
honour the caller's diagnostic-buffer capacity instead of using the enum's
larger maximum. This prevents validation from corrupting adjacent runtime
state. The focused boot/launch set and the external-data Nexus selection both
pass after the fix. Two further operator-only J-BIOS capture attempts
(`run-followup-20260813c12` and `run-followup-20260813c14`) stopped before a
raw witness during the external Mednafen video/init profile, so they add only
negative emulator diagnostics and do not change the production percentages or
open any Saturn gate.

The latest attached-media J-region run at
`/Volumes/Extern-disk/nexus-capture-20260813/run-attachment-j-20260813/`
used the hash-verified J BIOS 1.01 and English merged CUE, and produced 1,200
validated raw frames with 1,156 active VDP1 observations. It still contains
no source-owned LEV01 level/x/y/facing record. The raw witness is retained as
negative transport evidence; `capture_exit_status=143` records that the
emulator was stopped after the complete frame set had been written. It does
not open the start-pose, save, HUD/viewport, or audio gates.

The later J-BIOS/English-Merged run at
`/Volumes/Extern-disk/nexus-capture-20260813/run-followup-20260813c5/`
used the documented active-low debug sequence and produced a validator-clean
1,200-frame raw witness. It did not produce a source-owned LEV01 level/x/y/
facing record. Its long operator process ended before the launcher's final
receipt append, so the capture is diagnostic evidence only and is not counted
as a complete semantic gate. The launcher now finalizes `capture_exit_status`
and available trace hashes on signal/timeout; the regression is covered by the
raw-capture launcher test. This does not change the production status below:
Saturn startup, HUD/viewport, SLEV/SAL playback, and Saturn save import remain
closed until their source-owned runtime joins are authenticated.

The verified Nexus startup now explicitly distinguishes title/asset boot from
a playable start. Firestaff no longer writes `NEXUS STARTUP RECEIPT READY`
when `levelLoaded=0`; it instead reports `status=blocked` with the current
source-owned blocker. This does not change production readiness: the authentic
startup→menu→LEV01 chain still lacks a verified start-position and consumer
witness.

Current external-disk audit, 2026-08-13: the verified game corpus contains
CUE/ISO and extracted retail files. A new isolated J-BIOS/English-Merged
capture is now available as operator evidence on external disk:
`/Volumes/Extern-disk/nexus-capture-20260813/run-jp-merged/`. Den binder
binds BIOS and disc hashes, 60 raw frames, and 16 active VDP1 observations;
the separate validator passes with `--require-frames 60 --require-vdp1-activity`.
The capture is nevertheless semantic-blocked: no byte-exact startup→menu
identity, start pose, HUD/viewport consumer, or SLEV/SAL dispatch is verified.
Saturn BIOS, disc, and capture bytes remain outside the repository and must not
be treated as a Firestaff distribution.

| Area | Verified gates | Implementation coverage | Production readiness | Primary blocker |
|---|---:|---:|---:|---|
| Startup | 3/6 | 50.0 % | 0 % | The J-BIOS/media pair is now available and a reset frame is validated, but no valid startup→menu witness exists |
| Menu | 3/8 | 37.5 % | 0 % | PRS3 byte decoding, the NBG1 consumer, and a separate FONT256 CG/palette join exist, but page/text-code mapping and actual menu composition are missing |
| DGN face/mesh/texture | 4/7 | 57.1 % | 0 % | Format, mesh topology, material owner, and DMWeb 08h/28h source decoding are bound, but selector/UV, runtime transform, culling, and rasterization are missing |
| Saturn VDP1 capture | 9/12 | 75.0 % | 0 % | Raw capture, command framing, material/CLUT join, atomic replay, multi-command sequence, display origin, and separate direct-colour pixel decoding are verified; scene owner, Saturn face selection, transform/culling, and production consumer are missing |
| HUD/viewport | 1/7 | 14.3 % | 0 % | Layout/adapters and capture-only composition exist, but no authenticated VDP1/VDP2 pixel handoff to production |
| SLEV/SAL/SDDRVS | 2/8 | 25.0 % | 0 % | Corpus, driver, and write traces exist; selector, codec, MAP binding, event dispatch, and actual playback are unproven |

The arithmetic mean of implementation coverage is
`(50,0 + 37,5 + 57,1 + 75,0 + 14,3 + 25,0) / 6 = 44,0 %`.
As a checksum, the named gates are `22/48 = 45,8 %`; that figure does not
replace the area mean, because an area would otherwise weigh more merely for
having more sub-gates. For the prioritized startup → menu → HUD/viewport chain,
with weights 30/35/35, implementation coverage is
`50,0 × 0,30 + 37,5 × 0,35 + 14,3 × 0,35 = 33,1 %`.
Production readiness is currently 0% for both measures: the validated J
capture is a reset/transport witness without startup→menu identity, and the
existing E-BIOS/French capture likewise opens no semantic runtime consumer.
The figures must not be averaged across models.

## External retail corpus

The separate authentic corpus on external disk is verified through launcher
boot and level probes: `FIRESTAFF_NEXUS_DATA_DIR` points to the corpus, the
additional English CUE opens without repacking files, title and warning
surfaces load, and `LEV01.DGN`–`LEV15.DGN` pass the real 64×64 structure and
playability probe. This confirms data access and format reading, but not the
missing Saturn startup→LEV01 pose or VDP consumers. Firestaff must therefore
still stop before playable runtime where no authenticated save or Saturn witness
exists.

## Counting rule

Implementation coverage is the number of verified gates divided by an area's
named gates; the mean over the table's six areas is rounded to one decimal.
A parser, hash, or no-op may count only toward the gate it actually verifies.

The gates are fixed in this revision: startup = data/BIOS, region, input,
capture, start identity, startup→menu; menu = BPK, PRS3, source row,
pixel/mode semantics, palette, VDP2 map, FONT256, menu capture; DGN = DGN
format, mesh topology, face/material ownership, texture semantics, transform,
culling, production raster; VDP1 = raw transport, authenticated frame, VDP1
state, command framing, texture/CLUT join, direct-colour pixel decoding, atomic
replay, multi-command sequence, display origin, scene owner, face selection,
transform/culling; HUD/viewport = layout, HUD source, VDP2 source, VDP1 source,
pixel handoff, composition, production consumer; SLEV/SAL/SDDRVS = corpus,
driver, trace, selector, codec, MAP binding, event dispatch, playback.

Production readiness is a gated measure: original data, the Saturn runtime
owner, correct BIOS/media region, and production consumer must be bound in the
same verified witness. Parser, hash, static disassembly, capture-only adapter,
and no-op count as partial implementation coverage but as zero production
readiness.

The European VDP2 source comparator accepts both authenticated English and
French `MENU.BPK`. Missing optional sources, such as `TITLE.CG` in a partial
external extraction, are reported separately and do not invalidate the entire
raw witness; they do not open semantic admission.

## Latest authenticated VDP1 window

## Regionmatchad J/J-startup-witness

External disk now holds a separate hash-bound capture using Japanese Saturn
BIOS 1.01 and the Japanese-region English Nexus disc. Mednafen reports
`SGAREA=J`, and the launcher manifest binds BIOS and CUE hashes to 560 frames.
During the targeted reset window, VDP1 source `0x63e00` is written by the
observed SH-2 corridor (`pc0=0x0601307c`); the same session has a raw VDP1
snapshot and a verified write trace. This is stronger startup/writer provenance
than the E/French runs, but source spans still lack a byte-exact match in
MENU.BPK, TITLE.BIN/TITLE.CG, FONT256.S2D, or STABG.BIN. NBG1 bitmap state
(`TVMD=0x0080`, `BGON=0x0002`, `CHCTLA=0x1211`) and the VDP1 direct-colour
command are therefore capture evidence without menu identity or a production
consumer.

The region-matched J/J session
`run-codex-j-menu-long-20260809/runtime-vdp12.raw`
is now verified with 1,200 frames, BIOS J 1.01, a full merged disc, and START
handoff. VDP1 state reports `SysClipX=319, SysClipY=223` throughout the window.
VDP1 VRAM changes, but the examined frame-500 record is a mode-5 direct-colour
draw from `0x63e00` (33,280 bytes) with no exact match in the hash-verified
retail corpus. VDP2 is unchanged in its register/VRAM/CRAM configuration and
remains a lone NBG1 bitmap observation without a MENU.BPK/FONT256/TITLE/STABG
join. The session is therefore negative, authenticated evidence for transport/
state—not startup→menu or HUD/viewport evidence. The production gate remains
closed.

In the local, user-owned capture corpus,
`run-codex-menu-long-20260809f/runtime-vdp12.raw`,
frame 760 is the best investigated DGN candidate. C's command-chain adapter
binds 242 records and verifies order, clip, local coordinate, and END state,
but the first textured draw is direct-colour (mode 5, source `0x5ad68`,
130,320 bytes) and matches no byte-exact span in the hash-verified retail
corpus. A separate mode-1 observation at `0x0e160` matches LEV00 Structure2
image 49, but is not sufficient by itself to replace the unproven draw in the
full chain. Frame 760 is therefore capture-only evidence; it must not be used
as complete scene replay or proof of Saturn face selection, transform, culling,
or production rasterization. The C adapter
`nexus_v1_saturn_runtime_capture_frame()`
reads the same authenticated VDP1/VDP2 raw envelope in C, and
`nexus_v1_vdp1_capture_replay_runtime_frame()` passes VDP1 VRAM/COPR directly
to the bounded replay chain. The new `nexus_v1_vdp1_dgn_material_resolver()`
binds a verified LEV file to a unique mode-1 image and a unique reusable CLUT
with byte-identical Saturn ordering; palette ownership need not reside on the
same Structure2 descriptor as the image. CMDCOLR is converted from Saturn word
address to the correct byte offset (`<<3`). Ambiguous or unattested sources
are rejected. It remains capture-only, and consequently leaves face selection,
transform, culling, direct-colour materials, and ordinary production blocked.

### Direct-colour lane

VDP1 mode 5 is now semantically decoded in the separate capture-only lane
`nexus_v1_vdp1_capture_decode_direct_color()`. It follows Mednafen 1.32.1
`src/ss/vdp1.cpp::TexFetch` for 16-bit 32K-RGB words and ECD's transparency
code `(word & 0xc000) == 0x4000`, and writes to a separate RGBA surface rather
than quantizing colour to the Nexus indexed framebuffer. A synthetic test and
an external gameplay capture pass. The lane still always sets
`renderer_permitted=0`: it proves VDP1 pixel semantics, not DGN ownership,
face selection, transform, or production rasterization. It therefore does not
raise production readiness or the Nexus V1 target score. Frame 760's first
mode-5 record still has no byte-exact retail owner; its command-chain join also
lacks sufficient valid screen coordinates to open full replay.

The authenticated raw-frame chain is also now available as
`nexus_v1_vdp1_capture_decode_direct_color_runtime_frame()`. Den binder frame,
binds frame, COPR/command list, display origin, and the selected mode-5 command
to the same raw capture and returns the command offset in its receipt. The API
remains explicitly capture-only; it creates neither menu identity, material
ownership, nor a production consumer.

The gameplay witness also now has
`nexus_v1_vdp1_capture_replay_runtime_frame_mode1_material()`. It follows the
frame's complete COPR chain, tries only mode-1 draws, and passes a draw to the
verified DGN image/CLUT resolver when both spans match exactly. External EU
capture frame 760 passes with `LEV00.DGN`: analysis shows 227 of 231 mode-1
draws with image and palette matches and 198 with Structure3 face ownership;
the C test renders one such draw through the same runtime API. This is not yet
a full scene renderer: Saturn face selection, camera transform, culling, and
scene ownership of the complete draw list remain blocked.

The separate
`nexus_v1_vdp1_capture_replay_runtime_frame_mode1_sequence()` lane
now traverses the entire authenticated command list in frame 760 and atomically
replays the source-bound mode-1 subset. C verification counts 242 command
records, 235 draw records, 218 exact DGN image/CLUT joins, 16 unowned mode-1
records, and one unowned non-mode-1 record; seven control records remain
separate. The missing type-9 `system clip` record is separately marked in the
receipt and must not be replaced with a user clip or host bounds. When
authenticated runtime state exists, replay instead uses its live
`SysClipX/SysClipY` as the inclusive raster envelope. This remains neither a
full Saturn scene nor a production compositor because scene ownership and
several draw records are missing.

The instrumented Mednafen source now also captures VDP1's separate
`SysClipX/SysClipY` state. In a new 800-frame EU capture, frame 760's values are
`0x013f/0x00ff` (319×255), despite the command list having zero type-9
system-clip records. The C parser accepts both old and new capture rows and
marks state provenance explicitly. Gameplay frame 760 with `(319,223)` now
passes the source-bound VDP1 subset with the clip consumer active. A capture
with `SysClipY=255` preserves the receipt but closes `renderer_permitted`
against the 224-line host surface. VDP12 composition propagates this to the
viewport receipt; HUD and full viewport production remain closed until scene
ownership and display-window transform are verified.

The VDP2 raw format is now also correctly bound in C: every frame has 4,096
bytes of CRAM, 524,288 bytes of VRAM, and a 512-byte register window in the
same order as the external capture validator: `RawRegs → VRAM → CRAM`. The C
reader and `nexus_v1_saturn_runtime_capture_vdp2_register_receipt()` now use
the same order and deterministically select register byte order. At frame 80,
the authenticated English long run shows `TVMD=0x8000`, `BGON=0x0003`, and
active NBG1 in character mode; this is a hardware-state observation, not menu
ownership or a text binding.
`nexus_v1_vdp2_capture_replay_runtime_frame_nbg1_tilemap()`
can feed an authenticated raw frame to the NBG1 tilemap compositor when a
caller has already attested the source name table, character generator, CRAM,
and exact VRAM offsets. It does not guess MENU.BPK/FONT256 ownership or
placement; menu and production readiness therefore remain unchanged.

`nexus_v1_font256_vdp2_capture_join()` requires the same attested FONT256.S2D
for an exact CG span and 256-colour palette span. A changed source or capture
is rejected. The join does not set text-code→tile, page PND, placement, or layer
ownership; it is source provenance, not an unlock of FONT256 runtime or
startup→menu.

VDP2 also has an independent raw-capture consumer,
`nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_bitmap()`. It decodes the
authenticated frame's NBG1 512×256/8bpp span and CRAM to a separate RGBA
surface and verifies register byte order, BMPNA/CRAOFA addressing, and the same
frame envelope. J/J frame 500 passes this lane; the span is fully transparent,
which is preserved as `valid=1, written_pixels=0` rather than fabricating a
menu image. The API continues to block the production consumer and asset owner.

The title lane has a corresponding separate NBG0 consumer:
`nexus_v1_vdp2_capture_decode_runtime_frame_nbg0_bitmap()`. It accepts only
the measured 512×256/8bpp lane at VRAM offset zero and BMPNA and CRAOFA bank
zero, and produces a capture-only RGBA surface with `renderer_permitted=0`.
A new transient check against the owned JP CUE and JP-1.01 BIOS passed on
2026-08-26 for the title frame: `TVMD=0x8000`,
`BGON=0x0003`, `CHCTLA=0x0013`, `BMPNA=0x0000`; the first 131,072
VRAM bytes have SHA-256
`ad10d99f00c3eecdf9577b15af1a7b86870a4ba83299dc50a09881dc569ad5e8`.
Neither BIOS, capture, nor external producer is a Firestaff runtime dependency,
and the decoder does not establish VDP1 priority, title/menu ownership, timing,
or host-window placement.

The raw-frame receipt additionally exports `PRINA`/`PRINB` and unmasked
NBG0/NBG1 priorities using the same verified register byte order. This makes
the observed VDP2 ordering available to the next native gate, but is not proof
of VDP1 priority, window logic, colour calculation, or final composition.

A new transient JP title capture from the same 2026-08-26 revision also makes
the window gate concrete: `WCTLA–WCTLD=0x0000`, `SPCTL=0x1536`,
`CCCTL=0x0043`, `PRINA=0x0404`, and `PRINB=0x0000`. NBG0 and NBG1 therefore
have the same observed VDP2 priority (4) in this frame, and the four VDP2
window-control words are zero. This does not exclude VDP1's separate priority,
colour-calculation rules, or other timing; these facts do not open the native
final compositor.

### Frame 80: NBG1 owner remains unbound

A separate byte comparison of frame 80 from the authenticated long run shows
`BGON=0x0003`, `CHCTLA=0x1013`, and NBG1 in character mode. NBG1's visible raw
span lies in VDP2 VRAM around `0x5c000` and consists of two-word PND patterns;
it is unchanged between frames 78 and 80. No exact byte sequence for the
FONT256.S2D Page, Character Generator, or Palette region occurs there, and no
MENU.BPK/PRS3 source binds the same span. This is negative provenance evidence:
frame 80 must not be attributed to FONT256 or MENU.BPK and must not yet feed
the production compositor. The menu gate therefore remains at 3/8 and
production readiness at 0%.

### SLEV/SAL/SDDRVS trace gate

`nexus_v1_scsp_write_trace_parse()` and the separate main-SH-2 parser now save
the first raw-byte offsets for the produced mailbox command, sound-CPU mailbox,
SDDRVS handler, and SCSP voice register. For the sound-CPU trace, the order
mailbox → handler → voice register is marked only when it actually occurs in
the same raw trace. Two separate trace files lack a common time base and must
not therefore be used to claim event ownership, SAL codec, MAP binding, or
playback.

The external `NXSLSC01` artifact was inventoried with
`scripts/analyze_nexus_slev_capture_envelope.py`: header and payload are
structurally valid, all 65,536 records are SH-2 writes, but none of the four
retail FNV identities for `SLEV00.BIN`, `SNDLEV00.SAL`, `SNDLEV00.MAP`, and
`SDDRVS.TSK` matches. It is therefore a runtime observation, not a source-bound
capture. The SLEV/SAL/SDDRVS gate remains at 2/8 and production readiness at
0%.
