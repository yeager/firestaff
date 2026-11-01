# Theron T900 ground-item index candidates (2026-11-01)

## Finding

The existing authentic Track 02 quest-name test now enumerates ground-linked
carryable records whose category-specific `type` byte equals that dungeon's
quest-name table index. It reports category, source reference, source table
index, map cell, user-data offset and raw Track 02 offset. Each reported table
offset is checked against the exact record bytes decoded from the loaded
authentic user-data stream.

This is a capture shortlist only. A matching type index does not identify a
quest artifact or prove that original T900 reads, picks up, equips, consumes or
records the item. In particular, matching indexes can belong to unrelated
categories, and records that are not linked to a ground cell are not shown as
placements.

## Authenticated input and reproduction

| Edition | File size | MD5 |
|---|---:|---|
| US Track 02 | 8,104,992 | `f23601102138f87c33025877767ebf76` |
| JP Rev. 1 Track 02 | 8,102,640 | `b7afb338ad31be1025b53f9aff12d73a` |

The three direct runs used these exact local media identities and produced
byte-identical output. Reproduce with the real-data regression:

```sh
cmake --build build --target test_theron_v1_track02_quest_item_names
ctest --test-dir build -R '^theron_v1_track02_quest_item_names$' \
  --repeat until-fail:3 --output-on-failure
```

The source test walks authenticated map `has_things` cells in map/x/y order,
follows the corresponding real ground-reference chains and compares each
record's type index with the existing regional quest-name index. The raw-file
offset is derived from the extracted user-data offset as
`(user_offset / 2048) * 2352 + 16 + (user_offset % 2048)`.

## Matching ground-linked records

| Dungeon | Name-table index | Category | Source reference / item index | Map cell | US raw offset | JP raw offset |
|---:|---:|---:|---|---|---:|---:|
| 3 | 45 | 10 (misc) | `$280A` / 10 | 0: `(3,3)` | `$147364` | `$146A34` |
| 3 | 45 | 10 (misc) | `$6800` / 0 | 1: `(1,7)` | `$14733C` | `$146A0C` |
| 6 | 43 | 10 (misc) | `$680C` / 12 | 2: `(13,2)` | `$223750` | `$222E20` |
| 7 | 7 | 6 (clothing) | `$D833` / 51 | 0: `(4,0)` | `$26D02C` | `$26C6FC` |
| 7 | 7 | 6 (clothing) | `$D819` / 25 | 1: `(5,5)` | `$26CFC4` | `$26C694` |

The other dungeon indexes have no matching ground-linked records in either
region: dungeon 1 index 41, dungeon 2 index 63, dungeon 4 index 43 and dungeon
5 index 44. Some of those indexes do match records in the category tables,
but those records are not reached from a map cell's ground-reference chain.
US and JP have the same five ground-linked locations; regional record offsets
differ.

## Next capture boundary

The locations can guide a future authentic item interaction capture if a
reachable original-game route is available. Retain instruction-start PC,
fetched bytes and MPRs alongside same-session CD/FIFO source receipts and
reads/writes in `$2600–$27FF`. Pair an accepted item interaction with a
no-input control and a reproducible inventory/equip/use/drop transaction.
Do not introduce a synthetic save or change native quest/inventory semantics
from this static list. No T900 caller PC or quest-state relation is established
by this report.

## Related runtime record-table evidence

The repository also contains a separate authentic US source-to-runtime join:
Track 02 CD reads are joined to writes of ten-byte records at `$611D`, with an
executing `$C3A0` caller window. The capture did not classify those records as
items, and a separate interactive attempt produced zero valid `$B0E5`
categories. Use the [runtime record-table report](theron-disassembly/theron-runtime-record-table-consumer-20260814.md)
and its [US caller disassembly](theron-disassembly/theron-us-c3a0-record-consumer.asm)
as a source-led disassembly starting point only; neither binds these five
ground candidates to that table or proves T900 behavior. A same-session join
from a candidate ground reference through the executing record consumer and
an observed pickup/state transaction is still required.
