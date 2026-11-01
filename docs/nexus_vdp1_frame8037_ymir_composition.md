# Nexus frame 8037 capture composition

The authentic Japanese retail Ymir capture at VDP1 frame 8037 is replayed by
Firestaff's bounded, capture-only VDP compositor. The test joins the Ymir VDP1
display framebuffer to VDP2 VRAM, CRAM, and registers from the same pinned
capture bundle. It verifies the complete VDP1 command-trace receipt, including
178 rows for frame 8037 and zero dropped rows.

The compositor output matches every RGB pixel in both Ymir VDP2 pass buffers
(serials 9118 and 9119): 71,680 of 71,680 pixels, with zero channel
differences. The two pass buffers have identical bytes. Independently, the
retained 640×448 Ymir frontend screenshot is an exact 2× nearest-neighbor
match to those pass buffers.

The observed VDP2 state includes `TVMD=0x8000`, `BGON=0x0001`,
`CHCTLA=0x0013`, `SPCTL=0x1536`, and `CCCTL=0x0043`. The captured palette
pixels use priority 5 and ratio index 0, and the 8bpp NBG0 bitmap overlays the
same frame. The compositor resolves these samples from the captured FBS,
CRAM, bitmap, and register bytes. This does not join each displayed pixel to a
retail command or source asset.

The complete 8037 VDP1 command trace has 137 textured draw commands. Against
the hash-verified Japanese retail data, 126 captured texture spans exactly
match retail source bytes; 121 of those also match decoded DGN Structure2
image records. The complete sequence join finds 132 type-2 draws: all 121
informative spans bind to DGN source images, with 113 indexed palette joins
and eight direct-colour joins. Eleven more spans are all-zero and remain
unowned. A canonical Structure3 face owner exists for 105 draws, but this does
not identify which face was selected or prove transform and culling. Some DGN
images are byte-identical across levels, so those matches are source candidates
rather than unique level ownership. The independent
`nexus_v1_vdp2_ymir_frame8037_real` check still proves the complete composed
output against both Ymir VDP2 pass buffers, while source and face joins keep
semantic admission blocked.

The same hash-pinned frame-8037 screenshot also verifies five Japanese
title-menu sprites at VDP1 command indices 171–175. Their texels bind to
`TITLE.BIN` offsets `0x15f6`, `0x446`, `0x576`, and `0x7d6`; their CLUT words
bind to Japanese `TITLE.BIN` palette spans. All 1,495 visible sprite sample
points match the screenshot exactly, with NBG0 transparent at those points and
the captured VDP2 back-screen color calculation applied. This verifies the
selected “New Game” menu state’s sprite contribution, not menu input handling
or transition behavior. The screenshot time association is not independently
attested by a retained run log.

## Reproduce with authentic external data

Extract the hash-pinned raw dumps from
`firestaff-evidence/nexus/nexus_vdp2_frame_join_20261030/capture-artifacts.tar.gz`
to a local external directory. Keep the game data and capture dumps outside
Git. The pass references are in
`firestaff-evidence/nexus/ymir_nbg0_frame_serial_diagnostic_20261030/rendered_frame_capture/frame8037-authentic-target/vdp2-frames/`.

```sh
cmake --build build --target test_nexus_v1_vdp2_ymir_frame8037_real
FIRESTAFF_NEXUS_YMIR_FRAME8037_CAPTURE_DIR=/path/to/external/dumps \
FIRESTAFF_NEXUS_YMIR_FRAME8037_REFERENCE_DIR=/path/to/external/vdp2-frames \
ctest --test-dir build -R '^nexus_v1_vdp2_ymir_frame8037_real$' --output-on-failure
```

The retail source comparison uses the same capture bundle and the authentic
Nexus data directory:

```sh
FIRESTAFF_NEXUS_YMIR_FRAME8037_CAPTURE_DIR=/path/to/external/dumps \
FIRESTAFF_NEXUS_DATA_DIR=/path/to/authentic/Japanese/Nexus/data \
FIRESTAFF_NEXUS_YMIR_FRAME8037_SCREENSHOT=/path/to/external/title-menu-frame-8037.png \
ctest --test-dir build -R '^nexus_v1_vdp1_frame8037_source_join_real$' --output-on-failure
```

The source test pins `TITLE.BIN` to SHA-256
`51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3` and
`LEV00.DGN` to SHA-256
`24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a` before it
reads any command-source bytes.

The test pins SHA-256 for the FBS, VDP2 VRAM, CRAM, registers, command trace,
capture receipts, and both Ymir pass files. It skips when the external capture
paths are not provided and fails on missing or changed files.

This is emulator-internal same-run composition evidence. It does not establish
original-Saturn parity, complete retail source ownership, interactive menu
behavior, later startup transitions, or dungeon/HUD rendering. The result is
capture-only and `renderer_permitted=false`; the production renderer gate
remains closed.

## PR #239 rerun

The authentic replay was rebuilt and rerun from the Debug configuration at PR
code commit `c1d2664` on 2026-10-31. The test passed against the retained
external dump and both hash-pinned Ymir pass buffers, matching all 71,680 RGB
pixels in each pass with zero channel differences. The capture and pass files
remain external; the result is still limited to this captured display state.

## PR #252 frame-8037 rerun

On 2026-11-01, both authentic frame-8037 regressions passed against code head
`9aa4e140d2fabdb6c63e5e0f3068093d5b51b1c5`. The VDP2 compositor matched both
hash-pinned Ymir pass buffers exactly: 71,680 of 71,680 RGB pixels, with
70,185 direct-color and 1,495 palette pixels; NBG0 overlaid 18,513 pixels.
The VDP1 source-join test passed three consecutive runs using the Japanese
retail data whose `TITLE.BIN` and `LEV00.DGN` hashes are pinned above. It
matched 126 of 137 informative retail texture spans, joined all 121
informative draws to source data, kept 11 zero spans transparent and
unowned, and matched the five title-menu sprites and their CRAM palettes at
all 1,495 screenshot sample points. The VDP2 replay also passed three
consecutive runs against the same capture and both Ymir passes.

Capture dumps, rendered references, screenshot, and retail game data remain
outside Git. This verifies the captured menu display state only; selected
Structure3 face, transform/culling, interactive menu behavior, and production
renderer admission remain unresolved.
