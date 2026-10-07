# Theron JP Stage-2 `$4943` source window

This note records a bounded JP Track 02 decode at the Stage-2 user-data offset
also used by the existing US `$4943..$49f9` source lock. The offset is read
within the Stage-2 record's 2048-byte user-data stream; it must not be confused
with a CPU address relative to the `$4000` initial load base.

## Authentic source binding

The source images were the hash-verified original BINs:

| Edition | Track 02 SHA-256 | INDEX 01 raw sector | Stage-2 first raw sector | Span raw offset |
| --- | --- | ---: | ---: | ---: |
| JP | `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | 224 | 1223 | 2,898,003 |
| US | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | 225 | 1224 | 2,900,355 |

Stage-2 record offset is `0x3e7`. The 183-byte range `[0x4943, 0x49fa)` maps
to raw sector 1232/in-sector offset 339 for JP and sector 1233/in-sector offset
339 for US (the 16-byte raw-sector header is included in each raw offset).
Both editions contain byte-identical spans with SHA-256
`81baf6cd239f87bd4fac92e195c2bf09a20fd7fb39f97c79c5971540dfb56590` and
FNV-1a-64 `dcdc5b26be9afc17`.

MAME `unidasm -arch h6280 -basepc 0x4943 -count 183` produced identical
listings for JP and US in three loops (listing SHA-256
`bd3093d770d0a5e9de87902c8a6d80e62a02bae8856d583f7d159096195111c4`). The
leading control flow is:

```asm
$4943  TMA #$08 / PHA
$4946  TMA #$10 / PHA
$4949  TMA #$20 / PHA
$494c  TMA #$40 / PHA
$494f  LDA $300a
$4952  TAM #$08
$4955  INA / TAM #$10
$4958  INA / TAM #$20
$495b  INA / TAM #$40
...
$496f  JSR $5e2b
...
$4981  JSR $5ce4
$4984  JSR $4bb0
...
$49ba  JSR $56de
$49bd  JSR $563d
$49c0  JSR $50f1
$49c9  JSR $49fa
$49cc  JSR $5111
$49cf  JSR $570a
...
$49f9  RTS
```

The registered `test_theron_v1_stage2_disassembly_chain` now authenticates both
editions through the IPL loader, checks the JP and US span hashes, compares all
183 bytes, and locks the direct JSR and relative-branch destinations. This
establishes shared static bytes and decode only. It does not prove JP runtime
entry, VDC output, or graphics/gameplay parity.

## Direct `$49FA` entry

The caller's direct JSR at user offset `$49c9` targets `$49fa`, which is also
the end of the `$4943` bounded window above. Treat `$49fa` as a separate entry
rather than continuing the prior instruction stream: its 15-byte body ends at
`$4a09` with RTS at `$4a08`. For JP, the span begins at raw BIN offset
`2,898,186` (sector 1232, in-sector offset 522) and has SHA-256
`4aafed85e7bf93018d4a6a34d453f435031377eb90a60ce55dab828a8cb6c1b9` and
FNV-1a-64 `1d56ad35f7cb2eb3`. US has the same bytes and digest.

The focused test checks both media hashes, byte equality, the branches at
`$49fc` and `$4a03`, JSR targets `$4a09` and `$4a84`, and the terminal RTS.
This binds the direct-entry bytes and control-flow edges only; it assigns no
rendering semantics and proves no runtime selection or graphics parity.
