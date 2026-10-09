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

The immediate caller begins at HuC6280 `$966e` and is byte-identical in
authenticated US and JP Track 02 media. It occurs at raw BIN offsets
`0x2c4fde` (US) and `0x2c46ae` (JP),
and exactly once at cooked Track 02 ISO file offset `0x1f8e6e` in each
hash-verified MODE1/2048 image. The 23-byte sequence is:

```text
c8 b1 62 8d c4 27 c8 b1 62 8d c5 27 c8 b1 62 8d c6 27 44 23 a9 04 60
```

Its FNV-1a is `b3b3ccbb` and SHA-256 is
`9eeb4bfe631d9fd7831f1876b5f6bd665ee12bc7d52bb2d574f2a5c16dbd544f` in both
BINs; the bytes match exactly in both cooked ISO files. The listing decodes it
as three indexed reads through `($62),y`
that populate `$27c4`, `$27c5`, and `$27c6`, followed by `BSR L96A5`.
The bounded listing places this sequence immediately after `L966D: RTS`; its
last three bytes at `$9682..$9684` are `LDA #$04; RTS`. Thus `$9682` is not
the caller entry. The caller-to-consumer address delta (`$966e` to `$96a5`)
matches its raw BIN file-offset delta in both regional images. This binds the
consumer's immediate caller and the descriptor-relative source of those
bytes, but not the runtime provenance or value of `$62/$63`. The
stage-two listing has distinct setup routes: `L4995` copies the resolved
`$442f/$4430` pointer into `$27c0/$27c1` and `$62/$63`, while a separate
startup/runtime path copies `$37ce/$37cf` into those pairs. The listing does
not establish which route supplies the caller's `$62/$63` at the VCE call.

This is a static consumer-contract receipt only. `$27c4/$27c5` is populated by
a descriptor-relative read through `$62/$63`, so the receipt does not join the
consumer to the known `0x2a06a0` US or `0x29fd70` JP palette-shaped Track 02
spans. It therefore does not authorize palette, bitmap, HUD, tile, viewport,
or dungeon rendering. Those routes remain blocked until an authenticated execution trace
provides the source-LBA/FIFO and VCE/VDC destination join.

References: `docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm`
(`L966D..L96A5`, lines 12633-12666, and `L96c2`), HuC6260/HuC6270 hardware
format notes, DMWeb Theron's Quest edition provenance, and the Greatstone
extraction methodology recorded in `docs/DMWEB_REFERENCE.md`.
