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
inspection. This check passed again against PR head `c1d2664`; the stack hash is
`935af55c5d7429c4527fc379bb2fc6275e9a6d0447fbc8a0e5eb17a3ba6acc6a`. Outputs
are retained under
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/pr239-c1d2664-validation/`.

On 2026-11-01, the combined `nexus_v1_gameplay_frame300_composition_source_join_real`
regression passed against code head
`a05544775f8cf73b9ff4278b028ed0116cec57e2`, using this authentic capture and the hash-verified Japanese retail Nexus data.
It compared all 71,680 composed RGB triplets with the pinned Ymir render, then
verified the frame-300 VDP1 replay,
all visible command owners, and retail DGN source joins. The standalone
`nexus_v1_vdp1_frame300_dgn_stepper_real` also passed three consecutive runs
on the same code head. The diagnostic VDP1 path remains capture-only and does
not admit the production renderer.

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
spans match. The analyzer's per-command palette receipts are checked against
those totals by the real-data regression. Sixty-one draws have candidate
Structure3 face owners, but the selected face is still unresolved. The two
unmatched commands at
VDP1 offsets `0x0cbe0` and `0x0cc20` are zero-size bank-mode draws. Their four
vertices resolve to `(159,-16)` after the captured local origin is applied,
and both enable inside-mode user clipping against `(0,0)..(320,224)`. The trace
therefore proves that these unbound commands contribute no visible pixels in
this frame. Sequence-level DGN admission, the active face, transform/culling,
and production rendering remain unresolved.

Run the real-data source-join regression with the external capture and retail
corpus:

```sh
capture=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_face_vdp2_hud_join_20261030/capture-301
export FIRESTAFF_NEXUS_YMIR_GAMEPLAY_FRAME300_CAPTURE_DIR="$capture/dumps"
export FIRESTAFF_NEXUS_DATA_DIR=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_vdp1_frame8037_retail_japan
ctest --test-dir build -R '^nexus_v1_vdp1_gameplay_frame300_source_join_real$' --output-on-failure
```

## Combined frame-300 check

`nexus_v1_gameplay_frame300_composition_source_join_real` runs both bounded
checks against this same frame-300 evidence package: the C priority compositor
must match Ymir's complete rendered frame, and the VDP1 command trace must
match the hash-pinned Japanese `LEV01.DGN` materials. The combined check also
records the last VDP1 command that wrote each replay pixel and joins that row
to its source receipt. Before assigning owners, it verifies that every
selected visible VDP1 pixel is identical in the replayed FBS buffer and the
display FBS buffer. All 65,991 VDP1 pixels selected as the top VDP2 layer
map to rows backed by retail DGN or captured Ymir VRAM. The 491 CRAM pixels
split exactly into 60 pixels from captured polyline row 79 and 431 pixels from
captured mode-4 sprite row 80. The remaining 65,500 selected VDP1 pixels map
to retail indexed DGN rows. Fifteen direct-RGB DGN draws join retail data,
but none owns a final visible pixel in this captured frame;
the regression pins this per-pixel class distribution explicitly. Set the
evidence root and retail data directory before running it:

```sh
export FIRESTAFF_NEXUS_FRAME300_EVIDENCE_DIR=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_face_vdp2_hud_join_20261030
export FIRESTAFF_NEXUS_DATA_DIR=/path/to/authentic/Japanese/Nexus/data
ctest --test-dir build -R '^nexus_v1_gameplay_frame300_composition_source_join_real$' --output-on-failure
```

The pixel ownership link identifies the last command and source class for
visible VDP1 pixels; it does not establish a selected Structure3 face for each
pixel. Only 66 of 68 textured commands have retail DGN material matches, and
the 431 mode-4 pixels and 60 polyline pixels are capture-backed rather than
DGN-backed. Active face selection and transform/culling remain unverified, so
renderer admission remains blocked.

### Why projected-quad fitting does not identify a DGN face

The retained retail `LEV01.DGN` data provides Structure3 local 16.16 vertex
positions, face vertex indexes, material selectors, and per-face normal rows.
It does not provide per-corner UV coordinates in the parsed face records. The
VDP1 command instead records a source rectangle and four already-projected
screen-space corners. A candidate face's local vertices therefore cannot be
compared directly with the command's screen-space quad. Fitting a projection
to one quad would be underconstrained and would not prove that the game selected
that face or runtime instance.

Some material selectors have only one candidate face in this level's inventory,
but selector uniqueness alone does not establish which instance was drawn. The
next required evidence is a same-invocation retail trace binding a VDP1 command
VRAM offset to a Structure3 entry and face ordinal, together with the selected
runtime instance, projected vertex tuple, coordinate origin, clip state, and
texture-corner correspondence. Until that binding is captured, the source join
establishes only texture-material matches and a bounded candidate-owner set; it
does not establish active face ownership or enable production rendering.
