# Ymir 0.4.1 retail boot observation — 2026-10-24

Ymir 0.4.1 was launched on TRV2 under Xvfb with its debug option and
fast-forward enabled. It loaded the authentic Japanese Nexus retail CUE image
from `.firestaff/data/nexus` and reached the game's opening eye sequence. The
retained PNG is a full 1280×800 Xvfb screen capture showing the Ymir window;
it is not a direct framebuffer dump.

The run used a disposable profile under `/dev/shm`. It did not load a game
save state or modify the source Ymir profile. The BIOS and CUE hashes below
were checked before launch:

| Input | SHA-256 |
| --- | --- |
| Official Ymir Linux x86-64 SSE2 v0.4.1 release archive | `d7eebf7ac28e7273a924bde7ac6e4d28107d252733029679c2d0e1c590336b06` |
| Japanese Saturn BIOS | `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac` |
| Japanese retail disc CUE | `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699` |
| Retained screen capture | `aa0f9bcb16aea06cc1d1442606d45eab259f820c55353e4018e76a063f81d460` |

The release archive came from the [official Ymir v0.4.1 release](https://github.com/ymir-emu/Ymir/releases/tag/v0.4.1).

This observation confirms a retail-media boot path in Ymir. It does not establish
Firestaff renderer parity, title-menu composition, viewport/HUD parity, or
retail asset ownership. No synthetic game data, direct push to `main`, or
release was used.
