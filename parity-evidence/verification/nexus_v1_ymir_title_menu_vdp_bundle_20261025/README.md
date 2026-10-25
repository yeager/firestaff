# Nexus Japanese retail title-menu VDP bundle

This is a reproducible Ymir `--debug` capture of the Japanese retail Nexus
title menu. It was restored from the retained title-prompt state whose SHA-256
is `11d88ef517c408b3bca5c078c0ccb8081dbf3342f308239dd73403c73f0c3774`,
then Start was pressed and Ymir was paused on the menu with `NEW GAME`,
`CONTINUE`, and `CONFIGURATION` visible.

The disc is the real Japanese retail CUE, SHA-256
`878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`.
The screenshot and dumps are produced by Ymir 0.4.2-dev with PR #961's
atomic debug-memory bundle: VDP registers, VDP VRAM/CRAM, and the VDP1 command
history are captured together while emulation is paused.

The raw memory exports are not tracked because they can contain commercial
game data. Their hash-pinned identities are retained below. Keep them in a
local untracked directory and give that directory to the verifier.

This is emulator evidence, not a hardware capture. It does not satisfy the
Mednafen/original-Saturn provenance gate in Firestaff and it does not authorize
menu rendering, text decoding, palette promotion, or save-state decoding.

## Captured state

`vdp1-capture-bundle.txt` records frame 497, command index 178, 11,896 retained
commands from frames 430 through 497, 178 commands for the captured frame, and
zero dropped commands. `vdp-register-snapshot.txt` has the same frame, command
index, and current command address. The picture in `title-menu-paused.png` is
the UI state represented by those files.

The VDP2 VRAM and CRAM hashes equal the independently retained retail title
prompt capture. That is a useful observation that the transition into this menu
does not itself prove a new VDP2 background upload. It does not establish which
VDP1 commands own individual menu assets.

## PALT result

The verifier compares the exact 512 PALT entry bytes from the canonical real
`MENU.BPK` trailer with VDP1 VRAM, VDP2 VRAM, and VDP2 CRAM. No complete
big-endian or word-swapped PALT byte sequence occurs in any of those regions.
This negative result rules out a direct contiguous copy for this frame only;
it does not establish a palette format or a renderer route.

Run from the repository root with the real data directory:

```sh
python3 scripts/verify_nexus_ymir_title_menu_vdp_bundle.py \
  --data-root /path/to/.firestaff/data/nexus \
  --dump-root /path/to/local/ymir-title-menu-dumps
```

## File hashes

| File | SHA-256 |
| --- | --- |
| `title-menu-paused.png` | `e30bfc58f9c8a1bf8485007253dc150ef0091e01b670c1684010ac4ad0a346a9` |
| `vdp1-capture-bundle.txt` | `76e920d7ecb3efeb15599dd72772789fdcadc8c1604bb5828455dc17ecbd82b9` |
| `vdp-register-snapshot.txt` | `176d158e6bcdf6ea4a640b95be9f0cbe10532f0b34965f946fe90a5c577dd476` |
| `vdp1-command-trace.csv` | `47d55ec3a4b45c97a96390c60322edc0f56dd0aad6f0e11677e0b2dbff962d81` |
| `vdp1-vram.bin` | `38ec97126b500f6c7f4a34b6b4c1b54895ed6113f2b1b32b23b8635e07b79a18` |
| `vdp1-registers.bin` | `26d485a48f56ddf97c2209343b92f45fdcf4e41937d1e9f198db2b0dc1df26b5` |
| `vdp2-vram.bin` | `de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c` |
| `vdp2-cram.bin` | `db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de` |
| `vdp2-registers.bin` | `79efa616c2db727749aa9018864d1ad32c1583935652fafb16534ba69f8e1f2d` |
