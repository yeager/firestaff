# Nexus retail Magic window Y input (2026-10-29)

## Authentic source state

Ymir 0.4.2-dev ran on TRV2 with `--debug` against the Japanese retail Saturn
disc. The isolated profile used the Japanese IPL BIOS and an authentic
two-champion BKR. The BKR SHA-256 before and after the capture was
`3d60856119df55ffa4937d13b5dc7487157c7c6b131a999a2e61683316679634`, matching
`/home/trv2/.firestaff/data/nexus/nexus-two-champion-retail-written-20260926.bkr`.
The CUE SHA-256 is
`878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`; the IPL
BIOS SHA-256 is
`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`.

An isolated copy of the authentic Load-panel profile loaded its selected B01
save and reached a retail dungeon state. The gameplay checkpoint supplied to
the Y-input run has SHA-256
`3d50db52a6e9b947532211584ebcc86b1766187070dabddee78509218f683810`. The
checkpoint after Y has SHA-256
`e1323497f66da42974a8fd2ae8f46b1273b938df7c1f0a0cbd6750deec8c4071`.
Neither is synthetic state.

## Result

The capture loaded the authentic gameplay checkpoint and scheduled
`--capture-pad-input 0:y:1` for one frame, then ran 120 capture frames. The
resulting screenshot shows the native Magic window. This confirms that
Control Pad Y opens the retail Magic window from this state.

It does not establish rune selection, sequence validation, spell casting,
mana changes, effects, or Firestaff input/render parity. The SH-2 read trace
was bounded to `[0x0027fbb8,0x0027fd00)` for runtime champion offsets
`+0x18/+0x1a`; it is complete with zero matching rows. This run provides no
dynamic evidence for the rune validator or those fields.

The scheduled X/Y/Z/L/R support used for this run is in the existing
[Yeager/Ymir fork PR #1](https://github.com/yeager/Ymir/pull/1), not upstream
Ymir. That PR's CLI was built on TRV2; executable SHA-256:
`c0ff313552c17a8bd51f150241014f355bf68d25b061e7be3fa22242225c781c`.

## Raw evidence

The screenshot, logs, bounded trace, and two save states are retained outside
Git at:

`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028/nexus-magic-window-input-20261029/`

That directory includes a `SHA256SUMS` manifest covering its retained files.
