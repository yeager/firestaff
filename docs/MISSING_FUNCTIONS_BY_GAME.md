# Saknade funktioner och verifieringar per spel

Status: 2026-08-27. Detta är en sammanställning av kvarvarande luckor i
Firestaff, inte en lista över varje historiskt TODO- eller symbolnamn.

Varje punkt klassificeras så här:

- **Saknad kod** – funktionen eller dess produktionsväg är inte implementerad.
- **Saknad originaldata** – källformatet eller ett tillräckligt corpus saknas.
- **Saknad verifiering** – vägen finns, men behöver autentisk runtime-, pixel-
  eller capture-evidens innan den kan räknas som klar.
- **Avsiktligt stängd** – en syntetisk eller osäker väg ska inte öppnas igen.

Källan för detaljer är respektive spel-TODO: [DM1](../TODO-dm1.md),
[DM2](../TODO-dm2.md), [CSB](../TODO-csb.md), [Nexus](../TODO-nexus.md) och
[Theron](../TODO-theron.md).

## DM1

### Saknad originaldata eller evidens

- **Autentiskt C13-save:** inget lokalt PC34-save är verifierat med den
  efterfrågade C13-händelsen. Befintliga saves ger C03/C04-data, men ersätter
  inte ett C13-corpus.
- **Original-vs-Firestaff-pixelpar:** bredare par saknas för viewport,
  creature-chain, champion-HUD, paneler, launcher, effekter och Mac/app-
  körning.
- **Original capture av C13/HoC/top-row/action-rutter:** capture-skript och
  source gates finns, men täckningen är inte komplett.
- **FM Towns:** vissa TownsOS EGB-pixelsemantiker och mus/input-routing är
  fortfarande capture-/BIOS-beroende. Detta gäller originalparitet, inte
  Firestaffs native startväg: den äkta FM Towns-ZIP:en startar utan BIOS.

### Saknad eller ofullständig funktionalitet

- Bredare sensor-, removal- och DSA-interaktioner behöver fortfarande
  source-owned originalsave/runtime-evidens.
- Creature- och combat-kedjan har verifierade delar, men bredare grupp-layout,
  creature-combat och fler tidslinjefall är inte fullständigt bevisade.
- DM1 V2.2 saknar ett granskat, autentiskt materialpaket och komplett
  source-owned modern presentation. Placeholder/procedural-art får inte
  aktiveras som ersättning.

### Inte en kvarvarande produktionslucka

DM1 V1:s normala PC34 viewport-, HUD-, inventory- och actionvägar har
source-gates och fail-closed-beteende. Texten “synthetic” i receipts,
negative probes eller ReDMCSB:s egna `synthetic wall`-begrepp betyder inte i
sig att produktionskoden ritar syntetiska DM1-pixlar.

**Native startmatris (verifierad 2026-08-27):** Firestaff läser den verkliga
mediekopian direkt i minnet och når `dm1-runtime` med `levelLoaded=1` både
från CLI och via startmenyn för DOS PC 3.4, fransk DOS (manuellt uppackad
RAR2-media), Atari ST engelska och tyska arkiv, Amiga och FM Towns. Beviset
är de sex fokuserade CTest-fallen `dm1_v1_*_cli_boot`; de kontrollerar den
kanoniska tillgångens MD5 och använder aldrig en emulator eller BIOS vid
runtime. Detta är startbevis, inte ett påstående om full original-pixel- eller
spelsystemsparitet.

## DM2

### Saknad kod eller ofullständig produktionsväg

- **Database ownership:** Firestaffs DM2-modell behöver fortfarande fullt
  validerade original-record pools, länkar, mappar och relocation-semantik i
  stället för den reducerade save-state-layouten.
- **Input och dialog:** originalets resume-selector, keyboard/mouse ordering,
  held-button-semantik, modal dialog, text, cancel och event-queue-beteende är
  inte komplett.
- **Creature drop/AI:** AI-tabeller och delar av drop-routen finns, men
  source-owned CREATURE_AI-poster måste bindas till grafik, possession, death
  och cooldown innan vägen kan räknas som färdig.
- **Save/load och GAME_LOAD:** SKSAVE-formatet och återstående runtime-state-
  ownership kräver bredare originalcorpus och packaged-app-verifiering.
- **CCM och cell effects:** avancerad `DM2_PROCEED_CCM`, full cell-content-
  digest/map-change och teleporter-effekter är inte fullständigt implementerade.
- **V2.2 rendering:** riktig material-/pixelkonsumtion, clipping, fler
  djup/utomhus-rutter och runtime-wire-up saknas.
- **V2 HUD:** autentiska text-/bitmap-assets och fler widgets, bland annat
  inventory quick-view och action prompt, saknas.

### Saknad originaldata eller verifiering

- Weather behöver original save/memory snapshots som binder timer-recorden till
  rätt ägare och förändring.
- Full DOS framebuffer/blitter capture behövs för palette/clipping-parity.
- Ljudspåret kring MIDI och vissa runtime-sound owners saknar instruction-level
  trace och source-bound save/runtime-evidens.
- Demo- och icke-PC-extrakt behöver separat versions-/containerklassificering.

### Avsiktligt stängt

`examples/dm2_hud_widget_synthetic/` och procedural V2.2-art är endast
gate-fixtures. De får inte ersätta den riktiga DM2-GDAT-källan eller presenteras
som färdig Skullkeep-grafik.

**Native startmatris (verifierad 2026-08-27):** autentiska DOS-, Amiga-, FM
Towns- och Macintosh-ZIP:ar läses i minnet. DOS följer IBMIOP:s verkliga MVE
till SKULL och New Game; Amiga följer sin TITLE-ström till runtime; FM Towns
följer AUTOEXEC/TWANIM till GDAT:s New Game-rektangel och den källägda
championsvalsklickningen; Mac startar den verifierade retailprofilen. DM2 DOS
MVE kan dessutom verifieras headless med source-ordnade presentationsframes i
`dm2_v1_dos_native_cli_boot`. Detta är start-/runtimebevis, inte full
paritet för saves, rendering, AI eller audio.

## CSB

### Saknad originaldata eller evidens

- **CSBWin DSA-corpus:** ett checksum-validerat, DSA-bärande CSBWin-save med
  index/action-records behövs för positiv bredd.
- **Save-corpus per media/version:** ett verifierat original-save och
  round-trip-bevis behövs för varje påstådd media-/versionsgren.
- **Real capture:** bredare originalcapture av HUD, viewport, titel, dörrar och
  Mac/app-/övriga media-grenar saknas.
- **Audio:** full audio/runtime-parity med source-owned dispatch är inte klar.

### Saknad eller ofullständig funktionalitet

- Djupare end-to-end gameplay parity och playability utan DM1-antaganden.
- Fler DSA-timer-, generator-, teleporter- och sensortransaktioner med
  autentiska save/dungeon-par.
- Bredare source-locked viewport placements, masks, materialbindningar och
  draw-order för de CSB-specifika D2/D3-rutterna.
- V2.2 behöver en verklig PC34 GRAPHICS.DAT-baserad material/pixel-bindning
  innan modern art kan återaktiveras.

### Avsiktligt stängt

Syntetisk dungeon-loader/world-fixture och experimentella launch-fixtures får
fortsätta testa livscykel och negativa grenar. De är inte ett substitut för
CSB:s riktiga dungeon-, save- eller viewportdata.

**Native startmatris (verifierad 2026-08-27):** Firestaff läser verkliga CSB-
medier i minnet och genomför originalets title → runtime → första rörelse,
samt startmenyns launch-handoff, för Atari ST (rå STX och kapslat ZIP), Amiga
ADF-arkiv och FM Towns-CD-arkiv (engelska och japanska programkedjor).
`csb_v1_{atari_stx,atari_nested_zip,amiga,fmtowns_{en,ja}}_native_cli_*`
bevisar dessa vägar. CSB hade ingen DOS-utgåva; en begäran om `--platform pc`
avvisas därför fail-closed och får inte återanvända Atari- eller CSBWin-data.
Startbeviset ersätter inte de öppna originalcapture- och DSA/save-paritets-
kraven ovan.

## Nexus

### Saknad kod eller ofullständig produktionsväg

- **Structure2-material:** full descriptor-, UV-, textur- och palette-semantik
  saknas för att binda råa descriptors till renderbara material.
- **Animerade material:** payload-grammar, sequence semantics, flags och
  timing är inte kompletta.
- **Saturn runtime/capture:** äkta executable-/emulatortrace och frame capture
  krävs för pixelposition, mode, palette och timing. Statisk ISO-inspektion
  räcker inte.
- **Audio:** CUE-deklarerade retail-BIN-spår är nu source-bound; Saturns
  dispatcher, native decoder, literal sample/trigger-evidens och playback
  saknas fortfarande.
- **Structure1F/VDP1:** flera material-, texture/palette- och replay-gates är
  fortfarande capture- eller host-route-beroende.

### Saknad originaldata eller verifiering

- Den kompletta Saturn-körningen, inklusive BIOS/emulator/capturekedja, är den
  huvudsakliga externa luckan.
- Originala `LEV*.DGN`, `SLEV*.BIN`, `SNDLEV*.SAL` och `*.MNS` finns lokalt,
  men råa bytes räcker inte som bevis för semantisk material-, gameplay- eller
  animationstolkning.

### Avsiktligt stängt

Genererade DGN/DMDF/save-fixtures får användas för parser- och round-trip-
tester. De får inte befordras till en spelbar Nexus-värld eller användas som
Saturn-pixelbevis.

## Theron

### Saknad originaldata eller verifiering

- **Save body layout:** SRM/save-korrelation och full body-layout är inte
  tillräckligt source-locked.
- **Startup media:** autentisk decoding och pixel-evidens för Track 02:s
  startup-art, text och audio saknas.
- **Post-$3800-kedjan:** fortsättningskonsumenten efter den autentiserade
  `$3800`-gränsen behöver ytterligare live capture.
- **JP runtime:** JP-specifik media-/captureverifiering och vissa offsetfrågor
  kvarstår; de får inte härledas från US-data.
- **Bredare originalcorpus:** fler äkta CUE/BIN/ISO-kombinationer behövs för
  versions- och media-bredd.

### Saknad eller ofullständig funktionalitet

- **JP runtime är begränsad:** den autentiska JP Rev 1 CUE:n når native titel
  → stage → Soul Room → Akutuba och kan source-binda Drator, men detta är inte
  bred gameplay- eller övergångsparity.
- Full source-owned semantik för senare nivåer, objekt, champion-data och
  save/load är inte klar.
- Autentiska runtime traces behövs för dörrar, pits, teleporters, altar,
  combat, drops och sounds utanför de redan verifierade level-0-/table-slicarna.
- Full Track 02-bitmap/material-decoder och produktionsbunden viewport/UI-
  presentation saknas.
- V2.2 saknar ett autentiserat Track 02-materialpaket; procedural/AI-art ska
  förbli fixture-only.

### Avsiktligt stängt

No-op-, fixture-start- och synthetic parser-vägar får inte skapa en ersättnings-
level när autentiska Track 02-poster saknas. Produktion ska vara capture-gated.

## Gemensam prioriteringsordning

1. Skaffa eller verifiera den saknade autentiska save-/media-/capture-källan.
2. Bind bytes till rätt originalfunktion, record owner och runtime-route.
3. Lägg till source-lock och real-data-regression.
4. Lägg till original-vs-Firestaff pixel- eller timingpar där funktionen är
   visuell.
5. Låt osäker eller saknad data fortsätta fail-closed; skapa inga syntetiska
   saves, frames, rosterdata eller material för att fylla luckan.
