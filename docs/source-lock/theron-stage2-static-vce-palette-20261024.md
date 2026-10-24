# Theron's Quest Stage-2 static VCE palette source binding

## Authenticated source bytes

The authenticated US and JP raw Track 02 BINs contain an equivalent palette
routine reached by the Stage-2 dispatch target `$46B8` (opcode index 53). The
handler reads its operand and, on the nonzero branch, calls the regional
routine. The US listing records the handler at
[`theron-us-stage2-huc6280.asm`](theron-disassembly/theron-us-stage2-huc6280.asm)
lines 1003–1010 and the VCE routine at lines 4535–4555.

| Region | Track 02 BIN MD5 | Stage-2 start | Routine address / raw offset | Payload address / raw offset |
| --- | --- | ---: | --- | --- |
| US | `f23601102138f87c33025877767ebf76` | `0x2bed90` | `$5E4D` / `0x2c0f6d` | `$5E5F` / `0x2c0f7f` |
| JP Rev. 1 | `b7afb338ad31be1025b53f9aff12d73a` | `0x2be460` | `$5E7D` / `0x2c066d` | `$5E8F` / `0x2c067f` |

The physical raw offsets account for MODE1/2352 framing: for a CPU address
relative to `$4000`, add one 2352-byte raw sector for every 2048 user bytes
and preserve the within-sector user-data offset. A plain raw-file addition
across sector boundaries produces the wrong result. This correct mapping
places the routine exactly at its regional call target.

Both routines select VCE index `$00E0` by writing `$E0` to `$0402` and `$00`
to `$0403`, then execute `TIA` for 64 bytes into `$0404`. US source bytes name
payload `$5E5F`; JP names `$5E8F`. The 64-byte payload is byte-identical in
both regions (SHA-256
`6cb478db1e723a04d8dd46a14c4a669f2263c6a61404b3c536168693e82c8913`). It
decodes as 32 legal HuC6260 9-bit color words, entries `$E0..$FF`.

## Scope

This binds an authentic, statically present palette routine and its VCE
destination contract in both regions. It resolves the earlier apparent body
mismatch, which came from treating the raw MODE1/2352 BIN as a contiguous
2048-byte stream. Static bytes alone do not prove that a particular title,
Soul Room, forcefield, or dungeon command stream invokes this routine, nor do
they bind any pixels or a screen layout to the palette. Do not use this
palette alone to admit a native frame. Firestaff continues to run Theron in
its native C/SDL runtime; this note is source and original-media analysis,
not an emulator gameplay capture.
