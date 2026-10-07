# Theron JP Stage-2 `$4B24` helper

This note records the direct BSR target at `$4B24` from the caller at `$4AE1`
in the Stage-2 user-data stream. Offsets refer to the record's user bytes, not
to a raw-file-relative CPU address.

## Authentic source binding

The source images are the hash-verified original Track 02 BINs:

| Edition | Track 02 SHA-256 | Raw sector | In-sector offset | Span raw offset |
| --- | --- | ---: | ---: | ---: |
| JP | `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | 1232 | 820 | 2,898,484 |
| US | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | 1233 | 820 | 2,900,836 |

The 24-byte span `[0x4b24, 0x4b3c)` matches in JP and US. Its SHA-256 is
`bbca9fd632691c5d234d694fe4ed84f278c2b24d39da48a4f48475acee40b99d`; its
FNV-1a-64 is `65ad3d946517961d`. Both values were independently reproduced
from the original media. Three MAME 0.285 HuC6280 disassembly comparisons
produced the same JP/US listing SHA-256:
`d5c35a93f66068445124a7d493e020618bc4b414cedf383df0dcaa2d8b337f78`. The
complete listing is `theron-jp-stage2-l4b24-huc6280.asm`.

## Instruction-level behavior

The caller at `$4AD7..$4AE1` copies `$3B7A` to `$3B` and `$47D5` to `$3A`, then
branches to this helper. The helper shifts `$3B` left by six across `$3D:$3C`,
adds `$3A` to the low byte, propagates carry to `$3D`, and returns. Thus, for
the observed caller inputs, its exact arithmetic result is
`($3B7A << 6) + $47D5`, stored low byte first at `$3C/$3D`. The caller then
loads those bytes and writes them to `$0002/$0003` after selecting `ST0 #$00`.

This is a static decode of original JP/US bytes and their direct caller. It
assigns no semantic names to the RAM fields, proves no runtime entry, and does
not establish VDC/rendering behavior. The BSR target, conditional branch, and
RTS are covered by the focused source-lock test.
