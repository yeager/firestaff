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
- slot 0 begins with the `$86` writer bytes, followed by two transport-padding
  bytes; slots 1 and 2 are zero and the selected-slot index is 0 in this
  artifact.

Firestaff therefore preserves and exposes all 409 original bytes and selects
only indices 0–2 through the original
`$0198 → $278C → INY → $42B8 → {0000,0088,0110}` route. It does not interpret
the two per-slot tail bytes as gameplay state. The independently byte-bound
US and JP Stage 2 routines read `$88` bytes when transporting one selected
slot, then initialize a slot by writing zero to its first byte and using an
overlapping `TII` of length `$87` to clear the remaining bytes. The dungeon
handoff stores only the selected-slot ordinal; `DMS-SG.001` subsequently
copies exactly `$86` bytes into `$267C..$2701`. No tail byte reaches the
gameplay restore buffer. The final two bytes are therefore transport padding,
not an unresolved Continue field.

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

The separate restore consumer is now byte-bound in all seven authenticated US
dungeon blocks. Each copy begins at block offset `$0248` (logical `$2248`):
it rejects a zero campaign byte or a masked value of seven or more, then copies
the six-byte section to `$2978/$2980` and `$297C/$2984`, the seven-byte section
to `$29F4/$2A10`, and the six 20-byte columns to `$2A2C`, `$2A7C`, `$2ACC`,
`$2B1C`, `$2B6C` and `$2BBC`. This is the exact inverse data direction missing
from the writer analysis, including the duplicate destinations in the first
two sections.

The corresponding restore consumer is now independently byte-bound against
the authenticated JP Track 02 as well. Its seven identical copies begin at
user-data offsets `$081A45`, `$0C1A45`, `$101A45`, `$141A45`, `$181A45`,
`$1C1A45` and `$201A45` (sector-local offset `$0245` in sectors 259, 387,
515, 643, 771, 899 and 1027). The control flow and `$267C..$2701` inputs are
the same as in the US routine. Every JP live-RAM destination is one byte below
its US counterpart: `$2977/$297F`, `$297B/$2983`, `$29F3/$2A0F`, `$2A2B`,
`$2A7B`, `$2ACB`, `$2B1B`, `$2B6B` and `$2BBB`. This regional difference is
verified directly rather than inferred from US media.

The six final arrays are therefore proven column-major restore inputs for
twenty live records in both regions. Their gameplay meanings are still
identified by the downstream arithmetic consumers in every regional dungeon
copy:

- `$267D..$2682` contains Theron's maximum health, stamina and mana as three
  little-endian 16-bit values. The restore routine writes each value to both
  the current and maximum live fields. A later clamp compares each current
  field with its maximum and copies the maximum back when current is greater.
  The authentic progressed body decodes to 175, 1500 and 50, exactly matching
  Theron's hash-bound regional roster record.
- `$2683..$2689` contains Theron's seven maximum attributes in roster order:
  luck, strength, dexterity, wisdom, vitality, anti-magic and anti-fire. The
  restore routine initializes both current and maximum attribute columns. A
  seven-iteration consumer compares the two columns with a four-byte stride.
  The authentic values `80, 50, 40, 40, 45, 40, 45` match the regional roster.
- `$268A/$269E` are the low/high columns of a 16-bit temporary-experience
  value for each of 20 skill ordinals. `$26B2/$26C6/$26DA/$26EE` are the four
  little-endian columns of the corresponding 32-bit persistent experience.
  The original level consumer loads the four persistent columns, adds the two
  temporary columns with carry and evaluates the result. Separate consumers
  update the 16-bit and 32-bit values with carry propagation.

The classifier now exposes this typed projection alongside the unchanged raw
address projection. Its real-artifact regression requires the exact restore,
clamp, attribute-compare and skill-experience consumer bytes in all seven US
and all seven JP dungeon copies, plus each region's `$88`-byte Stage 2 slot
transport and clear routine. The native transactional restore now applies the
campaign byte, initializes Theron's current and maximum vitals from the saved
maxima, restores the seven attributes and preserves all 20 temporary and
persistent skill-experience pairs. It requires an authenticated roster-owned
`THERON` in party slot zero and leaves companions, inventory, equipment,
position and loaded dungeon media unchanged on both success and rejection.
Production Continue remains fail-closed until startup discovery and the
explicit Continue action select this original Backup RAM route.

The checked-in capture hook is
`scripts/mednafen_1.32.1_theron_save_manager_code_dump.patch`. The copyrighted
8 KiB page remains operator-owned data and is not committed to the repository.
