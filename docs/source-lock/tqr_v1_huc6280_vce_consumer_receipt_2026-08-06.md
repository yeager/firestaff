# Theron's Quest HuC6280 VCE consumer receipt

The authenticated raw Track 02 BINs contain the same HuC6280 VCE consumer
span at the regional bank-window offset plus `0x9e15`:

| Variant | Raw file offset | HuC6280 address | Bytes | Span FNV-1a |
| --- | ---: | ---: | ---: | ---: |
| US `TQUS02.bin` | `0x2c5015` | `$96a5` | 37 | `ff51fac4` |
| JP `TQJP02.bin` | `0x2c46e5` | `$96a5` | 37 | `ff51fac4` |

The span begins with the source-owned writes of the VCE index registers
`$27c2/$27c3` to HuC6260 registers `$0402/$0403`, adjusts the dynamic source
pointer in `$27c4/$27c5`, and invokes the inline `TIA ...,$0404,$0020`
transfer helper at `L96c2`. The bytes are verified against the authenticated
whole-file MD5 and the regional bank offsets in
`theron_v1_huc6280_disassembly.c`.

This is a static consumer-contract receipt only. `$27c4/$27c5` is populated at
runtime from a dynamic pointer, so the receipt does not join the consumer to
the known `0x2a06a0` US or `0x29fd70` JP palette-shaped Track 02 spans. It
therefore does not authorize palette, bitmap, HUD, tile, viewport, or dungeon
rendering. Those routes remain blocked until an authenticated execution trace
provides the source-LBA/FIFO and VCE/VDC destination join.

References: `docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm`
(`L96a5`, `L96c2`), HuC6260/HuC6270 hardware format notes, DMWeb Theron's
Quest edition provenance, and the Greatstone extraction methodology recorded
in `docs/DMWEB_REFERENCE.md`.
