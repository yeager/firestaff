# JP UP input latch and blocked-wall probe, 2026-10-31

## Authentic state and capture

Two isolated Mednafen 1.32.1 PCE Fast runs loaded the same pre-existing JP
Akutuba F5 state (`theron-jp-akutuba-f5-state.mca`, SHA-256
`2cc9938b96640a74db1a5b706113564b5d578d5011daf5f85c588ef1c98d70ee`). Both
used the authentic Japanese Rev. 1 CUE (SHA-256
`7bbbe8b077f35b2e9b05c68848f8cb86ea9aa74021a076e882eee8841bc56545`) and
verified System Card file (SHA-256
`df4f75feebb95e53dfef72dea0787df743e3474ba046ff5a4cb88e34dda93ff1`). The
isolated instrumented Mednafen binary used for the paired RAM traces had
SHA-256 `5fb4c92ceac92f79b9adb7546e8a828633a124522bc88072a1c6c44416ed356a`.
Each run had a separate profile and output directory; game data and the
source state were read-only. The input was injected only as a reference
stimulus; no game state or media bytes were synthesized or edited.

The UP run applied `up@61:120`; the neutral control used no scripted input.
Both ran to roughly 400 PCE Fast input frames. In a separate screenshot pair
from the same exact state, UP and neutral both still showed the same front
wall. A separate `down@61:120` screenshot also showed that wall. These screen
captures are visual checks from separate emulator sessions, not frame-joined
render receipts.

## Joined input and RAM observations

| Run | `$44C1` poll at frame 61 | `$2912` path in the bounded trace |
| --- | --- | --- |
| UP | `raw=0010`, active-low result `3e` | `$D334` read `00` at event 19536; `$D349` wrote `10` at frame 95/event 19538; `$D334` then read `10` at event 19564. Further `$D349` writes of `10` recur every 12 input frames through frame 179. |
| Neutral | `raw=0000`, active-low result `3f` | No `$D334` read of `$2912` or `$D349` write of `$2912` was observed. A separate `$2912=00` write occurred at frame 95 from PC `$D2F2`. |

Both captures wrote input, movement-context, and main-RAM-consumer traces. The
UP/neutral SHA-256 pairs are:

| Trace | UP | Neutral |
| --- | --- | --- |
| Input | `d6a1b8a256b07f76bfe783ff47c99f5d2caa0c5614477cbfaaaa4294c3b8b47e` | `7d9883b7571ecbefcebef19c10314268ff6ae82dcf9f53a7998b42512963d701` |
| Movement context | `33522dd50efe5581d167a7216143295cbb9d10c51df9c64a654b799996e00c80` | `1df79e928304ee0a922d65aa8d9615076f3db64e02e5dffd1d63afebe7011583` |
| Main-RAM consumer | `1921237dc45147201b939de7f1e2e6af0d41d942ce07801b74dcc860875419aa` | `013c514023e6fba7886e954c48e4e4763b23cebfa7d7342cfb004c2c00c0e308` |

## What this does and does not establish

This reproduces the input-to-`$2912` consumer/write ordering from the earlier
[same-state comparison](theron-jp-pcefast-input-state-compare-20261031.md)
and adds a visual check: the saved pose is facing a wall, and the tested
directions did not change the visible wall in the separate screenshots. The
movement-context trace contains no `$C1FC` TII candidate event and no observed
write through its `$2040/$2041` position hooks. The observation is bounded to
those hooks and this state; it does not rule out another position path.

The frame-61 poll and frame-95 latch transaction are ordered observations, not
proof that the poll caused the write. `$2912` semantics, a successful party
move, and native pose-to-screen rendering remain unproven. Save-state autoload
also bypasses the original CD-to-game-RAM load, so this capture cannot provide
that provenance. Do not change production movement or rendering from this
blocked-wall result. The next useful gameplay capture needs an authentic,
reachable position and a same-session join from input through changed party
position to the visible screen.

Raw emulator logs and screenshots remain private on TRV2 under
`/dev/shm/firestaff-theron-paired-movement-v2-20261031/` and
`/dev/shm/firestaff-theron-paired-screen-20261031/`; no game media, save state,
BIOS, or emulator binary is included in the repository.
