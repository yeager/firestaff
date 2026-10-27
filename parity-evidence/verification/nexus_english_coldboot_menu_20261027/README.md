# Nexus English cold boot to native menu — 2026-10-27

This receipt records an authentic English retail cold boot reaching the
three-choice Nexus title menu in Ymir. Raw game files, emulator dumps, and the
screenshot stay outside Git under
`/Volumes/Extern-disk/firestaff-data/nexus-english-controlled-boot-16001-20261027/`.
This report contains metadata and reproduction commands only.

## Inputs and run scope

- English retail ISO SHA-256: `16786e6165d8cbf7f6394dd9bc7171fbb561c1ba40b77ad7cba3c275fde2804e`
- English retail CUE SHA-256: `b96f01e2f8ce3ab9c8e4a33d5a0c7076cdc1bfd247a85a1454e6c36c8a616f33`
- USA 1.01a IPL ROM SHA-256: `96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f`
- Ymir: installed system executable, version `0.4.2-dev`, debug tracing enabled
- The profile used Japanese SMPC persistence bytes as emulator setup. No
  emulator state was loaded, no game save was present, and no disc or save data
  was modified.
- Deterministic schedule: Right for two frames at frames 700, 720, 740, 760,
  780, and 800; A for two frames at 830; Start for two frames at 7000, 60 frames
  at 9000, 60 at 13000, and 60 at 15000.

## Evidence

The screenshot shows the English retail title screen with `NEW GAME`,
`CONTINUE`, and `CONFIGURATION` visible. Screenshot SHA-256:
`341cb38191bfbb563912d6ef0c6e2989d51eb6136d48fcffee197cafd7264810`.

The same-frame VDP1 capture is frame 13106. Its bundle records 178 command rows,
a complete trace, and zero dropped rows. The command trace SHA-256 is
`13d6979375e38a1e1150b7b884de04a8c3083964d13a712d82898390154e511a`; the VDP1
VRAM dump SHA-256 is
`d6fa4fcfcfc7982fd64fd53aa6ac910c8a8b027e5da70a0d00e917edb6a57eb7`.

## Captured display composition

The frame's VDP2 registers enable only NBG0 (`BGON=0x0001`) and configure it as
a bitmap layer (`CHCTLA=0x0013`). A read-only replay of the captured VDP1
display framebuffer, NBG0 bitmap, VDP2 CRAM, and register-driven color
calculation produced a 320×224 image. Nearest-neighbour 2× scaling matched every
RGB pixel of the retained 640×448 Ymir screenshot. This binds the screenshot to
the same-frame raw snapshots and confirms the emulator composition. It does
not identify the NBG0 bitmap's named retail source or admit this capture-only
compositor as Firestaff's production renderer.

| Capture component | SHA-256 |
| --- | --- |
| `vdp1-fbs.bin` | `1a16318769e2a9f58fe85caef391140939a08665161ed7d11edd52784c62b383` |
| `vdp2-vram.bin` | `16ccb0336e23eb80c169cdd34489ace726114134548a88aceb8ce9183083ed58` |
| `vdp2-cram.bin` | `db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de` |
| `vdp2-registers.bin` | `e69d03094ef856922413e073cb9078015e52849786268d6421c7df00a655965e` |
| Replayed native-size PNG | `e2ac0b3eef315f5d18517db7f67faee0483a49efd85c01f49c9126afe0c32d1b` |

The authenticated English `MENU.BPK` has SHA-256
`f2f78dddfe37a5ff414775ae888f164624e987059934b034ba36299cc769d2ca`. The
snapshot analyzer decodes all 162 PRS3 surfaces and finds 86 whole-surface byte
matches in VDP1 VRAM. None of the 137 textured command source starts is inside
those matched ranges. This does not identify `MENU.BPK` as the menu's visible
source.

The retail source join reports informative nonzero source-byte matches for
126/137 textured draws. The other 11 command source spans are all-zero and are
excluded as uninformative padding. Matches include `LEV00.DGN` and
`TITLE.BIN`; they do not establish palette, placement, ordering, or display
ownership. The aggregate join remains `partial`, and semantic admission remains
`blocked`.

The Ymir log records a read of `0DMSTRT.BIN` and a CD Block soft reset, but no
`DM.BIN` read event. Separately, the `wram-hi.bin` snapshot contains an exact
230,669-byte prefix of the authenticated 555,144-byte English `DM.BIN` at raw
dump offset `0x10000`. This proves prefix residency in WRAM, not that the full
file was loaded or how the logged read path works.

## Reproduction

From the Firestaff repository root, set `NEXUS_DATA` to the local directory
containing the authenticated English Nexus files (normally
`.firestaff/data/nexus`):

```sh
CAPTURE=/Volumes/Extern-disk/firestaff-data/nexus-english-controlled-boot-16001-20261027

python3 scripts/analyze_nexus_menu_bpk_vdp1_snapshot_join.py \
  "$NEXUS_DATA/MENU.BPK" \
  "$CAPTURE/dumps/vdp1-vram.bin" \
  --vdp1-command-trace "$CAPTURE/dumps/vdp1-command-trace.csv" \
  --vdp1-command-trace-frame 13106

python3 scripts/analyze_nexus_vdp1_source_join.py \
  "$CAPTURE/dumps" --data-dir "$NEXUS_DATA" --frame 13106 --ymir-bundle
```

The capture and complete analyzer outputs remain in the external directory as
`menu-bpk-join.txt` and `vdp1-source-join-recheck.txt`.
