# Theron US Stage-2 `$4B3C` scroll-state initializer

This note records the 116-byte US Track 02 user-data span `[0x4b3c, 0x4bb0)`.
Offsets refer to the Stage-2 record's user bytes. The following `$4BB0` routine
uses several state fields initialized here.

## Authentic source binding

The source image is the hash-verified original US Track 02 BIN:

| Edition | Track 02 SHA-256 | Raw sector | In-sector offset | Span raw offset |
| --- | --- | ---: | ---: | ---: |
| US | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | 1233 | 844 | 2,900,860 |

The span SHA-256 is
`9da2cde29dc484b69b01bd836b3368eb87b4ffa13ef69368c706c640385d25df`; its
FNV-1a-64 is `b089a651068a5ce7`. The 116 bytes are byte-identical in the
authenticated JP Track 02 image, whose span raw offset is 2,898,508. The full
US linear Stage-2 listing is `theron-us-stage2-huc6280.asm`; the extracted
window is `theron-us-stage2-l4b3c-huc6280.asm`.

## Instruction-level observations

The routine saves `$220C/$220D/$2210/$2211` to `$4C12..$4C15`, then branches
on `$0E` and `$10` to clear, restore, or initialize the `$4C0D/$4C0E` signed
delta state. It writes `$12` to both `$4C0F` and `$4C10`, restores processor
flags, and returns. The exact source bytes and selected boundaries are checked
against the original US image by the focused Stage-2 disassembly test.

This is static source evidence only. It does not prove the initializer or its
adjacent `$4BB0` routine executes, nor does it establish rendered scroll
parity. The enclosing-callee verifier remains US-only.
