# Theron's Quest T900-bevis (2026-08-08)

## Slutsats

T900 är ännu inte bevisad som körbar object-/inventorykonsument. Det som är
bevisat är den autentiska datan och den statiska loadern runt den. Att kalla
detta full T900 skulle vara att lägga till spelbetydelse som inte finns i
bevismaterialet.

## Senaste transportcapture (2026-08-09)

Capture-gaten godkänner nu den lägre, source-bundna CD/FIFO→RAM-vägen även när
de valfria high-level-markörerna saknas. En extern-disk-körning med äkta US
Track 02 och System Card producerade 161 råsektorspaner, 51 SCSI READ-kommandon,
161 sektorbindningar, 25 CDIRQ-kvitton och två byte-exakta
`pce_cd_origin_ram_receipt`-poster. Den loggade dessutom 32
`game_main_ram_e009_dispatch`-poster och 4 096 main-RAM-konsumentläsningar.

Detta är ett godkänt transportkvitto, inte ett T900- eller level/object-bevis:
ingen `$2600`-konsument, RNG-retur, spawn-/AI-/combat-/loot-kedja eller
T700-tick publiceras från capturen. De semantiska spärrarna förblir därför
oförändrade.

## Beviskedja

| Led | Autentiskt bevis | Resultat |
| --- | --- | --- |
| Track 02-identitet | US `TQUS02.bin`, MD5 `f23601102138f87c33025877767ebf76`; JP `TQJP02.bin`, MD5 `b7afb338ad31be1025b53f9aff12d73a` | Godkänt |
| Object-/thing-records | Kategorier 0..10, 14 och 15 avkodas från riktiga Track 02-user-data; monster, weapon, clothing, scroll, potion, chest och misc behåller råfält och provenance | Datapost godkänd, semantik ej godkänd |
| Item properties | 66 × 6 byte matchar den riktiga US/JP-tabellen byte för byte; US/JP Track 19-proben passerar | Propertypayload godkänd, T900-consumer ej godkänd |
| Inventory-provenance | Pickup kopierar nu hela källrecorden (storlek + 16 bytes) och v7-save roundtrip återställer den för samma object | Provenance godkänd, T900-regler ej godkända |
| Dungeon-textkälla | US-loadern bevarar hela den riktiga codonströmmen i load-resultatet; JP Track 02 rapporterar verifierat noll textord | Source stream godkänd, HuC6280-textkonsument ej godkänd |
| Statisk HuC6280-kedja | `theron-us-bank1f-consumer.asm` och `$2386–$252A` verifieras mot båda retailbilderna | Loader/dekomprimering godkänd |
| Runtime object-consumer | En autentiserad Mednafen/System Card-körning på extern-disk når BIOS och producerar snapshots; den nya GUI-körningen bevisar BIOS Run → Track 02-sektorläsning, men ingen verifierad spelruntime-läsning i `$2600–$27FF` | T900-konsument saknas fortfarande i beviset |
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

En källavvikelse i kategori 4 är nu rättad: enligt
`DMBUILDER6/src/dms.h:145-157` är monsterrecordets första ord det signerade
`chested`-fältet. Det låg tidigare felaktigt under den generiska dekoderns
`next_ref`-namn. Firestaff dekoder och test bevarar nu `chested` separat,
medan den övergripande source-object-occurrence fortfarande behåller de råa
16 bytesen. Detta förbättrar monster- och lootprovenansen men öppnar inte
T900:s runtime-konsument.

Fältet följer nu också den source-bundna monsterrecorden till live creature
och save/load-version 9. Det är en lossless state-bindning av ett verkligt
källfält, inte en tolkning av vad T900 gör med det.

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
state-skrivningar när originalmedia och System Card faktiskt körs. Den
autentiserade System Card 3.0-identiteten är nu verifierad
(`ff1a674273fe3540ccef576376407d1d`), liksom US Track 02 ISO-identiteten
(`ceb02343868f80cec899e9b239aff2da`). Den externa capturekörningen producerar
dessutom 64 KiB VDC-VRAM och 1 KiB VCE-palette snapshots. Den nådde dock inte
en godkänd spelägd `$2600–$27FF`-läsning eller state-skrivning; RNG, AI, T700
och T900 förblir därför fail-closed.

Capturevägen stöder `THERON_CAPTURE_AUTOLOAD_STATE`, men den kandidat som
fanns på extern-disken var inte en Mednafen-savestate. Den är en 2 KiB
`HUBM`-SRAM-fil, vilket tidigare gav ett missvisande Mednafenfel om
`Unexpected EOF`. Capture-scriptet avvisar nu filen före emulatorstart med en
tydlig signaturkontroll. Någon fullständig, autentiserad Mednafen-savestate
med spelägd återupptagning är därför ännu inte tillgänglig eller bevisad.
En separat frame-bunden replay med äkta Track 02/System Card gav 47
inputtransaktioner och 2 CD-IRQ, men 0 icke-System-Card-CD-anrop och 0
`$2600–$27FF`-konsumentläsningar. Det är en negativ capturegräns, inte ett
tillstånd att aktivera RNG, AI, T700 eller T900.

Captureinfrastrukturen är nu också verifierad med en native SDL 2.30.9-build
på extern-disken och Cocoa som faktisk macOS-videobackend. En körning med
äkta Quartz RUN-events producerade fyra host key-events, 47 PCE-input-
transaktioner, två CD-IRQ samt VCE/VRAM-snapshots. Den gav fortfarande noll
icke-System-Card-CD-anrop och noll `$2600–$27FF`-konsumentläsningar. Det
stärker capturebevisets reproducerbarhet, men ändrar inte den semantiska
gränsen.

Den senaste native-körningen på extern-disken loggade dessutom BIOS-CD-portarna
med HuC6280-PC: `$1804` resetades med `02` och därefter `00`, följt av skrivningar
till `$1802` och statusläsningar från `$1802/$1803`. BIOS lämnar ändå inte denna
CD-initieringsväg: SCSI READ-kommandon, råa sektorer, FIFO-bindningar och
spelägda RAM-konsumenter är fortfarande noll. Detta skiljer ett verifierat
BIOS/CD-resetförsök från en faktisk Track 02-handoff och låser fortsatt
RNG-, AI-, T700- och T900-semantik.

Capture-scriptet stöder nu också `THERON_CAPTURE_SOUND=1` för en diagnostisk
CDDA-aktiverad körning; standardvärdet är fortfarande tyst (`0`). Den
autentiska US-körningen med ljud aktiverat gav samma negativa gräns
(`cd_irq_callbacks=2`, `non_system_card_pcecd_reads=0`), så ljudflaggan är en
capture-reproducerbarhet och inte ett bevis på en spelägd ljud- eller
objectkonsument.

Den positiva GUI-körningen är nu ett separat startup-/mediareceipt: en riktig
macOS Quartz Return-händelse (`SDL scancode 40`) når Mednafen, PCE-input visar
Run-biten `raw=0008`, och samma körning loggar 56 SCSI-läsningar samt 175
råsektorer från autentiserat US Track 02. Den första menyrutan är sparad som
`verification-screens/theron-quest-us-main-menu.png` och dess fullständiga
hashar och begränsningar finns i
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
