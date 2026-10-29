# Theron `$D39D` static source disassembly

Date: 2026-10-29

## Authentic Track 02 identity and window matches

The source comparison used the user-provided USA `TQUS02.bin` (8,104,992
bytes, MD5 `f23601102138f87c33025877767ebf76`, SHA-256
`f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565`) and
Japanese Rev. 1 `TQJP02.bin` (8,102,640 bytes, MD5
`b7afb338ad31be1025b53f9aff12d73a`, SHA-256
`d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`).

The 32-byte windows starting at logical `$D395` occur in seven repeated
groups per edition, separated by `0x49800` bytes:

| Edition | Track 02 offsets | Window SHA-256 |
| --- | --- | --- |
| USA | `0x0A4745`, `0x0EDF45`, `0x137745`, `0x180F45`, `0x1CA745`, `0x213F45`, `0x25D745` | `1df987fd3f39cab8af44dea067ac0cf89168e1fc41ae98e85b1bde7355c33634` |
| JP Rev. 1 | `0x0A3E15`, `0x0ED615`, `0x136E15`, `0x180615`, `0x1C9E15`, `0x213615`, `0x25CE15` | `a507dee0c26cc56bafcba54b16f338669d1319913bf170cd4fc8311e3e3174d5` for the first six; `6cc4a8be5a5c486029c6b63a011ce1cc663ddc8ac151db6f88d45db363c911cb` for the seventh |

The final JP copy has a regional `JSR $462D`; the other JP copies call
`$462A`. The USA copies call `$4630`. The listed offsets and hashes were
recomputed from the local authentic files. `MAME unidasm -arch h6280` decoded
the matched instruction window as:

```asm
00D395: 64 BA       STZ $BA
00D397: 64 BB       STZ $BB
00D399: A5 BB       LDA $BB
00D39B: C5 DA       CMP $DA
00D39D: B0 2D       BCS $D3CC
00D39F: A5 B4       LDA $B4
00D3A1: 20 30 46    JSR $4630       ; USA; JP copies call $462A or $462D
00D3A4: A6 B7       LDX $B7
00D3A6: 18          CLC
00D3A7: 65 B6       ADC $B6
00D3A9: 90 01       BCC $D3AC
00D3AB: E8          INX
00D3AC: C9 00       CMP #$00
00D3AE: D0 07       BNE $D3B7
00D3B0: E0 00       CPX #$00
00D3B2: D0 03       BNE $D3B7
00D3B4: A9 01       LDA #$01
00D3B6: 82          CLX
00D3B7: 85 8A       STA $8A
00D3B9: 86 8B       STX $8B
00D3BB: A5 BB       LDA $BB
00D3BD: A6 B9       LDX $B9
00D3BF: A4 B8       LDY $B8
00D3C1: 20 B8 D1    JSR $D1B8
00D3C4: F0 02       BEQ $D3C8
00D3C6: E6 BA       INC $BA
00D3C8: E6 BB       INC $BB
00D3CA: 80 CD       BRA $D399
00D3CC: A5 BA       LDA $BA
00D3CE: 60          RTS
```

This is a bounded indexed-record walk with an unresolved helper predicate and
computed intermediate output in `$8A/$8B`. The window does not directly read
mouse coordinates or `$2905`; the meanings of `$B4`, `$B6-$B9`, `$DA`, `$8A`,
and `$8B`, along with the helpers and callers, remain unknown. No direct
`JSR`/`JMP $D395` or `$D39D` byte sequence was found in either complete Track
02 file; indirect or trampoline entry remains possible.

## Correction to saved-state trace attribution

Earlier save-state traces reported `$2905` accesses at logical PCs `$D395`
and `$D3A0`, and a `$2904/$2905` clear at `$D39D`. The static retail bytes at
those addresses are respectively `STZ $BA`, a byte inside the operand of
`LDA $B4`, and `BCS $D3CC`. They cannot perform the reported `$2905` read/write
if this matched retail window is the executing mapping. Preserve the observed
RAM values as instrumentation output, but do not assign those accesses to the
matched `$D395` routine. The save-state traces lack an authenticated CD-to-RAM
receipt and do not include the live MPR/physical-PC/opcode evidence needed to
resolve a bank alias, stale PC attribution, or instrumentation boundary issue.

The next useful capture is a bounded same-session instruction trace at each
reported access, including logical PC, physical PC, active MPRs, opcode bytes,
effective address, and read/write value, followed by an LBA-to-RAM source
join. Until that is available, this decode does not establish a mouse handler,
click target, command consumer, or world-state behavior.

## Reference scope

[DMWeb's Theron's Quest edition documentation](http://dmweb.free.fr/games/therons-quest/editions/pc-engine-turbografx/)
describes the retail PC Engine / TurboGrafx controller controls.
[Greatstone SCK](http://greatstone.free.fr/dm/)'s Theron conversion is a custom
CSBWin dungeon and is not the retail PC Engine executable or data format; it
cannot identify this HuC6280 routine. The
[`yeager/pcrecomp` README](https://github.com/yeager/pcrecomp) describes
lifters and differential tests for x86-16/32/64, not HuC6280, so its
register-and-memory comparison discipline is a methodological reference only.
No code from that repository or from Greatstone is used as Theron retail source.
