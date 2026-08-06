# Theron's Quest — regional level-descriptor receipt

## Authenticated result

The logical Track 02 user-data span at `0x619900` is exactly 53 records of
six bytes (`318` bytes) in the supplied US BIN. Its FNV-1a-32 is
`0x7AA82BC7`. The source identity is the known US Track 02 MD5
`f23601102138f87c33025877767ebf76`.

The supplied JP Track 02 BIN has the same logical span, but all 318 bytes are
zero. Its FNV-1a-32 is `0x63D8DDFD`. The receipt reports this as `ZERO_FILL`;
it never interprets the JP bytes through the US descriptor table.

The focused `test_theron_v1_level_descriptor` test reconstructs the logical
2048-byte user-data stream from both authentic MODE1/2352 BINs and verifies
both regional outcomes. The receipt is transport/layout evidence only. It
does not assign the 53 records to map squares, objects, tiles, palettes or a
runtime consumer.

## Source boundary

The table is retained as a real-media receipt because it is a stronger input
than a generated descriptor fixture. The current code still stops before
decoding the referenced payloads: that requires the executing HuC6280 command
and source-LBA join described in the stage-2 and FIFO-origin disassembly
receipts.

Greatstone/DMWeb are used as format cross-checks for the wider Dungeon Master
family, but neither reference supplies a source-owned PC Engine Track 02
consumer for this table. No semantics are inferred from a matching byte
shape.
