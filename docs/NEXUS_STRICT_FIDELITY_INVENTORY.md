# Nexus – strikt källtrogen inventering

Detta är en arbetsinventering för DM Nexus (Saturn). En verifierad filhash
eller parserkännedom räcker inte som bevis för att en yta får renderas:
pixelavkodning och runtime-handoff måste också vara verifierade.

## Uppstart

| Källa/route | Status | Regel |
|---|---|---|
| `TITLE.CG` + `TITLE.BIN` MAPD | DMWeb-dekodern dokumenterar 5 MAPD-bilder på 64×28 tiles, 8×8 pixels, med 4bpp `TITLE.CG`-tiles och 16-färgspalett från MAPD; Firestaff avkodar nu alla fem retail-frames och paletten från MAPD-offset `0x8c54` | 512×224-bildernas Saturn→Firestaff-presentationsbindning är fortfarande blockerad; M11 gör ingen implicit top-left-kopia till 320×224 |
| `LOGOBG.DG2` | Retailens `PP`-header, 320×224 indexpixlar och 256 big-endian BGR555-palette avkodas till en valfri source-owned UI-surface med råbytesproveniens | VDP2-lager, palette-bank, timing och placering saknar capture; ingen host-presentering |
| Kodbyggda titelkanter/prompt | Borttagna från `nexus_v1_title.c` | Ingen syntetisk grafik läggs ovanpå `TITLE.CG` |
| Saknad/ej redo titelasset | Blockerad | Ingen syntetisk titelbild |
| Startup-fallback | Isolerad status/diagnostik | Får inte materialiseras som spelgrafik |
| `nexus_render_title_fallback` (äldre API) | Isolerad; ingen M11-produktionsanropsväg | Får inte återkopplas som Nexus-startbild |
| Hårdkodad roster i `nexus_v1_champions.c` | Borttagen från produktionsbiblioteket; kvar endast i `tests/nexus_v1_champions_fixture.c` för äldre kompatibilitetstester | Namn, japanska namn och attribut kommer i produktion endast från verifierad RLOWFIX/PLRD; saknad eller felaktig källa lämnar roster tom och championpresentation fail-closed |
| Saturns `FACE.BIN` | Verifierad 20-entry container; alla 20 verkliga PRS3-portraitrecords kan avkodas till 56×56 pixlar. Uppstartens loader bevarar nu varje frames 64-entry BGR555-källpalette och RGBA-expansion | Champion-index och VDP-placering saknar fortfarande verifiering; M11 laddar receipten men placerar inte portraitpixlar |
| Hårdkodad creature-statstabell i `nexus_v1_creatures.c` | Bortkopplad från runtime | MNS bevisar modellcontainer, inte HP/attack/försvar/XP; creature-init lämnar typregistret tomt tills DGN/DM.BIN-statkälla är verifierad |
| Creature→MNS-filnamn och AI-sentinel i `nexus_v1_creatures.c` | Retailfilnamn byteverifierade vid `DM.BIN+0x0385F0`; 30 AI/sentinel-poster byteverifierade vid `DM.BIN+0x0383A8`; engelska labels borttagna | Detta är källmetadata, inte en live-modellkonsument; CRET-stats kommer endast från retail `RLOWFIX.BIN`, och DGN/MNS-modellplacering kräver fortsatt bindning |
| DM1-inherited itemkatalog i `nexus_v1_inventory.c` | Bortkopplad från runtime | Filen anger själv att Saturnspecifika stats, namn och use-semantik inte är bekräftade; item-ID:n resolvear nu inte till DM1-stand-ins |
| Saturns `ITEM.IBS` | Verifierad 0x18800-byte visual/declaration-bank; real DGN Structure1Fa-referenser binds utan ersättningsbild; floor-image-renderern följer nu DMWebs palette-ID-bundna `palette_offset == 0`-återanvändning | IBS bevisar ikon-/deklarationsdata, inte combat-, vikt-, namn- eller use-stats; item-use i live mechanics är nu no-op tills Saturns action-dispatcher är bunden |
| Saturns `SMAP00-15.BIN` | Alla 16 verkliga LVMP-kartor avkodas till 640×608 med retail tilemap, 256-entry BGR555-palette och begränsade tile-index | Tilemap bit 0, palette bit 15 och VDP2/explored-state-ägare måste fortsätta vara källbundna; automap-pixlarna hålls no-draw |
| Infererad DM1-drop-tabell i `nexus_v1_drops.c` | Bortkopplad från runtime | Tabellen är uttryckligen DM1-kompatibel och härledd från XP; drops returnerar nu tomt tills Nexus-källa finns |
| DM1-inherited magiformel/stub i `nexus_v1_magic.c` | Bortkopplad från runtime | Rune-kombinationer och spell-effekter är inte verifierade; kostnad och cast returnerar nu blockerad route utan manaändring |
| DM1-style combat/XP i `nexus_v1_combat.c` | Bortkopplad från runtime; mechanics/engine action-gate stängd | Attackformel, kritisk träff, stamina-kostnad, creature attacks, spells, projectile damage och XP är inte Nexus-verifierade; live-routes muterar inte state tills Saturnkällan finns |

## Meny

| Källa/route | Status | Regel |
|---|---|---|
| `MENU.BPK` | Finns lokalt och hash-/strukturverifierad; alla 162 PRS3-ytor identifieras som 8-bitars indexdata efter DMWeb-avkodning | PRS3:s prefix-mode är ett internt formatfält, inte hostens bytebredd/färgklass. Placering, CLUT/palettbindning och menysemantik är fortfarande separata gates |
| Startup/save/champion-text | Riktiga TEXT4/TABL/FONT012-receipts behålls i engine-state; produktionsbyggaren lämnar textfält tomma och behåller endast bounded no-op/geometry-slots | Hoststrängar får inte ersätta Saturns textkonsument eller VDP2-placering före capture |
| DM.BIN startup/menu-resursankare | Den riktiga europeiska `DM.BIN`-korpusen binder `MENU.BPK`, `yam\\menu.c`, `FONT256.S2D` och `STABG.BIN` vid `0x373B4`–`0x373D8`; pointer-receipten är 1/10/1/1. SH-2-rutinen vid `0x18B60` och dess 0x90-byte literalpool har FNV-1a64 `0xF6D5CC046BAB98C7`, med `FONT256.S2D`-målet vid `0x18BF4`, meny-/STABG-adressmålen vid `0x18C00`/`0x18C20`; `TEXTTABL` finns vid `0x294C0` | Detta bevisar loaderns resursnamn, tabellmarkör och SH-2-adressreferenser, inte menyordning, textkonsument eller VDP1/VDP2-komposition |
| Menytextkonsument | Engine-init behåller nu den verkliga europeiska RLOWFIX TEXT4-receipten (resource 4, 15 strängar), 216-entry TABL-receipt och FONT012 #0/#1/#2 (291/250/710 glyphs) tillsammans med PLRD. DM.BIN har dessutom en vardera av registerkonstanterna `0x25F00006` och `0x25F80000`, som byteverifieras | Hostens chrome-strängar är inte en Saturn textkonsument; `menu_text_consumer_bound` är 0 på riktiga engine-instanser och routes förblir stängda tills TEXT4/TABL/FONT012 samt register-/VDP2-användning är capture-bundna. FONT256 är separat champion/spell-bank och öppnar inte menyrouten |
| `nexus_v1_prs3_decode.c` | DMWeb-reglerna är implementerade; alla 162 retail-ytor i MENU.BPK dekoderar korrekt till deklarerad storlek i real-corpus-testet. Ogiltiga backreferenser avvisas nu fail-closed i stället för att fyllas med syntetiska nollor. Lagrade, icke-PRS3-ytor behåller separat modebaserad bpp-tolkning | Får användas för byteavkodning, men inte som bevis för Saturns palettbindning, VDP1-upload eller skärmplacering |
| `SLEV00-15.BIN` task-entry-profil | Alla 16 verkliga filer passerar den gemensamma 36-byte SH-2-entry-spinen; bounded receipt bevarar setup-immediate, två PC-relativa literaladresser, RTS-gräns och opcode-form | Task-bodyns event-/actionägare är fortfarande okänd; ingen script-dispatch eller syntetisk regel aktiveras |
| `FONT256.S2D` CG-font | DMWeb:s verifierade CG-region ger 242 verkliga 8×8/8-bit tiles; loadern exponerar nu endast dessa 242 bytefönster och fyller inte längre 14 okända glyphplatser med nollor | SCR:s nominella 256 tecken, page/tilemap-kodning, Shift-JIS-mappning och Saturns textplacering är fortfarande obevisade; ingen produktionsrendering utan dessa källor |
| ISO-only Nexus-korpus | Source-detektionen skiljer nu virtuella ISO-hashträffar (`disc.iso::LEV00.DGN`) från lösa extracted-filer; ISO-only-data går genom ISO-läsaren och materialiseras inte som en falsk host-path | DGN/VDP1-runtime är fortfarande capture-gated efter korrekt level-handoff |
| Procedurbyggda save/champion-kommandon | Hostlogik och hit-testgeometri finns; M11-executorn lämnar text, fill-/outline-ramar och obevisad placering oritade. FACE laddas endast till en verifieringsreceipt; M11 placerar inga portraitpixlar | Får inte ersätta Saturn-menygrafik |
| PRS3-fallbackgrafik | Blockerad | Ingen syntetisk ersättningsyta |

## HUD över viewport

| Källa/route | Status | Regel |
|---|---|---|
| Saturn HUD-yta från `STABG.BIN` | Finns lokalt; STMP-container, 11 kartor och DMWeb:s första 40×21-karta avkodad till 320×168 indexpixlar. `nexus_ui_load_stabg()` materialiserar nu den riktiga ytan och bevarar alla 256 paletteord/deriverad RGBA-palette i UI-managern. `DM.BIN+0x376D0` är dessutom bunden till `yam\\menuctrl.c`, 80-entry FNV-1a64 och sju SH-2-adressreferenser; den separata `yam\\vdp2.c`-markören vid `0x38CF4` har sex literalreferenser | Ingen VDP1/VDP2-presentering eller runtime-bindning förrän placeringen är verifierad; sträng-/literalreceipts bevisar inte konsumentsemantik |
| `nexus_v2_hud_overlay.c` / `nexus_v2_hud_runtime.c` | Syntetisk font, labels, ikoner och hårdkodad presentation | Inte längre länkade i `firestaff_nexus`; endast uttryckliga test/probe-targets |
| Runtime-state (riktning, nivå, guld) | Delvis tillgängligt i engine-state, men ingen verifierad Saturn-HUD-bindning | Får inte målas in i syntetisk HUD |
| Blockerad viewport/HUD-route | Tidigare diagnostisk text är borttagen ur M11-spelytan | Blank fail-closed frame; status hör till launcher/statuslager |
| Structure3 textured mesh | DGN-face/texture-payload kan fortfarande inventeras och materialiseras som receipt | Host-rasterisering kräver nu uttryckligt bevis för transform samt pixel/palett/VDP1-semantik; format-/offsetbevis räcker inte |

## Saturn-referens

En användartillhandahållen europeisk Saturn BIOS 1.00 används endast som lokal
referens och har inte lagts in i Firestaff eller distribuerats med projektet.
Den extraherade dumpens SHA-256 är
`96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f`.
BIOS-dumpen kan stödja framtida boot-/VDP-capture, men bevisar inte ensam
Nexus-menyns `MENU.BPK`-placering eller `STABG.BIN`-layout. Därför ändrar den
inte de nuvarande produktionsgaterna.

En lokal Mednafen-körning med samma europeiska BIOS och retail-CUE identifierade
SGID `T-9111G`, SGNAME `DUNGEON MASTER NEXUS`, PAL-region (`0x4`) och 240
visade scanlines. En 13,8228 sekunders operator-lokal videofångst visar en
faktisk exekverad Saturn-titelsekvens.
Detta är runtime-bevis för att den exekverbara titeln måste hållas åtskild från
de avkodade `TITLE.BIN`-resurserna; utan VDP1/VDP2-register- eller VRAM-capture
bevisar det inte en Firestaff-layout.

## Övriga syntetiska vägar

`nexus_v1_rasterizer.c` hade en inbyggd palett med handskrivna färger i
`nexus_fb_init()`. Den är borttagen. En ny framebuffer börjar nu utan färgdata
och kan bara få palett via verifierad TITLE-, STABG- eller VDP1-materialbinding.

Döda fallback-title-deklarationer och fallback-planvärden är också borttagna
ur Nexus-headerkontraktet. Därmed finns ingen deklarerad API-väg tillbaka till
syntetisk titelgrafik.

`nexus_v1_raster_triangle_tex()` renderar inte längre en flat-colour-triangel
när textur eller palett saknas; den avstår från att rita tills ett verifierat
material finns. `nexus_v1_drops.c` hittar inte längre på ett guld-drop för en
okänd creature-typ. Nexus-ljudet loggar nu uttryckligen blockerad playback;
SAL/MAP-filerna får användas som evidens men ingen syntetisk sample eller
falskt "playing"-tillstånd produceras. Den äldre råa sample-index-vägen är
dessutom diagnostisk även när SAL-dekodning lyckas; endast en källbunden
event→MAP-selector från Saturn-trace får senare öppna playback. Oregistrerade
trapp-/chutelänkar
returnerar nu blockerad square-event i stället för en påhittad nivåförflyttning,
och dörrtest utan källbunden inventory passerar inte som om nyckel fanns.
Movement passerar inte heller en dörr som inte har en verifierad öppen status i
dörrregistret. `nexus_try_move()` kräver dessutom registrerade länkar för
trappor, teleporter och chutes innan den ändrar partypositionen.
Den äldre projektilrutinen använde paletteindex och `rand()`-jitter för att
fabricera spell-effekter. Den är nu helt spärrad tills en verifierad Saturn-
effektström och VDP1-bindning finns. Inga host-genererade projektilpixlar får
passera Nexus-rasterizern.
Den äldre dörrritaren hade dessutom DM1-härledd gapgeometri och palettindex
10/14. Den är nu uttryckligen no-draw även när en godtycklig host-textur
skickas in; dörrarnas gameplay-tillstånd kvarstår, men Saturnmaterial,
animationsramar och VDP1-destination måste bindas innan dörrpixlar får skrivas.
Mekanikproben är uppdaterad till detta kontrakt och passerar 285/285; de
Nexus-specifika itemrutterna är explicit blockerade tills en Saturn-katalog
är verifierad.

Det syntetiska BPX0/BPX3-kontraktet i `nexus_v1_bpx_bpk.c` är borttaget från
`firestaff_nexus`-biblioteket. Det kompileras endast uttryckligen i de två
arkivgränsproberna, så testformatet kan inte nå Nexus-produktionen via globbad
källista.

`nexus_v22_modern_assets_pc34.c` har ingen missing-asset-placeholder längre.
V2.2-modern assetpipeline och in-place-väg är fortfarande isolerade test/probe-
moduler; deras cell-till-asset-mappning returnerar NULL tills riktig Saturn-
eller manifestbindning finns.
Modulen är dessutom borttagen från `firestaff_nexus`-bibliotekets produktions-
källista; den byggs endast där dess isolerade katalog-/assettester uttryckligen
behöver den.
V2-belysning, atmosfär, partiklar, mjuk rörelse och touch ligger inte längre i
`firestaff_nexus` och initieras inte av M11. De kvarvarande implementationerna
är endast isolerade test/probe-fixtures tills motsvarande Saturn-bevis finns.

M12-launchern visar Nexus som `V1 Only (V2 Source Blocked)` och dess
`presentationReady`-gate avvisar Nexus V2.2-modernläge. Därmed kan den
procedurbyggda presentationen inte väljas som en till synes färdig route.

Den äldre ITEM.IBS-diagnostikdekodern återanvänder inte längre palette 0 för
DMWebs `FF00`-associationer eller ogiltiga floor-palette-ID:n. Sådana poster
förblir obevisade i receipten; den verifierade runtime-banken och dess
VDP1-gate ändras inte.

## Ny verifiering av HUD-källan

DMWebs `DMNDataFileDecoder.vbs`, `DecodeSTABGBIN`, är nu implementerad som
en separat decode receipt. Retail-`STABG.BIN` verifieras mot dess tre delar:
tilemap, 256-färgers palette och 791 8x8-indexerade tiles. Första kartan är
40x21 celler och kan avkodas till 320x168 indexpixlar; paletteord läses
little-endian enligt DMWebs uttryckliga `LoadSaturnPalette(...,
LITTLE_ENDIAN)`-anrop. Retailfilen innehåller inga vertikala flip-bitar.
Detta bevisar filens byte-/pixelkontrakt, men inte Saturn VDP-placering eller
hur runtime-state binds till statusrutorna. Därför är `nexus_ui_load_stabg`
fortsatt fail-closed och ingen syntetisk HUD-yta har aktiverats.

Den tidigare handskrivna master-paletten i `src/nexus/nexus_v1_palette.c` är
quarantänad och kompileras inte. Den var härledd från kommentarer/storlek, inte
från retaildata. `nexus_palette_init_defaults()` lämnar därför palette-state
tom. Den äldre globala `nexus_palette_load_stone()`-vägen är uttryckligen
blockerad; `STONE.BIN` måste gå genom sin verifierade image-local `pp`-dekoder.

Retail-censusen visar dessutom att `STONE.BIN` är 4 400 byte = åtta 550-byte
`pp`-poster. DMWebs `DecodeRawPPpp` läser varje post som 32×32 4bpp-bild med
16-entry big-endian palette. Den tidigare globala 256-entry-loadern är därför
spärrad; image-local palette-bank får inte slås ihop utan källbevis.

`nexus_palette_stone_pp_receipt()` verifierar nu exakt denna struktur mot
retailfilen och rapporterar 8 poster, 32×32, 16 palette entries och 512
packade pixelbyte per post. `nexus_palette_decode_stone_pp_record()` kan
dessutom läsa en vald verklig post till separata indexerade texels och
image-local palette utan global palette-sammanslagning. Detta är ett
format-/bytesbevis, inte ännu ett bevis för VDP1:s slutliga material- eller
skärmplacering.

Startup-gaten accepterar inte längre en fil enbart för att den heter
`DM.BIN`, `SN_FLOOR.MNS`, `SN_WALL.MNS` eller `LEV00.DGN`. Canonical paths
måste matcha den förväntade hash-identiteten; annars söks en hashmatch eller
gaten rapporterar mismatch. Boot-smoketestet passerar 26/26.

Samlad regression efter ändringen: startup-media PASS, startup-menu PASS,
DGN geometry PASS, audio 90/90 och mechanics parity 285/285. Ingen av dessa
tester aktiverar obevisad VDP1-presentation eller syntetisk fallback.

Runtime-läsaren `nexus_v1_read_extracted_file()` kräver nu samma kända MD5
för canonical loose files som startup-gaten gör. Ett rätt filnamn kan därmed
inte längre få fel `DM.BIN`, level eller UI-media att nå parser/renderkedjan.
Kända assets får dessutom inte falla vidare till DMDF-family/name-heuristik när
hashmatch saknas; sådana routes avvisas direkt.
Level-loadern följer nu samma regel och har ingen name-only `LEV%02d.DGN`
fallback. Hashscan-testet verifierar samtidigt att korrekt omdöpta levels
fortfarande hittas och laddas via innehållsidentitet.
För ISO-källor används den öppnade directory-entryn oförändrad och den
befintliga ISO-entry/source-receipten äger hashadmissionen; extracted bytes får
inte ersätta en disc-entry via filnamn.

## Kvarvarande källgap

`CHAMPIONS.DAT` krävs inte längre av Nexus boot-profilen. Retail-listan från
DMWeb innehåller inte den PC-liknande filen; Nexus identitet och rosterdata ska
fortsatt komma från Saturn-källorna, bland annat `DM.BIN` och `FACE.BIN`.
Detta tar bort ett syntetiskt filkrav utan att ersätta det med en namnbaserad
fallback.

Alla primära Nexus-assets finns i `/Users/bosse/.firestaff/data/nexus`.
Återstående gap gäller alltså inte assetförekomst utan byte-/pixelsemantik och
verifierad runtime-bindning.

1. Bind de fem verifierade `TITLE.BIN`/`TITLE.CG`-bilderna och `LOGOBG.DG2` till korrekt uppstartsroute utan obevisad 320×200-cropping. `test_nexus_v1_title_mapd_real` verifierar nu retailens fem MAPD/TIBG-kartor, tilepixlar och paletteord; endast displayplaceringen återstår.
2. Bevisa `MENU.BPK`-ytornas Saturn-placering, palettbindning och betydelse i menyn.
3. Bevisa Saturns VDP1/VDP2-placering för den nu verifierade `STABG.BIN`-dekodningen och bind HUD-ytan till verifierat runtime-state.

Retail-census för `STABG.BIN` stärker strukturbeviset utan att upphöja en
tolkning till grafikbevis: första kartan är 40×21 celler, filen har 11 kartor,
CLUT-regionen är 512 byte och pixelregionen är 50 624 byte. Cellreferenserna
ligger inom pixelregionen. DMWeb-dekodern verifierar 8x8-byte-indexerade tiles
och little-endian 5-bitars RGB-palette; det återstår att bevisa Saturns
VDP1-blitplacering och runtime-bindning från Saturn-körningen.

Källor: `src/nexus/nexus_v1_bpk_archive.c`,
`src/nexus/nexus_v1_engine.c`, `src/nexus/nexus_v1_ui_surfaces.c`,
`src/nexus/nexus_v1_startup_menu.c` och M11-handoff/rendering i
`src/engine/m11_game_view.c`.

Runtime-captureprovenans och reproduktionsregel finns i
[`docs/NEXUS_RUNTIME_CAPTURE.md`](NEXUS_RUNTIME_CAPTURE.md).

Den lokala Saturn-körbara `DM.BIN` innehåller dessutom retail-markörerna
`PRS3`, `MENU.BPK`, `STABG.BIN`, `yam\\menu.c`, `yam\\menuctrl.c` och
`yam\\vdp2.c`. Det bekräftar att arkiv-, meny- och VDP2-vägarna finns i det
levererade mediet, men bevisar inte ensamt Saturns VDP-placering eller
placeringsflöde. Därför förblir STMP-pixeltolkningen och meny-layouten
evidens-only tills motsvarande byte-/pixelbevis är säkrade.

Ytterligare palette-spans från ytor är nu fail-closed: negativa offsets,
överskridna paletteintervall och för korta källor avvisas före läsning.
Rutinen nollfyller inte längre saknade poster och en partiell yta kan inte
ensam markera hela palette-state som renderbar. Det ändrar inte den öppna
VDP1/VDP2-bindningen; den kräver fortfarande verklig Saturn-capture.

DM.BIN har dessutom en separat källbunden VDP1-register-/VRAM-state receipt:
den verifierar den unika statiska registertabellen och SH-2-literalflödet till
VDP1-registerfönstret och VDP1-VRAM-baskandidaten. Den bevisar inte en
STABG-specifik kommandoemission, palette-lane eller slutlig skärmplacering.

DMWebs formatbeskrivning anger big-endian som standard och dokumenterar PRS3:s
kontrollbitar, literal-/kopieringskod och relativa 12-bitarsoffset. Samma källa
beskriver `MENU.BPK` som Nexus UI-grafik och `STABG.BIN` som champion status box
graphics. DMWebs medföljande dekoder visar dessutom att `TITLE.CG` inte ska
blitas direkt: `TITLE.BIN` MAPD väljer tile-index, h/v-flip och 16-färgspalett
för fem 64×28-bilder. Den beskriver inte Firestaffs slutliga 320×200-output- eller
VDP1/VDP2-bindning: [DMWeb Nexus file formats](http://dmweb.free.fr/community/documentation/dungeon-master-nexus/file-formats/),
[DMWeb Nexus Data File Decoder](http://dmweb.free.fr/community/tools/dungeon-master-nexus-data-file-decoder/).

MNS-gränserna är nu härledda från hela den lokala, hashverifierade retailkorpusen
med 30 modeller. `VEXIRK.MNS` behåller 64 TEXT-deskriptorer och
`D_GOLD.MNS` behåller 11 MOTN-tabeller; tidigare gränser på 16 respektive 8
orsakade tyst bortfall av riktiga modeller. Parsern avvisar i stället deklarationer
som inte ryms, utan att skapa en trunkerad giltig modell. Testet dekoder alla 30
MNS-filer och renderar 815 källtexturer. Detta är fortsatt parser-/materialbevis,
inte bevis på Saturns slutliga VDP1-kommandoordning eller viewport-pixlar.

Alla 30 retail-MNS-identiteter finns nu också i den canonicala Track 1
MD5-katalogen, så `nexus_v1_load_model()` kan faktiskt öppna verifierade
creature-modeller (t.ex. `SCORPION.MNS`) från den riktiga dataroten. Detta
öppnar inte modellrendering: VDP1-kommandon, placering och pose är fortfarande
capture-gated.

DGN-korpusen motbevisar också en direkt Structure1B-selektor-till-MNS-ordinal:
de 16 europeiska LEV-filerna använder texturerade selektorer i intervallet
`0x01..0x7D`, medan både `SN_FLOOR.MNS` och `SN_WALL.MNS` har 15 TEXT-deskriptorer.
Firestaff lämnar därför Structure1B-bindningen stängd tills Saturnens verkliga
selektortransform och VDP1-materialägarskap har fångats. Materialplaneraren
avvisar dessutom nu material- och Structure2-index utanför den dekoderade
bankens bounded surface-count, så en felaktigt antagen bindning kan inte läsa
utanför källbanken.

DGN Structure2-dekodern följer nu också DMWebs palette-ID-regel: en descriptor
med `Palette offset = 0` återanvänder den senaste tidigare paletteassociationen
med samma ID; den får inte falla tillbaka till palette 0. Hashverifierad testning
av LEV00–LEV15 dekoderar 1 678 riktiga descriptors, varav 1 553 indexed-4bpp
och 125 direct-color-555. Det bevisar descriptor-, pixel- och palettebytes, men
inte ännu Structure3:s Saturn-VDP1-upload, UV-/draw-order eller viewportplacering.

## Aktuell produktionsaudit 2026-08-06

En ny genomgång mot aktuell `main` skiljer produktionsbiblioteket från de
uttryckliga test-/probe-fixtures som fortfarande använder syntetiska bytes för
parserkontrakt. `firestaff_nexus` länkar inte `nexus_v1_bpx_bpk.c`, S2D-
textlayouten, screen-text-wrappern, MNS-host-renderaren eller de procedurala
V2-HUD-modulerna. Den länkade viewporten returnerar ingen färgtriangel,
fallbackpalett eller procedurmodell när Saturnmaterial saknas.

Retailkörningen med `/Users/bosse/.firestaff/data/nexus` passerar de fokuserade
regressionerna för DM.BIN-startupankare, HUD-layout (80 poster), HUD-hitrects
(40 poster), championpanel, MENU.BPK-ytklassning, SLEV/SAL-discovery,
SAL-proveniens, ljud-runtime-receipt, SAL-dekodning, TITLE MAPD/TIBG och
save-roundtrip. Track-1-readinessproben passerar `29/0`; dess real-data-BMP
är avsiktligt svart och får inte räknas som en Saturn-skärmbild.

Det finns därför ingen verifierad lokal retailfil att byta in för de sista
presentations- eller ljudluckorna. Nästa källtroget tillåtna steg är en
instrumenterad Saturn-capture som binder `MENU.BPK`/`STABG.BIN` till
VDP1/VDP2, Structure3 till draw order och SLEV/SAL till selector/SDDRVS.
Stock Mednafen avvisas av `docs/NEXUS_RUNTIME_CAPTURE.md` eftersom den saknar
Firestaffs capture-hook; ingen syntetisk capture får ersätta den.
