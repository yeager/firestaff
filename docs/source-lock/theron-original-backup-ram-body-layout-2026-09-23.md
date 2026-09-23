# Theron original Backup RAM body layout — 2026-09-23

This note records a read-only capture from the authenticated US original
runtime. It does not introduce a host save format and does not assign names to
fields whose original consumers remain unidentified.

The instrumented Mednafen build mapped physical System Card RAM page `$6D`
only long enough to verify `DMS-SG.001` at logical `$DE01`, dump all 8 KiB,
and restore the caller's MPR6 mapping. The capture never wrote bytes to the
emulated machine.

- System Card page dump: 8,192 bytes
- MD5: `6b520314faa729149a91556488a421c4`
- Original writer entry: `$DE21`
- Backup RAM destination: `$267C`
- Original body length: `$86` (134) bytes
- Authentic progressed BRAM body FNV-1a: `b37e696e`
- Same-session 8 KiB main-RAM capture MD5:
  `6fcd2ab9d73abee8f1ba3ba2e651343b`

## Complete DMS-SG.001 record

The `$86` bytes above are the writer-owned body inside a larger original
record; they are not the complete file data. The hash-verified US Track 02
file-manager overlay (four MODE1 sectors, MD5
`9aa38417c25bd6d1717081d545fdcfa0`) supplies the complete contract:

- the HUBM record-size word is `$01A9`: 16 header bytes plus `$0199` data
  bytes;
- the argument setup beginning at logical PC `$413D` calls `$E04E` at
  `$415D` to read `$0199` bytes of `DMS-SG.001` to `$5313`;
- `$E051` at logical PC `$41A2` writes `$0199` bytes from `$517A`;
- the data area is three `$88`-byte slots plus one selected-slot byte
  (`3 × 136 + 1 = 409`);
- the common original program reads that last byte at file-data offset `$0198`
  into `$278C`, then executes `LDY $278C; INY` before its `$3800` handoff;
- all seven authenticated dungeon overlays store that one-based Y value at
  `$42B8`;
- the resident `DMS-SG.001` routine indexes the two parallel tables at
  `$DDF9` (`00 88 10`) and `$DDFC` (`00 00 01`) with `$42B8`, producing
  offsets `$0000`, `$0088`, or `$0110`, then calls `$E04E` for `$86` bytes;
- the authentic Akutuba-complete image has data FNV-1a `0ce6b7ba`;
- slot 0 begins with the `$86` writer bytes, followed by two still-opaque
  bytes; slots 1 and 2 are zero and the selected-slot index is 0 in this
  artifact.

Firestaff therefore preserves and exposes all 409 original bytes and selects
only indices 0–2 through the original
`$0198 → $278C → INY → $42B8 → {0000,0088,0110}` route. It does not interpret
the two per-slot tail bytes or any other opaque field until their consumers
are independently proven.

A fresh ordinal-0 run against the authenticated savestate reproduced the
original BRAM MD5 `ffabc8d19b0915d4d9632a7ae2e90a97`. At the same hook point,
the read-only runtime receipt reported `MPR1=f8`, `MPR2=68`, `$278C=00`,
`$42B7=0f`, and `$42B8=01`, matching the zero-based-to-one-based handoff.
The operator-owned stderr receipt is 554 bytes with MD5
`ad742d52c5ffb91a08d7ac554d4c2810`; it is not committed.

The writer at `$DE21..$DED1` constructs the body as follows:

| Body offset | RAM destination | Length | Original source expression |
| ---: | ---: | ---: | --- |
| `$00` | `$267C` | 1 | campaign byte, low seven bits independently proven |
| `$01` | `$267D` | 6 | three pairs from `$2980+Y` and `$2984+Y`, `Y += $10` |
| `$07` | `$2683` | 7 | seven bytes from `$2A10+Y`, `Y += 4` |
| `$0E` | `$268A` | 20 | `$2A2C+Y`, `Y += 4` |
| `$22` | `$269E` | 20 | `$2A7C+Y`, `Y += 4` |
| `$36` | `$26B2` | 20 | `$2ACC+Y`, `Y += 4` |
| `$4A` | `$26C6` | 20 | `$2B1C+Y`, `Y += 4` |
| `$5E` | `$26DA` | 20 | `$2B6C+Y`, `Y += 4` |
| `$72` | `$26EE` | 20 | `$2BBC+Y`, `Y += 4` |

Every byte in the authentic `$86`-byte BRAM body matches the corresponding
source address in that same-session main-RAM capture. The writer resets the
HuC6280 X and Y registers with `CLX`/`CLY` between sections; disassemblers that
treat opcodes `$82` and `$C2` as generic no-ops can obscure those loop bounds.
The address expressions above therefore record the original machine-code
accesses directly, rather than inferred host-side field names.

The access direction is now independently locked as well. After `$E04E`
returns the selected `$86`-byte body at `$267C`, this routine reads only
`$267C`: it merges the new campaign bit there and then overwrites every byte
from `$267D` through `$2701` from the live-RAM sources listed above. It has no
restore read of those 133 bytes. The authenticated 17-sector Stage 2 program
does read all `$0199` bytes to `$7E49`, derives the selected slot pointer as
`$7E49 + $88 × $278C`, and patches its slot-local operations to that pointer.
That establishes record and slot ownership, but not a gameplay restore
meaning for the individual body fields.

The six final arrays in the `$86` writer body are column-major fields for
twenty source records. Their
gameplay meanings are deliberately left unspecified until a separate
load/use consumer is captured. Firestaff may preserve and inspect these
bytes, but production Continue must remain fail-closed for party, inventory,
position and dungeon restoration until those joins are proven.

The checked-in capture hook is
`scripts/mednafen_1.32.1_theron_save_manager_code_dump.patch`. The copyrighted
8 KiB page remains operator-owned data and is not committed to the repository.
