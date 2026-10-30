# Nexus frame-13162 VDP1 DGN material window

This real-data verification checks the contiguous executed VDP1 command window
8–40 from the authenticated Ymir cold-boot capture. It binds each trace row to
the reconstructed little-endian VDP1 command bytes and confirms the Ymir
execution receipt includes every row. It resolves 25 indexed textures and six
of the eight direct-colour textures to exact retail `LEV00.DGN` Structure2
sources. Rows 8 and 9 have captured bytes but ambiguous retail-fill ownership;
the test does not claim a source join for those rows.

The execution receipt also includes rows 6 and 7, and the test checks those
commands against the reconstructed command bytes. Row 6 sets the inclusive
user-clip rectangle to `(0,0)..(320,224)`; row 7 sets local coordinates to
`(160,112)`. Rows 10–40 are type-2 draws with user clipping enabled in
inside-rectangle mode, and their trace records carry the same local origin.
There is no intervening user-clip or local-coordinate command in the checked
window. Consequently, user clipping rejects no pixel in the captured
`320x224` viewport for these rows. This proves the captured clip/local state;
it does not prove complete VDP1 clipping, polygon coverage, or pixel output.
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
parity even when the four Gouraud values happen to be uniform. This gives the
first bounded implementation target: apply the exact channel lookup to
manifest-verified uniform-table draws in an RGBA capture lane, then verify
coverage and pixel output against the retained emulator image before expanding
to varying tables or enabling renderer admission. The source references are
Ymir `vdp_renderer_sw.cpp::VDP1PlotTexturedLine` and
`VDP1PlotTexturedQuad`, plus `vdp1_steppers.hpp::GouraudChannelStepper`; and
Mednafen `vdp1_common.h::GourauderTheTerrible` and
`vdp1_poly.cpp::CMD_PolygonG_T`.

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

The bounded Ymir title capture compositor can now add one source-bound VDP1
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
retail data:

```sh
export FIRESTAFF_NEXUS_YMIR_FBS="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/dumps/vdp1-fbs.bin"
export FIRESTAFF_NEXUS_YMIR_VDP2_VRAM="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/dumps/vdp2-vram.bin"
export FIRESTAFF_NEXUS_YMIR_CRAM="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/dumps/vdp2-cram.bin"
export FIRESTAFF_NEXUS_YMIR_REGS="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/dumps/vdp2-registers.bin"
export FIRESTAFF_NEXUS_YMIR_VDP1_VRAM="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/dumps/vdp1-vram.bin"
export FIRESTAFF_NEXUS_YMIR_VDP1_RECONSTRUCTED_VRAM="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/vdp1-vram-command-trace-reconstructed-le.bin"
export FIRESTAFF_NEXUS_YMIR_COMMAND171_TITLE="$HOME/.firestaff/data/nexus/TITLE.BIN"
export FIRESTAFF_NEXUS_YMIR_COMMAND171_EXPECTED_RGB="$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/vdp12-frame13162-command171.rgb"
export FIRESTAFF_NEXUS_YMIR_CAPTURE_ATTESTED=1
export FIRESTAFF_NEXUS_YMIR_TRACE_ATTESTED=1
export FIRESTAFF_NEXUS_YMIR_REPLAY_OUT=/tmp/nexus-frame13162-command171.rgb
cmake --build build --target test_nexus_v1_vdp2_capture_compositor
build/test_nexus_v1_vdp2_capture_compositor
python3 scripts/verify_nexus_capture_rgb_screenshot.py \
  "$FIRESTAFF_NEXUS_YMIR_REPLAY_OUT" \
  "$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030/screenshots/DUNGEON MASTER NEXUS [T-9111G]-20261030T131353.620.png" \
  --manifest-root "$PWD/parity-evidence/verification/nexus_v1_ymir_coldboot_title_span_visible_capture_20261030"
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

The focused check verifies source joins, executed-command membership, the
captured user-clip/local-coordinate state, and the hash of the retained
FBS+NBG0+row-171 composition reference. It does not rasterize rows 8–40 or
compare their output pixels to the screenshot. A rasterization experiment
failed that comparison; therefore these DGN draws are not admitted to a parity
renderer. The eight mode-5 empty masks verify the captured direct-colour
transparency behavior, not full geometry or scene parity. Gouraud table data
is attested, but the compositor does not yet apply Ymir's per-pixel Gouraud
interpolation. Exact polygon coverage, command-to-face ownership, and overlap
ownership are also unresolved. The harness keeps `renderer_permitted=false`.

When the local capture files or `LEV00.DGN` are unavailable, CTest reports the
case as skipped (`77`). No captured VRAM or commercial game data is stored in
the repository.
