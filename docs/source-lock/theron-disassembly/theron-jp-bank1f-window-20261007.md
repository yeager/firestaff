# Theron's Quest JP bank-$1f disassembly window

This record binds the contiguous HuC6280 window `$2386..$2502` from the
authentic Japanese Rev. 1 ISO projection to the independently decoded US
listing. It adds no game-action or post-CD runtime semantics.

## Source and decoding

- Source: `TQJP19.iso`, authenticated MD5
  `f9f069a5e489b91207f3156059b756f1`.
- CPU window base: file offset `$1f0000` maps to bank `$1f`; the decoded
  span starts at file offset `$1f2386` and HuC6280 address `$2386`.
- Span: `$2386..$2502` (380 bytes), SHA-256
  `ae30eebf78d323122c6b2c99cb7bf2e8c8bbf5a81786059fdfa117fd3b37b956`,
  FNV-1a 32-bit `d5465b33`.
- Disassembler: MAME `unidasm -arch h6280`, run on `trv2` using only the
  extracted span in a task-specific `/dev/shm` directory; the temporary file
  was removed after decoding.
- Comparison source: authentic US `TQUS19.iso`, MD5
  `51b40a17b92a30339957ba564aa0015c`. The same-offset 380-byte US span has
  identical bytes and hashes. Both ISO spans also match their corresponding
  US/JP raw Track 02 BIN projections at bank offsets `$2bb200` and `$2ba8d0`.

The instruction rows and raw bytes are therefore the same as the US source
listing in [theron-us-bank1f-consumer.asm](theron-us-bank1f-consumer.asm),
including its caller at `$2386`, helper entry at `$23ad`, and byte reader at
`$243e`. The shared verifier additionally checks this full contiguous span for
all four authenticated ISO/BIN variants, beyond its existing focused checks.
The focused executable passed three repeated runs on `trv2` with the US ISO
and both raw BINs. Its JP-ISO branch skipped there because the full JP ISO was
not installed on that host; the JP ISO span itself was compared directly
against the other three authentic projections locally.

## Scope limit

This is static bank-$1f evidence only. The bytes do not prove which post-CD
RAM window invokes the code in a gameplay session, and do not identify level,
tile, object, or UI semantics. The `$2600` runtime consumer and its source-LBA
join remain open.
