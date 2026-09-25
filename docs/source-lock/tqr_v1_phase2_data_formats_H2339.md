# Theron's Quest V1 Phase 2 — Data Formats: Source-Lock Document

**Cron task:** `Theron_V1_Phase2_DataFormats_0527`
**Status:** PARTIAL — authenticated Track 02 record bindings are documented; unresolved
formats remain explicitly gated and are not runtime semantics.
**Author:** Firestaff agent (cron)
**Last revised:** 2026-05-27T06:17 UTC+2

---

## Scope

Source-lock every Theron's Quest V1 data format before converting into Firestaff
C structures. Applies to the PC Engine CD-ROM² / TurboGrafx-16 CD release
(JP: TGXCD1042, 1992-09-18; US: TGXCD1041, 1993). Covers endianness, byte
layout, provenance reference, and known gaps per data category.

**Provenance gate:** Phase 0 (`tqr_v1_phase0_provenance_gate_H2339.md`) passed.
CD-ROM Track 02 BIN hashes locked:
- JP: `b7afb338ad31be1025b53f9aff12d73a` (cdromance.org)
- US: `f23601102138f87c33025877767ebf76` (cdromance.org)

**Data policy:** DM1, CSB, and Nexus structures are not assumed to describe
Theron. Only data verified against authenticated Theron media or a source-owned
consumer is promoted; all other boundaries remain explicitly unresolved.

**Source-lock rule:** Every promoted format claim cites authenticated Theron
media or a source-owned consumer. Hash-verified Track 02 and Track 19 evidence
is used for the records described below. Other data remains explicitly
unresolved and is not promoted to runtime data.

---

## 1. Platform & Binary Structure

### 1.1 CPU / Endianness

| Property | Value |
|----------|-------|
| CPU | HuC6280 @ 7.16 MHz (65C02 derivative, 8-bit bus) |
| Endianness | **Little-endian** (65C02 is little-endian) |
| Word size | 8-bit bytes; 16-bit words stored low-byte first |
| Alignments | No alignment restrictions; HuC6280 allows unaligned access |

The authenticated map decoder uses one-byte tile records (§2.3). The CPU
platform is documented here for context only; its general byte order does not
prove the representation of every multi-byte Track 02 field.

Source: Phase 0 provenance gate §5.1 and HuC6280 platform reference.

### 1.2 Track 02 Binary Layout (CD-ROM Data Track)

Track 02 is the authenticated data-track input used by the parser. Its full
executable/data memory map and graphics/audio section boundaries are not
established by this source-lock. Do not infer a linear layout or section
offsets from the host's extracted user-data view.

**Current evidence:** The authenticated US and JP Track 02 inputs now provide
seven source-bound map groups, object-count tables, ground-reference tables,
door/teleporter records, creature-graphics-bank fields, and item-record byte
spans. These are retained as raw/decoded records. They do not yet prove the
HuC6280 consumer handoff, tile/material ownership, dungeon-square rendering,
palette ownership, or JP text ownership. Do not infer those semantics from the
startup candidate.

Source: authenticated US/JP Track 02 inputs and current source-bound parsers.

### 1.3 Track 02 input identity

Use the exact hash-verified Track 02 input identities recorded in the
provenance manifest. Disc-image size is not a Track 02 data-format property;
no estimate is used to infer its layout.

---

## 2. Dungeon Format

### 2.1 Dungeon Block Header

The authenticated loader recognizes seven dungeon map groups in each real
Track 02 variant. The current source decoder binds their map counts to
`{4, 8, 5, 6, 3, 4, 4}` and retains each map header and source span. This is a
map-group index, not evidence for the historical dungeon header struct or for
a per-dungeon random-seed table. The verified US/JP startup candidate carries
the 32-bit initial-level seed `0x0108e938`; later seed semantics remain
unresolved.

Source: `src/theron/theron_v1_track02_dungeon_map.c`,
`src/theron/theron_v1_world.c:theron_v1_world_load_track02_dungeon`, and the
hash-verified US/JP Track 02 inputs.

### 2.2 Dungeon Grid

Authentic Track 02 map headers store maximum X/Y indices, so the decoded
dimensions are each stored index plus one. Dimensions vary by map; there is no
single fixed grid size. Each tile occupies one packed byte, not a two-byte
DM1-style cell. The seven authentic groups contain 34 maps in total, using
the loader's group counts `{4, 8, 5, 6, 3, 4, 4}`. These map counts are not a
claim about campaign floor count or reachable-level topology.

The map decoder retains offsets, map IDs, unknown header bytes, creature
count, XP modifier, door types and tile bytes. A complete execution-side
binding for every header field, tile attribute and reachable-level graph is
not established. DM1/Nexus cell layouts and display-resolution arguments do
not define Theron data.

Source: `include/theron_v1_track02_dungeon_map.h`,
`src/theron/theron_v1_track02_dungeon_map.c`, and hash-verified US/JP Track 02
inputs.

### 2.3 Square Type Table

The authenticated Track 02 map parser reads one packed byte per tile. Its
layout is confirmed by the real US and JP Track 02 map groups and the retained
source-byte ledger:

| Bits | Meaning | Evidence/status |
|------|---------|-----------------|
| 7–5 | Tile family: 0 wall, 1 open, 2 pit, 3 stairs, 4 door, 5 teleporter, 6 fake wall, 7 type-7 | `Theron_TileType` in `include/theron_v1_track02_dungeon_map.h`; family values decoded from authentic Track 02 bytes |
| 4 | Tile has a linked thing list | Exposed by `theron_tile_has_things`; raw value retained with each tile |
| 3–0 | Tile attributes | Exposed by `theron_tile_attributes`; meaning remains unresolved and must not be treated as stair direction, destination, or gameplay state without a source consumer |

Map headers retain dimensions, offsets, map ID, two unknown bytes, creature
count, XP modifier, and door-type fields. Dimensions are stored as maximum
indices; runtime width and height are each the stored value plus one. These
fields and packed tile bytes are decoded from the seven real dungeon groups by
`theron_v1_track02_dungeon_map_load_for_variant`; they are not generated map
content.

**Still unresolved:** the low-nibble attributes' consumer semantics, including
stair direction and destination rules. Authentic Akutuba maps contain multiple
stairs with distinct attribute values, while the destination level need not
contain a stair at the same tile. The host transition therefore remains gated
until the original consumer or an authenticated runtime capture binds those
fields. DM1's `MASK0x0004_STAIRS_UP` and
`F0154_DUNGEON_GetLocationAfterLevelChange` are research leads, not proof that
Theron uses the same packed attribute or transition contract.

Source: `include/theron_v1_track02_dungeon_map.h`,
`src/theron/theron_v1_track02_dungeon_map.c`,
`src/theron/theron_v1_world.c:theron_v1_world_load_track02_dungeon`, and the
hash-verified JP/US Track 02 inputs.

### 2.4 Object Placement Records

Track 02 placement data is retained as authentic source occurrences, not as a
DM1-style synthetic `pos_x/pos_y/object_id/attributes` record. The current
decoder keeps the source reference, next reference, category, source index,
position byte, raw record size and bytes, map, and coordinates. Category
descriptor sizes and linked-list traversal come from the selected real
dungeon block. Unrecognized fields remain raw; they are not assigned host
inventory or object meanings.

The source-owned property table and the separate 69-row item-name/type tables
are independently authenticated. A subset of known records can be decoded,
but placement and item semantics are published only behind their own verified
consumer bindings. Remaining raw occurrence counts describe the source stream;
they are not a count of supported gameplay objects.

Source: `include/theron_v1_track02_dungeon_loader.h`,
`src/theron/theron_v1_track02_dungeon_loader.c`,
`src/theron/theron_v1_track02_item_name_source.c`, and the real JP/US Track 02
inputs. See also the source-lock entries for the Track 19 property and name
tables.

---

## 3. Item / Object Format

### 3.1 Item Type Subset

The real US and JP Track 19 images contain source-owned 69-entry item-name
and type-code tables. Their exact offsets and hashes are checked by the
inventory reader; Japanese names remain raw Shift-JIS bytes. The authenticated
property table contains 66 six-byte records. These are concrete table sizes,
not estimates of the number of distinct retail item categories.

**Still unresolved:** a complete mapping from every source type code and
property row to player-visible item names, shop behavior, and runtime object
semantics. No DM1 category counts or guessed TQ subset counts are used as
gameplay data. Quest-item names and their dungeon associations must be
published only where the authentic Track 02 / Track 19 record binding proves
them.

Source: `include/theron_v1_track19_inventory.h`,
`include/theron_v1_track19_item_names.h`,
`src/theron/theron_v1_track19_inventory.c`, and the hash-verified US/JP
Track 19 images.

### 3.2 Object Record Format

Track 02 object bytes use the source-occurrence model described in §2.4.
Their raw category records and references are retained, but fields whose
consumer has not been authenticated are not reinterpreted as DM1 charge,
curse, or used-bit fields. The decoder exposes category-specific raw spans;
only independently proven tables and consumers may promote a record into a
runtime item or object.

Source: `include/theron_v1_track02_dungeon_loader.h` and
`src/theron/theron_v1_track02_dungeon_loader.c`.

### 3.3 Item Icon / Sprite Mapping

The retail icon-index mapping and item-sprite consumer have not been proven
from the authenticated Theron data. DM1 icon constants and tile counts are
reference material only; no DM1 icon ID or synthesized sprite is treated as a
Theron asset. Keep item imagery unavailable until a Track 02/graphics capture
binds the source indices and renderer consumer.

Source boundary: authenticated Theron Track 02 maps and Track 19 item tables;
the graphics index/consumer mapping remains unresolved.

---

## 4. Text Format

### 4.1 Encoding

| Property | Value |
|----------|-------|
| Languages | English (US), Japanese (JP) |
| Encoding | Candidate source encoding not yet tied to an executing consumer |
| Text storage | Authenticated candidate records only; interpretation unresolved |
| UI text | Track 02 candidate records exist; executing consumer/control codes unresolved |

The authenticated US text codon stream remains available to diagnostics and
contains the real `GO AWAY AND RESURRECT THERON` prompt. Its ordered
eight-name champion sequence is source-bound through the authenticated
little-endian codon stream, while its following title/control fields remain
unresolved. The JP ASCII cluster is retained by the separate JP roster
receipt, including its source titles. Neither regional candidate is promoted
to a plaque, scroll, HUD or translated text surface until the original
HuC6280 text consumer is tied to the bytes by disassembly or capture.

The text/font consumer, character mapping, and control-code semantics are not
source-locked for Theron's Quest.

### 4.2 String format

Candidate byte/word regions are retained only where their source offsets are
authenticated; they are not promoted to a generic string or tile-index
stream. Terminators, lengths, control codes, and JP/US ownership require a
game-owned executing consumer or equivalent capture. Host progression labels
are not proof of the retail dungeon-name mapping.

### 4.3 Quest-item name boundary

The Track 19 source-owned name/type table is documented in §3.1. No complete
retail binding from each quest-item name to a dungeon, placement, or runtime
consumer is claimed based solely on the host progression table.

---

## 5. Champion / Character Format

### 5.1 Party Structure

Firestaff's host model supports Theron and up to three companions. This does
not establish the retail party size, character slots, or persistence rules.

Firestaff's keyboard quicksave keys follow this boundary as well: while a
Theron Track 02 dungeon is active, F5/F9 do not route through the generic DM1
world serializer. They report that saving belongs after stage clear and that
loading belongs at the start-menu file selector. This is a safety boundary,
not proof that Firestaff's stage-clear writer or slot selector is complete.

Source: Firestaff host party/save APIs; original-game party semantics remain
unresolved.

### 5.2 Champion Record Format

No source-backed retail champion binary record layout is claimed here. The
host champion structures and reset/persistence rules are application-side
contracts; they are not evidence of Theron's retail field widths, skill ranks,
inventory encoding, starting equipment, or champion-selection semantics.
Those retail bindings remain unresolved pending authenticated source or
runtime capture.

### 5.3 Theron-Specific Data

Theron-specific retail initialization, persistence, and companion reset
semantics remain unresolved by the authenticated media bindings in this
document. Host save and progression behavior is documented separately as
application behavior (§9), not as original-game proof.

---

## 6. Creature Format

### 6.1 Creature Type Subset

No source-backed mapping from Theron creature IDs to retail creature names or
DM1/CSB indices has been established here. The Track 02 map header's creature
count and the loader's creature-bank/source spans are retained as authentic
records, but neither is a name or behavior table. No DM1 creature roster,
absence claim, or spawn-rate comparison is promoted as Theron data.

Source: `include/theron_v1_track02_dungeon_map.h`,
`include/theron_v1_track02_dungeon_loader.h`, and
`src/theron/theron_v1_track02_dungeon_loader.c`; creature names and runtime
semantics remain unresolved.

### 6.2 Creature Record Format (In-Dungeon)

The current Track 02 decoder retains source creature occurrences and bank
references where the source category parser identifies them. It does not
decode them into the hypothetical DM1-shaped spawn struct above. Health,
behavior, combat values, and AI semantics need an authenticated consumer
binding and are not inferred from DM1.

Source: `src/theron/theron_v1_track02_dungeon_loader.c` and
`include/theron_v1_track02_dungeon_loader.h`.

### 6.3 Creature Graphics

The Theron creature-to-sprite index, sprite dimensions, palette selection,
and source span have not been bound to a verified Track 02 graphics consumer.
The host's creature presentation therefore cannot use an assumed DM1 mapping
or a synthetic sprite-attribute record.

Source boundary: authenticated Track 02 creature-bank/source spans; graphics
mapping and runtime consumer remain unresolved.

---

## 7. Graphics / Tile Format

### 7.1 Graphics evidence boundary

This source-lock phase has not bound the Theron graphics source spans to an
executing tile, sprite, palette, or viewport consumer. Consequently it does
not assert a Track 02 graphics block layout, bits-per-pixel format, tile
counts, sprite dimensions, or OAM record. General PC Engine hardware facts
do not establish which graphics formats or assets Theron uses.

DM1 VGA/planar graphics and Nexus Saturn geometry are unrelated references,
not substitutes for authentic Theron assets. The authenticated map tile
family in §2.3 is a map-data classification only; it does not identify the
rendered graphic.

**Unresolved:** source offsets and hashes, decoding format, palette ownership,
asset-index bindings, and the runtime renderer consumer. Keep graphics
unavailable rather than synthesizing artwork or mappings.

---

## 8. Spell format and runtime subset

No authenticated Theron spell index/name table or executing spell consumer
has been bound in this phase. DM1/CSB spell indices and spellbook assumptions
are not used as Theron data. Spell names, incantations, requirements,
effects, and party-pool behavior remain unresolved; do not expose a guessed
spell list as supported content.

### 8.1 Spell data record

No source-backed Theron spell record layout has been established. A record
shape must not be inferred from DM1 or from host-side casting structures.
The format remains unresolved; no field layout is claimed.

---

## 9. Firestaff host save format

### 9.1 Save Slot Layout

The host implementation has eight slots; that is an application policy, not
an established original-game save-slot count.

The host format is defined by the application serializer and must not be
presented as a reverse-engineered retail save format. Retail save ownership,
encoding, offsets, checksum, and in-dungeon restrictions remain unverified in
the authenticated evidence reviewed here.

Source: `include/theron_v1_save_load.h` and
`src/theron/theron_v1_save_load.c` (Firestaff host format only).

### 9.2 Obfuscation

The host serializer applies its own reversible encoding and validation. No
retail save obfuscation or checksum has been established; host implementation
details are not evidence of the original format.

Source: `src/theron/theron_v1_save_load.c` (host-side implementation only).

### 9.3 What Persists vs Resets

| Host data field | Firestaff application behavior | Retail behavior |
|-----------------|-----------------------------------|------------------------------|
| Theron state | Serializer-defined | Not fully authenticated |
| Companion champions | Serializer-defined | Not fully authenticated |
| Gold | Serializer-defined | Not fully authenticated |
| Quest progress | Serializer-defined | Not fully authenticated |
| Dungeon completion | Serializer-defined | Not fully authenticated |

Source: `include/theron_v1_save_load.h` and
`src/theron/theron_v1_save_load.c` (host-side behavior only).

---

## 10. CD-ROM Audio Format

### 10.1 Track structure evidence boundary

This document's authenticated data input is CD track 02. It does not lock the
complete CD-DA track catalog, regional spoken content, or audio timings.
Consult a hash-verified CUE/TOC and direct audio inspection before asserting
those properties.

### 10.2 ADPCM audio boundary

An ADPCM/SFX block, its location, codec parameters, and runtime consumer have
not been bound to the authenticated Track 02 records in this phase. No audio
format or channel count is assumed from the platform or host implementation.

---

## 11. Source inventory and unresolved runtime subsets

The authenticated Track 02 parser reports seven dungeon map groups with
34 source maps in total. These counts describe decoded map records, not
campaign floor count or reachable progression. Dungeon names, quest-item
associations and seed values are not inferred from the group count.

The authenticated Track 19 inventory source contains 69 item-name/type rows
and a 66-row, six-byte property table. Those table dimensions do not prove
how many distinct retail items are reachable or how the game interprets each
row. See §3.1 for the exact boundary.

Creature and spell subsets, their retail indices, names, and executing
consumers remain unresolved in the source material reviewed for this phase.
DM1/CSB/Nexus rosters and spell lists are not Theron evidence and are not
used as substitute game data. No creature or spell is marked supported from
those lists.

---

## 13. Phase 2 Evidence Checklist

```
[x] Track 02 provenance — authenticated US and JP Track 02 inputs
[x] Track 02 records — seven source-bound map groups and object-count tables
[x] Track 02 records — ground, door, teleporter, creature-bank and item spans
[ ] Quest item names — complete dungeon/placement/runtime binding
[ ] Dungeon format — HuC6280 loader handoff and level-record consumer
[ ] Dungeon format — TQ grid encoding and square-to-material mapping
[ ] Item format — object-record ownership in the executing game loader
[ ] Text format — executing text consumer and control-code semantics
[ ] Text format — JP text ownership and translated string storage
[ ] Champion format — retail party structure and reset rules
[ ] Champion format — retail champion record fields
[ ] Champion format — Theron persistence and initialization
[ ] Creature format — source-backed TQ creature subset and spawn semantics
[ ] Graphics hardware — PC Engine VDC/VCE snapshot format is retained
[ ] Graphics format — dungeon tile/material bindings and palette ownership
[ ] Graphics format — sprite attribute table (OAM) semantics
[ ] Spell format — source-backed spell subset and indices
[ ] Spell format — executing spellbook consumer
[x] Host save format — Firestaff serializer documented separately from retail
[ ] Retail save format — encoding, layout, checksum, and persistence semantics
[x] Audio source identity — authenticated Track 02 inputs
[ ] Audio format — CD-DA catalog and ADPCM block/consumer
[ ] Track 02 — executing post-startup consumer read with source LBA/span
[ ] Verify TQ dungeon grid size and square-to-tile mapping
[ ] Confirm TQ level-record decompression and object ownership
[ ] Confirm spell table size and indices
[ ] Confirm TQ creature types and spawn semantics
```

---

## 14. Reference Sources

| Source | Content |
|--------|---------|
| Track 02 map parser | `include/theron_v1_track02_dungeon_map.h`, `src/theron/theron_v1_track02_dungeon_map.c` |
| Track 02 occurrence loader | `include/theron_v1_track02_dungeon_loader.h`, `src/theron/theron_v1_track02_dungeon_loader.c` |
| Track 19 inventory | `include/theron_v1_track19_inventory.h`, `src/theron/theron_v1_track19_inventory.c` |
| Track 19 item names | `include/theron_v1_track19_item_names.h` |
| Host save implementation only | `include/theron_v1_save_load.h`, `src/theron/theron_v1_save_load.c` |
| Authenticated media | US/JP Track 02 and Track 19, identified by their manifests and hashes |

---

## 15. Next Steps

1. Capture a positive post-startup game-owned Track 02 read with executing PC,
   source LBA, destination, and byte-exact payload.
2. Correlate that read with the retained seven map groups and level records.
3. Decode the consumer-owned level/object stream before enabling dungeon
   handoff or production viewport drawing.
4. Bind authenticated bitmap/VCE/VDC records only after their owner and
   palette/material route is demonstrated.
5. Promote JP text and later-level records independently; no US fallback.

---

*Originally generated by cron job `Theron_V1_Phase2_DataFormats_0527`; revised to remove unverified retail-format assumptions.*
*Supersedes: tqr_v1_phase0_provenance_gate_H2339.md §4 (data format hypotheses)*
*Next: Phase 3 — Core world model, or Phase 8 verification suite*
