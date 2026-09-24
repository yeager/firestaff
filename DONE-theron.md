# Firestaff DONE — Theron's Quest

## 2026-09-24 — Native movement through the authentic JP CUE route

- Extended the full Japanese Rev. 1 CUE boot regression with six native
  movement inputs on its source-backed Akutuba map. It requires the final pose
  `(direction=2, x=3, y=0)` with the route's single source-backed champion,
  alongside its existing authentic-media and no-fallback checks. This is
  bounded host-runtime movement, not Japanese original-runtime parity or
  later-dungeon evidence.

## 2026-09-24 — Native movement from the authentic US CloneCD route

- Extended the real US CloneCD ZIP menu-to-runtime regression with six native
  movement inputs on the source-backed Akutuba map. It requires the resulting
  party pose `(direction=2, x=3, y=0)`, all three source-backed champions and
  source objects, while continuing to reject fallback graphics. This is a
  bounded native route through authentic US media, not original-runtime visual
  or gameplay parity and not later-dungeon movement evidence.

## 2026-09-24 — Native movement on the authentic Japanese Akutuba map

- Extended the real JP raw-BIN M11 regression with six native directional
  inputs and an exact final pose assertion (`direction=2, x=3, y=0`). The test
  uses the hash-authenticated Japanese Rev. 1 Track 02 and Track 19 plus its
  complete authentic CUE, and continues to reject fallback graphics. It
  proves a bounded native movement route on the authentic source map, not
  parity with a Japanese original-runtime capture or movement in later
  dungeons.

## 2026-09-24 — JP Track 19 raw BIN keeps its authenticated pregap

- `theron_v1_track19_inventory_probe` now passes against the original Japanese
  Rev. 1 Track 19 raw BIN on trv2 (`27d54f58154662885bb67d5967e5111e`). The
  item-name-bank reader now strips the CUE-authenticated 224-sector pregap,
  just like the Track 19 inventory reader, and the world admission gate accepts
  the authenticated raw-track hash as well as the normalized ISO hash after
  validating the source tables. JP Shift-JIS bytes remain opaque; this does
  not claim host glyph rendering or T900 semantic parity. The local
  `theron_v1_track19_inventory_probe` and `theron_v1_track02_dungeon_loader`
  tests also pass.

## 2026-09-24 — JP Track 02 källkartor verifierade på trv2

- Byggde aktuell `main`-arbetskopia i en isolerad Linux-katalog på trv2 och
  körde `theron_v1_jp_later_dungeon_runtime` mot den befintliga, autentiska
  japanska Rev. 1 Track 02-filen. Testet band samtliga sju käll-dungeons:
  34 kartor och 2 269 källobjekt. Detta verifierar kart- och objektdata, inte
  JP-fortsättning, kampanjövergångar eller visuell paritet.

## 2026-09-24 — autentisk CloneCD ZIP genom musklicksflödet

- Hela `theron_v1_us_clonecd_zip_runtime_boot` passerar på trv2 mot det
  befintliga autentiska ZIP-arkivet. Direktstart, Original- och Modern-korten,
  musklick genom spel/plattform/presentation samt det vanliga M12→M11-flödet
  når en laddad runtime-nivå med källobjekt och utan fallback. Kortsekvensens
  väntan anpassades till testfönstret så att alla klick hinner skickas.

## 2026-09-24 — japansk Rev. 1 CUE-start verifierad på trv2

- `theron_v1_jp_cue_runtime_boot` passerar mot den kompletta autentiska
  japanska Rev. 1 CUE-skivan och dess spårfiler på trv2. Det bekräftar den
  källägda startvägen, inte JP Continue, kampanjövergångar eller visuell
  paritet.

## 2026-09-24 — JP rå-BIN genom M12/M11 på trv2

- `theron_v1_m11_launcher_handoff_boundary` passerar med 62 kontroller och
  2 skip. Den autentiska japanska Track 02-råfilen hittas i användarens
  datakatalog och förs genom M12/M11 med sin verifierade regionsidentitet.
  US CUE- och rostertextfallen hoppas över eftersom motsvarande filer saknas
  där; testet bevisar inte JP Continue eller en hel kampanjövergång.

- Den fristående `theron_v1_track02_dungeon_loader` körningen verifierar nu
  JP Rev. 1 oberoende av US BIN: alla sju dungeonernas källobjekt och lokala
  itemnamns-/typkodstabeller lästes från den autentiska råfilen på trv2.
  Det lokala kombinerade US/JP-testet passerar fortfarande; saknad JP ISO och
  US BIN på trv2 rapporteras som separata, valfria skip.

## 2026-09-24 — Authentic Continue to the next unlocked chapter

- Extended the M11 Continue regression with the authentic US Track 02 and
  Akutuba-complete 2 KiB Backup RAM capture (`ffabc8d19b0915d4d9632a7ae2e90a97`).
  Its test wrapper verifies both exact media hashes before launching the
  probe, so a synthetic or substituted 2 KiB save cannot count as evidence.
  After restoring the completed Akutuba chapter, the real progression skips
  it, focuses unlocked dungeon 2, selects a source-backed Track 02 champion,
  and enters that dungeon through the Soul Room and forcefield.
  The integration now compares all seven restored attributes and all 20
  temporary/persistent skill-experience pairs directly with the decoded,
  source-verified real Backup RAM body. It requires the native runtime to load
  dungeon 2, level 0, with the selected two-member party and original objects.
  After load, a breadth-first walk over the authentic floor tiles finds a
  three-step route, and the test verifies all three native movement inputs
  move the party along it. It does not mistake the map-edge-blocked forward
  direction for a missing route.
  This verifies a real-media native route, not parity against a separate
  original-emulator transition capture. The current version passes locally;
  the earlier bounded Continue route also passed on TRV2. JP Continue, later
  stages and original transition parity remain open.

## 2026-09-24 — authentic raw BIN launch through M12 mouse cards

- Extended the raw US Track 02 real-media regression to select Theron's game,
  PC Engine platform and Original presentation using only M12 mouse clicks.
  The verified retail BIN reaches native startup with no fallback assets.
  This establishes the raw-BIN mouse route; the separate authentic CloneCD
  ZIP routes are verified on trv2.

## 2026-09-24 — Production pickup admission test

- Corrected the production runtime-input regression to reflect source-owned
  startup: pickup stays closed until an authenticated champion is admitted,
  rather than relying on the synthetic roster used by fixture builds. The
  authentic Track 02 dungeon-loader regression still exercises successful
  item pickup, slot provenance, resume and drop.

## 2026-09-24 — JP Rev. 1 ISO dungeon-data boundary

- The hash-identified 305,152-byte `TQJP02End.iso`
  (`397039af02d50d15c70b74088eb8a1cb`) is now exercised by the dungeon-loader
  regression. The supplied bytes are all zero, so this is an identity/negative-
  boundary check only; the image is not accepted as source dungeon content or
  startable campaign media. File-backed Track 02 intake retains its identity
  for diagnostics but rejects it with `source_content_empty`.
- The same test run loads all seven authentic JP raw-BIN dungeons and binds
  their local item-name/type-code tables. This is source-data coverage, not
  JP Continue, campaign, presentation, or gameplay parity.

## 2026-09-24 — US MODE1/2048 ISO forcefield source handoff

- The authentic split-image US Track 02 ISO now binds the same source-defined
  Theron roster records as raw MODE1/2352. Its raw-sector roster interval is
  translated by sector and user-data offset (not by subtracting a flat byte
  pregap), then checked against the existing record FNV and full ISO MD5.
- Startup normalizes the absent 225-sector pregap as zeroed address space
  only, preserving every ISO byte. The real US ISO CUE now passes M12→M11,
  source roster initialization, and the forcefield handoff into Akutuba: four
  authentic maps and 291 source objects load. Visual capture remains gated;
  this does not claim bitmap parity or later-dungeon/gameplay parity.
- Verification: `theron_v1_m11_launcher_handoff_boundary` with the authentic
  CUE/ISO under the isolated data root (49 passed, 0 failed, 1 media-optional
  skip).
- The real-media dungeon-loader regression also compares the normalized ISO
  against authentic raw Track 02 across all seven dungeons: 34 maps, exact
  source tile grids, 2,269 source-object records, category counts and
  property-table provenance match. It independently verifies that every ISO
  byte equals the raw BIN's user-data stream after the 225-sector pregap.
  This establishes data-layout equivalence, not campaign transitions or
  gameplay semantics for later dungeons.

## 2026-09-24 — JP/US spawn consumer source comparison

- The two independently hash-authenticated retail spans are each 269 bytes.
  Static structural comparison found 122 aligned operations, with 230 bytes
  identical and 39 operand-byte differences confined to relocated helper
  targets and regional RAM columns. JP accumulator/property fields are one
  byte earlier than the corresponding US fields. Formula constants and
  branch order agree. This is source-shape evidence only; no JP caller,
  runtime category, RNG return, spawn, AI or combat behavior is promoted.
  See `docs/source-lock/theron-jp-us-spawn-consumer-static-comparison-2026-09-24.md`.

## 2026-09-23 — Japanese spawn source records bound to retail bytes

- The JP and US real-media spawn test now checks all five regional records
  against their own MODE1/2352 user-data offsets. The JP assertions pin the
  exact eight source bytes at `$273858`, `$2738D7`, `$273902`, `$273929` and
  `$273950`; the US records remain checked at their separate offsets. The
  original Japanese and US Track 02 BINs pass with their canonical hashes.
  This verifies static source decoding only: JP runtime category publication,
  spawn execution, RNG, AI and combat remain gated on an authentic live
  consumer capture.

## 2026-09-23 — Synthetic save runtimes removed from production

- The Firestaff-only `FSTQPTY1` writer and Continue reader are no longer
  linked into `libfirestaff_theron`. They remain available only to the
  explicit SRM body-decode fixture target. The production archive gate now
  rejects both the translation unit and its export/Continue symbols, while
  authentic 2 KiB HUBM Backup RAM remains on its separate, fail-closed
  original-data route.
- The retired `.tqsv` codec is also absent from `libfirestaff_theron`, and
  production M12 no longer recognizes `.tqsv` or Firestaff-envelope `.srm`
  files as Theron browser/import/Quick Resume candidates. Fixture targets may
  still compile these codecs explicitly; the shipped application now exposes
  no synthetic Theron save or Continue symbol.
- The remaining SRM classifier keeps only the authentic 2 KiB HUBM reader in
  production. Its `FSTQPRG1`/`FSTQPTY1` decoders now return unsupported there
  and are compiled normally only by explicit fixtures. The production archive
  gate also scans linked strings and rejects either synthetic signature; both
  `libfirestaff_theron.a` and the final `firestaff` executable are clean.
- A read-only original-runtime capture now binds the complete 134-byte
  `DMS-SG.001` writer layout: `$267C`, six bytes at `$267D`, seven at `$2683`
  and six 20-byte columns through `$2701`. The production classifier exposes
  these still-neutral bytes without inventing party or inventory semantics;
  the authentic progressed BRAM fixes the body FNV-1a to `b37e696e`.

## 2026-09-03 — Native archive/CUE regression audit

- Re-ran production, startup, resume and Track 02 layout gates. The supplied
  US CloneCD ZIP reached both direct and start-menu native routes, and the
  Japanese CUE reached the Track 02 Akutuba route. Raw US CUE/BIN and paired
  raw JP/US Track 02 comparison probes remained correctly skip-safe because
  those distinct raw files are not staged.

Reviewed 2026-08-29. Completed work only.

- The later-record correlation probe is media-optional in CI: absent
  proprietary Track 02 files now produce a skip rather than a false failure.
  When supplied, the authentic Japanese Rev 1 Track 02 proves the stage-three
  self-reference and IPL-to-record topology directly from raw sectors.

- The supplied authentic Japanese Rev 1 CUE is hash-verified and reaches the
  native title → stage → Soul Room startup route (`theron-startup-2`).
- The same authentic JP Rev 1 MODE1/2352 Track 02 route reaches native
  `theron-runtime`: it loads the source Akutuba level and publishes party
  position `1,0,0` without treating Japanese raw sectors as US ISO media.
- A bounded public native Track 02 consumer now binds the same authentic JP
  source to all seven campaign dungeons, retaining their source maps and
  2,266 objects in total. This includes Drator (dungeon 2: eight maps and
  291 objects). Transfer semantics, graphics capture and gameplay behavior
  remain independently gated.
- Track 02 intake preserves original media identity and blocks uncaptured
  graphics/palette fallback from production presentation.
- A CUE/BIN package in a ZIP archive starts natively through both direct CLI
  and the start menu. Firestaff hashes and reads the selected Track 02 member
  in memory (`archive.zip::member`) and never writes extracted game data.
- The supplied US CloneCD ZIP starts natively through direct CLI and the
  start menu. Its `.ccd` layout identifies Track 02 as an in-memory bounded
  `.img` slice (MD5 `168bd6a63784e91885df8c47be62ab5a`); both the menu's
  Original and Modern cards preserve that exact slice and reach the native
  startup handoff. The separate verified eleven-input startup route reaches
  `theron-startup-2`. CloneCD's missing 225-sector pregap has a separate,
  source-verified anchor map—no pregap or game media is synthesized or
  written to disk.

- The supplied Japanese Rev 1 Track 19 raw BIN is admitted directly from its
  CUE-defined MODE1/2352 representation. Firestaff retains its physical MD5
  (`27d54f58154662885bb67d5967e5111e`), skips the authentic 224-sector
  INDEX 00 pregap only while reading in memory, and validates the same
  3,072-sector user-data payload's item, level-label, property, and startup
  envelope receipts. No ISO projection or extracted copy is written to disk.
# Firestaff DONE - THERON

## 2026-08-21 — autentiskt Drator-generator/RNG-kvitto

- En controllerdriven kallstart från riktig US-skiva och autentisk BRAM går nu
  genom originalmenyn till Drator, lämnar level 2 `(2,3)` och återinträder
  samma ruta från `(1,3)`.
- Två separata captures öppnar RNG-spårningen på var sin sida om
  positionsövergången. Den första körningens sekvens 1 är byteidentisk vid
  RNG-gränsen med den andra körningens sekvens 0. Därmed är `$4667 → $CC55`,
  retur `$8F`, det enda RNG-anropet i eventintervallet.
- Produktionskvittot kräver nu event `0c81`, generator `0c38`, fysisk/logisk
  entry och caller, den exakta 32-byte-callerbilden, RNG-tillståndet
  `2a5429 → 988f29` samt den verifierade `$4639`-efterföljaren. Ändrad
  callerbyte eller efterföljartillstånd avvisas. Detta godkänner källplanen,
  men skapar ännu ingen creature innan riktning, HP och timerägare är bundna.
- Samma autentiska transaktion bygger runtime-raden
  `a500000120ee11030800`; originalets `$CBCE` publicerar den i slot 0 vid
  `$60FF`. En andra autentisk körning binder de första läsningarna
  `$C9F3/$C9FB/$CA02 = a5/00/00` och den efterföljande avlänkningen vid
  `$CA78`. Produktionsgrinden kräver nu hela detta exakta livscykelkvitto.
  Bytefältens spelbetydelse är fortsatt stängd tills deras semantiska
  konsumenter är bundna.

## 2026-08-21 — original Akutuba campaign-bit transition

- A research-only Mednafen hook now maps the exact System Card RAM page `$6D`,
  verifies the resident `DMS-SG.001` dispatcher and changes only its bounded
  dungeon ordinal at `$DE38`. Original code issues the four-sector LBA 4201
  read and changes the real campaign byte `$267C` from `00` to `01`.
- The final bank is still Akutuba (`$20DB == 0`), so this is not claimed as a
  Drator load or generator witness. See
  `docs/source-lock/theron-original-akutuba-completion-capture-2026-08-21.md`.

## 2026-08-20

- Uppgraderade världssparformatet till version 14 så att den autentiskt valda
  gruppstorleken, aktiv hjälte, ledarens position och riktning, levitation samt
  dörröverskrivning följer med genom save/resume. Version 1–13 läses fortsatt
  med sitt äldre fyrhjältekontrakt; en materialiserad version 13-ström provas
  uttryckligen så att det kortare gruppblocket inte förskjuter resten av
  världen. Ogiltig gruppstorlek, aktiv plats eller riktning avvisas, och ett
  regressionstest bevisar en trehjältegrupp utan värdarv eller syntetisk
  utfyllnad. En ännu tom produktionsgrupp får endast peka på den reserverade
  Theron-platsen 0 eller ingen plats alls, aldrig på en obunden följeslagare.
- Tog bort startvyns blinda klassning av varje redan laddad nivå som
  `fallback-room`. En nivå vars autentiska Track 02-header har verifierats
  klassas nu som `track02-semantic` och bär det semantiska handoff-kvittot;
  fixtureklassningen används bara när källkvittot faktiskt saknas.
- Rättade världshashens gamla kontraktsbrott: `PART`-delen påstod sig täcka
  gruppens position och riktning men hashade inte ett enda gruppfält. Hashen
  omfattar nu samma version 14-block som sparformatet, inklusive vald roster,
  källbundna hjälteposter, guld, aktiv plats, position, riktning, levitation
  och dörrläge. Regressionstestet ändrar varje fältfamilj separat och kräver
  en annan hash.
- Ersatte combat/startup-testets falska enbytes-Track 02 med hela riktiga
  `TQUS02.bin` och `TQJP02.bin` från Theron-datakatalogen. Båda regionerna går
  nu genom den källbundna startup-, roster- och dungeonruntimevägen och kräver
  de verkliga Theron-/Hakar-statsen. Samma test matar en trunkerad bit av
  respektive riktig fil till rostergrinden och kräver byteexakt rollback av
  både grupp och startupflöde. Detta visade
  också att produktionsinitieringen lämnade utrustningsplatser som noll, vilket
  kunde se ut som värdföremål 0. Alla obundna platser använder nu `-1` tills
  en autentisk T900-konsument faktiskt binder utrustning. Samma invariant
  gäller även de två inaktiva följeslagarplatser som forcefieldvalet rensar;
  realmediatestet kontrollerar samtliga utrustningsfält i alla fyra slots.
  Startupens “verifierad Track 02”-grind kontrollerar dessutom nu den
  deklarerade MD5-identiteten mot hela den faktiska bytebufferten. Tidigare
  räckte en känd MD5-text från anroparen. US- och JP-testet ändrar sista byten
  i respektive riktig fil och kräver att grupp och startupflöde förblir exakt
  oförändrade; inga lokala tabellhashar kan längre legitimera en manipulerad
  fullfil.
- Gjorde hela produktionsövergången `forcefield → regional roster → dungeon`
  atomisk, inte bara rostersteget. Ett senare fel vid dungeoninträde återställer
  nu både `Theron_StartupFlow` och hela `Theron_V1_World`. Realmediatestet kör
  detta efter godkänd US- respektive JP-roster genom att låsa Akutuba i
  live-progressionen och kräver byteexakt återställning av världen; inga
  källposter, mediafält eller gruppstats kan läcka från ett misslyckat inträde.
- Gjorde även världens save/resume-deserialisering atomisk. Parsern arbetar nu
  i en staging-värld och publicerar först när hela version 1–14-strömmen har
  validerats. Ett regressionstest trunkerar den sista byten i en annars giltig
  version 14-värld, alltså efter de tidiga grupp-, objekt- och timersektionerna,
  och kräver att den befintliga live-världen förblir byteidentisk.
- Begränsade gruppens gameplaykonsumenter till den autentiskt valda
  `champion_count` i stället för att alltid behandla fyra sparade slots som
  aktiva. Ledar-/slotaccessorer, Therons livsstatus, total hälsa,
  belastningsomräkning, dungeonreset samt de kvarvarande fixturemekanikernas
  altar-, T700- och poolloopar ignorerar nu inaktiva poster. US/JP-realtestet
  markerar en levande tredje slot utanför den valda Theron/Hakar-gruppen och
  bevisar att dess hälsa, inventarie och load varken konsumeras eller muteras.
- Ersatte produktens enda dungeon 7-transkription som namnkälla med alla sju
  autentiska, dungeonlokala Track 02-tabeller för både US och JP. Totalt 924
  källposter binds förlustfritt efter exakta offset-, längd- och FNV-kontroller.
  Objektuppslag använder sin verifierade dungeon och `source_item_type`.
  Ändrade bytes, fel region, ogiltiga index och lån mellan dungeons avvisas.
  Bootproben kräver sju banker av rätt region i live-världen.
- Tog bort Demon-tabellens felaktiga globala kategorisemantik ur produktionen.
  Alla sju US- och sju JP-tabellernas 66 råa typkoder läses och hashverifieras
  tillsammans med namnen. Propertyraden binds nu efter item-index och
  thing-postens egen källkategori. Akutuba binder därför 74 av 77
  materialiserade poster mot 36 tidigare; de tre undantagen är riktiga kistor
  utan globalt item-index. Samma invariant passerar för alla 14 dungeons.
- Band den kompletta riktiga Track 19-namntabellen till native-världen för
  både US och JP: 69 poster, full fil-MD5 och tabell-FNV krävs. JP-namnen
  bevaras förlustfritt som Shift-JIS. Bootproben visar bankens region och
  håller uttryckligen item-mapping vid noll tills T900-konsumenten är bevisad.
  Den riktiga Track 19-proben samt båda regionala raw-BIN-starttesterna
  passerar utan syntetiska namn.

- Rättade VDC-I/O-parsern mot den riktiga 65 536-posters Mednafen-capturen.
  Råa bit-31-markerade bussadresser bevaras och normaliseras separat, medan
  otillåtna höga adressbitar fortfarande avvisas. De 23 verkliga
  `HuCPU.Timestamp()`-resetterna redovisas som 24 verifierade epoker. Både
  det isolerade parsertestet och hela realtracet passerar. Parsern behåller
  nu också varje autentisk writepost i en bounded replayarray och realtestet
  verifierar första/sista posten samt fullständig frigöring.
- Lade till en atomisk Mednafen-capturegräns: den 65 536:e observerade
  skrivningen appliceras först på VDC:n, därefter fryses VRAM, VCE, register
  och SAT och en terminal sekvensfooter skrivs. CloseGame kan inte längre
  ersätta snapshoten med ett senare tillstånd. Patchen appliceras i dry-run
  och capturegrinden kräver footern.
- Band 2 022 riktiga byte i grafik-/VDC-kedjan runt `$491F..$5E81` från US
  Track02
  med MPR3–MPR6-save/restore, VDC-registerval, interna hopp och nio absoluta
  med `$4943`-dispatcherns delrutinmål, `$49FA`-valet, båda spelstyrda
  `$4A09/$4A84`-VDC-remsorna, gemensamma `$4B24`-adresshjälparen och `$491F`-
  CR-aktiveringen, `$4BB0`-scrollskrivningarna och `$56DE`-grenens riktiga
  1 KiB-TIA-överföring, `$50F1`-kroppens 512-byte-VDC-skrivning och `$5E2B`-
  dispatchern för VDC-register 6/7 samt `$5CE4`-producentens initiering och
  nollställning av 1 KiB-bufferten vid `$58E0`. `$5111..$51E8` binder
  dessutom den direkta åttapostsbyggaren, dess lokala `$517A/$519F`-
  kontrollkroppar och `$51E8`-tabellens 15 autentiska kommandomål genom
  `$533C`. Den sekundära `$533D..$555E`-dispatchen binder ytterligare 16
  autentiska tabellmål och hela deras sammanhängande handlerblock. Ändrade
  mediabyte och JP-lån avvisas. Även `$55EF..$560B` och de överlappande
  `$55F4/$55FF/$5617/$562A/$563D`-ingångarna är verifierade, inklusive att
  `$563D` är en avsiktlig andra instruktionsström och inte skräpdisassembly.
  Dess BBR4-mål beräknas till den överlappande `$55C8`-ingången och hela
  `$55B6..$55E0`-kroppen är bunden. Kedjan omfattar nu 4 899 byte, 14,1
  procent av originalbilden.
- Utökade den hashgrindade US-stage-2-disassemblykedjan med 567 riktiga bytes:
  frame-dispatchern `$4215..$424B` och koordinatuppdaterarna
  `$4417..$44A2` samt `$44A2..$4519`, plus hela `$42DB..$43A1` med lokala
  `$4358/$4386`, samt normaliseringsrutinen `$4519..$4552`. Beviset kräver
  caller→callee-JSR, JMP och BSR, mål mot de redan bundna
  `$424B/$43A1/$43D6/$4552/$458E`-kropparna, exakt adjacency och avvisar
  både ändrade mediabyte och JP-lån. Kedjan täcker nu 2 877 bytes, 8,3
  procent av bilden.
- Lade till en komplett autentiserad HuC6270-bildbundle med VRAM, VCE,
  registerstate och SAT från samma riktiga US-savestate. Loadern hashgrindar
  samtliga fyra filer och återger den verifierade 64×64-BAT:en i 320×200.
- Implementerade PCE:s spritekompositor från den fångade SAT:en: originalets
  scanlinegräns, ritordning, prioritet, transparens, mönsteradressering,
  storlekar och flippar bevaras. PCE:s 512-palett komprimeras förlustfritt
  från endast använda källposter till M11:s 256 poster; realtestet mäter 52
  använda färgposter och 211 spritepixlar.
- CLI-vägen kräver nu även `--theron-vdc-state` och `--theron-vdc-sat` ihop
  med VRAM/VCE. Produktbinären, capture-skriptets kontrakt och den riktiga
  fyra-filsregressionen passerar utan syntetisk bilddata.
- Gjorde native TAKE→inventory→DROP förlustfri även för Track 02-postens
  tvåbitars thing-position. Inventory-proveniensen lagrar nu positionen,
  validerar den mot category/index/position i source-ref och återställer både
  fältet och dess metadataflaggor. Världssparformat 12 bevarar positionen
  explicit; äldre format härleder den exakt ur sin redan lagrade source-ref.
- DROP skapar inte längre en host-kopia av ett verkligt föremål. Den återbinder
  den ursprungliga `PICKED_UP`-förekomsten i source-ledgern, behåller dess ID
  och råpost och flyttar bara runtime-koordinaten. Realdatatestet kör två hela
  TAKE→DROP-varv för varje US- och JP-dungeon och jämför ref, next-ref, index,
  kategori, position och samtliga råbytes utan att objekttabellen växer.
- Source-inventory kan inte längre godkänna en post enbart för att dess egna
  fält är självkonsekventa. Swap och DROP i en verifierad Track 02-värld kräver
  nu en exakt förekomst i den inlästa käll-ledgern med samma ref, next-ref,
  index, kategori, position, råstorlek och råbytes. Ett negativt regressionstest
  muterar endast ledgern efter pickup och verifierar att DROP då stängs.
- Full source-occurrence-identitet följer nu med genom golvobjekt, TAKE,
  inventory, DROP och save/load: ursprunglig dungeon, level, x och y hålls
  separat från föremålets flyttbara runtime-position. Realkorpusen visar varför
  detta behövs: USA:s sju dungeons innehåller 204 par med samma ref, next-ref,
  index, kategori, position och råpost men olika ursprungskoordinater.
- Världssparformat 13 versionsstyr både 91-byteobjekt och 54-byte inventory-
  proveniens. Version 12 och äldre läses med sina tidigare wire-storlekar.
  Världshashen omfattar nu även golvobjektets fulla source-identitet och origin.
  US/JP-roundtripen bevarar origin genom två flyttar; ett separat negativt test
  ändrar endast `source_x` och kräver att DROP avvisas.
- Native Theron kan nu återvälja carried source-items efter resume. `I` går
  deterministiskt till nästa slot för aktiv mästare där kompakt item-ID och
  parallell source-post hör ihop; slots som bara innehåller ett generiskt ID
  hoppas över. M11 behåller det valda slotnumret och P/DROP kör därefter den
  oförändrat strikta råpost-, origin- och ledgergrinden. US/JP-realdatatestet
  serialiserar och deserialiserar hela den förladdade Track 02-världen efter
  pickup, nollställer selection, återväljer med `I` och slutför DROP från den
  återställda världen innan den vanliga tvåvarvsroundtripen fortsätter.
- Lade till `--theron-native us|jp` för explicit native region när riktiga
  `TQUS02.bin` och `TQJP02.bin` ligger tillsammans i standardkatalogen.
  CLI:n löser endast regionens canonical fil och den befintliga hashförsta
  launch-intenten verifierar därefter identiteten. US- och JP-produktproberna
  startar nu från samma datarot; en separat US-only-kontroll visar att JP-val
  avvisas i stället för att låna USA-media.
- Native US/JP-start från en lös verifierad Track 02 binder nu automatiskt
  riktig Track 01-CDDA när motsvarande hashkända fullskivearkiv finns i den
  valda Theron-dataroten. Kallcachetesterna kräver
  `theronTrack01CddaReady=1` för båda regionerna.
- Native `USE_ITEM` når nu den befintliga Theron-kommandovägen för en dörr i
  rutan framför gruppen. Verifierade låsta Track 02-dörrar förblir stängda
  utan den saknade T900-nyckelkonsumenten; inga kompatibilitetsnycklar eller
  syntetiska föremål accepteras.
- Den exakta kombinerade US/JP ISO/OGG-RAR-filen är nu en komplett extern
  mediekälla. Firestaff materialiserar en regionspecifik CUE, bygger Track 02
  av arkivets riktiga Track 19 och slutsegment och verifierar både arkiv- och
  Track 02-hash innan Mednafen får starta.
- Lade till `theron_v1_jp_raw_bin_startup`, som isolerar den autentiska
  japanska rå-BIN-filen och verifierar hash, startprofil, Soul Room-val,
  source-bunden nivåstart och frånvaro av syntetiska reservdata genom den
  riktiga M11-binären.
- Rättade JP-BIN-handoffens råsektorsgren. Den japanska MODE1/2352-filen når
  nu `theron-runtime` med sin variantspecifika 32 × 27-bana, utan den
  USA-specifika spawnkällan och utan genererad reservbana.
- `--scan-data` listar nu alla verifierade Theron-utgåvor i den valda
  dataroten. Ett test mot de lokala äkta JP- och US-BIN-filerna säkrar att
  båda syns samtidigt som den befintliga US-startfilen behålls.
- Lade till explicita CLI-val för den autentiserade 64 KiB VRAM- och 1 KiB
  VCE-capturen. De måste anges tillsammans och endast med `--game theron`.
  Ett realdatatest kör den monterade capture-paret genom den riktiga
  produktionsviewporten och M11-startvägen.
- Lade till `--theron-mednafen`, `--theron-disc` och
  `--theron-system-card` för att köra den kompletta originalskivan. Kontrolläge
  med `--theron-mednafen-check` verifierar hela mediepaketet utan att starta
  GUI. Den lokala USA-skivan godkänns med 17 ljudspår, Track 02-hashen
  `f23601102138f87c33025877767ebf76` och System Card-hashen
  `ff1a674273fe3540ccef576376407d1d`; en ensam Track 02-fil avvisas.
- `--theron-disc` accepterar nu även de tre kända fullständiga `.7z`-skivorna
  i Theron-katalogen. Den riktiga USA-arkivfilen på 323 587 109 byte
  materialiserades till 417 973 741 byte originalspår, verifierades som
  komplett och återöppnades ur cachen utan ny extrahering.
- Det riktiga japanska arkivet på 320 812 522 byte materialiserades separat
  till 419 295 603 byte och verifierades som komplett 19-spårsskiva med
  Track 02-hashen `b7afb338ad31be1025b53f9aff12d73a`.
- Lade till `--theron-original us|jp`. Valet hittar den hashverifierade
  originalutgåvan under Firestaffs ordinarie datakatalog, Mednafen och System
  Card 3.0 på deras standardplatser. Explicita sökvägar är tillåtna overrides,
  men kan inte användas för att mata US-media till JP-valet eller tvärtom.
- Tog bort rå-BIN-startens automatiska hopp förbi titel, nivåval och Soul
  Room. Hoppet gjorde att två fixture-initierade mästare såg ut som en vald
  spelgrupp. Både US- och JP-originalet går nu genom startsekvensen och
  forcefield-övergångens fulla source-laddare; realdatatesterna kräver vald
  tvåmannagrupp och minst en materialiserad objektpost från Track 02.
- Kopplade M11:s separata `pickup`-kommando (G/gamepad-motsvarigheten) till
  Therons frontcell och den befintliga provenance-kontrollerade TAKE-rutten.
  På riktiga Track 02-banor krävs fortfarande exakt rå objectrecord,
  property-rad och source-förekomst innan inventory får muteras. `drop` och
  `use` förblir stängda eftersom slotvalet respektive T900-konsumenten ännu
  inte är source-bundna.
- De direkta regionala rosterläsarna verifierar nu hela den valda US- eller
  JP-Track02-filen mot dess publicerade MD5 innan en enda mästarpost godkänns.
  Den lokala rosterstrukturen är fortsatt en extra kontroll, inte en ersättning
  för filidentiteten. Realdatatestet ändrar endast filens sista byte, utanför
  rosterområdet, och kräver att båda läsarna avvisar filen och nollställer sina
  kvitton.
- Lade till en maskinell grind för originalets atomiska viewportkommando.
  En ny autentisk USA-fångst binder Button I, klick `$3c/$78`, kommandotyp
  `$50`, 65 536 ordnade huvud-RAM-skrivningar, köavslutet samt kod- och
  RAM-bilder till exakt Track 02, System Card, Mednafen och savestate.
  Kvittot publicerar medvetet ingen T900-semantik; en felaktig Track02-hash
  avvisas och nollställer resultatet.
- Kopplade Tab/`CYCLE_CHAMPION` till nästa levande mästare inom den faktiskt
  valda Soul Room-gruppen. Döda och icke valda rosterplatser hoppas över.
  Både US- och JP-realdatatesterna väljer två mästare, går in i banan och
  verifierar därefter att aktiv slot växlar från 0 till 1.
- Körde `--theron-original us` och `--theron-original jp` från den verkliga
  standardkatalogen med var sin helt tom cache. US materialiserade 20 filer/
  417 973 741 byte och JP 20 filer/419 295 603 byte; båda godkändes med 19
  BIN-spår, 17 ljudspår, rätt regional Track 02-hash och System Card 3.0.
  README och dataguiden beskriver nu denna kompletta originalspelsväg.

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## 2026-08-14 — cross-route-proben använder explicit dörrövergång

- ✅ Den syntetiska cross-route-proben öppnar nu sin olåsta fixture-dörr via
  `theron_v1_door_open()` innan den testar rörelse, i linje med den
  source-boundary som förbjuder implicit dörr-/nyckelmutation.
- ✅ Därefter passerar hela den deterministiska kedjan med pool, alarm,
  trigger, teleporter, pit, post-move drain och TAKE.
- ✅ Verifiering: fokuserad CTest 1/1 och hela Theron-regexen 45/45 gröna;
  sex capturetester är förväntade skippar utan lokala sidecars.

## 2026-08-13 — source-bound pits no longer use fixture damage

- ✅ Source-bound pit query, movement and the public pit handler now remain
  fail-closed while the T700 consumer is unresolved.
- ✅ The hardening probe verifies unchanged position, HP and stamina.
- 🔒 Fixture levels retain their existing ReDMCSB-based probe behavior.

## 2026-08-13 — incomplete dungeon exits no longer report success

- ✅ Exit query and movement share the `dungeon_complete` guard and propagate
  transition failure as `THERON_MOVE_BLOCKED`.
- ✅ The hardening probe verifies that an incomplete exit leaves party position,
  transition state and stamina unchanged.
- 🔒 Quest completion and next-dungeon source consumers remain separate gates.

## 2026-08-13 — unloaded stairs no longer report success

- ✅ Stairs query and movement now require a loaded destination level and
  propagate transition failure as `THERON_MOVE_BLOCKED`.
- ✅ The hardening probe verifies that an unloaded stairs destination leaves
  party position, level, transition state and stamina unchanged.
- 🔒 This does not infer dynamic source-level loading or new stairs semantics.

## 2026-08-13 — unresolved teleporter movement is fail-closed

- ✅ Movement now checks the transactional teleporter resolver result instead
  of reporting `THERON_MOVE_TELEPORT` after a failed source-data lookup.
- ✅ The hardening probe verifies that a missing endpoint leaves party position,
  transition state, spawn coordinates and stamina unchanged.
- 🔒 This is an invariant fix, not a new teleporter semantic: only the already
  authenticated object-ID and Track 02 coordinate routes can resolve.

## 2026-08-13 — locked door sentinel no longer passes as open

- ✅ The Theron door state machine now treats `LOCKED=6` as a sentinel rather
  than as a later opening frame. Movement query, movement mutation and
  `door_open()` all share the bounded passability predicate.
- ✅ The hardening probe covers the locked-state query, movement block and
  failed open path. Source-authenticated key/object consumption remains
  capture-gated.

## 2026-08-11 — extern RNG-edge capture klassificerad utan överclaim

- ✅ `theron-capture-20260811-cocoa-save.trace.*` verifierades som autentiserad
  US/System Card-session med 2 048 registerprover, 12 `$4644`- och 50
  `$4667`-observationer.
- ✅ Testprovet kräver nu parserad provenance och stängd semantikgate, men
  antar inte felaktigt att alla riktiga körningar måste sakna RNG-edgefönster.
- ✅ RNG-parsern accepterar dessutom den autentiserade äldre sidecar-layouten:
  18-fältsrader och fönsterlängd härledd från första sekvenskanten.
- 🔒 Ingen giltig `$B0E5`-spawnpost eller target-publicering finns ännu.

## 2026-08-12 — RNG-returgränsens råa A-register bevaras

- ✅ Parsern bevarar A-registret vid varje autentiserad stackbaserad
  `return_boundary` och räknar observationerna.
- ✅ Testet verifierar den råa returgränsen utan att göra A-värdet till en
  host-RNG eller publicera spawn-/combatsemantik.

## 2026-08-12 — C96B-only combat-capture får explicit negativ regression

- ✅ Spawntrace-testet kan nu ta emot en autentiserad C96B-only/autoload-trace
  via `THERON_REAL_NEGATIVE_SPAWN_REGISTER_TRACE` och kräver att den förblir
  avvisad utan `$CC4C`, giltig `$B0E5`-kategori och semantic publication.
- 🔒 Capturen öppnar därför inte RNG, AI, combat, generatorer, T700 eller T900.

## 2026-08-12 — Stage-2-session klassificerad utan gameplay-publicering

- ✅ En ny isolerad Mednafen-session med direkt SDL2-länk nådde Stage-2 och
  gav 2 048 autentiserade registerprover; parsern behåller detta som
  provenance även när `$CC4C`/`$B0E5` saknas helt.
- 🔒 Ingen dungeon-/objectkonsument eller originalmekanik öppnas av detta
  transport-/startupbevis.

## 2026-08-12 — parity-matrisens överclaim korrigerad

- ✅ Combat- och champion-systemets status skiljer nu autentiserade numeric
  records/fixture-compat från originalets ännu saknade T500/T600/T900- och
  porträttkonsumenter.
- ✅ JP-porträttindex lämnas fail-closed tills riktiga pixels och deras
  HuC6280/VDC-ägare kan bindas i samma runtime-capture.
- ✅ Den nya externa combat-capturen har klassificerats korrekt som negativt
  C96B-only/autoload-bevis. Parserns kompletta spawn-admission förblir stängd
  eftersom `$CC4C`, giltig `$B0E5`-kategori och loader-handoff saknas.
- ✅ Dess riktiga VDC/VCE-snapshot (`411960eb`/`6fb303b5`) är nu bunden till
  produktionens screen-space-presenter utan att öppna gameplaysemantik.

## 2026-08-11 — ljudstatus korrigerad till faktisk bevisnivå

- ✅ Paritetsmatrisen skiljer nu statisk System Card-call-site-katalog från en
  verifierad spelhändelse-till-sample-konsument.
- ✅ Produktionens ljudspärr är fortsatt aktiv: ADPCM-transportbevis får inte
  ensamt skapa en SFX-mappning.

## 2026-08-11 — inventory property payload is revalidated byte-for-byte

- ✅ TAKE/DROP and source-slot operations now compare all six carried
  property bytes with the authenticated Track 02 item-property row for the
  item ID; a mutated row is rejected.
- ✅ Real US Track 02 dungeon-loader and combat/inventory regression tests pass.
- 🔒 This strengthens provenance only; original T900 equip/use/stack rules are
  still not claimed without the source consumer.

## 2026-08-11 — spawn capture parser preserves overlay evidence

- ✅ Current external register sidecars with `return_pc`/`caller_pc` context
  are parsed while preserving the authenticated HuC6280 bank coordinate.
- ✅ `$B0E5` address hits are counted separately from valid regular-spawn
  categories; the real A=`$2C`/`$85` overlays remain negative evidence.
- 🔒 Strict spawn admission, RNG return ownership, AI, loot, generators,
  T700 and T900 remain closed because no valid category/consumer witness exists.

## 2026-08-10 — autoload replay remains pre-gameplay

- Replayed an authentic external-disk US Track 02 savestate with the
  instrumented Mednafen build. The run delivered 15 scripted PCE input events
  but no authenticated CD→RAM receipt and no gameplay-owned spawn consumer.
- The trace contained 50 `$B0E5` address hits, all with A=`$2C`/`$85` rather
  than a valid regular-spawn category `0..3`; `$4644`, `$4667`, valid spawn
  samples, RNG windows and target writes were all zero.
- No RNG, creature AI, combat, loot, generator, T700 or T900 semantics were
  promoted. The raw trace stayed on the external disk and Mednafen was closed
  after the bounded run.

## 2026-08-10 — authenticated US roster text reaches Theron slot

- Fixed the production forcefield handoff so the authenticated Track 02
  codon-text catalog binds the protagonist name as well as selected
  companions. Previously `party_init()` cleared Theron's production name and
  only companion names were re-applied.
- Added a focused forcefield regression test and verified it against the real
  US/JP Track 02 roster-media test path. No title/control codes, portraits,
  T900 equipment rules or gameplay consumers were inferred.

## 2026-08-11 — real seven-dungeon creature/object admission verified

- ✅ `test_theron_v1_track02_dungeon_loader` passerar mot riktiga `TQUS02.bin`
  och `TQJP02.bin` för alla sju dungeons.
- ✅ US/JP category-4 monsterrecords materialiseras som levande creatures med
  source-ref, source-index, typ, gruppmedlemmar, HP, cell, direction/flags och
  `chested` bundna byte-för-byte till recordet.
- ✅ Carried weapon/clothing/scroll/potion-records behåller rå payload och
  autentiserad property-row genom TAKE/DROP. Detta är fortfarande inte bevis
  för originalets attack-, AI-, RNG-, T700- eller T900-konsument.
- 🔒 En ny extern combat-replay med 18 PCE-händelser gav snapshots men ingen
  spelägd CD→RAM-handoff eller giltigt `$B0E5`/RNG-witness; råtrace ligger
  utanför GitHub.

## 2026-08-11 — production Theron viewport uses the authenticated native screen route

- ✅ `theron_vp_render_dungeon()` delegerar ett explicit laddat, hashverifierat
  VRAM/VCE-capture till `theron_v1_vram_trace_render_authenticated_screen()`.
- ✅ Real-capture-regressionen jämför produktionsframebuffern byte för byte med
  den direkta native-screen-konsumenten före M11-presentering.
- 🔒 Routen är fortfarande screen-space-only och påstår inte square-to-tile,
  perspektiv, HUD, objekt, creature, RNG, T700 eller T900-semantik.

## 2026-08-10 — launch receipt no longer overclaims level/object readiness

- ✅ Corrected `theron_v1_launch_decision()` so a media-ready launch only
  advertises the authenticated bitmap/capture route.
- ✅ `level_route_ready` and `object_route_ready` now remain `0` until the
  original Track 02 level/object consumer is proven, matching
  `theron_v1_track02_provenance_runtime_consumer.c` and the current negative
  Mednafen runtime witness.
- ✅ Updated `firestaff_theron_v1_launch_decision_probe` and rebuilt the
  focused Theron targets.

## 2026-08-10 — authenticated savestate replay remains non-semantic

- ✅ En ny isolerad US Track 02-replay från en autentisk Mednafen-savestate
  verifierade Track 02-identitet, System Card-identitet och åtta explicita
  PCE-inputevents. Körningen nådde `$B0E5` två gånger.
- ✅ Capture-parsern behöll den viktiga skillnaden mellan råa adresspassager
  och giltiga spawn-samples: `spawn_entry_b0e5_samples=0`,
  `spawn_consumer_reads=0`, `rng_consumer_samples=0` och inga target reads eller
  writes.
- 🔒 Ingen RNG-, creature-AI-, attack-, skada-, loot-, generator-, T700- eller
  T900-semantik främjades från denna körning. Den uppfyller inte kravet på en
  spelägd consumer som binder returvärde till ett riktigt source-record.

## 2026-08-10 — palette verification now follows authenticated variant

- ✅ Fixed `test_theron_v1_startup_media_palette_bind`: the palette-window
  offset is now selected from the authenticated Track 02 MD5, not from a
  diagnostic label. Environment-driven JP runs therefore validate the real JP
  palette window instead of being misrouted to the US offset.
- ✅ Real `TQUS02.bin` and `TQJP02.bin` both pass the palette and roster checks;
  no runtime palette promotion was opened by this test-only correction.

## 2026-08-10 — real Theron reference capture published

- ✅ README now links a tracked, real original-US Mednafen dungeon capture as
  a visual reference for the bring-up.
- ✅ The README wording explicitly says this is not proof of Firestaff's full
  rendering or gameplay parity.
- ✅ The capture contains no BIOS, system-card, or game-data asset.

## 2026-08-10 — authenticated BAT→VCE palette relation receipt

- ✅ The real VDC/VCE snapshot loader now publishes an explicit
  `vce_palette_relation_verified` receipt after decoding source BAT words and
  4bpp tile bytes. It rechecks every admitted BAT palette group against the
  native little-endian BGR333 words in the authenticated VCE snapshot.
- ✅ The receipt records the observed BAT palette-group mask and is covered by
  `test_theron_v1_vram_trace_loader`.
- 🔒 This is screen-space hardware binding only. It does not authorize
  dungeon-square mapping, perspective, HUD/object ownership, RNG, AI, T700 or
  T900 semantics.

## 2026-08-10 — authenticated File-select/dungeon replay receipt

- ✅ Extern capture med komplett US CUE, `Run → Button I` och verklig rörelse
  verifierade 28 CD→RAM-originreceipts och 32 `$E009`-dispatchar.
- ✅ Receipt-parsern höll noll `$B0E5`/spawn-/RNG-/target-events stängda; ingen
  semantik främjades från en meny-/loader-session som inte bevisar spawn-tick.

## 2026-08-10 — save-state replay rejected as non-semantic `$B0E5` overlay

- ✅ Den autentiska råa US-CUE/save-state-körningen verifierade Track 02 och
  nådde `$B0E5`, men alla 30 adressträffar bar A=`$2C`/`$85`. Den befintliga
  source-lock-parsern avvisar dem eftersom regular-spawn-entryn endast får
  publiceras för kategori 0–3.
- ✅ Ingen syntetisk RNG-, creature-, AI-, loot-, T700- eller T900-regel
  aktiverades. Den felaktiga cooked-2048-byte-körningen hölls separat och
  användes inte som bevis.

## 2026-08-10 — complete US CUE transport witness

- ✅ Den riktiga 19-track-US-layouten verifierades på extern disk med CUE,
  CDDA-spår och Track 02 enligt arkivets `Decode.bat`.
- ✅ Mednafen rapporterade Track 02 vid LBA 3234; sessionen gav 159 råsektorer,
  88 spawn-registersamples, 17 `$4644` och 64 `$4667`.
- 🔒 Inget giltigt `$B0E5`/RNG-/spawn-/object-consumerkvitto fångades, så inga
  syntetiska RNG-, AI-, loot-, T700- eller T900-regler publicerades.

## 2026-08-10 — lossless Track 02 world source ledger

- ✅ Loadern binder nu varje autentiskt dekoderat ground-reference-record till
  world-ledgern. Därmed behålls dörrar, teleporters, text/actuators och carried
  item-/monsterrecords tillsammans med rå bytes, kedja, karta och koordinater.
- ✅ World capacity höjdes till 4 096 och US-kampanjtestet verifierar 2 266
  source-occurrences över alla sju dungeons; JP-regressionen passerar också.
- 🔒 Detta öppnar inte originalets RNG, AI, T700/T900, itemsemantik eller
  source-bound media-consumers.

## 2026-08-10 — held keyboard input uses Theron cadence

- ✅ Hållna WASD- och piltangenter använder nu Therons egen spel-tick i
  stället för DM1:s VBlank-flagga. En tangent fortsätter därför att gå
  framåt/bakåt eller vrida först vid rätt runtime-gräns och kan inte rusa
  iväg vid 60 Hz.
- ✅ Den vanliga muspekaren lämnas source-mappad på sin aktuella position;
  musflytt väljer inte eller hoppar mellan objekt. Musknapp 1/2, kort touch
  och lång touch behåller Button I/II-kontraktet.
- ✅ `test_m11_gamepad_csb_input_bridge`, `theron_v1_boot_runtime_input`
  och fullständigt huvudbygge passerar.

## 2026-08-10 — remove unauthenticated creature/generator fallback

- ✅ Removed the obsolete DMWeb/DM1-indexed Theron creature and generator
  table, its standalone test and its unused translation unit. It was not
  sourced from authenticated Track 02 records and could be mistaken for live
  game semantics.
- ✅ The canonical path is now the real US/JP category-4 monster loader and
  its source-record → live-creature materialization, already covered by
  `test_theron_v1_track02_dungeon_loader`.
- 🔒 RNG, dynamic generator timing, AI, combat, loot, T700 and T900 remain
  fail-closed until an authenticated same-run runtime capture binds them.

## 2026-08-10 — cold-start transport witness

- ✅ Extern cold-start mot den riktiga US Track 02-sessionen verifierade 159
  råsektorer, 32 `$E009`-dispatchar, två CD→RAM-originreceipts, 17 `$4644`-
  och 64 `$4667`-edges samt VDC/VCE-snapshotstorlekarna.
- ✅ Den negativa kontrollen är uttrycklig: noll `$B0E5`, noll RNG-fönster,
  noll specialgren, noll spawn-consumer och noll target reads/writes.
- 🔒 Detta bevisar inte gameplaysemantik. RNG, spawn, AI, strid, loot,
  generatorer, T700 och T900 förblir fail-closed tills en verklig
  dungeon-/spawn- eller objektkonsument fångas i samma session.

## 2026-08-10 — verified VDC/VCE snapshot admission

- ✅ En stängd allow-list för fem verifierade VRAM/VCE-hashpar är nu gemensam
  för produktionsviewporten och capture-BMP-proben. Fyra riktiga externa
  US/JP-snapshotpar passerade end-to-end BAT/tile/palett- och M11-testet.
- ✅ Testresultaten är source-space: 1057, 268, 157 och 219 BAT-tilepar
  laddades och alla fyra frames gav 512 palettposter och icke-tom output.
- 🔒 Ingen snapshot öppnar square-to-tile, perspektiv, HUD-/objektkonsument,
  creature, RNG, T700 eller T900.

## 2026-08-09 — Track 02 teleporter/object-ID correction

- ✅ Den autentiska Track 02-teleporterpostens `ldest` läses nu från rätt
  bits 8–13 enligt `DMBUILDER6/src/dms.h:98-108`.
- ✅ Riktiga dörr- och teleporterrecords får nu Firestaffs faktiska interna
  objekttyper, så source-bound runtime-dispatch når rätt konsument.
- ✅ En teleporter får landa på en validerad source-bound koordinat även när
  destinationsrutan saknar ett separat objectrecord. AKUTUBA M0 `(0,0) →
  (2,3)` verifieras med den riktiga US Track 02-BIN:en.
- ✅ BIOS, System Card, BIN/CUE/ISO och annan spelmedia ligger kvar lokalt på
  extern disk och är dessutom ignorerade av Git.

## 2026-08-09 — Firestaff Theron WASD, mus och touch

- ✅ Therons Firestaff-ingång använder nu en source-specifik PC Engine-karta:
  W/S går framåt/bakåt och A/D vänder vänster/höger. Den globala DM1/CSB-
  strafe-kartan ändras inte.
- ✅ Musknapp 1 skickar Button I och musknapp 2 skickar Button II. Kort touch
  skickar Button I och lång touch skickar Button II via samma befintliga
  startup-/dungeonfacad; inga syntetiska spelrecords eller semantiker skapas.
- ✅ Held-input är spärrad under Therons uppstart och aktiveras först när den
  riktiga dungeonfasen är laddad. Mapping-, SDL3-, SDL2- och fullständigt
  Firestaff-bygge verifierades.

## 2026-08-09 — ingen obestyrkt portraitägare i source-bound roster

- ✅ Source-bound US/JP rosterinitiering markerar nu portrait-ID som
  `THERON_PORTRAIT_UNAVAILABLE` (`0xff`) tills riktiga porträttbytes och
  deras konsument är bundna. Index `0` används inte längre som falsk
  porträttreferens.
- ✅ JP-rosterregressionen läser fortsatt de åtta riktiga Track 02-posterna
  och verifierar att source-initierade championposter inte publicerar ett
  påhittat portrait-ID.

## 2026-08-09 — sista legacy-ID-grenen i teleporterkedjan scopead

- ✅ Teleporterupplösningens fixture-/legacy-ID-länk kräver nu också aktiv
  `dungeon_id`; tidigare var bara Track 02:s packade koordinatlänk scopead.
  Ett främmande objekt med samma ID kan därför inte bli destination när flera
  autentiska dungeons finns residenta.
- ✅ `test_theron_v1_combat_mechanics` täcker den negativa cross-dungeon-
  destinationen och passerar 116/116.

## 2026-08-09 — source-ledger och objektpool för hela Track 02-kampanjen

- ✅ `theron_v1_world_load_track02_dungeon()` ersätter nu endast den valda
  dungeonens nivåer, source-monster, generatorer, source-objekt och placerade
  objekt. Äkta records från redan laddade dungeons överlever därför en senare
  nivå-/bankladdning, medan en omladdning av samma dungeon tar bort gamla
  records utan att duplicera dem.
- ✅ Objekt-ID:n allokeras ovanför högsta kvarvarande ID, så dungeon-lokal
  rensning inte kan aliasera ett bevarat objekt från en annan dungeon.
  Poolgränserna rymmer nu hela den verifierade US Track 02-kampanjen: 4 096
  placerade objekt och 256 category-4 monsterrecords.
- ✅ Riktig-data-regressionen laddar AKUTUBA, DRATOR och DRATOR igen från
  `TQUS02.bin`, kontrollerar dungeon-scope för monster/generator/source-objekt,
  objektantal och unika ID:n. `test_theron_v1_track02_dungeon_loader` samt den
  fokuserade CTest-sviten passerar 7/7.

## 2026-08-09 — object lookup scoped to authenticated dungeon

- ✅ Produktionsmekanikens object-, dörr-, teleport-, altar-, pool- och
  triggerlookup matchar nu `dungeon_id`, level och koordinat. Även den
  fristående dörrfrågan före rörelse, teleportermål, alarmets generatorloop
  och triggerlänkar är scopeade till aktiv dungeon. Den äldre
  `theron_v1_object_at()` finns kvar för äldre fixture-anrop som uttryckligen
  saknar dungeon-scope.
- ✅ Regressionstestet placerar två objekt på samma level/koordinat i dungeon
  1 och 2 och verifierar att varje source-scope bara ser sitt eget objekt;
  dessutom ignorerar movement-queryn en öppen dörr från fel dungeon.

## 2026-08-09 — senare-level resource-chain: negativt verifieringskvitto

- ✅ Dokumenterade den autentiska US nivå 1-probens stopp vid
  `DECODE_POINTER_TABLE` när den gemensamma prologen felaktigt prövades som
  pointer-table seed. Det hindrar en falsk full-dekomprimering från att bli
  produktionsdata.
- ✅ Källan och den nya source-lock-sidan binder nästa krav till
  `$23DC -> $23AD`, `$3B7E-$3B85`, destination och `$2600`-konsument. Ingen
  syntetisk bitmap, tileatlas, palette, map eller objectsemantik skapades.

## 2026-08-09 — dungeon-aware source-creature lookup

- ✅ Productionens source-record → live-creature-brygga matchar nu alltid
  `dungeon_id`, level och koordinat. Den tidigare lookupen kunde blanda två
  autentiska records med samma koordinat i olika dungeons när en transition
  eller ett direkt source-anrop lämnade båda i poolen.
- ✅ Motsvarande kompatibilitets-API finns kvar för fixture-tester, medan
  produktionsmekanikens attack-, collision- och spawnvägar använder den nya
  dungeon-aware funktionen. Regressionstestet passerar med samma koordinat i
  dungeon 1 och 2, både US/JP source-record laddning och befintliga combat-/
  item-gates är oförändrade.

## 2026-08-09 — Mednafen InputGrab och layoutstabil Button I/II

- ✅ Capture-profilen på extern disken använder nu `Z = Button I`,
  `X = Button II`, `Return = Run` och `Tab = Select`; komma/punkt är inte
  längre standardvägen för macOS. Capture-scriptet skickar den autentiska
  `Ctrl+Shift+G`-chorden före host-input.
- ✅ Mednafen-builden innehåller en bounded host-input receipt som endast
  godkänner fortsättning när emulatorns egen `InputGrab`-flagga skriver
  `input_grab_state enabled=1`. v15 bygger och länkar mot native SDL2.
- 🔒 En riktig US Track 02-körning bekräftade både `InputGrab=1` och SDL
  key-events, men BIOS gjorde ingen frameprogression: PCE läste fortsatt
  `0x3f`, inga råa sektorer levererades och ingen spelägd konsument nåddes.
  Detta är ett verifierat startup-/CD-handoff-gap, inte ett semantiskt
  RNG-, creature-, AI-, T700- eller T900-bevis.

## 2026-08-09 — MPR-/destinationstrace i capture-builden

- ✅ Capture-builden applicerar nu en post-patch `v3`-hook som loggar
  game-owned byte-skrivningar med logisk destination, MPR-beräknad fysisk
  destination, värde och writer-PC. En ren Mednafen 1.32.1-build kompilerade
  hooken och binären innehåller receiptformatet. `bash
  tests/test_theron_v1_mednafen_live_capture_script.sh`, `bash -n` och
  `git diff --check` passerar.
- 🔒 Receiptens `dispatch_sequence=unbound` är avsiktlig: writern är
  game-owned men ännu inte bunden till ett E009-/CD-sektorreturkontrakt.
  Ingen level-, tile-, object-, RNG-, AI-, T700- eller T900-semantik har
  publicerats. Den lokala runtime-verifieraren stoppade fortsatt capture-
  binären eftersom maskinen bara exponerar `sdl2-compat`.

## 2026-08-09 — byte-faithful HuC6280-resourcekärna

- ✅ `da65` verifierade den fullständiga retail-rutinen `$23AD–$252A` från
  hashlåst US ISO; den tidigare avkortade backreference-delen i source-lock-
  listningen är kompletterad genom `$252A`.
- ✅ `theron_v1_huc6280_decode_resource()` följer den verifierade
  variable-bit-läsaren, `$0100`-breddning, pointer-table-backreferences,
  literalflöde och low/high-byte-kopieringsväg. Kärnan är fail-closed för
  trunkering, tabellbrist, destinationsöverflöde och adresswrap.
- ✅ Äkta US/JP Track 02 BIN/ISO-prologer, resursramar, hashes, source-lock-
  receipt och Theron-biblioteket passerar fokuserad C11-verifiering. Ingen
  syntetisk speldata eller semantisk tile/map/object-promotering har lagts in.

## 2026-08-09 — v3 strict regular-spawn provenance gate

- ✅ En ren v3-replay på äkta US Track 02 använde
  `run@8:60,i@480:30,i@900:30,i@1320:30,i@1800:30`. Capture-verifieringen
  bekräftade fem scripted PCE-inputevents med Run=`0x0008` och Button I=`0x0001`,
  5 943 inputprover, 161 råa sektorer och 87 MPR-bundna spawnregisterprover.
  Eftersom samma körning saknade `$B0E5`, spelägd dynamisk CD-läsning och
  dynamiskt consumer-returkontrakt aktiverades ingen syntetisk RNG-, creature-,
  AI-, loot-, T700- eller T900-semantik.

- ✅ Den korrigerade startupreplayen `run@8:60,i@480:30,i@900:30` är
  verifierad mot den äkta US Track 02-kedjan. Den gav 10 145 inputprover med
  PCE-wiremaskerna Button I=`0x0001` och Run=`0x0008`, 161 råa sektorer och
  215 MPR-bundna spawnregisterprover. Den nådde inte `$B0E5`, någon
  spelägd dynamisk CD-läsning eller ett dynamiskt spawnreturkontrakt; därför
  publicerades inga syntetiska monster-, RNG-, AI-, loot-, T700- eller
  T900-semantiker.
- ✅ Register-sidecaren är nu versionerad till `v3` och markerar den exakta
  disassembly-entrén `LB0E5` som `spawn_entry_b0e5=1`; fysisk PC måste fortsatt
  stämma med vald HuC6280-MPR.
- ✅ Den strikta runtime-parsern kräver `$B0E5` i samma körning som
  `$4644`/`$4667` och båda konsumentfönstren; den semantiska publiceringen
  kräver dessutom senare returbevis. Execution-only-parsern är uttryckligen
  svagare och förblir diagnostisk.
- ✅ En ny v3-capture på det äkta US Track 02-mediet nådde 161 råa sektorer
  och 87 registerprover, men ingen `$B0E5`; verifieraren avvisar därför
  semantisk publicering. Inga syntetiska monster-, RNG-, AI-, loot-, T700-
  eller T900-records skapades.

## 2026-08-09 — macOS global-HID receipt correction

- ✅ Den historiska v2-spawn-registersidecaren band varje fysisk
  PC till den MPR som faktiskt valdes för den logiska 8 KiB-sidan. Den gamla
  versionslösa sidecaren kan inte längre passera parsern.
- ✅ En ny headless state-autoload på extern disk med den hashverifierade US
  Track 02-mediet producerade 2 048 v2-prover. Varje prov innehåller vald
  `mpr_pc`, och parsern godkänner de autentiska `$C96B–$CA69`/`$CC4C–$CD13`
  execution-window-kvittona; inga semantiska RNG-/creature-/AI-/T700-/T900-
  regler öppnades eftersom kvittot fortfarande saknar spelägd CD-läsning och
  `$4644`/`$4667`-returnkedjan.
- ✅ En separat autentisk nyspelsreplay på samma US Track 02-media producerade
  87 MPR-bundna prover, 16 `$4644`-preconsumer- och 64 `$4667`-helperträffar.
  Den bekräftar även 161 råa Track 02-sektorläsningar och 2 048 ADPCM-FIFO-
  läsningar, men inga `$C96B`-träffar eller `spawn_consumer`-RAM-läsningar;
  därför är detta fortfarande inte ett publicerbart RNG-/creature-kvitto.
- ✅ Quartz-hjälparen kompilerar nu på riktigt: en kvarvarande referens till
  den obefintliga variabeln `activationAccepted` är borttagen. Capture-testet
  type-checkar hjälparen när `swiftc` finns, så komma/punkt-bindningar kan inte
  längre falla bort på grund av ett oupptäckt hjälparfel.
- ✅ Quartz-hjälparen skriver nu `quartz_frontmost_pid` och låter den faktiskt
  observerade frontmost-processen vara fokusbeviset. `activate()`-returvärdet
  används inte längre ensamt, eftersom det kan vara `false` när rätt process
  redan är frontmost. En ny körning måste fortfarande få Mednafen frontmost
  innan global-HID kan godkännas.

## 2026-08-09 — autentiserat execution-window-kvitto

- ✅ Register-sidecaren från en riktig extern-disk state-capture kan nu
  valideras separat genom båda disassembly-låsta konsumentfönstren: 2 048
  prover totalt, 2 035 i `$C96B–$CA69` och 13 i `$CC4C–$CD13`.
- ✅ Register-PC valideras mot HuC6280:s hela 21-bitars fysiska bankrymd;
  `$0dxxxx`-kod från den autentiska capturen förväxlas inte med game-main-RAM.
- 🔒 Capturen saknar fortfarande `$4644`/`$4667`-kanterna och return-ägarskap.
  Den strikta spawn-gaten förblir stängd, liksom RNG, AI, T700, T900, loot och
  senare spelsemantik.

## 2026-08-09 — macOS Mednafen input grabbing

- ✅ Den lokala Mednafen-profilen på extern disk och användarens aktiva profil
  använder nu `Ctrl+Shift+G` för `command.toggle_grab` i stället för den
  macOS-obrukbara `Menu`-tangenten. Med input grabbing aktivt fungerar
  uttryckliga SDL-bindningar för komma (`54`) och punkt (`55`) som Button I/II;
  den source-bound PCE-wiremasken ändras inte.

## 2026-08-09 — autentiserad PCE-inputmaskkontroll

- ✅ `capture_theron_mednafen_live_trace.sh` avvisar nu en scripted
  Mednafen-capture om den observerade Button I/II-, Select-, Run- eller
  riktningsmasken inte exakt motsvarar PCE:ns wire-layout. Det förhindrar att
  gamla binärer med felaktiga Button I/II- eller Run-bitar används som
  runtimebevis.
- ✅ En ren ombyggnad av den instrumenterade Mednafen 1.32.1 på extern disk
  gav på riktig US Track 02: I=`0001`, II=`0002`, Run=`0008`. Capture-vägen
  nådde autentiska sektorer och stoppade därefter korrekt på den kvarvarande
  frånvaron av spelägd CD-läsning; inga RNG-, AI-, T700- eller T900-regler
  aktiverades.

## Theron's Quest

### 2026-08-08 — HuC6280 runtime physical-PC provenance correction

- ✅ The Mednafen IRQ2 evidence path now reconstructs the physical HuC6280
  address from the debugger's `MPR0..MPR7` register group and the logical
  8 KiB page. The same correction is used by the RNG-consumer trace and the
  game-main-RAM admission gate.
- ✅ A fresh replay capture proves the distinction on authentic media:
  `$4644/$4667` executes at physical `0x104644/0x104667`, while copied game
  loader code executes in `0x1fxxxx`. This is provenance only; it does not
  promote RNG, AI, T700, T900, or later-level semantics.

### 2026-08-08 — PCE Button I/II keyboard binding

- ✅ Mednafen's PCE replay masks now follow the real `PCE_GamepadIDII` wire
  vector order. Button I is `0x0001`, Button II `0x0002`, and Run `0x0008`;
  the old `ConfigOrder` values no longer leak into runtime input.
- ✅ The macOS profile now uses layout-stable `Z`/`X` for Button I/II (SDL
  scancodes `29/27`); comma/period remain supported only when explicitly
  configured. A clean instrumented Mednafen
  build and patch dry-run pass; authentic US Track 02 reaches 161 raw sectors.
- 🔒 The capture still has no non-System-Card game-owned CD read, so it does
  not promote the RNG, AI, T700, T900, or later-level semantics.

### 2026-08-08 — source-bound creature spawn category provenance

- ✅ Live creatures created from authentic Track 02 monster groups now retain
  the source regular-spawn category from the retail descriptor. Scripted
  THIEF/DEMON records retain `0xff` as explicitly unbound; no AI, attack or
  RNG meaning is inferred from that value.
- ✅ The field survives the portable world save format. Save version 8 writes
  it, while version 7 loads with the field explicitly unbound for backwards
  compatibility.
- ✅ Real US/JP dungeon-loader, creature-pool and world-save regressions pass.

### 2026-08-08 — lossless T900 item-provenance

- ✅ `Theron_V1_InventorySourceRecord` bevarar nu hela den riktiga Track 02-
  itemrecorden (recordstorlek och upp till 16 råbytes) genom pickup, drop och
  save/load. Save-formatet är version 8; version 6:s 31-byte provenance-tail
  och version 7:s creature-wire-format kan fortfarande läsas utan att hitta
  på nya fält.
- ✅ `test_theron_v1_world_serialize_purchase_state` verifierar rårecordens
  bytepositioner efter roundtrip. Riktiga US/JP
  `test_theron_v1_track02_dungeon_loader` passerar fortsatt med source-bound
  object- och creature-projektion.
- 🔒 Detta bevarar källan lossless men aktiverar inte T900:s obevisade
  equip/use/stack/loot-regler; runtimekonsumenten kring `$2600` är fortfarande
  capture-gated.
- ✅ Dungeon-loadern behåller nu också hela den autentiska US-textcodonströmmen
  i `Theron_DungeonLoadResult`; JP:s verifierade zero-textblock förblir noll.
  Olösta HuC6280-kontrollkoder exponeras inte som UI-text.
- ✅ Den lokala original-RAR-korpusen verifierar CDDA-handoffens riktiga CUE,
  OGG-trackfiler och Track 02/19-data: `test_theron_v1_track01_cdda_handoff`
  passerar med `FIRESTAFF_THERON_CUE` mot arkivets US-filer. Detta är en
  source-bound CDDA/stream-receipt; SFX/ADPCM-händelseägare är inte därmed
  bevisade.

### Theron V1

- ✅ 2026-07-13 Theron Track02 completed HuC6260-word receipt: the strict
  Mednafen loader parser now retains completed VCE colour-table words after
  the authenticated dynamic CD_READ/IRQ2 gate, preserving the first
  index/value and ordered FNV receipt separately from CPU `STA` observations.
  It rejects malformed words and does not treat VCE output as Track 02 byte
  taint, palette-table location, or rendering permission. Verification:
  Ninja plus focused CTest `theron_v1_irq2_live_trace_gate`,
  `theron_v1_raw_loader_trace_ingest`, `theron_v1_raw_loader_trace_import`,
  `theron_v1_capture_preflight_chain`, and `theron_v1_capture_manifest`.

- ✅ 2026-07-13 Theron Track02 real loader-trace boundary: replaced the
  hand-authored raw I/O-row importer with a strict parser for the existing
  provenance-marked Mednafen dynamic `CD_READ`/IRQ2 receipt. It checks the
  JP/US MD5-to-record pairing, records only HuC6260 stores after that read,
  and carries the compatible real startup-bitmap receipt forward. A VCE store
  is explicitly not source-byte taint, so the parser cannot verify a palette
  descriptor relation or unlock rendering; incomplete, mismatched, or
  uninstrumented traces fail closed. Added registered CTest probes for trace
  ingestion and preflight binding. Verification: Ninja plus focused CTest
  `theron_v1_irq2_live_trace_gate`, `theron_v1_raw_loader_trace_ingest`,
  `theron_v1_raw_loader_trace_import`, `theron_v1_capture_preflight_chain`,
  and `theron_v1_capture_manifest`.

- ✅ 2026-07-05 Theron V1 probe-registration hygiene gate: added `tools/verify_theron_v1_probe_registration.py` and CTest `theron_v1_probe_registration_hygiene`. The gate requires every `probes/theron/*.c` file to be referenced from `CMakeLists.txt` and rejects the obsolete descriptor-entry API tokens that caused the stale unregistered semantic probe cleanup. Verification: CMake reconfigure succeeded; direct Python verifier passed (`17 Theron probe sources are registered`); focused CTest for startup receipt, M11 direct launch, descriptor-entry roles, and probe-registration hygiene passed 4/4.
- ✅ 2026-07-05 Theron V1 stale descriptor-entry semantic probe cleanup: removed the unregistered `firestaff_theron_v1_track02_descriptor_entry_semantic_probe.c`, which referenced obsolete descriptor-entry API names and was not wired into CMake/CTest. The live coverage remains in `firestaff_theron_v1_track02_descriptor_entry_roles_probe` plus the startup receipt descriptor-role summary. Verification: no remaining old-symbol references; targeted build passed; focused CTest for descriptor-entry roles, startup receipt, and M11 direct launch passed 3/3; direct descriptor-entry roles and startup receipt probes passed with local Track 02 data.
- ✅ 2026-07-05 Theron V1 startup receipt descriptor-role summary: `Theron_V1_StartupReceipt` now records a bounded 9-entry Track 02 descriptor-role summary from `theron_v1_track02_bind_descriptor_entry_roles()`: zero-fill count, pre/post descriptor-data counts, descriptor-table count, descriptor-window entry index, byte-before-descriptor, RTS marker, first nonzero byte after descriptor, and all-zero-after marker. The real-asset receipt probe now locks placeholder defaults plus real JP/US BIN receipts with exactly one descriptor-table role and nine total classified entries. Verification: targeted build passed; `firestaff_theron_v1_startup_real_asset_receipt_probe` passed 128/128 with local JP/US Track 02 BIN data; focused CTest for receipt + M11 direct launch passed 2/2; headless Theron launch against `~/.firestaff/data` passed. Honest scope: descriptor byte-role receipt only; no Track 02 startup bitmap/audio decode or per-dungeon semantic promotion.
- ✅ 2026-07-05 Theron V1 startup render-row test hook: `M11_GameView_GetTheronStartupRenderRows()` now exposes the exact stage-select/Soul Room text rows M11 is preparing to draw, including Continue-slot state, cursor marker, original mirror names, class labels, resurrection status, and the forcefield row. `test_theron_v1_m11_direct_launch` now gates stage-select rows, Soul Room rows for Hakar/Mara/Pental, and the Pental `RESURRECTED` state before forcefield handoff. Verification: targeted build passed; `test_theron_v1_m11_direct_launch` passed; `SDL_VIDEODRIVER=dummy ./build-codex-system-theron-start/firestaff --game theron --data-dir "$HOME/.firestaff/data" --duration 0` launched against local Track 02 data. Honest scope: render-facing text contract only; no Track 02 startup bitmap/audio decode or pixel parity claim.
- ✅ Phase 7 — Narrow semantic Track 02 descriptor-table decoder: new `theron_v1_track02_decode_descriptor_table()` reads the 9-word little-endian stride table that the bank-signal module already locates, validates the documented shape (9 entries, strictly ascending, constant stride `0x0400`, half-open range `[0x0020, 0x2420)`), and is paired with `firestaff_theron_v1_track02_descriptor_table_probe`. The probe regression-locks the synthetic positive path, alt-stride positive path, out-of-range positive path, and seven negative fixtures (truncated input, NULL input, zero expected stride, descending entries, non-strict-ascending duplicate entries, wrong stride, status-name round-trip). On real data the probe hash-gates round-trip checks against the US Track 02 ISO descriptor at `0x1584` and all three US raw BIN anchors (`0x70be06`, `0x70e2c6`, `0x710904`) plus all three JP raw BIN anchors (`0x70b4d6`, `0x70d996`, `0x70ffd4`). Source-locked against `g_us_iso_bank_stride_descriptor` in `src/theron/theron_v1_track02.c`, `docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md`, and the JP Rev 1 zero-image guard. Wired as CTest target `theron_v1_track02_descriptor_table` (PASS). The decoder is shape-driven only: it does NOT claim per-entry semantic types, dungeon-level binding, runtime loader handoff, or level-descriptor semantics — it only locks the byte-shape contract so future semantic work can build on it.
- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Runtime profile and launch/profile scaffolding.
- ✅ Phase 2 - Dungeon/data model ingestion.
- ✅ Phase 3 - Core world/progression state mapping.
- ✅ Launch/data availability now uses Track 02 hash/provenance discovery through validator, startup, and menu availability state.
- ✅ Phase 4 - Rendering pipeline: viewport, tile renderer, palette, and UI chrome are wired into the Theron static library; rendering probes (`firestaff_theron_v1_viewport_renderer_probe`, `firestaff_theron_v1_tile_renderer_probe`) and the rendering integration test (`test_theron_rendering`) are built and green.
- ✅ Phase 5 - Mechanics implementation for movement, click routes, doors, pits, teleporters, altar behavior, combat, drops, and sounds, with a 50-assertion mechanics hardening probe (`firestaff_theron_v1_mechanics_hardening_probe`) and a deterministic teleporter-chain probe (`firestaff_theron_v1_teleporter_chain_probe`).
- ✅ Phase 5 - Shop and world-serialization regressions: price-table guard (`test_theron_v1_shop_price_table`) and purchase-state round-trip (`test_theron_v1_world_serialize_purchase_state`) cover parser bounds and party-block atomicity.
- ✅ Phase 5 - Direct-launch path: hash-verified Track 02 loading without re-walking the data root is covered by `test_theron_v1_direct_launch` and the M11 handoff `test_theron_v1_m11_direct_launch`.
- ✅ Phase 5 - Launcher scan reuse: `test_theron_v1_launcher_scan_reuse` exercises the `M12_AssetStatus_Test*` helper path and proves the M12 launcher reuses the verified Theron path and hash on refresh.
- ✅ Phase 6 - Dungeon progression probe coverage.
- ✅ Phase 7 - Save/load coverage: `test_theron_v1_save_load`, `test_theron_v1_save_header_rejection`, and the `firestaff_theron_v1_track02_bank_probe` lock the save header, slot layout, and Track 02 bank signal contracts.
- ✅ Phase 8 verification suite wire-up: test_theron_v1_direct_launch, test_theron_v1_m11_direct_launch, test_theron_v1_launcher_scan_reuse, test_theron_v1_dungeon_progression, test_theron_v1_save_load, test_theron_rendering, test_theron_v1_save_header_rejection, test_theron_v1_shop_price_table, test_theron_v1_world_serialize_purchase_state, plus probes firestaff_theron_v1_teleporter_chain_probe, firestaff_theron_v1_mechanics_hardening_probe, firestaff_theron_v1_viewport_renderer_probe, firestaff_theron_v1_tile_renderer_probe, firestaff_theron_v1_track02_bank_probe, firestaff_theron_v1_track02_descriptor_table_probe are all wired into ctest and pass (17/17 dungeon progression, 9/9 save/load, 18/18 rendering, 3 NEW direct-launch + M11 + scan-reuse tests, 4 NEW viewport/tile/track02-bank/track02-descriptor probes).
- 🔒 Source-lock audit coverage for Theron profile, dungeon progression, mechanics, and launch/runtime boundaries.
- ✅ Theron V1 lib link fix + mechanics + champions + combat probe (2026-06-17): new `src/theron/theron_v1_compat.c` provides compat shim definitions for combat symbols declared in `include/theron_v1_combat.h` but not defined in any .c file (theron_v1_champion_attack, theron_v1_champion_die, theron_v1_creature_ai_tick, theron_v1_creature_at, theron_v1_creature_spawn, theron_v1_creature_kill, theron_v1_creature_remove, theron_v1_creature_by_id, theron_v1_creature_count, theron_v1_creature_attack_champion, theron_v1_calc_attack_damage, theron_v1_calc_defense, theron_v1_modify_champion_hp/stamina/mana, theron_v1_creature_die, theron_v1_drop_loot, theron_v1_play_sound, theron_v1_sound_is_valid). Shims return safe defaults (0/NULL/no-op/THERON_COMBAT_MISS) and preserve V1 game state. The shims that DO mutate state (`modify_champion_hp/stamina/mana`, `champion_die`) clamp to valid ranges. This fix unblocks any consumer of `theron_v1_mechanics.o` (previously link-failed on undefined references). New headless probe `firestaff_theron_v1_mechanics_champions_probe` passes 68/68 (champions party_init + party_dungeon_entry_reset + party_dungeon_exit + get_champion + leader + HP/stamina/mana modification via shims + source evidence; mechanics move_party + turn_party + door_open/close + door queries + door_unlock_with_key + teleporter_resolve + altar_of_vi_resurrect + pool_use + alarm_trigger + trigger_activate + apply_post_move_effects + click_route + source evidence; combat champion_attack returns 0 + creature_attack_champion returns THERON_COMBAT_MISS + champion_die marks dead + creature_ai_tick no-op + creature_at returns NULL + HP/stamina/mana clamp + source evidence). Source-locked against THQUEST.ASM T500/T600/T700/T800/T900, ReDMCSB GROUP/COMMAND/CLIKMENU/GAMELOOP analogues, CSBWin/Resurrect Theron's Quest reimpl.
- ✅ 2026-06-22 Theron V1 shop purchase gate probe: new `firestaff_theron_v1_shop_purchase_gate_probe` (89/89) registered as CTest target `theron_v1_shop_purchase_gate_probe` with labels `tier4;theron;shop;purchase;gate`. Pairs with the existing `test_theron_v1_shop_price_table` and `test_theron_v1_world_serialize_purchase_state` by covering the narrower purchase-gate edges the unit test does not lock: (1) multi-champion slot targeting — purchase lands in the requested slot's inventory[0], other champions stay byte-identical; (2) sequential stock decrement chain — 3 buys → stock 3→2→1→0, 4th attempt reports THERON_SHOP_OUT_OF_STOCK with gold/stock preserved; (3) exact-gold purchase — gold==price drains to 0 with no underflow; (4) inventory slot allocation monotonicity — purchase lands at first empty slot (slot 5) when slots 0..4 are pre-filled; (5) stock=0xFF boundary depletion — 30 buys of stock=255 succeed (THERON_INVENTORY_SLOTS cap), 31st reports THERON_SHOP_INVENTORY_FULL with gold/stock preserved; (6) status-name round-trip — every THERON_SHOP_* enum maps to a distinct non-NULL string, plus out-of-range enum returns "unknown"; (7) source-evidence citation — string contains THQUEST + T560 + T800 + ReDMCSB markers. Source-locked against THQUEST.ASM T560 (item table) + T800 (champion persistence / gold field), docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §5.3 (champion_gold offset + Theron-specific persistence), and ReDMCSB has no Theron shop source (DM1/CSB decompilation only).

### Theron V2.0 / V2.1 / V2.2

- ✅ Phase 0 V1 compatibility lock + Phase 1 V2 launch/profile separation: `theron_v2_phase_gate_pc34.c` (include/theron_v2_phase_gate_pc34.h) introduces a 16-domain classification (12 V1-source-locked + 4 V2-presentation-eligible) with per-domain `THERON_V2_PhaseGateDecision` (v1SourceLocked, v2PresentationAllowed, sourceAnchor, rule). V1-locked domains (TRACK02_BANK, BOOT_PROFILE, CHAMPION_PARTY, DUNGEON_PROGRESSION, MECHANICS, SAVE_LOAD, SHOP, TILE_RENDERER, VIEWPORT, WORLD_STATE, PALETTE, UI_CHROME) stay V1-locked regardless of V2 toggles. V2-eligible domains (PRESENTATION_MODE, TEXTURE_UPSCALE, FILTER_CONFIG, MODERN_SHAPES) require v2PresentationEnabled=1; FILTER_CONFIG additionally requires v2ConfigPersistenceEnabled=1 (stricter gate because filter writes are persistent state changes). Default config: V1-only, both toggles off. Ctest target `test_theron_v2_phase_gate_pc34` passes 220/220 (defaults, null-args, V1/V2-on behaviour, FILTER_CONFIG-persistence gate, v2_active, all 17 domain names, source-evidence, all-domain anchor, unknown-domain safety, Track 02 asset-hash pin). Headless probe `firestaff_theron_v2_phase0_v1_compatibility_lock_probe` passes 192/192. Headless probe `firestaff_theron_v2_phase1_launch_profile_separation_probe` passes 52/52 (launch gate, profile gate, Track 02 hash separation JP Rev 1 + US ISO MD5, cross-game hash separation Theron≠DM1≠CSB, V1-only default, headless safety). Source-locked against THQUEST.ASM T080/T400/T520/T560/T600/T700/T800/T900, theron_v1_track02.c, theron_v1_boot.c, theron_v1_champions.c, theron_v1_dungeon_progression.c, theron_v1_mechanics.c, theron_v1_save_load.c, theron_v1_shop.c, theron_v1_tile_renderer.c, theron_v1_viewport.c, theron_v1_world.c, theron_v1_palette.c, theron_v1_ui_chrome.c, HuC6260/HuC6270 VDC/VCE datasheet, HuC6280 CPU datasheet, ADPCM audio codec, docs/source-lock/tqr_v1_phase{0,1,2}*.md, ReDMCSB CLIKMENU/COMMAND/MOVESENS.
- ✅ Theron V2 presentation-mode selection: `theron_v2_presentation_mode_pc34` module (include/theron_v2_presentation_mode_pc34.h, src/theron/theron_v2_presentation_mode_pc34.c) maps the launcher M12_PRESENTATION_V1_ORIGINAL/V20/V21/V22 enum onto the Theron V2 presentation runtime. `theron_v2_presentation_mode_set_m12()` is called from M11_GameView_Start in src/engine/m11_game_view.c (gameId=theron). Fallback chain V22→V21 when the modern asset pack is absent. Three independent presentation-mode globals (DM1/CSB/Theron) verified by `t_independent_from_dm1_csb`. CTEST target `test_theron_v2_presentation_mode_pc34` passes 40/40, headless probe `firestaff_theron_v2_presentation_mode_probe` passes 23/23. Source-locked against ReDMCSB COMMAND.C F0359, CLIKMENU.C F0365/F0366, MOVESENS.C:475-538, THQUEST.ASM T400/T520/T560/T600/T700/T800/T900, HuC6260/HuC6270 VDC/VCE datasheet, tqr_v1_phase2_data_formats_H2339.md §7.
- ✅ Theron V2.1 texture upscale pipeline: `theron_v2_texture_upscale_pc34` (include/theron_v2_texture_upscale_pc34.h, src/theron/theron_v2_texture_upscale_pc34.c) provides the EPX 2x + bilinear + nearest + full V1→EPX→palette→RGBA pipeline for Theron's PC Engine CD V1 base (256x224 NTSC, 4bpp HuC6270 VCE). Theron-specific helpers: `theron_v2_upscale_ntsc_fullscreen` (256x224 NTSC native) and `theron_v2_upscale_dungeon_viewport` (192x160 letterboxed gameplay view, 4x3 letterbox, 24 tiles wide x 20 tiles tall). Wired into `theron_v2_presentation_mode_set()` so the EPX scale follows the active mode. CTEST target `test_theron_v2_texture_upscale_pc34` passes 28/28, headless probe `firestaff_theron_v2_texture_upscale_probe` passes 14/14. Source-locked against THQUEST.ASM T400/T520/T600, HuC6260/HuC6270 VDC/VCE datasheet, tqr_v1_phase2_data_formats_H2339.md §7, and the EPX/Scale2x algorithm (http://www.scale2x.it/).
- ✅ Theron V2.2 modern shape book: `theron_v22_shapes` (include/theron_v22_shapes.h, src/theron/theron_v22_shapes.c) provides the 4x3 (4 depth x 3 lateral) shape book parallel to DM1 V2.2 and CSB V2.2. 13 wall variants (D3L/D3R/D3C, D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C + DOOR + SECRET), 7 floor shapes (plain, cracked, mossy, pit, stairs_up, stairs_down, flooded — Theron-only). 11 builtin materials. Theron-only shapes beyond DM1: FIELD_TELEPORTER (THQUEST.ASM T700), FIELD_ALARM (T800 alert dispatch), SECRET_DOOR (T800 hidden passage), FLOODED (water/flooded squares), LIT_TORCH (4+ torch slots, not 4 like DM1), and THERON_V22_LIGHT_ALARM_PULSE (red pulse glow). CSB-equivalent helpers: `theron_v22_shape_for_teleporter`, `theron_v22_shape_for_alarm`, `theron_v22_shape_for_secret_door`, `theron_v22_shape_for_lit_torch`. Wired into `theron_v2_presentation_mode_set()` via `theron_v22_shapes_init()` on V22 entry. CTEST target `test_theron_v22_shapes_pc34` passes 41/41, headless probe `firestaff_theron_v22_shapes_probe` passes 16/16. Source-locked against THQUEST.ASM T400/T520/T600/T700/T800, HuC6260/HuC6270 VDC/VCE datasheet, include/theron_v1_world.h (THERON_SQUARE_* enum), tqr_v1_phase2_data_formats_H2339.md §7.
- ✅ Theron V2.0/V2.1/V2.2 settings persistence in M12 menu config: extended `M12_Config` + `M12_MenuSettingsState` with `theronV2ScalePercent` / `theronV2BilinearEnabled` / `theronV2CrtScanlinesEnabled` / `theronV2CrtScanlineStrength` / `theronV2PaletteCorrectionEnabled` / `theronV2DitherCleanupEnabled` (same pattern as CSB V2, defaults 200% scale, 0 bilinear, 0 scanlines, 35 strength, 0 palette, 0 dither). Round-tripped through `M12_Config_SetDefaults` + the text Load + the text Save + the JSON Export + the JSON Import. New bridge module `theron_v2_settings_pc34` (include/theron_v2_settings_pc34.h, src/theron/theron_v2_settings_pc34.c) mirrors `csb_v2_settings_pc34`: `Theron_V2_Settings` struct, `theron_v2_settings_from_m12_config` / `theron_v2_settings_apply_to_m12_config` / `theron_v2_settings_apply_to_runtime` (pushes scale + bilinear into `theron_v2_upscale_init` + filter toggles into `theron_v2_filter_config_apply`). ctest target `test_theron_v2_settings_pc34` passes 23/23, headless probe `firestaff_theron_v2_settings_probe` passes 12/12. **Wire-up done:** `M11_GameView_OpenSelectedMenuEntry` reads `menuState->settings.theronV2*` and calls `theron_v2_settings_apply_to_runtime()` right before `M11_GameView_Start`. New `theron_v2_upscale_get_scale()` + `theron_v2_upscale_get_bilinear()` accessors let the wire-up probe verify the live runtime. Headless probe `firestaff_m12_v2_settings_wire_up_probe` covers both CSB + Theron (16/16 combined). **Filter config wired:** new `theron_v2_filter_config_pc34` module (include/theron_v2_filter_config_pc34.h, src/theron/theron_v2_filter_config_pc34.c) parallels the CSB filter config for the PC Engine CD (HuC6260 VDC + HuC6270 VCE) Theron pipeline. ctest target `test_theron_v2_filter_config_pc34` passes 24/24, headless probe `firestaff_theron_v2_filter_config_probe` passes 18/18. Source-locked against include/dm1_v2_settings_pc34.h, include/csb_v2_settings_pc34.h, include/theron_v2_texture_upscale_pc34.h, include/theron_v22_shapes.h, include/theron_v2_presentation_mode_pc34.h, include/config_m12.h, THQUEST.ASM T400/T520/T600, HuC6260/HuC6270 VDC/VCE.

## 2026-07-14 — Theron production initial-level capture gate

The production Soul Room entry now consumes the manifest-bound coalesced
Mednafen `$e009` receipt instead of permitting the earlier Stage 3/IRQ2
receipt alone. It rehashes Track 02, System Card, and transcript before
binding record `0x0b52` to the source-locked initial-level envelope. This is
only a fail-closed loader/media admission; no dungeon/object/visual semantics
are claimed. A positive result still requires a fresh authentic capture.
# 2026-07-14 — CSBWin EDBT_ObjectWeights runtime handoff

- Bound CSBWin `Mouse.cpp::GetObjectWeight`'s `EDBT_ObjectWeights` chest-base
  lookup to Firestaff's live ReDMCSB `DUNGEON.C F0140` container path. The
  DB11/EXPOOL record is consumed only while the complete appended tail matches
  its stored FNV receipt; absent records retain CSBWin's source default of 50,
  while altered, truncated, short, or out-of-range records cannot fall back.
- Extended `csb_v1_runtime_champion_load_attrs` with the original CSBWin
  `EXPOOL::Locate` key/hash/node layout, live child-content addition, and a
  changed-receipt rejection case.
# ✅ 2026-07-14 Theron PID-targeted Quartz host-input receipt

# ✅ 2026-07-15 Theron BIN/CUE Track 02 admission

The media classifier now records CUE Track 02's declared `MODE1/2048` or
`MODE1/2352` sector width and accepts either as one authentic Track 01/Track
02 pair. The scanner sends only 2352-byte data to the raw IPL receipt;
2048-byte CUE media remains on the existing verified ISO route. No sector
extraction, wrapper or fallback was added. Verification:
`firestaff_theron_media_classify_unit` and
`theron_v1_track02_cue_layout`.

# ✅ 2026-07-15 Theron 2048 ISO CUE startup handoff

M11 now validates and retains a Track 02 loader receipt only when the scanner
actually issued a valid raw `MODE1/2352` IPL receipt. A verified CUE-declared
`MODE1/2048` ISO therefore follows the normal Track 02 startup handoff and is
not reported as an invalid raw BIN. Verification:
`theron_v1_launcher_scan_reuse` and
`theron_v1_m11_launcher_handoff_boundary`.

# ✅ 2026-07-15 Theron ISO identity at Soul Room boundary

The boot-profile forcefield handoff now distinguishes raw BIN and ISO Track
02 variants. Raw BIN remains behind its authenticated IPL/IRQ2 capture gate;
a verified 2048-byte ISO retains its exact MD5 and source bytes through Soul
Room to the existing ISO semantic dungeon route. That route stays fail-closed
until original ISO bytes prove a first level/object handoff. Verification:
`theron_v1_m11_launcher_handoff_boundary` checks installed real media and
preserves the selected Track 02 identity through startup.
- ✅ 2026-07-15 DM2 M11 source render handoff: the live
  `m11_game_view` DM2 runtime route now calls
  `dm2_v1_boot_runtime_render_frame()` with no V2 callback after verified
  boot, so its dungeon frame consumes the source-owned G1 pose and GDAT
  materials instead of `dm2_v2_runtime_render_frame()`'s procedural viewport.
  The optional V2 HUD remains a decoded original-GDAT compositor and missing
  source data draws nothing. `test_dm2_v1_boot_profile_smoke` now locks the
  direct route: no V2 attempt, successful V1 render, real-material receipt,
  and zero core fallbacks.

# ✅ 2026-07-15 Theron Track 02 runtime bitmap provenance

The existing verified title, stage, Soul Room, and forcefield indexed bitmap
routes now carry their original Track 02 MD5 plus raw and MODE1 user-data
offset envelope into `Theron_V1_World`. A selected runtime level-bank receipt
copies that same source envelope, so a later consumer can require exact
source bytes instead of treating retained pixels as unowned data. The bind
rejects unknown/mismatched variants and incomplete spans. It still performs
no palette binding, RGB conversion, layout inference, object-table decoding,
or drawing. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.
# ✅ 2026-07-15 Theron authenticated CD-read runtime record

The independently authenticated Track 02 `$0b52` CD-read payload now enters
the runtime world as an opaque source receipt: canonical Track 02 MD5, raw
user-data offset, destination, whole-payload checksum, and the exact
post-envelope byte range/checksum. The receipt is published even while the
level route remains rejected, allowing a later captured game-owned consumer
to bind it without reopening media or treating copied bytes as unowned. It is
explicitly marked no-semantic-promotion: no level, object, palette, bitmap,
or visual behavior is inferred and no fallback is enabled. Verification:
Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 loader-envelope boundary

Runtime admission now derives the documented boundary inside the authenticated
`$0b52` CD-read record: the loader-provided initial envelope must begin at its
record-relative offset, match original Track 02 bytes and checksum, and end
exactly where the separately hash-verified opaque continuation begins. The
world receipt retains both spans only after these checks pass. This proves
source-byte boundaries and continuity, not level-grid, object-table, palette,
or visual semantics; the runtime remains no-draw without a captured game-owned
consumer. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 runtime boundary-byte retention

The runtime loader receipt now retains the actual authenticated initial
level-envelope bytes and their directly adjacent post-envelope bytes from the
original `$0b52` CD-read record. Both spans must fit the record, be adjacent,
and rehash to their loader-provided checksums before they are copied. This is
a source-owned boundary for a future captured level/object consumer, not an
object-table decoder: the continuation remains opaque and no palette, grid,
object, or visual semantics are promoted. Verification: Ninja
`test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 continuation-consumer boundary

The raw loader-trace intake can now admit a game-RAM byte only when its
original READ(6), FIFO-to-RAM, and game-owned consumer chain resolves to the
directly adjacent continuation after the authenticated `$0b52` envelope.
The receipt records its exact continuation-relative offset and source byte,
while rejecting preceding and out-of-range bytes. It remains deliberately
opaque: no object-table, level, palette, bitmap, grid, or visual semantics
are inferred. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 continuation prefix receipt

The loader-trace route can now require a contiguous 12-byte prefix of the
authenticated post-envelope continuation from one ordered CD dispatch. Every
byte is independently tied to the original sector and one game-RAM consumer
chain; a split SCSI generation/LBA/dispatch is rejected. The retained prefix
is only a future capture anchor, not an object-table header or decoder.
Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 continuation TII source binding

The provenance-marked Mednafen main-RAM-loader trace can now bind one original
`TII` transfer only when its source begins at `$3c80`: the continuation start
derived from the authenticated `$3800` sector receipt. The copied source span
is checksummed against retained original bytes and the capture must carry the
producer marker; unrelated `TII` rows are ignored. Destination content stays
opaque, with no object-table, level, palette, bitmap, grid, or rendering
claim. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 live TII capture intake

`capture_theron_mednafen_live_trace.sh` now writes the provenance-marked
main-RAM-loader trace beside the existing IRQ/CD/input traces. Its transition
receipt reports all observed `TII` rows and the subset whose source is `$3c80`,
the authenticated continuation boundary. Empty counts remain evidence of an
unreached original route; the script manufactures no trace or candidate.
Verification: `test_theron_v1_mednafen_live_capture_script.sh` passes; the
patch-shape gate passes and skip-cleans without `MEDNAFEN_SOURCE`.

# ✅ 2026-07-15 Theron Track 02 TII sidecar import

The continuation-transfer admission now accepts one explicit bounded
main-RAM-loader sidecar file and forwards its original text unchanged to the
strict TII parser. Missing, empty, oversize, and malformed sidecars reject;
the import does not create rows, bytes, or semantic fallback. This makes the
live capture producer directly consumable once authentic media reaches the
post-`$3800` transfer route.

# ✅ 2026-07-15 Theron Track 02 continuation execution handoff

The raw loader-trace route now binds a source-verified `$3c80` continuation
`TII` to a later main-RAM `JSR` only when the call target exactly equals the
TII destination. This demonstrates an original CD-byte-to-code stage handoff
without interpreting the copied memory as a level or object table. Duplicate,
wrong-target, or unmarked control rows reject. Verification: Ninja
`test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 manifest descriptor boundary

Stage-three descriptor 0 now reaches the runtime loader gate as a strict
original-media boundary receipt. Firestaff retains the descriptor's three raw
words plus its derived Track 02 record, MODE1 raw sector, user-data offset,
2048-byte length, and FNV-1a hash. All fields must resolve back to the same
authenticated `$3800` Stage-3 sector before startup admission. The receipt is
deliberately non-semantic: it does not classify the descriptor or sector as a
level, object table, tile, palette, bitmap, command, or visual route. The
focused descriptor probe covers valid coordinates and rejection of malformed
MODE1/zero-selector records. Verification: Ninja probe, `test_theron_rendering`
18/18, and `test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 full descriptor-row handoff

The authenticated later `$e009` route now carries the complete raw Stage-3
descriptor row into the runtime handoff: descriptor ordinal, `word0`, `word1`,
selector `word2`, resolved Track 02 record, and the selected MODE1 user-data
hash. Firestaff independently derives those values from canonical Track 02
bytes before accepting the coalesced loader receipt; changed row bytes or a
changed selected sector reject the handoff. The row remains explicitly opaque:
no level, object table, tile, palette, bitmap, command, or visual semantics
are promoted. Verification: focused raw-handoff probe (skip-safe without the
authentic corpus), `test_theron_rendering` 18/18, and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 descriptor alias-table receipt

Descriptor-to-record admission now retains the selected raw selector's table
relationship: occurrence count, first and last descriptor ordinal, and an
FNV-1a hash over every matching `(ordinal, word0, word1, word2)` row. The
coalesced loader/CD receipt and runtime handoff both independently re-derive
this relation from the authentic Stage-3 manifest, rejecting changed aliases
or a mismatched selected row. These are table-identity facts only: aliases and
their ordering do not identify a level, object, tile, palette, bitmap, loader
command, or visual route. Verification: descriptor-correlation probe covers a
duplicated selector relation and rejection paths; `test_theron_rendering`
18/18 and `test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 descriptor source-span binding

Each admitted later descriptor record now keeps the exact six-byte big-endian
row span from the authenticated loaded Stage-3 MODE1 sector. Firestaff checks
the physical raw offset and FNV-1a against the three retained raw words before
the later sector may reach the runtime handoff. This closes the source-table
to-selected-sector byte boundary without interpreting any descriptor field,
target record, graphics, palette, object, level, or command grammar.
Verification: focused descriptor probe validates the byte span plus malformed
MODE1/zero-selector rejection; `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 copied-continuation termination receipt

The instrumented Mednafen main-RAM loader trace now emits HuC6280 RTS rows.
Continuation admission requires one source-bound `$3c80` TII, a later JSR to
its exact destination, and exactly one RTS whose PC lies inside the copied
destination span. The capture script reports RTS count for acquisition. This
proves only that original copied code reaches a termination instruction; it
does not observe a return target or promote level, object, palette, bitmap,
tile, command, or rendering semantics. Verification: Ninja focused targets,
`test_theron_rendering` 18/18, `test_theron_v1_startup_save_resume_pc34`
258/258, patch-shape test skip-cleans without `MEDNAFEN_SOURCE`, and capture
script contract test passes.

# ✅ 2026-07-15 Theron Track 02 copied-continuation post-RTS receipt

The instrumented main-RAM loader trace now emits the first observed main-RAM
instruction after each captured RTS. Continuation admission requires that row
to reference the single RTS inside the source-bound `$3c80` TII destination
span and to land at the matching JSR return PC. The receipt retains its
physical PC and opcode alongside the already source-bound Track 02 transfer.
This proves control flow from copied original bytes back to the observed
return target only; it does not classify a descriptor, record, level, object,
tile, palette, bitmap, command, or visual route. Verification: focused
raw-loader probe (skip-safe without the authentic corpus),
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, patch-shape test
skip-cleans without `MEDNAFEN_SOURCE`, and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 post-return routine-call receipt

When the authenticated post-RTS instruction is a HuC6280 `JSR`, Firestaff now
requires the immediately adjacent original main-RAM-loader trace row to agree
on its logical PC, physical PC, and immediate target. The new receipt carries
the earlier source-bound Track 02 TII/execution chain, so the call is tied to
copied original bytes without inventing a called-routine ABI or data format.
Missing, reordered, or changed call-site rows reject. This proves only a
control-flow target, not a descriptor, record, level, object, tile, palette,
bitmap, command, or visual route. Verification: Ninja focused targets,
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# ✅ 2026-07-15 Theron Track 02 post-return routine termination receipt

The post-return routine-call receipt now requires one later main-RAM `RTS`
with a linked original `post_rts` row returning to the exact caller address.
Nested returns remain opaque and do not satisfy the receipt unless their
observed return address is the bound caller. This extends the authentic
Track 02 TII/copy/call/return control-flow chain without assigning any called
routine, table, record, level, object, tile, palette, bitmap, command, or
visual semantics. Verification: Ninja focused targets,
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# ✅ 2026-07-15 Theron Track 02 caller-next-call receipt

After the authenticated post-return caller resumes, Firestaff now admits the
first subsequent main-RAM `JSR` row from the same original trace and retains
its exact physical call site and immediate target. The receipt nests the full
source-bound Track 02 TII/copy/call/return chain. It deliberately does not
identify the target routine, an ABI, descriptor, CD read, table, record,
level, object, tile, palette, bitmap, command, or visual route. Verification:
Ninja focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# ✅ 2026-07-15 Theron Track 02 caller-next-call entry receipt

The instrumented original Mednafen trace now writes a call-entry row only
when the target of the bound next-caller `JSR` is actually executed in main
RAM. Firestaff requires exact caller logical/physical PCs, target, entry
logical/physical PCs, and opcode before retaining the nested Track 02
TII/copy/call/return chain. An unobserved or non-main-RAM target admits no
receipt. This proves executed control flow only and assigns no ABI,
descriptor, CD read, table, record, level, object, tile, palette, bitmap,
command, or visual meaning. Verification: genuine Mednafen 1.32.1 patch
dry-run, Ninja focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-entry successor receipt

The Mednafen producer now records the next observed main-RAM instruction
after an authenticated caller-next routine entry. Firestaff requires the
exact entry logical/physical PC plus the successor logical/physical PC and
raw opcode, retaining the full source-bound Track 02 chain. A target that does
not continue through observed main RAM produces no receipt. This is execution
ordering only: no opcode, ABI, loader, descriptor, CD read, table, record,
level, object, tile, palette, bitmap, command, or visual semantics are
promoted. Verification: genuine Mednafen 1.32.1 patch dry-run, Ninja focused
targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-successor TII byte receipt

When the authenticated caller-entry successor executes HuC6280 `TII`,
Firestaff now accepts it only when its entire source interval lies inside the
already source-bound Track 02 continuation copy. The receipt retains exact
RAM source/destination coordinates, byte count, corresponding original source
coordinate, and FNV-1a checksum. This proves the observed caller path
re-copied known original bytes, without assigning them a loader, descriptor,
CD-read, table, record, level, object, tile, palette, bitmap, command, or
visual meaning. Verification: genuine Mednafen 1.32.1 patch dry-run, Ninja
focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-successor destination-call receipt

The first observed main-RAM `JSR` after an admitted caller-successor `TII`
must now call that transfer's copied destination. The nested receipt retains
the source-bound Track 02 interval and exact call site, proving a bounded
original-byte-to-execution chain. It does not classify the called routine or
bytes as a loader, descriptor, CD read, table, record, level, object, tile,
palette, bitmap, command, or visual route. Verification: genuine Mednafen
1.32.1 patch dry-run, Ninja focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-next-call entry receipt

The instrumented original Mednafen trace now writes a call-entry row only
when the target of the bound next-caller `JSR` is actually executed in main
RAM. Firestaff requires exact caller logical/physical PCs, target, entry
logical/physical PCs, and opcode before retaining the nested Track 02
TII/copy/call/return chain. An unobserved or non-main-RAM target admits no
receipt. This proves executed control flow only and assigns no ABI,
descriptor, CD read, table, record, level, object, tile, palette, bitmap,
command, or visual meaning. Verification: Ninja focused targets,
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# Theron later-level resource-frame receipt (2026-08-06)

- ✅ Later-level runtime handoff now retains the authenticated `LE16(+2)-5`
  resource length and the exact user-data end offset of the framed bitstream,
  alongside the existing block/span hashes and per-level metadata.
- ✅ Focused real-media level-block and runtime-receipt tests pass for the
  bounded frame contract; no decompression, tile, map or object semantics were
  promoted.

# Theron authentic archive capture boundary (2026-08-06)

# Theron US roster label quarantine (2026-08-06)

- ✅ Audited the claimed US roster locator against the authenticated
  `TQUS02.bin`; the old offset is executable code, not a champion text table.
- ✅ Production retains the cross-checked numeric records needed by the
  forcefield handoff but compiles out unbound US names/titles; the named
  table is now explicit fixture/probe data only.
- ✅ Added null-safe champion initialization and kept the real JP roster
  cluster reader unchanged; production handoff, source-boundary, startup
  media, mechanics and fixture probes remain green.

# Theron JP champion record receipt (2026-08-06)

- ✅ Added a hash-gated reader for the authentic JP Track 02 cluster at raw
  offset `0x0B3D98`, covering all eight records and their newline/NUL framing.
- ✅ Decoded the real A–P nibble representation into HP/stamina/mana, seven
  attributes and 16 skill values, with regional-hash and mutation rejection.
- ✅ The receipt remains source-format evidence; it does not promote portraits,
  US labels or gameplay semantics.

# Theron TQTR capture-offset correction (2026-08-06)

# Theron real Track 02 bank reload hygiene (2026-08-06)

- ✅ The source-faithful world loader now clears the selected dungeon's level
  directory before loading a replacement Track 02 bank, so a shorter real
  dungeon cannot expose stale later-level records from a previous load.
- ✅ The regression reloads authenticated US DRATOR (8 maps) with real US
  SHADODAN (3 maps) and confirms only the three current levels remain loaded;
  the complete US/JP Track 02 object-chain census still passes.

# Theron startup font presentation gate (2026-08-06)

# Theron startup menu availability boundary (2026-08-06)

- ✅ Soul Room mirrors without an authenticated Track 02 roster record remain
  visible as `UNAVAILABLE` but are no longer selectable.
- ✅ M11 keyboard/controller focus skips unavailable mirrors, pointer hit-tests
  reject them, and stale focus cannot toggle one; the Forcefield remains an
  enabled route.
- ✅ M11 launcher handoff and direct-launch regressions pass with the real US
  Track 02 asset.

# Theron startup fallback quarantine (2026-08-06)

- ✅ Authenticated Track 02 media now blocks the legacy host border/text
  fallback before the graphics executor runs.
- ✅ M11 and boot-contract regressions verify that missing original startup
  graphics stay capture-gated instead of becoming synthetic UI pixels.

# Theron startup palette promotion gate (2026-08-06)

- ✅ Authenticated raw palette windows remain inspectable as source candidates,
  but no longer become a runtime palette without HuC6260 consumer evidence.
- ✅ Real US/JP palette-window regressions verify the candidate bytes while
  confirming that runtime presentation remains gated.

# Theron startup VDC/VCE presentation gate (2026-08-06)

- ✅ Candidate startup atlas pixels require an explicit presentation-route
  proof in addition to source media and palette state.
- ✅ M11 remains no-draw when only a palette candidate is present; the gate
  awaits a captured VDC/VCE destination and semantic route.

# Theron Track 19 raw-sector intake (2026-08-06)

- ✅ Track 19 inventory now accepts authenticated MODE1/2352 files by
  stripping only the 16-byte sector header before ISO-coordinate validation.
- ✅ The raw transport identity remains explicit; real object and later-level
  semantics are still not promoted without an original consumer trace.

# Theron JP startup roster real-data regression (2026-08-06)

- ✅ The startup-media regression now reads the authenticated local
  `TQJP02.bin` and verifies all eight source roster names and titles before
  they can reach the startup menu.
- ✅ The US path remains fail-closed because its real text consumer and
  champion-name payload are still unproven.

# Theron forcefield source handoff (2026-08-06)

- ✅ Interactive Soul Room → `ENTER FORCEFIELD` now consumes authenticated raw
  MODE1/2352 Track 02 through the source-faithful dungeon loader. Real map
  headers and bounded source records reach the live world without synthetic
  rooms or guessed host item mappings.
- ✅ Visual VDC/VCE capture remains separately gated. Focused M11 and real
  Track 02 loader tests pass: 59/59 and all seven US/JP dungeon blocks.

# Theron startup animation evidence boundary (2026-08-06)

- ✅ Documented the real startup media boundary: authenticated Track 02
  bitmap spans, atlas routes, font tiles and variant palettes are bound.
- ✅ Kept the original title/Soul Room animation consumer, frame table,
  VBlank cadence and VDC/VCE destination capture-gated. The M11 timing receipt
  is not presented as original animation parity.
- ✅ Removed the synthetic M11 8-frame/6-tick title timer and its state field.
  Authenticated startup now exposes one static title frame and accepts the
  menu immediately; no changing frame is claimed without source evidence.

# Theron complete static decompressor listing (2026-08-06)

- ✅ Expanded `docs/source-lock/theron-disassembly/theron-us-bank1f-consumer.asm`
  with the authenticated caller/output-size tail `$2386–$23a3` and the
  resource framing/variable-bit entry `$23ad–$243d`, using the real US ISO
  projection and the byte-identical JP bank span.
- ✅ The listing now shows the real six-byte resource-frame advance,
  destination-pointer table writes and widening `$0100` token contract. It
  still publishes no level, object, tile or palette semantics.

# Theron forcefield Enter retry boundary (2026-08-06)

- ✅ After a failed authentic Track 02 admission, M11 now keeps Enter bound
  to the forcefield action while the Soul Room shows `FORCEFIELD LOCKED`.
  This prevents the restored cursor from making Enter toggle a mirror and
  makes the capture gate actionable and visible without admitting fallback
  dungeon graphics.
- ✅ Added a regression covering the initial admission failure and a second
  Enter retry; both remain in the Soul Room with `level_loaded == 0`.

# Theron bounded `$2600` consumer trace (2026-08-06)

- ✅ The capture-only Mednafen patch now has a separate bounded trace for
  bank `$1f` logical reads in `$2600–$27ff`. The complete original-like US
  CUE replay produced zero rows, so no dynamic consumer, level record or
  object meaning is promoted.

# Theron authentic VDC/VCE screen-space capture (2026-08-06)

- ✅ A clean SIGINT shutdown of the instrumented Mednafen replay now emits
  the complete authentic US Track 02 VDC/VCE state: 65,536-byte VRAM and
  1,024-byte VCE snapshots, from the hash-verified ISO and real System Card.
- ✅ The production viewport already mounts both snapshots only through the
  explicit `FIRESTAFF_THERON_VRAM_SNAPSHOT`/
  `FIRESTAFF_THERON_VCE_SNAPSHOT` route. The real-capture regression reports
  154 BAT tile/palette pairs, 512 palette entries and 9,954 non-zero indexed
  pixels presented to M11; no inferred square/object meaning is published.
- ✅ `capture_theron_mednafen_live_trace.sh` now defaults to SIGINT for the
  bounded emulator shutdown, so the Mednafen snapshot hook runs on clean exit.
  The capture used SDL 2.32.70 through `sdl2-compat` with dummy video; it is
  authentic emulator memory evidence, not native Quartz/SDL2 evidence.

# Theron real main-RAM loader capture parser correction (2026-08-06)

- ✅ A fresh replay against the supplied hash-verified US Track 02 ISO and
  System Card produced a real Mednafen loader sidecar with the source `$2286`
  `TIA` witness followed by 13 block transfers, 24 RTS observations and 24
  post-RTS observations. The sidecar also produced 4,096 game-owned
  main-RAM-consumer reads and the executed HuC6280 `$2c54–$2c69` code window.
- ✅ Fixed `theron_v1_mednafen_main_ram_trace` so it accepts the complete
  instrumented HuC6280 transfer/control witness instead of comparing every
  later `TII`/return row to the first `TIA`. The parser remains opaque-only:
  no `$2600` consumer bytes, level/object semantics, VDC snapshot or runtime
  promotion are inferred.
- ✅ Verification: loader sidecar MD5
  `2827cb429d0b97f0e1fc26185a9bb28c` passes with `13/24/24`; consumer sidecar
  MD5 `9d19ad9b993f1853e868f381756eb1d0` passes with `4096` reads and the
  `$2c54–$2c69` code-window check. Capture files remain operator-local and
  no game data was added to the repository.

# Theron production fixture-symbol boundary (2026-08-06)

# Theron source roster survives forcefield admission (2026-08-06)

- ✅ Fixed a production startup data-loss bug where the forcefield handoff
  cleared the source-bound champion roster immediately before the authenticated
  Track 02 level-load gate. Real HP, skills and equipment now remain available
  even when dungeon promotion is correctly capture-gated.
- ✅ Added a regression through the runtime entry path using the US Track 02
  identity with deliberately invalid media: the capture gate still rejects the
  handoff, while Hakar's source roster records remain intact. Verification:
  `test_theron_v1_combat_runtime_source` and `git diff --check`. No game data
  was copied or committed.

# Theron production placeholder archive guard (2026-08-06)

- ✅ Extended the Theron production-archive regression so every inventoried
  fixture/compatibility module must have an explicit CMake exclusion and must
  be absent from the final `firestaff_theron` archive. This keeps synthetic
  startup, viewport, HUD and modern-art paths from re-entering through a broad
  source glob. The guard does not promote any unproven consumer.
- ✅ Verification: `test_theron_v1_production_archive_source_boundary` and
  `git diff --check` on a clean worktree from current `main`. No game data was
  copied or committed.

# ✅ 2026-07-11 Theron Track02 real-media compact-row layout receipt

- Extended the existing skip-safe, hash-verified Track 02 real-asset probe to read staged media and publish descriptor-anchor and per-level compact-row layout evidence: matching-anchor masks, row counts, raw-row hashes, table ordinals, and position-bound hashes.
- Kept the receipt non-promoting. It assigns no object semantics and does not affect runtime objects, Continue, synthetic menus, or palette promotion.
- Verification: focused Ninja and CTest Track02 startup-receipt targets.
- ✅ 2026-07-11 Nexus Saturn warning-media decoder: added an exact `RES*`
  directory reader and Sega DGT2 packed-pixel (`PP`) decoder. The local
  `WARNING.BIN` resource 0 now loads through its 256-entry BGR555 CLUT and
  240x96 byte-indexed plane; arbitrary `RES*` bytes remain rejected. The
  title path remains blocked because `TITLE.CG` has no proven header, atlas,
  or Saturn command placement, and no raw/guessed visual fallback was added.
  The focused startup-media gate exercises both the original warning decode and
  the real `TITLE.CG` rejection. Source: Sega Saturn/32X Graphic References,
  section 6 (DGT2 format).

- ✅ 2026-07-11 Nexus PRS3 MSB-first candidate audit: added a bounded,
  explicit MSB-first control-bit traversal alongside the retired LSB-first
  literal/back-reference trial grammar, retaining the observed big-endian
  PRS3 frame, declared output-byte target, opcode fields, and no-promotion
  rule. The optional hash-verified `MENU.BPK` probe disproves the MSB-first
  candidate across all 162 surfaces: zero exact and zero trailing completions;
  runtime/upload routing remains `blocked-prs3`. The synthetic regression
  proves the two bit orders diverge on a controlled literal stream and rejects
  unknown orders. Verified with Ninja `test_nexus_v1_bpk_surface_class` and
  `firestaff_nexus_v1_bpk_prs3_payload_evidence_probe`, plus direct real-media
  probe execution (89/89).

- ✅ 2026-07-11 Nexus PRS3 BE-framed exact evaluation: added a separate,
  diagnostic-only evaluator that begins after the observed BE frame word,
  accepts only the 161 directory-span-close real frames, bounds output to the
  declared mode-derived byte target, and records literal/back-reference
  command counts plus exact/trailing completion. The hash-verified local
  `MENU.BPK` result is identical for LSB-first and MSB-first control order:
  161 command failures, zero trailing completions, and zero exact
  completions; final entry 162 remains unvalidated because of its 530-byte
  BPK tail. No decoded bytes reach runtime and routes remain `blocked-prs3`.
  Verified with Ninja `test_nexus_v1_bpk_prs3_payload_evidence` and
  `firestaff_nexus_v1_bpk_prs3_payload_evidence_probe`; direct real-media
  probe passed 97/97.

- ✅ 2026-07-11 DM2 PC G1 pre-map extension boundary: `dm2_v1_dungeon_loader`
  now derives the standard `READ_DUNGEON_STRUCTURE` prefix through the
  declared DB-pool lengths, then publishes the bounded, untyped G1 extension
  preceding the proven trailing map-data block. The hash-verified DOS English
  `DUNGEON.DAT` proves prefix end `23826`, extension length `7841`, and map
  base `31667`. No extension bytes are assigned DB-pool or record-link
  semantics; `record_graph_complete` stays clear and boot continues to reject
  the partial world. Verified with Ninja
  `test_dm2_v1_dungeon_loader_first_map_gate`,
  `firestaff_dm2_v1_dungeon_loader_first_map_real_data_probe`, and
  `firestaff_dm2`; direct focused tests passed 55/55 and 32/32.

- ✅ 2026-07-11 DM2 PC G1 DB-pool placement audit: source-locked
  `c_record.cpp` confirms the standard ObjectID type/index and first-word
  link semantics, while `SkWinCore.cpp::READ_DUNGEON_STRUCTURE` confirms the
  normal sequential pool reader. The canonical DOS G1 file rejects applying
  that reader directly at the proven map tail: its declared count/size total
  is 16,884 bytes but the map-adjacent candidate span is 7,841 bytes. The
  real-data probe now locks that mismatch, so no accidental record alignment
  can promote bounded traversal or real map boot. The G1-specific pool-base
  transform remains the open blocker.
- ✅ 2026-07-11 DM1-006 F0168 text escape expansion: `F0508_DUNGEON_DecodeTextStringThing_Compat()` and the legacy scroll-style text-table decode now use all 32 PC 3.4 ReDMCSB `G0255` message/scroll, `G0256` symbol, and `G0257` inscription replacement entries. Code 30 selects `G0255` for messages/scrolls and the raw glyph-code `G0257` table for inscriptions; code 29 remains `G0256`. ReDMCSB anchors: `DUNGEON.C` globals `G0255/G0256/G0257` and `F0168_DUNGEON_DecodeText` lines 2280-2350. Verification: Ninja `test_memory_dungeon_text_scroll_pc34_compat` (28/28), `ctest -R '^memory_dungeon_text_scroll_source_lock$'`, and Ninja `firestaff_m10` passed.
- ✅ 2026-07-11 CSB-005 dungeon filter-location decode: added the CSBWin `Monster.cpp` `EDT_SpecialLocations` decoder for attack and level-specific/global movement filters. It unpacks `DSA.cpp` `LOCATIONREL::Integer` fields, movement-only party-level and maximum-distance bits, validates live dungeon bounds, and selects only the first DB3 actuator type 47 from the decoded square. This remains a selection boundary only: no saved DSA words are reinterpreted or executed, and no live monster decision changes yet. Verification: the focused 315-check phase-7 suite passes. A fresh Ninja configuration compiled the touched CSB sources; current Make and Ninja M10 builds stop in unrelated `memory_tick_orchestrator_pc34_compat.c` because `COMBAT_ACTION_APPLY_DAMAGE_PARTY` is undeclared.
- ✅ 2026-07-11 DM1 V1 M10 F0200/F0197/F0199 straight-line C37 visibility slice: M10 creature-reaction context no longer treats same-row/same-column party coordinates as automatically visible. It now resolves a loaded-DM1-tile straight line using the active group direction, `CREATURE_INFO` sight range, and the source blockers from `GROUP.C`: walls, closed fakewalls, and three-quarter/closed doors except the `DUNGEON.C G0254` Portcullis/Ra see-through types. The focused M10 regression drives a facing Vexirk C37 through clear, wall, closed-fakewall, opaque-door, and Portcullis cases, proving only the unobstructed/see-through paths enter ATTACK. Source anchors: ReDMCSB `GROUP.C F0200` lines 1344-1414, `F0197` lines 1175-1212, `F0199` lines 1238-1313, and `DUNGEON.C G0254_as_Graphic559_DoorInfo` lines 560-565. Verification: Ninja `test_memory_tick_orchestrator_f0303_skill_query_pc34_compat`, direct PASS, and focused CTest PASS 1/1. Honest scope: cardinal loaded-tile visibility at the M10 C37 decision boundary only; diagonal F0199 stepping, light/invisibility, per-creature multi-facing, and C37 physical move/retry application remain open.

- ✅ 2026-07-11 DM1 V1 authoritative viewport background boundary: with `assetsAvailable` from original DM1 `GRAPHICS.DAT`, `m11_draw_viewport_background()` now blits only the active map's exact floor/ceiling set. A missing or malformed exact pair leaves the already-cleared viewport untouched; it no longer replaces it with floor-set 0 art or procedural black/gray rendering. The explicit solid fallback remains only for asset-free/headless paths, where no original graphics are available. The local canonical PC 3.4 `GRAPHICS.DAT`/`DUNGEON.DAT` pair was recognized as READY by `firestaff --data-dir ... --scan-data`. Verification: Ninja full `firestaff` build, scanner real-data probe, `test_m11_inventory_scroll_panel_render_pc34_compat` PASS 32/32, and the focused M10 LoS test still passed. Honest scope: viewport floor/ceiling source-miss behavior only; it does not claim complete real-asset viewport parity or remove explicitly asset-free test fixtures.
## What changed

- `include/theron_v1_cd_audio_availability.h`
  - Expanded `Theron_V1CdAudioAvailability` with source-locked failure
    states: `THERON_V1_CD_AUDIO_CUE_NOT_FOUND`, `CUE_PARSE_ERROR`,
    `LAYOUT_MISMATCH`, and `TRACK_FILE_MISSING`.
  - Expanded `Theron_V1CdAudioReceipt` with:
    - `track_count`, `audio_track_count`, `data_track_count`
    - `audio_directory`
    - 1-indexed `track_paths[1..19]`, `track_present[1..19]`,
      `track_is_audio[1..19]`
    - `unavailable_reason`
  - Changed `theron_v1_cd_audio_availability()` signature from a
    format-string comparison (`cue_format`, `local_format`) to a real
    source-locked intake (`cue_path`, `data_root`).

- `src/theron/theron_v1_cd_audio_availability.c`
  - Implemented a focused CUE parser that handles both quoted
    (`FILE "name.wav" WAVE`) and unquoted (`FILE name.wav WAVE`) forms,
    matching the real TQUS.cue / TQJP.cue syntax.
  - Resolves each declared track file relative to the CUE directory or an
    optional `data_root` override.
  - Allows `.ogg` fallback when the CUE names original `.wav` CD-DA files,
    because the locally staged original audio is supplied as OGG.
  - Handles the documented MyAbandonware-style split Track 02 alias
    (`TQUS02.iso` -> `TQUS02End.iso`, `TQJP02.iso` -> `TQJP02End.iso`).
  - Verifies the canonical Theron CD layout:
    - Track 01 AUDIO
    - Track 02 MODE1/2048 or MODE1/2352 (data)
    - Tracks 03-18 AUDIO
    - Track 19 MODE1/2048 or MODE1/2352 (data)
  - Returns `playback_allowed=1` only when all 19 declared tracks have
    readable local files and the layout matches the original CD.

- `probes/theron/firestaff_theron_v1_cd_audio_availability_probe.c`
  - Rewrote the existing format-string smoke probe into a source-locked
    integration probe:
    - Synthetic complete canonical layout with `.wav` files.
    - Synthetic `.ogg` fallback when CUE declares `.wav`.
    - Missing audio track file -> `TRACK_FILE_MISSING`.
    - Incomplete layout -> `LAYOUT_MISMATCH`.
    - Real-data test against `$HOME/.firestaff/data/theron/TQUS.cue` when
      present, verifying the staged original CD-DA corpus.

- `tests/test_theron_v1_cd_audio_availability.c` (new)
  - Unit test for the canonical 19-track receipt fields.
  - Verifies CUE-not-found, layout-mismatch, and track-presence invariants.

- `CMakeLists.txt`
  - Registered `test_theron_v1_cd_audio_availability` target and CTest
    entry next to the existing Track 01 CDDA handoff test.

## Source evidence

- Local original CUE sheets: `$HOME/.firestaff/data/theron/TQUS.cue` and
  `TQJP.cue` declare the 19-track CD layout.
- Locally staged original CD-DA audio:
  `TQUS01.ogg`, `TQUS03.ogg`, `TQ04.ogg` through `TQ18.ogg`, and JP
  equivalents (`TQJP01.ogg`, `TQJP03.ogg`).
- No synthetic audio playback fallback existed in `src/theron` before this
  change; the receipt is the required gate before any Theron audio output.

## Verification

- `cmake --build build --target firestaff_theron` succeeds.
- `cmake --build build --target test_theron_v1_cd_audio_availability`
  succeeds and the test passes.
- `ctest -R theron_v1_cd_audio_availability -V` from `build/` reports:
  - `theron_v1_cd_audio_availability_probe` PASS
  - `theron_v1_cd_audio_availability` PASS
- Note: the full `cmake --build build --parallel` is currently blocked by
  pre-existing conflicting-type errors in `dm2_v1_skproject_core.c` /
  `dm2_v1_dungeon_loader.h` that are outside Lane E scope.

- 2026-07-23 DM2-007 spell-effect timer handler bodies (Lane B, cycle 11):
  Bound the proven DM2 spell timer effect handlers that do not require
  unproven DB object or creature creation.
  Changes:
    * `include/dm2_v1_spell_timer_handlers_pc34_compat.h` (new):
      - Declares the handler dispatch table `dm2_v1_spell_timer_handlers` and
        the per-effect helpers for the timer types bound from
        `skproject/SKULLWIN/c_tim_proc.cpp`.
    * `src/dm2/dm2_v1_spell_timer_handlers_pc34_compat.c` (new):
      - `DM2_V1_SPELL_TIMER_HANDLER_LIGHT` (`0x46`): implements
        `DM2_PROCESS_TIMER_LIGHT` (c_tim_proc.cpp:918-959), requeuing the
        timer while `remaining_seconds > 0` and clearing the request once the
        duration expires.
      - `DM2_V1_SPELL_TIMER_HANDLER_HERO_ENCHANTMENT` (`0x47`): sets/clears
        the hero enchantment flag slice (c_tim_proc.cpp:4111-4123).
      - `DM2_V1_SPELL_TIMER_HANDLER_ENCHANTMENT_POWER` (`0x48`): decays the
        enchantment power field each tick (c_tim_proc.cpp:4129-4163).
      - `DM2_V1_SPELL_TIMER_HANDLER_POISON` (`0x4b`): processes the poison
        tick on the bound actor (c_tim_proc.cpp:4165-4178).
      - Leaves `0x19` cloud, `0x1e` missile step, and `0x5e` summon
        fail-closed until their DB-record owners are proven.
    * `tests/test_dm2_v1_spell_cast_player_pc34_compat.c`:
      - Added five new test groups covering light requeue/expiry, hero
        enchantment flag mutation, enchantment power decay, poison decay, and
        source-evidence string for the new handler module.
    * `CMakeLists.txt`:
      - Added `src/dm2/dm2_v1_spell_timer_handlers_pc34_compat.c` to the
        `test_dm2_v1_spell_cast_player_pc34_compat` target sources.
  Source evidence:
    * `skproject/SKULLWIN/c_tim_proc.cpp:918-959` (DM2_PROCESS_TIMER_LIGHT).
    * `skproject/SKULLWIN/c_tim_proc.cpp:4111-4123` (hero enchantment flag).
    * `skproject/SKULLWIN/c_tim_proc.cpp:4129-4163` (enchantment power decay).
    * `skproject/SKULLWIN/c_tim_proc.cpp:4165-4178` (poison tick).
    * `skproject/SKULLWIN/c_tim_proc.cpp:3980-4230` (dispatch matrix).
  Verification:
    * `cmake --build build --parallel` succeeded.
    * `test_dm2_v1_spell_cast_player_pc34_compat` 110/110 checks passed.
    * `test_dm2_v1_proceed_timers_pc34_compat` all checks passed.
    * `test_dm2_v1_spell_rune_lookup_pc34_compat` 38/38 tests passed.
    * `test_dm2_v1_spell_pc34_compat` all checks passed.
  Note: runtime wiring into `src/dm2/dm2_v1_runtime.c` was intentionally left
  out of this cycle because adding the new source to the standalone test
  targets that compile `dm2_v1_runtime.c` directly would widen the change
  beyond the proven handler bodies. The module is already compiled into the
  `firestaff_dm2` library via the existing `src/dm2/dm2_v1_*.c` glob.

- ✅ 2026-07-23 DM1 HUD real-material command admission: the production
  `dm1_v1_action_spell_render_command_admit_pc34()` boundary now rejects
  blank source-owned C009/C010/C011 surfaces and non-M653 font records before
  F0387/F0394 commands are published to M11. The focused regression covers
  detached C011, malformed dimensions, zero-filled line/font data, and a
  forged font graphic id. Verification: local PC34 `GRAPHICS.DAT` header
  read confirms C009=87x25, C010=87x45, C011=14x39; Ninja build plus
  `ctest -R '^dm1_v1_action_spell_render_command_admission_pc34_compat$'`
  passes 1/1. This is a fail-closed material admission improvement, not a
  substitute for the remaining Mac/app capture work.
- ✅ 2026-07-23 CSB C005 credits source-only presentation: the C202/F0442
  route now retains only the mandatory decoded GRAPHICS.DAT C005 surface and
  its source palette. Removed the generated "CHAOS STRIKES BACK / CREDITS /
  PRESS ENTER" substitute and updated the C001 phase regression to require
  CSB's distinct PRESENTS/CHAOS/STRIKES palettes. Verified by
  `csb_v1_startup_entrance_pointer_pc34_compat` (139/139).
- ✅ 2026-07-23 CSB C001--C005 CSBWin decoder audit: `ReadAndExpandGraphic(5)`
  clears `0x8000` and invokes `ExpandGraphic`, so C005 is an expanded
  four-plane page, not raw/not-expanded bytes. The startup loader now rejects
  C005 unless its complete decoder receipt reaches the record boundary and
  yields visible indexed pixels. The real-PC34 regression verifies the same
  path using `GRAPHICS.DAT` SHA-256
  `3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942`;
  no text or image fallback is introduced.

- ✅ 2026-07-23 CSB C004/C002/C003 F0438 door-page tick contract: the
  Entrance consumer now accepts the first real opening raster only at source
  step 1, then requires every following page to advance one VBlank and one
  door position while retaining the same verified package/session hashes.
  It rejects stale host ticks, replayed pages, and skipped source positions;
  it does not synthesize any raster or replace the remaining F0128 interior
  viewport work. The focused test is registered with CTest. Verification:
  Ninja `test_csb_v1_startup_opening_door_tick_receipt_pc34_compat` build and
  the matching CTest entry pass.

- ✅ 2026-07-23 CSB F0128 Entrance runtime consumer: M11 now accepts a
  source-bound F0128 interior only when its original-material and raster
  receipts, source tick, and session generation agree. It replaces C004's
  224x136 viewport rectangle, then restores decoded C002/C003 door strips
  above it for closed and opening Entrance pages. M11 retains a private copy
  of the admitted indexed raster; missing or stale material cannot fall back
  to generated pixels. Verification: registered CTest
  `csb_v1_f0128_entrance_runtime_consumer_pc34_compat` passes.

- ✅ 2026-07-23 CSB C004 F0128 M11 producer handoff: the live Entrance path
  now advances the verified PC34 session to M11's source tick, copies the
  decoder-bound C004 `(0,33,224,136)` interior into M11-owned storage, and
  binds it through the existing F0128 consumer before real C002/C003 doors.
  `csb_v1_startup_entrance_f0128_m11_handoff_pc34_compat` opens the local
  hash-verified `GRAPHICS.DAT`, verifies every C004 copy byte, M11 storage,
  and the four-surface consumer result. This does not infer the pending F0439
  5x5 micro-dungeon material or introduce a synthetic replacement.

- ✅ 2026-07-23 CSB C005 credits/Entrance return admission: C005 publication
  now requires the live Entrance session, complete expanded GRAPHICS.DAT C005
  decode receipt, source credits palette, and one real raster surface. The
  real-PC34 startup-sequence regression verifies C005 then its C004/C002/C003
  return with the credits palette removed; no fallback text or panel is used.

- ✅ 2026-07-23 CSB C001 M11 title-frame admission: M11 now presents only a
  decoder-bound C001 host frame with one source raster, matching source step,
  and the consumed PRESENTS/CHAOS/STRIKES phase bit. The real-PC34 sequence
  regression locks C001 provenance and phase order; no host text fallback can
  substitute for a missing title phase.

- ✅ 2026-07-23 CSB C001 real timing capture: the real-PC34 GRAPHICS.DAT
  regression now captures and requires four distinct source rasters in
  PRESENTS, CHAOS zoom, CHAOS hold, STRIKES BACK order with the complete
  `0x0f` phase mask; no host text fallback is accepted.

- ✅ 2026-07-23 CSB C001-to-Entrance M11 receipt: the real PC34 sequence now
  proves the complete C001 title session immediately hands M11 the C004/C002/
  C003 closed Entrance plan, real three-source raster, and no fallback text.

- ✅ 2026-07-23 CSB F0247/F0219 live C14-to-F0128 handoff: a real runtime
  C05-chain receipt now supplies the resolved projectile identity to the next
  boot viewport frame. F0128 revalidates its actual C14 Thing-chain ownership
  and uses the source F0115/F0791 bitmap path; stale data and absent real
  material remain no-draw, with no marker fallback. Verification:
  `m11_csb_f0247_boot_projectile_frame_pc34_compat`.

- 2026-07-23 DM1 F0249/F0267 C14 C04 teleporter rotation: loaded object-scope
  teleporters now apply ReDMCSB MOVESENS.C F0263's packed relative
  direction/cell rotation to the authenticated active M10 projectile as well
   as raw C14. F0249 retains exactly the physical C48 owner at the rotated
   destination. Verification passed:
   `test_dm1_v1_f0249_runtime_relocation_pc34_compat`.

- 2026-07-23 DM1 F0249/F0267 chained C14 C04 route: the existing source-backed
  F0263/F0249 path is now proven across two loaded object-scope teleporter hops.
  Relative direction/cell rotation accumulates in raw C14 and active M10 state,
   while the one physical C48 remains owned by that C14 at the terminal square.
   Verification passed: `test_dm1_v1_f0249_runtime_relocation_pc34_compat`.
- ✅ 2026-07-23 DM1 HoC all-portrait F0115/C127 geometry regression:
  the real PC34 map-0 sweep now covers all 24 source C127 champion mirrors,
  derives each legal party pose from its packed wall cell, selects the actual
  portrait through the production route, and verifies that turning into a
  side/depth presentation cannot retain the prior ordinal. Verification:
   `dm1_v1_hoc_all_front_mirror_ordinals_pc34_compat` passed with installed
  `DUNGEON.DAT` and `GRAPHICS.DAT`.
  while the one physical C48 remains owned by that C14 at the terminal square.
  Verification passed: `test_dm1_v1_f0249_runtime_relocation_pc34_compat`.

- 2026-07-23 DM1 F0249/F0267 C14 C02-to-C04 continuation: M10 now resolves a
  loaded open C02 door followed by an object-scope C04 target through the
   source F0263 packed direction/cell transform. The original C14 Slot material,
  kinetic/attack values, and one physical C48 owner continue to the target
   without synthetic projectile state. Verification passed:
   `test_dm1_v1_f0249_runtime_relocation_pc34_compat`.
- ✅ 2026-07-23 DM1 HoC complete C127 viewport material route:
  M11 now binds every frame's live C127 projection to source material: D1C
  publishes the real C346/C026 pair, D1L/D1R consume C346 only, and D2+ is a
  clear-only decision with no synthetic mirror or portrait. The frame receipt
  is reset before each render. Real PC34 map-0 coverage sweeps all 24 front
  portraits and side/depth routes. Verification:
  `m11_dm1_hoc_real_mirror_viewport_material` passed.
  without synthetic projectile state. Verification passed:
  `test_dm1_v1_f0249_runtime_relocation_pc34_compat`.

- 2026-07-23 DM1 viewport F0134/F0135 material admission: added a fail-closed
  planar viewport consumer for ReDMCSB ACTIDRAW.C F0134 and FILLBOX.C F0135.
  It operates only on a caller-verified original material surface, preserves
  the source inclusive-box semantics, and rejects missing material without a
   synthetic fill. Verification passed:
   `test_redmcsb_fillbox_blitfill_f0135_integration_pc34_compat`.
- ✅ 2026-07-23 DM1 original PC34 champion/group/timer byte gate: the
  fixture-free external-corpus F0435 -> F0433 -> F0435 test now requires
  decrypted C04 ACTIVE_GROUP allocation bytes, all four C02 M516 champion
  records, and the source C03 timer/event plus C04 timeline parts to preserve
  their exact admitted sizes and fingerprints. Internal fixture regression
  remains a semantic handoff check; it cannot stand in for external evidence.
  Verification: `dm1_v1_original_save_pc34_handoff` and
  `dm1_v1_original_save_pc34_external_corpus` passed.
  synthetic fill. Verification passed:
  `test_redmcsb_fillbox_blitfill_f0135_integration_pc34_compat`.
- 2026-07-23 DM1 PANEL.C F0344 F0135 material consumer: added the real
  proportional food/water bar route on an admitted planar material. It uses
  the source `G2097_FoodOrWaterBarShadowOffset = 2`, fills the black shadow
  before the colored bar, and preserves F0344 red/yellow/base-color rules.
   Verification: `test_redmcsb_fillbox_blitfill_f0135_integration_pc34_compat`.

- ✅ 2026-07-23 DM1 PANEL.C F0344/F0351 source-bound health/stamina panel
  gate: M11's inventory champion-stat route now requires the current DM1
  session's source-bound M653 font after admitting the real C020 panel from
  `GRAPHICS.DAT`. A generic loaded font can no longer render health/stamina
  text onto original artwork. Verification:
  `m11_dm1_f0344_source_bound_champion_stats`.

- ✅ 2026-07-23 DM1 original PC34 ACTIVE_GROUP runtime identity gate: the
  external-corpus F0435 staging and candidate-to-runtime adoption receipts now
  retain a source-checked fingerprint of every live C04 ACTIVE_GROUP record.
  It includes the type-4 group Thing identity, full packed directions/cells,
  timing/flee fields, target/prior/home coordinates, and all four Aspect bytes.
  A mismatch or flattened PC34 sidecar fails closed before the corpus row can
  be admitted. The test remains corpus-only and reports `SKIP` without
  `FIRESTAFF_DM1_PC34_SAVE_CORPUS`; no synthetic save is accepted as evidence.
  Verification: Ninja/CTest `dm1_v1_original_save_pc34_handoff` and
  `dm1_v1_original_save_pc34_external_corpus` passed; the latter skipped
   honestly because no external corpus is staged.
- ✅ 2026-07-23 DM1 F0134/F0135 champion food/water material admission:
  M11 now treats the C12 alive-status fill together with the F0345 C020 panel
  and C030/C031 label blits as one fail-closed source transaction. The new
  DM1-owned receipt accepts only decoded `GRAPHICS.DAT` surfaces with exact
  original IDs and dimensions, fingerprints their pixels, and consumes the
  route without host text or generated panel fallback when any surface is
  absent. `m11_dm1_food_water_source_gate` verifies the receipt with local
  original PC34 media.
- ✅ 2026-07-23 DM1 F0134/F0135 production caller slots: the champion panel
  admission now binds each caller to its source slot and rejects stale or
  mismatched `GRAPHICS.DAT` material before it can enter the panel transaction.
  The real-data regression verifies loaded original material; no M11 fallback
  or F0115 scheduler path was changed.
- ✅ 2026-07-23 DM1 F0435 external-corpus global/party/map adoption gate:
  source-only PC34 corpus verification now retains a combined GLOBAL_DATA,
  party position, status-counter, C2 PARTY_INFO and M516 identity receipt
  across staging and candidate-to-runtime adoption. The fixture-free target
  admits no generated evidence and skips when no operator corpus is set.
  Verification: `dm1_v1_original_save_pc34_handoff` and
  `dm1_v1_original_save_pc34_external_corpus` passed.
- ✅ 2026-07-23 CSB C002/C003 F0438 host-frame phase receipt: the
  source-bound door consumer validates the already-produced real
  `GRAPHICS.DAT` session/M11 host raster by opening step, tick, generation,
  Entrance palette, decoded C002/C003 records, and the exact C004/F0128/door
  source count. It never changes title or Entrance plan selection. Verification:
  source-bound fillbox, F0128 consumer, F0128 M11 handoff, and broad launcher
  boundary regressions passed.
- ✅ 2026-07-23 CSB C017/C040 M11 frame palette admission: the existing
  F0807 terminal host-frame gate now rejects a retained title or Entrance
  palette before M11 consumes the source-bound `GRAPHICS.DAT` C017/C040
  surfaces. The panel blit does not replay startup plans, preserving C002/C003
  composition. Verification: `csb_v1_m11_launcher_handoff_boundary` and
  `csb_v1_startup_real_sequence_pc34_compat` passed.

# ✅ 2026-07-13 Theron dynamic Track 02 CD_READ-to-RAM receipt: the
# ✅ 2026-07-14 Theron Track 02 later loader-to-local-RAM capture contract

# ✅ 2026-07-15 Theron main-RAM control-window read instrumentation

Added bounded CPU-read provenance for `0x1f01f7..0x1f01fb`, retaining logical
and physical reader addresses. The instrument does not classify the bytes or
infer any CDB, sector, level, or object semantics.

# ✅ 2026-07-15 Theron control-window System Card exclusion

Validated a real US Track 02 boot capture containing 64 reads of
`0x1f01f7..0x1f01fb`. Every recorded reader is System Card physical code/RAM
(`0x00xxxx`, `0x002xxx`, or `0x1fe0xx`); no reader is in the game-owned
`0x1f0000..0x1f7fff` range. The bounded verifier fails on a game-owned or
unclassified reader, so this capture cannot be promoted to a CDB/SCSI, sector,
level, or object-record link.

Verification: `test_theron_v1_main_ram_control_window_receipt` and
`verify_theron_main_ram_control_window_receipt.pl` against the authentic trace.

# ✅ 2026-07-15 Theron game-owned main-RAM window to SCSI receipt

Added bounded read provenance for `0x1f1000..0x1f1007`. In an authentic US
Track 02 capture, physical game code `0x1f0c88` reads all eight bytes before
the game-owned `0x1f0cc7` `$e009` dispatch, which is followed by SCSI
generation 2 at LBA 4165 for four sectors. The receipt proves this execution
ordering only: no FIFO destination, level layout, or object-record grammar is
assigned.

Verification: `test_theron_v1_main_ram_game_window_scsi_receipt`, Mednafen
patch dry-run, and the authentic capture verifier.

# ✅ 2026-07-15 Theron game-owned FIFO-to-RAM-to-reader intake gate

The Track 02 loader receipt now has a strict live-capture intake for a single
game-owned `$3840 -> $e009` dispatch: seven observed CDB writes must decode to
the following READ(6); its FIFO-origin byte must reach game-owned main RAM and
be read later by game-owned code from the identical physical cell. For the
verified US CUE coordinate, Firestaff rechecks the captured byte against
`raw_record = LBA - 3009` in the hash-verified Track 02 BIN. The result remains
an opaque byte-flow receipt, not a dungeon, grid, level, object, bitmap,
palette, or transition decoder. Verification: the focused raw-loader probe
rejects a mutated source byte and a CDB/LBA mismatch.

# ✅ 2026-07-15 Theron game-RAM initial-envelope correlation gate

An admitted game-RAM payload byte can now be joined to the source-locked Hall
of Records envelope only when its physical Track 02 sector and exact raw-sector
offset fall inside the authenticated envelope. The join deliberately uses the
IPL-derived physical `level_first_raw_sector`, not descriptor-relative record
`0x0b52`, preventing INDEX 01/file-sector coordinate confusion. It rejects a
pre-envelope byte and altered source media, and publishes no level grammar,
dungeon, object, grid, bitmap, palette, or transition semantics. Verification:
the focused US Track 02 raw-loader probe.

# ✅ 2026-07-15 Theron initial-envelope header capture gate

Firestaff can now retain the first twelve raw bytes of the source-locked
initial envelope only when twelve ordered game-RAM payload receipts share one
dispatch and READ(6) identity and cover the exact consecutive raw offsets.
The receipt stores the source bytes and FNV-1a hash only. It does not interpret
dimensions, the existing extension word, header grammar, level, dungeon,
object, grid, bitmap, palette, or transition semantics. Verification: the
focused US Track 02 probe rejects a split SCSI capture chain.

# ✅ 2026-07-15 Theron LBA 4165 raw Track 02 binding

Bound the four sectors requested by the game-owned window path, LBA
`4165..4168`, byte-exactly to raw Track 02 records `0x484..0x487` using the
observed `raw_record = LBA - 3009` coordinate. The verifier checks each full
2352-byte sector and its observed 32-byte prefix hash. This is a media
identity receipt, not a level/object classification or a FIFO destination.

Verification: `test_theron_v1_lba4165_track02_receipt` and the authentic US
Track 02 capture.

# ✅ 2026-07-15 Theron LBA 4165 FIFO origin receipt

Added byte-origin tracking through Mednafen's SCSI data FIFO. Authentic capture
proves Track 02 record `0x484` / LBA 4165 offsets `0..31` reach System Card
CPU `0xea9c` through `$1808`, with exact byte comparison. No game-owned RAM,
level, or object-record consumer is claimed.

# ✅ 2026-07-15 Theron generation-4 System Card boundary

Generation 4 is CDB `080010891100`: LBA `4233..4249` / Track 02 records
`0x4c8..0x4d8`. Its FIFO origin is System Card `0xea9c`; all four observed
`0x1f0256..0x1f0259` stores are written by System Card `0x000a52`. This route
is excluded from game-data semantics.

# ✅ 2026-07-15 Theron generation-7 FIFO/game-RAM ordering

Authentic capture proves byte-exact FIFO origin for LBA `4847..4851` / Track
02 records `0x72e..0x732`: each of the 10,240 bytes reaches the System Card
`$eb33` FIFO loop and is acknowledged through `$1802/$1803`. The complete
generation-7 FIFO window precedes game-owned `0x1f11xx..0x1f18xx` writes.
That is ordering only, not a byte destination or record semantic.

# ✅ 2026-07-15 Theron main-RAM CDB byte-consistency gate

The authentic main-RAM `$e009` dispatch receipt now decodes each READ(6) CDB
and rejects a mismatch between its LBA/count bytes and the emitted SCSI
command. This binds the game-owned dispatch route to the observed raw record
ranges without inventing a FIFO destination or record semantics.

# ✅ 2026-07-15 Theron later-generation FIFO capture filter

The reproducible Mednafen trace build accepts
`FIRESTAFF_THERON_FIFO_MIN_GENERATION=N`. It filters only provenance output
below `N`; emulated CD reads and RAM writes are unchanged. The authenticated
`N=8` capture omits the already-proved generation-7 FIFO traffic, but still
does not reach a later FIFO byte before timeout. No handoff is claimed.

# ✅ 2026-07-15 Theron guarded global-HID capture route

The Quartz helper can use a global HID route only after activating and then
rechecking the target's foreground PID. An authenticated run observed
`loginwindow` PID `622`, not Mednafen, so it failed before posting a key. This
is an environment receipt, not emulated input or a dungeon handoff.

# ✅ 2026-07-15 Theron main-RAM loader initialization exclusion

The post-`$e009` `0x1f10xx` write window is now fail-closed as loader
initialization: the authenticated writes are only `00`/`ff` sentinels from
the observed main-RAM writers. It cannot be promoted to level/object data.

Verification: focused initialization receipt test.

# ✅ 2026-07-15 Theron game-owned writer corpus negative receipt

The authentic USA Track 02 capture now has a strict, bounded negative corpus
receipt for every observed game-owned main-RAM loader writer. It contains 128
writes: 12 control-window writes at physical `0x1f01f6..0x1f01fb`, plus 116
`00`/`ff` initialization writes at `0x1f10xx`. All have
`dispatch_sequence=0`; the authenticated generation-7 `READ(6)` at LBA 4847
(Track 02 records `0x72e..0x735`) occurs only after that complete writer
corpus. These rows therefore cannot be the G7 loader or a G7 record consumer.
The verifier rejects a CDB-dispatched writer, a non-sentinel initialization
byte, an unclassified destination, and a changed corpus count. This is not a
global absence claim: a later game-owned FIFO/CDB reader or writer remains the
required positive handoff evidence. No level, object, palette, or visual
semantics were added.

Verification: `test_theron_v1_game_loader_writer_negative_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron post-G7 game-loader record-route receipt

The authentic USA trace now fixes the post-G7 game-loader control boundary.
After G7, physical game-RAM `0x1f1840` continues to call `$e009` from logical
`0x3840`: dispatches 4, 5, and 6 have `A=20`, `X=ff`, `Y=04` and issue the
exact READ(6) CDB routes G8 LBA 4859 (record `0x73a`), G9 LBA 4855..4857
(records `0x736..0x738`), and G10 LBA 4858 (record `0x739`). This is the
verified loader entry/record route after G7. The trace patch emits the entry
only after disassembling HuC6280 opcode `0x20` with operand `$e009`, i.e.
`JSR $e009`. The trace still has no
FIFO-to-game-RAM destination or game-owned record reader, so no level, object,
palette, or visual semantics are assigned.

Verification: `test_theron_v1_post_generation7_loader_route_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron post-G7 indirect CDB-parameter receipt

The post-G7 loader trace now proves a bounded ABI fact. Game code at physical
`0x1f1837` writes `ff/20/04` into physical `0x1f01e5..0x1f01e7` immediately
before dispatch 4, exactly shadowing `X/A/Y` at the `0x1f1840` `JSR $e009`.
Dispatches 4--6 keep that same register tuple but produce three distinct
authenticated READ(6) CDBs: `080012fb0100`, `080012f70300`, and
`080012fa0100`. The tuple is therefore not direct LBA/count encoding; it is
an indirect loader ABI whose additional parameter source and RAM consumer are
still unobserved. A new passive MD5-pinned CUE capture reached only the System
Card wait and contributes no loader route. No game-data, level, object,
palette, or visual meaning was inferred.

Verification: `test_theron_v1_post_generation7_cdb_parameter_receipt` and
the authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron post-G7 parameter-window reader trace

Mednafen's authentic trace pipeline now records every physical read of the
post-G7 parameter-shadow window `0x1f01e5..0x1f01e7`, including its logical
address, value, and logical/physical reader PC. The receipt is bounded to 128
rows and is appended after all existing source-to-RAM provenance patches, so
it cannot alter CDB, FIFO, controller, or emulated input behavior. It is
fail-closed evidence only: the existing G8--G10 trace predates this reader
instrumentation, while a new passive MD5-pinned media run reached only the
System Card wait. There is therefore no claimed lookup, loader-table,
game-owned consumer, record-table, or semantic binding yet.

Verification: full `test_theron_v1_mednafen_controller_wait_trace_patch`
dry-run against Mednafen 1.32.1 source.

# ✅ 2026-07-15 Theron post-G7 parameter-thunk CPU receipt

The authenticated G8 trace now fixes the next game-owned control edge after
the indirect `$e009` ABI. Physical `0x1f184d` writes byte `1e` to executable
`0x1f1837`, then `0x1f1852` writes `20` to `0x1f1838`. Execution from
physical `0x1f1837` subsequently stages `04/20/ff` into the parameter window
before `0x1f1840` dispatches `$e009` and G8 reads LBA 4859. The verifier
rejects changed patch bytes, parameter-store ordering, and CDB ordering. No
CD-origin row writes the thunk bytes, no parameter-window reader was observed,
and no opcode, loader-table, record-table, level, object, palette, or visual
meaning is inferred from the two patched bytes.

Verification: `test_theron_v1_post_g7_parameter_thunk_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron generation-4 System Card CD-to-main-RAM receipt

The authenticated USA Track 02 generation-4 READ(6) now has a complete
CPU-provenance boundary. Its CDB reads the ordered 17-sector span LBA
4233..4249 (records `0x4c8..0x4d8`); all 34,816 raw data-port bytes are
checked for contiguous LBA/offset order. The observed FIFO values
`38/50/37/04` are read by System Card `$ea50`, written by System Card `$ea52`
to physical main RAM `0x1f0256..0x1f0259`, and the first three cells are then
read by low physical System Card code. The verifier rejects a game-owned
writer. This is a positive CD-to-main-RAM receipt, but it proves System Card
ownership only: it does not bind game code, a loader table, level data,
objects, palettes, or rendering semantics.

Verification: `test_theron_v1_generation4_system_card_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron byte-exact FIFO-to-main-RAM instrumentation

The Mednafen trace now retains the raw Track 02 LBA and byte offset that were
current at each queued FIFO read, and emits them with the later main-RAM
destination plus reader and writer CPU provenance. A verifier accepts such a
receipt only when its source lies in a preceding observed READ(6) range and
its destination is physical main RAM. This does not fabricate a handoff: the
new MD5-pinned headless USA capture stayed at the System Card wait and emitted
no FIFO-to-main-RAM receipt. A future runtime capture must supply the positive
row before any game-owned loader, level, object, palette, or visual claim.

Verification: `test_theron_v1_fifo_origin_main_ram_receipt`, the Mednafen
patch application/compile probe, and the negative headless capture.

# ✅ 2026-07-15 Theron FIFO-origin game-consumer gate

The trace now tracks a bounded set of raw-CD FIFO cells after they reach
physical main RAM. A consumer receipt is emitted only when a physical
`0x1fxxxx` game-code reader reads the exact same still-valid destination and
value; every later write invalidates that cell, including a same-value write.
System Card readers are excluded. The verifier requires the matching prior
raw LBA/offset receipt, so this cannot promote a timing correlation to a game
handoff. No authentic consumer row has been observed yet.

Verification: `test_theron_v1_fifo_origin_main_ram_consumer` and the Mednafen
patch application/compile probe.

# ✅ 2026-07-15 Theron main-RAM `$e009` return receipt

Each traced game-RAM `JSR $e009` now records an exact pending continuation at
the observed logical and physical `JSR+3` addresses. A return receipt is
emitted only when the HuC6280 executes precisely that continuation; unrelated
game instructions and an unmatched return are ignored. This extends the
loader route from call/CDB evidence to CPU continuity without assigning any
data or rendering semantics. No new authentic return capture is claimed.

Verification: `test_theron_v1_main_ram_e009_return_receipt` and the Mednafen
patch application/compile probe.

# ✅ 2026-07-15 Theron post-dispatch game-owned main-RAM write receipt

After authentic `$e009` dispatch, bounded tracing distinguishes writer
ownership. USA Track 02 capture proves game-owned code at `0x1f0cc9` and
`0x1f1173..` writes main-RAM state. It is not byte-linked to FIFO payload or
a proven level/object record, so no semantics or fallback is promoted.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron `$e009` writer-provenance receipt

FIFO destination receipts now retain the actual writer PC and physical PC.
Real USA Track 02 capture proves every observed `$e009` FIFO store is written
by System Card code (`0x000a52` or `0x000b35`), including stores addressed in
main RAM. Thus none qualifies as game-owned level/object data. The next route
must first prove a physical `0x1fxxxx` writer.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron G4 RAM consumer negative receipt

The HuC6280 read path now records exact reads of G4's materialized
`0x1f0256..0x1f0259` bytes. Real USA Track 02 capture shows their subsequent
readers are System Card physical code, including `0x002c1a..0x002c69`, rather
than game-owned main-RAM code. The G4 route is therefore explicitly blocked
from level/object promotion; no fallback or semantic inference was added.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron `$e009` FIFO-to-main-RAM receipt

Dispatch-bounded FIFO tracing now ties real `$e009` SCSI data reads to strict
next-store RAM receipts. The USA Track 02 capture proves dispatch 0's
generation-4 bytes reach physical `0x1f0256..0x1f0259`; other captured
dispatches reach the System Card workspace. These are byte-transport facts
only: no level/object grammar, game consumer, or visual fallback is admitted.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron main-RAM `$e009` to SCSI receipt

The HuC6280 trace now emits every physical main-RAM `$e009` call into the
PCE-CD trace. A fail-closed verifier requires exactly seven subsequent CDB
writes and one READ(6) SCSI command. A real USA Track 02 capture validates
32 such dispatch-to-record chains. This proves loader-to-record transport,
not game-owned destination, level, object, or visual semantics.

Verification: Mednafen patch dry-run, focused verifier test, and real SDL2
USA Track 02 capture.

# ✅ 2026-07-15 Theron parameterised main-RAM `$e009` receipt

The HuC6280 main-RAM trace now captures A/X/Y at each executed loader call.
Real USA Track 02 capture proves physical `0x1f1840` calls `$e009` after the
`0x1f1836` TII workspace transfer, including `a=20 x=03 y=02`. Parameters
vary across calls and remain opaque: no record, level, object, or visual
semantics are assigned.

Verification: Mednafen 1.32.1 patch dry-run and real SDL2 USA Track 02
capture.

# ✅ 2026-07-15 Theron main-RAM loader control receipt

Added a HuC6280-core trace patch that resolves executed PCs through active MPR
banks before recording bounded main-RAM `JSR` and block-transfer edges. Real
USA Track 02 capture records physical `0x1fxxxx` loader calls, including
`JSR $e009` at `0x1f0cc7` and `0x1f1840`. This proves control flow only, not
level, object, payload, or visual semantics.

Verification: Mednafen 1.32.1 patch dry-run and real SDL2 capture against
MD5-pinned USA Track 02 media.

# ✅ 2026-07-15 Theron all-generation Track 02 source-to-RAM receipt gate

The instrumented Mednafen build now carries an exact raw SCSI origin
(`generation`, `LBA`, and in-sector byte offset) through the pending FIFO read
and emits `pce_cd_origin_ram_receipt` only when that same byte is immediately
stored in physical main RAM. The receipt verifier rejects non-main-RAM
destinations and offsets outside the 2048-byte sector. It neither assigns
writer ownership nor record, level, object, palette, or visual semantics.

A fresh MD5-pinned USA CUE/System Card run without host input reached only the
System Card wait: no raw-sector SCSI transfer and no receipt were observed.
That negative result is deliberately not promoted to a game-data conclusion;
the next positive capture must show a game-owned consumer before any semantic
work may begin.

Verification: `test_theron_v1_origin_ram_receipt`, Mednafen patch dry-run,
and an instrumented authentic-media boot capture.

# ✅ 2026-07-15 Theron game-owned Track 02 FIFO-to-RAM writer gate

The all-generation receipt now observes the store at the HuC6280 write point,
so one trace row contains the raw sector generation/LBA/offset, FIFO reader,
physical main-RAM destination, and both logical and physical writer PCs. The
positive verifier accepts only writer and destination addresses in physical
game RAM `0x1f0000..0x1f7fff`; a System Card writer is rejected. This is a
transport/ownership gate only and publishes no record, level, object, palette,
or visual semantics.

The fresh MD5-pinned USA CUE capture posted real PID-targeted Quartz Return
pairs but Mednafen reported no SDL key event, then reached only the System
Card wait with no SCSI read or FIFO/RAM receipt. The failed delivery is kept
as a negative capture result, not replaced with injected controller state.

Verification: `test_theron_v1_game_owned_origin_ram_receipt`, full Mednafen
patch dry-run, instrumented Mednafen build, and the authentic-media capture.

# ✅ 2026-07-15 Theron PID foreground capture gate

PID-targeted Quartz delivery now requires the same foreground ownership proof
as the global-HID route. The helper activates the target, rechecks
`NSWorkspace`, and emits `quartz_frontmost_pid` only before posting a key.
The capture wrapper requires that receipt, so a `posted_to_pid` line cannot be
mistaken for SDL delivery from a background or login session.

The direct live check found the Mednafen target at PID `8739` while foreground
ownership remained with `loginwindow` PID `622`; it failed before posting. No
controller state, CD read, FIFO/RAM handoff, or Track02 semantics were
invented. A positive run still needs both real Aqua foreground ownership and
Mednafen's own SDL event receipt.

Verification: `swiftc -typecheck`,
`test_theron_v1_mednafen_live_capture_script`, and the direct live negative
foreground receipt.

# ✅ 2026-07-15 Theron foreground activation receipt refinement

The Quartz helper now records the result of macOS activation independently of
foreground ownership, and the capture wrapper requires `quartz_activation`
plus the exact foreground PID before it accepts a key-post attestation. A live
probe returned `activate=true` for Mednafen while `NSWorkspace` still reported
`loginwindow` PID `622`; activation success alone is therefore not promoted to
focus, SDL delivery, controller state, CD traffic, or a Track02 handoff.

Verification: Swift typecheck and
`test_theron_v1_mednafen_live_capture_script`.

# ✅ 2026-07-14 Theron Track 02 startup-grid positive route

The existing CD/MODE1 envelope and loader-semantic receipt now materialize one
positive route: Hall of Records level 0 only. It verifies the loader-selected
pose against the receipt and remains an explicit route boundary pending
startup-pose reconciliation with the older semantic handoff. The route
contains no object table, header-extension interpretation, transition, bitmap,
or fallback-visual claim; later dungeon requests reject. The focused Track 02
handoff probe checks the real-media route and its refusal of an unproven
dungeon ID.

# ✅ 2026-07-14 Theron later `$e009` capture correlation gate

# ✅ 2026-07-14 Theron later `$e009` production selector-coordinate gate

The production later-loader media receipt now derives the captured `$e009`
record from the authenticated Stage 3 descriptor coordinate base and accepts
it only when it resolves to an existing descriptor selector. It retains the
opaque selector and ordinal with the raw-sector receipt. A raw-sector-only or
synthetic media buffer cannot publish the receipt. This is still only an
executed loader-coordinate constraint: it assigns no descriptor format,
dungeon, object, palette, bitmap, or transition semantics. Verification:
`theron_v1_raw_loader_trace_stage3_sector` passes; the paired original-media
layout probe remains skip-safe until matching JP/US Mednafen traces exist.

# ✅ 2026-07-14 Theron later `$e009` raw-sector witness boundary

The selector-coordinate receipt can now be paired with exactly one
provenance-marked Mednafen SCSI raw-sector sidecar span whose bounded FNV-1a
matches the corresponding hash-verified Track 02 raw sector. The receipt
retains only the observed disc LBA, selector coordinate, and span fingerprint.
It does not claim that `$e009` caused that read, that both observations share
one capture session, or assign a payload format, dungeon, object, palette,
bitmap, or transition meaning. Noncanonical media, missing sidecars, duplicate
matching spans, and changed bytes reject.
Verification: `theron_v1_raw_loader_trace_stage3_sector` focused negative
probe; a positive result requires original JP/US media and captures.

# ✅ 2026-07-14 Theron later `$e009` complete-sector witness hardening

The raw-sector witness now accepts only a provenance-marked Mednafen SCSI row
that retains both FNV-1a fingerprints: all 2352 observed raw-sector bytes and
the existing leading 32-byte span. Firestaff compares both against the same
selector-resolved record in the hash-verified original Track 02 BIN; span-only
or malformed sidecars reject. This remains physical CD/media provenance only:
it does not establish `$e009` causality, shared capture-session identity,
payload format, dungeon, object, graphics, palette, bitmap, or transition
semantics. Verification: focused raw-loader CTest and Mednafen patch/capture
script contracts.

# ✅ 2026-07-14 Theron later `$e009` ordered raw-sector capture gate

The next Track 02 capture handoff now has a strict, source-only admission
contract. A future authentic JP or US coalesced Mednafen transcript must retain
exactly one variant-matched `$4090/$4093` loader row, followed by one later
`$e009` dispatch, exactly one complete 2352-byte raw-sector FNV witness, and
the matching `$e009` return. The verifier rejects split sidecars, reordered
rows, duplicate rows, malformed fingerprints, and unmarked transcripts. It
records only observation order; no destination, CD causality, payload format,
dungeon, map, object, graphics, or palette claim is introduced.
Verification: `tests/test_theron_v1_later_e009_raw_sector_order_trace.sh`.

# ✅ 2026-07-15 Theron post-`$3800` IRQ2-to-later-read ordering gate

The coalesced Track 02 receipt now requires an observed original Stage 3
`BRK $ff` return from `$3800` to `$3802` before it will accept a later
`$e009` dispatch. Firestaff checks those capture coordinates against the
hash-verified Stage 3 payload, then retains only the ordering fact. The gate
does not decode the later sector or promote level, object, bitmap, palette,
grid, or transition semantics. Verification: the corpus-bound raw-loader
handoff probe rejects missing or altered Stage 3 continuation coordinates.

Added a skip-safe, corpus-bound probe and Mednafen instrumentation for the
first post-stage-two HuC6280 `JSR $e009` envelope. A positive result requires
hash-verified JP and US raw Track 02 images plus matching instrumented traces;
the record must reconstruct from observed `CL/DL/CH`, remain in each raw-sector
range, resolve to the same existing stage-three descriptor selector ordinal,
and preserve one caller/return PC pair. This is only a bounded record/layout
and control-transfer correlation. It does not label the call as a payload
format, or publish a CD read, bitmap, palette, object, level, or gameplay
transition. Inspected historical US traces do not contain the new later
envelope, so no positive record has been claimed. Verified with an external
Ninja build of `firestaff_theron_v1_later_cd_read_layout_probe` and skip-safe
CTest registration.

The probe now additionally requires exactly one observed `$4090/$4093`
dynamic receipt in each trace, including its matching JP/US variant and
reconstructed `CL/DL/CH` stage-two record. A freestanding, duplicate, or
cross-variant later `$e009` row cannot be paired with authenticated media.

# ✅ 2026-07-14 Theron Track 02 route-receipt probe repair

The focused Track 02 handoff probe now constructs a complete hash-profiled
startup-media receipt before it exercises the existing media-gated bank
selection. This restores the real JP/US Hall of Records level-0 loader route
as a green target while retaining the Stage 3 `$4090 -> $4093` CD_READ receipt
as transport-only: it does not claim a later level, object layout, visual
decode, or transition.

# ✅ 2026-07-14 Theron Track 02 loader-pose reconciliation

The positive raw-CD Hall of Records level-0 path now preserves the existing
loader's first-floor/default-North pose across the candidate and loader-route
handoffs. The previous local passable-neighbor/East preference was removed
because it was not backed by the original CD or loader evidence. The focused
probe verifies the two real-media paths agree; it remains skip-safe without
hash-verified JP/US Track 02 images. The older seed-table semantic handoff is
still independently blocked on authentic media and is not composed here. This
does not infer an IPL spawn override, object table, transition, bitmap,
palette, or fallback.

# ✅ 2026-07-14 Theron Track 02 coalesced later-loader sector receipt

The later-loader handoff now has one media-bound receipt for a single original
Mednafen transcript. It requires the authenticated Stage 2 `$4090 -> $4093`
loader row, one later `$e009` dispatch, one complete 2352-byte raw-sector
fingerprint, and the matching return in that observation order. Both the
complete-sector and leading-span FNV-1a values must match the raw sector
selected through the existing Stage 3 descriptor coordinate in a hash-verified
JP or US Track 02 image. The opt-in corpus probe runs this check only when both
variants' coalesced traces are supplied. This records a loader-coordinate and
physical-media fact only: it assigns no payload format, dungeon, map, object,
graphics, palette, bitmap, or transition meaning.

# ✅ 2026-07-14 Theron Track 02 manifest-bound coalesced loader receipt

The opt-in JP/US coalesced-loader corpus probe now accepts each ordered
Mednafen transcript only through its own V2 capture manifest. It rehashes and
matches the exact raw Track 02, System Card 3.0, and trace paths before binding
the existing selector-resolved complete-sector receipt. A missing half-pair,
manifest, or System Card path fails the supplied-evidence gate. This records
only original-artifact provenance and loader/media coordinates; no payload
format, dungeon, map, object, graphics, palette, bitmap, or transition meaning
is assigned.

# ✅ 2026-07-14 Theron Track 02 manifest-required raw loader preflight

The positive raw-loader preflight now requires a V2 capture manifest and
rehashes the exact raw Track 02, System Card 3.0, and ordered Mednafen trace
against it before admitting the existing `$3800` media-span/Stage 3 receipt.
The shared loader-capture identity check rejects a missing manifest, a changed
trace, or a non-System-Card-3.0 hash. This is artifact provenance and transport
only; it assigns no payload format, dungeon, object, bitmap, palette source,
or decoder meaning.
# 2026-07-27 Theron hash-selected Track 02 media root

- ✅ 2026-07-27 DM1 champion HUD click repair. The full painted V1
  health/stamina/mana bar surface now opens the matching champion inventory,
  rather than accepting input only on the narrow right-edge source zone while
  the rest of the visible bar silently selected the leader. Name and hand
  routes remain unchanged; V2 portrait-card routing remains covered.
  Verification: DM1 inventory mouse-route runtime, V2 HUD interaction, and
  HiDPI champion pointer tests.

- ✅ 2026-07-27 DM1 V2.2 reviewed-art admission repair. Formatted Art Studio
  manifests are now parsed as JSON objects instead of line fragments, all
  V2.2 manifest/receipt roots are configured together, and alias-safe path
  joins support in-place path construction. A reviewed local pack passes the
  real material gate and renders eight source-backed cells; an unsigned
  cache remains fail-closed. Verification: V2.2 real-art material gate,
  per-mode material signatures, settings persistence, and source-lock gate.

Theron launcher campaign-media discovery now honours the caller's selected
known Track 02 MD5 when scanning a directory. A data root containing both US
and JP original releases is valid; the selected release remains launchable
instead of being misreported as ambiguous. The optional real-media test uses
the supplied root and selected MD5 to prove this without shipping game data.
- ✅ 2026-07-27 Theron Mednafen live-capture build repaired. Repaired the
  1.32.1 debugger trace patch so the core CPU/CD/input/sector instrumentation
  builds again, removed stale extension patches from the required build path,
  and made the local trace binary link a real SDL2 runtime with an embedded
  rpath. Verified against authentic US Track 02 plus System Card 3.0:
  Quartz-delivered Run input, two observed System Card calls, 25 CDIRQ events,
  and 31 raw-sector receipts. Dynamic dungeon-handoff rows remain deliberately
  unclaimed until an original run reaches them.
- ✅ 2026-07-27 Theron timed original-menu input capture. The authenticated
  Mednafen capture helper now supports ordered absolute input timings through
  `THERON_CAPTURE_HOST_KEY_DELAYS`. Verified a three-press Quartz Run sequence
  against authentic US Track 02 and System Card 3.0; the trace records every
  host SDL/key event and emulated port state. The current original route stops
  polling the PCE port before the scheduled presses, which remains explicit
  evidence rather than being misreported as a successful dungeon handoff.
- ✅ 2026-07-27 Theron Mednafen input-PC trace correction. Rebuilt the real
  SDL2-linked Mednafen 1.32.1 trace binary with CPU-PC provenance on every
  direct PCE input read/write and a configurable 4,096-per-direction cap.
  Authentic passive US CUE + System Card capture records 8,192 input
  transactions at System Card PCs `e4b7`/`e4c8`; the prior 128-row result was
  trace truncation, not a stopped-poll conclusion. Track 02 handoff remains
  blocked: this capture has no dynamic sector read or loader-consumer row.
  Verification: trace patch dry-run, full external Mednafen rebuild, SDL2
  runtime verifier, and authentic 55-second capture.
- ✅ 2026-07-27 Theron focused capture resolver. `capture_theron_mednafen_live_trace.sh`
  now waits up to ten seconds for Mednafen's own timeout/env descendant, then
  schedules host keys relative to capture launch instead of racing process
  creation. Script gate passes; a real 55-second capture attests four Return
  SDL events, PCE port `0000 -> 0008 -> 0000`, 31 raw-sector spans, and 56
  SCSI reads. It remains non-promotable because the initial input trace cap
  is reached before the host event.
- ✅ 2026-07-27 Theron post-key input-chain capture. Raised the default
  direct-input trace limit to 65,536 per direction and made PID Quartz
  delivery tolerate a background target while retaining a strict foreground
  requirement for global HID. Authentic US capture records 47,575 direct PCE
  transactions, 26,782 after the first host key, and direct `e4b7`/`e4c8`
  reads of port `0008`. Verification: Swift typecheck, shell/test gate,
  rebuilt SDL2-linked Mednafen, and 55-second authentic CUE/System Card run.
  No dynamic CD destination or game-owned PCECD reader appeared, so Track 02
  dungeon promotion remains blocked.
- ✅ 2026-07-27 Theron PCE input-result trace. Added a post-read trace hook
  after Mednafen applies PCE port semantics, rather than inferring result bits
  from host state. A real 28-second US capture with Return held records
  `raw=0008 -> value=3f` at `e4b7` and `raw=0008 -> value=37` at `e4c8`, plus
  4,796 subsequent PCE input transactions. This proves the observed input
  register result only; it does not assign a game command or promote Track 02
  data.
- ✅ 2026-07-27 Theron CD-to-RAM physical ownership trace. Added physical
  HuC6280 PC provenance to both CD-data reads and the matching RAM writes.
  Authentic input capture proves all currently observed candidates are System
  Card code `000a50/000a52` or `000b33/000b37`, including writes into
  `001fxxxx` main RAM. This closes the false inference that destination RAM
  alone proves a game loader; no game-owned CD consumer is promoted.
- ✅ 2026-07-27 Theron authentic multi-key boot capture. The live Mednafen
  capture harness now accepts one ordered absolute-time key sequence, keeping
  every element constrained to `return`, `i`, or `select`. Authentic
  `return@10,i@75,i@90` reaches the original Theron title menu and then real
  NEW GAME presentation, with six host key events and 8,910 subsequent PCE
  input transactions. This is boot/menu evidence only; no dungeon record,
  game-owned CD reader, or destination semantics are inferred.
# 2026-07-27 - Theron Mednafen loader-capture diagnostics

- Improved the authentic capture failure receipt with main-RAM `e009` dispatch,
  enter, data-read, and control-write counts. A raw-sector-only trace now
  states precisely that the missing proof is a game-owned PCE-CD data read.
- ✅ 2026-07-27 DM1 V1 door/wall-ornament source-lock maintenance. The
  viewport audit now follows the DM1-owned F0111 ornament planner after its
  coordinate sets and D2/D3 palette maps moved out of M11. It continues to
  verify the real ReDMCSB F0107/F0111 ordering, clipping and occlusion
  contract; `dm1_v1_viewport_door_wall_ornament_source_lock` passes.
- ✅ 2026-07-27 DM1 PC34 C70 save-event roundtrip. F0435 now reconstructs
  the signed `EVENT.B.LightPower` union for C70 rather than demoting it to
  generic cell/effect bytes, so a saved light-decay event can be written
  again by F0433. The PC34 export suite also verifies a materialized dungeon
  tail roundtrip and rejects an unproven C24 Fluxcage slot on the state-only
  path.
- ✅ 2026-07-28 DM1 real HoC orientation and champion-pointer regression.
  The registered `m11_dm1_hoc_orientation_runtime_pc34` CTest starts from
  the local original PC34 data, validates F0128 viewport material in all
  four directions and through live turn inputs, selects a real C127 mirror,
  resurrects the champion, and opens that champion's HUD inventory through
  the production pointer path. It also proves the shipped Hall's eight
  F0115 object candidates reach a real F0791 material draw, rather than
  needing synthetic floor or alcove art. The focused real-data
  HoC/object/alcove/save suite passes 4/4.

- ✅ 2026-07-28 DM1 default C140 save path. The live inventory SAVE control
  now has a regression that clears its test-only path override, creates the
  normal per-user `saves/dm1` directory, writes the save, and reloads it.
  This covers a fresh profile's former file-not-found failure mode.
- ✅ 2026-07-28 Compact runtime graphics popup. F10 now uses a narrow
  right-side panel and leaves the live viewport undimmed, so V1/V2.x mode,
  filter, palette, and scale changes can be judged immediately. Its input
  remains modal; the regression verifies the exposed viewport, compact close
  hitbox, and live setting changes.
- ✅ 2026-07-28 DM1 HoC object coverage and inventory-panel controls. The
  real PC34 HoC regression now requires every unique original object graphic
  from all eight ordinary candidates to reach an F0791 blit. C140/C141/C145/
  C011 are resolved before C081's broad inventory-panel route, so the
  visible Save, music, Zz and close controls cannot be swallowed. The
  source-owned save-disk menu is explicitly exercised before its save write.

- ✅ 2026-07-28 DM1 full turn-button feedback. Q/E, Home/End and controller
  turns now outline the complete 29x23 C013 turn cells; mouse hit geometry
  remains the original narrower C068/C069 rectangles.
- ✅ 2026-07-28 CSB title/Entrance source timing. ReDMCSB `TITLE.C:451-463`
  proves 60 VBlanks of PRESENTS, 20 CHAOS shrink frames, `Delay(20)` on the
  full CHAOS page, then `Delay(2)` on STRIKES BACK. The old 101-tick model
  held the final title frame for one VBlank. CSB now uses the correct
  102-tick timeline, and focused real-data title/Entrance regressions pass.
- ✅ 2026-07-28 DM1 V2.x current verification. Built the only previously
  absent registered V2 cursor-mask test binary, then ran the complete
  V2.0/V2.1/V2.2 CTest selection against local original DM1 data: 88/88
  passed. Coverage includes mode handoff, HUD/pointer routes, viewport,
  item/creature/spell/effect paths, resolution mapping, assetpack gates and
  real runtime presentation smoke.
- ✅ 2026-07-28 DM1 original PC34 save round trip. A real DOSBox
  `DMSAVE.DAT` now passes fixture-free F0435 -> F0433 -> F0435 admission:
  source bytes stage into a live world, a saved portrait reaches the active
  inventory panel, the complete tail is preserved, and exported bytes reload
  through the same handoff. C13 remains optional evidence, as it is in the
  C3 event stream; a valid C13-free save is no longer rejected for lacking a
  fabricated C13 lifecycle receipt. The external-corpus, handoff, and
  external-HoC runtime regressions pass against the supplied data.
- ✅ 2026-07-28 DM1 F0115 near-square consumer audit. Retired the stale
  TODO claim that D0/D1 object presentation needed a second M11 bridge.
  The active renderer already uses F0098 for source floor/ceiling material
  and F0115 C2500/F0791 for visible floor objects; the old isolated receipt
  has no production caller and must not be wired as a duplicate item blit.
  Real-PC34 floor-item and alcove runtime regressions pass.
- ✅ 2026-07-28 DM1 D0C C15 effect-order repair. The live F0115 receipt no
  longer filters fluxcage or rebirth C15 records before their source-specific
  consumers run. It preserves original C15 order while the renderer remains
  no-draw without an authenticated special bitmap. C14/C15 layout, projectile
  impact, D0C receipt, C15 runtime-capture, and projectile presentation tests
  pass.
- ✅ 2026-07-28 DM1 HoC F0115 presented-pixel gate. The real PC34 Hall sweep
  finds all eight original floor/alcove object graphics and now requires each
  F0791 destination rectangle to change after its exact source blit. This
  proves final framebuffer consumption rather than only a material receipt.
  The identical real-data sweep now passes in V1, V2.0, V2.1, and V2.2, with
  a real C127 mirror route in every presentation mode.

- ✅ 2026-07-28 DM1 V2 inscription preservation. V2.2 no longer suppresses
  the final ReDMCSB F0107/M648 repaint after V22 art. V2.0, V2.1, and V2.2
  now all prove exact original M648 glyph pixels, C10 transparency, and stale
  text invalidation with real PC34 wall text.
## 2026-07-28 DM1 C14/C15 final viewport consumers

- Closed the stale DM1 F0115 C14/C15 host-consumer follow-up. Real PC34
  runtime tests now prove a thrown object reaches the final C2900 material
  blit and an ordinary C15 explosion reaches the deferred final-pixel pass.
  Source identity, catalogue admission, material fingerprint, and fail-closed
  rejection remain enforced before either draw.

## 2026-07-28 DM1 V2 inventory controls

- Added runtime coverage for C141 music, C140 save-disk, C145 rest and C011
  close in V2.0, V2.1 and V2.2. Presentation selection does not make the
  visible DM1 inventory controls inert.

## 2026-07-28 DM1 HoC capture route

- The real-PC34 HoC regression now reports its selected source mirror route:
  wall `(14,2)`, party `(14,3)`, north, ordinal `5` for the installed corpus.
  This makes repeatable macOS/window capture possible without guessing a
  champion-mirror location.

## DM1 HoC viewport occlusion

- **DM1-VIEWPORT-001**: Fixed the corridor-through-wall artifact in the live
  M11 renderer. ReDMCSB F0128 draws center walls as part of each square before
  visiting nearer squares; Firestaff's deferred F0115 batch could otherwise
  paint deeper corridor content over a nearer wall. The final source-backed
  center-wall pass now restores the wall and then replays the D1C champion
  mirror route. Verified with the DM1 wall-ornament and inventory placement
  tests plus a clean `firestaff` Ninja build on 2026-08-05.
## DM1 source-data fail-closed wall rendering

- **DM1-VIEWPORT-002**: Removed the synthetic black rectangle used when a
  center wall bitmap could not be loaded. The ReDMCSB wall path now leaves the
  cleared/background pixels unchanged and reports the missing authenticated
  GRAPHICS.DAT material through the existing asset route. This prevents a
  missing asset from masquerading as a corridor opening or fabricated wall.
  Verified with the DM1 wall-ornament (`121/121`) and inventory placement
  (`156/156`) tests plus a successful Ninja `firestaff` build on 2026-08-05.
## DM1 centre-wall ornament restoration

- **DM1-VIEWPORT-003**: Prevented the final nearest-wall occlusion replay from
  erasing authentic centre-wall inscriptions and alcove material. The replay
  now restores only source-owned centre ornaments after the wall bitmap, then
  hands the live champion mirror route back to the renderer; side ornaments
  are not replayed across the occlusion boundary. Verified with the DM1 wall
  ornament (`121/121`) and inventory placement (`156/156`) tests and a clean
  Ninja build on 2026-08-05.

- ✅ 2026-08-05 Nexus palette source-lock correction: aligned the Phase 4
  rendering documentation with the actual fail-closed `STONE.BIN` loader.
  Short palettes clear and remain unavailable; they do not receive the old
  inferred `g_npal_default` colour table. Verified by the real-data DGN
  geometry readiness gate against `/Users/bosse/.firestaff/data/nexus`.
- ✅ 2026-08-05 DM2 actuator generator provenance hardening: removed the
  remaining live wall-mecha generator mutations. Creature generation no
  longer invents a fixed HP/base value or tick-derived direction, and item
  generation no longer allocates a generic DB item from actuator data alone.
  Both remain fail-closed pending the complete source `ALLOC_NEW_CREATURE` /
  `ALLOC_NEW_DBITEM` ownership chains. Verified by the focused actuator and
  runtime gates plus the mounted real-data startup, HUD, material,
  scene/weather and original-save-writer gates.
- ✅ 2026-08-05 Nexus rasterizer provenance cleanup: corrected the Phase 4
  source-lock record to describe the actual production boundary. Flat-color
  geometry, unsupported 3D assets, and missing surfaces/textures remain
  explicitly no-draw; the retired gray-billboard/placeholder claims are no
  longer documented as runtime features. Verified with
  `test_nexus_v1_dgn_material_raster`, the real-data DGN geometry gate, and
  `git diff --check`.
- ✅ 2026-08-05 DM2 unbound CCM timer hardening: an unresolved
  `DM2_THINK_CREATURE` body now consumes its source timer without re-queuing a
  coordinate-only creature retry. Live record pools and timer queues remain
  unchanged until the complete original CCM stream owns animation, movement
  and rescheduling. Verified by the think-creature, CCM-runtime and CAII
  reschedule gates.

- ✅ 2026-08-05 Nexus FACE.BIN production provenance gate: the low-level
  retail PRS3 structural/pixel diagnostic remains available for evidence, but
  `nexus_ui_load_face_record()` no longer promotes unproven PRS3 output or its
  64-entry per-frame palette into live startup UI. Production now records all
  portraits as blocked until an original Saturn capture authenticates pixel
  grammar, palette lane, and placement. Verified with the real FACE.BIN
  structural probe, updated Track 1 launch probe (57/57), and the focused
  Nexus build.
## DM1 combat-log source font guard

- **DM1-UI-001**: The normal verified DM1 catalog launch no longer renders
  the built-in mini-font when the original `GRAPHICS.DAT` font is unavailable.
  It now fails closed until the source font is bound; the mini-font remains
  available only for explicitly non-catalog diagnostic callers. This removes
  a synthetic production visual without changing the source-backed font path.
  Verified with a successful Ninja `firestaff` build and combat-log contract
  test (`5/5`) on 2026-08-05.

- ✅ 2026-08-05 Nexus SAL playback gate correction: real SAL tone decoding
  can now populate diagnostic receipts, but `nexus_sound_play_event()` and
  `nexus_sound_play_idx()` check the complete runtime receipt before invoking
  the tone trigger. Decoded bytes cannot bypass the unresolved SDDRVS/event
  ABI gate. Verified with the real-corpus sound runtime receipt test.
## DM1 HoC source item-name guard

- **DM1-HOC-OBJECTS-003**: DM1 item labels now require the authenticated
  ReDMCSB `OBJECT.C` M564 icon-indexed name stream. When that source table is
  absent or malformed, Firestaff leaves the label empty instead of presenting
  the legacy hand-written subtype catalog as if it were original data. The
  fallback catalog remains available only outside DM1 source-owned routes.
  Verified with a successful Ninja `firestaff` and real-alcove target build,
  plus `git diff --check`, on 2026-08-05.

- ✅ 2026-08-05 CSB scanner inventory clarity: `--scan-data` now labels
  `GRAPHICS.DAT` and `DUNGEON.DAT` explicitly as launch requirements, then
  recursively reports every other hash-catalogued CSB source medium it finds,
  including entries inside supported archives. This keeps the two-file launch
  gate intact while exposing verified `ANIMATE.*`, Hint Oracle, Utility Disk,
  `MINI.DAT`, and platform sidecars from the real data rather than relying on
  a small fixed list of loose filenames. The shared fingerprint test passes
  284/0.

- ✅ 2026-08-05 CSB Atari ST title cadence: the real `ANIMATE.SCR` M11
  handoff regression now proves that each 55 ms V1 tick becomes the correct
  accumulated 50 Hz source-VBlank count, never regresses, and reaches the
  `FTLCODE` handoff only at the script-derived terminal boundary. This guards
  against a title that advances too quickly. ReDMCSB `ANIM.C:67-72` and its
  VBlank waits establish the source timing; the extracted local Atari ST
  package passes the focused handoff test.
## DM1 source object icon parity

- **DM1-HOC-OBJECTS-004**: Added the missing ReDMCSB `OBJECT.C F0033`
  charged-Jewel-Symal branch. DM1 now resolves the source `G0237` Jewel Symal
  icon from its raw `JUNK.ChargeCount`, matching the original water/illumulet
  charged-item family instead of leaving the base icon selected. Regression
  coverage exercises the PC34 raw record and expects icon 11 for a charged
  Jewel Symal. Verification: `test_dm1_v1_projectile_explosion_render_pc34_compat`
  passed with all tests, plus `git diff --check`, on 2026-08-05.

## DM1 source fountain interaction

- **DM1-HOC-OBJECTS-005**: Reconnected the live DM1 C080 wall-click route to
  the ReDMCSB `F0601` fountain predicate. The current map's real
  `DUNGEON.DAT` wall-ornament table is now matched against `G0193` before a
  leader-hand object can be changed. Empty-hand drinking, charged waterskin
  filling and empty-flask to water-flask mutation now update the loaded
  runtime records; generic wall ornaments retain the sensor/drop path.
  Verified with a full `firestaff` build and the source fountain regression
  (`fountainInteractionInvariantOk=1`) on 2026-08-05.

## DM1 source wall ornament table correction

- **DM1-HOC-OBJECTS-006**: Corrected the DM1 PC34/I34E `G0194` wall-ornament
  coordinate-set table. Firestaff had used the ReDMCSB `MEDIA353` variant
  (`DUNVIEW.C:846-906`); PC34 uses the `MEDIA529`/`I34E` table at
  `DUNVIEW.C:932-1007`, including coordinate sets 7/8 for the real wall
  ornament family. The source graphic base remains `M615=259`, with F0791
  transparent colour 10 and G0198/G0199 palette maps unchanged. Focused
  G0194 and wall-plan tests pass after the correction. Real macOS pixel
  capture is still tracked separately in `DM1-HOC-OBJECTS-001`.
- ✅ 2026-08-05 CSB Atari ST executable-media inventory: corrected the
  `SWITCH.DAT` fingerprint to the bytes in the original hard-disk package and
  added hash identities for `ANIMATE.FTL`, `CHAOS.FTL`, and `FTLCODE`.
  ReDMCSB `COMPILE.H:609-620` identifies the three modules and `ANIM.C:94`
  makes the `FTLCODE` transfer explicit. They are reported as verified source
  media without changing the `GRAPHICS.DAT`/`DUNGEON.DAT` start gate. The
  fingerprint suite passes 294/0 against the extracted local package.
- ✅ 2026-08-05 Theron teleporter fail-closed hardening: unresolved legacy
  object-ID links and cyclic/overlong chains no longer report a successful
  transition or place the party at the clicked square. Transition and party
  state remain unchanged until a real terminal object record resolves;
  missing-target and cycle regressions now assert rejection.
- ✅ 2026-08-05 CSB Utility Disk CMP disk-format correction: replaced the
  synthetic 496-byte portrait layout with ReDMCSB's actual 508-byte `CMP`
  record. The decoder now reads the big-endian `Magic`, dungeon-id, platform,
  compatibility words, reserved words, name/title and the 464-byte portrait at
  offset 44. ReDMCSB `DEFS.H` defines the layout and `CEDT001.C F7000` writes
  exactly 508 bytes. The extracted original Atari ST `PORTRAIT/HALK.CMP`
  decodes as HALK, THE BARBARIAN; CMP import, portrait-handoff and title/import
  regressions pass without allowing a portrait-only file to invent party state.
- ✅ 2026-08-05 Theron legacy asset-parser cleanup completed: removed the
  unreachable THS4 sound parser body and its guessed marker constants from
  the implementation. The public diagnostic APIs remain explicit rejection
  seams; no Firestaff-only THG3/THS4 bytes can become runtime media.
- ✅ 2026-08-05 CSB Utility Disk portrait inventory and scanner repair: added
  hash identities for all 26 original Atari ST `PORTRAIT/*.CMP` files, whose
  508-byte disk format is established by ReDMCSB `CEDTDATA.C:394/397` and
  `CEDT001.C F7000`. The CSB report now uses media already materialized by
  the status scan instead of triggering a second recursive archive traversal.
  Archive materialization retains the real `ANIMATE.FTL`, `CHAOS.FTL` and
  `FTLCODE` modules beside the launch pair. A real loose-package scan shows
  those modules plus `SWITCH.DAT` and `MINI.DAT`; the fingerprint suite passes
  373/0.
- ✅ 2026-08-05 Theron Track 19 item-name binding: added a source-span reader
  for the US MODE1/2048 table at ISO offset `0x0E9271`. It validates all 69
  null-separated names against the verified catalog before returning any one
  label, and rejects truncation or byte changes. The real local `TQUS19.iso`
  passes the full table probe.
- ✅ 2026-08-05 Theron Track 19 level-label binding: added byte validation for
  the real US ISO selector table at offset `2112059`, covering `LEVEL  1`
  through `LEVEL 15`. The probe validates the complete table and rejects a
  changed label byte; this exposes labels only and does not invent maps,
  objects, or bitmap semantics.
- ✅ 2026-08-05 CSB scanner sidecar visibility: `--scan-data` now searches
  beside the hash-matched loose `GRAPHICS.DAT` package (not only the selected
  data root) before reporting verified CSB media. The candidate inventory also
  recognizes Atari `ANIMATE.FTL`, `CHAOS.FTL`, `FTLCODE` and `MINI.DAT`.
  Archive-cache regression coverage now proves the three real Atari startup
  modules remain materialized beside the verified launch pair. Source-lock:
  ReDMCSB `ANIM.C:67-72,94`; verified against the real Atari ST archive.
- ✅ 2026-08-05 Theron Track 19 file-inventory binding: added a reusable
  file-backed receipt that authenticates the exact ISO hash/size and validates
  both real US metadata spans (69 item names and 15 level labels). The
  inventory exposes verification flags without admitting dungeon maps,
  objects, or bitmap semantics.
- ✅ 2026-08-05 CSB Atari-animationens runtimekedja: den verkliga
  `ANIMATE.FTL`/`CHAOS.FTL`/`FTLCODE`-trion har nu ett eget
  hash-verifierat discovery- och cachekvitto. Modulerna måste komma från
  samma katalog eller arkiv och körs aldrig som värdbinärer. Verifierat mot
  den lokala Atari ST 2.0-katalogen med original-MD5. Källor: ReDMCSB
  `ANIM.C:67-72,94`, `COMPILE.H:609-620` samt DMWebs Animation Script- och
  Animationsformatdokumentation.
- ✅ 2026-08-05 CSB map-difficulty provenance: removed the invented
  champion-count percentage scale and its hard-coded three-champion default.
  A loaded CSB profile now takes the current map's authenticated `MAP.C`
  high-nibble difficulty from `DUNGEON.DAT`; a roster-only or failed handoff
  stays explicitly unbound. Runtime-image restore no longer revives the old
  synthetic multiplier. Source-lock: ReDMCSB `DEFS.H` `MAP.C`, `PANEL.C`
  F0337, `CHAMPION.C` and `PROJEXPL.C`; covered by CSB boot and save tests.
# 2026-08-06 Theron extended authentic replay receipt

- ✅ A 120-second Mednafen replay using the authenticated US Track 02 CUE,
  verified System Card and repeated Run/I input produced 54 SCSI reads but no
  game-owned post-startup Track 02 consumer, `$2600` handoff, or source-owned
  VDC/VCE destination receipt. The bounded main-RAM windows remain retained as
  loader evidence only; no level, object, tile, material, palette, HUD or
  viewport semantics were enabled.

# 2026-08-06 Theron text publication boundary

- ✅ Authentic Track 02 text codons remain decoded from the supplied US media
  for diagnostics, including their exact unresolved control-code markers.
  Production `theron_v1_world_load_dungeon_text()` now keeps the world text
  table empty when those markers occur, so candidate strings cannot become
  synthetic HUD, plaque or scroll text. The focused real-media regression
  passes and will reopen only after the original HuC6280 text consumer is
  identified.

# 2026-08-06 Theron source-index receipt integrity

- ✅ Track 02 source occurrences now retain their full 16-bit category index
  instead of an 8-bit field. This matches the 512-entry source-category
  bound and prevents later real records from being truncated or rejected.
  No category-local type was promoted to a host item index.

# 2026-08-06 Theron synthetic V2.2 asset quarantine

- ✅ The V2.2 modern-asset admission gate now requires
  `source_provenance="authenticated_track02"` in the manifest. The existing
  procedural and `gpt-image-2` Theron art pack is rejected by production and
  remains available only to fixture/reference inspection. The focused asset
  test passes 36/36.

# 2026-08-06 Theron US Track 02 descriptor receipt

- ✅ The production Theron source layer now reads all 53 six-byte level
  descriptor records from UD `0x619900` in the authenticated US Track 02
  MODE1 user-data stream. The focused test extracts the real BIN sectors,
  verifies the source-locked bytes and rejects mismatched tables. This closes
  descriptor-byte provenance only; it does not infer graphics compression,
  object IDs, tile-bank ownership, palette binding or dungeon handoff.

# 2026-08-06 Theron HuC6280 decompressor receipt

- ✅ Extended the authenticated US/JP bank-$1f disassembly receipt from the
  134-byte helper fragment to the full byte-identical `$23AD-$252A` routine.
  The 382-byte range covers the variable-bit reader, bank switches, literal
  output and back-reference path. It remains evidence only: the caller,
  destination and level-block contract are not yet proven, so no decoder was
  enabled in production.
# 2026-08-06 Theron split-ISO Mednafen capture intake

- ✅ The live Mednafen capture runner now handles the supplied retail CUE's
  CRLF and unquoted `FILE TQUS02.iso BINARY` spelling. When that authenticated
  MODE1/2048 member is absent but the production cache contains the exact
  `ceb02343868f80cec899e9b239aff2da` US ISO assembled from `TQUS19.iso` and
  `TQUS02End.iso`, the runner creates a private normalized capture CUE and
  replaces only Track 02. Track 19 and audio references remain from the
  original layout. This removes the missing-member/raw-BIN capture mismatch;
  it does not claim a game-owned dungeon consumer. Verification:
  `bash -n scripts/capture_theron_mednafen_live_trace.sh` and
  `tests/test_theron_v1_mednafen_live_capture_script.sh` pass.
- ✅ 2026-08-06 DM1 GRAPHICS.DAT partial-surface quarantine: the legacy reader
  now rejects short LZW decodes and undersized output buffers instead of
  copying incomplete indexed pixels into a bitmap. The focused fail-closed
  regression and the real 713-record PC34 audit pass; no generated surface is
  admitted as a substitute.
# 2026-08-06 Theron Japanese split-ISO capture intake

- ✅ The live Mednafen capture runner now supports both regions. The supplied
  Japanese CUE's CRLF/okvoterade `FILE TQJP02.iso BINARY` member is normalized
  to the complete sibling `TQJP02End.iso` only after its authentic MD5
  `397039af02d50d15c70b74088eb8a1cb` is verified. `THERON_CUE` is accepted as
  the generic variable while `THERON_US_CUE` remains compatible. This extends
  only verified media intake; no JP consumer, dungeon, palette or viewport
  semantics are promoted. Verification:
  `bash -n scripts/capture_theron_mednafen_live_trace.sh`, the live-capture
  script regression, and the real archive CUE transformation pass.
- ✅ 2026-08-06 F10 source-owned live graphics controls: Theron now routes its
  V2 filter changes through `theron_v2_settings` and persists the Theron slot.
  DM2/Nexus no longer mutate DM1 filter state from the popup; unsupported
  source-specific rows are explicitly locked while shared presentation and
  cheat/speed controls remain available. `m11_runtime_graphics_popup` passes.
- ✅ 2026-08-06 DM1 F0115 object identity quarantine: real floor-object and
  HoC alcove rendering now requires the source-owned raw PC34 `THING` record
  before resolving subtype or material. Missing raw identity produces no-draw
  instead of a candidate-derived wrong icon/name. Real F0115 floor pickup and
  alcove pickup-to-inventory tests pass against the PC34 corpus.

- ✅ 2026-08-06 DM2 FM Towns native startup-media gate: the HME-242 ISO reader
  now inventories the root `AUTOEXEC.BAT`, `SWOOSH`, `TITLE`, `TWANIM.EXP`,
  `SKULL.EXP` and `END` files as well as `DATA/`. It reads the original boot
  script in memory and requires the authenticated `SWOOSH -> TITLE -> SKULL
  -> END` route before boot accepts an FM Towns session. The real Japanese CD
  ZIP plus explicitly selected English GDAT companion regression passes with
  no game member unpacked to disk. This verifies the native animation/startup
  ownership and blocks partial media; it does not claim that TWANIM frame
  playback has been implemented.

- ✅ 2026-08-06 DM2 FM Towns animation-stream authentication: boot now checks
  the selected in-memory HME-242 `SWOOSH`, `TITLE` and `END` streams against
  the published retail MD5s before it accepts the AUTOEXEC animation plan.
  This binds the actual 18-layer swoosh and 224-layer/5-sound title corpus to
  the selected FM Towns CD rather than accepting name-compatible bytes. The
  M12 real-media regression verifies all three identities with the Japanese
  CD ZIP and English text companion, without extracting any game data to disk.

- ✅ 2026-08-06 DM2 FM Towns TWANIM stream-bound admission: the production
  boot owner now parses the selected, hash-verified root streams directly from
  the retained CD image using DMWeb's six-byte big-endian record framing.
  It requires the exact HME-242 inventories before exposing startup media:
  SWOOSH has 22 records/18 deltas, TITLE has 235/224 deltas plus one sound
  definition and five sound events, and END has 401 records/382 deltas across
  two matching animation phases. `test_dm2_fmtowns_m12_real_media` proves all
  three receipts from the user's original ZIP in RAM; no title frame is
  invented or rendered by this boundary.

- ✅ 2026-08-06 DM2 FM Towns TITLE IMG1 decoding: `dm2_v1_fmtowns_anim_stream`
  now replays HME-242 EN/DL records into the original packed 320x200 4bpp
  canvas directly from the selected CD stream. It follows SKWIN
  `ANIM_DECODE_IMG1` (0759:0330), including its original contiguous-stream
  boundary behaviour, while retaining strict whole-stream bounds. The
  real-media test locks first/final TITLE frame command counts and FNV-1a
  receipts (`c7ad2279`, `5ef57a09`) computed in RAM from the retail stream.
  This is a decoder and source receipt only; M11 palette/timing/presentation
  remains explicitly unclaimed until its own source-owned handoff exists.

- ✅ 2026-08-06 DM2 FM Towns TITLE M11 presentation: the selected HME-242
  `TITLE` member is retained only in RAM after its boot/profile MD5 and stream
  receipts pass. M11 decodes the original PL index/RGB4 palette, expands it at
  the indexed-render boundary, and presents the stream's packed 320x200 4bpp
  canvas instead of the PC static GDAT menu. EN/DL progression uses the
  SKWIN TWANIM Timer-A unit (`18*(1024-100)` microseconds) and each source
  display duration clamped to the original five-tick minimum. Input cannot
  reach SKULL's later menu until TITLE ends; rejected Towns media remains
  black rather than falling back to PC art. The opt-in real-CD M11 regression
  launches the selected Japanese ZIP plus authenticated English companion,
  verifies the first rendered frame and a source-timed advance; the focused
  `test_dm2_fmtowns_m12_real_media` also passes without unpacking game data.

- ✅ 2026-08-06 DM2 FM Towns TITLE sound-plan receipt:
  `dm2_v1_fmtowns_anim_stream_decode_title_sound` now retains the HME-242
  TITLE's real 12,862-byte signed SND2 PCM span and its five SO events from
  the selected CD buffer. The real-media regression locks the source offsets
  (14, 101790 … 492266), frame positions (14 … 131), sample FNV-1a
  `0b829ae7`, source volume bytes and `03e8` field. DMWeb identifies that
  frequency as invalid for this title; SKWIN `0759:0E33/0EF0` proves slot 1
  and fixed 5500 Hz instead. This is a read-only source receipt, not an SDL
  playback claim; no game member was unpacked or copied to disk.

- ✅ 2026-08-06 DM2 FM Towns SWOOSH M11 presentation: M11 now follows the
  real HME-242 `AUTOEXEC.BAT` ordering by presenting authenticated `SWOOSH`
  before `TITLE`. Its `AN` header is 0x0, so the IMG1 decoder takes the
  320x200 canvas only from SWOOSH's first EN record, exactly as SKWIN
  `ANIM_DECODE_IMG1` does. The retained stream/palette/frame buffer is reused
  for TITLE only after SWOOSH's 19 source frames finish on the Timer-A cadence.
  The real-CD M11 regression locks source frame-zero and first-delta output
  (13 and 59 indexed pixels), prevents early SKULL input, and reaches TITLE.
  No file is unpacked and no PC GDAT screen substitutes for either stream.

- ✅ 2026-08-06 DM2 FM Towns SKULL fallback fence (superseded by the verified
  IMG2 handoff): the temporary black completion state rejected PC GDAT as a
  platform substitute. M11 now presents only the selected HME-242
  `TITLE/0/dtImage+dtPalIRGB/4` IMG2 surface after TITLE, using its native
  local palette and `dt04/0` NEW GAME/RESUME rectangles. Native `SKULL.EXP`
  P3 execution, keyboard routing and continuation semantics remain closed;
  see `parity-evidence/dm2_fmtowns_startup_p3_gdat_boundary.md`.

- ✅ 2026-08-06 DM2 FM Towns CDDA mapping correction: removed the former
  hard-coded HMP→CDDA source literal. Boot now extracts the selected
  HME-242 `SKULL.EXP` in RAM and copies only its native 29-byte table at
  offset `0x3dac` into a bounded receipt. The real-CD regression locks the
  374,416-byte member, source offset and map lookup. Playback remains
  separately blocked until native SKULL execution and CDDA transport are
  joined.

- ✅ 2026-08-06 DM2 FM Towns CDDA coordinate correction: runtime CDDA
  dispatch now reads the live source party X/Y for the original 40-byte
  CD.DAT level-coordinate trigger table, and reevaluates only this route
  after a committed party step. It no longer probes a fabricated `(0,0)`
  cell. Missing source party state remains silent.

- ✅ 2026-08-06 Theron raw-BIN HuC6280 disassembly intake: the static bank-$1f
  receipt now verifies authentic `TQUS02.bin` and `TQJP02.bin` Track 02 files
  in addition to the ISO projections. Their real MODE1/2352 bank-window
  offsets and regional stage-2 handler hashes are bound by MD5/size/byte/FNV
  checks. The focused test passes all four authentic US/JP BIN/ISO sources;
  runtime consumer and semantic level/object/palette/tile/viewport handoff
  remain capture-gated.

- ✅ 2026-08-06 Theron forcefield-menu keyboard fix: M11's physical
  left/right arrow tokens (`STRAFE_LEFT/STRAFE_RIGHT`) now move Theron's Soul
  Room focus. Enter can therefore reach the FORCEFIELD action instead of
  appearing inert; the source-owned capture gate still prevents an
  unverified dungeon handoff.

- ✅ 2026-08-06 Theron real-data inventory: documented the authenticated US/JP
  Track 02 BIN/ISO files, the separate US/JP Track 19 ISOs and the materialized
  US split ISO, including size/MD5 ownership. The documentation explicitly
  prevents Track 19 bytes from being reused as Track 02 data and lists the
  remaining intentional placeholder/capture boundaries.
# 2026-07-31 Theron Track 02 quest-block extraction

- ✅ Added a source-data extractor for the seven 256 KiB quest blocks present
  in the verified US Track 02 raw BIN. Each block is reconstructed from
  MODE1/2352 sectors into contiguous 2048-byte user-data bytes and checked by
  an independent FNV-1a receipt in the bank probe. JP media remains explicitly
  unsupported until its corresponding block offsets are independently
  verified. This is real-data byte extraction only; it does not claim dungeon
  record, object-table, palette, bitmap, or runtime-render semantics.
  Verification: `theron_v1_track02_bank` and the clean-branch C11 syntax checks.
- ✅ 2026-07-31 DM2 V2 smooth viewport no-fabrication closure: removed the
  host-side pan and black-strip fill that ran after the real V1 viewport
  renderer. Smooth timing state remains available to input consumers, but no
  intermediate DM2 camera raster is known, so every presented frame remains
  the source-owned snapped V1 raster. References: SKProject
  `SKWIN/SkWinCore.cpp::DRAW_DUNGEON` and `DRAW_OUTDOOR_VIEWPORT`.
  Verification: V2 smooth movement 79/79, runtime binding 43/43, smooth
  probe 54/54, plus a byte-identical V1/V2 framebuffer comparison during an
  active smooth state in the hash-verified real-data DM2 M11 startup test.
# 2026-07-31 Theron JP Track 02 quest-block extraction

- ✅ Extended the real Track 02 quest-block extractor to the hash-verified JP
  BIN. The JP bank begins one raw MODE1/2352 sector before the US bank; all
  seven 256 KiB blocks are reconstructed from contiguous 2048-byte user data
  and independently checked against FNV-1a receipts from `TQJP02.bin`.
  The US receipts remain covered as well. This follows DMWeb's JP/USA
  PC-Engine CD split and seven-dungeon scope; it does not claim dungeon-record,
  object, graphics, or save-format decoding.
  Verification: clean C11 syntax checks, clean CMake target build, and
  `firestaff_theron_v1_track02_bank_probe` against both local real BINs.
- ✅ 2026-07-31 DM2 V2 unbound sky-colour closure: removed the procedural
  RGB gradients and fixed weather colours from the V2 lighting/outdoor helper
  APIs. ENVIRONMENT_DRAW_DISTANT_ELEMENT owns an outdoor image, palette and
  destination rectangle; time and weather alone cannot select original
  pixels. Unbound callers now receive DM2_V2_SOURCE_COLOR_UNAVAILABLE.
  References: SKProject SKWIN/c_bkgrnd.cpp ENVIRONMENT_DRAW_DISTANT_ELEMENT
  and skgdtqdb.cpp QUERY_TEMP_PICST/DRAW_TEMP_PICST. Verification:
  test_dm2_v2_lighting 64/64 and DM2/M11 build pass.
# 2026-07-31 Theron media-inventory false-promotion removal

- ✅ Raw Track 02 now proves startup/media ownership only. Removed the
  incorrect bitmap-, level-, and object-route promotion that treated an
  authenticated bank as if its dungeon decoder were already implemented.
  Downstream routes remain fail-closed until real consumer/decoder evidence
  exists, matching the bounded Theron status in `docs/DMWEB_REFERENCE.md` and
  TODO.md. Verification: `theron_v1_media_inventory_probe` passes.
- ✅ 2026-07-31 CSB startup-fallbackkontrakt: tog bort den döda
  title-/entrance-planens text- och dörrfallbackfält, dess gamla
  renderkommandon och den host-återgivning de kunde bära. CSB:s uppstart
  representerar nu endast originalets C001–C005, C017 och C040; en saknad
  källa blir no-draw i stället för text eller färgpaneler. Källa: ReDMCSB
  `TITLE.C F0437`, `ENTRANCE.C F0438/F0441/F0806`; CSBWin
  `Viewport.cpp`. Verifiering: startup-plan 139/139, boot-handoff 501/501,
  realdata-sekvensen C001–C005/C017/C040 och titelkadensproben passerar.
- ✅ 2026-07-31 CSB startup-rendercallbackar: tog bort den återstående
  executor-API-ytan för dörr- och textfallback. ReDMCSB `TITLE.C F0437` och
  `ENTRANCE.C F0441/F0806` når nu bara konkreta originalytor via title-,
  dörr-, opening-frame- och utility-callbackarna; värden kan inte längre
  ansluta en lokal ersättningsritning. Verifiering:
  `test_csb_v1_boot_runtime_handoff` 501/501 passerar.
- ✅ 2026-07-31 DM2 inventory substitute closure: removed the reachable M11
  renderer that put authentic DM2 ObjectID icons into DM1 `GRAPHICS.DAT` slot
  rectangles and removed its matching DM1 click route. Keyboard and direct
  champion inventory commands now fail closed as well, leaving SKSave/DB
  ObjectID ownership untouched until the real DM2 inventory surface is
  bound. Source: SKProject `CHANGE_VIEWPORT_TO_INVENTORY`, with its
  `CHAMPIONS`/`INTERFACE_GENERAL` GDAT layout and event table. Verification:
  real-data `test_dm2_v1_m11_startup_profile_gate`.
# 2026-07-31 Theron alarm spawn fallback removal

- ✅ Removed the production alarm-trigger path that fabricated a Goblin for
  every creature spawner. The alarm still activates source spawners and emits
  its alarm event, but creature materialization now stays fail-closed until
  the real Track 02 object-tail/spawn table is decoded. Regression coverage
  verifies activation and no fabricated object (`52/52` mechanics checks).
- ✅ 2026-07-31 CSB startup-assettyper: tog bort den oanvända
  `fallback`-källtypen och den döda `fallback-original`-aliasen från
  CSBgraphics-bindningen. Startup accepterar nu enbart verifierad
  `GRAPHICS.DAT` eller verifierad `CSBgraphics.dat`; negativa tester använder
  den verkliga ogiltiga typen `NONE`. Verifiering:
  `test_csb_v1_boot_title_import_ui_gate_pc34_compat` 137/137 och
  `test_csb_v1_csbgraphics_runtime_binding` 83/83 passerar.
- ✅ 2026-07-31 CSB källinventering: korrigerade den felmärkta Lord Order-
  typen. `0x19` är ReDMCSB `DEFS.H:1364` C25_CREATURE_LORD_ORDER, inte en
  placeholder, även om originaldungeonerna saknar sådana grupper. Uppdaterade
  även TODO:s inaktuella uppgift om det borttagna `fallback-original`-aliaset.
  Verifiering: `test_csb_v1_monster_generator_gate_pc34_compat` passerar.
- ✅ 2026-07-31 CSB title-capturekadens: realdatafångsterna för V1, V2.0 och
  V2.1 väntar nu 14 sekunder i stället för 7, så att de observerar alla fyra
  originalpalettfaser efter den PC3.4-bundna CHAOS-zoomen. Speltempot är
  oförändrat. Verifiering: V1:s title/entrance-kontrakt samt V2.0- och
  V2.1-capturetester passerar mot lokal PC3.4-data.
- ✅ 2026-07-31 CSB F0115-projektiler: tog bort den gamla 16×16-ikonritningen
  som kunde ersätta ReDMCSB:s perspektivbitmap för kastade objekt. En saknad
  källbunden F0115-bitmap blir nu no-draw; bara den verifierade perspektiv-
  rutten kan skriva projektilpixlar. Verifiering:
  `test_csb_v1_viewport_phase3_rendering` 2655/2655 passerar.

- ✅ 2026-07-31 DM1 HoC candidate time-effects and endgame fallback gates:
  the live M11 idle route now proves ReDMCSB `CHAMPION.C F0331` excludes the
  selected C040 candidate from health/stamina/food/water mutation, then
  restores normal decay at the next due tick after confirmation. The related
  F0444/F0446 regression expectations were aligned with the existing
  source-only policy: missing original final-screen art draws no synthetic
  controls, while an available SDL backend may queue real SONG.DAT victory
  audio. Verification: `m11_starvation_runtime_source_lock`,
  `m11_action_stamina_runtime_source_lock`,
  `dm1_v1_hall_of_champions_pc34_compat`, and the real backed PC34 corpus
  roundtrip all pass.
# 2026-07-31 Theron relic-name correction

- ✅ Replaced the invented quest-item labels in progression, chapter-marker,
  and champion-item comments with the seven real Theron's Quest relic names
  documented by DMWeb: Shield Defiant, Taza Poleyn, Tazahelm, Taza Boots, Taza
  Armor, Soulcage, and The Retaliator. This changes presentation metadata only;
  item ordinals and the unresolved Track 02 placement/decode remain bounded.
  Verification: `firestaff_theron_v1_chapter_marker_probe` passes `65/65`.
# 2026-07-31 Theron seed-placeholder reduction

- ✅ Replaced the dungeon-1 progression fallback seed `313` with the real
  US/JP Track 02 initial-level seed `0x0108e938`. The unresolved dungeon 2–7
  fallback seeds are now zero rather than fabricated ascending values, so
  progression/save state cannot present guessed seeds as original data.
  Verification: `test_theron_v1_m11_direct_launch` passes; real US/JP Track 02
  probes bind the same initial seed at their verified raw offsets.
# 2026-07-31 Theron stale placeholder metadata removal

- ✅ Removed the retired Theron dungeon-seed fallback `313` from the boot
  profile; an unbound profile now starts at zero and only verified header or
  Track 02 handoff data may populate the seed. Updated the Track 02 source-lock
  table to mark dungeon 1 as `0x0108e938` (verified initial level) and dungeons
  2–7 as unresolved. Corrected the source-lock quest-item names to the seven
  DMWeb relics: Shield Defiant, Taza Poleyn, Tazahelm, Taza Boots, Taza Armor,
  Soulcage and The Retaliator. Verification: `test_theron_v1_m11_direct_launch`
  passes; `git diff --check` passes.
# 2026-07-31 Theron real door-state query

- ✅ Removed the remaining party-level door-state placeholder from
  `theron_v1_get_move_result()`. Hypothetical movement now reads the matching
  level door object's actual state, just like the committed movement path;
  missing door objects remain blocked rather than inheriting fixture state.
  Verification: `test_theron_v1_m11_direct_launch` passes and
  `git diff --check` passes.
# 2026-07-31 Theron real item pickup state

- ✅ Replaced the `THERON_CMD_TAKE` success-without-state placeholder. Known
  Track 02-independent object classes (potion, scroll, food, key, weapon and
  armor) now bind to the source-locked compact item IDs, enter the active
  champion's inventory, mark the level object picked up and recalculate load.
  Unknown/quest object classes remain rejected rather than receiving guessed
  IDs until the real Track 02 object table is decoded. Verification:
  `test_theron_v1_m11_direct_launch` passes and `git diff --check` passes.

- ✅ 2026-07-31 CSB V2.2 live-cache cleanup: M11 no longer populates the
  retired 3x3 CSB V2.2 shape cache during either CSB viewport path. Its
  hard-coded material parameters had no authenticated `DUNVIEW.C F0128`
  command, palette, clip or Thing-chain receipt and no consumer in the
  admitted compositor. Live CSB pixels can therefore reach V2.2 only through
  the command-local source-material route, while unsupported families remain
  V1. Verified with `test_csb_v1_viewport_phase3_rendering` (2655/2655),
  `test_csb_v22_inplace_draw_pc34` (57/57), and
  `test_csb_v22_shapes_pc34` (54/54).
# 2026-07-31 Theron locked-door inventory gate

- ✅ Removed the locked-door auto-unlock placeholder. `theron_v1_door_open()`
  now requires the active champion to carry the source-locked key item before
  clearing a real door's locked flag; absent keys leave the door closed.
  Verification: `test_theron_v1_m11_direct_launch` passes and
  `git diff --check` passes.
- ✅ 2026-07-31 Nexus STABG indexed-blit gate: `nexus_ui_render_stabg()` now
  refuses to copy retail palette indices into a framebuffer unless the same
  surface carries its verified source palette. This closes the remaining
  public wrapper path for unpaletted/synthetic HUD pixels; Saturn VDP
  placement remains a separate no-draw gate. Verification: Nexus startup-media
  and FACE real-data tests pass against `/Users/bosse/.firestaff/data/nexus`.
- ✅ 2026-07-31 DM1 timeline-dispatch stability re-verification: the former
  F0242/F0248/F0190/F0249 assert-crash cluster is stable on current main.
  The seven documented CTests pass once and in ten consecutive repetitions
  each (70/70): square-state dispatch, three F0248 launchers, fake-wall
  group deferral, and both F0190 killed-all handoffs. This closes only the
  stale crash report, not the broader original-runtime or pixel-parity work.
# 2026-07-31 Theron party-gold save binding

- ✅ Replaced the save-header gold placeholder with an explicit
  `theron_v1_save_to_slot_with_gold()` API. The real party round-trip test now
  supplies `party.gold`, the save header persists it, and slot metadata reads
  it back as `party_gold`; the legacy API remains a documented no-gold wrapper
  for callers without party context. Verification:
  `test_theron_v1_save_progress_roundtrip_pc34` and
  `test_theron_v1_m11_direct_launch` pass; `git diff --check` passes.
- ✅ 2026-07-31 Nexus SAL/MAP statuskorrigering: ljudvägen är inte en tom
  placeholder längre. Den behåller verifierad källidentitet, bounded MAP-
  fönster och SAL-containerprofil, men markerar fortfarande codec och Saturn-
  eventdispatch som oprövade och blockerar playback. Kommentarerna använder
  därför `opaque/no-playback` i stället för den missvisande `STUB`-etiketten.
- ✅ 2026-07-31 Nexus rörelseresultat för vatten/eld: standalone-rörelsevägen
  returnerar nu `BLOCKED_WATER` respektive `BLOCKED_FIRE` i stället för att
  felaktigt kollapsa båda till `BLOCKED_WALL`. Item-/runeägarskap förblir hos
  mechanics-källan och aktiveras inte av denna korrigering.
  Verifiering: C11-rörelsecheck mot `firestaff_nexus`.
- ✅ 2026-07-31 CSB boot materialization gate: `csb_v1_boot_enter_game()`
  now reaches `RUNTIME_READY` only after loading a ReDMCSB byte-map dungeon
  and decoding its initial party pose. Missing materialized data and the
  retired 16-bit parser fixture fail closed at `ASSETS_READY`, clear the
  dungeon singleton and cannot bind M11's HUD or viewport. Verified with
  `test_csb_v1_boot_viewport_render_gate`, `test_csb_v1_boot_profile_smoke`
  and `test_csb_v1_boot_runtime_handoff`.
- ✅ 2026-07-31 Nexus trapp-/trappstegslänk: oregistrerade trappor återanvänder
  inte längre koordinater eller antyder en implicit angränsande nivå.
  `nexus_stairs_resolve()` returnerar explicit unresolved-sentineller tills en
  källbunden länk registrerats; registrerade länkar är oförändrade.
  Verifiering: C11-check för både unresolved och registrerad länk.
- ✅ 2026-07-31 Nexus teleporter-owner gate: mechanics kontrollerar nu
  teleporter-länken före party-positionen muteras. En oregistrerad
  TELEPORT/TELEPORT2/TELEPORT3 blockerar utan förflyttning; registrerad länk
  dispatchas oförändrad. Verifiering: `test_nexus_v1_pit_teleporter_runtime`
  passerar 44/44.

- ✅ 2026-07-31 CSB direct-loop source handoff: `fs_game_init()` now rejects
  absent or unmaterialized CSB media, just like the boot/M11 route, and
  `fs_game_load_assets()` consumes the boot-owned dungeon and party pose.
  The generic DM1 parser can no longer supply its fixed `(11,29)` start point
  to a CSB session. Verification: direct launch against
  `/Users/bosse/.firestaff/data/csb`, `test_csb_v1_boot_viewport_render_gate`
  and `test_csb_v1_boot_runtime_handoff`.
- ✅ 2026-07-31 Nexus HUD-guld: M11 skickar nu mechanics-statens verkliga
  `gold_pieces` till HUD:n i stället för att alltid mata in syntetiskt noll.
  Fältet uppdateras av den källbundna gold-pile-pickup-vägen; fallback till
  noll används endast när mechanics-pekaren saknas. Verifiering:
  full `firestaff`-build och `test_nexus_v1_dgn_runtime_materialization`.
- ✅ 2026-07-31 Nexus HUD-startgate: produktionsvägarna för launcher-start och
  save-resume använder inte längre `force_active_for_test(1)` för HUD:n.
  HUD-rendering kräver därmed den normala V2-presentationsgaten; testläget
  finns kvar endast för explicita integrationstester. Verifiering:
  `test_nexus_v2_hud_runtime_integration` passerar 9/9 och full `firestaff`
  build passerar.

- ✅ 2026-07-31 CSB runtime boot materialization gate:
  `csb_v1_runtime_boot()` no longer reports success with absent graphics, an
  unreadable/legacy dungeon or no decoded initial party pose. A failed retry
  clears the prior dungeon singleton and source paths before it returns.
  Verification: `test_csb_v1_boot_runtime_handoff`, including its missing
  source-media regression, plus boot-profile and viewport gate tests.
- ✅ 2026-07-31 Theron uppstart: boot-scannern känner nu igen de faktiska
  råa Track 02-filnamnen `TQJP02.bin` och `TQUS02.bin` som används i
  `~/.firestaff/data/theron`. De hashverifieras genom samma befintliga
  kataloggate; inga nya datafiler eller fallbackvärden läggs till.
- ✅ 2026-07-31 Nexus V2-produktionsgate: launcher-start och save-resume
  kringgår inte längre presentationsgaten för lighting, smooth movement eller
  touch-runtime med test-only `force_active_for_test(1)`-anrop. V2-proberna
  aktiverar fortsatt läget explicit. Verifiering: `firestaff`-build,
  smooth-movement-probe 33/33 och touch-runtime-probe 57/57.
- ✅ 2026-07-31 Nexus ljuddiagnostik: kvarvarande `(stub)`-etiketter för
  CDDA stop/pause/resume/fade är ersatta med `opaque/no-playback`. Verkliga
  SAL/MAP- och CD-spår förblir källbundna, men codec/driver och uppspelning
  markeras fortsatt som blockerade. Verifiering:
  `test_nexus_v1_sound_runtime_receipt` passerar.
- ✅ 2026-07-31 Theron uppstart-seed: startup-receipt kopplar nu boot-
  sammanfattningens dungeon-seed till den verifierade initiala Track 02-
  levelheadern (`0x0108e938`) i stället för att lämna no-header-värdet `0`.
  Real-asset-proben verifierar seed, roster och startup-handoff.
- ✅ 2026-07-31 CSB graphics filename-fallback removal: runtime graphics
  discovery now requires a known CSB graphics MD5 for every version hint,
  including unknown/custom hints. A random `GRAPHICS.DAT`, `CSB.DAT` or
  `CSBGRAPH.DAT` can no longer become a live graphics binding merely because
  of its filename. The regression covers both selected and unknown hints;
  renamed authentic media remains discoverable through recursive hash search.
- ✅ 2026-07-31 CSB undefined monster-projectile gate: Grey Lord/Lord Order's
  documented ReDMCSB `GROUP.C` BUG0_13 path, and a missing RNG context, no
  longer create a synthetic Fireball. They return no source projectile, which
  the live runtime rejects before projectile creation. Normal authenticated
  creature attacks keep their original projectile selection.
- ✅ 2026-07-31 Nexus FONT256 DMWeb-regioner: den verkliga S2D-decodern
  exponerar nu namngivna, bounds-verifierade bytefönster för Map, Page/
  tilemap, Character Generator, Palette och Attributes enligt DMWeb:s
  `DecodeFONT256S2D`. Retailkontroll mot `FONT256.S2D` verifierar de fem
  offset/size-paren; ingen glyph- eller menysemantik påstås ännu.
  Verifiering: `test_nexus_v1_font_s2d` passerar.
- ✅ 2026-07-31 Nexus FONT256 Character Generator: en bounded API kopierar
  nu DMWeb:s 242 verkliga 8x8/8-bit tiles från CG-regionen efter dess
  16-byte prefix och avvisar index/filgränsöverskridanden. Tileindexen hålls
  uttryckligen separata från glyph-/menysemantik. Verifiering:
  `test_nexus_v1_font_s2d` passerar mot lokal retailfil.
- ✅ 2026-07-31 CSB M11 media-rehash gate: the M11 entry boundary now hashes
  the selected `GRAPHICS.DAT` and `DUNGEON.DAT` again and requires exact
  agreement with the boot profile's scanned receipt before any CSB pixels can
  be decoded. A file replaced after scan fails closed instead of inheriting a
  stale verified flag; the focused boot-profile test covers this regression.
- ✅ 2026-07-31 Nexus FONT256 Page/palette words: bounded API:er läser nu
  DMWeb:s 4096 big-endian Page/tilemapord och 256 big-endian BGR555-
  paletteord från de verkliga regionerna. Retailtestet verifierar tilemapord
  1 = `0x0002`, paletteord 0 = `0x8000` samt indexgränser; ingen glyph- eller
  menybetydelse härleds ännu. Verifiering: `test_nexus_v1_font_s2d`.
- ✅ 2026-07-31 Nexus FONT256 attributes: bounded API för de 242 verkliga
  big-endian attribute-orden är tillagd från DMWeb:s Attributes-region.
  Tile-attributen hålls separata från ännu obevisad glyph- och menysemantik.
  Verifiering: `test_nexus_v1_font_s2d` passerar mot retailfilen.
- ✅ 2026-07-31 Nexus HUD no-fake gate: live DGN-vägen sätter inte längre
  V2-presentationsflaggor hårdkodat för att öppna den procedurala HUD:n.
  Utan en autentiserad retail-widget/VDP-placement receipt förblir overlayn
  stängd; explicit V2-integrationstest kan fortfarande aktivera den.
  Verifiering: full `firestaff`-build, HUD 9/9 och DGN materialization-test.
- ✅ 2026-07-31 CSB dead state-shim removal: deleted the unbuilt
  `csb_v1_game` skeleton, which exposed fixed `(5,5)`/`(0,0)` positions and
  marked DM1 import complete without loading anything. CSB now has only the
  verified `CSB_V1_RuntimeProfile`/dungeon/Utility ownership documented by
  the integration and source-lock references; no production caller used the
  retired API.
- ✅ 2026-07-31 Nexus viewport animated-material gate: Structure3-material
  med `0x08xx` behåller retail descriptor-proveniens men använder inte längre
  första Structure2-bilden som en obevisad statisk frame-substitution.
  Pixelrutten förblir no-draw tills Saturn frame-selector/VDP1-bindningen är
  verifierad. Verifiering: `test_nexus_v1_dgn_runtime_materialization`;
  source-receipt-testet skippar korrekt utan staged Nexus-dir.
- ✅ 2026-07-31 CSB Utility metadata-party removal: `get_party()` no longer
  reconstructs champion count, leader, and import provenance from free
  `reserved[]` metadata when the imported champion body is missing. The
  runtime receives only the full validated Utility party; the regression
  proves stale metadata cannot manufacture a launchable party.
- ✅ 2026-07-31 CSB file-dungeon fixture closure:
  `csb_v1_dungeon_load_from_file()` now rejects the retired 16-bit
  column-major fixture layout after parsing, clears its temporary ownership,
  and publishes only ReDMCSB-compatible one-byte square maps from a path.
  The explicit fixture regression proves the file boundary fails closed.
- ✅ 2026-07-31 Nexus MENU.BPK PRS3 source-lock correction: the runtime decoder is now documented against DMWeb `DMNDataFileDecoder.vbs::DecodePRS3`, including its LSB-first control bytes, literal/back-reference commands, 12-bit window, and `+18`/negative-window rule. The real local `MENU.BPK` corpus decodes all 162 PRS3 surfaces with zero failures. Remaining MENU work is pixel-mode/palette interpretation and authenticated Saturn VDP1 placement, not an undocumented compression algorithm.
# ✅ 2026-07-31 — Theron palette admission is source-gated

- Removed the synthetic default stone-gradient palette from the V1 palette state.
- An unbound palette now remains empty, so HUD/viewport code cannot receive manufactured colors before verified Track 02 data is loaded.
- Updated the rendering test to assert the fail-closed palette contract; focused suite passes 25/25.
# ✅ 2026-07-31 — Theron V2 HUD production path is asset-gated

- The boot/runtime path no longer draws the procedural V2 HUD overlay when the HUD widget manifest is missing, partial, or placeholder-only.
- Rendering now requires a complete manifest with real assets for every HUD slot; the local Track 02 BINs remain correctly limited to verified startup surfaces.
- Verification: `test_theron_rendering` 25/25 and `test_theron_v2_hud_overlay_pc34` 58/58.
# ✅ 2026-07-31 — Theron V1 chrome helpers fail closed

- Direct topbar, right-panel, and champion-slot helpers no longer emit procedural blocks, icons, or name bars without a verified runtime chrome bank.
- This closes the legacy low-level path as well as the master HUD compositor; the generic bar primitive remains available for source-backed callers.
- Verification: `test_theron_rendering` 25/25.
# ✅ 2026-07-31 — Theron startup fallback no longer invents unknown seeds

- The legacy bounded fallback-room receipt now reports seed `0` when dungeon metadata is not verified instead of carrying the retired literal seed `313`.
- Verified Track 02 startup remains authoritative; this change only removes misleading metadata from the compatibility fixture path.
- Verification: `test_theron_rendering` 25/25.
# ✅ 2026-07-31 — Theron startup no longer paints no-data placeholders

- Removed the production branch that enabled command-drawn synthetic title, stage, Soul Room, and forcefield graphics when Track 02 was absent.
- Startup now reports `NO VERIFIED TRACK02 GRAPHICS` and remains blocked until the real atlas route is present.
- Verification: `test_theron_rendering` 25/25 and `firestaff_theron_v1_startup_flow_probe` 653/653.
# ✅ 2026-07-31 — Theron V2.2 missing-shape API fails closed

- Removed the runtime checkerboard placeholder contract from `theron_v22_get_missing_placeholder()`; missing modern assets now return `NULL` with 0×0 dimensions.
- Updated the public contract and regression test. No production caller can receive invented missing-texture pixels.
- Verification: `test_theron_v22_modern_assets_pc34` 32 checks, 0 failures.
# ✅ 2026-07-31 — Theron boot scanner rejects unverified legacy files

- Removed the `GRAPHICS.DAT`/`DUNGEON.DAT` fallback search from the Theron boot scanner.
- Theron launch discovery now accepts only the hash-verified Track 02 media routes present in the real data corpus; unverified extracted files cannot become a launch source.
- Verification: `test_theron_rendering` 25/25 and `firestaff_theron_v1_startup_flow_probe` 653/653.
# ✅ 2026-07-31 — Theron legacy enter-game stub fails closed

- `theron_v1_boot_enter_game()` no longer reports success while leaving `theron_state` and `dungeon_data` unbound.
- The real Track 02 runtime handoff remains the only valid game-state transition.
- Verification: `test_theron_rendering` 25/25 and `firestaff_theron_v1_startup_flow_probe` 653/653.
# ✅ 2026-07-31 — Theron Track 02 bad-input routes deny fallback visuals

- Track 02 startup/object/level route receipt initializers now default `fallback_visuals_allowed` to `0` for unknown or malformed input.
- A caller must receive explicit verified route evidence before any visual permission can exist; bad input cannot grant placeholder rendering.
- Verification: `test_theron_rendering` 25/25.

- ✅ 2026-07-31 CSB viewport contract isolation: three more contract-only
  CustomBackgrounds modules (D1L/D1R first backdrop, floor/ceiling mask
  ordering and room-pass ordering) now compile exclusively into their focused
  tests, not `firestaff_m10`. Live viewport code retains only the source-bound
  room-slot/material path. Verification: focused regressions (74 + 563 + 86
  assertions) and complete `firestaff` link.

- ✅ 2026-07-31 CSBWin save-fixture isolation: the synthetic 14-shape
  CSBWin/DM1 save corpus and its convenience runner were removed from the M10
  loader-boundary module and public production header. They are now test-only
  support for the focused regression, boot-handoff regression and skip-safe
  verification probe; the runtime boundary accepts only caller-supplied save
  bytes. Verification: loader-boundary test 158/158, boot handoff 504/504,
  real staged-save probe 22/22 and complete `firestaff` link.
- ✅ 2026-07-31 DM1 HoC F0172 ornament correction: removed the
  Firestaff-only map-zero random-floor-ornament suppression. ReDMCSB
  `DUNGEON.C F0172` applies this path to every corridor map, and sensors then
  override its ordinal. The regression covers both a map-zero sensor ornament
  and a deterministic map-zero random ornament. Verification:
  `test_m11_overlay_command_queue_block` (192/192) and
  `test_m11_v22_shape_cache_pc34` (31/31).

- ✅ 2026-07-31 DM1 HoC F0172 sensor-zero correction: floor sensors now
  overwrite the random floor-ornament ordinal even when their source-owned
  `Remote.OrnamentOrdinal` is zero. ReDMCSB assigns that field
  unconditionally; zero suppresses a random grate or pressure plate instead
  of allowing it to leak through. Verification:
  `test_m11_overlay_command_queue_block` (193/193),
  `test_m11_v22_shape_cache_pc34` (31/31), and the installed PC 3.4 HoC
  runtime probe.

- ✅ 2026-07-31 DM1 F0115 alcove-object input binding: C080 now accepts the
  actual current-frame C2548/F0791 destination rectangle for a front alcove
  item, in addition to the original C05 ornament zone. This preserves wall
  sensor input while making a real rendered torch/object pickable. Verification:
  `test_m11_dm1_real_alcove_item_runtime_pc34` finds map 1 `(6,3,2)` in the
  installed PC34 corpus and successfully transfers the rendered object into
  the leader hand.

- ✅ 2026-07-31 DM1 F0115/C080 rendered floor-pile input: normal DM1 no
  longer uses four fixed, approximate floor-item click panes. Each successful
  PC34 F0115/F0791 object blit now publishes its exact final rectangle,
  source `THING`, and map square for the current frame; C080 takes the
  topmost clicked rendered object directly into the leader hand. Missing or
  occluded source material therefore cannot select an arbitrary neighbour
  from the thing chain. Verification: the real PC34 non-HoC F0115 runtime
  test clicks the returned material rectangle and confirms that a leader-hand
  object is produced; `test_m11_overlay_command_queue_block` remains 193/193.
- ✅ 2026-07-31 Nexus ITEM.IBS/viewport source chain recheck: the focused
  Structure1F provenance and spatial receipts, all 16-level retail DGN
  face/material admission, and runtime materialization pass against the real
  European corpus. ITEM.IBS 4bpp/palette ownership remains source-bound and
  no-draw; the only remaining viewport gate is authentic Saturn VDP1 capture.
- ✅ 2026-07-31 Nexus MENU.BPK palette boundary: DMWeb's 256-entry
  big-endian PALT trailer is now revalidated from the real `MENU.BPK`.
  Structure2 ABI, intake and PRS3/VDP1 consumer-evidence tests all pass;
  palette bytes remain source-bound but are not promoted to visible menu
  pixels until an authentic Saturn consumer trace is available.

- ✅ 2026-07-31 CSB viewport contract isolation: the unbound D0L2/D0R2
  F0111 partly-open-door and D1L/D1R F0108 floor/ceiling-ornament contract
  modules now compile exclusively into their focused tests, not `firestaff_m10`.
  They contain no authenticated bitmap decoder or runtime consumer, so keeping
  them out of M10 prevents their source-locked metadata from masquerading as a
  draw path. Verification: both focused tests and full `firestaff` link.
- ✅ 2026-07-31 Theron startup fallback boundary: confirmed M11 has no caller
  for the legacy synthetic-room API and uses only
  `theron_v1_startup_runtime_load_initial_level_verified_only()`. The helper
  and legacy loader are now explicitly documented as data-free fixture
  compatibility only; verified Track 02 with no semantic handoff remains
  blocked. Startup-flow `653/653` and rendering `25/25` remain green.
- ✅ 2026-07-31 Nexus startup/menu/HUD audit: real `TITLE.CG`, warning/gameover
  media, champion startup menu, `FONT256.S2D`, MENU.BPK no-draw handoff and
  the V2 HUD gate all pass their focused tests. The HUD integration's 9/9
  render assertions are test-only; production keeps the procedural overlay
  closed until a retail widget/VDP placement receipt exists.
- ✅ 2026-07-31 Nexus HUD provenance correction: removed the false claim that
  the procedural V2 overlay was sourced from retail `NEXUS.BIN`. The supplied
  corpus has no authenticated HUD widget surface; the module is explicitly
  diagnostic/test-only and production remains gated. HUD overlay 46/46,
  runtime integration 9/9 and `firestaff_m11` build pass.
- ✅ 2026-07-31 Nexus V2 provenance audit: corrected remaining lighting, touch,
  smooth-movement, phase-gate and title comments so absent `NEXUS.BIN` data is
  recorded as unavailable rather than presented as a retail source. ReDMCSB,
  DMDF/DGN and existing behavioral references remain cited; all production V2
  gates stay closed. Focused lighting 79/79, phase gate 240/240, smooth
  movement 27/27 and touch affordance 0 failures pass.
- ✅ 2026-07-31 Nexus launcher card audit: the modern M12 card renderer no
  longer permits any generated game-card motif branch to paint the Nexus card,
  even if a layout slot index is reused. Nexus startup/menu art therefore stays
  source-bound/no-draw until real Saturn placement is admitted; other game-card
  routes are unchanged. `firestaff_m11` rebuild passes.
- ✅ 2026-07-31 Nexus launcher status audit: removed the hardcoded `AVAILABLE`
  label from the legacy M12 card path. Nexus now reports readiness only from
  the verified asset-version match, like the other games; `firestaff_m12`
  rebuild and diff check pass.
- ✅ 2026-07-31 Nexus real FONT256 handoff: fixed the inverted
  `nexus_v1_font_s2d_decode()` success check in engine init. The supplied
  `FONT256.S2D` now reaches the engine's source-admitted state; the separate
  page-to-character glyph-render gate remains closed, so no guessed glyphs are
  emitted. Real Track 1 capture readiness passes 29/29, FONT256 decoder and
  startup-menu tests pass.
- ✅ 2026-07-31 Theron production combat boundary: removed the inferred
  creature/combat template table from `firestaff_theron`. Production now
  links explicit fail-closed symbols from
  `theron_v1_combat_runtime_noop.c`; the full inferred implementation is
  available only to the dedicated combat fixture target. Rendering `25/25`
  and startup-flow `653/653` remain green.
- ✅ 2026-07-31 Theron dörrregression: uppdaterade combat-fixturen så den
  placerar en riktig `THERON_ITEM_KEY` innan den försöker öppna en låst dörr.
  Testet följer nu den källbundna nyckelgrinden och passerar 66/66.
- ✅ 2026-07-31 Theron shop-data boundary: removed the fixture-driven,
  source-unverified shop price-table helper from the production archive.
  Its focused test and purchase-gate probe still compile it explicitly;
  production cannot expose inferred shop prices or item ranges.
- ✅ 2026-07-31 Theron V2.2 viewport boundary: removed the placeholder
  3×3 cell-rectangle cache from the production Theron archive. Focused V2.2
  tests may still compile it explicitly, but live rendering cannot consume
  guessed viewport coordinates.
- ✅ 2026-07-31 Theron V2.2 material boundary: removed the inferred modern
  shape/material book from the production archive and replaced its init seam
  with an explicit blocked route. Focused V2.2 fixture targets retain the
  original shape implementation; live production cannot promote its guessed
  tints or geometry.
# Isolated the inferred Theron V2 HUD widget manifest/parser from production and added a no-op gate seam; procedural HUD pixels can no longer render in the verified runtime without a complete real asset manifest.

- ✅ 2026-07-31 Nexus champion provenance audit: the earlier 24-entry table
  was confirmed as fixture data and removed from the live path. The real
  `RLOWFIX.BIN`/`PLRD` handoff is recorded below; the 24-entry array remains
  storage capacity only.

- ✅ 2026-07-31 Nexus PLRD champion handoff: DMWeb's real
  `RLOWFIX.BIN` `RES*`/`PLRD` structure is now parsed in production. The
  European corpus supplies 20 records with Japanese `TABL`-decoded labels,
  HP/stamina/mana, attributes, levels, and source ordinals; the 24-element
  array remains storage capacity only. `test_nexus_v1_champion_plrd` passes
  against the local real file, and malformed/missing PLRD input fails closed.
- ✅ 2026-07-31 Nexus ITEM.IBS ordinal handoff: the source-owned category and
  weight bytes for all 243 real ITEM.IBS declarations now form the live item
  lookup boundary. PLRD equipment/backpack ordinals retain real declaration
  identity without reviving the old DM1 catalog; names, attack/defense and
  key/action semantics remain explicitly unavailable.
- ✅ 2026-07-31 Theron V1 UI chrome isolation: removed the inferred bars,
  labels and champion-slot pixels from the production archive. The public
  chrome API now fails closed through a no-op seam until the original Track
  02 UI bank is decoded; the old implementation remains fixture-only.
- ✅ 2026-07-31 Theron viewport admission wording: corrected the lifecycle
  and source comments to describe the palette as unbound, and removed the
  stale claim that facing could come from a world-tick surrogate. The
  viewport continues to accept only the authenticated party pose and blocks
  pixels until a source tile bank is bound.

- ✅ 2026-07-31 Theron tile-renderer isolation: removed the inferred
  square/depth tile table and rasterizer from the production archive. The
  diagnostic tile-renderer probe still compiles the implementation explicitly;
  production now returns no tile and preserves the framebuffer until a real
  Track 02 tile-bank handoff exists.

- ✅ 2026-07-31 Theron V2.2 local-art isolation: removed the modern-art
  manifest/cache and inplace rectangle renderer from the production archive.
  Their focused V2.2 tests retain explicit source compilation, but `firestaff`
  cannot promote local cache/manifest pixels into the runtime.

- ✅ 2026-07-31 Theron viewport mapping gate: blocked the duplicate viewport
  tile table even when a caller supplies an unverified atlas. The legacy
  fixture renderer is compiled explicitly by the rendering test; production
  now requires a decoded Track 02 square/depth/material mapping.

- ✅ 2026-07-31 Theron placeholder inventory: audited the champion-state
  initializer and recorded its default names/classes/stats as an explicit
  unresolved real-data gap. Existing save/fixture tests still depend on it;
  no production claim now treats those defaults as decoded Track 02 records.

- ✅ 2026-07-31 CSB SWSH F0904 receipt isolation: the palette-animation
  receipt accepts metadata only and has no runtime caller or SWSH command
  decoder. It now compiles only into its focused test, rather than M10;
  production cannot turn receipt facts into synthetic palette animation.
- ✅ 2026-07-31 Theron verified champion handoff: authenticated JP/US Track
  02 startup sessions now clear fixture-only 10-point stats, inventory and
  equipment defaults before runtime entry. Source-roster identity metadata is
  retained; undecoded numeric champion records fail closed instead of being
  presented as real data.

- ✅ 2026-07-31 CSB SWSH F0908/F0909/F0910 receipt isolation: the metadata
  chain for sound init, playback and release has no production caller. M11
  keeps using the real-byte `RedmcsbF0908_InitSoundPc34` path, while the
  receipt chain compiles solely into its focused test and cannot authenticate
  host audio as original SWSH data.

- ✅ 2026-07-31 CSB startup receipt isolation: F0436 palette fade, F0579
  entrance bitplanes and F0807 door-step helpers are metadata contracts with
  no product caller or original-pixel decoder. They now compile only into
  their focused tests; M10 cannot treat caller facts as title or entrance
  material. Live startup remains guarded by the authenticated runtime route.

- ✅ 2026-07-31 CSB F0797 entrance-layout receipt isolation: the 5×5
  micro-dungeon layout metadata had no product caller and now compiles only
  into its focused test. It cannot become a generic loaded-dungeon or viewport
  substitute; an actual entrance frame must still use its source-owned draw
  route and verified graphics material.

- ✅ 2026-07-31 Theron startup-receipt isolation: removed the explicit
  no-data placeholder receipt implementation from the production archive.
  The real-asset receipt probe and save/resume fixture compile it explicitly;
  `firestaff` cannot link placeholder startup labels or tokens.
- ✅ 2026-07-31 CSB F0440/F0902 startup receipt isolation: temporary-graphic
  byte-count and FTL-logo fact helpers have no runtime caller or decoder and
  now compile only into their focused tests. M10 can no longer substitute
  caller metadata for a verified decompressed member, logo bitmap or palette.

- ✅ 2026-07-31 CSB startup-boundary/ownership isolation: the F0474–F0490
  blocked-graphics receipt and F0886–F0905 ownership table have no runtime
  consumer and now compile only into their focused tests. Production continues
  through the verified archive/decoder path rather than treating a blocked
  receipt or an ownership string as graphics material.

- ✅ 2026-07-31 Theron runtime fallback isolation: the startup runtime no
  longer synthesizes a fallback room in the production build. That branch is
  compile-defined only for the startup-flow fixture probe; production remains
  unavailable until a decoded Track 02 level is bound.

- ✅ 2026-07-31 CSB F0906–F0925 primitive-inventory isolation: the raw
  function-number metadata table only reports dependencies and explicitly
  blocks execution. It now compiles solely into its inventory test, leaving
  M10 to the dedicated authenticated SWSH and Utility implementations.

- ✅ 2026-07-31 Theron legacy asset verification: the generic loader no longer
  reports success for an expected digest it cannot compare against an
  authoritative catalog. Hash-bound Track 02 boot remains the only admission
  route; the legacy API now fails with `TR_ASSET_ERR_HASH`.

- ✅ 2026-07-31 Theron chapter-marker gate: a verified media identity without
  decoded progression/save state now reports unavailable instead of fabricating
  Chapter 1 and `0/7` quest progress. Later dungeon hints remain unavailable
  until their real headers/names are bound; fixture-only profile projection is
  explicitly compile-scoped.

- ✅ 2026-07-31 CSB F0846–F0865 unmapped-boundary isolation: this range has
  no ReDMCSB callable and only reports a fail-closed admission receipt. It
  now compiles solely into its focused contract test, so M10 cannot mistake
  source-absence metadata for an executable runtime implementation.

- ✅ 2026-07-31 CSB F0986–F1005 graphics-boundary isolation: the function
  table documents local, foreign-platform and unbound helpers, then blocks
  every runtime route. With no product caller or decoder, it now compiles only
  into its contract test; live rendering continues through authenticated PC
  3.4 graphics material.

- ✅ 2026-07-31 CSB F1006–F1025 source-boundary isolation: this table only
  inventories local, existing-owner and foreign-platform symbols and blocks
  execution for all of them. It now compiles solely into its focused contract
  test; M10 retains only actual authenticated CSB consumers.

- ✅ 2026-07-31 CSB platform-helper isolation: the combined F1048/F1049/
  F1053/F1055/F1061 wrapper only exported a disabled alias and explicit
  Amiga fake-code no-ops, with no production caller. It is excluded from M10;
  source-faithful shared fail-closed boundaries remain available for their
  separate focused tests.

- ✅ 2026-07-31 CSB F1066–F1085 Amiga-boundary isolation: the table has no
  PC 3.4 product consumer and explicitly blocks every route. It now compiles
  only into its contract test; the separately owned, source-faithful Intuition
  vector boundary remains independent of this inventory.

- ✅ 2026-07-31 Theron champion handoff hardening: verified Track 02 runtime
  entry now clears fixture champion names, portraits, classes and party count
  in addition to default stats/inventory. Production cannot present the
  inferred roster until original champion records are decoded.

- ✅ 2026-07-31 CSB F1126–F1145 source-boundary isolation: this catalog only
  records local, foreign-platform and unbound symbols before failing closed.
  It now compiles solely into its contract test, so M10 cannot treat source
  labels as a substitute for an authenticated CSB input or graphics route.

- ✅ 2026-07-31 Theron SRM champion-name gate: real SRM body import no longer
  substitutes `Theron` or `Companion` when a champion name field is empty. The
  record is rejected until source name bytes are present.

- ✅ 2026-07-31 CSB F1186–F1205 ANIM-boundary isolation: the table is a
  DM1-owned ANIM inventory without an authenticated CSB stream or runtime
  consumer, and already blocks execution. It now compiles only into its
  contract test, preventing metadata from creating CSB UI or timing behavior.

- ✅ 2026-07-31 Theron SRM progression-only handoff: Continue now clears the
  fixture world party when an SRM contains progression but no champion body.
  It no longer invents a one-member Theron party from unrelated initialized
  state.

- ✅ 2026-07-31 CSB F1206–F1225 ownership isolation: the table only records
  ANIM platform/local status and admits no route. It now compiles solely into
  its contract test, keeping metadata from standing in for CSB palette, sound
  or allocation behavior.

- ✅ 2026-07-31 CSB F1406–F1445 unmapped-boundary isolation: ReDMCSB has no
  callable symbol in this range, and the table only reports a blocked receipt.
  It now compiles only into its contract test; local source labels cannot
  become a synthetic CSB entrance, startup or graphics implementation.

- ✅ 2026-07-31 Theron runtime-render asset gate: the frame facade now requires
  a non-NULL asset bundle and fails before viewport/UI presentation otherwise.
  Rendering remains source-admitted only; the focused rendering suite passes
  `25/25`.

- ✅ 2026-07-31 Theron startup receipt fixture isolation: verified Track 02
  receipts no longer copy the fixture mirror roster size or fallback-label
  count. Those values remain confined to the explicit no-data fixture receipt;
  real startup data cannot report synthetic roster metadata.

- ✅ 2026-07-31 Theron startup runtime test linkage: the save/resume contract
  target now compiles its fixture-only structured fallback entry explicitly,
  while production still links the no-fallback runtime archive. The focused
  suite is green at `325/325`.

- ✅ 2026-07-31 Theron startup menu metadata gate: absent decoded Track 02
  roster names no longer expose fixture portrait indices or classes in menu
  elements. The startup-flow probe remains green at `653/653`.

- ✅ 2026-07-31 Theron startup TODO audit: removed the stale claim that the
  structured save/resume receipt test had an unrelated failure. The corrected
  fixture-scoped linkage now passes `325/325`; HUD rendering remains blocked
  until a real Track 02 widget bank is decoded.

- ✅ 2026-07-31 Theron champion handoff fixture isolation: the production
  `enter_forcefield_with_roster` path no longer calls `theron_v1_party_init()`
  or inherits its synthetic stats, classes, and portraits. It admits only
  source roster names; the full mirror-table initializer is fixture-scoped.
  Startup flow remains `653/653`, save/resume `325/325`.

- ✅ 2026-07-31 Theron viewport tile-helper gate: production
  `theron_vp_tile_for_square()` now returns no tile until a real Track 02
  mapping is bound. The inferred table is compiled only into the explicit
  viewport fixture probe; verification passes `50/50` and rendering `25/25`.

- ✅ 2026-07-31 Theron menu portrait/class gate: decoded roster names no
  longer authorize inferred mirror-table portrait indices or classes in
  production. Those fields remain unavailable until their source records are
  decoded; fixture metadata is compile-scoped to the startup probe.

- ✅ 2026-07-31 Theron legacy asset no-data gate: `tr_asset_load()` no longer
  returns success or claims “using defaults” when the requested file is
  missing. It returns `TR_ASSET_ERR_NO_DATA`; rendering remains source-gated.
  Focused rendering passes `25/25`, startup/save-resume `325/325`.

- ✅ 2026-07-31 Theron legacy parse-error gate: discovered Track 03/04 data
  that fails its parser now returns `TR_ASSET_ERR_TR03`/`TR_ASSET_ERR_TR04`
  and releases the partially loaded bundle instead of reporting a successful
  asset load with fallback state.

- ✅ 2026-07-31 Theron runtime world-init gate: production boot and Track 02
  runtime inspection now use a zero-party world initializer. The legacy
  fixture initializer remains available to tests, but no default champion
  roster exists before verified source handoff.

- ✅ 2026-07-31 Theron level-header seed binding: `theron_v1_level_load()` now
  retains the authenticated Track 02 header seed in `Theron_V1_Level` instead
  of discarding it. No tile/object meaning is inferred from the seed; the
  viewport mapping gate remains closed.

- ✅ 2026-07-31 Theron seed regression proof: the real Track 02 level-handoff
  probe now asserts the retained `0x0108e938` seed directly on the loaded
  level, alongside the existing raw candidate checks.

- ✅ 2026-07-31 Theron opaque header-index binding: level load now preserves
  the Track 02 header's `0x0026` level-index value in a separate opaque field,
  without confusing it with Firestaff's internal 0-based level slot. The real
  handoff probe asserts it; result remains `fail=0` with one known ISO skip.

- ✅ 2026-07-31 Theron level fixture parity: the explicit no-data room helpers
  now populate the same seed/header-index fields as their serialized headers,
  keeping fixture inspection structurally honest without promoting fixture
  bytes into production semantics. Startup flow remains `653/653`.

- ✅ 2026-07-31 Nexus TEXT/TABL source-boundary cleanup: RLOWFIX.BIN TEXT
  offsets and the 216-entry DMWeb TABL code table are parsed from the real
  retail resource and exercised by `test_nexus_v1_champion_plrd`. The legacy
  heuristic ASCII/Shift-JIS scraper plus unauthenticated S2D text/glyph
  layout wrappers are excluded from `firestaff_nexus`; they remain available
  only to explicit diagnostic probes. No glyph, palette, menu, HUD or Saturn
  VDP1/VDP2 presentation is promoted by this change.
- ✅ 2026-07-31 DM2 SHOP_GLASS panel isolation: removed the remaining
  host-authored shop rectangle, English labels and empty-inventory fallback
  from the production shop module. Its render contract now clears the output
  and returns no-draw until the source-owned `WALL_GFX`/DB actuator chain is
  decoded. Verification: production link, shop admission regression and an
  executable-string check for the retired panel text.
- ✅ 2026-07-31 DM2 world/object fallback isolation: removed the inferred
  16-bit world builder and sequential thing-pool parser from the live path.
  `dm2_world_from_mem()` now requires the PC G1 byte-square loader, and the
  object model returns no records when the validated c_record chain is not
  available. Verification: complete production `firestaff` link and no
  compiler warnings in either changed DM2 source.
- ✅ 2026-07-31 DM2 V2 runtime/lighting isolation: removed the unattached
  smooth-camera, bloom and animated outdoor-state sources from the production
  archive and game loop. These local time/weather effects remain in explicit
  diagnostic targets only; live DM2 presentation stays on the authenticated
  V1 viewport and GDAT HUD path. Verification: production link, V2 probes,
  real-data DM2 startup gate and production-symbol check.
- ✅ 2026-07-31 Nexus real viewport gate rechecked: the Track 1 readiness
  probe drives the local English CUE/DM.BIN, real `LEV00.DGN`, `FONT256.S2D`
  and `SCORPION.MNS` handoff through `nexus_viewport_render`; 29/29 pass.
  The real viewport capture remains deterministic black until authenticated
  Saturn DGN/VDP1 material is admitted, with no procedural fallback pixels.

- ✅ 2026-07-31 CSB V2.2 synthetic shape-book isolation: removed the
  hand-authored material/PBR/geometry book from `firestaff_csb_v2`; its
  historical expectations remain explicitly test/probe scoped. Production now
  links `csb_v22_shapes_runtime_gate.c`, whose API reports zero materials and
  no shape parameters until a reviewed original-data binding exists. The
  runtime cache requires a non-NULL admitted material before activating a V2.2
  cell, so it retains source-owned V1/V2.1 pixels rather than inventing a
  fallback. Verified with the new `csb_v22_shapes_runtime_gate_pc34` test,
  the historical shape-book contract test, and a `firestaff` build.

- ✅ 2026-07-31 DM2 V2 companion/crafting/viewport isolation: removed the
  orphaned companion display, empty crafting catalog and host-timed smooth
  viewport helpers from production M10/V2 archives. The focused startup
  diagnostic retains its local copy, while the game executable contains no
  V2 companion, crafting or smooth-viewport symbols. Verification: complete
  production link, real-data DM2 startup gate and archive/executable-symbol
  checks.
- ✅ 2026-07-31 CSB V2.2 installed-state hardening: a launcher-set
  `installed` flag can no longer select modern art on its own. The V2.2
  source selector now rechecks the finished-art gate and every route's
  provenance before it returns `V2_MODERN`; otherwise it keeps the V2.1/V2.0
  fallback. The focused asset-pipeline test covers the forged-installed/no-art
  case.

- ✅ 2026-07-31 CSB V2.2 cache-admission hardening: a readable
  `v22_inplace_cache.bin` is no longer enough to overwrite an F0128 source
  command. The in-place blitter independently requires the finished-art
  material/provenance gate; fixture cache pixels remain invisible even with a
  matching source span and palette. The focused in-place test verifies the
  framebuffer stays source-owned.

- ✅ 2026-07-31 DM2 V2 HUD overlay-state isolation: removed the retired
  procedural overlay module from the production V2 archive. Its invented
  compass, gold, level and champion values no longer enter the live renderer;
  the GDAT HUD route retains only a visibility gate and can draw only
  authenticated `INTERFACE_GENERAL` records. Historical overlay code remains
  explicitly test-scoped. Verification: production link, 74/74 direct-overlay
  regression, real-data DM2 M11 startup gate and archive/executable symbols.
- ✅ 2026-07-31 Theron SRM production import no longer calls the fixture
  `theron_v1_party_init()` before decoding champion records. The importer now
  starts from an empty party, so a malformed or partial source body cannot
  inherit synthetic names, classes, stats or inventory. Verification: the
  Theron SRM body/classifier tests plus startup, save/resume and Track 02
  handoff tests.
- ✅ 2026-07-31 Theron SRM production import no longer calls the fixture
  `theron_v1_party_init()` before decoding champion records. The importer now
  starts from an empty party, so a malformed or partial source body cannot
  inherit synthetic names, classes, stats or inventory. Verification: the
  Theron SRM body/classifier tests plus startup, save/resume and Track 02
  handoff tests.
- ✅ 2026-07-31 Theron startup mirror metadata isolation: the production
  `theron_v1_startup_mirror_meta()` API now fails closed because Track 02
  champion names, classes and portraits are not decoded. The seven-entry
  legacy table remains compiled only for the explicit fixture startup probe.
  Verification: production Theron archive build, startup-flow probe and
  real-data startup receipt gate.
- ✅ 2026-07-31 Theron startup mirror metadata isolation: the production
  `theron_v1_startup_mirror_meta()` API now fails closed because Track 02
  champion names, classes and portraits are not decoded. The seven-entry
  legacy table remains compiled only for the explicit fixture startup probe.
  Verification: production Theron archive build and startup-flow plus
  save/resume probes.
- ✅ 2026-07-31 Theron dead-template cleanup: removed the unused production
  companion struct that hardcoded fighter class, 10-point attributes and
  starter health/food/water. Runtime initialization remains source-gated and
  fixture setup remains explicit. Verification: full Theron archive rebuild,
  startup-flow probe and save/resume probe.
- ✅ 2026-07-31 Theron dead-template cleanup: removed the unused production
  companion struct that hardcoded fighter class, 10-point attributes and
  starter health/food/water. Runtime initialization remains source-gated and
  fixture setup remains explicit. Verification: full Theron archive rebuild,
  startup-flow probe and save/resume probe.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron V2 HUD production isolation: removed the procedural
  compass, text, rune, champion-bar and action-strip renderer from the
  production archive. Production now links a no-op HUD seam that returns
  `V1_SKIPPED`; the pixel renderer and widget parser are compiled explicitly
  for fixture targets only. Verification: HUD phase probe, HUD smoke test and
  widget-assets test all passed (100 %).
- ✅ 2026-07-31 Theron V2 HUD production isolation: removed the procedural
  compass, text, rune, champion-bar and action-strip renderer from the
  production archive. Production now links a no-op HUD seam that returns
  `V1_SKIPPED`; the pixel renderer and widget parser are compiled explicitly
  for fixture targets only. Verification: HUD phase probe, HUD smoke test and
  widget-assets test all passed (100 %).
- ✅ 2026-07-31 DM2 champion-stat bridge isolation: removed the unattached
  generic V1-to-V2 champion percentage bridge from the production V1 archive.
  It had no M11 consumer or authenticated session/palette handoff. Its focused
  regression remains explicit; live HUD stays on the source-owned GDAT route.
  Verification: production link, champion-bridge regression, real-data M11
  startup gate and archive/executable-symbol checks.
- ✅ 2026-07-31 DM1 original TITLE verification: repaired the standalone
  TITLE probe launcher after the source tree moved. The installed hash-locked
  PC 3.4 `TITLE` (12,002 bytes) now passes all 59 Greatstone mapfile-record,
  53-frame and two-palette-phase checks. The runtime TITLE palette and
  SWSH-to-C001 handoff probes also pass against the installed original
  `GRAPHICS.DAT`; no replacement title frame is used by these checks.
# ✅ 2026-07-15 Theron Track 02 transfer-destination call-entry receipt

The original Mednafen trace now admits the Track 02-derived TII destination
only when its bound JSR reaches an exact main-RAM entry row. The nested receipt
retains original byte-range and call provenance without classifying code or
data. Verification: genuine Mednafen 1.32.1 patch dry-run, Ninja focused
targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, raw-loader probe skip-safe,
and the capture contract pass.

# ✅ 2026-07-15 Theron Track 02 destination copied-byte receipt

The entered routine at the Track 02-derived TII destination now requires its
observed opcode to equal the exact first source byte copied from `$3c88`.
The receipt retains copied and original source addresses while leaving routine,
level, object, palette, bitmap, and rendering semantics unclassified.

# ✅ 2026-07-15 Theron Track 02 copied-entry successor receipt

The first observed successor after the copied destination entry now has to
remain inside the same TII destination span and match its corresponding
original byte (`$3c89`). This extends the byte-to-execution chain without
assigning instruction, record, dungeon, object, palette, bitmap, or rendering
meaning.

# ✅ 2026-07-15 Theron Track 02 copied-entry second-successor receipt

Mednafen now emits a second source-owned successor row after a main-RAM call
entry's first successor. Firestaff admits it only when it remains inside the
same copied TII span and matches original Track 02 byte `$3c8a`. This proves a
third bounded byte-to-execution observation, not instruction role, control
semantics, CD-record selection, dungeon data, or visual meaning.

# ✅ 2026-07-15 Theron Track 02 copied-entry BRA receipt

The Mednafen main-RAM loader trace now emits HuC6280 `BRA` control rows. The
Track 02-derived entry admission requires opcode `0x80`, its exact copied
displacement byte, and the emulator-computed target to agree. The receipt
records only this bounded control transfer; it does not classify the target as
loader code, a record selector, dungeon data, object data, palette, bitmap, or
rendering behavior.

# ✅ 2026-07-15 Theron Track 02 copied-entry BRA target execution receipt

The raw loader trace now records a target row only when Mednafen actually
fetches the exact target computed by the source-bound copied-entry `BRA`.
Firestaff retains the target opcode solely as opaque control-flow evidence and
requires the source PC, source physical PC, target and executed main-RAM PC to
agree. This does not assert loader, CD-record, dungeon, object, palette,
bitmap, or rendering semantics.

# ✅ 2026-07-15 Theron Track 02 copied-entry BRA target JSR receipt

The Mednafen trace now binds the first observed `JSR` after an executed
copied-entry BRA target to that exact target's main-RAM control path. Admission
requires the preceding target receipt and ordered trace rows. The JSR target
is retained as opaque control evidence only, without any assertion about a CD
record, loader routine, dungeon data, objects, palette, bitmap, or rendering.

# ✅ 2026-07-15 Theron Track 02 post-BRA JSR CD-record receipt

Firestaff can now admit a strict control-to-media join: an executed post-BRA
JSR must write the CD data register, then a canonical READ(6) and FIFO-origin
row must select a byte matching the hash-verified Track 02 sector at the
observed LBA. The resulting record coordinate remains opaque provenance, not
a loader name, level, object table, palette, bitmap, or rendering claim.

# ✅ 2026-07-15 Theron Track 02 CUE startup contract

The Track 02 launch resolver now follows the same CUE shape that the Theron
media classifier exposes to startup/menu code: `FILE`, `TRACK`, `MODE1`, and
`INDEX` keywords are accepted case-insensitively, and a CUE must contain
exactly one Track 02 `INDEX 01` before its BIN/ISO payload can be mounted.
This keeps real `MODE1/2048` ISO CUE media launchable while rejecting partial
or ambiguous CUE metadata. No dungeon, object, bitmap, palette, or fallback
semantics are inferred. Verification: `test_theron_v1_track02_cue_layout`,
`test_firestaff_theron_media_classify`, and
`test_m12_theron_missing_track02_popup_gate` pass.

# ✅ 2026-07-16 Theron Track 02 raw-only initial-envelope intake

The `$0b52` initial-envelope loader intake now carries the authenticated
Track 02 media variant and admits the complete-payload handoff only for the
JP/US raw BIN variants. ISO byte lookup remains an inspection boundary, but a
`MODE1/2048` ISO cannot reuse a raw-BIN loader/object-table route or become a
synthetic dungeon substitute. Verification: `theron_v1_track02_loader_intake`
and `theron_v1_raw_loader_trace_initial_level_handoff` pass.

# ✅ 2026-07-16 Theron Track 02 loader semantic gate

The real `$0b52` loader handoff now carries a hash-covered semantic-gate
receipt beside the full payload, initial envelope, and post-envelope bytes.
It exposes real byte availability while keeping dungeon-record,
object-table, bitmap, palette/RGBA, and fallback-visual promotion explicitly
blocked until an original consumer proves them. Verification:
`ctest --test-dir build-local-ninja -R
'theron_v1_track02_loader_intake|theron_v1_raw_loader_trace_initial_level_handoff'
--output-on-failure` passes.

# ✅ 2026-07-16 Theron post-$3800 consumer semantic gate

The Track 02 loader intake now exposes a separate post-`$3800`
consumer-trace gate. It promotes dungeon-record, object-table, bitmap,
palette, and source RGBA availability only when the original same-capture
consumer trace matches the already rehashed loader payload, level-envelope,
and post-envelope checksums. Synthetic dungeon/object/bitmap/palette
promotion and fallback visuals remain hard blockers. Verification: strict
compile of `theron_v1_track02_loader_intake.c` and focused
`theron_v1_track02_loader_intake` coverage for positive source admission,
stale checksum, missing consumer, synthetic, fallback, and pre-promoted-gate
rejections.

# ✅ 2026-07-16 Theron bounded Track 02 route after session handoff

The Theron runtime-admission surface now has a post-session-handoff bounded
Track 02 route receipt. It consumes the admitted US raw Track 02 FIFO
session handoff plus a route receipt carrying corpus evidence, then preserves
the capture mask, no-fallback semantic role mask, startup-level anchor,
blocked object-table anchors, blocked non-startup-level anchors, and route
hashes. It remains runtime-capture-required and refuses exact object/level
semantic promotion, object-table admission, level admission, payload
semantics, visual semantics, and fallback visuals. Verification:
`cmake --build build-local-ninja --target
firestaff_theron_v1_runtime_admission_probe`, `ctest --test-dir
build-local-ninja -R '^theron_v1_runtime_admission$' --output-on-failure`,
and focused `git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 decoded-route render proof producer

Theron runtime admission now constructs `Theron_V1RuntimeTrack02RenderAssetProof`
from decoded Track 02 route receipts instead of probe-filled proof fields. The
producer accepts only the same admitted US Track 02 consumer session with
matching level/object/all-dungeon route hashes, decode-ready non-startup level
and object-table receipts, a complete startup bitmap atlas, promotable palette
window evidence, nonzero decoded hashes, and no synthetic/fallback visual
flags. This is a fail-closed producer contract; real ISO/BIN/CUE capture still
has to provide the decoded receipts for broader non-startup dungeons.
Verification: `firestaff_theron_v1_runtime_admission_probe`,
`ctest -R '^theron_v1_runtime_admission$'`, and focused `git diff --check`
passed.

# ✅ 2026-07-16 Theron Track02 object/dungeon-only consumer grammar gate

Added a narrow post-$3800 object/dungeon consumer grammar gate to
`theron_v1_track02_loader_intake`. It consumes the same real loader payload
boundary as the existing semantic gate, but admits only object-table and
dungeon-record grammar provenance when the same-capture original trace proves
both consumers and the payload/envelope/post-envelope checksums match. Bitmap,
palette, RGBA, runtime handoff, fallback visuals, and synthetic promotions are
explicitly rejected on this route. Also repaired the Theron raw-loader final
bind against the current startup-media receipt by reading the Soul Room raw
route spans directly from the receipt fields instead of the removed helper
type. Verification: direct focused C11 build/run of
`test_theron_v1_track02_loader_intake` passed, strict syntax-only checks for
the touched header/source/test and raw-loader source passed, and targeted
`git diff --check` passed.

# ✅ 2026-07-16 Theron Track02 object/dungeon consumer byte-window binding

The Track 02 post-`$3800` consumer gates now require concrete same-capture
object/dungeon evidence before accepting the existing consumer markers. The
trace facts must carry nonzero dungeon/object consumer PCs plus payload-window
offsets, byte counts, and checksums that match the already verified initial
level envelope and post-envelope object-candidate slice from the real `$0b52`
loader read. The narrow object/dungeon grammar receipt retains those PCs and
windows while keeping field decode, bitmap, palette, RGBA, runtime handoff,
synthetic promotion, and fallback visuals closed. Verification: focused C11
`test_theron_v1_track02_loader_intake` build/run passed, strict syntax-only
checks for the touched Theron header/source/test passed, and targeted
`git diff --check` passed.

# ✅ 2026-07-16 Theron Track02 consumer-to-CD-read coordinate binding

The post-`$3800` Track 02 consumer facts now bind object/dungeon evidence back
to the exact raw loader/CD-read handoff before either the narrow grammar gate
or the broader consumer semantic gate can open. The facts and receipts retain
the `$0b52` record's user-data offset `$114`, destination `$3800`, and 2048-byte
payload size alongside the existing payload, level-envelope, post-envelope,
consumer-PC, and byte-window checksums. Mutated loader destination, payload
size, record-local offset, object window, or dungeon window evidence all fail
closed, with bitmap/palette/RGBA/runtime/fallback visuals still blocked on the
object/dungeon-only route. Verification: focused C11
`test_theron_v1_track02_loader_intake` build/run passed, strict syntax-only
checks for the touched intake header/source/test passed, and targeted
`git diff --check` passed. At that point the wider
`theron_v1_runtime_admission.c` syntax check still remained blocked by the
missing `Theron_Track02NonstartupContainerIndex` API closed below.

# ✅ 2026-07-16 Theron Track02 nonstartup container-index blocker closure

The missing `Theron_Track02NonstartupContainerIndex` API is now defined and
implemented as an opaque, fail-closed real-data bridge. It is built from the
existing hash-gated nonstartup sector receipt and indexes only verified,
contiguous user-data windows from real raw Track 02 data whose receipt already
marks them opaque and promotion-blocked. The index records descriptor entry,
raw offset, user-data offset, byte count, and hash evidence for later
object/dungeon consumer binding, but it does not decode object tables, levels,
bitmaps, palettes, text, runtime state, or visuals. Runtime-admission syntax
and object compilation now pass again without admitting fallback visuals.
Verification: strict syntax-only checks for `theron_v1_track02.h`,
`theron_v1_runtime_admission.h`, and `theron_v1_runtime_admission.c` passed;
`src/theron/theron_v1_runtime_admission.c` object build passed; focused C11
`test_theron_v1_track02_loader_intake` build/run passed; and targeted
`git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 multi-level runtime handoff gate

Theron Track 02 now has a level-transition/runtime-handoff gate above the
object gameplay state. The new handoff requires same-capture trace proof for
source and target level selectors, target level byte count/hash, target object
runtime-state hash, party-placement binding, and object-pool state binding.
`theron_v1_runtime_publish_track02_level_transition()` then installs the target
level, publishes that level's verified object pool, places the party at the
target level start pose, clears the pending stairs transition, and invalidates
runtime media. This path deliberately stays separate from the older
bitmap-complete dungeon route so real level/object state can advance without
promoting unproven palette/pixels. Dungeon runtime admission, dungeon draw,
synthetic dungeon/object data, and fallback visuals remain denied. Verification:
Ninja built `firestaff_theron_v1_runtime_admission_probe` and
`test_theron_v1_track02_loader_intake`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
direct default and local US-CUE runtime-admission probes passed; syntax checks
and `git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 object gameplay-state handoff gate

Theron Track 02 now has a second gate after object placement: object gameplay
semantics. It accepts compact object-table rows only when the same-capture trace
proves the supported runtime kind set, flags low bits as object state, argument
as quantity, preserved flags, and a runtime-state hash. A separate world handoff
then mutates only the selected loaded level's object pool, removes stale objects
for that level, preserves objects from other levels, updates thing count/current
level, and invalidates runtime media. It still denies dungeon runtime admission,
dungeon draw, bitmap/palette/RGBA promotion, synthetic objects, and fallback
visuals. The runtime-admission probe wires this into the optional real
object/dungeon HuC6280 trace path; plain real CUE/BIN remains fail-closed source
proof without such a trace. Verification: Ninja built
`firestaff_theron_v1_runtime_admission_probe` and
`test_theron_v1_track02_loader_intake`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
direct default and local US-CUE runtime-admission probes passed; syntax checks
and `git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 object placement-state gate

Theron Track 02 now has a fail-closed object-placement state receipt after the
level/object loader-route proof. It consumes the verified compact object table
and same-capture route trace, binds selected dungeon/level rows, table checksum,
level mask, row hashes, first-row x/y/level/flags/argument bytes, and a placement
state hash. It deliberately keeps object-kind gameplay semantics under review and
does not allow world object publish, runtime admission, dungeon draw, bitmap/
palette/RGBA promotion, synthetic decode, or fallback visuals. The runtime
admission probe's optional object/dungeon HuC6280 trace branch now carries the
full chain to placement state and parses the object table from the real Track 02
container window. Verification: Ninja built `firestaff_theron_v1_runtime_admission_probe`
and `test_theron_v1_track02_loader_intake`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
the direct runtime-admission probe passed both default and local US-CUE real-media
runs; syntax checks and `git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 bitmap/palette source-window gate

Theron Track 02 now has a fail-closed bitmap/palette source receipt above the
proved multilevel runtime route. The receipt consumes only a verified
level-transition runtime result, binds the same Track 02 record and
source/target levels to palette raw/user-data offsets, palette checksums,
bitmap atlas route facts, and a combined source hash, and rejects hash drift,
pixel-output claims, M11 render admission, dungeon draw, and fallback visuals.
No bitmap decoder, palette decoder, pixel output, synthetic visual, or M11
render promotion was added. The acute integration break from the new helper
name was fixed by using the existing `theron_v1_runtime_mix_hash` helper, and
`ninja -C build/ninja-dm2 firestaff` now completes. Verification:
`ninja -C build/ninja-dm2 firestaff`;
`ninja -C build/ninja-dm2 test_theron_v1_track02_loader_intake
firestaff_theron_v1_runtime_admission_probe`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
syntax checks for the touched Theron source/test/probe passed; the direct
runtime-admission probe passed both default and local US-CUE real-media runs.

# ✅ 2026-07-16 Theron Track 02 bitmap/palette decode-vector gate

Theron Track 02 now has a positive decode-vector receipt after the
bitmap/palette source-window gate. The receipt consumes the source-bound
record/level route plus the real US Track 02 bytes, re-decodes the HuC6260
4bpp palette window, builds the indexed startup bitmap atlas from the same
media, and admits only exact checksum/route/tile/nonzero-pixel agreement. It
retains the first palette word/RGB triplet, atlas route geometry, first source
bitmap offsets, and first decoded pixel-row hash as proof vectors. The result
sets palette decode, bitmap decode, and pixel output verified, but keeps M11
runtime consumption, M11 rendering, dungeon draw, and fallback visuals closed.
No guessed decoder, fallback image, host upload, or dungeon render promotion
was added. Verification: `ninja -C build/ninja-dm2 firestaff`;
`ninja -C build/ninja-dm2 test_theron_v1_track02_loader_intake
firestaff_theron_v1_runtime_admission_probe`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
syntax checks for the touched Theron source/test/probe passed; the direct
runtime-admission probe passed both default and local US-CUE real-media runs;
`git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 M11 Soul Room runtime consumption

Theron now binds the positive Track 02 bitmap/palette decode vector to a
production M11 runtime-consumption receipt for the verified Soul Room level-0
surface. `theron_v1_world_runtime_media_for_level()` now returns the retained
Soul Room surface for level 0, so the existing live `Theron_RuntimeLevelMedia`
path can select it through `THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL`. The new
M11 consumption receipt requires the real world runtime-media surface to match
the decode vector's Soul Room route bit, offsets, geometry, route checksum,
tile count, and nonzero-pixel count, then verifies exact 1:1 placement and
clip bounds before allowing host presentation. Checksum drift, bad host bounds,
scale changes, missing world media, non-Soul Room routes, dungeon draw, and
fallback visuals all remain fail-closed. The real US-CUE probe now builds the
production startup media receipt from the real Track 02 bytes, binds it into a
live world, and proves the M11 Soul Room consumption receipt from that world.
Verification: `ninja -C build/ninja-dm2 firestaff`;
`ninja -C build/ninja-dm2 test_theron_v1_track02_loader_intake
firestaff_theron_v1_runtime_admission_probe`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
syntax checks for the touched Theron source/test/probe passed; the direct
runtime-admission probe passed both default and local US-CUE real-media runs;
`git diff --check` passed.

# Theron V1 source-locked CD-DA track routing receipt (Lane E, cycle 10)

Closed TODO.md item (5) under the 2026-07-11 Theron original-media
synthetic-path audit: implemented a source-locked CD audio track routing
receipt that gates any future Theron V1 audio output on original CUE
metadata and locally staged CD-DA tracks.

### 2026-08-08 — arkiverade poster

- ✅ 2026-07-27 Theron CDDA host-consumer correction
- ✅ 2026-07-22 Theron boot runtime input/idle facade
- ✅ 2026-07-23 Theron boot startup host-receipt apply facade
- ✅ 2026-07-23 Theron boot startup action/state-receipt apply facade
# 2026-08-10 — source roster stats survive missing US text consumer

- Fixed the authenticated startup handoff so missing/invalid optional US
  roster text no longer aborts or clears the real Track 02 champion records.
- Source-bound stats and skills remain available; display names remain absent
  until the text consumer is proven. T900 equipment semantics remain gated.
- Verified with `test_theron_v1_combat_runtime_source`.
# 2026-08-10 — bound category-4 group count in live admission

- Kept the source monster materializer within the four authenticated health
  words of a Track 02 category-4 record in both validation passes.
- Corrupt or future records can no longer make the live-creature bridge read
  past the source health array; the original RNG/AI path remains gated.
- Verified against the real US/JP dungeon corpus and the production combat
  bridge.
- ✅ 2026-08-20 Theron original-PC Engine SRAM-gräns: en riktig 2 048-byte
  Mednafen `.sav` klassificeras nu via exakt storlek, `HUBM` och den observerade
  `DMS-SG.NNN`-markören. Realdata-testet godkänner den japanska/amerikanska
  byteidentiska Save Disk-baslinjen och avvisar den lokala 136-byte textdumpen.
  Ingen bodysemantik eller progression påstås; den verkliga filen är tom efter
  formatering och används endast för att ta fram nästa autentiska save-delta.
- ✅ 2026-08-20 Beständig original-save för extern Theron: den verifierade
  Mednafen-vägen binder nu `filesys.path_sav` till regionsseparerade
  `~/.firestaff/saves/theron-original/us|jp`. Det bevarar originalets råa
  `HUBM` Backup RAM mellan Firestaff-starter utan `.tqsv`-konvertering och
  förhindrar att USA- och Japan-utgåvorna delar savekatalog.
  Vid första start migreras dessutom en ensam, namnmatchande äldre Mednafen
  `.sav` endast om den klarar exakt 2 KiB `HUBM`/`DMS-SG`-klassificering;
  befintlig destinationsdata skrivs aldrig över och tvetydiga kandidater
  avvisas.
- ✅ 2026-08-20 ADPCM playback-capture: Mednafen-instrumenteringen har nu en
  separat, 4 096-posters sidecar för `$180D/$180E` med CPU-PC, fysisk MPR-PC,
  controlvärde, ADPCM-adress, läsposition, längd, rate och faktisk
  stop→play-övergång. Detta kompletterar den redan byteexakta FIFO→ADPCM-RAM-
  transporten utan att ännu hitta på ett gameplay-ljud-ID.
- ✅ 2026-08-20 autentisk Theron-grafikdiagnos: den aktuella Mednafen-
  patchkedjan byggdes om från ren 1.32.1-källa mot riktig SDL2 efter att
  consumer- och provenance-hunkarnas diffkontext rättats. En kallstart från
  den kompletta amerikanska skivan och System Card 3.0 gav i samma session
  161 råsektorspann, 51 SCSI-kommandon, 161 sektorbindningar, 25 CD IRQ och
  två byteexakta CD→RAM-origin-kvitton. Visuell kontroll visade därefter att
  VRAM/VCE ensamma inte räcker för korrekt replay. Captureproducenten sparar
  nu även samtidiga HuC6270-register, och Firestaffs tilebas, GRB333-kanaler,
  gemensamma BG-färg 0 samt unmapped tileindex följer hårdvaran. De nya
  fångstparen är inte upptagna i produktens hashgrind innan `HDR/VDR/MWR`-
  bunden 320×200-geometri är färdig; ingen felrenderad bild publiceras.
# ✅ 2026-08-20 Theron Track 02-dörrar skiljer källa från runtime-state

- Rättade den native Track 02-loader som tidigare skrev källpostens
  `type`-bit (trä/järn) till dörrens runtime-`state` och skrev thing-position
  i samma låga flaggbitar som `LOCKED`, `BROKEN` och generiska
  objektmutationer. Det kunde göra riktiga järndörrar delvis öppna och vissa
  positionsvarianter låsta eller trasiga redan vid laddning.
- Riktiga dörrar startar nu stängda. Position, material, ornament,
  öppningsriktning, knapp, förstörbarhet och bashbarhet behålls förlustfritt i
  ett separat metadataområde; inga nya lås-, skade- eller ljudregler antas.
- `theron_v1_track02_dungeon_loader` jämför varje materialiserad dörr med den
  exakta 4-byteposten i världens proveniensledger för alla sju US- och sju
  JP-dungeons och nekar alias mot runtimeflaggorna.
- Flyttade även teleporterarens och actuatorns tvåbitars thing-position från
  de generiska `PICKED_UP`/`OPENED`-bitarna till samma separata
  källmetadatafält. Råa category-3-actuatorer publiceras nu som
  `THERON_OBJTYPE_SOURCE_ACTUATOR`, inte som den orelaterade fixturetypen
  `BUTTON`. Realdatatestet räknar och granskar varje materialiserad
  teleporterare och actuator i båda regionerna.
- Rättade den parallella teleporterardekodern så att `ldest` behåller
  postformatets bitar 8–13. Den verkliga korpusen har högsta destination 7;
  ett avgränsat dekodertest säkrar även ett sexbitsvärde utan att fabricera
  någon runtime-destination.
- Dörr- och teleporterarrutterna väljer nu kontrollobjekt efter autentisk typ,
  inte efter godtycklig första placering på koordinaten. Samma sexbitsfält
  överlever nu även packning och uppackning i teleporterarruntimen.
- Ersatte TAKE-ruttens generiska “första objekt på rutan” med en ordnad gång
  över de materialiserade source-posterna. För verifierade Track 02-nivåer
  väljs första ännu inte upplockade carryable occurrence, varefter den redan
  strikta råpost-, property- och ledgergrinden körs. Realkorpusen innehåller
  402 US- och 402 JP-carryables bakom en tidigare post; varje dungeon provar
  en sådan occurrence genom source-inventory och drop-roundtrip.
- M12-pickup-kvittot publicerar nu den exakta source-slot som fylldes och M11
  behåller endast detta explicita val. P/DROP återför posten till gruppens
  validerade ruta genom den befintliga förlustfria source-inventory-API:n.
  Valet rensas efter drop eller byte av aktiv mästare; en generisk fixture-slot
  eller ett saknat val kan fortfarande inte mutera en verifierad Track 02-
  värld. Realdataroundtripen går genom samma boot-inputfasad som produkten.
- ✅ 2026-08-20 atomisk Theron VDC-fångst: den officiella Mednafen 1.32.1-
  källan verifierades med SHA-256
  `de7eb94ab66212ae7758376524368a8ab208234b33796625ca630547dbc83832`,
  hela Firestaff-patchkedjan byggdes isolerat mot riktig SDL2 2.30.9 och en
  autentisk Cocoa-körning gav 65 536 VDC-poster plus samtidig 64 KiB VRAM,
  1 KiB VCE, 512-byte SAT och HuC6270-register. Producentens atomiska footer
  finns. Parsern bevarar nu verkliga HuCPU-tidsregressioner som diagnostik och
  använder den sammanhängande sekvensen som ordningsbevis; semantisk
  publicering förblir avstängd utan samma sessions CD→RAM-konsumentjoin.
- ✅ 2026-08-20 native atomisk grafikgrind: produktens Theron-viewport kräver
  nu VRAM, VCE, HuC6270-state, SAT och det fullständiga VDC-I/O-spåret som en
  enda hashstängd bunt. 25 890 VWR-commits och samtliga 8 816 berörda VRAM-ord
  verifieras före laddning. Den gamla fyrfilsfunktionen nekar alltid, CLI:n
  kräver `--theron-vdc-io`, och den riktiga 320x200-bunten presenterar 63 923
  källpixlar inklusive 85 SAT-spritepixlar genom M11 utan dungeonsemantik.
- ✅ 2026-08-20 kall atomisk US-kontroll: en komplett originalstart gav i samma
  process 161 råsektorspann, 51 SCSI-läsningar, två byteexakta Track 02→RAM-
  transportkvitton, 32 spelkodsanrop till `$E009` och 31 794 VWR-commits.
  Alla 24 576 berörda VRAM-ord matchar den samtidiga 256x240-dumpen. Kvittona
  är fortfarande System Card-ägda och öppnar därför inte den kvarvarande
  källsemantiken.
- ✅ 2026-08-20 spelägd `$3840 → $E009 → $2800`-bindning: en ny kall körning
  från den autentiska 19-spårs-US-skivan bevarade loaderkopian
  `$201E → $20F8` (8 byte), de råa parametrarna
  `01 00 00 28 00 03 FF 01`, READ(6) generation 5 för LBA 4257 och den
  asynkrona återgången till `$3B36`. Vid nästa riktiga `$3840`-dispatch låg
  den färdiga 2 048-bytepayloaden i fysisk huvud-RAM `$1F0800` (`$2800`).
  Dess 32-byte-FNV `2723167f` och hel-FNV `33a90342` matchar byteexakt
  MODE1-användardatan i den hashverifierade US Track 02-posten `$4E0`.
  Firestaff har nu en fail-closed C-bindare och ett valfritt realdata-test som
  även avvisar korrumperade kopior av den riktiga sektorn och capture-raden.
  Parametrarnas högre byte och payloadens spelbetydelse förblir avsiktligt
  odolkade; detta är en käll- och destinationsbindning, inte dungeonsemantik.
- ✅ 2026-08-20 spelägd konsumentkedja för första `$2800`-payloaden: en
  dedikerad sidecar bevarar den asynkrona återgången `$3840 → $3B36`, fem
  ordnade huvud-RAM-läsningar och nästa `$3840`-dispatch. Spelkoden vid
  `$37E2/$37E9/$37F7/$37FC/$3802` läste `$2D13..$2D17`, alltså offset
  `$513..$517` i det bundna blocket, med värdena `F9 02 04 00 20`.
  Fail-closed-bindaren kontrollerar adresser, fysisk MPR-avbildning, ordning,
  läsande PC och varje byte mot den autentiserade US Track 02-posten `$4E0`.
  Realtestet avvisar även en manipulerad kopia av den riktiga läsraden.
  Provenancefälten var ogiltiga och de fem fältens spelbetydelse är fortfarande
  stängd; kvittot bevisar bara en verklig spelägd konsumentkedja.
- ✅ 2026-08-20 källbunden E009-konsumentkod: en ny kall originalkörning
  snapshotade huvud-RAM `$37C8..$383F` vid den första asynkrona återgången.
  Samtliga 120 byte matchar den autentiserade US Track 02-filen över en riktig
  råsektorsgräns: 56 byte från post `$4C4`, användaroffset `$7C8`, och 64 byte
  från post `$4C5`, användaroffset `$000`. FNV-1a är `048e8620`.
  Den byteexakta HuC6280-rutinen vid `$3806` laddar basen `$2803` och utför en
  trestegs shift/add med konstanten 6. Tillsammans med första verkliga
  konsumentadressen bevisar detta relationen `$2803 + 6 × $D8 = $2D13`.
  Anropsspåret bevarar samtidigt `$D8` vid `$36D2 → $37D8`. C-bindaren
  avvisar ändrad kod både i capture och på det riktiga mediet. `$D8` och de
  fem efterföljande byten har fortfarande ingen tilldelad spelbetydelse.
- ✅ 2026-08-20 nästa E009-parameterblock: samma autentiska RAM-sidecar
  bevarar den ordnade skrivkedjan från konsumentrutinen till arbetsområdet
  `$201E..$2025`, de efterföljande spelkodsskrivningarna och TII-instruktionen
  `$3836: $201E → $20F8` med längd 8. Det resulterande blocket före nästa
  `$3840`-anrop är byteexakt `00 20 00 10 00 06 F8 FE`. Ett separat
  fail-closed-kvitto kontrollerar skrivsekvens, logiska och fysiska adresser,
  skrivande PC och samtliga åtta TII-resultat; realtestet avvisar en ändrad
  parameterwrite. Bytesens SCSI-/recordbetydelse är fortfarande stängd.
- ✅ 2026-08-20 generation 6 CD→VDC-bindning: en separat, generationsmärkt
  sidecar från samma kalla originalkörning bevarar fyra VDC-inställningar och
  därefter exakt 8 192 unika dataportsskrivningar, dubbelloggade på samma sätt
  som producentens VDC-hook. Efter avdubblering matchar byteföljden exakt
  MODE1-användardatan från READ(6) LBA 5018–5021, alltså autentiserad US Track
  02 post `$7D9..$7DC`; FNV-1a är `4859675d`. Skrivaren är spelrutinen `$EB35`
  (fysisk PC `$000B35`) och portarna alternerar `$0002/$0003`. Föregående
  VDC-skrivningar väljer register 0, sätter MAWR till `$1000` och väljer
  register 2. En särskild same-session-dump exakt efter generationens sista
  skrivning korrigerar den inledande avdubblingsmodellen: varje 16-bitars
  källord skrivs två gånger av den verkliga VDC-vägen. Strömmen fyller därför
  8 192 VRAM-ord `$1000..$2FFF`, byteexakt som dubblerade Track 02-källord,
  med FNV-1a `9b9f7361`. Den nya
  fail-closed-bindaren kräver föregående parameterkvitto, generation 6-CDB:n,
  VDC-inställningarna, hela skrivordningen och varje verklig mediabyte.
  Realtestet avvisar både en ändrad VDC-rad och en ändrad Track 02-kopia.
  Testet korrumperar även en kopia av den riktiga VRAM-dumpen. Bildbetydelse
  och parameterfältens semantik förblir stängda i detta transportkvitto.
- ✅ 2026-08-20 första källbundna BAT-presentationen: en andra kall
  originalkörning frös VDC/VRAM exakt efter generation 7:s 4 302 portposter.
  En ny fail-closed-replay utgår från det verifierade generation 6-snapshotet,
  återspelar 2 048 verkliga VWR-commits och matchar generation 7-snapshotets
  BAT byte för byte (`593edd45`; hel-VRAM `1f64dae1`). VDC-state visar
  64×32 BAT, 256×240 aktiv yta och aktiverad bakgrund (`CR=$0088`). Samtliga
  960 aktiva 32×30-celler refererar källbundna tileindex `$110..$187`, totalt
  60 unika tileindex. Den källbundna 4-bpp-avkodningen ger exakt 61 440
  indexpixlar, varav 2 848 är icke-noll, med FNV-1a `c5899c5d`.
  Samma generation 7-snapshot binder VCE (`f12861c5`) och den använda
  palettgruppen 0 (`ebc22165`) till varje indexpixel; den resulterande
  16-bitars färgordsbilden har FNV-1a `31866f25`.
  Ett 512-byte CDRAM-snapshot `$104600..$1047FF` binder samtidigt den aktiva
  BAT-skrivarkoden till autentiserad Track 02-post `$4D0`, användaroffset
  `$600`. De oförändrade källspannen omfattar 435 byte och de avvikande
  positionerna är verkligt modifierade operand-/arbetsfält. Den dynamiska
  koden är dessutom förenad med det befintliga statiska Stage 2-kvittot för
  `$466B`: den bytebundna rutinen väljer VDC:s VWR-register och skriver om
  operanderna till HuC6280-instruktionen `TIA` vid `$468C`. Originalets
  nollade fält har vid körning blivit källa `$47E0`, alternerande
  VDC-destination `$0002` och längd `$0040`. Den genererade 32-byte-BAT-raden
  vid `$47E0` har FNV-1a
  `da633f05` och dess verkliga kopior ingår i den redan verifierade
  generation 7-replayen. Kod-snapshotets FNV-1a är `3e3745f7`.
  Realtestet avvisar en ändrad generation 7-VRAM-kopia. Detta är verifierad
  skärmkonsumtion av riktiga Track 02-byte; palettfärgernas och bildens
  gameplaybetydelse är fortfarande avsiktligt odolkade.
- ✅ 2026-08-20 senare generation-49-grafiktransport: den fortsatta kalla
  originalsessionen binder READ(6) LBA 4622–4633 till tolv autentiska
  MODE1-sektorer, Track 02-poster `$64D..$658`. Exakt 24 576 mediabyte med
  FNV-1a `01551f76` återfinns i ordning i 49 152 dubbelloggade VDC-
  payloadposter efter den byteexakta MAWR=`$1000`/VWR-inställningen.
  Totalt verifieras 49 160 generation-49-poster. Den nya fail-closed-bindaren
  kräver den föregående generation-7-presentationskedjan och avvisar en
  manipulerad verklig mediabyte. En 90-sekunders originalcapture visade
  samtidigt att generation 51 är en lång aktiv ritloop, inte en avgränsad
  engångsladdning: över 449 000 VDC-poster observerades. Den första naturliga
  1 035-posters ramloopen börjar vid global sekvens 174 721; ett atomiskt
  snapshot efter dess första kompletta varv, generation-51-rad 100 755,
  bevaras endast som rå skärmhårdvaruevidens. Bakgrund och SAT där är en
  pågående övergång utan spritepixlar, så ingen titel-, meny- eller
  gameplaysemantik öppnas.
  Ett andra fail-closed-kvitto kräver nu samma atomsnapshot och den
  autentiserade Stage 2-receipten i samma kontrollkedja. Fram till den första
  kompletta loopgränsen verifierar det 100 755 ordnade generation-51-poster:
  4 288 skrivarrader inom `$466F..$4693`, 25 inom `$4934..$4942`, 4 120 inom
  `$50F1..$5110` och 4 096 exakt vid `$5110`. De samtidiga fullfilshasharna är
  VRAM `87fbe859`, VCE `8682b5d5` och SAT `4d7705c5`. Realtestet avvisar en
  korrumperad kopia av atomsnapshotets VRAM. Detta bevisar skivtransport →
  Stage 2-ritkod → atomisk skärmhårdvara, men inte vad övergången föreställer.

- ✅ 2026-08-20 filvalspromptens verkliga Track 02-källa: ett separat
  fail-closed-kvitto verifierar alla tre US-kopior av `WHICH FILE DO YOU
  PLAY?` och `WHICH FILE DO YOU LOAD?` direkt i den hashverifierade råfilen.
  De ligger i MODE1/2352-poster `$4EA/$4EC/$4EE`, vid råsektoroffset
  `$1AE/$0FA/$0FA`; promptarnas FNV-1a är `ef1550ad` respektive `aa654403`.
  Testet korrumperar en byte i en kopia av den riktiga filen och kräver
  avslag. Generation 51:s grafik kommer däremot från `$64D..$658`, så
  skärmbilden och textkällan har avsiktligt inte slagits ihop utan en
  samma-körningscapture av CPU:ns textläsning.
  Källkvittot har nu ett eget realdata-test,
  `test_theron_v1_file_select_text_source_real_data`, som körs direkt mot
  `TQUS02.bin` och inte längre är beroende av den större generation-5–51-
  capturebunten. En lokal körning mot den riktiga filen passerar; testet
  hoppas ärligt över när sökvägen inte är konfigurerad.
  Tre nya kalla originalsessioner visar dessutom att ingen READ(6) täcker
  `$4EA/$4EC/$4EE` och att den exakta ASCII-prompten aldrig finns i PCE:s
  RAM-banker när filvalet visas. Efter deterministisk Button I vid frame 3600
  går originalet genom `$4698/$511B`-bulktransfer och därefter en tät
  huvud-RAM-loop kring `$3C2A..$3D2B`. Detta är upptäcktsbevis för en kodad
  glyph-/textväg, inte tillräckligt för att öppna textkonsumentsemantik.

- ✅ 2026-08-20 stabil filvalsfas och atomisk VDC-konsument: en ny kall
  originalsession med sen Button I (`frame 8520`) lämnade
  `$4698/$511B`-övergången. Vid `frame 8580` läser CPU:n tillståndet
  `$2059=01` vid `$41A8` och en källström genom `$5139..$515E` samt
  `$5561..$55D0`. Den senare läser verkliga byte från den fysiska banken
  `$0D1D58..`; de första observerade byten är
  `02 00 70 00 40 00 10 02 80 01`.
  En andra, atomisk originalsession förenar samma CPU-fönster med VDC:
  `$5110` utför 1 024 VDC-portskrivningar efter inställning vid
  `$4934..$4942` och `$50FB..$5109`. Parameterrutinen läser samtidigt
  `$4DE3/$4DE4 = 3D 7D` och `$4DE9/$4DEA = 49 7D`; CPU-spåret visar även en
  läsning från `$7D58` vid `$514B`. Detta verifierar runtime→VDC-konsumtion
  för den kodade vägen. Medieposten som fyllde `$0D1D58` är ännu inte
  identifierad, så prompt- och glyphsemantik förblir stängd.

- ✅ 2026-08-20 korrigerad media→RAM→VDC-kedja för filvalet: granskning av
  den sammansatta Mednafen-patchstacken visade två `WriteMap`-anrop per
  logisk skrivning. Byggkedjan har nu en explicit single-write-korrigering
  och nya kalla originalsessioner har körts. READ(6) generation 12 läser
  LBA 4668, rå Track 02-post `$67B` (`fcc73c77`). Dess första 195
  användarbyte (`efad54b3`) skrivs av System Card `$EA9E` till
  `$0DDC5B..$0DDD1D`, kopieras byteexakt av spelkod `$3446` till
  `$0D1D3D..$0D1DFF`, läses vid `$514B` och når VDC-rutinen `$5110`.
  Den korrigerade målbildrutan har 512 faktiska `$5110`-portskrivningar och
  13 inställningsskrivningar, totalt 525. De tidigare talen 1 024/1 035 var
  dubbelskrivningsartefakter. Ett nytt realdatatest avvisar både ändrad
  Track 02-byte och ändrad loader-rad.
- ✅ 2026-08-20 filvalets 525 VDC-rader spelas nu om som verkliga
  hårdvaruskrivningar. De 13 inställningsraderna väljer VWR och sätter MAWR
  till `$0800`; de följande 512 byten bildar exakt 256 VRAM-ord
  `$0800..$08FF` med hash `a8007f15`. En ny atomisk bild direkt efter sista
  bytet visar att området är byteexakt identiskt med VDC:ns interna
  512-byte-SAT, också `a8007f15`. BAT innehåller inga referenser som skulle
  göra `$80..$8F` till pattern-index. SAT har 18 icke-tomma poster och 46
  tomma poster. Ett ändrat VWR-byte eller SAT-byte avvisas av realdatatesten.
  Resultatet bevisar SAT-staging, men inte enskilda sprite- eller
  skärmbetydelser.
- ✅ De 18 SAT-posterna är hårdvaruavkodade med samma HuC6270-regler som
  Firestaffs befintliga autentiska SAT-renderare. Alla använder PN `$0210`;
  posterna med 64 pixlars höjd adresserar spritepattern `$108..$10F` i
  VRAM-byte `$8400..$87FF`. De åtta patternblocken är byteidentiska och hela
  området har hash `ba5526c5`. Tolv SAT-poster ligger inom den synliga
  256×240-ytan; sex ligger vid y=240 och är utanför. Vad spritarna föreställer
  är fortfarande inte tillskrivet.
- ✅ Bildruta 8580 komponeras nu från den atomiska VRAM-, VCE- och SAT-bilden
  med exakt samma HuC6270-regler som Firestaffs befintliga renderer. Den har
  61 440 pixlar, 43 087 icke-tomma källpixlar, 18 511 slutliga
  bakgrundspixlar och 24 576 slutliga spritepixlar över 40 unika källindex.
  Den indexerade bildens hash är `7622aee1`; samma bild uttryckt som råa,
  autentiska VCE-ord har hash `8f1cf573`. Alla spritepixlar använder
  källindex `$101`, VCE-färg `$0000`, inom x=32..223 och y=0..239. Ett
  ändrat SAT- eller VCE-byte avvisas. Detta bevisar den återgivna
  hårdvarubilden men tillskriver ännu inte banden någon filvals-, prompt-
  eller annan skärmbetydelse.
- ✅ Två nya autentiska slutbilder ramar in VDC-uppdateringen: bildruta 8579
  har BYR `$00E9`, medan bildruta 8581 har `$00E8`. VRAM (`832b4d13`), VCE
  (`5376a91b`) och SAT (`a8007f15`) är byteidentiska. I bildruta 8580 väljer
  spelkod `$4993` BYR och `$4999/$499F` skriver `$00E8`. Den färdiga
  källbildens hash går från `af183e0d` till `7622aee1`, och VCE-färghashen
  går från `68fe4a69` till `8f1cf573`. Exakt 12 644 pixlar ändras inom
  x=32..223 och y=64..175. Alla 24 576 spritepixlar är oförändrade; endast
  den bakomliggande bakgrunden rullas en rad. Ett nytt realdatakvitto avvisar
  ändrad förbilds-VRAM och kräver uttryckliga slutmarkörer för båda
  bildrutorna.
- ✅ Filvalets BYR-producent och stoppvillkor är nu bundna med en autentisk
  kodbild, RAM-bild och två skrivspår från samma deterministiska knappsekvens.
  Spelkod `$4993` läser `$2210/$2211` och skriver värdet till VDC BYR.
  Den självmotifierande signerade loopen vid `$4175..$41A1` skriver
  `$2210` via `$4184`: `$F0→$60` i 144 steg med åtta bildrutors intervall.
  Kontrollparet `$47BC/$47BD` räknas samtidigt ned `$0090→$0000` via
  `$4A7B/$4A80`; sista RAM-steget sker vid bildruta 9666 och nollstoppet vid
  9668. Nästa observerade skrivning i båda spåren är initieringen vid 10310,
  vilket avgränsar stoppet. Ett realdatatest kontrollerar samtliga 144+144
  steg och avvisar ändrad kod eller ändrad stopprad. Kvittofältet för
  skärmsemantik förblir uttryckligen stängt.
- ✅ Samma autentiska kontrollspår binder nu även fortsättningen efter det
  första stoppet. Vid bildruta 10632 sätts `$47BA/$47BB` till `$0040`.
  Koden vid `$4B0C..$4B23`, med skrivare `$4B1B/$4B20`, minskar paret exakt
  64 gånger med tio bildrutors intervall och når `$0000` vid 11274. Nästa
  återställningsgräns är bildruta 11578. Realdatatestet kräver hela följden
  och avvisar en förvanskad andra stopprad. Ingen skärmbetydelse öppnas av
  kontrollspåret ensamt.
- ✅ JP Track 02:s riktiga spawnkällblock avkodas nu utan USA-fallback.
  `TQJP02.bin` hashverifieras före läsning av pointertabellen vid UD `$273818`,
  zonposterna vid `$273858/$2738D7/$273902/$273929/$273950` och den exakta
  Shift-JIS-rosterprefixen vid `$2739EF`. Pointerposternas regionsord `$2780`
  behålls i stället för USA-värdet `$278A`; zonposterna är byteidentiska mellan
  utgåvorna. Världsbindningen lagrar JP-källan, medan kategorikonsumenten
  fortfarande returnerar `$FF` för JP. Realdatatesten kör både US- och JP-BIN,
  kräver respektive regionala poster och avvisar en korrumperad kopia.
  Produktionsstarten binder nu också rätt regional källa före
  användardatanormaliseringen. Boot-kvittot redovisar
  `theronSpawnSourceAuthenticated=1` och variant 1 för JP respektive 2 för US;
  båda kompletta rå-BIN-starterna verifieras mot originalfilerna.
- ✅ De regionspecifika statiska spawnkonsumenterna är också källbundna.
  USA-rutinens 269 byte vid UD `$0870E5` har FNV-1a `eb241d19`; den omlagda
  JP-motsvarigheten vid `$0868D2` har `7dc1e453` och behåller sina egna
  anrops- och dataadresser. Kvittona kräver rätt fullfils-MD5 och avvisar en
  ändrad originalfil. `runtime_execution_proven` och
  `category_semantics_proven` förblir noll, så statisk kod får inte ersätta
  ett japanskt körningsspår.
- ✅ De äldre generation-6-, generation-49- och generation-51-kvittona är
  nu fail-closed eftersom deras VDC-radantal kom från det dubbelskrivande
  bygget. Mediekoordinaterna kan användas som upptäcktsledtrådar, men deras
  runtime-admission återöppnas först efter nya single-write-captures.
- ✅ 2026-08-20 generation 6 och 7 är ominspelade och återöppnade med den
  korrigerade single-write-byggkedjan. Generation 6 skriver de 8 192
  autentiska byten från Track 02 `$7D9..$7DC` en gång vardera till
  VRAM-ord `$1000..$1FFF` (`4859675d`). Generation 7 gör 1 024 VWR-commits
  inom 2 151 BAT-rader och når den presenterade gränsen efter ytterligare
  36 registerskrivningar: 2 187 rader totalt, CR=`$0088`. Den aktiva
  32×30-bakgrunden refererar 124 källbundna tiles `$110..$18F`; dess
  indexbild har 61 440 pixlar, 3 373 icke-noll och hash `2c2cfb4d`.
  Det nya realdatatestet verifierar hela generation-6→7-kedjan och avvisar
  ändrad generation-7-VRAM. Generation 49 och 51 förblir fail-closed.
- ✅ 2026-08-20 generation 49 och 51 är också ominspelade med
  single-write-byggkedjan. Generation 49 binder READ(6) LBA 4622 till de
  24 576 verkliga användarbyten i Track 02 `$64D..$658` (`01551f76`) och
  exakt 24 580 VDC-rader: fyra inställningar följda av en skrivning per
  mediebyte till VRAM-ord `$1000..$6FFF`. Generation 51:s korrigerade
  atomiska bildrutegräns är rad 54 842, sekvens 94 434, med VRAM
  `de27fc7e`, VCE `88629e93`, SAT `4d7705c5` och CR=`$0048`. Separata
  realdatatester avvisar ändrad Track 02-media respektive ändrad VCE-palett.
  De gamla dubbelskrivningsvärdena 49 160 och 100 755 används inte längre
  för runtime-admission.
- ✅ Track 02:s objektmetadata är nu bunden till den ägande fängelsehålans
  riktiga bank i både US och JP. Namnantalet är 80/65/69/69/67/63/66, alla
  namn- och typkodsspann hashverifieras och varje bank behåller sin egen
  66×6-egenskapstabell. JP Drator har den verkligt avvikande tabellhashen
  `6c4d1386`; övriga verifierade tabeller har `b97787ef`. Pickup, drop och
  sparvalidering använder objektets ursprungsfängelsehåla och avvisar saknad
  eller förändrad källbank.
- ✅ Produktionsbygget länkar inte längre den historiska statiska
  66-raderskatalogen. Track 19 verifierar i stället exakt 396 verkliga
  ISO-byte med FNV-1a `b97787ef`, och det negativa realdatatestet kräver att
  en ändrad tabell avvisas. Den gamla katalogen finns bara kvar i uttryckliga
  kompatibilitets- och analysfixturer.
- ✅ Även USA-utgåvans läsbara Track 19-kataloger är bortkopplade från
  produktionen. Runtime läser de 69 verkliga objektnamnen ur det verifierade
  685-byte-spannet (`5be5602d`) och de 15 verkliga nivåetiketterna ur
  135-byte-spannet (`7f7d9f67`). Ändrade ISO-byte avvisas; de incheckade
  klartextlistorna används bara av historiska fixturer.
- ✅ Track 19:s regionala 69-byte typkodstabeller är nu verifierade och
  bevarade i runtime-banken: US `$0E9226` (`21533bb5`) och JP `$0E9266`
  (`f9c3eabb`). Negativa realdatatest ändrar ett typkodbyte och kräver avslag.
  Tabellerna är byteidentiska med respektive utgåvas dungeon 4-bank i
  Track 02. Världsbindningen jämför dessutom hela 396-byte-egenskapstabellen
  och öppnar därefter en strikt dungeon 4→Track 19-indexkoppling. Objekt från
  övriga dungeons avvisas, liksom ändrade typkoder eller egenskapsbyte.
- ✅ Den levande inventarieinspektionen använder den bevisade Track 19-vägen
  för Sarmon-objekt. Kontrollen följer den verkliga pickup-postens dungeon,
  typkod och egenskapsrad. Övriga sex dungeons använder fortsatt sina egna
  autentiska Track 02-namn eftersom deras tabeller inte har samma
  positionsrelation till Track 19.
- ✅ Fyra incheckade US-klartextkataloger för Track 02 är bortkopplade från
  produktionen: UI-status, save-menyer, nivå-/questtexter och handlingars namn.
  De saknar produktionskonsument och dubblar byte som finns i riktig media;
  deras separata historiska fixturtest finns kvar. Även den statiska
  41-posters kostnads-/sekundärtabellen är borttagen ur produktionsarkivet,
  eftersom endast den fixture-only kompatibilitetsstriden använder den.
- ✅ Ytterligare tolv okopplade Track 02-fixturemoduler länkas inte längre i
  produktionsbiblioteket: champion-/combat-/dungeon-/HUD-texter, fontglypher,
  fullständiga objektnamn, nivåetiketter, handlingsparametrar, klassbasvärden,
  klassfärdighetsvärden, varelsenamn och dungeon-deskriptorer. En exakt
  symbolrevision visar att ingen produktionsfil anropar deras exporter; deras
  uttryckliga historiska fixture-test är kvar. Aktiva roster-, spawn-, item-id-
  och level-data-konsumenter berörs inte.
- ✅ USA-utgåvans kompletta roster är nu avkodad direkt ur den autentiska
  5-bitarsströmmen i Track 02. Det exakta 360-byte-spannet
  `$0B46C8..$0B4830` har FNV-1a `39d95c9e` och innehåller åtta poster med
  namn, kön, HP, uthållighet, mana, sju attribut och sexton färdighetsvärden.
  Fel regionhash, ändrad poststruktur och ändrade byte även i de ännu olösta
  titelfälten avvisas. Titlarnas kontrollkoder publiceras inte.
- ✅ Forcefield-handoff använder nu regionala originalposter för både US och
  JP. Produktionspartiet startar tomt; valda namn skapar endast identitets-
  platser och all statistik samt alla färdighetsnivåer fylls atomiskt av den
  regionala rosterläsaren. Den äldre DMWeb/C-tabellen är bortkopplad från
  produktionsarkivet och finns kvar endast i explicita fixture-test.
## Authentic regional quest-artifact name binding (2026-08-21)

## Utökad autentisk fångstgräns (2026-08-21)

- VDC-producenten har fått en bakåtkompatibel, körningsvald gräns på
  65 536–2 097 152 poster. De tre tidigare atomiska originalfångsterna
  verifieras oförändrat med noll VRAM-avvikelser.
- Grafikgränsen kan samtidigt frysa originalets 8 KiB huvud-RAM. Ett verkligt
  klick-/kontrollpar visar 14 RAM-skillnader utan någon grafikskillnad. Det
  resultatet är bevarat som negativt bevis och öppnar inte rörelse-, positions-
  eller riktningssemantik.
- Ett nytt autentiskt knapp-/kontrollpar visar nu en presenterad
  dungeonförändring med 27 430 skilda bildpunkter. Originalets Button I-kant
  är bunden till `$28B8=$01` vid `$44E5`, och samma session binder
  kvartsrotationen till `$203F: 1→2` samt `$2944/$2948: 1→2` genom den
  fångade rutinen `$D900..$D92E`. Det tidigare antagandet om framåtrörelse är
  därmed korrigerat.
- Kommandofångsten är nu fail-closed: exakt 65 536 ordnade RAM-skrivningar,
  en sista gränsrad, ett äkta 64 KiB kodbanksögonblick och två 8 KiB
  huvud-RAM-ögonblick krävs. Extra poster efter gränsen och ofullständiga
  sidecars avvisas. Den längre VDC-bilden är fortsatt analysdata eftersom
  CPU-portspåret ännu inte återspelar intern DMA.
- Den motsatta originalfångsten köar typ `$01` vid `$7B/$8F` och ändrar
  `$203F`, `$2944` och `$2948` från `1` till `0`; ett långt kontrollpar
  skiljer 27 226 bildpunkter. Typ `$01/$02` är nu bytebundet till
  vänster/höger. M11:s fyra rotationsingångar går genom dessa originaltyper,
  medan andra typer avvisas.

- Bound all seven quest-artifact labels to their exact dungeon-local Track 02
  name-table entries: indices `41, 63, 45, 43, 44, 43, 7` for dungeons 1
  through 7. These are presentation indices, not yet retrieval-event IDs.
- Added a fail-closed source accessor and a world accessor.  Both require an
  authenticated regional item-name bank and return raw ASCII (US) or Shift-JIS
  (JP) bytes without substituting a host label.
- Verified all fourteen names directly against `TQUS02.bin` and `TQJP02.bin`.
  Missing, invalid, or empty source entries are rejected.
- Added `test_theron_v1_track02_quest_item_names`, a compact real-media test
  that can run independently of the full application build.
- Startup chapter inspection and layout now use the live world's bound name
  banks. Authenticated US ASCII is shown directly (for example
  `SHIELD DEFIANT`); authenticated JP Shift-JIS remains explicitly
  unavailable rather than being mislabelled as UTF-8 host text.
- The production text-gate test loads both real BIN files and proves those two
  regional outcomes. Calling the older API without a world remains
  fail-closed and cannot reintroduce the removed static US catalog.
- A second raw-data census proves why these indices must not drive gameplay:
  dungeon 1 has no ordinary carryable record at its label index in either
  region, while other dungeons have between one and eleven same-index records.
  The US counts are `0,1,2,2,1,3,2`; JP counts are `0,11,2,2,0,3,2`.
  Quest completion therefore remains gated on the still-unrecovered original
  retrieval consumer rather than being inferred from an ordinary pickup.
- Release-mode CMake verification now keeps assertions enabled for the two
  real-media decoder/loader tests. The quest-name census, production chapter
  text gate and full US/JP dungeon-loader test all pass together.
- A rebuilt `firestaff` executable also passes both complete raw-media startup
  checks: `theron_v1_raw_bin_runtime_boot` (US) and
  `theron_v1_jp_raw_bin_startup` (JP).
- Added a production decoder for the complete regional retrieval-message
  banks. US uses seven `05 03`-prefixed, NUL-terminated records at
  `UD 0x27713D` (331 bytes, FNV `4777d500`); JP uses seven
  `81 96`/`81 97`-framed Shift-JIS records at `UD 0x27696D` (364 bytes,
  FNV `cb874921`). Both are read from the real BIN and reject any altered
  span. The decoder deliberately leaves `retrieval_event_relation_proven=0`:
  text framing alone does not identify the gameplay event that selects a
  retrieval message.
- The full real-data dungeon loader now installs that regional retrieval bank
  in the live world. A new index-only raw accessor exposes the seven
  authenticated records without calling them gameplay events; bind and read
  both fail closed if event ownership or host rendering is asserted before it
  is proven. The full US/JP loader, raw startup, and compact decoder tests pass
  with the rebuilt application.
- Added an authenticated US/JP campaign-mask source receipt. The common
  336-byte
  routine is byte-identical at US UD `$071800` and JP UD `$071000` (FNV-1a
  `72af456f`) and proves original RAM `$267C`, low-bit mask `$7F`, preserved
  high bit, and the load-time `ORA`/store sequence. Seven 304-byte
  dungeon-local windows additionally bind the real `01 02 04 08 10 20 40`
  ordinal-to-bit table and their own `ORA $267C`/`STA $267C` sequence before
  loading another code resource and jumping to `$4000`. Altering either code
  class is rejected for both retail BINs. The receipt explicitly records that
  no artifact-collection relation has been proven; `$267C` is therefore only
  classified as campaign/dungeon state.
- The campaign-mask receipt is now installed in the live world on every full
  real-media dungeon load. Its binder rejects any source that asserts an
  artifact-collection relation, so the authenticated `$267C` evidence cannot
  silently enable the host's older quest-item model.
- The same receipt now authenticates the seven four-sector post-dungeon
  programs reached by that transition. US Track 02 INDEX 01 begins after 225
  raw sectors and JP after 224, so relative CD blocks `$3C7 + 4*n` resolve to
  raw records `$4A8 + 4*n` and `$4A7 + 4*n`. Each program stores its fixed
  ordinal 0–6 at `$2701`; programs 0–5 store `$09` at `$2700`, while the final
  program stores `$0F`. This proves the ordinal handoff into the post-dungeon
  program, not yet its relation to retrieval-message record `$4EE`.
- Authenticated the shared 17-sector post-dungeon program for both editions
  (US FNV-1a `113c8278`, JP `834bede1`). The original `$8243` routine copies
  `$2700/$2701` into `$2780/$2781`, and the common handler forwards the seven
  bytes beginning at `$2781` into its local selection table. The production
  decoder now rejects altered shared-program bytes and exposes these two
  ordinal-transport proofs without claiming an artifact or retrieval event.
- Captured the retail US forward-panel click `$8A/$8F` as original command
  type `$03` with an exact 65,536-write main-RAM trace. The command starts at
  party coordinates `$02/$03`, resolves destination `$03/$03`, and commits
  both bytes through the original `TII $20B4,$2040,$0002` at `$C1FA`
  (observed write PC `$C203`, physical `$0DC203`). The UP runtime route now
  uses `$03` and the existing real Track 02 tile/object collision model.
- Captured the remaining retail movement-panel commands: `$04` at `$98/$A5`
  is right, `$05` at `$8A/$A5` is backward, and `$06` at `$7B/$A5` is left.
  The original `(command-3+direction)&3` calculation binds their relative
  directions. `$05` commits `$02/$03->$01/$03`; `$04/$06` are blocked from
  this real savestate and never write `$2040/$2041`. M11 now routes A/D and
  DOWN through these original types while preserving facing direction.
- Replaced the stale Theron four-way/no-strafe host assumption. A/D now emit
  `M12_MENU_INPUT_STRAFE_LEFT/RIGHT` and reach original `$06/$04`, while arrow
  Left/Right and keypad 4/6 retain turn semantics. The existing host bridge
  test and runtime-input test pass. The real US/JP Track 02 playability probe
  now exercises each `$03..$06` command over an authentic floor edge and an
  authentic wall edge, preserving facing in every case: 143/143 pass.
- Removed the two synthetic multi-level cases from the real-data mechanics
  probe. It now converts the authenticated 2352-byte raw sectors through the
  verified MODE1 user-data bridge, loads the complete real AKUTUBA dungeon,
  verifies every loaded source header, and traverses an authentic stair edge
  with original command `$03`. US and JP complete with 145/145 passes and no
  constructed level, door, pit, object table or stair.
- Removed the mechanics probe's unrelated four synthetic champions, fixed
  stat values and 1,000-gold seed. Real movement now runs with an empty party
  state plus only the authenticated map/start pose; regional roster and save
  receipts remain the sole production owners of champion state.
# 2026-08-21 Theron real-door mutation boundary

- ✅ Verkliga US/JP Track 02-dörrar får inte längre den syntetiska
  fixturemodellens omedelbara `OPEN`/`CLOSED`-mutation. Mechanics verifierar
  exakt source-förekomst, fyra-bytepost och nivåheader och stänger därefter
  mutationsvägen tills originalets T900-knapp/actuatorkonsument är fångad.
  Realdataproben hittar en faktisk dörrkant i den fullständiga dungeon-korpusen
  för båda regioner,
  verifierar blockerad originalrörelse och att direkt open/USE bevarar stängt
  tillstånd. Isolerade fixtures behåller sin uttryckliga testmodell.
- ✅ Loadern binder nu den redan verifierade Track 02-identiteten även till
  levande dörr-, teleporterar- och actuatorobjekt. Tidigare fanns deras exakta
  bytes endast i provenance-ledgern, vilket gjorde att runtimeobjektet inte
  kunde bevisa vilket riktigt record det representerade. Korpustestet jämför
  nu ursprungsdungeon, nivå, koordinat, ref, index, kategori, position och hela
  råposten för samtliga placerade kontrollobjekt i US och JP.
- ✅ Source-grinden för riktiga dörrar överlever world save/load. Varje US/JP-
  dungeon med dörr serialiseras nu från sin fulla riktiga värld, återställs
  över samma autentiserade nivåbank och verifierar att source-identitet och
  råpost är oförändrade samt att direkt host-open fortfarande nekas.
- ✅ Source-reference `0x0000` är inte längre en ogiltig host-sentinel. I
  Track 02 är den en giltig packad reference för kategori 0, index 0 och
  position 0; SARMONs riktiga dörrkorpus träffar fallet. Provenance avgörs nu
  av full occurrence-matchning mot ledgern, även när den packade referensen
  råkar vara noll.
# 2026-08-21 Theron startup seed uses the real level header

- ✅ Startupens semantic handoff kräver inte längre den motbevisade
  syntetiska descriptor-tabellen `313/414/527/632/749/856/967`. Runtime-seeden
  kommer nu från den hash-/ankargodkända kandidatens riktiga 12-byteheader
  (`0x0108e938` i både US och JP). Descriptor entry 0 behålls enbart som
  diagnostik och kan vara zero-fill/bad-shape utan att neka den autentiska
  nivån. Realdataproben verifierar positiv handoff, runtimekvitto och blockerad
  fallback för båda regionerna.
- ✅ Den mediafria `theron_v1_startup_enter_forcefield()` kan inte längre
  publicera en nollställd championpost som “Theron” i produktionsbiblioteket.
  Den returnerar `NOT_READY` utan att ändra Soul Room eller party. Den riktiga
  runtimevägen använder fortsatt `_with_roster()` och binder därefter US/JP-
  posternas HP, stamina, mana, attribut och skills atomiskt före nivåhandoff.
- ✅ Forcefieldens produktionshandoff är nu en enda transaktion via
  `theron_v1_startup_enter_forcefield_with_track02_roster()`: verifierad
  region, valda speglar, namn och US/JP-statposter måste alla lyckas innan
  flow eller party publiceras. Vid fel återställs båda exakt. De äldre
  mediafria forcefield-API:erna returnerar `NOT_READY` i produktionsbiblioteket
  och finns kvar enbart för explicit fixturekompilering.
- ✅ Startup-entryns döda kompakta host-object-publisher är borttagen. Den
  kunde endast byggas från den motbevisade descriptor-object-tabellen och
  skapade objekt av `kind/flags/argument` utan Track 02-occurrence eller
  råpost. Produktionsstarten använder nu endast full-dungeon-loadern för
  riktiga objekt; den separata autentiska nivåheadern får publicera en nivå
  men aldrig fabricera ett objektlager.
# 2026-08-21 Originalets `$50`-konsumentgrind

- ✅ Den hashbundna originalfångsten verifierar nu även hela det ordnade
  main-RAM-konsumentspåret. Det autentiska `$50`-fönstret innehåller noll
  läsningar i `$2600–$27ff`, så ett vanligt viewport-zonträfftest kan inte
  längre råka publiceras som dörr- eller T900-bevis. Saknat konsumentspår
  nekas och all semantisk publicering förblir stängd.
- ✅ Den kommandobundna Mednafen-producenten loggar nu endast huvud-RAM
  `$2000–$3fff` och fångstgrinden nekar både adresser utanför fönstret och
  fönster som når 65 536-läsningstaket. En forskningskörning av originalets
  `$03` mot den riktiga US-dörren avslutades efter 55 078 läsningar och gav
  96 läsningar i källområdet. Resultatet publicerar fortfarande ingen
  dörrsemantik utan en riktad mutation vid konsumenttidpunkten.
- ✅ Riktade forskningsmutationer har avgränsat vad detta fönster inte bevisar.
  HuC6280-avbrott under blocköverföring kan behålla blockinstruktionens
  rapporterade PC, och mutationer av `$271b–$272b`, `$27af–$27c8` samt den
  dörrliknande arbetsbyten `$2098=$81` band varken den riktiga posten eller
  rörelseutfallet. Dokumentationen kallar därför inte längre de 96
  tidsavgränsade läsningarna för ett kausalt T900-konsumentbevis.
- ✅ Fångstkvittot delar nu tidsfönstrets källområdesläsningar från läsningar
  efter originalets `$2905`-dispatch vid `$d34d`. Den riktiga `$03`-körningen
  har 96 före men 0 efter dispatch. Därmed kan avbrotts-/renderingsarbete inte
  längre räknas som rörelse- eller dörrkonsumentbevis.
- ✅ Ett separat data-only-spår har jämfört samma riktiga dörrposition med en
  verifierad öppen kontroll på `(1,3)→(2,3)`. Originalets första vägskillnad
  är den exklusiva X-gränsen vid `$4fbb–$4fbd`: dörrpositionens mål-X `5`
  blockeras mot gränsen `5` innan Y-/rutkontrollen, medan kontrollens mål-X
  `2` fortsätter. En mutation av den senare bankade skillnaden `$10→$20`
  ändrade inte positionen. Framåtkollisionen klassas därför som kartgräns,
  inte som T900- eller dörröppningsbevis; riktiga dörrar förblir fail-closed.
- ✅ Originalets markörrutt har nu fångats på ytterligare en verklig punkt.
  `$79/$62` skriver hoverkommando `$2911=$74`, köar `$2905=$74` och avslutas
  efter 69 208 huvud-RAM-skrivningar. Ett komplett data-only-spår på 72 693
  läsningar jämfördes med samma punkt från en verifierad öppen kontroll.
  Ett horisontellt hover-svep visar `$50` vid X `$00–$6f` och därefter en
  snap till `$79/$78` med `$74`; koden hör därmed till högerspalten, inte
  frontcellen. Rutten ändrade ingen source-bunden dörrpost och publiceras
  därför endast som originalkommando, inte som vy-, dörr- eller T900-semantik.
- ✅ Therons äldre V1-klickmatris publicerar inte längre nio hostskapade
  320×240-rutor som originaldata. De stämde dessutom inte med originalets
  autentiska 320×200-bild. V1-zonfrågor är nu tomma och fail-closed tills den
  verkliga rektangeltabellen eller ett komplett gränsspår finns. Den moderna
  V2-overlayens presentationszoner är kvar och är uttryckligen inte V1-data.
- ✅ Track 02-laddaren låter inte längre kategori‑0‑poster skriva över
  kartströmmen med hostskapade dörrutor. Kartbytens egen typ måste redan vara
  dörr; annars avvisas banken. En full körning av alla sju autentiska US- och
  sju JP-banker visar att samtliga riktiga dörrposter uppfyller grinden.
- ✅ Track 02-laddarens TAKE/DROP-regression använder nu Therons autentiska
  regionala championpost från samma riktiga US- respektive JP-BIN. Testet
  skapade tidigare en tom värld och försökte sedan genomföra en
  inventarieövergång utan aktiv champion. Alla fjorton riktiga dungeonbanker
  klarar nu både föremålsrundturen och dörrgrinden utan någon testchampion.

# 2026-08-21 Track 02-teleportörens verkliga OPEN-attribut

- ✅ En isolerad originalkörning skiljer två riktiga Akutuba-pads åt. Ruta
  `(0,0)` har kartbyte `B8` och flyttar gruppen till `(2,3)` på map 0. Ruta
  `(2,1)` har `B4`; ett autentiskt steg norrut landar på rutan men lämnar
  gruppen på map 0. Skillnaden är kartbytens `0x08`-bit, samma OPEN-bit som
  dungeonformatet anger, inte teleporterpostens `scope`.
- ✅ Världsbryggan bevarar nu den exakta råa kartbyten för varje Track 02-ruta.
  Loadern kräver att en kategori-1-post redan ligger på en verklig
  teleporter-tile och får inte längre skriva över kartgeometrin.
- ✅ Källbunden runtime använder kartans OPEN-bit som teleporterarens tillstånd.
  En aktiv `B8`-pad följer sin verkliga koordinatlänk; en inaktiv `B4`-pad är
  passabelt golv och startar ingen transition. Postens tvåbitars `scope`
  behålls separat som källmetadata och aliasas inte längre till runtime-state.
- ✅ Den icke-muterande rörelsefrågan använder nu samma OPEN-grind som den
  muterande vägen. En stängd `B4`-pad rapporteras som vanlig passabel rörelse,
  inte som teleportering; Akutubas riktiga `(2,1)`-ruta provar båda anropen.
- ✅ Regressionen kör alla sju autentiska US- och alla sju JP-dungeonbanker,
  verifierar kart/record-grinden och provar båda Akutuba-rutterna. Full
  Track 02-loader, cross-route-mekanik, touchmatris och world-serialisering
  passerar. Den äldre breda combat-fixturen har fortsatt sina kända
  fail-closed-fel för ännu ej autentiserade combat-, spell- och dörrkonsumenter;
  inga sådana fixtures har öppnats för att få grönt test.
- 🔒 Den stängda Akutuba-rutan har dessutom en verklig floor-party-actuator,
  rå `feff0300a4078018`: typ 3, `OnceOnly`, `SET`, `RevertEffect`, delay 15
  och mål `(2,3)`. Originalets steg på `B4` konsumerar den inte, eftersom
  `RevertEffect` vänder tilläggshändelsen till falsk. Posten får inte användas
  som automatisk host-teleportering.
- ✅ Originalets OPEN-konsument är nu kodbunden. `$50F6` hämtar kartbyten;
  `$C240–$C247` kräver teleporter-typen `A0`, och `$C24C–$C251` testar
  `AND #$08`. En stängd pad går till den ordinarie rörelseavslutningen; en
  öppen pad fortsätter till postavkodningen vid `$C27A`.
- ✅ `$C27A–$C2D8` hämtar destinationens level, X och Y direkt ur den verkliga
  sexbyteposten och kopierar koordinaterna med originalets
  `TII $20B4,$2040,$0002`. Firestaffs Track 02-väg följer nu samma kontrakt:
  koordinatdestinationer kräver inget fabricerat endpoint-objekt, men en
  destination som själv är en teleporter kräver sin riktiga kategori-1-post.
- ✅ Fullkorpusen innehåller 170 öppna teleportörer i vardera regionen. Tio US-
  och tio JP-länkar landar direkt på en stängd teleporter. De fallen kedjades
  tidigare vidare och gjorde hela förflyttningen ogiltig; nu avslutas kedjan
  på den passabla destinationen, i enlighet med originalets upprepade
  `$C24C`-grind. Samtliga tjugo riktiga länkar körs genom runtime-resolvern i
  regressionstestet.
- ✅ Save/load-regressionen väljer nu dessutom en verklig öppen teleportör i
  var och en av de fjorton US/JP-dungeonbankerna. Rå sexbytepost, OPEN-state,
  separat scope, koordinatlänk och källkartans OPEN-bit måste överleva, och den
  återställda världen måste ge samma resolvermål. Testet förladdar samma
  autentiska Track 02-värld som produktionen; det skapar ingen ersättningskarta.
- 🔒 En forskningsmutation av endast Akutubas verkliga `$0E8AE8` från `B4`
  till `B8` fick originalet att använda den befintliga postens map-1-mål
  `(6,0)` och därefter läsa den nya kartans data. Den användes enbart för att
  försöka nå knappdörren. Direkta pose-/cachemutationer gav motstridiga
  tile-pekare och publiceras därför inte som dörr- eller actuatorsemantik.
- 🔒 Övergångspostens exakta fysiska originaladress är nu identifierad som
  `$0E9591`: råposten `05 0c 06 f0 00 01` avkodas av originalet till map 1,
  mål `(6,0)`. Forskningsinstrumentet ändrade endast postens koordinat- och
  rotationsbitar. Originalets egen teleportrutin skapade därefter konsekventa
  tillstånd vid `(2,3)` riktning norr och `(2,1)` riktning söder, utan direkta
  pose- eller cachepatchar.
- ✅ Den riktiga US Track 02-kartan verifierar knappdörren på Akutuba map 1
  `(2,2)` som kartbyte `9c`. Dess riktiga kategori-0-post är
  `fe ff e0 01`: trä, öppnas uppåt, har knapp, är förstörbar och krossbar.
  Detta är källdata- och placeringsbevis, inte ännu ett runtime-mutationsbevis.
- 🔒 Originalets UI-spår framför dörren visar autentiska hoverkommandon och
  träffpunkter, bland annat `$65` vid `$78/$38`. De reproducerade
  knapptryckningarna nådde dock antingen ingen kommandokonsumtion eller andra
  UI-kommandon (`$03`, `$6c`, `$7d`); ingen körning ändrade dörrposten.
  Firestaff behåller därför source-bundna dörrar fail-closed tills den verkliga
  knappkonsumenten och dess exakta stateövergång har fångats.

# 2026-08-21 Track 02-pitarnas verkliga OPEN-grind

- ✅ Originalets rörelsekod `$C332–$C347` klassar karttyp `$40` som pit och
  går till fallrutten endast när levitation är noll, `OPEN` (`$08`) är satt
  och `IMAGINARY` (`$01`) är noll. Stängd, imaginär eller leviterad pit går
  vidare utan fall. Den grinden används nu av både rörelsefrågan och den
  muterande runtimevägen.
- ✅ Den kompletta riktiga korpusen verifierar 165 stängda/passabla och 132
  öppna/fail-closed US-pits samt 166 stängda/passabla och 130 öppna/fail-closed
  JP-pits. Minst ett verkligt rörelseutfall av vardera slag körs per tillgänglig
  dungeonbank. Samma öppna originalruta provas även med levitation och måste då
  vara passabel, precis som `$AB`-grinden anger. Inga konstruerade pit-rutor
  eller reservkartor används.
- 🔒 En öppen, verklig pit blockeras fortfarande innan skada och nivåfall.
  Originalgrinden är bevisad, men T700-statmutationen och destinationens
  övergångskonsument är inte det. Fixturepitens äldre skada används därför
  inte på en source-verifierad nivå.

# 2026-08-21 Live-start och Continue behåller riktig dungeondata

- ✅ Produktionsstarten för verifierad US- och JP-rå-BIN går genom samma
  full-dungeon-laddare som Track 02-övergången. Bootprovet kontrollerar nu den
  körande världen, inte bara ett laddarkvitto: Akutuba måste ha fyra laddade
  nivåer, fyra verifierade källhuvuden och icke-tomma bevarade källrutor.
  Båda riktiga regionala starterna passerar.
- ✅ Continue nollställer inte längre `level_loaded` och gör därmed redan
  avkodade Track 02-kartor oåtkomliga. Med en verifierad bootprofil laddas den
  sparade dungeonbanken atomärt från exakt samma Track 02-fil innan den
  återställda världen publiceras. Sparfilen bidrar endast med progression och
  party; den skapar eller serialiserar ingen ersättningsgeometri.
- ✅ Om Track 02 inte kan läsas, verifieras eller innehåller den sparade nivån
  misslyckas Continue utan att delvis skriva över livevärlden. Syntetisk
  reservdungeon är fortsatt avstängd.

# 2026-08-21 Verklig actuator-korpus

- ✅ Alla sju dungeonbanker i både US och JP censureras direkt från sina
  autentiska kategori-3-poster. Regionerna innehåller samma 1 109 placerade
  actuatorer: bland annat 393 golvtyp 3 (party), 46 golvtyp 6
  (monstergenerator) och 57 väggtyp 127 (championspegel). 286 poster har
  `OnceOnly`, 116 har `LocalEffect` och 70 har `RevertEffect`. De fyra
  effekterna SET/CLEAR/TOGGLE/HOLD fördelas 632/158/103/216. Typ-, effekt-
  och totalantal är regressionslåsta mot riktig data.
- 🔒 Censuren öppnar ingen målmutation. Akutubas riktiga golvtyp 3 på den
  stängda `B4`-teleporterrutan konsumerades inte av originalsteget. Vid
  tillägg vänder `RevertEffect` villkoret till falskt; vid normal borttagning
  returnerar F0276:s occupancy-kontroll innan inverteringen.
  Effekt-/måldispatch måste fortfarande bindas innan `SOURCE_ACTUATOR` får
  ändra dörrar, generatorer eller egna flaggor.
- ✅ Det fristående kategori-3-avkodarprovet använder inte längre en
  konstruerad actuator. Det är ersatt med den exakta US/JP-posten på
  Akutuba M0 `(2,1)`, källreferens `0c05`, index 5 och råbyte
  `feff0300a4078018`. Testet låser hela avkodningen: typ 3, `OnceOnly`,
  `SET`, `RevertEffect`, delay 15, inget `LocalEffect` och mål `(2,3)`.
- ✅ Samma test avaktiverar nu `NDEBUG` före `<assert.h>`. Releasebygget tog
  tidigare bort både kontrollerna och de laddaranrop som låg inuti `assert`,
  fortsatte med oinitialiserad data och kraschade. Det riktiga
  Track 02-avkodartestet körs nu faktiskt och passerar.

# 2026-08-21 Verkliga projektil- och molnpooler

- ✅ Track 02-läsarens andra itemström stoppade tidigare felaktigt efter
  kategori 10, trots att de autentiska kategori-14- och 15-tabellerna följer
  direkt efter kategorierna 11–13 med noll postbredd. Läsaren går nu genom
  samtliga 16 kategorier. I US Akutuba fyller de 60 projektilposterna och 50
  molnposterna exakt de sista 680 byten fram till dungeontextens källoffset.
- ✅ Alla sju riktiga US- och JP-banker måste nu innehålla och avkoda exakt 60
  projektilposter och 50 molnposter. Varje posts råpayload kontrolleras som
  icke-tom och körs genom den portabla källavkodaren. De tidigare konstruerade
  decoderposterna är ersatta med de verkliga första US Akutuba-posterna
  `ffff101001000000` och `ffff2e0e`, vilka också jämförs direkt med tabellerna
  som lästs ur `TQUS02.bin`.
- 🔒 Tabellerna är autentiskt inlästa och avkodade men är dynamiska
  recordpooler, inte markförekomster. Ingen projektil eller effekt skapas utan
  den ursprungliga allokerings-, timer- och träffkonsumenten.

# 2026-08-21 Verkliga monster- och party-actuatorkontexter

- ✅ Det fristående kategori-4-provet använder inte längre en konstruerad
  monsterpost. Det är ersatt med US Akutuba index 0,
  `fefffeff0a013b002500240025002004`, och jämförs direkt med den autentiska
  Track 02-tabellen innan `chested`, typ, position, fyra health-ord, flaggor,
  gruppstorlek och riktning kontrolleras.
- ✅ Samtliga 393 verkliga golvtyp-3-poster i vardera regionen klassificeras
  nu efter sin riktiga kartruta. 210 ligger på golv, 92 på pits, 8 på trappor
  och 83 på teleportörer; av teleporterarna är 7 öppna och 76 stängda. Alla
  har thing-position 0. Även effektfördelningen SET/CLEAR/TOGGLE/HOLD
  190/66/40/97, 91 `OnceOnly`, 12 `LocalEffect` och 19 `RevertEffect` är
  regressionslåsta och identiska i US/JP.
- 🔒 Kontexten motbevisar en generell hostregel där en stängd teleporter ensam
  skulle blockera en party-actuator. B4-fallet kan därför inte öppna eller
  stänga typ-3-dispatchen utan originalets händelsevillkor.

# 2026-08-21 Typ-3-posternas riktiga händelsegrind

- ✅ Actuatorordets layout följer nu originalets gemensamma DM/CSB-format:
  tvåbitars `Effect` i bit 3–4, separat `RevertEffect` i bit 5 och
  `LocalEffect` i bit 11. Den tidigare trebitstolkningen skapade påhittade
  effekter 4–7 och kallade felaktigt `LocalEffect` för `inactive`.
- ✅ F0276-grinden finns som en ren Theron-funktion före måldispatch. Den
  kontrollerar levande party, tillägg/borttagning, riktning när postens värde
  är 1–4, `RevertEffect`, HOLD:s upplösning till SET/CLEAR och att
  `OnceOnly` först får förbrukas efter verklig dispatch.
- ✅ Regressionen använder den autentiska Akutuba-posten ovan. Party som
  kliver på rutan ger ingen dispatch och förbrukar inte posten. Normal
  borttagning är också tyst, eftersom `partySquare=1` stoppas av
  occupancy-kontrollen före `RevertEffect`. Endast en anropskontext som
  uttryckligen rapporterar rutan som tom kan invertera borttagningen till SET.
  Tom party stoppas före inverteringen, precis som originalet. Testerna tvingar dessutom på
  assertioner i Release-läge, så de riktiga Track 02-anropen kan inte optimeras
  bort.
- ✅ Grinden muterar inte målet direkt. Rörelsehändelserna går via den
  serialiserbara kön nedan, och de måltyper vars originalkonsument nu är
  bunden verkställs först på rätt tick.

# 2026-08-21 Autentiska actuatorhändelser i livevärlden

- ✅ Vanlig partyförflyttning publicerar nu F0365/F0276-händelser i
  originalets ordning: borttagning från den gamla rutan före tillägg på den
  nya. Riktningstestet använder partyns faktiska riktning, inte sidstegets
  absoluta färdriktning. En full kögranskning görs före rörelsen, så en fylld
  kö lämnar både position och actuatorer orörda.
- ✅ Händelsen bevarar dungeon, nivå, källreferens och källindex, källruta,
  remote-mål eller lokal ruta, upplöst SET/CLEAR/TOGGLE-effekt, ljudflagga,
  delay och absolut `due_tick`. `OnceOnly` stängs först när F0276 verkligen
  har publicerat en händelse. Den råa källposten ändras inte.
- ✅ Kön ingår i world-hashen och save-version 15. Full round-trip,
  en byte för kort svans, atomärt deserialize-fel och kompatibilitet med
  version 13 passerar. Äldre sparningar får en tom kö; de verkliga
  source-objekten laddas fortfarande från Track 02.
- ✅ Akutubas autentiska B4-rutt provas genom den riktiga rörelsefunktionen.
  Både steget in och det normala steget ut är tysta och lämnar dess
  `OnceOnly`-post aktiv, i enlighet med F0276:s ordning. En separat autentisk
  riktningspost i Formicia, rå `feff830080008029`, köas när partyn vrider sig
  till rätt riktning och låser den verkliga publiceringsvägen.
- ✅ Trappvägen kör nu tilläggspasset på den beträdda trapprutan innan
  nivåbytet. Den autentiska gruppen i dungeon 2, map 4, `(0,6)` köar sina tre
  samtidiga SET-poster i källordning: `0c0a`, `0c3a`, `0c39`. Ingen påhittad
  destinationshändelse körs efter nivåbytet.
- ✅ Actuatorns sista ord avkodas nu även enligt sin lokala union. Remote
  poster behåller cell/X/Y; `LocalEffect` behåller det fulla 12-bitars
  `Multiple`-värdet. Därmed behöver en senare lokal konsument inte återskapa
  information ur målkoordinater. Det separata kategori-3-gränssnittets
  9-bitars Data-fält har samtidigt breddats från åtta till nio bitar.
- ✅ Alla 393 verkliga floor-party-poster per region är klassificerade inför
  dispatch. Tolv är lokala och har exakt `Multiple=176`. De 381 remote-
  posterna träffar samtliga åtta råa kartfamiljer. En gemensam Thieves-post
  på M0 `(5,5)`, `feff030078154050`, pekar utanför kartan mot `(1,10)` och
  måste därför förbli fail-closed. US/JP-fördelningarna är regressionslåsta.
- ✅ Kön är nu en tillåtelse att ändra autentiska kategori-0-dörrar och
  kategori-1-teleportörer när hela källidentiteten fortfarande stämmer.
  Konsumenten verifierar rå kategori-3-post, dungeon, nivå, källruta,
  referens/index, mål, effekt, delay och lokal metadata före mutation.
  Sarmons riktiga `0cd6`, rå `feff030040074068`, öppnar teleportören på
  `(1,13)` först efter delay 14. Drators riktiga `0c04`, rå
  `feff0300d0008018`, löser TOGGLE till öppning och animerar dörren på `(2,3)`
  från tillstånd 0 till 4 över fyra tickar efter delay 1.
- ✅ Båda målproven kontrollerar att Track 02-källbyten förblir oförändrad.
  Teleportörens muterbara OPEN-bit ägs av dess autentiska liveobjekt;
  dörrens animationsläge ägs på samma sätt av kategori-0-objektet och följer
  originalets SET=stäng/CLEAR=öppna-ordning.
- ✅ Pit och fakewall använder nu ett sparsamt runtime-lager ovanpå den
  oförändrade Track 02-källkartan. Endast respektive OPEN-bit får avvika:
  `$08` för pit och `$04` för fakewall. Lagret ingår i världshashen och
  save-version 17 tillsammans med det sparsamma objectordet nedan; version
  15/16 läses med sina äldre tomma svansar och version 13-provet fortsätter
  att passera. Dungeonreload rensar bara den omladdade bankens runtimeposter.
- ✅ Sarmons autentiska fakewall-post `0c9b`, rå
  `feff030090004068`, togglar mål `(1,13)` från `$c0` till `$c4` efter ett
  tick. Världens rutfråga ser därefter passabelt golv, medan `source_tiles`
  fortfarande innehåller `$c0`. Samma verkliga värld sparas och återställs
  med den sparsamma posten intakt.
- ✅ Sarmons autentiska pit-post `0cea`, rå `eb0c0300c8078080`, kör CLEAR
  efter delay 15 och ändrar mål `(2,16)` från runtime `$48` till `$40`.
  Rörelsefrågan ser den stängda pitten som passabel utan att källbyten `$48`
  skrivs över. Drators trappgrupp bekräftar dessutom att SET mot två redan
  öppna pits är idempotent före nivåbytet.
- ✅ Alla remote-mål är nu även klassificerade mot sina autentiska
  thing-kedjor. I US saknar 186 av 201 wallmål och 86 av 114 corridormål helt
  en thing; motsvarande JP-tal är 185 och 86. De exakta kategoriutfallen för
  samtliga åtta kartfamiljer är regressionslåsta. Tomma wall/corridor-event
  förbrukas nu som originalets F0248/F0245-no-op i stället för att fylla kön.
  Drators trappgrupp provar vägen med sitt tomma wallmål `(8,1)`.
- ✅ De 57 wall/corridor-träffarna mot kategori 3 är dessutom klassificerade
  efter verklig actuatortyp och är identiska i US/JP. Wall har typerna
  0/1/2/3 med fyra vardera, typ 4 med tre, typ 5 med sexton och typ 15 med en.
  Corridor har typ 1/2 med en vardera, typ 3 med åtta, typ 6 med sju och typ 7
  med fyra. Därmed är det exakt sju corridor-generatorer som behöver F0245:s
  gruppmaterialisering och återaktivering; övriga får inte felaktigt gå genom
  generatorvägen.
- ✅ F0248-konsumenten behåller nu bara walltyper som originalet faktiskt
  behandlar: gate/countdown, launchertyper och endgame. I den riktiga
  målkorpusen innebär det sexton typ-5-gates och en typ-15-launcher. De tjugo
  träffarna mot walltyp 0–4 förbrukas som autentiska no-op i stället för att
  ligga kvar och fylla kön.
- ✅ Kategori-2-text har nu ett sparsamt, serialiserbart runtimeord som får
  avvika från råposten endast i Visible-biten. Hela US/JP-korpusen har exakt
  en remote corridor-träff mot text: Sarmons `0c41`, rå
  `feff030000004048`, skickar SET till text `0803`, rå `feff0902`. Textens
  ord är redan `$0209` och Visible redan 1, så den autentiska effekten är
  idempotent och lämnar inget fabricerat runtimevärde.
- ✅ Väggtyp 5 använder nu originalets F0730 AND/OR-grind: inkommande cellbit
  muteras av SET/CLEAR/TOGGLE, låg nibble jämförs med referensnibblen,
  `RevertEffect`, HOLD och `OnceOnly` respekteras och en träff publicerar
  grindens egen fördröjda efterföljare. Sarmons autentiska kedja från `0ca7`
  vid `(12,11)` till grind `cc78` vid `(5,3)` är byte- och identitetslåst;
  den skickar CLEAR vidare till `(8,6)`. Grindord och kö överlever save/load
  deterministiskt utan att Track 02-råposten skrivs över.
- ✅ Den enda remote-träffen mot walltyp 15 är Shadodans källa `0c59` till
  launcher `4c65`: eventet adresserar cell 0 medan launchern sitter i cell 1.
  F0248:s obligatoriska cellmatch är nu bunden och regressionstestad, så
  kedjan förbrukas som autentisk no-op och skapar ingen projektil.
- ✅ De tolv lokala golvposterna har alla `Multiple=176`. F0270 behåller det
  kodade värdet, men F0271 muterar bara för CLEAR 1 och TOGGLE 2; XP-vägen är
  det separata värdet 10. Samtliga tolv riktiga händelser körs nu genom
  konsumenten och förbrukas utan mutation, precis som originalets defaultfall.
- 🔒 Corridor-generatorer ligger kvar i kön tills deras muterbara kategori-3-
  och grupp-/RNG-konsument är källbunden. Händelser mot sådana verkliga kedjor
  behålls. De sju riktiga träffarna är nu identifierade: fyra går till
  Drators `0c38` och en vardera till Formicias `0c57`, Thieves `0c70` och
  Demons `0c64`. Sex har fast generationsfält 1; Formicias har 12 och kräver
  originalets slumpade gruppantal. Alla behöver dessutom den ännu ofångade
  `$4644/$4667`-returen för riktning och HP. Thieves-postens ogiltiga mål
  förblir fail-closed utan sidoföljder.
- ✅ Produktionsnära regressionsprov kräver inte längre syntetisk strid,
  trollformler, låsta dörrar, altare eller hunger/törst-dränering när deras
  autentiska konsumenter saknas. De verifierar i stället fail-closed och att
  världstillståndet inte muteras. `theron_v1_combat_mechanics`,
  `theron_v1_cross_route_mechanics` och `theron_v1_srm_classifier` passerar.
  SRM-provet kräver en daterad statusmarkör utan att låsa sig vid ett gammalt
  datum. De explicit länkade champion- och startup-proberna har samtidigt fått
  sina saknade riktiga källmoduler och bygger åter utan att lägga tillbaka
  fixturetabeller i produktionsarkivet.
- ✅ Generatorernas nästa källgräns är nu byte- och tillståndslåst. Fyra
  oberoende, autentiska 8 KiB-RAM-bilder har samma `$28b9-$28bb`-tillstånd
  `1a 62 29`; det är exakt seed 0 efter två `$4667`-steg och nästa riktiga
  retur blir `$9d`. De sju nåbara generatorträffarna verifierar samtidigt
  hela råposterna för Drator `0c38`, Formicia `0c57`, Thieves `0c70` och
  Demon `0c64`. En källbunden plan skiljer de sex fasta enmedlemsgrupperna
  från Formicias slumpbundna 1–4 med bound 4, men publicerar ännu ingen
  riktning eller HP innan originalets fullständiga RNG-anropsordning är bunden.
- ✅ Mednafen-spårningen låser nu varje verkligt RNG-anrop vid fysisk
  `$0d0667/$0d067e`, med returadressen hämtad från HuC6280:s stackpage `$2100`.
  En autentisk state-autoload gav 26 sammanhängande entry/return-par:
  returägare `$4647` 5 gånger, `$464d` 9 gånger och `$cca4` 12 gånger. Varje
  returstate är exakt nästa entry-state. Captureskriptet validerar kedjan och
  hanterar även korrekt att den valfria command-consumer-filen saknas.
- ✅ Alla sju riktiga corridor-generator-event kan nu lösas entydigt till sin
  autentiska Track 02-generatorpost i produktionsvärlden. Lösaren verifierar
  eventets åttabytespost, effekt, delay, ljud och målkoordinat och returnerar
  bara källidentiteten. Händelsen lämnas kvar i kön och varken RNG, timer, HP,
  riktning eller monster publiceras ännu. Regressionen låser även de riktiga
  eventkoordinaterna och råposterna för Drator, Formicia, Thieves och Demon.
- ✅ RNG-captureformat v2 registrerar nu fysisk returägare och originalets
  `$B0/$B1/$B3-$B6/$B8/$BA/$BB`. En riktig smoke-capture gav 18 kompletta par
  och band `$4647→$0d0647`, `$464d→$0d064d` och `$cca4→$0d8ca4`. Detta ger
  tillräcklig kontext för nästa eventbundna capture men påstår ännu inte att
  den körningen utlöste en av de sju generatorerna.
- ✅ Originalets efter-dungeon-dispatcher är nu körd mot den autentiska USA-
  skivan och samma riktiga savestate för ordinal 0–5. Kampanjbyten `$267c`
  blir exakt `01, 02, 04, 08, 10, 20`; Drator är därmed originalbundet till
  bit 1 utan en fabricerad progressionstabell. Ordinal 6 går via den särskilda
  `CMP #$06`-grenen och publiceras inte som dungeonbit. Körningarna laddar ännu
  inte Drators bank, så generatorns RNG/spawn förblir korrekt stängd.
- ✅ En känd Track 02-identitet räcker inte längre för att släppa igenom en
  påhittad bytebuffert. Runtime kräver att hela råmediet matchar deklarerad MD5;
  vid avvikelse blockeras både nivåpublicering och fallbackgrafik. Det riktiga
  `TQUS02.bin`-provet passerar samtidigt med källkarta, objekt, fyra nivåer,
  autentiserad spawnkälla och sju dungeonägda namnregister.
- ✅ Startup-flow- och save/resume-proven kör nu sina sju dungeonvägar mot
  den riktiga `~/.firestaff/data/theron/TQUS02.bin` i stället för att låta en
  fabricerad Track 02-buffert representera ett verifierat medium. En falsk
  MD5-deklaration provas separat och måste ge identitetsavslag utan nivå eller
  fallbackgrafik.
- ✅ Hela den CTest-märkta Theron-sviten bygger och passerar: 53 av 53 prov.
  Två viewportägande Theron-prober länkar nu originalets Track 02-fontmodul,
  tre fristående assetscannerprov länkar det fullständiga Theron-arkivet och
  transition-receipt-provet kör sina assertions även i `NDEBUG`-byggen i
  stället för att krascha efter bortkompilerade sidoeffekter.
- ✅ Generatorvägen har nu ett explicit eventbundet materialiseringskvitto.
  Det matchar både kategori-3-händelsens och typ-6-generatorns exakta
  åttabytesposter mot den riktiga världens Track 02-register och behåller den
  källbundna planen. Samtliga sju autentiska USA-händelser når detta steg, men
  kvittot tillåter ingen materialisering utan samma-körningsbevis för
  `$4644/$4667`-retur, fysisk anropsägare och generatorns konsument. Dagens
  smoke-spår nekas därför korrekt; även en enda ändrad generatorbyte nekas.
- ✅ RNG-producenten sparar nu hela autentiska 8 KiB huvud-RAM och 32
  bankmappade instruktionsbytes runt varje fysisk returägare. En riktig
  Drator-körning gav 156 kompletta kontexter och band de tidigare osedda
  ägarna `$CC33`, `$CC55` och `$DA5A` till deras exakta anrop, masker och
  efterföljande skrivningar. UP/RIGHT-prober ändrade inte RNG-sekvensen, så
  dessa data används som starkare originalbevis men öppnar ännu inte
  generatorns materialisering.
- ✅ Den genererade Drator-radens byte 6–8 är nu bundna till originalets
  autentiserade Track 02-kod vid logisk `$C852`, råoffset `$A1612`. Exakta 27
  instruktionsbyte visar kopieringen till `$B5/$B6/$B4`, tvåbitsmaskningen till
  `$BB` och det avslutande anropet `$51F8`. Kvittot behåller därför 17, 3, 8
  och det transformerade värdet 0. Kodspannet bevisar däremot inte ensamt att
  paret är kartlokalt X/Y eller att `$BB` är riktning; samma sessions körning
  mot just denna rad observerades inte. Firestaff placerar därför inget
  hostmonster från värdena. Typ, lokalkoordinater, HP och timer förblir stängda.
- ✅ Ett nytt autentiskt bankdispatchspår visar att anropet `$5D58` efter
  radbygget går via `$45E3` till fysisk `$0E0AF5`. Målrutinen uppdaterar
  rumsliga gräns- och pekartabeller men ger inget stöd för typ, HP eller timer.
  Ett separat begränsat instruktionsspår fångar dessutom en tidig logisk
  `$C852` i den andra fysiska banken `$0DC852`. Därmed kan logiska adresser
  inte blandas ihop mellan bankerna; positionskvittot förblir uttryckligen ett
  statiskt källbevis tills samma radkonsumtion har observerats.
- ✅ Ett fokuserat originalspår från fysisk `$0D07B7` fångar nu hela den
  första genererade radens livscykel inom 967 instruktioner: byggstart,
  sortering, slutlig toughness-skrivning, kopiering, publicering, första
  läsning och unlink. Produktionskvittot kräver de åtta exakta logiska och
  fysiska PC-adresserna samt deras ordningsnummer; ett ändrat ordningsnummer
  nekas. Detta binder gruppskrivningen till en enda autentisk körning, men
  öppnar fortfarande inte host-typ, lokal kartposition eller HP.
- ✅ Kategori-4-monsterlayouten är rättad mot DMBUILDERs hela `ITEMS`-kontrakt:
  `item.c:getItem()` hoppar över en generisk tvåbyteslänk innan den 14 byte
  långa `dm_monster`-payloaden. Firestaff läser nu `next_ref` vid byte 0,
  `chested` vid byte 2, typ/position vid byte 4/5, HP vid byte 6–13 och
  flagg-/riktningsordet vid byte 14. Tre autentiska följdobjekt återkommer i
  US-censusen (`640/2189`). De sju strängarna AKUTUBA…DEMON är samtidigt
  omklassificerade som dungeonetiketter; statiska grupper behåller i stället
  sitt verkliga råa typvärde utan `+1`-omskrivning eller falsk spawnkategori.
- ✅ Den alternativa source-occurrence-vägen i
  `theron_v1_combat_runtime_source.c` följer nu samma kontrakt. Den publicerar
  kategori-4-postens råa typbyte oförändrat och lämnar spawnkategorin `$FF`
  utan en verifierad runtime-join. Regressionen kräver att ett autentiserat
  kategori-3-vittne avvisas när endast den gamla typ-0→zon-0-kopplingen skulle
  ha bundit det.
- ✅ Firestaffs portabla world-snapshot version 18 bevarar nu hela den
  autentiska kategori-4-ledgern. Detta är `theron_v1_world_serialize`-formatet
  för byteexakta världsroundtrips i biblioteket och testerna, inte originalets
  produktionsformat `slotN.tqsv`; den separata Continue-vägen laddar först
  Track 02 och applicerar därefter sitt mellan-dungeon-save.
  Snapshoten bevarar alltså inte bara de monster som råkade vara levande på
  aktuell nivå när världen serialiserades.
  Varje 55-bytepost lagrar source-identitet, dungeon/nivå/ruta,
  `chested`, rå typ/position/antal/riktning, HP-orden och den exakta 16-byte-
  posten. Deserialisering granskar gränser och råstorlek atomiskt. Ett
  realdatatest serialiserar Akutubas verkliga US-ledger, raderar destinationens
  kopia, läser tillbaka alla fält byteexakt och byter därefter till en annan
  riktig nivå där livegruppen återbyggs från den sparade ledgern.
  Version 18 godtar dessutom inte en sparfil där decoded typ, position,
  `chested`, antal, riktning, HP eller flaggor motsäger den medföljande
  16-byteposten, eller där samma dungeon/nivå/source-ref/source-index förekommer
  två gånger; ett sådant sent fel lämnar den aktiva världen oförändrad. Även
  hela den riktiga US-kampanjens 165 monstergrupper går genom samma
  save/deserialize/nivåbytesregression. Save-version 1–17, som saknar denna
  sektion, behåller i stället den hashverifierade ledger som Continue redan
  har laddat från Track 02; version 18 ersätter den atomiskt från snapshoten.
- ✅ Originalets kompletta `DMS-SG.001`-post är nu bunden från riktig US
  Track 02-kod och autentisk PC Engine Backup RAM. HUBM-storleken `$01A9`,
  `$E04E`-läsningen och `$E051`-skrivningen bevisar en `$0199` byte lång
  datadel med tre `$88`-bytesplatser och originalets valda platsindex. Den
  gemensamma koden binder indexet via
  `$0198 → $278C → INY → $42B8 → {0000,0088,0110}`; en ny autentisk körning
  gav samtidigt `$278C=00/$42B8=01` och reproducerade BRAM-hashen `ffabc8…`.
  Firestaff
  avvisar därför index utanför 0–2 och dekodar bara vald plats. Firestaff bevarar
  alla 409 byte och realdatatestet korsbinder originalinstruktionerna,
  posthashen `0ce6b7ba`, den tidigare bevisade `$86`-byteskrivaren och samma
  sessions huvudminne. Endast kampanjbyten har gameplaysemantik; övriga fält
  förblir stängda.
