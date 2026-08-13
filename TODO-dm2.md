# Firestaff TODO - DM2

## Senaste verifiering (2026-08-13)

- ✅ FM Towns M12 archive-launch är nu verifierad med originalets
  `Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip`: disc image, AUTOEXEC:s
  SWOOSH→TITLE→SKULL→END-ordning, TWANIM/P3, EN/DL/PL-strömmar och English
  `GRAPHICS.DAT` från DOS-arkivet bindas helt i RAM. Den gemensamma DM2-roten
  resolverar dessutom DOS `data`-symlink, Amiga-arkiv, FM Towns-arkiv och
  Mac-arkiv till respektive source-owner.

- ✅ FM Towns M11:s privata GAME_LOAD hade redan materialiserat första
  `STARTEND`-hjälten. Mirror-klicket försökte därför välja samma autentiska
  root en gång till och avvisades som dubblett. Handoff-kedjan använder nu
  nästa `selected_mirror_count`-ordnade rosterpost för FM Towns, med samma
  autentiska mirror-direction och source-map-gate som M12; inga host-
  koordinater eller testteleport används.

- ✅ Efter title→NEW GAME använder M11 nu GAME_LOAD:s autentiserade privata
  preselection-viewport: G1-squares, GRAPHICSSET-scene/light och väggmaterial
  går genom den riktiga DM2-renderaren innan mirror-eventet committar c_hero.
  Titelmenyn ligger inte längre kvar visuellt under champion-förvalet; ingen
  party eller runtime-tick publiceras före source-owned mirror-handoff. Den
  breda startup-gaten jämför nu också framebuffer före/efter NEW GAME och
  kräver att titelbilden faktiskt ersätts. Förvalets renderpass kör med
  source-material-gate och statisk GDAT-HUD; ersättningsmaterial är blockerat.

- ⚠️ `sksave1.dat`-Resume är nu kontrollerad mot den källbundna WIELD-formeln:
  det autentiska vapnet `0x1407` ger i den valda hjälte-/skill-/load-state:n
  cirka 43 effektiv styrka före weapon power, vilket ger nollskada mot de
  första autentiska drop-creaturerna (type 25, armor 80). WIELD-gaten förblir
  därför korrekt röd (`launch=1, prepare=1, commit=1, drop=0`); den får inte
  öppnas genom att sänka armor, höja skada eller fabricera en weapon/drop.
  Nästa positiva fixture kräver ett autentiskt save eller en senare retail-
  position med ett source-vinnande vapen/creature-par.

- ✅ FM Towns M11-real-media-gaten är reproducerad med den autentiska
  HME-242-lösroten `/Users/bosse/.firestaff/data/dm2/fmtowns_iso` och
  PC-English companion som RAM-only virtual path
  `Dungeon-Master-II-Skullkeep_DOS_EN.zip::data/graphics.dat`. Rätt indata
  passerar startup, NEW GAME, inventory, rörelse, pit/trappa/DB1 och aktiv
  DB4→0x22→THINK_CREATURE-runtime; en ren ZIP-sökväg eller DOS-roten som
  companion väljs nu automatiskt till den källägda `data/graphics.dat`-medlemmen
  och hashverifieras utan extraktion.

- ✅ SKSAVE Resume är nu verifierad genom den riktiga atomiska
  `clone → retain → commit`-vägen, inte bara via CLI-handoff: alla fyra
  savefiler i `/Users/bosse/Downloads/dm2` publicerar en autentiserad live-
  kandidat. Korpustestet passerar 437 kontroller utan fel.
- ✅ Resume-kandidaten kopierar nu den autentiserade c_map-dungeonreceipten
  privat. Underlay-klonen lånar inte längre den kompakta `state.dungeon`
  utan mapmetadata; det förhindrar nollade kartdimensioner efter kloning.
- ✅ Source-ordnad `RESET_CAII` nollställer DB4:s byte@5 före static/dynamic
  fill och återställer exakt föregående state vid rollback. De dynamiska
  DB4-posterna kan därför materialiseras lazy och följa med i kandidaten.
- ✅ Den fokuserade DM2-regressionen passerar 10/10 valda DOS-, Amiga-,
  FM Towns- och Mac-real-media/runtime-gates.
- ✅ Occupancy-fixturen beskriver nu explicit den autentiserade V5/Rect14-
  vägen, och ISO-cache-fixturen jämför virtual paths samt macOS:s `/private`
  path-alias korrekt.
- ✅ Den publika title→NEW GAME-handoffens UI-owner är nu source-ordnad:
  NEW GAME materialiserar DUNGEON/CAII/GAF/local-context/timer-kedjan och
  första autentiska champion-steget privat, men committar inte runtime före
  den efterföljande mirror-eventen. Den bredare M11-startprofilgaten passerar
  därför utan host-fabricerad party eller för tidig session-publicering.
- ✅ Den faktiska Firestaff/M11-CLI:n kan nu Resume:a alla fyra primära DOSBox-
  saves från `/Users/bosse/Downloads/dm2` mot DOS-roten i
  `.firestaff/data/dm2/dos_extract`: `sksave0.dat`–`sksave3.dat` når
  `dm2-runtime`, `levelLoaded=1`, fyra champions och tickande runtime. Detta
  är starkare evidens än den äldre read-only-korpusproben, som fortfarande
  korrekt rapporterar att dess isolerade CAII-census saknar sparade
  0x21/0x22-matchningar. Full längre save/runtime-regression och native saves
  för Amiga, FM Towns och Mac återstår.
- ✅ En längre CLI-regression kör nu 32 boot frames, fem riktiga input-events
  (`right,up,left,down,action`) och 192 script frames per save. Alla fyra
  savesen behåller aktiv runtime och når tick 384 utan process-/state-fel;
  den direkta `firestaff --boot-probe`-körningen verifierar dessutom
  `levelLoaded=1`, fyra champions och respektive autentiska map/pose för
  `sksave0.dat`–`sksave3.dat`.
- ✅ M11:s DM2-spellpanel är nu kopplad till den source-owned cast-ägaren:
  panelens rune-buffer överförs till live `c_hero`, `CAST_SPELL_PLAYER` körs
  och missile spells går genom den autentiserade DB14/0x1E-transaktionen.
  DM2-cast från verklig UI-input är därmed inte längre en DM1-only-fallthrough.
- ✅ Mac M11-real-media verifierar nu Fireball efter source-owned rörelse:
  big-endian DB14-länkar/objektord, empty-tile ground-stack-insert och
  efterföljande 0x1E-publicering commit:as utan syntetisk tile-teleport.
- ✅ DOSBox `sksave0.dat` verifierar nu samma M11-spellpanel och Fireball-
  handoff efter autentiserad Resume. Resume-spegeln väljer source-owned
  `curacthero` från M11:s levande hjälteval; en död sparad actionhjälte får
  inte blockera casten.
- ✅ Den första source-owned `DM2_STEP_MISSILE`-rörelsen accepterar nu även
  dynamiska DB14-poolposter utan materialiserad rå DB14-post. Poolkedjan är
  primär ägare för passage, tile-root och energy-drain; råa länkar speglas
  bara när motsvarande source-record finns. Creature-hit/despawn använder nu
  samma pool-authoritative unlink och DB14-objektordet är endian-korrekt.
  DOSBox- och Mac-fixtures går faktiskt igenom 0x1E efter cast. Full
  reflection och alla avancerade map-/CCM-effekter är fortfarande separata
  gates.
- ✅ Nästa-ruta-creature-hit följer nu `c_tim_proc.cpp:795-802`: Fireball
  kontrollerar destinationens DB4 efter rörelsen, anropar
  `ATTACK_CREATURE` med attackord `0x2006` och skadeargument `0`, och tar
  sedan bort DB14:n via pool-authoritative `CUT_RECORD_FROM`. Mac-real-media
  verifierar destination-hit, reaktionsanropet och projektilkonsumtion.
  Projektilens HP/drop-väg går nu vidare till den separata
  `WOUND_CREATURE`-ägaren när creature-timern återplaneras på autentiserad
  aktuell karta.
- ✅ Den separata think-timer-bindningen är nu källtroget införd: spelarens
  `ATTACK_CREATURE`-skada ligger kvar i CAII `word@0x14`, och
  `DM2_THINK_CREATURE` flyttar den vid due timer till DB4 `record+6` innan
  lethal `WOUND_CREATURE`-logik. Kill-flaggan går vidare till den befintliga
  `DELETE_CREATURE_RECORD`-/dropägaren; icke-dödlig skada och dying-mode är
  också bundna. Mac-real-media verifierar nu `HP 30→25` genom faktisk
  `THINK_CREATURE`→`WOUND_CREATURE`; lethal/death/drop och positiv DOS
  real-media-träff i pit-saven står fortfarande kvar som öppna kontroller.

## Senaste source-koppling: projektilträff (2026-08-13)

- ✅ Projektilskapandet följer nu `CAST_CHAMPION_MISSILE_SPELL` och
  `DM2_SHOOT_ITEM`: kraften skrivs som DB14 byte@4, `0x5A` som byte@5 och
  mana-justerad accuracy som timersteg. Den tidigare stamina/skill-formeln
  är borttagen.
- ✅ Projektilträffens attackvärde går genom den källbundna
  `DM2_move_075f_06bd`-beräkningen och därefter CAII med attackord `0x200D`,
  styrka `100`; den separata `0x2006`-reaktionen ligger kvar efteråt.
  HP/death/drop ägs fortsatt av due `THINK_CREATURE`/`WOUND_CREATURE`.
- ✅ Mac M11-real-media verifierar nu en positiv Fireball-träff: källans
  `DM2_move_075f_06bd` ger ett positivt attackvärde, CAII word@0x14 ändras
  och DB14-projektilen konsumeras. Fixturen använder ingen syntetisk HP-
  skrivning.
- ✅ Mac M11-real-media verifierar nu hela positiva
  `CAII→THINK_CREATURE→WOUND_CREATURE`-handoffen: Fireball ger `HP 30→25`
  med faktisk skada `5` efter kartmedveten timeråterplanering.
- ✅ Samma Mac-fixture verifierar nu källans lethalgren: Lightning ger
  `HP 3→1`, `lethal=1`, och creature:n går in i source dying-mode eftersom
  den autentiska type25-posten saknar kill-flagga. Efterföljande 96 source-
  ticks deallokerar den inte, vilket är korrekt; delete/drop för en
  kill-flagged creature är fortfarande en separat positiv gate.
- ⚠️ DOS-pitens aktiva WIELD-kast missar fortsatt creature-typen och visar
  `hits 0`; positiv WIELD-skada/death/drop är ännu inte verifierad.

## Arbetsdata

- ✅ Mac pointer-input skickar nu exakt rendererat `c_rwbb`-target-index till
  den autentiserade wall-owner:n; den får inte reduceras till en kolumn. Den
  lokala DB3-listrotationen behåller alla övriga records i tilekedjan.
  En positiv real-media-klickregression återstår efter source-owned rörelse
  till en tile där en Mac-mekanism faktiskt är synlig.
- ⏳ Mac retail NEW GAME börjar på map 0 `(1,8)` och censusens verifierade
  lokala DB3-switchar ligger på map 2. En source-owned teleporter/map-transition
  krävs innan positiv musklikks-evidens kan köras mot dessa mekanismer; ingen
  koordinat- eller testteleport får öppna vägen.
- ✅ Den generiska DB1-teleporterägaren är nu kopplad efter autentiserad
  rörelse, via `DM2_move_2fcf_0434`; den är inte Mac-specifik och fabricerar
  ingen övergång. Mac-startens positiva wall-pointer-regression återstår tills
  en source-valid route når map 2.
- ✅ En autentiserad Mac/retail transition-fixture verifierar nu den nya
  runtime-committen positivt; den separata NEW GAME map 0→2-vägen är fortsatt
  öppen.
- ✅ En autentisk Mac fixture verifierar nu positiv DB1-transition map 11
  `(0,0)` → map 1 `(2,3)` genom den riktiga runtime-rörelsen. Den separata
  Mac NEW GAME map 0→2-vägen är fortfarande inte bevisad.
- ✅ Dörrinput är nu kopplad till source-timerkön och klass-4-handlern.
  Autentisk Mac-data verifierar hela bounded `0x04 → 0x01`-kedjan från en
  angränsande party-position och state 4→0. Ljud, partyträff vid stängning
  och bredare dörrsemantik är fortfarande separata owners.
- ✅ Runtime-normaliseringen skiljer nu bytekartans källklasser från
  2-byte-formatets enum: byte `2` är grop, `3` trappa, `4` dörr och `5`
  teleporter. Den tidigare gemensamma mappingen kunde läsa en grop som dörr
  och en teleporter som grop; real-media-regressionerna för Mac, Amiga och
  FM Towns passerar efter korrigeringen.
- ✅ Source-owned trappövergång är nu kopplad till normal party-rörelse:
  runtime bygger kartdeskriptorer från autentiska world-offsets, använder
  `DM2_LOCATE_OTHER_LEVEL`-motsvarigheten och committar map/pose atomiskt.
  Oupplösta eller utanförliggande mål blockeras. Mac real media verifierar
  en positiv övergång map 8 `(11,18)` → map 1 `(8,11)`.
- ✅ Alla fyra lokala editioner verifierar nu en positiv source-owned trapp-
  route genom normal party-rörelse och samma `DM2_LOCATE_OTHER_LEVEL`-
  semantik: DOS map 8 `(12,1)` → map 2 `(12,1)`, Amiga map 8 `(11,17)` →
  map 1 `(8,11)`, FM Towns map 8 `(12,1)` → map 2 `(12,1)` och Mac map 8
  `(11,17)` → map 1 `(8,11)`. Oupplösta mål och testteleport förblir
  blockerade.
- ✅ Öppen pit/fall-transition är nu kopplad till normal rörelse med
  `DM2_query_19f0_124b`-regler: bytekartans klass-2, bit 3 öppen och bit 0
  fri, därefter source-locatorns destination. Mac, Amiga och FM Towns har
  positiva real-media-transitions; vägg/inaccessible-landningar blockeras.
- ✅ DOSBox-korpusen har nu en separat real-media runtime-fixture: den
  autentiska DOS-roten commit:ar en öppen pit-transition map 4 `(6,4)` →
  map 1 `(4,4)` genom samma source-owned rörelseowner.
- ✅ DOS och Amiga verifierar nu autentiska DB1-mål och rotationsordning i
  aktiv runtime. Amiga-route map 38 `(15,17)` → map 38 `(16,4)` bevisar
  dessutom att 68k-recordord måste läsas big-endian; den gemensamma
  record-word-API:n använder nu editionens autentiserade byteordning.
- ✅ FM Towns M11-real-media verifierar nu DB1-transition map 38
  `(16,3)` → map 38 `(16,4)` inklusive source-rotation, utöver den redan
  verifierade pit-route:n.
- ✅ Efter FM Towns DB1-map-handoff produceras dessutom en source-owned
  viewport-frame; renderkedjan faller inte tillbaka till en host-grid.
- ⏳ Mac retail map 0-startens klass-5-aktorer är inventerade men inte
  ännu säkert tolkade: record `cc10` ligger på map 0 `(6,1)` och `(5,7)`.
  Med endian-korrekt source-dekodning är `w2=0x057e`, `w4=0x1084`, `w6=0x1160`;
  formen uppfyller inte DB1:s party-teleporter-gate och måste därför fortsatt
  behandlas som en DB3 local-action/legacy-form, inte som en gissad route.

- ✅ Mac retail NEW GAME når nu också en autentisk aktiv DB4/F9-creature i
  runtime: karta 2, cell `(14,14)`, type 5, med source-owned `0x22`
  THINK_CREATURE-dispatch och konsumerad CCM-body. Detta kompletterar DOS,
  Amiga och FM Towns; full dynamisk pathfinding, target-val och death/action-
  owners är fortfarande separata gap.
- ✅ Party-rörelsen avvisar nu live DB4-celler före positionscommit och gör ett
  source-bundet melee-försök mot den aktiva handen: CMDSTR, itemets DBSPEC,
  creature-AI och `CALC_PLAYER_ATTACK_DAMAGE` verifieras, varefter träffen går
  genom `CAII_ATTACK_CREATURE`. Missar förblir blockerade steg. Push, kast,
  death/drop och full `skmove.cpp`-encounterklassning är fortfarande öppna.
- ✅ DM2:s action-panel sparar nu den autentiserade CMDSTR-slotten i
  `hero->handcmd[curactmode]` och accepterar både melee-action 1 och
  cast/missile-action 2. Det gör att vald hand kan återanvända samma source-
  command vid creature-kollision; faktisk missile-/death-/dropföljd är ännu
  separat arbete.

- Primär speldata ligger i `.firestaff/data/<game>`: `dm1`, `dm2`, `csb`,
  `nexus` och `theron`. En explicit `--data-dir` får fortfarande användas
  för testkörningar, men implementation och verifiering ska kunna hitta den
  här standardroten.
- DOSBox-savegames för DM2 ligger separat i `Downloads/dm2`; de är testdata
  och ska inte blandas ihop med FM Towns-media.
- ✅ M12 Quick Resume validerar nu source-formade `sksave*.dat` och lämnar
  exakt DOSBox-sökväg till DM2:s M11-handoff när en DM2-datarot är vald.
- ✅ Uttryckligt valt Amiga/Mac/FM Towns-arkiv prioriteras nu före sibling-
  arkiv i samma rot, så plattformsvalet behåller sin source-owned mediaägare.
- ✅ Macens source `c_rwbb` wall-target går nu från rendererad väggcell till
  autentiserad DB3-kedja för push-button och lokala switchar. Inkompletta,
  okända och item-krävande kedjor förblir fail-closed tills deras separata
  source-owner är bunden.

- ✅ DM2-007 klassar nu fasta spell-index 1 (`DES IR SAR`, Darkness) genom
  source GENERAL -> 0x46 light-timerfamiljen. Den fulla signerade timer-
  payloaden och production-owner:n är fortfarande separata fail-closed gränser.
- ✅ DM2-007:s spell-cast receipt separerar nu 0x46:s source timer-A från
  första due-tick: Long Light, Darkness och Light använder c_light:s
  source-delay och ett bounded signed table1d6702-steg. Oägda production-
  timergrenar är fortsatt fail-closed.
- ✅ DM2-007:s Aura/Enchantment-cast använder nu source 0x48
  `DM2_PROCEED_ENCHANTMENT_SELF`-familjen med self-target-mask och separat
  power i `value_a`; 0x47 hero-flag-countdown blandas inte längre in.

- ✅ Kandidatens 0x19-creature-target använder nu explicit CAII-attackcontext
  med privat AI/GDAT-provenance, CAII-allokering, timer-delete och scheduling.
  M11 har positiv type-7 poison-cloud damage/decay på en autentiserad creature;
  creature attackens privata think-timer och cloudens continuation ligger båda
  i kandidatens heap. Slot-, pool-, timer- och RNG-state snapshotas gemensamt.
- ✅ Cloud-receipten exponerar nu DB15 `word@2` korrekt: subtype ligger i låg
  7-bit och cloud-parametern/styrkan i högbyte. M11 verifierar både poison-
  `0x08→0x05` och `0x28` `0x38→0x10`; parametern hålls separat från timer-
  handle och resurrectionens `adddata(5)`.

- ✅ Den centrala `DM2_PROCEED_TIMERS`-matrisen erkänner nu source-runtime:s
  alternativa `0x1D STEP_MISSILE`-kod tillsammans med `0x1E`. Den tidigare
  registrerade runtime-handlern kunde inte nås eftersom `0x1D` hoppades över
  som okänd timer; missilegrenarnas övriga map-/collisionägare är oförändrat
  fail-closed.

- ✅ Den generiska timer-wiringen kopplar nu också `0x1D` till samma
  source-formade `STEP_MISSILE`-adapter som `0x1E`; wiring-count och
  handler-regressionen täcker båda koderna.

- ✅ DM2-007:s separata spell-timeradapter accepterar nu också source-runtime:s
  alternativa `0x1D STEP_MISSILE` och delar handler med `0x1E`; den tidigare
  aliaskoden kunde annars falla ur spellens dispatchgräns.

- ✅ CCM-runtime-anropet skickar nu `source_sleeping` som skproject:s
  `v1e0238` sleep/wake-lås. Den tidigare kopplingen använde `view_dir`, vilket
  kunde göra CCM:s pre-check riktningsberoende; den fulla production-owner:n
  för `DM2_PROCEED_CCM` är fortfarande fail-closed tills callbackkedjan är
  komplett.

- ✅ Runtime-gränsen för generisk `0x04 ACTUATE_TILE` avvisar nu 2-byte/Towns-
  kartor vid tile-class lookup. `c_tim_proc` väntar här på bytekartans
  `mapdat.map[x][y] >> 5`; 2-byte-formatets normaliserade lågbitar är inte
  samma actuator-klass och får därför inte dispatchas förrän en source-ägd
  map-class-adapter finns.

- ✅ DM2-007:s fasta 34-spelltabell avkodar nu source-objektseffekten från
  `w6`-fältets bitar 4–9. Därmed bevaras exempelvis Fireball `0`, Lightning
  `2`, Poison Bolt `6`, Poison Cloud `7` och summon-selektorerna `0x31/0x34/0x35`;
  extended/custom-spells utan sådan owner förblir unavailable. Själva DB14-
  projektil-, DB4/CAII-summon- och slutliga UI-owners är fortfarande separata
  fail-closed gränser.
- ✅ DM2-007 klassar nu Spell Reflector (index 12) som source cloud creation
  med `OBJECT_EFFECT_REFLECTOR`-selector `0x0e`, enligt
  `SkWinCore.cpp:17762`; den passerar inte längre felaktigt genom hero-
  enchantmentens `0x48`-gren.

- ✅ DM2-007:s summon-timer använder nu samma source-payload som runtime:s
  autentiserade `0x5e ALLOC_NEW_CREATURE`: `value_a` packar mål-x/y och
  `value_b` är creature type `0x31/0x34/0x35`. Den tidigare reducerade formen
  med y i `value_b` och effekt i `reserved` kunde aldrig nå DB4-ownern.

- ✅ Produktions-`spell_cast_player_apply` köar nu den verifierade `0x5e`-
  summonformen för de tre kända creature-typerna. Runtime-castet skapar även
  Poison Cloud som source DB15-record + `0x19`-timer med atomisk
  pool/raw/timer/sound-rollback. Reflector-subtyp `0x0e`, projektiler och
  övriga cloudfamiljer köas fortfarande inte utan sina respektive owners.

- ✅ DM2-007:s fasta general-gren skickar nu Spell Reflector (index 12) till
  source cloud-grenen i stället för den tidigare felaktiga hero-enchantment-
  grenen.
  Poison Cloud (index 14) följer samtidigt den faktiska fasta postens
  `w6`-klass `MISSILE` och behåller objekt-effekt `7`; runtime-castets
  DB15-cloud-ägare använder den fasta objekt-effekten som cloud-subtyp.
  Reflector-cloudens klassificering är source-korrekt men dess produktion-
  creation/lifecycle är fortsatt stängd tills `0x0e`-ägaren är verifierad.

- ✅ Produktions-`0x5e ALLOC_NEW_CREATURE`-admissionen är nu atomisk: en full
  eller avvisad source-timerkö återställer mana, hand-cooldown, runor, flask-
  state och köinnehåll före någon publicering. Cloud-cast använder nu samma
  atomiska DB15/0x19-gräns; DB14-projektiler är fortsatt spärrade.

- ✅ Produktions-castet avvisar nu Light/Aura/Enchantment/Cloud/Projectile
  före resource-writeback när deras source-timer-owner saknas. Summon-castet
  kräver både känd creature-selector och en faktisk source-timerkön; ingen
  lyckad spell får längre konsumera mana/runor utan en publicerad effekt.
- ✅ Runtime `0x19 PROCESS_CLOUD` avvisar nu också reflector-subtyp `0x0e`;
  den får inte gå genom poison-cloudens vanliga decay/requeue-owner innan
  source incoming-spell/bounce-semantik är bunden.
- ✅ Runtime har nu en source-owned `DM2_CAST_SPELL_PLAYER`-transaktion för
  Long Light, Darkness och Light. Den läser den överförda `c_hero`-runsträngen
  och wizard-skillen, kör `DM2_PROCEED_LIGHT`, köar den källberäknade `0x46`
  med exakt due-tick, och skriver mana/cooldown/rensade runor först efter
  lyckad köning. Hjälte, timerkö och source light-level återställs vid köfel.
  Aura/enchantment och summon har sourceägda cast-callers; DB14-projektiler,
  custom/item-grenar och cloudens inkommande reflector-consumer är fortsatt
  fail-closed.
- ✅ Samma runtime-transaktion äger nu de tre source-verifierade shield-spells:
  Spell Shield (party mask `0x0f`, aura `2`), Magical/Spell Shield (self,
  aura `1`) och Fire Shield (self, aura `0`). `c_hero::ench_power`, aura,
  mana, hand-cooldown och runor publiceras först efter en ny `0x48`-timer.
  Vid aura-byte kapas de berörda gamla `0x48`-timer-maskerna atomiskt.
  Invisibility och reflector-cloudens inkommande spell-consumer saknar
  fortfarande sina respektive source owners. Aura of Speed har nu en separat
  skrivbar `savegames1.b_04`-owner i färsk GAME_LOAD; SKSAVE Resume måste ännu
  överföra sin autentiserade snapshot till samma owner.
- ✅ Fyra stat-auror är nu också bundna genom source `ench_aura - 2`-mappingen:
  Wisdom→5, Dexterity→4, Vitality→6 och Strength→3, med party-mask `0x0f`
  och samma atomiska `0x48`-power-timer. Aura of Speed är fortsatt spärrad:
  den använder den separata globala `savegames1.b_04`-ägaren och får inte
  reduceras till en hero-aura.

- ✅ Kandidatens source `savegames1.b_02` är nu samma runtime-owner för både
  engage-commandens attack-counter och 0x47 hero-flag-countdown. Full SKSAVE
  Resume måste fortfarande överföra blocket tillsammans med den övriga
  skrivbara sessionen.

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## Active Cycle 16 Jobs (DM2 only — continuous operation)

Per directive: DM2 only, auto mode. Lanes pull jobs from this file; the
orchestrator keeps them fed, assembles, and pushes. Fix synthetic paths when
real game data is available; batch small jobs into larger ones. Source-lock
against skproject (SKULLWIN/SKWIN); keep fail-closed where evidence is
missing. Do not push — the orchestrator pushes after assembly. Update this
file and DONE.md after every completed job.

## Senaste DM2-bindning

- ✅ GAME_LOAD-kandidaten har nu en privat `0x1E STEP_MISSILE`-owner för den
  autentiserade creature-fria ordinary-passagegrenen: DB14-recordets byte
  `@6`/`@7` måste äga aktuell timer-slot, både pool- och raw-mirror-kedjan
  prewalkas, energi och continuation skrivs atomiskt, och terminal energi
  kapas/dealloceras i samma rollback. Teleporter, impact/reflection,
  creature-collision och cross-map-följder är fortsatt fail-closed.
- 🔒 De åtta PC-DOS-saves i `Downloads/dm2` innehåller inga `0x1E`-timers.
  M11/save-load passerar därför ägar-/dispatch-regressionerna men kan inte ge
  retail-positiv missile-evidens ännu. En negativ M11-regression täcker nu att
  oägt DB14/timerpayload lämnas kvar utan mutation.

- ✅ Creature-combatens alternativa poison-helper exporterar nu en unik C-
  symbol; runtime fortsätter använda den source-sized ops-ownern. M11/M10
  kan därmed länkas utan duplicate-symbol-konflikt.
- ✅ Creature-AI-loadern kräver nu exakt `CREATURES word@5` för typen→AI-row-
  bindningen. En annan CREATURES-entry, till exempel animationdata, kan inte
  längre öppna en okänd aidef genom att råka se ut som en owner.
- ✅ `0x5E`-recycler-censusen budgeterar nu både primära DB-pooler och
  autentiserade PC G1-extension spans, så en DB4-kandidat efter ObjectID-
  gränsen inte kapas bort av read-only-walken.
- ✅ Samma census failar nu stängt vid korrupt DB-länk, mid-chain
  `OBJECT_NULL` eller avklippt/cyklisk kedja efter budgetgränsen; en tidigare
  observerad DB4-kandidat kan inte ligga kvar som giltig efter en sådan skada.
- ✅ Den separata `DM2_DELETE_CREATURE_RECORD`-kompositionen verifierar nu
  hela tile-rooted-kedjan före någon mutation, räknar även PC G1-extension
  spans och stoppar vid mid-chain `OBJECT_NULL`. En regression visar att
  invoke, cut, drop och dealloc lämnas orörda vid skadad tail.
- ✅ Delete-kompositionen snapshotar nu pool, rå dungeon, timer, CAII och RNG
  före den första mutationen. Om invoke, cut eller possession-drop faller
  efter admission rullas hela DB4-delete-slicen tillbaka; fokustestet täcker
  den tidigare delvis muterade post-cut/drop-fail-vägen.
- ✅ `DM2_DROP_CREATURE_POSSESSION` räknar nu extension spans, avvisar
  `OBJECT_NULL` i possession-kedjor och preflightar hela possession-graphen
  före generated drops/RNG. En korrupt tail kan därför inte lämna en delvis
  muterad drop-cell; regressionen täcker att både RNG och ground-head förblir
  orörda.
- ✅ Drop-ägaren har nu också en explicit context-variant för AI-flaggor.
  Kandidatens privata CAII/GDAT-owner kan därmed användas utan att låna
  processglobal creature-state när recycler-transaktionen kopplas in.
- ✅ `DM2_DELETE_CREATURE_RECORD`-kompositionen har nu samma explicita
  context-bound AI/GDAT-gräns. Den använder context-ägaren även genom
  possession-drop. GAME_LOAD `0x5E` använder nu denna owner mot kandidatens
  privata CAII/GDAT och timerheap; invoke-message byggs som kandidatens riktiga
  12-byte `DM2_TimerEntry`, inte via processglobal state eller en parallell
  source-kö. Allokeringen försöker sedan om efter lyckad dealloc.
- ✅ `0x5E`-censusens `delete_inputs_ready` kräver nu en matchande CAII-slot
  när DB4-recordet har byte@5 != `0xff`; en felkopplad slot rapporteras inte
  som verifierad delete-indata.
- ✅ SKSAVE:s generiska recycler-walk räknar nu också PC G1-extension spans
  och avvisar `OBJECT_NULL` som tile-chain-tail. Den muterande dynamic-detach-
  vägen gör dessutom en full read-only prewalk före första unlink; prewalken
  accepterar nu delade suffix först efter att de verifierats i sin helhet,
  men avvisar fortfarande cykler inom den aktuella pathen. Detta matchar den
  autentiska DOS-kartan där flera tile-roots delar exempelvis DB3-tail `0x0D12`
  och håller retailkartor inom M11:s watchdog.

- ✅ Cloudens DB3-actuator-scan skiljer nu source `OBJECT_END` (`0xfffe`) från
  `OBJECT_NULL` (`0xffff`). Saknad eller ofullständig tile-/record-callback
  avvisas före dereferens; regressionen täcker null-chainen.
- ✅ Samma scan gör nu en read-only förwalk före första actuator-invoke. Cykel,
  null-länk eller saknad invoke-owner kan därför inte lämna en delvis muterad
  DB3-kedja; regressionen täcker den saknade invoke-ownern.
- ✅ `DM2_CALC_CLOUD_DAMAGE` avvisar nu saknad DB0-target, creature-AI-owner
  eller RNG-owner före skade-/resistanscallback. En positiv creature-regression
  täcker den saknade AI-ownern.
- ✅ Runtime `0x19` kräver nu poolens DB15-record och autentiserad membership i
  tile-rooted chainen innan requeue, decay eller dealloc. Om en separat rå
  record-spegel finns måste dess subtype matcha; dynamiska DB15-slots får
  source-korrekt använda poolbytes plus den gemensamma råa länken.
- ✅ Runtime-cloudens admission förwalkar nu hela tile-chainen till
  `OBJECT_END`, inte bara prefixet fram till DB15. `OBJECT_NULL`, saknat
  record och avklippt tail avvisas före cloud-effect eller lifecycle-mutation.
- ✅ Kandidatens privata `0x19`-owner gör nu samma kompletta, tile-rooted
  DB15-chain-preflight före både `0x64→0x65`-skrivning och slutlig `0x65`-cut.
  En `OBJECT_NULL`-tail lämnar timerkö, pool och mapstate orörda; M11 täcker
  den rollback-säkra negativa vägen.

- ✅ `DM2_CREATE_CLOUD` skriver nu DB15 word@2 source-korrekt: subtype och
  omnidirectional-bit ligger i lågbyte/låg-7-bitar, medan styrkan ligger i
  högbyte. Den tidigare OR-skrivningen kunde blanda in styrkan i subtype och
  därmed göra exempelvis `0xff80` till en ogiltig cloudtyp. Regressionen
  verifierar både `0xff81` och den riktiga spread-cloudens `0xff80`-layout.

- ✅ `0x19 PROCESS_CLOUD` använder nu timer-B som DB15-record i dispatchen
  (timer-A är endast x/y). Recordets word@2 styr subtype och lifetime enligt
  SKProject: `0x07` minskar high-byte med 3 från `>=0x0600`, `0x28` minskar
  high-byte med `0x28` från `>0x3700`, och `0x64` stegar low-7-bitars värdet
  samt köar GEN2-ljud `(0x0d, subtype, 0x81, 0xfe, ..., 0x6c, 0xc8)`.
  Dörrskada använder nu tile-record som damage-target och source mode `1`.
  Full `DM2_CALC_CLOUD_DAMAGE`/combat-owner är fortsatt fail-closed när
  callbackägaren saknas.
- ✅ När den befintliga kompletta `DM2_V1_CloudCallbacks`-ownern finns
  materialiserad väljer dispatch-wiringen den direkt, inklusive
  `DM2_CALC_CLOUD_DAMAGE`, poison-resistance, combat och viewport-dirty.
  Den smala `_tile`-slicen är kvar som explicit fallback och krockar inte
  längre symboliskt med den fulla cloud-ownern.
- ✅ `DM2_CALC_CLOUD_DAMAGE` följer nu source-kontrollflödet där
  cloud-subtypen, inte target-recordtypen, väljer type-2-halvering,
  type-3-AI-flagga och type-7-poisonformel. Poison-resistance körs därefter
  endast för DB4-target. En positiv cloud-regression täcker subtype `0x07`.
- ✅ Cloudens creature-parameter är nu source-korrekt: DB4-targetens byte@4
  extraheras först och skickas som creature-typ till AI-spec, AI-flaggor och
  poison-resistance. DB4-handtaget används inte längre som typparameter.
- ✅ Runtime `0x19` har nu en source-gated dörr-undergren för vanliga cloud-
  typer: en autentiserad DB0-dörr utan party/creature får DB15-skadan räknad
  med source-RNG och kan skrivas till DESTROYED tillsammans med cloudens
  decay/requeue eller chain-cut i samma rollback-transaktion.
- ✅ Samma runtime-undergren kan nu applicera cloudens DB4-creature-skada
  genom den befintliga CAII/`DM2_ATTACK_CREATURE`-ownern, med creature-typens
  AI/GDAT-parameter, think-timer och RNG i samma rollback-transaktion.
- ✅ Runtime `0x19` kan nu applicera party-skada genom source-
  `DM2_ATTACK_PARTY`: DB15-skadan använder source-RNG:n, varje levande hjälte
  får väntande `damagesuffered` och `HERO_FLAG_0800`, och party/RNG/cloudens
  övriga state rullas tillbaka gemensamt vid fel. Saknad `c_party` är fortsatt
  fail-closed; `0x64→0x65`-fasen med GEN2-ljud, requeue och slutlig
  chain-cut/dealloc är separat bunden.
- ✅ Runtime `0x19` accepterar nu source-lifecycle för cloud-subtyper `0..7`
  och `0x28` utöver `0x64/0x65`. Vanliga clouds får source-korrekt effect-pass
  och terminal cut; poison `0x07` requeuear med styrke-decay och `0x28`
  requeuear med `0x28`-decay utan `0x64`-ljud. Saknade effektägare förblir
  fail-closed.
- ✅ M11:s runtime-cloud-fixture väljer nu den autentiserade partycellen när
  den är en source-kompatibel no-DB3-kedja. Det verifierar `0x64`-cloudens
  lifecycle på den riktiga party/map-owner:n, men öppnar inte full party-,
  creature- eller dörrskada.
- ✅ Den privata GAME_LOAD-`0x19`-ownern följer nu samma lifecycle för
  autentiserade tomma golvceller och den vanliga party-targeten: `0x07`
  requeuear med `-3`, `0x28` med `-0x28`, och party-skada använder kandidatens
  DB15-styrka/RNG genom `DM2_ATTACK_PARTY`. Pending damage, `HERO_FLAG_0800`,
  party och RNG rollbackas gemensamt. En autentiserad DB0-dörr kan nu få
  source cloud-skada och DESTROYED-state med råkarts-rollback; creature-
  effekter och spread är fortsatt fail-closed.
- ✅ Runtime `0x3C/0x3D` accepterar nu source-party-sentinel `0xFFFF` för
  samma-map vanlig golvruta utan creature. Position, timer och GEN1-ljud
  återställs atomiskt vid fel; cross-map, specialrutor, actuator- och
  wake/sleep-följder är fortsatt fail-closed.
- ✅ Cloudens generiska skade- och processgräns avvisar nu saknad/ofullständig
  callback-owner fail-closed före recordläsning, dörranrop, requeue eller
  deallocation. Runtime har separata source-gated owners för dörr, creature
  och party; den generiska callback-baserade runtime-combat-ownern och övriga
  cloud-effekter är fortfarande inte en enda publicerad owner.
- ✅ `DM2_CREATE_CLOUD` har nu en positiv source-regression för `0xff80`:
  spread-skadan träffar party på ursprungstilen, viewport-dirty sätts och
  ursprungsstilens DB3-lista skannas med universal `0x26`-matchning.
  Actuatorns source-action och nollvärde verifieras; kandidatens separata
  `0x0D yB==2`-kedja är nu också positiv för type `0x26`.
- ✅ Kandidatens `0x0D yB==2` blockerar nu endast en autentiserad, matchande
  DB3 type-`0x26` enligt cloud-spellens `w2/w4`-gates. Orelaterade DB3-records
  i origin-chainen överblockerar inte längre cloud-creation. Invoke-mutationen
  för type `0x26` är nu bunden source-korrekt till `once-only`-biten och
  action `0/1/2`; övriga actuator-effekter är fortsatt fail-closed.

- ✅ Generiska `0x0D RESURRECTION`-slicen följer nu SKProject exakt för
  fasrequeue: DB-recordens unsigned handle jämförs mot timer-`xB`,
  `ADD_ITEM_CHARGE(record, 0)` jämförs mot actor, och cloud-anropet använder
  `(0xffe4, 0, xA, yA, xB)`. `yB-1` skickas explicit till queue-callbacken;
  cloudens `adddata(5)` är separat från cloud-parametern. Regressionen täcker
  både DB2-handle, altar-cut och cloud-payload.

- ✅ Sourceaudit av `0x3C/0x3D PROCESS_TIMER_3D` är klar. Runtime äger nu den
  autentiserade DB4-undergrenen med source-chain-sökning, map-/destination-
  gate, rollbackad `MOVE_RECORD_TO`, CAII-think-timeruppdatering och GEN1-
  ljud. Kandidaten äger dessutom en mirror-verifierad plain-floor cross-map-
  undergren med atomisk pool-/råkedje-cut och append. Real-media-proberna
  dispatchar nu den producerade 0x3c-timern på DOS, Amiga, FM Towns och Mac;
  alla fyra commit:ar same-map DB4-flytten genom sina autentiserade råkedjor.
  BE-recordord läses nu source-korrekt i raw `CUT/APPEND`, och WALK_NOW:s
  autentiserade källcell förs över eftersom 0x3c-payloaden bara innehåller
  destination och record-handle.
  Optional CAII AI-flag-tails faller fortsatt closed när editionens data inte
  kan autentiseras. Runtime, party-sentinel, wake/sleep och
  actuator-tails är fortsatt fail-closed; `0x5D` har en separat same-map
  party-owner. SKProject läser
  `xA/yA` som destination, `B` som record-handtag och kör
  `MOVE_RECORD_TO(record, -3, 0, x, y)`. Därefter köas GEN1-ljud endast när
  flytten lyckas eller timertypen är exakt `0x3D`, med source-trippeln
  `(3,0,0x89)` och ljudparametrarna `0x61/0x80`. `0x3E` är inte en separat
  timerägare i den verifierade `c_tim`-kedjan. DOSBox-korpusen saknar dessa
  moverec-timers; Firestaff håller därför familjen stängd tills full
  link/wake/sleep-, map- och ljudtransaktion finns.

- ✅ Den generiska timer-wiringen binder nu både `0x3C` och `0x3D` till samma
  source-payload: `valueA` dekoderas som packad destination `(x,y)` och
  `valueB` som unsigned record-handle. Den tidigare omkastade adaptervägen är
  korrigerad; ljud-callbacken krävs uttryckligen så en lyckad flytt inte kan
  följas av ett NULL-anrop. Runtime-undergrenen är nu bunden för autentiserade
  DB4-records på aktuell karta samt en separat same-map party-sentinel på
  vanlig golvruta; kandidatägaren har dessutom en spegelverifierad plain-floor
  cross-map-undergren. Runtime har samma bounded spegel-/mapägare i koden,
  men M11-fixturen saknar en positiv DB4 på annan karta; runtime-cross-map
  hålls därför fortsatt fail-closed tills positiv runtime-evidens finns.
  Wake/sleep-owner och atomisk actuator-tail återstår.

- ✅ Den generiska `0x5D`-adaptern använder nu timerpostens mapbyte från
  `ticks_and_map` i stället för hostens aktuella map, och kräver både
  party-moverec- och rotationscallback innan den kör. Regressionen verifierar
  source-sentinel `0xffff`, source-map-gate, destination och rotationsordning;
  GAME_LOAD/runtime-owner för full partykedja är fortfarande stängd.

- ✅ Den privata GAME_LOAD-kandidaten har nu en source-gated DB4-undergren för
  `0x3C/0x3D`: timer-B måste vara en befintlig DB4 med CAII-think-timer,
  recordets autentiserade ground-chain söks på aktuell karta, och
  `MOVE_RECORD_TO`, CAII timeruppdatering samt GEN1 `(3,0,0x89,0x61,0x80)`
  körs i samma rollback-transaktion. M11 har en positiv real-map regression.
  En separat party-sentinel-owner accepterar nu endast samma-map vanlig
  golvruta utan creature och håller pose/timer rollback-säker; static-
  allocation, malformed unrelated chains, cross-map, wake/sleep och full
  actuator-tail öppnas inte.

- ✅ DOSBox-regressionen räknar nu 0x3D- och 0x5D-moverec-timers över alla
  åtta PC-DOS-saves och kräver uttryckligen noll förekomst. Om framtida
  retaildata tillför en sådan timer måste den få en ny autentiserad owner;
  den får inte passera genom en syntetisk dispatch.

- ✅ Sourceaudit av `0x5D MOVE_RECORD_ROTATE` är klar. Timern kräver samma
  mapmatchning som party-kartan, packar destination/riktning från `timer.A`,
  skickar party-sentinel `0xFFFF` genom den verifierade same-map-
  `MOVE_RECORD_TO`-ägaren och roterar först efter godkänd flytt. Runtime
  uppdaterar party-position samt heroernas partypos/absdir atomiskt på
  verifierade vanliga golvrutor.
- ✅ Runtime-`0x5D` avvisar dessutom en autentiserad DB4 på destinationen
  före någon pool-, map- eller party-mutation. Party-sentinel-ownern kan
  därför inte gå över en creature som om rutan vore tom.
- 🔒 DOSBox-korpusen innehåller ingen `0x5D`-post i någon av de åtta saves.
  Cross-map, actuator-följder och övrig full `MOVE_RECORD_TO`-semantik är
  fortsatt fail-closed tills deras separata owners är verifierade.
- ✅ Den generiska source-slicen för `0x5D` dekoderar nu destinationens y från
  timer-A enligt `(A << 6) >> 11`; den tidigare hårdkodade y=0-vägen är borta.
  Runtime använder samma packning och den atomiska same-map party-owner:n.

- ✅ Sourceaudit av kandidatens `0x5E ALLOC_NEW_CREATURE` är klar. `xA/yA`
  är spawnkoordinater, `yB` är creature-typen och source använder health-
  multiplier `7` samt `RANDDIR` eller `CALC_VECTOR_DIR` före
  `ALLOC_NEW_CREATURE`. Runtime och kandidat binder nu den nya DB4-roten,
  CAII-initiering och timerägarskap i samma placementtransaktion. Recycler,
  cross-map och saknad source-AI/GDAT-owner förblir fail-closed.
- ✅ Runtime-dispatchen binder nu `0x5E` till en source-formad direkt-
  free-slot-owner när AI/GDAT-owner, aktuell karta och DB4/CAII/sound-state
  finns. Den reducerade spell-delegaten är borta; health/RNG, DB4-root,
  tile-chain, CAII/think-timer, `0a48` och GEN1-ljud rollbackas tillsammans.
  Source-recycler, cross-map och saknad AI/GDAT-owner är fortsatt
  fail-closed.
- ✅ Runtime- och GAME_LOAD-`0x5E` avvisar nu en redan creature-rotad
  destination före RNG, DB4-allokering, CAII- eller timer-mutation. Det
  skyddar source-cellens collisiongräns även när en fri DB4-slot finns.
- ✅ Samma owner avvisar nu partycellen som spawn-destination före allokering.
  M11 täcker både full-poolens befintliga fail-closed och den nya party-
  collisiongränsen med kvarlämnad timer/state.
- ✅ Runtime-M11 använder nu dessutom en autentiserad AI-owner i den negativa
  party-collisionregressionen; avslaget beror inte längre bara på saknad
  AI/GDAT-owner.
- ✅ Kandidaten har nu en rollback-säker direkt free-slot-väg för `0x5E`:
  DB4-roten, source-HP/RNG, tile-chain, CAII och första think-timer binds i
  samma privata transaktion. DOSBox-korpusen har inga lediga DB4-slots, så
  regressionen verifierar full-pool-fallets fail-closed-gräns när retail-
  delete-indata saknas. `0a48`-ljudets GEN1-owner är nu snapshotad och bunden med
  source-klasserna `(0x0f, creature_type, animation_index, 0x46, 0x80)`;
  full-pool-fallet gör dessutom en read-only source-census av DB4-kandidater.
  Census-receipten kvitterar nu första berättigade DB4-handle och map/cell,
  samt recordets CAII-slot, slotmatchning och pending timer som läsbar
  provenance. Den läser nu också possession-root, AI-flaggor, `CREATURES`
  word@1 och drop-slots direkt ur kandidatens autentiserade GRAPHICS.DAT-owner;
  den visar därmed possession-root, kända AI-flaggor och om
  GDAT:s 11 drop-slots samt `CREATURES` word@1 för invoke-gaten är bundna.
  När alla dessa och tile-root/CAII-gates är positiva går kandidaten nu genom
  den context-bound, transaktionella delete/dealloc-adaptern och försöker
  därefter `ALLOC_NEW_RECORD` igen. En positiv retail-spawn återstår som
  separat korpusbevis eftersom de tillgängliga DOSBox-saves saknar 0x5E-timer.

- 🔒 Den tidigare empty-creature-recyclerproben är stängd igen. Source-
  censusen kräver `word@2 != 0xffff` före `DELETE_CREATURE_RECORD`, vilket
  gör possession-root `0xffff` oåtkomlig i den här admissionen. Full
  retail-evidens för en positiv DB4-recycling/spawn saknas fortfarande, men
  den privata delete/dealloc- och timer/CAII-transaktionen är nu bunden.

- ✅ Kandidatens CAII-slot+2 är uttryckligen bunden som ett index i
  kandidatens `DM2_V1_TimerQueue` (inte som source-köns stabila ticket).
  Den fullständiga recycler-transaktionen använder denna CAII-owner,
  context-bundet possession-drop, invoke-gate, tile-rooted delete och
  dballoc/dealloc i samma rollback-gräns. Timerplats 0 behandlas korrekt
  som “ingen aktiv timer”; ett giltigt ticket är aldrig 0.

- ✅ DOSBox-censusen räknar nu explicit source-`0x5E ALLOC_NEW_CREATURE` över
  alla åtta PC-DOS-saves och finner noll poster. Det är en evidensgräns, inte
  en positiv spawn-regression; DB4-delete/recycling förblir stängd.

- ✅ DB4-delete-ägaren är map-aware på samma sätt som source `c_map`:
  creature lookup, tile-chain membership, ground-stack cut och possession-
  append använder den autentiserade kandidatens karta. GAME_LOAD-recyclern
  binder dessutom samma karta, privata CAII/timer-owner och rollback-
  transaktion. En positiv retail-spawn/recycling-körning saknas fortfarande
  eftersom de tillgängliga DOSBox-savesen inte innehåller `0x5E`-timerdata.

- ✅ Kandidatens privata `0x15 PROCESS_SOUND` följer nu source-payloaden:
  timer A väljer en autentiserad delayed-slot, map-gate jämför slotkartan med
  kandidatens sound-kartor, och `DM2_QUEUE_NOISE_GEN1` körs mot den dynamiska
  GAME_LOAD-tabellen. Slot, timerheap och sound-state återställs vid saknad
  binding eller köfel; M11 har en positiv slot-/timerregression.

- ✅ Runtime `0x1D/0x1E STEP_MISSILE` läser nu source-payloaden korrekt:
  `timer.A` autentiseras som DB14-handtag, `timer.B` som x/y + energisteg,
  recordets bytes 6–7 måste matcha source timerarray-indexet och recordet
  måste ligga på den kodade tile-kedjan. No-creature-grenen gör source-formad
  energiförbrukning och terminal cut/dealloc med rollback. Creature-grenarna
  `ABSORBS_MISSILE` och `REFLECTOR` använder nu samma terminala owner. Vanlig
  HIT räknar nu attacken via `DM2_move_075f_06bd` och applicerar den via
  source-bunden `DM2_ATTACK_CREATURE`/CAII med GDAT- och RNG-admission samt
  rollback; `NONMATERIAL` och full `DM2_MOVE_RECORD_TO` utanför den nya
  direkta teleporter-undergrenen är fortsatt fail-closed. `TURNS_MISSILE`
  går genom samma source-hit-owner; dess riktning flippar inte före träffen
  enligt `c_move.cpp`. En vanlig passagecell flyttar nu DB14 både i
  runtime-poolen och den råa dungeon-kedjan innan `0x1E` requeue. Detta gäller
  även `NONMATERIAL` utan skada; endast source-klasserna FLOOR/PIT/STAIRS
  släpps igenom här, medan DOOR/TRICK_WALL/MAP_EXIT förblir stängda.

- ✅ Runtime DB14 har nu en separat, rollback-säker direkt-teleportergren för
  source-verifierade riktningsändringar.
  `GET_TELEPORTER_DETAIL` och `map_3BF83` validerar destinationen; därefter
  flyttas DB14 i både source-poolens kedja och båda kartornas råa
  ground-stack-kedjor innan continuation-timern får ny karta/position.
  Saknad destination, creature på målrutan eller ofullständig kedja lämnar
  hela energisteget, poolen, dungeonbilden och timerheapen återställda.

- 🔒 Creature/actuator-följder och övriga `MOVE_RECORD_TO`-följder är
  fortfarande stängda tills deras fulla map-owner är verifierade.

- 🔒 Den återstående DB14-teleporteradmissionen gäller creature/actuator-
  följder och annan full `DM2_MOVE_RECORD_TO`-semantik. Den nya direkta
  undergrenen använder en verklig map-/record-/timertransaktion och är inte
  en förenklad koordinatflytt.
- ✅ Den direkta DB14-teleporterflytten verifierar nu hela avresans chain före
  mutation, inte bara prefixet fram till missile-recordet. Raw-recordlänkar
  kräver dessutom den autentiserade recordstorleken vid omskrivning.

- ✅ Kandidatens privata `0x04 ACTUATE_TILE` bevarar nu source-klass-3:s
  avsiktliga tomma gren. Efter due-pop och map/cell-admission konsumeras
  timern som no-op, utan syntetisk actuator- eller kartmutation; regressionen
  går genom den privata timerheapen.

- ✅ Kandidatens privata `0x0E PROCESS_TIMER_0E` använder nu source A/B-
  kontraktet: DB-typ väljer autentiserad itemrot, B väljer temporär itemtype,
  source `PROCESS_ITEM_BONUS` körs mot c_party/c_hero och itemrecordet
  återställs byte för byte. Timer, party, map och record rullas tillbaka om
  item-/GDAT-owner saknas. Positiv direct-root-regression är bunden till
  M11-profilens riktiga itemdata.

- ✅ Tidslinjen kan avboka ett DB14-refererat source timerarray-index när det
  finns exakt en levande ägare. Tvetydiga index lämnas orörda och kan därför
  inte välja fel missiltimer.

- ✅ Runtime-committen kräver nu samma `source_transaction_hash` i den
  privata runtime-kandidaten och dess autentiserade `GameLoadWorldOwner`.
  Hash-tampering testas rollback-säkert; ingen kandidat kan publiceras från en
  annan GAME_LOAD-transaktion.

- ✅ Den read-only runtime-handoff som föregår committen använder samma
  source-owner-matchning. Tampering lämnar bootens kandidat kvar och ändrar
  inte runtime eller `source_game_load_session_ready`.

- ✅ Sourcekontrollen av `DM2_ALLOC_NEW_RECORD` visar att den anropar
  `DM2_RECYCLE_A_RECORD_FROM_THE_WORLD` efter en full pool även för DB2, och
  Firestaffs privata GAME_LOAD-owner följer nu den begärda DB-typen genom
  Text-barriären. En autentiserad direkt DB2-post nollställs source-ordnat
  genom samma allokeringsbrygga; skyddad Text och ofullständiga kartkedjor
  förblir fail-closed. Korpusen når nu nästa source-gräns (DB3/DB0) i stället
  för att stanna vid DB2. Full DB3/DB14 unlink/delete-owner och ytterligare
  retailpositiv evidens återstår; DB4-delete/recycling är nu privat bunden.

- ✅ `PROCESS_ITEM_BONUS` följer nu source-rutinen för återställda icke-item-
  objekt: DBSPEC-frågan ger noll och bonussteget blir no-op i stället för att
  kräva en syntetisk DB5–DB10-klassning. Strikt itemägarskap ligger kvar för
  klassning, vikt, containerinnehåll och GDAT-bonusar. Med den korrigeringen
  når alla åtta DOSBox-saves kartfasen; kvarvarande fel är source-ägda
  DB0/DB2-allokeringar i `READ_SKSAVE_DUNGEON`.

- ✅ DB0-recyclerns AI-förscan avvisar inte längre en fristående DB4-post med
  saknad AI-row. Source frågar AI-ägaren först när karttraverseringen faktiskt
  går in i den statiska varelsens possession-tail; de riktiga kandidatposterna
  unlinkas och allokeras därefter. DOSBox-korpusen når nu DB2/Text-spärren i
  alla filer som klarar direktrötterna. DB2 får fortfarande inte återanvändas.

- ✅ Direktrötternas source-läsare kan nu dela GAME_LOAD:s autentiserade
  privata `c_map` med `ALLOC_NEW_RECORD` när DB0-poolen måste återvinnas.
  Den tidigare map-fria diagnostikvarianten finns kvar, men den riktiga
  GAME_LOAD-ordningen använder kartägaren före item-bonusfasen. Korpusen visar
  därefter exakt vilka filer som fortfarande faller på icke-item-rootar
  (DB0) eller DB2/Text; de rootarna öppnas inte genom en påhittad itemtabell.

- ✅ DB0:s source-recycler har nu en källbunden tile-chain-cut före
  `ALLOC_NEW_RECORD`: den valda posten tas bort från den autentiserade
  ground-stack-kedjan och nollställs först därefter. Kandidater som ligger i
  statiska varelsers possession-kedjor förblir spärrade tills deras DB4-delete-
  och CAII-ägare finns. Regressionsprovet verifierar både unlink och
  `OBJECT_END`-markering; skyddad DB2/Text återanvänds inte, medan en
  source-godkänd direkt DB2-post nu hanteras av den separata allokeringsgrenen.

- ✅ DB0-commiten använder nu samma klonade map-/record-owner för cut,
  predecessor-unlink och `ALLOC_NEW_RECORD`-clear. Den tidigare risken att
  c_map publicerades muterad medan poolklonen behöll den gamla länken är
  regressionssäkrad med en DB4-föregångare.

- ✅ DB2-allokeringsgränsen kvitterar nu både autentiserade fria slots och
  source-ordnade direkta DB2-recyclerreturer. DOSBox-korpusen visar att fulla
  DB2-pooler kan gå vidare genom Text-barriären utan att skyddade Text-poster
  återanvänds eller Resume öppnas. DB3 och DB7 har nu source-godkända
  direkt-returer. DB5, DB6, DB8, DB9 och DB10 följer nu också sina
  source-gated `MOVE_RECORD_TO(..., -1, ...)`-cut-only eller direkt-return-
  grenar, inklusive byte-3-skyddet där source har det. DB13/DB14/DB15 och
  ogiltiga poolreferenser lämnas fail-closed; nästa ägare är DB14:s
  missile-delete samt de återstående unsupported-poolgränserna. En första
  DB14-admission gate avvisar nu rutor med DB4-creature utan att öppna en
  ofullständig delete-transaction.

- ✅ Runtime har nu en source-gated `0x5A CONTINUE_ORNATE_NOISE`-adapter. Den
  läser source-payloadens DB3-handle i `valueB` och x/y i `valueA`, skiljer på
  inaktiv frame-clear och aktiv vägg/golv-decoration, använder bootens
  ornamentlängd, köar GEN2-ljud best-effort och requeuear med samma
  animationskadens. Record, timerkö och ljudkö återställs om requeue misslyckas;
  M11 verifierar nu den aktiva GAME_LOAD→runtime-dispatchen mot en riktig
  commit-karta och en separat DB3-record, samt den source-ordnade
  animationstakts-continuationen utan 0x55-ägarkollision. GEN2:s fjärde
  sourceklass är korrigerad till `0xFE` i både kandidat och runtime.

- ✅ Kandidatens `0x55 CONTINUE_ORNATE_ANIMATOR` är nu positivt verifierad med
  en riktig DB3-record, source-layoutens övre ornamentindex och autentiserad
  ornamentlängd. Timern avancerar frame och requeueas vid `+1` tick utan att
  tolka record-handlebytes som tilekoordinater.

- ✅ Runtime har nu en source-gated `0x55 CONTINUE_ORNATE_ANIMATOR`-adapter
  som använder DB3-handle, timerpayloadens wall/floor-mode, mapens
  wall/floor-GDAT-lista och bootens ornamentlängd. Tile/map-admission,
  frame-step och `+1`-requeue återställs vid köfel. M11 verifierar nu en
  positiv GAME_LOAD→runtime-dispatch med aktiv DB3-record och den följande
  source-ordnade continuation-timern.

- ✅ Runtime `0x46 LIGHT` använder nu kandidatens överförda source light-owner
  och samma verifierade 16-stegs ljuskurva som `c_tim_proc.cpp`. Signed
  timeramount, map-admission, lightdelta och `+8`-requeue är atomiska; den
  äldre champion-proxykonteksten används inte i runtime.

- ✅ Runtime `0x47 HERO_ENCH_FLAG` är nu bunden till GAME_LOAD:s överförda
  `savegames1.b_02`/`v1e0976`-owner och source-sized `c_hero`. Countdown,
  one-based target och `heroflag 0x4000` på levande hero följer kandidatens
  source-semantik; ogiltig map/target avvisas utan host-substitut.

- ✅ Runtime `0x4B POISON` är nu bunden direkt mot source-sized `c_hero`.
  Den verifierade `v1e0288`/`source_next_champion_number`-ägaren följer med
  GAME_LOAD-handoff; saknad eller ogiltig owner, fel hjälte eller tom poison-
  counter avvisar timersteget. Wound, `poison`/`poisoned`, `heroflag 0x2800`
  och fortsatt `0x24`-timer följer kandidatens source-kedja med rollback vid
  köfel. M11-regressionen använder en source-formad poison-counter på en
  riktig överförd hero.

- ✅ Runtime `0x48 ENCH_POWER` är nu bunden direkt mot den överförda
  source-sized `c_party/c_hero`-ägaren. Actor-mask, signed decrement, levande
  hero-filter och klampning till noll följer kandidatens source-kedja; den
  äldre `DM2_ChampionRecord`-delegaten får inte skriva över runtime-handlern.
  M11-regressionen verifierar en positiv ändring på en verklig överförd hero.

- ✅ Runtime `0x56 TICK_GENERATOR` är nu bunden till den source-owned DB3-
  actuator-recorden. Source-semantiken läser kontroll/alternating-state,
  köar `0x04 ACTUATE_TILE` med recordets riktiga x/y/direction och requeuear
  nästa `0x56` atomiskt. Den generiska `0x04`-konsumenten är fortfarande
  fail-closed tills dess fulla DB3/DB14-owner är verifierad. M11-regressionen
  använder en source-formad DB3-generator byggd från kandidatens riktiga
  recordpool.

- ✅ Runtime `0x0E PROCESS_TIMER_0E` är nu bunden till source-sized
  `c_hero`/inventarieägaren. Den temporära itemtypen går genom den befintliga
  source-trogna `PROCESS_ITEM_BONUS`-kedjan med klassning från record-poolen
  och DBSPEC från boot-ägd GRAPHICS.DAT. Hero-statistik och vikt uppdateras
  atomiskt; saknad record/GDAT/viktkedja eller en ljusbonus utan verifierad
  global ägare avvisar hela operationen.

- ✅ M11-regressionen använder nu en verklig source-itemrot från kandidatens
  recordpool och verifierar 0x0E:s temporära itemtype-morph, source-owned
  `PROCESS_ITEM_BONUS`-anrop och återställning av itemrecordet. Timerfallets
  source-mode `0xFFFE` stänger MP/ability-slingorna; en separat DOSBox-post
  med faktisk aktiv skill/walkspeed-bonus behövs fortfarande för en positiv
  statdelta-regression.

- ✅ En separat riktig M11-profil passerar nu den positiva GAME_LOAD→runtime-
  committen efter timer/sound-preflight. `source_game_load_session_ready`
  publiceras med source-party pose, och både första source-ordnade tick och
  första source-owned frame verifieras efter commit.

- ✅ Kandidaten konsumerar nu source-`0x55 CONTINUE_ORNATE_ANIMATOR` för
  autentiserade DB3-actuatorer: `xA/yA` identifierar recordet, `wvalueB`
  väljer animationens vägg/golv-läge, GDAT-längden hämtas via kandidatens
  lokala grafiklistor och frame-steget plus `+1`-tick-requeue sker atomiskt.
  Felaktig map, record, decoration eller GDAT lämnar timer och record
  oförändrade.

- ✅ Kandidaten konsumerar nu source-`0x5A CONTINUE_ORNATE_NOISE` med korrekt
  payload: `valueA` är `(x,y)` och `valueB` är actuator-recordet. Aktiv arm
  väljer vägg/golv-decoration från tileklass och actuator-word, läser GDAT-
  längden, köar GEN2-ljud best-effort och requeuear samma timer med den
  source-bundna animationstakten. Inaktiv arm behåller sin frame-clear.

- ✅ GAME_LOAD har nu en explicit source-bunden GEN2-ljudadapter som behåller
  den dynamiska `xsndptr2`-tabellen och transaktionellt speglar c_sfx-köerna.
  `0x0D yB==2` kan nu skapa en bounded cloud på kandidatens dynamiskt
  materialiserade DB15-owner när origin-chainen saknar DB3-actuator, och
  `0x19` processar den verifierade `0x64`-armen med source-requeue samt
  deallocerar cloud-recordet source-korrekt efter `0x65`.

- ✅ `0x4B POISON` är nu bunden mot kandidatens riktiga `c_party/c_hero` och
  privata c_tim-heap: giftfält, pending wound, source-flaggor och requeue med
  delay `0x24` ändras atomiskt. Kandidaten kopierar nu också source
  `ddat.v1e0288` och lämnar endast den source-matchade sista hjälten
  fail-closed.

- ✅ Kandidaten konsumerar nu source-`0x0D` RESURRECTION-finalfasen `yB==0`:
  hjälten återställs med source max-HP-straff, halverad HP, `heroflag
  0x4000`, rensade possessions/enchantments och rollback.

- ✅ `0x0D yB==1` är nu bunden: source DB10 hero-bones/charge matchas i den
  riktiga tile-chainen, kapas och deallokeras atomiskt med `0xffff` marker.
  `yB==2` har också en positiv narrow owner för autentiserad type `0x26`:
  source action `0/1/2` och once-only-bitens mutation ingår i samma rollback
  som DB15-cloud, timer och ljud. Övriga actuator-effekter och full runtime-
  DB3-owner återstår.

- ✅ Kandidaten har nu en privat source-`0x0C` PROCESS_TIMER_0C-consumer:
  `hero.timeridx` nollställs och `heroflag 0x0800` sätts för levande hero.
  Ogiltig actor lämnas kvar i heapen.

- ✅ Kandidaten har nu en privat source-`0x02` DESTROY_DOOR-consumer: efter
  map/cell-admission sätts tilets låga tre statebitar till destroyed-door `5`
  och timerposten konsumeras atomiskt. Ogiltig cell lämnas kvar i heapen.

- ✅ Runtime commit binder nu också source-`0x02` direkt i
  `DM2_PROCEED_TIMERS`: map hämtas från timerns `ticks_and_map`, koordinaten
  från `valueA`, och en ogiltig map/cell avvisas utan att party-map ändras.
  Positiv post-commit-regression kör timern mot den riktiga DOSBox-världen.

- ✅ Runtime commit binder nu `0x58/0x59/0x5B/0x5C` mot den autentiserade
  recordgraphen. `0x58` använder payload A, `0x59` payload B och de två
  övriga payload A; ogiltig record eller timer-map ger ingen host-mutation.
  En riktig post-commit `0x5C`-timer verifieras i M11-regressionen.

- ✅ Runtime commit binder nu source-`0x01 STEP_DOOR` för en autentiserad
  direkt DB0-root: timer-map och koordinater, `actor`-riktning, klass-4-cell,
  komplett recordgraph och frånvaro av DB4/party-kollision krävs. Tile-state,
  DB0-riktning/moving/queued-bitar och source-requeue följer den verifierade
  dörrsteget. Ljud och full moverec-/collision-semantik är fortsatt stängda.

- ✅ Runtime commit binder nu source-`0x0C PROCESS_TIMER_0C` mot den
  överförda source-sized c_party/c_hero-ägaren. Timer-map och hero-index
  valideras, `timeridx` rensas som 16-bitarsfält och levande hero får
  `heroflag 0x0800`; ogiltig map/hero ger ingen mutation.

- ✅ Runtime binder nu alla tre `0x0D RESURRECTION`-faserna mot source-owner:
  phase zero följer c_hero-formeln för max-HP/HP, possessions,
  `heroflag 0x4000` och aura/power; phase one validerar DB10 hero-bones och
  kapar altar-recordet med rollback; phase two skapar DB15 type `0x64`, köar
  source `0x19` och `0x19` avancerar clouden till type `0x65` med source
  requeue och slutlig chain-cut. Sound är best-effort och fulla cloud-effects
  utanför denna verifierade lifecycle är fortsatt stängda.

- ✅ Class-4 `0x04`-actuation kan nu, efter bounded direct-DB0-admission och
  chain-kontroll, köa en privat `0x01` door-step med source action/direction.
  DB0-recordens verifierade runtimebitar för riktning, moving och queued
  uppdateras atomiskt och återställs vid rollback. Destroyed-door konsumeras
  som explicit no-op; party/DB4-kollision, ljud och övrig door-semantik
  förblir stängda tills deras källägare är verifierade. Kandidatgaten
  konsumerar nu också det köade `0x01` med samma privata record-/timerowner.

- ✅ Kandidaten har nu en privat source-`0x48` ENCH_POWER-consumer: actor är
  hero-mask, timerbytes 6–7 är signed decrement, levande heroes klampas till
  noll och döda heroes lämnas orörda. Party, timerheap och kartcontext rullas
  tillbaka atomiskt vid admissionsfel.

- ✅ Kandidaten har nu en privat source-`0x47` HERO_ENCH_FLAG-consumer:
  `savegames1.b_02` minskas en gång per pop och `heroflag 0x4000` sätts på
  den källvalda levande hjälten när räknaren går till noll. Ogiltigt mål
  rullar tillbaka timer, räknare, party och kartbyte atomiskt. `0x4B` är nu
  också source-bunden; aktiv `0x5A` är nu bunden med samma privata owner.

- ✅ Kandidaten har nu en privat source-`0x46` LIGHT-consumer med den
  verifierade ljuskurvan, signed `wvalueA`, source-delta och +8-ticks requeue.
  Både första steget och den fortsatta timerkonsumtionen testas privat; värden
  utanför källtabellens gräns förblir fail-closed.


## Skproject Audit (DM2)

- **SKPROJECT-DM2-FUNCTION-COVERAGE-2026-08-06:** The earlier informal
  “31 missing functions” count is stale. The current named-symbol audit
  (`docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`) records
  `DM2_SOUND1` through `DM2_SOUND7`, the applicable `c_move.cpp` paths, and
  all source-owned `c_map.cpp` paths as `IMPLEMENTED_PARITY` (1,118 total
  symbols). The remaining `c_dialog.cpp` and `c_eventqueue.cpp` entries are
  explicitly `NOT_APPLICABLE_ARCH`: they are DOS UI/event-loop owners
  replaced by M11, not callable game-data substitutes. Keep auditing the
  real M11/GDAT path for missing ownership; do not revive any retired
  callback transcript merely to reduce a function-count metric. **2026-08-06
  eventqueue correction:** the retained test transcript now matches the
  source `0x02`/`0x04` capacity edge, keyboard seven-entry cap and init-vs-
  flush sentinel split; it remains excluded from every production path.

- SKPROJECT-DM2-STARTUP-001 — `SKWIN/SkWinCore.cpp::SHOW_MENU_SCREEN`
  (`TITLE/0 dt07/4`): Firestaff now treats the menu as one static GDAT draw
  command owned by DM2 startup presentation; `TITLE/0 dt07/1` is retained as
  title/credit query receipt evidence, not a second host menu draw or
  synthetic overlay. Verification is now executable: the current
  `test_dm2_v1_m11_startup_profile_gate` passes against the hash-verified
  PC-DOS data and `firestaff --game dm2 --boot-probe` reaches the active
  `dm2-startup-menu` phase. An installed v3.0.288 app was inspected only as
  a stale external comparison, not as evidence for this v3.0.290 build.

- SKPROJECT-GAP-001 — `SKULLWIN/c_weather.cpp::DM2_SET_TIMER_WEATHER` and
  record OWNER (which saved record is the weather timer) and corpus traces
  with known weather transitions.

- SKPROJECT-GAP-002 — `SKWIN/DME.h::DistantEnvironment` fixes the ten-byte
  in-memory shape but not allocation owner, persistence location, or save
  encoding. Risk: ENVIRONMENT material could pair with stale slot bytes.
  Required: DOS memory/save snapshots across weather updates.

- SKPROJECT-GAP-003 — `SKULLWIN/c_sound.cpp` retains TODOs around MIDI calls,
  sample-state returns, and queue fields. Risk: voice lifetime/music semantics
  can diverge. Required: original executable trace and sound corpus.

- SKPROJECT-GAP-004 — `SKULLWIN/c_map.cpp` marks map globals and ground-stack
  table meanings unresolved. Risk: over-promoted G1 record/tile ownership.
  A raw-only G1 receipt now preserves verified column-index, ground-stack,
  and trailing map-data bounds, counts, and hashes from hash-verified corpus.
  A second raw-only receipt correlates every verified `Map_definitions` row
  to its bounded trailing-map span and hash, without assigning tile meaning.
  Both receipts intentionally leave the table and tile semantics absent.
  Direct DB0 and DB3 root receipts now read only their independently defined
  payload words after runtime admission; DB3 extension records and every
  `GenericRecord::w0` route remain unread and untraversed.
  Required: multi-map original DUNGEON.DAT corpus plus debugger traces that
  define `v1e03f4`, `dunGroundStacks`, and bit `0x10` beyond their observed
  indexing contract.

- SKPROJECT-GAP-005 — `SKWIN/DME.h` labels CCM `0x32..0x34` unknown. Risk:
  fabricated creature behaviour. The corpus receipt now hashes only verified
  `CREATURE_AI/row/dt00` AIDefinition rows, rejects all adjacent fields, and
  records no 0x32..0x34 stream bytes. Required: original opcode streams and
  instruction-level traces that bind a file/save owner and grammar.

- SKPROJECT-GAP-006 — `SKWIN/SkWinCore.h::_44c8_0f29` is unresolved blitting.
  Risk: local-palette clipping/mirroring differs despite decoded GDAT pixels.
  Required: original framebuffer captures and DOS blitter trace.

- SKPROJECT-GAP-007 — **The named-symbol inventory has no verified behavior
  mappings yet.** `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`
  records 1,751 skproject callable definitions: 142 exact-name candidates,
  1,540 missing names, 69 desktop-variant exclusions, and zero implementation
  claims. Risk: promoting a literal identifier collision as a DM2 port.
  Required: per-family call-path evidence, owned input/GDAT/save data, and a
  focused Firestaff regression before any `UNCERTAIN` row becomes
  `IMPLEMENTED`.

- SKPROJECT-GAP-008 — **Title/menu GAME_LOAD is now split by route.** The
  authenticated FM Towns CD path has a verified `SHOW_MENU_SCREEN` pointer
  route and a source-owned NEW GAME transaction: the real DUNGEON.DAT/GDAT
  pair is admitted, an authentic mirror is selected, and M11 receives the
  committed session before the first active frame. This is covered by the
  opt-in `dm2_fmtowns_m11_gameplay_real_media` regression. Resume remains a
  separate source boundary: the FM Towns corpus contains no SKSAVE, so no
  save-derived party or dungeon may be invented or imported from the DOS
  edition.

- SKPROJECT-GAP-009 — **Two skproject source files are presently unreadable
  locally.** `SKULLWIN/c_music_wav.cpp` and `SKULLWIN/c_rect.cpp` are retained
  as explicit audit sentinels rather than guessed symbols. Risk: treating a
  partial source tree as exhaustive coverage. Required: readable local source
  copies followed by a regenerated inventory and reviewed mappings.

- 2026-07-13 CSBWin restored `TT_60`/`TT_61` follow-up: only the exact
  party-square, non-Lord-Chaos `+5` successor is live before M10 can mutate
  `timerObj8`. Object movement, TT_61 sound, occupied-square checks, and the
  Lord Chaos random detour remain fail-closed without their CSBWin owners.

  - 2026-07-14 hardening: every queue-owned saved `TT_60`/`TT_61` receipt is
    now consumed before M10's incompatible C60/C61 group path. Unsupported,
    malformed, off-party-square, and Lord-Chaos shapes create no successor
    and retain no generic movement or sound behavior.

  - 2026-07-15 queue-retirement correction: the source-owned receipt is now
    neutralized only after the common F0239 extraction. Marking the queued
    event as `NONE` before extraction left the original TIMER in the heap and
    could duplicate the authenticated `+5` successor. The source gate remains
    fail-closed for every unsupported receipt.

- 2026-07-13 CSBWin restored `TT_22` follow-up: the imported restart timer's
  exact source no-op is live. Its original creation context and the removed
  historical restart work remain unavailable; do not infer a C22 action.

## DM2 V2.0 Runtime Follow-up (2026-07-13)

M11 now binds the selected DM2 V2 presentation mode to the persistent V2
that verified image route; do not derive overlays from Firestaff weather
enums or intensity.
live real-material renderer. Remaining V2.2 work is a renderer that consumes
clipping is still unproven. Remaining weather work is to bind real original

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- DM2-001 — `skproject/SKULLWIN/c_gdatfile.cpp` GDAT query/load path and `c_loadlevel.cpp` level materialisation: the hash-verified DOS EN/FR shared dungeon member is discovered and materialized through the normal scanner, and its typed GDAT ENT1 payload graph validates. PC G1 parsing bounds the real pre-map extension and exposes the proven `c_map.cpp` route: its 256-byte post-descriptor G1 block precedes the 480-word column-prefix table, which reaches the bounded 2360-word ground-stack table. The source-ordered `c_record.cpp` pool transform and DB3/DB4 continuation addresses are proven.

- DM2-002 — `skproject/SKULLWIN/c_dballoc.cpp`, `c_record.cpp`, `c_map.cpp`, and `c_moverec.cpp` database-record ownership: `src/dm2/dm2_v1_world_model.c`, `dm2_v1_world_state.c`, and `dm2_v1_runtime.c` retain reduced Firestaff records, including a stub save-state layout. Replace the parallel model with validated original record pools, links, maps, and relocation semantics.
  - **2026-08-05 c_move inventory correction:** the former
    `DM2_move_075f_1bc2` target-cell and `DM2_move_2c1d_028c` commit receipts
    were synthetic. In SKProject `c_move.cpp:2861` selects four candidate
    player positions using party state and `DM2_RANDBIT`; `:2914` searches an
    adjacent party member and returns its index or `-1`. Neither routine is
    collision nor movement commit. Both adapters now reject explicitly until
    the real party-position, RNG, and caller state are bound. Keep collision
    in the separately source-scoped runtime route; do not reuse these names
    to admit a DUNGEON.DAT movement result.

- DM2-003 — `skproject/SKULLWIN/c_timer.cpp`, `c_tim_proc.cpp`, `c_events.cpp`, and `c_eventqueue.cpp` timer order: `src/dm2/dm2_v1_timeline.c`, `dm2_v1_runtime.c`, `src/memory/`, and `src/engine/m11_game_view.c` do not execute the original timer-type matrix and still contain an M11 creature-tick simulation. Route every DM2 timer through a DM2-owned source-order dispatcher and remove host-side behavioural substitution.

  - 2026-07-21 update (round 23): the event-driven activation callers
    actuator subsystem), and the 1c9a_0247 dballoc tag system
    (host-owned preserved-GFX cache).
    game assets), zero new failures.  Remaining: runtime wiring of the

- DM2-004 — `skproject/SKULLWIN/c_input.cpp`, `c_keybd.cpp`, `c_tmouse.cpp`, `c_clickrect.cpp`, `c_buttons.cpp` UI event routing: `src/engine/m11_game_view.c`, `src/dm2/dm2_v1_startup_menu.c`, and `dm2_v1_inventory_panel.c` cover only bounded menu/viewport actions. The original `INTERFACE_GENERAL dt07/2` group spans are now materialized as typed primary/secondary/tail data; default door-button receipts now expose skproject `MAKE_BUTTON_CLICKABLE` rectnos 3/4 and reject custom wall-GFX buttons as non-clickable. The title-menu NEW path expands original `INTERFACE_GENERAL/0/dt04/0` rectangle `0xD7` and consumes it through M11; the hard-coded startup panel no longer accepts M11 clicks. The matching `0xD9` surface has a source-owned pointer receipt and is explicitly selector-unavailable, so it cannot fall through into a synthetic resume row. Both title actions now require the original primary mouse event; only the separate credits screen retains its common secondary-button dismissal. The title/menu indexed presentation now expands `dtPalIRGB`'s source 6-bit DAC channels to SDL's 8-bit RGBA after `DM2_CONVERT_DRIVERPALETTE`, while retaining raw GDAT palette bytes for receipts. Bind the original resume-selector state machine before it can create a resume action. Consume the remaining original click-rectangle, keyboard, mouse, held-button, and modal-dialog ordering. Unsupported controls must remain unavailable.

- DM2-006 — `skproject/SKWIN/c_creature.cpp` AI/death paths and `c_ai.cpp`: the bounded real-data chain `CREATURES[type] dtWordValue(0x05) -> CREATURE_AI row -> AIDefinition.w30/w32` is available as evidence for `DRAW_PUT_DOWN_ITEM`; it preserves the source w30 eligibility gate and still does not create a click target until owner records and rect expansion are both proven. Bind real GDAT AI records and reproduce source eligibility, possession, death, and cooldown ordering.

  - 2026-08-12 update: active FM Towns M11 now consumes an authentic
    DB4→CREATURES/type/F9 map-chip frame. Per-root admission keeps valid
    creature materials when an unrelated DB4 type lacks F9/AI evidence;
    those roots remain no-draw. Rect14 FB/FC/FD selection is restricted to
    the separately admitted V5/live-CCM path, so ordinary DB4 roots retain
    their source F9 identity. DOSBox and Amiga now exercise the same active
    DB4/F9 render path against their own original media. Full CCM animation,
    AI and death ownership remain open. The source creature scheduler now
    resolves the authenticated current map instead of hard-coding map 0;
    FM Towns M11 reaches a real map-3 DB4→0x22→THINK_CREATURE dispatch, with
    the CCM body still explicitly fail-closed pending the complete stream
    owner. The source-static `DM2_14cd_09e2` goal branch is now wired through
    the authenticated AI/table owner. The source `WALK_NOW` commands now
    pass only an authenticated adjacent-floor/map/occupancy gate and enqueue
    the existing 0x3c `MOVE_RECORD_TO` owner; full original A* path state,
    door/cloud/teleporter path semantics and other action dispatch remain
    gated. The authenticated adjacent melee branch now selects CCM 0x08 and
    dispatches through the source-port `CREATURE_ATTACKS_PARTY` /
    `CREATURE_ATTACKS_PLAYER` path into the live hero damage owner; ranged,
    death/drop and the remaining CCM action handlers stay gated.

  - 2026-08-12 follow-up: the production runtime now owns the first
    source-callback `FIND_WALK_PATH` slice for authenticated File_header maps.
    Creature ticks obtain the party target from the committed GameState,
    select an authenticated adjacent floor cell rather than the party's
    occupied cell, traverse authenticated floor/ornate-floor cells while
    rejecting occupied creature cells, and feed the first source direction
    into `WALK_NOW` and the existing `MOVE_RECORD_TO` owner. Two-byte Mac/FMTowns maps now use
    the editions-aware passage/solid adapters; door/cloud/teleporter path
    semantics and the complete original A* walk state remain gated. Real
    media probes now admit and queue the first dynamic move on DOS, Amiga,
    FM Towns and Mac.

  - 2026-07-23 update (Lane E, cycle 16): the real-data drop route is now
    up the defense/BaseHP route, DUNGEON.DAT door-record evidence for the
    door-destruction table, and ALLOC_NEW_DBITEM item-record creation.
    locally. Remaining: a CREATURE_AI-proven graphics session to light

- DM2-007 — `skproject/SKULLWIN/c_events.cpp` `DM2_TRY_CAST_SPELL`, `DM2_FIND_SPELL_BY_RUNES`, `DM2_CAST_SPELL_PLAYER`, and `DM2_PROCEED_SPELL_FAILURE`: `EXTENDED_LOAD_SPELLS_DEFINITION` is a bounded GDAT `SPELL_DEF` receipt over exact dtWordValue fields 1-7 plus dtText field `0x18`. The fixed original table, live rune lookup and fixed-table `w6[9:4]` object-effect decode are now source-exact; DB14 projectile creation/stepping, DB4/CAII summon creation, remaining timer owners and final UI feedback remain unbound. Unsupported custom object effects must remain unavailable rather than use a spell-index mapping.

  - 2026-08-06 update: M11 binds the SDL backend only after
    `dm2_v1_boot_startup_launch_alloc_with_language()` succeeds, then
    unbinds it in `M11_GameView_Shutdown()`. The real-data M11 startup gate
    now covers both sides, preventing a verified DM2 backend from leaking
    into an unverified or later game launch.

- DM2-012 — `skproject/SKULLWIN/c_item.cpp`, `c_hero.cpp`, `c_dialog.cpp`, and `c_engage.cpp`: `src/dm2/dm2_v1_inventory_panel.c`, `dm2_v1_shop.c`, `dm2_v1_companion.c`, and M11 expose catalog-driven panels and simplified interactions. `c_dialog.cpp::DM2_dialog_2066_3820` now carries the real `DIALOG_BOXES/0x81/0` pixels and local palette to the viewport through its expanded `RECT_453` host command, and remains no-draw unless the source dialogue owner marks it active. Remaining: original modal state/event, text, button and cancellation semantics; no catalog panel or fallback dialogue may replace them.

  - 2026-08-06 update: the exact static material half of inventory survey and
    hand-action rendering is now real-data covered. The receipts accept only
    `INTERFACE_CHARSHEET/0/dtImage/1` at `RECT_1EE` and
    `INTERFACE_GENERAL/4/dtImage/2..5` at their source direction rectangles;
    local palette, raw payload and decoded-pixel identity are rechecked when
    consumed. `test_dm2_v1_inventory_gdat_real_data` verifies all 64 source
    hand routes and the survey frame in the mounted PC English corpus. The
    later platform-specific M11 gates now reopen only the authenticated
    CHARSHEET frame for DOS, Amiga, FM Towns and Mac; unsupported event/modal
    owners still remain unavailable rather than falling back to a DM1 panel
    or host UI.

  - 2026-08-06 update: the V2 palette-control LUT is no longer a stub. It
    reads only the immutable V1 palette table, preserves every source RGB
    byte at neutral settings, and applies a bounded user-requested
    presentation transform only after source rendering. It does not admit
    generated art, a replacement palette, or an unverified V2 surface.

  - 2026-08-06 follow-up: the former static action-icon row was removed from
    the HUD material plan. SKProject selects those hand backdrops dynamically
    through `INTERFACE_GENERAL/4` and `RECT_46..RECT_4d`; static `/3/2..6`
    keys and host coordinates could not represent that source route.

  - 2026-08-06 follow-up: the production GDAT fetch and RAW4 crop route for
    those four hand backdrops became source-bound and fail-closed. At that
    point it remained unwired from normal gameplay because the live original
    champion formation, possession and hand-selection state had not yet been
    recovered; M11 could not infer that tuple from party order, pointer
    position or a Firestaff default.

  - 2026-08-11 update: GAME_LOAD now retains the source-owned
    `party.curacthero` and `party.curactmode` fields and the Towns renderer
    binds the selected champion's authentic action image when that state is a
    valid source selection. The source `handcooldown[hand]` byte now drives
    the original checker-pattern overlay, and the selected hand's real record
    link resolves through the authenticated pools to its real item image when
    command text resolves. Positive `CnNC` requirements now read the source
    record `w2` with exact `ADD_ITEM_CHARGE(object, 0)` semantics, including
    the special 16/17/18 cases; the probe never mutates the mounted pool. The
    source sleep/wake input now owns the one-bit overlay state; the full sleep
    tick cadence and charge-consuming action mutation remain. DB9
    `ContainerType()==0` hand items now follow the source's separate
    `IS_CONTAINER_MONEYBOX`/`IS_CONTAINER_CHEST` admission branch, including
    the authentic `CONTAINERS/cls2/dtText/0x40` lookup; no command-entry or
    charge requirement is invented for those containers. No hand selection is
    fabricated when the source fields are clear.

  - 2026-08-11 follow-up: the source `ACTIVATE_ACTION_HAND` state transition
    is now exposed through `dm2_v1_runtime_activate_action_hand`. It accepts
    only a live hero retained by the authenticated GAME_LOAD candidate and
    updates `party.curacthero/curactmode` together. The real FM-Towns M11
    gameplay regression exercises both hand selections. The native Towns
    rectangle/event owner is still required before pointer clicks can invoke
    this transition, and command-specific action execution remains fail-closed
    when the selected hand has no authenticated action entry.

  - 2026-08-11 follow-up: authenticated GAME_LOAD now exposes one source
    inventory transaction seam. The runtime exchanges a real
    `c_hero::item[30]` link with `LeaderPossession`, validates both links
    against the admitted record pool, preserves the `OBJECT_NULL` sentinel
    boundary, verifies the read-back, and rolls back on failure. The native
    inventory panel, remaining context/event ordering, and pouch/quiver/
    scabbard/backpack ownership are still open and remain unavailable.

  - 2026-08-11 follow-up: FM Towns M11 now admits the real
    `INTERFACE_CHARSHEET/0/dtImage/1` inventory frame after validating its
    global 255-colour source route, raw material receipt, decoded IMG3/U4
    pixels, and `RECT_1EE` RAW4 crop placement. The authenticated Towns
    inventory context table is now consumed by M11 for panel pointer routing
    and slot selection. Item movement/equip commits, source text, and the
    remaining non-equipment owners are still separate fail-closed work.

  - 2026-08-11 follow-up: the authenticated Towns `MOUSE_INPUT` event 71
    (`rect 0x8222`, source group `inventory.eye`) is now identified and
    routed through the source champion/status context. The current loose
    Towns corpus does not expose an authenticated RAW4 rectangle for this
    record, so M11 remains fail-closed and does not claim a clickable eye
    until that native geometry is recovered. Mouth/consume, status, moneybox,
    save, sleep/wake, and the remaining inventory owners remain unavailable
    until their live source state and native geometry are bound.

  - 2026-08-06 follow-up: command dispatch now propagates a rejected
    source-GDAT image callback, so a failed title/menu blit aborts the
    presentation transaction instead of being reported as successfully drawn.

  - 2026-08-06 update: the real PC English corpus regression now opens the
    direct DM2 data root without extraction and locks its `PAL_IRGB` route to
    Greatstone's documented system palette for IMG9 raw 0174/0175 credits and
    menu. The M11 startup test also proves the original Credits click,
    countdown, and either-button dismissal with that global palette. This
    closes palette provenance for the static menu/credits route; interactive
    packaged-app capture and the wider `GAME_LOAD` flow remain open.

  - 2026-08-06 update: `test_dm2_v1_boot_profile_smoke` now also accepts
    `FIRESTAFF_DM2_DATA_DIR` as its read-only direct root before the legacy
    home-directory fallback. This runs the complete verified PC boot,
    GDAT-HUD, G1 dungeon-material, palette/light, and no-procedural-V2 route
    without copying, unpacking, or staging game data. Broader original
    SKSAVE parsing and the remaining runtime-state ownership still remain
    required before playable-parity claims.

  - 2026-08-06 update: the old `test_dm2_v1_save_load_real_data` no longer
    interprets arbitrary SKSave-header bytes as a champion name or looks only
    in the obsolete `dm2-extras` tree. It reads the configured corpus in
    place, verifies each authentic 42-byte DM2 header and the source-owned
    raw-dungeon prefix for `sksave0..3.dat/.bak`, and deliberately keeps the
    unbound SUPPRESS tail out of playable state.

  - 2026-08-06 update: the startup-menu action contract now exercises that
    same mounted real corpus. Its valid raw prefixes cannot create Continue
    or slot rows before the complete original `GAME_LOAD` stream is owned;
    the only available menu action remains New Game, which itself stays
    behind the original-data initialization gate.

  - **FM Towns save boundary:** the mounted FM Towns corpus contains the
    authenticated CD/runtime media but no FM Towns `SKSAVE` artifact. The
    external `Downloads/dm2` corpus now supplies eight authentic DOSBox
    `sksave0..3.dat/.bak` files. The real-data loader verifies all eight DOS
    files and 269 source-boundary checks pass, but they are still a different
    platform and are not evidence for a Towns save writer. `DM2_GAME_SAVE`
    and full `GAME_LOAD` ownership therefore remain fail-closed for FM Towns
    until an authentic Towns save is available and the platform-specific
    stream passes a copied load/write/load regression. Do not use the DOS
    files or a generated fixture to close this gate.

  - **2026-08-11 FM Towns HUD receipt:** the native v4 `CHAMPIONS` portrait
    route is covered for every authentic type 0..15 by
    `test_dm2_v1_fmtowns_hud_portraits_real_data`. This closes portrait
    material binding only; source champion selection, full inventory/dialogue
    semantics, and FM Towns SKSAVE ownership remain separate gaps.

  - **2026-08-11 FM Towns pointer subset:** the authenticated Towns
    `INTERFACE_GENERAL` RAW4 table now owns source movement events 1..6 and
    champion action-hand selection events 116..123. The M11 route converts
    the native 640x400 rectangles to the 320x200 presentation surface and is
    covered by `test_dm2_fmtowns_m11_gameplay_real_media`. This does not claim
    inventory/dialogue pointer ownership, or a complete viewport interaction
    map; those remain fail-closed until their source event/rectangle mappings
    are recovered. The three action-panel pointer events now reach the
    existing CMDSTR-backed command owner, but a command still fails closed
    when its authenticated item/action record is not admissible. The native
    viewport event 0x50/rect 0x0007 reaches the DM2 c_rwbb target resolver
    and no longer falls through to DM1's C080 front-cell/door/mirror handler.
    Sensor/object mutation remains fail-closed until the corresponding DM2
    c_events owners are bound to runtime state.

  - **2026-08-11 FM Towns input identity hardening:** the pointer owner now
    requires both the authenticated Towns `GRAPHICS.DAT` and the native
    `SKULL.EXP` MD5 (`0f4b44d286cbee35924a95e7d75ad7e5`). It also verifies the
    disassembled `SKULL.EXP` MOUSE_INPUT anchor for events `0x70..0x72`
    (`0x003b/0x003f/0x0040`) and the complete 264-record table span
    (FNV-1a `0x1500c4c9`) before enabling the existing pointer subset.
    The full source table is context-sensitive: the same rect IDs are reused
    by inventory, status, and action-panel branches. Those branches still
    require a source UI-context owner and are not promoted by geometry alone.

  - **2026-08-11 FM Towns MOUSE_INPUT receipt:** after authenticating the
    `SKULL.EXP` identity, the boot profile now retains the complete 264-record
    (1584-byte) source span in memory. The input owner exposes each raw
    event/flag/rect/mask candidate with its original record ordinal and
    re-hashes the retained bytes before returning them. This is an evidence
    API for the next context-bound UI owners; it does not make the candidates
    globally clickable, because Towns reuses rectangle IDs across branches.
    Route ordinal `117` is now bound back to the source `hand_panel.action_1`
    context, and source event `0x70`/112 (rune-quit) closes the active action
    panel through the M11 owner. The remaining context-specific candidates
    are still inventory/status/dialogue work, not generic hitboxes.

  - **2026-08-11 FM Towns dungeon-context guard:** the live dungeon pointer
    route now consults the authenticated source context inventory before
    emitting an event. A native rectangle shared with inventory, status, or
    dialogue is not sufficient by itself; records without a dungeon context
    remain unavailable until their own live owner is bound. This closes the
    cross-view hit-test leak without borrowing PC geometry or inventing a
    replacement control.

  - **2026-08-11 FM Towns inventory layout census:** the native RAW4 bridge
    now resolves 129 of the 166 source inventory route contexts. Ordinals
    47-49, 52-83, 99, and 110 have no matching rectangle in the authenticated
    Towns RAW4 set. They remain fail-closed; no PC rectangle or replacement
    control is substituted.

  - **2026-08-11 FM Towns M11 event bridge:** the authenticated Towns
    MOUSE_INPUT route now reaches the source panel-close event 11 after the
    full SKULL.EXP table receipt and native rectangle admission. The generic
    c_input sleep/wake callbacks remain available, but event 142/143 are not
    present in the authenticated 264-record pointer span and are therefore
    not claimed as pointer routes. The seven source-explicit equipment slots
    now commit through the authenticated item-slot transaction when their
    native pointer context is clicked. Rune, moneybox, status, and
    non-equipment inventory mutations remain fail-closed until their original
    record-chain owners are recovered.

  - **2026-08-11 source-session inventory links:** source-complete GAME_LOAD
    sessions now expose and update `c_hero::item[30]` through the runtime
    inventory API. Each non-empty write is checked against the admitted DB
    record pool; the old 32-bit host cache remains unavailable before a real
    source session. Raw DOSBox SKSave files still stop at the authenticated
    pre-link GAME_LOAD boundary and cannot be promoted to a playable resume.

  - **2026-08-11 FM Towns explicit UI-context routing:** the authenticated
    264-record `MOUSE_INPUT` receipt can now resolve a pointer through an
    explicitly selected source branch (dungeon, inventory, status, or
    dialogue). The route returns both the source semantic context and the
    native `INTERFACE_GENERAL` RAW4 rectangle; it never promotes shared
    rectangle IDs to global hitboxes or reuses PC geometry. The M11 dungeon
    owner continues to expose only the dungeon branch. Inventory/status/
    dialogue mutation owners still need to consume these receipts before
    those controls become playable.

  - **2026-08-11 FM Towns equipment-slot provenance:** the seven unambiguous
    source inventory groups (`hand_right`, `hand_left`, `head`, `body`,
    `legs`, `foot`, and `neck`) now decode to the original slots 0 through 5
    and 10 from ReDMCSB `DEFS.H`. Pouch, quiver, scabbard, and backpack groups
    remain unavailable; their ownership is not inferred from screen labels or
    PC geometry.

- 🔧 Phase 5 - Creature/combat parity: creature AI table (64 entries with names + AI flags, 352-line implementation in `dm2_v1_creature.c` with spawn/tick/death_check) + combat resolver (now Phase 5-locked above) are source-locked. **2026-06-17 projectile routing + death sound landed:** new `dm2_v1_projectile_pc34_compat.c/h` provides the DM2→DM1 projectile bridge — maps DM2 creature `AttacksSpells` flags (12 bits: SHOOT/FIREBALL/LIGHTNING/DISPELL/POISON_CLOUD/POISON_BOLT/POISON_BLOB/PUSH_BACK) to DM1 `PROJECTILE_CATEGORY_*` + `PROJECTILE_SUBTYPE_*` via `dm2_v1_projectile_pick_category()`, then dispatches via F0810_PROJECTILE_Create_Compat. Three dispatch entry points: `dm2_v1_projectile_dispatch()` (auto-pick from creature AI flags), `dm2_v1_projectile_dispatch_spell()` (CCM 0x15 CAST_SPELL explicit subtype), `dm2_v1_projectile_dispatch_bomb()` (DM2 new area-effect). Plus magic-number fix in `dm2_v1_creature.c`: creature death sound now uses `DM2_SOUND_CREATURE_DEATH` constant instead of hardcoded `0x11`. New accessor `dm2_v1_creature_get_instance()` exposes creature pool read-only to the projectile module. Source-locked against SKULL.ASM:10620-10710 (SKULL_COMBAT_ResolveRanged), 11100-11200 (projectile routing), ReDMCSB PROJEXPL.C:76-92 (F0212), GROUP.C:1695-1770 (F0207 creature attack), skproject/SKWIN/SkWinCore.cpp:10479-10561 (AI_W30_TURNS_MISSILE). CTEST `test_dm2_v1_projectile_pc34_compat` 23/23 (all 7 attack-flag → category mappings, dispatch invalid/dead/melee-only rejection, archer guard + amplifier dispatch, spell + bomb dispatch, 3 observability counters, reset, source evidence, magic-number constant check). **2026-06-22 projectile-vs-creature collision gate landed:** new `dm2_v1_projectile_creature_collision_pc34_compat.c/h` resolves the DM2-specific missile-redirect dispatch when a live projectile reaches a square with a creature instance. 5-branch priority order: NONMATERIAL > ABSORBS_MISSILE > REFLECTOR > TURNS_MISSILE > HIT. Deterministic damage formula `max(1, impact_attack - armor_class/2)`; HIT/ABSORBED/REFLECTED despawn the projectile, and tests/probe cover each branch plus invalid slot/source evidence. **2026-06-28 projectile step/drain gate landed:** the runtime now advances the Firestaff DM2 projectile cache once per tick, consumes per-slot kinetic energy with the one-step grace boundary, despawns drained slots through the same observable path, and rebuilds the M11 drain view from post-step survivors. Remaining work: advanced CCM (`DM2_PROCEED_CCM`) full implementation, full cell-content digest/map-change/teleporter effects, and broader real-route runtime evidence.

### DM2 V2.0 / V2.1 / V2.2

- 🔧 Phase 2 - Enhanced asset pipeline: `dm2_v2_asset_pipeline.c` (V2.1 EPX + V2.2 modern-asset fallback chain) is source-locked against SKULL.ASM T520/T560/T580/T600 + ReDMCSB DUNVIEW.C:575-586/148-157/2962-3047/3048-3070/3082-3095/3940-4015/4016-4050/4119-4270 + PANEL.C:418-428 + DATA.C:359-360; probe `firestaff_dm2_v2_phase2_asset_pipeline_probe` is green. **2026-06-19 DM2 V2.2 modern-asset module landed:** new `dm2_v22_modern_assets_pc34.c/.h` mirrors dm1/csb/theron/nexus modules with DM2 paths (`~/.firestaff/assets/dm2/modern/`) and DM2 source-locks (SKULL.ASM T520/T560/T600 + ReDMCSB DUNVIEW.C:2962-3047 outdoor). Ctest `test_dm2_v22_modern_assets_pc34` 33/33. **2026-06-19 DM2 V2.2 first-cut asset pack landed:** `.openclaw/tmp/dm2_v22_asset_author.py` (5 PNGs + manifest v1.0.0). Smoke: `dm2_v22_modern_assets_available()=1` end-to-end. **2026-06-29 T560 indoor route gate landed:** `DM2_V22_T560IndoorRoute` exposes all nine indoor D0..D2 x L/C/R route names, raw-cell discriminators, category/asset ids, clipped rects, and active/no-op state; `firestaff_dm2_v22_inplace_render_probe` is now 33/33 PASS with a synthetic cache and cache-type mismatch rejection. Remaining: real PBR hero art for DM2 via gpt-image-2 batch, real-runtime wire-up of `dm2_v22_viewport_swap_render()` from the DM2 V2 viewport draw path, outdoor T600 route-depth follow-up beyond the existing 3-cell synthetic paint, and per-mode pixel/material verification gates.

- 🔧 Phase 3 - Enhanced UI overlays: **2026-06-16 HUD runtime wire-up landed (this pass):** new `dm2_v2_hud_runtime.c/h` provides the V1→V2 HUD bridge layer (mirrors `csb_v2_hud_runtime.c`). API: init/shutdown, set_gate_config, set_party_gold, set_direction, set_level, set_champion, set_action_active, trigger_hit_flash, set_opacity, render (gated on DM2_V2_PHASE_DOMAIN_HUD, V1 framebuffer preserved when V1 active), is_active, force_active_for_test. Source-locked against SKULL.ASM T560, skproject/SKULLWIN/c_gui_vp.cpp, ReDMCSB PANEL.C F0354, DUNGEON.C F0260, COMMAND.C, DISPLAY.C, dm2_v2_phase_gate.h. **M11 wire-up:** `firestaff_game_loop.c` (src/engine) now calls `dm2_v2_hud_runtime_render(g_framebuffer, 320, 200)` right after the DM2 V2 smooth-movement viewport render, gated on phase gate (no-op when V1 active, no V1 chrome pollution). CMakeLists: `firestaff_dm2_v2` linked from `firestaff_m11`. Probe `firestaff_dm2_v2_hud_runtime_probe` 23/23 (init/shutdown, all 7 setters, gated render is no-op when V2 off, paints into fb when V2 on, opacity=0 short-circuits, force_active_for_test bypass, V1 framebuffer preserved, champion bar pixels, action strip pixels, null-fb safe, source evidence). Remaining work: actual HUD text/bitmap assets, more HUD widgets (inventory quick-view, action prompt).

### DM2 CLI launch

- DM2-016 — `skproject/SKULLWIN/main.cpp`, `fileio.cpp`, and `c_gdatfile.cpp`: `src/shared/asset_status_m12.c`, `src/dm2/dm2_v1_boot.c`, and CLI launch still need corpus-verified classification/materialisation for every supported PC variant and valid container before entering DM2. Preserve hash-based discovery, but reject demo, incomplete, or cross-version mixes before boot rather than normalising them into a generic launch profile.

- 🔧 DM2 extras/cross-version launch remains open for demo and non-PC extracted paths that need separate version classification/container normalization.
  **2026-08-07 PC-9821 catalog correction:** the authenticated retail
  `GRAPHICS.DAT`/`DUNGEON.DAT` pair is now represented as `pc9821-ja` in the
  launcher catalog, separate from the PC-9801 demo. Its required dungeon hash
  follows the graphics-selected pair from `dm2_v1_boot.c`; other non-PC
  variants remain separately gated until their catalog and runtime owners are
  proven.

# DM2 PC-DOS File_header continuation and champion activation (2026-08-07)

- [ ] Derive the PC-DOS record/map continuation after the 44-entry
  `File_header` from an original-loader trace. The former 28-map pseudo-header
  accidentally produced 16 champion mirrors and a DYN4 selection; it is not
  valid evidence and must not be restored. Champion selection remains gated
  until the real DB3/DB4 ownership and marker route are independently proven.
  The authenticated DOSBox corpus also contains no `c_tim` type `0x3D` or
  `0x5D` entries (all eight primary/backup saves), so those moverec timers
  remain source-gated rather than being opened from a synthetic timer.
- [~] SKSAVE-prefixen har nu en explicit byte-order-kvittens mot
  `READ_DUNGEON_STRUCTURE`: 44-byte `File_header`, 16-byte map descriptors,
  kolumnindex, ground-stack, text, samtliga 16 source-sized DB-pooler och
  map-data slutar exakt vid SUPPRESS-starten. Detta verifierar råa
  sektionsgränser i den autentiska DOSBox-korpusen, men ersätter inte den
  återstående original-loader-tracen för champion-/runtime-ägarskap.

## DOSBox SKSAVE phase-4 blocker (2026-08-12)

- [x] Bind the source `DM2_ALLOC_NEW_RECORD`/DB0 recycler transaction required
  by `READ_SKSAVE_DUNGEON`: candidate selection, tile-chain cut, predecessor
  unlink, `OBJECT_END` clear and cursor update commit atomically in one private
  map/pool owner. Positive synthetic source regression covers the DB4-
  predecessor case and the DOSBox corpus reaches the candidate boundary.
- [x] Bind DB14 missile-delete together with its timer/CAII owner before
  publishing a Resume session. A DB14 clear without that coupled owner remains
  forbidden; do not reuse a DB0 slot or make the inspection owner playable.
- [x] Phase-4 diagnostics now expose the source boundary per save. Most
  failures request dynamic DB14/DB15 records through an empty tile root;
  `sksave1.dat` additionally reaches a resident-root validation failure, and
  `sksave2.bak`/`sksave3.dat` reach the source missing-record path. Existing
  DB0 free-slot counts (38–45 in the observed saves) are not interchangeable
  with those DB14/DB15 allocations.
- [x] SKSAVE-ägaren behåller nu en separat, oföränderlig snapshot av exakt
  source `savegames1`/`c_wbbb` (sex bytes, inklusive `b_04`) bredvid den
  muterbara post-load-arbetskopian. Korpustestet verifierar snapshotens
  giltighetsmarkör och den autentiserade source-hashen.
- [x] Koppla snapshoten till Resume-commitens skrivbara global-state-owner.
  Committen kräver nu byte-identitet mellan SKSAVE-ownerns oföränderliga
  snapshot och kandidatens sexbytesblock före någon ägaröverföring. Den färska
  GAME_LOAD-vägen har redan en `savegames1.b_04`-owner och source-tickens
  expiry-consumer; positiv DOSBox Resume-evidens återstår eftersom CAII-gaten
  fortfarande stänger kandidaten.

- ✅ SKSAVE-ownern har nu en read-only kandidat-admission som bekräftar
  party/map/record-pool/timer/savegames1 och skiljer poolägarskap från den
  ofullständiga record-graphen. Den pekar dessutom ut CAII-arrayen och
  sound-ownern. Receipten publicerar inte state och öppnar inte Resume.
- [x] SKSAVE har kapacitets- och AI-källan samt en separat DYN4/SOUND9-owner.
  Resume-kandidatens `LOAD_LOCALLEVEL_DYN` går nu mot den klonade, muterade
  SKSAVE-c_map-kopian i stället för pristine `DUNGEON.DAT`; det förhindrar en
  blandad map-/record-transaktion. Själva 34-byte CAII-arrayen och den
  entydiga DB4 → tile-chain → think-timer-bindningen är fortfarande inte
  autentiserad för de åtta DOSBox-saven och öppnar därför inte Resume.
- [x] SKSAVE-underlayen kan nu byggas till en GAME_LOAD-formad kandidat och
  bootprofilen har en separat Resume-retain/commit-väg som validerar source-
  provenance atomiskt. Positiv live-runtime-publicering med en fullständig
  DOSBox save/load-loop återstår fortfarande; New Game-vägen får inte användas
  som bevis för Resume.
- [x] SKSAVE har nu en komplett staging-klon av det muterbara runtime-underlaget:
  map-arrayer, recordpooler, CAII, c_tim-heap och SOUND9 queue/sample-arrayer
  får separata allocationer med bibehållen source-state. Denna klon är ännu
  inte publicerad som en live GAME_LOAD-session.
- [x] Materialisera SKSAVE:s autentiserade 12-byte `c_tim`-poster till en
  separat skrivbar GAME_LOAD-formad timerheap med sparad gametick, heapindex
  och free-chain. Den privata heapen dispatchas ännu inte.
- [x] Lägg till en läsande `0x1e`-dispatchkandidat som binder due-timer,
  DB14-handle, timer-backlink, packad koordinat och aktuell tile-kedja. Den
  konsumerar inte timer eller projektil.
- [x] Positiv DOSBox/savegame-fixture för en komplett DB4 → tile-chain →
  think-timer-CAII-länk saknas ännu. De åtta aktuella savesen avvisas därför
  korrekt före Resume-publicering när den länken inte kan autentiseras.
- ✅ En read-only CAII-admission räknar nu varje levande DB4-position via den
  muterade c_map-ägaren och jämför dynamiska records mot exakt map/type/packed
  A-koordinat i sparade 0x21/0x22-timers. DOSBox-korpusen har 14 dynamiska
  records, 0 matchningar, 14 saknade och 0 tvetydiga timerägare; den skrivbara
  CAII-arrayen förblir därför stängd.
- ✅ Samma admission följer nu också SkProjects `FILL_CAII_CUR_MAP`-gren för
  static-flaggan: static DB4-poster delas upp i lazy-fill-kandidater
  (`byte@5 == 0xff`) och råa poster med sparad slotmarkör. Markören är bara
  ett save-byte; source `RESET_CAII` nollställer den på liveägaren före
  `FILL_ORPHAN_CAII`. Detta är därför en read-only markeringsreceipt och
  ändrar inte DB4 eller skapar en syntetisk slot; den separata dynamiska
  `0a48`-vägen är fortfarande inte öppnad.
- ✅ CAII-kapaciteten är source-beräknad för varje komplett SKSAVE-owner från
  DB4-poolen och de autentiserade AI-raderna. Slot-rekonstruktion är däremot
  fortsatt separat; oidentifierade slotar lämnar CAII-admissionen stängd.
- ✅ SKSAVE-ownern exponerar nu en read-only `RESET_CAII → FILL_ORPHAN_CAII`
  preview: den räknar post-reset static-fill, dynamisk aktivering och det
  totala antalet första think-timers utan att skriva DB4 byte@5, skapa slotar
  eller köa timers. Det är nästa verifieringsunderlag för en riktig
  all-karts-owner; Resume är fortfarande stängt.
- ✅ En direkt DB4-handle-positionreceipt löser nu handle → map/x/y genom
  SKSAVE:s muterade c_map-ground-stack. Den använder inte pristine
  `DUNGEON.DAT` och är avsedd som första halva av den framtida owner-bundna
  think-timer-schedulern.
- ✅ Den privata schedulern kan nu köa source `0x21`/`0x22` direkt från
  verifierat DB4-handle + map/x/y till SKSAVE:s timer-owner. DOSBox-saven är
  redan fulla (`120/120`), så produktionsägaren avvisar enqueue utan fri slot;
  en separat timer-shadow verifierar den positiva enqueue-vägen utan att ändra
  save-state.

- ✅ En lyckad source-ordnad preflight markerar nu den överförda map-/pool-
  transaktionen som `record_graph_complete`; råbaslinjer och partiella
  recycler-boundaries förblir uttryckligen ofullständiga.
- ✅ SKSAVE-ownern kan nu binda en separat autentiserad PC-DOS `dungeon.dat`
  genom att kopiera och jämföra exakt kartantal och alla kartdimensioner mot
  savefilens receipt. Detta är layout-underlaget för nästa runtime-adapter;
  det publicerar ännu ingen session.
- ✅ Fältkartläggningen visar att alla åtta saves bevarar tile-typen i bits
  5..7; bits 0..4 är save/runtime-fält och varierar. Bindningen verifierar nu
  den stabila tile-typen och lämnar de muterade lågbitarna till SKSAVE-ownern.

## DM2 Macintosh support

DM2 Macintosh is a separate 68k platform family, not a DOS or FM Towns
asset variant. The local corpus contains authentic English and Japanese Mac
archives, a French StuffIt image, and Mac-specific `GRAPHICS.DAT` fingerprints.
The large English retail ZIP and the smaller English "The First Chapter" demo
are admitted and read without extracting game files. Firestaff reads the raw
MODE1/2352 BIN, Apple Partition Map and HFS catalogue directly into RAM; the
demo additionally walks its genuine StuffIt 2 `DMFiles` member in RAM. The
retail and demo use separate hash-paired big-endian dungeon receipts.

- [x] Admit the authentic large US English retail ZIP independently as
  `mac-en-retail`; keep its HFS container as the runtime owner and never
  extract its game files to the staging directory.
- [x] Admit the smaller US English "The First Chapter" demo independently.
  The authentic installer is read in RAM, its `DMFiles/Graphics.dat` and
  6,535-byte `Dungeon.dat` are hash-verified, and its truncated big-endian
  File_header/map data enters the real dungeon loader. The leading bytes are
  the dungeon header, not a payload to expand.
- [x] Publish the authentic Mac `Dungeon.dat` member as a required-file receipt
  for both English editions. Direct launch from either original ZIP now remains
  launch-ready without extracting the BIN, HFS files or resource forks.
- [x] Keep both authenticated English Mac editions visible when the launcher
  scans a shared DM2 data root containing both original ZIPs. The scan no longer
  stops after the first Mac archive.
- [ ] Admit the authentic Japanese 1.0 and French StuffIt editions
  independently once their container/resource-fork readers are verified.
- [~] Add a source-owned Mac container/resource-fork reader for the verified
  CD/content archives, including StuffIt/HQX/resource-fork provenance. The
  reader now reads classic HFS data and resource forks in RAM, including the
  late-partition, case-preserving Japanese CD data track, and preserves its
  identity. Japanese boot admission and French StuffIt/HQX handling still fail
  closed until their platform/runtime owners are independently verified.
- [ ] Bind the Mac big-endian `DUNGEON.DAT` and `GRAPHICS.DAT` pair to one
  platform-specific boot receipt. Japanese 16-colour and US English
  256-colour graphics must remain separate layouts and hashes. The shared
  dungeon reader now preserves the source bytes, carries explicit 68k word
  endianness through column/ground-stack/record links, and proves the
  record-graph and start-pose gate for both authentic English ZIPs. Retail
  map-wide GAME_LOAD now passes all 44 authentic maps. The Mac retail
  File_header uses the canonical 44-map byte-square layout and its complete
  DB0..DB10 pools; empty object-bearing squares are accepted only when the
  source null marker is present. The demo has no champion-DYN4 roster and
  therefore remains a title/demo route.
- [~] Read the authentic retail Mac QuickTime `MooV` data and resource forks in
  RAM. The verified retail image contains `Title.MooV`, `Swoosh.MooV`,
  `Credits.MooV` and `Ending.MooV` (CTest records their presence, sizes, HFS
  resource forks, and `moov` resource payloads); `Story.MooV` is absent
  from this original image and is not invented. The complete source-owned
  HFS fork pair is now available to the boot profile without writing a
  flattened movie. `dm2_v1_mac_movie_decoder_real` and
  `dm2_v1_mac_m11_movie_runtime_real` now prove authentic Cinepak/PCM decode
  and M11 startup binding from the retail ZIP. The decoder gate covers all
  four present movies (`Title`, `Swoosh`, `Credits`, `Ending`); `Swoosh.MooV`
  is admitted with its authentic four-byte prefix before the `mdat` atom.
  A bounded in-memory QuickTime view now joins each exact `moov` atom to its
  matching `mdat` atom in the boot profile, including that source-owned zero
  prefix; the retail gate passes for the four present movies (`0x19`, with
  `Story.MooV` absent). The M11 Mac startup route now presents the authentic
  Title movie and binds the authentic Credits movie from the source-owned
  Credits rectangle; Mac Return/Enter closes Credits through the source input
  table. The demo keeps its static credits route because its real media has no
  Credits movie. Presentation timing and ownership through the original
  QuickTime/MooV owner remain open. A native optional
  FFmpeg-backed in-memory decoder now rebases the private QuickTime chunk
  offsets, decodes the authentic Cinepak video and PCM audio, and owns the
  Mac startup surface until the title reaches EOF; no movie file is written.
  Do not
  replace these with converted MP4 files in the source runtime; converted
  files may be verification derivatives only.
- [~] Retain the authentic US English Mac `Music`, `General.sounds`, and
  `Weapon.sounds` Resource Manager forks in the boot profile. The retail image
  verifies their exact fork sizes and presence; MIDI/sound-resource decoding,
  timing, and playback ownership remain open. A bounded format-1 `snd ` parser
  now enumerates the authentic 2/19/12 resource sets and exposes the exact
  sample range, rate, loop fields, and resource ID without copying the bytes.
  The exact signed 8-bit sample range from `General.sounds` resource 10001 is
  now hash-checked and transported through the existing SDL host mixer at its
  source rate. The 28 authentic application `Midi` resources (IDs 1000-1027)
  are parsed in place as SMF and the Mac map-trigger route now selects them
  directly; native CoreMIDI scheduling parity and the remaining `snd ` timing
  classes remain open.
- [x] Retain the authentic retail `Dungeon Master II` application data and
  resource forks in RAM (`484,944` and `5,046,234` bytes). The complete
  MacBinary/application resource owner is now available for source-locked
  `Midi`, `snd `, menu and event-resource work; no application is extracted.
  The Resource Manager type list is now readable without copying payloads;
  the real retail fork is source-locked with `CNTL` 130/131 (32 bytes each),
  plus its `MENU`/`DITL`/`DLOG` families. These static dialog controls do not
  close the separate dynamic gameplay Control/Event gate.
  Bind the Japanese
  CD-audio route separately. DOS HMP, FM Towns CD.DAT, and Amiga MOD paths are
  not fallbacks.
- [x] Add the source-locked English Macintosh keyboard/menu table for both
  admitted US versions. `dm2_v1_mac_input` covers champion/leader inventory,
  movement, freeze, Command-O/S/Q, entrance New, credits close, and the three
  wall-button columns. Queue-compatible actions are forwarded to the existing
  command boundary; Mac-only actions remain explicit and unavailable until a
  native Mac dispatcher owns them. Source: DMWeb Macintosh edition page.
- [~] Bind the Mac input table to the native M11/SDL runtime dispatcher. The
  admitted Mac profile now takes precedence over PC aliases for the English
  retail/demo gameplay route, and movement, champion inventory, leader
  inventory, freeze, wake, save and quit reach the existing M11 boundaries.
  Held keyboard/gamepad motion now uses the source input boundary and the Mac
  A/D/W/S/X/Z/C and keypad meanings; SDL autorepeat is ignored for those
  actions. The authenticated `c_rwbb` wall-target path now reaches the
  source DB3 owner for push-button and local switch mechanisms; unsupported,
  incomplete and item-admission branches remain explicit and fail closed
  instead of becoming a synthetic attack.
- [ ] Acquire an authentic Mac save corpus for both language families and
  verify native load/save round trips. A DOSBox `SKSAVE` or a generated save
  cannot close this gate.
- [x] Keep the save-corpus evidence path endian-explicit. The shared 42-byte
  container gate now routes the admitted raw body through an explicit
  little-endian DOS or big-endian Macintosh reader; no filename, host byte
  order, or fallback guess selects the word order. This does not claim Mac
  Resume: an authentic Mac save corpus and the complete GAME_LOAD owner are
  still required.
- [x] Carry the authenticated Macintosh byte order through the retained
  GAME_LOAD inspection owner: c_map column/ground links, c_record words,
  c_hero scalar words, and c_tim scalar words now use the source dungeon
  receipt instead of the host/DOS default. This is an import-path correction,
  not a Mac Resume claim; no authentic Mac save is present to exercise it.
- [ ] Bind the small-edition dynamic Mac application owner. The separate
  `Downloads/DungeonMasterII_demo.hqx` preservation source contains an
  authentic 484,815-byte application data fork and 1,889,960-byte resource
  fork, including an edition-specific `CODE(11)`; the authoritative small CD
  ZIP under `.firestaff/data/dm2` contains only `DMFiles` and no application
  fork. Do not copy or synthesize that application in the production path.
- [~] Add end-to-end Mac startup, viewport, inventory-cursor, movie, audio,
  input, save/load, and pixel/audio regression gates before claiming Mac
  gameplay support. Retail startup/movie/audio and both-version boot gates
  pass; viewport/inventory-cursor, native Mac held input, save/load, and
  source-pixel/audio parity gates remain open. The admitted English retail
  GDAT v5 has authenticated `INTERFACE_CHARSHEET/0/1` and all four
  `INTERFACE_GENERAL/4/{2..5}` hand-action images. Mac RAW4 decoding is now
  endian-correct, including the native hand-action chains `RECT_46..4D`, and
  is covered by the real-media gate. The endian-correct Mac table now resolves
  `RECT_1EE` to the same authenticated 119×70 crop at 99,55 as DOS/Amiga;
  M11 inventory open/draw/close is covered by the Mac real-media gate.

  - 2026-08-12 disassembly follow-up: the retail Mac HFS application is now
    identified as `Dungeon Master II` with a 5,046,234-byte resource fork and
    24 authentic 68k `CODE` resources. The apparent `00 01 EE` matches in
    the code disassembly are 68k branch displacements, not direct
    `RECT_1EE` resource calls. The native geometry is nevertheless proven by
    the endian-correct RAW4 query: `0x1EE` resolves to destination 99,55 with
    a 119×70 crop. No guessed candidate such as `0x01F7` is used.

- [~] Bind source-owned inventory rendering across the admitted platforms.
  DOS and FM Towns already pass their authenticated CHARSHEET/RAW4 gates.
  Amiga and Mac now also pass their authentic `RECT_1EE` routes and M11
  open/draw/close regressions. Item context, pointer owners and native saves
  remain separate work.

Required evidence: hash-identified Mac CD/content media, resource-fork
receipts, original Mac or emulator traces for menu/input/audio/movie timing,
and at least one authentic save per claimed edition.

## Runtime actuator follow-up (2026-08-12)

- [x] Runtime `0x04` klass 4 är nu source-bundet till direkt DB0-dörr,
  komplett record-kedja och source-0x01 animationstimer efter GAME_LOAD.
- [x] DOSBox/real-media verifierar nu en verklig klass-4-kedja på map 2
  `(19,10)`: autentisk DB0-door-root, source `0x04` actuator, `0x01`
  animationsteg och state `4 → 0`. Bredare dörrsemantik och party-/ljudsvansar
  är fortfarande separata owners.
- [x] DOSBox-korpusens åtta `sksave0..3.{dat,bak}` är nu körda explicit via
  `FIRESTAFF_DM2_SKSAVE_CORPUS=/Users/bosse/Downloads/dm2`: 275/275 checks
  passerar. Korpusen saknar fortfarande `0x04`, `0x3D/0x5D`, `0x5E` och
  resurrectionstimers, så den styr negativa gates men öppnar ingen syntetisk
  positiv runtime-fixture.

## SKSAVE phase-4 diagnostic boundary (2026-08-12)

- [x] SUPPRESS-recordläsaren rapporterar nu även bitströmsfel med explicit
  `DM2_READ_RECORD_FAILURE_INPUT`; tidigare kunde samma gren lämna reason 0.
- [x] Map-restoren respekterar nu DMWebs source-bit 4: endast tile-rutor med
  objektlista konsumerar en record-kedja. De åtta DOSBox-saven når därmed
  hela 44-map chain-boundary utan falsk DB14/DB15-typ vid EOF.
- [x] Den privata GAME_LOAD-ownern materialiserar nu en source-ägd CAII/AI-
  tabell från den autentiserade asset-loadern och den materialiserade DB4-
  poolen. Tabellen används i den privata AI-admissionen utan global GDAT-
  fallback.
- [x] DB0-recyclerns source-walk föredrar `state.dungeon` när den kompletta
  receipten finns. Efter atomisk kandidatöverföring binds map-ownerns interna
  dungeonpekare om till den nya ownerens receipt; source-map 0 läser därmed
  korrekt bredd i stället för stale stackdata.
- [x] DOSBox-korpusen materialiserar nu alla åtta privata pre-session owners.
- [x] Full Resume-session är verifierad för de fyra levererade DOSBox-saven:
  `source_game_load_session_ready`
  kräver återstående skrivbara CAII-, timer-, party- och runtime-owners. DB14/
  DB15-reserven (`table_1d281c`: +0x3c/+0x32) får inte ersättas med uppfunna
  AI-flaggor eller DB0-slots.
- [x] DB14-delete har nu en read-only source-admission: no-creature tile,
  DB14-handle, recordets timer-slot och direkt `c_tim`-slot måste matcha, med
  timer type `0x1e` och timer-A pekande på samma missile. Timerheapens sorterade
  `timer_indices` behandlas inte som slot-identitet.
- [x] Den privata pre-session-ownern kan nu commitera no-creature DB14-delete
  atomiskt: tile-chain cut, `0xffff`-markering, timer-clear och timerlistans
  free-chain rebuild publiceras tillsammans. Rå-SKSAVE-bytes och
  `source_game_load_session_ready` ändras inte.
- [x] CAII-moverecens no-slot-gren markerar nu även den nyallokerade första
  `c_tim`-ägaren som `timer_updated`. Utan detta flaggade receipten den
  source-giltiga slot-/think-timer-allokeringen som misslyckad i den högre
  moverec-transaktionen.
- [x] Resume-committen är kopplad till skrivbar CAII-, timer-, party- och
  runtime-owner genom den atomiska kandidatvägen. Separata native saves för
  Amiga, FM Towns och Mac är fortfarande inte levererade.

# DM2: SKSAVE CAII source-slot reconstruction (2026-08-12)

- [x] SKSAVE försöker nu materialisera 34-byte CAII-slotar genom att kräva
  samma DB4-record, tile-chain-position och 0x21/0x22-think-timer som i
  sourcekedjan. Dubbla eller saknade länkar avvisar hela arrayen.
- [x] Slotens record-index, timer-index, position och source-tidsfält fylls
  med PC-DOS-layout; arrayen frigörs atomiskt vid fel.
- [x] Full record-graph och skrivbar CAII-/timer-dispatch är verifierade i
  DOSBox Resume-kandidaten. `RESET_CAII` kör före fill och den lazy dynamiska
  CAII-aktiveringen publiceras först efter atomisk kandidatkontroll.

## SKSAVE timer-owner capacity receipt (2026-08-12)

- [x] Runtime-materialiseringen skiljer nu source-rätt på aktiv `num_timers`
  och `num_indices` från `DM2_REARRANGE_TIMERLIST`; den senare kan vara 120
  även när c_tim-listan har lediga slotar. Fri-listans huvud följer med utan
  att 120-postskapaciteten utökas eller kringgås.
- [x] Runtime-candidate-receipten rapporterar den faktiska aktiva c_tim-counten
  efter GAME_LOAD:s specialtimerkedjor; indexspannet exponeras inte längre
  felaktigt som antal aktiva timers.
- [x] SKSAVE:s temporära `vsgame[120]` nollställs nu före SUPPRESS-dekodning;
  oinitierade tail-bytes kan inte längre skapa falska aktiva timers eller en
  falskt full fri-lista vid owner-handoffen.
- [x] Även när kön är full kan think-timer-adaptern testas isolerat på en
  privat shadow-owner genom att först frigöra en befintlig timerpost. Den
  riktiga ownern muteras inte och Resume-gaten förblir stängd.
- [x] En save utan aktiva c_tim-poster behandlas som ett source-rätt no-op;
  regressionen fabricerar inte en think-timer för att tvinga fram en positiv
  gren.
- [x] Den separata SKSAVE-passagen för statisk `09db`-animation återställer
  DB4 byte@5 och gör source-merge atomiskt. Sex saves passerar positivt; två
  rullas tillbaka när deras animationsrad saknar verifierad lokal ägare.
- [ ] Koppla den statiska passagen till GAME_LOAD-transaktionen först när de
  två saknade animationsägarna är bundna; den får inte öppna Resume ensam.
- [ ] Återstår: positiv DOSBox/retail-fixture där en dynamisk DB4-creature
  entydigt binds till sin source-ägda `0x21/0x22`-timer och CAII-slot.

## DM2 SKSAVE source-owned Resume handoff (2026-08-12, latest)

- [x] DOSBox `--save` går nu via M11:s DM2 `savePath` till source-owned
  `GAME_LOAD`, med autentiserad save-header, CREATURES→AI-bindning,
  DUNGEON.DAT-layout, SOUND9 och privat timerägare.
- [x] Static-only-save kan ha en tom selector-kö i `LOAD_LOCALLEVEL_DYN`;
  new-game-gatens krav på minst en selector gäller inte SKSAVE-kandidaten.
  Källans cross-map-actuator-fakta behålls i kandidatens receipt.
- [x] Alla fyra primära DOSBox-saves (`sksave0.dat`–`sksave3.dat`) når
  `startupActive=0`, `levelLoaded=1`, fyra champions och tickande runtime;
  de återställer sina respektive source-poser.
- [x] Verifierat med DOSBox-korpusen: `436 PASS, 0 FAIL`; DOS-manifestet
  passerar `30/30`, Amiga boot, FM Towns HUD/media och Mac retail boot
  passerar med de autentiska lokala filerna.
- [x] En Resume-inputprobe med `up` driver runtime-ticken efter source-owned
  handoff. Den fulla längre input-/skrivregressionen för timer/DB14/DB15 och
  separata native saves för Amiga, FM Towns och Mac återstår.

## DM2 platform data/runtime matrix (2026-08-12)

- [x] DOS/PC-DOS data är den aktiva referenskedjan: DUNGEON.DAT, GRAPHICS.DAT,
  SONGLIST.DAT och DOSBox-SKSAVE-korpusen är verifierade.
- [x] Amiga har en autentiserad arkiv-/LZX-bootägare, big-endian dungeon-graph,
  CD.DAT-musikkarta och originalanimationer. Den opt-in boot/game-load-gaten
  finns i `test_dm2_v1_amiga_boot_real_media`.
- [x] FM Towns har egen HME-242/GRAPHICS.DAT- och CDDA/input-kedja samt en
  opt-in M11 gameplay-gate. Den får inte låna DOS GRAPHICS.DAT.
- [x] Mac har egen ZIP-/resource-fork-/MooV-mediaägare, big-endian
  DUNGEON.DAT och 176-byte musikmapp; retail- och demo-bootgater finns.
- [x] DOSBox har nu gemensam source-owned CAII/0a48/think-timer-kedja och
  Resume-publicering verifierad för alla fyra primära saves. Amiga, FM Towns
  och Mac har fortsatt media/gameplay-evidens, men inga native save-fixtures.
- [x] DOS NEW GAME går nu från den autentiska startend-spegeln genom
  `SELECT_CHAMPION` och atomisk runtime-commit. Den lokala boot-proben når
  `dm2-runtime` med `levelLoaded=1`, en source-hjälte och tickning aktiv.
- [x] Positiva Amiga/FM Towns/Mac gameplay-fixtures använder nu samma
  DB4→c_map→CAII→c_tim-receipt och verifierar NEW GAME, runtime-commit och
  en source-owned rörelse: `test_dm2_v1_amiga_new_game_real_media`,
  `dm2_fmtowns_m11_gameplay_real_media` och
  `test_dm2_v1_mac_new_game_real_media`. Native Resume-saves för dessa tre
  plattformar saknas fortfarande.

- [x] Mac retail har nu verifierad HFS/MooV, 44-map big-endian dungeon och
  13 autentiska champion-mirror-rötter. Den source-owned NEW GAME-kedjan
  passerar den gemensamma CAII/record-pool-adaptern, Mac-native GDAT-ord,
  hjälteadmission, första `SELECT_CHAMPION`, runtime-commit och en verklig
  rörelse. `test_dm2_v1_mac_m11_new_game_real_media` verifierar dessutom
  Title.MooV → autentisk NEW GAME-rect → aktiv M11-session → turn/movement.
  Native Resume-save och bredare inventory/action UI återstår.

## SKSAVE dynamic CAII bridge (2026-08-12)

- [x] Den privata SKSAVE-ownern kan nu skicka en komplett source-bunden
  GAME_LOAD-formad context till den befintliga dynamiska CAII-transaktionen.
  DB4, CAII, c_tim, RNG och SOUND9 rullas tillbaka atomiskt vid ofullständig
  `0a48`-proveniens.
- [x] Bryggan är byggd och DOSBox-korpusen passerar `436/436`; alla fyra
  primära saves ger positiv dynamisk `0a48`/CAII/think-timer-handoff där
  source-data kräver det.
- [ ] Separata native save-fixtures för Amiga, FM Towns och Mac återstår.
# DM2: kandidatens PROCESS_SOUND (0x15) (2026-08-11)

Den privata GAME_LOAD-kandidaten saknar fortfarande full täckning av alla
timerfamiljer, men `PROCESS_SOUND` är nu bunden: timer A väljer den autentiskt
materialiserade delayed-slotten, map-gate följer `c_sfx.cpp`, `GEN1` använder
det privata sound-owner och både timerheap samt sound-state återställs vid
ofullständig binding. M11 har en positiv regression som kvitterar slotten och
timerposten.

- [x] Den source-formade `dm2_v1_perform_move_exec` kvitterar inte längre en
  lyckad open-tile-flytt när `DM2_MOVE_RECORD_TO` failar eller returnerar
  `fail_closed`; full party-/moverec-runtimepublicering är fortfarande stängd
  tills återstående source owners är bundna.
- [x] Runtime Poison Cloud kräver nu en giltig `DM2_QUEUE_NOISE_GEN2`-kvittens
  innan DB15-record och `0x19`-timer publiceras. Ljudköfel återställer hela
  cloud-transaktionen; övriga DB14-/reflector-owners är fortsatt fail-closed.
- [x] Den separata spell-timeradaptern avkodar nu source-payloaden för `0x19`
  och `0x1E`: packad A-koordinat, B-recordhandle för DB15 respektive A-DB14-
  handle och packad B-koordinat/energi. Subtype/objektidentitet läses inte ur
  timer-reserver utan kräver den autentiserade record-ownern.
- [x] `SHOOT_ITEM` har nu en source-owned player-runtime-owner. `CAST_SPELL_PLAYER`
  väljer `0xff80 + SpellCastIndex()` enligt SKProject `SkWinCore.cpp:17588-17596`,
  och runtime-transaktionen allokerar DB14, skriver objektord/energi/step,
  länkar recordet till partyrutan och köar 0x1E med samma source-index. Den
  atomiska rollbacken återställer pool, ground-chain och timerkö vid fel.
  Positiv Mac-real-media-kastfixture är nu verifierad även efter rörelse;
  positiv DOSBox-kastfixture är nu verifierad; fulla `DM2_STEP_MISSILE`-
  kollisioner är fortfarande en separat verifieringsgate.
- [x] Den generiska `dm2_v1_spell_cast_player_apply`-adaptern fabricerar inte
  längre en `0x1e`-timer med koordinater i A/B för Fireball eller andra
  projektiler. Utan DB14-handle, recordord och timer-slotindex rullas hela
  cast-writebacken tillbaka atomiskt.
- [x] Aura of Speed, spell 11 (`OH IR ROS`), är regressionstäckt som GENERAL
  med sitt source-cast-payload och har nu en skrivbar `savegames1.b_04`-owner
  efter färsk GAME_LOAD. SKSAVE Resume saknar fortfarande snapshot-transfern.

## DM2: party attack early-return and death gate (2026-08-13)

- [x] CAII-attacker som har skrivit source-slotens ackumulerade skada före en
  legitim tidig retur markerar nu `hp_applied`. Runtime-kollisionen blockerar inte längre
  partyns attack bara för att creature-turn/reschedule-svansen stannar
  fail-closed.
- [x] Missile-destinationen följer nu källans nollskade-anrop till
  `ATTACK_CREATURE` (`c_tim_proc.cpp:795-802`); Mac-real-media verifierar
  destination-hit och DB14-konsumtion utan att låtsas att detta är
  `WOUND_CREATURE` eller en HP-write.
- [x] När due `THINK_CREATURE` för över ackumulerad CAII-skada och den blir
  lethal går runtime nu samma autentiserade `DELETE_CREATURE_RECORD`-
  komposition med DB4-tile, CAII, timerkö, dropord och dealloc i source-ordning.
  Om någon admission saknas förblir dödsföljden fail-closed och partyt får
  inte skriva över DB4-cellen.
- [ ] Positiv real-media-fixture där en faktisk partyattack dödar en DB4,
  placerar dess drop och bekräftar kartkedjans efterföljande cell återstår.

## DM2: WIELD action owner (2026-08-13)

- [x] `DM2_ENGAGE_COMMAND` exekverar nu source WIELD-callbacken i stället för
  att bara lämna en receipt-markering.
- [x] Runtime löser CMDSTR-action 8, den riktiga facing-cellen och DB4-creaturen
  från monterad dungeon-data och återanvänder combat→CAII-ägarkedjan.
- [x] DOS-real-media-fixturen går genom WIELD→combat med rätt source-fält
  (`field 11` som attackstyrka och `field 10/15` som variantdata) och rätt
  tile-owned DB4-handtag. Startvapnet missar korrekt mot de första målen med
  hög rustning, och missen blockerar nu collisionen i stället för att
  rapporteras som en träff.
- [ ] Positiv skada, död, drop och DB4-deallokering behöver ännu en autentisk
  save/fixture där vapnet faktiskt övervinner rustningen.
- [x] Attackstyrkans runtime-bindning följer nu också `c_querydb.cpp:2237-2378`:
  den beräknade förstyrkan (ability + load + skill + DBSPEC) går vidare till
  `get_stamina_adj`; rå ability får inte ersätta den. Combat-enhetstester,
  DOS NEW GAME och Mac/DOS save-laddning måste fortsatt verifieras efter denna
  korrigering.
- [ ] Samma fixture måste ännu fånga en creature-typ med faktisk RNG-genererad
  possession-drop. Drop-ägaren försöker nu den source-korrekta
  `RECYCLE_A_RECORD_FROM_THE_WORLD`-walkern över autentiska c_map-kedjor,
  och återvinningsaren når nu även DB4:s possession-kedjor. DOS-fixturen
  kör fortfarande ett iterationssteg utan en godkänd icke-viktig DB5/6/8/10-
  kandidat. Ingen host-cache får användas; nästa steg är att verifiera en
  fixture med en sådan återvinningsbar kandidat.
- [ ] Den separata opt-in `sksave1.dat` Resume→WIELD-gaten använder nu den
  publika save-path-handoffen, den live-publicerade runtime-profilen och en
  autentisk inventory-swap från savefilens vapenpool till den aktiva handen.
  Fixturen använder nu det starkaste autentiska save-vapnet (`0x1407`), och
  Resume/commit, swap och source-CMDSTR-WIELD passerar. De aktiva creature-typerna
  i `sksave1.dat` har dock fortfarande för hög rustning för en positiv träff;
  någon WIELD-död/deallocation kan därför inte bevisas från denna save. Generated-drop-steget placerar dock
  fortfarande 0 objekt, eftersom den autentiska saveprofilens item-pool inte
  ännu ger en godkänd DB5/6/8/10-allokering. Ingen fabricerad command eller
  drop-state får öppna gaten.
- [x] Recycler-walkern stoppar nu vid samma actuator-/Text-extension-barriärer
  som source-ordningen i `sksave`-referensen, även när den inte hittar någon
  kandidat i den aktuella DOSBox-korpusen.
- [x] `SET_ITEMTYPE` använder nu record-poolens autentiserade ordbyteordning
  för DB5/6/8/10. Big-endian Mac/FMTowns-pooler har därmed samma source-byte-
  mutation som DOS, med separat syntetiskt endianprov.
- [x] Generated-drop receipten skiljer nu en korrekt genomförd source-drop
  från `ALLOC_NEW_DBITEM == OBJECT_NULL`. WIELD-fixturen visar därmed den
  faktiska kvarvarande gränsen (`iterations=1`, `alloc_failures=1`,
  `drops=0`, DB4 deallokerad) i stället för en presentationscache; den
  autentiserade world/possession-recyclern läser samtidigt big-endian word@2
  och söker även PC G1-extension-records.

- [x] Runtime `0x04` binder nu source-klass 0 wall-mecha och klass 1
  floor-mecha till de kompletta `ACTUATE_*_MECHA`-kedjorna. Klass 2/4/5/6
  och övriga actuator-tails förblir stängda tills deras separata owners är
  verifierade.
