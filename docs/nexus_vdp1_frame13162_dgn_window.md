# Nexus frame-13162 VDP1 DGN material window

This real-data verification checks the contiguous executed VDP1 command window
10–40 from the authenticated Ymir cold-boot capture. It binds each trace row to
the reconstructed little-endian VDP1 command bytes, confirms the Ymir execution
receipt includes every row, and resolves each 4bpp or direct-colour texture to
the actual retail `LEV00.DGN` Structure2 image and, for indexed textures, its
palette. The current window has 25 mode-1 joins and 6 mode-5 joins.

The execution receipt also includes rows 6 and 7, and the test checks those
commands against the reconstructed command bytes. Row 6 sets the inclusive
user-clip rectangle to `(0,0)..(320,224)`; row 7 sets local coordinates to
`(160,112)`. Rows 10–40 are type-2 draws with user clipping enabled in
inside-rectangle mode, and their trace records carry the same local origin.
There is no intervening user-clip or local-coordinate command in the checked
window. Consequently, user clipping rejects no pixel in the captured
`320x224` viewport for these rows. This proves the captured clip/local state;
it does not prove complete VDP1 clipping, polygon coverage, or pixel output.
All 31 draws also enable Gouraud shading. The verifier follows each command's
captured Gouraud pointer (`word_address * 8` bytes), bounds-checks the table in
the reconstructed VDP1 VRAM, and confirms that its four vertex colors are
identical for that command. This makes constant shading a useful diagnostic
for this particular window; it does not validate Gouraud interpolation for
general quads.

All six authentic mode-5 draws (rows 13, 14, 28, 29, 33, and 34) are replayed
through Firestaff's direct-colour capture decoder after their exact
`LEV00.DGN` source joins are verified. Each 1,024-word source consists entirely
of `0x000f`; the decoder reports valid commands with zero opaque writes,
matching Ymir's bit-15 transparency rule. The compositor unit test separately
checks bit-15 transparency and the independent ECD and TPD controls.

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
FBS+NBG0+row-171 composition reference. It does not rasterize rows 10–40 or
compare their output pixels to the screenshot. A rasterization experiment
failed that comparison; therefore these DGN draws are not admitted to a parity
renderer. The six mode-5 empty masks verify the captured direct-colour
transparency behavior, not full geometry or scene parity. Gouraud table data
is attested, but the compositor does not yet apply Ymir's per-pixel Gouraud
interpolation. Exact polygon coverage, command-to-face ownership, and overlap
ownership are also unresolved. The harness keeps `renderer_permitted=false`.

When the local capture files or `LEV00.DGN` are unavailable, CTest reports the
case as skipped (`77`). No captured VRAM or commercial game data is stored in
the repository.
