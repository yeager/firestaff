# Theron JP dungeon-state block-transfer sample

An instrumented Mednafen 1.32.1 `pce_fast` replay of an emulator-created JP
Ak-Tu-Ba dungeon state recorded 3,452 main-RAM consumer reads. Four frequently
observed HuC6280 block-transfer instructions have complete source-range
coverage in the bounded sidecar. This is runtime disassembly of a restored
state, not a fresh CD-to-RAM capture, a source-origin receipt, or evidence for
the meaning of any game field.

## Captured transfer windows

| Logical PC | Physical PC | Instruction | Source range | Destination | Length | Bounded rows per source byte |
| ---: | ---: | --- | --- | --- | ---: | ---: |
| `$b9fc` | `$0e19fc` | `TIA` | `$2062-$2081` | alternating `$0002/$0003` | 32 | 16 per source byte |
| `$b985` | `$0e1985` | `TII` | `$2f2c-$2f40` | `$20a7-$20bb` | 21 | 16 per source byte |
| `$a1b7` | `$0e01b7` | `TII` | `$287f-$2892` | `$2883-$2896` | 20 | 16 per source byte |
| `$c1e7` | `$0d21e7` | `TII` | `$2090-$2093` | `$2897-$289a` | 4 | 16 per source byte |

The `TIA` destination alternation and the `TII` forward-copy form follow the
HuC6280 implementation in Mednafen 1.32.1 `src/pce_fast/huc6280.cpp`
(`BMT_TIA` and `BMT_TII`). The original read sidecar records source reads and
instruction windows. A separate, narrowly filtered write sidecar now records
the `$A1B7` transfer's mapped source, destination, prior byte, and written byte;
neither sidecar provides Track 02 origin or higher-level field meaning.

At `$a1b7`, the first contiguous 20-read window observed `$ff $00 $00 $00`
repeated five times across `$287f-$2892`. The post-replay 8 KiB BaseRAM snapshot
matches those 20 bytes (SHA-256
`ca64be33c241c1f73d9ee72c1b977b169a19817a03314ebcde348aab2f38c60a`). The
write replay below shows that the overlapping copy left every observed
destination byte unchanged in this restored state. No pre-transfer snapshot
exists, so the evidence does not establish when the pattern was first written.

## Bounded `$A1B7` destination-write replay

An additional Mednafen 1.32.1 `pce_fast` hook records writes only when the
restored instruction at `$A1B7` still decodes as `TII $287F,$2883,$0014`, the
HuC6280 reports `TII` active, and both source and destination resolve to
BaseRAM. Each row includes the source/destination mappings, BMT iteration,
prior destination byte, and byte written. The hook and verifier are
`scripts/mednafen_1.32.1_theron_pce_fast_a1b7_bmt_write_trace.patch` and
`scripts/verify_theron_jp_a1b7_bmt_write_trace.py`.

Three bounded eight-second replays of the same authentic emulator-created JP
dungeon state each produced 600 rows for 30 complete transfer IDs. All three
sidecars had SHA-256
`2d486957242a6d50c2ccc26b6574f0a2523ddff126b1967cb2e2c976e2313a96`. The
verifier passed on each sidecar. Every write had `old_value == written_value`,
including the four-byte overlap. Thus the traced instruction did not change
these destination bytes in this restored state; this does not establish when
or how those values were first written. The bounded emulator was terminated by
the timeout after each sample, so these are complete write windows, not claims
of a graceful whole-session capture.

The instrumented executable SHA-256 was
`189cb634b808441386f7ad99480f14c443c6b16725d0e28b488ba70966578f54`.
The emulator state, BIOS, game media, and trace sidecars remain private on
TRV2; none are committed.

The capture emits at most 16 rows per physical BaseRAM byte. Every byte in
each listed interval reached that cap, so the sidecar establishes complete
range coverage and at least 16 observed reads per byte—not the total number of
block-transfer executions.

## Reproduction identity and limits

- Authentic JP Track 02 `TQJP02.bin`, SHA-256
  `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`.
- Authentic JP Rev. 1 CUE, SHA-256
  `7bbbe8b077f35b2e9b05c68848f8cb86ea9aa74021a076e882eee8841bc56545`.
- Emulator-created dungeon-state file, SHA-256
  `1231f1cccfeddaf08f519e3c2a72251139cf5012a5cae327db6a594f3adc8f24`.
- Instrumented Mednafen executable, SHA-256
  `e43f7a9d199e1c0d53eb499dbc818bf23984bbfc0fbae7c84a8ecd88f9f8d8de`.
- Main-RAM consumer sidecar, SHA-256
  `532e7c928b2200baf318dfc80dfc8f3eb9b7103dc11236dfc0d25651fc0bf5c9`.

A three-pass run of
`scripts/verify_theron_jp_dungeon_block_transfer_trace.py` confirmed contiguous
sequence numbers, the four decoded operand sets, and 16 bounded rows for every
source byte in each listed range. The sidecar remains private and ephemeral on
TRV2; it is not included in Git.
The capture process did not pass its separate PCE Fast VDC snapshot gate; this
partial sidecar must not be reported as a complete capture. No Track 02 byte
origin was attached to the restored RAM, and no routine names or gameplay
semantics are established here. Continue with a fresh authentic loader
transition that binds source bytes through RAM writes to executing consumers.
