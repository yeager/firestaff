# Nexus Japanese cold-boot VDP composition at frame 13162

The authentic Japanese Ymir cold-boot capture provides a frame-specific
composition check for the title prompt. The capture-only analyzer combines
captured VDP1 framebuffer pixels, the captured NBG0 8bpp bitmap, and executed
VDP1 command 171. It checks the screenshot without promoting this lane to the
Firestaff production renderer.

## Source and command evidence

Command 171's command words are present at byte address `0x01560` in the
captured VDP1 VRAM. It is an executed 168×12 4bpp normal sprite at `(79,164)`;
its texture span `0x415a0..0x41990` is an exact 1,008-byte match to Japanese
retail `TITLE.BIN` DGT2/PP record 21 at file offset `0x1f26`. The captured
VDP2 CRAM palette at entry `0x360` matches that source record's palette.
These source joins are independently recorded in the frame's retail texture
join report.

The visible NBG0 bitmap includes a separately verified 921-byte span from
Japanese `TITLE.BIN` TITL record 2 at VRAM byte offset `0x0d404`. This binds
that visible span only; the rest of the bitmap remains unowned.

The reconstruction matches all 286,720 doubled screenshot samples within
eight RGB levels per channel; maximum error is 8. The report identifies 963
opaque command-171 pixels at native resolution. Commands with authentic trace
rows marked skipped are rejected even if their command words are present in
the captured VRAM. The selected commands must also pass the hash-checked
frame execution receipt.

## Partial mode-1 command replay

The capture-only VDP1 RGBA sequence also replays the source-bound mode-1
distorted sprites from authentic trace rows 8–40. It resolves each command's
own joined palette before drawing the next command, preserving the different
CLUTs in the RGBA output. The real-data test verifies 25 exact DGN image and
palette joins, 9,088 written pixels, and 8 command gaps in this trace window.
The eight mode-5 draws are separately verified as transparent capture data;
they are not rendered by this sequence.

All 25 replayed commands enable Gouraud shading. Their hash-verified captured
VRAM tables contain four identical corner values per command, so the capture
decoder applies the constant RGB555 correction used by Ymir:
`clamp(base_channel + shade_channel - 16, 0, 31)`. Non-uniform Gouraud tables
are rejected. The decoder still rasterizes textured quads with a different
algorithm than Ymir and has not passed an image comparison against Ymir, so
this does not establish frame parity. Its `renderer_permitted` value remains
false. The tested 174-row authentic trace slice is retained in
`tests/fixtures/nexus_v1_vdp1_frame13162_trace.csv`; the full raw Ymir trace
remains in the external capture archive. The reference behavior is in Ymir's
[`VDP1PlotTexturedQuad`](https://github.com/ymir-emu/Ymir/blob/main/libs/ymir-core/src/ymir/hw/vdp/renderer/vdp_renderer_sw.cpp)
and [`GouraudChannelStepper`](https://github.com/ymir-emu/Ymir/blob/main/libs/ymir-core/include/ymir/hw/vdp/renderer/common/vdp1_steppers.hpp).

## Reproduction

Build the real-data evidence folder following its `README.md`, then run:

```sh
python3 scripts/analyze_nexus_vdp12_capture_composition.py \
  "$CAPTURE" "$CAPTURE/screenshots/DUNGEON MASTER NEXUS [T-9111G]-20261030T131353.620.png" \
  --capture-manifest "$CAPTURE/SHA256SUMS.txt" \
  --frame-index 13162 --commands 171 \
  --output-png /tmp/nexus-frame13162.png \
  --output-rgb /tmp/nexus-frame13162.rgb \
  --output-json /tmp/nexus-frame13162.json
```

The analyzer normalizes manifest paths with or without a leading `./`, checks
all capture inputs against the manifest, verifies command bytes against VDP1
VRAM, and checks each selected command against the captured executed-command
receipt. Output files are comparison artifacts and are not game inputs.

This result covers only NBG0, the captured VDP1 framebuffer, and the selected
command for this title-prompt frame. Other VDP1 draws, the full NBG0 source,
other frames, startup timing, and the title-to-menu transition remain open.
The output does not permit production rendering.
