# Nexus VDP1 frame 300 replay

This capture-only verifier replays the authenticated VDP1 commands for Ymir
frame 300 and compares the result with the same run's VDP1 framebuffer. It
uses Ymir's integer quad, line, texture, clipping, and Gouraud stepping rules.
On code head `a05544775f8cf73b9ff4278b028ed0116cec57e2`, the viewport matched
all 71,680 RGB pixels and all 71,680 framebuffer MSB bits in three consecutive
runs against the authentic capture and hash-verified Japanese retail data.
The combined `nexus_v1_gameplay_frame300_composition_source_join_real` check
also passed on that code head, verifying the VDP2 composition and per-pixel
VDP1 source ownership against the same capture.

## Source evidence

The capture is stored outside Git at
`firestaff-evidence/nexus/nexus_face_vdp2_hud_join_20261030/capture-301/dumps`.
The verifier pins the hashes of the framebuffer, VDP1 VRAM, VDP1 and VDP2
registers, complete command trace, and Ymir capture receipt. The receipt
attests all 83 command rows for frame 300 and zero dropped trace rows. The
replay includes the initial full-screen polygon, all visible textured draws,
the trailing polyline and normal sprite, and accounts for five draw-skip
commands plus the end marker. Japanese
retail `LEV01.DGN` is separately pinned to SHA-256
`b359a89ffa344439b6d0d15de223f3211caa7725b13dfdb4461b579c48f06723`.

The replay admits 51 indexed texture spans through unique retail DGN image
and palette joins and 15 direct-RGB texture spans through retail DGN image
joins. Two mode-0 commands are verified clipped no-ops. Row 79 is a polyline;
row 80 is a 16-by-41, 8-bit, 256-colour bank-mode normal sprite. Its 656-byte
texture is taken from the hash-pinned VDP1 VRAM snapshot and exactly matches
decoded Japanese retail `MENU.BPK` entry 144. Every used nontransparent PALT
word matches same-frame VDP2 CRAM. The capture-only replay still consumes the
captured texture bytes, while the real-data test separately verifies this
retail source and palette join. The polyline and sprite are both included in
the exact framebuffer comparison.

## 16-bpp source-address alignment

Ymir decodes CMDSRCA as an eight-byte address, then aligns the fetched
character address down to a 16-byte boundary for 16-bpp RGB mode in
`VDP1PlotTexturedLine` (`libs/ymir-core/src/ymir/hw/vdp/renderer/vdp_renderer_sw.cpp`,
line 1263). Firestaff's command parser now reports that effective VDP1-VRAM
offset for mode 5, so source-range validation, material joins, and capture
replay use the same starting bytes as Ymir. Other color modes retain the
encoded eight-byte offset.

The authenticated frame-300 mode-5 draws all use CMDSRCA `0xbdac`, which is
already 16-byte aligned. They validate that the change preserves this capture;
they do not exercise an odd CMDSRCA. The parser's boundary regression covers
align-down semantics independently and is not presented as game data or as a
substitute for an authentic odd-address capture.

## Reproduce

Build the target and point it at the retained capture and authentic game data:

```sh
cmake --build build --target test_nexus_v1_vdp1_frame300_dgn_stepper_real
FIRESTAFF_NEXUS_YMIR_GAMEPLAY_FRAME300_CAPTURE_DIR=/path/to/capture-301/dumps \
FIRESTAFF_NEXUS_DATA_DIR=/path/to/authentic/Japanese/Nexus/data \
ctest --test-dir build -R '^nexus_v1_vdp1_frame300_dgn_stepper_real$' --output-on-failure
```

The verifier skips when external capture or game data is unavailable and
fails if a pinned source file changes. A skipped run does not execute the
parity check; CTest may report `Not Run` while returning success. Treat this
replay as passed only when CTest reports `Passed`. The three current-code
runs and combined composition run above reported `Passed` with the retained
authentic capture and retail game data.

The implementation follows Ymir's VDP1 software renderer for the 8-bit,
256-colour bank mode and MSB behavior:
[`vdp_renderer_sw.cpp`](https://github.com/ymir-emu/Ymir/blob/main/libs/ymir-core/src/ymir/hw/vdp/renderer/vdp_renderer_sw.cpp).

## Scope

This proves a VDP1 framebuffer replay for one authenticated Ymir capture. It
does not prove VDP2 composition, pixel-to-face ownership for every draw,
original Saturn parity, complete Nexus support, or gameplay admission. The
stepper remains diagnostic-only and always reports
`renderer_permitted=false`.
