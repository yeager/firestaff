# Theron JP Stage-2 `$491F` callee

This note records the 19-byte direct callee entered by the JSR at `$4A89` in
the Stage-2 user-data stream. The offsets refer to record user bytes, not to a
raw-file-relative CPU address.

## Authentic source binding

The source images are the hash-verified original Track 02 BINs:

| Edition | Track 02 SHA-256 | Raw sector | In-sector offset | Span raw offset |
| --- | --- | ---: | ---: | ---: |
| JP | `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | 1232 | 303 | 2,897,967 |
| US | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | 1233 | 303 | 2,900,319 |

The 19-byte span `[0x491f, 0x4932)` is identical in both editions. Its SHA-256
is `e72e8eca173f7ef11504b52c850637d7bc68159e047c39e09d8909f7407b6fea` and
FNV-1a-64 is `4974eb3a0826633f`. Three MAME 0.285 `unidasm` runs decoded both
editions to the same listing SHA-256:
`58516ae71204c206553c168603cf7f2e98d8f4806b3c0c0647bac29f1ff8fff6`. The
listing is `theron-jp-stage2-l491f-huc6280.asm`.

## Instruction-level behavior

At instruction level, the callee selects ST0 immediate value `$05`, writes
`$F3` to `$0002`, transforms `$F4` with `($F4 & $07) | $10`, stores that value
back to `$F4`, and writes it to `$0003` before returning. Its direct caller is
source-bound at `$4A89` in
`theron-jp-stage2-l4a84-window-20261008.md`. The focused Stage-2 verifier
admits the exact 19 authentic bytes for both editions.

This is a static instruction decode only. It assigns no semantic names to
`$F3/$F4` and does not prove runtime entry, display effects, or rendering
parity.
