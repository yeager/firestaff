# English Nexus title-menu sprite join — 2026-10-27

This receipt records source and pixel verification for the English retail
title-menu sprites. Raw captures and extracted game data remain on the
external evidence volume at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_english_menu_source_join_20261027/`.
No synthetic image or game data was used.

## Authentic inputs

- English retail ISO SHA-256:
  `16786e6165d8cbf7f6394dd9bc7171fbb561c1ba40b77ad7cba3c275fde2804e`
- English retail CUE SHA-256:
  `b96f01e2f8ce3ab9c8e4a33d5a0c7076cdc1bfd247a85a1454e6c36c8a616f33`
- USA 1.01a IPL ROM SHA-256:
  `96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f`
- English `TITLE.BIN` SHA-256:
  `a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44`
- Menu savestate SHA-256:
  `a997636cb5b40745e1065827361d3653c07475e716c8397542f36c372514f62b`
- Ymir: TRV2 system build, version `0.4.2-dev`, `--debug` enabled.

The savestate and profile were copied into an isolated temporary profile. The
no-input run retained New Game selected. A separate run held Control Pad 1 Down
for four frames beginning at capture frame 4 and retained Continue selected.
Both captures loaded the same savestate and ran 12 capture frames; the retained
VDP1 trace frame was 5, with all 178 command rows present and zero dropped rows.

## Source and pixel results

The `TITLE.BIN` hash is admitted by the retail source manifest. The verifier
matched five VDP1 sprite commands to exact source offsets in the English
`TITLE.BIN`, then reconstructed each nontransparent sprite texel from its
captured CRAM color and VDP2 back-screen color. VDP1 horizontal flipping is
applied before comparison.

| Retail state | Sprite offsets in command order | Retail palette byte matches | Screenshot texels matched |
| --- | --- | --- | ---: |
| New Game | `0x14a6`, `0x14a6`, `0x0446`, `0x0576`, `0x07d6` | Icon palette: `0x1486`; text palette: `0x02f6`, `0x0426`, `0x0556`, `0x0686`, `0x07b6`, `0x0986`, `0x1f06` | 1,451 / 1,451 |
| Continue | `0x0316`, `0x14a6`, `0x14a6`, `0x06a6`, `0x07d6` | Icon palette: `0x1486`; text palette: `0x02f6`, `0x0426`, `0x0556`, `0x0686`, `0x07b6`, `0x0986`, `0x1f06` | 1,451 / 1,451 |

The two distinct 32-byte palettes match captured CRAM exactly. The icon palette
occurs uniquely at `TITLE.BIN` offset `0x1486`; the text palette has identical
copies at the seven listed offsets, so the capture does not identify which copy
the game uploaded. Both captures report sprite priority 5 and color ratio
31/32. These receipts bind the five visible English menu sprites to retail
texture bytes, retail palette bytes, placement, and screenshot pixels. The
wider source join still finds informative retail bytes for only 126 of 137
textured draws; 11 remaining spans are zero padding. This does not bind the
NBG0 background image, the complete screen compositor, the original upload
timing, configuration behavior, or Firestaff's runtime route.

## Reproduction

Extract the hash-verified ISO member with `iso_members_in_memory` from
`scripts/analyze_nexus_title_vdp2_source.py`, then run:

```sh
python3 scripts/verify_nexus_title_sprite_screenshot.py \
  /path/to/capture/dumps /path/to/screenshot.png /path/to/TITLE.BIN
```

The retained verifier output identifies the detected layout and reports every
source offset and per-sprite pixel match. The English menu-layout checks were
run against both external captures on 2026-10-27.
