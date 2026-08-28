# Theron's Quest T900 proof (2026-08-08)

## Conclusion

T900 has not yet been proven as an executable object-/inventory consumer. What
has been proven is the authentic data and the static loader surrounding it. To
call this full T900 would add gameplay meaning that is absent from the
evidence.

## Latest transport capture (2026-08-09)

The capture gate now accepts the lower, source-bound CD/FIFO→RAM path even
when the optional high-level markers are absent. An external-disk run with
authentic US Track 02 and System Card produced 161 raw-sector spans, 51 SCSI
READ commands, 161 sector bindings, 25 CDIRQ receipts, and two byte-exact
`pce_cd_origin_ram_receipt` entries. It also logged 32
`game_main_ram_e009_dispatch` entries and 4,096 main-RAM consumer reads.

This is an accepted transport receipt, not T900 or level/object proof: no
`$2600` consumer, RNG return, spawn/AI/combat/loot chain, or T700 tick is
published from the capture. The semantic gates therefore remain unchanged.

## Evidence chain

| Link | Authentic evidence | Result |
| --- | --- | --- |
| Track 02 identity | US `TQUS02.bin`, MD5 `f23601102138f87c33025877767ebf76`; JP `TQJP02.bin`, MD5 `b7afb338ad31be1025b53f9aff12d73a` | Accepted |
| Object-/thing-records | Categories 0..10, 14, and 15 are decoded from real Track 02 user data; monster, weapon, clothing, scroll, potion, chest, and misc retain raw fields and provenance | Data record accepted, semantics not accepted |
| Item properties | 66 × 6 bytes match the real US/JP table byte-for-byte; the US/JP Track 19 probe passes | Property payload accepted, T900 consumer not accepted |
| Inventory provenance | Pickup now copies the full source record (size + 16 bytes), preserves property category, and a v7-save roundtrip restores it for the same object | Provenance/integrity check accepted, T900 rules not accepted |
| Dungeon text source | The US loader retains the entire real codon stream in the load result; JP Track 02 reports a verified zero text words | Source stream accepted, HuC6280 text consumer not accepted |
| Static HuC6280 chain | `theron-us-bank1f-consumer.asm` and `$2386–$252A` are verified against both retail images | Loader/decompression accepted |
| Runtime object consumer | An authenticated Mednafen/System Card external-disk run reaches BIOS and produces snapshots; the new GUI run proves BIOS Run → Track 02 sector reading, but no verified game-runtime read in `$2600–$27FF` | The T900 consumer is still absent from the evidence |
| Capture instrumentation | The Mednafen harness now captures both reads and writes in `$2600–$27FF`, with PC, physical address, and MPR-derived physical PC | Measurement path accepted, no semantics accepted |

The local real-data run `test_theron_v1_track02_thing_data` also passes
against `TQUS02.bin` and `TQJP02.bin`: both variants match the source-bound
66×6-byte property table, and all seven dungeon blocks load their real ground
refs, object counts, and category-4 monster records. This proves that T900's
raw object/property basis reaches Firestaff's data layer; it does not prove
that the original T900 routines consume or mutate state.

`test_theron_v1_track02_dungeon_loader` further verifies that every
source-bound category-4 group with valid type/count/HP is projected into the
live creature pool for US, and now JP, while retaining source ref, cell, type,
and HP. Attack, AI, loot, and generator spawn are intentionally left unbound
until their original consumers are captured.

Inventory provenance is now also lossless through pickup, drop, and save/load:
the complete raw item record accompanies the property row and named state
fields. This preserves source bytes; it does not claim that Firestaff has
recovered T900's equip/use/stack rules.

One source discrepancy in category 4 is now corrected: according to
`DMBUILDER6/src/dms.h:145-157`, the monster record's first word is the signed
`chested` field. It was previously incorrectly named `next_ref` by the generic
decoder. Firestaff's decoder and test now preserve `chested` separately, while
the overall source-object occurrence still retains the raw 16 bytes. This
improves monster and loot provenance but does not open T900's runtime consumer.

The ground-reference walker now uses the same boundary: category 4 terminates
its chain after the monster record instead of interpreting `chested` as the
next object reference. Against the authentic US campaign, the census therefore
changes from `640/2189` to `637/2186` source-/placed entries; the monster count
`165` and generator count `46` do not change. The previous three entries were
false follow-on objects from a containment field, not new T900 objects.

The field now also follows the source-bound monster record to the live creature
and save/load version 9. This is a lossless state binding of a real source
field, not an interpretation of what T900 does with it.

## What T900 proof would need to include

An accepted capture must show all of the following simultaneously:

1. The CD/FIFO source and bytes loaded into the RAM window around `$2600`.
2. The executing HuC6280 PC and bank/MPR state when the object record is read.
3. The source LBA or Track 02 user-data offset for the same record.
4. Which bytes are written to object-/thing-/inventory state after the read.
5. A reproducible use/equip/stack/drop/loot transaction against the same
   source-record.

The static VCE and bank-$1f receipt does not meet these requirements. Neither
does a fixture test, a host model, a property table, or a structural object
record.

The capture harness has a defined `main_ram_target_write` report for future
state writes when original media and System Card actually run. The
authenticated System Card 3.0 identity is now verified
(`ff1a674273fe3540ccef576376407d1d`), liksom US Track 02 ISO-identiteten
(`ceb02343868f80cec899e9b239aff2da`). The external capture run also produces
64 KiB VDC-VRAM and 1 KiB VCE-palette snapshots. However, it did not reach an
accepted game-owned `$2600–$27FF` read or state write; RNG, AI, T700, and T900
therefore remain fail-closed.

The capture path supports `THERON_CAPTURE_AUTOLOAD_STATE`, but the candidate
on the external disk was not a Mednafen save state. It is a 2 KiB `HUBM` SRAM
file, which previously caused the misleading Mednafen `Unexpected EOF` error.
The capture script now rejects the file before emulator startup with an
explicit signature check. No complete authenticated Mednafen save state with
game-owned resumption is therefore yet available or proven. A separate
frame-bound replay with authentic Track 02/System Card produced 47 input
transactions and 2 CD IRQs, but 0 non-System-Card CD calls and 0
`$2600–$27FF` consumer reads. This is a negative capture boundary, not
permission to activate RNG, AI, T700, or T900.

The capture infrastructure is now also verified with a native SDL 2.30.9 build
on the external disk and Cocoa as the actual macOS video backend. A run with
authentic Quartz RUN events produced four host key events, 47 PCE input
transactions, two CD IRQs, and VCE/VRAM snapshots. It still produced zero
non-System-Card CD calls and zero `$2600–$27FF` consumer reads. This strengthens
the reproducibility of the capture evidence, but does not change the semantic
boundary.

The latest native run on the external disk also logged the BIOS CD ports with
HuC6280 PC: `$1804` was reset with `02` and then `00`, followed by writes to
`$1802` and status reads from `$1802/$1803`. The BIOS still does not leave this
CD-initialization path: SCSI READ commands, raw sectors, FIFO bindings, and
game-owned RAM consumers remain zero. This distinguishes a verified BIOS/CD
reset attempt from an actual Track 02 handoff and keeps RNG, AI, T700, and T900
semantics locked.

The capture script now also supports `THERON_CAPTURE_SOUND=1` for a diagnostic
CDDA-enabled run; the default remains silent (`0`). The authentic US run with
sound enabled produced the same negative boundary
(`cd_irq_callbacks=2`, `non_system_card_pcecd_reads=0`), so the sound flag
supports capture reproducibility rather than proving a game-owned sound or
object consumer.

Den positiva GUI-körningen är nu ett separat startup-/mediareceipt: en riktig
macOS Quartz Return-händelse (`SDL scancode 40`) når Mednafen, PCE-input visar
Run-biten `raw=0008`, och samma körning loggar 56 SCSI-läsningar samt 175
råsektorer från autentiserat US Track 02. Den första menyrutan är ett
lokalt originalemulatorartefakt; inga sådana skärmbilder trackas eller används
som Firestaff-output. Fullständiga hashar och begränsningar finns i
`docs/source-lock/theron-authentic-track02-handoff-2026-08-08.md`.
Detta ersätter den tidigare negativa slutsatsen om att ingen Track 02-handoff
alls var bevisad, men den visar fortfarande inte `$2600`-konsumenten eller
någon T900/RNG/AI/T700-semantik.

En äldre captureväg väljer explicit Mednafen-medieindex `0` via
`-which_medium 0`. Den eliminerar en initieringsambiguity i RMDUI-defaults:
extern-disken loggar att den autentiska Track 02-skivan faktiskt sätts in och
tray stängs, men just den körningen fastnade i BIOS-CD-statusloopen utan SCSI
READ. Den nya GUI-körningen ovan ersätter den gamla negativa slutsatsen för
startup-handoffens del. RNG-, AI-, T700- och T900-konsumenterna är fortfarande
inte bevisade.

Capture-scriptet kan välja den andra officiella HuC6280-kärnan med
`THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast` endast när Mednafen själv annonserar
modulen i `-help`; standarden är fortsatt `pce`. En tidigare extern körning
visade att binären kan innehålla `pce_fast`-strängar utan att acceptera
`-force_module pce_fast`. Den vägen avvisas nu direkt, före capture, i stället
för att skapa en tom eller missvisande receipt. En faktisk `pce_fast`-modul
måste därför först bevisas av Mednafen:s modulista och får därefter samma
System Card-, CUE-, Track 02- och semantikgrindar som `pce`.

## Verification

```text
test_theron_v1_bank1f_consumer_receipt          PASS
firestaff_theron_v1_track19_inventory_probe    PASS
theron_v1_track02_provenance_runtime_consumer  PASS
theron_v1_track02_level_object_trace_preparation PASS
test_theron_v1_track02_thing_data (US + JP real BIN) PASS
test_theron_v1_track02_dungeon_loader (US + JP live creature projection) PASS
```

This proves the current boundary, not completed T900 parity. Production must
continue to deny T900-driven inventory, loot, use, and equip semantics until
the `$2600` consumer is captured from original media.
