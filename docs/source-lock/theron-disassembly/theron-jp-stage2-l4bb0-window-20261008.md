# Theron JP Stage-2 `$4BB0` scroll-update callee

This note records the 93-byte direct callee at `$4BB0`, reached from `$4984`
in the Stage-2 user-data stream. Offsets refer to the record's user bytes, not
to raw-file-relative CPU addresses.

## Authentic source binding

The source images are the hash-verified original Track 02 BINs:

| Edition | Track 02 SHA-256 | Raw sector | In-sector offset | Span raw offset |
| --- | --- | ---: | ---: | ---: |
| JP | `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | 1232 | 960 | 2,898,624 |
| US | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | 1233 | 960 | 2,900,976 |

The 93-byte span `[0x4bb0, 0x4c0d)` is identical in both editions. Its
SHA-256 is `2cc2462ea866e130b0aede80ac40d29aa769a6649ff6244cc9d228b3390e21a9`
and FNV-1a-64 is `9e75a8a513fb0994`. Three MAME 0.285 HuC6280 disassembly
loops matched JP and US listings with SHA-256
`beca5347eeeb875f64293ce010eef58bd1f967b909f7fe436027bc6c9f0b9640`. The
complete linear listing is `theron-jp-stage2-l4bb0-huc6280.asm`.

## Instruction-level observations

The routine first checks `$4C11`, then decrements `$4C10`; a nonzero result
returns at `$4C0C`, while zero reloads `$4C10` from `$4C0F` and proceeds. It
transforms the values at `$4C0D` and `$4C0E` in place, selects opcode `$69`
(`ADC`) or `$E9` (`SBC`) by writing to the opcode bytes at `$4BE2` and `$4C05`,
and updates the two low/high register pairs `$220C/$220D` and `$2210/$2211`.
The successful update path loads `$01` into A before returning. The source
verifier recognizes the static register-store instruction bytes. This proves
that the authenticated image contains those instructions, not that execution
reached them or that a runtime VDC write occurred.

The code stream is self-modifying at `$4BE2` and `$4C05`; the checked-in
listing records the authentic loaded bytes, not a post-execution state. This
static decode and the existing media verifier do not prove runtime selection,
the frequency or direction of updates, or rendered scroll parity.
