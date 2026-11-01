# Nexus frame-13162 VDP1 DGN material window

This real-data verification checks the contiguous executed VDP1 command window
8–40 from the authenticated Ymir cold-boot capture. It binds each trace row to
the reconstructed little-endian VDP1 command bytes and confirms the Ymir
execution receipt includes every row. It resolves 25 indexed textures and six
of the eight direct-colour textures to exact retail `LEV00.DGN` Structure2
sources. Rows 8 and 9 have captured bytes but ambiguous retail-fill ownership;
the test does not claim a source join for those rows.

A second capture-only regression scans the complete 174-row frame trace and
resolves 113 executed type-2 mode-1 draws to unique `LEV00.DGN` image and CLUT
sources. It submits those commands in trace order to Firestaff's RGBA sequence
compositor and records 145,171 written pixels. The other 49 of the frame's 162
draw commands remain gaps in that DGN-bound sequence. A separate check binds
11 of those commands to hash-pinned all-zero 4bpp spans in both the raw and
trace-reconstructed VDP1 VRAM; all 9,952 source texels are transparent under
the captured PMOD, and the no-op compositor leaves the framebuffer unchanged.
These 11 checks claim no DGN image or CLUT owner. The sequence proves
command/source/CLUT composition for the joined subset; its isolated output is
not compared to the Ymir screenshot or framebuffer, and it does not close the
other sequence gaps or enable the production renderer. The receipt retains
`capture_only=true` and `renderer_permitted=false`.

Run this regression with the complete private Ymir capture bundle available at
the configured capture path and the user's retail data:

```sh
cmake --build build --target test_nexus_v1_vdp1_frame13162_dgn113_replay
ctest --test-dir build -R '^nexus_v1_vdp1_frame13162_dgn113_replay$' --output-on-failure
```

The capture bundle is not committed. When it is absent from the checkout, the
CTest is skip-safe; a skip is not evidence that the 113-command replay ran.
To run it directly against the external capture bundle, pass its path to the
test executable:

```sh
build/test_nexus_v1_vdp1_frame13162_dgn113_replay \
  "$FIRESTAFF_NEXUS_YMIR_FRAME13162_CAPTURE_DIR" \
  "$FIRESTAFF_WORKSPACE_DATA_DIR/nexus/LEV00.DGN" \
  tests/fixtures/nexus_v1_vdp1_frame13162_trace.csv \
  parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/vdp1-command-execution-frame13162.json \
  /tmp/nexus-frame13162-dgn113.rgba
```

The execution receipt also includes rows 6 and 7, and the test checks those
commands against the reconstructed command bytes. Row 6 sets the inclusive
user-clip rectangle to `(0,0)..(320,224)`; row 7 sets local coordinates to
`(160,112)`. Rows 10–40 are type-2 draws with user clipping enabled in
inside-rectangle mode, and their trace records carry the same local origin.
There is no intervening user-clip or local-coordinate command in the checked
window. Consequently, user clipping rejects no pixel in the captured
`320x224` viewport for these rows. This proves the captured clip/local state
for this window; the full-frame stepper result below verifies polygon coverage
and final VDP1 pixel output for frame 13162.
All 31 draws also enable non-neutral Gouraud shading. The verifier follows each command's
captured Gouraud pointer (`word_address * 8` bytes), bounds-checks the table in
the reconstructed VDP1 VRAM, and confirms that its four vertex colors are
identical for that command. This makes constant shading a useful diagnostic
for this particular window; it does not validate Gouraud interpolation for
general quads.

The Ymir and Mednafen software renderers agree on the Gouraud color rule:
each 5-bit RGB channel is offset by its 5-bit table value minus 16, clamped
to `0..31`; the source pixel's bit 15 is retained. The renderer applies this
after fetching the texel color. Their polygon paths interpolate the four
vertex corrections along the A-D and B-C edges, then across each scanline
using integer error steppers. Clipping skips advance those steppers too. A
two-triangle barycentric approximation therefore cannot establish Ymir pixel
parity even when the four Gouraud values happen to be uniform. Firestaff's
source-joined RGBA compositor still uses that approximation and remains
capture-only. A separate `nexus_v1_vdp1_ymir_stepper_capture` module now ports
Ymir's integer quad, line, texture, and Gouraud stepping for authenticated
mode-1 distorted sprites. It requires verified command, source, palette,
VRAM, coordinate, clipping, and single-density state; unsupported states are
rejected, and it never grants renderer admission. For indexed mode-1 draws,
the compositor verifies that the captured texture span is the word-normalized
form of the retail DGN image, then reads texels from the original DGN byte
order so nibble selection follows the actual Saturn texture stream.

A real-data CTest now joins the DGN source resolver to the Ymir-stepper capture
path for the complete frame-13162 VDP1 capture. It verifies the hash-pinned
168-row trace, command bytes, clip/local-coordinate state, single-density
register tuple, and capture receipt. All 113 uniquely source-bound mode-1
draws use their hash-verified `LEV00.DGN` texels and CLUTs; 11 additional
mode-1 draws use captured all-zero texels already verified as transparent
no-ops. The stepper rasterizes all 124 mode-1 draws and 28 flat polygons in
trace order. The other eight mode-5 commands are checked by the companion
frame-window test, which verifies their captured source pixels are fully
transparent. Together these cover all 160 draw commands in this frame's trace
window.

The resulting first 512x256 Ymir FBS draw buffer matches the capture-only
replay at native 320x224 coordinates: all 71,680 RGB555 pixels and all 71,680
opacity bits match exactly while the 113 joined draws are sourced from retail
DGN data. This closes the previous 500-pixel residual for this captured VDP1
state and validates the DGN-to-VRAM word normalization through the same-run
framebuffer comparison. Structure3 face selection, transform and culling
ownership remain unproven; 11 transparent no-op spans have no DGN image or CLUT
owner. The stepper keeps `renderer_permitted=false`; this one-state result does
not establish original-Saturn, arbitrary-frame, VDP2-composition, or
production-renderer parity. The capture and retail game data stay outside Git.
The reconstructed RGBA and hash-bound report are retained at
`firestaff-evidence/nexus/nexus_vdp1_frame13162_retail_dgn_stepper_20261101/`
on the external disk.
Set `FIRESTAFF_NEXUS_YMIR_FRAME13162_CAPTURE_DIR` to its profile directory (or
`dumps/`) and make the authentic Japanese data directory available through
`FIRESTAFF_NEXUS_DATA_DIR`, then run:

```sh
ctest --test-dir build -R '^nexus_v1_vdp1_ymir_stepper_capture_real$' --output-on-failure
```

The Ymir stepper is a separate capture-only path; it does not enable the
production renderer. The source references are Ymir
`vdp_renderer_sw.cpp::VDP1PlotTexturedLine` and
`VDP1PlotTexturedQuad`/`VDP1Cmd_DrawPolygon`, plus
`vdp1_steppers.hpp::GouraudChannelStepper`; and Mednafen `vdp1_common.h` and
`vdp1_poly.cpp`.

The companion palette-pressure analysis shows why applying these Gouraud
colors by replacing entries in Firestaff's shared 256-entry indexed palette
cannot preserve the lower layer. In the authenticated visible `320x224` area,
the captured FBS+NBG0 source planes contain 555 distinct RGB555 colors. The
25 mode-1 commands use 19 distinct CLUT+Gouraud variants (304 slots if each
variant reserves all 16 CLUT entries), with 61 distinct shaded colors present
in their nontransparent source texels. This is a capacity diagnostic from the
captured planes, not a rendered-pixel count. It points the composition work
toward an RGB surface instead of palette replacement.

Recompute the report from local, manifest-verified Ymir evidence:

```sh
python3 scripts/analyze_nexus_vdp1_gouraud_palette_pressure.py \
  parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030 \
  --frame 13162 --first 10 --last 40
```

The expected RGB replay is capture media and is kept outside Git. Set
`FIRESTAFF_NEXUS_YMIR_COMMAND171_EXPECTED_RGB` to its local path; the test
checks the file hash against the retained manifest entry. The bounded Ymir
title capture compositor can now add one source-bound VDP1
mode-0 sprite after constructing the captured FBS+NBG0 RGBA base. Its frame
13162 check binds command bytes at VDP1 VRAM `0x1560`, preserves the captured
4bpp texture byte order, and verifies the 1,008-byte span at `TITLE.BIN`
offset `0x1f26` against an accepted retail file hash. The sprite is 168x12 at
`(79,164)` and contributes 963 opaque pixels. The C capture output is checked
against the retained independent RGB reconstruction at every pixel (maximum
channel difference 8); a separate comparison against the authentic 640x448
Ymir screenshot checks all 286,720 doubled samples within 7 channel levels.
The receipt remains capture-only with `renderer_permitted=false`; this is not
a general title/menu or gameplay compositor.

Run the extended C capture check with the local evidence files and authentic
retail data. The reconstructed VDP1 VRAM file and expected RGB capture are
external inputs; the C test verifies both against the committed SHA-256
manifest and skips if either path is absent. Set the VDP1 VRAM path before
running `nexus_v1_vdp1_frame13162_dgn_window` as well as the VDP2 compositor
check:

```sh
export FIRESTAFF_NEXUS_YMIR_VDP1_RECONSTRUCTED_VRAM="/path/to/external/vdp1-vram-command-trace-reconstructed-le.bin"
export FIRESTAFF_NEXUS_YMIR_COMMAND171_EXPECTED_RGB="/path/to/external/vdp12-frame13162-command171.rgb"
export FIRESTAFF_NEXUS_YMIR_FBS="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/dumps/vdp1-fbs.bin"
export FIRESTAFF_NEXUS_YMIR_VDP2_VRAM="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/dumps/vdp2-vram.bin"
export FIRESTAFF_NEXUS_YMIR_CRAM="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/dumps/vdp2-cram.bin"
export FIRESTAFF_NEXUS_YMIR_REGS="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/dumps/vdp2-registers.bin"
export FIRESTAFF_NEXUS_YMIR_VDP1_VRAM="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/dumps/vdp1-vram.bin"
export FIRESTAFF_NEXUS_YMIR_VDP1_RECONSTRUCTED_VRAM="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/vdp1-vram-command-trace-reconstructed-le.bin"
export FIRESTAFF_NEXUS_YMIR_COMMAND171_TITLE="$HOME/.firestaff/data/nexus/TITLE.BIN"
export FIRESTAFF_NEXUS_YMIR_CAPTURE_DIR="/path/to/external/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030"
export FIRESTAFF_NEXUS_YMIR_COMMAND171_EXPECTED_RGB="$FIRESTAFF_NEXUS_YMIR_CAPTURE_DIR/vdp12-frame13162-command171.rgb"
export FIRESTAFF_NEXUS_YMIR_CAPTURE_ATTESTED=1
export FIRESTAFF_NEXUS_YMIR_TRACE_ATTESTED=1
export FIRESTAFF_NEXUS_YMIR_REPLAY_OUT="$FIRESTAFF_NEXUS_YMIR_CAPTURE_DIR/replay-command171.rgb"
cmake --build build --target test_nexus_v1_vdp2_capture_compositor
build/test_nexus_v1_vdp2_capture_compositor
python3 scripts/verify_nexus_capture_rgb_screenshot.py \
  "$FIRESTAFF_NEXUS_YMIR_REPLAY_OUT" \
  "$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/screenshots/DUNGEON MASTER NEXUS [T-9111G]-20261030T131353.620.png" \
  --manifest-root "$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030" \
  --tolerance 7
```

All eight authentic mode-5 draws (rows 8, 9, 13, 14, 28, 29, 33, and 34) are
replayed through Firestaff's direct-colour capture decoder. Rows 13, 14, 28,
29, 33, and 34 have exact `LEV00.DGN` source joins. Rows 8 and 9 use the same
captured all-`0x000f` texture bytes, but their retail-fill source ownership is
ambiguous; those two are decoded from the hash-attested Saturn capture without
claiming a DGN source join. Every draw reports zero opaque writes, matching
Ymir's bit-15 transparency rule. The compositor unit test separately checks
bit-15 transparency and the independent ECD and TPD controls.

Pinned local capture inputs (the VRAM and trace files are generated evidence
and must be kept outside the Git history):

- Command trace SHA-256:
  `a963e0d6e7667ac4a947a546933d0b306ea5caf13a511702a898c30472c12a88`
- Executed-command receipt SHA-256:
  `2417d30d651f385ef384881d3b14524576069c22ac1f2fe53f71f9938e3355b4`
- Reconstructed VDP1 VRAM SHA-256:
  `90b85d0358c7605c8186828d2e42ab46412f504f12c40348197a46db8f770913`
- Retail `LEV00.DGN` SHA-256:
  `24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a`

Place the authenticated Ymir evidence files under
`parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030`
and run the focused check against the user's real Nexus data:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DFIRESTAFF_WORKSPACE_DATA_DIR="$HOME/.firestaff/data"
cmake --build build --target test_nexus_v1_vdp1_frame13162_dgn_window
ctest --test-dir build -R '^nexus_v1_vdp1_frame13162_dgn_window$' --output-on-failure
```

The `nexus_v1_vdp1_frame13162_dgn113_replay` RGBA preview verifies its
source joins and command state, but it does not rasterize rows 8–40 with the
Ymir stepper or compare that preview to the Ymir framebuffer. Do not use that
preview as parity evidence. The separate
`nexus_v1_vdp1_ymir_stepper_capture_real` regression does use Ymir's per-pixel
Gouraud interpolation and matches all RGB555 pixels and opacity bits while
feeding retail DGN bytes for the 113 joined draws. The eight mode-5 empty masks
verify captured direct-colour transparency, not general mode-5 geometry or
scene parity. Structure3 face selection, transforms, culling, and overlap
ownership remain unresolved. Both paths keep `renderer_permitted=false`.

When the local capture files or `LEV00.DGN` are unavailable, CTest reports the
case as skipped (`77`). No captured VRAM or commercial game data is stored in
the repository.
