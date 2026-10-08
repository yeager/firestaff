# Theron JP Stage-2 `$571A` direct target

This note binds the 24-byte window `[0x571A, 0x5732)` from the Stage-2 user
record. It is reached by the direct `JMP $571A` at `$56E8`, inside the
already-locked `$56DE` routine. `$56DE` itself is called from the locked
Stage-2 `$4943` chain. The caller/target bytes establish a static edge, not
that execution takes it.

## Authentic media

| Edition | Track 02 SHA-256 | Raw sector | In-sector offset | Span raw offset |
| --- | --- | ---: | ---: | ---: |
| JP | `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | 1233 | 1834 | 2,901,850 |
| US | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | 1234 | 1834 | 2,904,202 |

The JP and US windows are byte-identical. Their 24-byte span SHA-256 is
`b90beca7963cdbb93612902a015b013e69eedb6c1cc8e77426d2b57ddef7da0a` and
FNV-1a-64 is `0c210360aebbf9f7`. The focused real-media test checks both
digests and compares every JP byte against US. The linear HuC6280 decode is
[`theron-jp-stage2-l571a-huc6280.asm`](theron-jp-stage2-l571a-huc6280.asm).

## Static instruction facts

The bounded decode starts with `JSR $58C5`, writes `$09`, `$10`, `$E0`, and
`$58` to `$58DC..$58DF`, then returns. These are the loaded instruction and
operand bytes only; this lock makes no claim about the callee's behavior,
runtime execution, or rendered gameplay. The broad US listing is not used to
decode this window because its mapping conflicts with the authentic raw
Track 02 bytes.
