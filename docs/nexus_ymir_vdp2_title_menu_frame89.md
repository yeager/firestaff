# Nexus title menu VDP composition at frame 89

This capture binds a real Japanese retail Nexus disc, an existing Ymir title
prompt save state, raw VDP1 and VDP2 memory, the complete VDP1 command trace,
and Ymir's composed RGBX output to one deterministic Start-to-menu run. It is
the title menu with `NEW GAME` selected.

Ymir was built from the Yeager fork's
`codex/nexus-vdp2-layer-output-capture-20261031` branch at
`f1afeaa4858bad5217272cfb6fb1c286fa83ec68`. The capture used `--debug`, the
software VDP renderer, and `--capture-no-present`. It loaded
`ymir-title-prompt-slot-2.savestate`, sent Start on capture frame 0, and ran
180 frames. The retained VDP1 frame index is 89; its VDP2 frame serials are
179 and 180. Both RGBX pass buffers are byte-identical.

The VDP1 receipt reports 178 commands for frame 89, a complete trace, and no
dropped rows. The VDP2 state is `TVMD=0x8000`, `BGON=0x0001`,
`CHCTLA=0x0013`, `BMPNA=0`, `SPCTL=0x1536`, `CCCTL=0x0043`, and `PRINA=0x0004`.
Ymir's layer trace contains 3,136 complete records across the two VDP2 passes.
NBG0 and the VDP1 sprite plane carry visible content; the other captured
background planes are transparent. NBG0 marks 18,513 pixels for special
color calculation, and the sprite plane has 1,487 such pixels.

The existing bounded title-menu compositor replays the matching FBS, VRAM,
CRAM, and register dump and matches all 71,680 RGB pixels in both Ymir passes.
The replay reports 70,229 direct-color VDP1 pixels, 1,451 palette VDP1
pixels, and 18,513 NBG0 overlay pixels. The compositor remains capture-only:
`renderer_permitted=false`. This result does not prove original-Saturn pixel
parity, assign retail source ownership to every VDP1 command, or admit the
profile to Firestaff's live renderer.

The raw capture, receipts, pass buffers, extracted PNG previews, per-layer
analysis reports, and a SHA-256 manifest are retained under
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_ymir_vdp2_layers_title_menu_frame89_20261031`.
The capture uses authentic media and save-state data; it contains no generated
game data.

To replay it when the evidence directory is available:

```sh
FIRESTAFF_NEXUS_YMIR_TITLEMENU_FRAME89_CAPTURE_DIR=/path/to/evidence/dumps \
FIRESTAFF_NEXUS_YMIR_TITLEMENU_FRAME89_REFERENCE_DIR=/path/to/evidence/nexus-ymir-vdp2-title-menu-20261031/rendered-frames \
ctest --test-dir build -R '^nexus_v1_vdp2_ymir_title_menu_frame89_real$' --output-on-failure
```

The test skips when either external-data path is unset. If paths are supplied,
it checks the pinned capture and pass-buffer hashes before comparing pixels.
