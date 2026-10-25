# Nexus gameplay VDP2 CCMD capture (2026-10-25)

## Purpose

This authentic Ymir frame records the VDP2 color-calculation mode while the
Japanese retail game is displaying its dungeon viewport and HUD. It shows why
the ratio-only mixer used by the existing helper is not valid for every
captured Nexus frame. It does **not** show NBG0 using that mixer: the NBG0
enable bit is clear in this frame. This record is evidence for implementing
and gating the CCMD branch without misattributing it to a layer; it does not
claim that Firestaff currently reproduces this frame.

## Source and capture identity

Ymir 0.4.0-dev loaded the Japanese retail CUE and matched Japanese IPL BIOS.
The source hashes from the isolated Ymir profile and launch log are:

| Input | SHA-256 |
| --- | --- |
| Japanese retail CUE | `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699` |
| Japanese IPL BIOS | `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac` |
| Retail `DM.BIN` | `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124` |
| Internal backup RAM | `87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035` |

Ymir's frame-bound capture manifest identifies frame 1800 at 320×224, with
the display and draw VDP1 framebuffer banks recorded. Its raw files are
preserved outside Git at
`/Volumes/Extern-disk/nexus-framebound-20261020-authentic/`.

| Capture file | SHA-256 |
| --- | --- |
| `nexus-frame-1800.manifest.txt` | `061d05c60524d67e3023478c3585103601395b7bdc26810c40c93ada85e14492` |
| `nexus-frame-1800.preview.png` | `cb60386f71ba6818f82154e2797fe6627c8b04993afdf5d70dd4e4d0b091200a` |
| `nexus-frame-1800.screen.xrgb8888` | `494b2bf712b2ad7fcb6b8910c5535a4c1cc200cd3c0bbd0ab76d6006e56fa359` |
| `nexus-frame-1800.vdp1-vram.bin` | `98ac79e56cbf85e2044f9d8a1cc15596dc627535cf9658afc2dc0b0a8f4ea428` |
| `nexus-frame-1800.vdp1-regs-le16.bin` | `7e98a435a494b31a9551a817bdeb0003a180347a1f4d4375d0444127c214d4f7` |
| `nexus-frame-1800.vdp1-fbram0.bin` | `c7eb21cf043d528c0c19e4eef7878594d785d09f0f84b02be0e59fbc9ab1e417` |
| `nexus-frame-1800.vdp1-fbram1.bin` | `c7eb21cf043d528c0c19e4eef7878594d785d09f0f84b02be0e59fbc9ab1e417` |
| `nexus-frame-1800.vdp2-vram.bin` | `317915dac02650cc13283cf9dce6c89c0e1d1520428a7c242245795598890faa` |
| `nexus-frame-1800.vdp2-cram.bin` | `e7bf414b61c066475cdd6caafbeda3a9a40f97474aa760a419dc84a1a5effab0` |
| `nexus-frame-1800.vdp2-regs-le16.bin` | `82ddd6c84bfff8f6a7bf0e32a2b7965bca54ee104eea15a8ca2c70078d177b40` |

## Register evidence

The capture's little-endian VDP2 register span gives:

| Register | Value | Relevant meaning |
| --- | ---: | --- |
| `TVMD` | `0x8000` | Display enabled |
| `BGON` | `0x0007` | NBG0, NBG1 and NBG2 enabled |
| `CHCTLA` | `0x1013` | Captured NBG0/NBG1 format configuration |
| `CCCTL` | `0x0142` | NBG0 bit 0 clear; NBG1 bit 1 and sprite bit 6 set; global `CCMD` bit 8 set |
| `SFCCMD` | `0x0004` | Captured special-function mode selection |
| `CCRNA` | `0x1000` | Raw ratio register; bypassed by `CCMD` |
| `WCTLA`–`WCTLD` | all `0x0000` | No window-control bits set in this capture |

Mednafen's VDP2 `MixIt` implementation selects the mixer from `CCCTL` bit
`0x0100`. Its `CCMD` branch ignores pixel ratios and adds the two color
channels with saturation for pixels admitted to color calculation. In this
frame, `CCCTL` bit 0 is clear, so NBG0 is not admitted by this layer bit;
`CCRNA=0x1000` and `CCMD=1` do not establish an NBG0 blend. The enabled NBG1
and sprite paths require per-pixel source/priority evidence before assigning a
visible output to the additive branch. This differs from the title-menu
capture, where screen color calculation is enabled and the 31/32 ratio path is
matched exactly.

## Scope

This capture proves the register state and supplies a same-frame native output
and VDP buffers for a future pixel-level comparison. It does not by itself
prove which captured pixels traverse the additive branch, authorize a
production render path, or establish general parity across gameplay states.
