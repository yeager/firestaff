# Firestaff TODO - Open Work

- 🔧 DM2:s återstående byte-tillstånd får endast aktiveras genom den atomära,
  originalägda GAME_LOAD-kedjan. Produktionsgrinden verifierar nu också att
  den historiska word-square-fixturen kompileras exklusivt i sitt enskilda
  regressionstest; den får aldrig återinföras i en produktionsarkivväg.
  Direktrotternas source-ordnade possessionlänkar är nu bevarade, men den
  gemensamma ägaren måste först lägga till specialtimer- och kartkedjornas
  länkar innan DM2_2066_062b får läsa några continuation-värden.
  Sparfilernas typ-5-rutor följer nu den källägda DB1/c_map-frågan, men i den
  här fasen saknar de återställda DB1-kedjor och måste därför fortsätta genom
  originalets no-detail-gren tills kartkedjorna har lästs.

- 🔧 DM2 GAME_LOAD: File_header-starten på karta 0 är bunden till den riktiga
  monterad dungeon eller tom lokal cooldown får inte presenteras som en
  rörlig party.
  medium, inklusive hela File_header-världens interaktionskvitto. Nästa atomära
  inte användas som SKSAVE- eller GAME_LOAD-ersättning; nästa ägare ska

- 🔧 Theron: lossless rårecord-provenance är nu bevarad genom inventory och
  save/load. Nästa krav är en autentisk `$2600`-konsumentcapture som binder
  T900:s equip/use/stack/loot-regler; gissade mutationer ska fortsatt nekas.

- 🔧 Theron: source-spawnkategorin följer nu autentiska live creatures och
  save/load. När `$4667`/`$5D64`/`$5D6A` fångas ska den kopplas till originalets
  RNG-resultat, utan att kategorin i sig används som gissad gameplaystatistik.

- 🔧 Theron: CDDA-handoff är verifierad mot den lokala original-RAR-korpusen.
  Nästa ljudkrav är source-bound SFX/ADPCM-eventägare; CDDA-tillgänglighet får
  inte tolkas som bevis för gameplay-ljud.

- 🔧 CSB:s Amiga A31-väg visar nu den hashverifierade, separata TITL.DAT-
  animationen genom APPA.C → ANIM.C:s första bild och 30 kompletta DL-steg.
  M11 äger nu exakt en 50 Hz-VBL-klocka för den vägen; PC34:s
  startkvittomotor kan inte längre ticka samma A31-bild en andra gång.
  När den verkliga 606-VBL-strömmen har löpt klart stannar M11 på den sista
  autentiska bilden. A31M:s `APPB.FTL` språkvalsyta avkodas nu från samma
  hashverifierade program. Dess språkval följer nu `SWITCH.C` F1288:s
  releaseväg till samma hashverifierade `KAOS.FTL`, med APPA.C:s autentiska
  parameter 0/1/2 för engelska, franska respektive tyska. A31:s senare meny-, Prison-
  och Utility-ytor saknar fortfarande autentisk capture.
  Den sista DL-posten läser utanför FTL-itemet i originalprogrammet och
  lämnas därför synligt på senast kompletta originalbild tills en autentisk
  allocationscapture finns. A31E saknar den här verifierade TITL/APPA-
  mediefamiljen i den valda originaldisken och är fortsatt fail-closed. A35M:s
  verkliga APPB-språkval och engelska KAOS-handoff är nu bundna till sina
  ADF-hashar och exponeras som en egen M11-capturefas. A35E:s separata,
  engelska `APPB.FTL`-C03-handoff och `BJELoad_R`-C02-launcher är nu också
  hashbundna till M11 utan en A35M- eller PC34-ersättningsyta. A35E:s
  direktlagrade IMG1-poster avkodas nu ur den autentiska `GRAPHICS.DAT`-filen
  enligt IMAGE1.C och C013-rörelsepanelen samt C017-inventariebackdropen visas
  för A31M/A35E/A35M med ursprunglig G0021-palett. Kandidatläget komponerar
  också den äkta C040-resurrect/reincarnate-ytan över C017 med originalets
  C06-transparens. Amiga-DMCSB2 binds enbart via den egna decoded-only-vägen;
  PC34-laddaren och dess fontkonsument får inte tolka samma bytes. De verifierade
  Amiga-utgåvornas storlekstabeller är direktlagrade och följer F0474, inte
  Atari ST:s LZW-väg; icke-IMG1-poster avvisas tills de har egen konsument.
  Återstående Amiga-dungeonviewport, champion-HUD och övriga
  statusboxöverlägg samt inventarie-/kandidatinmatning saknar dock egna
  autentiska konsumenter. C026-porträttet har nu en avgränsad native
  kandidat-/statusboxkonsument, inklusive C027:s källägda namnändringsyta,
  men inte hela den interaktiva panelkedjan.
  Den återstående A35M-runtimen saknar fortfarande egen verifierad
  konsumentväg. Atari ST:s redan
  separata ANIMATE.SCR/ANIMATE.DAT-väg behöver
  fortfarande full runtime-/menyhandoff efter animationsprogrammet.

- 🔧 PC34:s Utility-import kan nu i den opt-in-bundna realdatakedjan läsa en
  klassificerad original-`DMSAVE.DAT` tillsammans med den hashverifierade
  Utility-ADF:n. Den lilla konstruerade DM1-bufferten finns kvar enbart för
  datafri CI. Kvar är autentiska CSB-sparningar och DSA (uttryckligen
  uppskjutet), bytekorrekt originalskrivning och den kompletta filväljarytan.

- 🔧 CSB:s FM Towns-väg återupptar nu verkliga MINI.DAT-party-, dungeon-,
  som mus/tangentbord. Kvar är extern app-capture och bred fysisk
  touch-/hjälpmedelsverifiering på varje plattform.
  Kvar för en komplett plattformsväg är autentisk Utility Disk-filväljare,

- 🔧 Theron real Track 02 loading now retains reserved category-4 monster
  bytes as source records while admitting only the authenticated 0..6 roster
  to live creatures. The original RNG consumer, generator timing,
  reactivation, AI, combat, loot and T700/T900 consumers remain capture-gated.

- 🔧 Theron verified-level movement now advances the common world tick and
  timer/consumer dispatcher while leaving unresolved T700-owned stats
  untouched. The hardening probe is aligned with that boundary; exact T700
  cadence and field mutations still require original runtime evidence.

- 🔧 Theron type-6 generator records now decode the real generation,
  toughness and pause overlay and persist it in save version 6. This remains
  source data only: original RNG consumption, activation timing,
  reactivation and creature-spawn ownership still require the authentic
  HuC6280/System-Card capture and must remain fail-closed.

- 🔧 Theron authentic runtime capture now has a source-built instrumented
  Mednafen path, but the local runtime verifier rejects the linked
  SDL2-compat library. A verified US System Card 3.0 and original media
  capture are still required; RNG return values, spawn timing, AI, T700/T900
  consumers, media bindings and gameplay semantics remain fail-closed.

- 🔧 Theron T900 capture instrumentation now records both reads and writes in
  `$2600-$27FF`, including logical/physical address and executing PC provenance.
  The instrumented source builds through the Mednafen link step, but the local
  runtime verifier still rejects the linked SDL2-compat library; no T900
  gameplay semantics are enabled from this instrumentation alone.

- 🔧 Theron media audit 2026-08-08 confirms the real US/JP Track 02 bitmap and
  palette bytes, but no authenticated VDC/VCE snapshot is present locally.
  Keep startup presentation and README screenshot publication blocked until a
  System Card-backed capture joins FIFO/RAM to the VCE/VDC destination. Never
  fill this gap with generated or guessed media.

- 🔧 Theron `.spawn-registers` sidecars now have a strict parser tied to the
  authentic `$4644`, `$4667`, `$C96B-$CA69` and `$CC4C-$CD13` disassembly
  windows. The parser records CPU/RAM provenance only; it still must not
  publish RNG, creature, AI, loot or generator semantics without the original
  System Card-backed capture.

- 🔧 Theron live creature materialization now translates the zero-based
  Track 02 roster types (`AKUTUBA=0..DEMON=6`) to the one-based runtime enum.
  An authentic raw type `7` is retained as source data but remains unbound
  until a matching roster/graphics consumer is proven; RNG spawns, AI, combat,
  loot and generator execution remain blocked.

- 🔧 Theron real-level pickup now requires the source property row as well as
  the object reference/category/type. Equip, consume, stack, drop and save
  consumers still need their original T900 ownership rules; incomplete source
  objects remain rejected rather than entering inventory.

- 🔧 Theron T900 proof audit 2026-08-08 confirms the real 66-row property
  tables and source object records, but the verified bank-$1f receipt still
  reports `ram_consumer_2600=not_present` for both US and JP. Keep all T900
  use/equip/stack/drop/loot promotion blocked until a real `$2600` runtime
  capture joins the record source to the object-state writes.

- **DM2-CHAMPION-DYN4-LOCALLEVEL-QUEUE:** PC-DOS boot now retains the
  riktiga spegelmarkörernas egna `0x16<hero-type>ffff`-DYN4-selektioner är
  nu verifierad mot GDAT; det ersätter inte den sammanhängande originalkön.
  explicita kontraktstest, eftersom den saknar den atomära ägaren för

- **DM2-GAME-LOAD-OWNER-HANDOFF:** New Game and Resume now both stop at the
  original `DM2_GAME_LOAD` boundary. Recover one atomic owner for map,
  record pools, possessions, heroes, timers and actuator generators, then
  install its source-shaped session handoff here. The pre-
  `DM2_READ_SKSAVE_DUNGEON` prefix is now one source-identity receipt for the
  raw dungeon, shared SUPPRESS stream and source-sorted c_tim queue; recover the linked
  record, possession, actuator-generator and post-load-effect phases next.
  A boolean or a parsed save receipt alone must never make a game playable.

- **DM2-GAME-LOAD-OWNER-HANDOFF (path identity):** The hash-selected loose
  `GRAPHICS.DAT`/`DUNGEON.DAT` owner is now normalized through filesystem
  links before boot opens optional companion media. A selected archive keeps
  its original user-visible path instead, so a link cannot silently replace
  its M12 media receipt. Continue to keep this as path identity only: it
  does not supply the missing original `GAME_LOAD` session owner or permit
  play.
  Rörelse, vändning och runtime-tick kräver nu samma owner-bit som M11.
  Timer- och renderhändelser ska fortsatt flyttas till den atomära handoffen i
  stället för att förlita sig på en monterad File_header-värld.

- **DM2-NEW-GAME-MEDIUM-IDENTITY:** New Game now refuses a stand-alone
  `assets_verified` flag or a caller-supplied rescan path. It requires the
  M12/M11-mounted, hash-selected graphics and File_header dungeon pair, but
  still stops before `DM2_GAME_LOAD` creates the missing party/record/timer
  graph. `DM2_LOAD_NEW_DUNGEON` also clears a prior session-readiness bit
  when it clears party and LeaderPossession; only the later complete original
  transaction may publish it again.

- **DM2-SKSAVE-CORPUS-INVENTORY:** Startmenyn kan nu hitta en verklig,
  källstavad SKSave-katalog under den hashskannade dataroten och skannar den
  därefter i originalets direkta slotordning. Detta är bara inventering:
  Resume är fortsatt spärrad tills den kompletta `GAME_LOAD`-ägaren finns,
  och ingen rå SKSAVE importeras eller skrivs. 2026-08-08: även den tidigare
  separata specialtimerläsaren är borttagen ur M10; produktionsgränsen avvisar
  fasen tills hela originaltransaktionen äger den. Den finns kvar endast i
  sitt explicita läsartest och får inte användas för att publicera en session.

- **DM2-FILE-HEADER-LOCALLEVEL-OWNER:** The callback-shaped
  `DM2_LOAD_LOCALLEVEL_DYN` translation is now explicitly blocked unless a
  single live owner proves File_header maps, record pools and links. Replace
  its test-only map bridge with that real runtime owner, then recover the
  complete original tile/actuator/sensor resource queue before calling DYN4.
  The M11 boundary now keeps a parsed File_header world out of
  `level_loaded` and party publication until that GAME_LOAD owner exists.
  Tick, rotation, movement, front-cell interaction and the rotating runtime
  HUD capture are likewise blocked until the same owner restores c_hero,
  record pools, possessions and timers.
  Den tidigare testadaptern kan inte längre verkställa en callback-uppbyggd
  DYN4-kö; återinför bara detta genom en produktionsväg som kan bevisa hela
  `DM2_LOAD_LOCALLEVEL_DYN`-transaktionen från en atomär `GAME_LOAD`-session.

- **DM2-FILE-HEADER-LOCALLEVEL-CONSUMER:** Map-0's bounded File_header
  från samma kedja. Bind dem till originalens inventory-, scen- och
  projektilkonsumenter innan någon värdmodell får materialiseras.

- **DM2-CREATURE-POSSESSION-RUNTIME:** DB4-ägd possessionslänk och varje
  verklig underkedja är nu verifierade utan mutation. Återställ därefter
  originalens inventarie-, dropp-, flytt- och timertransaktioner innan några
  föremål kan bli spelbara.

- **DM2-FILE-HEADER-DOOR-RUNTIME:** Canonical DB0 door records are now
  retained across all 44 File_header chains. Bind their original animation,
  lock/key, sound, button and sensor consumers before publishing door
  transitions to gameplay.

- **DM2-FILE-HEADER-ACTUATOR-RUNTIME:** Canonical DB3 records now decode
  their source target/delay/effect fields across every File_header chain;
  dense original maps are no longer truncated at 64 entries. Recover
  `DM2_INVOKE_ACTUATOR`, DB14/timer ownership and target record mutation
  before any actuator fires.

- **DM2-FILE-HEADER-TELEPORT-RUNTIME:** Canonical DB1 records now expose the
  original `w2` destination, scope, sound and rotation fields, plus `w4`
  destination map across every File_header chain. Bind `c_moverec` map
  changes, party/session ownership, collision and sound before allowing a
  transition.

- Keep `docs/DATA_SETUP.md` aligned with every change to a game's hash-gated
  launch roles or optional original-media routes. Do not turn optional media
  into a filename-only fallback or require users to unpack their archives.

- Theron: static Track 02 category-4 monster groups now enter the live
  creature pool with source type/position/count/HP. Bind the original random
  wave consumer, creature AI/combat/drop ownership and generator timing before
  treating live creatures as complete gameplay parity.

- Theron: the raw US spawn body is now source-locked at HuC6280 $B0E5,
  including its L4644/L4667 call sites and HP clamp. Recover the external RNG
  and helper consumers before replacing the old diagnostic formula helper.

- Theron: the static US `$4667` helper contract is now byte-verified at raw
  BIN offset `$9c4e7`; recover its RAM-loaded `$5d6a/$5d64` bodies and dynamic
  RNG state before promoting it into live spawn logic.

- Theron: the adjacent US `$4644` preconsumer and its static `$C96B`/`$CC4C`
  body spans are now byte-verified; recover bank-switched runtime state,
  helper callees and return-value ownership before promoting the original
  random-spawn path into live gameplay. The live capture now emits a separate
  disassembly-bound `.spawn-consumer` receipt for those windows and `$5D64/$5D6A`,
  and Firestaff now strictly validates that receipt; runtime RNG/spawn
  semantics are still not admitted. The capture also records lazy register/
  `$B3-$BB` snapshots at the four disassembly boundaries; bind the returned
  value and bank-switched helper contract before enabling gameplay.

- Theron: runtime VRAM/VCE file admission is now byte-identified by the authenticated capture FNV receipts; the remaining presentation gap is the original square/material/perspective consumer and a source-owned HUD/text/portrait route.

- Theron: an earlier Track 02 screen-space artifact was withdrawn from README
  after the current promotion gate rejected it. Publish the first real image
  only after an authenticated runtime receipt proves the VDC/VCE route; then
  replace the capture-only path with the T520/T600 square, perspective and HUD
  consumers when their runtime ownership is recovered.

- Theron: world snapshot version 5 now preserves all 64 source-generator runtime slots and reads version 4's five-slot tail; bind the original T700 generator consumer, cadence and reactivation semantics before making those records executable.

- Theron: regular-spawn admission now requires the matching raw Track 02 monster type and non-empty source group record; bind the dynamic RNG return contract before publishing those records as new live creatures.

- Theron: verified-level moves now dispatch the common world tick without guessed stat drains; bind the original T700 field consumer and exact cadence before mutating hunger, water, stamina or poison.

- Theron: source inventory, object, timer and admitted live-creature provenance now use explicit field-by-field wire layouts; retain version-1/2 readers only as migration support.

- Theron: source-level pickup now rejects unbound/generic object fallback; bind source-authenticated use/equip/consume consumers before allowing those transitions on real levels.

- **DM2-PC-DOS-LEGACY-G1-RECEIPTS:** The verified PC-DOS `DUNGEON.DAT` takes
  the 44-map `File_header` route. The completed audit repaired twelve
  real-data tests that had asserted an older shifted 28-map interpretation.
  Audit the remaining `g1_direct_*`, champion and material receipts one by
  one before they can claim PC-DOS provenance. They must either use the
  File_header-owned record graph or explicitly prove that the legacy G1-only
  path remains unavailable. The isolated real-data champion receipt now has
  only its actual dungeon/GDAT/lifecycle link dependencies; do not re-add a
  SKSAVE translation unit to make it carry unrelated runtime ownership. Do
  not restore a continuation segment,
  pseudo-pool or DYN4 selection to make an old receipt pass.

- **CSB-PLATFORM-NATIVE-STARTUP-OWNERS:** The PC34 launcher-handoff regression
  now pins its PC34 package instead of inheriting a persisted platform choice;
  it therefore cannot mistake Atari, Amiga or FM Towns startup for TITLE.C
  F0437/ENTRANCE.C F0806. The Amiga A31/A35 route is now fail-closed rather
  than replaying that PC34 session. Continue with a native `TITL.DAT`/APPA.C
  application handoff; do not reuse PC34 title, entrance, HUD or viewport
  receipts for that platform. The selected A31 7z→ADF core is now explicitly
  materialized and hash-checked before this boundary. Its native TITL.DAT and
  DUNGEON.DAT evidence is separately real-media gated; preserve those exact
  package identities throughout the native handoff.

- **NEXUS-VDP2-WRITER-CANDIDATE-OWNERSHIP:** The authentic 64-window VDP2
  code receipt now has a bounded partial-match analyzer. The primary writer
  at `0x06011924` reaches only four aligned words in `DM.BIN` and three in
  `TM.BIN`; a ten-word match at another runtime PC is shared by both files.
  Keep these as review leads only. The source owner, relocation/decompression
  path, tilemap/CLUT consumer and menu/HUD/viewport meaning remain unbound.

- **NEXUS-VDP2-LAYER-CONSUMER:** The authentic European raw witness now decodes
  `TVMD=0x0080`, `BGON=0x0002` (only `NBG1` enabled), and `CHCTLA=0x1211`
  selects NBG1 bitmap mode, colour code `1`, bitmap-size code `0`, and
  `BMPNA=0x0000` bitmap palette `0`. `PNCN1=0x00c0`, `MPOFN=0x3000`,
  `CRAOFA=0x1000` and `PRINA=0x0503` are also observed, but the map registers
  are not the active NBG1 source in this bitmap-mode frame. Join the bitmap
  VRAM span and CLUT to a retail source record before assigning it to title,
  menu, HUD or viewport composition.

- **NEXUS-CAPTURE-INVENTORY:** `analyze_nexus_capture_inventory.py` now
  inventories the complete external raw-witness directory without treating
  hardware state as a screen label. The current 38 valid captures contain
  12 reset/no-layer frames, 14 RBG0-only CD-player frames, 100 NBG1-only
  dungeon frames and 14 other active VDP2 frames. The latest European
  `run-french-hold-starta-skip18000` run contributes eight active VDP1 frames;
  its source span and NBG1 bitmap still have no exact retail MENU/MNS/DGN/
  ITEM/STABG/TITLE or CLUT join. It also accepts a single operator run as the
  capture root, so a fresh witness cannot be silently omitted before corpus
  merge. No startup/menu/HUD/viewport asset consumer is authenticated; future
  capture work must add the asset and consumer join.

- **NEXUS-VDP2-BITMAP-SOURCE-JOIN:** The bounded real-data comparator now
  derives the active 131072-byte NBG1 bitmap span at VRAM `0x000000` and
  compares it with 162 decoded retail MENU.BPK PRS3 surfaces plus 242
  authenticated FONT256 character-generator tiles and five DMWeb
  TITLE.BIN/TITLE.CG MAPD/TIBG maps plus the first authentic STABG.BIN map.
  The authenticated 256-entry MENU.BPK PALT record is also checked. The
  one-frame and frame-7 eight-frame captures have zero non-zero exact matches,
  and MENU, title and STABG palettes have no exact byte or word-swapped CRAM
  match. Keep the result negative and continue with dungeon bitmap and CLUT
  producer joins; do not promote a guessed source.
  2026-08-08 CLUT follow-up: the comparator now checks 1,266 nonzero 32-byte
  palette anchors from hash-verified LEV00--LEV15 Structure2 descriptors.
  European frame-1 and frame-7 captures have no native or word-swapped exact
  CRAM match. Palette-bank ownership and layer composition remain blocked.

- **NEXUS-SCSP-READ-CORRIDOR:** The external European gameplay producer now
  supports bounded sound-CPU SCSP-read tracing with optional 68K-PC filtering.
  A 100-record authenticated window reached `SDDRVS.TSK` setup/shared-RAM
  reads, but contained no reads in the `0x100400..0x100401` SCSP mailbox
  range and no `0x3224`-filtered row. This is a negative observation, not a
  SLEV/SAL playback proof; retain semantic admission and host playback as
  blocked until a trace joins a real event command to the driver consumer.

- **NEXUS-SATURN-ACTIVE-VDP1-WITNESS-JOIN:** An external European Mednafen
  2026-08-08 whole-file follow-up: the same audit scans 126 SHA-256-verified
  extracted retail files, rejecting five variant identities instead of using
  filename-only evidence. Neither frame-1 nor frame-7 has a native or
  word-swapped whole-file owner; relocated/decompressed runtime ownership
  remains open.

- **NEXUS-SATURN-VDP1-SECOND-SOURCE-SPAN:** A second authenticated European
  START+A capture (raw SHA-256 `d648bd88…`) has active VDP1 draws in all eight
  frames. Its later type-2 source span is 16bpp, 33280 bytes at `0x63e00`, with
  source hashes `5cca9793…` and `58afb9c9…`; neither span has an exact match in
  the local Nexus corpus. VDP2 remains byte-stable. Keep this as negative
  source evidence and recover the `0x06013098` writer's relocated/decompressed
  owner before admitting DGN/MNS/ITEM/HUD pixels.

- **NEXUS-SATURN-VDP1-PC-SOURCE-JOIN:** The external producer now supports an
  operator-only VDP1 VRAM-write trace with SH-2 PC values. The first bounded
  source probe reached the live VRAM window but was dominated by colour/
  framebuffer fills at `0x06026260`/`0x06026270`; it did not identify the
  captured type-2 DGN texture source. Keep the negative receipt and semantic
  admission blocked; repeat only with a source-span-specific execution window.

- **NEXUS-SATURN-VDP1-SOURCE-WRITER-CORRIDOR:** A source-span-specific
  European startup run observed 4,601 writes from runtime PC `0x06013098`
  into the beginning of the captured type-2 source window
  (`0x47c00..0x49ffe`). The same trace separately records the known
  framebuffer/colour writers at `0x06026260`, `0x06026270`, `0x060262c4` and
  `0x060262d4`. This is the first positive writer corridor for that frame,
  but the runtime PC has not yet been joined to a verified DM.BIN/TM.BIN
  source span or a named `DMV`/TITLE/MENU consumer. Keep DGN/PRS3/menu/HUD
  promotion blocked.

- **DM1-MIRROR-RESURRECT-CHEST-CLOSE-ORDER-SYNTHETIC-AUDIT:** This
  C040/chest/queue fixture fabricates party, Thing and slot state. It loads
  no original DM1 data and has no M11/runtime caller; retain it only as
  explicit ReDMCSB coverage until a source-bound interaction owner is
  recovered.

- **DM1-MIRROR-C545-DROP-PANEL-LIVE-SYNTHETIC-AUDIT:** This C545/C040
  fixture fabricates party, chest, panel and Thing state. It loads no
  original DM1 data and has no M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-SCROLL-PICKUP-NONLEADER-SYNTHETIC-AUDIT:** This C038/C040
  fixture fabricates party, panel and chest state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-CHEST-C061-ROTATION-SYNTHETIC-AUDIT:** This C061/C540 queue fixture
  fabricates chest, hand, load and rotation state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound command-queue owner is recovered.

- **DM1-CHEST-MID-CLOSE-HAND-SWAP-SYNTHETIC-AUDIT:** This manual chest-close
  fixture fabricates linked items, slots and hand state. It loads no original
  DM1 data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound chest owner is recovered.

- **DM1-AUTO-CHEST-ACTION-HAND-SWAP-SYNTHETIC-AUDIT:** This chest close/swap
  fixture fabricates linked items, slots and hand state. It loads no original
  DM1 data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound chest owner is recovered.

- **DM1-CHEST-PICKUP-PENDING-RESURRECT-SYNTHETIC-AUDIT:** This asset-free
  chest/pending-resurrect fixture fabricates chest, candidate and queue
  state. It loads no original DM1 data and has no M11/runtime caller; retain
  it only as explicit ReDMCSB coverage until a source-bound chest owner is
  recovered.

- **DM1-INVENTORY-HAND-BELT-QUIVER-SWAP-SYNTHETIC-AUDIT:** This slot-mask
  probe fabricates item types, weights and inventory state. It loads no
  original DM1 data and has no M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound inventory interaction owner is
  recovered.

- **DM1-CHEST-OPEN-STACK-SPLIT-PRESS-EYE-SYNTHETIC-AUDIT:** This chest
  close/pickup fixture fabricates linked items, stack and hand state. It
  loads no original DM1 data and has no M11/runtime caller; retain it only as
  explicit ReDMCSB coverage until a source-bound chest owner is recovered.

- **DM1-MIRROR-CLOSE-C045-PENDING-SYNTHETIC-AUDIT:** This C045/C160 queue
  fixture fabricates candidate-chain, hand and panel state. It loads no
  original DM1 data and has no M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound command-queue owner is recovered.

- **DM1-MIRROR-KEYROT-COMBO-INVCLICK-SYNTHETIC-AUDIT:** This F0361/F0380 race
  fixture fabricates candidate, queue and redraw state. It loads no original
  DM1 data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound command-queue owner is recovered.

- **DM1-MIRROR-OPEN-THEN-RESELECT-SYNTHETIC-AUDIT:** This C159/C040 fixture
  fabricates champion, hand and slot-fingerprint state. It loads no original
  DM1 data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound candidate-panel owner is recovered.

- **DM1-STATUS-HAND-CLOSED-CHEST-SYNTHETIC-AUDIT:** This closed-chest status
  hand probe fabricates party, chest and hand state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound inventory interaction owner is recovered.

- **DM1-INVENTORY-HAND-BELT-ROUND-TRIP-SYNTHETIC-AUDIT:** This hand/belt swap
  probe fabricates item types, weights and slot state. It loads no original
  DM1 data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound inventory interaction owner is recovered.

- **DM1-CHAMPION-PANEL-INVENTORY-WALK-SYNTHETIC-AUDIT:** This F0296
  inventory/chest icon-walk fixture fabricates champion, slot and icon state.
  It loads no original DM1 data and has no M11/runtime caller; retain it only
  as explicit ReDMCSB coverage until a source-bound redraw owner is recovered.

- **DM1-CHEST-OPEN-MIRROR-ROTATION-SYNTHETIC-AUDIT:** This C540 wheel-swap
  fixture fabricates chest, candidate and command-queue state. It loads no
  original DM1 data and has no M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-KEYBOARD-BROWSE-SYNTHETIC-AUDIT:** This keyboard-browse fixture
  fabricates roster and C127 portrait-token state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-THOUGHT-PROJECT-SYNTHETIC-AUDIT:** This C157/C158 overlay
  fixture fabricates candidate and text state. It loads no original DM1 data
  and has no M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound interaction owner is recovered.

- **DM1-CHAMPION-DEAD-HAND-REFRESH-SYNTHETIC-AUDIT:** This F0296/F0295/F0386
  fixture fabricates party, icon and slotbox state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-CHEST-SCROLL-RESURRECT-CONFIRM-SYNTHETIC-AUDIT:** This C040/C545
  confirmation fixture fabricates chest, party and command-queue state. It
  loads no original DM1 data and has no M11/runtime caller; retain it only as
  explicit ReDMCSB coverage until a source-bound interaction owner is
  recovered.

- **DM1-MIRROR-PICKUP-RIGHT-CLICK-SYNTHETIC-AUDIT:** This C159-row fixture
  fabricates party, panel and hand state. It loads no original DM1 data and
  has no M11/runtime caller; retain it only as explicit ReDMCSB coverage until
  a source-bound interaction owner is recovered.

- **DM1-MIRROR-LEFT-CLICK-ROTATION-SYNTHETIC-AUDIT:** This C040 view-rotation
  fixture fabricates candidate, hand and panel state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-INVENTORY-CLICK-ROTATION-SYNTHETIC-AUDIT:** This C156/C157
  dispatch-table fixture fabricates click and rotation state. It loads no
  original DM1 data and has no M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-RESURRECT-REINCARNATE-SKILLS-SYNTHETIC-AUDIT:** This C160/C161
  fixture fabricates party, champion and slot state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-KEYBOARD-ROTATION-SYNTHETIC-AUDIT:** This command-queue fixture
  fabricates candidate, roster and rune-buffer state. It loads no original
  DM1 data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-CLOSE-BUTTON-SYNTHETIC-AUDIT:** This C040 panel-chrome fixture
  fabricates party, portrait and panel state. It loads no original DM1 data
  and has no M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound interaction owner is recovered.

- **DM1-MIRROR-C040-ROTATION-SAVELOAD-SYNTHETIC-AUDIT:** This F0433/F0435
  round-trip fixture fabricates save, party and panel state. It loads no
  original DM1 data and has no M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound interaction owner is recovered.

- **DM1-CHEST-MULTI-CHAMPION-CLOSE-SYNTHETIC-AUDIT:** This chest-close fixture
  fabricates champions, Things and weights. It loads no original DM1 data and
  has no M11/runtime caller; retain it only as explicit ReDMCSB coverage.

- **DM1-MIRROR-REOPEN-SAVELOAD-SYNTHETIC-AUDIT:** This C040 save/load model
  fabricates save parts, party and UI state. It has no original DM1 data input
  or M11/runtime caller; retain it only as explicit ReDMCSB coverage.

- **DM1-MIRROR-CANCEL-ROTATION-SYNTHETIC-AUDIT:** This C040/C162 rotation
  fixture fabricates party, panel and hand state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-HUD-EXIT-SYNTHETIC-AUDIT:** This C040 inventory-exit HUD model
  fabricates party, panel, chest and overlay state. It has no original DM1
  data input or M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-CHEST-OPEN-PENDING-SYNTHETIC-AUDIT:** This C040/F0333/F0282
  fixture fabricates candidate, chest and slot state. It loads no original
  DM1 data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-CLICK-CANCEL-ROTATION-SYNTHETIC-AUDIT:** This C040 race model
  fabricates party, candidate-chain, panel and slot state. It has no original
  DM1 data input or M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-PENDING-HAND-QUEUE-SYNTHETIC-AUDIT:** This C040 chest/hand-
  queue fixture fabricates party, chest and Thing state. It loads no original
  DM1 data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-C004-C006-C040-SYNTHETIC-AUDIT:** This C004..C006/C040 fixture
  fabricates party, panel and chest state. It loads no original DM1 data and
  has no M11/runtime caller; retain it only as explicit ReDMCSB coverage until
  a source-bound movement owner is recovered.

- **DM1-MIRROR-TELEPORTER-SURVIVAL-SYNTHETIC-AUDIT:** This teleporter fixture
  fabricates a dungeon, C040 panel, party and chest-slot state. It has no
  original DM1 data input or M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-NO-PENDING-RESURRECT-SYNTHETIC-AUDIT:** This no-op C040/G0299
  fixture has no original DM1 data input or M11/runtime caller; retain it only
  as explicit ReDMCSB coverage until a source-bound interaction owner is
  recovered.

- **DM1-CHAMPION-HAND-SLOT-PRIORITY-SYNTHETIC-AUDIT:** This source-only
  hand-slot priority trace fabricates party, slot and Thing state. It has no
  original DM1 data input or M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound interaction owner is recovered.

- **DM1-D1C-F0115-SYNTHETIC-AUDIT:** This D1C door-frame/F0115 contract uses
  fixed geometry and local probe pixels. It has no original material input or
  M11/runtime caller; retain it only as explicit ReDMCSB coverage.

- **DM1-MIRROR-C545-C160-SYNTHETIC-AUDIT:** This C545/C160 fixture fabricates
  party, panel and pixel state. It loads no original DM1 data and has no
  M11/runtime caller; retain it only as explicit ReDMCSB coverage until a
  source-bound interaction owner is recovered.

- **DM1-MIRROR-C159-C160-SYNTHETIC-AUDIT:** This C159/C160 fixture fabricates
  party, panel and hand state. It loads no original DM1 data and has no
  M11/runtime caller; retain it only as explicit ReDMCSB coverage until a
  source-bound interaction owner is recovered.

- **DM1-MIRROR-C061-C028-SYNTHETIC-AUDIT:** This C061/C028 fixture fabricates
  slot, party and panel state. It loads no original DM1 data and has no
  M11/runtime caller; retain it only as explicit ReDMCSB coverage until a
  source-bound interaction owner is recovered.

- **DM1-MIRROR-C045-C160-ROTATION-SYNTHETIC-AUDIT:** This C045/C160 rotation
  fixture fabricates party, panel and item state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-F1506-F1525-METADATA-AUDIT:** This SWITCH/VDI ownership inventory has
  no M11 caller or authenticated PC34 data input. Keep it as explicit
  ReDMCSB coverage until a source-bound PC34 owner is recovered.

- **DM1-MIRROR-C045-DEAD-OWNER-SYNTHETIC-AUDIT:** This asset-free C045/C040
  fixture fabricates party, panel and item state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-C100-C040-SYNTHETIC-AUDIT:** This C100/C040 fixture fabricates
  party, panel, G0299 candidate and G0514 caster state. It loads no original
  DM1 data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound command owner is recovered.

- **DM1-D1C-WALL-SYNTHETIC-AUDIT:** This D1C wall pixel model uses
  caller-provided local buffers and fixed route metadata. It has no original
  DM1 data input or M11/runtime caller; retain it only as explicit ReDMCSB
  coverage while the source-bound D1C renderer owns wall material.

- **DM1-MIRROR-C040-C545-REDRAW-SYNTHETIC-AUDIT:** This asset-free C040/C545
  fixture fabricates party, chest and candidate state. It loads no original
  DM1 data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound panel owner is recovered.

- **DM1-MIRROR-CHEST-CLOSE-PENDING-SYNTHETIC-AUDIT:** This C011/C038/C162
  fixture fabricates party, chest, panel and Thing state. It has no original
  DM1 data input or M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound command owner is recovered.

- **DM1-MIRROR-C040-PICKUP-ROTATE-SYNTHETIC-AUDIT:** This C040/chest/rotation
  fixture fabricates party, panel and Thing state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-C007-C011-C040-SYNTHETIC-AUDIT:** This C007..C011/C040 gate
  fabricates party, panel and G0299 candidate state. It loads no original DM1
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound command owner is recovered.

- **DM1-F0099-ROW-FLIP-SYNTHETIC-AUDIT:** This F0099 row-flip fixture
  fabricates caller buffers and fixed dimensions. It has no original DM1 data
  input or M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound flip owner is recovered.

- **DM1-MIRROR-C546-C040-SYNTHETIC-AUDIT:** This C546/C040 fixture fabricates
  panel, chest and Thing state. It loads no original DM1 data and has no
  M11/runtime caller; retain it only as explicit ReDMCSB coverage until a
  source-bound panel owner is recovered.

- **DM1-MIRROR-C111-C040-SYNTHETIC-AUDIT:** This C111/C040 gate fabricates
  party, panel and G0299 candidate state. It loads no original DM1 data and
  has no M11/runtime caller; retain it only as explicit ReDMCSB coverage until
  a source-bound command owner is recovered.

- **DM1-MIRROR-C040-PANEL-EXIT-SYNTHETIC-AUDIT:** This C040 close/reopen
  fixture fabricates panel and candidate state. It loads no original DM1
  assets and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound panel owner is recovered.

- **DM1-MIRROR-C040-NONLEADER-SCROLL-SYNTHETIC-AUDIT:** This seeded
  C040/C537/C162 fixture fabricates party, chest and scroll state. It has no
  original DM1 data input or M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound interaction owner is recovered.

- **NEXUS-SATURN-RUNTIME-CAPTURE-PRODUCER:** The external-disk Mednafen 1.32.1
  producer is now compiled with Saturn support and has emitted a validated
  two-frame raw VDP1/VDP2 witness. Region provenance is now explicit: the
  English ISO is `SGAREA U`, the merged English image is `SGAREA J`, and only
  the French ISO is `SGAREA E` for the supplied European E-BIOS. The raw
  validator proves transport/layout only. Do not admit it as PRS3,
  SLEV/SAL/SDDRVS, HUD, or viewport semantics until those consumers have
  authenticated runtime observations and source-owned bindings. The producer
  now supports an external skip/count window and captures at the VDP2 frame
  assembly hook immediately before `VDP2REND_EndFrame()`; E-BIOS +
  French-media windows show the real TrueMotion and changing startup scenes in
  VDP1 framebuffer data. This is still a raw witness, not a menu/HUD/viewport
  import or a PRS3 decoder proof.

- **NEXUS-SATURN-RUNTIME-CAPTURE-REGION-IDENTITY:** The raw witness analyzer
  now reports SHA-256 identities for every VDP1/VDP2 payload region and can
  require observed changes between adjacent frames. The real French/E-region
  four-frame startup witness changes VDP1 FB0, VDP1 VRAM and the draw-buffer
  selector, while VDP2 registers/VRAM/CRAM remain unchanged in that sample.
  This is stronger transport evidence for the authentic startup animation,
  but it still does not identify a menu, HUD, viewport, CLUT, or source-owned
  consumer; semantic admission remains blocked.

- **NEXUS-SATURN-VDP1-STATE-CAPTURE:** The external producer now emits VDP1
  V2 state lines alongside raw VRAM/framebuffers. A real two-frame E-region
  witness records `PTMR=02`, `EDSR=03`, `LOPR=0008`, `COPR=000008`,
  `RET=ffffffff`, and framebuffer selection `1 -> 0`. This binds observed
  VDP1 state to the emulator frame, but does not identify the active command
  list's source record, CLUT, destination or menu/HUD/viewport owner.

- **NEXUS-SATURN-VDP1-COMMAND-WINDOW:** The real two-frame E-region witness
  now has a bounded command-window inspection: `COPR=8` maps to VDP1 VRAM
  offset `0x40`, whose raw record is END; the preceding records at `0x00` and
  `0x20` have control types `0x09` and `0x0A`. This is a genuine command-state
  receipt, not a texture/menu draw proof. The source record, CLUT, destination
  and asset join remain open.

- **NEXUS-SATURN-VDP1-DRAW-SOURCE-JOIN:** A later authentic E-region startup
  frame now exposes one bounded distorted-sprite command at `0x0040`
  (`PMOD=0x0028`, `SRCa=0xc7c0`, `SIZE=0x28b4`) and END at `0x0060`. The
  analyzer computes the command's source VRAM span and SHA-256. It still does
  not identify whether the bytes belong to TITLE.CG, an intro decoder, or
  another retail source; that source join and CLUT/placement ownership remain
  required before any production presentation route changes.
  The observed source span hashes to `0a87c97db9dcaf9e74df11cb85b35084edc0e37daa74e6012ce1fc131a2d5575`;
  it has no exact-file or first-32-byte match in the local `TM.BIN`
  (`d87485fe…`), `TITLE.CG` (`fda4da4c…`), `TITLE.BIN`
  (`a634e8da…`) or `STABG.BIN` (`7b8e44ff…`) corpus. This is a negative
  join receipt, not evidence that the bytes are synthetic.

- **NEXUS-TM-BIN-VDP-OWNER-DISASSEMBLY:** The authenticated retail `TM.BIN`
  (`160044` bytes, SHA-256 `d87485fe…`) contains SH-2 PC-relative literal
  corridors to VDP1 register values `0x25d00000`, `0x25d00002`, `0x25d00006`,
  `0x25d00008`, `0x25d0000a` and `0x25d00010`, plus VDP2 register literals.
  `scripts/analyze_nexus_tm_bin_vdp_owner.py` verifies this against the real
  file. The same scan now accepts the hash-verified `DM.BIN` as well; its
  VDP2 literal set is `0x25f00000`, `0x25f00006`, `0x25f00018`, `0x25f000a0`.
  This improves static owner evidence only; execution, command-source
  identity, CLUT and asset join remain capture-gated.

- **NEXUS-SATURN-STARTUP-INPUT-CAPTURE:** The external Saturn producer now
  supports a bounded, active-low START pulse through Mednafen SMPC at an
  operator-selected emulated frame, with an explicit mask for START (`0x10`),
  A (`0x20`) or both (`0x30`). Initial E-region tests at frame 1000, 4500,
  6500 and 8000 did not yet reach a menu; the captured framebuffer remains
  authentic intro imagery. A further E-region run with START+A at frame 100,
  60 frames held, and capture beginning at runtime frame 6000 produced eight
  active VDP1 frames (`ce800662…` raw SHA-256); VDP2 registers/VRAM/CRAM stayed
  unchanged and the framebuffer remained intro/dungeon imagery. This is a
  negative input-timing receipt, not a menu observation. Do not infer the
  correct skip control or admit menu state until a post-input screen transition
  and source-owned menu consumer are both observed.

- **NEXUS-MENU-CAPTURE-GATE:** TITLE.CG timing must remain on the title screen
  until the real MENU.BPK capture route is joined. A source-owned runtime
  capture is still required before menu, HUD, and viewport composition can be
  enabled.

- **NEXUS-MENU-PRS3-CONSUMER:** The real retail `MENU.BPK` corpus now passes a
  regression that decodes all 162 PRS3 surfaces and checks each output against
  its declared `width × height`. This closes the byte-decoder gap only. The
  decoded pixels remain evidence-only until Saturn VDP1/VDP2 capture identifies
  the original CLUT, draw-command and layer-composition consumer.

- **DM1-D1R2-WALL-SYNTHETIC-AUDIT:** This D1R2-wall fixture uses local
  320×200 probe buffers and fixed route metadata. It has no original DM1 data
  input or M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound D1R wall owner is recovered.

- **DM1-F0292-NAME-BOX-CLIP-SYNTHETIC-AUDIT:** This F0292 name/title clip
  fixture fabricates champion, name and title inputs. It has no original DM1
  data input or M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound panel owner is recovered.

- **DM1-F0296-INVENTORY-VIEWPORT-WALK-SYNTHETIC-AUDIT:** This F0296
  inventory/chest sub-walk fixture fabricates party, slot and icon state. It
  has no original DM1 data input or M11/runtime caller; retain it only as
  explicit ReDMCSB coverage until a source-bound panel owner is recovered.

- **DM1-MIRROR-C160-ROTATION-SYNTHETIC-AUDIT:** This C040/C160 rotation-close
  fixture fabricates party, chest and panel state. It has no original DM1 data
  input or M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound interaction owner is recovered.

- **DM1-MIRROR-RESURRECT-INTERRUPT-SYNTHETIC-AUDIT:** This C040/C160/C038
  fixture fabricates party, hand and chest state. It has no original DM1 data
  input or M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound interaction owner is recovered.

- **DM1-MIRROR-INVENTORY-PORTRAIT-SYNTHETIC-AUDIT:** This C175 portrait-click
  guard fabricates party, hand and candidate state. It has no original DM1
  data input or M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound panel owner is recovered.

- **DM1-MIRROR-CANCEL-REOPEN-SYNTHETIC-AUDIT:** This same-tick C040/C162/C127
  model fabricates party, panel and sensor state. It has no original DM1 data
  input or M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound interaction owner is recovered.

- **DM1-MIRROR-RESURRECT-ROTATION-SYNTHETIC-AUDIT:** This C040/C160 rotation
  fixture fabricates party, C30 and G0425 state. It has no original DM1 data
  input or M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound interaction owner is recovered.

- **DM1-MIRROR-RESHUFFLE-SYNTHETIC-AUDIT:** This C040 reshuffle fixture
  fabricates party, chest and leader-hand state. It has no original DM1 data
  input or M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound interaction owner is recovered.

- **DM1-MIRROR-C040-OWNER-SWAP-SYNTHETIC-AUDIT:** This C040 chrome fixture
  fabricates party, portrait and status-icon ownership state. It has no
  original DM1 data input or M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound panel owner is recovered.

- **DM1-MIRROR-C045-NONCANDIDATE-SYNTHETIC-AUDIT:** This C045 close-after-
  transition fixture fabricates chest, leader-hand and visible-slot state. It
  has no original DM1 data input or M11/runtime caller; retain it only as
  explicit ReDMCSB coverage until a source-bound panel owner is recovered.

- **DM1-MIRROR-CHEST-CLOSE-PICKUP-SYNTHETIC-AUDIT:** This C040/C537/C162
  scenario fixture fabricates party, chest and Thing state. It has no original
  DM1 data input or M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-MIRROR-THOUGHT-CANCEL-SYNTHETIC-AUDIT:** This C040/C537/C162
  thought-project scenario fabricates champion, chest and scroll state. It
  has no original DM1 data input or M11/runtime caller; retain it only as
  explicit ReDMCSB coverage until a source-bound interaction owner is
  recovered.

- **DM1-MIRROR-DOUBLE-OPEN-CLOSE-SYNTHETIC-AUDIT:** This C040/C537 lifecycle
  fixture fabricates champion, hand and chest-slot state. It has no original
  DM1 data input or M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-CHEST-ANOTHER-OPEN-SYNTHETIC-AUDIT:** This F0333/F0334 scenario
  model fabricates Thing ordinals, container chains and leader-hand state in
  a local Next array. It reads no original game data and has no M11/runtime
  caller; retain it only as explicit ReDMCSB coverage until a source-bound
  chest owner is recovered.

- **DM1-D1L-D1R-STAIRS-PIT-SYNTHETIC-AUDIT:** This D1 side stairs/pit
  dispatch model uses fixed graphics slots and zones. It reads no original
  game data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound owner is recovered.

- **DM1-MIRROR-C040-SAVE-GATE-SYNTHETIC-AUDIT:** This C140/C040 save gate
  fabricates party, panel and candidate state. It has no original DM1 data
  input or M11/runtime caller; retain it only as explicit ReDMCSB coverage
  until a source-bound command owner is recovered.

- **DM1-D0L-D0R-F0111-SYNTHETIC-AUDIT:** This D0 side-door composition uses
  fixed zones and blit rectangles. It reads no original game data and has no
  M11/runtime caller; retain it only as explicit ReDMCSB coverage until a
  source-bound owner is recovered.

- **DM1-MIRROR-FIRST-FOCUS-SYNTHETIC-AUDIT:** This first-C127/C040 focus
  fixture fabricates party, panel and input-focus state. It has no original
  DM1 data input or M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-CHEST-NINTH-ITEM-SYNTHETIC-AUDIT:** This hidden-tail chest model
  fabricates item types, weights and a sentinel. It reads no original game
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound chest owner is recovered.

- **DM1-D1L2-D1R2-F0111-SYNTHETIC-AUDIT:** This partly-open D1 side-door
  model uses fixed zones and framebuffer pixels. It reads no original game
  data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound owner is recovered.

- **DM1-D2L2-D2R2-F0108-FLOOR-CEILING-SYNTHETIC-AUDIT:** This D2 side
  floor/ceiling contract uses fixed zones and framebuffer pixels. It reads no
  original game data and has no M11/runtime caller; retain it only as explicit
  ReDMCSB coverage until a source-bound owner is recovered.

- **DM1-MIRROR-CWRPIP-SYNTHETIC-AUDIT:** This C040/C537 close-while-pending
  scenario fabricates champion, hand and chest-chain state. It has no original
  DM1 data input or M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound interaction owner is recovered.

- **DM1-D3L-D3R-F0108-FLOOR-CEILING-SYNTHETIC-AUDIT:** This D3 side
  floor/ceiling contract uses fixed zones, orders and framebuffer pixels. It
  reads no original game data and has no M11/runtime caller; retain it only as
  explicit ReDMCSB coverage until a source-bound owner is recovered.

- **DM1-D3L2-D3R2-F0108-FLOOR-CEILING-SYNTHETIC-AUDIT:** This post-D3C
  floor/ceiling contract uses fixed zones, ordinals and probe pixels. It reads
  no original game data and has no M11/runtime caller; retain it only as
  explicit ReDMCSB coverage until a source-bound owner is recovered.

- **DM1-MIRROR-FULL-CHAIN-SYNTHETIC-AUDIT:** This deterministic mirror-candidate
  chain synthesizes party and hand state. It has no original DM1 data input or
  M11/runtime caller; retain it only as an explicit ReDMCSB regression until a
  source-bound interaction owner is recovered.

- **DM1-D0C-F0098-SYNTHETIC-AUDIT:** This D0C floor/ceiling row-ownership
  model uses fixed viewport rows and C10 probe pixels. It reads no original
  game data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound F0098 owner is recovered.

- **DM1-CHEST-ROUND-TRIP-SYNTHETIC-AUDIT:** This C537/C538 hand-swap model
  fabricates visible chest slots, item kinds and weights. It reads no original
  game data and has no M11/runtime caller; retain it only as explicit ReDMCSB
  coverage until a source-bound chest transaction owner is recovered.

- **DM1-MIRROR-C045-FOOD-WATER-SYNTHETIC-AUDIT:** This asset-free C045/chest
  scenario fixture reads no original DM1 data and has no M11/runtime caller.
  Retain it only as an explicit ReDMCSB regression until a source-bound C045
  interaction consumer is recovered.

- **DM1-C146-WAKE-UP-SYNTHETIC-AUDIT:** This C040/C146 wake-up regression
  simulates fixed panel and rest state with no original game-data input. It
  has no M11/runtime caller; retain it as explicit ReDMCSB coverage until a
  source-bound wake-up/panel owner is recovered.

- **DM1-D2C-F0111-SYNTHETIC-AUDIT:** This contract-only D2C F0111 trace has
  no game-data input or M11/runtime caller. Retain it as explicit ReDMCSB
  coverage until a source-bound D2C door consumer is recovered.

- **DM1-MIRROR-OCCUPIED-HAND-SYNTHETIC-AUDIT:** This C040 occupied-hand
  fixture synthesizes panel and candidate state. It has no original DM1 data
  input or M11/runtime caller; retain it only as an explicit ReDMCSB
  regression until a source-bound interaction consumer is recovered.

- **DM1-D2L-D2R-F0111-SYNTHETIC-AUDIT:** This asset-free F0111 model
  simulates D2-side door composition in a local framebuffer without original
  material. It has no M11/runtime caller; retain it as explicit ReDMCSB
  coverage while the independent D2 side-door route owns source material.

- **DM1-D3C-F0111-SYNTHETIC-AUDIT:** This fixed D3C door-front source-order
  and C10 model has no original-material input or M11/runtime caller. Retain
  it as explicit ReDMCSB regression until a source-bound D3C door consumer is
  recovered.

- **DM1-CHAMPION-STATUS-RECOMPUTE-SYNTHETIC-AUDIT:** This visible-delta
  model uses synthetic panel state, reads no original DM1 data and has no
  M11/runtime caller. Retain it only as an explicit ReDMCSB regression until
  a source-bound status consumer is recovered.

- **DM1-D3C-F0108-SYNTHETIC-AUDIT:** This contract-only D3C model uses fixed
  floor, ceiling, ornament and thing probe pixels without original material.
  It has no M11/runtime caller; retain it as explicit ReDMCSB regression until
  a source-bound D3C floor-ornament consumer is recovered.

- **DM1-D1L-D1R-F0111-SYNTHETIC-AUDIT:** This asset-free F0111 model records
  D1-side door control and synthetic blits without original game data. It has
  no M11/runtime caller; retain it as explicit ReDMCSB coverage while the
  independent D1-side door receipt owns source material.

- **DM1-CHAMPION-PORTRAIT-SYNTHETIC-AUDIT:** This portrait-route model uses
  synthetic panel state and no bitmap sampling. It reads no original DM1 data
  and has no M11/runtime caller; retain it only as an explicit ReDMCSB
  regression until a source-bound portrait consumer is recovered.

- **DM1-D1C-F0111-SYNTHETIC-AUDIT:** This asset-free D1C F0111 geometry and
  transparency model uses synthetic blits and no original data. It has no
  M11/runtime caller; retain it as an explicit ReDMCSB regression while the
  independent D1C door receipt remains the source-material owner.

- **DM1-D1C-F0107-SYNTHETIC-AUDIT:** This asset-free F0107 model fixes
  D1C wall-ornament routing and local C10 probes. It has no original DM1 data
  input or M11/runtime caller; retain it as an explicit ReDMCSB regression
  while the separate source-bound inscription/wall-material paths own pixels.

- **DM1-MIRROR-RESELECT-INVENTORY-SYNTHETIC-AUDIT:** This C040/C038 fixture
  synthesizes champion and inventory state, reads no original DM1 data and has
  no M11/runtime caller. Retain it only as an explicit ReDMCSB regression
  until a source-bound interaction consumer is recovered.

- **DM1-D2L-D2R-F0108-SYNTHETIC-AUDIT:** This contract-only D2 side model
  fixes zone and ordered-cell metadata without comparing original bitmap data.
  It has no M11/runtime caller; retain it as an explicit ReDMCSB regression
  until a source-bound D2 side floor-ornament consumer is recovered.

- **DM1-D2C-F0108-SYNTHETIC-AUDIT:** This contract-only D2C F0108 model
  fixes zones, cell ordering and a local framebuffer probe. It has no original
  DM1 data input or M11/runtime caller; retain it as an explicit ReDMCSB
  regression until a source-bound D2C floor-ornament consumer is recovered.

- **DM1-D3L2-D3R2-F0108-OCCLUSION-SYNTHETIC-AUDIT:** This asset-free F0676/
  F0677 ordering model fixes side-pair zones, cell orders and C10 probes. It
  has no original DM1 data input or M11/runtime caller; retain it as an
  explicit ReDMCSB regression while the independent source-bound side-pair
  renderer owns live pixels.

- **DM1-MIRROR-SCROLL-ROTATION-SYNTHETIC-AUDIT:** This deterministic C040,
  chest and leader-rotation scenario fixture reads no original DM1 data and
  has no M11/runtime caller. Retain it only as an explicit ReDMCSB regression
  until a source-bound interaction consumer is recovered.

- **DM1-D3L-D3R-F0108-SYNTHETIC-AUDIT:** This asset-free D3 side-lane F0108
  model fixes zones, cell orders and C10 probes. It has no original DM1 data
  input or M11/runtime caller; retain it as an explicit ReDMCSB regression
  while the independent source-bound D3 renderer owns live pixels.

- **DM1-D3L2-D3R2-F0108-COMPOSITION-SYNTHETIC-AUDIT:** This contract model
  fixes D3 side-pair routes, zones and local C10 pixel probes. It has no
  original DM1 data input or M11/runtime caller; retain it as an explicit
  ReDMCSB regression while the independent source-bound D3 route owns live
  GRAPHICS.DAT pixels.

- **DM1-MIRROR-PARTY-DIRECTION-SYNTHETIC-AUDIT:** This C040/G0299 harness
  uses a synthetic portrait token and has no original DM1 data input or
  M11/runtime caller. Retain it only as an explicit ReDMCSB regression until
  a source-bound mirror-direction consumer is recovered.

- **DM1-D1C-F0108-SYNTHETIC-AUDIT:** This asset-free F0108 occlusion model
  fixes D1C zones, cell orders and C10 pixel probes. It has no original DM1
  data input or M11/runtime caller; retain it as an explicit ReDMCSB
  regression until a source-bound D1C floor-ornament consumer is recovered.

- **DM1-CHAMPION-INVENTORY-TAIL-SYNTHETIC-AUDIT:** This source-locked
  inventory-tail model uses synthetic slot and dirty-bit rows. It reads no
  original DM1 data and has no M11/runtime caller; retain it only as an
  explicit ReDMCSB regression until a source-bound inventory-tail consumer is
  recovered.

- **DM1-D3C-BACK-WALL-ITEM-SYNTHETIC-AUDIT:** This asset-free F0115 model
  uses synthetic framebuffer writes, cells and zones. It has no original DM1
  data input or M11/runtime caller; retain it as an explicit ReDMCSB
  regression until a source-bound D3C back-wall item consumer is recovered.

- **DM1-CHAMPION-DISABLED-ICON-SYNTHETIC-AUDIT:** The disabled-icon model
  synthesizes champion rows and G0491 state. It reads no original DM1 data;
  retain it as an explicit ReDMCSB regression. M11 owns only the separate,
  source-cited ACTIDRAW F0386 global hatch predicate; it must not relink this
  synthetic champion-row model until a source-bound action-icon consumer is
  recovered.

- **DM1-D2C-F0115-FRONT-REAR-SYNTHETIC-AUDIT:** This door-pass helper
  hard-codes cell orders, zones and synthetic pixel composition. It reads no
  original DM1 data and has no M11/runtime caller; retain it only as an
  explicit ReDMCSB regression until a source-bound D2C door-pass consumer is
  recovered.

- **DM1-MINIMAP-SYNTHETIC-BOUNDARY:** The corner minimap is now blocked for
  authenticated PC34 DM1 source sessions because it is a host-drawn
  diagnostic surface, not an original game display. Keep it available only
  to diagnostic worlds unless an authenticated source minimap owner is
  recovered.

- **DM1-LEGACY-DUNGEON-BRIDGE-ORNAMENT-OWNER:** The legacy
  `firestaff_dungeon_state` bridge now derives F0170/F0171 random wall/floor
  ordinals from the authenticated PC34 DUNGEON.DAT seed, map dimensions,
  counts and raw square flags. It also reads F0510/F0511's compact
  Thing-list chain to let a source sensor's F0172 ornament ordinal replace
  the random value only on the viewed wall cell; unrelated sensor cells and
  non-sensor Things retain the calculated source ordinal. It still lacks an
  authenticated GRAPHICS.DAT consumer; recover that owner before claiming
  complete ornament parity. M11's source-owned viewport remains separate.

- **NEXUS-SDDRVS-68K-EVENT-HANDOFF:** The authenticated `SDDRVS.TSK` is now
  byte-bound as a 26,610-byte 68000 sound-CPU image. Its entry, command-nibble
  dispatch and PCM voice-register corridors are receipt-verified, replacing
  the old incorrect SH-2 description. The actual game event→MAP selector,
  SAL codec contract and native driver handoff still require an original
  Saturn execution trace; keep host playback blocked until those observations
  are joined.

- **NEXUS-SCSP-RUNTIME-PC-TRACE:** An external Mednafen sound trace now records
  a real European gameplay mailbox write: main SH-2 PC `0x06001652` writes
  value `0x02` to SCSP address `0x100400`. The 68K sound task then writes the
  same mailbox family from PCs `0x3224`, `0x3258`, `0x1090`, `0x2824`,
  `0x16c6`, `0x34aa`, `0x34cc`, `0x108e` and `0x1b2e`; with the authenticated
  load base `0x1000`, those PCs resolve inside the real `SDDRVS.TSK` hash.
  This proves a live SCSP/68K handoff corridor, not the SLEV event selector,
  SAL sample identity or playback ABI. Keep host audio blocked until those
  joins are captured.

- **NEXUS-SCSP-DRIVER-HANDLER-JOIN:** The observed nonzero 68K PC `0x3224` now
  binds to the authenticated `SDDRVS.TSK` handler at file offset `0x2220`
  (`SHA-256 68890ee4…`). The verified instruction window reads an incoming
  command byte, rejects values at/above `0x12`, updates driver state, shifts a
  channel index and writes the SCSP per-channel register family at offset
  `$17` from `a5=0x100000`. This is a source-owned command-to-driver corridor,
  not the SLEV event selector, MAP row or SAL sample; playback remains blocked.

- **NEXUS-SLEV-SAL-RUNTIME-CORRIDOR:** The new
  `analyze_nexus_slev_sal_runtime_corridor.py` receipt joins the authenticated
  European gameplay SCSP trace to all 16 real `SLEV##.BIN`, `SNDLEV##.MAP`,
  `SNDLEV##.SAL` pairs and `SDDRVS.TSK`. It applies the same DMWeb eight-byte
  MAP grammar as the C loader and records 154 terminated rows, four non-zero
  68K mailbox writes (all raw `0x02`) and five main-SH-2 mailbox records. Fifty-
  four MAP windows are beyond the extracted SAL file length as direct file
  intervals; the retail source treats those fields as an opaque driver-memory
  area, so this is recorded as provenance, not rejected or relocated data.
  Event-selector semantics, SAL codec and host playback remain explicitly
  unproven/blocked.

- **NEXUS-SLEV-SH2-STATIC-OWNER:** The 16 hash-authenticated `SLEV##.BIN`
  files now have a reproducible big-endian SH-2 static receipt. They share
  the `0x2fe6` entry word and contain 1,271 `RTS`, 2,220 `JSR`, 5,164
  immediate, 948 branch and 3,536 PC-relative-load observations across
  111,776 bytes. Eight PC-relative literal rows land in the observed
  `0x25000000/0x25010000/0x26000000/0x26010000` address corridors (SLEV02,
  03, 11 and 15); see `scripts/analyze_nexus_slev_sh2_owner.py` for exact
  file offsets. This is static owner evidence only: it does not prove the
  event selector, callback ABI, SLEV dispatch order or SDDRVS playback, so
  no runtime script route is enabled.

- **NEXUS-SAL-HOST-PCM-BOUNDARY:** The production SAL path remains a
  byte-level receipt path only. DMWeb directory/profile facts are retained
  from the real 16-bank corpus, but PCM format/rate/looping, voice ownership
  and MAP→event handoff still require a Saturn SCSP/SDDRVS execution capture;
  `nexus_sound_decode_sal()` therefore remains an explicit no-op. The public
  mixer is also silence-only, even when diagnostic voice fields are manually
  populated. Do not turn the profile's bounded tone entries into host PCM
  candidates until that capture is admitted.

- **NEXUS-NGLT-RESUME-BOUNDARY:** Native FNXS/NGLT light state is no longer
  rehydrated by the retail launcher resume path while Saturn action/state-write
  ownership is unproven. The NGLT ABI remains available only to explicit study
  probes; no native light model may stand in for a Saturn save section.

- **NEXUS-V2-PROCEDURAL-PRESENTATION-BOUNDARY:** The production Nexus library
  no longer links the procedural V2 lighting, smooth-movement or touch/
  controller implementations. Their M11 adapters are fail-closed no-ops
  because the supplied Saturn corpus has no authenticated VDP1/VDP2 lighting
  owner, presentation timing/camera consumer or `NEXUS.BIN` input consumer.
  The original algorithms remain only in explicit probes; do not restore them
  to production without real Saturn evidence.

- **NEXUS-MAGIC-PRODUCTION-BOUNDARY:** The DM.BIN spell-table study is now
  excluded from `firestaff_nexus`; production receives only a fail-closed ABI
  adapter because the Saturn spell dispatcher, caster/effect writes and
  SLEV/SFX publication remain uncaptured. Keep `nexus_v1_magic.c` test-only
  until those real consumers are bound.

- **NEXUS-COMBAT-PRODUCTION-BOUNDARY:** The DM.BIN/ReDMCSB-shaped combat,
  wound, XP and RNG study is now excluded from `firestaff_nexus`; production
  uses a state-preserving fail-closed adapter. Keep the formula implementation
  test-only until Saturn action/stat/writeback and SLEV/SFX consumers are
  captured.

- **NEXUS-EXPERIENCE-PRODUCTION-BOUNDARY:** The DM.BIN-shaped XP award,
  level-up and class-table implementation is now excluded from
  `firestaff_nexus`; production uses a state-preserving fail-closed adapter.
  Keep the byte-bound study test-only until the Saturn actor-death event,
  class-XP producer, champion writeback and HUD level-up consumer are
  captured.

- **NEXUS-STARTUP-SH2-VDP2-CAPTURE:** The authenticated European `DM.BIN`
  these static receipts do not authorize host drawing before admitting menu
  placement, FONT256 text, HUD composition or viewport pixels.
  executed VDP1 command-list/VRAM writes, the remaining VDP2 register/VRAM

- **DM1-D0C-F0108-SYNTHETIC-AUDIT:** The D0C/F0108 floor-ceiling-ornament
  helper remains a contract-only probe with fixed coordinate, zone and
  framebuffer values. Its ReDMCSB control-flow notes are useful, but no
  production caller or authenticated GRAPHICS.DAT material is bound to it.
  Keep it test-only until the real D0C consumer and source bitmap ownership
  are recovered from disassembly/capture; do not expose its synthetic values
  through M10.

- **DM1-D0L-D0R-F0107-SYNTHETIC-AUDIT:** The D0L/D0R F0107 wall-ornament
  helper is a contract-only synthetic-framebuffer audit with no authenticated
  bitmap input or production caller. Its ReDMCSB dispatch notes remain useful
  for parity work, but the real D0L/D0R GRAPHICS.DAT consumer and pixel
  ownership must be recovered before any runtime rendering claim.

- **DM1-PENDING-WOUNDS-SYNTHETIC-AUDIT:** The pending-wounds tick helper is a
  contract-only synthetic state machine. It mirrors ReDMCSB F0320/F0321 but
  does not consume live champion records, original saves, or authenticated
  DM1 data. Keep it test-only: the live M10 F0321 staging and F0320 consume
  route already owns real `GameWorld_Compat.pendingChampionCombat` state and
  source-backed HUD handoff. Do not introduce a second production caller
  through this fixture.

- **DM1-F0344-PLANAR-FILL-SYNTHETIC-AUDIT:** The standalone F0344 planar-fill
  helper has no M11/runtime caller; its callers supply test-owned bitmap
  buffers and layout boxes. The live HUD uses the independent source-bound
  champion status-box route. Keep this helper test-only until a real panel
  material consumer owns the original layout and pixels.

- **DM1-D3C-STAIRS-PIT-SYNTHETIC-AUDIT:** The D3C stairs/pit dispatch helper
  remains a test-only contract fixture for ReDMCSB dispatch/C10 rules. The
  live M11 D3C route is now source-bound through the real floor-pit/stairs
  zone plans and authenticated PC34 GRAPHICS.DAT loader; the generic
  procedural pit face is suppressed whenever that owner is active. Keep the
  fixture itself out of production and continue capture work for full
  pixel-level D3C parity.

- **DM1-D2C-CENTER-WALL-SYNTHETIC-AUDIT:** The D2C center-wall composition
  helper remains an asset-free, test-only trace of ReDMCSB ordering/C10
  rules. The live M11 D2C wall now takes only the authenticated GRAPHICS.DAT
  wall-set bitmap through C709; the generic grey wall face is suppressed for
  source-owned DM1 sessions so C10 pixels preserve earlier source layers.
  Keep the fixture out of production and retain original-data capture work
  for full D2C pixel parity.

- **DM1-CHEST-PARTIAL-DROP-SYNTHETIC-AUDIT:** The partial-drop-to-floor probe
  creates its chest stack, leader hand, floor link and coordinates locally.
  It has no M11/runtime caller and consumes no original DM1 save, dungeon or
  graphics data; keep it test-only until a live source-backed chest
  transaction owns the behavior.

- **DM1-CHEST-ROTATE-PICKUP-SYNTHETIC-AUDIT:** The chest-pickup-during-party-
  rotation model creates party direction/cells, chest slots, leader hands and
  queued pickup state locally. It reads no original DM1 input and has no
  M11/runtime caller; retain it only as an explicit ReDMCSB regression until
  the live source-backed transaction is recovered.

- **DM1-MIRROR-RESURRECT-CROSS-CANDIDATE-SYNTHETIC-AUDIT:** This helper
  models stale mirror-panel state during resurrection with synthetic
  champions and has no production caller or original-data input. Keep the
  race/clear contract test-only until the live F0280/F0346 owner is bound.

- **DM1-MIRROR-C545-PICKUP-SYNTHETIC-AUDIT:** This C545/C040 helper fabricates
  floor objects, chest slots, candidate state and panel counters. It reads no
  original DM1 data and has no M11/runtime caller; retain it only as an
  explicit ReDMCSB regression until a live pickup owner is bound to authentic
  object and panel state.

- **DM1-MIRROR-PENDING-HAND-CHEST-RACE-SYNTHETIC-AUDIT:** This race helper
  fabricates champion hands, chest slots, candidate state and panel counters.
  It reads no original DM1 data and has no M11/runtime caller; retain it only
  as an explicit ReDMCSB regression until a live queue/pickup owner is bound
  to authentic runtime state.

- **DM1-D1L2-D1R2-F0108-WALL-SYNTHETIC-AUDIT:** This wall-composition helper
  hard-codes ornament ordinals, zones, seeds and probe pixels. It reads no
  original DM1 data and has no M11/runtime caller; retain it only as an
  explicit ReDMCSB regression until a source-bound D1L2/D1R2 F0108 consumer
  is recovered.

- **DM1-D1L2-D1R2-F0108-FLOOR-CEILING-SYNTHETIC-AUDIT:** This floor/ceiling
  helper hard-codes ornament indices, zones, seeds and probe pixels. It reads
  no original DM1 data and has no M11/runtime caller; retain it only as an
  explicit ReDMCSB regression until a source-bound D1L2/D1R2 owner is
  recovered.

- **DM1-D2C-F0107-WALL-ORNAMENT-SYNTHETIC-AUDIT:** This D2C helper hard-codes
  wall-ornament ordinals, framebuffer pixels and probe boxes. It reads no
  original DM1 data or GRAPHICS.DAT material and has no M11/runtime caller;
  retain it only as an explicit ReDMCSB regression until a source-bound D2C
  F0107 consumer is recovered.

- **DM1-D3C-F0107-WALL-ORNAMENT-SYNTHETIC-AUDIT:** This D3C-only F0107 model
  fixes probe pixels, geometry and ornament ordinals, and explicitly reads no
  GRAPHICS.DAT material. It has no M11/runtime caller; retain it only as an
  explicit ReDMCSB regression until a live D3C F0107 material consumer is
  bound to authentic graphics and Thing state.

- **DM1-CHAMPION-PORTRAIT-BOX-REDRAW-SYNTHETIC-AUDIT:** This redraw-state
  matrix fabricates panel geometry, champion ownership and dirty-bit flows.
  It reads no original DM1 data and has no M11/runtime caller; retain it only
  as an explicit ReDMCSB regression until the live HUD binds authentic panel
  and champion state.

- **DM1-D2L2-D2R2-F0115-SYNTHETIC-AUDIT:** This D2 side-lane helper is a
  no-draw receipt with fixed F0115 rows and suppression values. It reads no
  original DM1 data and has no M11/runtime caller; retain it only as an
  explicit ReDMCSB regression until a source-bound D2 side-lane consumer is
  recovered.

- **DM1-D2L2-D2R2-SIDE-WALL-SYNTHETIC-AUDIT:** This F0678/F0679 source-lock
  model has fixed side-wall route data and a synthetic 8×8 probe. It reads no
  authenticated `GRAPHICS.DAT` material and has no M11/runtime caller; retain
  it only as an explicit regression while the live viewport uses the separate
  source-bound asset route.

- **DM1-D1L-D1R-F0107-SYNTHETIC-AUDIT:** This D1 side-wall helper hard-codes
  wall-ornament zones, ordinals and probe pixels. It reads no original DM1
  data or GRAPHICS.DAT material and has no M11/runtime caller; retain it only
  as an explicit ReDMCSB regression until a source-bound D1 side owner is
  recovered.

- **DM1-C545-OCCUPIED-LEADER-HAND-SYNTHETIC-AUDIT:** This C545 helper seeds
  party, chest, icon and panel state around inventory helpers. It reads no
  original DM1 data and has no M11/runtime caller; retain it only as an
  explicit ReDMCSB regression until a live chest transaction binds authentic
  party and object state.

- **DM1-D0L-D0R-F0115-SYNTHETIC-AUDIT:** This D0 side-lane helper hard-codes
  F0115 cell orders, rows and probe pixels. It reads no original DM1 data and
  has no M11/runtime caller; retain it only as an explicit ReDMCSB regression
  until a source-bound D0 side consumer is recovered.

- **DM1-F0449-F0450-FLOPPY-PLATFORM-AUDIT:** The F0449/F0450 helper is a
  fail-closed Atari-ST floppy boundary, not a PC34 runtime owner. Keep its
  source evidence test-only until a real platform-specific media consumer is
  identified; never synthesize PC34 floppy availability.

- **DM1-G0601-G0650-OWNER-AUDIT:** The G0601–G0650 table is metadata-only
  source-owner inventory. It must remain separate from production consumers;
  the actual mouse, champion, GRAPHICS.DAT and runtime-memory owners are the
  modules named by its rows.

- **DM1-F0181-F0200-OWNER-AUDIT:** The F0181–F0200 table is metadata-only
  source-owner inventory. It has no M11/runtime caller and does not consume
  real GROUP or DUNGEON bytes. Keep it in the explicit audit test until each
  listed function is bound to its live owner and authenticated DM1 state.

- **DM1-L0201-L0250-LOCAL-OWNER-AUDIT:** This ReDMCSB local-label table is
  source metadata only. It has no M11/runtime caller or authenticated game
  input; retain it in the explicit audit test until a live owner needs its
  local-storage evidence, never as standalone production state.

- **DM1-MIRROR-EYE-SLOT-SWAP-SYNTHETIC-AUDIT:** The C546/C09 eye-route model
  uses fabricated things, chest slots, panel state and icons, with no runtime
  caller or authentic save/graphics input. Keep it test-only until the live
  mirror/chest owner is bound to real DM1 state.

- **DM1-CHEST-SCROLL-DROP-ROTATION-SYNTHETIC-AUDIT:** This C540 regression
  fabricates items, charges, slots and command-queue state. It has no runtime
  caller or original-data input; keep it test-only until the live chest and
  command owners can be verified against authentic DM1 state.

- **DM1-L0151-L0200-LOCAL-OWNER-AUDIT:** This is metadata for F0115/F0116
  automatic locals, not an implementation or data owner. It has no runtime
  caller or authentic data input; retain it only in the explicit audit test.

- **DM1-F0410-F0411-SPELL-CONTINUATION-AUDIT:** This receipt contract has no
  M11/runtime caller; F0412 remains the live spell-result owner. Keep it
  test-only until authentic live spell state needs the bounded continuation.

- **DM1-D2L2-D2R2-WALL-SYNTHETIC-AUDIT:** This wall contract fills local
  framebuffers with synthetic probe pixels and has no M11/runtime caller or
  authenticated GRAPHICS.DAT input. Keep it test-only until a real D2L2/D2R2
  asset consumer requires the source dispatch evidence.

- **DM1-D3L2-D3R2-F0115-THING-PASS-SYNTHETIC-AUDIT:** This D3 side-route
  model hard-codes viewport zones, source coordinates and pixel blends. It
  reads no original DM1 data and has no M11/runtime caller; retain it only in
  its explicit contract tests until a real F0115 D3L2/D3R2 consumer is bound
  to authenticated GRAPHICS.DAT and live Thing records.

- **DM1-D1L-D1R-F0108-SYNTHETIC-AUDIT:** This D1 side-view F0108 contract
  hard-codes zones, seeds and probe pixels. It has no M11/runtime caller and
  reads no original DM1 data; retain it only in its explicit regression until
  a live F0108 consumer is bound to authenticated GRAPHICS.DAT material.

- **DM1-D0L-D0R-F0108-SYNTHETIC-AUDIT:** This D0 side-view F0108 contract
  hard-codes zones, seeds and probe pixels. It has no M11/runtime caller and
  reads no original DM1 data; retain it only in its explicit regression until
  a live F0108 consumer is bound to authenticated GRAPHICS.DAT material.

- **DM1-D1L2-D1R2-F0115-THING-PASS-SYNTHETIC-AUDIT:** This D1 side-route
  table hard-codes rows, zones and cell orders. It reads no original DM1 data
  and has no M11/runtime caller; retain it only in its explicit regression
  until a live F0115 consumer is bound to authentic Thing and GRAPHICS.DAT
  material.

- **DM1-D0C-F0108-FLOOR-ORNAMENT-SYNTHETIC-AUDIT:** This D0C contract uses
  fixed pixels, seed and zone metadata. It has no M11/runtime caller and
  reads no original DM1 data; retain it only in its explicit regression while
  the independent D0C real-material route remains the production owner.

- **DM1-D0L2-D0R2-F0108-SYNTHETIC-AUDIT:** This D0 side-route F0108 contract
  hard-codes rows, zones, seeds and pixels. It reads no original DM1 data and
  has no M11/runtime caller; retain it only in its explicit regression until a
  live F0108 consumer is bound to authenticated GRAPHICS.DAT material.

- **DM1-D0L2-D0R2-F0111-SYNTHETIC-AUDIT:** This partly-open-door route model
  hard-codes frames, zones and states. It reads no original DM1 data and has
  no M11/runtime caller; retain it only in its explicit regression until a live
  F0111 consumer is bound to authenticated door graphics and Thing state.

- **DM1-D0L2-D0R2-F0115-THING-PASS-SYNTHETIC-AUDIT:** This D0 side-route
  model hard-codes rows, zones and pixel composition. It reads no original
  DM1 data and has no M11/runtime caller; retain it only in its explicit
  regression until a live F0115 consumer is bound to authentic material.

- **NEXUS-FULL-BUILD-EXTERNAL-LINK-GAP:** The Nexus production archive now
  links the real SCR section-table parser while compiling out the unproven
  flat glyph/framebuffer writer. Focused Font256 S2D targets are green. The
  aggregate Firestaff build currently stops later in an unrelated DM2
  FM-Towns link (`dm2_v1_fmtowns_anim_stream_*`); keep that separate from the
  remaining Nexus Saturn page/tilemap/VDP2 gap and do not bypass either with
  synthetic data.

- **CSB-FMTOWNS-C06-UTILITY-TRANSACTIONS:** F31E:s första C06-editorbild är
  2026-08-08: filväljarpilarna avkodas nu med den verkliga F0689-striden
  (31 logiska pixlar, 32-pixelbuffert). Återstår att binda den källägda
  dialogytan och katalogvalet innan Load får lämna sin fail-closed-grind.

- **THERON-FORCEFIELD-REAL-DUNGEON:** Enter now reliably dispatches from the
  Soul Room forcefield focus, including the first attempt without prompt text.
  Authenticated raw MODE1/2352 Track 02 now reaches the source-faithful
  initial level/object handoff. Keep VDC/VCE presentation and later object
  semantics gated until their original consumers are bound; do not replace
  either gate with fallback dungeon data.

- **THERON-STARTUP-ANIMATION-CONSUMER:** Real Track 02 startup bitmap spans,
  required before admitting startup animation timing, menu input, or later
  level consumers.
  rather than leaking synthetic pixels. Authentic font-tile bytes also remain
  but the available headless SDL run remains before the game-owned CD read

- **NEXUS-PRS3-PIXEL-CAPTURE:** Retail Structure2 descriptors still expose
  bounded DMWeb format evidence. The source-bound DMWeb PRS3 byte decoder now
  admits all 162 real MENU.BPK surfaces to a decoded-byte receipt; it does not
  emit renderer pixels. Recover the Saturn pixel order, CLUT owner and VDP1
  command capture before reopening texture surfaces, palette conversion or
  DGN raster submission.

- **NEXUS-STARTUP-TEXT-CAPTURE:** Retail PLRD rows now refuse stale
  serialized ASCII names in the startup footer/row-label lane; only the
  isolated compatibility roster may expose host labels. Recover the Saturn
  TEXT4/TABL/FONT256 consumer and placement before admitting real names,
  stats, or action labels to the menu.

- **NEXUS-FONT256-FRAMEBUFFER-CAPTURE:** The obsolete flat 1bpp text bridge
  2026-08-08 follow-up: the real S2D receipt now keeps the 4096 page words,
  2048 page-pattern capacity and 242 actual CG tiles as separate facts. The
  pattern capacity is no longer available as if it were a 2048-entry glyph
  table; code mapping and VDP2 placement remain capture-gated.

- **NEXUS-ACTION-DISPATCH-CAPTURE:** Retail mechanics no longer consume
  queued movement or turn commands merely because DGN floor geometry decoded;
  the prior compatibility loop could mutate party coordinates before the
  action gate. Recover the Saturn event producer, command queue and SDDRVS
  state-write owner before reopening any retail mechanics mutation.

- **NEXUS-SQUARE-DOOR-PRODUCTION-BOUNDARY:** The explicit fixture/study door
  registry still documents the DM1-shaped stepped animation, but the retail
  `firestaff_nexus` build now rejects direct square-door open/close/lock,
  passability and animation calls. A DGN type-8 byte alone does not prove the
  Saturn SDDRVS state write or VDP1 door-frame consumer. Remove this boundary
  only after authentic door-event and frame capture binds those owners.

- **NEXUS-BOOT-FEATURE-CONSUMER:** The default boot profile now enables only
  the authenticated Saturn data-container route. Historical flags for a
  six-member party, Nexus stat growth, rune UI, party swapping, and 2D/3D
  behavior remain opt-in until their retail consumers are proven from
  disassembly or capture.

- **NEXUS-CREATURE-VDP1-CAPTURE:** The generic creature billboard API is now
  no-draw. Its old DM1-shaped perspective/flag behavior accepted a host
  texture without Saturn VDP1 command, CLUT, placement, or DMDF/MNS owner
  proof. The retail MNS corpus regression now requires all 30 production MD5
  identities before decoding or texture rendering. Recover Saturn VDP1
  command/CLUT/placement bindings before admitting creature pixels.

- **NEXUS-UI-VDP1-VDP2-CAPTURE:** UI surface loaders retain verified source
  pixels and palettes, but the public blit, palette-remap and darken helpers
  are now no-draw/source-preserving. Recover Saturn command order, CLUT bank,
  destination and brightness/composition ownership before restoring them. The
  startup-media regression now requires retail hashes for its STABG/WARNING/
  GAMEOVER/TITLE/LOGOBG inputs before format receipts count. The V2 HUD
  diagnostic probes now match that boundary: enabling the phase gate, setting
  champion/action state, or using the test force switch cannot write synthetic
  pixels while the retail HUD/VDP1/VDP2 capture is absent. 2026-08-08:
  the STABG DMWeb receipt now retains offset, dimensions, cell count and
  maximum tile index for all 11 retail maps; this is still source inventory,
  not a VDP placement or drawable-HUD proof.

- **DM2-PRODUCTION-PLACEHOLDER-INVENTORY:** The source tree retains bounded
  **2026-08-06 V2 HUD update:** removed the disabled procedural pixel renderer
  rather than retaining generated compass, label, bar and action-strip code.
  The source-backed GDAT HUD runtime remains the only admissible draw owner.

- **DM2-SOUND-TEXT-CONSUMER:** `c_sound.cpp` binds class triples to GDAT sound
  **2026-08-07 corpus update:** the SOUND-GDAT real-data probe now accepts
  only explicit `FIRESTAFF_DM2_DATA_DIR/graphics.dat`; a selected unreadable
  corpus fails instead of falling back to a private installation.

- **DM2-WEATHER-TEXT-CONSUMER:** SKProject's `c_weather.cpp` exposes numeric
  **2026-08-07 source-text audit:** the selected PC-DOS set-5 corpus contains
  nine weather-command text rows at fields `0x64..0x6c`, plus two generic
  environment-element rows at `0x01` and `0x63` used by the broader
  `skguivwp.cpp` distant-element scan. None is a weather display-name record;
  the real-data regression asserts this split and keeps
  `dm2_v1_weather_name()` unavailable.

- **DM2-FIXED-SPELL-AI-TEXT-CONSUMER:** The fixed `dSpellsTable` and genuine
  **2026-08-07 corpus update:** the SPELL_DEF regression reads only explicit
  `FIRESTAFF_DM2_DATA_DIR/graphics.dat`; the PC-DOS corpus's zero-row result
  is asserted as a source fact, while an unreadable selected file fails.

- **DM2-INVENTORY-TEXT-CONSUMER:** `defines.h`'s inventory-slot constants are
  internal indices, not display strings. SKProject `DRAW_ITEM_SURVEY` renders
  the selected record through `GET_ITEM_NAME()`, but that original text route
  is not yet bound in Firestaff. The inventory helper therefore emits no
  `ready_hand`, `EMPTY`, `POOL … INDEX …`, or `UNRESOLVED` fallback text.
  Bind the selected profile's authenticated item-name record and font route
  before presenting any inventory description. **2026-08-11 update:** the
  inventory-panel module's GDAT backdrop/frame receipts remain explicit-test
  owners only; it has no M11/runtime caller and its description parameter is
  caller-supplied. It is now excluded from `firestaff_dm2` until the live
  `DRAW_ITEM_SURVEY`/`GET_ITEM_NAME()` handoff is connected.
  **2026-08-07 corpus update:** the inventory-GDAT receipt probe likewise
  accepts only explicit `FIRESTAFF_DM2_DATA_DIR/graphics.dat`; missing selected
  media is a verification error rather than an implicit HOME fallback.

- **DM2-DOOR-TEXT-CONSUMER:** Door state and type constants own mechanics and
  GDAT selects their image materials; no original text producer has been
  recovered for `OPEN`, `CLOSED`, `WOODEN`, or similar labels. The compatibility
  label APIs return `NULL`; bind an authenticated player-facing text route
  before showing a door description.
  **2026-08-07 real-data audit:** the selected PC-DOS `DOORS` category has no
  typed text rows. The real door-panel regression now asserts that fact while
  continuing to verify the actual RAW4/GDAT door material route, and also
  checks every state/type label accessor remains unavailable. SKProject's
  `DRAW_DOOR`/`DRAW_DOOR_FRAMES` path is image/word-data based, not a text
  producer.

- **DM2-CDDA-OTHER-PLATFORM-MEDIA:** FM Towns CDDA now has a verified
  disc can enter the startup route. Recover the native P3/TBIOS menu code,
  its input loop and the handoff back to M11 before showing any SKULL menu.
  equivalent selected-medium reader and therefore must remain silent rather

- **NEXUS-VDP1-COMMAND-SOURCE-JOIN:** The authentic eight-frame witness now
  provides a strict command-to-VRAM join: `COPR=0x00000c` gives system
  records `0x09`/`0x0a`, one type-2 bitmap draw and END; frame 7 uses
  `PMOD=0x0028`, `SRCa=0x8f80`, `SIZE=0x28b4`, mapping to byte offset
  `0x47c00`, which matches writer PC `0x06013098`. The 33,280-byte source
  span still lacks a decoded MENU/DGN/ITEM owner and VDP2 CLUT/tile join, so
  menu/HUD/viewport production drawing remains blocked.

- **NEXUS-VDP1-RUNTIME-WRITER-JOIN:** The captured writer code window at
  `PC=0x06013098` is now checked against hash-authenticated `DM.BIN` and
  `TM.BIN`. Under the explicit `0x06010000` hypothesis, `DM.BIN+0x3058`
  mismatches and the best whole-file native overlap is one word. Recover the
  actual relocated/decompressed code image and its loader handoff before
  claiming a runtime source owner; short byte overlaps must not promote a
  DGN, MENU, ITEM, HUD or viewport consumer.

- **NEXUS-VDP2-WRITE-OWNER:** The authentic startup producer now emits a
  strict VDP2 write witness covering registers (`0x180000..0x18011e`), VRAM
  (`0x000000..0x0743fe`) and CRAM (`0x100000..0x1007fe`). The current
  external hook now records nonzero SH-2 PCs; primary VRAM writers are
  `0x06011924`, `0x060118fc` and `0x06002fc4`, with register writes dominated
  by `0x0600231c`. This proves executing-code ownership of the writes, but
  the decoded tilemap/CLUT bank and final layer placement are still missing;
  recover those before promoting startup menu, HUD or viewport presentation.
  The new code-window witness captures 64 unique writer PCs; primary
  `0x06011924` exposes `25fe 0000 25fe 007c ...`, while `0x06001416` exposes
  the VDP-register setup literals. Exact windows do not match the current
  hash-verified `TM.BIN`/`DM.BIN` byte ranges, so determine the retail
  relocation/decompression owner before interpreting the runtime words.

- **NEXUS-STARTUP-CAPTURE-ENVIRONMENT:** The supplied European BIOS is
  authenticated by SHA-256 `96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f`
  and the local English CUE by
  `b96f01e2f8ce3ab9c8e4a33d5a0c7076cdc1bfd247a85a1454e6c36c8a616f33`.
  The English CUE/ISO is now known to be `SGAREA U`, not European media. The
  extracted French ISO is `SGAREA E` (SHA-256
  `a7644c7cfecda2d604a1bd6b1df70124eafe4357692eedfe60b1d9c3efc4dc43`) and is
  the current European capture target. Its supplied CUE references Japanese
  audio files that are absent locally, so only a temporary data-only CUE was
  used for the E-BIOS startup witness. Stock Mednafen is rejected at launch with
  exit 78 because it lacks `FIRESTAFF_NEXUS_TRACE_OUTPUT`. No startup/menu,
  HUD, VDP1, SLEV, or SAL artifact may be admitted from that binary.
  2026-08-06 follow-up: the generic Mednafen launcher now rejects a shared
  `--trace`/`--manifest` path before writing either artifact, matching the
  collision guard already used by the specialized VDP1/SLEV launchers.

- **NEXUS-TITLE-TRUNCATED-SOURCE-GATE:** `nexus_title_load` now refuses to
  form the DMWeb MAPD/TIBG offset when a cached title surface is paired with
  a short `TITLE.BIN`. The remaining work is the original Saturn VDP2
  tile-map/CLUT/timing capture; decoded retail MAPD/TIBG bytes are not yet a
  presentation proof.

- **NEXUS-STARTUP-ANIMATION-CAPTURE:** Startup timing and real
  `WARNING.BIN`/`TITLE.CG`/`TITLE.BIN` source receipts remain available for
  diagnostics, but they are not a Saturn presentation capture. The animation
  package gate now requires an explicit original VDP1/VDP2 frame binding before
  it can ever report draw permission. Recover the title/roster animation
  command order, palette/tile ownership and frame capture before setting that
  binding; no host timing plan or source asset alone may unlock animation
  pixels.

- **NEXUS-EVENT-DISPATCH-CAPTURE:** The 61 `DM.BIN` event names remain a
  byte-exact source receipt at `0x36D04..0x3702F`, but the former host
  event→command map and accepting
  dispatcher were synthetic DM1-shaped behavior. They are now unbound and
  state-free. The event regression now rejects any `DM.BIN` whose MD5 is not
  the authenticated European retail identity
  `e88d60859f65f08fa622e1992b02280f`. Recover the Saturn event producer, command queue, SLEV callback
  and SDDRVS state writes before admitting movement, inventory, spell, save,
  menu or HUD actions.

- **NEXUS-RAW-BIN-OWNER-CAPTURE:** The retail receipts for `DM.BIN`,
  `NBG3.BIN`, `STONE.BIN`, `DEATH.BIN`, `SWTCHR.BIN`, `TM.BIN`, and
  `SDDRVS.TSK` now deliberately report `UNKNOWN` content ownership. The
  previous byte-frequency classifier mislabeled opcode-like data as SH-2 or
  tilemap content. Recover each consumer from Saturn execution/VDP capture or
  disassembly before admitting VDP1/VDP2 pixels, tilemaps, switch graphics,
  or executable task semantics.

- **THERON-PALETTE-ROUTE:** A raw verified Track 02 load now starts with an
  empty palette. The former procedural stone palette was not source evidence
  and could make an unbound bundle look renderable. Keep the runtime blocked
  until a captured HuC6260 palette span and consumer are hash/offset-bound.
  2026-08-06 follow-up: the raw 4bpp window remains diagnostic evidence only;
  `theron_v1_startup_media_bind_runtime_palette()` now refuses to promote it
  into `runtime_media`, so a candidate palette cannot unlock M11 startup art.

- **THERON-T700-STAT-CONSUMER:** The production mechanics path now leaves
  source-authenticated levels completely unchanged after movement. The old
  host-side T700 model drained stamina/food/water and cleared poison without a
  recovered PCE bank-switched consumer, and therefore remains fixture-only.
  Recover the original stat-write owner, cadence and poison lifetime from
  disassembly/capture before reopening world ticks or champion mutations.

- **THERON-BITMAP-ROUTE-PROVENANCE:** The raw Track 02 bitmap catalog and
  indexed atlas retain real byte spans from `TQUS02.bin`/`TQJP02.bin`, but the
  `title`, `stage`, `Soul Room`, and `forcefield` route bits are still bounded
  candidate classes from the layout scan, not VDC/VCE consumer bindings. The
  receipt remains diagnostic evidence only: do not promote its route labels
  to semantic screen ownership, palette assignment, viewport pixels, or a
  later-level record until a captured original consumer read joins the source
  LBA/span to the executing HuC6280 code. See
  `docs/source-lock/tqr_v1_bitmap_route_provenance_audit_2026-08-06.md`.
  2026-08-06 follow-up: M11 now has an independent startup presentation gate;
  even a palette flag cannot draw candidate atlas pixels until a captured
  VDC/VCE destination and semantic route are explicitly bound.

- **THERON-US-ROSTER-CONSUMER:** The production startup receipt no longer
  copies the eight JP roster literals into an authenticated US Track 02
  receipt. The real US BIN exposes the startup prompt, but its champion
  names/titles are not yet proven as an ASCII cluster or bound to an
  executing text consumer. Recover that encoded payload from the US
  Track 19/Track 02 disassembly and capture before restoring US mirror labels;
  until then the menu exposes those entries as `UNAVAILABLE`, disables their
  hit targets, and skips them during keyboard/controller focus rather than
  publishing host-owned names. The authenticated JP Track 02 roster remains
  independently bound and is now covered by a real-media regression for all
  eight names and titles.
  2026-08-06 production audit: the cross-reference numeric champion records
  remain available to the source-bound forcefield handoff, but production
  now compiles out their unproven US names/titles. The named table is fixture
  and probe-only; no US text consumer is implied by its presence.

- **M11-F10-SOURCE-SPECIFIC-CHEATS:** The all-game F10 CH page now exposes
  the verified shared cheat master switch and live speed control. Do not add
  game-specific god mode, infinite-item or debug toggles until an original
  source/runtime owner is proven for that game; research those controls as
  separate source-locked work rather than fabricating behavior.

- **M11-F10-SOURCE-SPECIFIC-FILTERS:** The live panel now routes Theron V2
  scanline, palette, dither, scale and bilinear changes through its own
  `theron_v2_settings` owner. DM2 and Nexus no longer inherit DM1 filter
  controls: their global presentation controls and shared cheat/speed owner
  remain available, while source-specific filter/effect pages report
  `SOURCE LOCKED` until a real post-process chain is admitted. Add those
  controls only after verified DM2/Nexus runtime ownership and pixel evidence.

- **ASSET-SCAN-CROSS-PLATFORM-CORPUS:** DM2 never materializes game data:
  launches; keep the remaining title/entrance capture work separate from this
  data-admission boundary.
  reader; protected STX remains intentionally

- **NEXUS-DGN-MATERIAL-ISO-RECEIPT:** The extracted European retail layout now
  authenticates `SN_FLOOR.MNS` and `SN_WALL.MNS` from the co-located Track 1
  ISO through the same fallback used by the loader. The 16-level selector
  census reaches `0x7D`, while each MNS bank has only 15 TEXT descriptors;
  direct selector-to-descriptor ordinal binding is therefore disproven.
  Recover the Saturn selector transform and VDP1 ownership from executable or
  runtime capture; palette, transform and drawing evidence remain gated. The
  direct Structure1F→Structure2 material-target API now also rejects the retail
  corpus while `structure1b_selector_binding_proven` is false; raw geometry and
  Structure2 payload anchors remain separate capture evidence rather than a
  guessed material owner.

- **NEXUS-SAL-MAP-CORPUS-RECEIPT:** The 16 real European SAL/MAP pairs now
  pass the retail byte-zero MAP parser (154 bounded eight-byte rows, `FF FF`
  terminators, all SAL windows in bounds). The SAL provenance regression now
  also rehashes every real `dsp01.EX` header through the direct-identity route
  while keeping codec and playback flags false. Continue with the missing Saturn
  event→MAP selector trace and SDDRVS playback consumer; do not infer those
  semantics from row order or decoded tone candidates.

- **NEXUS-SLEV-TASK-CORPUS-RECEIPT:** The real `SLEV00.BIN` through
  `SLEV15.BIN` corpus now passes the common SH-2 entry-spine and bounded
  PC-relative literal checks. `nexus_v1_slev_task_corpus_receipt` now
  rehashes and profiles all 16 files in one retail-data test. Continue with
  execution-capture proof of task body, callback owner and event semantics;
  no task opcode is executable yet.

- **NEXUS-CDDA-LEVEL-SELECTOR-CAPTURE:** The European CUE/ISO receipt proves
  2026-08-06 follow-up: the Nexus default boot profile no longer advertises
  `SATURN_CDDA_AUDIO`; the flag is reserved for a future source-bound capture
  and cannot be mistaken for playback readiness.
  The same default profile now leaves `RESTRICTED_DOOR_CLOSES` clear because
  the Saturn door-transition/timer consumer is still capture-gated.

- **NEXUS-STALE-ISSUE-PAGES:** The three historical Nexus issue pages are now
  explicitly labelled as snapshots. Keep the strict-fidelity inventory and
  capture gates authoritative when further format or runtime gaps are found.

- **NEXUS-STARTUP-MNS-REGRESSION:** Keep the real launch-smoke assertions for
  supplemental-ISO MNS material binding and the closed TEXT4/TABL/FONT012
  capture gate green while the Saturn VDP1/VDP2/text consumer remains open.
  2026-08-06: save/champion chrome and save-row builders no longer emit
  host-invented ASCII labels; bounded `DRAW_NONE` slots retain capture
  accounting until the real TEXT4/TABL/FONT012 consumer is captured.
  The runtime save-menu handoff now applies the same boundary and strips
  generic builder text commands before M11/M12 can consume the package.
  The Track 1 real-data launch probe now distinguishes complete FACE.BIN
  source coverage from uncaptured VDP1 placement and keeps `font_loaded` at
  zero until Saturn text page/attribute mapping is captured.

- **NEXUS-CREATURE-MNS-CONSUMER:** The 30-entry MNS filename roster is now
  authenticated byte-for-byte against European retail `DM.BIN` at `0x0385F0`.
  Production creature types now expose those retail filenames instead of
  invented English labels, and the 30-entry AI/sentinel table is checked
  against `DM.BIN+0x0383A8`. Keep both as source metadata until a production
  creature-model consumer binds CRET indices to DGN/MNS records; do not infer
  stats or render models from filenames/addresses alone.

- **NEXUS-PRS3-VDP1-STATE-RECEIPT:** The real MENU.BPK PRS3 decoder now
  2026-08-06: the retail V8/V9/V10 schema regression no longer crashes on
  its appended DGN fields or leaves a negative receipt in place for later
  readiness checks; this repairs verification coverage only and does not
  promote PRS3 decoding or VDP1 presentation.

- **NEXUS-PLRD-FIXTURE-QUARANTINE:** The inferred 24-name champion roster is
  no longer compiled into `firestaff_nexus`; it is isolated in the explicit
  compatibility-fixture library used by legacy tests. Keep production
  champion initialization on authenticated European RLOWFIX/PLRD records and
  add no inferred labels, stats or party members while the Saturn text/menu
  consumer remains capture-gated.

- **NEXUS-STALE-DOCUMENT-CLAIMS:** Older Nexus content/format documents still
  contain historical “implemented” wording for DM1-derived roster/combat,
  host mesh/title/HUD rendering, text conversion, and audio playback. The
  focused audit in `docs/NEXUS_STALE_CLAIM_AUDIT.md` quarantines those claims
  and points to the current retail receipts and capture gates. Continue the
  sweep when a new source-owned consumer is proven; do not reopen a route from
  documentation alone. The testing, armor, potion and combat-item pages now
  carry explicit historical/diagnostic banners; their old “implemented” and
  “no tests” wording is not current status.
  2026-08-08 follow-up: `docs/nexus_data.md` no longer describes retail DGN
  files as 32×32/DM1-shaped or `DM.BIN` as a disc image. It now records the
  mounted corpus's 64×64 Structure1B/8-byte cells, 25,012-byte FONT256
  revision and 555,144-byte DM.BIN resource, while retaining presentation and
  audio capture gates.

- **NEXUS-COMPAT-GATE-COMBAT-CLAIM:** The Nexus phase-gate combat description
  now calls the DM1-shaped combat helper diagnostic only. The retail Saturn
  attack dispatcher, target admission, RNG and effect writes remain missing;
  `nexus_v1_action_semantics_proven()` must stay closed until those receipts
  are captured.

- **NEXUS-CREATURE-AI-RUNTIME-GATE:** Retail CRET statistics and DGN actor
  identities are now retained as source receipts, but production no longer
  advances the host creature AI, spawner, or projectile loop while the
  SLEV/DM.BIN actor/action consumer is uncaptured. Recover those state-write
  and dispatch traces before enabling movement, spawn, ranged attacks, or
  projectile ticks. The direct helper tests remain diagnostic only.

- **NEXUS-PLRD-PROVISIONS-GATE:** `RLOWFIX.BIN/PLRD` does not carry food,
  water, gold, or a saved alive/dead state. The production parser no longer
  seeds the DM1 value `1500`; provisions stay unbound until the Saturn
  new-game/save consumer is captured. Keep `alive` only as a menu-availability
  state and do not use it as save provenance.

  2026-08-06 follow-up: the mechanics tick no longer drains unbound retail
  food/water or applies the inherited starvation stamina penalty in ISO or
  extracted engines. The compatibility fixture path remains available for
  isolated tests; capture the Saturn hunger/start/save consumer before
  enabling production resource mutation.

- **NEXUS-REST-STATUS-LIGHT-CAPTURE:** The DM1-shaped rest regeneration,
  2026-08-06 follow-up: M11 no longer initializes, restores or ticks the
  data-free light-overflow host timeline while
  `nexus_v1_action_semantics_proven()` is closed. The standalone
  `nexus_v1_light_runtime`/`light_overflow` implementation remains probe-only
  evidence until Saturn F0238/F0257 execution and save ownership are captured.

- **NEXUS-RETAIL-TICK-STATE-GATE:** Retail ISO/extracted ticks now leave the
  unbound action cooldown, door animation, trap cooldown and DM1-derived
  step-stamina state unchanged. Movement over decoded floor geometry remains
  available, but registered doors still require the Saturn SDDRVS transition
  receipt before opening. Creature death/XP/script/spawner follow-up,
  damage-display/message timers and game-over transitions are likewise closed
  for retail. Unproven teleporter and level-transition writes are also held
  pending rather than applied to retail pose/level state. The fixture path
  remains available for isolated helper tests; restore each retail route only
  from a captured Saturn owner.

  2026-08-06 follow-up: `nexus_v1_action_timer.c`, `nexus_v1_doors.c`,
  `nexus_v1_traps.c` and `nexus_v1_projectiles.c` are now excluded from
  `firestaff_nexus`; their original state machines remain explicit study
  inputs and production exports only a state-preserving ABI adapter.

- **NEXUS-CONTAINER-LOOT-CAPTURE:** The old DM1-shaped Nexus container helper
  accepted caller-supplied chest/crate contents despite the retail DGN corpus
  exposing no authenticated container owner, content chain, key dispatch or
  Saturn loot writeback. The public route is now fail-closed; recover those
  real records and the original consumer before admitting chests or loot.

- **NEXUS-SHOP-ACTION-CAPTURE:** DM.BIN's eight retail price rows remain a
  verified catalog receipt, but the old host shop manager could still invent
  shop instances and stock. Runtime shop registration, stock and open/lookup
  admission is now blocked until the Saturn shop-object consumer is captured.

- **THERON-REAL-CONSUMER-HANDOFF:** The authenticated US/JP Track 02 map,
  2026-08-06 production-boundary follow-up: the fixture-only party reset and
  first-room constructors are now absent from the `firestaff_theron` archive;
  keep this boundary when adding the authenticated consumer rather than
  re-exporting the old synthetic helpers.

- **THERON-M11-FORCEFIELD-INPUT:** M11 now lets authentic Track 02 startup
  input reach the forcefield admission path even when the source-owned
  post-startup VDC/VCE capture is unavailable. The old pre-dispatch atlas
  check returned to the launcher before Enter was interpreted. Pointer input
  now follows the same route and keeps the Soul Room visible while reporting
  the admission boundary. Keep dungeon promotion fail-closed and replace the
  remaining capture gate only after a real consumer receipt binds the level,
  object, tile and palette routes. The production no-roster API now also keeps
  its successful forcefield state instead of undoing the transition with
  `NOT_READY`; it admits Theron alone and leaves companion display names
  unavailable. When mirrors are selected, production now binds their real
  Track 02 champion records (stats/skills/equipment) without inventing names;
  the remaining gap is the original text consumer and later dungeon capture.

- **NEXUS-STARTUP-SOURCE-BYTES:** The verified TITLE.CG, WARNING.BIN,
  2026-08-06: the legacy Nexus overview/language/startup/champion/features
  documents were corrected to stop publishing synthetic Japanese-only claims,
  eight/24-name rosters, DM1-derived semantics, or completed renderer/audio
  behavior. Remaining documentation outside the focused startup/menu/HUD/
  viewport set still requires a full stale-claim sweep.

- **DM2-REAL-DATA-RENDER-INVENTORY:** The PC English `GRAPHICS.DAT` corpus is
  **2026-08-07 coordinate audit:** the mounted PC-English corpus expands `0x5C`
  to `(456,100,92,77)`, while NEED_FLASK decodes to `92x25`. These coordinates
  are outside the 320x200 dungeon viewport and therefore prove that this route
  belongs to the larger M11 game surface, not the viewport framebuffer; do not
  blit it through the viewport consumer.

- **DM1-HOC-VIEWPORT-AUDIT:** Real PC34 HoC object-name, alcove placement,
  2026-08-06 macOS capture update: a clean real-data Entrance frame and a
  post-Entrance dungeon viewport frame are now checked in under
  `docs/screenshots/`. These prove the packaged window can present source
  DM1 data, but they do not close the remaining HoC/HUD object, torch-holder,
  side/depth mirror or held-object-cursor capture requirements.

- **DM1-HOC-ORNAMENT-RENDER-CAPTURE:** The generic DM1 dungeon bridge now
  consumes the real PC34 map ornament counts, seed, square attributes and
  F0169/F0170 random wall/floor ordinal formulas. The real PC34 map-0 receipt
  verifies wall ordinal 3 and floor ordinal 3. The D1/D0 native-palette
  regression is fixed; remaining work is external Mac/app viewport capture
  for every wall, inscription, torch, alcove, stairs and floor ornament.

- **DM1-DIRECT-LOOP-CAPTURE:** The legacy direct DM1 loop now rejects missing
  or unparsable hash-verified GRAPHICS.DAT/DUNGEON.DAT instead of continuing
  with an invented Hall-of-Champions `(11,29,N)` pose. Capture the resulting
  source-backed direct loop on macOS and compare its complete viewport/HUD
  composition against ReDMCSB.

- **DM1-M653-FONT-CAPTURE:** Authenticated DM1 text now refuses the generic
  host 5x7 font when the original M653 GRAPHICS.DAT record is unavailable.
  Broader macOS capture must still verify every message, inscription and HUD
  text route with the loaded PC3.4 font.

- **CSB-FMTOWNS-RUNTIME-PARITY:** The authentic FM Towns CD's verified
  2026-08-06: a normal broad data-root scan now also admits the named retail
  FM Towns archive below its documented `csb/` child, so the launcher retains
  both F31 English and Japanese profiles rather than hiding them unless that
  child is selected directly.

- **NEXUS-SFX-EVENT-DISPATCH-CAPTURE:** Host sound-request names are now
  2026-08-06 follow-up: the M11 Light/Torch/Darkness bridge now checks the
  same action-semantics gate before touching its compatibility light timeline;
  recognized rune shapes remain input-only until the Saturn spell command,
  caster-state write, event-70 producer and SLEV/SDDRVS side effects are
  captured.

- **NEXUS-PALETTE-CAPTURE:** The retail SMAP/FACE/ITEM/MNS palette decoders
  now share the Saturn BGR555 channel order. Remaining work is an independent
  VDP1/VDP2 capture that proves each asset's upload/CLUT destination and final
  display composition; decoded host surfaces are not themselves presentation
  evidence.
  2026-08-06 follow-up: standalone ITEM.IBS/TITLE.CG RGBA writers are now
  excluded from `firestaff_nexus`; their source decoders remain explicit
  probes while VDP1/VDP2 upload, CLUT and placement ownership stays open.

- **NEXUS-SHOP-ACTION-CAPTURE:** The eight retail shop price rows are now
  source-bound from `DM.BIN`, but shop stock placement, purchase/sell dispatch,
  item ownership and UI feedback still require the authentic Saturn
  action/event consumer. Do not treat the price catalog as proof of gameplay
  semantics.
  The standalone host shop manager now keeps both purchase and sell mutations
  fail-closed; only the verified price catalog remains usable until capture.

- **NEXUS-GOLD-DROP-CAPTURE:** Creature gold/drop producer and pickup event
  remain unbound. The public gold-pile add path is now fail-closed as well;
  no host amount may become a floor object without a Saturn capture.
  The real ITEM.IBS corpus regression now also requires the authenticated
  retail hash before its 243-item/223-image census is accepted.
  The exported item-use helpers now also remain no-op/fail-closed; the old
  DM1-derived food/potion magnitudes and status durations are not inferred from
  the unproven `ITEM.IBS` Word36 field.

- **NEXUS-MENU-SEQUENCE-CAPTURE:** `docs/nexus_menu.md` no longer treats the
  2026-08-06: the startup regression now also verifies the `FONT256.S2D`
  literal-pool target at `DM.BIN+0x18BF4` and the adjacent `TEXTTABL` marker at
  `0x294C0`; these are loader/table receipts only, not text-render proof.
  The same receipt now pins the SH-2 routine at `0x18B60` and its literal
  pool hash, including the `yam\\menu.c` and `STABG.BIN` address targets.

- **NEXUS-MANIFEST-CONTAINER-COVERAGE:** The asset verifier now recognizes
  the authenticated English/French `MENU.BPK` and English `RLOWFIX.BIN`
  alternate retail identities by exact SHA-256 instead of reporting stale
  canonical-size mismatches. Direct ISO members are now listed and streamed
  through 7-Zip, then accepted only after exact size/SHA-256 identity checks;
  the supplied English ISO therefore verifies all 137 disc assets without
  extraction. Nested ISO files inside a 7z archive remain uninspected until
  an explicit container traversal path is added.

  2026-08-06: the runtime now keeps a valid co-located retail ISO as a
  supplemental source when the hash-verified extracted corpus wins startup.
  Exact missing members such as `DMN_ABS.TXT` are read from that ISO only;
  loose files remain authoritative. Nested 7z traversal is still out of scope.

- **NEXUS-PLRD-TABL-NAME-CONSUMER:** Production champion initialization is
  already fail-closed on the verified European `RLOWFIX.BIN` PLRD resource;
  the legacy hardcoded roster is test-fixture-only and must not populate live
  names, classes, stats, portraits or inventory. The real 20×64-byte PLRD
  records and six TABL indices/codes are admitted, but the TEXT/FONT256
  consumer that renders those codes as champion names has not been captured.
  The startup row now carries the verified FONT256 glyph-code sequence without
  converting it to guessed ASCII/JIS. Keep visible text no-draw until the
  Saturn TEXT/FONT256 VDP2 consumer and glyph placement are authenticated.

- **NEXUS-FACE-VDP1-PLACEMENT:** FACE.BIN source portraits are now loaded from
  the real PRS3 streams as 20 indexed 56×56 surfaces with their 64-entry
  BGR555 palettes retained. The startup planner no longer emits its former
  synthetic 10×10 host rectangles or borders; it retains only the PLRD
  portrait ordinal. The real-data regression now requires the authenticated
  European FACE.BIN identity, and the production loader's old 24-entry bound
  has also been removed; it now admits only the authenticated 20-record layout.
  Bind the real Saturn VDP1 destination, scale, flip and command order before
  restoring portrait draw commands.

  2026-08-06: launcherns full-start receipts kräver nu exakt full FACE-täckning
  utan fallback; partiell `loaded + fallback == expected` räknas inte längre
  som real-ready.

- **NEXUS-STARTUP-V2-LABEL:** The launcher no longer advertises a Nexus V2
  presentation fallback. Keep the game card capture-gated until the real
  Saturn title/menu owner and VDP1/VDP2 composition are authenticated.

- **NEXUS-LOGOBG-VDP2-CAPTURE:** The real `LOGOBG.DG2` PP surface is now
  decoded as 320×224 indexed pixels with its 256 BGR555 palette and raw-byte
  provenance. It is retained as an optional startup surface, but VDP2 layer,
  palette-bank, timing and display placement still require an authenticated
  Saturn capture; no host draw is enabled.

- **NEXUS-TITLE-VDP-CAPTURE:** Startup title reveal geometry now uses the
  2026-08-06: a separate title VDP capture-admission bit now gates the
  full-start title-capture receipt; retail TITLE.CG/TITLE.BIN bytes alone
  cannot make M11/M12 advertise a drawable title package.

- **NEXUS-DGN-MATERIAL-VDP1-CAPTURE:** The real Structure2 decoder now
  2026-08-08: the large real-data readiness regression now asserts the
  authenticated 16-level parse plus the current blocked/no-draw contract;
  its removed pre-capture READY_MESH expectations are legacy diagnostics only.

- **NEXUS-STONE-PP-VDP1-BINDING:** The missing image-local `STONE.BIN`
  decoder is now implemented from DMWeb `DecodeRawPPpp`: all eight retail
  32×32 records, 16-entry big-endian BGR555 palettes and 512 packed texel
  bytes are validated and a selected record can be decoded to caller-owned
  buffers. The decoder does not create a global palette or authorize drawing;
  bind each record to the DGN/VDP1 material consumer before viewport use.

- **NEXUS-HUD-SATURN-DISPATCH-CAPTURE:** The real DM.BIN hit-rectangle
  2026-08-06 follow-up: level loading and current-level validation now use the
  same `NEXUS_MAX_LEVELS` bound as the verified 16-level corpus; no duplicate
  literal `15/16` limit remains in the production game/engine path.

- **NEXUS-HUD-PANEL-CORPUS-IDENTITY:** The champion-panel regression now
  requires the authenticated European DM.BIN before accepting stat-bar,
  inventory-slot and equipment-slot geometry. Continue binding those real
  rectangles to Saturn input/VDP1/VDP2 consumers; geometry alone is not a
  drawable HUD proof.

- **NEXUS-STARTUP-MENU-CORPUS-IDENTITY:** The startup/menu source regression
  now requires the authenticated European DM.BIN before accepting its loader,
  FONT256, STABG and VDP2-register receipts. These are source ownership facts;
  menu order, text placement and Saturn presentation remain capture-gated.

- **NEXUS-SPELL-ACTION-CAPTURE:** DM.BIN spell tables and effect constants
  remain available as disassembly evidence, but the standalone host effect
  helper is now test-only. Bind the Saturn spell action, status writes,
  projectile DMA and HUD feedback before exporting any effect mutation.

- **NEXUS-FOUNTAIN-ACTION-CAPTURE:** The former public fountain helper was a
  DM1-shaped synthetic mutation path: caller-supplied type/coordinates and
  restore values could change water, health or mana without a retail Nexus
  fountain record or Saturn action/effect trace. Registration and drinking are
  now fail-closed/no-op; recover the real DGN/DM.BIN consumer before restoring
  any fountain state or effect.

- **NEXUS-SAL-MAP-DISPATCH-CAPTURE:** Retail `SNDLEV##.MAP` byte-zero
  2026-08-06 SLEV follow-up: the legacy host event-selector setter is now
  inert until a complete Saturn event→MAP/SDDRVS capture authenticates the
  dispatch owner; numeric host selectors cannot even populate the diagnostic
  route.

- **NEXUS-DGN-CORPUS-IDENTITY:** The DGN level-content regression now requires
  the 16 production European `LEV00-15.DGN` MD5 identities before its
  item/decoration/sensor census is accepted. Continue binding those real
  Structure1F records to the Saturn object/trigger consumer; counts alone do
  not prove loot, pickup, sensor dispatch or viewport ownership.

- **NEXUS-ITEM-CORPUS-IDENTITY:** The real Structure1F→ITEM.IBS coverage
  regression now requires the authenticated European `ITEM.IBS` MD5 before
  accepting descriptor/ floor-image coverage. Continue binding the raw DGN
  item declaration to the Saturn action/slot consumer; source bytes alone do
  not prove loot or pickup semantics.
  2026-08-06 follow-up: the generic caller-supplied `nexus_floor_drop()`
  mutator is now fail-closed; only explicit DGN Structure1Fa source admission
  can populate the diagnostic floor corpus, while pickup/drop writeback still
  requires Saturn action capture.

- **DM1-FMTOWNS-STARTUP-ANIMATION-MENU:** The real DM1 FM Towns BIN/CUE is
  2026-08-08: FM Towns action menu click handling closed. The M11 input
  handler now hit-tests clicks against the source-locked 87×45 panel at
  (232,77)–(318,121) with three 7-pixel-tall rows (CHAR_Y_HYT=7),
  dispatching to `M11_GameView_TriggerActionRow`. Disabled slots (0xFF)
  are respected. The PC34 route-table path is skipped when FM Towns is
  active. Test: `test_dm1_v1_fmtowns_menu_click_geometry`.

- **DM1-PLATFORM-ATARI-ST-PIXELS:** DM1 Atari ST 1.0a/1.0b/1.1/1.2/1.3
  2026-08-08: the M11 asset loader now has a dedicated Atari ST DMCSB1 path
  (`M11_AssetLoader_InitDm1AtariStFromFile`) that opens the 563-record
  DMCSB1 container, decompresses Atari-LZW per record, and decodes the
  big-endian IMG1 pixels through the shared legacy decoder. The DM1 graphics
  binding chain in `m11_game_view.c` tries PC34 IMG3 → legacy LE (FM Towns)
  → legacy BE (Amiga) → Atari ST DMCSB1, so all four GRAPHICS.DAT formats
  are now live. DUNGEON.DAT is byte-identical across PC/Amiga/Atari ST and
  the loader auto-detects endianness. Amiga and Atari ST DM1 are now
  launch-ready when their game data archives are available.

- **DM1-BLOCKED-STEP-AUDIO-DAMAGE-HUD:** The DM1 runtime now owns the remaining
  wall-step audio and damage-overlay corrections: blocked step commands emit
  ReDMCSB `C00_SOUND_METALLIC_THUD` through authenticated source audio, the
  host-drawn yellow attack-X is removed, and C015/C016 damage text is converted
  from the original F0053 baseline to the native font's first-row coordinate.
  The inventory C016 path and food/water labels also refuse host-colour or
  host-text substitutes when their source records are unavailable.
  2026-08-06: the F0623 damage-number text now uses source `C15` foreground
  and `C08` red background; the previous orange host slot was incorrect.
  Keep the packaged Mac capture open until a real wall bump and creature-hit
  capture confirms the source sound and C015/C016 placement. The former red
  viewport damage frame is now suppressed for authenticated DM1 frames because
  ReDMCSB has no corresponding viewport primitive; it remains diagnostic-only.

- **NEXUS-MNS-SATURN-RENDER-HANDOFF:** Retail MNS admission now validates
  DMDF block size plus MOTN/TEXT section envelopes, and the real corpus decodes
  30/30 models with non-zero meshes and 815 source textures. Remaining work is
  still the Saturn/VDP1 command-order capture that binds these source faces,
  materials, palettes and poses to the live viewport; do not promote host mesh
  output to a parity screenshot without that capture.
  2026-08-06: the canonical Track 1 hash catalog now admits all 30 retail MNS
  identities; `nexus_v1_load_model("SCORPION.MNS")` reaches the DMDF model pool
  in the real Track 1 readiness probe. The source-only viewport remains
  capture-gated.

- **NEXUS-PRS3-INVALID-REFERENCE-GATE:** PRS3-avkodaren avvisar nu framtida,
  ogiltiga backreferenser utan att skapa nollfyllda pixlar. Verifiera fortsatt att nya
  källkorpusar följer DMWebs fönsterregel; palettbindning, VDP1-upload och
  menyrendering är fortfarande capture-gated.

- **NEXUS-SMAP-LVMP-STRICTNESS:** LVMP-dekodaren avvisar nu tilemap-bit 0,
  paletteord utan DMWebs bit 15 och tile-index utanför den verkliga tileseten.
  Alla 16 retailkartor passerar; VDP2-placering och explored-state-skrivning är
  fortfarande capture-gated.

- **NEXUS-HUD-SATURN-DISPATCH-CAPTURE:** `DM.BIN` now supplies the runtime
  hit-rectangle table and a raw, order-preserving screen hit-test. Keep this
  API placement-only until an original Saturn/VDP1 capture binds each region
  to the ring-menu command, pad/mouse event, or viewport action. In
  particular, overlapping viewport rectangles must not be resolved by a
  guessed priority outside the retail table order.

- **THERON-V1-VIEWPORT-REAL-DATA:** the old inferred viewport renderer is now
  explicitly fixture-only and excluded from the production archive. Replace
  its no-op seam only after one original Track 02 capture binds the actual
  square-to-tile/map consumer, tile-bank bytes, VCE palette route, UI chrome,
  and viewport destination together. The existing level grid and font bytes
  are not sufficient evidence; do not revive the fixture tile table or its
  fallback geometry.

- **DM2-PLATFORM-MEDIA:** Keep the DOS, FM Towns and Amiga boot paths
  the original quit route. The 1+2+14 SD records and their SO scheduling are
  retained in the source streams, but no Amiga mixer is connected yet.
  full GAME_LOAD/runtime ownership remains separately gated. **2026-08-08

- **DM2-DOS-MVE-PLAYBACK:** The selected PC English install now produces a
  **2026-08-06 FM Towns GDAT gate update:** the bounded format classifier now
  also requires HME-242's actual 3,407-entry raw catalogue. A made-up
  0x8004 header and plausible buffer size therefore cannot enter even the
  format receipt; the boot profile retains the stricter full-media identity
  gate.

- **DM2-RESURRECTION-OWNERSHIP:** Production type-0x0D resurrection remains
  **2026-08-07 real-corpus census:** all eight supplied PC-DOS
  `sksave0..3.dat/.bak` files decode to zero source `c_tim` records with
  `ttype == 0x0D` at the SKProject `c_timer.h` offset `0x04`. The census is
  read-only evidence, not permission to synthesize a resurrection timer or
  promote the test-only phase callbacks; the complete owner remains open.

- **NEXUS-SATURN-PRESENTATION-HANDOFF:** Nexus production no longer contains
  palette and cadence remain unbound until the Saturn text/HUD consumer is
  captured.
  Title timing remains a host state receipt only; the former synthetic edge

- **DUNGEON-STUDIO-FSDUNG:** Dungeon Studio: import original dungeons from
  all five games (DM1, CSB, DM2, Theron's Quest, DM Nexus) and export to a
  custom binary `.fsdung` format. The format is a superset of all games'
  dungeon structures (maps, things, tiles, creatures, items, sensors, doors).
  Importers per game read from DUNGEON.DAT (DM1/CSB), DM2 DUNGEON.DAT
  (skproject layout), Theron Track 02 level records, and Nexus .DGN files.
  Export produces a single portable binary file.

- **DM2-REAL-RUNTIME-CORPUS:** The former runtime-handoff smoke fixture is
  **2026-08-06 world-state boundary:** the partial `dm2_v1_world_state`
  SKSave projection is test-only. It has no M11/runtime caller and does not
  own SKProject's continuous SUPPRESS stream, so it cannot be a resume route.

- **THERON-V1-TRACK02-HANDOFF:** The production viewport now has a
  2026-08-06 archive hygiene: the legacy DMWeb creature-table translation unit
  is now excluded from `firestaff_theron`; it remains available only to the
  explicit diagnostic/fixture table tests.

  THIEF/DEMON scripted encounters no longer use approximated template stats;
  bind their source encounter records before admitting them to combat.

  Removed the public synthetic creature-name aliases used by old tests;
  tests and probes now name the seven real Track 02 dungeon creatures.

  Roster initialization no longer derives portrait IDs from party slots;
  bind actual portrait tile records before exposing portrait graphics.

- **THERON-TRACK19-RECORD-INTAKE:** Verified US/JP Track 19 ISO identities
  admitted. This is a loader-integrity fix only, not semantic promotion of
  unbound records.
  both metadata-verification flags; later record semantics remain closed.
  and later-level records remains unproven, so runtime publication stays closed.

- **THERON-LEGACY-ASSET-PARSER-CLEANUP:** The unreachable THG3/THS4 tile
  parser body has been removed. These Firestaff-only marker formats remain
  rejected; future real bitmap/palette work must enter through an
  authenticated Track 02 loader route.

  The dead implementation body is now also removed from the C source; only
  the explicit rejected API and diagnostic wording remain.

- **THERON-QUEST-BLOCK-RECORDS:** v3.0.215: Binary analysis proves blocks 2-5
  contain graphics/tile data (225-227 KiB nonzero each), block 1 is System
  Card BIOS + credits, block 6 is 0xFF padding, block 0 is empty. Level data
  and descriptor tables are in the post-block code area (UD 0x1C0000+), not
  in quest blocks. Block 3 has a compression signature (0x14 marker, C1/C5/C7
  prefix bytes). Exact tile format and decompression algorithm require
  HuC6280 disassembly of the graphics driver routine.
  The former `theron_v1_tile_descriptor.h` was removed on 2026-08-06: its
  claimed `$43E4`/`$4914` routines and `$4DC6` 32-byte records are absent from
  the versioned HuC6280 disassembly, and no production or test caller used the
  header. No tile/material semantics are promoted by this cleanup; bind the
  actual graphics consumer and VRAM/palette trace before adding a replacement.

- **THERON-V1-UI-CHROME-REAL-DATA:** *(Partially resolved v3.0.246)*
  2026-08-06 host-frame cleanup: M11 no longer paints its synthetic layout
  border over an authenticated Track 02 startup atlas. The border remains
  available only on the explicit no-media/fixture route; the real HUD frame
  still needs its source-owned tile bank and geometry.

- **THERON-V1-VIEWPORT-REAL-DATA:** The initial 32×27 Track 02 grid is now
  byte-faithful but its square values have no proven tile/material meaning.
  Keep the inferred depth tile table and VGA index reduction behind the
  existing no-draw gate until original loader/VRAM evidence identifies the
  dungeon tile bank and palette mapping. The verified level header seed is
  now retained in the level record, together with the opaque header level
  index; tile semantics remain unresolved. The viewport remains no-draw until
  an authenticated tile/material/palette route exists.

- **THERON-V1-TILE-RENDERER-REAL-DATA:** The inferred square/depth tile
  table is now fixture-only. Production exposes no tile selection, decode or
  raster pixels until Track 02 tile-bank semantics are decoded. The viewport
  helper now also returns no tile in production instead of exposing the
  inferred table through a public utility call. The obsolete fixture-only
  tile renderer and its synthetic probe are now removed; remaining work is
  an authenticated Track 02 tile-bank/material binding, not another inferred
  decoder or generated tile surface.

- **THERON-V22-LOCAL-ART-REAL-DATA:** The local modern-art manifest/cache
  and inplace rectangle renderer are now fixture-only. Do not reconnect them
  until source-owned Track 02 V2.2 records and pixel assets are decoded.
  The retired magenta checkerboard bytes have been removed; missing art now
  remains an explicit no-draw result. The remaining work is real Track 02
  tile/palette ownership and loader/VRAM evidence, not another fallback image.

- **THERON-V1-VIEWPORT-MAPPING-REAL-DATA:** The viewport's duplicate inferred
  tile table is now behind an unconditional source-mapping gate. Bind the
  real square-value/depth/material mapping before enabling it.

- **THERON-V1-CHAMPION-STATE-REAL-DATA:** *(Partially resolved v3.0.234-250)*
  randomGeneratorSeed; seeds are likely in PCE code, not data.
  Remaining: portrait graphics from tile banks.
  unproven media. Remaining roster work is the US text/portrait consumer and

- **THERON-STARTUP-RECEIPT-REAL-DATA:** The no-data startup receipt and its
  placeholder labels are now fixture/probe-only. The verified receipt no
  longer projects fixture mirror counts or fallback-label counts; runtime
  must consume the verified startup media receipt or remain unavailable.
  Receipt summaries keep mirror portrait/class/fallback-label fields empty;
  decoded Track 02 text names do not authorize inferred champion metadata.

- **THERON-STARTUP-LEGACY-FALLBACK-ROOM:** The M11 production path uses
  `verified_only` Track 02 admission and the authenticated startup bitmap
  atlas. The runtime-entry fallback-room branch is now compiled only for the
  startup-flow fixture probe; production returns unavailable until a decoded
  Track 02 level is bound. The standalone room generator remains a legacy
  fixture API and must not be used for launch.

  The save/resume contract test now links that fixture-scoped runtime entry
  explicitly, so its structured fallback assertion cannot accidentally test
  the production no-fallback object from the static archive.

- **THERON-LEGACY-ASSET-VERIFY:** The generic asset loader has no authoritative
  SHA256 catalog. Its verification API now rejects supplied digests instead of
  returning a false success; callers must use the hash-bound Track 02 boot
  path. A missing legacy asset path now also returns explicit `NO_DATA`
  instead of reporting success with removed procedural defaults. A discovered
  but unparsable legacy Track 03/04 payload now fails explicitly as well.

- **THERON-SRM-CHAMPION-NAMES:** SRM body import now rejects an empty source
  name instead of synthesizing `Theron`/`Companion`; valid real champion name
  bytes are still required for party admission.

- **THERON-SRM-PROGRESSION-ONLY-PARTY:** Progression-only SRM Continue no
  longer revives the world's fixture Theron slot. A party is admitted only
  when the SRM contains decodable champion records.

  The decoded-party importer also no longer seeds the legacy fixture roster
  before reading source champion records; malformed or partial records cannot
  inherit synthetic names, classes, stats or inventory.

- **THERON-RUNTIME-RENDER-ASSET-BUNDLE:** The runtime frame facade now rejects
  a NULL asset bundle before rendering or presenting a frame. This closes the
  source-less facade path; the save-resume structured-receipt contract is now
  green at `325/325`.

- **THERON-CREATURE-COMBAT-REAL-DATA:** *(Open — RNG consumer missing)*
  2026-08-06 source-object update: every decoded map-reachable category
  4–10/14/15 occurrence is now copied into a persistent world source bank
  with raw bytes, chain links, category/index, position and map coordinates.
  Host object, inventory and item-kind publication remains deliberately
  unbound until the original ownership consumer is proven.

- **THERON-V22-CELL-RECTS:** The V2.2 shape-cache rectangle table still uses
  placeholder 1920×1080 cell coordinates. It is now excluded from the
  production archive; keep the cache available only to its focused fixture
  targets until an original viewport coordinate/material handoff exists.

- **THERON-V22-SHAPE-BOOK:** The inferred V2.2 material/shape book remains
  fixture-only. Production now exposes only a no-op initialization seam;
  bind real Track 02 tile/material records before enabling any modern shape
  consumer.

- **ALL-GAMES-NO-PLACEHOLDER-WHEN-SOURCE-EXISTS:** Active 2026-07-30.

  - 2026-07-31: CSB graphics discovery is now hash-only for both recognised
    and unknown launcher hints. Continue auditing remaining CSB import and
    renderer boundaries for filename-, fixture-, or fallback-derived media
    that could reach a live runtime without a source receipt.

  - 2026-07-31: CSB monster projectile admission now rejects source-undefined
    attacks instead of substituting Fireball. Continue replacing only live
    placeholder behaviour; fixture builders and isolated no-draw contracts
    remain test support rather than production media.

  - 2026-07-31: The CSBWin save-shape byte builder and its convenience runner
    now compile only into tests and the verification probe. The M10 boundary
    accepts caller-supplied save bytes only; continue auditing the remaining
    CSB fixture-marked modules for the same production separation.

  - 2026-08-06: The FSSB export/import wrapper and its Utility transaction
    are also test-only. They reconstruct a CSBGAME-shaped party buffer and
    cannot stand in for an original CSBGAME/CSBWin save; production continues
    through the authenticated Atari/Amiga/CSBWin resume readers.

  - 2026-08-06: The compact `CSBGAME` roster reader is likewise test/probe-
    only. It has no complete original GAMEBLOCK body, so production save
    discovery classifies raw bytes without admitting a roster as a resume.

  - 2026-08-06: Portrait-only CMP helpers are test/probe-only. Runtime CMP
    admission stays limited to a verified portrait/name/title overlay on an
    already authenticated champion; a CMP may not manufacture party state.

  - 2026-08-06: The CSB Atari/Amiga hidden-item safety loader is test/probe-
    only until a live platform renderer consumes it. Its real-media probes
    retain the dmweb/ReDMCSB hidden-code checks, but M10 has no caller and
    must not advertise a replacement bitmap route.

  - 2026-08-06: The D2L/D2R and D3L2/D3R2 viewport wall traces are test-only.
    They encode ReDMCSB order and local pixel probes but load no verified CSB
    bitmap; only an authenticated viewport material consumer may reintroduce
    either route to M10.

  - 2026-08-06: The D1C F0115, D1L2/D1R2 F0111, D2L2/D2R2 F0111/wall and
    D3C F0107/F0108 viewport traces are test/probe-only. They retain source
    order evidence, not a production renderer. Reintroduce them only with
    authenticated GRAPHICS.DAT material and an M11 consumer.

  - 2026-08-06: CSB's shared cache may hold several admitted editions. A31M's
    TITL.DAT can discriminate its selected `csb-amiga31-multi` cache package,
    but cannot rewrite a verified PC34 pair in a generic root.

  - 2026-08-06: The normal M12→M11 route now materializes every
    archive-selected CSB edition into its version-private cache, including
    the scanner's first match. This keeps A31M's paired title/program media
    from inheriting generic PC34 cache bytes; remaining Amiga title/entrance
    presentation capture is tracked separately.

  - 2026-08-07: The combined launcher regression now skips its PC-only V2
    lane when the supplied root contains no M12-launchable PC34 package. A
    pre-existing materialized PC receipt must not turn the independent A31M
    selected-package handoff into a false test failure.

  - 2026-08-07: The original A31M program receipt is now materialized and
    hash-verified. M11 presents its native title and correctly pauses at
    APPB's language-selector boundary; `KAOS.FTL` is now retained and
    verified as the subsequent game program. The APPB language surface,
    selection and persistent language-file transaction still need a
    source-bound presenter before this package can enter gameplay.

  - 2026-07-31: The unbound D0L2/D0R2 partly-open-door and D1L/D1R
    floor/ceiling-ornament contracts now compile only into their tests. Their
    source metadata stays available for verification, but live pixels still
    require authenticated `GRAPHICS.DAT` material.

  - 2026-07-31: The SWSH F0904 palette-animation receipt is test-only; its
    inputs have no product caller or decoder yet. Production remains blocked
    on authentic SWSH palette commands rather than admitting receipt facts.

  - 2026-07-31: The F0908/F0909/F0910 SWSH sound receipt chain is also
    test-only. M11 uses the real-byte decoder directly; no metadata chain may
    claim that host audio or a synthetic buffer is source sound.

  - 2026-07-31: The F0436/F0579/F0807 title/entrance receipt helpers are
    test-only. Live startup remains routed through the authenticated title and
    entrance material path, not caller-invented palette or bitplane facts.

  - 2026-07-31: The F0797 5×5 entrance-layout receipt is test-only. The
    source-owned micro-dungeon remains an explicit startup rendering task, not
    a generic world or viewport substitute.

  - 2026-07-31: F0440/F0902 startup facts are test-only. The live title path
    must obtain decompressed member sizes, FTL pixels and palette from its
    authenticated source binding rather than receipt metadata.

  - 2026-07-31: F0474–F0490 and F0886–F0905 source tables are test-only;
    runtime retains only the real graphics archive route, never blocked-receipt
    bits or ownership strings as a substitute for source material.

  - 2026-07-31: F0906–F0925 is a read-only primitive inventory and is
    test-only. The live SWSH and Utility paths retain their dedicated
    authenticated byte consumers.

  - 2026-07-31: F0846–F0865 has no corresponding ReDMCSB callable and is
    test-only. Keep its fail-closed admission receipt out of M10; source
    material must be bound by an actual CSB runtime consumer.

  - 2026-07-31: F0986–F1005 is a source-boundary inventory, not a graphics
    decoder. It is test-only; live PC 3.4 rendering must use the authenticated
    graphics archive route.

  - 2026-07-31: F1006–F1025 is also a platform/ownership inventory without a
    CSB package-backed consumer. It is test-only; do not promote an ownership
    label, host substitute or foreign-platform route into M10.

  - 2026-07-31: The combined F1048/F1049/F1053/F1055/F1061 compatibility
    wrapper contains only non-PC aliases and explicit no-ops. Keep it out of
    M10; use the shared source-faithful fail-closed boundaries if needed.

  - 2026-07-31: F1186–F1205 documents DM1-owned ANIM routines but has no
    authenticated CSB ANIM stream or runtime consumer. It is test-only; do
    not use the inventory to synthesize CSB title, entrance or UI timing.

  - 2026-07-31: F1206–F1225 is an ANIM ownership inventory which admits no
    route. It is test-only; palette, sound and allocation behavior must come
    from a separately authenticated CSB runtime consumer.

  - 2026-07-31: F1406–F1445 has no ReDMCSB callable symbol and is test-only.
    Local-source labels must never be used as a portable CSB startup or
    entrance implementation.

  - 2026-07-31: F1726–F1765 contains only local, platform and debug source
    labels. It is test-only; do not derive any CSB input or visual behavior
    from this blocked metadata.

  - 2026-07-31: F1886–F1925 is a source receipt inventory. It is test-only;
    its separately implemented save/import owners remain the only admissible
    CSB paths and no hint labels may synthesize presentation data.

  - 2026-07-31: M11 rehashes the selected CSB pair at entry, preventing a
    changed file from inheriting scan-time admission. Continue auditing
    long-lived runtime caches for the same source-receipt lifetime rule.

  - 2026-07-31: Removed the unbuilt `csb_v1_game` shim that carried fixed
    positions and a no-op DM1 import. Continue checking unreferenced CSB
    compatibility surfaces before treating their fixture support as runtime.

  - 2026-07-31: Utility handoff now requires its full imported party receipt,
    not compatibility metadata. Continue auditing ownership fields that could
    describe source state without carrying its real payload.

  - 2026-07-31: The F0435 package-identity regression no longer constructs a
    32-byte stand-in `DUNGEON.DAT`. Its save/load provenance assertion now
    requires the scanner-issued receipt for the hash-verified PC 3.4
    `GRAPHICS.DAT` and `DUNGEON.DAT` pair.
    Continue converting any remaining production-adjacent CSB media fixture
    into an explicit no-data rejection or an external-corpus test.

  - 2026-07-31: The integration plan now names
    `csb_v1_dungeon_load_from_file()` as the runtime file boundary. Continue
    checking newly added callers so the generic in-memory parser remains
    limited to authenticated save images and isolated tests.

  - 2026-07-31: The local hash-verified PC 3.4 corpus now passes the first
  **2026-08-06 M11 gate environment correction:** the real M11 startup/profile
  regression now accepts the shared `FIRESTAFF_DM2_DATA_DIR` root used by the
  other PC-DOS corpus probes. Its default is now the mounted PC-DOS owner
  directory rather than the broad multi-game root, and watchdog expiry is a
  nonzero test failure rather than a false passing skip. This is verification
  discovery only; it neither supplies fixture data nor admits an incomplete
  save/session into runtime.

- **CSB-TITLE-CADENCE:** The M11 CSB title zoom now holds frames 60--79 for
  **2026-08-06 scanner build follow-up:** the shared scanner no longer retains
  an unused DM1 FM Towns admission result while it scans CSB media. The
  admission itself remains mandatory; only its dead local copy was removed so
  focused scanner builds stay warning-free.

- **DM2-M11-GAME-LOAD-ORIGINAL-HANDOFF:** M11 now keeps New Game at the
  **2026-08-08 map-index recovery:** the raw SKSAVE receipt now resolves each
  tile with bit `0x10` through the original `v1e03d8` column-index span and
  `dm2_v1e038c` ground-stack links. This identifies the resident root before
  any stream bytes are consumed. The next transaction phase must restore the
  root's masked DB0..DB3 bytes in place, then restore only empty-tile dynamic
  chains and the final possession continuations.

- **DM2-MERCHANT-CCM-OWNER-HANDOFF:** The coordinate-only NPC route is
  **2026-08-06 production-boundary update:** removed the empty
  `dm2_v1_shop.c` state carrier and coordinate-only
  `dm2_v1_shop_npc_pc34_compat.c` classifier from `firestaff_dm2`. Focused
  shop tests still compile their explicit contracts, but no production code
  can mistake a zero catalog or coordinate flag for a live SHOP_GLASS/CCM
  transaction. **2026-08-08 real-data audit:** the mounted PC-DOS
  `GRAPHICS.DAT` was admitted through the same production field-probe gate;
  no source-owned CCM byteprogram field was found. Merchant enablement still
  requires the live DB4 creature, its b_1a/b_17 record operands, and the
  source merchandise/text owner. **2026-08-13 regression audit:** the M11
  startup gate now rejects arbitrary real-data squares as NPCs and verifies
  that reputation/dialogue state remains untouched until that owner chain is
  connected.

- **DM2-DYNAMIC-CLIGHT-OWNER-HANDOFF:** The map-only dynamic-light fallback
  **2026-08-13 formula correction:** the light helper now preserves
  SKProject's `v1e0978 > 0x0c ? 1 : v1e0978` normalization before the final
  clamp. A large caller value can no longer become an artificial light-level
  delta; the complete live party/save/weather owner is still required for
  dynamic-map admission.

- **DM2-SKSAVE-SESSION-OWNER-HANDOFF:** Original SKSave import currently
  **2026-08-07 source-owner update:** the fixed receipt now names the six-byte
  `c_wbbb`/`ddat.savegames1` section explicitly and centralizes its source
  size. Real saves retain its raw hash only. The surrounding
  `globalb`/`globalw`/hero/timer reads provide no proven scalar gold,
  reputation, or time owner, so those session fields remain unavailable
  rather than being seeded from fixture defaults.

- **DM2-CREATURE-AI-ROW-HANDOFF:** Replace the data-free direct
  **2026-08-08 build-hygiene update:** the production AI-loop translation unit
  is warning-clean under `-Wall -Wextra`; removed dead local word helpers and
  made the deliberately unbound wound/tick ABI explicit. This changes no AI
  decision or callback ordering. The live DB4/CAII/CCM handoff remains open.

- **DM2-LEGACY-GAME-LOOP-DATA-ADMISSION:** `src/engine/firestaff_game_loop.c`
  **2026-08-06 object-model boundary:** the standalone
  `dm2_v1_object_model.c` parser is no longer linked into `firestaff_dm2`.
  Its loader-backed inspection remains available only in the dedicated probe;
  the inferred sequential-pool branch is removed and cannot become an
  alternate GAME_LOAD/M11 object-data owner. Re-admit it only after its full
  G1 chain is consumed by a live source-owned runtime route.

- **DM2-ACTUATOR-SHOOTER-OWNER-HANDOFF:** Port the actual shooter actuator's
  **2026-08-13 real-data census:** the canonical PC-English G1 corpus contains
  four class-`0x08` shooter roots and zero roots for `0x07`, `0x09`, `0x0a`,
  `0x0e` or `0x0f` across all maps. These are source-presence facts only and
  do not authorize DB14/projectile mutation.

- **DM2-ACTUATOR-GENERATOR-OWNER-HANDOFF:** Creature and item generator
  defaults are closed. Port the DB14/DB-record allocation, payload and timer
  ownership from `DM2_INVOKE_ACTUATOR` before any generator can create an
  object or creature from original records. **2026-08-05 update:** the
  remaining wall-mecha compatibility calls are fail-closed too: neither the
  generic creature generator's former fixed HP/tick-direction path nor the
  item generator's generic DB allocation can mutate the live record pools.
  The production runtime now consumes those incomplete generator records
  without inventing a creature, item, placement or timer.
  **2026-08-13 real-data census:** the canonical G1 actuator regression now
  inventories generator roots as well as shops. The mounted PC-English
  `DUNGEON.DAT` contains no direct creature-generator `0x2e` or item-generator
  `0x3c` roots; this absence is recorded as source evidence, not filled with
  host defaults. The DB14/DB-record/timer owner handoff remains required for
  any profile that supplies those actuator classes.

- **DM2-ACTUATOR-RECORD-HANDOFF:** Generic type/coordinate/flag entry and
  **2026-08-13 querydb handoff:** the standalone actuator-type query now reads
  the source DB3 record's `w2` low-seven-bit `ActuatorType()` field through the
  authenticated record callback. It rejects non-actuator classes, ObjectID
  sentinels and missing records; activation, payload and timer ownership are
  intentionally still open.

- **DM2-SHOP-GLASS-OWNER-HANDOFF:** The fixed-coordinate catalog path is
  **2026-07-31 follow-up:** the standalone companion no-op boundary is also
  outside the production archive. It has no live DB4/CAII/CCM caller and must
  remain an isolated rejection contract until that complete source route is
  imported.

- **DM2-OBJECT-TEXT-OWNER-HANDOFF:** Bind leader-hand/item text only through
  **2026-08-13 owner-receipt update:** added a source-bound read-only bridge
  that requires a validated DB5..DB10 record in the record pool, derives its
  CLS1/CLS2 with the SKProject record routines, and only then queries the
  original GDAT name stream. The real PC-DOS corpus resolves DB5 subtype 3
  through `WEAPONS/3` to `KALAN GAUNTLET`; M11 still does not admit the route
  until its live leader-hand ObjectID owner is connected. **2026-08-13
  real-record audit:** the mounted G1 DB5 object `0xD407` resolves to
  `WEAPONS/126`, but the selected PC-DOS GDAT has no admissible text for that
  index; the test now verifies that the HUD remains unnamed instead of
  inventing a label.

- **DM2-DIALOGUE-TEXT-OWNER-HANDOFF:** Save/load-dialogue labels require the
  **2026-08-07 real-data tightening:** the mounted PC-DOS viewport regression
  now requires both non-zero raw source-text hashes in addition to the
  displayed `SAVE`/`CANCEL` labels.

- **DM2-V22-REAL-MATERIAL-HANDOFF:** Missing V2.2 art now returns no pixels.
  covered by an opt-in real-data regression; GRAPHICS.DAT presence without a
  modern-art manifest remains `NO_MANIFEST` and cannot promote V2.2 pixels.

- **DM2-STEP-MISSILE-OWNER-HANDOFF:** Port the source-owned DB14/timer
  or turning a DB14 record into a viewport projectile; other profiles still
  require the complete `DM2_STEP_MISSILE` owner handoff.

- **DM2-SKSAVE-ORIGINAL-WRITER:** `dm2_v1_world_state_serialize()` is now
  **2026-08-07 corpus audit:** the supplied DOS data root contains eight
  original `sksave0..3.dat/.bak` files (51,521–51,574 bytes). The real-data
  suite passes 126/126 and confirms the source raw prefix, fixed SUPPRESS
  order, timer boundary, DB-pool receipts and fail-closed runtime handoff;
  the focused source-order orchestrator passes 5/5. These are diagnostic
  proofs only: no live writer owner exists yet for the complete graph.

## Active DM1/CSB Symbol Queue

## Priority Cross-Game Implementation Queue

Run these in order with at most five workers. A worker must verify the route
is still open before starting it, reuse original game material where it is
available, and move a completed item to DONE.md only after focused tests and
an integration build pass.

### Original-data replacement inventory (2026-07-30)

This is a production-runtime inventory only.  Synthetic test fixtures,
receipt-only probes, and deliberate fail-closed/no-draw boundaries are not
replacement targets.  A target is closed only when the runtime consumes the
listed local original material and a focused real-data test proves it.

**Policy:** this includes every user-visible placeholder: generated menus,
flat-colour/chrome fallbacks, diagnostic sprites, checkerboards, substitute
icons, and temporary text.  When authenticated original game bytes exist in
the selected game's data set, Firestaff must decode and use those bytes.  A
missing or still-undecoded original format must fail closed with a specific
diagnostic; it must not silently fall back to a generated visual.

- **CSB-ORIGINAL-REPLACE-001:** Replace the remaining V2.2 viewport
  **2026-07-31 D2C partly-open-door follow-up:** removed the duplicate,
  contract-only F0111 metadata/probe surface. It had no M11 consumer and no
  original bitmap decoder; the active D2C F0111 source-material route remains
  the sole production owner.

- **DM2-ORIGINAL-REPLACE-001:** Replace the V1 viewport's placeholder wall
  **2026-08-06 V5 corpus correction:** canonical PC `GRAPHICS.DAT` does have
  an authenticated FB/FC/FD chain for `CREATURES/02/dtImage/12`; its current
  G1 maps do not contain a root that owns it. Keep every present G1 creature
  fail-closed until its own DB4 record/animation chain is bound; do not infer
  a sprite from this unrelated real material.

- **DM2-ORIGINAL-REPLACE-002:** Replace the V2 HUD's synthetic 1x1/overlay
  **2026-08-06 cache boundary update:** removed the remaining local
  `v22_inplace_cache.bin` reader from the production V2.2 compatibility
  module. It was already unable to draw, but it still admitted locally
  generated RGBA bytes into memory. The API now stays explicitly inactive and
  no-draw without opening a cache file. Future V2 work must start with an
  original-data provenance policy, not revive the generated-art cache.

- **THERON-ORIGINAL-REPLACE-001:** Replace the coloured UI chrome, checkerboard
  palette and chapter-marker placeholders with real US/JP Track 02 bitmap,
  palette and loader-selected records.  The supplied `TQUS02.bin`, CUE and
  ISO corpus are the admission sources; unknown records remain blocked.
  The production archive boundary now rejects every currently excluded
  inferred/procedural Theron translation unit, so future source bindings must
  enter through an explicit real-data review rather than a broad glob.

- **THERON-ORIGINAL-REPLACE-002:** Replace item-as-creature and direction-bar
  viewport placeholders only after the HuC6280 CD-read table binds their
  exact Track 02 records.  No inferred object graphics or palette may ship.

- **ALL-ORIGINAL-REPLACE-001:** Audit startup, title, entrance, HUD and
  dungeon runtime paths for placeholder pixels on every supported game before
  release.  Where matching original data exists under `.firestaff/data`, bind
  it; otherwise make the route visibly fail-closed with a precise missing-data
  diagnostic, never a generated visual substitute.

## Top 30 Implementation Queue

These are the next thirty substantial coding jobs. They refine, rather than
duplicate, the numbered requirements below. Take one only after confirming
that its exact runtime path is not already source-locked and tested.

### DM1

### CSB

15. **Q-CSB-05 HUD and champion panels:** C017/C040 champion, inventory,
    2026-07-25: F0806 entrance loop runtime handoff, F0050 text message
    area print space, and F0425/F0426 dialog symbol tests wired with
    ReDMCSB source-name wrapper macros. 3 new tests pass.

### DM2

31. **DM2-PARITY-GAP-CLOSE:** ~~Implement remaining 92 MISSING skproject symbols~~
    **DONE** — audit shows 1118/1118 applicable symbols at IMPLEMENTED_PARITY.
    All 19 CCM advanced handlers implemented. CREATURE_KILLER actuator wired.
    Remaining work is runtime integration (wiring callbacks into timer
    processing, actuator dispatch, and glob var updates).
    Most remaining symbols need full runtime state bridge (map, timer queue,
    UI, graphics) before they can be wired.

1. **DM1-HOC-RUNTIME-RENDER:** Finish the M11 HoC render consumer for mirrors,
   2026-08-06: the legacy generic DUNGEON.DAT bridge no longer guesses the
   raw-map base from EOF or skips the column/SFT/text/thing prefix. It now
   follows the PC34 header, MAP descriptors, column bases, square-first-thing
   table, text words and G0235 thing byte counts before reading column-major
   squares, and retains door state bits. This removes a source of walls and
   doors being projected from object bytes. Real PC34 layout/state verification
   is covered by `test_firestaff_dm1_dungeon_state_real_data`; broader M11
   viewport capture remains open.

2. **DM1-PC34-FULL-ASSET-VISUAL-AUDIT:** Finish the non-raster source
   2026-08-06: the legacy `firestaff_graphics_dat_reader` now rejects a
   short LZW decode and undersized output buffer instead of copying a partial
   pixel stream. This closes the remaining partial-surface admission found in
   the reader; packaged macOS app capture remains open.

4. **CSB-REAL-STARTUP-HUD:** C001-C005 title, Entrance, door-opening and
  2026-07-30: the supplied standard CSBWin package has the canonical Atari
  `GRAPHICS.DAT`/`DUNGEON.DAT` pair but no `ANIMATE.SCR`/`ANIMATE.DAT`.
  Firestaff now admits its decoded original C001-C005 title/Entrance route in
  V1, V2.0 and V2.1 (V2.2 correctly resolves to V2.1 without an admitted
  pack). It deliberately does not produce a release/runtime receipt: C040
  expands to `144x0`, so the later PC3.4 terminal-HUD and dungeon handoff
  remain blocked until CSBWin's real runtime panel owner is established.
  The shared M11 source-graphic/HUD/viewport decoder now correctly selects
  CSBWin's DMCSB1 decoder for ST20/ST21 instead of attempting PC3.4 IMG3;
  this prepares real runtime ingress but does not prove CSBWin F0128 geometry.
  Real CSBWin HUD dimensions differ from PC3.4: C009=`96x33`, C010/C013=
  `96x45`, C017=`224x136`, C028=`80x14`. A separate source layout is required;
  PC3.4's `87px` and `76px` panel positions must not be stretched or reused.

11. **DONE 2026-07-23 DM1-CHAMPION-MIRROR-RESURRECTION:** C127 mirror
    selection, C160 resurrection, C161 rename/reincarnate, C162 cancellation,
    real C026 portraits, sensor state, party handoff, and HiDPI/fullscreen
    input are verified against PC34 data. Do not reopen without a repro.

21. **CSB-DSA-FULL-OPCODE-FAMILY:** Extend authenticated CSBWin DSA execution
   2026-07-29: direct `CAST` and `FILTEREDCAST` now preserve CSBWin
    Magic.cpp's exact signed 14-word `SPELL_PARAMETERS` payload and defer it
    until the complete authenticated action succeeds. They require a real
    runtime spell owner and reject incomplete payloads rather than reading an
    invented parameter tail. The live Magic.cpp level switch, optional
    spell-filter actuator traversal, and full source spell effects remain
    open; indirect cast routes remain intentionally fail-closed. CSBWin 2023
    `DSA.cpp:4963` serializes `I_CAST` with `INDIRECTP(..., 0, 13)`, while
    `Magic.cpp:1411-1416` copies the 14-word `SPELL_PARAMETERS` structure
    from `pDSAparameters + 1`. Until a real saved indirect record proves the
    source-owned fourteenth word, Firestaff must not recover it from adjacent
    memory or invent a value.

24. **CSB-ORIGINAL-SAVE-CORPUS:** Admit real CSB/CSBWin save corpus, complete
    2026-07-29: the export now also writes the documented, already-decoded
    champion fields in each 800-byte original record (identity, pose/action,
    vital stats, skills/experience, slots, load and shield), re-encrypting
    that section and rebuilding its checksum. The external corpus runner is
    ready to prove a champion-name/health round-trip without touching the
    embedded dungeon once a real saved position is supplied. Remaining scope
    is original dungeon/object/timer mutation and DB11/EXPOOL corpus evidence;
    do not infer them from the PC body layout.

25. **CSB-TITLE-AUDIO-CADENCE:** C001 timing and FTL/PRESENTS/CHAOS/STRIKES
    2026-07-30: a skip-safe, real-PC3.4 title/Entrance regression now captures
    all four source stages and verifies their distinct visible palette/raster
    signatures (PRESENTS, FTL, CHAOS STRIKES BACK, Prison/Entrance). It catches
    a black/flat presentation or palette collapse without using replacement
    art. Original audio media and packaged-app capture remain open.

49. **DM2-REAL-DATA-REGRESSION-CORPUS:** Build hash/provenance-verified DM2
    GRAPHICS/DUNGEON/SKSAVE/SND test corpus and end-to-end runtime regressions
    that exercise the authentic production paths.

50. **DM2-END-TO-END-PLAYABILITY:** Integrate all verified DM2 startup,
    save, HUD, dungeon, scene, input, AI, audio, and transition routes into a
    complete real-data play session with fail-closed unsupported content.

## ReDMCSB DM1 Reference Boundaries (2026-07-13)

These are reference limits, not claims that ReDMCSB logic is wrong. Each item
marks a place where a Firestaff PC34 claim needs evidence in addition to the
ReDMCSB WIP 2021-02-06 source tree.

- REDMCSB-DM1-GAP-001 — **ReDMCSB `Documentation/Readme.htm`, scope and
  terminology sections.** The project states that it is reverse engineered,
  not FTL's original source, and that names are reconstructed from binaries
  and secondary material. **Firestaff risk:** a plausible identifier or C
  expression can be mistaken for a proven PC34 ABI, byte layout, or side
  effect. **Required independent evidence:** a hash-identified original PC
  3.4 executable/disassembly and a minimal runtime trace for every ABI or
  save-layout assertion that depends on reconstructed naming.

- REDMCSB-DM1-GAP-002 — **ReDMCSB `Documentation/Readme.htm`, Accuracy /
  Atari ST.** The supplied Megamax 1.1 rebuilds are deliberately only
  near-identical to FTL's Atari binaries and fail some copy-protection checks;
  the original compiler/linker is unavailable. **Firestaff risk:** source
  control-flow is not binary proof for checksums, address-sensitive code, or
  instruction timing. **Required independent evidence:** the original PC34
  executable plus DOS/emulator execution trace; do not use a ReDMCSB rebuild
  as a checksum or timing oracle.

- REDMCSB-DM1-GAP-003 — **ReDMCSB `Documentation/Engine.htm:10` and
  `Documentation/Readme.htm`, Accuracy / Other platforms.** The engine page
  is explicitly written from Atari ST source, while the other-platform
  accuracy section is marked "TO BE COMPLETED". **Firestaff risk:** an ST
  render, palette, disk, or input branch can be silently promoted to PC34.
  **Required independent evidence:** PC 3.4 English media, executable
  disassembly, and frame/input captures; cross-platform branches are
  explanatory only until those agree.

- REDMCSB-DM1-GAP-004 — **ReDMCSB `DUNVIEW.C` F0115 setup, around line 2464
  original-PC custom-dungeon capture remains the blocker for the legacy
  memory-limit behavior.
  the documented platform-memory gap: an original-PC custom-dungeon capture is

- REDMCSB-DM1-GAP-005 — **ReDMCSB `CHAMPION.C` F0306/F0319/F0320/F0321.**
  Firestaff now locks the PC34 F0306 compiler-order branch and M11 owns the
  F0319 one-shot death record: inventory/bones are not duplicated across host
  ticks or an original-save reload that already has its bones record, poison
  lifecycle records clear with the champion, and the champion direction plus
  C026 portrait record remain source-backed. M11 creature melee now publishes
  the actual F0320 C015/C016 champion-damage receipt after its F0321 result.
  **Still required independent evidence:** PC34 executable capture covering
  nonlethal F0321 damage, death, save/reload, bones pickup, and resurrection,
  with recorded portrait/HUD frames and save bytes.

- REDMCSB-DM1-GAP-006 — **ReDMCSB `DUNGEON.C` thing allocation, around line
  2099 (`BUG0_10`).** The reserved champion-bones type uses bit 15 and the
  legacy compiler's shift happens to discard it before indexing; the source
  documents that a normal compiler can index out of bounds. **Firestaff risk:**
  source-shaped C alone cannot define the PC34 allocation semantics for dead
  champion bones. **Required independent evidence:** real PC34 saves and
  runtime traces covering party death, bone allocation, pickup, save, load,
  and export byte comparison.

- REDMCSB-DM1-GAP-007 — **ReDMCSB `LOADSAVE.C` load branch around
  lines 2860-2895.** The code documents a broken historical DM/CSB dungeon
  detector and format/header changes that make older saves impossible to load.
  **Firestaff risk:** ReDMCSB's broad multi-platform loader is not a complete
  PC34 interchange specification, especially for damaged, backup, and
  version-mismatched files. **Required independent evidence:** provenance
  recorded PC34 save corpus spanning new game, HoC selection, deaths/bones,
  active groups, pending events, backup, and rejected/corrupt files, with
  original-load and byte round-trip results.

  - 2026-07-13 update: external-PC34 corpus discovery and per-file receipts
    now retain the exact F7057 five-part envelope endpoint and the untouched
    trailing-byte count that F0435 must consume as portraits/dungeon tail.
    This distinguishes a valid envelope with a later corrupt suffix from a
    malformed F7057 body without decoding or promoting tail bytes. Remaining
    evidence is a provenance-recorded real PC34 corpus across the listed
    gameplay states and original executable load results.

  - 2026-07-13 update: corpus certification now validates the populated
    receipt itself: header/part shape, F7057 boundary, atomic C3 EVENT bytes,
    raw C4 TIMELINE bytes, and the optional F0433 dungeon tail. C13/C24/C25
    are subtype receipts only when source rows exist, so an absent optional
    subtype cannot block independent C3/C4/tail evidence.

  - 2026-07-13 update: PC34 import now rejects any active C3 EVENT omitted
    from C4 before runtime materialization. The focused C13 regression keeps
    a valid rebirth EVENT but substitutes a different active C4 index, proving
    exact rollback provenance instead of silently losing the timer. Remaining
    evidence is still a provenance-recorded real PC34 corpus and original
    executable load results.

  - 2026-07-13 update: corpus discovery and F0435 import now bind to the
    same reclassified byte snapshot. A DMSAVE.DAT replaced after recursive
    discovery is rejected with a diagnostic instead of inheriting stale
    header/envelope provenance. This is transaction hardening only; it does
    not replace the required original PC34 corpus.

  - 2026-07-14 update: `dm1_v1_original_save_pc34_external_corpus` is a
    fixture-free admission target for an explicitly staged corpus. It reports
    each admitted file's source/export hashes, F7057 envelope boundary,
    trailing-tail size, and no-fallback runtime-stage/adoption results; an
    unset corpus root is a non-promoting skip. Remaining evidence is still
    provenance-recorded original PC34 saves and original executable results.

  - 2026-07-14 update: fixture-free external-corpus promotion now requires
    every qualified PC34 candidate to stage an owned F0435 dungeon and adopt
    its EVENTS/TIMELINE queue before it is called runtime-admitted. A
    tail-less byte roundtrip remains diagnostic-only; it cannot borrow a host
    dungeon or satisfy the corpus runtime gate.

  - 2026-07-14 update: the external HoC runtime gate now requires the live
    M11 runtime to retain the `ORIGINAL_SAVE_PC34` viewport origin and emit a
    nonblank, byte-stable 224x136 PC34 viewport crop. HUD chrome cannot
    satisfy this rendering receipt. This records Firestaff consumption of an
    admitted save's live dungeon state; it remains neither a DOS pixel-parity
    claim nor a replacement for provenance-recorded original executable runs.

  - 2026-07-14 update: the same fixture-free HoC runtime gate now requires
    M11's canonical post-adoption world hash to match an independently staged
    F0435 world from the identical external save snapshot. This binds the
    live handoff to its restored party, active state, timeline, and
    dungeon-backed runtime rather than only its pose or viewport. It remains
    host-runtime evidence, not original-PC execution or pixel-parity proof.

  - 2026-07-13 update: F0435 tail validation now verifies the persisted
    per-column cumulative SquareFirstThings table against raw-map thing-list
    flags before M10 can reconstruct its lookup. The remaining requirement is
    still provenance-recorded original PC34 saves and original executable
    results, not a different tail format.

- REDMCSB-DM1-GAP-008 — **ReDMCSB `LOADSAVE.C`
  F1057/F0433/F1059 and `COMMAND.C` save-command checksum gates.** Save
  control flow is wrapped by platform-specific checksum/copy-protection
  helpers such as F0464; the ReDMCSB accuracy note says non-identical rebuilt
  Atari binaries have incorrect checksum values. **Firestaff risk:** source
  call order does not prove original PC34 save UI acceptance, protection, or
  failure presentation. **Required independent evidence:** original PC34
  executable and save-media corpus with recorded save command, produced bytes,
  reload result, and dialog/frame capture.

- REDMCSB-DM1-GAP-009 — **ReDMCSB `MEMORY.C` graphic loading calls to
  F0497_LZW_Decompress and `Documentation/Readme.htm`, graphics compression
  and caching.** The source explains the codec and cache policy, but carries
  no Firestaff-owned proof that a particular PC34 `GRAPHICS.DAT` bitmap,
  palette, title, or sound entry has the assumed identity. **Firestaff risk:**
  valid decoding can still select the wrong asset, palette, frame, or cached
  representation. **Required independent evidence:** hash-identified PC34
  `GRAPHICS.DAT` plus asset-offset/decoded-pixel corpus and original frame
  captures for title, entrance, HoC, HUD, inscriptions, and dungeon cells.

- REDMCSB-DM1-GAP-010 — **ReDMCSB `GRF1.C`, `IBMIO.C`, `IO.C`
  S0075/S0076, and `SOUND.C` F0061.** Low-level graphics, input interrupts,
  mouse state, and sound routes include platform assembly/system calls or
  platform-specific tables. **Firestaff risk:** the high-level source cannot
  prescribe SDL event coalescing, audio scheduling, palette latch timing, or
  host frame presentation. **Required independent evidence:** PC34 DOS/emulator
  input/audio/frame capture with tick markers, then Mac/SDL packaged-app
  comparison; no synthetic timing or sound substitute may be promoted.

  - 2026-07-14 IBMIO.C F8099/F8100/F8111/F8112 now have a source-locked
    PC34 state adapter for lock depth, cursor coordinates and formatted button
    state. Host interrupt scheduling, SDL event coalescing and cursor drawing
    timing remain outside this narrow implementation and still require the
    recorded PC34 evidence above.

  - 2026-07-14 IBMIO.C F8101/F8108/F8109 now preserve handler registration,
    72-byte pointer-slot registration and visible-pointer transition ordering.
    The DOS interrupt, deferred mouse history and host video drawing remain
    callback boundaries and still require the recorded PC34 evidence above.

  - 2026-07-14 IBMIO.C F8123/F8124 now preserve the empty CD-track route and
    the device-specific raw sound-progress query. F8128 remains unavailable:
    its hardware/ISR polling has no portable progress contract without the
    PC34 timing evidence required above.

  - 2026-07-14 IBMIO.C F8129/F8130 now preserve the defined device-type table
    and three-attempt first-sector probe through host I/O callbacks. The DOS
    IOCTL/BIOS transports and source-undefined device types outside 0–7 remain
    external evidence boundaries.

  - 2026-07-14 IBMIO.C F8131/F8132/F8133 now preserve FAT-label filtering,
    DOS-time `DX` seed packing and the empty floppy route. DOS FCB/DTA lookup
    and clock provenance remain host callback boundaries requiring PC34 proof.

  - 2026-07-14 IBMIO.C F8134 now preserves the defined DOS EXEC command-tail
    prefix and normal-termination result. Program loading and DOS exit status
    remain callback boundaries requiring PC34 proof.

  - 2026-07-14 NEC816.C F8137 now preserves the MEDIA457_P20JA 4bpp packed
    nibble fill, including odd/even start and bounded write behavior. The
    MEDIA472 byte-per-pixel path and the live framebuffer consumer remain
    separate, unproven work.

  - 2026-07-14 NEC816.C F8140/F8162 now preserve the PC 3.4 source-defined
    overlap-copy direction and 160-byte-stride multi-plane message-area
    transfer. Binding those logical planes to the live Mac/SDL framebuffer
    remains a separate presentation/capture task; no hardware aperture is
    claimed by this narrow adapter.

  - 2026-07-14 VIDEODRV.C/NEC816.C F8151 now preserves the PC 3.4 C25
    source-bitmap-to-aperture rectangle path, including source/destination
    even strides, transparency and vertical flip. C25 F0681/F0683 are empty,
    so horizontal flip deliberately remains a source-defined no-op. The F8143
    C25 aperture-to-bitmap primitive is now separately ported; live framebuffer
    presentation remains unproven.

  - 2026-07-14 VIDEODRV.C F8152 now preserves the PC 3.4 C25 inclusive
    rectangle fill through F8137, including fixed 320-byte rows and the
    viewport color-index offset. The live SDL/Mac aperture binding remains
    separate.

  - 2026-07-14 VIDEODRV.C F8143 now preserves the PC 3.4 C25 aperture-to-
    packed-bitmap readback with low-nibble extraction and retained opposite
    boundary nibbles. It is a standalone aperture primitive until the live
    host framebuffer supplies the C25 surface.

  - 2026-07-14 VIDEODRV.C F8154 now preserves the PC 3.4 C25 inclusive
    aperture inversion rectangle, XORing exactly `0x04` at `(y * 320) + x`.
    Its live Mac/SDL aperture consumer remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8155 now preserves the PC 3.4 C25 hatch box:
    `((x ^ y) & 1) == 0` aperture pixels clear to zero and the others are
    retained. Live Mac/SDL aperture consumption remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8167/F8168 now preserve the C25 mouse-pointer
    background lifecycle: a real aperture snapshot clamped to 18x18 and the
    screen edge, then direct F8166 restoration. Live mouse compositing and
    host framebuffer binding remain separately unproven.

  - 2026-07-14 VIDEODRV.C F8169 now preserves the C25 blackening animation's
    real 16-bit LFSR order, including its explicit final write to pixel zero
    and `G8177 | black` aperture value. The live host framebuffer consumer
    and frame pacing remain separately unproven.

  - 2026-07-14 VIDEODRV.C F8163 now preserves the C25 caller-bitmap transfer:
    the original packed 4bpp source indices and destination aperture indices
    flow unchanged into F0680. Binding that aperture to the live SDL/Mac
    framebuffer remains separately unproven.

   - 2026-07-14 VIDEODRV.C F8213 now preserves C25's direct one-byte aperture
    write: `G8177 | color` goes to the requested `G8134` pixel index. Binding
    this logical aperture to live SDL/Mac presentation remains separately
    unproven.

  - 2026-07-14 VIDEODRV.C F8153 now preserves C25's two-phase 0x3DA vertical
    blank poll: leave an active blank, then wait for the following one. The
    live SDL/Mac VBlank-status provider and frame presentation remain
    separately unproven.

  - 2026-07-14 VIDEODRV.C F8139 now preserves C25's direct packed-4bpp
    source-to-aperture loop, including source parity and bytewise G8177 OR.
    Binding the logical aperture to live SDL/Mac presentation remains
    separately unproven.

  - 2026-07-14 VIDEODRV.C F8137 now has its C25 VGA aperture-fill variant:
    direct `G8177 | color` bytes over the real 320x200-compatible aperture.
    Its runtime call-site consumption and live capture remain separately
    unproven.

  - 2026-07-14 IMAGE3.C F0684 now preserves the PC 3.4 C25 packed-bitmap
    source-to-aperture route for all four flip modes and transparency. It
    consumes the direct 320x200 host-compatible aperture; live game call-site
    capture remains separately unproven.

  - 2026-07-14 DUNVIEW.C F0675 now preserves native/derived/temporary bitmap selection before real F0129 scaling; M11 call-site binding remains unproven.

  - 2026-07-14 VIDEODRV.C F8216 now preserves C25's forward aperture copy
    from one 320-byte row above, including its source-visible propagation for
    spans larger than a row. Binding this logical aperture effect to live
    SDL/Mac rendering remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8230 now preserves the C25 single-colour palette
    mutation: real RGB4 components become `(component << 2) + 3` RGB6 bytes,
    then publish through F8156 only while the curtain is active. The live
    SDL/Mac DAC consumer remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8166 now consumes the real C25 F8165 aperture
    snapshot format directly: three 16-bit prefix words and raw 320-stride
    rows are replayed without palette conversion. Binding the host aperture to
    the live SDL/Mac framebuffer remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8165 now preserves C25's real aperture snapshot
    layout: a 6-byte partial-box prefix followed by raw byte-per-pixel rows at
    the original 320-byte stride. The capture source is still an explicit host
    aperture; live SDL/Mac framebuffer binding remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8156/F8157 now preserve the PC 3.4 C25 RGB6
    palette bytes: F8157 updates only terminated table entries whose index is
    below 32, and F8156 publishes all 32 rows only after a host VBlank gate
    when the curtain is active. This does not fabricate a VBlank or a palette:
    M11/SDL consumption of the verified DAC rows remains separate work.

  - 2026-07-14 VIDEODRV.C F8159 now preserves the PC 3.4 C25 RGB6 curtain:
    black waits at the caller-owned VBlank boundary and writes 32 zero rows;
    normal restores verified source RGB6 bytes through F8156. SDL/Mac DAC
    consumption remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8160 now preserves the PC 3.4 C25 creature palette
    mutation: six existing palette-table rows receive only the RGB6 bytes from
    one real `G8175_CREAT_PAL` set. Publishing that altered logical palette to
    the live SDL/Mac DAC remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8161 now preserves the PC 3.4 C25 viewport
    source-to-aperture consumption: it applies the original `0x10` RGB bank
    only to an opaque, unflipped `224 -> 320` F8151 blit. Binding this logical
    aperture to the live SDL/Mac framebuffer remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8158 is not a PC 3.4 C25 task: its source body is
    guarded to EGA/Tandy builds only, so no C25 behavior is invented.

  - 2026-07-14 DM1 V1 spell HUD now routes only through CASTER.C/MENUDRAW.C
    C009/C011/C013 with real GRAPHICS.DAT and original-font gates. Palette
    capture parity remains subject to the PC34 evidence requirement above.

- REDMCSB-DM1-GAP-011 — **ReDMCSB `GAMELOOP.C` lines 171-181 and `IO.C`
  mouse interrupt path.** The source records a platform/version race fix for
  eye/mouth press state between interrupt and command-queue processing.
  **Firestaff risk:** polling SDL input can look correct while diverging on
  press/release ordering, dialogs, chest panels, or save-menu entry. **Required
  independent evidence:** original PC34 input traces for click/hold/release
  around eye, mouth, chest, scroll, resurrection and save commands, plus the
  same scripted sequence through the packaged host app.

- REDMCSB-DM1-GAP-012 — **ReDMCSB `ENTRANCE.C`
  F0438_STARTEND_OpenEntranceDoors, `TITLE.C` F0437, and platform headers.**
  The common source supplies control flow but timing, bitmap presentation,
  palette behavior, and sound backend are selected by platform conditionals.
  **Firestaff risk:** copying the sequence can produce a visually plausible
  but wrong PC34 title/swoosh/entrance cadence or palette. **Required
  independent evidence:** frame-numbered, audio-synchronised PC34 startup
  captures and raw title/animation asset corpus, compared against the packaged
  Firestaff frame stream.

- REDMCSB-DM1-GAP-013 — **ReDMCSB's documented bugs are observations, not a
  modern-engine policy.** `Documentation/BugsAndChanges.htm` records original
  behaviors such as object cloning, timeline exhaustion, and malformed custom
  dungeon crashes, but does not establish whether a safe host should emulate,
  contain, or reject each failure. **Firestaff risk:** either erasing a
  source-visible PC34 behavior or reproducing memory corruption under C11.
  **Required independent evidence:** a PC34 reproduction for the claimed
  release and an explicit per-route emulate/guard/reject decision backed by a
  regression; without it, keep the input bounded and mark the route
  unavailable rather than synthesising a result.

- REDMCSB-DM1-GAP-014 — **Copy-protection and physical-media behavior is not
  a portable game contract.** `CopyProtection.htm`, `GRAPH21.C`, and platform
  disk/I/O paths expose fuzzy-sector and protection control flow, but do not
  provide an authentic PC disk signal, DOS driver timing, or a normalised
  failure contract for a modern filesystem. **Firestaff risk:** a fabricated
  success/failure result can alter startup, free memory, event scheduling, or
  endgame presentation. **Required independent evidence:** an archived
  original PC media image with emulator/real-machine trace; otherwise the
  protection branch remains explicitly unavailable and cannot be replaced by
  synthetic state.

- 2026-07-13 CSBWin restored `TT_FALSEWALL` follow-up: authenticated SET now
  and DSA-free CLEAR/TOGGLE now update the original falsewall cell flag.
  Portrait/DSA-owned squares and parameter-message payloads remain fail-closed.
  CLEAR and the open-wall TOGGLE branch defer only where the saved timer's
  party/nonmaterial-group owner can be retained.

- 2026-07-13 CSBWin restored `TT_24` follow-up: exact saved-object removal
  and free are live only for a validated source Thing chain. Timer-owned
  clouds, source sound/party effects, malformed chains, and all broader
  object-lifetime routes remain fail-closed until their CSBWin owners exist.

- 2026-07-13 CSBWin restored `TT_13` follow-up: the exact final Vi Altar life
  stage and the old-save no-EXPOOL DB10-bones state-1 handoff are live. The
  `packedState` 2 cloud and Wings state-1 branches remain fail-closed until
  those original owners are retained by the imported runtime record. The
  current-save `EDT_ChampionBones` EXPOOL state-1 branch is complete and
  tracked in DONE.

- 2026-07-13 CSBWin restored `TT_53` follow-up: live runtime now retains the
  exact saved watchdog TIMER/queue owner while requeueing its source `+300`
  level-zero successor. Broader watchdog diagnostics remain blocked without
  their complete CSBWin runtime state.

- 2026-07-13 CSBWin restored `TT_65` follow-up: the live queue now restores
  only the saved generator's exact `timerObj8` actuator identity, retaining
  CSBWin's documented first-disabled-actuator fallback for old saves. Broader
  C65 sensor execution and generator materialization remain blocked unless
  their complete CSBWin timer ownership is preserved.

- 2026-07-13 CSBWin restored `TT_75` follow-up: exact saved 8-bit poison
attacks now enter the existing source C75 damage and `+36` requeue chain.
2026-07-14 update: the live event-slot receipt now retains the complete
16-bit `timerWord6` through every `+36` continuation, clearing only when its
exact event is consumed. Source panel redraws and candidate-champion UI
effects remain blocked without a restored HUD owner.

- 2026-07-13 CSBWin restored `TT_78`/`TT_79` follow-up: Fire Shield and
  Magic Footprints now consume exact saved timer queue/event identities and
  update only their authenticated character-tail counters. Portrait redraw,
  footprint cleanup, and visual footprint material stay blocked without their
  corresponding CSBWin runtime/HUD owners.

- 2026-07-13 CSBWin restored `TT_77` follow-up: live dispatch now expires an
  exact saved Spell Shield receipt only for a positive, non-underflowing
  signed defense delta. The source all-portrait redraw remains blocked without
  a restored M11 HUD owner.

- 2026-07-13 CSBWin restored `TT_74` follow-up: live dispatch now expires an
  exact saved Party Shield receipt only for a positive, non-underflowing
  signed defense delta. The source all-portrait redraw remains blocked without
  a restored M11 HUD owner; timer families needing omitted object identity
  remain blocked pending a source-backed timer-record expansion.

- 2026-07-13 CSBWin restored `TT_73` follow-up: live dispatch now expires an
  exact saved Thieves' Eye receipt only while its imported party count is
  positive. Timer families requiring unpreserved object-word identity remain
  blocked pending a source-backed timer-record expansion.

- 2026-07-13 CSBWin restored `TT_72` follow-up: live dispatch now applies an
  exact saved champion-shield expiry only when its imported champion and
  unsigned defense delta are coherent. Underflowing records and the source
  status-panel redraw remain blocked; timers requiring omitted `timerWord8`
  object identity remain blocked as well.

- 2026-07-13 CSBWin restored `TT_71` follow-up: live dispatch now expires an
  exact saved invisibility receipt only while its imported party count is
  positive. The source inventory/status redraw has no restored UI owner, and
  timer families requiring omitted `timerWord8` object identity remain blocked.

- 2026-07-13 CSBWin restored `TT_1` timer follow-up: collision-free original
  door stepping and saved-owner requeue are live. Party damage, material-group
  damage/reaction, source sound data, malformed Thing chains, and nonterminal
  collision ownership remain blocked until their complete saved runtime state
  is source-backed.

- 2026-07-13 DM2 viewport source-material follow-up: source-required wall and
  door passes now reject a decoded GDAT image when its own IMG3 local-palette
  receipt is absent. Extend that same per-image ownership rule to remaining
  map-chip consumers only with proven skproject lookup and palette evidence;
  do not borrow `INTERFACE_GENERAL` colors or fabricate a fallback plane.

  - 2026-07-13 update: direct G1 DB2 Text and DB3 Actuator `WALL_GFX` routes
    now carry `dtImage/1` metadata plus the matching
    `QUERY_GDAT_IMAGE_LOCALPAL` receipt into `DRAW_DEFAULT_DOOR_BUTTON`.
    The source-required viewport fetches that exact IMG3 before comparing its
    palette hash and blocks any absent/mismatched ornate/button. The lookup
    now also consumes skproject's real `MISCELLANEOUS/FE/FE` GDAT default
    palette when an otherwise valid source image lacks a four-bit tail. This covers
    only the proven field-1 button route; broader ornate placement and other
    fields remain unavailable rather than inferred.

- 2026-07-13 DM2 viewport material follow-up: source-required creature,
  floor-object, projectile, carried-item, possession, and CHAMPIONS portrait
  GDAT drawing, plus T600 outdoor sky and ground planes, now require decoded
  IMG3 pixels and their exact local-palette receipts. Weather now verifies its
  GRAPHICSSET environment IMG3 address and local palette and carries its
  receipt into live frame ownership, but remains no-draw until skproject proves
  the destination clip; do not borrow interface colors or synthesize
  replacement art.

  - 2026-07-13 update: PC G1 DB4 creature map-chip receipts now bind the
    exact `CREATURES/type/F9` local palette alongside the decoded image. The
    viewport rejects a palette-hash mismatch; this does not infer animation,
    clipping, or a new draw route.

  - 2026-07-13 update: weather ENVIRONMENT commands now additionally require
    a bounded decoded IMG3 pixel receipt that matches their metadata and local
    palette. The decoded material remains no-draw until its complete source
    `QUERY_TEMP_PICST`/`DRAW_TEMP_PICST` execution route is consumed.

  - 2026-07-13 update: the DM2 runtime now carries the verified indoor
    floor/ceiling required and consumed masks into its M11 handoff receipt.
    A source-required indoor frame is invalid unless both GRAPHICSSET planes
    completed their renderer-owned material transactions; an incomplete plane
    is not presented through a substitute surface.

  - 2026-07-13 update: source-required `DM2_DRAW_DOOR` now prebinds every
    visible panel, ornament/destroyed-mask, frame, and button IMG3 together
    with its local palette before the first door blit. A missing component
    blocks the complete door pass instead of leaving partial or fallback door
    pixels. Remaining door work is exact source placement/clipping breadth.

  - 2026-07-13 update: source-required skproject T600 now prebinds both
    active `GRAPHICSSET` sky and ground IMG3s with their own local palettes
    before either outdoor scene plane is drawn. Weather stays no-draw because
    its real `DRAW_TEMP_PICST` image/destination route remains unproven.

- 2026-07-13 CSBWin saved-DSA parameter-message follow-up: the bounded
  `TT_ParameterMessage` runtime path now owns authenticated EXPOOL payloads
  through the 26-word stack ABI for source stone/open-room dispatch. Larger
  source records, non-DSA timer effects, master-state persistence, and DSA
  world/filter opcodes remain intentionally unavailable until each has an
  independently source-owned runtime surface; do not truncate parameters or
  route a timer by inferred room state.

- 2026-07-13 CSBWin saved-DSA LocalState follow-up: normal saved queue
  entries now execute source `LocalState=2` only when compact DB3 `ParameterB`
  has no widened high bits. Widened ParameterB values, slave-master routing,
  master-state writes, timer cell effects, and all world/filter opcodes remain
  blocked pending complete source-owned runtime records; do not coerce a
  compact actuator into a widened state value.

- 2026-07-13 CSBWin saved-DSA tick follow-up: restored timer queue entries
  remain save-owned after the tick. It still skips without an explicit original
  package and does not create a substitute save, DSA, or timer fixture.
  inventory/status-panel redraw branches remain blocked until their live

- 2026-07-13 CSB F0282 probe follow-up: keep the repaired probe-local C040
  candidate-panel receipt fail-closed as additional real-save variants are
  staged. Do not reintroduce the removed M11 diagnostic export or admit any
  state beyond source-proven F0280/F0282 panel facts.

- 2026-07-13 DM1 F0407 action-enable receipts: `THROW` and the real
  action-hand `SWING` route now complete through their F0330 C11 owners.
  Other F0407 action families still require separate source-owned live
  receipt work; do not generalize C11 scheduling from a UI cooldown or add a
  fallback timer. In particular, a delayed SWING C11 must remain locked until
  the authentic receipt reaches F0253. C11 receipts must retain their
  original ordinal and be rejected once their live owner is consumed.

- 🔧 2026-07-13 Nexus Structure3 follow-up: documented `0x800`-byte block
  requires an unavailable original Saturn capture and continues to block
  normal-plane, transform, texture/palette, and draw behavior.
  semantics remain unproved. The documented entry-local
  1,144 entries / 18,478 pairs. Next remains original Saturn evidence for

  - 2026-07-14 update: Structure1A byte 0 now reaches the DGN handoff and
    render-plan receipts only through complete Structure1F owner relations.
    Its raw reuse is counted, but its grammar remains unassigned; it cannot
    select a face, model, transform, mesh, material, or draw route.

  - 2026-07-14 update: the renderer-facing DGN plan now retains the same
    bounded Structure3 texture-selector and face/normal ordinal receipts as
    the handoff, including the complete retail LEV00--LEV15 selector joins.
    This remains no-draw provenance: original Saturn capture/executable
    evidence must still establish payload/palette decode, transforms, and
    VDP1 ordering before any mesh command can render.

  - 2026-07-14 update: those bounded face selectors now retain per-level
    unique/reused occurrence accounting for both Structure2 and Structure1G
    joins. This records raw source selector reuse only; it does not assign
    payload contents, texture dimensions, UVs, palette semantics, animation,
    transforms, or a draw route.

  - 2026-07-14 update: the hash-verified LEV00--LEV15 corpus now exercises
    the Structure3 capture binder for every retail level with source-only
    input. All 16 remain explicitly blocked before candidate framing, source
    binding, or renderer handoff. This is a no-draw regression guard, not
    Saturn capture evidence; texture/palette decoding, transforms, and VDP1
    draw ordering still require an authenticated original executable trace or
    capture containing every bound span.

  - 2026-07-14 update: the bounded Structure3 grammar now has a caller-owned,
    source-hash-checked typed entry extractor for documented signed-16.16
    vertex/normal rows and entry-local face rows. It rejects partial buffers
    and mutated payloads, retains raw byte 9 without assigning it a role, and
    never grants transform, palette, texture, VDP1, or draw semantics. The
    retail LEV00--LEV15 corpus extracts every 1,144 entries while preserving
    the 18,478 face/normal totals. Next remains original Saturn evidence for
    payload/palette decoding and VDP1 ordering, not fallback visuals.

  - 2026-07-14 update: hash-verified retail Structure1A/Structure1F records
    now bind their documented Structure3 model and face selectors to one
    bounded entry-local face row and its same-ordinal normal row. Any
    out-of-range model or face selector rejects the complete attachment
    receipt. This does not establish placement, transform, normal-plane use,
    texture/palette behavior, culling, VDP1 state, or a draw route. The next
    boundary remains original Saturn execution/capture evidence for those
    behaviors; do not promote the attachment receipt into rendering.

  - 2026-07-17 update: the parser now exposes source-bound opaque 12-byte
    ordinal rows for all three counted Structure3 entry regions, plus
    face-indexed first-region rows, source-order face-index sets,
    same-ordinal third-region rows, and raw face prefix/tail slices. Each
    admission rechecks the direct DGN identity, entry/region FNV, and row
    bounds; it grants no coordinate, topology, normal, material, texture,
    geometry, or draw semantics. The next concrete intake must be a separately
    evidenced raw relation, while original Saturn evidence remains required
    before any geometry or renderer promotion.

  - 2026-07-17 update: a 0x21-tagged Structure1F wall-decoration record can
    now join its already-admitted raw selector byte to one admitted Structure3
    second-region row ordinal. The join rechecks direct identity, package and
    retained record/entry/region/row FNV witnesses and rejects selector or
    offset drift. It is an opaque equality witness only, not a face, owner,
    topology, geometry, material, texture, placement, or draw relation.

  - 2026-07-17 update: the equivalent 0x20 Structure1F alcove selector path
    is now admitted against the same bounded Structure3 second-region row
    ordinal. Its independent record FNV, source tag, selector byte, direct
    identity, entry/region/row witnesses, and offset all fail closed. This is
    not portal, face, owner, topology, geometry, material, texture, placement,
    or draw evidence; floor-decoration and floor-sensor payload bytes remain
    unlinked until a separate source-backed relation exists.

  - 2026-07-17 update: Structure1F directory evidence now gives each 0x11
    floor-decoration and 0x12 floor-sensor opaque payload tail a strict source
    owner: family, tag, record ordinal/span/FNV, payload span/FNV, and direct
    package identity. Other families reject, and the receipt cannot grant a
    Structure3 relation or object, sensor, placement, geometry, material,
    texture, or draw semantics. A future relation requires separate source
    evidence rather than payload-byte inference.

## M12 Localization Completion (2026-07-12)

The launcher now resolves all 20 shipped locales from `LC_ALL`, `LC_MESSAGES`,
2026-07-30 launcher reliability update: the selected game-data directory now
retains the player's normalised path across scans and restarts instead of
persisting macOS's scanner-only `/private` alias. The embedded changelog also
derives its current-build header from CMake version metadata. Keyboard, mouse,
touch, language-popup, data-picker, accessibility, save-browser and launcher
handoff coverage is green in the full M12/launcher test selection.

## DM1/CSB Render Follow-up (2026-07-12)

DM1's C38 projectile precheck now carries ReDMCSB F0190's live active-group
a real slave-master route, source world effects, and a real-save corpus. Do
not promote unsupported world or text opcodes from fixtures.
pixels into its route hash. Remaining CSB startup work is the clean-build
Remaining CSB DSA work is writable widened `LocalState=2` ParameterB state,

  - 2026-07-15 update: the exact compact `LocalState=2` `PutState` form now
Saturn frames 47, 48, and 102. MENU.BPK remains blocked until its PRS3 pixel
and palette decoder is proven; the next Nexus work is real DGN rendering.
resolve a timer or filter callback. Remaining DSA work is still writable

- 🔧 2026-07-14 Nexus Structure3 face-geometry follow-up: the retail
executable or capture that proves their byte grammar; do not reinterpret them
as line/circle coordinates or promote them into movement geometry.
Nexus's original Structure1B wall-selector transform remains unproven. Real
authenticated BPK). The remaining work is a Saturn executable or capture

## Legend

- ❌ Not started

- 🔧 In progress / partial

- 🐛 Known bug

## Dungeon Master Nexus

### Nexus V1

- 🔧 2026-07-09 Nexus MENU.BPK/DGN/SLEV/SNDLEV follow-up: engine init exposes hash-resolved PRS3 decode and upload-plan receipts for `MENU.BPK`; DGN level load exposes renderer/runtime mesh-readiness and viewport render-plan receipts and hash-resolves renamed `LEV00.DGN`; SLEV runtime receipts block unsupported script dispatch without fallback rules; SNDLEV runtime receipts load real SAL/MAP bytes and block unsupported SFX decode/playback. 2026-07-10 update: Nexus now has one complete-support receipt requiring title, save, champion, dungeon/DGN host routes, Saturn timing/capture matrices, no fallback visuals, and material-validated DGN viewport rendering together. 2026-07-10 update: known Nexus DGN levels 00-15 plus SLEV00-15 and SNDLEV00-15 SAL/MAP now resolve hash-first before filename fallback, with renamed real local LEV01/SLEV00/SNDLEV00/MENU.BPK proof. 2026-07-10 update: real `MENU.BPK` PRS3 streams decode and upload as `ready-decoded`, and champion-start host routes now require the DGN commands to come from the material plan/viewport path before drawing. 2026-07-11 update: DGN Structure1B mesh refs are now budgeted alongside Structure1C collision refs and propagated into render-plan receipts; bounded 4-byte mesh descriptors are decoded and applied to DGN command quads; SLEV trigger dispatch now has a bounded receipt-gated rule-table parser while unknown real candidates still block fallback dispatch. 2026-07-11 update: SNDLEV MAP data now has a bounded event-to-sample route receipt, while SAL sample decode and real playback remain blocked. 2026-07-11 update: Structure1F descriptors now carry bounded footprint semantics through geometry, handoff, and render-plan receipts. 2026-07-11 update: real SLEV00-15 files are now profiled as SH-2 task-like streams with dispatch still blocked, including JSR, PC-relative load, immediate, branch, and literal pointer operand receipts. 2026-07-11 update: real SAL00-15 packages now emit bounded package metadata receipts, SNDLEV MAP record tables expose bounded SAL offset/size windows, first/last record windows expose checksum/nonzero/high-bit metadata, and blocked event-selected SFX calls now report the matching SAL window metadata without playback. 2026-07-11 update: SAL record windows now also expose payload-shape diagnostics (first/last nonzero relative offsets, distinct byte count, and byte-transition count) for first, last, and event-selected windows without enabling playback. 2026-07-11 update: SNDLEV MAP headers now expose checksum, nonzero byte count, distinct byte count, and transition count as bounded diagnostics before record parsing; MAP records also expose min/max/span event IDs plus unique/duplicate event counts and an explicit duplicate-event flag. Remaining work is broader real Saturn capture comparison beyond the material-route proof, decoding SLEV call targets/operands into safe dispatch rules, actual SAL payload/sample decode/playback, and confirming the Structure1F descriptor interpretation against a larger real DGN corpus.

  - 2026-07-15 update: an engine-owned route now admits one raw MAP selector only when the active level's SAL and MAP identities are hash-verified and the selector resolves uniquely to a bounded SAL window. The selector remains opaque: original Saturn event-dispatch, SAL payload decoding, SDDRVS driver ABI, and playback are still blocked pending source/capture proof.

  - 2026-07-15 update: the active engine now also admits the SLEV entry receipt only when the current level, hash-verified `SLEVxx.BIN`, VM source, and corpus-proven SH-2 header agree. It exposes bounded entry/literal facts only; original task-body dispatch, callback targets, and trigger semantics remain blocked.

  - 2026-07-15 update: the active verified SLEV route can now write an execution-capture target that pins the canonical SLEV identity, entry framing, and literal addresses and demands observed entry PC, task-body transfer, and callback-or-write evidence. It remains a producer request, not a task decoder or dispatcher.

  - 2026-07-15 update: the source-owned SLEV campaign probe can emit those no-dispatch targets for every canonical `SLEV00.BIN`--`SLEV15.BIN` from the local retail corpus. This supports offline capture planning without a Saturn BIOS, but it does not create a trace or prove task-body semantics. The remaining need is still one authentic Saturn SH-2 capture per promoted behavior.

  - 2026-07-15 update: the active hash-verified SNDLEV route can now emit a capture target for each uniquely bounded raw MAP selector across the retail corpus (106 targets across LEV00--15). Each target pins SAL, MAP, and `SDDRVS.TSK` identities plus the exact SAL window and asks for original selector-dispatch, SAL-read, and driver-output evidence. It cannot decode, map host events, or play the bank; those semantics still require authentic Saturn capture.

  - 2026-07-15 update: admitted SLEV trace evidence now reaches a separate active host-consumption receipt only after the current SLEV target is rebuilt and revalidated. Level/VM source drift is rejected without replacing prior host evidence. Consumption does not execute the observed opcode or callback/write location; semantic dispatch remains open.

  - 2026-07-15 update: host consumption now additionally requires raw Mednafen trace bytes to match the manifest's declared FNV-64 receipt. A manifest-only trace remains evidence-only and cannot reach the host route. This binds imported bytes but still does not prove opcode meanings or authorize dispatch.

  - 2026-07-15 update: raw-trace evidence now verifies that the bound capture contains the exact declared entry, task-body, and callback/write observations. This establishes occurrence only, not task-body grammar, callback ownership, or gameplay semantics; dispatch remains blocked.

  - 2026-07-15 update: the evidence receipt now also requires byte-order entry → task-body → callback/write within one raw trace, with each offset retained for audit. This is observation ordering, not execution semantics.

  - 2026-07-15 update: the same raw trace must now contain both corpus-proven PC-relative SLEV literal addresses. This verifies that both entry operands occur in capture, not what either literal owns or dispatches.

  - 2026-07-15 update: trace admission now additionally binds canonical SLEV name, task-header size, and both literal values to the active target. Any cross-level or partial-header manifest is rejected before raw evidence can be consumed.

  - 2026-07-15 host-route update: SLEV host intake now also requires the
    bound raw trace's ordered entry/task-body/callback observation and both
    literal observations. This validates capture occurrence only; the task
    body remains opaque and dispatch/callback execution stays blocked.
    Evidence retains the exact raw-trace FNV and byte count, so an older
    same-level observation cannot satisfy a changed active trace.

  - 2026-07-11 update: Nexus `runtime_screenshot_readiness` and `track1_real_screen_capture_readiness` now pass locally. The runtime gate avoids the old M12 screenshot-gallery startup timeout by using a boot-probe app receipt for Nexus launch metadata and the Nexus-owned Track 1 BMP probe for the real-data image receipt. The Track 1 probe is self-contained, no longer links `firestaff_m11`, writes deterministic 24-bit BMP receipts, and stamps a real `FONT256.S2D` glyph into the indexed framebuffer before BMP export. Remaining capture work is reviewed Saturn capture comparison and eventual public screenshot promotion, not the readiness plumbing.

- 🔧 Runtime handoff/playability proof: V1 phases 0-7 are implemented/source-locked. The M11 launcher handoff boundary (`nexus_v1_m11_launcher_handoff_boundary`) passes against local retail ISO. Real Saturn asset-path proof for the DGN material containers is now anchored by the boot profile's hash-first validation of `SN_FLOOR.MNS`/`SN_WALL.MNS`. Remaining work is the capture-blocked DGN material raster decode and broader packaged startup capture proof, not synthetic fallback rendering.

- 🔧 2026-07-14 update: DGN face/material admission now requires the exact
  launcher-reopened LEV bytes to match the authenticated canonical entry before
  raster input is accepted. Remaining work is real face/mesh/pixel decode and
  Saturn capture, not fallback rendering.

  - 2026-07-13 update: the selected retail DM.BIN V1 SH-2 route now has an importable instruction receipt for its R11 control test, bounded R12 post-increment byte read, R13/R0 byte store, and loop branch. It is not a live MENU.BPK binding or VDP1 capture. Remaining work is an original execution capture connecting one hash-verified BPK entry to those reads, its full output range, and a real VDP1 command/source range before PRS3 decoding or menu handoff can be considered.

- 🔧 Mechanics parity hardening: movement, click routes, item usage, doors, pits, teleporters, triggers, combat, AI, and sound are implemented; remaining work is broader runtime/probe coverage beyond compile/save-load gates. 2026-07-22 update (Lane D, cycle 3): creature attack damage is now applied to the party leader (or first living party member) and total party death sets `game_over=1` / `game_over_reason=2 (all_dead)`. The empty-party `nexus_mechanics_party_alive()` bug is fixed (empty party is dead, not alive). The mechanics parity probe now covers the integrated tick with a synthetic scorpion-vs-party combat scenario. 2026-07-22 update (Lane D, cycle 4): champion death auto-leader promotion is implemented. `nexus_v1_champion_on_death_update_leader()` in `src/nexus/nexus_v1_champions.c` promotes the first living party member to leader when the current leader dies, matching ReDMCSB CHAMPION.C F0319 lines ~1662-1679. The mechanics tick calls it after creature-attack damage and stamina-collapse death. The mechanics parity probe now verifies non-leader death leaves leader unchanged, leader death promotes the next living member, and total party death returns no successor. 2026-07-22 update (Lane D, cycle 5): pit/chute square-event integration is implemented — stepping on a `NEXUS_SQUARE_CHUTE` now forces a level transition to `map_index + 1` via `pending_level_change`. Item usage/click-route wiring is implemented — `NEXUS_CMD_USE_ITEM` consumes the selected leader inventory slot (`use_item_slot`), applies consumables (health/mana/stamina potions, antidote, corn, water flask) and equips weapons/armor, then clears the slot and recalculates load. Source locks: DM1 MOVESENS.C F0267/F0268 (chute/pit), COMMAND.C item-use dispatch, CHAMPION.C F0309 equipment slots. The mechanics parity probe now covers both new behaviors (207/207 PASS). 2026-07-22 update (Lane D, cycle 6): mouse click-route dispatch for inventory/world objects is implemented — `nexus_click_route_dispatch()` translates inventory-slot, equipment-slot, world-square, door-square, and floor-item clicks into the same command queue used by keyboard input (`NEXUS_CMD_USE_ITEM`, turns, `NEXUS_CMD_FORWARD`, `NEXUS_CMD_INTERACT`). New `NEXUS_CMD_INTERACT` picks up floor items at the party's current square into the leader's inventory. Source locks: DM1 COMMAND.C mouse/click dispatch, CLIKMENU.C F0366 command queue, CHAMPION.C F0309 equipment slots, MOVESENS.C F0267/F0268 square interaction. The mechanics parity probe now covers click-route dispatch (218/218 PASS) and the dedicated `test_nexus_v1_click_route` regression test covers 31 checks. 2026-07-23 update (Lane D, cycle 7): pit/teleporter broader runtime coverage is implemented — `nexus_process_square_event` now reports the registered stair facing (`out_target_dir`) for stairs up/down; `nexus_mechanics_tick` processes `pending_teleport` before the step cooldown so teleporter warps are immediate, and cross-level teleporters set `pending_level_change` to the target level. New regression test `test_nexus_v1_pit_teleporter_runtime` covers chute step, chute max-level clamp, same-level/cross-level/unregistered teleporters, and stairs down/up targets (24/24 PASS). The mechanics parity probe adds Probe 12 for teleporter runtime (same-level, cross-level, unregistered) and now passes 226/226. Source locks: DM1 MOVESENS.C F0267/F0268 (teleporter/pit/stair sensors), DUNGEON.C square type dispatch, CLIKMENU.C:264-276 level-transition special cases. 2026-07-23 update (Lane D, cycle 8): stairs/exit/alarm broader runtime coverage is implemented — unregistered stairs now fall back to the adjacent level (down +1, up -1, clamped to [0,15]); registered stairs keep their exact target level/coordinates/facing; exit squares only end the game on the final level (level 15), with non-final exits treated as ordinary floor; alarm traps now alert only creatures on the current level and set a bounded 60-tick alarm timer that keeps alerted creatures chasing even when the party moves out of normal detection range. `Nexus_Creature` gains a `level` field, `Nexus_V1_CreatureManager` gains `alarm_timer`, and `nexus_v1_creature_spawn_on_level()` is added so probes/tests can place creatures on specific levels. `nexus_v1_creatures_tick()` now skips/attacks only creatures on the active level. `test_nexus_v1_pit_teleporter_runtime` expanded to 34 checks covering stairs down/up registered/unregistered and final/non-final exits. The mechanics parity probe adds Probe 14 for stairs/exit/alarm runtime and now passes 240/240. Source locks: DM1 MOVESENS.C F0267/F0268 (stairs/exit sensors), F0277 ALARM; CLIKMENU.C F0364_COMMAND_TakeStairs; ReDMCSB CHAMPION.C F0309 equipment slots. 2026-07-23 update (Lane D, cycle 9): water/fire square traversal mechanics are implemented — water squares (type 21) now block movement unless the party leader carries a Rope (item 65); fire squares (type 22) block movement unless the party leader carries a Rune of Fire (item 80). The passability gate lives in `nexus_mechanics_tick()` alongside the existing door key check; the square event layer now emits `NEXUS_EVENT_CROSS_WATER` and `NEXUS_EVENT_CROSS_FIRE`. New `NEXUS_MOVE_CROSS_WATER`, `NEXUS_MOVE_CROSS_FIRE`, `NEXUS_MOVE_BLOCKED_WATER`, `NEXUS_MOVE_BLOCKED_FIRE`, and `NEXUS_MOVE_BLOCKED_DOOR` result codes are defined in `nexus_v1_movement.h`. `test_nexus_v1_pit_teleporter_runtime` expanded to 44 checks covering water/fire blocked/crossed and square-event returns. The mechanics parity probe adds Probe 15 for water/fire square runtime and now passes 251/251. Source locks: DM1 MOVESENS.C F0267/F0268 water/fire square sensors; nexus_v1_inventory.c Rope (65), Rune of Fire (80). 2026-07-23 update (Lane D, cycle 10): real-DGN playability probe is implemented — new `firestaff_nexus_v1_mechanics_playability_probe` loads retail `LEV00.DGN` from `FIRESTAFF_NEXUS_DATA_DIR` (or `~/.firestaff/data/nexus`), verifies 64x64 Structure1B load, initializes a party on the actual starting floor square, exercises forward movement/turning on real geometry, verifies OOB/map-edge blocking, reports decoded floor/wall/door counts, and flood-fills reachable passable squares. The probe is skip-safe when the retail corpus is absent. Source locks: DMWeb DGN Structure1B format; ReDMCSB DUNGEON.C, COMMAND.C, MOVESENS.C, CHAMPION.C. CTest `firestaff_nexus_v1_mechanics_playability` passes 16/16 against the local Track 1 LEV00.DGN and exits 0 (skip) when data is missing. 2026-07-23 update (Lane D, cycle 11): expanded the real-DGN playability probe to all 16 retail levels (LEV00–LEV15). `firestaff_nexus_v1_mechanics_playability_probe` now loops over LEV00.DGN–LEV15.DGN, loads each through the existing Structure1B decoder, verifies 64×64 dimensions, counts floor/wall/door squares, checks OOB boundary blocking, real wall blocking, forward movement/turning on real floor, and flood-fills reachable passable squares; the probe reports 253/253 PASS against the local Track 1 corpus and remains skip-safe when data is absent. A companion CTest regression test `nexus_v1_dgn_multi_level_playability` (`tests/test_nexus_v1_dgn_multi_level_playability.c`) covers the same core checks across all 16 levels and returns 77 when no data is present. Remaining mechanics work: sound playback binding (still blocked on SAL decode), stairs/exit/alarm exact original timing/feedback, and real-data playability probes for additional square-event semantics once Structure1B wall/special-square decoding is source-locked against original Saturn evidence.

- 🔧 DMDF embedded BITMAP/palette/string runtime handoff remains open after the parser-level bounds gates. The real MNS `TEXT` descriptor and BGR555 material-bank route is now regression-covered: all 30 retail models retain matching descriptor/pixel receipts and all 815 source textures decode. The seven creature banks whose source colour cardinality exceeds the indexed 256-entry host bank now retain exact BGR555 words in a source-only direct-colour lane; they are not quantized, substituted, or admitted to the indexed viewport. VDP1 command/CLUT ownership, direct-colour display semantics, texture upload and runtime render binding remain capture-gated.

- 🔧 2026-06-28 Nexus V1 save multi-slot round-trip follow-up: new `test_nexus_v1_save_multislot_roundtrip_pc34_compat` (CTest `nexus_v1_save_multislot_roundtrip_pc34_compat`) drives 4 distinct slots (0..3) with distinct per-slot world + champion state through `nexus_v1_save_full` / `nexus_v1_load_full` and verifies party_level/x/y/dir + world_tick + per-object (type, state, x, y, level, quantity, linked_id, flags) + per-event (type, level, x, y, arg0, arg1, fired, repeat) + per-active-timer (id, kind, level, remaining_ticks, interval_ticks, flags) + transition (pending, target, spawn_x, spawn_y) + per-champion stat blobs (name, primary_class, hp, max_hp, stamina, max_stamina, mana, max_mana, str, dex, wis, vit, anti_magic, anti_fire, fighter/ninja/priest/wizard level, food, water, alive, portrait_index, wounds, attributes, inventory[30]) + party[] indices round-trip per slot, plus manager slot cache + scan() + isolation + deletion + CRC tamper rejection (one-byte flip in the data section → `NEXUS_SAVE_ERR_CRC`) + foreign-magic rejection (`NEXUS_SAVE_ERR_UNKNOWN_VARIANT` + non-empty diagnostic). Source-lock: `src/nexus/nexus_v1_save_load.c` (NEXUS_SAVE_MAGIC='FNXS', CRC-32 over champion+world data sections) + `src/nexus/nexus_v1_world.c` (party + objects + events + active timers + transition + world_tick + state_hash) + `src/nexus/nexus_v1_champions.c` (CHPN magic, 270-byte champion blob) + ReDMCSB LOADSAVE.C F0433/F0434 lineage. Same family, disjoint scope: existing slot-0/party-x test still covers the single-field gate; this new test extends coverage to 4 slots + 30+ per-slot world/champion fields + cache/scan/isolation/deletion + CRC + unknown variant. Companion source-side fixes (also shipped this pass): (a) `nexus_v1_champion_pool_serialize_size` now matches the actual `wr32`-based 24-byte header (was claiming 22 with a `version(2)` that the serialize code does not write); (b) `champion_blob_size` now counts 25 int fields per champion (was 23, which under-counted by 8 bytes/champion and silently overflowed the 24-champion pool blob in older code paths); (c) `nexus_v1_world_serialize_size` now omits the bogus 4-byte object-count prefix (the actual serialize path reads the count once from the header); (d) `nexus_v1_load_full` and `nexus_v1_load_full_from_path` now allocate buffers via the new `nexus_v1_save_max_champion_pool_size` / `nexus_v1_save_max_world_size` helpers instead of asking the destination's serialize_size (which underestimates because the destination has not been loaded yet — the prior code only worked when the saved world happened to have no objects/events/timers). Remaining save-slot work: original Saturn 8 KB memory card format reverse-engineering (Firestaff-native only today), real-asset save compatibility artifacts, and broader per-game (DM1/CSB/DM2/Theron) save interoperability.

- 🔧 2026-07-17 FONT256 first-section witness: the canonical SHA-256-attested
  Treat that observed ramp as opaque and capture-required, never as a glyph
  table or pixel layout.

  - 2026-07-19 update: all four populated SCR sections (table indices
    unchanged: an original Saturn trace or independently reviewed format
    material before any subrecord grammar, palette, glyph, or draw route.
    pixel meaning; draw routes remain blocked. CTest
    section, the preamble, and the section table. Remaining FONT256 work is

  - 2026-07-20 update: the subrecord question is now answered read-only.
    independently reviewed format material before any glyph layout,
    palette, encoding, or draw route is assigned to these structures.
    Remaining FONT256 work is unchanged: an original Saturn trace or

  - 2026-07-20 update (round 15): the ordinal-1 section (table index 2,
    independently reviewed format material before any subrecord grammar,
    glyph layout, palette, encoding, or draw route.
    `nexus_v1_font256_s2d_subrecord_grammar` (+ `_real`) PASS. Remaining

- 🔧 2026-07-17 WARNING.BIN source-only follow-up: the canonical, directly
  header values only; neither the prefix nor body is assigned CLUT, pixel,
  colour, stride, or draw semantics without further original evidence.
  semantics; those remain separate original-Saturn evidence requirements.
  and the two trailing bytes before the next descriptor. Width/height remain

  - 2026-07-17 update: Sega Saturn/32X Graphic References ST-124-R1 section
    6 now supplies the missing PP contract: a 256-word BGR555 CLUT follows the
    six-byte PP header and a one-byte palette code follows for each image
    pixel. The canonical resource-0 executor consequently accepts only the
    admitted 240x96 body with stride 240, copies its exact index bytes and
    original BGR555 words to caller-owned buffers, and invokes an explicit
    presentation callback. It has no default presentation, host-RGBA
    conversion, CLUT substitution, trailing-byte interpretation, or fallback.
    Remaining evidence is an original Saturn display/VDP route if this asset
    is to be connected to a live screen rather than an externally supplied
    source-faithful presenter.

  - 2026-07-17 update: resource 0 now reaches the real 320x200 M11 indexed
    presentation surface. Each warning frame reopens the direct canonical
    source, checks the engine's exact asset identity, then revalidates the
    full PP receipt before it writes the top-left 240x96 index plane. The
    host palette receives only the 256 BGR555 words in ST-124 order
    `B4..B0/G4..G0/R4..R0`, expanded by exact bit replication to M11's RGB6
    palette API. A changed source/body, noncanonical asset, wrong host size,
    or any failed receipt leaves the already-cleared M11 frame unpresented;
    title, generic UI surface, and solid-image substitutes are not used.
    This does not prove a Saturn VDP display command, interlace, colour-DAC,
    gamma, timing, or placement contract beyond the documented PP resource-0
    bytes and the explicit M11 host surface.

  - 2026-07-19 update: all four canonical DGT2/PP resources now carry the
    same admission -> execution -> M11 presentation chain, not only resource

    0. New `nexus_v1_warning_dgt2_resource_corpus` module
    warning flow shows, in which order, remains original-Saturn evidence
    work).

- 🔧 2026-07-20 TITLE.BIN RES* directory corpus follow-up: new
  original-Saturn evidence work); 0DMSTRT.BIN shows no RES* framing and
  stays excluded from this block pending original evidence.
  [0x2e8, 0x1b658) that covers the source tail with zero gap; a bounded
  the original title/startup flow uses, in which order, remains

- 🔧 2026-07-20 TITLE.BIN TITL PP payload admission follow-up: new
  original title flow draws, where, and in which order, remains
  original-Saturn evidence work).

- 🔧 2026-07-20 TITLE.BIN DGT2 payload admission follow-up: new
  draws, where, and in which order, remains original-Saturn evidence
  work.

- 🔧 2026-07-20 TITLE.BIN MAPD TIBG admission follow-up: new
  assignment; how the original title flow uses this payload remains
  original-Saturn evidence work.

- 🔧 2026-07-20 TITLE.BIN CNFD payload admission follow-up: new
  the original title flow uses these payloads remains original-Saturn
  evidence work.

- 🔧 2026-07-20 0DMSTRT.BIN structure admission follow-up: the file
  execution route; how the original boot flow loads and uses this
  image remains original-Saturn evidence work.
  and rejection across NULL arguments, size/identity drift, gap

### Nexus V2.0 / V2.1 / V2.2

- 🔧 Phase 2 - Enhanced asset pipeline: presentation-mode selection API + filter config + V2.1 EPX upscaler pipeline are wired (`nexus_v2_upscaler.c` provides `nexus_v2_epx_upscale` indexed→RGBA via palette, `nexus_v2_bilinear_smooth` post-filter, `nexus_v2_upscaler_source_evidence`). Headless probe `firestaff_nexus_v2_upscaler_probe` 23/23 (palette lookup, deterministic output for same input, 2x scaling fills all dst pixels, 1x1 boundary case, null-arg safety on src/dst/palette/zero-dims, bilinear null-arg safety on null/0x0/1x1/4x4, source evidence). Ctest `nexus_v2_upscaler_probe` 1/1. **2026-06-19 Nexus V2.2 modern-asset module landed:** new `nexus_v22_modern_assets_pc34.c/.h` mirrors dm1/csb modules with Nexus paths (`~/.firestaff/assets/nexus/modern/`) and Saturn source-locks (SATURN_DMDF T400/T520/T600 + Saturn VDP1/VDP2). Ctest `test_nexus_v22_modern_assets_pc34` 33/33. **2026-06-19 Nexus V2.2 first-cut asset pack landed:** `.openclaw/tmp/nexus_v22_asset_author.py` (5 PNGs + manifest v1.0.0). Smoke: `nexus_v22_modern_assets_available()=1` end-to-end. Remaining work: real PBR hero art for Nexus via gpt-image-2 batch + per-cell modern-art swap in Nexus V1 draw pipeline.

## Cross-Cutting

- 🔧 Asset scanner archive coverage: `.zip`, ISO/BIN/CD images, `.cue`, `.tar`, `.tgz`, `.tar.gz`, `.gz/.gzip`, LHA/LZH, and common external archives (`.7z`, `.rar`, `.cab`, `.arj`, `.arc`, `.zoo`, `.ace`, `.sit/.sitx`, `.dms`) are hash-scanned when the built-in parser or available system extractor supports them.

- 🔧 2026-06-27 Nexus BPX/BPK MENU.BPK byte-level boundary inspection (pass1082) follow-up: `nexus_v1_bpk_archive` now exposes bounded inspection and DMWeb `DecodePRS3` decompression for the verified real MENU.BPK structure: 163 entries, 162 PRS3-bearing entries, mode distribution {6:14, 14:62, 22:39, 30:47, 10:1}, and one directory trailer. Retail verification now decodes all 162 PRS3 surfaces with zero failures. Remaining work is the still-unbound pixel/mode interpretation, palette/VDP1 handoff, and first authentic Nexus menu capture; do not reintroduce synthetic menu pixels while those placement facts are absent.

  - 2026-07-15 update: `firestaff_nexus_v1_prs3_capture_campaign` now creates one source-bound external-capture target for every one of the 162 retail PRS3 streams. Each target binds both canonical assets, their full FNV witnesses, the exact bounded stream plan, and the independently checked static DM.BIN V1 SH-2 control/read/store route. The static route now also binds the `BRA` loop body start/length, backward target, and raw FNV witness; this proves only byte framing and control-flow destination, not opcode grammar. It requests original input/read, output/write, VDP1-command, and palette-state observations. It does not decode a stream or authorize a menu handoff; authentic Saturn execution evidence is still required.

  - 2026-07-15 transfer-trace update: the capture boundary now accepts one
    `NEXUS_PRS3_SH2_TRANSFER_TRACE_V1` observation only when it binds exact
    canonical MENU.BPK/DM.BIN FNVs, one bounded stream byte, the fixed V1
    `MOV.B @R12+` read and output-store instruction offsets, ordered capture
    sequences, and equal observed input/output byte values. This establishes
    only a source-bound byte transfer after a real trace is supplied; it does
    not name a PRS3 command, authenticate the producer, infer bit order, or
    permit pixels. The next requirement is an original Saturn trace for a
    complete command/block, including its control state and output range.

  - 2026-07-15 loader-state update: the selected V1 route now has a
    source-locked R11 control-state receipt for the shared re-entry: `SHLR
    R11`, zero test, guarded `@R12+ -> R11` refill, byte extension/merge with
    the original `0x0100` sentinel, then the existing low-bit split. It proves
    the next control/refill state but not control-bit order, command names,
    lengths, backreferences, termination, or output semantics. A real trace
    must still bind the state values and branch outcomes to one MENU.BPK block.

  - 2026-07-15 nonzero-byte rule update: an SH-2 transfer trace now also has
    to name the source low-bit test, its zero-side branch, the `R14-1`
    fallthrough, and an observed nonzero/not-taken outcome before the known
    `@R12+ -> R2 -> output` byte path is accepted. This is the first exact
    source-derived byte-transfer rule, not a PRS3 literal/opcode claim. It
    remains blocked pending an authentic Saturn trace and complete token
    grammar/output-range proof.

  - 2026-07-15 zero-side update: the zero-low-bit path is now constrained as
    two adjacent MENU.BPK bytes through the original `R14-2` gate and exact
    `@R12+ -> R4` / `@R12+ -> R7` reads. Its observed merge must equal the
    instruction-proven `(first | ((second << 4) & 0x0f00))` value. This remains
    opaque control data: no backreference, offset, length, or output meaning
    is inferred without an authenticated original Saturn command trace.

  - 2026-07-15 zero-side continuation update: DM.BIN now proves that the
    merged zero-side value is copied, masked with the source `0x0fff`, and
    used in the exact `@(R0,R13) -> R1` read path. This is an opaque bounded
    history/index read, not evidence that R13 is a backreference window or
    that R1 is decoded output. An original Saturn trace must prove buffer
    ownership, read address, branch outcome, and resulting output before any
    PRS3 backreference/control semantics can be named.

  - 2026-07-15 zero-side repeat update: after the bounded R13-indexed read,
    DM.BIN proves the R1/R10 comparison, R10 increment, delayed repeat branch
    to the R6 loop, delay-slot R5/R6 mask, and outer return to R11 control.
    This is a source-owned repeat-control shape only, not a copy length or
    backreference condition. A Saturn trace must still prove the compared
    values, branch outcome, R13 buffer ownership, and any output writes.

  - 2026-07-15 complete-stream capture update: the canonical PRS3 campaign
    now locks the nonzero output predecessor (`@R12+ -> R2`, `R6 -> R0`,
    `R2 -> @(R13,R0)`) and requests one complete source span/output range per
    MENU.BPK stream, plus control outcomes, palette state, and VDP1 command
    consumption. Targets remain no-decode: an authentic trace must prove that
    the complete output range has the declared size and reaches the observed
    Saturn presentation route before any decoder can be enabled.

  - 2026-07-15 startup diagnostic update: canonical `MENU.BPK` was correctly
    found and parsed but Firestaff's generic M11 text said `REAL SATURN
    DECODER REQUIRED`, which looked like a missing emulator/decoder error.
    The launcher now reports the source-bound PRS3 prerequisite instead. For
    the retail archive this is `MENU.BPK PRS3 TRACE REQUIRED`: the package is
    valid, but an authenticated SH-2/VDP1 capture and complete grammar remain
    required. No decode or substitute menu is enabled.

  - 2026-07-17 startup-state update: the launcher now classifies this as the
    terminal `menu-bpk-prs3-capture-required` state. M12 can return it to
    idle or rescan it without waiting or drawing; a verified imported capture
    is consumed as a separate terminal no-draw state. Decoder, pixel, palette,
    and replacement-menu claims remain blocked.

  - 2026-07-15 resolver update: a Nexus data-dir resolution failure happened
    before the launcher receipt existed, leaving M11's outcome blank. It now
    publishes `NEXUS DATA ERROR`; this is distinct from the canonical PRS3
    trace blocker and does not alter discovery or decode policy.

  - 2026-07-15 launcher-failure update: scanner/init failure can also arrive
    with an incomplete host receipt. M11 now publishes `NEXUS DATA ERROR`
    deterministically, retaining `NEXUS LEVEL ERROR` only when the launcher
    established that source-specific state. This is a diagnostic correction;
    it does not relax canonical MENU.BPK/DM.BIN checks or PRS3 gating.
    A level error is now retained only after an actual DM.BIN/SEGADATA.BIN or
    CD image was observed; an empty directory correctly remains a data error.

  - 2026-07-15 safety update: missing, invalid, stored, or PRS3-blocked MENU.BPK handoffs never permit generated replacement visuals. Launcher main-menu readiness now also requires a valid, actually renderable BPK receipt rather than treating an absent receipt as success. The real retail archive remains blocked pending authenticated Saturn palette/VDP1 evidence and pixel/mode interpretation.

- 🔧 Nexus S2D real-font parity: the DMWeb FONT256 region decoder now exposes the real page, character-generator, palette and attribute regions with bounded retail tests. Remaining work is page-to-character mapping, runtime text-layout binding, and an authentic screen capture; no flat 1bpp glyph guess may be promoted.

### Launcher and Settings

- 🔧 Start-menu feature hardening: first-pass persistence exists for quick resume, minimap, automap, combat log, soundtrack, ambient audio, UI scale, streamer mode, custom music, custom dungeon, screenshot path, session timer, save export/import, manual/docs launcher, polished UI flow, and all five per-game option slots. **2026-07-19 launcher-options runtime handoff landed (Jobb F2, commit 0f7fc0a43):** new `M12_LauncherRuntimeOptions` snapshot (global launcher settings + per-game language/cheats/speed folded in) is exported via `M12_StartupMenu_ExportLauncherRuntimeOptions()` with clamped ranges, carried on `M12_LaunchIntent.launcherOptions` after `m12_enforce_mode_constraints`, handed to `M11_GameLaunchSpec.launcherOptions`, and applied through `m11_apply_launcher_options_handoff` in all five game-start branches (dm1/csb/dm2/nexus/theron) after Shutdown/Init; CTest `m12_launcher_options_runtime_handoff` PASS. Remaining work is cloud sync.

- 🔧 Custom dungeon import: `custom_dungeon_import` now CTest-gates synthetic M12 launcher and DM1 V1 engine scanning for `DUNGEON.DAT` fixtures, including header/map-count parsing, case-insensitive paths, optional `GRAPHICS.DAT`, compressed/tiny rejection, and valid-entry selection. **2026-06-26 CSBWin `CSBgraphics.dat` bounded classifier landed:** new `csb_v1_csbgraphics_dat_classify` reads the on-disk count + parallel compressed/decompressed size tables and the optional `0x8001` little-endian sentinel without LZW/payload decode; companion `csb_v1_csbgraphics_dat_real_scan` mirrors the HCSB.HTC scanner pattern with an empty default known-hash list so the real-asset probe SKIPs cleanly on hosts without a user-staged CSBgraphics.dat. **2026-06-28 payload-span boundary added:** `csb_v1_csbgraphics_dat_entry_span()` now returns one override entry's compressed payload offset, compressed size, and decompressed budget using CSBWin `LocateNthGraphic(n)` offset math, still without LZW decode or runtime override. **2026-06-30 bounded payload decode added:** `csb_v1_csbgraphics_dat_decode_entry()` now decodes one declared entry through the existing ReDMCSB-compatible graphics LZW decoder and rejects undersized output buffers, compressed-empty mismatches, and bad streams without interpreting or overriding the bitmap. CTest `csb_v1_csbgraphics_dat_classify_unit` PASS 19/19 (argument/too-small/empty/oversized count rejection, big-endian + LE-marker round-trip, total-compressed overflow rejection, truncated tables rejection, max-entry tracking, big-endian and LE-marker entry spans, zero-length entry preservation, entry-range rejection, LZW round-trip decode, output-too-small rejection, empty-entry decode, bad-LZW rejection, source-evidence citation) and `csb_v1_csbgraphics_dat_real_scan` PASS (skip-safe when known-hash list is empty; locally verified 22/22 checks on a synthetic CSBgraphics.dat with a temporary hash registration). **2026-06-27 CSBWin `dmsave`/`csbgame` loader-boundary contract landed:** new `csb_v1_csbwin_save_loader_boundary_pc34_compat` (include + src/csb) builds a 14-shape CSBWin/DM1 save contract (3 accept + 11 reject: CSB v2.0/v2.1/.bak payload, DM1 RDMCSB15, CDSA marker, CSBWin 512-byte CSB\1/DM\0\1/CEDT, too-small, no-magic, champion-count-out-of-range, truncated records, bad-version) and runs every shape through the existing `csb_v1_import_csb_save_buffer()` entry point to record the documented accept/reject verdict and surface a `contract_match` flag. CTest `csb_v1_csbwin_save_loader_boundary_pc34_compat_unit` PASS 79/79 (contract-table invariants, per-shape loader-boundary check on synthetic fixtures, hand-rolled 2-champion v2.0 round-trip, builder determinism, accept-shape helper, source-evidence citation) and `csb_v1_csbwin_save_loader_boundary` PASS (skip-safe when no user-staged csbgame.dat/csbgame.bak/dmsave.dat/dmsave.bak exists). Source-locked against ReDMCSB CEDTINC8.C:101-118 + LOADSAVE.C F0433/F0435 + SAVEHEAD.C F0429/F0430 + DEFS.H:1289 and CSBWin SaveGame.cpp:927/1711/2111 + CSBCode.cpp:421-422 (csbgame.dat / csbgame.bak literals) + Data.h:590 (SaveGameFilename). Disjoint from the sibling `csb_v1_csbwin_save_classify_pc34_compat` (sibling is on-disk shape detection; this loader-boundary gate exercises the actual importer against each shape — they are complementary, not duplicative). Remaining work is real community dungeon corpus handoff, CSBgraphics.dat payload bitmap interpretation + M11 viewport binding, the CSBWin 512-byte obfuscation-key decoder, and the end-to-end CSBWin importer wiring that this loader-boundary gate is the contract for.

### Touch and Controller Support

- 🔧 2026-06-28 runtime gesture navigation gate landed (input translation + touch target safety): new `runtime_gesture_navigation_gate` module (`include/runtime_gesture_navigation_gate.h` + `src/engine/runtime_gesture_navigation_gate.c`) wraps the existing `firestaff_touch.c` swipe + edge-strafe primitives into a deterministic cross-game contract behind the existing touch/controller settings. `FirestaffRuntimeGestureNav_Evaluate(event, policy, result)` maps swipe up/down/left/right to FS_CMD_MOVE_FORWARD/BACKWARD/TURN_LEFT/TURN_RIGHT (cross-V1/V2), edge-left/right to FS_CMD_STRAFE_LEFT/RIGHT (V2-only with v1ParityPreserve guard), pins the 44 px Apple HIG touch-target floor (`RUNTIME_GESTURE_NAV_MIN_TARGET_PX`), and rejects disabled / too-short / too-small-target / ambiguous-diagonal / V1-only paths. Source-locked against ReDMCSB `COMMAND.C:2045-2155 F0380_COMMAND_ProcessQueue_CPSC` + `CLIKMENU.C:142-174 F0365 turn` + `CLIKMENU.C:180-390 F0366 move` + `GAMELOOP.C:164-219 V1 input wait loop` + `DEFS.H:238-243 C001..C006` + `firestaff_touch.c FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX=40/FIRESTAFF_TOUCH_TAP_TOLERANCE_PX=24/FIRESTAFF_TOUCH_EDGE_ZONE_FRAC=0.20`. New CTest `runtime_gesture_navigation_gate` (15 invariant groups: setting gate, four swipe paths, threshold rejection, diagonal ambiguity, edge-strafe paths, target-size safety, touch-target safety, source-viewport scale safety, null-pointer safety, default-threshold fallback, V1 swipe parity, decision-name contract, source-evidence citation, cross-V1/V2 command codes, travel-pixel threshold boundary) and headless probe `firestaff_runtime_gesture_navigation_gate_probe` (9 groups + 50-iteration determinism) both PASS; existing touch + session_timer + V2-touch affordance CTest targets still PASS (28/28 in the touched ctest set, no regressions). **2026-06-30 bridge wire-up landed:** `firestaff_touch.c` now exposes runtime-gated swipe and edge-strafe emit APIs and the legacy wrappers route through the same gate before pushing to `FS_InputQueue`; CTest `firestaff_touch_runtime_gesture_bridge` covers disabled/touch-off rejection, ambiguous swipe rejection, V1 edge-strafe rejection, V2 strafe emission, and wrapper compatibility. Remaining work is actual UI scaling / touch-target audit across launcher + game views and any Sphenx/Greatstone-style paired original-vs-Firestaff touch-zone pixel evidence.

- 🔧 UI scaling and touch-target audit across launcher and game views.
  original-table Theron extraction, and original Nexus input
  structures only via future Saturn capture work.
  Remaining: the CSB-native graphic-561 extraction once the file is
  remaining there is a real THQUEST.BIN disassembly before any

### Accessibility

- 🔧 Screen reader / launcher-state manifest: new `m12_launcher_a11y_emit` converts the public `M12_StartupMenuState` into the same `~/.firestaff/accessibility.json` schema M11 already writes, with stable element IDs (`GAME_CARD_DM1..THERON`, `MENU_SETTINGS`, `MENU_MUSEUM`, `TAB_*`, `ROW_*`, `POPUP_*`). `M12_StartupMenu_Draw` calls it on every frame when `fs_ax_is_enabled()` is true. New `firestaff_m12_launcher_screen_reader_manifest_probe` PASS 40/40 (envelope, main view, settings, popup, ordering, scaling). `tests/test_firestaff_accessibility_manifest.c` was already on disk; wired into CMakeLists.txt as `firestaff_accessibility_manifest_unit` (39/39 PASS). Privacy: data-dir line is suppressed by default and only emitted when the caller passes `includePaths=1`. Atomic write / no `.tmp` residue verified. **2026-06-27 bestiary / item encyclopedia / screenshot gallery / museum cell-by-cell manifest landed:** new element types `FS_AX_CATEGORY_TAB`, `FS_AX_BESTIARY_ROW`, `FS_AX_ITEM_ENCYCLOPEDIA_ROW`, `FS_AX_SCREENSHOT_THUMB`, `FS_AX_MUSEUM_CATEGORY`, `FS_AX_MUSEUM_BULLET` in `firestaff_accessibility.h`; new emit functions `emit_bestiary_view` / `emit_item_encyclopedia_view` / `emit_screenshot_gallery_view` / `emit_museum_view` in `menu_startup_a11y_m12.c` plus public `M12_Museum_GetCategoryTitle` / `M12_Museum_GetCategoryPageCount` / `M12_Museum_GetBullet` getters so the museum's private static data is reachable without leaking the table. Probe now PASS 69/69 (4 new subtests H/I/J/K cover category tabs, creature rows, item rows, thumbnail rows, museum sections, and page-driven bullet content). Companion `firestaff_accessibility_manifest_unit` still PASS 39/39. **2026-06-29 M11 gameplay manifest landed and hardened:** `m11_screen_reader_update_ex()` now emits deterministic gameplay-side zones for normal play plus inventory, automap, dialog, candidate mirror, and endgame, covered by CTest `m11_screen_reader_gameplay_state_manifest`; the follow-up CTest probe `m11_gameplay_screen_reader_manifest_probe` additionally gates open-chest slots/arrow-eye labels, classifier precedence, atomic-write/no-launcher-leak invariants, redraw idempotence, and per-state bounds containment, and fixed the endgame manifest coordinate bug where plaque/mirror/portrait zones were offset by the dungeon viewport origin instead of matching the framebuffer-space victory overlay. **2026-07-01 fallthrough views navigation-anchor gate landed:** new subtest N (`subtest_fallthrough_views_navigation_anchor`) in `firestaff_m12_launcher_screen_reader_manifest_probe` (now PASS 175/175, up from 91/91) pins the documented "out of scope" fallthrough contract for the 12 launcher views that intentionally fall through to `emit_main_view` as a navigation anchor: data-validator / audio-settings / accessibility / theme / save-browser / input-remap / custom-dungeon / campaign / spell-reference / map-viewer / touch-layout / presentation-preview. For each of the 12 views, the subtest asserts (a) the envelope `gameState` matches the active view (`launcher_<viewName>`), (b) the `GAME_CARD_DM1` / `MENU_SETTINGS` / `MENU_MUSEUM` / `GAME_CARD_THERON` navigation anchors are emitted, (c) the framebuffer dims stay pinned to 480x270, and (d) no foreign view markers leak into the manifest (the forbidden-marker table covers `ROW_*`, `TAB_*`, `CHANGELOG_LINE_*`, `MANUAL_DOC_*`, `BESTIARY_CAT_*`, `BESTIARY_ROW_*`, `ITEM_CAT_*`, `ITEM_ROW_*`, `SCREENSHOT_ROW_*`, `MUSEUM_CATEGORY_*`, `MUSEUM_BULLET_*`, `POPUP_MESSAGE`, `POPUP_OK`, `POPUP_LINE*`). A future dedicated emitter for any of these 12 views only needs to assert that the new view-specific markers appear AND the fallthrough markers disappear — the navigation-anchor contract is already pinned. Remaining work is real assistive-technology smoke evidence beyond JSON shape (the remaining 12 fallthrough views still intentionally fall through to main-view emission as a navigation anchor, now with the contract machine-checked).

### Build and CI Health

- 🔧 Watchdog parity-evidence manifests: parity-evidence files are refreshed by automated watchdog passes on every regression run. Manifests may report transient `FAIL` on gates whose line number has shifted (see the line-drift bullets above) or where a recent change has altered the test binary output; verify against the current source before treating any one FAIL as a real regression.

- 🔧 2026-07-10 release follow-up: GitHub Actions release run `29111129206` for `v3.0.71` is in progress after the tag push. Confirm all platform package jobs finish and that the published GitHub Release has the expected artifacts.

## Known Bugs

- 🔧 2026-07-18 Worktree-merge build drift (df88dbda4 + a192cb2b0) repaired:
  failures predate the repair in the sense that main could not build at
  all; they are tracked as jobb A–G follow-ups, not new regressions.

  - 2026-07-18 CSB triage follow-up (Jobb D): five repair commits
    **2026-07-31 re-verification:** this historical timeline-dispatch
    blocker is no longer reproducible on current main: all seven named
    F0242/F0248/F0190/F0249 CTests pass both once and through ten repeated
    executions each (70/70). Keep the wider DM1 suite audit open, but do
    not count these tests as current failing or flaky work.

- 🐛 Viewport/collision reports without capture manifests must stay as bugs until paired original PC 3.4 evidence or a reproducible local probe exists. **2026-06-28 TODO100 skip-safe scaffold landed:** `todo100_dm1_v1_viewport_collision_report_repro_gate` now CTest-gates the open-bug rule, writes `parity-evidence/verification/todo100_dm1_v1_viewport_collision_report_repro_gate/manifest.json` with status `BUG_OPEN_CAPTURE_MANIFEST_MISSING` when no operator capture directory is configured, and records the promotion contract in `parity-evidence/todo100_dm1_v1_viewport_collision_report_repro_gate.md`. This is not a bug closure, not a full collision transcript, and not an original-vs-Firestaff pixel diff; it only makes future unmanifested viewport/collision reports reproducible or explicitly skip-safe.

- 🐛 DM1 v2.9.0/latest macOS release (Mac Mini + MacBook Pro, Apple Silicon/arm64): Dungeon Master title/intro animation can show the wrong palette. Sibling to pass841 (FTL swoosh fix above). v2.7.11 title-palette fixes (`970d5607`, `09091de4`, `38d0b76b`) are present in v2.9.0, so not a recent-source regression. The title path uses `m11_unpack_title_4bpp_to_indexed` correctly. Probes cover the known palette layers on Apple Silicon: pass842 CPU fallback TITLE.DAT palette mapping, pass897 GPU/Metal `M11_Render_PresentIndexedWithSpecialPalette` readback, and the SWSH→GRAPHICS.DAT C001 handoff. 2026-07-13 source correction: M11 now rejects a handled DM1 TITLE receipt unless it matches ReDMCSB `TITLE.C F0437` C12 PRESENTS, C13/C14 zoom/reveal palette selection, and the corresponding VBlank cadence, rebuilding the canonical receipt otherwise. C13/C14 now also latches on the cleared indexed surface before the first zoom VBlank, matching TITLE.C:362-387; the focused fallback gate and `firestaff_m11` build are green. The user MacBook Pro release smoke remains open until a same-state app/release capture identifies the exact failing phase or proves it fixed in current main.

- 🐛 **DM1-SWSH-PSG-ORIGINAL-AUDIO:** The FTL swoosh currently consumes the
  exact 56-byte `V0901005_SoundCommands` stream, but its PCM renderer is an
  approximation rather than a proven Atari-ST PSG capture. ReDMCSB
  `SWSH.C:252-288` confirms the source stream only writes registers 06--09,
  0C and 0D; it relies on pre-existing PSG register state. Do not replace it
  with a generic effect or guessed complete YM2149 initialization. Obtain a
  same-version Atari-ST/DOS original capture or a verified initial PSG state,
  then bind a deterministic waveform regression before claiming sound parity.

- 🔧 Next: V2 material/pixel gates. DM1 V20/V21/V22 Apple-Silicon renderer readbacks are now covered by `dm_v20_filtered_renderer_silicon`, `dm_v21_upscale_renderer_silicon`, and `dm_v22_modern_renderer_silicon`; DM1 and CSB V2.2 in-place render paths both have cache-independent CTest probes (`dm1_v22_inplace_render_probe`, `csb_v22_inplace_render_probe`). DM1's synthetic probe now distinguishes wall/floor/pit/stairs/teleporter-field assets, blocks teleporter field-to-wall fallback, pins the synthetic material framebuffer signature, and verifies repeated-update plus 4-direction deterministic 9-cell selection; the M11 handoff is source-locked by `dm1_v22_m11_inplace_handoff_source_lock` so the draw path prefers `m11_v22_inplace_render_pass` with overlay fallback. 2026-07-14: the in-place resolver now refuses to substitute `creature_demon_01` for item/item-floor/item-projectile shapes; without an operator-reviewed V2.2 item material it returns no V2.2 asset and leaves the source-owned V1 pixels intact, with a focused CTest assertion. 2026-06-28: `dm1_v22_real_asset_material_gate_pc34` now runs a synthetic placeholder material-pixel fallback in CI when real hero art is absent, and `dm1_v22_finished_art_material_gate_pc34` + `firestaff_dm1_v22_finished_art_material_gate_probe` classify `NO_MANIFEST` / `SYNTHETIC_PLACEHOLDER` / `PARTIAL` / `FINISHED_REAL` manifests. 2026-06-29: the DM1 finished-art classifier now requires non-placeholder slots to resolve to PNG files whose signature and IHDR dimensions match the manifest before promotion to `REAL`; the same gate also reads optional `dm1_v22_real_screenshot_material_receipt_01` metadata under `runtime_screenshot_receipts`, distinguishes `NO_RECEIPT` / `SYNTHETIC_PLACEHOLDER` / `PARTIAL` / `FINISHED_REAL`, resolves receipt files only under `~/.firestaff/assets/dm1/modern/receipts/`, and requires both the seven-slot material gate and receipt gate to be `FINISHED_REAL` before treating a runtime screenshot/material receipt as final evidence. 2026-07-03: the active V22 source-owned/route/actual/Silicon screenshot probes now exercise the source-palette-aware material-shadow overlay path, normal CTest redirects their receipt output through `FIRESTAFF_PROBE_OUTPUT_ROOT`, the V22 shape-cache/overlay tests now prove explicit reset plus unpopulated-cache no-op behavior through CTest-registered probes, `m11_v22_inplace_draw_init()` now resolves its cache through the same `m11_v22_set_manifest_path(data/dm1)` modern-asset root used by M12 scanning/runtime launch, and the in-place blit now preserves V1 pixels under fully zero transparent cache pixels. The separate skip-safe operator-review receipt gate `dm1_v22_finished_pack_receipt_pc34` still reads `finish_receipt.json` and promotes only when the receipt hash matches the manifest, the material gate is `FINISHED_REAL`, and the reviewer covered every required slot. Remaining follow-up gates: (a) operator-reviewed DM1 V2.2 finished-art pack plus real screenshot receipt promoted to final local evidence; (b) later CSB real-asset pixel/material verification once the 9-square runtime draw passes consume finished modern art.

  - 2026-08-06 honest-receipt repair: `firestaff_dm1_v2_source_owned_screenshot_probe` no longer forces a modern-pack flag or records the unchanged V1 framebuffer as V2.2. It emits only authenticated V1/V2.0/V2.1 rows (12 rows) until a finished real V2.2 pack and reviewer receipt are present; the verifier accepts that explicit no-V2.2 state. The remaining V2.2 work is the real artpack and capture, not a synthetic distinctness claim.

- 🔧 Pool / heartbeat maintenance: 2026-06-23 monitor drained the stale champion-mirror visibility queue by narrowing that probe's data-root scan to `dm1/` when possible. Watch for recurrence of `firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe` being killed under the pool supervisor; no active `.job` file remained after this pass, so treat it as a recurring blocker only if the next refill reproduces it.

- 🔧 2026-06-22 DM1 V1 Hall of Champions south_return portrait_rect_position table follow-up: ordinal 13 (WUUF / THE BIKA) is now source-locked on the south_return route at (1, 5, SOUTH) → portrait_rect_position (96, 35, 32, 29) via `firestaff_dm1_v1_champion_mirror_portrait_13_south_return_portrait_rect_position_runtime_probe` (26/26 invariants on real PC 3.4 DUNGEON.DAT / GRAPHICS.DAT). **2026-06-28 PPM capture/readiness gate landed:** `firestaff_dm1_v1_hoc_ordinal_13_south_return_180_turn_capture_ppm_probe` skip-passes when the staged DM1 fixture does not expose WUUF at `(1,5,SOUTH)`; when the reference fixture is present, it drives the live M11 input path `(1,5,SOUTH)` WUUF → TURN_LEFT → `(1,5,EAST)` clear → TURN_LEFT → `(1,5,NORTH)` GANDO, writes build-local full-frame 320x200 and viewport-crop 224x136 PPMs plus a manifest, and verifies the final frame clears stale ordinal-13 pixels. Remaining open row: ordinal 13 south_return pixel parity against an original DM1 PC 3.4 captured frame at (1, 5, SOUTH) — needs a DOSBox `DM -vv -sn -pk` route to the south pose plus a 224x136 viewport crop under `verification-screens/`. Ordinal 13 east_walkpath route coverage already exists (`firestaff_dm1_v1_champion_portrait_13_east_walkpath_portrait_rect_position_runtime_probe`), and front_north_entry is covered by `firestaff_dm1_v1_hall_champion_portrait_13_front_north_entry_runtime_probe`.

- 🔧 DM1 V1 Hall of Champions portrait per-ordinal per-route aspect sweep: ongoing lane-by-lane proof that the 24 C026 atlas ordinals (col 0..7 row 0..2) render at the source-locked portrait_rect_position (96, 35, 32, 29) for the routes that have a C127 sensor in the local PC 3.4 fixture, and remain empty for the negative routes. Each slice = one runtime probe + ctest registration. **Done so far (gate IDs from the worker-pool lane labels):** ordinal 00 / south_return / portrait_rect_position, ordinal 01 / south_return + west_negative + front_east_entry + input_focus_restore / portrait_rect_position, ordinal 02 / south_return + west_negative + d2l_negative + leave_and_reenter / portrait_rect_position, ordinal 02 / palette_match_rect / portrait_rect_position (synthetic — mutates the (1,1,2) C127 sensor to ordinal 2; per-pixel VGA palette index match between the C026 ordinal-2 cell (64, 0, 32, 29) and the D1C rect; aggregate 100%, per-row 100%, per-column 100%, no-float 0% at (1,2) EAST, C040 panel drops to 0%, framebuffer byte-equal across two renders), ordinal 03 / east_walkpath / portrait_rect_position, ordinal 04 / south_return + east_walkpath + portrait_rect_position via sealed-chamber + after_party_shuffle portrait_rect_position (close_after_party_shuffle runtime slice — live BUG-120/121 panel lock + post-close F0284 rotation re-blt) + candidate_panel_open / portrait_rect_position (narrow panel-open slice at (2,1) DIR_SOUTH — C040 panel drawn >= 99% opaque pixels, C017 backdrop >= 95% opaque, C026 cutout strip-cell match <= 20% (LEIF atlas cell retains more opaque pixels in the cutout row band than HALK; observed ~9% on the shipped DM1 V1 DUNGEON.DAT vs the ordinal-1 sibling.s ~2%), status/inspect readout text, byte-stable draw stability, zone-identifier source-lock) + side_wall_negative (three source-visible wrong-wall approaches around the LEIF chamber — (1,2) EAST, (3,2) WEST, (2,3) NORTH — must not surface ordinal 4 in the D1C cutout or float over either side-wall band) + panel_chrome_preserve + sleep_repaint / portrait_rect_position, ordinal 05 / front_south_entry + candidate_panel_cancel / portrait_rect_position, ordinal 06 / south_return + west_negative + d2l_negative + input_focus_restore / portrait_rect_position back-route, ordinal 07 / south_return + east_walkpath + walkpath_from_stairs + turn_away_return + side_wall_negative (gate 271, batch group 11 — three source-visible wrong-wall approaches around the (2, 17) cell: (1, 17) EAST, (3, 17) WEST, (2, 18) NORTH; stale-positive seed at (2, 17) SOUTH then F0128 overpaint assert that the seeded ordinal-7 C026 sprite is cleared from the D1C rect and does not slide into either side-wall band; matches the ordinal-04 / ordinal-21 side_wall_negative envelopes at 35% leak tolerance) / portrait_rect_position, ordinal 08 / south_return + walkpath_from_entrance / portrait_rect_position, ordinal 09 / south_return + west_negative / portrait_rect_position, ordinal 10 / south_return + west_negative / portrait_rect_position, ordinal 11 / west_negative / portrait_rect_position, ordinal 12 / east_walkpath / portrait_rect_position, ordinal 13 / south_return + west_negative + front_north_entry / portrait_rect_position, ordinal 14 / south_return / portrait_rect_position, ordinal 15 / east_walkpath + west_negative / portrait_rect_position, ordinal 16 / south_return / portrait_rect_position, ordinal 17 / west_negative + approach_from_left / portrait_rect_position, ordinal 18 / approach_from_right / portrait_rect_position, ordinal 19 / east_walkpath / portrait_rect_position, ordinal 20 / (3,11) SOUTH / D1C portrait rect + turn_away_return / portrait_rect_position, ordinal 21 / south_return + west_negative / portrait_rect_position, ordinal 22 / front_north_entry + front_south_entry + walkpath_from_entrance + redraw_after_candidate + d2c_far_positive / portrait_rect_position, ordinal 23 / east_walkpath + palette_match_rect / portrait_rect_position. **Remaining gap:** no dedicated ordinal lane is currently known missing. Non-claim: the sweep proves Firestaff runtime correctness on the local PC 3.4 DUNGEON.DAT, not DOS pixel parity.

- 🔧 2026-06-24 DM1 V1 Hall of Champions ordinal-4 sleep_repaint follow-up: gate 172 covers the row-0 LEIF sleep/candidate-panel-return slice, and the 2026-06-25 `firestaff_dm1_v1_hoc_champion_portrait_04_wake_repaint_portrait_rect_position_runtime_probe` covers the separate LEIF wake_repaint variant. **2026-06-28 capture scaffold landed:** `firestaff_dm1_v1_hoc_ordinal4_resting_original_capture_scaffold_probe` now data-free gates the future original-capture geometry/route vocabulary: 320x200 screen, `(0,33,224,136)` viewport crop, LEIF ordinal-4 C026 atlas cell `(128,0,32,29)`, D1C portrait rect `(96,35,32,29)`, C145/C146 rest/wake route points, Firestaff RESTING overlay `(100,70,120,30)`, and stable shot labels/artifact root. Remaining follow-up is still the actual original DOS pixel-parity capture for the RESTING overlay; the scaffold does not commit screenshots or claim original-vs-Firestaff pixel equality.

- 🔧 2026-07-08 follow-up: CMake `cmake_check_build_system` still hangs in the existing `build/` tree after configure on this machine; investigate build-tree regeneration separately. Direct compile/tests were used for pass1105.

- 🔧 2026-07-08 follow-up: continue auditing probe-only and runtime helper paths that still mention canonical filenames; boot/runtime paths should keep moving toward hash/signature-first with filename fallback only for synthetic/custom fixtures.

- 🔧 2026-07-08 Nexus follow-up: extracted level/menu helpers still have some canonical DMDF/MNS material/model filename diagnostics; runtime boot marker, startup surfaces, MENU.BPK, DGN levels 00-15, SLEV00-15, SNDLEV00-15 SAL/MAP, and supplementary `.MNS`/`.DMDF` family loads now keep canonical runtime filenames authoritative and use MD5/signature fallback only for renamed/misplaced files. Remaining Nexus discovery work is real MENU.BPK/PRS3 corpus proof without reintroducing fallback-over-canonical runtime loads.

- 🔧 2026-07-08 Nexus follow-up: DGN/SLEV/SNDLEV runtime load now uses canonical filename first and MD5 fallback second; supplementary MNS/DMDF startup/runtime asset helpers now use exact-file plus DMDF-signature fallback for extracted assets and ISO-internal `.MNS`/`.DMDF` files.

- 🔧 2026-07-08 Nexus follow-up: `DM.BIN`, `LEV00.DGN` startup detection, DGN levels 00-15, SLEV00-15, SNDLEV00-15 SAL/MAP, and extracted/ISO-internal `.MNS`/`.DMDF` supplementary loads support renamed assets through MD5/signature fallback. Remaining Nexus work is broader PRS3/BPK material decode proof without reintroducing hash-over-canonical runtime loads.

- 🔧 2026-07-08 DM2 follow-up: M11 tick/move/turn, ACTION/front-cell, inventory slot swap, and V1/V2 render handoff now route through DM2 boot runtime receipts. Remaining DM2 cleanup is smaller read-only diagnostics/probe adapters.

- 🔧 2026-07-09 DM2 follow-up: packaged full-start now exports a DM2-owned consumer receipt for boot-probe and startup draw gates. Remaining boot/start cleanup is switching any out-of-scope engine callers to this receipt where they still read loose timing/asset fields.

- 🔧 2026-07-15 DM2 M11 map-transition receipt follow-up: indoor frames require an exact non-empty `UPDATE_GFXSET` WALL_GFX plan on both boot and M11 receipts. Outdoor maps may omit that indoor plan only when both source receipts explicitly report zero; an unexpected or stale plan blocks presentation. Remaining work is broader real GDAT scene/light/weather coverage across original save corpora.

- 🔧 2026-07-14 CSB real title/HUD/door capture follow-up: the staged-data M12-to-M11 launcher test now verifies C426 STRIKES BACK source bytes at their original geometry and palette, in addition to PRESENTS, CHAOS, opening-door, C017, and C040 captures. The package terminal receipt and fixture-free package probe now also require both distinct `TITLE.C F0437` CHAOS zoom and full-size hold phases before C017/C040 may be accepted. The legacy in-memory tile grid is now isolated to data-free test fixtures; live CSB resolves only loaded `DUNGEON.DAT` square and Thing records. Remaining work is manual Mac/app evidence and broader original-presentation comparison.

  - 2026-07-31 update: the local real-data capture contract passes title,
    Entrance and first-opening-door checks. The active translocated macOS
    `Firestaff.app` v3.0.195 independently captured the four real-data
    PRESENTS/FTL/STRIKES/Entrance palette phases, but its boot-probe bypassed
    Entrance and entered runtime before the Prison pointer. It therefore
    cannot be accepted as HUD or F0807 door-opening evidence. Re-run the
    bundle-bound capture against a package built from the current source;
    do not treat the older app as proof of the corrected door frame.

- 🔧 2026-07-14 CSBWin real-package resume follow-up: the opt-in package
  handoff probe now fingerprints the supplied decoded `Dungeon.dat` before
  the production resume attempt. A rejected `csbgame*.dat` must retain the
  original live dungeon bytes, owner, level, and empty runtime-save state; an
  accepted save must retain only exact serialized `TIMER`/`TimerQueue` slots
  after its first runtime tick. The probe has no generated dungeon, save, or
  fallback queue. Remaining work is a positive original CSBWin save corpus
  with broader DSA/door/world effects and package-app capture evidence.

- 🔧 2026-07-10 Nexus follow-up: remaining real-asset promotion is authentic Saturn palette/VDP1 capture comparison beyond the DGN material block. DMWeb-compatible MENU.BPK PRS3 decode and BPK/DMDF host-route receipts are verified; pixel-mode interpretation and presentation handoff remain gated.

  - 2026-08-08: `analyze_nexus_vdp2_composition.py` and
    `analyze_nexus_vdp2_bitmap_source.py` now accept an authenticated
    multi-frame count and inspect a selected frame without truncating the
    witness. The real eight-frame European sample remains `NBG1` bitmap
    (`BGON=0x0002`, `CHCTLA=0x1211`) at frame 7 with zero exact retail-source
    joins; this is observation coverage only and does not relax any gate.

  - 2026-07-15 MENU.BPK source-admission update: startup now binds the archive to the canonical retail Track 1 hash before it can produce a decode/upload receipt or reach the launcher. A parseable same-named archive remains blocked; the still-required work is an authentic PRS3 decoder/capture, not a substitute menu surface.

  - 2026-07-15 PRS3 promotion update: generic `ready-decoded` receipts are
    explicitly blocked on the retail menu route until authentic opcode and
    Saturn render provenance exists.

  - The same restriction now applies to the BPK-to-DGN material host route.

- 🔧 2026-07-13 DM2 G1 scene runtime follow-up: canonical G1 maps use
  `MapGraphicsStyle()` 1..5, and each exact `GRAPHICSSET` now has verified
  floor/ceiling IMG3 metadata plus source-required scene colorkey/flag words.
  The remaining runtime owner must remove its four-word readiness requirement
  and any cross-set scan: skproject map setup reads only the active set's
  `GDAT_GFXSET_SCENE_COLORKEY` and `GDAT_GFXSET_SCENE_FLAGS` before it queries
  that same set's floor and ceiling. Keep unproven light/weather semantics
  separate and fail closed; do not borrow another graphics set.

  - 2026-07-14 done: the real-data M11 handoff gate now audits every distinct
    G1-referenced `MapGraphicsStyle`, requiring each matching `GRAPHICSSET`
    floor/ceiling transaction to decode and render callback-free. The negative
    cross-set plan remains an explicit blocked no-draw check.

  - 2026-07-15 done: the M11 scene command builder now requires only the
    active set's scene colorkey/flags plus exact floor/ceiling GDAT material.
    Ambient light, highest-light, and ambient-darkness words are retained when
    present for the separate `c_light` receipt, but missing light words no
    longer block a real source scene or borrow another graphics set.

- 🔧 2026-07-13 Nexus Structure1Fa special floor images: ITEM.IBS descriptors, inherited local BGR555 palettes and canonical-corpus-proven `0008` packed-4bpp spans now reach their matching DGN floor-command consumer. The LEV00–LEV15 corpus now proves 446 direct item records, including 174 separate `0008` floor-image references, all resolving to verified ITEM.IBS sources with no missing or unsupported descriptor. The codec gate now distinguishes the documented VDP1 high-nibble rule from game-specific proof and blocks retail ITEM.IBS until an original Nexus VDP1 command stream binds the mode and byte route. Exact Saturn command provenance and 3D item placement remain unproven, so the material consumer stays no-draw with no inventory-icon or synthetic fallback.

  - 2026-07-14 packet-shape update: the gate now parses only a complete
    little-endian 32-byte VDP1 command record and requires its documented
    texture-source word, 4bpp colour-bank mode, and declared dimensions to
    match the descriptor. This validates a future packet's hardware framing;
    the focused fixture is synthetic and does not supply original-Saturn
    provenance. Canonical ITEM.IBS therefore remains no-draw until a genuine
    captured packet also binds command order, VDP1 state, palette state, and
    DGN placement.

- 🔧 2026-07-14 DM2 GDAT follow-up: canonical G1 GRAPHICSSET 1..5 floor and
  ceiling IMG3 material records now decode through their source-selected
  C4/C8 paths, and C8 accepts only skproject `DECODE_IMG9` selector layouts
  2 and 3. Remaining dungeon/HUD material work must bind only further
  source-proven GDAT category/field queries to runtime consumers; unknown
  IMG3 selector bytes remain no-draw.

  - 2026-07-16 done: `DM2_QUERY_GDAT_IMAGE_ENTRY_BUFF` and
    `DM2_QUERY_GDAT_IMAGE_METRICS` now have source-backed receipts over real
    parsed GDAT dtImage rows, including the skproject real `MISCELLANEOUS/FE/FE`
    default-image route when the requested image is absent. No generated image
    or synthetic HUD/dungeon visual is admitted. Remaining DM2 GDAT/HUD work is
    binding more DRAW_* consumers to these receipts and broader real-data
    runtime capture.

  - 2026-07-16 done: `DM2_QUERY_PICT_BITS` and
    `DM2_QUERY_4BPP_PICT_BUFF_AND_PAL` now preserve the skproject real-data
    admission routes for image descriptors and map-chip field `0xF9` plus local
    palette. Cached/current-bitmap paths require caller-owned evidence, and
    absent/non-4bpp/default-image routes fail closed. Remaining work is wiring
    more DRAW_* HUD/dungeon consumers to these receipts.

  - 2026-07-16 done: `DM2_QUERY_PICST_IMAGE` and
    `DM2_QUERY_GDAT_SUMMARY_IMAGE` now bind picture descriptors to real GDAT
    image-entry metadata and local palettes. The source `cls1 == 0xff` summary
    bypass is preserved as no-GDAT/no-draw state. Remaining work is DRAW_*
    consumer wiring and broader live HUD/dungeon capture.

- [ ] Nexus Structure3 face rendering capture: bind an original Saturn trace
  from face rows to transform, winding/culling, normal use, fill-selector
  texture and palette data, and a concrete draw command. The retained
  entry-local pair multiplicity is row incidence only until this exists.

  - 2026-08-08: regenerated the source-bound all-level campaign from the
    real LEV00.DGN–LEV15.DGN corpus: 0x482e (18,478) no-draw face targets,
    `original_saturn_capture_required=1`, `decoder_or_renderer_authorized=0`.
    The remaining work is the corresponding authentic Saturn face trace, not
    another synthetic target or renderer.

  - 2026-07-14: `nexus_v1_dgn_bind_structure3_face_capture_candidate` now
    defines the fail-closed byte-binding boundary for that trace: it requires
    the retail typed-row corpus identity, exact DGN and Structure3 payload
    hashes, entry/face row, referenced vertices, normal row, fill selector,
    texture span, palette state, VDP1 state, transform/culling state, and
    command bytes to match together. No capture is currently supplied, so the
    binding cannot verify original-Saturn provenance and always blocks DGN
    mesh rendering. The next increment is importing a genuine capture packet,
    not constructing palette, pixel, VDP1, or fallback evidence.

  - 2026-07-14 follow-up: the candidate binder now also requires separate,
    caller-owned canonical-DGN hash admission. A packet cannot self-admit
    arbitrary or fixture bytes by repeating their fingerprint. This remains a
    no-draw boundary; a genuine original-Saturn trace importer is still
    required.

  - 2026-07-14 capture-admission follow-up: the trace source itself is now a
    separate caller-owned gate. A hash-verified DGN plus matching fixture
    bytes cannot form a complete binding without a verified original-Saturn
    capture manifest. No such manifest or trace has been imported.

  - 2026-07-14 host-route follow-up: real DGN material-plan assembly now also
    rechecks the retained canonical `LEVxx.DGN` byte identity before either
    MNS or BPK material routes can become presentable. A stale or mutated
    in-memory level therefore blocks the host route rather than reusing a
    file receipt. This is package-consumption hardening only; it does not
    establish the required Saturn texture/palette or mesh semantics.

  - 2026-07-14 raw-trace tooling follow-up: the capture path can now read six
    immutable raw trace spans from files and atomically validate their exact
    manifest hashes, session, and externally attested bundle before exposing
    the existing no-draw import packet. There is still no local genuine
    Saturn trace, and the reader deliberately assigns no VDP1, palette,
    transform, culling, texture, or draw semantics.

  - 2026-07-14 package-to-host follow-up: the same reader-owned packet now
    crosses into DGN host intake only in manifest order after all six spans,
    session, bundle, and external Saturn attestation match. A changed lane
    stops before host intake. This is opaque transport only, not a claim about
    an original Saturn capture-file format or any VDP1/palette/pixel semantics.

  - 2026-07-14 trace-order follow-up: the manifest now records all six opaque
    observation ordinals and the reader requires an external attestation of
    their exact lane-to-ordinal relation. Missing, duplicate, out-of-window,
    or mismatched order evidence blocks before host intake. This validates
    transport chronology only; actual Saturn render semantics remain unknown.

  - 2026-07-14 runtime-provenance follow-up: the attested trace-order identity
    now survives raw import, host intake, engine-owned Structure3 storage, and
    the viewport packet gate. An otherwise byte-complete packet lacking that
    verification is rejected. This preserves evidence transport only; it does
    not establish any Saturn graphics semantics or relax no-draw.

  - 2026-07-14 launcher-route follow-up: the launcher now supplies only its
    currently owned, hash-verified canonical DGN bytes to the strict raw
    capture reader/host path. It fails before reading any capture if no real
    loaded level exists, and the admitted source remains no-draw. A genuine
    original-Saturn capture is still required for render semantics.

  - 2026-07-15 VDP1 command-admission update: an already source-bound PRS3
    V3 command sidecar can now be checked as one complete documented 32-byte
    VDP1 texture command before any later capture analysis. Independent Saturn
    provenance, PRS3 opcode grammar, pixel mode, palette format, and runtime
    rendering remain explicitly unproved and blocked.

  - 2026-07-15 executable-anchor update: canonical retail `DM.BIN` contains
    one exact eight-word big-endian table at `+0x77114` with the VDP1 register
    base `0x25D00000` and `+0x10`. Firestaff now hashes and retains this as a
    static capture-producer anchor only. It does not prove SH-2 writes,
    command-list emission, DGN ownership, palette/transform semantics, or a
    drawable primitive. The next evidence must be a real execution trace that
    connects this executable route to a command snapshot and active DGN face.

  - 2026-07-15 state-route update: a second unique `DM.BIN` table at
    `+0x7d498` is statically consumed by 21 decoded SH-2 PC-relative literal
    loads. Six loads target VDP1 register literals and one targets
    `0x25C00000` (VDP1 VRAM), making it a verified command-storage candidate
    in the original executable. This is not execution evidence: command
    production, active DGN ownership, transform, palette, and drawing still
    require an authentic Saturn trace.

  - 2026-07-15 static-write update: a bounded original SH-2 corridor now
    proves three `MOV.W` state writes from that map: `0x8000` to VDP1 `+0x06`,
    zero to `+0x08`, and `0xffff` to `+0x0a`. This establishes only static
    code/dataflow, not that the routine executes for an active scene. A real
    trace must still connect emitted VDP1 command bytes, transform state, and
    palette state to an active DGN face before rendering is possible.

  - 2026-07-15 command-control update: the same static corridor also proves
    `MOV #2,R1`, a PC-relative load of `0x25D00004` into `R0`, then
    `MOV.W R1,@R0`. This is concrete original code for a VDP1 `+0x04`
    control-register write, but not proof that a frame executes it or that a
    command list is emitted. Preserve no-draw until a real Saturn trace joins
    this path to VDP1 command bytes, palette state, transform state, and DGN.

  - 2026-07-15 VDP1-VRAM handoff update: `DM.BIN+0x7d3e8` loads its verified
    `0x25C00000` literal, immediately followed by `MOV.L R2,@R14`. Firestaff
    retains this exact original memory handoff but does not assign an owner to
    `R14`, infer a command-list address or bytes, or claim execution. A trace
    must still bind palette/transform state and an active DGN face.

- [ ] Nexus Structure3 real-dungeon geometry and texture decode capture:

  - 2026-07-15 direct face trace-route update: transform trace intake now
    requires a separate, exact direct-face package target file before an
    independently attested Saturn trace can enter the opaque host receipt.
    This joins the capture route to original LEV ownership, but does not prove
    a transform, material, palette, pixels, VDP1, culling, or drawing.

  - 2026-07-15 direct face host-consumption update: the package boundary now
    re-derives and validates every direct Structure1F face manifest field
    against the active canonical LEV before accepting it as a no-draw capture
    request. A manifest is not a Saturn observation: transform, culling,
    material, VDP1, palette, pixels, and drawing remain blocked.

  - 2026-07-15 direct owner bridge update: a selected active Structure1F row
    can now emit one atomic no-draw capture request containing its canonical
    LEV hash, exact Structure1A/Structure3 owner fields, face/vertex/normal
    row hashes, parsed vertex indexes, and raw transform-table fingerprints.
    It is capture-producer input only; Saturn execution must still establish
    transform, culling, material, VDP1, palette, and draw semantics.

  - 2026-07-15 raw-capture host update: the launcher now joins a direct
    Structure1F manifest to a six-lane Structure3 capture only when every
    original-Saturn attestation, lane hash/order, canonical DGN binder field,
    and the full direct face candidate agree. Accepted bytes remain opaque and
    no-draw. The remaining work is a genuine Saturn capture that proves VDP1
    command fields, texture pixels, palette/CRAM relation, and transform
    semantics for that exact face; do not promote an opaque lane or static
    DM.BIN VDP1 state into a visual command.

  - 2026-07-15 VDP1 material-link update: the direct-face route now rechecks
    its engine-owned texture, palette, VDP1 state, and command copies against
    the authenticated capture before accepting a unique VDP1 command and its
    exact CMDSRCA texture window. It retains the raw `CMDCOLR` word beside the
    hash-bound palette lane, but does not interpret it as a CRAM/CLUT address
    or decode any texel. The remaining work is a real Saturn capture proving
    that command's palette addressing/format and pixel order for this exact
    Structure1F face; all drawing remains blocked.

  - 2026-07-15 mode-1 lookup decoder update: the Sega VDP1 manual proves one
    bounded source path: 4bpp mode-1 texture bytes are high-nibble first and
    `CMDCOLR * 8` selects a 32-byte, 16-entry lookup table in VDP1 VRAM.
    Firestaff now decodes only those raw 16-bit VDP1 colour codes after the
    direct capture, full VRAM, and exact CMDSRCA window gates pass. The
    remaining requirement is a real Nexus capture whose command is mode 1 and
    whose independently observed colour-code output proves the game-specific
    VDP2/CRAM handling; raw codes are not RGBA pixels and cannot be drawn.

  - 2026-07-15 mode-1 palette-chain update: VDP1 `CMDPMOD` now supplies the
    documented source-index-0 transparency and source-index-F end-code gates;
    lookup words with bit 15 set resolve as direct RGB555, while colour-bank
    words use captured VDP2 `SPCAOS`, `RAMCTL/CRMD`, and the complete 4 KiB
    CRAM image to select the exact RGB555/RGB888 entry. The decoder rejects a
    partial CRAM/register image, prohibited CRMD=3, or an unattested capture,
    and deliberately emits no host draw. What remains is an independently
    authenticated Nexus Saturn capture containing this complete VDP2 state
    plus the final VDP1/VDP2 output pixels; only a byte-for-byte match against
    that output can promote this hardware chain into real DGN rendering.

  - 2026-07-15 PRS3 V4 capture-intake update: complete-stream traces now have
    a strict schema for the nonzero `R6 -> R0`, `R2 -> @(R13,R0)` store lane,
    its contiguous output address range, sequence interval, and byte witness.
    The asset binder rechecks the two store PCs against the original `DM.BIN`
    SH-2 receipt. The trace itself remains untrusted external evidence: it
    does not prove PRS3 grammar or authorize decoding, palette use, VDP1
    upload, or drawing.

  - 2026-07-15 first-operation update: the V5 receipt can now join the
    source-bound nonzero SH-2 byte path to one exact `MENU.BPK` input/output
    position. It is deliberately an opaque byte-emission candidate, not a
    literal/token rule: the local corpus has no independently authenticated
    Saturn trace, so token grammar and all decoder/render permissions remain
    blocked.

  - 2026-07-15 dual-branch review update: V5 now joins the bounded nonzero
    byte-emission path and the independent zero-side two-byte merge under one
    full-stream receipt. This proves capture coverage, not grammar: neither
    branch has a literal/copy/length/offset meaning until authenticated Saturn
    execution evidence supplies those facts.

  - 2026-07-15 FACE capture-target update: each canonical `FACE.BIN` PRS3
    frame can now be requested with separately hashed prefix, PRS3-header,
    and stream lanes under a caller-owned retail-source gate. The next needed
    evidence is an authentic Saturn loader trace joining one target to input,
    output, palette, and menu placement; the target itself proves none of
    those semantics and cannot decode or draw a portrait.

  - 2026-07-15 FACE complete-campaign update: all 20 source-hash-gated frame
    targets now form one ordered byte-lane ledger. Missing, reordered, or
    substituted frame requests change the campaign identity before trace
    analysis. It still requires a genuine Saturn capture to establish loader
    execution, token grammar, palette, and menu placement; no portrait route
    is admitted.

  - 2026-07-15 Structure2 payload-anchor update: every active descriptor's
    image anchor and each nonzero palette anchor now reaches the hash-bound
    viewport boundary with the next observed anchor (or bounded payload end).
    These intervals are capture framing only, not image lengths, palette
    lengths, texels, colours, encoding claims, VDP1 state, decoder input, or
    draw authorization. An authentic Saturn trace must still establish every
    one of those semantics independently.

  - 2026-07-15 Structure1C source-route update: every addressable bounded
    four-byte Structure1C record now reaches the active hash-bound viewport
    boundary with its Structure1B reference occurrence retained. The bytes
    remain opaque: they do not establish collision shapes, blocking,
    transforms, meshes, materials, pixels, VDP1 state, or drawing.

  - 2026-07-15 source-route update: every parsed Structure1F row now reaches
    the viewport boundary from the active hash-bound LEV, retaining direct
    coordinates and Structure1A-owned rows separately. This is source
    transport only: object placement, trigger behavior, transforms, meshes,
    texture/palette/VDP1 semantics, and drawing remain blocked pending an
    original Saturn capture.

  - 2026-07-15 Structure3 face-campaign update:
    `firestaff_nexus_v1_structure3_face_capture_campaign` now walks each
    canonical LEV directly by MD5 and emits one no-draw request for every
    bounded Structure3 face/mesh row. The all-level retail corpus yields
    18,478 targets while requiring complete Structure1F/1A face-normal
    attachment evidence. The requests deliberately retain
    `structure1a_model_entry_mapping_proven=0`; original Saturn capture must
    still establish that relation, texture/palette/VDP1 semantics, transforms,
    culling, and any decoder or draw route.

  - 2026-07-15 campaign-ledger update: the all-face producer now writes one
    ordered, source-fingerprinted campaign ledger beside its individual
    no-draw targets. It detects missing, reordered, or substituted target
    requests without treating the ledger as a Saturn trace. Model-to-entry,
    texture, palette, VDP1, transform, culling, decoder, and draw semantics
    remain unproven and blocked.

  - 2026-07-15 campaign-verification update: the verifier rebuilds every
    target from the canonical LEV00--LEV15 sources and requires the published
    target bytes and ledger fingerprints to agree. This protects acquisition
    evidence only; it is not a Saturn trace and cannot promote mesh, texture,
    palette, VDP1, decoder, or draw semantics.

  - 2026-07-15 update: the active canonical LEV route can now atomically emit
    its verified dual-source Structure1F/Structure1A and Structure3-face
    target for an external capture producer. The written request contains no
    capture lanes or decoder output; unresolved Saturn semantics remain
    fail-closed.

  - 2026-07-15 update: external dual-source capture targets now originate at
    the active engine package route. The engine requires retained canonical
    LEV bytes and a complete live Structure1F/Structure1A topology candidate
    before it emits the requested Structure3 face target. This is producer
    acquisition plumbing only; model-entry mapping, texture, palette, VDP1,
    transform, and draw semantics remain unproven and no-draw.

  - 2026-07-15 update: an independently attested Structure3 raw capture can
    now retain one revalidated Structure1F/Structure1A owner context in the
    engine-owned runtime packet. The active canonical LEV bytes, exact owner
    row, Structure1A model row, and independently selected face target are
    rebuilt before storage; a changed target cannot replace an admitted
    correlation. This still proves no model-index-to-entry relation and keeps
    texture, palette, VDP1, transform, and draw semantics fail-closed.

  - 2026-07-15 direct owner/material update: documented active
    `Structure1F -> Structure1A -> Structure3` entry/face rows now join their
    exact static Structure2 descriptor and bounded image/palette candidate
    lanes when the referenced face is source-proven static. Animated,
    untextured, or unresolved faces remain unavailable. A genuine Saturn
    trace must still establish the payload format, palette, transform, VDP1,
    and draw behavior; no material is decoded or rendered here.

  - 2026-07-15 direct owner/raw-fill update: a direct Structure1F owner can
    now retain its exact non-textured Structure3 face, vertices, normal, and
    raw fill selector when that is what the source row selects. The selector
    remains opaque: an authentic Saturn trace must still prove flat-fill,
    palette, transform, VDP1, and draw semantics. Textured faces cannot enter
    this route and no substitute colour or texture is allowed.

  - 2026-07-15 direct owner/08xx update: direct Structure1F owner rows can
  the capture reproduces the observed command and rejects altered source rows,
  selectors, texture bytes, palette state, and trace coordinates atomically.

- 🔧 2026-07-14 coalesced `$e009` manifest-to-runtime binding: the
  initial-level handoff now carries a receipt hash bound to the selected
  original Track 02, System Card 3.0, and exact coalesced trace MD5s. Runtime
  consumes the opaque 2048-byte payload only when that binding and every
  source receipt remain complete; altered, missing, or cross-capture evidence
  fails closed without a generated bitmap or dungeon fallback. The remaining
  gap is still a positive original capture that assigns actual dungeon-route
  semantics to this authenticated byte transfer.

- 🔧 2026-07-14 authenticated Hall of Records route receiver: the completed
  manifest-bound handoff now re-derives and compares the source-locked level-0
  route before atomically publishing its loader-accepted grid to runtime. The
  receiver rejects altered payloads, route hashes, source receipts, and any
  object-tail or fallback flag. It deliberately receives no objects, bitmap,
  palette, or broader transition interpretation; those require separate
  original execution evidence.

- 🔧 2026-07-15 Theron startup synthetic-visual audit: verified Track 02
  title, stage, Soul Room, and forcefield atlas routes now own startup
  presentation exclusively. Firestaff plan fill/border/cursor rectangles are
  suppressed on that route rather than painted over original pixels. Regions
  without authenticated loader/CD bitmap evidence remain untouched; the
  remaining gap is positive original evidence for the missing panel/cursor
  art, not a generated replacement.

  - 2026-07-15 update: the legacy `ui_chrome` compositor is also blocked once
    the authenticated atlas is restored. Its topbar, right-panel, champion,
    glyph, bar, and compass blocks have no bound original UI bank; it may
    remain available only before source-owned startup media exists.

  - 2026-07-15 receipt update: all five startup graphics receipt wrappers now
    require an executed Track 02 atlas, a nonzero source-pixel receipt, and
    zero generated fill/border calls. This proves the presentation boundary
    remains complete while unbound UI art is fail-closed.

  - 2026-07-15 correction: source-atlas execution previously cleared each
    destination rectangle with a generated zero-colour fill before copying
    authentic pixels. The clear is removed; atlas presentation now has only
    authenticated `plot_pixel` output and leaves any unbound area untouched.

  - 2026-07-15 runtime tile correction: the old viewport and standalone tile
    renderer also used generated black clears and palette-7 gray squares for
    an unbound dungeon tile bank. Both now preserve the existing surface and
    draw only supplied tile bytes. The remaining gap is a positive original
    Track 02 loader/CD capture that binds level records to tile/palette banks.

  - 2026-07-15 runtime UI correction: both active V1 chrome compositors now
    reject their generated bars, glyphs, portraits, compass, and stat panels.
    The known startup atlas is not promoted as a runtime UI bank; a positive
    loader/CD capture must bind that bank before any UI pixel is emitted.

  - 2026-07-15 Track03/04 correction: legacy `THG3` tile and `THS4` audio
    marker parsers are hard-rejected. Retail CUE provenance has only Track 02
    as MODE1 data, so the remaining real-data job is a captured HuC6280
    loader/CD byte-span binding for a bitmap, object, world, UI, or audio
    consumer; marker syntax alone cannot create an atlas or playback route.

  - 2026-07-27 update: verified JP/US Track 02 now blocks rendering from any
    preinitialized default tile/palette state unless an original graphics bank
    and verified palette route are both present. Runtime capture rejects a
    deterministic-fallback marker, and current JP/US/canonical captures report
    no fallback assets. The remaining work is still positive original graphics
    bank/loader evidence, not a generated substitute.

- 🔧 2026-07-15 CSB M11 terminal host-surface consumer: M11 now consumes the
  session-owned, hash-bound host-surface receipt for C001-C005/C017/C040 and
  leaves its existing host page untouched when the receipt is incomplete.
  This removes the local black-frame replacement path. Remaining CSB startup
  work is real package/app capture breadth, not substitute presentation.

- 🔧 2026-07-15 CSB M11 live HUD/viewport transaction: the F0128/F0115
  caller now renders its complete verified viewport into a candidate host
  page and publishes it only on success. C017/C040 is likewise validated as
  one terminal-session pair before either surface changes the page. Missing
  source receipts preserve the previously presented page; remaining work is
  real package/app capture breadth and original-save/DSA corpus evidence.
  The F0098 PC3.4 baseline now receives the actual C079/C078 package aperture
  through the same M11 session binding; remaining viewport work is real wall,
  door, and object bitmap ownership rather than a floor/ceiling substitute.

- 🔧 2026-07-15 CSB viewport/HUD lane separation: the F0128 boot viewport
  now applies only real CSBgraphics derived-viewport entries. C017/C040 stay
  in the source-ordered PANEL.C terminal-session consumer and cannot replace
  every dungeon frame. Remaining work is an actual CSB package/app capture
  across the title, door, HUD, and panel transitions.

- 🔧 2026-07-15 CSB startup fallback cleanup: the obsolete M11 render-plan
  executor, including its black entrance-aperture path, is removed. Startup
  presents only the verified session-owned C001-C005/C017/C040 host surface.
  Remaining work is real package/app capture across all title and entrance
  phases, plus real DSA/save corpus evidence.

- 🔧 2026-07-15 CSB C001 title-plan cleanup: PRESENTS/CHAOS/STRIKES plans
  now contain only their ReDMCSB C424/C425/C426 source regions and palettes;
  they no longer construct replacement text. Remaining work is capture of
  those real title phases in a packaged app, not a synthetic title route.

- 🔧 2026-07-15 CSB presented-frame capture: M11 now hashes the exact
  indexed frame passed to the renderer after a successful presentation,
  including the CSB V2 presentation filter when selected. Remaining work is
  a real packaged-app/Mac capture, not another host-side frame substitute.

- 🔧 2026-07-15 Nexus DGN Structure1F active face/mesh route: active,
  not an entry grammar, mesh, face, texture, material, or draw contract; no
  retail corpus result exists to promote it to DONE.
  transform, texture, material, or draw action, and remains outside DONE until
  index, and byte 2 as an opaque rotation selector; bytes 3--23 remain an

- 🔧 2026-07-15 Nexus active DGN face/material selector route: active retail
  LEV sources now bind bounded Structure3 face topology to the documented
  Structure2/Structure1G selector joins. This remains identifier provenance,
  not material/pixel/palette/UV/VDP1/draw semantics. Original Saturn capture
  evidence is still required before any renderer promotion.

- 🔧 2026-07-15 Nexus active Structure1A owner chain: the active canonical
  LEV now carries the complete Structure1F -> Structure1B owner ->
  Structure1A -> model/face selector chain. Owner coordinates, selectors, and
  model indexes remain source evidence only; original Saturn capture is still
  required for placement, transforms, materials, pixels, and drawing.

- 🔧 2026-07-15 Nexus active Structure2 descriptor envelope: canonical LEV
  runtime now consumes the bounded descriptor table, opaque post-FFFF span,
  and verified Structure1G global-to-local descriptor joins. The payload has
  no proven encoding, pixel ordering, palette layout, animation timing, or
  VDP1 route; original Saturn evidence remains required before drawing.

- 🔧 2026-07-15 Nexus Structure2 capture acquisition: the active canonical
  LEV can now emit a source-bound capture target for one exact Structure2
  descriptor and its opaque post-FFFF payload span. The remaining work is an
  authentic Saturn trace/frame that binds those bytes to a real decoder and
  palette/VDP route; the target itself is never a decoder or runtime fallback.

- 🔧 2026-07-15 Nexus Structure2 Saturn trace import: a raw external trace can
  now be bound to the active LEV, exact descriptor, and opaque payload hashes,
  but local corpus contains no authentic Saturn trace. An independent capture
  owner must verify its provenance before opaque admission; pixel/palette/VDP1
  semantics, decoder selection, and all drawing remain blocked.

- 🔧 2026-07-15 Nexus Structure3 static-material capture: hash-verified
  LEV00–LEV15 data now yields a capture target that joins one real textured
  Structure3 face row to its exact static Structure2 descriptor, image-payload
  byte anchor, and optional palette-payload byte anchor. This is source
  acquisition only. Authentic Saturn trace evidence must still prove pixel
  order, palette addressing, UV/VDP1 semantics, and the render route before
  any decoder or draw can be enabled.

- 🔧 2026-07-15 Nexus Structure3 package-geometry route: the active canonical
  och transformbevis saknas fortfarande och all presentation är fortsatt
  blockerad.
  and otherwise unproven faces remain outside that route. Traversal is still

- 🔧 2026-07-15 Nexus Structure2 pixel/palette format gate: the real retail
  corpus has only raw descriptor classes `0x0008` (1,553 rows) and `0x0028`
  (125 rows). Every image target is an in-payload anchor; `0x0028` has no
  palette anchor while `0x0008` has a mixed present/absent palette split. This
  rules out selecting an indexed or direct-color decoder from descriptor bytes
  alone. A genuine Saturn trace must still establish pixel span/order, absent
  palette behavior, palette entry format, and VDP1 command mode.

- 🔧 2026-07-15 Nexus Structure3 captured-VDP1 framing: an independently
  authenticated Structure3 capture now carries its complete 32-byte VDP1
  command through the active canonical LEV renderer and viewport receipts.
  The documented command-table fields are parsed only after all capture lanes
  and the exact DGN face binding have been revalidated. This does not prove
  source-address byte semantics, colour mode, pixel order, CLUT/palette
  addressing, transform/culling, or drawing; each remains blocked pending a
  real retail Saturn capture with those relations.

  - 2026-07-15 command-table correction: the shared VDP1 parser now follows
    the documented 32-byte table offsets `CMDCTRL +00`, `CMDLINK +02`,
    `CMDPMOD +04`, `CMDCOLR +06`, `CMDSRCA +08`, and `CMDSIZE +0A`.
    `CMDCOLR` is retained as an opaque raw word; this correction does not
    infer a palette, CLUT, texture byte span, or draw route.

  - 2026-07-15 format-span update: for an authenticated texture primitive,
    documented `CMDPMOD` modes 0--5 and `CMDSIZE` now determine only the
    command-required byte count (4/8/16 bpp). The captured texture lane must
    exactly match that count before the renderer marks it format-framed.
    Mode 6/7, palette/CLUT meaning, source-address interpretation, pixel
    ordering, and all decoding or drawing remain fail-closed.

  - 2026-07-15 binding-admission update: Structure3 capture binding now
    rejects a malformed/non-texture command, undocumented colour mode, or
    hash-matched texture lane whose length disagrees with the documented VDP1
    command requirement. This protects engine-owned storage as well as the
    viewport; it still does not identify source bytes, palette data, or texels.

  - 2026-07-15 CMDSRCA range update: a texture command now derives its local
    VDP1-VRAM byte interval from the documented `CMDSRCA * 8` address unit and
    rejects ranges outside the 512 KiB VDP1 VRAM. This is a command-local
    bound only: an authentic trace must still prove that its captured texture
    lane was read from that VRAM interval.

  - 2026-07-15 VDP1-VRAM snapshot update: when an independently authenticated
    capture supplies the complete 512 KiB VDP1 VRAM, the exact CMDSRCA window
    is now compared byte-for-byte to the captured texture lane before it is
    exposed to renderer or viewport receipts. Partial `vdp1_state` lanes stay
    opaque and cannot establish this relation. Pixel order, colour expansion,
    palette/CLUT state, and draw semantics remain unproved and blocked.

  - 2026-07-15 coordinate-framing update: authenticated commands now retain
    the documented signed `CMDXA`--`CMDYD` and `CMDGRDA` words through the
    active renderer/viewport receipts. A real Saturn trace must still bind
    those words to Structure3 transforms, camera, clipping, Gouraud data, and
    command ordering before any geometry can draw.

  - 2026-07-15 command-VRAM update: a full authenticated VDP1-VRAM snapshot
    must now contain the exact captured command once at an eight-byte boundary
    before its documented `CMDLINK * 8` address is retained. Command-flow
    control bits, target-table identity, palette/CLUT behavior, texel format,
    and raster output remain unproved and blocked pending a real trace.

  - 2026-07-15 link-target update: the bounded `CMDLINK` target is now framed
    as a complete raw command from the same authenticated snapshot. A real
    Saturn trace must still prove control-flow selection, target command role,
    ordering, palette/CLUT state, texel semantics, and output before drawing.

  - 2026-07-15 runtime-admission update: external Structure3 captures now
    require a complete 512 KiB VDP1 snapshot, an exact CMDSRCA texture-window
    match, and a unique command-lane occurrence before engine storage. This
    proves no palette/CLUT source or format, command flow, pixel ordering,
    transform, culling, raster, or draw behavior; those remain blocked.

  - 2026-07-15 source-palette update: generated Nexus palette fallbacks are
    disabled; incomplete source spans now block indexed lookup. The remaining
    task is a positive authentic Saturn capture that proves the palette source
    and format plus its `CMDCOLR`/CLUT relation for an admitted DGN command.
    The next implementation step must consume that authentic trace; no legacy
    palette accessor or flat-colour substitute remains available.

  - 2026-07-15 real-corpus capture-tool update: use the emitted hash-bound
    Structure2 target with an independently authenticated Saturn producer to
    obtain the descriptor's palette-state, VDP1 command/VRAM, and texture
    lanes. Only then may the actual pixel/palette format work begin.
    The tool can now emit every descriptor target in a selected LEV, allowing
    one authenticated Saturn campaign to cover all active Structure2 classes
    instead of choosing a representative byte pattern.
    The all-level campaign now emits 1,678 targets from LEV00--LEV15. The
    installed Mednafen 1.32.1 reaches its Saturn module but cannot boot this
    corpus without the configured `sega_101.bin` BIOS; provide a legal local
    BIOS, then capture the VDP1/CRAM lanes against these targets. Do not use a
    substitute BIOS, generated trace, or inferred decoder.

- 2026-07-15 DM1 GROUP F0181 follow-up: the source C29..C41 event deletion
  primitive is now DM1-owned and source-locked. Remaining work is having
  each live group-retirement owner consume this common primitive rather than
  duplicating its bounded cleanup loop; delete the complete C29..C41 range
  but retain all other maps and squares exactly as GROUP.C does.

- 2026-07-15 DM1 GROUP F0194 follow-up: source-defined active-group
  retirement now writes raw loaded C04 cells, low packed direction, and the
  F0184 behavior threshold before retiring every slot. F0195 now consumes
  loaded C04 chains for initial map activation; remaining work is to route
  later M10 map/save handoffs through the same retirement boundary. Malformed
  C04 references must remain a no-mutation failure, without default groups.

- 2026-07-15 DM1 GROUP F0195 follow-up: initial PC3.4 startup now consumes
  the loaded map's native SFT/C04 chains in source order, deleting exact-square
  C29..C41 events and scheduling C37 at GameTime + 1. Remaining work is the
  F0194/F0195 pair for later map and original-save transitions; do not retain
  an old active table or manufacture a missing C04 group/square chain.

- 2026-07-15 DM1 GROUP F0201 follow-up: M10 now consumes a bounded,
  fingerprint-locked G0407 runtime scent receipt through the current loaded
  map and F0198/F0199 route. The remaining gap is a raw PC34 save/import
  owner for that opaque scent ring; no missing import record may be replaced
  with a guessed direction or callback.

- 2026-07-17 DM1 GROUP F0205/F0206 follow-up: M10 C37 now consumes the
  source live-RNG direction form, retains the full authenticated
  `ACTIVE_GROUP::Directions` byte, and writes only its low slot to raw C04.
  C38-C41 now own their source line-of-attack F0205 turn and same-event
  two-tick retry. C29/F0267 remains an explicit no-op without source-backed
  physical-relink ownership. F0219 now owns the live C14/C48-C49 motion
  receipt and rejects a drifted raw C14 mirror before mutation; remaining
  projectile work is broader F0212/F0215 lifecycle coverage, not a fallback
  motion path.

- 2026-07-17 DM1 PROJEXPL F0215 follow-up: the verified C14 delete tail now
  consumes the matching raw C05 potion record when the authenticated F0217
  receipt requires it. The C15 owner now has a verified F0516-backed reserve,
  source-layout initialization, F0514 SFT link and F0515/raw rollback
  transaction for Type/Attack/Centered rows. Its C25 receipt now binds exact
  MapTime/B.Location/C.Slot plus the live C15 FNV through F0802/F0435.
  Remaining work is F0217 consumption of that authenticated handoff; do not
  invent an explosion Thing for a runtime-only explosion slot.

- 2026-07-17 DM1 PROJEXPL F0216 follow-up: C05..C0B C14-slot impact receipts
  are source-locked through raw C14, F0140, F0158 and RNG. C15 explosion-slot
  handling remains with the separate F0217/F0213 ownership chain; do not
  substitute a runtime explosion subtype here.

- 2026-07-15 DM1 GROUP F0207 follow-up: source-defined projectile selection
  no longer substitutes a fireball for BUG0_13's undefined C25/C26 Thing or
  an invented Trolin palette. Remaining work is live M10 F0212 consumption of
  the verified projectile plan and true attack-sound ownership.

- 2026-07-23 DM1 GROUP F0208/F0209/F0238 follow-up: M10 now consumes the
  source C38-C41 to C33-C36 plan with F0179's timestamp and an atomic F0238
  insertion. C29-C41 now require a live C04/raw/SFT/ACTIVE_GROUP identity;
  stale events are no-mutation. Remaining work is broader real-corpus
  coverage of unusual multi-event queues, not a fallback event generator.

- 2026-07-15 DM1 GROUP F0226: the C29-C41 current-group distance now uses
  the exact source Manhattan primitive. Remaining related work is F0227/F0228
  directional visibility over loaded map facts, not an inferred route.

- 2026-07-15 DM1 GROUP F0227: exact directional-cone visibility now has one
  owner for F0200 and M10. Remaining related work is the F0228 primary and
  secondary direction selection with its real RNG ordering.

- 2026-07-15 DM1 GROUP F0228: primary/secondary direction selection now
  consumes source RNG before F0200 in M10. F0229 now uses actual target and
  party coordinates for M10 melee-cell ordering. M10 F0230 now carries the
  original invisibility/night-vision/palette term and source F0308 ordering.
  2026-07-23 update: the M10 F0209 C38/C39 melee path now consumes the
  source F0230 -> F0304 Parry XP through a raw-C04/event/champion/action
  receipt. It admits only C38/C39, keeps source hit-or-miss XP semantics,
  synchronizes the visible Fighter base XP, and rejects stale/unknown
  records without an M11 or synthetic fallback. Remaining related work is
  broader original-corpus/runtime capture, not this live consumer.

- 🔧 2026-07-15 Nexus animated payload-anchor route: every active non-control
  Structure1G image instruction now has to resolve to the exact bounded
  Structure2 image anchor, and to its separate nonzero palette anchor when
  one exists. This is source provenance rather than image decoding: candidate
  interval lengths, palette layout, texel order, VDP1 mode, timing, and draw
  semantics remain fail-closed pending an authentic Saturn trace.

- 🔧 2026-07-15 Nexus static face payload intervals: every active static
  Structure3 face now carries the exact next-anchor bounded Structure2 image
  interval and the matching palette interval when present. These are only
  source-owned capture windows, never guessed texture/palette lengths or a
  decoder contract. Saturn pixel, palette, VDP1, transform, and draw evidence
  remain required.

- 🔧 2026-07-15 Nexus descriptor capture target: each Structure2 capture
  request now includes hash-bound image and optional palette candidate windows
  from the same canonical LEV. An admitted external trace must repeat those
  exact source windows before it can become even an opaque receipt. This does
  not assert source-read semantics, decompression, pixel order, palette
  format, VDP1 state, decoder behavior, or drawing; all remain blocked pending
  authentic Saturn observations.

- 🔧 2026-07-15 Nexus owner/material capture correlation: a capture producer
  can now receive one independently hash-bound Structure1F/1A owner/face
  request together with one static Structure3/Structure2 material window.
  The owner model-index to Structure3-entry relation is still unproved, so a
  real trace must establish that correlation as well as pixel, palette, VDP1,
  transform, and draw semantics before rendering can be enabled.

  - 2026-07-15 update: the producer now receives one atomic target containing
    the hash-bound Structure1F/1A owner, exact typed Structure3 face, and the
    selected Structure2 image/palette candidate windows. It requests source
    reads, palette state, VDP1 VRAM/command, transform, and culling lanes from
    one original Saturn session. The target still cannot claim a model-entry
    match, pixel span/order, palette format, VDP1 mode, or drawing.

  - 2026-07-15 consumption update: external traces now need the deterministic
    atomic-target fingerprint plus the owner/face identifiers before the
    existing exact Structure2 trace gate runs. A successful trace is retained
    only as an opaque original-Saturn receipt. It still proves neither a
    Structure2 decode nor pixel/palette/VDP1 semantics; a legal BIOS-backed
    capture with those observations remains the next blocker.

  - 2026-07-15 collector update: `firestaff_nexus_v1_saturn_owner_material_trace_collector`
    now packages a nonempty Mednafen debugger raw trace beside all required
    fields copied from one atomic target, including its missing opaque-payload
    fingerprint. It creates no raw trace and carries an explicit non-attestation
    marker. This machine has Mednafen 1.32.1 but no local `sega_101.bin` or
    verified Nexus media, so no authentic trace or format-derived decoder was
    produced here.

- 🔧 2026-08-08 Theron source-backed ground items: authenticated Track 02
  weapon, clothing, scroll, potion, and chest records now need pickup,
  inventory ownership, equip/use, stack, and T900 consumer binding. The
  ground-object representation preserves raw bytes and decoded fields, but
  no inferred inventory semantics are enabled. Misc, missile, and cloud
  records remain source-only until their consumers are recovered.

- 🔧 2026-08-08 Theron item property consumer: weapon, armor, scroll,
  potion, and chest ground objects now carry matched 66-entry category and
  six-byte property records where the source type agrees. Remaining source
  object types still need the original T900/item-ID translation and full
  inventory ownership/equip/use semantics; do not promote them by range.

- 🔧 2026-08-08 Theron source-backed pickup provenance: a picked-up
  Track 02 object now retains its exact decoded payload in a parallel
  inventory-source slot. T900 ownership, equip/use, stack, save persistence,
  and item-specific consumption rules still require the original consumer.

- 🔧 2026-08-08 Theron source-backed inventory roundtrip: a proven carried
  weapon, clothing, scroll, potion, or chest record can now be placed back on
  the ground while retaining its source references, decoded payload, charges,
  and matched property bytes. This is an explicit provenance roundtrip API;
  the original T900 drop command, slot translation, and save semantics remain
  unproven and are intentionally not wired to the legacy drop command.

- 🔧 2026-08-08 Theron live creature cell binding: static category-4 records
  now publish each live group member's exact two-bit cell ordinal from the
  packed source `position`/cells byte. Group coordinates, current HP, source
  refs, and member slots are retained, but the original AI, attack, RNG,
  generator, and T900 consumers remain open.

- 🔧 2026-08-08 Theron monster raw-word retention: category-4 decoding and
  live creature admission now retain both source flag words verbatim. Their
  behavior/count/direction/do-not-discard meanings remain unassigned until
  the original AI/T900 consumers are captured; no host bit interpretation is
  enabled.

- 🔧 2026-08-08 Theron generator consumer boundary: source generator records
  are retained, but original RNG consumption, spawn timing, reactivation,
  value semantics, and creature publication still require a verified runtime
  capture. The bind API now rejects non-generator actuator types and
  out-of-map coordinates rather than admitting malformed records.

- 🔧 2026-08-08 Theron roster text/class join: authenticated startup text
  supplies US names/titles while the static production roster keeps labels
  unavailable. Startup layout now joins decoded text to numeric class records
  by source mirror index; portrait pixels and portrait IDs remain unbound.

- 🔧 2026-08-08 Theron object-record admission: source object binding now
  requires a loaded, header-verified level and in-map coordinates. Full T900
  ownership/equip/use/stack semantics remain pending the original consumer.

- 🔧 2026-08-08 Theron monster-record admission: valid category-4 records now
  require a source creature type, group count 0..3, and in-map coordinates.
  RNG publication remains blocked until the dynamic `$4644/$4667` return
  contract is captured.

- 🔧 2026-08-08 Theron startup fixture linkage: production startup remains
  source-bound, while the save/resume fixture target now compiles its own
  fixture-enabled `startup_flow.c`; keep production and fixture fallback
  behavior separately verified.

- 🔧 2026-08-08 Theron spawn-capture correlation: the consumer and register
  sidecars can now be paired only when both carry the same disassembly-bound
  windows, sequence, bank coordinates, and boundary markers. Dynamic
  `$4644/$4667` return ownership and semantic RNG publication remain blocked;
  no creature or spawn state is synthesized from the capture.

- 🔧 2026-08-08 Theron inferred spawn-stat audit: the public Track 02 spawn
  API no longer publishes HP/attack/defense from a host seed. The disassembly
  branch constants remain a receipt, but the bank-switched RNG return contract
  and consumers are unresolved. Legacy data-free combat probes use an
  explicitly fixture-only helper; production remains fail-closed.

- 🔧 2026-08-08 Theron US roster codon binding: all eight champion names are
  now verified from the ordered 5-bit little-endian codon stream in real
  `TQUS02.bin` and reach the startup media receipt. Title/control fields and
  the general text consumer remain open; no title text is synthesized.

- 🔧 2026-08-08 Theron production start inventory: DMWeb-derived starting
  equipment is excluded from production because it is not yet bound to a real
  Track 02 start-object/T900 consumer. Recover the original inventory path
  before publishing live item state.

- 🔧 2026-08-08 CSB inventory: the live GAMEBLOCK-to-M11 C00..C29 slot map
  is now source-locked. Remaining HUD work is source-raster coverage and
  complete interaction behavior, not an inferred slot-order translation.

- 🔧 2026-08-08 CSB inventory input: status-box/F1 inventory selection now
  refreshes the GAMEBLOCK/CHARDESC party receipt before accepting the
  requested champion. This closes the stale-M11-mirror route, but complete
  source-raster coverage and the remaining panel interactions still need
  real-runtime evidence.
  2026-08-08 update: the same refresh now precedes C020--C027 status-hand
  clicks and C028--C065 inventory slots, including a live PC 3.4 regression
  for stale champion selection. Remaining work is complete panel behavior,
  not a stale display-mirror write path.

# Nexus source-bound LEV00 start pose (2026-08-08)

- **NEXUS-STARTUP-POSE-CAPTURE:** The previous production request `(LEV00,
  11,29,N)` was a synthetic fixture, not a retail Nexus start position.
  Real `LEV00.DGN` (`24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a`)
  has eight zero bytes at Structure1B cell `(11,29)`, so production now
  refuses startup with an unbound pose. Keep the gate closed until an
  authentic Saturn start selector/capture is joined to the retail bytes.
  The mechanics reset now preserves that same unplaced state (`map_index=-1`);
  the launcher no longer silently reinstates LEV00 after a reset. The
  16-level bound is derived from the authenticated LEV00-LEV15 corpus rather
  than a second production literal.

- **NEXUS-WORLD-POSE-BOUNDARY:** `nexus_v1_world_init()` must remain
  unplaced until the same Saturn start-selector evidence exists. Fixture
  probes may set coordinates explicitly, but native world/save state must not
  inherit `(0,11,29,N)` from the retired compatibility path.
  2026-08-08 follow-up: `nexus_mechanics_init()` now also keeps an unplaced
  `(-1,-1,-1)` caller inert (`party_alive=0`); only explicit fixture placement
  can mark the mechanics party alive. No native start pose or party status is
  inferred from a DM1-shaped default.

- **NEXUS-DOC-STALE-GRID-CLAIMS:** `docs/nexus_dungeon.md`,
  `docs/nexus_content.md` and `docs/nexus_math.md` contained historical
  32×32/fully-unparsed wording that contradicted the authenticated retail
  intake. Keep their corrected 64×64 Structure1B and bounded Structure2/3
  wording aligned with `NEXUS_STRICT_FIDELITY_INVENTORY.md`; do not let this
  documentation correction be read as Saturn transform or VDP1 parity.

- **NEXUS-DOC-SCRIPT-PLACEHOLDER:** `docs/nexus_dungeon.md` also described a
  blank 5,448-byte script/VM route. Keep the actual 16-file `SLEV00.BIN`–
  `SLEV15.BIN` corpus and its opaque dispatch status synchronized with the
  authenticated SLEV receipts.

- **NEXUS-DOC-MENU-GRAPHICS-CLAIMS:** Keep `docs/nexus_menus.md` and
  `docs/nexus_graphics.md` explicit that host state/input and bounded mesh
  records are not retail Saturn title/menu/VDP1 parity. Do not reintroduce
  “3D animated logo”, options or host polygon rendering as implemented until
  the consumer capture is source-joined.

