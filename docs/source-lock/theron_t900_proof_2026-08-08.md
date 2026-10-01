# Theron's Quest T900 Evidence (2026-08-08)

## Conclusion

T900 has not yet been proven as an executable object/inventory consumer. What
has been proven is the authentic data and the static loader around it. Calling
this full T900 support would add gameplay meaning that is not present in the
evidence.

## Latest Transport Capture (2026-08-09)

The capture gate now admits the lower, source-bound CD/FIFO→RAM path even when
the optional high-level markers are absent. A run from an external drive with
authentic US Track 02 and System Card produced 161 raw-sector spans, 51 SCSI
READ commands, 161 sector bindings, 25 CDIRQ receipts and two byte-exact
`pce_cd_origin_ram_receipt` records. It also logged 32
`game_main_ram_e009_dispatch` records and 4,096 main-RAM consumer reads.

This is an admitted transport receipt, not T900 or level/object evidence: the
capture publishes no `$2600` consumer, RNG return, spawn/AI/combat/loot chain
or T700 tick. The semantic gates therefore remain unchanged.

## Evidence Chain

| Link | Authentic evidence | Result |
| --- | --- | --- |
| Track 02 identity | US `TQUS02.bin`, MD5 `f23601102138f87c33025877767ebf76`; JP `TQJP02.bin`, MD5 `b7afb338ad31be1025b53f9aff12d73a` | Admitted |
| Object/thing records | Categories 0..10, 14 and 15 decoded from real Track 02 user data; monster, weapon, clothing, scroll, potion, chest and misc retain raw fields and provenance | Data record admitted, semantics not admitted |
| Item properties | 66 × 6 bytes match the real US/JP table byte for byte; US/JP Track 19 probe passes | Property payload admitted, T900 consumer not admitted |
| Dungeon-local item names | Seven 66-entry Track 02 tables per region are read from their authentic user-data spans; every offset, length and FNV is verified | The object's verified dungeon + item type may look up the raw name; text/glyph consumer not admitted |
| Inventory provenance | Pickup now copies the complete source record (size + 16 bytes), preserves property category, and the v7 save roundtrip restores it for the same object | Provenance/integrity check admitted, T900 rules not admitted |
| Dungeon text source | US loader preserves the complete real codon stream in the load result; JP Track 02 reports verified zero text words | Source stream admitted, HuC6280 text consumer not admitted |
| Static HuC6280 chain | `theron-us-bank1f-consumer.asm` and `$2386–$252A` verified against both retail images | Loader/decompression admitted |
| Runtime object consumer | An authenticated Mednafen/System Card run on the external drive reaches BIOS and produces snapshots; the new GUI run proves BIOS Run → Track 02 sector reads, but no verified game-runtime read in `$2600–$27FF` | T900 consumer remains unproven |
| Capture instrumentation | Mednafen harness now captures both reads and writes in `$2600–$27FF`, with PC, physical address and MPR-derived physical PC | Measurement path admitted, no semantics admitted |

The local real-data run `test_theron_v1_track02_thing_data` also passes
against `TQUS02.bin` and `TQJP02.bin`: both variants match the source-bound
66×6-byte property table, and all seven dungeon blocks load their real ground
refs, object counts and category-4 monster records. This proves that T900's
raw object/property inputs reach Firestaff's data layer; it does not prove
that the original T900 routines consume or mutate state.

`test_theron_v1_track02_dungeon_loader` also verifies that each source-bound
category-4 group with valid type/count/HP is projected into the live creature
pool for US, and now JP as well, preserving source ref, cell, type and HP.
Attack, AI, loot and generator spawning intentionally remain unbound until
their original consumers are captured.

Inventory provenance is now also lossless through pickup, drop and save/load:
the complete raw item record accompanies the property row and named state
fields. This preserves source bytes; it does not claim that Firestaff has
recovered T900's equip/use/stack rules.

The earlier static US list at `$21A08E` turned out to be only dungeon 7's
table. The six other quest blocks have their own 66-entry lists, and JP has
seven corresponding Shift-JIS lists at regional offsets. Production now reads
all 14 lists directly from the already authenticated Track 02 user-data
stream. Each table has a pinned offset, exact byte count and its own FNV-1a.
The object's already verified `source_dungeon` and `source_item_type` select
the entry; no Track 19 indexing or fallback text is used. Dungeon 6, entry 64
is empty in both US and JP and is therefore preserved as a valid empty source
entry.

This proves the static relation between the source object's local item type
and the quest block's parallel name entry. It does not prove the original's
screen rendering, Shift-JIS glyph selection, use/equip/stack rules or any
mutation of T900 state.

The parallel 66-byte table at `$21A046` was also misclassified. Values
`$22/$80/$81/$82` apply to the Demon block, but the same structural table uses
other type codes in other dungeons, for example `$95–$98` in Akutuba. JP has
additional regional variations. The table must therefore not determine
whether a thing record is weapon, clothing, scroll, potion or misc. That
category is already explicit in the record reference as 5, 6, 7, 8 or 10.

Firestaff now preserves every dungeon-local type-code table with its exact
offset and FNV, but uses the record reference's category to bind the
item-indexed 6-byte property row. The real-data corpus requires every
materialized item record to have a property row and exempts only category-9
chests. This still opens no use/equip/stack rules; the type code remains raw
until its original consumer is identified.

Category 4 has now been corrected against the complete DMBUILDER contract.
`item.c:getItem()` skips the record's generic two-byte link before returning a
pointer to the 14-byte `dms.h:dm_monster` structure. Firestaff previously read
the link word as `chested`, thereby shifting the type, position and all HP
values. The decoder now reads `next_ref` at byte 0, `chested` at byte 2,
type/position at bytes 4/5, four HP words at bytes 6–13 and the trailing word
at byte 14.

The ground-reference walker therefore also follows the real `next_ref` of a
category-4 record. Against the authentic US campaign, the census returns to
`640/2189` source/placed records; the three chained objects are real links.
Monster count `165` and generator count `46` are unchanged.

The field now also follows the source-bound monster record into the live
creature and save/load version 9. This is a lossless state binding of a real
source field, not an interpretation of what T900 does with it.

## Required T900 Evidence

An admitted capture must show all of the following at once:

1. The CD/FIFO source and bytes loaded into the RAM window around `$2600`.
2. The executing HuC6280 PC and bank/MPR state when the object record is read.
3. The source LBA or Track 02 user-data offset for that same record.
4. Which bytes are written to object/thing/inventory state after the read.
5. A reproducible use/equip/stack/drop/loot transaction against that same
   source record.

The static VCE and bank-$1f receipt do not satisfy these requirements.
Neither does a fixture test, host model, property table or structural object
record.

The capture harness has a defined `main_ram_target_write` report for future
state writes when the original media and System Card are actually running.
The authenticated System Card 3.0 identity is now verified
(`ff1a674273fe3540ccef576376407d1d`), as well as the US Track 02 ISO identity
(`ceb02343868f80cec899e9b239aff2da`). The external capture run also produces
64 KiB VDC-VRAM and 1 KiB VCE-palette snapshots. However, it did not reach an
admitted game-owned `$2600–$27FF` read or state write; RNG, AI, T700 and T900
therefore remain fail-closed.

The capture path supports `THERON_CAPTURE_AUTOLOAD_STATE`, but the candidate
found on the external drive was not a Mednafen save state. It is a 2 KiB
`HUBM` SRAM file, which previously caused a misleading Mednafen `Unexpected
EOF` error. The capture script now rejects the file before emulator startup
with a clear signature check. A complete, authenticated Mednafen save state
with game-owned resumption is therefore not yet available or proven. A
separate frame-bound replay with authentic Track 02/System Card produced 47
input transactions and 2 CD IRQs, but 0 non-System-Card CD calls and 0
`$2600–$27FF` consumer reads. This is a negative capture boundary, not
permission to enable RNG, AI, T700 or T900.

The capture infrastructure has also been verified with a native SDL 2.30.9
build on the external drive and Cocoa as the actual macOS video backend. A run
with authentic Quartz RUN events produced four host key events, 47 PCE input
transactions, two CD IRQs and VCE/VRAM snapshots. It still yielded zero
non-System-Card CD calls and zero `$2600–$27FF` consumer reads. This strengthens
the reproducibility of the capture evidence but does not change the semantic
boundary.

The latest native run on the external drive also logged BIOS CD ports with
HuC6280 PC: `$1804` was reset with `02` and then `00`, followed by writes to
`$1802` and status reads from `$1802/$1803`. BIOS still does not leave this CD
initialization path: SCSI READ commands, raw sectors, FIFO bindings and
game-owned RAM consumers remain zero. This distinguishes a verified BIOS/CD
reset attempt from an actual Track 02 handoff and continues to gate RNG, AI,
T700 and T900 semantics.

The capture script also supports `THERON_CAPTURE_SOUND=1` for a diagnostic
CDDA-enabled run; the default remains silent (`0`). The authentic US run with
audio enabled produced the same negative boundary
(`cd_irq_callbacks=2`, `non_system_card_pcecd_reads=0`), so the audio flag is
for capture reproducibility and is not evidence of a game-owned audio or
object consumer.

The positive GUI run is now a separate startup/media receipt: a real macOS
Quartz Return event (`SDL scancode 40`) reaches Mednafen, PCE input shows the
Run bit `raw=0008`, and the same run logs 56 SCSI reads and 175 raw sectors
from authenticated US Track 02. The first menu screen is a local original
emulator artifact; no such screenshots are tracked or used as Firestaff
output. Full hashes and limitations are in
`docs/source-lock/theron-authentic-track02-handoff-2026-08-08.md`.
This supersedes the earlier negative conclusion that no Track 02 handoff at
all had been proven, but it still does not show the `$2600` consumer or any
T900/RNG/AI/T700 semantics.

An older capture path explicitly selects Mednafen media index `0` using
`-which_medium 0`. It removes an initialization ambiguity in RMDUI defaults:
the external drive logs that the authentic Track 02 disc is actually inserted
and the tray closed, but that particular run stalled in the BIOS CD status
loop without SCSI READ. The new GUI run above supersedes the old negative
conclusion for the startup handoff. RNG, AI, T700 and T900 consumers remain
unproven.

The capture script can select the second official HuC6280 core with
`THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast` only when Mednafen itself lists the
module in `-help`; the default remains `pce`. An earlier external run showed
that the binary can contain `pce_fast` strings without accepting
`-force_module pce_fast`. That path is now rejected immediately, before
capture, rather than producing an empty or misleading receipt. An actual
`pce_fast` module must first be confirmed by Mednafen's module list, and is
then subject to the same System Card, CUE, Track 02 and semantic gates as
`pce`.

## Verification

```text
test_theron_v1_bank1f_consumer_receipt          PASS
firestaff_theron_v1_track19_inventory_probe    PASS
theron_v1_track02_provenance_runtime_consumer  PASS
theron_v1_track02_level_object_trace_preparation PASS
test_theron_v1_track02_thing_data (US + JP real BIN) PASS
test_theron_v1_track02_dungeon_loader (US + JP live creature projection) PASS
```

This is evidence of the current boundary, not a claim of complete T900 parity.
Production must continue to reject T900-driven inventory, loot, use and equip
semantics until the `$2600` consumer is captured from original media.
