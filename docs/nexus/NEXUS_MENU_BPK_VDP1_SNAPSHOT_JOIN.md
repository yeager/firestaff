# Nexus `MENU.BPK` to VDP1 snapshot join

This receipt retains a byte-level observation from the Japanese retail title
menu. It is deliberately narrower than a renderer admission: it proves a
subset of real `MENU.BPK` output was present in one VDP1 VRAM snapshot, not
which VDP1 commands consumed it or how the Saturn palette/compositor presented
it.

## Inputs

| Input | SHA-256 |
| --- | --- |
| Japanese retail `MENU.BPK` | `740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636` |
| Paused retail-title-menu `vdp1-vram.bin` | `968e9d0f05de45ff8c97ea562988cd0aea7c125b54822983ca52c12edc88b895` |

The 512 KiB VDP1 image belongs to
`nexus_v1_ymir_title_menu_replay_20261020`. Its accompanying receipt identifies
the state as a Japanese retail-disc boot followed by one Start press at the
title prompt. It is an emulator observation, not a hardware capture.
The snapshot has no command trace from the same capture, so do not join it
with a trace from another run.

## Reproduction

```sh
python3 scripts/analyze_nexus_menu_bpk_vdp1_snapshot_join.py \
  /home/trv2/.firestaff/data/nexus/MENU.BPK \
  /home/trv2/work/firestaff-parity-debug-20261024/parity-evidence/verification/nexus_v1_ymir_title_menu_replay_20261020/dumps/vdp1-vram.bin \
  --expected-vdp1-sha256 968e9d0f05de45ff8c97ea562988cd0aea7c125b54822983ca52c12edc88b895
```

Only add `--vdp1-command-trace` when the trace and VRAM dump are from one
atomic capture, and pair it with `--vdp1-command-trace-frame` for the frame
represented by the snapshot. This retained replay has no such command trace.

The tool rejects a different `MENU.BPK` identity, malformed BPPK/PRS3 framing,
an image that is not a 512 KiB VDP1 dump, or a supplied snapshot hash mismatch.
It decodes the documented bounded DMWeb PRS3 stream and searches for every
possibly overlapping occurrence of each whole decoded surface, directly and
with 16-bit word order swapped. A supplied command trace must come from the
same capture as the VRAM snapshot. The analyzer counts source-start checks
only for its explicitly paired frame.

## Result

The hash-pinned rerun decodes all 162 declared PRS3 surfaces. Entries **77
through 162** occur byte-exactly in VDP1 VRAM (86/162 surfaces); none of those
joins requires word swapping. Their first direct-match offsets descend from
`0x6c700` (entry 77) to `0x61d60` (entry 162). The other 76 declared surfaces
do not have a whole-surface direct or word-swapped match in this snapshot.

This is byte-level evidence that authentic menu-package surfaces were present
in VDP1 VRAM. It does not prove which command consumed them or authorize a host
menu renderer. The following independent evidence is still required before
changing presentation admission:

- a command-chain receipt tying each admitted source range to rendered commands;
- Saturn CLUT/palette ownership and pixel-mode evidence;
- destination placement/order evidence; and
- an end-to-end captured comparison of the admitted route.

Until then, the tool emits `semantic_admission=blocked` and no decoded pixels
are handed to the production renderer.

## Command trace status

### Retained same-run capture (2026-10-26)

A new Ymir 0.4.2-dev run loaded the hash-verified Japanese title-prompt
state, held Start for 60 of 120 capture frames, and dumped memory and the
VDP1 trace in the same run. The saved screenshot shows the retail three-choice
menu. The VDP1 VRAM dump has SHA-256
`0989ce1cba450dbe5a766d346b979ac038a236589225f3f795c8dc06f2ec6ea7`;
the complete command CSV has SHA-256
`b80c7a118e7f9df4f941804cdc045f4024336080504c512bbdf5c4e0df19d8d3`.
The bundle records 10,668 command rows, zero dropped rows, and 178 commands
in its last captured VDP1 frame, index 59. Filtering the CSV to frame 59 and
joining with the final VRAM dump finds 86 whole `MENU.BPK` surface matches,
137 executed textured commands, and no textured command whose source start falls in
those matched ranges.

An initial analysis incorrectly included skipped VDP1 commands and reversed
the character width and height fields. With both corrected, the independent
source scan initially matched all 137 executed texture spans to retail files.
Eleven spans are all zero and also match unrelated file padding, so they do
not establish source identity. Excluding those uninformative matches leaves
126 of 137 spans with nonzero byte matches to authenticated retail files.
This is a byte-source observation, not proof of final pixel composition or
input behavior.

The source scanner verifies each MNS file against its retail hash before
extracting candidate surfaces. Re-running the retained frame with this check
still finds 126 informative source matches among the 137 textured commands.

Five informative commands in that trace instead match the Japanese retail
`TITLE.BIN` (SHA-256
`51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3`).
Commands `0xd8e0` and `0xd900` both match 288 bytes at file offset `0x15f6`;
commands `0xd920`, `0xd940`, and `0xd960` match spans of 256, 256, and
416 bytes at offsets `0x0446`, `0x0576`, and `0x07d6`, respectively. Each
is a normal sprite in 4-bit colour-bank mode, with captured CMDCOLR values
`0x52f0`, `0x52f0`, `0x5220`, `0x5230`, and `0x5250`. These are byte
matches against the later VRAM snapshot, not a time-aligned palette or a
proof of the final menu image. The later full-frame capture replay combines
the captured VDP1 display framebuffer with VDP2 state; it does not reconstruct
the frame by replaying these command records. Separate capture-only paths
resolve source-bound mode-1 lookup textures and preview or blend captured
mode-0 bank sprites. Those paths do not establish sprite ordering or priority,
VDP2 layer selection, or a time-aligned reference image.
`nexus_v1_vdp1_decode_mode0_bank_texture` now decodes the sprite's packed
indices and VDP1 colour-bank words from a VRAM snapshot. The retained `0xd8e0`
command and its 576 texels pass the external capture test. These words still
need transparency, end-code, sprite priority and time-aligned VDP2 colour RAM
handling before they can produce a trustworthy displayed image.
The retained VDP2 snapshot has SPCTL `0x1536` (sprite type 6), RAMCTL
`0x0300` (CRAM mode 0), and CRAOFB `0x0000`. The mode-0 type-6 capture
resolver now maps the observed title sprite's ten-bit colour indices through
that captured CRAM; the external test verifies its palette addresses and RGB
components. It rejects unsupported sprite formats, enabled end codes, and
shadow/RGB cases. The separately dumped state remains a snapshot, so this is
not proof of time-aligned composition or permission to display the result.
An isolated normal-sprite preview now places these mode-0 pixels using the
captured local coordinate and clip, including horizontal and vertical flips.
The external test checks the `0xd8e0` sprite at `(104,144)` against all 576
decoded texels. It remains a capture preview; sprite priority, the other VDP1
commands, VDP2 layers, and a time-aligned reference image are not established.
With all external capture paths set, the test now exercises the preview for all
five commands, including the horizontally flipped sprite. Its visible pixels
are checked against both captured VDP1 buffers and the captured CRAM values.
The test accepts `FIRESTAFF_NEXUS_YMIR_DUMP` (VDP1 VRAM),
`FIRESTAFF_NEXUS_VDP1_TRACE` (frame-59 little-endian records),
`FIRESTAFF_NEXUS_YMIR_CRAM`, `FIRESTAFF_NEXUS_YMIR_REGS`, and
`FIRESTAFF_NEXUS_YMIR_VDP1_FBS` (Ymir's draw/display buffers). None of these
game-derived files is stored in the repository.

The retained Ymir framebuffer dump supplies a stronger VDP1 comparison. Both
the draw and display buffers contain the expected big-endian colour-bank word
at every nontransparent pixel of all five sprites: 213, 213, 313, 293, and
463 pixels respectively, for 1,495 matches out of 1,495 per buffer. The
external lookup test checks the command positions and horizontal flip against
both buffers when `FIRESTAFF_NEXUS_YMIR_VDP1_FBS` is provided alongside the
trace and VRAM paths. Transparent pixels and final VDP2 output are not covered
by this comparison, and the dump does not prove per-command snapshot timing.
The separately saved 640×448 screenshot contains all 1,495 nontransparent
sprite pixels at twice the VDP1 coordinates. Their RGB channels equal the
captured CRAM components converted to 8 bits and multiplied by 31/32, rounding
down. The earlier zero-match result compared unscaled colours and is withdrawn.
Recheck with `python3 scripts/verify_nexus_title_sprite_screenshot.py BUNDLE
SCREENSHOT TITLE.BIN`; Pillow is required. The retained registers explain the
top-layer factor: CCCTL `0x0043` enables sprite colour calculation in top-ratio
mode, SPCTL `0x1536` selects the priority-equals-5 condition, and PRISC
`0x0504` assigns priority 5 to these type-6 pixels. Their ratio index is zero
and CCRSA `0x1000` selects a top ratio of 31/32. At all 1,495 compared
positions, the captured NBG0 bitmap contains transparent index zero. The
captured per-line back-screen table is black on the affected lines. The
verification now reads these lower-layer values from the VDP2 dump instead of
assuming black. The screenshot was taken after the dump, so exact capture
timing remains unproven; this covers these sprites, not the complete image or
menu interaction.
The capture preview can now blend a normal mode-0 sprite over a supplied lower
framebuffer using type-6 priority and top-screen ratio registers. The retail
capture regression checks the observed 31/32 result over black and the blend
over a nonblack test background. It does not select the lower VDP2 layer or
apply sprite-window masking; those inputs must be established separately.

For mode-1 palette analysis, Ymir's software VDP1 renderer counts two end codes
per rasterized line: each end code is skipped and the second stops the line.
The no-draw palette decoder follows that count. The triangle capture
compositor also skips the first code and stops at the second in source order,
but this remains an approximation: VDP1 resampling and texture direction can
change which texels are encountered. If the texture contains a code, its preview
cannot authorize rendering.

The same CMDSIZE and skipped-command corrections also apply to the Mednafen
VDP1 source, material, and write-join analyzers. Earlier numerical receipts
produced by those analyzers must be regenerated against their original raw
captures before they are used as evidence; they are not automatically
validated by the Ymir result above.

One retained Mednafen startup capture has now been reanalyzed with the
corrected command decoder. Frame 0 of the four-frame
`nexus_v1_startup_render_27518_20261009` capture (SHA-256
`1eaa507e314b60f1cc123dd6eb9fbcff4e635dcd1ea5b6cb7aeb16d12e33baae`)
has a 236-record chain and 221 executed textured draws. The authenticated
`LEV00.DGN` material join finds 215 matching source/palette pairs, including
186 with Structure3 face owners. Six draws remain unmatched. This is a
capture-level result; it does not establish display composition or permit
production rendering.

The broader source-only scan counts 201 of 221 draws with nonzero matching
retail bytes when the verified Japanese `ITEM.IBS` is included. Of the six
unmatched source/palette pairs, five use the same 448-byte all-zero texture,
which cannot identify a resource. The remaining draw at command `0x00f00`
uses a 1,152-byte span matching the word-swapped Japanese `ITEM.IBS` bytes
at file offset `0x123dc`; the file SHA-256 is
`fc32ca5875906e6e0dc69e0b5edfa5d00cb1f4401b7d497397c699be7c4530c1`.
This initial file-level join does not establish item identity, placement, or
production renderer permission. A matching all-zero image must
not be used to infer its owner or promote a renderer gate.

After aligning the C chain walker with Ymir's single return-address model, the
snapshot heuristic selected an invalid longer candidate that drew before any
captured local coordinate. Candidates without an origin before their first draw
are now excluded. On the same frame the C replay selects 222 draws, resolves
221, and verifies 201 nonzero source/palette joins. This remains a heuristic
snapshot chain, not proof of the original execution order; the Python scanner
and direct trace may select different chains.

Further inspection of that same Japanese `ITEM.IBS` and capture binds the
1,152-byte image to floor-image descriptor 33: encoding `0x0008`, dimensions
48×48, palette id 8. Its descriptor reuses the palette first supplied by
descriptor 0 at file offset `0xb098`. All 32 palette bytes, after the same
16-bit word swap, equal the VDP1 command's captured CLUT at VRAM byte offset
`0x18820`. This establishes the descriptor, image bytes, and CLUT byte join;
the displayed item's game identity, transform, and composition remain open.
The retained files can be rechecked with
`python3 scripts/verify_nexus_item_floor_capture.py CAPTURE ITEM.IBS --frames 4 --frame 0 --command-offset 0xf00 --floor-index 33`.
Firestaff's verified ITEM.IBS parser now resolves reused palettes by palette
ID; previously it reused the most recently declared palette even when that
palette belonged to another ID. The real Japanese corpus regression checks
all 16 words for descriptor 33 against the original palette at `0xb098`.

The frame counter in the VDP1 trace is distinct from the 120 requested capture
steps, and the screenshot is taken after the dump. This receipt therefore
does not establish that all three artifacts represent precisely the same
displayed frame or exclude use of `MENU.BPK` in other frames. It also tests
source starts, not complete texture spans. The raw files are retained on the
external evidence volume; no game-data payload is committed.

Comparing the 178 traced command records in frame 59 to the final VRAM
snapshot gives 177 exact 32-byte matches. The first traced command at address
zero differs in its link word: the trace has `0x1874`, while the final dump
has `0x0004`. This confirms that the command table changed between execution
and the snapshot; it does not identify when any texture bytes changed. A
snapshot-only source join therefore cannot prove the historical command chain
or promote a renderer on its own.

The recorded frame-59 command order was independently checked against each
record's CMDCTRL jump mode and CMDLINK: all 177 transitions match, and the last record at
`0xd9a0` has END set. The observed root is `0x00000`; its skip-and-jump
command uses the traced link `0x1874`. This validates the recorded order for
this Ymir frame, without claiming that its texture bytes or later screenshot
were frozen at each command's execution time.

A previous draft of this note reported a one-frame VRAM/command-trace pair and
concluded that no textured command started in a matched `MENU.BPK` range. The
two raw files are not present together in the retained evidence, so that join
cannot be independently reproduced. Withdraw the 168-command, 135-textured,
zero-source-hit result and do not use it as evidence. The retained 2026-10-25
command history and the 2026-10-20 VRAM snapshot are separate captures;
pairing them would be invalid. A new atomic capture with both outputs is
required before making any command-consumption claim.

The confirmed VRAM snapshot proves only that 86 real `MENU.BPK` surfaces were
resident in that snapshot. It does not establish whether the title menu reads
them. This remains non-admission evidence and does not identify the active
menu assets, palette, placement or compositor path.

## Five-frame Start write trace

A separate isolated capture loaded the same retained Japanese title-prompt
state, held Control Pad 1 Start for capture frame zero, and ran five frames with
`--debug --trace-sh2-writes-begin 0x05c00000 --trace-sh2-writes-end
0x05c80000`. Ymir wrote a complete 1,082-row VDP1 SH-2 trace without dropped
rows. It is intentionally a post-state-load observation: it can identify work
caused by Start, but cannot claim to contain the earlier pre-load that produced
the title-prompt textures. Its trace is not paired with the 2026-10-20 VRAM
snapshot above.

The most frequent writer was master SH-2 PC `0x0608ba46`, with 1,004 writes in
`0x05c000c0..0x05c18710`. Disassembly of the captured WRAM bytes at that PC is
a zero-fill loop (`mov.l r5,@r4`, then `add #16,r4` and branch). It is therefore
a VDP1 clear, not a texture upload. No traced write covers the title-frame
texture starts `0x05c02460`, `0x05c02660`, or `0x05c02740`, nor the resident
`MENU.BPK` match interval.

Use the retained deterministic trace with:

```sh
python3 scripts/analyze_nexus_vdp1_sh2_write_trace.py \
  /path/to/sh2-memory-write-trace.csv \
  --source-address 0x05c02460 \
  --source-address 0x05c02660 \
  --source-address 0x05c02740
```

It retains only the observed PC/address facts and always leaves semantic
admission blocked.

The C command-sequence validator also accepts ordered command words from a
producer trace. It checks every jump, call, return and END, and tracks local
coordinates without applying skipped commands. The caller must first verify
that the trace covers a complete frame and that its producer dropped no rows.
The Japanese retail title-menu trace at Ymir frame 59 passes with 178 commands;
the captured words are used directly, rather than substituted from the later
VRAM snapshot. This does not establish that textures or the final image match.
Transition checks follow Ymir's VDP1 model: a single saved return address,
sequential execution on a return without a call, 32-byte link alignment, and
512 KiB address wrapping. Ymir describes nested-call behavior as observed,
so this model is not independent hardware verification.
The capture compositor can consume those traced command words while resolving
materials from a separate VRAM snapshot. It requires explicit verification of
trace completeness and keeps renderer permission disabled because the later
snapshot alone cannot prove the texture and palette state at execution time.

## Full Ymir title-frame capture replay

The atomic frame-59 Ymir bundle also contains the VDP1 display framebuffer,
VDP2 VRAM, CRAM, and registers. The bounded C function
`nexus_v1_vdp2_capture_replay_ymir_title_menu` now reconstructs the full
320x224 capture from those real buffers. It admits only the observed NBG0
8bpp bitmap configuration, register offsets, scroll/scale, priority and colour
calculation settings, black per-line back screen, and type-6 sprite palette
lane. It rejects missing capture/trace attestations, other register layouts,
unexpected sprite priorities, and a nonblack back-screen row.

The verifier pins all inputs to this retained capture by SHA-256:

| Capture input | SHA-256 |
| --- | --- |
| `vdp1-fbs.bin` | `2dc15ef07aaacd89a00af18fe3e0bf3677bdb626f3a852f37820eb29bc9ce1ce` |
| `vdp2-vram.bin` | `de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c` |
| `vdp2-cram.bin` | `db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de` |
| `vdp2-registers.bin` | `79efa616c2db727749aa9018864d1ad32c1583935652fafb16534ba69f8e1f2d` |
| `vdp1-command-trace.csv` | `b80c7a118e7f9df4f941804cdc045f4024336080504c512bbdf5c4e0df19d8d3` |
| Frame-59 screenshot | `8721cce3354bc20b1cd367fd44be3d4ecc60ca65fa89c7d00654e83f5ad7db23` |

The display framebuffer contributes 70,185 direct RGB words. NBG0 overlays
18,513 pixels with the captured CRAM and 31/32 top ratio; 1,495 indexed
type-6 VDP1 pixels use their captured priority and CRAM words over the
verified black back screen. The output matches every one of the screenshot's
286,720 pixels exactly, including each of the four pixels in the 2x-scaled
source pixel. Re-run the C implementation against the retained real capture:

```sh
python3 tests/test_nexus_ymir_title_capture_parity.py \
  /path/to/test_nexus_v1_vdp2_capture_compositor \
  /path/to/ymir_nexus_atomic_menu_20261026/dumps \
  '/path/to/ymir_nexus_atomic_menu_20261026/screenshots/DUNGEON MASTER NEXUS [T-9111G]-20261026T200759.691.png'
```

This result establishes exact static capture composition for the retained
frame. The screenshot was taken after the dump, so frame identity remains
unproven. The API reports `capture_only=1` and `renderer_permitted=0`; it does
not yet feed the live Firestaff viewport, prove the startup sequence, or test
controller and mouse interaction. The evidence dumps remain on the external
disk and are not committed.

### Frame 8047 repeat menu capture

The later `ymir_nexus_menu_source_wide_frame_read_20261027` capture provides a
second full-frame replay, now paired with the Japanese Continue-menu screenshot
in one Ymir run. Its VDP1 trace contains 331,364 rows, including all 178
commands for frame 8047, and reports zero dropped rows. The matching register
receipt also names frame 8047. The C replay matches all 286,720 pixels in the
640×448 screenshot exactly after 2× scaling.

| Capture input | SHA-256 |
| --- | --- |
| `vdp1-fbs.bin` | `2752a34f5d200c965e2fe5c35ff6f973361319d769a2d76944af2c7b0f480979` |
| `vdp2-vram.bin` | `de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c` |
| `vdp2-cram.bin` | `db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de` |
| `vdp2-registers.bin` | `e69d03094ef856922413e073cb9078015e52849786268d6421c7df00a655965e` |
| `vdp1-command-trace.csv` | `d13fcede5adcad648b9c167df9c54eb928760185886bf667223f1272bd505e98` |
| Frame-8047 screenshot | `741c620e945b5ab3e05f6bc5d182fc4ada9439071b79718f67326bcdd56c6118` |

The replay reports 70,239 direct VDP1 framebuffer pixels, 1,441 type-6
palette pixels, and 18,513 NBG0 overlays. These counts differ from the older
frame-59 capture while covering the same full 320×224 output. The separate
source verifier binds all 1,441 visible Japanese menu-sprite texels to
`TITLE.BIN`. The screenshot has no embedded frame number; the run log and
register receipt bind it to the same capture run, while exact acquisition-frame
identity remains unproven.

Reproduce this paired capture against the authentic Japanese data and retained
external evidence:

```sh
python3 tests/test_nexus_ymir_title_capture_parity.py \
  /path/to/test_nexus_v1_vdp2_capture_compositor \
  /Volumes/Extern-disk/firestaff-evidence/ymir_nexus_menu_source_wide_frame_read_20261027/dumps \
  '/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_menu_source_wide_frame_read_20261027/screenshots/DUNGEON MASTER NEXUS [T-9111G]-20261027T194440.170.png' \
  --run-log /Volumes/Extern-disk/firestaff-evidence/ymir_nexus_menu_source_wide_frame_read_20261027/run.log
```

Both profiles are capture-only and keep `renderer_permitted=0`. The exact
composition replay uses the emulator's final display framebuffer, VDP2 memory,
CRAM, and registers as inputs; it does not reconstruct the menu from retail
assets or admit them to Firestaff's live startup renderer. The Japanese source
read trace still omits byte values at access time, and the NBG0 source join is
partial. Runtime input, mouse hit areas, startup presentation, save loading,
and playable-level/HUD parity remain open.
