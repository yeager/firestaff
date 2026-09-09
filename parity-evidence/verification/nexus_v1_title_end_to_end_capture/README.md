# Nexus V1 title end-to-end Saturn capture receipt

## Scope

This is a same-session, original Saturn title capture used to bind the native
title loader to observed hardware transport. The emulator and BIOS were used
only to create development evidence; Firestaff's runtime neither requires nor
loads them.

The session uses the Japanese retail disc revision with SHA-256
`878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699` and
the capture session identifier `nexus-title-end-to-end-r6`.

## Bound artifacts

| Artifact | SHA-256 |
| --- | --- |
| Combined VDP1/VDP2 raw state | `8830da5cc8a3ba336193d214969335b25f5374c787f0d4cc841e7b8e45d65164` |
| CD block FIFO words | `03940ba711586532b0683ca734391b5bfb900c443ee9bcae65a7510f30e39c0e` |
| SH-2 RAM source writes | `042b72ff2eefb73587b2642163eb29f81c4feced764a90392eb8d86d72c5f6b5` |
| SH-2 instruction byte reads | `dafe46aea3df67a4536bfa9326cc0b662a0222de5846f69ce380af8e381f8f6d` |
| VDP2 writes | `1149747382ee01b4c981fe3c0ef733625a424664068210cf21d51bd7bb07177b` |
| VDP2 writer registers | `43ac9738985d3ffd4d9251b11e3f1e2ef0a12046d2a34cfdb9ee7cecff56c852` |

The source verifier validates the CD FIFO against Track 1, then validates the
RAM, instruction, writer-register and VDP2 value/call-chain links before
admitting the VDP1/VDP2 raw state.

## Honest boundary

This receipt proves the title transport and display-write chain. It does not
yet bind the title MAPD asset transform to its final display consumer, and it
does not prove menu interaction, HUD, dungeon geometry, palette state during
gameplay, or timing after title handoff. Those require separate same-revision
runtime captures and remain fail-closed.
