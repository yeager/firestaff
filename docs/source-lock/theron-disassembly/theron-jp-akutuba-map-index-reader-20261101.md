# JP Akutuba candidate map-index reader (2026-11-01)

## Evidence identity

This analysis uses the authenticated Japanese Rev. 1 Track 02 BIN (MD5
`b7afb338ad31be1025b53f9aff12d73a`) and an existing Mednafen 1.32.1 JP
save state (MD5 `8bf0caf3a98b5748d2a78ba8016d2f01`; SHA-256
`b4f2a06788e4723110b19892f93e8ac4aef1b5c18c6358a9fe36f1baf747b068`). The
state's `HuC.ROMSpace` bank `$68` contains the code window at CPU `$50D2`;
bank `$74` contains the observed data reads and the authentic map-source
bytes. The state and raw traces remain outside Git. No synthetic game data was
used.

## Reader disassembly and source match

The 256-byte state window beginning at CPU `$50D2` has SHA-256
`5dc329642bb2a090c6ccd92c067bc68b950d7ad26e7592a46ca5b9501fb7ab5f`. It
occurs once in the authenticated raw BIN at byte offset `$95ED2`.

```asm
50D2: B1 2A       LDA ($2A),Y
50D4: AA          TAX
50D5: C8          INY
50D6: B1 2A       LDA ($2A),Y
50D8: A8          TAY
50D9: 8A          TXA
50DA: 18          CLC
50DB: 65 33       ADC $33
50DD: AA          TAX
50DE: 90 01       BCC $50E1
50E0: C8          INY
50E1: A5 2C       LDA $2C
50E3: 42          SAY
50E4: 20 A7 4F    JSR $4FA7
50E7: 86 14       STX $14
50E9: 85 15       STA $15
50EB: 98          TYA
50EC: 20 E7 44    JSR $44E7
```

The visible instructions read a little-endian word through zero-page `$2A`,
add zero-page `$33` to its low byte with carry into the high-byte index, load
`$2C`, exchange A/Y with `SAY`, and call `$4FA7`. This describes the
instruction sequence, not the routine's full semantics or its caller contract.

## Pointer-like bytes before the map arrays

The authentic 114-byte span at logical `$6A91..$6B02` (raw BIN `$B1061`)
has SHA-256
`30fd48219520053e4588aecc357bd211a6c3a3f3cb217dfb40c41544426431db` and
occurs once in the raw BIN. The source parser places this span at the tail of
the 272-byte `thing_list` region, immediately before the map arrays. Its
internal field semantics are not identified. The header words are
`$6A99, $6AA3, $6AC7, $6AEB`; they point to four lists of 16-bit values:

| List address | Entry count | Initial values |
|---|---:|---|
| `$6A99` | 5 | `$6B03, $6B0A, $6B11, $6B18, $6B1F` |
| `$6AA3` | 18 | `$6B26, $6B32, $6B3E, ...` |
| `$6AC7` | 18 | `$6BFE, $6C0E, $6C1E, ...` |
| `$6AEB` | 12 | `$6D1E, $6D29, $6D34, ...` |

The list lengths match the four Akutuba descriptor `x_dim` bytes
`[5, 18, 18, 12]`; values advance by the corresponding `y_dim` bytes
`[7, 12, 16, 11]` and fall inside the aggregate map-data address span. These
relationships make the bytes a candidate index or locator structure. They do
not make the listed addresses map bases or tile-column starts: the actual
arrays begin at `$6B03`, `$6B33`, `$6C2A`, and `$6D6D`, with lengths 48, 247,
323, and 156 bytes respectively. The parser's dimensions are stored as
width-minus-one and height-minus-one; its documented array sizes use
`(x_dim + 1) * (y_dim + 1)`. Do not reinterpret the current map boundaries
from the pointer-like lists alone.

## Bounded runtime observation

In an autoloaded state-only reference trace, the movement-context sidecar
records execution at `$50D2/$50D6` reading logical `$6A9D/$6A9E` through MPR3
`$74`, with values `$11,$6B`. Those bytes are the third 16-bit value in the
first pointer-like list, and the containing 114-byte source span is unique in
the authentic BIN. The pair by itself occurs 17 times. The trace proves those
instruction PCs fetched those bytes; it does not capture Y, A, X, zero-page
`$2A/$33`, or the subsequent effective read at `$4FA7`.

The autoload state's initial BaseRAM has `$2A/$2B=$6A99` and `$33=3`, while
its close-time snapshot has `$2A/$2B=$6A99` and `$33=4`. Neither snapshot is a
same-event register record. Therefore it is not established that `$6A99` was
the pointer base or that either `$33` value applied during the observed read.
The state-only UP/control traces also report `transition=missing` and do not
establish a cold CD handoff, movement, an active map, a consumed tile, or a
rendered response. See the [matched input/data-read report](theron-jp-state-only-up-control-data-read-20261101.md).

## Evidence boundary and next capture

The source table and code bytes are authentic and uniquely matched as stated
above. The relation between the reader, the pointer-like `thing_list` tail,
and a map cell remains a candidate. A follow-up reference trace should record
CPU registers and zero-page `$2A..$33` at `$50D2`, the resolved address and
registers at `$50E4`, subsequent reads within the real map spans, and any
`$4FA7` consumer in shared event order. To support rendering, join that trace
to input receipt, active-map/pose evidence, and atomic VDC/VCE/VRAM state. Until
then, keep the list and fields opaque and do not use them to select or render
a native map.

The source-map report records the authenticated descriptor and true array
boundaries: [JP Akutuba source-sector report](theron-jp-akutuba-map-source-sector-20261101.md).
