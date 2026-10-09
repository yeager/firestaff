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
identically in three runs. Its listing is retained in
`theron-disassembly/theron-jp-4661-rng-helper.asm`. The matching US `$4667`
entry has the same 25-byte instruction shape, with relocated branch and
`$5D6A/$5D64` call operands in place of JP `$467A` and `$5D68/$5D62`. This
static correspondence does not establish either region's runtime bank mapping,
caller selection, or helper return values.

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
same helper. Complete MAME HuC6280 listings for both JP candidate windows are
retained in `theron-disassembly/theron-jp-c95d-spawn-target-candidate-20261009.asm`
and `theron-disassembly/theron-jp-cc3e-spawn-target-candidate-20261009.asm`.
The listings preserve the exact instruction boundaries—including the initial
`ill $BB` at `$CC3E` and RTS boundary at `$CC5C`—without claiming runtime
reachability. `semantic_publication_allowed` remains false, and no JP RNG,
spawn, or combat behavior is enabled by this receipt.

## Bounded JP/US target-candidate decode comparison (2026-10-09)

On `trv2`, MAME 0.285 `unidasm -arch h6280` was run three times over each of
the four authenticated 255-byte/200-byte candidate windows. Each listing was
identical across its three runs. The JP listing SHA-256 values matched the
receipts above; the corresponding US listings hash to
`0efb0a80f5efe80b896590d2d1676d2918aab2797b42ac1217833437a93687b8` for
`$C96B` and
`7674bf8f304d44163d39c367765795641bb9243242f55422d15d062eeb5b8696` for
`$CC4C`.

The linear `$C95D/$C96B` decodes contain 114/113 instructions. Their first
two instruction mnemonics agree (`PHA`, `JSR`), but the third differs:
JP decodes `CPX #$6D`, while US decodes `INC $686D`. Their first decoded RTS
instructions occur at relative offsets `$BE` and `$FE`, respectively. These
are disassembly boundaries, not positions found by a raw `$60` byte search.
Since each window continues beyond its first RTS, these counts and boundaries
describe the bounded byte windows, not complete routine lengths.

The `$CC3E/$CC4C` decodes each reach their first RTS at relative offset `$1E`.
The 31-byte prefixes through that RTS have 17 identical decoded instruction
mnemonics; only relative byte offsets `$05` and `$14` differ. Those are
operands in the indexed table read and helper call. Both starts decode as
`ill $BB`, so this matching linear decode does not establish executable
semantics. After the RTS, the windows diverge: at relative offset `$45`, JP
decodes `SMB3 $52` and US decodes `DEX`; subsequent instruction boundaries
also differ. The windows are therefore not interchangeable as whole spans.

This narrows the static comparison: the early `$CC3E` prefix has a bounded
operand-relocated shape in common with `$CC4C`, while `$C95D` differs from
`$C96B` almost immediately. Neither finding binds a JP bank, proves runtime
reachability, or identifies spawn/RNG meaning. Keep JP behavior closed until
a same-session capture binds the caller and both mapped return edges.

An exact byte search of each authenticated BIN also found six copies of the
191-byte JP `$C95D` prefix through its first RTS and six copies of the
255-byte US `$C96B` prefix through its first RTS. In each region those copies
are spaced by `$49800` raw bytes (128 2352-byte sectors). The 31-byte
`$CC3E/$CC4C` prefixes through RTS occur five times in JP and six times in
US; their first copies after the source-locked windows are `$93000` bytes
(256 sectors) later, followed by `$49800`-byte spacing. These are source-file
occurrence positions only. Repetition and sector spacing do not identify the
runtime bank, caller choice, or a gameplay role.
