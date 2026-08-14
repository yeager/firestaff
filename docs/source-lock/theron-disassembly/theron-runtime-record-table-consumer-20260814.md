# Theron runtime record-table consumer (2026-08-14)

## Status

This document contains two distinct witnesses. The original savestate
observation remains a runtime record-table witness, not a Track 02
level/object promotion. A later fresh real-SDL capture adds a direct
source-LBA-to-record-table byte join, verified by
`scripts/verify_theron_record_table_provenance.py`.

## Capture identity

| Input | Identity |
| --- | --- |
| Mednafen savestate | `MDFNSVST`, MD5 `d8ed74a33ac4d770b6cdebcf92b70344` |
| System Card | `syscard3.pce`, MD5 `ff1a674273fe3540ccef576376407d1d` |
| Capture binary | Instrumented Mednafen 1.32.1, with the repository IRQ2 trace patch and a local research-only execution-window logger |
| Execution window | Logical HuC6280 `$C800–$D4FF` |
| Capture result | 200,001 execution rows; no CD-origin receipt in this run |

The local execution-window logger is deliberately not part of Firestaff's
production capture path. Its output is a research artifact on the external
disk and is not a repository input.

## Observed consumer chain

The execution trace repeatedly shows the following source-level operations:

1. `$C9BD` reads `$6000,X`, derives a pointer in `$BC/$BD`, and uses it as a
   record-table base.
2. `$CB89` seeds the table walk from `$292A`, calls the pointer helper, then
   scans records in a 10-byte stride at `$611D` onward until the zero sentinel.
3. `$CBCC` copies the ten bytes at `$2935–$293E` into `$611D–$6126`.
4. `$CC1B`/`$CC27` compare the current record against `$2929–$292D`, while
   `$CC4C` calls the helper at `$4667` for a runtime calculation.
5. `$CA35` and `$CA7E` select/update the current table entry and call the
   surrounding runtime helpers.

The trace therefore proves execution of a mutable runtime record table with
an index table at `$6000` and a ten-byte working record at `$611D`. It does
not yet prove whether those records are dungeon placements, creatures,
inventory, or another runtime table. The `$4667` call is retained as a helper
observation and is not promoted to RNG/object meaning by itself.

## Bounded receipt replay and raw-byte overlap

A second local replay used the checked-in bounded receipt against the same
US savestate. It emitted 4,096 `theron_runtime_record_table` rows from the
`$C9C2/$CA2A/$CB94/$CBCC/$CC0E/$CC27` execution points. The binary MD5 was
`e81a3c8bef98062f8fc95268417a0756`; the savestate MD5 remained
`d8ed74a33ac4d770b6cdebcf92b70344`.

One recurring runtime record, `4080007098a8c8700020`, occurs byte-exactly at
seven offsets in the authenticated US `TQUS02.bin`
(`f23601102138f87c33025877767ebf76`):

```text
0x0b0eed  0x0faa0d  0x144a03  0x18da0d  0x1d7b7d  0x2206ed  0x26a85c
```

This is useful source-byte overlap, not a record-role binding. The replay
autoloads the savestate and reports no CD-origin receipt in that same run, so
it cannot establish that the runtime bytes were produced by those seven
source locations during the captured session. No level, square, object,
creature, inventory, or timer semantics are promoted from this overlap.

## Same-process continuity replay

A fresh real-SDL instrumented process then read the authenticated US medium
before state injection. Its sidecars contain 256 authenticated CD→RAM
receipts, 32 game-owned `$E009` dispatches, and 4,096
`theron_runtime_record_table` rows. The verified Theron state was copied into
the private Mednafen state directory, state slot 6 was selected, and Load
State was issued in that same process. The capture binary MD5 was
`2d84469309f81c582ed59160493fa170`.

This is stronger process continuity than the autoload replay, but it is not a
causal source join. The runtime table appears after explicit savestate
injection; the trace does not bind a Track 02 source LBA to a particular RAM
write and subsequent `$6000`/`$611D` mutation. No level, object, square,
creature, inventory, HUD, T700, or T900 semantic gate is opened by this
receipt.

## Production boundary

The savestate receipt closed a useful execution-window gap but left
`THERON-V1-TRACK02-LIVE-LOADER-CONSUMER` open. The later direct capture now
closes the source-to-RAM-to-record instrumentation join. The remaining
semantic boundary is:

```text
Track 02 sector/user-data bytes
  -> game-owned post-CD RAM destination
  -> executing consumer PC and bank/MPR state
  -> `$6000`/`$611D` record-table mutation
  -> source record/LBA and a reproducible gameplay transaction
```

Until an original semantic consumer and reproducible gameplay transaction are
captured, Firestaff must keep the source-bound raw thing data and all
T900/level/object, VRAM, and draw production gates fail-closed.

## Natural interactive replay receipt

A later fresh real-SDL process booted the authenticated US CUE and reached the
same consumer without savestate autoload. Its sidecars contain 256
`pce_cd_origin_ram_receipt` rows, 32 game-owned `$E009` dispatches and 4,096
`theron_runtime_record_table` rows. The first runtime row contains the known
`$611D` value `4080007098a8c8700020`.

This is a live execution witness after authentic CD activity, but it still has
no `fifo_origin_main_ram_consumer` row and no source-LBA → RAM-write →
`$6000`/`$611D` mutation join. The production gate therefore remains closed.

## Direct source-to-record join

On 2026-08-14, build r25 was run for 120 seconds against the authenticated US
CUE, real System Card, and the external-disk Mednafen profile. The capture
produced 238 authenticated CD-to-RAM receipts and four complete ten-byte
records bound to the runtime table:

| Source LBA | Source offsets | Destination physical | Destination logical | Reader PC | Writer PC |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 4880 | 301–310 | `0x0f811d–0x0f8126` | `$611d–$6126` | `$f406` | `$f427` |
| 4886 | 301–310 | `0x0d011d–0x0d0126` | `$611d–$6126` | `$f406` | `$f427` |
| 4896 | 301–310 | `0x0d011d–0x0d0126` | `$611d–$6126` | `$f406` | `$f427` |
| 4901 | 301–310 | `0x0f811d–0x0f8126` | `$611d–$6126` | `$f406` | `$f427` |

All 40 direct record-byte rows had exactly one matching
`theron_record_watch` write on logical address, physical address, value, and
writer PC. The direct rows are the CD-origin path; `ram_provenance_copies=0`
is therefore expected. The sidecars and raw capture remain local research
artifacts on the external disk.

This closes the instrumentation/provenance boundary:

```text
Track 02 source LBA/offset -> game-owned RAM write -> $611D record mutation
```

It does not prove the record's level, square, object, creature, inventory, or
gameplay role. The production level/object, VRAM, draw, and gameplay gates
therefore remain fail-closed until an original semantic consumer and a
reproducible gameplay transaction are bound.
