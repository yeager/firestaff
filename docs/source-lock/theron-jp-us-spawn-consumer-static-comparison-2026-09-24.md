# Theron JP/US regular-spawn consumer — static comparison (2026-09-24)

This comparison uses only the hash-authenticated retail raw Track 02 BINs:

| Region | Track 02 MD5 | Spawn consumer user-data offset | Bytes | FNV-1a |
| --- | --- | ---: | ---: | ---: |
| JP Rev 1 | `b7afb338ad31be1025b53f9aff12d73a` | `$0868D2` | 269 | `7dc1e453` |
| US | `f23601102138f87c33025877767ebf76` | `$0870E5` | 269 | `eb241d19` |

Both source spans are admitted independently by
`theron_v1_track02_bind_spawn_consumer_source()`. A byte-for-byte comparison
found 230 identical bytes and 39 differing bytes. A 65C02-mode structural
disassembly of the two spans decoded 122 instructions at identical relative
boundaries and with the same operation sequence. HuC6280-specific branch
labels and relocated targets were reviewed against the existing US source
lock at `theron-us-spawn-consumer.asm`; the 65C02 disassembler output is not
itself treated as an authoritative HuC6280 disassembly.

The differences are operand relocations, not changed immediate formula
constants or reordered branches:

- US-local code and shared routine targets move by `$13` relative to JP
  (`$D19D/$D0FE/$D23A` in US versus `$D18A/$D0EB/$D227` in JP).
- The regular-spawn helper and arithmetic helpers have distinct addresses
  (`$4667/$5B8F/$5A76/$5BA5` in US versus `$4661/$5B8D/$5A74/$5BA3` in JP).
- The JP live-RAM accumulator and property columns are one byte earlier than
  the US fields: `$29A0/$29A4`, `$2980/$2984`, `$2990/$2994`, and `$2A10`
  correspond to JP `$299F/$29A3`, `$297F/$2983`, `$298F/$2993`, and `$2A0F`.
- Category immediates, branch order, arithmetic operations, and the 269-byte
  span length agree across the two authenticated regions.

This raises confidence that the static category arithmetic has a JP
counterpart, while identifying its separate RAM and helper addresses. It does
not prove that either regional routine executed in a gameplay session, that
the source pointer/caller selects these categories, or that the relocated
helpers return the same values. JP runtime category publication, RNG, spawn,
AI, and combat therefore remain closed until a same-session authentic capture
binds caller, helper returns, and the resulting live creature record.

## JP caller and relocated helper byte receipt (2026-10-09)

The focused HuC6280 source receipt now checks two additional spans in the
hash-verified JP raw BIN (MD5 `b7afb338ad31be1025b53f9aff12d73a`):

| JP source listing | Raw BIN offset | Bytes | FNV-1a | Static boundary |
| --- | ---: | ---: | ---: | --- |
| `$C414` caller/preconsumer | `0x9bb94` | 27 | `3d11a727` | Covers `$C414..$C42E` (exclusive end `$C42F`); calls `$C95D` and `$CC3E` |
| `$4661` helper entry | `0x9bbb7` | 25 | `1a732d61` | RTS at `$4679`; branch target `$467a` is outside the span |

The first span is a subrange of the already-locked 150-byte `$C3A0` window
at raw offset `0x9bb20`. Raw byte `0x19` at `0x9bbb6` is the preceding
overlapping instruction's opcode; the `$4661` entry begins at the next byte,
`0x9bbb7`. MAME `unidasm -arch h6280` decoded this overlapping helper entry
identically in three runs.

The JP caller bytes at `$C414` target `$C95D` and `$CC3E`. Two raw windows
0x930 bytes before the already-locked US `$C96B/$CC4C` spans are retained as
static candidates:

| JP target candidate | Raw BIN offset | Bytes | FNV-1a | Repeated listing SHA-256 |
| --- | ---: | ---: | ---: | --- |
| `$C95D` | `0x0a3ebb` | 255 | `063b99e9` | `76ecb69756ce1329fa8be3bb9afa72569b25509b402d2f9d8596eb4c374f3b20` |
| `$CC3E` | `0x0a419c` | 200 | `13a65ea6` | `a689255f19cda570f115a45d3f9cd01232ea3358784942bf385d7f8a7ab292d9` |

Their within-sector positions match the corresponding US windows. The two JP
windows differ from US in 42/255 and 9/200 bytes, respectively. This supports
static regional candidates only; identical offset shift and caller target
operands do not prove runtime bank ownership, mapping, execution, or return
semantics. The `$4661` helper has six identical source copies; another prefix
match at `0x254bb7` uses different JSR operands and is not admitted as the
same helper. `semantic_publication_allowed` remains false, and no JP RNG,
spawn, or combat behavior is enabled by this receipt.
