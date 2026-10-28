# Japanese Nexus title-menu sprite join — 2026-10-28

This receipt records the Japanese New Game and Continue foreground sprite
joins against authentic retail source bytes, captured CRAM, and retained Ymir
screenshots. Game media and capture dumps remain on the external evidence
volume. No synthetic artwork or game data was used.

## Authenticated inputs

- Japanese retail CUE SHA-256:
  `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`
- Japanese `TITLE.BIN` SHA-256:
  `51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3`
- Japanese `TITLE.BIN` FNV-1a64:
  `064414f722d0409c`
- New Game screenshot SHA-256:
  `8721cce3354bc20b1cd367fd44be3d4ecc60ca65fa89c7d00654e83f5ad7db23`
- Continue screenshot SHA-256:
  `741c620e945b5ab3e05f6bc5d182fc4ada9439071b79718f67326bcdd56c6118`

The source was read from the authentic Japanese CUE/ISO in memory. A scan of
all 137 ISO members found byte-exact CRAM palette matches in Japanese
`TITLE.BIN`.

## Source and pixel results

| State | Frame receipt | Ordered `TITLE.BIN` sprite offsets | Icon palette | Text palette candidates | Screenshot texels |
| --- | ---: | --- | --- | --- | ---: |
| New Game | 8050 | `0x15f6`, `0x15f6`, `0x0446`, `0x0576`, `0x07d6` | `0x15d6` | `0x02f6`, `0x0426`, `0x0556`, `0x0686`, `0x07b6`, `0x0986`, `0x1f06` | 1,495 / 1,495 |
| Continue | 8047 | `0x0316`, `0x1b36`, `0x1b36`, `0x06a6`, `0x07d6` | `0x1b16` | `0x02f6`, `0x0426`, `0x0556`, `0x0686`, `0x07b6`, `0x0986`, `0x1f06` | 1,441 / 1,441 |

The icon palettes each have one exact 32-byte match in the retail file. Seven
identical text-palette copies remain candidates; the screenshots do not
identify which copy was uploaded. Both captures report sprite priority 5,
31/32 color-calculation ratio, and a VDP2 back-screen contribution. The
verifier checks each visible sprite texel against the screenshot using the
captured CRAM and back-screen state. The screenshot has no independent frame
number, so exact screenshot-to-command-frame identity remains unproven.

The New Game and Continue captures have byte-identical first 128 KiB of VDP2
VRAM, the configured NBG0 bitmap: SHA-256
`0a3b8b5ba0bd9e9c4b549d136961b44439d2534cbb7060c5bcbdca6da9c53d08`, with
18,513 nonzero bytes. The full Japanese retail-source scan found no complete
decoded NBG0 source match.

## Frame-bound NBG0 source span

A complete Ymir VDP2 write trace from the Japanese retail title session binds
a partial raw-source span to the final NBG0 snapshot. The trace receipt records
1,559,722 rows, zero dropped writes, and writes through frame 7,729; its
frame-8,060 VDP2 snapshot has `BGON=0x0001`, `CHCTLA=0x0013`, `BMPNA=0`,
`PLSZ=2`, and `MPOFN=0`.

The verified Japanese retail `TITLE.BIN` `TITL` record 2 begins at `0xb438`;
the joined span is at relative offset `0x46a` (file offset `0xb8a2`). Those
921 bytes have SHA-256
`6b9a333965267770333949175397a1faab028dd9d64d9f76ff9ae7a9e5d093fd` and
295 nonzero values. They map to VDP2 VRAM offset `0xd404`. Frame 6,869 writes
zero to each byte in that span. After replaying every subsequent frame-tagged
write, the 608 byte lanes written in frame 7,729 all match their retail source
offsets and the complete 921-byte target equals `TITLE.BIN`.
The 295 nonzero indices form a one-pixel line at native NBG0 coordinates
`x=13..307, y=106`, matching the blue divider below the title logo in the
paired 640×448 Ymir screenshot.

Reproduce this check against the external capture and authentic local CUE:

```sh
python3 scripts/verify_nexus_title_nbg0_source_trace.py \
  /Volumes/Extern-disk/firestaff-evidence/ymir_nexus_vdp2_complete_trace_receipt_20261027/dumps \
  --cue "$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue"
```

The trace CSV SHA-256 is
`c9007575bf688e8d4f8da1d85bd3632b46c18fdefa8eb32979f780b8b92befb3`; the
captured NBG0 snapshot file SHA-256 is
`de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c`.
This joins only 921 bytes. It does not decode the full NBG0 bitmap or establish
the complete layer consumer or native screen presentation; semantic admission
remains blocked.

## Reproduction

```sh
python3 scripts/verify_nexus_title_sprite_screenshot.py \
  /Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/dumps \
  /Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/title-menu.png \
  "$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue"

python3 scripts/verify_nexus_title_sprite_screenshot.py \
  /Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/menu_continue_input_20261027/dumps \
  /Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/menu_continue_input_20261027/screenshots/menu-down.png \
  "$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue"
```

The capture bundles and raw retail media are external evidence, not Git
payloads. This receipt proves only the listed foreground sprite layouts; it
does not admit the shared background or Configuration, or prove mouse hit
areas, save loading, or gameplay parity.
