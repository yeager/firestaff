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
| Statisk HuC6280-kedja | `theron-us-bank1f-consumer.asm` och `$2386–$252A` verifieras mot båda retailbilderna | Loader/dekomprimering godkänd |
| Runtime object-consumer | Samma verifiering rapporterar `ram_consumer_2600=not_present` för US och JP | T900-konsument saknas i beviset |
| Capture-instrumentering | Mednafen-harnessen fångar nu både läsningar och skrivningar i `$2600–$27FF`, med PC, fysisk adress och MPR-avledd fysisk PC | Mätväg godkänd, ingen semantik godkänd |

Den lokala real-data-körningen `test_theron_v1_track02_thing_data` passerar
dessutom mot `TQUS02.bin` och `TQJP02.bin`: båda varianterna matchar den
source-bound 66×6-byte propertytabellen, och alla sju dungeonblock laddar sina
riktiga ground refs, object counts och kategori-4 monsterrecords. Detta
bevisar att T900:s råa object-/propertyunderlag når Firestaffs data-lager; det
bevisar inte att originalets T900-rutiner konsumerar eller muterar state.

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

Captureharnessen har därför utökats med `main_ram_target_write` för att kunna
visa state-skrivningar när originalmedia och System Card faktiskt körs. En
byggd instrumenterad binär innehåller både read- och write-formatsträngarna,
men lokal publicering och T900-semantik är fortfarande spärrad: den verifierade
capturekörningen saknar ännu en godkänd System Card/media-session.

## Verifiering

```text
test_theron_v1_bank1f_consumer_receipt          PASS
firestaff_theron_v1_track19_inventory_probe    PASS
theron_v1_track02_provenance_runtime_consumer  PASS
theron_v1_track02_level_object_trace_preparation PASS
test_theron_v1_track02_thing_data (US + JP real BIN) PASS
```

Detta är ett bevis på den nuvarande gränsen, inte ett påstående om färdig
T900-paritet. Produktionen ska fortsätta neka T900-driven inventory-, loot-,
use- och equipsemantik tills `$2600`-konsumenten är fångad från originalmedia.
