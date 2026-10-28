# Japanese Configuration scene DGN trace prefix

This receipt joins the authentic Japanese retail Nexus Configuration-page
capture to its Ymir-executed VDP1 command prefix and hash-verified retail DGN
materials. It is source evidence only; it does not admit a 3D renderer.

## Capture

The external capture is
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_jp_config_selection_preaccept_20261028/configuration/`.
Its screenshot shows the Configuration page over the active dungeon scene.
The VDP1 receipt identifies frame 599, 168 observed commands, a complete trace
file, and zero dropped trace rows. The authentic Japanese `TITLE.BIN` SHA-256
is `51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3`.

## DGN material join

The trace prefix contains 132 textured draws. Eleven are all-zero indexed
textures with transparent pixels enabled, so they have no visible texels and
cannot identify a retail material. The remaining 121 informative draws match
exact Structure2 image and corresponding VDP1 CLUT or direct-colour bytes in
the 16 authentic DGN level files: 113 indexed image/CLUT pairs and 8
direct-colour images. At least one possible Structure3 face owner exists for
105 matches; 16 matched draws have no candidate face-owner row. There are no
unmatched informative draw spans in this prefix.

The eleven zero-filled command addresses are `0x01360`, `0x013a0`, `0x013c0`,
`0x00be0`, `0x00c00`, `0x01280`, `0x00c60`, `0x01480`, `0x014a0`,
`0x014c0`, and `0x00cc0`. Many matched materials are shared by multiple
levels or faces. The receipt does not identify the active level instance,
selected face for each draw, camera transform, culling, display origin, or VDP2
composition. Therefore
`semantic_admission=blocked`; Firestaff must not use these joins as permission
to draw a reconstructed scene.

## Reproduction

With the authentic retail files installed in `.firestaff/data/nexus`, run:

```sh
python3 scripts/analyze_nexus_vdp1_dgn_command_sequence_join.py \
  /path/to/configuration/dumps \
  --data-dir .firestaff/data/nexus \
  --frame 599 \
  --ymir-trace-prefix
```

The prefix mode verifies the trace receipt, row count, zero drops, contiguous
command indices, current command address, and VDP1 VRAM size. It preserves the
observed Ymir execution order without requiring an END command, since this
capture ends before the display list is proven complete. The full-chain mode
continues to validate CMDLINK transitions and END.

The complete report and raw captures remain on the external evidence volume;
no game media or emulator dumps are included in Git.
