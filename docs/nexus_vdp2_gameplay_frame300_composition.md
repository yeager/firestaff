# Nexus gameplay frame 300 VDP composition

Firestaff's capture-only VDP2 priority compositor reproduces the RGB output of
an authentic Ymir gameplay capture for all 71,680 pixels at 320x224. The
capture is from the Japanese retail disc and an accepted four-champion save;
its provenance and raw data are retained outside the repository at
`firestaff-evidence/nexus/nexus_face_vdp2_hud_join_20261030/capture-301`.

The capture bundle records VDP1 frame 300, 83 VDP1 commands, and zero dropped
commands. VDP2's software-rendered reference is named
`vdp1-300-vdp2-301-320x224.rgbx8888le`; its 71,680 RGB pixels match the
screenshot from that same Ymir run. The reference file SHA-256 is
`0428f9e456ea7b211657a5a6b012f08f72ded4c73a75327edc4dc90d3e607273`.

For this capture, the priority stack assigns 65,500 pixels to RGB VDP1,
491 to CRAM VDP1, 2,110 to NBG0, 74 to NBG1, and 3,505 to NBG2. It reports no
empty pixels, unsupported color-calculation cases, or unsupported shadow
cases. The test pins the input VDP1 framebuffer, VDP2 VRAM, CRAM, and register
snapshot by SHA-256, then compares every composed RGB triplet against Ymir's
same-run rendered frame. It also writes a per-pixel layer ownership map for
inspection.

Run the real-capture regression after building the existing target:

```sh
capture=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_face_vdp2_hud_join_20261030/capture-301
export FIRESTAFF_NEXUS_YMIR_FRAME300_VDP1_FBS="$capture/dumps/vdp1-fbs.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_VRAM="$capture/dumps/vdp2-vram.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_CRAM="$capture/dumps/vdp2-cram.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_REGS="$capture/dumps/vdp2-registers.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME300_RENDERED="$capture/vdp2-rendered/vdp1-300-vdp2-301-320x224.rgbx8888le"
export FIRESTAFF_NEXUS_YMIR_FRAME300_STACK_OUT=/Volumes/Extern-disk/nexus-frame300-vdp2-layer-map.bin
build/test_nexus_v1_vdp2_runtime_tilemap
```

This confirms the implemented layer decoders, VDP1 color interpretation,
priority ordering, and pixel composition against Ymir for this captured state.
The screenshot and rendered reference share one emulator run, so this is an
emulator-to-emulator implementation check rather than independent Saturn
hardware validation. It does not prove the semantic owner of every HUD asset,
FACE portrait selection, or behavior in other game states.

## VDP1 source join

The same 83-record Ymir trace has a complete command chain and zero dropped
rows. Firestaff's DGN material join matches 66 of 68 textured draws to
hash-verified Japanese retail data: 51 indexed palettes and 15 direct-color
spans match. Sixty-one draws have candidate Structure3 face owners, but that
does not identify the face selected at runtime. The two unmatched commands at
VDP1 offsets `0x0cbe0` and `0x0cc20` are zero-size bank-mode draws; they remain
unbound. Sequence-level DGN admission, transform/culling, VDP2 composition
admission, and production rendering remain blocked.

Run the real-data source-join regression with the external capture and retail
corpus:

```sh
capture=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_face_vdp2_hud_join_20261030/capture-301
export FIRESTAFF_NEXUS_YMIR_GAMEPLAY_FRAME300_CAPTURE_DIR="$capture/dumps"
export FIRESTAFF_NEXUS_DATA_DIR=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_vdp1_frame8037_retail_japan
ctest --test-dir build -R '^nexus_v1_vdp1_gameplay_frame300_source_join_real$' --output-on-failure
```
