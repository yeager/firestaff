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
