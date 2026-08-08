# Syntetiskt data per spel

Inventering gjord 2026-08-08 i Firestaff-repot. Dokumentet skiljer mellan

1. syntetiskt data som bara används av tester och negativa kontrakt,
2. diagnostiska världar eller fallback-vägar som inte får användas i produktion,
3. placeholder/procedural/AI-genererat material som kan se ut som riktigt speldata.

En test-fixture ska inte ersättas med en kopierad originalfil enbart för att
den finns. Fixturen testar ofta ett avgränsat formatfel eller en negativ gren.
När samma produktionsväg redan har en autentiserad källa ska däremot den vägen
inte marknadsföras eller verifieras med fixturdata.

## Sammanfattning

| Spel | Syntetiskt material som hittades | Autentisk källa finns | Rätt åtgärd |
|---|---|---|---|
| DM1 | V2/V2.2 modern-art placeholders, diagnostiska V2-modeller, test-fixtures och capture-fixtures | Ja: PC/DOS, FM Towns och flera originalarkiv under `~/.firestaff/data/dm1` | Behåll test-fixtures isolerade. V1 ska läsa originaldata eller ge no-draw. V2.2 får inte använda placeholder-art som riktig DM1-grafik. |
| DM2 | V2/V2.2 HUD- och modern-art-fixtures, syntetisk dungeon/overlay-testning och bounded fallback-fixtures | Ja: DOS och FM Towns under `~/.firestaff/data/dm2` | Behåll endast i test/scratch. Produktion ska använda verifierad GDAT/DUNGEON-data eller fail-closed. |
| CSB | Syntetisk dungeon-loader/world-fixture, experimentell launch-fixture och V2.2 procedural art | Ja: PC/Amiga/FM Towns under `~/.firestaff/data/csb` | Behåll negativa och parser-fixtures. Byt inte in procedural art; bind motsvarande originalposter innan någon V2.2-väg öppnas. |
| Nexus | Genererade DGN/DMDF/save-fixtures och legacy synthetic fallback i äldre probes | Ja: Saturn ISO/CUE, DGN, SLEV och MNS under `~/.firestaff/data/nexus` | Använd originalfiler i real-data-prober. Behåll synthetic fallback endast explicit fixture/test och märk den inte som gameplay-bevis. |
| Theron | Procedural/AI-genererat V2/V2.2-material, no-op/fixture-start och synthetic parser/runtime-fixtures | Ja: autentiska US/JP Track 02 BIN/ISO under `~/.firestaff/data/theron` | Låt produktion vara capture-gated. Bind bara autentiserade Track 02-poster; ersätt inte saknade semantiska rutter med genererade data. |

## DM1

### Träffar

- `src/dm1v2/` innehåller kompatibilitetsmodeller som tidigare kunde skapa
  host-inventerade champion-värden, väder, partiklar, skakningar, loggar,
  övergångar och liknande presentation. De är numera inert/no-draw eller
  testbundna, men ska inte räknas som originaldata.
- `docs/source-lock/dm1_v22_finished_art_material_gate_pc34.md` beskriver
  V2.2:s placeholder/procedural-art-gate. Det paketet är inte en autentisk
  DM1-grafikkälla.
- `tests/fixtures/minimal.DAT`, `parity-evidence/fixtures/` och DM1:s
  `*_fixture`/`*_probe`-program är syntetiska eller kontraktsbundna. De testar
  parser-, no-draw- och negativgrenar och får inte användas som pixelbevis.
- `probes/dm1/firestaff_dm1_v1_original_fakewall_view_collision_probe.c` och
  `src/dm1/dm1_v1_viewport_fakewall_pc34_compat.c` innehåller en diagnostic
  fake-wall-väg. Den ska inte överta en autentiserad PC34-vy.

### Riktig källa

- PC/DOS-original finns i `~/.firestaff/data/dm1`, bland annat den extraherade
  `Dungeon-Master_DOS_EN_Version-34`-katalogen med `DATA/`.
- Originala PC34-sparningar finns utanför repot under
  `~/.firestaff/saves/dm1/original-pc34/` och i användarens Downloads-korpus.
  De får inte ersättas med genererade saves; C13 måste fortfarande styrkas av
  en autentisk save som faktiskt innehåller C13-händelsen.
- FM Towns-originalet finns i `~/.firestaff/data/dm1/fmtowns_iso/`, inklusive
  `EDM.EXP`, `TBIOS.BIN` och diskbilden.

### Beslut

DM1 V1 ska fortsätta att konsumera verifierade GRAPHICS.DAT/DUNGEON.DAT- och
save-bytes eller lämna materialet tomt. V2.2-placeholder-art ska inte ersätta
original PC34-poster bara för att den är visuellt komplett. De kvarvarande
pixelpar- och C13-capture-gaterna är evidensarbete, inte tillstånd att skapa
syntetiska saves eller skärmbilder.

## DM2

### Träffar

- `examples/dm2_hud_widget_synthetic/` är uttryckligen en syntetisk
  1x1-fixture för HUD-gate-testning. Manifestets `generator` är
  `synthetic_test`; filerna är inte DM2-grafik.
- DM2 V2/V2.2 har dokumenterade placeholder/procedural-art-gates och
  scratch-fixtures. Dessa beskriver gatebeteende, inte färdiga Skullkeep-
  material.
- Dungeon-loader-, väder-, overlay- och startup-fixtures kan använda små
  syntetiska världar eller input. De är tillåtna för deterministiska tester
  men är inte real-data-runtimebevis.

### Riktig källa

DM2-original finns under `~/.firestaff/data/dm2`, med DOS-extraktet och
`fmtowns_iso/` som de viktigaste lokala källorna. Produktionsprober ska välja
de verifierade GDAT/DUNGEON-filerna därifrån när de finns och ska inte främja
syntetiska HUD- eller dungeonbytes till produktion.

### Beslut

HUD-fixturen ska ligga kvar som fixture eftersom den testar state-gaten. Den får
inte installeras som ett riktigt artpack. V2.2 ska förbli stängd tills riktiga
source-owned materialposter och en pixelverifiering finns.

## CSB

### Träffar

- CSB V1:s loader/world-fixtures använder små syntetiska dungeon-buffertar för
  livscykel, rescan och negativa tester.
- `parity-evidence/verification/csb_v1_experimental_launch_intent_fixture.json`
  är en explicit experimentell fixture och får inte räknas som autentisk
  launch- eller gameplay-evidens.
- CSB V2.2:s tidigare procedural/material-fixtures är testmaterial. De ska
  inte ritas i produktion när motsvarande GRAPHICS.DAT-post saknas.

### Riktig källa

CSB-original finns under `~/.firestaff/data/csb`, inklusive PC/Amiga-arkiv och
`fmtowns_iso/`. PC34 GRAPHICS.DAT/DUNGEON.DAT ska användas av real-data-
proberna när de är tillgängliga.

### Beslut

Fixtures får finnas för parser- och rescan-kontrakt. De ska vara explicit
fixture-only. Procedural V2.2-art ska inte ersätta riktiga CSB-poster.

## Nexus

### Träffar

- `scripts/generate_nexus_v1_fixtures.py` skapar syntetiska DGN-, DMDF- och
  FNXS-save-filer. `scripts/fixtures/nexus_v1_save_synthetic.dat` är därför
  aldrig ett riktigt Nexus-save.
- `docs/source-lock/nexus_v1_phase7_verification_suite_H0357.md` beskriver
  dessa fixtures och en äldre synthetic fallback för parser-/round-trip-
  prober.
- `tests/fixtures/` och `*_fixture`-targets är avsedda för deterministisk
  testning, inte för att fylla en saknad Saturn-källa.

### Riktig källa

Nexus-original finns under `~/.firestaff/data/nexus`, inklusive den engelska
ISO/CUE-källan, `LEV*.DGN`, `SLEV*.BIN`, `SNDLEV*.SAL` och `*.MNS`.

### Beslut

Synthetic DGN/DMDF/save ska behållas för parser- och negativtester men ska
inte passera som ett spelbart original. Real-data-prober ska använda
hash-/formatverifierade filer ur Nexus-katalogen. Saturn-pixel- och
runtime-capture-gater förblir öppna tills äkta capture finns.

## Theron

### Träffar

- `src/theron/theron_v22_modern_assets_pc34.c` och närliggande V2/V2.2-
  material beskriver generated/procedural/AI-art som fixture-only. Det får
  inte annonsera en riktig Track 02-installation.
- `src/theron/theron_v1_viewport_runtime_noop.c` och fixture-startvägar är
  avsiktligt no-op/capture-gated; de skapar inte en ersättningsvärld.
- Track 02 parser-, descriptor- och runtime-fixtures kan innehålla
  syntetiska byteformer för negativa och shape-bundna tester.

### Riktig källa

Autentiska US/JP Track 02-filer finns under `~/.firestaff/data/theron`, bland
annat `TQUS02.bin`, `TQJP02.bin` och deras ISO-filer. De ska vara enda grund
för produktionens level-, item-, champion- och bitmapclaims.

### Beslut

Behåll fixture- och no-op-vägarna som tydligt märkta tester. Ersätt inte
öppna semantiska eller visuella Theron-gater med procedural art, AI-upscale
eller genererade roster-/leveldata. Nästa steg är source-lock och autentisk
capture, inte mer syntetiskt innehåll.

## Kontrollregel framåt

Ny data får bara räknas som riktig speldata när dess originalkälla, format,
hash/proveniens och runtime-ägare är dokumenterade. Om motsvarande original
finns lokalt ska produktionskod läsa den eller vägra rita/ladda. Syntetiska
filer får endast ligga i `tests/`, `probes/`, `examples/` eller explicit
fixture-dokumentation och ska aldrig användas som positiv real-data-evidens.
