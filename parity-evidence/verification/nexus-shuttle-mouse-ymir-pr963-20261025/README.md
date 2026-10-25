# Ymir PR #963 Nexus input probe (2026-10-25)

## Purpose

Verify that Ymir PR #963 builds and connects SDL's global mouse to an emulated
Saturn Shuttle Mouse when the desktop reports the pointer as SDL mouse ID `0`.
This is an emulator input-path check. It does not by itself establish Nexus's
retail mouse controls or authorize Firestaff inventory, combat, spell, or item
actions.

## Authentic media and build

- Disc: Japanese retail Dungeon Master Nexus from
  `/home/trv2/.firestaff/data/nexus/`.
- Retail `DM.BIN` SHA-256:
  `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`.
- CUE track 1 SHA-256:
  `91a6e0bbed515475be57bf073aa9ea613b49eb65fb4b85ff06472025f9f3676d`.
- Saturn IPL SHA-256:
  `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`.
- Ymir source commit built:
  `447508706ee161ba100e55adc10b958cfaa0caf7` (PR
  [#963](https://github.com/ymir-emu/Ymir/pull/963)).
- Linux Debug configuration and `ymir-sdl3` target both succeeded on TRV2
  (`x86_64`, Clang 22.1.8). The app build completed all 598 Ninja steps,
  including `mouse_capture_service.cpp`.
- Dependencies and build output were isolated under `/dev/shm`. No operating
  system packages were installed. Vulkan shader compilation was disabled
  because DXC was unavailable; SDL3 reported the X11 video driver as
  `x11(dynamic)`, and the runtime used Xvfb display `:101`.

## Runtime setup and findings

Ymir ran with `--debug --fast-forward`, authentic retail media, and a private
profile containing the hash-verified IPL. The input profile used a ControlPad
on port 1 and a Shuttle Mouse on port 2.

The SDL global pointer was moved and clicked under Xvfb. Ymir displayed
“Mouse bound to Shuttle Mouse on port 2” and “Entering mouse capture mode”.
This verifies that the PR's global-mouse path reaches Ymir's selected
ShuttleMouse peripheral. See
[`shuttle-mouse-bound-port2.png`](shuttle-mouse-bound-port2.png).

The same authentic boot showed keyboard input reaching the ControlPad: holding
the profile's `W` binding changed the date setup year from 2026 to 2027, and
the `D` binding moved the selected field. See
[`keyboard-date-year-change.png`](keyboard-date-year-change.png).

The game later reached a rendered Nexus scene. A baseline and a capture after
a 180-pixel rightward XTEST pointer movement are included as
[`nexus-game-baseline.png`](nexus-game-baseline.png) and
[`nexus-game-after-mouse-right.png`](nexus-game-after-mouse-right.png). The
scene is animated and no clearly identified hand-pointer movement or
retail-visible action can be attributed to the input from these two frames.
That result is inconclusive about Nexus gameplay semantics. A future probe
needs a controlled, identifiable pointer state and a retail state change
before Firestaff can claim mouse-action parity.

## Screenshot hashes

| File | SHA-256 |
| --- | --- |
| `keyboard-date-year-change.png` | `0c1cde30e8e960efa6b0291323bc32801a040dfb336405791fd28b9f9b098bc2` |
| `nexus-game-after-mouse-right.png` | `17ba884a1c893229b79bcae352b541efa468d51fc1a7efad641d2822a2934c16` |
| `nexus-game-baseline.png` | `ead5d3ae905e0f563a4b53f1597f62d28a38e9a583a96641f77e580a7f1102df` |
| `shuttle-mouse-bound-port2.png` | `a487cf193fb732faa6442b86d284d90a71c8a52ff9aecaf66d5f9afbf1074c58` |

No ROM image, dump, or save-state binary is included in this evidence directory.
