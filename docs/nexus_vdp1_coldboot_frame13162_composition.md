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
