# Firestaff TODO - THERON

> **Latest capture boundary (2026-08-13):** The authenticated Main-RAM
> consumer sidecar is parser-ready but still init-only: `$2600-$27FF` has 512
> zero-valued `$CB22` reads, no `$C3A0` reader, and no dynamic level/object
> consumer. Use `THERON_MEDNAFEN_MAIN_RAM_CONSUMER_PARSE_ONLY=1` for that
> receipt; keep the `$2c54-$2c69` execution-window gate separate.

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## 2026-08-13 — låst dörr är åter en blockerande sentinel

- ✅ Dörrmaskinen skiljer nu `LOCKED=6` från öppningsframerna
  `QUARTER_OPEN..DESTROYED`. Query, movement och `door_open()` kan inte längre
  råka behandla den numeriskt högre låsta sentinelstaten som passabel.
- ✅ Mechanics-hardening-proben täcker låst sentinel i både query och muterande
  öppningsväg. Detta är en lokal state-machine-korrigering; den öppnar inte
  den fortfarande capture-gated T900 key/object-consumern för source-levels.

## 2026-08-13 — ny instrumenterad cold-start når bara CD-transport

- ✅ En färsk extern-disk-körning mot hashverifierad US Track 02/System Card
  läser 256 råa 2352-byte-sektorer från LBA 3234. Den visar dessutom 3 584
  target-writes och 4 096 spawn-consumer-läsningar som rå provenance.
- 🔒 Körningen loopar i BIOS/CD-läsaren: den ger noll game-owned `$E009`-
  dispatchar, noll CD/FIFO→RAM-origin-kvittot, noll RNG-fönster och noll
  autentiserad level/object-consumer. De 512 läsningarna i `$2600–$27FF` är
  alla nollor från `$CB22`; de får inte öppna level, object, square, HUD,
  creature, combat, T700 eller T900.
- 🔒 Spawn-sidecaren läser endast initieringsområdet `$20EC–$20EE` och
  saknar `$B0E5`-kategori, returägarskap och source-owned target-publicering.
  Capturet är därför ett reproducerbart negativt witness, inte ny gameplay-
  semantik. Råa sidecars och den instrumenterade byggningen ligger kvar på
  extern-disk.

## 2026-08-13 — source-gated object handoff is transactional

- ✅ Den redan source-gated object-gameplay-handoffen validerar nu hela den
  valda nivån före mutation och återställer objektpool, aktuell nivå,
  `thing_count` och runtime-media om ett senare placement-steg fallerar.
  Regressionen tvingar fram ett `INT_MAX`-ID på ett kvarvarande objekt och
  verifierar att världens hash och objektpool är oförändrade efter avslag.
- 🔒 Detta ändrar inte semantikgrinden: samma-sessionens autentiserade
  object-consumer krävs fortfarande innan handoffen får öppna dungeon-draw,
  square-to-tile eller fallback-free gameplay.

## 2026-08-13 — source-runtime state invariants are no longer no-op

- ✅ Produktionsadaptern klampar nu championens HP, stamina och mana till
  respektive maxvärde och nollgräns. Champion-death nollställer dessutom
  health och `alive` på samma rena state-livscykel som den redan source-bundna
  creature-retire-rutinen.
- 🔒 Detta öppnar inte attack, spell, AI, RNG, loot, ljud eller T700/T900;
  deras originalkonsumenter är fortfarande fail-closed.

## 2026-08-13 — längre replay når source-owned spawnförkonsument

- ✅ Den autentiserade replayen parseras nu som en positiv execution-window:
  `$CC4C`-konsumenten, 48 `$4644`-förkonsumentprover och 160 `$4667`-helper-
  prover finns i samma sidecar. Testet kräver dessa edges och skyddar samtidigt
  att ingen giltig `$B0E5`-kategori eller RAM-laddad helpergren har observerats.
- 🔒 `$B3=$FF` vid samtliga `$4667`-prover innebär att den särskilda
  `$B3 & 7 == 4`-grenen inte nås. Utan `$B0E5` med A=`0..3`, returägarskap,
  source-owned target-write och live creature-record öppnas inte spawn, RNG,
  AI, combat, loot, generator, T700 eller T900.

## 2026-08-13 — autentiserad CD→RAM-transport från replay är nu verifierad

- ✅ Replayens transition-receipt (`theron-capture-20260813/replay`) är
  parserad som `observed`: 161 råsektorer, 47 byte-exakta CD→RAM-origin-
  receipts och 32 game-owned `$E009`-dispatchar, med hashverifierad US Track
  02 och System Card.
- ✅ Regressionen kräver nu verifierade minimikrav i stället för den tidigare
  felaktiga exakta kampanjlängden (`2` CD-receipts/`3584` RNG-prover), så nya
  autentiserade replaylängder inte avvisas godtyckligt.
- 🔒 Samma replay har fortfarande 512 `$2600`-läsningar, alla från `$CB22`
  och med noll icke-nollvärden. Transporten öppnar därför inte level/object,
  square-to-tile, HUD, creature, combat, T700 eller T900-semantik.

## 2026-08-13 — consumer-receipt skiljer initiering från source-caller

- ✅ Receipten räknar nu separat `$CB22`-initieringsläsningar, övriga
  runtime-läsningar och läsningar från det byte-lockade `$C3A0–$C429`-fönstret.
  För C3A0-fönstret behålls även icke-nollantal och distinkta reader-PC:er.
- ✅ Den externa VDC-replayens main-RAM-sidecar (MD5
  `c6f8f3bc32ce4b29ac32b376096756d1`) passerar parser-only med 311
  target-läsningar, 128 icke-noll, och fortsatt `semantic_publication=blocked`.
  Fälten bevarar caller-proveniens men klassificerar inte level, square,
  object, HUD, creature, T700 eller T900.
- 🔒 Replayen saknar fortfarande autentiserad CD/FIFO→RAM-origin i samma
  session. Ingen gameplaysemantik öppnas av den nya shape-kvittensen.

## 2026-08-13 — autentiserad VDC/VCE-pair från RAM-replay admitted screen-space

- ✅ `theron_v1_vram_trace_load_known_capture_files()` accepterar nu den
  externa, hashverifierade pairen `theron-vdc-ram.exXuQu`:
  VRAM FNV-1a `087da136`, VCE FNV-1a `5376a91b`.
- ✅ Pairen kan användas av produktionsviewportens autentiserade
  screen-space-rendering; råfilerna ligger kvar lokalt på extern-disk och
  kopieras inte till GitHub.
- 🔒 Capturens `$2600–$27FF`-läsningar föregås av samma `$CB22`-rutin som
  skriver nollor till RAM-fönstret. Det är därför inte ett bevis på level-,
  object-, square-, HUD-, T700- eller T900-konsument. De semantiska grindarna
  förblir stängda.

## 2026-08-13 — consumer-receipt skiljer initiering från runtime-läsning

- ✅ Receipten behåller nu antal `$2600–$27FF`-läsningar, antal icke-nollvärden
  och antal distinkta reader-PC:er.
- ✅ Den externa combat-replayen (`live.trace.main-ram-consumer`, MD5
  `4d9da34dd8a0042dc302449af78c54cc`) visar 19 target-läsningar, 3 icke-noll-
  värden och 19 reader-PC:er. Det är starkare runtime-proveniens än
  `$CB22`-initieringen, men replayen saknar CD/FIFO-join och får inte öppna
  level/object, creature, combat, T700 eller T900-semantik.

## 2026-08-13 — game-owned `$2600`-fönster bevaras som proveniens

- ✅ `theron_v1_mednafen_main_ram_consumer_trace_parse_file()` behåller nu
  `target_2600_bytes_present` när en verifierad `main_ram_consumer_read`
  faktiskt ligger i `$2600–$27FF`. Den tidigare slutinitieringen nollställde
  flaggan och kastade bort observationen.
- ✅ Ett nytt parser-test täcker en läsning över fönstergränsen. Den lokala
  MPR-capturen från extern-disken (`mpr.trace.main-ram-consumer`, MD5
  `12f470ef2c38febd9b2c9699dad3b4cb`) passerar parser-only och rapporterar
  `target_2600=present`.
- 🔒 Detta klassificerar endast adressproveniens. Det identifierar inte bytes
  som level, object, T700 eller T900 och öppnar ingen gameplaysemantik.

## 2026-08-13 — textcodonens positionsproveniens är nu bevarad

- ✅ Track 02-textavkodaren behåller varje packat 5-bitarsvärde tillsammans
  med källord och slot (`word_index`/`packed_slot`) i en tokenvy.
- ✅ Codec-lagret skiljer nu råtecken, kända codec-markörer och slutmarkör
  utan att påstå vad de ursprungliga HuC6280-kontrollkoderna betyder.
- ✅ Live world state behåller nu både råa textord och deras positionsbundna
  tokenvy genom dungeon-loadern; en senare consumer-bindning behöver inte
  återskapa tokenpositioner från media.
- 🔒 Detta är förlustfri positionsproveniens, inte en öppning av text-, meny-
  eller HUD-semantik. Den game-owned textkonsumenten och dess VDC-mål måste
  fortfarande bindas i samma körning innan world/UI-publicering tillåts.

## 2026-08-13 — savestate-replayen är fortfarande negativ för game-owned CD

- ✅ En ny lokal replay från den autentiserade dungeon-savestaten gav 65 756
  registerprover, 256 `$B0E5`-adressöverlagringar, 4 096
  `spawn_consumer_read`-rader och 2 213 RNG-prover. US Track 02 och System
  Card var hashverifierade.
- 🔒 Replayen gav bara en CD IRQ efter autoload: noll råsektorer, noll
  source-backed CD→RAM-receipts, noll giltiga `$B0E5`-kategorier och noll
  `$4644/$4667`-prover. Den får därför inte öppna spawn, RNG, AI, combat,
  loot, T700 eller T900.
- ✅ Samma externa US/JP mechanics-playability-probe passerar 79/79 och
  fortsätter att täcka source-bound grid/loader medan de dynamiska
  originalkonsumenterna är fail-closed.

## 2026-08-13 — capturebaserad Theron-regression är verifierad

- ✅ Hela Theron-regressionen på extern `TMPDIR` passerar: 253 valda tester,
  varav 247 körda och 6 korrekta capture-skippar utan lokala fixtures.
- ✅ Med autentiserade lokala fixtures passerar även VRAM/VCE-readiness,
  Main-RAM-consumer och CD-state-sidecar. Den färska replayen ger 161 råa
  sektorer, 51 SCSI-läsningar, 25 CD IRQ, 47 FIFO→RAM-receipts och 65 536
  VDC-skrivningar.
- 🔒 Detta löser testmiljö- och transportblockern. Gameplaysemantik är ännu
  inte öppnad: sessionen saknar game-owned FIFO→RAM-receipt, spawn-consumer
  och RNG-window.

## 2026-08-13 — VDC-I/O-proveniens ingår nu i transition-admission

- ✅ Capture-skriptet räknar autentiska `vdc_io_write`-rader och skriver
  `vdc_io_writes` i transition-receiptet. Receipt-parsern kräver ett positivt
  antal tillsammans med 64 KiB VDC-VRAM och 1 KiB VCE.
- 🔒 Detta binder transportproveniens, inte text-, BAT-, square-, HUD- eller
  gameplaysemantik. Den ursprungliga HuC6280-textkonsumenten är fortfarande
  nästa källgräns.

## 2026-08-13 — TQTR-verifieringen kan köras på extern temporär disk

- ✅ `test_theron_v1_vram_trace_loader` använder nu `TMPDIR` för sin utökade
  TQTR-fixture. Därmed kan den köras när macOS-systemvolymens `/tmp` är full,
  utan att testet skriver till eller kräver plats där.
- ✅ Den autentiserade US screen-space-capturen passerar separat med
  `vram_nonzero=24336`, `bat_tiles=1057` och `presented_nonzero=44947`.
- 🔒 Detta öppnar fortfarande inte square-to-tile, text, HUD- eller
  gameplaysemantik.

## 2026-08-11 — RNG edge capture is still not a spawn handoff

- 🔒 En extern autentiserad US-save-replay observerar `$4644`/`$4667` och
  RNG-fönster, men ingen giltig `$B0E5`-kategori eller target-publicering.
  Den får inte öppna RNG-return, monsterstats, AI, combat, loot, generatorer,
  T700 eller T900.
- ✅ Den äldre 18-fälts-sidecarens 192-stegsformat kan nu läsas utan att
  moderna return-boundary-fält eller semantik uppfinns.

## 2026-08-12 — rått A-värde vid RNG-returgräns sparas, semantik fortsatt stängd

- ✅ RNG-parsern sparar nu A-registret och antal observationer vid den
  instrumenterade stackbaserade returgränsen.
- 🔒 Fältet är endast provenance. Det öppnar inte RNG-return, spawnstats eller
  AI utan en source-bound caller och samma-session target-consumer.

## 2026-08-12 — C96B-only combat-capture registreras som negativt testfall

- ✅ Den autentiserade externa combat-capturen kan nu köras som ett explicit
  negativt parserfall: `$C96B`-läsningar och `$B0E5`-adressöverlagringar
  bevaras, medan avsaknad av `$CC4C` och giltig kategori fortsätter att
  avvisa runtime-semantic publication.
- 🔒 Detta är captureklassificering, inte återvunnen RNG, AI, combat, T700,
  generator eller T900-semantik.

## 2026-08-12 — ny Stage-2-session saknar fortfarande gameplay-consumer

- ✅ En ny isolerad session med verifierad direkt-SDL2-binär nådde riktig
  Stage-2/System Card-kod och producerade 2 048 registerprover.
- 🔒 Sessionen saknade `$CC4C`, `$B0E5` och efterföljande dungeon-/objectmål;
  den får därför inte öppna creature-, RNG-, T700- eller T900-semantik.

## 2026-08-12 — JP-porträtt och originalmekanik är fortfarande öppna

- 🔒 JP Track 02-rosterposterna är autentiserade, men ingen source-bound
  porträttpixelkonsument eller porträtt-ID-bindning är fångad. `portrait_index`
  ska därför fortsätta vara `THERON_PORTRAIT_UNAVAILABLE`.
- 🔒 Paritetsmatrisen räknar nu fixture-/numeric-record-bevis som `PARTIAL` för
  combat och champion-systemet; T500/T600/T900-konsumenterna måste fortfarande
  bindas mot samma-session runtime-data innan produktionen öppnas.
- 🔒 Den nya externa combat-capturen är verifierad som autoload/C96B-only:
  ingen `$CC4C`, giltig `$B0E5`-kategori eller CD→RAM-loadertransition. Den får
  inte användas för att fylla i syntetisk AI, RNG, T700 eller T900-semantik.
- 🔒 Samma capture har ett nytt VDC/VCE-par som nu kan replayas screen-space;
  square-to-tile, HUD- och gameplayägarskap är fortfarande separata gates.

## 2026-08-11 — ljudkonsument förblir capture-gated

- 🔒 Den statiska System Card-katalogen klassificerar riktiga CD/ADPCM-vektor-
  anrop, och den autentiserade capture-vägen binder CD/FIFO→ADPCM-RAM.
- 🔒 Ingen samma-session CPU-läsning, sample-start eller spelhändelseägare är
  verifierad ännu. `theron_v1_play_sound()` ska därför fortsätta returnera
  fail-closed; inga creature-, actuator- eller menyhändelser får trigga
  syntetiska ljud. Paritetsmatrisens tidigare `PROVEN`-rad är korrigerad till
  `PARTIAL`.

## 2026-08-11 — registertrace kan nu binda `$C3A0`-callerfönstret

- ✅ Mednafen-instrumenteringen skriver nu optional `record_c3a0_window=1`
  i samma registertrace som `$C96B/$CC4C`; parsern räknar fönstret utan att
  bryta äldre v3-traces.
- 🔒 Flaggan är captureproveniens, inte semantik. `$C3A0` måste fortfarande
  fångas i samma körning som dess `$C96B/$CC4C`-anrop och målskrivningar innan
  creature-, objekt-, generator-, T700- eller T900-regler öppnas.

## 2026-08-11 — ny autentiserad `$C3A0`-caller är source-lockad

- ✅ Ett nytt 150-byte US Track 02-fragment från raw-offset `$9C450` / HuC6280
  `$C3A0` matchar `TQUS02.bin` byte för byte och FNV-1a `$666DED61`.
- ✅ Disassembly-admissionen verifierar nu fragmentet tillsammans med de
  befintliga `$4667`, `$C96B` och `$CC4C`-fönstren.
- 🔒 Fragmentet visar källkodens caller-/tabellflöde men identifierar inte
  `$2998/$299C` som creature-, generator-, T700- eller T900-records. Ingen
  spelsemantik öppnas utan samma-session runtime-bevis.

## 2026-08-11 — live source creatures no longer receive synthetic PASSIVE AI

- ✅ Category-4 creatures admitted from authentic US/JP Track 02 records now
  carry `THERON_AI_UNAVAILABLE` until the original T500/T600 AI consumer is
  authenticated. The AI tick ignores that explicit unavailable state.
- 🔒 This is a correctness boundary, not recovered AI: RNG-spawn, creature
  AI, attacks, damage, loot, generator timing, T700 and T900 remain closed
  until the disassembly consumer and a same-session runtime capture agree.

## 2026-08-11 — flerfönster-RNG-captures valideras korrekt

- ✅ RNG-consumer-parsern räknar nu kompletta 512-stegsfönster i en längre
  samma-session-trace; en korrekt trace avvisas inte längre bara för att den
  innehåller flera fönster.
- ✅ Extern US Track 02-capture har 22 kompletta `$5D64`-fönster och en
  source-byte-matchad `$5D64`-kodwindow.
- 🔒 Detta bevisar källkonsumentens körning och kodproveniens, men inte ännu
  vilket returvärde som ägs av spawnstats eller senare creature-semantik.

## 2026-08-11 — inventory transitions now require the authenticated property table

- ✅ The loaded level now retains whether the complete source-owned 66-row
  Track 02 item-property table matched the selected US/JP bank.
- ✅ Source inventory swap/drop transitions require both the object-record
  header and that table-authentication bit; a map header alone is no longer
  sufficient.
- 🔒 This remains provenance validation. T900 equip/use/stack semantics are
  still not implemented without the original consumer capture.

## 2026-08-11 — authenticated BAT preview now decodes real PCE tiles

- ✅ The source-bound VRAM/VCE presentation route now runs every admitted BAT
  tile through the real PCE planar 2/4bpp decoder before applying its BAT/VCE
  palette group. It no longer treats raw 32-byte 4bpp planes as indexed
  pixels.
- ✅ The real external US dungeon pair (`VRAM=5d20ebc7`, `VCE=ea83f117`)
  passes the production capture test with 1,057 atlas tiles, 896 screen cells
  and a non-empty authenticated frame.
- 🔒 This fixes bitmap decoding only. Square-to-tile, depth/perspective and
  creature/object atlas ownership remain separate source-consumer gates.

## 2026-08-11 — disassembly-visible spawn arithmetic is receipt-only

- ✅ `theron_v1_track02_apply_spawn_consumer_witness()` now reproduces the
  instruction-visible arithmetic in `$B0E5-$B1EB` from a same-session witness:
  category branches, `$B8` scaling, `$B4/$B5` divide, bounded `$4667` values,
  HP cap `#$0384` and the `$2980/$2990` caps.
- 🔒 This API does not generate RNG values, does not publish `Theron_SpawnStats`
  and is not wired into creatures. `$5A76`, `$5B8F`, `$D23A`, `$4667`, the
  `$2A10/$D0FE` writes and the later stat/AI/combat owners still need one
  authenticated runtime execution window before gameplay semantics can open.

## 2026-08-11 — M11 handoff regression test is headless-safe

- ✅ The boundary test uses SDL dummy audio by default, preventing a local
  CoreAudio wait from being mistaken for a Theron runtime hang.
- 🔒 This does not alter production audio-device selection.

## 2026-08-11 — JP roster text now copies verified raw bytes

- ✅ JP startup names and titles are emitted from the authenticated raw
  offsets after matching, rather than from the expected search literals.
- 🔒 This proves payload provenance only; the original JP portrait/font/VDC
  consumer remains unresolved.

## 2026-08-11 — authenticated manual VRAM/VCE capture is admitted

- ✅ The production viewport now accepts the externally captured US Track 02
  screen pair `VRAM=5d20ebc7`, `VCE=ea83f117` after exact-size/hash checks.
- 🔒 This is screen-space bitmap/palette ownership only; square-to-tile,
  perspective, HUD and gameplay consumers remain separately gated.

## 2026-08-11 — inventory property category is source-checked

- ✅ Pickup, source-slot movement and drop now reject a carried record when
  its property-category byte no longer agrees with the source object class.
- 🔒 This hardens provenance only; property-byte meaning and T900 equip/use/
  stack rules remain unpromoted.

## 2026-08-11 — unbound spawn categories are now fail-closed

- ✅ Direct level loads no longer copy a reconstructed static spawn-zone
  category into live creature provenance.  The field is published only after
  an authenticated US Track 02 spawn source is bound.
- 🔒 This does not enable random spawning, AI, combat, generators, T700 or
  T900 semantics; those still require their original runtime consumers.

## 2026-08-11 — source creature IDs now survive pool rebuilds

- ✅ Both authenticated category-4 level materialization and explicit source
  admission derive IDs from `source_ref` plus member slot. Removing or
  reloading a pool no longer renames a source creature by its array position.
- 🔒 This is provenance-only; no unproven RNG, AI, combat, generator, T700,
  T900, loot, presentation or event-audio semantics were enabled.

## 2026-08-11 — unbound source creatures cannot enter fixture combat

- ✅ Source-backed members with authentic HP remain visible/collidable, but
  champion damage, creature attacks and spell damage now reject them while
  their original attack consumer is unknown.
- 🔒 This is a safety boundary, not completed combat parity; the real attack,
  damage, AI and event-sound owners still require the authenticated runtime
  capture described below.

## 2026-08-11 — category-4 members now materialize from real HP records

- ✅ Live static creatures are now admitted one-for-one from authenticated
  Track 02 category-4 group members. Each member copies its real HP word,
  packed cell ordinal, group count and source identity into the runtime pool;
  the previous fixture-stat path is no longer used for this source route.
- 🔒 Attack, defense, speed, AI, loot and generator behavior remain explicitly
  unpopulated until their original consumers are bound by the HuC6280
  disassembly and a same-session authenticated runtime capture.

## 2026-08-11 — production replay now uses the authenticated native screen consumer

- ✅ När ett hashverifierat VRAM/VCE-par uttryckligen monteras går Therons
  produktionsviewport nu via den explicita 256×224 native-screen-konsumenten.
  Det riktade real-capture-testet jämför produktionsframebuffern byte för byte
  med den autentiserade screen-routen.
- 🔒 Detta är fortfarande screen-space BAT/tile/VCE-bindning. Ingen cell
  tilldelas till square-to-tile, perspektiv, HUD, objekt eller creature-
  semantik utan motsvarande originalkonsument.

## 2026-08-11 — save-state `$B0E5` hits are not the regular-spawn caller

- ✅ Den ombyggda externa Mednafen-capturen mot den riktiga US-CUE:n loggade
  nu även HuC6280-stackens returord vid varje `$B0E5`-träff. Den autentiserade
  Track 02-hashen är fortsatt `f23601102138f87c33025877767ebf76` och capturen
  gav 50 `$B0E5`-träffar.
- 🔒 Alla 50 träffar hade A=`$2C` eller A=`$85`, inte disassemblyns spawn-
  kategori 0–3, och ingen träff följdes av `$4667`, `$5D64` eller `$5D6A`.
  Stackorden var dessutom `return_pc=$0002`/`$3F3F`, vilket inte är en
  verifierad game-code caller. Detta är därför ett avvisat overlay-/state-
  witness, inte ett RNG- eller spawnbevis. RNG, AI, generatorer, T700, T900,
  loot och combat får inte öppnas från denna session.
- 🔧 Nästa capture måste nå en faktisk dungeon-tick eller objektaktion och
  samtidigt visa giltig caller, kategoriargument, RNG-retur och konsumentens
  målskrivning i samma autentiserade session.

## 2026-08-11 — `$B07D` caller window is source-locked

- ✅ Den statiska US-disassemblyn har nu en separat, hashverifierad caller-
  window för `$B07D-$B1EB`. Den visar fyra `$4644`-anrop före `$B0E5` och
  vilka register-/RAM-fält som förs in i dispatchen.
- 🔒 Window:n bevisar ännu inte att `$2980/$2990/$29A0` eller `$2A20/$2A28`
  är creature-statistik. Nästa positiva capture ska binda samma caller,
  giltig kategori 0–3, RNG-retur och efterföljande writes till ett riktigt
  Track 02-record innan någon gameplaysemantik aktiveras.
- ✅ Register-sidecaren kan nu märka den statiska caller-window:n som
  `caller_b07d_window=1`; äldre v3-sidecars fortsätter att läsas som
  provenance utan den nya flaggan.

## 2026-08-10 — README capture is reference-only

- 🔒 The published screenshot documents the original US presentation only.
  A Firestaff-native capture with authenticated rendering/gameplay parity is
  still required before claiming Theron is complete.

## 2026-08-10 — BAT→VCE relation is bound; world mapping remains open

- ✅ The authenticated VRAM/VCE loader now verifies BAT palette-group bits
  against the exact VCE snapshot and exposes the relation receipt.
- 🔒 The same evidence still does not identify which decoded screen-space BAT
  cells belong to a dungeon square, depth/perspective slot, HUD element,
  object, or creature. Those consumers remain source-capture gated.

## 2026-08-10 — File-select replay still lacks regular-spawn handoff

- ✅ Kompletta `Run → Button I → rörelse`-replayen mot verklig US Track 02
  gav 28 autentiserade CD→RAM-originreceipts och 32 `$E009`-dispatchar.
- 🔒 Samma session gav noll `$B0E5`, RNG-returner, spawn-consumer reads och
  target writes. Nästa capture måste nå en verifierad dungeon-tick innan
  RNG/AI/generator/T700/T900 eller loot kan implementeras.

## 2026-08-10 — save-state replay reaches only a rejected `$B0E5` overlay

- ✅ En autentisk Mednafen-save-state kördes mot den kompletta råa
  MODE1/2352-US-CUE:n på extern disk. Capturen verifierade Track 02-hashen
  `f23601102138f87c33025877767ebf76` och observerade 30 träffar på `$B0E5`.
- 🔒 Samtliga träffar hade A=`$2C` eller A=`$85`, inte disassemblyns giltiga
  regular-spawn-kategorier 0–3. Parsern avvisar därför korrekt träffarna som
  samma-adress-overlay; ingen RNG-return, spawnrecord, AI, loot, T700 eller
  T900-semantik öppnas. Den tidigare 2048-byte CUE-körningen avvisades också
  eftersom den saknade authenticated CD→RAM-origin.

## 2026-08-10 — complete US CUE capture remains transport-only

- ✅ Kompletta `TQUS.cue` med 19 spår kördes från extern disk. Track 02
  rekonstruerades med arkivets verkliga `TQUS19.iso + TQUS02End.iso`.
- 🔒 Sessionen gav 159 råsektorer, 88 spawn-registersamples, 17 `$4644` och
  64 `$4667`, men noll giltiga `$B0E5`, RNG-windows, spawn-consumer reads eller
  target writes. Semantiska konsumenter är fortsatt stängda.

## 2026-08-10 — all decoded Track 02 occurrences retained; consumers gated

- ✅ Den riktiga US-kampanjen behåller nu alla 2 266 autentiska ground-reference-
  occurrences i world source-ledgern, inklusive control records och carried
  objects. Detta är lossless provenance från Track 02, inte syntetisk data.
- 🔒 Originalets RNG, spawn-timing, creature-AI, attack/skada/loot, T700/T900,
  itemsemantik och source-bound presentation/ljud är fortsatt spärrade tills
  deras riktiga consumers är bundna av disassembly och samma-körnings-capture.

## 2026-08-10 — inputfix klar; semantikspärrar kvar

- ✅ Held WASD/piltangent-input är nu kopplad till Therons egen tick-cadence.
  Vanlig mus rör pekaren fritt utan objekt-hopp; Button I/II och touch är
  oförändrade.
- 🔒 Detta ändrar inte den separata spärren för originalets RNG, creature-AI,
  T700/T900, objectrecords eller source-bound ljud/presentation.

## 2026-08-10 — remaining creature semantics are source-capture gated

- ✅ Removed the unauthenticated DMWeb/DM1 creature-generator fallback; real
  Track 02 category-4 records are the only source for live creature creation.
- 🔒 Do not add replacement tables. The next implementation witness must bind
  the original RNG return, generator reactivation/timing, AI/attack/damage/loot
  consumers and T700/T900 state writes in one authenticated runtime.

## 2026-08-10 — cold-start transport witness is still semantically negative

- ✅ En extern cold-start mot US Track 02 verifierade 159 råsektorer, 32
  `$E009`-dispatchar, två CD→RAM-originreceipts, 17 `$4644`- och 64
  `$4667`-observationer samt VDC/VCE-snapshots i samma autentiserade session.
- 🔒 Samma körning gav noll `$B0E5`, noll specialgren, noll RNG-fönster och
  noll målskrivningar. Implementera inte RNG, spawn, AI, strid, loot,
  generatorer, T700 eller T900 från detta; nästa witness måste fånga en
  faktisk dungeon-/spawn- eller objektkonsument.

## 2026-08-10 — VDC/VCE screen-space capture admission

- ✅ Produktionsintaget har nu en stängd allow-list för fem verifierade
  kompletta VRAM/VCE-hashpar. US-dungeon, US-interaktiv, JP-start och
  US-cold-start passerar den riktiga BAT/tile/palett-bindningen och M11-
  presentationen från extern disk.
- 🔒 Detta är fortfarande en skärmkvittens. Square-to-tile, perspektiv,
  HUD-/objektkonsument, monster, RNG, T700 och T900 öppnas inte av en
  screen-space-snapshot.

## 2026-08-09 — aktuell kalla capture har endast transportbevis

- ✅ Den autentiska US-körningen når `transition=observed` och ger fyra
  byteidentiska source-backed CD→RAM-receipts som nu kan verifieras i båda
  receiptformaten.
- 🔒 Samma körning har inga `pce_cd_fifo_origin_main_ram_consumer`-rader och
  ingen RNG-return/spawn-entry. Originalets creature-, T700-, T900-, item-,
  grafik- och ljudsemantik får därför fortfarande inte implementeras från
  denna transport-only evidens.

## 2026-08-09 — nästa capture kräver aktiv dungeon

- 🔧 Capture-scriptets macOS-input-grab är nu retry-säkert och väntar på både
  Quartz-kvitto och Mednafen-gjord `InputGrab=1` innan sekvensen skickas.
- 🔒 Nästa autentiserade körning måste använda den verifierade startupkedjan
  till Akutuba och därefter nå aktiv dungeon; den tidigare bounded-körningen
  stannade före game-owned CD→RAM-consumer. RNG, spawn, AI, T700, T900 och
  presentation är fortsatt spärrade tills samma körning binder konsumenterna.

## 2026-08-09 — summary-only original-consumer admission stängd

- ✅ Runtime-admission kräver nu råa, exakt sammanfogade
  `pce_cd_fifo_origin_main_ram_receipt`/`...consumer`-rader i samma capture
  för palett-, non-startup- och object-table-offsetarna. Ett sammanfattnings-
  kvitto utan dessa rows öppnar inte längre original-consumersemantik.
- 🔒 Den riktiga externa US-sessionen är fortfarande korrekt blockerad: den
  har två CD→RAM-originreceipts men ingen game-owned FIFO-consumer. Nästa
  steg är en ny autentiserad session som faktiskt producerar dessa rader;
  RNG, spawn, AI, T700, T900, rendering och save förblir stängda tills dess.

## 2026-08-09 — kombinerad cold-start fortfarande utan spawnretur

- 🔒 En ny bounded cold-start på autentisk US Track 02 gav 256 verifierade
  CD→RAM-originreceipts, 26 `$E009`, 33 `$4644` och 96 `$4667` i samma
  session, men noll `$B0E5`, RNG-samples och `.rng-code`-windows. De
  förkonsumenterna är därför inte en RNG-retur eller spawnhändelse.
- 🔧 Capture-scriptets `pce_fast`-gate avvisar nu builds som bara innehåller
  `pce_fast`-strängar men inte annonserar modulen i Mednafen:s egen modulista.

## 2026-08-09 — råkodens source-byte-join verifierad

- ✅ Parsern för `.rng-code` kräver nu sidecar-header, korrekt `$5D64/$5D6A`,
  256 byte hexkod, HuC6280-adressgräns och den riktiga 8 104 992-byte US
  Track 02-filen. Den jämför hela fönstret mot de sju observerade offsetarna
  `0x975c4 + n*0x49800` och körs med den riktiga externa capture-receipten.
- 🔒 Det här bevisar byteproveniens men inte mappad bank, RNG-returvärde,
  caller, spawnkategori eller gameplaysemantik. Nästa witness måste fortfarande
  binda samma körning till den riktiga retur- och spawnkonsumenten.

## 2026-08-09 — rå RNG-kod fångad, semantiken fortfarande spärrad

- 🔧 Capture-scriptet och den reproducerbara Mednafen-patchkedjan skriver nu
  `.rng-code` med 256 faktiska byte vid `$5D64/$5D6A`, logisk PC och fysisk
  HuC6280-adress. En autentiserad `.mc0`-körning gav `$5D64`, 50 `$B0E5`-
  entries och 512 instruktionsprover.
- 🔒 Körningen saknade CD→RAM-originreceipts och visade ingen verifierad
  RNG-returägare. Råkodsidecaren får därför inte användas för att hitta på
  RNG-värden, monsterstats, AI, loot, T700 eller T900.

## 2026-08-09 — kvarvarande Theron-semantik efter teleporterfix

- 🔧 Den externa Mednafen-capturen har nu en explicit förlängd, begränsad
  registergräns. En autentiserad `.mc0`-körning nådde `$B0E5` och `$5D64`,
  medan en separat cold-start bevisade CD→RAM-transport och `$4644`/`$4667`.
  Sessionerna hålls separata; inget RNG-, spawn-, AI-, T700- eller T900-
  resultat publiceras från dem.

- 🔒 RNG-return, levande creature-AI, attacker/skada/loot, generatorernas
  timing, T700-statistik och T900-regler är fortfarande spärrade tills samma
  autentiserade runtime-capture binder deras riktiga konsumenter.
- 🔒 Dungeonmaterialbank, perspektiv/square-to-tile, VCE-palettägare,
  bitmapdekomprimering, US-textconsumer, JP-porträtt och ljud/ADPCM/SFX-
  konsument är fortfarande separata source-join-gates.

## 2026-08-09 — native SDL-capture verifierad men semantik fortsatt spärrad

- ✅ Capture-scriptet accepterar nu en autentiserad instrumenterad Mednafen-
  PCE-binär även när dess `-help` saknar modul-listan. Fallbacken kräver
  binärsignaturerna för PCE CD-kärnan och lämnar media-, runtime- och
  semantikgates oförändrade.
- 🔒 En riktig körning med native SDL 2.32.10, USA Track 02, System Card och
  savestate producerade VDC/VCE-snapshots och autentiserade input/CD-start-
  receipts, men nådde inte game-owned CD→RAM-consumer: `host_keys=0`,
  `authenticated_cd_ram=0` och inga dynamiska RNG/creature/AI/T700/T900-
  receipts. Ingen semantik får därför öppnas från denna körning.

## 2026-08-09 — verifierad spärr för senare-nivåns frame-chain

- 🔒 Äkta US/JP senare-nivåblock och deras sexbytesframing är hashverifierade,
  men ett direkt försök att köra US nivå 1 genom den platta host-liften
  stannar vid `DECODE_POINTER_TABLE`, även när den gemensamma `$E8`-prologen
  endast används som diagnostiskt seed. Det är ett negativt bevis, inte en
  anledning att skapa en tabell.
- 🔧 Nästa capture måste binda rekursionen `$23DC -> $23AD`, frame-chainens
  slut, destinationspekaren, MPR-tabellen `$3B7E-$3B85` och den efterföljande
  `$2600`-konsumenten i samma autentiserade körning. Fram till dess är
  bitmap/tileatlas, square-to-tile, perspektiv, VCE-palett och objektsemantic
  fortsatt stängda.

## 2026-08-09 — kvarvarande source-semantik efter dungeon-lookup-fix

- 🔒 Dungeon-aware source-creature lookup är verifierad. Den stora spärren
  kvarstår: originalets `$B0E5`/`$4644`/`$4667`-RNG/spawnretur, T500/T600-AI
  och attack/skada/loot, T700-statkonsument samt T900 object/inventory-ägare
  har ännu inget komplett autentiserat runtime-bevis och får därför inte
  ersättas med hostdata.

## 2026-08-09 — kvarvarande startupspärr efter InputGrab-bevis

- 🔒 Den nya v15-capture-binären bekräftar Mednafen-ägda
  `input_grab_state enabled=1` efter den riktiga macOS-
  `Ctrl+Shift+G`-chorden. `Z`/`X` levereras som SDL-scancode 29/27 och
  Run som 40, men den autentiska US Track 02-körningen står fortfarande i
  System Card/BIOS: 47 PCE-inputtransaktioner, 2 IRQ2-callbacks, 0 råa
  sektorer och alla PCE-läsningar returnerar `0x3f`. Nästa steg är därför
  startup-/CD-frame progression med autentisk runtime, inte fler host-
  tangentbindningar. Ingen RNG-, creature-, AI-, T700- eller T900-semantik
  får öppnas från denna negativa capture.

## 2026-08-09 — loader-write instrumentation

- 🔧 Capture-builden applicerar nu en post-patch `v3`
  `main_ram_loader_write`-hook. En ny riktig Mednafen-körning med en native
  SDL2-runtime återstår; den kompilerade lokala binären är därför ännu inte
  ett runtimebevis.

## 2026-08-09 — efter byte-dekomprimeringslyftet

- 🔒 Den fullständiga retailrutinen `$23AD–$252A` är nu lyft på byte-nivå och
  testad med säkra gränser. Nästa nödvändiga bevis är samma-capture MPR-tabell,
  destination och pointer-table-state från stage-2 för en verklig senare nivå.
  Utan den får de autentiska avkodade bytesen inte kallas tileatlas, bitmap,
  dungeonmap eller objektrecord.
- 🔧 Bind `theron_v1_huc6280_decode_resource()` till en sådan autentiserad
  runtime-window och kontrollera resultatets längd/hash mot spelkonsumentens
  CD-sektor och `$2600`-RAM. Därefter kan atlasbindning och square-to-tile-
  mappning tas vidare; RNG/AI/T700/T900 förblir separata capture-gates.

## 2026-08-09 — fortsatt autentisk runtimecapture

- 🔒 Den senaste rena v3-capturen använde replayen
  `run@8:60,i@480:30,i@900:30,i@1320:30,i@1800:30` på äkta US Track 02.
  Alla fem scripted events verifierades på PCE-bussen: Run=`0x0008` och
  Button I=`0x0001`. Capturen producerade 5 943 inputprover, 161 råa sektorer
  och 87 MPR-bundna spawnregisterprover. Den nådde fortfarande inte `$B0E5`,
  någon spelägd dynamisk CD-läsning eller ett dynamiskt returkontrakt; RNG,
  creatures, AI, loot, T700 och T900 förblir därför spärrade.

- 🔒 En ny 120-sekunders v3-capture med replayen `run@8:60,i@480:30,i@900:30`
  använder nu den korrekta startupsekvensen Run följt av Button I på äkta US
  Track 02-media. Den verifierade PCE-inputreceipten innehåller 10 145
  inputprover, `I=0x0001` och `Run=0x0008`, samt 161 råa sektorer och 215
  spawnregisterprover. Capturen saknar fortfarande `$B0E5`, spelägd dynamisk
  CD-läsning och `$C96B/$CC4C`-konsumentretur; den avvisas därför fortsatt av
  den strikta grinden och får inte driva T900, RNG, AI, loot eller T700.
- 🔒 Den nya v3-sidecaren är nu strikt: en semantisk spawn-korrelation måste
  observera `LB0E5` (`$B0E5`) i samma körning som `$4644`/`$4667`, konsument-
  fönstren och det dynamiska returkontraktet. En v3-capture nådde 161 äkta
  Track 02-sektorer och 87 registerprover men saknade `$B0E5`; den avvisas
  därför korrekt och får inte driva T900, RNG, AI, loot eller T700.

- 🔒 Om komma/punkt inte reagerar i en native Mac-körning ska fångsten först
  ha ett godkänt Quartz-hjälparbygge och Mednafen måste ha input-grab aktivt.
  `Z`/`X` är den layoutstabila Button I/II-fallbacken. Detta påverkar inte
  spärren för spelägd CD-läsning eller den senare RNG/AI/T700/T900-semantiken.
- 🔧 macOS global-HID-hjälparen rapporterar nu den observerade frontmost-PID:n
  och använder den som fokusbevis. En lokal körning stoppades fortfarande när
  macOS höll ett annat fönster frontmost; detta är ännu inte ett spelägt
  input- eller CD-handoffbevis.
- ✅ En separat execution-window-parser godkänner nu den riktiga state-capturens
  2 048 registerprover i `$C96B–$CA69`/`$CC4C–$CD13` även när `$4644` och
  `$4667` saknas. Register-PC:n valideras mot HuC6280:s fulla 21-bitars
  bankadressrymd i stället för felaktigt enbart `$1fxxxx`.
- 🔒 Den strikta semantikgrinden kräver fortfarande `$B0E5`-spawnentry,
  `$4644`-preconsumer, `$4667`-helper och dynamiskt returkontrakt. Den nya
  receipt-vägen publicerar inga RNG-, creature-, AI-, loot-, T700- eller
  T900-regler.
- 🔧 Förena i en och samma autentiserade körning `$4644`/`$4667`, hela
  `$C96B–$CA69`-konsumentfönstret och de RAM-läsningar som instrumenteras som
  `spawn_consumer_read`. Nyspelsreplay bevisar nu preconsumer/helper, medan
  state-autoload bevisar `$C96B`-fönstret; två separata körningar får inte
  blandas till ett syntetiskt spawnrecord.
- ✅ Macens Mednafen-profil har nu en fungerande input-grab-genväg på
  `Ctrl+Shift+G`; standardens `Ctrl+Shift+Menu` fungerar inte på tangentbord
  utan Menu-tangent. Därmed kan explicit konfigurerade komma/punkt-bindningar
  för Button I/II nå den emulerade PCE-handkontrollen.
- 🔧 Capture-scriptet verifierar nu själva PCE-wiremaskerna i varje scripted
  input-receipt: Button I `0x0001`, Button II `0x0002`, Select `0x0004`, Run
  `0x0008` och riktningsbitarna `0x0010..0x0080`. En gammal eller felbyggd
  Mednafen-binär stoppas i stället för att kunna ge ett falskt positivt
  inputunderlag. Den nya rena instrumenteringen passerar maskkontrollen med
  riktig US Track 02-media; spelägd CD-läsning är fortfarande nästa spärr.

## 2026-08-08 — nästa T900-bevis

- 🔧 Inputtransporten är nu verifierad med riktiga PCE-wiremasker och lokal
  macOS-profil: Button I `Z`, Button II `X` (layout-stabila SDL-bindningar).
  Komma/punkt får bara användas när de uttryckligen finns i `mednafen.cfg`.
  IRQ2-tracen har nu korrekt MPR-baserad fysisk PC-proveniens. Fortsätt fånga den saknade
  spelägda CD-läsningen efter de 161 autentiska Track 02-sektorerna innan
  någon RNG-, AI-, T700- eller T900-semantik aktiveras.

- 🔧 Använd den sparade source-spawnkategorin när den autentiserade RNG-
  konsumenten fångas. Kategorin är nu provenance i live-poolen, men får inte
  driva HP, AI, attack eller generatorer innan `$4667`/`$5D64`/`$5D6A` är
  runtimebundna.

- 🔧 Råa itemrecords följer nu inventory genom pickup, drop och save/load.
  Bind den ursprungliga T900-konsumenten för equip/use/stack och validera
  dess state-skrivningar mot samma bytes innan någon regel aktiveras.

- 🔧 Kör den befintliga Mednafen/System Card-capturevägen med originalmedia
  för att ersätta `ram_consumer_2600=not_present`; utan den fångsten ska
  T700/T900-statistik, loot, AI och generatorlogik fortsatt neka mutation.

- 🔧 Bind den bevarade US-textcodonströmmen till originalets HuC6280
  textkonsument och kontrollkodtabell. Loadern får inte göra en hoststräng av
  `{...}`-värden innan den kedjan är fångad.

- 🔧 CDDA-intag och stream-handoff är verifierade mot den lokala original-RAR-
  korpusen. Bind fortfarande originalets spelhändelser till rätt CDDA/ADPCM-
  eller SFX-konsument innan ljud triggas från creature-, actuator- eller
  menylogik.

## Theron Authentic CD Trace Follow-up (2026-07-12)

2026-07-13 live stage-two correction: the authentic US-CUE/System Card capture
2026-07-15 post-`$3800` order gate: a future positive transcript must now
record the original Stage 3 `BRK $ff` IRQ2 return from `$3800` to `$3802`
before its later `$e009` dispatch. This proves ordering through the original
loader entry only. It does not classify the later sector, promote a grid,
or establish level, object, bitmap, palette, or transition semantics.

## Theron CUE IPL/Stage-Two Follow-up (2026-07-12)

The documented converted CUE layout now resolves only its explicit
that other selectors are CD commands or bind any later record to an object or
level; later loader execution evidence is still required.
physical MODE1 sectors are validated in JP/US media. Its payload role remains
218-unit manifest envelope, but its entries remain unclassified; do not treat

## Theron Track 02 Semantic Binding Follow-up (2026-07-11)

## Theron Original SRM Body Correlation Follow-up (2026-07-11)

Startup now exposes only fully gzip-trailer-authenticated unknown Save Disk
containers as opaque transfer candidates. They remain unavailable to Continue,
and failed SRM Continue leaves the world unchanged. The outstanding work is
still source-backed original body-layout correlation before any original SRM
can restore progression, party, or runtime state. Firestaff-native SRM export
also now publishes atomically without replacement, so it cannot overwrite a
staged original Save Disk artifact while the corpus remains unbound. The
direct SRM runtime handoff now requires all four hash-verified Track 02 media
surfaces and a selected real-media level bank before committing a restored
world; identity-only media rejects without mutation. Its structured receipt
now exposes the consumed media route mask, checksum, and selected level bank;
the remaining SRM blocker is still only original body-layout correlation.

## Theron's Quest

### Theron V1

- 🔧 2026-07-15 Track 02 post-Stage-2 `$e00f` service boundary: the same
  authentic 45-second boot receipt now covers direct non-System-Card calls to
  both System Card loader entries. Across two Stage-2 returns and 52 observed
  post-stage physical code pages, the only `$e00f` call is the already-known
  Stage-2 `$40a4 -> $e00f` setup, with `ff0000`/`ffff`/`ff` sentinel fields;
  the only `$e009` call remains `$3840` with the same invalid fields. No later
  direct game loader call to either entry and no game-owned `$1801` writer is
  observed. Indirect, block-transfer, or unobserved-route calls remain
  unclassified, so this is a boot-path boundary, not a universal absence
  claim. The next route still requires a non-sentinel caller correlated with
  a raw-sector receipt and verified return destination.

- 🔧 2026-07-15 Track 02 post-Stage-2 game-call boundary: an authentic
  45-second US CUE + System Card 3.0 capture accepts two real host RUN
  transitions, reaches two Stage-2 returns, and observes 61 physical code
  pages afterwards. It contains exactly one direct non-System-Card
  `$3840 -> $e009` call, but its record (`ff0000`), destination (`ffff`), and
  mode (`ff`) are all sentinel values; it is not followed by a game-owned
  `$1801` writer (only System Card `$e90d/$e92d/$e981` are observed). The
  candidate therefore remains rejected and cannot be treated as a later
  record or dungeon handoff. Next evidence must be a non-sentinel game call
  correlated with a subsequent raw-sector/SCSI receipt and a verified return
  destination.

- 🔧 2026-07-15 Track 02 live SCSI caller/destination boundary: a fresh
  authentic US CUE + System Card 3.0 capture records every `$1801` SCSI CDB
  byte with its HuC6280 caller, alongside each decoded READ(6) packet and raw
  sector binding. All 48 observed READ(6) packets, including later reads
  through generation 48 / LBA 4265, were issued by System Card `$e981`
  (command bytes) after `$e90d` selection; FIFO bytes were copied only by
  `$ea50` into System Card RAM `$1f:2256+`. No non-System-Card CD caller,
  dynamic `$e009`, or game-owned destination was observed, so none of those
  later records may enter the dungeon handoff. Next admissible evidence is a
  real game-code caller and destination after the System Card returns, tied
  to a hash-verified Track 02 sector and an original level/object consumer.

- 🔧 2026-07-14 Track 02 initial-level payload handoff: the one complete,
  trace-witnessed 2048-byte `$e009` payload is now copied atomically from the
  rehashed original MODE1 user-data sector into the runtime boot receipt.
  Record `0x0b52`, source coordinate `0x114`, destination `$3800`, byte
  count, and FNV-1a checksum must all agree; any change rejects the Soul Room
  route and cannot select a generated fallback. The payload remains opaque:
  its dungeon/object/tile/bitmap/palette grammar and a positive level
  transition still need original execution evidence.

- 🔧 2026-07-15 Track 02 level/object boundary: the authenticated original
  evidence is a game-owned post-`$3800` consumer that reads a separately
  hash-bound level/object record and proves its grammar.
  boundary: level envelope `[0x114,0x480)` and the remaining opaque bytes
  remains blocked. This proves media identity and record coordinates only;

  - Update 2026-07-20: the chain now generalizes the loader's per-byte
    consume/dispatch loop on original media, and evidence of where the
    loop terminates or dispatches into a record consumer.
    provenance only. Remaining: an authentic capture of the repeated

- 🔧 2026-07-11 Theron paired-CUE real-media follow-up: the hash scanner now
  accepts a CUE only when its one readable Track 01 AUDIO plus Track 02
  MODE1/2352 declaration canonically resolves to the independently
  hash-verified Track 02 payload. M12 passes that original CUE path to the
  launch profile, while an absent, malformed, renamed, or mismatched pair
  stays Track-02-only. No media is copied or synthesized. The bounded Track 01
  consumer now accepts only the CUE-declared WAV stem's local OGG counterpart
  and decodes it through optional Vorbis support; platforms without that
  decoder remain silent. Remaining work is user-staged JP/US title
  playback/capture evidence, not broader filename pairing or invented audio.

- 2026-07-27 Theron raw-CUE runtime launch regression: the current M11 path
  reaches the real startup route from the authentic USA MODE1/2352 CUE/BIN
  set (`f23601102138f87c33025877767ebf76`) and no longer relies on a direct
  Track-02-only probe. The focused runtime CTest advances title, stage, and
  Soul Room inputs under the dummy SDL driver, then requires
  `phase=theron-startup-2` and the original US asset identity. This proves
  startup admission and flow only; it does not promote unbound Track 02
  graphics, later dungeon records, or save semantics.

- 🔧 Track 02 graphics-format follow-up: the real hash-verified JP/US raw-BIN
  catalog found 1,522 strict HuC6260-shaped windows and 78 strict LE16
  stride-shaped windows across 2,022 exact matching nonzero MODE1 sectors.
  Its bounded detail list retained 64 records and overflowed 1,536; these are
  overlapping syntax matches, not independently proven palettes/tables. The
  catalog authorizes no decoder or runtime route. Exact media receipt:
  `docs/source-lock/tqr_v1_track02_graphics_format_real_media_2026-07-11.md`.
  Next evidence must trace one catalogued user-data offset through HuC6280 CD
  loader code to a VCE palette write or VDC VRAM destination, including the
  loaded byte count; only that can bind a candidate to graphics, a palette,
  or a compression routine.

- 🔧 2026-08-06 JP Stage-2 disassembly follow-up: the authentic JP Track 02
  BIN is now materialised as `~/.firestaff/data/theron/TQJP02.bin` and its
  IPL loader plus dynamic `$3800` payload receipt pass against record `0x4df`.
  The later static Stage-2 byte windows remain US-only because the JP image
  has region-specific bytes; do not widen those verifier gates until a JP
  disassembly identifies equivalent instruction/data spans and their callers.

- 🔧 2026-07-11 IPL-loader provenance update: original CUE sheets prove Track
  01 is CD-DA narration, while Track 02 is the MODE1 code track. The
  hash-gated JP/US Track 02 IPL information block at logical sector 1 selects
  record `0x0003a3`, load/entry `$4000`, and a 3-sector JP or 4-sector US
  executable. Both actual executables contain `JSR $e009` (System Card
  `CD_READ`) at CPU `$40cd`; the immediately verified setup selects local RAM
  `$3000` (`DH=$01`), not VRAM (`DH=$fe/$ff`). This is the first genuine
  loader/media linkage, but it does not bind the selected record, count,
  decompressor, palette, or graphics candidate. The next admissible step is
  bounded dataflow from this loader's record table through one complete read
  setup to a verified VDC/VCE destination; generated rendering remains
  fail-closed meanwhile.

  - Update 2026-07-21: L424B's callees and the $45A6 TII gap stream
    far-call targets, and L383E in the dynamic payload are future
    windows; the post-$3800 consumer chain remains capture-blocked.
    streams. Remaining: JP verification awaits staged JP media; the

- 🔧 2026-07-13 dynamic Track 02 RAM receipt: the instrumented original
  Mednafen route now requires a 32-byte FNV-1a receipt from System Card
  destination `$3800` immediately after the authenticated dynamic `CD_READ`
  returns. This proves record-to-RAM transfer but does not identify a Track 02
  source byte, decompressor, palette, VCE word, VDC transfer, level, or object
  family. Next evidence must tie that exact destination span to a hash-verified
  source sector and follow its bytes through one original VCE/VDC operation.

- 2026-07-16 update: the Track02 loader-intake chain now has a
  post-predecode-to-dungeon-level gate that preserves object/dungeon
  read-window topology only when it can also consume the source-locked initial
  level handoff for the same JP/US Track02 media. Missing raw media produces
  an explicit no-fallback blocker, and the positive branch remains conditional
  on `FIRESTAFF_THERON_TRACK02_RAW`. Remaining work is still real original
  loader/CD-read evidence that assigns a verified object-table or
  dungeon-record grammar before runtime/render admission.

- 2026-07-16 update: a grammar-admission barrier now consumes that
  dungeon-level topology receipt and preserves the original CD-read record,
  byte-window, hash, and topology evidence while explicitly requiring a future
  original object-table/dungeon-record grammar witness. It admits no grammar,
  decoder, runtime, rendering, fallback visual, or synthetic byte path.
  Remaining work is a real HuC6280/System Card trace that follows one of these
  exact windows into the original object or dungeon parser.

- 2026-07-16 update: the grammar boundary now also binds back to the
  read-table/layout-binding receipt, so a positive real-media path must
  preserve the exact original CD-read records, MODE1 user-data offsets,
  destinations, byte windows, copied-byte hashes, and topology hash before it
  can reach the grammar-witness-required blocker. Remaining work is still the
  original parser trace itself; this gate deliberately admits no object-table
  fields, dungeon-record grammar, runtime handoff, rendering, fallback visuals,
  or synthetic bytes.

- 2026-07-16 update: a parser-witness gate now admits object-table and
  dungeon-record grammar provenance only when supplied original trace facts
  prove that the original loader/parser consumed those exact preserved
  CD-read windows. Even that positive receipt keeps object fields, dungeon
  record fields, decoder semantics, runtime handoff, rendering, fallback
  visuals, and synthetic bytes blocked. Remaining work is to source such
  witness facts from a real HuC6280/System Card trace instead of a caller
  supplied receipt.

- 🔧 Phase 5 - Mechanics parity hardening: 50-assertion mechanics probe covers movement, click routes, doors, pits, teleporters, altar, combat, drops, and sounds. **2026-07-23 update (Lane E, cycle 11):** new `firestaff_theron_v1_mechanics_playability_probe` loads the authentic JP/US Track 02 Hall-of-Records level-0 grid and verifies movement, turning, wall blocking, and floor movement on the real 32×27 loader-accepted grid (36/36 PASS on staged TQUS02.bin + TQJP02.bin). **2026-08-06 update:** the real-data thing-data regression now discovers the supplied standard `~/.firestaff/data/theron/TQUS02.bin` path (or `FIRESTAFF_THERON_TRACK02_RAW`) before the legacy fixture path and verifies all seven dungeon object/text regions: AKUTUBA 228 ground refs/1021 items, DRATOR 249/969, FORMICIA 224/871, SARMON 226/1132, SHADODAN 264/980, THIEVES 255/988, DEMON 190/881. The loader also rejects non-sector-aligned raw input. Remaining work is broader real-asset gameplay traces for doors, pits, teleporters, altar, combat, drops, and sounds once those object semantics are source-locked.

- 2026-08-06 update: the real-data map, ground-reference and door/teleporter regressions now discover `FIRESTAFF_THERON_TRACK02_RAW` or standard `~/.firestaff/data/theron/TQUS02.bin` before the legacy fixture path. Against the supplied US BIN they verify all seven map groups, 4, 8, 5, 6, 3, 4 and 4 maps respectively; all seven ground-reference chains; and all seven door/teleporter tables. JP-specific map offsets remain a separate source-format gap and are not inferred from the US table.

- 2026-08-06 update: Track 02 raw-media intake now parses `FILE`, `TRACK`, and
  related CUE directives case-insensitively, matching the CUE format instead of
  depending on one editor's capitalization. A real-data regression builds a
  temporary CUE around the supplied `TQUS02.bin`, verifies the US pregap/index
  at raw sector 225, the authenticated BIN MD5, and trace preparation. The
  remaining intake gap is broader real CUE/BIN/ISO corpus coverage, not a
  generated fixture.

- 🔧 2026-08-06 Theron drop-placeholder removal: the old category-to-item
  resolver accepted synthetic item IDs and a host seed, then presented a
  guessed weapon, armour, consumable, scroll, or key as a real drop. The
  category table remains a verified item-name/category receipt, but no drop
  can be admitted until the original T900 consumer and selection record are
  decoded from Track 02. `theron_v1_drop_loot()` already fails closed at that
  boundary; the obsolete resolver and its positive fixture assertions are
  removed. Next evidence is a real T900 drop record plus its consumer.

- 🔧 2026-08-05 Theron production combat boundary: removed the inferred
  `theron_v1_compat.c` implementation from the `firestaff_theron` library.
  Production now uses the existing fail-closed adapter, so creature speed,
  AI, attack/defense formulas, spell combat, drops and sound IDs cannot be
  published from guessed records. Compatibility mechanics remain explicit in
  fixture/probe targets. The next replacement is still the authenticated
  Track 02 T500/T600/T900 consumer, not a new host-side table.

- 🔧 2026-08-05 Theron static consumer receipt: the authenticated US Track 19
  image now has a byte/MD5-locked regression for bank `$1f` `$243e–$24c3`.
  It proves the existing HuC6280 bitstream/register-map fragment against the
  real `TQUS19.iso` and explicitly records that the `$2600` consumer is absent
  from static ROM. The next step remains a real post-CD RAM capture with PC
  and source-LBA provenance; no RAM bytes or level/object semantics are
  inferred from this receipt.

- 🔧 Startup presentation hardening: stage/Soul Room render rows, enriched startup layout labels, and Track 02 descriptor-role receipt summaries are now test-visible; remaining work is real Track 02 startup art/audio decoding and pixel evidence instead of fallback text presentation.

  - 2026-07-08 update: Theron boot now owns the runtime dungeon/UI/V2-HUD/present render frame facade. M11 no longer calls `theron_vp_render_dungeon`, `theron_vp_render_ui`, V2 HUD render, or `theron_vp_present` directly in the Track 02 runtime path.

  - 2026-07-08 update: Theron boot now owns runtime ownership release for profile/world/viewport/assets, and M11 shutdown no longer frees those Track 02 objects directly.

- 🔧 Phase 7 - Save/import compatibility: round-trip, header-rejection, world-serialize-purchase-state, shop price-table regressions, and data-free cross-slot export/import are green. Remaining work is a real Track 02 save artifact import/export pass when such a save is available.

### Theron V2.0 / V2.1 / V2.2

- 🔧 Phase 2 - Enhanced asset pipeline: presentation-mode selection API + filter config + V2.1 EPX upscaler pipeline are wired (`theron_v2_texture_upscale_pc34.c` provides `theron_v2_epx_upscale` indexed→RGBA via PCE palette). The Theron V2.2 manifest parser remains available for fixture inspection, but production now requires `source_provenance="authenticated_track02"`; the existing procedural/gpt-image-2 pack is explicitly rejected as real data. Remaining: obtain source-owned Track 02 bitmap/material records and bind them before enabling V2.2 art.

- 🔧 **2026-06-27 Theron V2 Phase 3 initial seed landed (presentation-only, data-free):** `theron_v2_hud_overlay_pc34.c/.h` is the Theron-specific sibling of `csb_v2_hud_overlay_pc34.c` + `dm2_v2_hud_overlay.c`. New CTest `theron_v2_phase3_hud_overlay_probe` (40/40 PASS, labels `tier2;theron;v2;phase3;hud;presentation-only`) covers the phase-gate + presentation-mode selector contract (V1_FAITHFUL → no HUD overlay, V20_FILTERED / V21_UPSCALED / V22_MODERN → HUD active), all 6 setters (direction, quest items, dungeon progress 1/7, relic counter 0/7, spell-rune ready indicator, 4-champion bars), render into a 256×224 indexed framebuffer, V1 chrome preservation when V2 inactive, source evidence citations (THQUEST.ASM T520/T560/T600/T700/T800/T900 + HuC6260/HuC6270 + dmweb Theron 7 dungeons + 7 relic goals + sibling csb/dm2 modules), and null safety. Companion smoke test `theron_v2_hud_overlay_pc34` (58/58 PASS, CTest `theron_v2_hud_overlay_pc34`) covers init/reset, hit-flash decay, low-HP pulse trigger, top-bar / stats-bar / action-strip visibility toggles, and per-region pixel-write assertions (compass / quest / dungeon / relic / champion bars / action strip all paint when active, and `visible=0` or `opacity=0` writes zero pixels). HUD surface: top-bar (compass + quest items + dungeon progress 1/7 + relic counter 0/7 + spell-rune ready indicator), bottom-panel (4 champion mini-bars HP/Stamina/Mana with Theron-as-leader at slot 0), and bottom action strip (ATK/CST/USE/DRP/MOV with active underline and hit-flash). Theron-specific surfaces (PC Engine 256×224 indexed fb, HuC6260 VDC layout, 7 dungeons + 7 relic goals, rune magic ready indicator) are mirrored from `dm2_v2_hud_overlay.c` + `csb_v2_hud_overlay_pc34.c`. **2026-06-27 Phase 3 placeholder-vs-real asset gate landed:** `theron_v2_hud_widget_assets_pc34.c/.h` is the Theron-specific sibling of `dm2_v2_hud_widget_assets` (the original Phase 3 gate pattern). New CTest `theron_v2_hud_widget_assets_pc34` (105/105 PASS) and headless probe `firestaff_theron_v2_hud_widget_assets_probe` (65/65 PASS, labels `tier2;theron;v2;phase3;hud;widget-assets;presentation-only`) cover `NOT_PROBED`/`NO_MANIFEST`/`PLACEHOLDER`/`PARTIAL`/`COMPLETE` gates with the NO_MANIFEST-by-default baseline matching the current runtime. Slot table (7 slots, stable order, ordinals = indices): 5 Phase 3 primary (`compass_rose`, `quest_items`, `dungeon_progress`, `relic_counter`, `rune_indicator`, category `hud_widgets`) + 2 chrome supporting (`champion_bars`, `action_strip`, category `hud_chrome`). Manifest schema `{ id, generator, source_file, width, height }` aligned with sibling `theron_v22_modern_assets_pc34` and `dm2_v2_hud_widget_assets` shapes; manifest path `~/.firestaff/assets/theron/hud/hud_widget_manifest.json`. Companion source-lock doc `docs/source-lock/theron_v2_phase3_hud_widget_assets_H2340.md` documents the slot table, schema, gate state machine, M12/Phase 7 integration points, and honest boundary. Source-locked against THQUEST.ASM T520/T560/T600/T700/T800/T900, HuC6260/HuC6270, ReDMCSB PANEL.C F0354 + DUNGEON.C F0260, dmweb Theron overview, `docs/source-lock/tqr_v1_phase2_data_formats_H2339.md`, sibling `dm2_v2_hud_widget_assets.h`. **2026-06-28 runtime handoff landed:** M11 now calls `theron_v2_hud_render()` in the live Theron Track 02 render path after `theron_vp_render_ui()` and before `theron_vp_present()`, gated by non-V1 presentation mode. **2026-06-29 overlay seed gate landed:** `theron_v2_hud_seed_from_v1_world()` now owns the V1-world snapshot mapping and returns explicit `V1_SKIPPED` / `V2_READY` states; `firestaff_theron_v2_overlay_seed_gate_probe` covers V1 hidden/no-paint behavior, V2 field mapping, byte-identical synthetic V1 world state before/after seeding and rendering, deterministic framebuffer output, gate-name stability, and NULL safety. **Remaining Phase 3 work:** (a) finish PBR top-bar / bottom-panel / action-strip bitmap assets under `~/.firestaff/assets/theron/hud/hud_widgets/` and `~/.firestaff/assets/theron/hud/hud_chrome/`, (b) author an example `~/.firestaff/assets/theron/hud/hud_widget_manifest.json` with `generator ≠ "placeholder"` so the gate can promote to `PARTIAL`/`COMPLETE`, and (c) real-art visual verification + per-region pixel gates against real Track 02 captures.

- ❌ Phase 4 - Enhanced lighting/effects.

- ❌ Phase 5 - Smooth movement and viewport interpolation.

- 🔧 Phase 6 - Touch/controller ergonomics: **2026-06-29 initial Theron-specific input seed landed (presentation-only, data-free):** `theron_v2_touch_controller_affordance.c/.h` maps Theron V2 touch swipes, edge-strafe, D-pad, left-stick, and right-stick affordances onto the shared DM1-family C001-C006 command ids while rejecting every affordance when V2 presentation is off; `theron_v2_touch_runtime.c/.h` translates accepted affordances into `Dm1V1QueuedCommandPc34Compat` entries and adds a Theron 256x224 HUD-chrome exclusion gate for touch starts on the V2 top bar, champion mini-bars, and action strip while controller inputs bypass the framebuffer coordinate gate. New CTest `theron_v2_touch_controller_affordance` (267/267 PASS) and probe `theron_v2_touch_runtime_probe` (138/138 PASS) are data-free and source-locked against THQUEST.ASM T520/T560/T600 plus ReDMCSB DEFS.H:238-243, COMMAND.C:2045-2155, CLIKMENU.C:142/180, and GAMELOOP.C:164-219. **2026-08-09:** live M11 Theron binds W/S/A/D to the four-way PCE pad, mouse 1/2 to Button I/II, and short/long touch to Button I/II; held motion is gated to the loaded dungeon phase. Shared M11 SDL gamepad routing now exists; remaining Theron-specific work is a real touch-layout target-size audit across launcher/game views and real Track 02 runtime proof.

- ❌ Phase 7 - V2 verification suite.

## Theron Track 02 remaining evidence

- [ ] THERON-V1-TRACK02-LIVE-LOADER-CONSUMER: the latest replay against the
  authenticated US Track 02 ISO now gives a real HuC6280 loader witness
  (`$2286` `TIA` followed by 13 block transfers, 24 RTS and 24 post-RTS rows)
  plus 4,096 static-bank consumer reads and an executed `$2c54–$2c69`
  code-window receipt. The parser now accepts this richer real trace. It still
  has no `$2600` dynamic consumer bytes, no VDC VRAM/VCE snapshot, and no
  source-owned level/object field decisions, so visual runtime drawing and
  source-consumer correlation remain blocked. The interactive forcefield route
  now admits a source-only map/thing handoff from authenticated raw BIN data;
  it does not promote VDC/VCE pixels, host item semantics, or guessed field
  meanings. Next evidence is a capture that reaches the game-owned post-CD
  consumer and closes the VDC snapshot on clean exit.

- 2026-08-06 update: the Track 02 thing-category enum is now source-bound to
  the retail order used by DMBUILDER6 (`4=monster`, `5=weapon`, `6=clothing`,
  `7=scroll`, `8=potion`, `9=chest`, `10=misc`, `14=missile`, `15=cloud`).
  A real US Track 02 regression now checks all seven dungeon object-count
  tables and requires nonzero copied payload for every populated category.
  This is raw record provenance only; runtime item/monster publication and
  combat/render semantics remain closed until their consumers are bound.

- 2026-08-06 update: categories 4–10 now have a portable little-endian raw
  record decoder. It binds the two-byte next-reference prefix and the
  DMBUILDER field layouts for monsters, weapons, clothing, scrolls, potions,
  chests, and misc across every populated record in the real US corpus.
  Categories 14/15 now use the same source decoder for their six-/two-byte
  payloads; no item is published into the runtime object model yet.

- 2026-08-06 update: the full Track 02 dungeon loader now consumes those
  source-bound records and follows their authentic next-reference chains on
  all seven US dungeons. It reports decoded/unbound records separately and
  leaves `Theron_V1_Object` untouched for categories whose host owner is not
  proven. The remaining handoff is the original object-kind/item-index
  consumer, not raw media intake or chain traversal.

- 2026-08-06 update: each real Track 02 map header now survives the world
  handoff as an exact verified receipt (`x/y` offsets, opaque bytes, XP and
  door bytes, map id and creature count). These fields remain semantic
  read-only evidence; seed, spawn direction and object-kind publication stay
  closed pending the original consumers.

- 2026-08-06 update: the same world handoff now retains each real map's
  `creature_gfx_bank` and cumulative column thing-count from the Track 02 map
  directory. They remain raw level-record evidence; no creature graphics or
  object semantics are inferred from either field.

- 2026-08-06 update: every real category 4–10, 14 and 15 occurrence now carries
  both its exact raw bytes and the decoded source record (including missile and
  cloud payload fields) through the full-dungeon handoff. Host object-kind,
  inventory and projectile/cloud ownership remain deliberately unbound; no
  synthetic object is created.

- 2026-08-06 update: the real-data thing-record regression now covers both
  authenticated `TQUS02.bin` and `TQJP02.bin`. All seven Japanese dungeon
  blocks use their source-bound map/item offsets, retain 871–1 132 records per
  dungeon and decode every populated category without publishing a host
  object. Japanese text remains at zero until its codon consumer is proven;
  no translated or synthetic text is inserted.

## Theron Track 19 remaining evidence

- 2026-08-06 update: the authenticated 32x27 Track 19 startup-level record now
  survives the file-inventory handoff with its six raw header words, payload
  size/nonzero count and payload FNV-1a. This remains a source receipt only;
  tile, object and later-level semantics still require the original consumer.

- 2026-08-06 update: the real US and JP Track 19 startup envelope now has a
  bounded structural reader: big-endian 32×27 dimensions, six retained raw
  header words, and an 864-byte borrowed payload span are checked against the
  authenticated envelope hash. The payload remains opaque; tile/object
  ownership and later-level consumer semantics still require disassembly.

- 2026-07-15: Runtime level-bank selection now retains the authenticated
  startup bitmap's Track 02 MD5 and raw/user-data sector envelope. Remaining:
  obtain original loader/CD-read evidence that binds a post-startup bitmap or
  object-table record to a concrete runtime consumer. Do not infer palette,
  layout, object fields, or draw behavior from the retained startup envelope.

  - Update 2026-07-20: the chain now generalizes the loader's per-byte
    loop on original media, and evidence of where the loop terminates or
    dispatches into a record consumer.
    Remaining: an authentic capture of the repeated consume/dispatch

  - Update: the render-asset admission receipt can now feed a dungeon-facing
    real-data handoff receipt only when the same admitted US raw Track 02
    session carries matching route hashes, payload/envelope/consumer checksums,
    decoded level/object-table/bitmap/palette hashes, source-byte binding,
    object-table layout proof, and bitmap/palette decode proof. The handoff
    explicitly keeps dungeon drawing and fallback visuals closed and rejects
    synthetic dungeon state, synthetic object layout, synthetic bitmap/palette
    decode, hash drift, and fallback observation. Remaining: the positive
    original capture/decoder producer that supplies these real proofs from
    Track 02 without sidecar or generated visual data.

  - Update: the multilevel Track 02 runtime path can now retain a same-capture
    bitmap/palette source-window receipt after a real level transition. The
    receipt binds the selected record, source/target levels, palette raw and
    MODE1 user-data offsets, palette payload/decode checksums, bitmap atlas
    route facts, and a combined source hash while explicitly requiring
    palette decode, bitmap decode, pixel output, M11 render admission, dungeon
    draw, and fallback visuals to remain closed. Remaining: acquire a positive
    original loader/decoder trace that proves palette words and bitmap pixels
    before connecting this source receipt to render-asset or M11 admission.

  - Update: a positive decode-vector receipt now consumes that source-window
    receipt plus the real US Track 02 bytes and verifies the HuC6260 palette
    words, the indexed bitmap atlas, route/tile/nonzero-pixel counts, first
    pixel row hash, and source checksum agreement. This proves a real
    palette/indexed-pixel vector on the multilevel route, but it deliberately
    still blocks M11 runtime consumption, M11 rendering, dungeon draw, and
    fallback visuals. Remaining: capture the original nonstartup dungeon
    graphics consumer that binds these decoded vectors, or another real
    Track 02 bitmap/palette window, to the active dungeon level before any
    render-asset admission or host-surface upload.

  - Update: the positive decode vector can now feed a production M11
    Soul Room runtime-consumption receipt. The receipt selects Track 02 level
    0 through the live `Theron_RuntimeLevelMedia` Soul Room surface, verifies
    exact indexed-atlas route checksum/nonzero pixels/offsets against the
    decode vector, verifies 1:1 host placement and clipping, and permits M11
    host presentation only for that source-owned Soul Room surface. Generic
    dungeon draw, fallback visuals, scale changes, checksum drift, later-level
    graphics, and non-Soul Room routes remain blocked. Remaining: prove the
    original nonstartup dungeon graphics consumer and per-level render layout
    before promoting broader dungeon rendering or host uploads.

- Nexus Saturn memory-card intake remains opaque: the verified boundary accepts
  only an authenticated, hash-bound 8 KiB image with 16 x 512-byte blocks on
  an active title/champion route. Remaining work is an original-card corpus
  and capture proving the proprietary header, slot layout, checksums, and any
  state semantics; FNXS/native-save fallback remains prohibited.

- Nexus Mednafen capture remains operator-only: a dry-run manifest now binds
  VDP1 word layout, decoder, palette, pixel, or render admission from the
  current files.
  exact byte count, FNV, and SHA-256. Both byte streams remain uninterpreted;
  therefore remain capture-required: do not infer a source-to-command parser,

  - 2026-07-17 M11 presentation audit: the full-output admission is still an
    opaque evidence receipt. It authenticates one complete output byte range,
    its SHA-256/FNV, and later VDP1 command order, but deliberately publishes
    no indexed-pixel declaration, width, height, stride, CLUT/palette span,
    BGR/RGB ordering, transparency rule, or host placement. Both
    `graphics_permitted` and `decoder_promoted` remain zero. Do not connect
    this output to M11's indexed/palette surface, reuse WARNING.BIN's PP
    contract, or synthesize a title/menu image. A future original trace must
    attest all of those output-format facts before a byte-exact M11 consumer
    can be added.

- Nexus Structure1F multi-level capture remains no-draw: LEV00--LEV15 now
  original Saturn trace observations; mesh/face geometry and all
  pixel/palette semantics stay uninterpreted.
  remain missing.
  Remaining work is direct,

- The direct SLEV/SAL/MAP/SDDRVS discovery route now has the materialized
  English retail auxiliary corpus with positive hash/identity and bounded
  parser receipts. Retail-positive script/audio trace evidence, dispatch,
  decoding, and playback remain blocked. The direct SDDRVS dungeon
  admission also revalidates its direct file at consumption, but it still
  awaits authentic package/level/trace evidence before any script claim. The
  matching direct SAL/SLEV/MAP dungeon route now has the same identity-only
  rehash-on-consume guard; it does not establish a codec, event meaning,
  playback, or script semantics. The verified SAL `dsp01.EX` container
  preamble and bounded opaque payload interval are now retained only as
  provenance; descriptor/sample grammar and codec evidence remain open.
  Direct SNDLEV MAP provenance now also retains only its 24-byte header,
  bounded 8-byte rows, and terminator. M11 can bind one rehashed row to the
  active level/package/card/epoch, but selector/event semantics, codec proof,
  and playback remain unproven and blocked.

- Nexus SLEV task-body capture remains no-dispatch: every SLEV00--15 target
  requires matching admitted header/literal, raw-trace, and source-order
  receipts plus opaque external opcode and callback-owner labels. Remaining
  work is reviewed original-Saturn task-body grammar and callback ABI proof;
  no task opcode executes and no fallback script is admitted.
  The selected target can now enter M11 startup only through the matching
  direct SLEV/SAL/card/package/epoch receipt and exact SLEV FNV. That is an
  opaque source-order/trace admission only; authentic retail task-body and
  callback evidence is still required before any dispatch claim.

- Nexus SNDLEV/SAL capture planning remains playback-blocked: each unique
  audio, play sound, or draw. A real retail `NXSLSC01` capture and original
  command/driver semantics remain required.
  The payload remains opaque and non-retained; a real command grammar and

- Nexus PRS3 original-execution intake remains evidence-only: one independently
  authenticated V10 export must bind one MENU.BPK stream's complete SH-2 input
  reads, output fingerprint/range, and later VDP1 source command. Remaining
  work is reviewed opcode, pixel, and palette semantics; no decoder or graphics
  route is admitted.

  - 2026-07-22 capture-admission update: the final byte-admission stage now
    rehashes the supplied full MENU.BPK and DM.BIN bytes, derives the exact
    bounded MENU.BPK stream by the V10 offset/length, and requires FNV-1a plus
    SHA-256 agreement for those three source lanes before it accepts opaque
    output and VDP1 capture bytes. It also repeats the trace's strict final
    output-write -> VDP1-command ordering. This is not a PRS3 decoder, VDP1
    command parser, palette interpretation, pixel path, or draw permission.
    The remaining blocker is still an independently authenticated retail
    Mednafen/Saturn V10 export and its four real byte artifacts.

- Nexus PRS3 multi-capture review remains non-promoting: representative,
  independently authenticated MENU.BPK modes must agree on opaque bit-order
  and termination observations before a decoder candidate may be reviewed.
  Decoder, palette/pixel meaning, rendering, and fallback visuals remain off.

- Nexus Structure3 face/texturing capture remains capture-only: DGN face and
  Structure1F/2 provenance must agree with opaque material candidates and VDP1
  evidence. Pixel and mesh semantics remain unproven and no draw route opens.

- Nexus multi-level DGN capture remains opaque: LEV00--15 needs matched
  Structure1F, Structure2 placement, Structure3 face targets and ordered
  command/frame receipts. No decoder, mesh inference, or rendering is admitted.

- Nexus active dungeon route may report only capture-ready coverage when its
  loaded DGN identity matches the full multi-level adjudication receipt. Level,
  package, PRS3 trace FNV, or trace-size drift clears it. Decoder,
  mesh/texturing, and rendering remain unavailable.

- Nexus multi-level capture jobs remain operator-only planning data. A future
  Mednafen invocation must independently re-hash every staged retail asset and
  preserve the emitted job order; this planner never launches, captures, or
  interprets a trace.

- Nexus campaign asset intake is read-only and hash-first for explicitly staged
  loose files, ZIP members, and ISO/BIN/CUE members. Virtual container entries
  are never extracted or copied; unsupported containers remain blocked.

- Nexus Saturn memory-card startup intake remains opaque: authenticated 8 KiB
  card identity and selected route epoch may gate champion startup only. Save
  layout, FNXS fallback, and native-save semantics remain blocked.

- Nexus M12 card-startup selection consumes only exact opaque card/epoch
  readiness; native FNXS resume remains a separate route.

- Nexus Saturn-card discovery currently admits only one direct 8 KiB file;
  virtual ZIP/ISO/BIN/CUE identities are diagnostic-only and contents stay
  opaque; container launch remains blocked.

- Nexus champion startup accepts only an atomically bound direct card, package
  identity and current M11 route epoch; when the M11 PRS3 presentation receipt
  is present, it must share that exact package and epoch. Card bytes remain
  opaque and PRS3 remains no-draw.

- Nexus Structure1F records now retain parser-observed raw spans only; face,
  mesh, palette and texture semantics remain unproven and no-draw.

- Nexus Structure2 descriptor spans are source provenance only; codec, pixel
  and palette meaning remain blocked pending original evidence.

- Nexus Structure3 face spans are raw package provenance only; PRS3, palette,
  pixel and texture semantics remain blocked. The direct-source admission now
  also retains one hash-bound 40-byte entry header, its raw tag/count fields,
  and the three count-bounded 12-byte intervals only when the already admitted
  Structure3 target and ordinary source file still agree. This is framing, not
  a geometry, normal, material, texture, transform, or draw claim. The local
  retail LEV corpus is still absent, so positive corpus confirmation remains
  pending.

- Nexus Structure3 image/palette references are bounded source intervals only;
  codec and decoded surface admission remain blocked.

- Nexus MENU.BPK startup provenance now binds a selected PRS3 entry's bounded
  payload offset/length/FNV and header facts through an epoch- and
  package-bound M11 no-draw host receipt. Any engine-owned verified row may be
  selected, but its recognized mode byte, bounded opaque compressed body,
  declared output size, and body FNV must exactly match; unknown modes and
  declaration/span/FNV drift reject, including across launcher/card epoch
  transitions. PRS3 pixels, opcode grammar, and decoder promotion remain
  unavailable pending independent original-Saturn codec evidence.

- The legacy `nexus_v1_bpk_surface_class` synthetic fixture still asserts a
  synthetic PRS3 literal decoder and decoded material import. Its stored
  payload receipt now keeps the fallback-provenance bit closed, but it is
  incompatible with the current retail fail-closed PRS3 route and is not
  evidence for a Saturn codec; replace it with authenticated capture-backed
  expectations before treating it as a promotion test.

- 2026-07-17 DM1 original-save C-event package completed: F0435 now retains
  C2 `ActionIndex` and `PoisonEventCount`; F0802/F0796 preserve their bounded
  PC34 bytes. C25 and C29 exports require authenticated F0435 provenance,
  while C3/C4 snapshot drift, malformed poison width, synthetic C25/C29, and
  invalid source squares reject. The targeted original-save handoff suite is
  green; remaining work is external original-save corpus evidence.

- 2026-07-17 DM1 C2 PARTY_INFO follow-up completed: source byte 86
  `Event71Count_Invisibility` now materializes into both M10 invisibility
  owners and F0802 writes it back only as a bounded PC34 byte. The focused
  C71 path and full original-save handoff suite are green.

- 2026-07-17 DM2 DB14: the normal `QUERY_PICST_IT` `0x40`/neutral-mode branch
  now copies only authenticated native-size indexed IMG3 pixels under matching
  RAW4 clip and palette receipts. Flip, crop, nonzero offset, scaling, and
  every other blitmode remain fail-closed. Remaining: source-proven non-normal
  transform branches and live frame ordering.

- 2026-07-17 DM2 HUD SUMMARY_IMAGE: `c_gui_draw.cpp:926-942` now has a
  no-draw M11 receipt for exact `(1,vb_144,field)` HUD commands. It requires
  the source plan's decoded GDAT pixels, local palette, and RAW4 destination
  identity; tuple mismatch, absent palette, and stale destination reject.
  Remaining: source-proven HUD transform admission before any new draw path.

- 2026-07-17 DM2 HUD PICST transform: the exact `c_gui_draw.cpp:926-942`
  branch admits only source values `0..0x28`, retaining X scale `0x1f` for
  `0..0x0f` or `0x2f` otherwise and Y scale `0x35`. Out-of-range values,
  missing SUMMARY_IMAGE material, or stale destination reject; it remains
  source-gated for draw only where the resolved destination is the complete,
  exact scaled rect. Partial/unknown `QUERY_BLIT_RECT` clipping, flips, and
  every other HUD transform remain no-draw.

- 2026-07-17 DM2 pit viewport admission: `c_gui_vp.cpp:234-292`
  `DM2_DRAW_PIT_TILE` now has a bounded source receipt for cells 1..15. It
  binds `table1d6c70/90/a0/b0` selection, the live cell's `+8` state word,
  `DRAW_DUNGEON_GRAPHIC` light parameter, exact `(GRAPHICSSET,field)`
  SUMMARY_IMAGE, GFX256 raw material, decoded U4 bytes, and local palette.
  It remains `no_draw`: cell 0's `SET_GRAPHICS_FLIP_FROM_POSITION` and the
  selected `QUERY_BLIT_RECT` placement/clip chain are not yet proven.

  - 2026-07-17 composition update: cells 1..15 now bind their accepted
    SUMMARY_IMAGE/GFX256 material identity into the current DM2 viewport
    composition session/data epoch and parent ordering receipt. The receipt
    explicitly records that PIT_TILE's own draw slot is unresolved, so it
    cannot consume pixels. The sole remaining promotion precondition is the
    source's per-cell `QUERY_BLIT_RECT` destination/clip transaction.

  - 2026-07-17 RAW4 placement update: `table1d6c70[cell]` now binds through
    `DRAW_DUNGEON_GRAPHIC`/`QUERY_PICST_IT` to the exact
    INTERFACE_GENERAL/0/RAW4 root row, with destination, full material extent
    and table/row hashes retained in the PIT composition receipt. Chained
    rectangles, crop and clip grammar remain rejected. It stays no-draw until
    a PIT-owned ordered composition slot and authenticated buffer handoff are
    proven together.

  - 2026-07-17 buffer/slot update: PIT_TILE now retains its own authenticated
    decoded U4 buffer handoff and binds it to the generic DM2 viewport
    before/after surface snapshot and composition identity. Pointer, extent,
    stride, palette, material and surface-generation drift reject with no
    write. The slot deliberately remains no-draw: source proof is still
    missing for PIT_TILE's normal-branch `DRAW_PICST` row ordering.

  - 2026-07-17 normal-row update: cell 1's `blitmode=0` branch is now bound
    to `DRAW_PICST`'s top-to-bottom/left-to-right U4 row order and exact RAW4
    placement identity. It remains no-draw because `DRAW_DUNGEON_GRAPHIC`
    applies `DM2_query_B073` before that row loop; PIT still lacks its own
    authenticated transformed palette transaction.

  - 2026-07-17 B073 update: cell 1 now binds `DM2_query_B073`'s RAW7
    count/left/right/lookup palette program to its material, RAW4 placement
    and normal-row receipt. RAW7, placement or palette drift rejects. The
    transformed palette remains no-draw until alpha ownership and the final
    ordered handoff consumer are jointly admitted.

  - 2026-07-17 cell-1 consume update: only cell 1's normal (`blitmode=0`)
    path now consumes the authenticated U4 handoff through B073's transformed
    palette and low-nibble alpha into the current ordered owner surface. All
    other PIT cells, mirrors, crops and chained clips remain fail-closed.

  - 2026-07-17 cell-3 consume update: cell 3's independent normal
    (`blitmode=0`) route now admits only its exact GRAPHICSSET field `0x6e`,
    RAW4 rect `0x35b`, B073 transaction and ordered U4 handoff. Cell 2 and all
    other mirrored or unproven normal forms remain fail-closed.

  - 2026-07-17 cell-4 consume update: cell 4's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x6f`, RAW4 rect
    `0x35a`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Cell 2 and every other mirrored or unproven normal form remain
    fail-closed.

  - 2026-07-17 cell-6 consume update: cell 6's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x71`, RAW4 rect
    `0x358`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-7 consume update: cell 7's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x72`, RAW4 rect
    `0x357`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-11 consume update: cell 11's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x76`, RAW4 rect
    `0x355`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-12 consume update: cell 12's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x77`, RAW4 rect
    `0x354`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-14 consume update: cell 14's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x79`, RAW4 rect
    `0x352`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-2 HFLIP consume update: cell 2 admits only GRAPHICSSET
    field `0x6c`, RAW4 rect `0x35f`, B073/RAW7, and its own source-locked
    reverse-X U4 row walk. Crop, chained clips, vertical flip and all other
    mirror cells remain fail-closed.

  - 2026-07-17 cell-5 HFLIP consume update: cell 5 admits only GRAPHICSSET
    field `0x6f`, RAW4 rect `0x35c`, B073/RAW7 and its own source-locked
    reverse-X U4 row walk. All other mirrored forms remain fail-closed.

  - 2026-07-17 cell-8 HFLIP consume update: cell 8 admits only GRAPHICSSET
    field `0x72`, RAW4 rect `0x359`, B073/RAW7 and its own source-locked
    reverse-X U4 row walk. All other mirrored forms remain fail-closed.

  - 2026-07-17 cell-13 HFLIP consume update: cell 13 admits only GRAPHICSSET
    field `0x77`, RAW4 rect `0x356`, B073/RAW7 and its source-locked reverse-X
    U4 row walk. All other mirrored forms remain fail-closed.

  - 2026-07-17 cell-15 HFLIP consume update: cell 15 admits only GRAPHICSSET field `0x79`, RAW4 rect `0x353`, B073/RAW7 and its source-locked reverse-X U4 row walk.

  - 2026-07-17 crop/chained-clip update: `QUERY_BLIT_RECT` source-coordinate mutation remains no-draw behind a source-locked PIT provenance receipt; root RAW4 does not prove crop or chaining.

- 2026-07-17 DM2 `DRAW_STAIRS_FRONT` primary GDAT material admission:
  `SKULLWIN/c_gui_vp.cpp:480-511` and `dm2data.cpp:289-310` now bind the
  successful `QUERY_GDAT_ENTRY_IF_LOADABLE` branch only: exact state-table
  lane, GRAPHICSSET SUMMARY_IMAGE/GFX256 raw bytes, decoded U4 indices, local
  palette, root RAW4 placement and the live DM2 composition/surface snapshot.
  It remains no-draw. The `QUERY_TEMP_PICST` fallback and the downstream
  B073/`QUERY_PICST_IT`/`DRAW_PICST` transform must be proven separately.

  - 2026-07-17 fallback update: the exact non-loadable `table1d6f7c` path at
    `c_gui_vp.cpp:514-527` now admits its own SUMMARY_IMAGE/GFX256 U4 and
    RAW4/M11 receipt plus `QUERY_TEMP_PICST(1,0x40,0x40,0,0,0,rect,-1,light,
    -1,8,graphicsset,field)` provenance. It remains no-draw because
    `query_32cb_0804` selects a live B073/field-7 palette transaction from
    `c_querydb.cpp:2415-2465`, which is not yet authenticated.

- 2026-07-17 DM2 `DRAW_STAIRS_SIDE` primary material admission:
  `SKULLWIN/c_gui_vp.cpp:540-565` and `dm2data.cpp:275-287` bind only cells
  1..8 with a defined `table1d6fdc/table1d6fee` state lane to authentic
  GRAPHICSSET SUMMARY_IMAGE/GFX256 U4 bytes, local palette, root RAW4 and M11
  owner surface. B073/`DRAW_PICST` remains no-draw pending a live palette and
  transform receipt.

  - 2026-07-17 transform provenance update: `SKULLWIN/c_image.cpp:450-475`
    now binds the side-stairs `DRAW_DUNGEON_GRAPHIC` delegation to blit mode 0,
    default normal scale and zero source offset; its source rects explicitly
    exclude the `0x2bc/0x2bd` offset special case. Material, RAW4 and M11
    identities must agree. The live `DM2_query_B073(image.palette,
    ddat.v1e12d2, alpha, -1, ...)` transaction remains unauthenticated, so
    the complete branch is intentionally no-draw.

  - 2026-07-17 live `DRAW_WALL` update: the receipt now binds one existing
    `QUERY_TEMP_PICST` wall command to the same recomputed material hash,
    M11 wall-composition identity and atomically identical owner snapshots.
    Only the source's `0x40` normal scale, RAW4 `0x2be + cell`, movement
    offset and source flip are recorded. This gate remains no-draw; it does
    not introduce a second wall renderer.

- 2026-07-17 DM2 `DRAW_WALL_TILE` admission: `SKULLWIN/c_gui_vp.cpp:6703-6741`
  and `dm2data.cpp:266-273,602-605` now bind every `table1d7012` cell branch
  to the existing authenticated wall/M11 identity. The receipt records the
  exact 0/1/2 delegated-call count and `table1d6afe` orientation; it remains
  no-draw because `DM2_guivp_32cb_15b8` has separate unbound GDAT transforms.

  - 2026-07-17 `32cb_15b8` input update: the first simple `QUERY_TEMP_PICST`
    call at `c_gui_vp.cpp:6618-6628` now has a source-owned no-draw input
    receipt for category 9 selector/image field, exact `0x40` scales, flip,
    query parameters and RG71l alpha. Record layout and destination remain
    explicitly unavailable.

  - 2026-07-17 loadable `0x0f` update: the distinct `c_gui_vp.cpp:6651-6692`
    category-9 `QUERY_GDAT_ENTRY_IF_LOADABLE` branch now binds its successful
    `0x0f` selector, normal scales, transform inputs and RG71l alpha as a
    no-draw receipt. Its destination is still not inferred.

  - 2026-07-17 category-8 overlay update: `c_gui_vp.cpp:6322-6329` now has
    its own no-draw QUERY_TEMP_PICST input receipt for selector/image field,
    normal scales, flip, transform parameters and RG71l alpha.

  - 2026-07-17 branch-set update: the three authenticated category-8/9
    `32cb_15b8` input receipts now combine only when their independent
    identities and the loadable `0x0f` field agree; the aggregate stays
    no-draw and has no placement contract.

  - 2026-07-17 DRAW_TEMP_PICST admission update: the aggregate now has a
    no-draw consumption gate that rechecks every branch-set identity,
    category, `0x0f` field and normal-scale transform before admitting the
    source call. It carries no destination or pixel information.

- 2026-07-17 DM2 `query_B073` input admission: `c_querydb.cpp:2506-2545`
  now requires authentic palette, live light, alpha/mask, colors/cache,
  RAW7, lookup and traversal identities in one no-draw receipt. No palette
  buffer or pixel result is produced.

  - 2026-07-17 B073/DRAW_TEMP_PICST surface update: authenticated B073 and
    DRAW_TEMP_PICST receipts now bind to an owned viewport-surface snapshot
    in one no-draw palette/surface receipt. No buffer is borrowed or written.

  - 2026-07-17 original palette update: the next consumer may borrow only
    original 16/256-byte palette storage when its bytes hash matches the
    caller's authenticated identity and the B073/surface receipt remains
    current. No transformed palette or pixel buffer is created.

  - 2026-07-17 M11 palette-consumer update: borrowed original palette bytes
    now bind to a current owner-surface generation in a no-draw M11 receipt,
    with no transform, destination or pixel material.

  - 2026-07-17 original material update: a later consumer may borrow only
    original decoded GDAT storage with proven dimensions, stride, byte count
    and byte hash paired to the current M11 palette consumer. No decoder or
    render path is admitted.

  - 2026-07-17 M11 material/palette pair update: original material and
    original palette now admit only as a matching no-draw pair with current
    owner generation and verified dimensions/stride. No render contract.

  - 2026-07-17 live materialization update: the validated pair now has a
    no-draw M11 handoff guarded by the same live owner generation. It carries
    only borrowed bytes/layout, never a blit or destination.

  - 2026-07-17 DRAW_PICST trace update: source handoff now reaches an exact
    `QUERY_PICST_IT`/`DRAW_PICST` trace receipt, but missing source and
    destination rectangles remain an explicit no-draw blocker.

  - 2026-07-17 DRAW_PICST rect update: `query1 == -1` now admits only the
    exact direct `srcx/srcy + imgdesc.x/y` source rectangle branch from
    `c_image.cpp:240-296`; all QUERY_BLIT_RECT, flip and destination paths
    remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT trace update: `c_xrect.cpp:217-280` now
    admits only an authenticated unsigned root rectangle node with
    `query2 == -1`, `mode1 <= 8`, `mode2 == 0`, a present bitmap and its
    captured source-rectangle identity. Signed, overridden, mode-9 and
    chained nodes remain no-draw until their clip/destination semantics are
    separately evidenced.

  - 2026-07-17 QUERY_BLIT_RECT signed-root update: `c_xrect.cpp:228-276`
    now records the exact signed-node `datax/datay + input-x/input-y`
    transform for an authenticated unchained root. `crdecode`, final clip,
    destination, overrides and every chained node remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT mode-1 update: the signed-root receipt now
    reaches the exact `crdecode(1, ...)` origin assignment in
    `c_xrect.cpp:162-211,426-436`, guarded by current authenticated material
    dimensions and surface generation. All other modes, clipping and final
    destination bounds remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT default-clip update: `c_xrect.cpp:239,438-470`
    now admits the untouched `rc=[-10000,10000)` range only for a current
    mode-1 receipt whose full material rectangle lies inside it. Global clip
    override, chained terminal nodes and all surface-specific destinations
    remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT global-clip update: the explicit
    `c_gui_vp.cpp:570-573` `TRIM_BLIT_RECT` transaction now provides the only
    admitted `dm2rect1` override input for `c_xrect.cpp:438-439`, with active
    flag, trim-call, material and surface identities. Intersecting that clip
    with the destination rect and every final blit remains no-draw.

  - 2026-07-17 QUERY_BLIT_RECT global-intersection update:
    `c_xrect.cpp:446-470` now admits the exact `dx/dy` source-offset and
    clipped destination-rectangle calculation for the authenticated mode-1
    global-clip path. Missing overlap or any clip/material/surface identity
    drift rejects; no blit is admitted.

  - 2026-07-17 DRAW_PICST surface-address update: `c_image.cpp:293-335` and
    `c_gfx_blit.cpp:604-656` now admit only the native 8-bit `gfxsys.dm2screen`
    row-address path with packed original source stride, exact source/dest
    offsets, no palette translation and no alpha mask. The receipt borrows
    addresses only; all pixel writes and other surface formats remain no-draw.

  - 2026-07-17 DRAW_PICST row-traversal update: the original material bytecount
    now remains attached through the M11 handoff. `c_gfx_blit.cpp:604-656`
    default `BLITMODE0` admits only forward rows with authenticated first/last
    row offsets and exclusive source/destination bounds. Other modes and every
    pixel operation remain no-draw.

  - 2026-07-17 DRAW_PICST mask/palette update: `c_image.h:45-70` and
    `c_gfx_blit.cpp:655-760` now admit only the masked translated `BLITMODE0`
    input transaction with 256 authenticated palette bytes, exact alpha index,
    original material bytecount and forward row bounds. Palette translation
    and every pixel write remain no-draw.

  - 2026-07-17 DRAW_PICST palette-index update: `c_gfx_blit.cpp:39-42,675-682`
    now has a source-locked trace that records the exact ordering: compare raw
    8-bit source index to alpha first, then use that same index in PAL256.
    It does not dereference source/palette bytes or write pixels.

  - 2026-07-17 DRAW_PICST palette-write update: source proof now fixes each
    `t_palette` entry to one `c_pixel256` byte and PAL256 to 256 bytes. The
    masked destination write order is carried as no-draw row metadata with
    current surface identity; no conditional pixel write is executed.

  - 2026-07-17 DRAW_PICST native execution update: the fully authenticated
    8-bit BLITMODE0/PAL256/mask branch now has its first source-backed pixel
    consumer. It revalidates all receipts and owner generation before the
    exact forward masked writes; every mismatch is no-write.

  - 2026-07-17 DRAW_PICST M11 update: the native executor now enters only
    through a DM2-owned M11 consumer that requires the exact live material
    handoff buffer/palette and owner generation. No legacy renderer or
    fallback path can reach this consumer.

  - 2026-07-17 DRAW_WALL admission update: authentic GDAT wall commands now
    enter a strict DRAW_PICST admission with their raw/decoded/palette/geometry
    receipts, but remain no-draw because the source route owns PAL16 rather
    than the proven native PAL256 executor contract.

  - 2026-07-17 DRAW_WALL B073 update: PAL16 now binds to a strict PAL256 cache
    output receipt only with complete RAW7, lookup, traversal and allocation
    identities from `c_querydb.cpp:2506-2668`; no expansion or write occurs.

  - 2026-07-17 DRAW_WALL B073 contiguous RAW7 loader update: only the
    original `INTERFACE_GENERAL/0/RAW7/2` record admitted by
    `dm2_v1_asset_load_typed_sized()` may bind its contiguous bytes, exact
    length and FNV identity to the wall PAL16/B073 cache allocation.

  - 2026-07-17 DRAW_WALL B073 interpreter update: `c_gdatfile.cpp:1919-2003`
    and `c_querydb.cpp:2506-2668` now source-bind RAW7's descriptor, interval,
    output and lookup regions to a supplied owned PAL256 cache. The resulting
    cache is attached to the wall `DRAW_PICST` output receipt, but remains
    no-draw until the authentic U4-to-PAL256 blit consumer is proven.

  - 2026-07-17 DRAW_WALL native M11 update: the proven normal, unflipped,
    unmoved 0x40 U4-to-8 branch now consumes the authenticated B073 cache
    using `c_gfx_blit.cpp:495-548` source order. Scaling, flip, movement,
    clip, cache, surface and composition drift remain fail-closed.

  - 2026-07-17 DRAW_WALL HFLIP M11 update: the separately proven BLITMODE1
    branch now follows `blitline_48_mi/mima` reverse-X destination order.
    Vertical/chained flips, movement and scale changes remain fail-closed.

  - 2026-07-17 DRAW_DOOR panel M11 update: the stationary, closed, unflipped
    and unscaled DOORS panel now consumes its exact IMG3 U4 bytes, local PAL16,
    colour key, RAW4 rectangle and composition-owned surface. Opening,
    movement and light-remap branches remain fail-closed.

  - 2026-07-17 DRAW_DOOR split M11 update: only the source-proven horizontal
    opening states 1..3 now consume the paired halves in the `DRAW_DOOR`
    table order: right half (`base + state + 6`), then left half
    (`base + state + 3`). Both halves require the same authenticated DOORS
    material receipt and distinct RAW4 geometry rows, plus current composition
    and owner surface identities. Vertical opening, movement, flip and every
    incomplete table/material chain remain fail-closed.

  - 2026-07-17 DRAW_DOOR vertical M11 update: the source-proven vertical
    intermediate states 1..3 now retain the whole original DOORS image and
    select exactly `tlbRectnoDoorPosition[cell] + state` before one forward
    palette-mapped consume. The raw material, RAW4 table row, composition and
    live surface must all still match; horizontal split, movement and flip
    remain separate fail-closed routes.

  - 2026-07-17 DRAW_DOOR_FRAMES right-jamb M11 update: the stationary
    `QUERY_TEMP_PICST(1, 0x40, 0x40, ..., rect, 3)` route now admits the
    authenticated GRAPHICSSET U4/PAL16 side-frame with reverse-X writes. Its
    scene-owned colour key and current scene hash are required alongside RAW4
    geometry, composition and surface identity. Left jamb, panel flips,
    frame motion and every other transform remain fail-closed.

  - 2026-07-17 DRAW_DOOR_FRAMES left-jamb M11 update: the matching stationary
    `QUERY_TEMP_PICST(0, 0x40, 0x40, ..., rect, 4)` branch now consumes its
    authenticated GRAPHICSSET U4/PAL16 material in forward-X order. Receipt
    identity locks the jamb kind, RAW4 row, scene colour key/hash, composition
    and live surface, so it cannot be used as the mirrored right route.
    Frame motion, scaling and panel flips remain fail-closed.

  - 2026-07-17 DRAW_DOOR_FRAMES movement M11 update: only the source's
    `v1e12d0` branch may select `table1d6b2c[cell]` and its swapped
    `table1d6ee1` jamb column, while preserving the original cell's RAW4
    rectangle and normal jamb direction. The movement owner bit, selected
    field, scene, composition and surface must all match; panel motion,
    scaling and every unrelated transform remain fail-closed.

- 2026-07-17 DM2 pit-roof viewport admission: `c_gui_vp.cpp:118-206` now
  source-gates cells 1..8 on the exact roof flag, `LOCATE_OTHER_LEVEL`
  success, remote tile type 2, and remote bit 0x08 before applying
  `table1d6c4c/5e/67`. The resulting GRAPHICSSET SUMMARY_IMAGE, GFX256 raw
  receipt, decoded U4 bytes and local palette remain `no_draw`; cell 0's
  position flip, the actual remote-map address walk, and `QUERY_BLIT_RECT`
  placement/clip still require separate evidence.

  - 2026-07-17 prerequisite update: the admitted PIT_ROOF receipt now also
    binds `DRAW_DUNGEON_GRAPHIC`'s `DM2_query_B073` c_light transaction and
    the authentic INTERFACE_GENERAL/0/RAW4 row for rects `0x360..0x368`.
    Only the exact root `mode1=1/mode2=0` `QUERY_BLIT_RECT` form is admitted;
    changed c_light identity, palette, RAW4 row/table, clip chain, cell 0,
    and every richer rectangle branch reject. It remains no-draw until the
    full B073 palette expansion and a pixel consumer are separately proven.

  - 2026-07-17 alpha/blend update: `SKULLWIN/c_image.cpp:450-475` and
    `c_gfx_blit.cpp:370-549` now bind the exact U4 alpha transaction to the
    B073 and RAW4 identities. The source alpha mask is retained in full and
    its low nibble is the only admitted transparent source index; only the
    proven normal and horizontal-mirror modes enter the no-draw receipt.
    Mask drift, vertical/combined modes, palette drift, and destination
    identity drift reject. B073's transformed palette and final destination
    composition still need independent source proof before any blit.

  - 2026-07-17 B073 table update: `SKULLWIN/c_gdatfile.cpp:1919-2003` now
    binds the exact `INTERFACE_GENERAL/0/dt07/2` RAW7 program that initializes
    `v1e020c` and `v1e0210` for `DM2_query_B073`. The count/length layout,
    both packed table regions, trailing color lookup region, raw hash, and
    B073/material identities are retained as no-draw evidence. Missing dt07/2,
    malformed lengths, and any valid raw-data drift reject.

  - 2026-07-17 B073 traversal update: `SKULLWIN/c_querydb.cpp:2506-2668`
    now admits only the cache-free per-color traversal for the authenticated
    U4 palette. Every palette byte must have an in-range two-byte RAW7 lookup,
    group, subindex, interval and alternate alpha neighbour; index drift and
    alpha-branch ownership drift reject. The transformed palette is still a
    no-draw receipt pending the exact QUERY_PICST_IT destination composition.

  - 2026-07-17 destination update: `SKULLWIN/c_image.cpp:98-410` now binds
    the normal-scale (`0x40/0x40`), zero-crop PIT_ROOF `QUERY_PICST_IT` path
    to its root RAW4 `QUERY_BLIT_RECT`, B073 palette traversal, alpha mask and
    source-proven horizontal flip. Clip receipt drift and every scale/crop or
    unsupported flip reject. It remains no-draw: the destination bitmap's
    live ownership, dimensions/resolution and final viewport clip are inputs
    to `DRAW_PICST` that are not yet retained by this DM2 receipt chain.

  - 2026-07-17 surface-owner update: DM2 viewport ownership now publishes an
    atomic framebuffer snapshot with pointer, dimensions, stride, resolution
    and monotonically advanced generation. PIT_ROOF binds only the exact
    current generation and remains no-draw on stale or rebound surfaces.

  - 2026-07-17 composition-slot update: PIT_ROOF additionally requires the
    DM2 composition slot's before/after owner surface pointer and generation,
    session identity, data epoch and ordered-member identity. Every mismatch
    remains no-draw; native blit still lacks a source-owned M11 consume hook.

  - 2026-07-17 material-buffer handoff update: PIT_ROOF now retains a borrowed
    identity receipt for the already authenticated decoded U4 buffer. Its
    pointer, width, height, stride, pixel count, palette hash and material
    identity must equal the composition candidate; every buffer or receipt
    drift remains no-draw.

  - 2026-07-17 ordered-consume update: the source-owned PIT_ROOF hook now
    executes only `DRAW_PICST`'s authenticated normal-scale U4-to-8bpp masked
    rows, including the proven horizontal mirror. It consumes the borrowed
    handoff buffer directly after the composition-order and before/after
    surface checks; there is no reload or re-decode. Every crop, scale,
    vertical/combined flip, changed source index, composition/surface drift,
    or incomplete receipt remains no-write.

# Theron V2 HUD widget pixels remain blocked in production: the manifest parser is fixture-only and the runtime now fails closed until all seven slots resolve to decoded Track 02 source assets.

- 🔧 CSB V2.2 artpack follow-up: the hand-authored per-cell asset-id catalog is
  contract-test-only; production retains just the F0128 source-provenance
  admissions. A reviewed PC 3.4 GRAPHICS.DAT pixel binding is still required
  before any modern art is admitted.

- 🔧 CSB V2.2 artpack follow-up: both mode selection and F0128 cache blits now
  reject a launcher flag or readable RGBA cache until the complete
  PC 3.4 source-material/provenance gate passes. The remaining work is a
  reviewed original GRAPHICS.DAT extraction and pixel binding; no generated
  cache or PBR substitute may be admitted.

- 🔧 CSB Utility Disk CMP follow-up: production accepts CMP bytes only as a
  portrait/name/title overlay for an already authenticated champion. A
  positive original CMP-plus-save corpus is still needed before exposing that
  combined import route in the launcher.

- 🔧 CSB creature-drop follow-up: the old no-op fixed-possession API and
  no-context DSA stubs are contract-only. Bind original dungeon placement and
  the imported DSA interpreter before enabling either live creature drops or
  DSA filters.

- 🔧 CSB hidden-graphics follow-up: only the real source-loader is available
  in production. Bind a verified original GRAPHICS.DAT hidden-item corpus to
  a visible owner before promoting those records into a runtime presentation.

- 🔧 CSB Atari ST graphics follow-up: the production DMCSB1 reader accepts
  only user-supplied Atari ST data. Bind verified original animation/image
  records to the startup presentation before promoting this container reader
  beyond its current source-data loading role.

- 🔧 CSB Mac app-capture follow-up: an interactive capture of the installed
  opening-door capture; compare a rebuilt installed app against v3.0.197
  before diagnosing or masking the old red-strip report.
  an invalid step-zero gap and retained the closed C004/C002/C003 page. The

- [ ] DM1-HOC-OBJECTS-001 Capture the corrected live PC34 HoC wall-torch
  material and holder composition against the original GRAPHICS.DAT. The
  source mapping is now corrected to ReDMCSB I34E `G0194` (DUNVIEW.C:932-1007)
  and the exact `G0198`/`G0199` palette/depth route remains source-bound; close
  only after a real app capture proves the torch and holder pixels at each
  visible depth. No synthetic black ornament is admitted. Invalid global
  ornament indices outside the 60-entry G0194 table now fail closed; the
  real capture is still required. Runtime now distinguishes the synthetic
  final local inscription slot from real global ornament 0, so a real
  ornament-0 torch/holder cannot enter the inscription path.

  - 2026-08-06 fallback audit: the remaining legacy wall/door/floor helper
    paths now fail closed unless the authentic per-map ornament table and
    decoded pixel buffer are present. They cannot manufacture a global
    ornament index or draw a dimension-only slot. This is code-side cleanup;
    the real Mac/window torch-and-holder capture is still open.

  - 2026-08-06 viewport-coordinate audit: the live M11 F0128 iterator uses
    normalized D3 outer-wall offsets `-1/+1`, while the raw F0115 D3L2/D3R2
    source contract also exposes `-2/+2` aliases. The C127 mirror admission
    now accepts both representations and keeps the real C346 backing material
    for `viewWallIndex` 0/1. Real PC34 all-cell coverage passes; Mac/window
    pixel capture remains open.

- [ ] DM1-HOC-OBJECTS-002 Capture a real PC34 HoC pickup/placement round trip
  The manual does not replace the required original PC34 runtime capture or
  the M564 name/slot evidence.
  C00/C01 hand masks and backpack ownership remain source-backed. The F0033

- 2026-08-06 source-runtime verification: the real PC3.4 alcove test now
  completes pickup-to-placement for Thing 5196 (graphic 511), preserving the
  source `AllowedSlots=0x40` mask and placing it in legal quiver slot C519.
  M564 name-table validity remains intact after placement. Remaining scope is
  real macOS/window capture plus the requested weapon, potion, scroll,
  container and junk corpus; do not reopen the source route without a failing
  real-data case.

  - 2026-08-06 source-identity hardening: the live DM1 F0115 floor and
    F0121/F0124 alcove consumers now require the raw PC34 `THING` record before
    resolving subtype or drawing an icon. Candidate viewport metadata can no
    longer manufacture a plausible but incorrect object when the source chain
    is incomplete; the real floor-item and alcove pickup/place tests still pass.

- 2026-08-06 update: the active legacy stairs helper now rejects dimension-only
  cache entries unless the authentic GRAPHICS.DAT surface is decoded
  (`loaded` and `pixels` are both present). This prevents an invalid stair
  cache record from reporting a successful draw and covering the source wall
  or floor. Real Mac capture of each visible stair depth is still required.

- 2026-08-06 update: the active DM1 zone-blit, door-ornament, destroyed-door,
  Thieves' Eye, and door-button consumers now use the same decoded-surface
  gate. Dimension-only cache records cannot reach `BlitRegion`/`BlitScaled`
  in those F0102/F0110/F0111/F0113 routes. The real PC34 sweep remains the
  authoritative data check; packaged Mac capture is still required.

- 2026-08-06 update: the DM1 action/spell utility-panel admission now also
  requires decoded C010/C009 pixel payloads, not only loaded flags and native
  dimensions. A dimension-only cache record can no longer suppress the real
  source-owned panel route while leaving the action/spell strip empty.

- [ ] DM1-HOC-OBJECTS-003 Capture the live held-object cursor on the host window
  after pickup and during movement. The source framebuffer now invalidates on
  pointer motion and hides the host arrow while G4055 is occupied; close only
  after a real Mac capture proves the object-shaped pointer remains visible at
  the mapped pointer position. 2026-08-06 source-side proof: the real-data
  `test_m11_dm1_real_object_names` now verifies 169 non-zero F0702 pixels for
  `EYE OF TIME`; only the packaged macOS/window capture remains.

- 2026-08-06 source-runtime hardening: authenticated DM1 V1 F0287 bar graphs
  now ignore `FIRESTAFF_V1_BAR_GRAPHS=0` and never re-enable the retired
  horizontal host bars. The switch remains available for non-source/debug and
  V2 compatibility sessions. Real object-corpus and held-cursor tests pass;
  packaged Mac capture remains governed by the open capture items above.

- 2026-08-06 CI follow-up: the CSB V2 touch/controller test now has its
  source-required PC34 VGA palette module. Continue watching the main build
  matrix; this closes only the missing-link regression, not a presentation
  parity claim.

- Theron teleporter resolution now rejects unresolved object-ID links and
  cycles; restore positive legacy links only when backed by an authenticated
  Track 02/T900 record corpus.

- [ ] THERON-V1-TRACK02-JP-LEVEL-DATA: promote the authenticated Japanese
  decompression and tile/map/object publication remain gated.
  Decompression and tile/map/object publication remain gated.

- [ ] THERON-V1-TRACK02-VRAM-CONSUMER: bind the real VDC BAT/tile and VCE
  palette snapshot to the source-owned square/material/UI consumer. An
  instrumented Mednafen replay now emits exact 64 KiB VRAM and 1 KiB VCE
  snapshots; the production viewport can explicitly mount that pair through
  `FIRESTAFF_THERON_VRAM_SNAPSHOT` and `FIRESTAFF_THERON_VCE_SNAPSHOT`, and
  the real-capture regression verifies non-zero BAT/tile data, 154 tile/palette
  pairs and 512 palette entries. This remains a screen-space capture binding:
  `$2600` source-LBA joins, object/level records, square-to-tile semantics,
  and production dungeon/UI admission remain blocked until the HuC6280
  consumer is disassembled and tied to Track 02. The current instrumented
  build uses SDL 2.32.70 through `sdl2-compat` with dummy video, so it does
  not claim native Quartz/SDL2 capture parity.

- [ ] THERON-V1-HUC6280-RAM-CONSUMER: the real US/JP bank-$1f static support
  fragment at `$243e` is now byte-verified in both retail ISO projections.
  It proves the bounded bit/byte helper, bank-switch table and forward/reverse
  byte paths, but it is not the post-CD `$2600` RAM-loaded consumer. Capture a
  source-owned RAM instruction window around `$2400–$2800` with executing PCs
  before promoting decompression, tiles, maps, objects or HUD pixels.

- 🔧 DM2 HUD follow-up: M11 now leaves the accepted V1 runtime frame as the
  sole production HUD owner. The retired V2 compatibility blit used a static
  GDAT plan without SKProject's live GUI/session inputs, so it cannot return
  until complete per-command, party and champion-state receipts drive the
  original UI route. Diagnostic V2 HUD modules remain non-production only.

- [ ] DM2 SKSAVE runtime restoration: the corpus reader now follows the
  **2026-08-07 real possession-continuation gate:** the corpus regression now
  passes every genuinely decoded direct-root link, in source order, into the
  bounded `DM2_2066_062b` 10-bit continuation reader. The 135/135 real
  PC-DOS checks therefore cover both record-body consumption and the
  subsequent type-9/type-0xE continuation boundary. The receipt remains
  read-only; live record-pool, possession-index, timer and GAME_LOAD owners
  are still not connected.

- [ ] DM2 champion-mirror activation: the canonical PC G1 dungeon has 16
  **2026-08-13 source-bound transaction progress:** the lifecycle seam now
  exposes a source-bound `SELECT_CHAMPION` transaction that requires the
  authenticated marker identity and every live mutation owner before it can
  commit. Its callback order follows `c_hero.cpp:1052-1200` (creation-map
  switch, signed `REVIVE_PLAYER`, first-party leader, tile possession
  transfer, champion-strip refresh, map restore, weight recompute). The
  mounted PC mirror receipt now drives a positive callback-order regression;
  production GAME_LOAD/session wiring and source hero-stat ownership remain
  open, so this does not yet claim playable champion selection.

- [ ] DM2 delayed movement ownership: `PERFORM_MOVE`'s real
  **2026-08-13 delayed-owner audit:** when the exact half-step gate admits,
  the execution receipt now exposes six missing live-owner bits (hero load,
  wounds, walk speed, Aura-of-Speed, current pose and tick/countdown). The
  proven mask remains zero for caller-supplied compatibility snapshots; no
  interpolation or viewport offset is enabled.

- [ ] DM2 creature animation-frame ownership: `DM2_1c9a_0958` now carries
  **2026-08-13 0958-owner progress:** the exact DB4 cursor now also performs
  the source `DM2_query_1c9a_02c3`/`DM2_query_4E26` 0xfc read during boot
  materialization. Static AI rows retain the real `frame_bit14`, query index
  and blended value through the viewport/runtime receipts; dynamic rows retain
  an explicit CAII block. No command-0 or `0xffff` frame is promoted.

- 2026-08-06: PC-DOS startup's decoded `TITLE/0/4` surface is now named and
  receipted as an original GDAT image route, not a fallback. It remains the
  verified alternative only when `SHOW_MENU_SCREEN` has no source raw-screen
  record; generated menu text or rectangles remain forbidden.

- 2026-08-06: DM2's cross-platform CMake build now has its immediate Windows
  and macOS linkage faults corrected. Re-run the GitHub build matrix after the
  verified main push; retain the usual platform-specific test coverage.

- [ ] DM2 startup status-panel ownership: host-authored English status,
  **2026-08-13 empty-panel removal:** successful DM2 launch/resume and the
  generic DM2 launch-failure callback now return M12 to its ordinary main
  view instead of displaying a blank host message panel. The launch intent
  and structured failure receipt remain intact; M11 can therefore hand the
  next visible frame directly to the source-owned `SHOW_MENU_SCREEN` or
  dialogue path. The actual source failure dialogue producer is still open.

- [ ] DM2 runtime action/save text ownership: action, shop, movement and save
  **2026-08-13 pre-resolver correction:** DM2 quick-save and quick-load now
  enter the source-owned silent boundary before shared path resolution. This
  prevents path-length, directory and other generic host errors from leaking
  into the DM2 status channel. The original `DM2_GAME_SAVE_MENU`/GAME_LOAD
  producer is still not connected, so the item remains open.

- [ ] DM2 GDAT structure loader: `DM2_READ_GRAPHICS_STRUCTURE` remains
  **2026-08-07 underlay progress:** a source-owned materializer now resolves
  the exact `dtRaw8/0/0` ENT1 row, reads its real four-byte image-to-underlay
  table through the ULP raw-entry reader, validates source raw-index bounds
  and sorted order, and returns payload/pair hashes. The mounted PC-DOS v5
  corpus has no such source row, so its regression stays fail-closed; no
  empty or synthetic underlay table is admitted. Positive underlay-corpus
  wiring and decoded overlay/cache ownership remain gated.

- **2026-08-07 save-dungeon parity correction:** the isolated
  `DM2_STORE_EXTRA_DUNGEON_DATA` teleporter gate now matches SKProject's
  `current_map > target_map` backward-reference skip; the complete raw-dungeon
  record allocator and runtime restore owner remain gated.

- [ ] DM2 combat source contract: a creature Defense GDAT row alone cannot
  **2026-08-07 party-wound correction:** the diagnostic `DM2_ATTACK_PARTY`
  seam now applies the source `DM2_MAX(1, per_hero_damage)` clamp before
  `WOUND_PLAYER`, matching `skhero.cpp:3365-3392`; a `base_damage=1` regression
  is green. The live champion/target/RNG/writeback chain remains absent.

- [ ] DM2 FM Towns English text consumption: a selected FM Towns Japanese CD
  **2026-08-13 direct-launch parity:** `firestaff --game dm2 --fm-towns`
  now accepts `--dm2-english-companion <PC-English GRAPHICS.DAT>`, forwarding
  that explicit path through the same M12→M11 launch receipt as the menu.
  The boot layer still verifies its canonical hash and keeps it in RAM; the
  option does not broaden text-consumer admission or unpack game data.

- 2026-08-06: the full 30-file retail MNS corpus now decodes without silent
  texture/MOTN truncation (VEXIRK=64 TEXT descriptors, D_GOLD=11 MOTN
  tables). Remaining work is original Saturn/VDP1 capture and source-locked
  face/mesh texture placement; parser success is not viewport proof.

- 2026-08-06: the MNS pose/texture helper is now excluded from the production
  Nexus library because its fixed-point Taylor trig and BGR555 conversion
  have no Saturn execution/capture receipt and no production caller. The
  real-data decoder test still compiles it explicitly; restore a production
  mesh route only after VDP1/VDP2 capture proves rotation, CLUT and draw order.

- 2026-08-06: DGN Structure2 texture decode now resolves DMWeb's real
  `Palette offset = 0` reuse rule by prior Palette ID association. The
  hash-verified LEV00-LEV15 corpus decodes 1,678 descriptors (1,553 indexed4,
  125 direct555). Remaining gap is Saturn VDP1 upload/CLUT and Structure3
  face-to-texture/draw-order capture; do not promote this byte proof to pixels.

- 2026-08-06: Nexus spell lookup remains available from the real DM.BIN table,
  but `nexus_v1_cast_spell()` is now side-effect free and returns `-1` until a
  Saturn dispatcher capture binds mana commit, effect/target routing, RNG and
  SLEV/SFX publication. The previous host mana/damage mutation was synthetic.

- **NEXUS-EVENT-DGN-OWNER-CAPTURE:** Real DGN Structure1F/Structure1B bytes
  remain retained as source evidence, but the runtime no longer promotes
  apparent door/teleporter/pit/stairs records into live registries. The
  verified corpus does not prove that low DGN bits select DM1-like events,
  nor that `SDDRVS.TSK` dispatches them. Original-Saturn capture must bind
  event owner, selector order, destination fields, and state transitions.

- **NEXUS-UI-EVENT-DISPATCH-CAPTURE:** Retail `nexus_mechanics_dispatch_event()`
  now rejects host UI events for ISO/extracted data until the Saturn SLEV/SDDRVS
  producer, queue and state-write contract is captured. The source-less fixture
  lane remains available for isolated tests. Bind the original event route before
  admitting automap, inventory, save, leader, throw or drop mutations.

- **NEXUS-LEVEL-TRANSITION-CAPTURE:** The public level-transition helper now
  rejects ISO/extracted transitions until the Saturn SLEV/SDDRVS owner is
  captured; the tick gate alone was insufficient because callers could invoke
  the helper directly. Bind the original transition producer, destination
  fields and level-load timing before enabling retail level changes.

- **NEXUS-BPK-NO-DRAW-REGRESSION:** The bounded PRS3 presentation receipt must
  continue to admit exact retail-shaped rows only as opaque no-draw evidence;
  decoder drift, payload/hash drift, unknown modes and malformed spans must
  remain rejected before M11. The previously inverted matching-row assertion
  is corrected and the focused BPK/M11/Saturn-card gates are green.

- **NEXUS-WORLD-SCRIPT-CLAIM-QUARANTINE:** The linked native world/save state
  now labels its event, timer, hash and provisional action vocabulary as
  Firestaff-native/test state rather than recovered SDDRVS/SLEV semantics.
  Keep the actual SLEV task body, callback owner, event selector and dispatch
  capture-gated; do not promote the compatibility enum into Saturn opcodes.

- **NEXUS-SAVE-ROUNDTRIP-STACK:** The manager-level native save round-trip is
  now verified with heap-owned test state; keep the serialized world contract
  unchanged while extending real Saturn-card save provenance separately.

-  - 2026-08-06 Nexus PRS3 capture-schema correction: the real retail
    `MENU.BPK` MD5 admission constant was stale (`c277...`) while the
    verified corpus and boot profile use `a6f2272a4f6cb3c6b3b33012bc5b15ed`.
    Update the capture-sidecar evidence only; Saturn authentication and
    runtime texture upload remain blocked until independent VDP1 capture.

-  - 2026-08-06 Nexus production-source boundary now has a CTest verifier.
  It keeps synthetic V2 HUD/renderer modules and unproven text/MNS
  presentation paths out of `firestaff_nexus` during future source-list edits.

-  - 2026-08-06: `.github/workflows/verify.yml` now hard-runs that data-free
  production-source boundary after the cross-platform Nexus library build.
  Real retail-media and Saturn-capture tests remain local by design.
2026-08-06 regional capture follow-up: the same private CUE normalization
now accepts the archive's Japanese `TQJP02.iso` alias and binds the complete
sibling `TQJP02End.iso` only after the authenticated JP ISO MD5 matches
`397039af02d50d15c70b74088eb8a1cb`. The new generic `THERON_CUE` variable
retains `THERON_US_CUE` compatibility. A fresh JP consumer capture remains
required before semantic promotion.

- **CSB-AMIGA-LIVE-AUDIO:** M11 now transports the selected authentic Amiga
  `GRAPHICS.DAT` sample bytes through the F0709 period calculation
  (`ioa_Period = 72800 / SOUND_DATA.Period`) rather than falling back to the
  PC3.4 PIT/marker route. The remaining Amiga work is source-captured
  audio.device voice allocation, left/right volume arbitration and overlap
  behavior; do not infer those from PC3.4's distance-volume model.

- 2026-08-07: An authentic European Mednafen capture now records a 48-word
  SH-2 code window around the VDP1 source writer at runtime PC `0x06013098`
  while it writes `0x47c00`. The routine contains a real branch to
  `0x06012f52`, but relocated/decompressed code is not yet joined to an
  authenticated DM.BIN/TM.BIN source span. Keep VDP1/VDP2 composition and
  production draw admission blocked until that identity and command/CLUT
  contract are proven.

- 2026-08-07: The authentic high-RAM load trace shows 3,080 writes into the
  `0x06013000..0x06013fff` code corridor from runtime loader PC `0x2368`.
  This is a BIOS/runtime-loader receipt only; the trace does not yet expose
  the CD source read or identify the retail member that supplied the bytes.
  Keep the VDP1 source join and production composition blocked.

- 2026-08-07: The Saturn-CDB hook now traces the real `cdb.cpp` data-sector
  path. The current bounded run reaches only BIOS LBA `0..16` (1,024 reads);
  no `DM.BIN`/`TM.BIN`/other retail member has been joined yet. Continue with
  a capture route that reaches the authenticated game startup window; do not
  promote the BIOS sector receipt to SLEV/SAL or VDP1 source evidence.

- 2026-08-07: The corrected input ordering now reaches the authenticated
  French Nexus startup window. A 50,000-read CDB trace joins `DM.BIN`,
  `TM.BIN`, `ITEM.IBS`, `MENU.BPK`, `SLEV00.BIN`, `SDDRVS.TSK`, DGN and SAL
  spans to the retail ISO; the new analyzer reports this as LBA provenance
  only. The same run records 3,080 runtime-loader writes and one raw frame,
  but no VDP1 writer trace. Keep PRS3 pixel consumers, VDP1/VDP2 composition,
  HUD/viewport, SLEV/SAL/SDDRVS semantics and SFX playback blocked pending a
  live producer/consumer join to those authenticated bytes.

- **DM2 SKSAVE direct-root pool ownership:** The raw DB baseline and DB4–DB15
  clear phase are now followed by source `READ_RECORD_CHECKCODE` allocation
  into the authenticated c_record pools, including source list links,
  child-owner fields, type-9/type-0xE continuation writes, and a hash/count
  receipt. Remaining work is attaching the returned roots to champion/hand,
  possession-index and tile-chain owners; failed decode restores the cleared
  baseline and never publishes a session. The mounted workspace has no raw
  SKSAVE corpus, so this positive path remains compile/test-gated until one is
  supplied.

- **THERON-RNG-RETURN-OWNER:** The external-disk `.mc0` replay now captures a
  declared 4,096-step `$5D64` execution window, but the state reaches no
  `$4667` helper or game-owned CD→RAM join and exposes `return_pc=0001` rather
  than an authenticated caller return. Keep RNG, spawn, creature AI, loot,
  generator, T700 and T900 admission closed. The next required witness is one
  same-session state or live replay that joins `$4667` → `$5D6A/$5D64` → return
  value to the authenticated Track 02 payload and consumer.
# 2026-08-10 — source roster/stat handoff is fixed

- Completed: optional US roster text no longer blocks the source-owned
  champion stats/skills handoff at forcefield entry.
- Remaining: authenticate the US text consumer and T900 equipment semantics.
# 2026-08-10 — source group bounds fixed

- Completed: category-4 live-creature admission now applies the same
  four-member source bound in its counting and materialization passes.
- Remaining: authenticate dynamic RNG, AI, T700 and T900 consumers.

# 2026-08-13 — Theron-verifier tests respect external TMPDIR

- Completed: capture-manifest, HuC6280 event-log, SRM-classifier and rendering
  fixtures now place their temporary files below `TMPDIR` when it is set.
  This allows the focused Theron verification set to run on the external disk
  when the macOS system volume is full, without changing runtime paths or
  promoting synthetic rendering.
- Remaining: the full suite still requires complete authenticated runtime
  capture inputs beyond the available System Card and media, and the semantic
  text, square/material, RNG/AI/loot and T700/T900 consumers remain closed
  until their source/runtime joins are proven.

# 2026-08-13 — fresh System Card replay confirms transport-only boundary

- Completed: a new local replay with hash-verified US Track 02
  (`f23601102138f87c33025877767ebf76`), real System Card 3.0 and instrumented
  Mednafen ran from the external disk. It produced 161 raw sectors, 51 SCSI
  commands, 25 CD IRQ callbacks, 161 sector bindings, 47 byte-exact FIFO-to-RAM
  receipts and 65,536 VDC-I/O writes. `verify_theron_origin_ram_receipt.pl`
  passes all 47 receipts.
- Remaining: the same session has no game-owned FIFO-to-RAM receipt, spawn-
  consumer reads or RNG windows. It therefore does not open dungeon-consumer,
  square/material, RNG, AI, loot, T700 or T900 semantics. Raw output remains
  local at `/Volumes/Extern-disk/theron-capture-20260813/replay/` and is not
  pushed.
