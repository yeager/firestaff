# Theron's Quest T900-bevis (2026-08-08)

## Slutsats

T900 är ännu inte bevisad som körbar object-/inventorykonsument. Det som är
bevisat är den autentiska datan och den statiska loadern runt den. Att kalla
detta full T900 skulle vara att lägga till spelbetydelse som inte finns i
bevismaterialet.

## Beviskedja

| Led | Autentiskt bevis | Resultat |
| --- | --- | --- |
| Track 02-identitet | US `TQUS02.bin`, MD5 `f23601102138f87c33025877767ebf76`; JP `TQJP02.bin`, MD5 `b7afb338ad31be1025b53f9aff12d73a` | Godkänt |
| Object-/thing-records | Kategorier 0..10, 14 och 15 avkodas från riktiga Track 02-user-data; monster, weapon, clothing, scroll, potion, chest och misc behåller råfält och provenance | Datapost godkänd, semantik ej godkänd |
| Item properties | 66 × 6 byte matchar den riktiga US/JP-tabellen byte för byte; US/JP Track 19-proben passerar | Propertypayload godkänd, T900-consumer ej godkänd |
| Inventory-provenance | Pickup kopierar nu hela källrecorden (storlek + 16 bytes) och v7-save roundtrip återställer den för samma object | Provenance godkänd, T900-regler ej godkända |
| Dungeon-textkälla | US-loadern bevarar hela den riktiga codonströmmen i load-resultatet; JP Track 02 rapporterar verifierat noll textord | Source stream godkänd, HuC6280-textkonsument ej godkänd |
| Statisk HuC6280-kedja | `theron-us-bank1f-consumer.asm` och `$2386–$252A` verifieras mot båda retailbilderna | Loader/dekomprimering godkänd |
| Runtime object-consumer | En autentiserad Mednafen/System Card-körning på extern-disk når BIOS och producerar snapshots, men ingen verifierad spelruntime-läsning i `$2600–$27FF` | T900-konsument saknas fortfarande i beviset |
| Capture-instrumentering | Mednafen-harnessen fångar nu både läsningar och skrivningar i `$2600–$27FF`, med PC, fysisk adress och MPR-avledd fysisk PC | Mätväg godkänd, ingen semantik godkänd |

Den lokala real-data-körningen `test_theron_v1_track02_thing_data` passerar
dessutom mot `TQUS02.bin` och `TQJP02.bin`: båda varianterna matchar den
source-bound 66×6-byte propertytabellen, och alla sju dungeonblock laddar sina
riktiga ground refs, object counts och kategori-4 monsterrecords. Detta
bevisar att T900:s råa object-/propertyunderlag når Firestaffs data-lager; det
bevisar inte att originalets T900-rutiner konsumerar eller muterar state.

`test_theron_v1_track02_dungeon_loader` verifierar dessutom att varje
source-bunden kategori-4-grupp med giltig typ/count/HP projiceras till den
levande creature-poolen för US, och nu även JP, med bibehållen source-ref,
cell, typ och HP. Attack, AI, loot och generator-spawn lämnas avsiktligt
obundna tills deras originalkonsumenter är fångade.

Inventoryprovenancen är nu också lossless genom pickup, drop och save/load:
den fullständiga råa itemrecorden följer med utöver propertyraden och de
namngivna tillståndsfälten. Detta är ett källbytebevarande, inte ett påstående
om att Firestaff har återfunnit T900:s equip/use/stack-regler.

## Vad T900-bevis skulle behöva innehålla

En godkänd capture måste samtidigt visa:

1. CD/FIFO-källan och bytes som laddas till RAM-fönstret runt `$2600`.
2. Exekverande HuC6280-PC och bank/MPR-läge när objectrecorden läses.
3. Käll-LBA eller Track 02-user-data-offset för samma record.
4. Vilka bytes som skrivs till object-/thing-/inventory-state efter läsningen.
5. En reproducerbar use/equip/stack/drop/loot-transaktion mot samma
   source-record.

Den statiska VCE- och bank-$1f-receipten uppfyller inte dessa krav. Inte heller
gör ett fixture-test, en hostmodell, en propertytabell eller ett strukturellt
objectrecord det.

Captureharnessen har en definierad `main_ram_target_write`-rapport för framtida
state-skrivningar när originalmedia och System Card faktiskt körs, men den
aktuella HuC6280-patchen kopplar ännu bara in läsningen. Den
autentiserade System Card 3.0-identiteten är nu verifierad
(`ff1a674273fe3540ccef576376407d1d`), liksom US Track 02 ISO-identiteten
(`ceb02343868f80cec899e9b239aff2da`). Den externa capturekörningen producerar
dessutom 64 KiB VDC-VRAM och 1 KiB VCE-palette snapshots. Den nådde dock inte
en godkänd spelägd `$2600–$27FF`-läsning eller state-skrivning; RNG, AI, T700
och T900 förblir därför fail-closed.

Den reproducerbara externa capturevägen stöder nu även
`THERON_CAPTURE_AUTOLOAD_STATE` för en autentisk Mednafen-savestate. På
extern-disken autoloadades den hashmatchande US-savestaten utan fel, men den
återupptog inte en körande spelägd HuC6280-loop. En frame-bunden replay med
äkta Track 02/System Card gav 47 inputtransaktioner och 2 CD-IRQ, men 0
icke-System-Card-CD-anrop och 0 `$2600–$27FF`-konsumentläsningar. Det är en
negativ capturegräns, inte ett tillstånd att aktivera RNG, AI, T700 eller T900.

Captureinfrastrukturen är nu också verifierad med en native SDL 2.30.9-build
på extern-disken och Cocoa som faktisk macOS-videobackend. En körning med
äkta Quartz RUN-events producerade fyra host key-events, 47 PCE-input-
transaktioner, två CD-IRQ samt VCE/VRAM-snapshots. Den gav fortfarande noll
icke-System-Card-CD-anrop och noll `$2600–$27FF`-konsumentläsningar. Det
stärker capturebevisets reproducerbarhet, men ändrar inte den semantiska
gränsen.

## Verifiering

```text
test_theron_v1_bank1f_consumer_receipt          PASS
firestaff_theron_v1_track19_inventory_probe    PASS
theron_v1_track02_provenance_runtime_consumer  PASS
theron_v1_track02_level_object_trace_preparation PASS
test_theron_v1_track02_thing_data (US + JP real BIN) PASS
test_theron_v1_track02_dungeon_loader (US + JP live creature projection) PASS
```

Detta är ett bevis på den nuvarande gränsen, inte ett påstående om färdig
T900-paritet. Produktionen ska fortsätta neka T900-driven inventory-, loot-,
use- och equipsemantik tills `$2600`-konsumenten är fångad från originalmedia.
