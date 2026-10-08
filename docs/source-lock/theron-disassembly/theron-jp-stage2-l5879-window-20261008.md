# Theron JP Stage-2 conditional target `$5879`

The JP-only `BBR4` at `$58C5` branches to `$5879` when bit 4 of zero-page
`$C8` is clear. The relative operand is `$B1`, so the target is computed from
the next instruction address `$58C8` as `$58C8 - $4F = $5879`. The branch is
inside a JP-only `$58C5` window reached by the `$571A` routine's `JSR $58C5`.
These locked edges describe static source bytes, not runtime control flow.
The `+3` relative base follows MAME's HuC6280 `ea_zpg`, `bra`, and `bbr`
implementation ([`h6280.cpp:2886-2904, 2935-2945, 3627-3632`](https://github.com/mamedev/mame/blob/master/src/devices/cpu/h6280/h6280.cpp#L2886-L2904)).

## Authentic source spans

Both images are hash-verified original Track 02 BINs:

| Edition | Track 02 SHA-256 | Raw sector | In-sector offset | Span raw offset | Span SHA-256 | Span FNV-1a-64 |
| --- | --- | ---: | ---: | ---: | --- | --- |
| JP | `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | 1226 | 137 | 2,883,689 | `bbee9339b6547617c75393de4a712a818dbc5619a756f56f01e70d57d3522aab` | `2647c26e2d10c559` |
| US | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | 1227 | 137 | 2,886,041 | `085b2ecd1ea18171b2e35e41f7871ec27eb2c1052b713d4d48d0f2268eb66add` | `fa454293b9aaadb1` |

The 15-byte target windows at user offset `$1879` differ. JP enters at
`$5879` with `STY $C84F`, an overlapping entry into the byte sequence whose
earlier entry begins at `$5878`; it reaches `RTS` at `$5887`. The US bytes at
the same user offset are distinct and are not this branch target. The focused
real-media test checks the JP branch opcode and target calculation, checks
both edition-specific digests, and compares each JP target byte with the
expected bounded bytes. MAME 0.285 HuC6280 listings
were produced directly from both original Track 02 BINs; the JP listing is
[`theron-jp-stage2-l5879-huc6280.asm`](theron-jp-stage2-l5879-huc6280.asm).

## Bounded JP decode

`$5879` stores Y to `$C84F`, loads through `($18),Y` and stores to `$4F8D`,
increments Y, loads/stores at `$4F8E`, then returns. This overlapping entry is
a second decoding path over the final operand bytes of the preceding
instruction; no independent routine boundary, runtime register state, semantic
role, or gameplay parity is inferred from this static decode.
