# Nexus gameplay keyboard input (2026-10-25)

## Scope

This is a controlled Ymir 0.4.1 `--debug` comparison of directional input in an authentic Japanese retail Nexus gameplay state. It verifies retail ControlPad response; it does not by itself establish Firestaff runtime parity. Firestaff uses its DM1-style host keyboard layout, which intentionally differs from mapping the raw key letters directly to Saturn D-pad directions.

## Authentic source state

- Retail disc: `/home/trv2/work/firestaff-nexus-emulator-save-trace-20261005/disc/Dungeon Master Nexus (Japan).cue`, SHA-256 `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`.
- Gameplay savestate: `DUNGEON MASTER NEXUS`, product `T-9111G`, version `V1.003`, Japan. The source savestate was copied read-only to a private profile and loaded as slot 1 with `Ctrl+1`; no save state was written.
- Japanese Saturn BIOS SHA-256: `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`.
- Internal BUP SHA-256: `87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035`. It matches the authentic four-champion retail-written BKR corpus sample byte-for-byte.
- Gameplay savestate SHA-256: `942154f51530094c4f553d3254573098d056f90da2fbb5966448aefdefeb3372`.

All copies used for the run were confined to a private profile under `/dev/shm`. The source state, BIOS, and BUP hashes were rechecked after the run and unchanged.

## Reproduction

Ymir ran on a private Xvfb display with Openbox, `--paused --debug`, CPU affinity 7, a 50% CPU quota, a 1 GiB memory limit, and a 64-task limit. The profile used a ControlPad on port 1, with W/S/A/D bound to D-pad up/down/left/right, J to A, and L to C.

1. Load slot 1 with `Ctrl+1`, resume with Space, and wait six seconds for the baseline.
2. Hold W for two seconds, release it, wait four seconds, and capture the forward image.
3. Reload slot 1 with `Ctrl+1`, pause and resume, wait six seconds, hold A for two seconds, release it, wait four seconds, and capture the turn image.

No inventory action or game save was issued.

## Results

ImageMagick `compare -metric AE -fuzz 5%` compared each full 1280×800 capture to the baseline. Reloading the same state without input changed 0.00175% of pixels, providing a stability control. W changed 3.08408% of pixels and visibly advanced the dungeon view. A changed 5.08610% and visibly rotated the view. The party HUD remained visible in all three frames.

| Capture | SHA-256 |
| --- | --- |
| `captures/baseline.png` | `c9045ed18f3ad9764e5b6c7f59b4b19a9464d610e73962888d75bd8cbafc7253` |
| `captures/keyboard-forward-W.png` | `8924128a632960c134a2c2e85ab392fcea2f0e36ec2ca70c9e12032c3e141f7a` |
| `captures/keyboard-turn-A.png` | `908e9cb894bfdd7c4907fd5706aa8c4dc9abb7f0e80690fa13414a750f315443` |

This confirms authentic retail forward and turn response through Ymir's ControlPad mapping. Firestaff maps A/D to Nexus turns from this evidence while preserving DM1's separate A/D strafe bindings; Q/E and Home/End remain additional turn bindings.

## Mouse status

The isolated Xvfb run exposed only its XTEST core pointer. A nonzero SDL mouse device was not available for Ymir's ShuttleMouse path, so this run does not verify physical ShuttleMouse behavior, pointer targeting, or inventory clicks. Firestaff's desktop mouse movement-pad mapping is covered separately by `test_nexus_desktop_input_mapping`; unsupported retail click actions remain gated on a verified consumer.
