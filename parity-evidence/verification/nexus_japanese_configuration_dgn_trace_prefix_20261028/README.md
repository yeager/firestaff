# Japanese Configuration scene DGN trace prefix

This receipt joins the authentic Japanese retail Nexus Configuration-page
capture to its Ymir-executed VDP1 command prefix and hash-verified retail DGN
materials. It is source evidence only; it does not admit a 3D renderer.

## Capture

The authentic capture is retained outside Git in operator-managed evidence
storage. Its screenshot shows the Configuration page over the active dungeon
scene.
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

The 16 matched draws without Structure3 face-owner rows are all early
VDP1 distorted-sprite quads. Their five distinct 48x48 indexed materials
(Structure2 IDs 1 through 5) all come from `LEV00.DGN`; their screen vertices
form a receding tiled plane along the lower scene. This is consistent with a
floor or menu backdrop, but it does not prove a Structure1F map selector or
active level instance. The zero-filled spans above are confirmed invisible for
this capture by VDP1 mode 1 and PMOD `0x048c`: Ymir source commit
`534ed10c6f7f13a3ba752476eea191be3ab553b4` treats CLUT index zero as
transparent, and PMOD bit 6 does not disable transparency.

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

The complete report and raw captures remain in operator-managed external
evidence storage; no game media or emulator dumps are included in Git.
