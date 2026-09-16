# CSB V1 Atari ST and Amiga original startup capture receipt

## Scope

This receipt records independent startup/title captures from original CSB
media. Both emulator sessions mounted media write-protected. The captures are
reference material for title timing, palette, and presentation only. They do
not establish input acceptance, HUD, door, audio, DSA, or full Firestaff pixel
parity.

## Atari ST

- Emulator: Hatari 2.6.1, STE profile, TOS 1.62.
- TOS SHA-256:
  `220fc9b35fd99908db9f9075fb3d850bf196d25741405ac6fa062facbbbd1583`.
- Original CSB STX SHA-256:
  `d9aed23f7916d60dfef61c7b79bc3eb1995f8afbb6a6c8b7b4160ee12ada1025`.

| Elapsed boot time | Observed presentation | Frame SHA-256 |
| --- | --- | --- |
| 18 seconds | FTL ident | `115f087c9838e0e02dd1677134da04aa21392e61720b8a45d5cfd6cb33509c91` |
| 36 seconds | Chaos title | `8ead262db0a0b467c8464e51836acb7c2102f1185ddefc075ef20e5e344695a0` |

A second independent, write-protected session confirmed the same startup
route: 20 seconds produced the FTL ident
`60e0cb7f203f0acefb72bd47914413868ef0eeea9e92d1d9de720cdabf626b2d`,
and 40 seconds produced the Chaos title
`02320a65f5f455523b3672eb2577aaa43b4a113d0b7a8ca9ee74a76d782f00e6`.

### Atari stable Entrance presentation receipt

After the presentation had settled, a separate write-protected session
produced a stable Entrance corridor viewport. Two captures at 110 and 128
seconds had the identical SHA-256
`9ce88e2af00863312426c515c3f72b662121f156fe12bbd95ce92aa5d8e150f6`.

This provides a source-bound Atari ST Entrance viewport reference. A
five-second host mouse hold at the documented Prison control `(260,50)` was
included in that session, but control sessions have not yet shown an
unambiguous state change. It must therefore not be treated as proof that the
emulated input route was accepted. The capture does not identify the party
state, prove a DSA state, or establish any Firestaff parity claim.

## Amiga

- Emulator: FS-UAE, A500 profile, Kickstart 1.3.
- Kickstart SHA-256:
  `ee05862d8102a08436ac4056da7d549db31625c7d47b24dfb7b3c9a5c113ca53`.
- Original CSB ADF SHA-256 values: disk 1
  `addaaa51255affcd9c53cc40026470b880eaba337c54a62a48ecb5b3b31a5f4d`,
  disk 2 `7fd59a061ab92a4f3380393e19ad294a6eef9adb31fcf71380850c0c9b91185e`,
  disk 3 `a6972616639c2d5fba8ca71aab2f1460e254605206a786f7576aa855bdef9c01`.

| Elapsed boot time | Observed presentation | Frame SHA-256 |
| --- | --- | --- |
| 32 seconds | FTL ident | `e9b5baa4ad41919b308a4e4c2ee39f1f16345ab3c5a289dd2bf99baa91f08675` |
| 52 seconds | Chaos Strikes Back title | `7db79506f9849ae8e61ad999141c7265e9b1b174cb432d10b021cbb5ca1cbf8e` |

A second independent, write-protected A500 session reached the same title
sequence at 24 seconds (`add7c11fa2d8b659cf634db581202183e4b415ce098135d54275e8250d915f6d`)
and 42 seconds (`09a576ebf918a20700ab648a1bd459fa0921916540054dcf7ca8e7dc237bf6c8`).

A third independent A500 session, using the capture helper's SDL dummy-host
audio path and its non-uniform-canvas admission gate, reached the FTL ident at
32 seconds (`1dae95b5addd44eb804639e5cf844aba34be5c946ad801a0f1c9031c25ee6482`).
This validates the native-capture route after rejecting an earlier all-white
boot canvas; it is still title evidence only.

The original screenshots remain outside the repository with the legally-owned
source media. The hashes above bind reviewable private captures to the exact
media and capture points without publishing game art.
