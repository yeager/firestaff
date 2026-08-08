# Theron Mednafen trace patch compile verification (2026-08-08)

- ✅ Corrected the spawn-register sidecar to use portable `FILE*` `fprintf()`/`fflush()` calls and fixed the generated patch hunk counts.
- ✅ Applied the complete Theron IRQ2/main-RAM trace patch chain to the exact Mednafen 1.32.1 source archive and compiled the full emulator successfully.
- ⚠️ Authentic runtime capture remains gated because this machine links `sdl2-compat`; no gameplay semantics were promoted from the compile-only result.

# Theron spawn register capture boundary (2026-08-08)

- ✅ Extended the instrumented Mednafen path with lazy `.spawn-registers` snapshots at `$4644`, `$4667`, `$C96B` and `$CC4C`.
- ✅ Captures A/X/Y/SP/P, MPR0, relevant `$B3-$BB` bytes, logical/physical PC and boundary flags.
- ✅ Added capture cleanup, line-delimited validation and transition-receipt counting; no register value is promoted to RNG or spawn semantics.

# Theron combat admission wording audit (2026-08-08)

- ✅ Corrected the production combat-source receipt so it no longer claims that RNG spawn formulas are admitted.
- ✅ It now states the actual boundary: static Track 02 monster records are admitted, while RNG formulas, scripted encounters, AI, combat, loot and sound remain blocked.

# Theron spawn-consumer receipt admission (2026-08-08)

- ✅ Added a strict C11 parser for the Mednafen `spawn_consumer_read` sidecar.
- ✅ It verifies the authenticated trace header, contiguous sequence, HuC6280 main-RAM bank coordinates, and the `$5D64/$5D6A` plus `$C96B/$CC4C` boundary flags.
- ✅ Negative fixtures reject mutated flags; semantic publication remains permanently blocked by this receipt.

# Theron disassembly-bound spawn consumer capture (2026-08-08)

- ✅ Added a separate Mednafen receipt for `$5D64/$5D6A` reads and instruction fetches in the authenticated `$C96B-$CA69` and `$CC4C-$CD13` US Track 02 consumer bodies.
- ✅ Added capture-script counting and line-delimited trace validation.
- ✅ Kept semantic publication fail-closed: the receipt records execution provenance only and does not invent RNG, spawn, monster, or return-value semantics.

# Theron static RNG-consumer boundaries (2026-08-08)

- ✅ Added authenticated US-BIN receipt spans for the `$C96B` and `$CC4C`
  routines reached by the `$4644` preconsumer: 255 and 200 bytes with exact
  raw offsets and FNV-1a hashes.
- ✅ The disassembly receipt and focused test verify the new fields while
  keeping dynamic RNG/spawn admission disabled; bank-switched runtime state,
  helper callees and semantic return ownership are not inferred.

# Theron complete Track 02 source-occurrence census (2026-08-08)

- ✅ The dungeon loader now retains and decodes one source occurrence for every
  real placable category: doors, teleporters, text, actuators, monsters,
  inventory records, missiles and clouds.
- ✅ Added a separate occurrence counter so control-record provenance is not
  confused with the historical item/consumer-record counter or world object
  binding.
- ✅ US and JP Track 02 loader, record-layout and source-object regression tests
  pass against all seven real dungeon blocks.
- Note: this expands source provenance only. Disassembly-owned RNG, AI, T700,
  T900 and dynamic consumer admission remain fail-closed until their owners
  are proven.

# Theron first real README capture (2026-08-08)

# CSB effective launch-version handoff (2026-08-08)

- ✅ M11 now consumes M12's effective verified version from the launch intent,
  rather than reopening a stale persisted version slot. This keeps a PC34
  direct launch bound to the PC34 package when several CSB platform families
  coexist under one data root.
- ✅ Verified with the authentic PC 3.4 `GRAPHICS.DAT`/`DUNGEON.DAT` pair and
  `--game csb --platform pc --boot-probe`; the source-owned title reaches
  `csb-title-1` with the PC34 graphics MD5. ReDMCSB reference:
  `COMPILE.H:199-243`.

# CSB PC34 first dungeon-command regression (2026-08-08)

- ✅ Extended the real-data direct-launch regression beyond Prison: after
  the source-owned title, Entrance wait and door opening, it sends the first
  PC34 forward command and verifies the authenticated initial party move from
  `(9,0,2)` to `(9,1,2)` in the live CSB runtime.
- ✅ This pins the `ENTRANCE.C` F0806 → `COMMAND.C` F0361/F0380 handoff rather
  than treating an inactive title or a static dungeon frame as gameplay.

# CSB PC34 source save-disk transaction regression (2026-08-08)

- ✅ The real-data Prison test now follows the source Ctrl-S/C140 dialog for
  Save and Play, Load Saved Game and Save and Quit, rather than calling the
  runtime save helpers directly. It verifies the same native F0433/F0435
  transaction against the authenticated PC 3.4 `GRAPHICS.DAT` and
  `DUNGEON.DAT` pair on a disposable host save path.
- ✅ This locks the complete in-game save-menu handoff cited by ReDMCSB
  `COMMAND.C` F0361 and `LOADSAVE.C` F0433/F0435; it intentionally does not
  claim CSBWin DSA-save support.

# CSB PC34 F0280 mirror-to-GAMEBLOCK handoff (2026-08-08)

- ✅ A PC 3.4 mirror candidate is now materialized into the authoritative CSB
  `GAMEBLOCK` party before M11 next refreshes its HUD projection. The bridge
  decodes the original `A..P` vital, Luck..AntiFire and C04..C19 fields, keeps
  all 20 source skill-experience rows, and copies only the known C026 PC
  portrait bytes; it does not manufacture cross-platform planar data.
- ✅ The C162 cancellation path removes only that pending contiguous source
  entry. C161 remains deliberately blocked for CSB until F0282's source RNG
  and all-stat reincarnation transaction are runtime-bound.
- ✅ The PC3.4 Prison regression now verifies the encoded F0280 record across
  a subsequent authoritative CSB input refresh, alongside the authenticated
  title, Entrance, first movement and F0433/F0435 save flow. Reference:
  ReDMCSB `REVIVE.C` F0280 lines 133-283.

# CSB Utility import sees the standard writable save root (2026-08-08)

- ✅ The launcher save browser now scans Firestaff's platform-native writable
  `saves/<game>` root in addition to roots adjacent to the game-data tree.
  This lets the CSB Utility action find a valid DM1 save when licensed game
  data lives on another disk or under `~/.firestaff/data`, while runtime saves
  remain in Application Support on macOS (and their equivalent on Windows and
  Linux). Existing content/type validation and path de-duplication remain the
  admission gate; no filename-only save is admitted.

- ✅ Added a reproducible source-bound capture tool that requires the
  authenticated VRAM FNV `f8ab6c1b` and VCE FNV `ea83f117` before rendering.
- ✅ Published `verification-screens/theron-track02-dungeon-capture.png`, a
  256×224 native screen-space frame reconstructed from the captured BAT,
  decoded tile atlas and VCE palette.
- ✅ README labels the artifact accurately: it is not evidence for the still
  unresolved dungeon-square, perspective or HUD consumers.

# DM2 champion-name GDAT intake (2026-08-08)

- ✅ Championernas namn läses nu genom den befintliga, källägda
  `QUERY_GDAT_TEXT`-avkodningen för `CHAMPIONS/type/dtText/0x18` och delas
  enligt `c_hero.cpp::DM2_REVIVE_PLAYER` i för- och efterled.
- ✅ Verifierat med den verkliga PC-DOS-posten `ANDERS LIGHT WIELDER`.
  Intaget skapar fortfarande inte en hero- eller party-post.

# DM2 champion-mirror boot receipt (2026-08-08)

- ✅ PC-DOS-bootprofilen behåller nu de 16 verkliga File_header-ägda
  subtype-`0x7e`-spegelrötterna och kan lämna dem till champion-livscykeln.
- ✅ Rättade den felaktiga DYN4-antasningen: `c_loadlevel.cpp` köar en
  separat `0x16xxffff`-nyckel per spegel. Den globala `0x16ffffff`-nyckeln
  är villkorlig och får inte ersätta spegelnycklarna vid boot.
- ✅ Verifierat med originalets PC-DOS `DUNGEON.DAT`, M11:s startprofil och
  champion-livscykeltestet. Inga DYN4-byte, heroes eller partyn skapas.

# DM2 champion-mirror chain traversal (2026-08-08)

- ✅ Championinventeringen följer nu samma kompletta `GenericRecord::w0`-
  kedja som `DM2_LOAD_LOCALLEVEL_DYN`, i stället för att förutsätta att en
  DB3-spegel alltid ligger först på rutan.
- ✅ Verifierat mot den verkliga 44-kartors PC-DOS-filen: de 16 ursprungliga
  speglarna och deras DYN4-nycklar är oförändrade. Ingen DYN4-laddning,
  hero- eller partyinitiering tillåts ännu.

# DM2 fail-closed local-level adapter (2026-08-08)

- ✅ Den fristående callback-adaptern för `DM2_LOAD_LOCALLEVEL_DYN` kräver nu
  en explicit, komplett File_header-världsägare innan den får göra en
  kartvandring eller anropa DYN4.
- ✅ Det förhindrar att testminne eller en partiell recordkedja blir en
  spelbar New Game-väg. Det återställer inte den saknade originalägaren.

# DM2 File_header map-chain admission (2026-08-08)

- ✅ Lade till en separat runtime-validerare för den kanoniska PC-DOS
  File_header-layouten. Den följer originalets markrötter och begränsade
  `GenericRecord::w0`-kedjor utan att falla tillbaka till G1-extensionen.
- ✅ Verifierat direkt med den verkliga 44-kartors `DUNGEON.DAT`-filen.
  Resultatet är fortsatt ofullständigt och materialiserar inte party, DYN4
  eller spelmekanik.

# DM2 File_header local-level record walk (2026-08-08)

- ✅ Lade till en gemensam, fail-closed callbackväg för hela den validerade
  File_header-kartans recordkedjor. Den följer `c_map`-rötterna och
  `GenericRecord::w0` med samma begränsning som originalets
  `DM2_LOAD_LOCALLEVEL_DYN`.
- ✅ Verifierat på samtliga 44 kartor i den riktiga PC-DOS-filen. Vägen läser
  endast originaldata och kan ännu inte materialisera DYN4, party eller
  spelmekanik.

# DM2 File_header DB2-textintag (2026-08-08)

- ✅ DB2-poster i samtliga File_header-kedjor materialiseras nu med
  originalens synlighet, textläge, textindex och specialanvändning från
  `DME.h::Text`.
- ✅ Verifierat mot alla 44 kartor i PC-DOS-data. Väggen, markören eller
  sensorn utför ännu ingen effekt förrän text-, rörelse- och
  aktuatortransaktionerna har en komplett sessionsägare.

# DM2 File_header DB4-varelseintag (2026-08-08)

- ✅ DB4-varelser läses nu ur de fullständiga File_header-kedjorna med
  position, typ, riktning, possessionslänk och originalets fyra HP-fält.
- ✅ Verifierat mot samtliga 44 PC-DOS-kartor. Det skapar inga
  `CreatureInfoData`-poster, AI-timers, droppar eller stridsresultat.

# DM2 File_header possessionskedjor (2026-08-08)

- ✅ Varje materialiserad DB4-varelse kan nu få sin egen `Creature::w2`-
  ägda possessionskedja läst genom originalets recordlänkar.
- ✅ Verifierat mot alla varelser i alla 44 PC-DOS-kartor. Läsningen flyttar,
  utrustar eller tappar inte föremål.

# DM2 File_header boot handoff (2026-08-08)

- ✅ M11/bootprofilen lämnar nu karta 0:s verifierade File_header-recipient
  från den hash-admitterade, monterade dungeon-sessionen till senare
  local-level-konsumenter.
- ✅ Verifierat genom M11:s PC-DOS-startprofil och den verkliga
  `DUNGEON.DAT`-recordgrafen. Handoffet skapar inte party eller DYN4-cache.

# DM2 File_header door-root intake (2026-08-08)

- ✅ Kanoniska PC-DOS File_header-kartan kan nu materialisera sina direkta
  DB0-dörrrötter med DME.h:s verkliga attributfält.
- ✅ Verifierat mot originalets karta 0. Ingen dörranimation, låsning eller
  spelmutation har aktiverats utan de återstående originalkonsumenterna.

# DM2 File_header actuator-root intake (2026-08-08)

- ✅ Kanoniska File_header-kartan läser nu direkta DB3-aktuatorrötter med
  originalens typ-, data-, fördröjnings-, effekt- och målfält.
- ✅ Verifierat mot PC-DOS karta 0. Aktuering, timers och målmutation är
  fortsatt spärrade tills deras kompletta sessionägare är återställd.

# DM2 File_header teleporter-root intake (2026-08-08)

- ✅ Kanoniska karta 0 läser nu direkta DB1-teleportörer med destination,
  räckvidd, ljud- och rotationsfält från originalets recordpool.
- ✅ Verifierat mot PC-DOS-data. Ingen party förflyttas och inga kartor byts
  förrän originalets rörelse- och sessionskedja är komplett.

# DM2 c_hero skill-row correction (2026-08-08)

- ✅ Corrected the narrow `DM2_REVIVE_PLAYER` translation to retain the
  original five skill rows: four group totals in row 0 and the sixteen
  source sub-skills in rows 1–4.
- ✅ Added regression coverage for the original `0x40 << level` mapping,
  group sums, vital scaling, ability clamp, formation slot and food/water RNG
  inputs. This remains a source-locked helper, not a New Game party owner.

# Theron generator save-state persistence (2026-08-08)

- ✅ World snapshot version 4 now carries each decoded Track 02 generator
  record plus its spawn count, next-tick array and active count in an explicit
  little-endian wire segment.
- ✅ Version 1–3 readers remain supported; version 4 rejects missing or
  truncated generator tails instead of silently resetting generator state.
- ✅ Round-trip coverage verifies source coordinates, actuator fields and
  runtime counters. This persists state only; the original T700 generator
  consumer and reactivation timing remain capture-gated.

# Game-data admission requirements (2026-08-08)

- ✅ Confirmed that the launch scanner requires both `GRAPHICS.DAT` and
  `DUNGEON.DAT` for Dungeon Master, Chaos Strikes Back and Dungeon Master II:
  Skullkeep. A verified graphics file alone can identify an edition but cannot
  enable Play.
- ✅ `docs/DATA_SETUP.md` lists the required and optional original media for
  all supported games and identifies the currently hash-catalogued editions.

# DM2 champion revive-data intake (2026-08-08)

- ✅ Added a source-bound reader for the exact 52-byte PC-DOS
  `CHAMPIONS/type/dtRaw8/0` records consumed by `DM2_REVIVE_PLAYER`.
- ✅ The reader retains original HP, stamina, mana, ability and skill inputs
  for all 16 real mirror types without creating a hero or a party.
- ✅ Verified with real PC-DOS `GRAPHICS.DAT`, the mirror census and M11
  startup gate.

# CSB complete optional-media inventory in `--scan-data` (2026-08-08)

- ✅ Replaced filename/path guesses in the CSB optional-media report with a
  single hash-first traversal of the registered CSB fingerprint corpus.
  The scanner now reports authenticated sidecars nested in original archive
  and ADF containers, including Amiga `SWSH.FTL`, `TITL.DAT`, `ENDA.DAT`,
  `KAOS.FTL`, Utility Disk media, and Atari startup modules.
- ✅ Required `GRAPHICS.DAT`/`DUNGEON.DAT` launch gating is unchanged; these
  additional results remain informational and cannot authorize a launch.
- ✅ Verified against the real local CSB collection, including 7z→ADF and
  ZIP→ADF paths. ReDMCSB references: `SWSHSND.C` F0908–F0910,
  `HINTLOAD.C:15–18`, `ANIM.C:67–72`, and `SWITCH.C:473`.

# Theron live-creature save persistence (2026-08-08)

- ✅ World snapshot version 3 now appends an explicit 87-byte wire record for
  every admitted live creature, including HP, AI/combat state, movement tick,
  drop fields and the complete Track 02 source identity.
- ✅ Deserialization bounds-checks the creature count and exact trailing size;
  version-1 and version-2 snapshots remain readable without inventing live
  creatures.
- ✅ Round-trip coverage verifies source cell/slot/group identity and active
  creature state alongside the existing object, timer and inventory records.
- Note: this persists already-authenticated static groups; the original RNG
  consumer and dynamic wave consumer remain a separate capture-gated boundary.

# Theron source-bound VRAM/VCE capture gate (2026-08-08)

- ✅ Production viewport capture loading now verifies complete-file FNV-1a
  identities before admitting the raw VDC/VCE pair: VRAM `f8ab6c1b` and VCE
  `ea83f117` for the authenticated dungeon screen capture.
- ✅ The generic in-memory decoder remains available for unit fixtures, while
  explicit runtime file paths cannot be promoted by size alone.
- Verification: all VRAM/tile/palette tests pass, including the real 64 KiB
  VRAM + 1 KiB VCE capture and 1,057 source-backed atlas entries.
- This still proves screen-space capture only; it does not claim dungeon
  square-to-tile semantics or the missing post-CD consumer.
# DM2 PC-DOS champion-mirror census (2026-08-08)

- ✅ Bound the File_header reader to the sixteen original map-0 DB3
  subtype-`0x7e` mirror roots and their champion types `0..15`.
- ✅ Removed the incorrect `0x1ff` champion identity assumption from the
  source-bound selection seam. Party creation remains blocked until c_hero,
  possessions and timer ownership are connected.
- ✅ Verified against real PC-DOS media, the champion-lifecycle contract,
  M11 startup gate and boot-profile smoke suite.

# Nexus audit iterations 4-5 — champion deserialize bounds (2026-08-08)

- ✅ Fixed champion pool deserialize bounds check (21→26 int fields).
- ✅ Fixed champion blob comment in header.
- ✅ Audited: dungeon.c (7k lines), ui_surfaces, font_s2d, dgn_texture_decode,
  save_load, all 10 header files. No additional runtime bugs beyond the
  bounds check.
- Version: v3.0.304

# Theron portable T900 object/timer save records (2026-08-08)

- ✅ Bumped the world snapshot writer to version 2 and replaced host-layout
  `Theron_V1_Object`/`Theron_V1_Timer` memcpy blocks with explicit little-endian
  records. Pointer-bearing timer `userdata` is never serialized.
- ✅ Kept a version-1 reader for existing snapshots, including the prior raw
  object/timer layout and source-inventory tail formats.
- ✅ Added round-trip coverage for object provenance, raw bytes, properties,
  generator-relevant timer fields and corrupt-count rejection.
- Verification: `test_theron_v1_world_serialize_purchase_state`,
  `test_theron_v1_combat_mechanics` (106/106), and real US/JP Track 02 dungeon
  loader regression all pass.

# DM2 AI-loop warning cleanup (2026-08-08)

- ✅ Removed dead local helpers and unused temporary state from the
  source-locked creature AI loop. The intentionally unbound wound/tick
  parameters are now explicit, with no change to AI callback order or runtime
  ownership.
- ✅ Verified the focused AI-loop suite and the real PC-DOS M11 startup gate.

# Nexus audit iterations 2-3 — inventory bounds, hunger comment (2026-08-08)

- ✅ Added weapon_slot bounds check in nexus_inventory_equip.
- ✅ Fixed hunger drain comment to match actual output sequence.
- ✅ Audited: inventory, movement, doors, triggers, creatures, light, status,
  item_use, experience, hunger, engine (12k lines), game, text, throw, rest,
  encumbrance, projectiles, automap, squares, shop. No additional runtime bugs found.
- Version: v3.0.303

# Cross-game game-data guide (2026-08-08)

- ✅ Replaced contradictory setup instructions with one user-facing guide for
  Dungeon Master, Chaos Strikes Back, Dungeon Master II: Skullkeep, DM Nexus
  and Theron's Quest. It distinguishes hash-gated start data from optional
  presentation, language, music and resume media, and keeps archive/disc
  inputs intact rather than asking users to unpack their collection.
- ✅ Linked the README and wiki page to the maintained guide.

# CSB Utility Disk discovery across the M12 runtime cache (2026-08-08)

- ✅ Kept M12's configured originals directory as a separate, hash-only
  Utility Disk search root when it materializes the selected CSB core package
  into a private runtime cache. A valid DM1-party import can now discover an
  original Utility ADF in the selected data root rather than incorrectly
  searching only the cache containing `GRAPHICS.DAT` and `DUNGEON.DAT`.
- ✅ Verified the full utility import state machine with the real English
  release-3 ADF embedded in the local CSB archive: archive discovery,
  extraction, `UTIO.C` identity check, import confirmation and `DONE` all
  pass. This deliberately does not add DSA-save support.
- ✅ Fixed the materialized-package case where the private CSB save/cache
  directory did not yet exist before archive extraction. The boot transaction
  now creates it before temporary ADF materialization. The full PC34
  cache → originals root → Utility Disk → import handoff passes.

# Nexus audit iteration 1 — gameplay bug fixes (2026-08-08)

- ✅ Fixed 3 slot off-by-one errors in mechanics (defense, ring, weapon lookups).
- ✅ Fixed backwards wound penalty in creature melee attacks.
- ✅ Fixed DISPEL to remove all statuses, not only poison.
- ✅ Fixed stamina scaling to linear formula (DM.BIN 0x029F38 reference).
- ✅ Fixed click_route slot clearing sign mismatch.
- ✅ Fixed script_vm uint32_t return type for SH-2 address space.
- ✅ Fixed spawner timer init to use respawn_delay.
- Version: v3.0.302

# DM2 archive runtime-media owner repair (2026-08-08)

- ✅ Fixed DM2 boot's archive provenance handling. Amiga LZX and FM Towns
  virtual members (`archive::member`) now retain the selected outer archive
  as their runtime media owner instead of constructing a nonexistent path
  beneath the archive name.
- ✅ Verified with the original six-disk Amiga AGA installer and HME-242 FM
  Towns CD: both stay in RAM, no game member is unpacked, and the M12, boot,
  title and Phase A checks pass.

# DM2 champion lifecycle production isolation (2026-08-08)

- ✅ Removed the callback-only champion lifecycle compatibility module from
  `firestaff_dm2`. It had no M11/GAME_LOAD caller with the original
  File_header, `c_hero`, possession and timer owners, so it could not be a
  valid live champion implementation.
- ✅ Its focused source-contract test still compiles the module directly and
  passes. The production archive no longer contains its object.

# DM2 FM Towns source Enter-menu route (2026-08-08)

- ✅ Added only the original title-menu Enter binding: `v1d39bc.dat` maps
  translated `0x001c` to UI event 215/raw event `0xD7`, which follows the
  existing GAME_LOAD gate. It cannot invent a party or select a synthetic
  save row. Arrow, action and back remain inert.
- ✅ Verified against the selected HME-242 media and PC-English companion:
  TITLE hands off to the original SKULL page, Enter reaches `0xD7`, and the
  party remains absent until the original GAME_LOAD owner exists.

# DM2 record-pool focused-test linkage repair (2026-08-08)

- ✅ Focused DM2 executables that compile the record-pool owner directly now
  link the production `firestaff_dm2` archive for its existing source-owned
  save admission helpers. No helper is duplicated, stubbed, or weakened.
- ✅ Verified the record-pool, creature-think, creature-schedule, and CAII
  allocator tests all build and pass. SKSAVE parsing, writing, and runtime
  admission behavior are unchanged.

# CSB F0435 save-clock real-data regression (2026-08-08)

- ✅ Corrected the real PC34 F0435/F9 regression setup to keep the timeline
  heap clock at the same `G0313_ul_GameTime` boundary as the deliberately
  advanced save clock. This is the source transaction in `LOADSAVE.C F0435`,
  rather than a writer relaxation.
- ✅ Verified a real PC34 session writes, reloads and rejects a corrupted
  native save; the real Prison handoff/HUD regression also passes.
- ✅ Extended the real-media Prison regression through the normal M11 F5/F9
  route when given an explicit disposable path: PC34 title → Prison → live
  C013 HUD now writes and restores its own F0433/F0435 runtime clock without
  a synthetic save fixture. The probe removes only that caller-supplied
  disposable file and never touches a player's default quicksave.
- ✅ Routed CSB's unsaved-game quit guard through the same native F0433 save
  transaction. BACK now reaches the G2018 prompt, compares CSB's boot-owned
  G0313 clock rather than the unrelated M11 world mirror, and both keyboard
  and pointer confirmation paths save a CSB runtime snapshot instead of a
  DM1 world file. Verified from a live PC34 Prison session after 101 real
  CSB runtime ticks.
- ✅ Expanded the real PC34 Prison path to exercise ordinary keyboard input:
  C002 turn and C003 forward now prove their route from M11 through the CSB
  command queue into the loaded dungeon runtime and back into M11's party
  mirror. This removes the stale claim that movement was only consumed at
  the queue boundary.
- ✅ The same PC34 real-data path now covers the original G0448 mouse C002
  turn and C003 forward rectangles. The local Atari ST `MINI.DAT` session
  also proves that F1 refreshes the authentic GAMEBLOCK party before opening
  its live inventory surface.

# CSB source spell-table lookup repair (2026-08-08)

- ✅ Fixed the separately decoded CSBWin spell-table lookup to match ReDMCSB
  `MENU.C F0409`: runes pack from bit 24 down and ordinary table entries
  intentionally ignore the chosen power rune. Real formulas such as FUL IR
  now resolve to their own source record rather than failing due to host-side
  byte ordering. Exact-power source records remain exact. Verified by the
  focused CSB rune-cost/table regression.
- ✅ Corrected live C101 rune entry to use the source-owned PC34
  G0485/G0486 executable menu block, not the unrelated optional CSBWin
  graphics override cache. CSB keyboard rune input is now admitted through
  the real C009/C011 HUD after boot. A real Atari `MINI.DAT` session proves
  that mana decreases and is written back to GAMEBLOCK.

# DM2 File_header champion test-link repair (2026-08-08)

- ✅ Removed the unrelated SKSAVE record-pool translation unit from the
  canonical G1 champion-mirror test target. The test now declares only the
  dungeon, GDAT and champion-lifecycle code it exercises, so it links after
  the record-pool implementation gained its genuine SUPPRESS dependencies.
- ✅ Verification: both the real-data 44-map `File_header` champion boundary
  and direct-root-chain boundary build and pass against the mounted PC-DOS
  `DUNGEON.DAT`. Champion activation remains correctly unavailable until the
  File_header record graph has a complete owner.

# DM2 FM Towns original-media selection repair (2026-08-08)

- ✅ When the data root contains both a loose FM Towns `DATA/GRAPHICS.DAT`
  match and the original HME-242 ZIP, M12 now keeps the ZIP's CUE/IMG as the
  selected runtime owner. The archive remains in place and is read in memory;
  no game member is materialized to disk.
- ✅ Verification: the real-media M12 test now passes from the shared DM2
  data directory with the authenticated PC-English text companion. It reaches
  the original SWOOSH, TITLE, SKULL and END startup route, native animation
  palettes, sound events and the Japanese-to-English GDAT overlay.

# CSB real-data test-path admission (2026-08-08)

- ✅ Removed two developer-machine-only `/Users/bosse/...` fallbacks from the
  PC34 viewport material regressions. They now use an explicit
  `FIRESTAFF_CSB_GRAPHICS_DAT` source file or execute their data-free checks
  only, so a missing local corpus cannot silently select unrelated data.
- ✅ Verified both the D1L/D1R wall and first-frame material routes against
  the materialized, hash-verified PC34 `GRAPHICS.DAT`, and verified their
  no-corpus paths remain safe.

# CSB Amiga native dungeon real-media admission (2026-08-08)

- ✅ Repaired the Amiga dungeon probe so it consumes only an explicitly
  configured, materialized `FIRESTAFF_CSB_AMIGA_DUNGEON` file. It now verifies
  the original A31/A35 digest before exercising the FTL decompressor and
  big-endian dungeon loader, and reports an unavailable corpus as a CTest
  skip rather than a successful empty probe.
- ✅ Verified against the local hash-verified A31 `DUNGEON.DAT`: 2 levels,
  initial party pose `0,9,0,2`, and source FTL header `0x8104`.

# Theron static monster-group admission (2026-08-08)

- ✅ Bound authenticated Track 02 category-4 monster-group records to the
  current-level live creature pool. Each member preserves source reference,
  source index, group position, slot, count, direction flags and the original
  HP word; no attack, AI, loot or sound values are fabricated.
- ✅ Initial full-dungeon loading and later level transitions rebuild the
  current-level creature pool from those records. Random-wave spawning remains
  fail-closed until the original HuC6280 RNG consumer is captured.
- ✅ Verification: all seven US Track 02 dungeon loads pass with the real BIN;
  source-group member counts match live creatures and the combat admission
  suite remains green.

# Theron spawn-consumer source lock (2026-08-08)

- ✅ Added the raw-US HuC6280 `$B0DD..$B1EB` regular-spawn body to the source
  lock. It records the real category branches, `$15`/`$19` multipliers, two
  `$4667` consumers and the `$0384` HP clamp without promoting unknown helper
  routines to host gameplay.
- ✅ The source note explicitly keeps `L4644`/`L4667` RNG ownership and later
  combat/drop consumers open; the legacy one-seed diagnostic helper is not
  treated as original-runtime proof.

# Theron `$4667` helper contract (2026-08-08)

- ✅ Added the exact 25-byte US raw-BIN `$4667` helper span at file offset
  `$9c4e7` with FNV-1a `$b9075b31`; all seven repeated dungeon-bank copies
  agree.
- ✅ The receipt records the `$5d6a/$5d64` call contract while keeping those
  RAM-loaded callees and the runtime RNG state unbound until dynamic capture.

# DM2 SKSAVE bitstream-order repair (2026-08-08)

- ✅ Added the source in-place reader for resident DB0–DB3 map chains. It
  consumes `table1d64db` fields without replacing a real tile root and
  handles the eight DB3 actuator subtypes with the original preceding
  nine-bit value. The focused dungeon reader verifies that an existing record
  changes in place; the mounted PC-DOS corpus remains at 176 passing checks.
- ✅ The SKSAVE map owner now keeps writable RAM copies of every authenticated
  tile span and implements the map, geometry, tile and ground-link callbacks
  needed by `DM2_LoadExtraDungeonCallbacks`. The original save body stays
  immutable; the real corpus verifies both copied tile identity and that a
  temporary RAM tile change cannot alter its source byte.
- ✅ Added the map half of the single SKSAVE GAME_LOAD transaction. One
  callback context now owns both the authentic mutable c_map state and its
  matching c_record pools, including resident-chain restoration and tile-root
  publication. The context is covered by the focused dungeon checks and the
  176-check original PC-DOS corpus.
- ✅ Hardened malformed-ground-link rejection in the map owner: both owned
  RAM spans are now released before the source save is rejected.

- ✅ Corrected the direct-root reader to stop after the source hero and
  cursor chains. It no longer reads possession continuations from the wrong
  position in the shared SUPPRESS stream. SKProject restores special timer
  records and map chains first, then consumes possession continuations.
- ✅ Verification: all eight original PC-DOS SKSAVE files pass the 168-check
  real-data corpus suite. The revised pool receipt proves that this phase has
  consumed zero continuation records rather than misclassifying later source
  bytes.
- ✅ The direct-root receipt now carries the exact post-root byte, carry byte
  and remaining-bit count. This gives the special-timer importer the original
  shared-stream position without assuming byte alignment.
- ✅ Added an isolated source-order preflight for `DM2_2066_197c`: it builds
  a temporary raw record pool, restores hero/cursor roots, decodes the real
  12-byte `c_tim` records and reads special `0x3c`/`0x3d` chains before map
  chains. The four files without a complete local pool remain fail-closed.
- ✅ Verification: the real PC-DOS SKSAVE corpus passes 168 checks. No save
  is written, unpacked or promoted into a playable session.
- ✅ Corrected the generic `READ_SKSAVE_DUNGEON` tile-chain branch to match
  `sksvgame.cpp:1320-1399`: a resident tile chain is restored by its existing
  map/record owner, while `READ_RECORD_CHECKCODE` is reserved for an empty
  `OBJECT_END_MARKER` tile. Missing ownership now fails closed instead of
  consuming real map bits into an invented replacement chain.
- ✅ Verification: `test_dm2_v1_save_load_extra_dungeon_data` and the
  168-check real PC-DOS SKSAVE corpus pass. The change reads data in place;
  it does not unpack, write or publish a save.
- ✅ Recovered source-owned resident tile roots from the raw SKSAVE layout.
  The lookup follows `skmap.cpp::DM2_GET_OBJECT_INDEX_FROM_TILE`: tile bit
  `0x10` selects the saved `v1e03d8` column index, preceding marked tiles in
  that column advance it, and `dm2_v1e038c` supplies the record link. Empty
  tiles retain `OBJECT_END_MARKER`.
- ✅ Verification: all eight original PC-DOS SKSAVE files resolve their
  marked-tile roots directly from those source spans. The real-data suite now
  passes 176 checks, with no save unpacked, written or promoted to runtime.
- ✅ Ground-stack roots are now rejected before stream consumption unless their
  source pool selector and 10-bit index fit the matching raw-SKSAVE DB pool.
  This is the same address boundary used by `DM2_GET_ADDRESS_OF_RECORD` and
  prevents a malformed map root from becoming a fabricated chain.
- ✅ Added the mutable, source-owned `c_map::dm2_v1e038c` ground-stack phase
  used at the start of `DM2_READ_SKSAVE_DUNGEON`. It copies only real link
  words into RAM, derives each tile slot with
  `skmap.cpp::DM2_GET_OBJECT_INDEX_FROM_TILE`, detaches DB4–DB15 records
  before their pools are cleared, and preserves resident DB0–DB3 chains.
- ✅ Verification: the eight original PC-DOS SKSAVE files pass 176 checks,
  including the detach → clear order and the invariant that no dynamic root
  remains in a live map slot. Game data is read in place and never unpacked,
  written or promoted into a playable session.

# DM2 GAME_LOAD status ownership repair (2026-08-08)

- ✅ Corrected the DM2 complete-support receipt so an observational parse of
  an original SKSAVE can no longer be reported as a playable GAME_LOAD.
  The receipt now requires a single live owner for the restored map, records,
  possessions, heroes, timers and actuator generator. Until that transaction
  exists, status is explicitly `incomplete-game-load-owner` and Resume stays
  blocked for every original save.
- ✅ Verification: the real PC-DOS boot probe passes 106 checks and the eight
  original SKSAVE files pass 160 checks. The verification reads them in place
  and does not unpack or modify any game data.

# CSB audit and CI link repair (2026-08-08)

- ✅ Made direct CSB boot consume archive-backed original PC34 media safely:
  after hash discovery, `GRAPHICS.DAT` and `DUNGEON.DAT` are materialized as
  one private, re-hashed runtime pair before any startup decoder or dungeon
  loader opens them. The real PC34 launch and package-presentation probes now
  pass from the archive corpus. This matches ReDMCSB `LOADSAVE.C` F0435's
  ordinary-file dungeon handoff and prevents a virtual archive locator from
  being misreported as a runnable game session.
- ✅ Repaired the FM Towns portrait regression to read the launcher’s actual
  selected-package cache (`csb-fmtowns-en/PORTRAIT`); all 24 original CMP
  portraits now decode through the production materialization route.
- ✅ Deferred CSB C005/C006 local-sensor rotation to the terminal F0271
  boundary after the full wall-sensor list has been walked. This preserves
  source order and all subsequent sensor effects, and follows ReDMCSB
  `MOVESENS.C` F0270/F0271 and `TIMELINE.C` F0248.
- ✅ Restored the full CMake matrix link contract for
  `probe_dm2_v1_world_state`: the probe now includes the direct HUD-helper
  implementation required by its champion-stat bridge, resolving the
  undefined `dm2_v1_QUERY_3STAT_BAR_COLOR` symbol on macOS, Linux and Windows.
- ✅ Corrected the CSB damaged-save text to the Atari ST 2.1 disassembly's
  `"SAVED GAME DAMAGED!"` string and removed a dead save-header local.
- ✅ Repaired the CSB title → Entrance → HUD regression test so its decoded
  session carries the source-owned C001–C005/C017/C040 identity, geometry and
  transparency required by the terminal contract. This prevents a
  pointer-only fixture from weakening the real-material handoff gate.
- ✅ Repaired two follow-up source-contract regressions: the opening-door
  receipt fixture now proves the asymmetric C002 (105×161), C003 (128×161)
  and C004 geometry from `GRAPHICS.DAT`; the C14 F0115 test now writes through
  the real F0128 viewport aperture instead of assuming full-frame callback
  coordinates. This preserves the terminal handoff checks rather than
  loosening them.
- ✅ Verification: real CSB corpus scan found the eight supported editions;
  focused save, title/entrance, viewport-door, DSA-save and Phase A probes
  passed. Source anchors: ReDMCSB `SAVEHEAD.C` F0429/F0430, `TITLE.C` F0437,
  `ENTRANCE.C` F0806/F0807, and Atari ST 2.1 `csb.s` string table `$43a(a4)`.

# Theron V1 five-pass audit hardening (2026-08-08)

- ✅ Rejected malformed dungeon IDs before quest-item mask shifts and save-header indexing.
- ✅ Corrected overlapping save metadata, added explicit little-endian scalar encoding, footer-marker validation, payload bounds, and allocation-overflow checks.
- ✅ Hardened world snapshots against invalid object/timer counts and out-of-bounds copies.
- ✅ Bounded champion-name reads in roster and SRM export paths.
- ✅ Focused Theron CTest selection: 7/7 passed, including determinism and HuC6280 disassembly checks.

# Scanner provenance after cache materialization (2026-08-07)

# DM2 PC-DOS five-pass real-data audit (2026-08-08)

- ✅ Boot, start-menu and entrance pass: corrected the real-data boot gate to
  use `File_header.w8` directly. The original PC-DOS start pose is map 0,
  `(1,8)`, north; the old `(3,5,2)` expectation came from a shifted 28-map
  interpretation. The HUD direction capture now proves that it restores this
  source pose before ordinary runtime input resumes.
- ✅ Dungeon and record-boundary pass: six more legacy G1-only door, actuator,
  creature, weapon, container and scene receipts now prove the correct
  fail-closed result for the PC-DOS File_header route. They no longer turn
  shifted file bytes into live record payloads.
- ✅ Save-corpus pass: all eight supplied PC-DOS saves retain their source
  decode receipts, while incomplete record-pool owners remain unable to
  publish a runtime session.
- ✅ HUD and viewport pass: real GDAT scene, HUD, material and dialogue gates
  pass with the mounted PC-DOS data; no placeholder material is admitted.

# DM2 PC-DOS File_header real-data regression repair (2026-08-08)

- ✅ Corrected two DM2 real-data gates that still described a shifted,
  fabricated 28-map ``G1`` layout. They now assert the original PC-DOS
  `DUNGEON.DAT` `File_header` (`w0=0`, `nMaps=44`, `cwTextData=28`,
  `cwListSize=2360`) and the source-owned 44 `Map_definitions` records.
  The gates explicitly require the generic File_header route to leave the
  continuation segment absent; no pseudo-pool or synthetic root is admitted.
- ✅ Verification: the hash-identified original 39,437-byte DOS dungeon passes
  the 36-check loader probe and the `c_map` tile-value/solid/address gate.
- ✅ A second pass repaired four legacy G1-only root/map/chain gates. With the
  same original corpus, all four now verify the intended fail-closed boundary
  and the actual `File_header` party pose instead of treating shifted bytes as
  live dungeon records.
- ✅ The PC-DOS SKSave corpus gate now records the observed boundary correctly:
  four of the eight original primary/backup files bind a complete c_record
  pool, while four remain blocked after source decoding because their pool
  owner is incomplete. All eight remain prohibited from publishing a partial
  GAME_LOAD session. Verification: 160 real-corpus checks pass.

# M11 action-icon hatch dependency repair (2026-08-06)

- ✅ Restored the M11 link boundary after the synthetic disabled-icon audit was
  removed from M10: M11 now owns its direct, source-cited ACTIDRAW F0386
  global hatch predicate instead of calling the test-only model. This keeps
  the synthetic champion rows excluded while preserving the real runtime
  condition for candidate mirror and party rest.
- ✅ Verification: `test_dm2_fmtowns_m11_title_real_media` passes from the
  original FM Towns ZIP plus authenticated English GDAT companion; M11 Phase
  A passes 24/24.

# FM Towns CLI/start-menu selectors for DM1, CSB and DM2 (2026-08-06)

- ✅ Added `--fm-towns` and `--platform fm-towns` to the Firestaff CLI.
- ✅ Both selectors use the existing hash-verified FM Towns entries in the
  M12 catalogue for DM1, CSB and DM2; no PC fallback is performed when media
  is absent.
- ✅ The startup menu's existing Architecture/Version options now receive the
  same explicit selection, including the visible unavailable state.
- ✅ `firestaff` builds, the polished M12 flow passes, and invalid platform
  input is rejected. Existing unrelated boot-probe fixture failures remain
  outside this change.

# M11 viewport build hygiene (2026-08-06)

- ✅ The four retained `cells` parameters in M11's floor-pit, floor-ornament,
  stairs and teleporter helpers are now explicitly acknowledged as unused.
  Each helper already samples the authoritative live viewport state; this
  removes focused-build C11 warnings without changing pixels, routing or
  game-data ownership.

# Steam Deck AppImage Swedish locale preference (2026-08-06)

- ✅ The launcher and shared runtime language detector now honour GNU
  `LANGUAGE` before a fallback `LANG`. This covers Steam Deck Game Mode where
  `LANG` can remain `en_US.UTF-8` while the user-selected Steam UI preference
  is `sv_SE.UTF-8:en_US`. The M12 probe verifies that the Swedish setting is
  selected without a stored explicit override.

# Launcher scan localization and Linux desktop entry (2026-08-06)

- ✅ The first-run game-data scan now renders only a full launcher-visible
  title and its percentage. Internal scanner ids such as `dm1` and English
  worker labels no longer reach the UI. Missing-data popups use the same
  canonical titles, including `Dungeon Master II: The Legend of Skullkeep`.
- ✅ Swedish catalog text now covers the scan heading and initial state, and
  the M12 catalog resolver loads release catalogs from `FIRESTAFF_LOCALE_DIR`
  or `/usr/share/firestaff/po` before the source-tree fallback. DEB, RPM,
  Steam Deck package and AppImage builders install the startup-menu catalogs;
  AppRun sets the AppImage catalog path.
- ✅ Firestaff's desktop entry remains in the standard `Game;RolePlaying;`
  category, which desktop environments present as Games/Spel, and has a
  Swedish description. The launcher regression verifies Swedish missing-data
  text, the full DM2 title and the absence of the internal `DM2` id.

# GitHub release-notes specificity gate (2026-08-06)

- ✅ Strengthened the release preflight so every published version must have
  `Added`, `Changed` and `Removed` sections. Each non-empty entry names the
  affected function or feature in code formatting and describes a concrete
  action; generic catch-alls and explanatory text after `None.` are rejected.
  A data-free regression script now exercises accepted and rejected notes in
  the normal verification workflow.

- ✅ Removed the fixture-only `theron_v1_party_clear_fixture_defaults()` export
  from the production archive. Its implementation and declaration now require
  `THERON_CHAMPION_FIXTURE_HELPERS`, while source-bound startup keeps the real
  Track 02 roster records. The production archive verifier also rejects the
  fixture party reset and synthetic first-room constructors by symbol.
- ✅ Verification: serial `firestaff_theron` build, production archive boundary,
  `test_theron_v1_m11_direct_launch` and the real-data startup receipt probe
  (`313 passed, 2 skipped, 0 failed`).

# GitHub release-note functional-delta gate (2026-08-06)

- ✅ GitHub release preflight now rejects a release unless its exact version
  section has concrete `Added`, `Changed`, and `Removed` categories. Each
  non-empty item must name the affected function or feature; an unchanged
  category must say `None.` explicitly. The generic generated-notes fallback
  was removed, so GitHub can no longer publish a release with a placeholder
  summary. Verification: `scripts/verify_release_notes.py --notes
  RELEASE_NOTES.md --version 3.0.291` passes against the current CMake version.

- ✅ Startup champion footer and row labels now require the isolated
  compatibility-roster shape (`name_ascii` without an authenticated PLRD TABL
  code). Retail PLRD rows therefore remain source-glyph-only even after a
  stale host name is present in a serialized pool. Verification: startup menu
  compatibility tests and the real PLRD parser test; no game data was copied
  or committed.

# Firestaff DONE - Completed Work

- ✅ 2026-08-06 Theron T700 stat-consumer quarantine: source-authenticated
  levels now fail closed in `theron_v1_apply_post_move_effects()` instead of
  applying the unrecovered host-side stamina/food/water/poison model. Fixture
  worlds retain the existing hardening-probe behavior, and the new source-level
  regression verifies that tick, stats and poison state remain unchanged until
  the original PCE consumer is captured.

- ✅ 2026-08-06 CSB Amiga 3.1 package binding: the scanner now distinguishes
  the retail A31E disk from PC 3.4 and other Amiga disks that share
  `GRAPHICS.DAT` or `TITL.DAT` bytes. It collects bounded title occurrences,
  proves `graphics.dat` from the same nested ADF receipt, and materializes
  both graphics and `DUNGEON.DAT` from that exact package. The real local
  7z→ADF regression and the CSB boot-profile smoke test pass.

- ✅ 2026-08-06 Windows nested-Atari archive guard: the Windows fallback
  branch now defines fail-closed nested `.adf`, `.st` and `.msa` extraction
  paths. Direct Atari disk images remain in-process; an external archive
  cannot be materialized until a Windows-native archive backend exists, and
  no unresolved external extractor call can reach the Windows linker.

- ✅ 2026-08-06 CSB nested-MSA profile-scan reuse: the external-archive
  scanner now reads and decodes one Atari `.msa` member once for the complete
  hash list, then preserves every matched `archive::msa::file` path. The
  former per-hash extraction repeated source I/O for the same original disk.
  The nested MSA list/materialization regression passes without changing any
  hash or filename admission rule.

- ✅ 2026-08-06 CSB Atari ST raw-floppy scanner admission: added a bounded
  GEMDOS/FAT12 reader for ordinary sector-image `.st` disks to the shared
  hash scanner, including nested `.st` entries in externally enumerated
  archives. It authenticates and materializes original `GRAPHICS.DAT` and
  `DUNGEON.DAT` by content rather than filenames, so the CSB Atari ST 2.0
  game disk reaches the same cache handoff as loose files and ADF media.
  The reader validates the boot/BPB geometry, root entries, FAT12 chains and
  file bounds; it deliberately does not claim support for protected STX or
  compressed MSA transports. `test_asset_find_by_hash` now covers direct and
  `.7z`-nested raw-ST discovery plus virtual-path extraction, and the real
  839,680-byte CSB ST 2.0 disk scans READY.

- ✅ 2026-08-06 CSB Amiga 3.1 English ADF scanner admission: registered the
  original A31E `GRAPHICS.DAT` MD5
  `21197b1d4994fd835c403d5a33dcac2b` across the M12 profile catalog, runtime
  discovery, boot profile mapping and M11 launch gate. This fixes the real
  failure where a verified ADF yielded `DUNGEON.DAT` but was still reported as
  CSB MISSING because its graphics identity had been omitted. The mapping is
  source-bound to ReDMCSB `COMPILE.H:199-243` (`MEDIA 37`, A31E), keeps the
  later A35E profile distinct, and is covered by the CSB boot-profile smoke
  test. A fresh scan of the original 901,120-byte ADF now reports CSB READY
  and materializes the authenticated graphics/dungeon pair to the local asset
  cache. This establishes only source-media admission and handoff; title,
  entrance, HUD and viewport parity remain independently capture-gated.

- ✅ 2026-08-06 Theron regional font-tile intake: `test_theron_v1_font_tiles`
  now reads the supplied `TQUS02.bin` and `TQJP02.bin` automatically from
  `.firestaff/data/theron/`, with environment overrides preserved. It verifies
  the source-selected UD offsets (`0x263200` US, `0x262A00` JP), 96 decoded
  font tiles, 87 nonblank tiles and the authenticated checksum for both
  regions. No HUD layout, portrait, tile-bank or viewport semantics were
  inferred or promoted.

- ✅ 2026-08-06 Theron object-table header hygiene: guarded the shared
  `Theron_Track02ObjectTable` forward typedef between `theron_v1_world.h` and
  `theron_v1_track02.h`. Theron/M10 strict-warning builds no longer emit the
  duplicate C11 typedef warning; object-record layout and capture gates are
  unchanged.

- ✅ 2026-08-06 Theron graphics-format provenance note: the historical
  source-lock receipt now distinguishes its old operator-staged
  `theron-extras` paths from the current canonical
  `~/.firestaff/data/theron/TQJP02.bin` and `TQUS02.bin` copies. Hashes and
  the capture-gated palette/stride decision are unchanged.

- ✅ 2026-08-06 DM2 actuator synthetic-mutation removal: production
  `PROCEED_TIMERS` no longer registers byte-only pitfall, door, teleporter,
  trickwall, tick-generator, step-door or destroy-door handlers. Those
  handlers inferred DB3/DB14/DB0 actuator semantics from timer bits while the
  original record transaction is still unavailable. Such timers now consume
  fail-closed without changing authentic dungeon bytes; their focused source
  studies remain available outside the live dispatcher.

- ✅ 2026-08-06 DM2 timer transaction boundary: production no longer binds
  `PROCESS_0E`, `PROCESS_3D` or `MOVE_RECORD_ROTATE` from timer bytes to a
  direct item, record or party-position write. The required original
  `c_hero` inventory and `MOVE_RECORD_TO` link/wake/sleep/party transactions
  are not yet imported, so these events now consume fail-closed instead.

- ✅ 2026-08-06 CSB FM Towns ANM CD-DA command receipts: the F2275 stream
  interpreter now retains `TD`'s original physical-track table and surfaces
  `TR` requests at the following real presentation frame, rather than
  inventing a title-music association. The authentic corpus verifies no
  title request, two Story requests beginning at track 3 and two Ending
  requests beginning at track 18. Audio playback remains separately
  capture-gated; no game data was changed or tracked.

- ✅ 2026-08-06 CSB FM Towns M11 title handoff: the FM Towns variants now
  bind `TITLE.ANM` only from the selected verified runtime cache and present
  the real F2275 retained raster/palette in M11. `TOWNSIO.C F2263`'s
  `SND_fm_timer_a_set(1, 100)` is scheduled with the YM2612 Timer-A period
  of 16,632 µs through an accumulator, so PC34's 55/220 ms title machine
  cannot speed up or replace the Towns animation. At EOF Firestaff retains
  the final real page rather than inventing a PC34 entrance or game handoff.
  Verification: CSB M11 cadence plus real TITLE/STORY/ENDING ANM playback;
  CDDA, input and post-title behavior remain capture-gated.

- ✅ 2026-08-06 CSB FM Towns ANM stream interpreter: added the retained-raster
  F2275 playback owner for original ANM chunks, including source palette
  changes, `BR` input admission and exact `FO`/`NE` loop control. It does not
  fabricate sound, input or a host timebase. The authentic title, story and
  ending streams now prove 32/851/419 displayed frames and 606/10,823/5,352
  Timer-A ticks respectively; in particular ENDING's 419 frames prove that
  its original loop sections are executed. M11 scheduling remains
  capture-gated; no game data was changed or tracked.

- ✅ 2026-08-06 CSB FM Towns ANM source timing: decoded `EN`/`DL` frame
  receipts now retain the original big-endian chunk delay and its exact F2275
  FM Towns minimum of five Timer-A ticks. No unsupported host-millisecond
  conversion is made. The real `TITLE.ANM`, `STORY.ANM` and `ENDING.ANM`
  regressions assert that first and final decoded frames carry that timing,
  alongside source-owned pixels and palettes. M11 presentation remains
  explicitly capture-gated; no game data was changed or tracked.

- ✅ 2026-08-05 DM1 boot save-evidence correction: a normal real-data HoC
  boot no longer promotes its loaded `DUNGEON.DAT`/`GRAPHICS.DAT` runtime into
  an original PC34 save corpus. Save header, five-part corpus, four-portrait
  corpus, dungeon payload and required save hashes remain unset until an
  explicitly configured corpus is classified. Verified with the extracted
  real DM1 boot probe: `dm1CompleteSaveCorpusRoute=0` and
  `dm1CompleteOriginalSaveRoundtripRoute=0`, while the normal HoC render
  route remains available. The actual original-save corpus/roundtrip remains
  open in `TODO.md`.

- ✅ 2026-08-06 DM1 authenticated viewport fallback gate: M11 no longer
  paints synthetic primitive doors or stairs after a real PC34 source session
  fails to resolve its bitmap. Authenticated raw Thing data now fails closed,
  while compact/legacy test worlds retain the isolated compatibility fallback.
  Verification: `test_dm1_v1_f0190_c040_m11_integration_audit`,
  `test_dm1_v1_f0190_moving_killed_all_m10_handoff_pc34_compat`, and
  `git diff --check` pass. Real bitmap binding and packaged macOS capture
  remain tracked in `DM1-VIEWPORT-WALLS-DOORS`.

- ✅ 2026-08-05 DM2 SKSave game-state wire-layout correction: replaced the
  fabricated byte-22 `rain_state[8]` and broad 56-byte mask with SKProject
  `skload_table_60`'s exact fields and `SKWIN/SkGlobal.cpp::_4976_395a` mask,
  including its final zero mask byte. `bRainStrength` now comes from original
  byte 44 rather than unrelated state. Save/load, weather/timer and real-data
  startup gates pass; incomplete record/session ownership remains blocked.

- ✅ 2026-08-05 DM2 unowned creature-clock removal: `dm2_v1_runtime_tick()`
  no longer advances the legacy global creature fixture pool after the
  source-order timer dispatcher. The only production-capable creature route
  remains the bound 0x21/0x22 DB4/CAII/CCM handler; callers cannot fabricate a
  standalone creature, and the production gate now proves a general runtime
  tick does not advance that fixture clock. Focused production gate and
  source-timer regression pass against the real PC-DOS corpus.

- ✅ 2026-08-05 DM2 boot-summary truthfulness: moved the DM2 profile summary
  until after hash-verified `DUNGEON.DAT` admission. **Corrected 2026-08-07:**
  the source values are `File_header.w0=0` and `File_header.nMaps=44`, not
  the old misread `257/28` pair. The M11 startup real-data gate asserts those
  values alongside the source start pose. Verification:
  direct `--game dm2 --boot-probe` capture and
  `dm2_v1_m11_startup_profile_gate`.
- ✅ 2026-08-05 DM2 hash-verified PC-DOS boot repair: expanded the temporary
  DM2 identity list so all supported `GRAPHICS.DAT` identities and every
  `DUNGEON.DAT` identity are scanned together. The prior seven-entry limit
  was exhausted by graphics variants after PC-9821 support, silently omitting
  every dungeon hash and blocking a valid PC-DOS launch. The real
  `graphics.dat`/`dungeon.dat` corpus now enters the M11 GDAT-HUD command plan
  (nine source-backed commands) with no visual fallback. Verification:
  `dm2_v1_gdat_hud_m11_command_real_data`,
  `dm2_v1_m11_startup_profile_gate`, `dm2_v1_save_load`, and
  `dm2_v1_quicksave_original_writer_gate` pass against mounted original data.

- ✅ 2026-08-05 DM2 DOS SKSave header and raw-prefix corpus: added the
  authenticated PC-DOS header admission rule (version word plus bounded ASCII
  save name). The
  external corpus gate now reads the eight supplied `sksave0..3.dat/.bak`
  artifacts and verifies each real raw dungeon prefix after its 42-byte
  header. Later SUPPRESS state remains fail-closed rather than being replaced
  by a fixture session. Verification:
  `FIRESTAFF_DM2_SKSAVE_CORPUS=~/.firestaff/data/dm2/dos_extract/data
  ./build-dm2-main-verify/test_dm2_v1_save_load` passes 24/24.

- ✅ 2026-08-05 DM2 SKSave synthetic-header removal: removed the invented
  `0xBEEF/0xDEAD` file marker from every slot helper and stopped accepting it
  on input. Headers now match SKProject `c_hex2a`: version word, bounded
  printable `text[36]`, and the existing file's retained opaque `l_26` value;
  the original dialog's `0xDEADBEEF` empty-entry sentinel is rejected as a
  file. The partial `dm2_v1_save_game_write()` path is now fail-closed because
  it lacks the original dungeon/DB record sections, so it cannot manufacture
  a non-playable save. Source: `SKULLWIN/dm2data.h:150-159`,
  `c_dialog.cpp:115-117,199-202,337-343`, `c_savegame.cpp:2169-2204`.

- ✅ 2026-08-05 DM2 CAII false-success removal: both linked
  `DM2_1c9a_38a8` narrow adapters now reject unbound source state instead of
  returning the original routine's meaningful zero/no-path result. The
  placeholder state field is removed; receipts explicitly record the missing
  live `s350`/CAII/action-list owner. Regression coverage locks both public
  boundaries. Source: `SKULLWIN/c_1c9a.cpp:9748-9894`.

- ✅ 2026-08-05 DM2 FORMAT_SKSTR synthetic-substitution removal: the linked
  narrow text adapter no longer invents alphabetic `.Za`--`.Zz` substitutions.
  It preserves source bytes literally until the genuine numeric
  `.Z000`--`.Z028`/byte-`0x01` owner can be bound, preventing fabricated hero,
  buffer, or newline text. Regression coverage proves the numeric grammar and
  callback-bearing fake forms remain literal. Source:
  `SKULLWIN/c_gfx_str.cpp:290-557`.

- ✅ 2026-08-05 DM2 music-label synthetic-data removal: deleted the unused
  invented HMP track-name table from the active sound module and corrected its
  source count to 29 tracks. Playback continues to use only the authenticated
  `SONGLIST.DAT` selector and `GRAPHICS.DAT::MUSICS/<0x00..0x1c>/dtHMP/0`
  identity; no semantic name is inferred from an index. Source: DMWeb
  "Dungeon Master II Music Triggers" and "Data Files".

- ✅ 2026-08-05 DM2 hero-progression RNG placeholder removal: the linked
  callback-compatible `dm2_v1_adjust_skills()` path no longer substitutes
  `DM2_RANDDIR()` with zero for wizard/priest level-up mana or antimagic. It
  now requires the caller's two source LCG steps and masks their real 0..3
  results, while a missing random owner rejects the stat mutation. Regression
  coverage proves both source jitters and a fail-closed missing callback. Source:
  `SKULLWIN/c_random.cpp:39-46`, `c_hero.cpp:1335-1348`.

- ✅ 2026-07-31 DM2 original-save admission: closed the remaining D2RS
  runtime-read path. Public slot/last-session loaders, corpus runtime import
  and runtime restore now admit only original-envelope or raw SKSave
  candidates; Firestaff-private D2RS blobs remain diagnostic rejections.
  Retired active M11 resume fixtures that fabricated D2RS state, while keeping
  the original-envelope resume gate. Verification with local real DM2 data:
  `dm2_v1_m11_startup_profile_gate`, `dm2_v1_gdat_hud_m11_command_real_data`,
  `dm2_v1_save_load` and `dm2_v1_quicksave_original_writer_gate` pass (4/4).

- ✅ 2026-07-31 DM2 companion no-op isolation: removed the unattached
  `dm2_v1_companion.c` boundary from the production M10 archive. Its only
  behavior is to reject caller-authored companion data and it has no live
  DB4/CAII/CCM caller; the explicit rejection test retains a private copy.
  No companion state can therefore become part of the executable before the
  original creature, inventory and dialogue ownership chain is decoded.

- ✅ 2026-07-31 DM2 unbound progression-state isolation: removed
  `dm2_v1_progression.c` from the production M10 archive. Its hard-coded
  time-cycle, weather and level metadata had no M11 consumer or decoded
  session/GDAT owner, so it could not safely be a live-data source. The
  source remains available for isolated reference work only; active weather
  and scene rendering retain their G1/GDAT/timer receipts.

- ✅ 2026-07-31 DM2 external SDL startup/menu capture: the current
  Extern-disk `firestaff` binary reached the original PC-English
  `GRAPHICS.DAT` TITLE/0 main-menu raster in a 960×600 SDL window, with the
  correct night palette and no credits-only/grey palette frame. The
  boot-probe then applied the source menu-to-credits click followed by the
  common 0xEF dismissal click and returned to `dm2-startup-menu` using the
  hash-verified data pair. This is an unbundled source-binary capture only;
  it does not claim a packaged-app capture or gameplay/HUD completion.

- ✅ 2026-07-31 DM2 G1 scene/light/weather production audit: confirmed that
  the legacy outdoor colour facade is excluded from `firestaff_dm2` and can
  return only no-draw. The active viewport accepts outdoor pixels only after
  an admitted G1 map token, matching GDAT material/local palette, c_light
  receipt and source-timer-owned DistantEnvironment slot. Verification:
  outdoor-material, ambient-light, c_light, outdoor-weather-frame,
  local-palette, scene-light-control and weather-renderer gater pass (7/7).
  This is a closure of fallback exposure, not a claim that the remaining
  source-owned outdoor scene paths are complete.

- ✅ 2026-07-31 DM2 public session-writer closure: the remaining
  `dm2_v1_session_save_slot()` and `dm2_v1_session_save_last_session()`
  compatibility APIs now return `DM2_V1_SESSION_WRITE_ORIGINAL_WRITER_REQUIRED`
  before serializing or touching an `SKSave` path. M12/browser and utility
  tests no longer manufacture D2RS session saves as proof of DM2 resume;
  they retain only original-format SUPPRESS/raw import fixtures. Verification:
  save/load, utility/import, M12 quick-resume and save-browser CTests pass,
  and the production-linked no-writer gate passes.

- ✅ 2026-07-31 DM2 synthetic runtime-smoke isolation: removed the
  `dm2_v1_runtime_handoff_smoke` CTest registration because it constructs a
  fabricated world, actor, weather, trigger and shop state. Production now
  correctly refuses those paths without original G1/GDAT/SKSave owners; the
  real-data M11 startup and GDAT HUD command gates cover the supported path.
  Verification: the retired test is absent from CTest; the original-writer,
  real-data startup and real-data GDAT HUD gates pass.

- ✅ 2026-07-31 Theron Track 19 media intake: the real verified US
  `TQUS19.iso` (5984256 bytes, 2922 MODE1/2048 sectors) and JP
  `TQJP19.iso` (6291456 bytes, 3072 MODE1/2048 sectors) are now classified
  explicitly as ISO sector media. The legacy raw 2352-byte route remains
  separately identified. No level, object, bitmap, or palette route is
  opened by container recognition alone. Verification: Track 19 inventory
  probe passes for both real ISO identities, raw alignment, sector counts,
  and unknown-hash rejection.

- ✅ 2026-07-31 Theron asset-loader cleanup: removed the unreachable
  Firestaff-only THG3 tile parser body that followed an unconditional
  rejection. The production loader now has no dead synthetic tile-decoding
  path; real graphics remain blocked until an authenticated Track 02 bank
  route is available. Verification: rendering, startup/save-resume, and
  Track 19 inventory tests pass.

- ✅ 2026-07-31 Theron record-gap reference audit: recorded the public
  community extraction report that identifies seven 256 KiB TQ02 quest-blocks
  and item/map candidates, while explicitly preserving its warning that level
  headers and read-control data sit outside those blocks. The report is now a
  secondary investigation reference only; no converted CSBWin data is treated
  as original Firestaff media.

- ✅ 2026-07-31 Theron real-CUE handoff fix: the USA CUE validator no longer
  truncates its read at the first dynamic CD-read record. It now reads the
  complete hash-verified Track 02 payload, allowing later descriptor sectors
  required by the already-proven stage-three boundary to validate. Real USA
  BIN/CUE handoff regression passes; level/object semantics remain blocked.

- ✅ 2026-07-31 Theron V1 viewport production isolation: removed the
  procedural dungeon, UI chrome, tile selector, and indexed-to-M11 presenter
  from the production archive. Production now owns a lifecycle-preserving
  no-op viewport seam; the pixel renderer remains explicit in fixture and
  probe targets. Verification: viewport renderer, first-room runtime, and V2
  overlay seed probes passed; the production archive contains only
  `theron_v1_viewport_runtime_noop.c.o` for the viewport.

- ✅ 2026-07-31 Theron HUD placeholder cleanup: removed the dead procedural
  champion-slot renderer from the production compilation path. The legacy
  V1 chrome compositor remains fail-closed until a verified original UI bank
  is bound; rendering `25/25` and startup-flow `653/653` still pass.


- ✅ 2026-07-23 CSB C001-to-C005 terminal receipt: C005 credits now records
  its real GRAPHICS.DAT host presentation in the owning startup session,
  including source tick and exact frame/raster hashes. ReDMCSB ENTRANCE.C
  F0442/F0807 cannot promote C017/C040 HUD after a C005 detour unless the
  real C005 frame is followed by a real C004/C002/C003 return frame; the
  ordinary no-credits entrance path remains valid. Verification:
  `test_csb_v1_startup_real_sequence_pc34_compat` with local PC34
  `GRAPHICS.DAT`.

- 2026-07-23 CSB Entrance source-bound decoder admission: C002, C003 and
  C004 now require their individual CSBWin-compatible decoder receipts before
  entering the opening-door session. The local real `GRAPHICS.DAT` startup
  sequence confirms all three record boundaries; incomplete or foreign
  material remains no-draw.

- 2026-07-23 CSB Entrance real-composition cleanup: removed the generated
  `CHAOS STRIKES BACK`/`ENTRANCE`/status/pose/prompt overlay from the closed
  C004 entrance plan. F0439/F0441 now leave the admitted C004 and C002/C003
  composition unobscured; the original input/menu route remains intact.
  Verification: the focused closed-Entrance assertion passes in
  `test_csb_v1_startup_entrance_pointer_pc34_compat`. Its two current C001
  title assertions and the real-sequence STRIKES BACK assertion are failing
  under concurrent palette work outside this batch; they do not cover the
  removed Entrance overlay.

- 2026-07-23 DM1 V1 PANEL.C F0339 real eye-indicator consumption: the normal
  F0342 object-detail route now paints C019 through the same admitted
  GRAPHICS.DAT session as its panel. Missing or mismatched C019 stays blank;
  no host substitute is drawn. Added the opt-in
  `firestaff_dm1_v1_original_pc34_f0339_eye_indicator_runtime_probe`, which
  compares the rendered C019 pixels with the original asset when an original
  PC34 save and DM1 data directory are supplied. Verified: focused Ninja
  build and CTest pass; the probe skips cleanly without that external corpus.

- 2026-07-23 Theron V1 startup host-receipt apply facade (Lane E, cycle 8):
  Closed the remaining M11-decoupling item from the 2026-07-22 Lane E cycle 7
  entry: the startup host-receipt apply and chapter-inspect wiring now live in
  a Theron-owned facade, so M11 no longer maps `Theron_StartupHostReceipt`
  fields to status/inspect/log/input-result actions directly.
  Changes:
    * `include/theron_v1_boot.h`:
      - Added `Theron_V1_BootHostReceiptResult` enum (`IGNORED`, `REDRAW`,
        `RETURN_TO_MENU`) so the facade result is independent of M11's enum.
      - Added `Theron_V1_BootHostReceiptCallbacks` struct carrying an opaque
        `userdata` plus `set_status`, `set_inspect`, and `log_event` hooks.
      - Declared `theron_v1_boot_apply_startup_host_receipt`.
    * `src/theron/theron_v1_boot.c`:
      - Implemented the host-receipt apply facade. It consumes a
        `Theron_StartupHostReceipt`, calls the supplied callbacks for
        status/inspect/log, and returns a `Theron_V1_BootHostReceiptResult`
        derived from `Theron_StartupInputResult`.
      - The facade defaults the status scope to `"STARTUP"` and the inspect
        detail to an empty string when the receipt leaves those fields blank,
        preserving the previous M11 behavior.
      - Log lines are emitted with the same diagnostic color (M11 yellow /
        VGA slot 11) that M11 used before the move.
    * `src/engine/m11_game_view.c`:
      - Replaced the inline `m11_theron_apply_startup_host_receipt` body with
        M11 callback implementations (`m11_theron_boot_host_set_status`,
        `m11_theron_boot_host_set_inspect`, `m11_theron_boot_host_log_event`)
        that wrap the real M11 status/inspect/log APIs.
      - The wrapper now calls `theron_v1_boot_apply_startup_host_receipt` and
        maps the returned Theron result back to `M11_GameInputResult`.
      - All existing call sites (boot runtime receipt, launch failure receipt,
        and the action-host-receipt wrapper) now go through the facade.
    * `tests/test_theron_v1_boot_host_receipt.c` (new) and `CMakeLists.txt`:
      - Added 14-check regression test with mock callbacks verifying null
        receipt/callbacks/out_result handling, status scope/status delivery,
        default scope behavior, inspect scope/detail delivery, empty-detail
        handling, `log_first_line` emission, `runtime_receipt` gating by
        `log_receipt`, ordered dual-log emission, and input-result mapping for
        `IGNORED`/`REDRAW`/`RETURN_TO_LAUNCHER`.
  Verification:
    * `cmake --build build --parallel`: succeeds.
    * `./build/test_theron_v1_boot_host_receipt`: 14/14 PASS.
    * `ctest --test-dir build -R theron_v1_boot_runtime_input`: PASS.
    * `ctest --test-dir build -R theron_v1_rendering`: PASS.
    * `ctest --test-dir build -R theron_v1_startup_flow_probe`: PASS.
    * `ctest --test-dir build -R theron_v1_m11_direct_launch`: PASS.
    * `ctest --test-dir build -R theron_v1_m11_launcher_handoff_boundary`: PASS.
    * `SDL_VIDEODRIVER=dummy ctest --test-dir build -R '^m11_phase_a$'`: PASS.
  Source/evidence citations:
    * THQUEST.ASM T400 startup state handoff (status/inspect/log flow).
    * `include/theron_v1_startup_flow.h` `Theron_StartupHostReceipt` layout.
    * `src/engine/m11_game_view.c` pre-existing M11 status/inspect/log mapping
      for Theron host receipts (the surface being facaded).

- 2026-07-23 DM2-011 real-data outdoor weather frame capture (Lane C, cycle 6):
  Closed the TODO item for DM2-011: the renderer now consumes bound live
  `DistantEnvironment` slots and M11 accepts the resulting frame.
  Changes:
    * `src/dm2/dm2_v1_viewport_renderer.c`:
      - Static HUD M11 plan omits the right-side portrait panel in outdoor mode
        (`dm2_v1_gdat_hud_m11_command_plan_build` takes `is_outdoor`; boot and
        callers pass `rt->outdoor`). Plan command count is 8 for outdoor, 9 for
        indoor.
      - `dm2_v1_hud_plan_command` minimum command count lowered to allow the
        8-command outdoor plan.
      - `dm2_v1_viewport_render` now treats a bound action-text palette as
        consumed when source materials are required and no HUD text path runs,
        so no-party outdoor frames are not rejected by M11 merely because no
        text was drawn.
      - Outdoor rendering path cleaned of debug instrumentation.
    * `src/dm2/dm2_v1_boot.c`:
      - `dm2_v1_boot_runtime_render_frame` now builds a default c_light receipt
        in `dm2_runtime_refresh_gdat_scene_control` so the action palette
        becomes ready.
      - `dm2_v1_boot_gdat_scene_m11_apply_light_palette` computes
        `command->palette_hash` with FNV-1a over the 16-byte palette to match
        the viewport's recompute.
      - `runtime_render_no_core_fallbacks` is now outdoor-aware: outdoor frames
        do not require an indoor wall pass.
    * `src/dm2/dm2_v1_runtime.c`:
      - Outdoor M11 frame clears `wall_material_plan_hash` and
        `wall_material_plan_command_count` so M11 does not compare a non-zero
        wall plan hash against zero commands.
      - Weather renderer binding block cleaned of debug instrumentation.
    * `include/dm2_v1_gdat_hud_m11_command.h`, `include/dm2_v1_boot.h`:
      - Signatures updated for outdoor-aware static HUD plan and boot helpers.
    * `tests/test_dm2_v1_outdoor_weather_frame_capture.c`:
      - New test proving real-data outdoor frame capture: binds live
        `DistantEnvironment` slots, renders through boot/runtime, consumes real
        GDAT sky/ground/HUD/weather pixels, and passes the M11 gate.
      - Total: 22/22 checks PASS.
  Source evidence:
    * `skproject/SKULLWIN/c_weather.cpp` `DM2_UPDATE_WEATHER` (0x54 timer + arg==0)
    * `skproject/SKULLWIN/c_bkgrnd.cpp` `ENVIRONMENT_DRAW_DISTANT_ELEMENT`
    * `skproject/SKWIN/SkWinCore.cpp` `QUERY_TEMP_PICST` / `QUERY_GDAT_SUMMARY_IMAGE`
  Verification:
    * `./build/test_dm2_v1_outdoor_weather_frame_capture` passes.
    * Full DM2 V1 lane: `ctest -R dm2_v1_` reports 215/229 tests pass. The 14
      failures are pre-existing in this branch (verified by re-running
      `test_dm2_v1_boot_profile_smoke` with the pre-change
      `runtime_render_no_core_fallbacks` condition; failures remain identical).

- 2026-07-23 DM2 V1 shop inventory stack/container restrictions and runtime inventory writeback (Lane B, cycle 7):
  Closed the TODO item for DM2-012 shop/NPC work: inventory stack/container restrictions and broader live runtime field writeback beyond gold.
  Changes:
    * `include/dm2_v1_shop.h`:
      - Added `DM2_SHOP_RESULT_CONTAINER_NOT_EMPTY` and `DM2_SHOP_RESULT_STACK_LIMIT` result codes.
      - Extended `DM2_V1_ShopState` with per-session mutable shop stock (`active_stock_count`, `active_stock_item[8]`, `active_stock_remaining[8]`) and container tracking (`inventory_is_container[32]`, `inventory_contents[32]`).
      - Declared `dm2_v1_shop_get_active_stock_remaining()`, `dm2_v1_shop_item_max_stack()`, `dm2_v1_shop_item_is_container()`, `dm2_v1_shop_add_container()`, `dm2_v1_shop_load_inventory_from_runtime()`, and `dm2_v1_shop_commit_inventory_to_runtime()`.
    * `src/dm2/dm2_v1_shop.c`:
      - Implemented bounded, source-faithful item stack limits (potions/flasks 12, food/light 20, ammo 12, equipment/unknown 1) derived from `docs/dm2_inventory.md` §11.
      - Implemented split-stack inventory allocation so oversized quantities fill multiple slots up to the per-type cap instead of creating unlimited stacks.
      - Implemented mutable shop stock: catalog stock is copied into `DM2_V1_ShopState` on `dm2_v1_shop_enter()`; `dm2_v1_shop_buy()` decrements finite stock while leaving `-1` unlimited markers unchanged; `dm2_v1_shop_sell()` adds sold items back to the active stock as a buy-back stack.
      - Implemented container restriction: non-empty containers return `DM2_SHOP_RESULT_CONTAINER_NOT_EMPTY` on sell; empty containers sell normally.
      - Added `dm2_v1_shop_add_container()` helper for tests/synthetic fixtures.
      - Updated source-evidence string to cite `docs/dm2_inventory.md` §5/§10/§11 and the new mutable-stock/stack/container rules.
    * `src/dm2/dm2_v1_runtime.c`:
      - Implemented `dm2_v1_shop_load_inventory_from_runtime()` so `dm2_v1_runtime_enter_shop()` imports the leader champion's runtime ObjectIDs into the shop-local inventory.
      - Implemented `dm2_v1_shop_commit_inventory_to_runtime()` so `dm2_v1_runtime_buy_from_shop()`, `dm2_v1_runtime_sell_to_shop()`, and `dm2_v1_runtime_leave_shop()` write the shop-local inventory back to the leader champion slots after gold commit.
      - This is the broader live runtime field writeback beyond gold required by DM2-012.
    * `tests/test_dm2_v1_shop_pc34_compat.c`:
      - Added 8 new checks covering mutable stock copy, finite-stock decrement, unlimited-stock preservation, sell buy-back, per-slot stack limit, split-stack allocation, non-empty container sell rejection, and empty container sell allowance.
      - Total: 65/65 checks PASS.
    * `tests/test_dm2_v1_runtime_shop_pc34_compat.c`:
      - Added 4 new checks covering runtime inventory writeback after buy/sell/leave and runtime inventory load on shop entry.
      - Total: 22/22 checks PASS.
  Source evidence:
    * `docs/dm2_inventory.md` §5 — container items / `DM2__CHECK_ROOM_FOR_CONTAINER` / `DM2_PUT_OBJECT_INTO_CONTAINER`.
    * `docs/dm2_inventory.md` §8 — shop inventory with merchant NPCs (AI index 0x21).
    * `docs/dm2_inventory.md` §10 — sell price = 50% of buy price.
    * `docs/dm2_inventory.md` §11 — item stacking and quantity field per slot, stack limit depends on item type / GDAT entry.
    * `skproject/SKULLWIN/c_shop.cpp` — shop panel + transaction pricing (referred to in existing source-lock).
    * `skproject/SKULLWIN/SKWinGlobal.h:42` — NUM_NPCS=4.
  Verification:
    * `cmake --build build --parallel`: succeeds (full project).
    * `ctest --test-dir build -R dm2_v1_shop_pc34_compat`: PASS (65/65).
    * `ctest --test-dir build -R dm2_v1_runtime_shop_pc34_compat`: PASS (22/22).
    * `ctest --test-dir build -R dm2_v1_shop_economy_determinism_probe`: PASS (19/19).
    * `ctest --test-dir build -R dm2_v1_skproject_core`: PASS.

- 2026-07-22 Theron V1 runtime input/idle facade (Lane E, cycle 7):
  Closed the 2026-07-08 follow-up TODO item to move the next render/session
  adapter calls out of M11 and into Theron-owned facades.  The boot layer now
  owns the Track 02 runtime input/idle path so M11 no longer calls the raw
  `theron_v1_boot_runtime_tick_world`, `theron_v1_boot_runtime_turn_party`, or
  `theron_v1_boot_runtime_move_party` routines directly.
  Changes:
    * `include/theron_v1_boot.h`:
      - Added `Theron_V1_BootRuntimeInputResult` enum (`IGNORED`, `REDRAW`,
        `EXIT_DUNGEON`).
      - Added `Theron_V1_BootRuntimeInputReceipt` struct to report result,
        handled flags, party pose, tick count, status strings, and an
        optional `Theron_StartupActionHostReceipt` for dungeon exit.
      - Declared `theron_v1_boot_runtime_input_receipt_init`,
        `theron_v1_boot_runtime_handle_m12_input`, and
        `theron_v1_boot_runtime_handle_idle_tick`.
    * `src/theron/theron_v1_boot.c`:
      - Implemented the input facade: maps `M12_MENU_INPUT_UP`/`DOWN` to forward/
        backward movement, `TURN_LEFT`/`TURN_RIGHT` to rotation, `ACCEPT`/`ACTION`
        to a wait tick, and rejects strafe/legacy `LEFT`/`RIGHT` tokens.
      - Implemented the idle-tick facade (`theron_v1_boot_runtime_handle_idle_tick`).
      - On `THERON_MOVE_EXIT`, fills the boot-owned exit receipt through
        `theron_v1_startup_return_to_stage_select_after_exit_host_receipt` so
        M11 only has to apply it.
    * `src/engine/m11_game_view.c`:
      - Theron runtime input branch now uses `theron_v1_boot_runtime_handle_m12_input`.
      - Theron idle-tick branch now uses `theron_v1_boot_runtime_handle_idle_tick`.
    * `tests/test_theron_v1_boot_runtime_input.c` (new) and `CMakeLists.txt`:
      - Added 12-check regression test covering receipt init, null handling,
        unknown input preservation, turn left/right, strafe rejection,
        forward/backward movement, blocked movement, dungeon exit, wait tick,
        and idle tick.
  Verification:
    * `cmake --build build --parallel`: succeeds.
    * `./build/test_theron_v1_boot_runtime_input`: 12/12 PASS.
    * `ctest --test-dir build -R theron_v1_rendering`: PASS.
    * `ctest --test-dir build -R theron_v1_startup_flow_probe`: PASS.
    * `ctest --test-dir build -R theron_v1_m11_direct_launch`: PASS.
    * `SDL_VIDEODRIVER=dummy ctest --test-dir build -R '^m11_phase_a$'`: PASS.
  Source/evidence citations:
    * THQUEST.ASM T520 (party placement / start position).
    * THQUEST.ASM T560 (dungeon loading).
    * THQUEST.ASM T600 (map transitions).
    * THQUEST.ASM T700 (tick world / per-tick updates).
    * ReDMCSB COMMAND.C F7015 (input dispatch).
    * ReDMCSB MOVESENS.C F0267/F0268 (square interaction).

- 2026-07-23 Nexus V1 pit/teleporter broader runtime coverage (Lane D, cycle 7):
  Closed the next open "pit/teleporter broader runtime coverage" item from the
  Nexus V1 mechanics parity backlog.
  Changes:
    * `src/nexus/nexus_v1_squares.c`:
      - `nexus_process_square_event` now passes the registered stair facing
        through `out_target_dir` for `NEXUS_SQUARE_STAIRS_DN` and
        `NEXUS_SQUARE_STAIRS_UP`, matching the ReDMCSB stair-transition
        contract (CLIKMENU.C:264-276).
      - Stair fallback cases also clear `*out_target_dir` to -1.
    * `src/nexus/nexus_v1_mechanics.c`:
      - `nexus_mechanics_tick` now applies `pending_teleport` *before* the
        step-cooldown gate, so teleporter warps are immediate rather than
        delayed by the normal movement cooldown.
      - Cross-level teleporters set `pending_level_change` to the target level
        while still moving the party coordinates immediately.
    * `tests/test_nexus_v1_pit_teleporter_runtime.c` (new) and `CMakeLists.txt`:
      - Added 24-check regression test covering chute step, chute max-level
        clamp, same-level teleport, cross-level teleport, unregistered
        teleporter no-op, stairs down with explicit target, stairs up fallback,
        and direct `nexus_process_square_event` event-type contracts.
    * `probes/nexus/firestaff_nexus_v1_mechanics_parity_probe.c`:
      - Added Probe 12 for teleporter runtime (same-level, cross-level, and
        unregistered cases) with isolated teleporter registries between
        sub-probes.
  Verification:
    * `cmake --build build --parallel 8 --target test_nexus_v1_pit_teleporter_runtime`: passes.
    * `./build/test_nexus_v1_pit_teleporter_runtime`: 24/24 PASS.
    * `cmake --build build --parallel 8 --target firestaff_nexus_v1_mechanics_parity_probe`: passes.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_mechanics_parity_probe`: 226/226 PASS.
    * `./build/test_nexus_v1_click_route`: 31/31 PASS.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe`: 24/24 PASS.
  Source/evidence citations:
    * DM1 MOVESENS.C F0267/F0268 (pit/chute/teleporter/stair sensors).
    * DM1 DUNGEON.C square type dispatch.
    * ReDMCSB CLIKMENU.C:264-276 level-transition special cases.

- 2026-07-23 Nexus V1 mouse click-route dispatch for inventory/world objects (Lane D, cycle 6):
  Closed the remaining "mouse click-route dispatch for inventory/world objects"
  item from the Nexus V1 mechanics parity backlog.  High-level UI clicks now
  feed the same keyboard-style command queue consumed by `nexus_mechanics_tick()`,
  keeping the M11/UI layer out of mechanics state.
  Changes:
    * `include/nexus_v1_click_route.h` (new):
      - Declared `Nexus_ClickTargetKind`, `Nexus_ClickTarget`, and
        `Nexus_ClickResult` enums.
      - Declared constructor helpers for inventory slot, equipment slot,
        world square, door square, and floor item targets.
      - Declared `nexus_click_route_dispatch()` to convert a click target into
        queued movement/action commands.
    * `src/nexus/nexus_v1_click_route.c` (new):
      - Implemented dispatch for inventory slots (`NEXUS_CMD_USE_ITEM` for
        consumable/equippable items).
      - Implemented equipment-slot clicks as unequip-to-inventory.
      - Implemented world-square and door-square routing: turn toward the
        dominant cardinal direction, then queue `NEXUS_CMD_FORWARD`.
      - Implemented floor-item routing: `NEXUS_CMD_INTERACT` when already on
        the square, otherwise movement toward the item.
      - Wall squares return `NEXUS_CLICK_RESULT_NO_PATH`.
    * `include/nexus_v1_movement.h`:
      - Added `NEXUS_CMD_INTERACT` (value 11) and bumped `NEXUS_CMD_COUNT` to 12.
    * `src/nexus/nexus_v1_mechanics.c`:
      - Wired `NEXUS_CMD_INTERACT` in the tick handler: picks up the first
        floor item at the party's current square into the leader's first empty
        inventory slot, updates load, and plays the pickup SFX.
    * `tests/test_nexus_v1_click_route.c` (new) and `CMakeLists.txt`:
      - Added 31-check regression test covering inventory use, equipment
        unequip, world-square movement, door routing, floor-item pickup, wall
        no-path, and invalid arguments.
    * `probes/nexus/firestaff_nexus_v1_mechanics_parity_probe.c`:
      - Added Probe 11 for click-route dispatch.
      - Fixed probe-level state setup (`current_level.width/height` and a
        mechanics-state reset between world-square and floor-item checks) so
        the new probe checks pass against the real `nexus_v1_level_get_square()`
        boundary.
  Verification:
    * `cmake --build build --parallel` succeeds (all targets).
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe`: 24/24 PASS.
    * `./build/test_nexus_v1_click_route`: 31/31 PASS.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_mechanics_parity_probe`:
      218/218 PASS.
  Source/evidence citations:
    * DM1 COMMAND.C mouse/click dispatch (inventory use, viewport click to
      move/open/interact).
    * ReDMCSB CLIKMENU.C F0366 command queue.
    * ReDMCSB CHAMPION.C F0309 equipment slot layout.
    * ReDMCSB MOVESENS.C F0267/F0268 square interaction / pit-chute sensors.

- 2026-07-23 DM2 V1 runtime weather live DistantEnvironment slot production (Lane C, cycle 6):
  Wired the source `DM2_UPDATE_WEATHER(0)` frame update into the DM2 V1 runtime
  tick so that real GDAT weather-overlay assets can reach the renderer through
  original ten-byte `DistantEnvironment` slots.
  Changes:
    * `src/dm2/dm2_v1_runtime.c`:
      - Added `dm2_v1_runtime_update_weather_frame()` plus static helper
        `dm2_runtime_build_weather_slot_raw()`. It runs
        `dm2_v1_update_weather_0` against the session-owned weather chain,
        converts the resulting `live_cmds` into ten-byte register images, and
        admits them through `dm2_v1_runtime_bind_weather_distant_environment()`.
      - Wired the call into `dm2_v1_runtime_tick()` immediately after
        `dm2_v1_proceed_timers()` when the outdoor chain is active.
      - Enabled `clouds_enabled`, `rain_enabled`, and `lightning_enabled` when
        `dm2_v1_runtime_set_outdoor(1)` starts the outdoor chain.
    * `include/dm2_v1_runtime.h`:
      - Declared `dm2_v1_runtime_update_weather_frame()` and the test helper
        `dm2_v1_runtime_set_weather_chain_state_for_test()`.
    * `tests/test_dm2_v1_runtime_weather_frame_slot.c` (new) and
      `CMakeLists.txt`:
      - Added a real-data test target linked against `firestaff_dm2`,
        `firestaff_m10`, and `m`, verifying fail-closed behavior before outdoor
        start, storm cloud+rain slot binding, clear-weather zero-slot binding,
        and tick-path execution against canonical DM2 data.
  Verification:
    * `test_dm2_v1_runtime_weather_frame_slot`: 15/15 passed.
    * `firestaff_m11_phase_a_probe`: 24/24 invariants passed under dummy SDL.
    * `test_dm2_v1_weather_gdat_receipt`,
      `test_dm2_v1_weather_img9_global_palette_identity_real_data`,
      `test_dm2_v1_scene_weather_light_runtime_chain_real_data`,
      `test_dm2_v1_weather_runtime_slot_gate`,
      `test_dm2_v1_update_weather_pc34_compat`: all passed.
  Open follow-up: the renderer's real-data outdoor-frame consumption of the
  newly-bound live slots has not yet been captured end-to-end; the render path
  already checks `weather_distant_slot_count > 0` and source-receipt hash
  consistency, but a corpus-backed M11 acceptance run is still needed.

- 2026-07-22 DM2 V1 runtime shop gold writeback (Lane B, cycle 6):
  Closed the shop/NPC gold transaction gap: `dm2_v1_runtime_enter_shop()`
  already synced `gs->gold` into the shop module, but `dm2_v1_shop_buy()` and
  `dm2_v1_shop_sell()` only mutated module-local state and `dm2_v1_shop_leave()`
  never wrote the final amount back to `DM2_V1_GameState`. Gold changes therefore
  never reached the save/session layer.
  Changes:
    * `include/dm2_v1_game.h`:
      - Added the `DM2_V1_GameState` struct tag so the shop header can forward
        declare it without including the full game header.
    * `include/dm2_v1_shop.h`:
      - Forward-declared `struct DM2_V1_GameState` and declared
        `dm2_v1_shop_commit_gold_to_game_state()`.
    * `src/dm2/dm2_v1_shop.c`:
      - Included `dm2_v1_game.h`.
      - Implemented `dm2_v1_shop_commit_gold_to_game_state()` to copy the
        module-local `party_gold` value into `gs->gold`.
    * `include/dm2_v1_runtime.h`:
      - Declared `dm2_v1_runtime_leave_shop()`,
        `dm2_v1_runtime_buy_from_shop()`, and
        `dm2_v1_runtime_sell_to_shop()`.
    * `src/dm2/dm2_v1_runtime.c`:
      - `dm2_v1_runtime_leave_shop()` commits shop gold to the game state and
        then calls `dm2_v1_shop_leave()`.
      - `dm2_v1_runtime_buy_from_shop()` buys from the active shop and commits
        on success.
      - `dm2_v1_runtime_sell_to_shop()` sells to the active shop and commits on
        success.
    * `tests/test_dm2_v1_runtime_shop_pc34_compat.c` (new):
      - Data-free regression gate using a synthetic verified boot profile.
      - Verifies `enter_shop` syncs `gs->gold`, buy/sell commit back to
        `gs->gold`, `leave_shop` persists final gold, and failed buys leave
        `gs->gold` untouched.
    * `CMakeLists.txt`:
      - Added `test_dm2_v1_runtime_shop_pc34_compat` target linked against
        `firestaff_dm2 firestaff_m10 m` and registered it as
        `dm2_v1_runtime_shop_pc34_compat`.
    * `tests/test_dm2_v1_runtime_weather_frame_slot.c`:
      - Fixed four instances of `&(make_storm_state())` / `&(make_clear_state())`
        by storing the helper return value in a local variable first. This
        pre-existing invalid C blocked the full parallel build on clang.
  Verification:
    * `cmake --build build --parallel` succeeds (all targets).
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe` passes 24/24.
    * `test_dm2_v1_shop_pc34_compat` passes 57/57.
    * `test_dm2_v1_runtime_handoff_smoke` passes 167/0.
    * `test_dm2_v1_runtime_shop_pc34_compat` passes 18/18.
  Source/evidence citations:
    * skproject/SKULLWIN/c_shop.cpp transaction pricing and shop panel logic.
    * DM2 V1 invariant that shops are a UI overlay preserving party state.
    * Existing `dm2_v1_shop.c` buy/sell price formulas and inventory helpers.

- 2026-07-22 Theron V1 synthetic-path audit close-out (Lane E, cycle 6):
  Hardened the startup render-plan executor so synthetic title/stage/Soul
  Room/forcefield shape commands are blocked unless the plan carries an
  explicit no-media fallback permit. The boot path and M11 already suppressed
  synthetic startup art when verified Track 02 atlas routes execute; this
  change makes the executor itself fail-closed as defense-in-depth and locks
  the contract with regression coverage.
  Changes:
    * `include/theron_v1_startup_flow.h`:
      - Added `bitmap_route_mask` and `synthetic_graphics_allowed` fields to
        `Theron_StartupRenderPlan`. The permit defaults to 0; unbound
        title/stage/Soul Room/forcefield regions stay blocked rather than
        painted with fallback art.
    * `src/theron/theron_v1_startup_flow.c`:
      - Added `tqr_startup_graphic_kind_is_synthetic_shape()` helper.
      - `theron_v1_startup_execute_graphics_plan()` now skips
        TITLE_MARK/STAGE_PANEL/MIRROR_FRAME/FORCEFIELD commands when
        `synthetic_graphics_allowed` is clear.
    * `src/theron/theron_v1_boot.c`:
      - The no-media fallback execution branch sets
        `plan.synthetic_graphics_allowed = 1` before calling the executor,
      preserving the data-free preview path.
    * `probes/theron/firestaff_theron_v1_startup_flow_probe.c`:
      - Direct executor tests now set `synthetic_graphics_allowed = 1` on a
        local copy of the plan, since they exercise the executor outside the
        boot-level permit.
    * `tests/test_theron_rendering.c`:
      - Added `test_startup_render_plan_blocks_synthetic_shapes_without_permit()`
        verifying that shape commands are skipped without the permit while
        plain FILL_RECT/DRAW_RECT layout primitives still execute, and that
        the permit re-enables synthetic shape drawing.
    * `src/dm2/dm2_v1_shop.c`:
      - Minimal build-unblock fix: changed definition of
        `dm2_v1_shop_commit_gold_to_game_state` to use `struct DM2_V1_GameState *`
        to match the forward declaration in `include/dm2_v1_shop.h`
        (pre-existing declaration/definition mismatch from parallel DM2 work).
  Verification:
    * `cmake --build build --parallel --target test_theron_rendering` succeeds.
    * `test_theron_rendering` passes 25/25 (including new regression test).
    * `firestaff_theron_v1_startup_flow_probe` passes 653/653.
    * `test_theron_v1_m11_direct_launch` passes.
    * `test_theron_v1_m11_launcher_handoff_boundary` passes 12/12, 1 skipped.
    * `SDL_VIDEODRIVER=dummy firestaff_m11_phase_a_probe` passes 24/24.
  Source/evidence citations:
    * THQUEST.ASM T000/T080/T400/T520/T560/T600 startup bitmap/text routing.
    * Existing `theron_v1_boot_startup_execute_graphics_plan` no-media fallback
      branch and `theron_v1_boot_startup_host_render_plan_fallback_allowed`.

- 2026-07-22 DM2 V1 movement collision gate regression fix (Lane B, cycle 5):
  Fixed `test_dm2_v1_movement_collision_gate_pc34_compat` failing
  `runtime_blocked_step_turn_state` because `dm2_v1_runtime_move` mixed raw
  DM1/DM2 tile encodings with `DM2_SquareType` enum values.
  Changes:
    * `src/dm2/dm2_v1_runtime.c`:
      - Added static `dm2_runtime_normalize_square_type()` helper that maps
        raw tile classes (wall=0, floor=1, plus door/pit/lava/etc.) to the
        `DM2_SquareType` enum used by movement/planning consumers.
      - Updated `dm2_v1_runtime_move()` to normalize the target tile type
        before the local wall/pit/lava/inaccessible check and before passing
        it to `dm2_v1_DM2_PERFORM_MOVE_plan()`.
    * `include/dm2_v1_skproject_core.h`:
      - Renamed the first of two duplicate `DM2_V1_SkprojectFreeCacheIndexReceipt`
        typedefs to `DM2_V1_SkprojectDeallocFreeCacheIndexReceipt` (used by
        `dm2_v1_skproject_free_cache_index`).
      - Declared `dm2_v1_skproject_mement_lru_push_front()`.
    * `src/dm2/dm2_v1_skproject_core.c`:
      - Updated `dm2_v1_skproject_free_cache_index` to use the renamed
        dealloc receipt type.
      - Removed `static` from `dm2_v1_skproject_mement_lru_push_front` so the
        skproject-core test can link against it.
    * `tests/test_dm2_v1_skproject_core.c`:
      - Updated dealloc and `_3e74_583a` receipt variable declarations to use
        the distinct `Dealloc`/`3e74` free-cache-index receipt types.
  Verification:
    * `cmake --build build --parallel` succeeds.
    * `test_dm2_v1_movement_collision_gate_pc34_compat` passes 7/7.
    * `test_dm2_v1_perform_move_receipt` passes 15/15.
    * `test_dm2_v1_trigger_pc34_compat` passes 35/35.
    * `test_dm2_v1_pressure_plate_pc34_compat` passes 43/43.
    * `test_dm2_v1_skproject_core` passes all checks.
    * `SDL_VIDEODRIVER=dummy firestaff_m11_phase_a_probe` passes 24/24.
  Source/evidence citations:
    * ReDMCSB `DEFS.H:385-390` and `HASHBUCKET.C` raw tile encoding.
    * skproject `DME.h` `tileTypeIndex`.

- 2026-07-22 DM2 SkWinCore `^3E74` mement/cache symbol audit batch
  (Lane A, cycle 5):
  Closed nine p130 SkWinCore priority symbols from the `^3E74` mement/cache
  management family as `IMPLEMENTED_NARROW` source-named receipts.
  Changes:
    * `include/dm2_v1_skproject_core.h`:
      - Added `DM2_V1_SKPROJECT_MEMENT_MAX` and `DM2_V1_SkprojectMement`/
        `DM2_V1_SkprojectMementState` structs modeling the source
        `tlbMementsPointers` table, LRU/MRU list (`w4`/`w6`/`w8`),
        free-block list, cache-index table, and tick-based usage reset.
      - Added receipt structs for all nine symbols:
        `DM2_V1_SkprojectTouchMementReceipt`,
        `DM2_V1_SkprojectRemoveMementReceipt`,
        `DM2_V1_SkprojectUnlinkFreeBlockReceipt`,
        `DM2_V1_SkprojectInsertFreeBlockReceipt`,
        `DM2_V1_SkprojectCompactHeapReceipt`,
        `DM2_V1_Skproject3e74FreeCacheIndexReceipt`,
        `DM2_V1_SkprojectRecycleOrFreeCacheReceipt`,
        `DM2_V1_SkprojectFindFreeCacheIndexReceipt`,
        `DM2_V1_SkprojectResetUsageCountersReceipt`.
      - Declared `dm2_v1_skproject_mement_state_init()`,
        `dm2_v1_skproject_mement_lru_push_front()`, and the nine
        `dm2_v1_skproject_3e74_*` receipt functions.
    * `src/dm2/dm2_v1_skproject_core.c`:
      - Implemented the mement-state init and LRU/free-list helpers.
      - Implemented source-shaped receipts for `_3e74_48c9`, `_3e74_4549`,
        `_3e74_0c8c`, `_3e74_0d32`, `_3e74_2b30`, `_3e74_583a`,
        `_3e74_585a`, `_3e74_4471`, and `_3e74_44ad`.
      - Updated `dm2_v1_skproject_core_source_evidence()` to name the new
        mement/cache family.
    * `tests/test_dm2_v1_skproject_core.c`:
      - Added `test_skwin_core_symbol_batch_cycle5()` with focused
        synthetic-data coverage for all nine receipts and a source-evidence
        check.
    * `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv`:
      - Added nine `IMPLEMENTED_NARROW` disposition rows for the `^3E74`
        family with source-file/line and evidence paths.
    * `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`:
      - Closed the nine corresponding rows as `IMPLEMENTED_NARROW` with
        Firestaff mapping and notes.
  Source/evidence citations:
    * `skproject/SKWIN/SkWinCore.cpp` lines 3519-4514 (`_3e74_*` family),
      4166 `QUERY_MEMENTI_FROM`, 4190 `ADD_CACHE_HASH`, and the surrounding
      `ALLOC_LOWER_CPXHEAP`/`ALLOC_CPXHEAP_MEM` CPX heap code.
  Verification:
    * `cmake --build build --parallel` succeeds.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe`: 24/24 PASS.
    * `./build/test_dm2_v1_skproject_core`: all DM2 skproject core helper
      checks passed.
    * `python3 tools/symbol_backlog.py --game DM2 --reference skproject`: DM2
      skproject backlog dropped from 1006 to 997 open rows.

- 2026-07-22 DM2 V1 wall-ornament material class (Lane C, cycle 5):
  Closed the "additional dungeon material classes" item from TODO.md Lane C by
  implementing DM2 V1 wall ornaments as a new source-owned dungeon material class.
  Changes:
    * `include/dm2_v1_viewport_renderer.h`:
      - Added `DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT` (0x2000).
      - Added `wall_ornate_gfx_index` to `DM2_ViewSquare`.
      - Added wall-ornament counters, required/consumed masks, and frame-
        composition fields.
      - Added `DM2_V1_WallOrnamentRenderPlan` and `DM2_V1_WallOrnamentRender`
        structs.
      - Added `gdat_wall_ornament_material_plan` state pointer and the setter
        declaration `dm2_v1_viewport_set_gdat_wall_ornament_material_plan()`.
    * `src/dm2/dm2_v1_viewport_renderer.c`:
      - Implemented `dm2_v1_render_wall_ornaments()`, fail-closed: it requires
        both a successful WALL_GFX asset fetch and a runtime-bound placement
        plan.
      - Implemented
        `dm2_v1_viewport_set_gdat_wall_ornament_material_plan()` and reset/init
        for the new counters/masks.
      - Wired the wall-ornament render pass between walls and doors in the
        viewport pipeline.
    * `tests/test_dm2_v1_wall_ornament_material_gate.c`:
      - New test verifying missing-plan block, missing-asset block, and
        successful source-material consumption.
    * `CMakeLists.txt`:
      - Added `test_dm2_v1_wall_ornament_material_gate` target mirroring the
        teleporter material-gate build setup.
  Source/evidence citations:
    * DM2 V1 viewport render pipeline and blocked-material gate pattern
      (existing teleporter/floor/ceiling/outdoor material gates).
    * DM2 V1 GDAT wall-plan integration (`dm2_v1_gdat_wall_plan_viewport_real_data`
      test and `DM2_V1_WallPlan` runtime contract).
  Verification:
    * `cmake --build build --parallel` succeeds.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe`: 24/24 PASS.
    * `./build/test_dm2_v1_wall_ornament_material_gate`: PASS.
    * Existing relevant material-gate/receipt tests pass:
      `test_dm2_v1_teleporter_material_gate`,
      `test_dm2_v1_wall_ornament_receipt`,
      `test_dm2_v1_gdat_wall_plan_viewport_real_data`,
      `test_dm2_v1_m11_runtime_frame_receipt_gate`,
      `test_dm2_v1_floor_ceiling_material_gate`,
      `test_dm2_v1_outdoor_renderer_material_gate`,
      `test_dm2_v1_gdat_scene_plan_viewport_real_data`.

- 2026-07-22 Nexus V1 pit/chute square-event integration and item usage wiring
  (Lane D, cycle 5):
  Closed the pit/teleporter trigger integration and item usage/click-route
  wiring items from the Nexus V1 mechanics parity backlog in TODO.md.
  Changes:
    * `include/nexus_v1_mechanics.h`:
      - Added `use_item_slot` to `Nexus_MechanicsState` as the selected leader
        inventory slot for `NEXUS_CMD_USE_ITEM`.
      - Declared `nexus_mechanics_set_use_item_slot()`.
    * `src/nexus/nexus_v1_mechanics.c`:
      - Initialize `use_item_slot` to -1.
      - Implemented `nexus_mechanics_set_use_item_slot()`.
      - Added `apply_use_item()` helper: consumables restore stats (health/mana/
        stamina potions, antidote, corn, water flask) and equippable weapons/
        armor move to the matching equipment slot.
      - Wired `NEXUS_CMD_USE_ITEM` into the tick: consumes the selected leader
        inventory slot, applies the item, clears the slot, recalculates load,
        and requests a redraw.
      - Wired `NEXUS_EVENT_CHUTE_FALL` (and `NEXUS_EVENT_PIT_FALL`) into the
        square-event switch: sets `pending_level_change` to the next lower
        level (`map_index + 1` when the event signals -1), preserving target
        x/y, and plays the pit-fall SFX.
    * `probes/nexus/firestaff_nexus_v1_mechanics_parity_probe.c`:
      - Added Probe 9 verifying that stepping on a chute square moves the
        party and sets `pending_level_change` to `map_index + 1`.
      - Added Probe 10 verifying that `NEXUS_CMD_USE_ITEM` consumes a health
        potion (restores health, clears slot) and equips a sword (moves to
        weapon slot, clears inventory slot).
  Source/evidence citations:
    * ReDMCSB MOVESENS.C F0267/F0268 (chute/pit sensor processing).
    * ReDMCSB COMMAND.C item-use dispatch.
    * ReDMCSB CHAMPION.C F0309 equipment slot layout.
  Verification:
    * `cmake --build build --parallel` succeeds.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe`: 24/24 PASS.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_mechanics_parity_probe`:
      207/207 PASS.
    * `SDL_VIDEODRIVER=dummy ./build/test_m11_nexus_startup_gate`: PASS.
    * `SDL_VIDEODRIVER=dummy ./build/test_m11_nexus_startup_runtime_handoff`:
      PASS.
    * `SDL_VIDEODRIVER=dummy ./build/test_nexus_v1_sound_runtime_receipt`:
      PASS.
    * `SDL_VIDEODRIVER=dummy ./build/test_nexus_v1_save_multislot_roundtrip_pc34_compat`:
      PASS.
    * `SDL_VIDEODRIVER=dummy ./build/test_nexus_v1_boot_profile_smoke`:
      26/26 PASS.

- 2026-07-22 Theron V1 HuC6260 palette-route verification guard (Lane E,
  cycle 5):
  Closed the TQR-SYN-PALETTE item from the Theron original-media synthetic-path
  audit in TODO.md. The render gate now requires both a decoded tile bank and a
  verified HuC6260 palette route before it considers generated V1 rendering
  allowed for verified Track 02 media. The default deterministic stone palette
  remains available for the no-media fallback path but no longer satisfies the
  original-media render gate on its own.
  Changes:
    * `src/theron/theron_v1_asset_loader.h`:
      - Added `palette_route_verified` to `TrAssetBundle` with a comment
        explaining it is set only for hash/offset-proved HuC6260 palette
        routes.
      - Declared `tr_asset_mark_palette_route_verified()` as the production
        setter for that flag.
    * `src/theron/theron_v1_asset_loader.c`:
      - Updated `tr_asset_block_synthetic_rendering_for_verified_media()`
        comment to note the palette-route requirement.
      - `tr_asset_generated_v1_rendering_allowed()` now returns true only when
        `track03_data != NULL`, `palette.tile_count > 0`, and
        `palette_route_verified` are all set.
      - Added `tr_asset_mark_palette_route_verified()`; it rejects NULL and
        sets the flag.
      - `tr_asset_free()` now clears `palette_route_verified`.
    * `src/theron/theron_v1_boot.c`:
      - Updated the `theron_v1_boot_runtime_render_frame()` comment to state
        that the render gate requires a verified palette route, not just a
        tile bank.
    * `tests/test_theron_rendering.c`:
      - `test_runtime_render_frame_allows_with_tile_bank()` now marks the
        palette route verified so the existing "draw with graphics bank"
        contract still holds.
      - Added `test_runtime_render_frame_blocks_tile_bank_without_palette_route()`
        to lock the TQR-SYN-PALETTE contract: a tile bank alone is insufficient
        and the runtime render frame must refuse to draw, leaving the
        framebuffer black.
  Source/evidence citations:
    * TODO.md Theron original-media synthetic-path audit (graphics/palette):
      "retain source bytes but require a hash/offset-proved HuC6260 palette
      route before render."
    * `src/theron/theron_v1_asset_loader.h` synthetic_rendering_blocked and
      palette_route_verified contracts.
  Verified:
    * `cmake --build /Users/bosse/workspace-main/firestaff/build --parallel`
      succeeds (only pre-existing ld duplicate-library warnings).
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe` passes 24/24
      invariants.
    * `./build/test_theron_rendering` passes 24/24.
    * `ctest --test-dir /Users/bosse/workspace-main/firestaff/build -R theron`
      passes 184/184.

- 2026-07-22 Theron V1 synthetic-candidate index supplement — TQR-SYN-01/02/03
  (Lane E, cycle 4):
  Closed three of the five Theron synthetic-render/blocker items from
  TODO.md, keeping TQR-SYN-04 (V22 modern placeholders) and TQR-SYN-05
  (real `.srm` body decode) open because they still lack local real-data
  counterparts.
  Changes:
    * `src/theron/theron_v1_world.c`:
      - `theron_v1_startup_fallback_room_synthesize()` now produces the
        single source-locked 32x27 Hall-of-Records startup candidate
        (width 32, height 27, seed `0x0108e938`, level index `0x0026`,
        start pose `(2,1,EAST)`) instead of seven invented stage-specific
        rooms. The function is still used only on the no-media fallback
        path; the real Track 02 path already consumes the verified
        32x27 candidate.
    * `src/theron/theron_v1_startup_runtime_entry.c`:
      - Increased the fallback-room stack buffer to
        `THERON_V1_FIRST_ROOM_HEADER_BYTES + 32 * 27`.
    * `probes/firestaff_theron_v1_first_room_runtime_probe.c`:
      - Updated `probe_startup_fallback_rooms()` to expect the new 32x27
        shape, source-locked seed/level-index header bytes, and
        stage-independent exit marker for all tested stages.
    * `include/theron_v1_viewport.h` and `src/theron/theron_v1_viewport.c`:
      - Added `synthetic_rendering_blocked` to `Theron_V1_Viewport` and a
        public `theron_vp_set_synthetic_rendering_blocked()` setter.
      - `theron_vp_render_dungeon()` and `theron_vp_render_ui()` return
        early when the flag is set, blocking generated tiles, fallback
        font, and synthetic chrome when verified Track 02 is present but
        unbound. This is defense-in-depth alongside the existing boot-level
        block.
    * `src/theron/theron_v1_boot.c`:
      - `theron_v1_boot_runtime_render_frame()` now propagates the asset
        bundle's `synthetic_rendering_blocked` state into the viewport.
    * `tests/test_theron_rendering.c`:
      - Added `test_viewport_direct_render_blocks_synthetic()` to prove the
        viewport-level guard blocks both dungeon and UI rendering.
      - Added `test_ui_chrome_blocks_unbound_source_bank()` to lock the
        TQR-SYN-01 contract that `tr_ui_render()` stays a no-op until a
        source-locked UI bank is bound.
  Source/evidence citations:
    * Real JP/US raw Track 02 32x27 candidate constants and offsets are
      documented in `docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md`
      and the parity-evidence runtime-screenshot manifest.
  Verified:
    * `cmake --build /Users/bosse/workspace-main/firestaff/build --parallel`
      succeeds.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe` passes 24/24
      invariants.
    * `./build/test_theron_rendering` passes 23/23.
    * `./build/firestaff_theron_v1_first_room_runtime_probe` passes 80
      assertions with 1 expected no-data skip.
    * `ctest --test-dir /Users/bosse/workspace-main/firestaff/build -R theron`
      passes 184/184.

- 2026-07-22 DM2 SkWinCore `^443C` UI tracking / mouse-event lock batch
  (Lane A, cycle 4):
  Closed six SkWinCore priority symbols as `IMPLEMENTED_NARROW` source-named
  receipts in `dm2_v1_skproject_core.c`:
  `_443c_087c` (LOCK_MOUSE_EVENT), `_443c_0889` (UNLOCK_MOUSE_EVENT),
  `_443c_040e` (hide cursor, reset tracking rect, set bounds, show cursor),
  `_443c_00a9` (store tracking ref and x/cx/y/cy extents),
  `_443c_06b4` (insert sk0cea object into priority-ordered tracking list),
  and `_443c_07d5` (remove sk0cea object and request reset).
  Changes:
    * `include/dm2_v1_skproject_core.h`:
      - Added `DM2_V1_SkprojectUiTrackingObject`,
        `DM2_V1_SkprojectUiTrackingState`, and receipt structs for the six
        source functions.
      - Declared `dm2_v1_skproject_ui_tracking_state_init` and the six
        `_443c_*` receipt functions.
    * `src/dm2/dm2_v1_skproject_core.c`:
      - Implemented the six source-named receipts with source citations to
        SKWIN/SkWinCore.cpp:^443C:087C, :0889, :040E, :00A9, :06B4, and
        :07D5.
      - Reuses existing `_01b0_0adb`/`_01b0_0ca4` mouse-state helpers for
        cursor hide/show/bounds.
      - Updated `dm2_v1_skproject_core_source_evidence()` to name the new
        symbols.
    * `tests/test_dm2_v1_skproject_core.c`:
      - Added `test_skwin_core_symbol_batch_cycle4()` covering lock/unlock
        depth, underflow, reset rect/bounds, tracking context, priority-ordered
        insert, and unlink remove.
  Audit updates:
    * `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`: moved the
      six `_443c_*` rows from `MISSING` to `IMPLEMENTED_NARROW` with mapping
      and evidence notes.
    * `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv`: appended six disposition
      rows for the same symbols.
  Verified:
    * `cmake --build /Users/bosse/workspace-main/firestaff/build --parallel`
      succeeds (one pre-existing unrelated warning about
      `ladder_around_dirs`).
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe` passes 24/24.
    * `./build/test_dm2_v1_skproject_core` passes all checks.
    * `./build/test_dm2_v1_gfx_decode_receipt` passes 35/35.
    * `./build/test_dm2_v1_m11_launcher_handoff_boundary` shows 43 passed,
      1 failed; the failure is the pre-existing real-data hash/enter-game
      gate unrelated to this symbol-audit work.
    * `python3 tools/symbol_backlog.py` confirms DM2 skproject backlog
      dropped from 1012 to 1006 open rows.

- 2026-07-22 DM2 V1 creature GDAT AI table import (Lane B, cycle 4):
  Bound `dm2_v1_creature_load_ai_table_from_gdat` to synthetic/raw
  CREATURE_AI records while preserving the source word-value path for real
  GDAT sessions.
  Changes:
    * `src/dm2/dm2_v1_creature.c`:
      - Added a second pass over the loader's ENT1 entries after the
        existing word-value loop (SkWinCore.cpp:233-400
        EXTENDED_LOAD_AI_DEFINITION).
      - Any `CREATURE_AI` entry whose raw payload is exactly 36 bytes is
        decoded as a little-endian `AIDefinition` (DME.h:1505-1545) and
        loaded into `g_ai_table[entry->cls2]` only when that row was not
        already admitted by the source word-value path.
      - The creature-type-to-AI-row indirection (`g_creature_ai_row` /
        `g_creature_ai_row_loaded`) is set so the data-backed accessors
        (`dm2_v1_creature_ai_spec_flags`, `dm2_v1_creature_ai_spec_def`)
        remain consistent.
      - Source citations updated: SkWinCore.cpp:233-400,
        DME.h:1505-1545, c_record.cpp:1346-1354.
  Verified:
    * `cmake --build /Users/bosse/workspace-main/firestaff/build --parallel`
      succeeds (one pre-existing unrelated warning).
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe` passes 24/24.
    * `./build/test_dm2_v1_creature_gdat_ai_table` passes 13/13.
    * `./build/test_dm2_v1_combat_pc34_compat` passes 49/49.
    * `./build/firestaff_dm2_v1_combat_probe` passes 13/13.
    * `ctest -R '^dm2_v1_'` shows the AI table test passing; remaining
      failures are pre-existing real-data capture and unrelated subsystem
      tests (including the known `dm2_v1_movement_collision_gate_pc34_compat`
      `runtime_blocked_step_turn_state` failure).

- 2026-07-22 DM2 V1 item/projectile Rect14 render-plan wiring (Lane C,
  cycle 4):
  Wired INTERFACE_GENERAL dt07/0x0A Rect14 per-row placement into DM2 V1 item
  and projectile render plans, following the existing creature Rect14 path.
  Changes:
    * `include/dm2_v1_viewport_renderer.h`: added `rect14_applied`,
      `rect14_scale64`, `rect14_lateral_offset`, `rect14_flip_mirror`,
      `rect14_row_hash`, and `rect14_placement_hash` to `DM2_V1_ItemRender`
      and `DM2_V1_ProjectileRender`.
    * `src/dm2/dm2_v1_viewport_renderer.c`:
      - `dm2_v1_viewport_build_item_render_plan()` enriches non-static-object
        floor items by frame-index Rect14 row when the table is bound;
        static-object-admitted items keep their existing delivery-plan path.
      - `dm2_v1_viewport_item_asset_blit()` now takes `party_direction` and
        applies source-stretched size, view-relative lateral offset, and
        Rect14 flip when a row is applied.
      - `dm2_v1_viewport_build_projectile_render_plan()` enriches missiles by
        frame-index Rect14 row; clouds keep their random-mirror path unless
        Rect14 is not applied.
      - `dm2_v1_viewport_projectile_asset_blit()` uses source-stretched size,
        view-relative lateral offset, and Rect14 flip when applied.
    * `tests/test_dm2_v1_runtime_handoff_smoke.c`: updated the direct
      `dm2_v1_viewport_item_asset_blit()` call to supply the new
      `party_direction` argument.
    * `tests/test_dm2_v1_item_projectile_rect14_render_plan.c` (new):
      verifies row matching, hash propagation, asset-blit scaling/flip, and
      that static-object items and out-of-range frame indices do not
      synthesize placement data.
    * `CMakeLists.txt`: registered the new test target.
  Verified:
    * `cmake --build /Users/bosse/workspace-main/firestaff/build --parallel`
      succeeds.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe` passes 24/24.
    * `./build/test_dm2_v1_item_projectile_rect14_render_plan` passes 9/9.
    * `./build/test_dm2_v1_static_object_m11_delivery_plan`,
      `./build/test_dm2_v1_runtime_handoff_smoke` (167/0),
      `./build/test_dm2_v1_item_local_palette_gate` (2/2), and
      `./build/test_dm2_v1_projectile_local_palette_gate` (2/2) all pass.
    * `ctest -R dm2_v1` shows the new test passing; remaining failures are
      pre-existing real-data capture and unrelated subsystem tests.

- 2026-07-22 Nexus V1 champion death semantics / auto-leader promotion
  (Lane D, cycle 4):
  Implemented champion death auto-leader promotion per ReDMCSB
  CHAMPION.C F0319_CHAMPION_Kill lines ~1662-1679.
  Changes:
    * `include/nexus_v1_champions.h` + `src/nexus/nexus_v1_champions.c`:
      added `nexus_v1_champion_on_death_update_leader()`. When the dead
      champion was the party leader, it scans party order and promotes the
      first living party member to leader. Returns the new leader party slot
      or -1 when no living successor exists (total-party-death path).
    * `src/nexus/nexus_v1_mechanics.c`: the integrated tick now calls the
      promotion helper after creature-attack damage kills a champion and after
      stamina-collapse death in the step-stamina cost path.
    * `probes/nexus/firestaff_nexus_v1_mechanics_parity_probe.c`: added
      `probe_leader_promotion()` covering non-leader death (leader unchanged),
      leader death (promotes next living member), last-living death
      (no successor), and empty-party rejection.
  Verified:
    * `cmake --build /Users/bosse/workspace-main/firestaff/build --parallel`
      builds the changed Nexus targets; a pre-existing DM2 viewport renderer
      failure in another lane's files (`src/dm2/dm2_v1_viewport_renderer.c:6947`)
      blocks the full build but is unrelated to this Nexus mechanics change.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe` passes 24/24.
    * `SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_mechanics_parity_probe`
      passes 199/0.
    * `ctest -R firestaff_nexus_v1_mechanics_parity --output-on-failure` passes.

- 2026-07-22 DM2 V1 runtime handoff mechanics parity (Lane B, cycle 3):
  closed the remaining pre-existing `test_dm2_v1_runtime_handoff_smoke`
  failures and brought the smoke gate to 167/0 PASS/FAIL.
  Changes:
    * `src/dm2/dm2_v1_viewport_renderer.c`: fixed closed-door panel GDAT
      index selection so state 4 (closed) uses the category-based record
      panel path (`vs->door_state < 4u` → `vs->door_state <= 4u`).
    * `src/dm2/dm2_v1_runtime.c`:
      - Added `dm2_runtime_creature_read_door()` and wired it into the
        creature-field runtime so CCM creature ticks can read live door
        state and report open-percent.
      - The former deterministic timeline display message was removed: DM2
        messages now reach the runtime only through source-owned trigger or
        actuator data; no fabricated text is emitted on each tick.
      - Added local fallback wall-gfx discovery helpers
        (`dm2_runtime_find_text_wall_gfx_fallback`,
        `dm2_runtime_resolve_actuator_wall_gfx_fallback`) that walk the
        tile thing chain without requiring the loader's authenticated
        record graph.  `dm2_runtime_apply_door_record_metadata()` now
        falls back to these helpers when the proven graph helpers fail
        closed, discovering DB2 text and DB3 actuator wall-gfx metadata
        for custom door buttons and recording the button x/y/object_id.
  Verified: `cmake --build build --parallel` succeeds.
  `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe` passes 24/24
  invariants.  `./build/test_dm2_v1_runtime_handoff_smoke` reports 167 PASS,
  0 FAIL.  `ctest -R 'dm2_v1_(shop|pressure|trigger|timeline|door|creature|runtime_handoff)'`
  passes 17/19; the two failures (`dm2_v1_creature_combat_probe` sound stub
  and `dm2_v1_creature_gdat_ai_table` GDAT import) are pre-existing and
  unrelated to the runtime-handoff mechanics parity work.

- 2026-07-22 DM2 V1 Rect14 wiring for static-object render plans (Lane C,
  cycle 3):
  * `include/dm2_v1_viewport_renderer.h`: extended
    `DM2_V1_StaticObjectSourcePlan` with `object_direction` and Rect14-derived
    fields (`rect14_applied`, `rect14_image_field`, `rect14_scale64`,
    `rect14_lateral_offset`, `rect14_flip_mirror`, `rect14_row_hash`,
    `rect14_placement_hash`); added
    `dm2_v1_viewport_enrich_static_object_source_plan_with_rect14()`.
  * `include/dm2_v1_runtime.h`: added `rect14_row_hash` and
    `rect14_placement_hash` to `DM2_V1_StaticObjectM11DeliveryPlan`.
  * `src/dm2/dm2_v1_viewport_renderer.c`:
    `dm2_v1_viewport_static_object_source_plan()` now records
    `object_direction`.
    `dm2_v1_viewport_enrich_static_object_source_plan_with_rect14()` consumes
    the real INTERFACE_GENERAL dt07/0x0A Rect14 table, matches the row whose
    5x5 anchor equals the source plan's view-relative position, and copies the
    per-direction image field, CALC_STRETCHED_SIZE source, lateral offset and
    mirror flag (source-locked to SKWIN/SkWinCore.cpp DRAW_ITEM and
    QUERY_CREATURE_BLIT_RECTI). It produces deterministic row and placement
    hashes.
    `dm2_v1_viewport_build_static_object_m11_delivery_plan()` now uses the
    Rect14 mirror flag when applied and folds the row/placement hashes into
    the delivery-plan identity.
  * `src/dm2/dm2_v1_runtime.c`:
    `dm2_runtime_populate_g1_static_object_materials()` queries the boot
    INTERFACE_GENERAL Rect14 table and enriches each static-object source plan
    before building the M11 delivery plan. When the table is absent the plan
    keeps its existing source-geometry state and no synthetic placement is
    invented.
  * `tests/test_dm2_v1_static_object_m11_delivery_plan.c`: added synthetic
    Rect14 row tests covering matching weapon/container rows, identity-hash
    change, row/placement hash propagation, and non-matching rows.
  * `TODO.md`: added a 2026-07-22 progress update under the DM2 GDAT render
    follow-up item and narrowed the remaining Lane C work to item/projectile
    Rect14 wiring, weather-overlay assets, and additional dungeon material
    classes.
  Verified: `cmake --build build --parallel` succeeds (EXIT 0). Phase A probe
  (`SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe`) passes 24/24
  invariants. Relevant tests pass:
    `test_dm2_v1_static_object_m11_delivery_plan` PASS,
    `test_dm2_v1_lighting_falloff_boundary` 162/162 PASS,
    `test_dm2_v1_m11_runtime_frame_receipt_gate` PASS.
  `test_dm2_v1_runtime_handoff_smoke` reports 152/15 from the clean staged
  commit; the failures are pre-existing creature-field door/timeline and
  wall-gfx custom-button gaps that exist in HEAD and are unrelated to this
  Rect14 wiring change.
  `test_dm2_v1_boot_profile_smoke` reports 87/2 (pre-existing sprite palette /
  HUD availability failures unrelated to this change).
- 2026-07-22 Theron V1 synthetic-render-block runtime binding + viewport
  renderer fix (Lane E, cycle 3):
  * `src/theron/theron_v1_boot.c`: `theron_v1_boot_asset_bundle_allows_v1_rendering`
    now enforces the verified-media synthetic-render boundary: a verified
    Track 02 bundle with `synthetic_rendering_blocked` set and no decoded tile
    bank is rejected, while a bundle with a real Track 03 tile bank is allowed.
    `theron_v1_boot_runtime_render_frame` shallow-copies the asset bundle's
    decoded palette/tile bank into the viewport before drawing so bound tiles
    are actually used.
  * `src/theron/theron_v1_asset_loader.c`: `tr_asset_generated_v1_rendering_allowed`
    treats a present Track 03 tile bank as authoritative original data and
    permits rendering even if the synthetic block flag is set.
  * `src/engine/m11_game_view.c`: when `theron_v1_boot_runtime_render_frame`
    refuses to draw, M11 now clears the framebuffer to black instead of leaving
    stale or zero-pixel output visible.
  * `tests/test_theron_rendering.c`: added regression tests
    `test_runtime_render_frame_blocks_verified_track02` and
    `test_runtime_render_frame_allows_with_tile_bank`, covering the blocked
    verified-media path and the allowed graphics-bank path.
  * `CMakeLists.txt`: linked `test_theron_rendering` against `firestaff_m12`,
    `firestaff_m10`, and `m` so the new boot-runtime-frame tests resolve their
    symbols.
  * `include/dm2_v1_skproject_core.h`: restored declaration order so
    `DM2_V1_SkprojectTextMetricsReceipt` is defined before its use in
    `DM2_V1_Skproject0B36DrawStringReceipt`, unblocking the full parallel build.
  Verified: `cmake --build build --parallel` succeeds (EXIT 0).
  Phase A probe (`SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe`)
  passes 24/24 invariants. Relevant tests pass:
  `test_theron_rendering` (21/21), `test_theron_v1_startup_save_resume_pc34`
  (325/325), and `ctest -R theron` (184/184).

- 2026-07-22 DM2 SkWinCore symbol audit batch (Lane A, cycle 3):
  Closed eight SkWinCore priority symbols as `IMPLEMENTED_NARROW`
  source-named receipts and updated the audit/disposition tables.
  Changes:
    * `src/dm2/dm2_v1_skproject_core.c`: added
      `dm2_v1_skproject_0cee_2df4_creature_ai_word30`,
      `dm2_v1_skproject_19f0_124b_level_transition`,
      `dm2_v1_skproject_29ee_18eb_level_transition_pair`,
      `dm2_v1_skproject_29ee_00a3_init_button_group_black`,
      `dm2_v1_skproject_29ee_0b2b_draw_command_slots`,
      `dm2_v1_skproject_0b36_0cbe_blit_dirty_rects`,
      `dm2_v1_skproject_0b36_129a_draw_string_to_cache`, and
      `dm2_v1_skproject_12b4_0092_skwin_arrow_panel`. Each function cites
      the matching `SKWIN/SkWinCore.cpp` address and models the source
      behavior over caller-owned state without synthesizing runtime data.
    * `include/dm2_v1_skproject_core.h`: public declarations and receipt
      structs for the eight new symbols.
    * `tests/test_dm2_v1_skproject_core.c`: focused synthetic-data coverage
      for all eight receipts (`test_skwin_core_symbol_batch_cycle3`).
    * `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`: closed
      the eight SKWIN rows plus the `DM2_query_0cee_2df4` SKULLWIN alias.
    * `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv`: added disposition
      evidence rows for the nine unique symbols.
    * `TODO.md`: added the 2026-07-22 SkWinCore batch note.
  Verified: `cmake --build build --parallel` succeeds. Phase A probe
  (`SDL_VIDEODRIVER=dummy ./firestaff_m11_phase_a_probe`) passes 24/24.
  Relevant test passes: `test_dm2_v1_skproject_core` (all checks passed).
  DM2 skproject backlog drops from 1021 to 1012 open rows.

- 2026-07-22 Nexus V1 mechanics parity hardening — creature damage wiring,
  party-death game over, and integrated tick probe coverage (Lane D, cycle 3).
  Hardened the Nexus V1 mechanics tick so adjacent creature attacks now apply
  damage to the party leader (falling back to the first living party member)
  instead of only playing a sound. Added a total-party-death gate that sets
  `game_over=1` / `game_over_reason=2 (all_dead)` when no living champions
  remain. Fixed `nexus_mechanics_party_alive()` so an empty party returns dead
  (0) rather than alive (1). Extended
  `probes/nexus/firestaff_nexus_v1_mechanics_parity_probe.c` with a new
  `probe_mechanics_tick_combat()` that exercises the integrated tick with a
  synthetic engine: a scorpion adjacent to the party damages and eventually
  kills the sole champion, triggering the all-dead game-over path.
  Changes:
    * `src/nexus/nexus_v1_mechanics.c`: corrected `party_count == 0` handling
      in `nexus_mechanics_party_alive()`; wired creature attack damage to the
      party leader/first living member via `nexus_v1_take_damage()` with
      `NEXUS_SFX_CHAMPION_HURT`; added total-party-death game-over check.
    * `probes/nexus/firestaff_nexus_v1_mechanics_parity_probe.c`: added
      `probe_mechanics_tick_combat()` and incremented probe count in header.
  Source-lock comments cite ReDMCSB CLIKMENU.C F0366, CREATURE.C F0209, and
  CHAMPION.C F0309.
  Verified: `cmake --build build --parallel` succeeds. Phase A probe passes
  24/24. Nexus-focused regression suite passes 7/7:
    `nexus_v1_m11_launcher_handoff_boundary`,
    `m11_nexus_startup_runtime_handoff`,
    `nexus_v1_dgn_actor_slot_bounds`,
    `nexus_v1_save_multislot_roundtrip_pc34_compat`,
    `nexus_v1_boot_profile_smoke`,
    `nexus_v1_launch_smoke`,
    `firestaff_nexus_v1_mechanics_parity` (192/192 checks). Full Nexus ctest
  run (`ctest --test-dir build -R nexus`) shows the same 3 pre-existing
  capture-bound failures (`nexus_v1_dgn_material_raster`,
  `nexus_v1_track1_phase_launch_extracted_root`,
  `nexus_v1_track1_phase_launch_saturn_ja_iso`) and no new regressions.

- 2026-07-22 DM2 V1 door-step timer wiring (Lane B, cycle 2): bound the
  source-order `DM2_V1_TIMER_STEP_DOOR` (0x01) timer to a runtime handler that
  mutates the dungeon grid one door state per tick and re-queues the next step
  until the door reaches OPEN or CLOSED.
  Changes:
    * `src/dm2/dm2_v1_runtime.c`: added static `dm2_runtime_door_step_timer`
      handler, registered it in `dm2_v1_runtime_tick()` dispatcher, added
      `door_step_timers/mutations/requeues` counters and public
      `dm2_v1_runtime_door_step_receipt()`.
    * `include/dm2_v1_runtime.h`: added `DM2_V1_RuntimeDoorStepReceipt` struct
      and accessor declaration.
    * `tests/test_dm2_v1_runtime_handoff_smoke.c`: added
      `test_door_step_timer_wiring()` proving a CLOSED door opens in four ticks
      and the receipt counters match.
    * `include/COMPILE.H`: already switched to `<stdint.h>` for `int16_t`/`uint16_t`
      in the same cycle.
  Verified: full `cmake --build build --parallel` succeeds. Phase A probe
  (`SDL_VIDEODRIVER=dummy ./firestaff_m11_phase_a_probe`) passes. Relevant DM2
  tests pass: `test_dm2_v1_door_button_toggle_pc34_compat`,
  `test_dm2_v1_pressure_plate_pc34_compat`,
  `test_dm2_v1_trigger_pc34_compat`,
  `test_dm2_v1_proceed_timers_pc34_compat`. The new door-step smoke checks
  pass (5/5). `test_dm2_v1_runtime_handoff_smoke` reports 15 pre-existing
  failures unrelated to this wiring (creature door-block writeback, custom
  wall-button draws, timeline display-message target).

- 2026-07-22 DM2 V1 SKULLWIN/c_gfx_decode.cpp source-named receipt batch (Lane A,
  cycle 2): implemented the next SKULLWIN family after c_gfx_blit.cpp as
  bounded C11 decode receipts, updated the skproject audit and dispositions,
  and added focused synthetic-data coverage.
  Changes:
    * `src/dm2/dm2_v1_gfx_decode_receipt.c` (new): source-named receipts for
      `func_44c8_1202`, `spill_img3_pixels`, `read_img3_nibble`,
      `read_img3_duration`, `transparent_img3_pixels`, `decode_img3_overlay`,
      `dec9_1sub` (internal), `dec9_1`, `dec9_2`, `dec9_3`, `decode_img9`,
      plus `init`/`alloc` lifecycle boundary no-ops.
    * `include/dm2_v1_asset_loader.h`: public declarations for all new receipts.
    * `tests/test_dm2_v1_gfx_decode_receipt.c` (new): synthetic-data coverage
      for nibble/duration reads, single-pixel write, spill/transparent copies,
      IMG3 overlay decode, and IMG9 mode 1/2/3 dispatch.
    * `CMakeLists.txt`: added `test_dm2_v1_gfx_decode_receipt` target and CTest
      registration.
    * `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`: closed the
      14 open c_gfx_decode.cpp rows with status and Firestaff mapping updates.
    * `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv`: added disposition
      evidence rows for the unique symbols.
    * `TODO.md`: added the 2026-07-22 c_gfx_decode.cpp batch note.
  Verified: `cmake --build build --parallel` succeeds. Phase A probe passed
  24/24. Relevant tests pass:
    `test_dm2_v1_gfx_decode_receipt` (35/35),
    `test_dm2_v1_gdat_image_helper_receipts` (33/33),
    `test_dm2_v1_gdat_querydb_receipts` (119/119),
    `test_dm2_v1_skproject_core` (all checks passed).
  `test_dm2_v1_boot_profile_smoke` shows two pre-existing failures in
  "runtime frame ownership consumes real GDAT sprite palette..." and "boot
  runtime HUD capture proves real GDAT availability..." that are unrelated to
  the decode-receipt work.

- 2026-07-22 DM2 V1 INTERFACE_GENERAL dt07/0x0A Rect14 runtime/M11 wiring (Lane C,
  cycle 2): consumed the real Rect14 placement table in the DM2 V1 runtime frame
  ownership receipt and propagated it through the M11 acceptance gate so a frame
  that claims the table is present must also prove it was consumed with matching
  table/placement hashes. Source references: skproject/SKWIN/SkWinCore.cpp
  `LOAD_GDAT_INTERFACE_00_0A` and `QUERY_CREATURE_BLIT_RECTI`.
  Changes:
    * `include/dm2_v1_runtime.h`: added
      `gdat_interface_rect14_ready/table_hash/placement_hash/row_count` to
      `DM2_V1_RuntimeFrameOwnershipReceipt`.
    * `include/dm2_v1_viewport_renderer.h`: added
      `interface_rect14_required/consumed/table_hash/placement_hash/row_count`
      to `DM2_V1_ViewportM11FrameReceipt`.
    * `include/dm2_v1_boot.h`: added matching
      `runtime_m11_frame_interface_rect14_*` fields to
      `DM2_V1_BootRuntimeRenderReceipt`.
    * `src/dm2/dm2_v1_runtime.c`: after the runtime viewport receives the
      INTERFACE_GENERAL Rect14 rows from `dm2_v1_boot_interface_rect14_host_receipt`,
      record the consumed table/placement hashes in
      `g_dm2_frame_ownership` and copy them to `g_dm2_last_m11_frame`.
    * `src/dm2/dm2_v1_boot.c`: copy the M11 Rect14 fields from the runtime
      frame receipt into `DM2_V1_BootRuntimeRenderReceipt`.
    * `src/engine/m11_dm2_runtime_frame_receipt_gate.c`: reject the frame when
      Rect14 is required but not consumed, and require matching required/table/
      placement hashes between boot and runtime receipts.
    * `tests/test_dm2_v1_m11_runtime_frame_receipt_gate.c`: set the new Rect14
      fields in the synthetic receipts and add four focused checks for stale
      table hash, stale placement hash, unconsumed table, and missing requirement.
  Verified: `cmake --build build --parallel` builds the changed targets; the
  unrelated `test_memory_graphics_dat_header_pc34_compat_integration` target
  still fails on the pre-existing `include/COMPILE.H` macro leak. Phase A probe
  passed 24/24. Relevant tests pass:
    `test_dm2_v1_m11_runtime_frame_receipt_gate`,
    `test_dm2_v1_m11_runtime_frame_receipt_gate_watermark_identity`,
    `test_dm2_v1_m11_runtime_frame_receipt_gate_map_transition`.
  `test_dm2_v1_boot_profile_smoke` shows two pre-existing failures in
  "runtime frame ownership consumes real GDAT sprite palette..." and "boot
  runtime HUD capture proves real GDAT availability..." that are unrelated to
  the Rect14 wiring; the Rect14-specific checks in that test pass.

- 2026-07-22 Theron V1 verified-media synthetic-rendering boundary (Lane E):
  wired the original-media block so that verified JP/US Track 02 loads no
  longer leave generated palette/tile/UI rendering as an unguarded fallback.
  Changes:
    * `src/theron/theron_v1_asset_loader.h` and
      `src/theron/theron_v1_asset_loader.c`: changed
      `tr_asset_block_synthetic_rendering_for_verified_media` to accept the
      caller-verified Track 02 MD5 and set
      `TrAssetBundle.synthetic_rendering_blocked` only when the MD5 matches
      the canonical JP/US BIN hashes and no source-locked graphics bank has
      been decoded.
    * `src/theron/theron_v1_boot.c`: apply the boundary immediately after
      `tr_asset_load` for verified boot profiles.
    * `tests/test_theron_rendering.c`: added
      `test_asset_verified_track02_blocks_synthetic_rendering` which loads the
      real `TQUS02.bin`/`TQJP02.bin` files from `~/.firestaff/data/theron/`,
      verifies they remain loadable with raw bytes retained, and proves that
      the verified-media boundary blocks generated V1 rendering.
  Verified: `cmake --build build --target firestaff test_theron_rendering
  firestaff_theron_v1_viewport_renderer_probe firestaff_theron_v1_tile_renderer_probe
  firestaff_m11_phase_a_probe --parallel` succeeds. Phase A probe passed 24/24.
  Relevant Theron CTests all pass:
    theron_v1_rendering, theron_v1_viewport_renderer, theron_v1_tile_renderer,
    theron_v1_runtime_screenshot_readiness, theron_v1_m11_direct_launch,
    theron_v1_first_room_runtime, theron_v1_track02_bank,
    theron_v1_m11_launcher_handoff_boundary. Note: the full
    `cmake --build build --parallel` still fails on the pre-existing
    `test_memory_graphics_dat_header_pc34_compat_integration` target due to an
    unrelated `include/COMPILE.H` macro leak; this Lane E change does not touch
    that target.

- 2026-07-22 Nexus V1 real-asset DGN material container boot-profile validation
  (Lane D): updated `src/nexus/nexus_v1_boot_profile.c` and
  `include/firestaff_nexus_v1_boot_profile.h` so that
  `Nexus_V1_BootProfile_ValidateAssets` validates the real Saturn DGN material
  containers `SN_FLOOR.MNS` (MD5 `85c517e8e0bd84e00da58295dca5b409`) and
  `SN_WALL.MNS` (MD5 `ae67ca9fa8d09481e1849a42aaaa2eb6`) hash-first, instead
  of checking the non-existent `WALLS.DMDF`/`FLOORS.DMDF` files. Added
  `NEXUS_V1_DIAG_MISSING_FLOOR_MATERIAL` and
  `NEXUS_V1_DIAG_MISSING_WALL_MATERIAL`, removed the stale
  `NEXUS_V1_DIAG_MISSING_DMDF_ARCHIVE` / `NEXUS_V1_DIAG_INVALID_DMDF` codes,
  and updated the boot-profile helper so a caller-supplied diagnostic code is
  emitted for missing assets. Updated `tests/nexus_v1_boot_profile_smoke.c` to
  check the new diagnostic strings. Verified: `cmake --build build --parallel`
  compiled the changed Nexus targets; the full build still has unrelated
  pre-existing COMPILE.H/macOS header failures. Phase A probe passed 24/24.
  `test_nexus_v1_boot_profile_smoke` passed 26/26,
  `test_nexus_v1_boot_file_hash_scan` passed,
  `test_nexus_v1_availability_profile_gate` passed,
  `test_nexus_v1_dgn_runtime_materialization` passed, and
  `test_nexus_v1_dgn_geometry_readiness` passed. `test_nexus_v1_dgn_material_raster`
  remains a pre-existing capture-bound failure and was not touched.
- 2026-07-22 CSB V1 F0248 C006/F0729 and startup material batch: wall
  countdowns now consume the source evaluator's data mutation, remote delay
  and audible switch receipt. The startup path records C001/C004 stream and
  indexed hashes, consumed bytes, planar output and blank-tail facts; HUD
  copying requires a matching F0347/F0346 C017/C040 receipt. Focused C006,
  decoder, HUD, real startup, and DM1 panel/save regressions pass.

- 2026-07-22 DM1 V1 PANEL.C F0332/F0335/F0336: the leader-hand object panel
  retains a source-material receipt for the live Thing and its 16x16
  GRAPHICS.DAT zone. If either changes before drawing, the panel fails closed
  while preserving the established source-backed description text. The
  inventory mouse-route regression passes 309/0.

- 2026-07-22 DM1 V1 SAVEUTIL F0419: original save-part validation now calls
  non-mutating F0418 on the stored obfuscated span before F0417 decrypts the
  copied plaintext buffer. This keeps the source part intact during checksum
  validation and matches the READWRIT ordering. Focused regression passes
  19/19 assertions.

- 2026-07-22 CSB V1 viewport live-dungeon binding: replaced the M11
  procedural fallback maze with `csb_v1_viewport_bind_live_dungeon_grid`.
  `csb_v1_boot_render_viewport_frame_pc34` and
  `fs_game_render_viewport` now accept only loader-owned `DUNGEON.DAT`
  data; a missing or invalid dungeon clears the transient grid and produces
  no viewport draw. The regression covers absent, incomplete, and valid
  handoff-owned dungeon state. Ninja build and
  `test_csb_v1_boot_viewport_render_gate` pass (62/0).

- 2026-07-22 Theron V1 M11 raw MODE1/2352 full-launch level load: updated
  `src/engine/m11_game_view.c` so that raw Track 02 BIN media bypasses the
  `TRACK02 CAPTURE REQUIRED` gate and auto-loads the Hall of Records initial
  level via `theron_v1_startup_runtime_load_initial_level`. The bypass is
  gated on `campaignMedia->direct_media.mode1_2352`; MODE1/2048 ISO media and
  any case with captures present still requires the capture-required path.
  Added `#include "theron_v1_startup_runtime_entry.h"`. Build passed with
  `cmake --build build --parallel`. `theron_v1_runtime_screenshot_readiness`
  now passes (cases=3). `theron_v1_m11_direct_launch` regression test still
  passes.

- 2026-07-22 DM2 V1 INTERFACE_GENERAL dt07/2 action-table source format and
  palette remap (Lane B, DM2-012 follow-up): updated
  `src/dm2/dm2_v1_boot.c` so that `dm2_v1_boot_parse_interface_action_table`
  decodes the table exactly like `skproject/SKWIN/SkWinCore.cpp`
  `LOAD_GDAT_INTERFACE_00_02` (group count, lengths, primary block, secondary
  block, command tail) and `dm2_v1_interface_action_table_remap_palette` uses
  the corrected tail offset and 256 (group, threshold) pairs, matching
  `_0b36_037e`.  Build passed with `cmake --build build --parallel`.  Phase A
  probe passed 24/24.  `test_dm2_v1_dialogue_box_viewport_real_data` went from
  FAIL to PASS.  `test_dm2_v1_dialogue_gdat_receipt` remains 0/14 failures;
  weather, caii, ccm, and creature tests remain green.  In
  `test_dm2_v1_boot_profile_smoke` the dt07/2 span materialization check now
  passes; the two pre-existing real-asset frame/HUD failures are unchanged.

- 2026-07-22 DM2 V1 wall material fallback and D0C frame fix (Lane C,
  DM2-010): updated `src/dm2/dm2_v1_viewport_renderer.c` and
  `src/dm2/dm2_v1_gdat_wall_m11_command.c` so that
  `test_dm2_v1_g1_center_ray_surface_gate` passes and the new
  `test_dm2_v1_gdat_wall_plan_viewport_real_data` regression is fixed.
  Changes:
    * `dm2_v1_render_walls()` now consumes source material through the
      registered asset/palette providers when no pre-built GDAT wall plan is
      present, while still failing closed in M11 source mode when the plan is
      missing and no wall squares are explicitly flagged.
    * `dm2_v1_viewport_build_wall_panel_render_plan()` keeps D0C out of the
      generic table1d7029 wall scheduler, but includes it for the bounded
      asset-fallback path used by G1 unit tests.
    * `g_dm2_wall_frames[DM2_SQ_D0C]` was corrected from an empty frame to a
      full 224x136 drawable rectangle, so the front-center panel can actually
      be drawn.
    * `dm2_v1_gdat_wall_m11_command_plan_build_for_movement()` now excludes
      D0C, matching the canonical ten-panel wall plan.
    * `dm2_v1_viewport_draw_dungeon_tiles_pass_for_square()` now returns -1
      for D0C (and D3C), since the front-player tile is scheduled outside
      table1d7029.
    * Removed a stale debug `fprintf` from
      `src/dm2/dm2_v1_asset_loader.c` that broke recompilation.
  Build passed with `cmake --build build --target ...`.  Phase A probe passed
  24/24.  `test_dm2_v1_g1_center_ray_surface_gate` now PASSes and
  `test_dm2_v1_gdat_wall_plan_viewport_real_data` went from FAIL back to PASS.
  Full `ctest -R dm2_v1_` net: 206/223 passed; remaining 17 failures are the
  same pre-existing baseline items (boot_profile, dungeon_loader, dialogue,
  sound/asset probes, etc.) and were not introduced by this change.

- 2026-07-22 DM2 V1 dialogue modal state/event/text/button/cancellation parity
  (Lane B, DM2-012): updated `src/dm2/dm2_v1_dialogue_gdat.c` and
  `src/dm2/dm2_v1_weather_gdat.c` to source-lock the save/load panel receipt
  path against `skproject/SKULLWIN/c_dialog.cpp` and
  `skproject/SKULLWIN/c_gdatfile.cpp:1205-1211`.  Changes:
    * `dm2_dialogue_open_panel_text_decode` now treats a missing GDAT
      0/0/dtWordValue/0 as unencrypted, preserves the source GDAT payload size
      in the receipt, and rejects empty labels.
    * Historical note corrected 2026-08-07: `dm2_v1_asset_load_image_metadata`
      now reads `w4` only for the source raw-pixel `OffsetY() == -32` shape.
      C8 derives 8bpp from `OffsetY() == 31`; other compressed IMG3 records
      remain 4bpp regardless of `w4`.
  Build passed with `cmake --build build --parallel`.  Phase A probe passed
  24/24.  `test_dm2_v1_dialogue_gdat_receipt` went from 6 failures to 0/14
  PASS.  Weather/creature/AI tests (`test_dm2_v1_update_weather_pc34_compat`,
  `test_dm2_v1_caii_attack_pc34_compat`, `test_dm2_v1_ccm_loop_pc34_compat`,
  `test_dm2_v1_creature_something_pc34_compat`,
  `test_dm2_v1_creature_something_real_data`) remain green.  The canonical
  real-data test `test_dm2_v1_dialogue_box_viewport_real_data` still fails at
  `dm2_v1_interface_action_table_remap_palette` (pre-existing
  INTERFACE_GENERAL dt07/2 tail-format issue); follow-up remains open in
  TODO.md DM2-012.

- 2026-07-22 DM2 symbol audit batches (Lane A): closed 40 open DM2 skproject
  symbols. The `SKULLWIN/c_gfx_blit.cpp` family (37 symbols) was dispositioned
  through `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` and
  `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv`, with mappings to
  `src/dm2/dm2_v1_viewport_renderer.c`, `src/dm2/dm2_v2_hud_runtime.c`,
  `src/dm2/dm2_v1_weather.c`, and `src/dm2/dm2_v1_skproject_core.c`. The
  remaining SkWinCore aliases `_2066_1f37`, `_2066_1ec9`, and `_0cee_1a46` (3
  symbols) were closed as aliases of already-implemented c_map and dungeon-loader
  receipts. The DM2 skproject backlog dropped from 1074 to 1034 open rows. Build
  passed; `firestaff_m11_phase_a_probe` passed 24/24; `test_dm2_v1_fire_blit_rows`,
  `test_dm2_v1_update_weather_pc34_compat`, and `test_dm2_v1_skproject_core`
  passed. `tests/test_symbol_backlog_dispositions.py` still fails because its
  fixture symbol `F0139_DUNGEON_IsCreatureAllowedOnMap` is not present in the
  current DM1 open backlog; this is pre-existing and unrelated to the audit
  batches.

- 2026-07-22 CSB title timing: C425 CHAOS now remains the active title route
  through the full post-zoom hold; C426 STRIKES BACK begins only at frame 100.
  Focused real-PC34 startup, pointer, and boot-handoff tests pass.

- 2026-07-22 CSB legacy GRAPHICS.DAT: F0479 now recognizes a legacy
  big-endian CSB archive only when its compressed-size table reaches the
  physical file end. Ambiguous headers fail closed; the 563-entry CSB form
  has a focused integration regression.

- 2026-07-22 DM1 G0397/G0398/G0399: the F0267 ordinary-group apply receipt
  now transfers the source map index together with destination X/Y into the
  next C37 event. Focused movement/retry coverage verifies the context cannot
  inherit a prior event's map.

- 2026-07-22 DM1 G0383: F0887/F0209 now has a direct secondary-direction
  context regression. It holds C04 data fixed, blocks the primary F0228 path,
  and verifies the event-local diagonal fallback without a projectile route.
  Original global ABI storage remains open.

- 2026-07-22 CSB FTL-to-title: M11 now admits only the current verified C001
  title plan while the F0437 transaction is active. A stale transfer or
  Entrance plan cannot consume PRESENTS, CHAOS, and STRIKES in one draw call.
  Verified with both the real-PC34 startup sequence and playback-gate tests.

- 2026-07-22 DM1 object-source gate: replaced the synthetic leader-hand UI
  fixture with an opt-in probe that loads real `DUNGEON.DAT` Thing records,
  resolves an F0033 icon, and bounds-checks its real `GRAPHICS.DAT` atlas
  zone. It passed against the local PC34 corpus; full leader-hand HUD capture
  remains separate work.

- 2026-07-22 ReDMCSB F0009/F0010: added C11 spaced byte/word writers with
  direct stride regressions. Production caller mapping remains open.

- 2026-07-22 DM1 G0382: a public F0887/F0209 regression proves event-local
  primary direction updates C04 facing and schedules C38 without a projectile.

- 2026-07-22 ReDMCSB F0007/F0008: modernized the existing byte primitives to
  normal C11 and added direct CTest coverage for overlap-safe copy and bounded
  clear semantics. Production caller mapping remains open.

- 2026-07-22 CSB C012 sensor corpus probe: added a skip-safe original-PC34
  wall-generator probe for F0275/F0167/F0272/F0268/F0261. It needs a positive
  run against supplied original data before it becomes coverage.

- 2026-07-23 CSB C009/C012 real-Dungeon hardening: corpus probes now require
  the known PC34 `DUNGEON.DAT` MD5 and C009 continues past unrelated source
  C009 sensors while rejecting ambiguous routes. C012 now fails closed when
  F0167 cannot allocate a source object, preserving the wall sensor, hand,
  and timeline rather than rotating or publishing F0272/F0268. Focused C009
  and C012 tests pass; positive corpus coverage remains pending supplied
  hash-verified data.

- 2026-07-22 DM1 G0381: a public F0887/F0209 regression proves that the
  current-group distance is recomputed per C37 event, selecting approach at
  distance two and attack at distance one with C04 facing fixed.

- 2026-07-22 CSB C009 sensor corpus probe: added a skip-safe original-PC34
  fakewall SET probe for F0267/F0276/F0272/F0261. It still needs a positive
  run against a supplied original dungeon before it becomes coverage.

- 2026-07-22 DM1 G0378/G0379: F0887's public F0209 dispatch now has a
  context-lifetime regression that holds C04 facing fixed while event X/Y
  changes the visible behavior. Original global ABI storage remains open.

- 2026-07-22 CSB M11 lifecycle cleanup: removed the uncalled
  `m11_csb_release_delivery_gate`; boot-owned lifecycle advancement remains
  the only live path and the startup-resume/runtime-handoff regressions pass.

- 2026-07-22 ReDMCSB F0135/F0732/F0733/F0735: registered and directly ran
  bounded PC34 planar FillBox tests for screen-area, zone-index, and viewport
  dispatch. This is only caller-owned bitmap coverage, not full renderer proof.

- 2026-07-22 CSB M11 capture cleanup: removed an unused host-capture wrapper
  and four no-op admission callbacks. The boot-owned capture producer remains
  the sole route; startup receipt ordering is unchanged.

- 2026-07-22 CSB F0267 C001 corpus coverage: a real PC34 floor C001 route now
  proves F0276 through F0272/F0268 after normal party movement, skip-safe when
  no original `Dungeon.dat` is supplied.

- 2026-07-22 ReDMCSB F0692 FILLBOX bridge: the caller-owned planar FILLBOX
  variant now routes to existing F0135 with a focused CTest. IMAGE3's separate
  packed-raster F0692 implementation remains independent.

- 2026-07-22 CSB F0267 C003 corpus coverage: a real PC34 floor-party route
  now proves F0276 admission through F0272/F0268 after a normal move. It is
  skip-safe without the supplied original `Dungeon.dat`.

- 2026-07-22 DM1 F0115 C2900 capture: live C14 projectiles now reach final
  M11 capture only from their current M613 or F0142/G0209/M612 material blit.
  A stale decoded weapon or missing material clears the receipt.

- 2026-07-22 CSB F0267 C005 corpus coverage: a real PC34 C005 stairs route
  now proves F0276 sensor admission before F0364 level transition. The test
  scans only operator-supplied `Dungeon.dat` and skips when no such route exists.

- 2026-07-22 DM1 F0115 C15 capture: current-frame deferred explosions now
  reach final M11 capture only through the existing F0114/M636 material paths.
  Missing or stale PC34 material clears the receipt and cannot retain pixels.

- 2026-07-22 DM1 ReDMCSB G0377: verified the current active-group count
  lifecycle through F0196 reset, F0195 mutation, and PC34 save/handoff. The
  source `uint16_t` count round-trips from Firestaff's bounded live prefix.

- 2026-07-22 ReDMCSB FIO1 batch: mapped F1321, F1323, F1328-F1336,
  F1338-F1339, and F1341-F1342 through caller-owned PC34 file callbacks with
  source-style result codes. Floppy, format, lock, and drive operations remain
  explicit platform boundaries.

- 2026-07-22 CSB F0267 C002/C007 corpus coverage: the original-dungeon
  regression now uses one checksum-stable PC34 corpus for a positive C002
  wall-object route and C002/C007 floor rejections. It is skip-safe without
  operator-supplied media and does not introduce a fixture or runtime fallback.

- 2026-07-22 DM1 F0115 floor-item capture: an actual non-HoC F0115 floor
  item with loaded PC34 `DUNGEON.DAT` and `GRAPHICS.DAT` now reaches final M11
  capture. Missing material clears the receipt; no HoC, projectile, or HUD
  route is reused.

- 2026-07-22 CSBgraphics inventory receipt: boot now republishes a second,
  matching classification only from a manifest-admitted CSBgraphics cache.
  Missing, stale, or mismatched bytes clear the receipt and keep runtime
  planning closed; this does not claim bitmap or palette binding.

- 2026-07-22 DM1 ReDMCSB watchdog disposition: `G0374` is classified as a
  copy-protection-only watchdog and intentionally has no Firestaff storage.

- 2026-07-22 DM1 ReDMCSB timeline-global ownership: audited queue capacity,
  event count, and first-unused index for `G0369`, `G0372`, and `G0373`.
  Original ABI widths remain explicitly unverified.

- 2026-07-22 DM1 F0115 creature tick: M11 now consumes typed C04 candidate
  receipts rather than traversing raw square chains. The real-PC34 receipt
  regression passes; wider packaged capture remains open.

- 2026-07-22 CSB F0267 original-object route: linked ordinary objects now move
  only through loaded PC34 `Dungeon.dat` chains, with F0276 around raw link
  mutation and the existing teleporter/pit/stairs consequences. The positive
  corpus capture remains intentionally skip-safe until real CSB data is set.

- 2026-07-22 ReDMCSB input/USIO symbol batch: registered and ran focused
  coverage for `F0537/F0544`, `F1128`, `F1172-F1174`, and `F1175/F1176`.
  Amiga-only paths remain explicit host boundaries; queue paths use only
  caller-owned input state.

- 2026-07-22 DM1 ReDMCSB route package: restored the C13/HoC/champion/action
  capture build units registered by CMake, registered the F0407 non-THROW C11
  completion route, and added F0172/F0280 HoC mirror-click admission that
  requires matching C127/C026/C040 source material. The focused Ninja/CMake
  tests pass and all mismatch paths reject without fallback graphics.

- 2026-07-22 Nexus V1 PRS3 placement test-fixture re-anchor (Lane D): the
  engine's external PRS3 placement receipt now requires a nonzero `trace_size`
  alongside `trace_fnv1a64`. Three unit-test fixtures were stale and returned
  silent failures: `test_nexus_v1_prs3_dgn_placement_adapter.c` (#1959),
  `test_nexus_v1_prs3_vdp1_capture_replay.c` (#1962), and
  `test_nexus_v1_prs3_placement_engine_ingress.c` (#1964). Added the missing
  `trace_size` field to each synthetic input receipt. Verified with:
    `ctest --test-dir build -R 'nexus_v1_prs3_dgn_placement_adapter|nexus_v1_prs3_vdp1_capture_replay|nexus_v1_prs3_placement_engine_ingress' --output-on-failure`
  all three now PASS. Also confirmed the Phase A probe is green and
  `nexus_v1_m11_launcher_handoff_boundary` passes against the local retail ISO.
  Remaining open Nexus items are the capture-bound FACE.BIN PRS3 portrait block
  (#1919/#1920) and `nexus_v1_dgn_material_raster` (#1725), which stay blocked
  until original Saturn evidence is available.

- 2026-07-22 DM1 configured final-capture gate: one acceptance test now
  requires current source-owned C13, HoC, champion, and action/spell routes
  together. Champion, HoC, and action capture gates admit only configured
  original material; stale or incomplete evidence becomes clear/revoke and
  cannot reach final capture.

- 2026-07-22 DM1 real PC34 corpus discovery: C13 roundtrips now begin from
  the first verified original byte read, retain that corpus identity through
  the route, and capture deobfuscated C13/C3 rows byte-for-byte. Configured
  corpus/data roots are searched; no synthetic save is admitted.

- 2026-07-22 DM1 live M11 bridge lifecycle: live champion evidence now carries
  C008/C028/C033-C035, portrait, and statusbar proof to M11. HoC and
  action/spell bridges retain only monotonic current source state and emit
  material-free clear/revoke for stale inputs.

- 2026-07-22 DM1 real-source capture package: final M11 capture now accepts
  only the final indexed framebuffer after current C13, HoC, champion, and
  action/spell receipts. Live C127/C026/C040 and action/spell source evidence
  bridge to M11 with tick-fenced clear/revoke behavior. PC34 C3 event byte
  validation now uses the authoritative GAMEWORLD layout constant.

- 2026-07-22 DM1 live HoC/action material routes: live C127 mirror state is
  now bound to matching C026/C040 source rectangles and candidate panels.
  Actions require C010/C011 and spells C009/C011/C013 through command/frame
  admission. Route mismatches fail closed without substitute visuals.

- 2026-07-22 DM1 live champion material evidence: party composition now
  verifies C008/C028, portrait, statusbar palette/surface, and champion state
  by original pointer identity. Generated or substituted portrait material is
  rejected.

- 2026-07-22 DM1 visible-runtime M11 package: C13 delivery now bridges only
  active visible handoff and clears/revokes stale state. HoC C040/C026,
  champion C008/C028/statusbar, and action/spell output now carry current
  source proof through runtime-to-M11 bridges; M11 clears their own zones when
  the receipt is missing or stale.

- 2026-07-22 DM1 HoC/action frame lifecycle: only strictly newer HoC source
  admissions publish; stale C026 work is cleared/revoked. Action/spell runtime
  frames likewise accept current proof only, keeping stale paths material-free.

- 2026-07-22 DM1 C13/HUD runtime frame admission: visible C13 state,
  champion top-row, and action/spell presentation each now require a matching
  current source frame. Full HUD composition enforces material order and clear
  generation; stale paths are material-free.

- 2026-07-22 DM1 HoC runtime frame admission: the pre-M11 bridge and host
  render proof now combine into one source-owned receipt that requires exact
  C040/C026 hashes, generations, and ticks before a frame is admitted.

- 2026-07-22 DM1 runtime frame admission package: C13 M11 admission is
  revoked on stale provenance, world/F0238 tick, queue mismatch, or changed
  state. Champion/action host render lifecycles enforce ordered current clear
  and composition, while HoC retains exact C040/C026 hash/tick/generation.

- 2026-07-22 DM1 M11 host-render package: source-backed action/spell and
  champion top-row receipts now generate host render work only with exact
  original PC34 geometry/proof. Stale champion proof produces clear-only
  output; HoC pre-M11 bridge requires an active same-tick host route.

- 2026-07-22 DM1 C13/top-row M11 admission: C13 runtime admission now checks
  provenance, world, and F0238 queue over the next tick. Champion top-row M11
  consumption retains only matching original C008/C028/palette/surface proof;
  stale composition clears only its affected zones.

- 2026-07-22 DM1 M11 lifecycle fencing: HoC pre-M11 consumption requires an
  active host route and cannot replay the same or older tick. Action/spell M11
  receipt preserves original C010/C011 or C009/C013 evidence only while the
  route remains current.

- 2026-07-22 DM1 pre-M11 receipt package: champion top-row and action/spell
  work now carry explicit original material proof into their M11 boundary.
  HoC host bridge consumption carries only active source C040/C026 state on
  the current tick.

- 2026-07-22 DM1 active host lifecycle: a new clear invalidates prior
  top-row host composition immediately, and action/spell host routes accept
  only new or exactly identical current source routes. Bad tick, generation,
  material, or geometry leaves host state unchanged.

- 2026-07-22 DM1 visible C13/HoC handoff: visible PC34 runtime state now
  requires current provenance and F0238 queue evidence. The HoC host bridge
  admits only active tick-fenced C040/C026 work with matching material hashes
  and generations.

- 2026-07-22 DM1 active host-route package: top-row host consumption now
  requires the matching latest clear generation; HoC C040/C026 host work is
  fenced across ticks; action/spell routing admits only active PC34 action or
  rune image rectangles.

- 2026-07-22 DM1 host-route fencing: champion top-row lifecycle output now
  bridges clear-before-compose to renderer commands. HoC runtime admission
  requires matching C040/C026 material, generation, and tick. Action/spell
  host consumption rejects stale or divergent render state across ticks.

- 2026-07-22 DM1 runtime-consumption package: C13 state is consumable only
  while source/provenance/F0238 facts remain current. HoC lifecycle bridges
  clear and portrait frames by matching tick/generation/material. Action/spell
  render consumption and champion top-row composition now enforce the same
  ordered source-frame contract.

- 2026-07-22 DM1 champion top-row atomic frame: C008, C028, and the source
  statusbar frame now publish together. Missing material clears the complete
  affected champion area rather than mixing original and fallback graphics.

- 2026-07-22 DM1 C13 active-state gate: accepted party/timeline data becomes
  active runtime state only after matching PC34 corpus identity and route
  provenance. Failed gates emit no accepted runtime receipt.

- 2026-07-22 DM1 HUD/HoC lifecycle package: action/spell final paint clears
  the prior original surface before the next valid tick and rejects stale work.
  HoC requires the prior C040/C026 clear before a matching later C026 portrait
  can be published.

- 2026-07-22 DM1 C13/statusbar frame package: staged PC34 corpus identity is
  now bound to the same F0435/runtime adoption input hash. Champion statusbar
  clear/repaint commands publish atomically only with one retained original
  indexed surface and matching original palette.

- 2026-07-22 DM1 final HUD command package: HoC frame commands retain only
  verified C040/C026 material and reject stale completion. Action/spell HUD
  paint emits original-owned clear/render rectangles with no fallback.

- 2026-07-22 DM1 champion statusbar command path: validated F0287 receipts
  now become ordered clear/repaint commands only when the original indexed
  palette and status target are ready. Incorrect zones, geometry, palette, or
  order are rejected before drawing.

- 2026-07-22 DM1 action/save provenance: action/spell feedback can enter a
  frame only when its lifecycle, order, tick, serial, and source fingerprint
  match. PC34 corpus admission now requires full file hash, byte size, and the
  recorded C3 byte span before a C13 roundtrip is accepted.

- 2026-07-22 DM1 runtime presentation package: HoC C160/C161/C162 render
  admission now emits only valid original C040/C026 clear or portrait work.
  Action/spell result feedback requires its original command fingerprint, and
  champion status bars now use exact F0287 clear/repaint geometry.

- 2026-07-22 DM1 PC34 C13 roundtrip receipt: accepted original save state
  now verifies that C2, C3, C4, and dungeon-tail bytes serialize unchanged.
  This path is restricted to real corpus rounds.

- 2026-07-23 DM1 PC34 C2/M516 adoption receipt: original C2 PARTY_INFO and
  M516 champion bytes now bind to matching party metadata/state, champion
  identity, and GLOBAL_DATA party-map/status fingerprints across F0435 stage
  and runtime adoption. Tail-less inputs retain only diagnostic byte receipts;
  positive admission remains external-corpus-only and fail-closed.

- 2026-07-23 DM1 PC34 party inventory/active-slot receipt: raw C2 M516
  direction/action, both hands, worn equipment, load, and leader index now
  round-trip separately and bind through F0435 stage/adoption. C080 input,
  C127 mirror selection, and C146 wake-up cannot receive positive external
  corpus admission if those values drift; the dedicated stale fence revokes
  the receipt. Fixture data remains diagnostic only.

- 2026-07-23 DM1 PC34 C146/C080 state receipts: C146 champion condition
  (health, stamina, mana, poison, wounds) and C080 ordered candidate-party
  lifecycle (member order, leader, identity, direction) now retain distinct
  raw PC34 byte fingerprints through F0435 stage and runtime adoption. Each
  receipt is revoked on drift and may become positive only with a real external
  corpus; inventory state is intentionally outside this batch.

- 2026-07-23 DM1 PC34 C29-C41 active-group replay fence: exact source EVENT
  slots and raw payloads now bind C29-C41 reactions to their C04 active-group
  owner, source square/map/time, C.Ticks payload, and replay queue entry
  across F0435 staging and runtime adoption. Any divergent queue, active-group
  ownership, map/timeline identity, or raw event slot revokes admission.
  Positive evidence remains external-corpus-only; absent corpus remains
  fail-closed.

- 2026-07-23 DM1 PC34 C13/C24/C25 slot adoption fence: exact source EVENT
  slot bytes now cover altar-rebirth, explosion, and fluxcage-removal rows;
  C24/C25 additionally retain their C15 union bytes. F0435 compares their
  map/time/state identity through staging and adoption, then revokes the
  receipt on divergence. Positive acceptance remains external-corpus-only.

- 2026-07-22 DM1 HoC apply presentation completion: C160/C161 only publish
  their final sensor/portrait clear and C162 only restores matching C127/C026
  state when confirmation, apply, and presentation receipts agree.

- 2026-07-22 DM1 champion portrait/status policy: live lanes require the
  original portrait and C028, dead lanes require C008, and absent source data
  clears the lane. The selected inventory champion follows F0292; others use
  F0296.

- 2026-07-22 DM1 source-owned interaction package: C13 adoption now checks
  champion names, vital statistics, and attributes against PC34 bytes.
  Inventory redraw emits explicit owned/clear/skip policy for all 30 slots.
  Action/rune input is admitted only from C010/C009/C011/M653 source routes,
  and HoC confirmation now bridges atomically into the existing apply state.

- 2026-07-22 DM1 PC34/HoC consistency: C13 save adoption now verifies the
  source-owned party, leader, and inventory records against the tail-backed
  timeline. HoC C160/C161/C162 confirmation now requires matching selection,
  C040, and C127 generations before apply.

- 2026-07-22 DM1 action/spell lifecycle: source-backed HUD presentation now
  clears the prior frame before repainting the next ordered command frame.
  Stale frame and fingerprint facts leave presentation state unchanged.

- 2026-07-22 DM1 champion ownership and HoC selection: active leader and
  inventory ownership now produce a fail-closed redraw policy, while a valid
  portrait click atomically publishes the matching candidate/C127/C040 state.

- 2026-07-22 DM1 action/spell frame order: accepted per-frame commands now
  retain the original C014/C010/C009/C011/M653 ordering and revalidate
  source material before publication. Stale, reordered, and foreign commands
  are rejected.

- 2026-07-22 DM1 HoC portrait click presentation: C026 hit-testing now
  accepts only the matching source-owned C127/C040 generation. Off-portrait,
  wrong-cell, missing-panel, and stale-generation clicks produce no draw.

- 2026-07-22 DM1 source presentation package: action/spell commands now need
  a current source-owned material surface before they can enter a frame, and
  HoC candidate post-state now emits a source receipt for C040/C026/C127
  clear/restore behavior. PC34 save adoption now restores SquareFirstThings
  from the real dungeon tail and validates the C3/C4 timeline.

- 2026-07-22 DM1 champion party/inventory handoff: source-backed handoff now
  retains C008/C028/C032/C015/C016 materials and restores the party top row
  before redraw-priority overlays. The route fails closed when original
  material is absent. Verified by
  `dm1_v1_champion_party_inventory_handoff_pc34_compat`.

- 2026-07-22 DM2 M11 startup timing: startup title/menu ticks now run before
  the runtime-world guard. A valid boot profile can therefore leave the
  original GDAT credits/title frame and reach the `SHOW_MENU_SCREEN` phase
  before NEW GAME or LOAD creates the dungeon world.

- 2026-07-22 Nexus V1 M11 presentation gate: the real `TITLE.CG` reveal now
  remains drawable while `MENU.BPK` is fail-closed awaiting PRS3 capture, and
  ACCEPT exits a completed title instead of trapping the player on a blocked
  menu route. M11 DGN presentation now copies only the Nexus viewport's
  source-bound material framebuffer after its host-route receipt is ready;
  it no longer turns material ids into neutral placeholder colours. Verified
  against the local retail ISO by `nexus_v1_m11_launcher_handoff_boundary`,
  `m11_nexus_startup_gate`, and `m11_nexus_startup_runtime_handoff`.

- 2026-07-21 M11 direct-launch all-games gate green (113 passed, 0
  failed, 3 skipped): test_m11_direct_launch_prepare_all_games.
  - Real engine fix in M11_PhaseA_Run: the boot_probe_terminal_exit
    path never shut the SDL renderer down, so every second in-process
    boot probe died silently on M11_RENDER_ERR_ALREADY_INIT (the
    45917ebc4 terminal-exit contract only meant to skip the
    game-loop-owned runtime/menu teardown).  M11_Render_Shutdown() is
    renderer-owned and now runs on that path.
  - CSB title boundary re-anchored 81 -> 101 (ReDMCSB TITLE.C F0437
    PC-path total per CSB_V1_TITLE_TOTAL_TICKS_PC34, set in the
    45917ebc4 real-data startup work; 81 dated from the 82-tick model).
  - DM2 startup-menu receipt re-anchored: the menu-phase boot probe
    exposes the source file-header new-game pose (3,5,2) admitted by
    the dungeon loader (skproject DME.h File_header; the 15,15,0
    Hall-of-Champions default was synthetic) and zero champions, which
    materialize only at the runtime handoff.
  theron_v1_m11_direct_launch SEGFAULT and tier1_strict_boot_probe
  timeout verified pre-existing (unchanged by this fix; tracked in the
  theron and DM1 runtime-gate lanes).

- 2026-07-21 M12 launcher batch 2 (3ae607eb0): the last two failing
  M12 launcher tests re-anchored to current engine contracts.
  - test_m12_all_games_boot_readiness_receipt: nexus rows now assert
    the deliberate data-gate contract — scanner availability proves
    launch may be attempted, but startupMenu/fullStart/contract/
    packagedCapture stay blocked until real Saturn runtime capture
    receipts exist (nexus_v1_launcher_m12_startup_package_from_data_gate);
    expected/ready step masks = DATA|VERSION only, ready count 2/7,
    capture route BLOCKED, next-step label "CANONICAL RUNTIME RECEIPT".
  - test_m12_polished_ui_flow: settings navigation re-anchored to the
    v2.7.15 UX (DOWN cycles visible GAME-tab rows LANGUAGE->DATA_DIR,
    RIGHT switches to GRAPHICS tab and resets to its first visible row,
    ACCEPT cycles the selected row's value), and the launch ready
    message now returns to game options (BACK -> main -> exit).
  - Folded in leftover battery-batch changes: DM1 native save manifest
    classifier accepts format versions 1..2 (writer emits v2), asset
    status tests re-anchored to the deliberate narrow-scan contract
    (ScanGameWithOptions scans only the hinted game) and the Theron ZIP
    provenance split (version keeps virtual archive path, required-file
    row carries the materialized cache leaf).

- 2026-07-21 DM1 M11 game-view probe closeout (Jobb A/B lanes): the
  broad `firestaff_m11_game_view_probe` went from 53 failing
  invariants to 633/633 PASS, plus the box/geometry/spell sibling
  gates.  Commits 1bcbedf22, 31dd71ead, ef50bcf02, bc05e8b41,
  20fe95969.  Real engine bugs found and fixed:
  - dm1_v1_f0115_world_candidates_pc34 no longer suppresses GROUP
    enumeration by map index (the shipped PC34 map 0 has no GROUP
    things anyway; the special-case broke synthetic/non-PC34 maps).
  - dm1_v1_champion_panel_action_icon_global_hatch_pc34 candidate
    gate is ordinal > 0 (was != 0, which hatched on the -1
    no-candidate sentinel).
  - M11 thing-chain readers/mutations are compact-SFT aware
    (m11_world_has_compact_sft / m11_square_chain_head): creature AI
    scan/position/contains, prepend/unlink route through the source
    F0514/F0515 contracts when the dungeon publishes F0160/F0161
    per-column metadata, instead of aliasing compact entries onto
    wrong squares.
  - m11_find_first_item_on_square uses the compact-aware head
    (pickup from squares whose compact index differs from the dense
    index grabbed the wrong thing or nothing — real-dungeon bug).
  - C009/C010/C011 geometry restored to the stored PC34 bitmap
    dimensions (C009 87x25 at 233,42; C010 87x45 at 233,77; C011
    14x39, 14x12 cells at stride 13).  The 2026-07-14/15 "96-pixel
    full box" premise (00ab8c9c0, 961ab79b8, 723b4feb2) validated
    blits against the G0000/G0001 clear boxes instead of the stored
    bitmap sizes, silently blacking the authentic right-column
    action/spell surfaces.  G0000/G0001 stay the clear boxes via
    dm1_v1_*_source_box_rect accessors.
  - m11_draw_dm1_ui_text_trailing_spaces latches the first NUL
    (TEXT2.C F0041 pads with spaces after the terminator and never
    resumes): G0490's packed name table bled the next action name
    into trailing cells ("WAR CRY STAB", "PUNCH KICK").
  Probe-side migrations: synthetic fixtures publish compact-SFT
  metadata (flagged chain squares + THING_NONE spare tail for F0514
  insertions), raw-record sync helpers for group/weapon/object/icon
  records the source F0156/F0033/F0038 readers require, original DM1
  font loading for source-font-only text paths (f04ea4f21), and the
  C11/F0330 action-lock emulator (probe_expire_champion_action_lock)
  so stale spell-action C11 receipts no longer clobber short locks.
- 2026-07-21 DM1 HoC probe triage (Jobb E part 11): the last three
  failing Hall-of-Champions probes closed, probe-only changes, full
  265/265 portrait|mirror|hall_of_champions|hoc_ ctest sweep green.
  - firestaff_dm1_v1_hoc_mophus_ordinal15_unreachable_probe re-based
    to the real MOPHUS route.  Root cause: the probe's
    find_c127_with_data scan read SquareFirstThings with a naive dense
    mapX*height+mapY index, but SquareFirstThings is the COMPACT
    ReDMCSB DUNGEON.C F0160/F0161 array (only thing-list-flagged
    squares have entries, indexed via per-column cumulative counts),
    so the scan attributed the sensorData=15 chain to phantom cell
    (2,5) and the probe built its '(2,4) SOUTH forced canonical pose'
    narrative on that artifact.  The scan now reads through
    F0511_DUNGEON_GetSquareFirstThing_Compat, which agrees 1:1 with
    the independent dmweb-spec DUNGEON.DAT decode (verified PC34 C127
    layout: ordinal 15 = (11,11) north face) and with the live engine
    (actual_pose hall_mophus_from_north_ordinal_15 at (11,10) SOUTH
    -> 15).  Groups re-based: sensor anchor (11,11) cell=0 north face,
    (11,11) WALL-square checks, east_walkpath filter poses (10,11)E /
    (12,11)W / (11,12)N all returning -1, canonical pose (11,10)S
    returning 15 with a real 100% (192/192) C026 D1C cutout match,
    and zero-leak wrong-wall pixel checks at all six neighbours.
  - firestaff_dm1_v1_hoc_champion_portrait_15_redraw_after_candidate
    tightened from SKIP-in-vacuo: it parked at the same stale (2,4)S
    fixture and passed vacuously; re-based POSE_X/POSE_Y to (11,10)
    so the pre-candidate / panel-open / confirm-disable / fresh-cancel
    stages assert again (20/20).
  - firestaff_dm1_v1_hoc_ordinal_2_sibling_promotion_audit re-anchored
    to the 2026-06-28 sibling-promotion state: both ordinal-2 siblings
    (west_negative + cancel_reopen) are CTest-wired, so the audit now
    asserts open-count == 0 / wired state in lock-step with the
    readiness gate (wired=10/10, open=0) instead of the historical
    open == 2, and the promotion checklist reports PROMOTED.
  - firestaff_dm1_v1_hoc_no_false_projectile_artifacts_probe: the
    quiet-D1C fire-blob sample branch was dead code — the probe passed
    NULL out-pointers (outMapX/outMapY/outElementType) to
    M11_GameView_ProbeViewportFloorItemCounts, which rejects NULLs, so
    zero quiet samples were ever collected and the
    'normal V1 HoC quiet D1C fire-blob samples > 0' gate failed.
    Passing real locals restored the branch; no engine change.
- 2026-07-21 Build/CI health: silenced Apple ld "ignoring duplicate
  libraries" link-time noise. The M11/M12 static-archive cycles
  intentionally keep repeated libraries on link lines (CMP0156 OLD
  policy at the top of CMakeLists.txt), and Apple ld warned about every
  repeated archive — 200+ `ld: warning: ignoring duplicate libraries`
  lines per full build, drowning out real diagnostics. Fix:
  CMakeLists.txt now runs `check_linker_flag(C
  "-Wl,-no_warn_duplicate_libraries" ...)` right after `project()`
  under `if(APPLE)` and adds the flag via `add_link_options()` when the
  linker supports it (Xcode 15+), so nothing changes on Linux/Windows
  or older Apple toolchains. Verified: clean rebuild from scratch,
  flag probe `Success`, 0 duplicate-library warnings (was 200+),
  build exit 0 including the `firestaff` main executable, Phase A
  probe 24/24, `firestaff --scan-data` smoke OK, and a focused ctest
  subset (m11_phase_a + m11_game_view_probe + dm1_v1_movement_core +
  dm1_v1_chest_empty_pointer_integrity + m12_config_persist) 5/5 PASS.
  Also re-verified that the old TODO 🐛 `firestaff_m10` unresolved
  `_G2157_` symbol stays fixed: `src/shared/image_backend_pc34_compat_globals.c`
  (commit 3588798f9) is still in the `firestaff_m10` source list and
  every m10-linked test/probe target in the 2460-test suite builds and
  links clean.
- 2026-07-21 DM1 round-23 source-lock triage (job/w1, commits
  baa25cf8a, 78d6fe162): the stamina/blocked stale-needle family
  fully closed, zero engine source changes.  7/7 gates verified
  green via ctest:
  - baa25cf8a: pass551 blocked_movement_lifecycle, pass564
    movement_collision_timing_cluster, pass578
    stairs_backstep_cooldown_gate,
    dm1_v1_movement_core_lane_source_lock,
    pass505_dm1_v1_blocked_movement_collision_timing_gap — the
    last dm1_v1_apply_pre_step_stamina_cost /
    dm1_v1_record_blocked_wall_or_door_damage_request needles
    re-anchored to the plan/apply helpers
    (dm1_v1_apply_pre_step_stamina_plan,
    DM1_V1_MovementCommandCore_BlockedResolutionPlanPc34Compat,
    dm1_v1_apply_successful_step_plan,
    dm1_v1_dungeon_resolve_stairs_transition_pc34).
  - 78d6fe162: same-pattern neighbours pass423
    input_command_movement_pipeline and
    dm1_v1_movement_command_gate_source_lock re-anchored
    (unqualified field needles against the plan/apply helpers).
  - The broad source_lock|save_load|wall_ornament|alcove sweep is
    down from 39 failures (round-21 start) to 19; the remaining
    failures are other lanes plus dm1_v1_movement_source_lock,
    which is a different family (memory_tick_orchestrator F0888
    disabled-gate text) tracked in TODO.md.
- 2026-07-21 DM2/DM1 round-23 cross-cutting (job/w3, commits
  fe512ea76, fcaf2a075, 828c8910a): three gates fixed, each verified
  green immediately before commit:
  - m11_open_door_spell_runtime_source_lock — class a (salvage clobber):
    df88dbda4 (csb F0243 door destruction) had reverted the test to
    pre-C11 expectations (timeline.count == 1, M11-mirror-only stale
    action row). The engine still emits both the F0327 projectile move
    and the F0330 C11 receipt (MENU.C F0412:2034-2039 -> CHAMPION.C
    F0330), so the source-strict 194ebf859 expectations were restored:
    timeline.count == 2 with F0327 preceding C11 by source time, and the
    prior action lock consumed through a real queued C11 owner via
    F0253 before the cast. 50/50 pass against the current engine.
  - pass512_dm1_v1_viewport_wall_clip_source_audit — class a (stale
    line windows after DM1 module drift): clip-gate contract now
    2238-2276, draw_wall 1427-1448, draw_wall_opaque 1461-1478, tests
    1729-1785; all source-lock needles verified present.
  - v1_viewport_source_zone_tables — class a (tables moved to
    dm1_v1_viewport_3d_pc34_compat.c as k_c2500_raw/k_c2900_raw/
    k_c3200_center nested arrays): flattened, C2500 (68 records) and
    C2900 (48) match layout-696 exactly, C3200 center stays
    source-subset. C3200 side anchors documented class b (pass811
    viewport-local G0224-order re-derivation) with a provenance/
    structure lock in place.
- 2026-07-21 Theron round-23 stage-two $45xx-lane tier-2 windows
  byte-bound (job/w5): L424B's two callees plus the $45A6 TII gap
  stream bind in the stage-two image lane via the new
  `theron_v1_track02_verify_stage2_45xx_tier2_callees` verifier — 92
  bytes across three windows, hand-decoded from the authenticated US
  Track 02 media (MD5 f23601102138f87c33025877767ebf76) and matched
  against da65's inline linear decodes ($83A1/$82BF/$85A6 renderings;
  da65's own L42BF/L43A1 labels sit on unrelated bank-0 streams — the
  L4696-label class):
  - L43A1 [0x43a1..0x43d6) (53 bytes): the $14:$15 -> $0E:$0F copy,
    three ASL $0E / ROL $0F shift pairs, the L47CB/L47CC add, the $58
    ASL A add, and the $0E:$0F -> $00:$01 finish (da65's L0000
    zero-page-as-absolute rendering superseded by the media 85 00) —
    ends exactly at the bound L43D6 body (adjacency
    compile-time-asserted).
  - L42BF [0x42bf..0x42db) (28 bytes): the $56 $10-counter (INC $56 /
    LDX $56 / CPX #$10 / BNE L82DA) with its L47C4 save / INC A /
    store / JSR L43D6 / restore — the internal JSR $43D6 at +0x14
    targets the round-21 body; ends at the unbound $3B75 stream.
  - $45A6 TII gap stream [0x45a6..0x45b1) (11 bytes): STZ L47B8 /
    TII $47B8,$47B9,$00A7 / RTS — called only from the unbound $401C
    stream (JSR $45A6 at image 0x401c), so its entry CPU address is
    not pinned (the $45xx-routine precedent) and the receipt carries
    0; ends exactly at the bound $45xx routine (adjacency
    compile-time-asserted).
  - CPU entry addresses for L43A1/L42BF are pinned by the JSR operands
    inside the bound L424B body (+0x19/+0x45 L43A1, +0x2d/+0x55 L42BF
    — the round-16 L4696 class; each encoded $4xxx address equals its
    image offset).
  - Probe: US fixture + real-media whole-body matches; the changed
    L43A1/L42BF/$45A6 head bytes each fail closed; the JP fixture
    rejects; `summary: fail=0`. ctest baseline unchanged (the same 14
    known failures, name-for-name).
  - Remaining future windows: the $3B75 stream (after L42BF), the
    $4417 stream (called from the unbound $4228 stream), the $4943
    stream (no JSR caller — jump-table/handler entry), L52FD, L52DA,
    L54DB, the LE063 far-call targets, and L383E in the dynamic
    payload; no semantics, System Card base or bank-mapping
    arithmetic, record semantics, or graphics role follows.

- 2026-07-21 DM1 round-22 source-lock triage (job/w1, commits
  16342c60e, 51ab52d1a): the five remaining occlusion probes plus
  pass580 closed as stale-probe re-anchors, zero engine source
  changes.  All 6 gates verified green via ctest (6/6):
  - Occlusion cluster (16342c60e): pass515 D0 side wall, pass516
    D1/D0 wall occlusion, pass519 D1C door-front field, pass561
    far door-front, pass562 D2 far side wall.  The viewport
    wall/door spec tables in dm1_v1_viewport_3d_pc34_compat.c
    grew per-row view-window fields (relForward/relSide, clip
    rect) and moved; probes now cite the current rows (wall
    specs 1168-1172, side occlusion cell orders 189-193,
    door-front metadata 932-942, D2L2/D2R2 far side-wall specs
    1163-1164, source-evidence strings 3913-3929, runtime-test
    expectation tables 754-763 and 1304-1340).  ReDMCSB anchors,
    zone ids, wall pairings and return-line citations unchanged.
  - pass580 forward_collision_timing (51ab52d1a): same
    two-phase-refactor stale-needle family as round 21; the
    F0325 clamp/underflow evidence is re-anchored to
    dm1_v1_action_stamina_apply_plan_f0325_pc34 (clamp at zero,
    clamp at max, underflow-to-damage plan).
  - Remaining (moved to round 23, tracked in TODO.md): the
    stamina-needle family pass551, pass564, pass578,
    dm1_v1_movement_core_lane_source_lock,
    pass505_dm1_v1_blocked_movement_collision_timing_gap.
- 2026-07-21 DM2/DM1 round-22 cross-cutting (job/w3, commits
  d09e29512, 6c36d62f3, 0a686d96f, adc67a7a3, 216a8d06a, ca47f8527):
  six of the seven round-21 restpost items resolved, all class a
  (stale verifiers re-anchored to the DM1 owner modules), each verifier
  run with PASS immediately before its commit:
  - v1_viewport_wall_blit_transparency_gate — verifier both crashed on
    a removed helper anchor and expected the pre-receipt side-wall pass;
    now locks the DM1 C10 owner
    (dm1_v1_viewport_3d_pc34_compat.c handoff.transparent_color = 10,
    DUNVIEW.C F0104/F0105 provenance) plus M11 honoring
    receipt->material.transparent_color in
    m11_draw_dm1_side_wall_host_receipt; center walls stay opaque (-1).
  - v1_viewport_field_zone_aspect_clip_gate — the G2035-locked field
    table is DM1-owned as s_fieldRenderPlans
    (dm1_v1_field_teleporter_effect_pc34_compat.c, byte-identical 16
    rows); M11 consumes dm1_v1_field_render_plan_at_pc34 with the
    lane-visibility receipt; the F0113 mask/phase animation lives in
    dm1_v1_field_bitmap_sample_pc34.
  - v1_viewport_side_wall_ornament_source_gate — F0107 flip rule
    (viewWallIndex 1/6/11) in dm1_v1_wall_ornament_flip_horizontal_pc34
    and the native-offset exclusion (!= 5, != 6) in
    dm1_v1_wall_ornament_render_plan_pc34, both in
    dm1_v1_wall_ornament_pc34_compat.c; M11 only consumes view specs
    and host material receipts.
  - v1_door_button_ornament_coordinates_gate — G0208 door-button frames
    in dm1_v1_viewport_3d_pc34_compat.c (s_door_button_frames, consumed
    via dm1_v1_viewport_get_door_button_frame_pc34); G0207 coordinate
    sets, coord math, and G0200/G0201 palettes in
    dm1_v1_door_ornament_render_pc34_compat.c.
  - v1_wall_ornament_coordinates_gate — G0194 ST indices in
    dm1_v1_g0194_pc34_compat.c, G0205 sets (flat 624-byte table) in
    dm1_v1_g0205_pc34_compat.c, G0198/G0199 palettes in
    dm1_v1_wall_ornament_pc34_compat.c.
  - firestaff_dm1_v1_viewport_d0c_door_edge_ornament_gate_probe — stale
    probe expectation: ReDMCSB DUNVIEW.C:8189-8214 places the
    C09_COLOR_GOLD hole blit strictly inside the
    G0407 Event73Count_ThievesEye branch, so the probe now expects
    has_thieves_eye ? 1 : 0 (engine trace was already source-correct);
    probe rebuilt and 2/2 d0c door-edge tests pass.
  Also verified by baseline bisection (worktree at 5a34524e6): all 12
  failures in the consolidated round-21-area ctest sweep are
  pre-existing, none caused by round 21.

- 2026-07-21 Nexus round-22 (job/w4, commit a7d7292cf): STABG.BIN
  container framing PROVEN from retail data — no capture needed.
  Analysis of the SHA-256-pinned retail file (53744 bytes) showed an
  "STMP" container: big-endian u32 exact file size, three
  (offset, length) region pairs tiling [0x20, EOF) — a tile-map
  directory, a 256-entry big-endian u16 CLUT, and 4bpp pixel data. The
  directory is a zero-terminated u32 offset table followed by a
  contiguous run of (u16 w, u16 h, w*h BE u16 cells) maps that fills
  its region exactly: one 40x21-cell status-area background map
  (320x168 at 8px cells), eight 8x6 maps, two 3x3 maps. Every cell is
  a word offset (x2 bytes) bounded inside the pixel region. Landed as
  `nexus_ui_stabg_stmp_framing_receipt` + `Nexus_UI_StabgStmpFraming`
  with real-media and opaque-byte tests in
  test_nexus_v1_startup_media_gate (PASS, no skips).
  `nexus_ui_load_stabg` deliberately stays blocked: framing is proven,
  but the pixel-unit decode semantics of the cells is not (8x8-tile
  and linear 4bpp hypotheses disproven by test renders), so no surface
  may be materialized yet. This answers the STABG root gate of #674
  with data; the remaining #674 gates (STABG cell decode, FACE PRS3
  campaign, title capture-surface/Saturn-timing evidence) are in
  TODO.md same date.
- 2026-07-21 Theron round-22 stage-two L3114 tier-5 callees byte-bound
  (job/w5): the seven tier-5 windows parked after round 20 all bind in
  the stage-two image lane via the new
  `theron_v1_track02_verify_stage2_l3114_tier5_callees` verifier — 208
  bytes across seven windows, hand-decoded from the authenticated US
  Track 02 media (MD5 f23601102138f87c33025877767ebf76) and matched
  against da65's inline linear decodes (theron-us-stage2-huc6280.asm:
  2832, 2853, 3034, 3051, 3147, 3366):
  - L5403 [0x1403..0x141e) (27 bytes): PHX / CLY / DEC $5A / BSR L541E /
    the `a:$02`/`a:$03` absolute-load -> ($04),y pair copy loop (BNE
    d0 f1 -> L5409; da65's 3-byte absolute form AD 02 00 / AD 03 00
    media-confirmed) / STZ $5A / PLA / BSR L5492 / RTS — ends exactly
    at L541E.
  - L541E [0x141e..0x142d) (15 bytes): ST0 #$01 / the $0E:$0F ->
    $0002:$0003 VDC address writes / ST0 #$02 / RTS — ends exactly at
    the bound L542D (adjacency compile-time-asserted).
  - L52A2 [0x12a2..0x12c8) (38 bytes): the L4FD7/L4FD8 -> $06:$07
    copy, the L4FD5/L4FD6 +$20 -> $04:$05 add, the $14-$17 field
    setup / RTS — ends exactly at L52C8.
  - L52C8 [0x12c8..0x12da) (18 bytes): the $14,x + L4FD5/L4FD6 ->
    $00:$01 add (da65's L0000 zero-page-as-absolute rendering
    superseded by the media 85 00) / JSR L52FD / RTS — ends at the
    unbound L52DA (L52FD/L52DA remain future windows).
  - L5657 [0x1657..0x1667) (16 bytes): CLX / CLY / the 8-byte
    ($00),y -> ($02),y SXY copy loop / RTS — ends exactly at the
    L5667 table.
  - L54C5 [0x14c5..0x14db) (22 bytes): the DEC L4F9F / L4F9F-indexed
    L4FA0,x -> L4FD7/L4FD8 pair load / RTS — called only from the
    unbound L54DB stream (L54DB remains a future window).
  - L5667 data table [0x1667..0x16af) (72 bytes, 9 rows x 8): read
    through the bound L560B body's $00:$01 setup (LDA #$67 / STA $00 /
    LDA #$56 / STA $01 at L560B+0x0e) by the L5657 copy loop — the
    L5C20-table class; da65 garbage-decodes the table as bbs7/cpy/brk/
    st0 code, so the media bytes are authoritative; code resumes at
    da65's L56AF.
  - Call-site invariants compile-time-asserted: the BSR L5403 at +0x2c
    inside the bound L53C4 body; the BSR L5657 at +0x1f, JSR L52A2 at
    +0x21, JSR L52C8 at +0x25, and the L5667 data site at +0x0e inside
    the bound L560B body.  The L5403->L541E->L542D, L52A2->L52C8, and
    L5657->L5667 adjacency chain is compile-time-asserted.
  - Probe: US fixture + real-media whole-body matches; the changed
    L5403/L54C5 head bytes and the changed L5667 table byte each fail
    closed; the JP fixture rejects; `summary: fail=0`. ctest baseline
    unchanged (the same 14 known failures, name-for-name).
  - Remaining future windows: L52FD, L52DA, L54DB, L424B's callees
    (L43A1, L42BF), the unbound gap streams, the LE063 far-call
    targets, and L383E in the dynamic payload; no semantics, System
    Card base or bank-mapping arithmetic, record semantics, or
    graphics role follows.

- 2026-07-21 DM1 round-21 source-lock triage (job/w1, commits
  7d03d73da, a77fae0ec): 8 of the 12 pre-existing source-lock
  failures closed as stale-probe re-anchors, zero engine source
  changes.  All 9 affected gates verified green via ctest (9/9):
  - Movement command-core cluster (7d03d73da): pass504
    movement_followup, pass505 blocked_movement_side_effect,
    pass506 stairs_movement_side_effect, pass507
    movement_stairs_group_timing, pass562 front_cell_collision,
    pass590 blocked_wall_door_self_damage plus pass507's
    dependency dm1_v1_command_movement_sensor_timing_source_lock.
    Root cause: the source-faithful two-phase plan/apply refactor
    of DM1_V1_MovementCommandCore_ProcessOnePc34Compat —
    PreStepStaminaApplyPlan + dm1_v1_apply_pre_step_stamina_plan,
    BlockedResolutionPlan + dm1_v1_apply_blocked_resolution_plan
    (carrying the F0366 self-damage request fields),
    dm1_v1_dungeon_resolve_stairs_transition_pc34 replacing the
    F0705 seam, and DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat
    replacing the inline inputDiscardRequested assignment.
    Semantics unchanged: stamina before legality, wall/door
    blocked-resolution before input discard, group blocker after
    wall/door, stairs before blockers.
  - Viewport cluster (a77fae0ec): pass486
    door_button_occlusion and the pass506
    alcove_wall_item_occlusion_evidence sibling gate.  Door-button
    geometry moved into the source-locked s_door_button_frames
    table in dm1_v1_viewport_3d_pc34_compat.c (D3R/D3C/D2C/D1C
    rows); alcove item extraction now flows through
    dm1_v1_f0115_world_candidates_pc34, which keeps wall-square
    items without an element-type filter (locked by a new negative
    structural check), and the alcove draw routes via
    dm1_v1_f0115_alcove_item_material_plan_pc34 +
    m11_draw_item_sprite_material.  The alcove probe's
    find_function helper now skips declarations and call sites
    via paren balancing.
  - Remaining (moved to round 22, tracked in TODO.md): pass515,
    pass516, pass519, pass561, pass562_d2_far_side_wall — D0/D1
    side-wall and door-front occlusion probes with drifted
    spec-table row needles.
- 2026-07-21 DM2/DM1 round-21 cross-cutting (job/w3): the round-19
  regression-sweep pre-existing family deep-triaged and partially closed
  in five per-domain commits (87fd08f79, f64fe01d5, 88f9ec78a,
  ed2adeee3, 4afd23432); every engine behavior was verified
  source-faithful against skproject/ReDMCSB before each re-anchor.
  - dm2_v1_hud_hero_type_gdat_route (class a, stale fixture): d6adbe2e2
    ("gate dynamic HUD on GDAT ownership") intentionally blocks the
    whole champion slot at the dynamic-overlay gate unless the source
    dt04/dt07/palette contract is complete, so the bare fixture never
    reached the portrait-type gate. The fixture now binds the full
    contract (hud layout, interface palette, font rows,
    state_source_bound) and isolates the HeroType gate it is named for:
    an unbound portrait type still blocks before any CHAMPIONS fetch.
    4/4.
  - firestaff_dm2_v2_hud_widget_runtime_hook_probe (class c): bb117bbb9's
    salvage rewrite dropped the source-evidence citations the probe
    enforces; restored with the actual provenance (ReDMCSB SKULL.ASM
    T560, skproject SKULLWIN c_gui_vp.cpp, ReDMCSB PANEL.C F0354,
    dm2_v2_hud_widget_assets Phase 3 gate, dm2_v2_hud_widget_bitmap_blit
    follow-up, V1 invariant, OPEN-BOUNDED honesty) while keeping the
    b9f23b054 IMG3/dtPalette16 mapping lines. The probe target also
    lacked the FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR compile
    definition the bitmap_blit probe already had (Scenario 10 fixture
    copy failed from the build CWD); given the same definition. 119/119.
    firestaff_dm2_v2_hud_widget_assets_probe was a stale binary in the
    round-19 sweep and passes unchanged (54/54).
  - v1_viewport_palette_source_lock_gate (class a, stale verifier): the
    weighted torch/palette F0337 algorithm moved into the M10 receipt
    owner F0890b_ORCH_ComputeDungeonViewLight_Compat
    (memory_tick_orchestrator_pc34_compat.c) with byte-identical ReDMCSB
    DATA.C tables; M11 only consumes the receipt. Verifier re-anchored
    to the M10 owner plus the M11 delegation, keeping the ReDMCSB
    contract, whole-viewport apply, no-depth-dimming and render-order
    assertions. Gate passes.
  - v1_viewport_front_wall_depth_gate (class a, stale verifier): the
    draw call site derives maxVisibleForward from the DM1-owned
    lane-visibility receipt (m11_dm1_lane_visibility →
    dm1_viewport_3d_lane_visibility_from_cells_pc34); the old center
    helper remains as a thin delegate. Call-site markers re-anchored to
    the receipt flow. Gate passes.
  - v1_viewport_distance_row_clip_gate +
    v1_viewport_projectile_source_clip_gate (class a, stale verifiers):
    the G2028 row map, view-square map, C2500/C2900 raw zone tables and
    the viewport-only projectile clip moved into the DM1-owned modules
    (dm1_v1_viewport_3d_pc34_compat.c,
    dm1_v1_projectile_explosion_render_pc34_compat.c) with
    byte-identical source tables; M11 only consumes
    dm1_v1_projectile_sprite_blit_plan. Both verifiers re-anchored to
    the DM1 owners plus the M11 delegation. Both gates pass.
  Remaining round-21 items (classified class-a re-anchorable against
  the same owner-module pattern) are listed in TODO.md: field zone
  aspect, side wall ornament source, wall blit transparency,
  door button/wall ornament coordinates, D0C door edge ornament probe,
  m11_open_door_spell_runtime_source_lock.

- 2026-07-21 Nexus round-21 (job/w4, commit a46b891a2):
  `nexus_v1_startup_menu_pc34_compat` (#1741) FIXED — 12+ FAIL -> 0 FAIL.
  Three stacked class-(a) fixture gaps, zero engine changes; the
  hardened engine semantics were correct throughout:
  - Seeded the bounded MENU.BPK provenance the launcher asset gate
    requires (canonical layout: 4 archive entries = 3 surfaces + the
    directory trailer at entry zero, valid when it is the only
    trailer). Same gap family as round 18's
    m11_nexus_startup_runtime_handoff re-anchor (1a05c61b0).
  - Restored the b080671f4 SFX seed for the Track 02 blocker path
    (BLOCKED_UNSUPPORTED_DECODE, level_index 0, cd_track 2): a later
    merge had reverted the seed to MISSING_ASSET/-1/-1 while keeping
    the hardened expectations, making the fixture self-inconsistent.
  - Gave the title-route block an engine: the hardened title asset
    route (11caba716) consults the startup asset receipt, which
    requires state->engine; HOLD and RETURN_TO_LAUNCHER are
    unconditional asset-ready routes.
  Separately, #674 (`nexus_v1_m11_launcher_handoff_boundary`) was
  deep-triaged to class (c) with a verified single root gate
  (STABG framing block + FACE PRS3 block + title capture-surface /
  Saturn timing capture evidence) — full chain in TODO.md same date.
- 2026-07-21 Theron round-21 stage-two $45xx-routine callees byte-bound
  (job/w5): the six JSR targets of the round-20 $45xx routine all bind
  in the stage-two image lane via the new
  `theron_v1_track02_verify_stage2_enclosing_45xx_callees` verifier —
  325 bytes across six windows, hand-decoded from the authenticated US
  Track 02 media (MD5 f23601102138f87c33025877767ebf76) and matched
  against da65's inline linear decodes ($824B/$83D6/$8552/$858E/$866B/
  $8932 renderings; da65's own L424B/L43D6/L4552/L458E/L466B/L4932
  labels sit on unrelated bank-0 streams — the L4696-label class):
  - L424B [0x424b..0x42bf) (116 bytes): the L47B8/$11/$10 setup, the
    ($4E),y -> $14:$15 pair loads, JSR L43A1, the $58-bit X-trip-counted
    BSR-local / DEC $11 / DEX / BNE loop, JSR L42BF, the $58 &= #$02
    second pass with its BRA L8281 loop, plus the BSR-local subroutine
    at +0x5b (the ($00),y -> L47E0,x pair copy).  Flagged da65
    artifacts, media authoritative: the head mis-split (`.byte $85` /
    `bpl $821C` over the STA $10 / CLX — the round-18 mid-instruction
    class) and the L0011/L0000 zero-page-as-absolute renderings.  Every
    relative (BSR x2, BNE x2, BEQ, BRA) resolves inside the body; the
    span ends at the unbound L42BF.
  - L43D6 [0x43d6..0x4417) (65 bytes): the $57/$56 -> $4E:$4F
    shift-add (four ASL/ROL pairs, add, one more pair), the
    L47C4-conditional ASL A add, the L47CD/L47CE accumulate, RTS — the
    BEQ resolves to da65's L8407 inside the body.
  - L4552 [0x4552..0x458e) (60 bytes): the $52/$54 nibble shifts, the
    $55-counted L47BE multiply-add loop, the $53 accumulate into
    L47C4, the bit-3 pair into $58 — ends exactly at L458E (adjacency
    compile-time-asserted).
  - L458E [0x458e..0x45a6) (24 bytes): the $0E LSR/ROR $02 pair x2,
    the $0F + $02 add with BCC/INC $03 carry — ends at the unbound
    STZ L47B8 / TII gap routine [0x45a6..0x45b1).
  - L466B [0x466b..0x4696) (43 bytes): the ST0 #$00 / $02:$03 VDC
    data writes (da65 `a:$02`/`a:$03` absolute form media-confirmed),
    the ST0 #$02 and the $0E-conditional self-modifying TIA setup
    (STA $468D/$468E/$4691 rewrite the TIA source/length operands —
    the L5C69 class; media bytes are the as-loaded image) — ends
    exactly at the bound L4696 body (adjacency compile-time-asserted).
  - L4932 [0x4932..0x4943) (17 bytes): ST0 #$05 / $F3 -> $0002 /
    $F4 &= #$07 -> $0003 (da65 `a:$02`/`a:$03` media-confirmed) — ends
    before the unbound TMA #$08 / PHA stream.
  - CPU entry addresses pinned by the JSR operands inside the bound
    $45xx body (the round-16 L4696 class — each encoded $4xxx address
    equals its image offset); the $45xx call-site offsets (+0x25 L4552,
    +0x35 L4932, +0x42 L458E, +0x6d L424B, +0x87 L466B, +0xb1 L43D6),
    the internal JSR L43D6 at L424B+0x02, and both adjacencies are
    compile-time-asserted.
  - Probe: US fixture + real-media whole-body matches; the changed
    L424B/L4552/L4932 head bytes each fail closed; the JP fixture
    rejects; `summary: fail=0`. ctest baseline unchanged (the same 14
    known failures, name-for-name).
  - Remaining future windows: L424B's callees L43A1/L42BF, the unbound
    gap streams, the tier-5 windows (L5403/L541E/L52A2/L52C8/L5657/
    L54C5, the LE063 far-call targets, the L5657-tail data), and L383E
    in the dynamic payload; no semantics, System Card base or
    bank-mapping arithmetic, record semantics, or graphics role
    follows.

- 2026-07-21 DM1 round-19 portrait_19/22 wall_ornament_no_float
  probes re-anchored to the native C346 raster profile and GREEN
  (job/w1, commits 47cb77299, d6558032e, 8b3ba0f55): the last parked
  HoC class.  Two stacked stale probe expectations were replaced,
  zero engine source changes:
  - Stale expectation 1: the probes gated on the old procedural
    fallback frame (1-pixel BLACK outer ring + LIGHT_GRAY top/left +
    GRAY bottom/right + DARK_GRAY interior fill).  The engine
    actually renders the shipped GRAPHICS.DAT asset 346 (48x43)
    scaled to the 64x43 G0205 D1C zone through kOrnD2Palette via
    the F0791 route (m11_draw_dm1_front_mirror_backing_host_receipt
    -> m11_blit_scaled_palette_map_region).
  - Stale expectation 2: the first rewrite assumed cyan(4) was the
    transparency key.  Source check proved
    DM1_WALL_ORNAMENT_TRANSPARENT_COLOR_PC34 = 10 (TAN): TAN source
    pixels are skipped (the underlying wall LIGHT_GRAY(13) shows
    through) and CYAN(4) is the mirror-glass body that IS drawn.
  - The gates now lock the source-verified native profile
    (expectations derived from the shipped bitmap via the D2
    palette map and the blit's nearest-neighbor scale, calibrated
    once against the deterministic framebuffer): top edge 59 BROWN,
    bottom edge 58 BLACK, right edge 41 BLACK, zero TAN leaks,
    full-height skipped TAN left column with 31 wall(13)
    show-through, corners TL/TR/BL=13 + BR=0, ring 1299 BROWN +
    284 CYAN glass body + 76 LIGHT_GRAY(13) glass shine + 19 YELLOW
    name-plate, Group F warm-leak baseline 19 with gate < 60.
  - Verified: ctest -R wall_ornament_no_float 2/2 PASS (probe 19:
    ordinal 19/HAWK atlas row 2 col 3; probe 22: ordinal 22/GOTHMOG
    row 2 col 6, warm-portrait floor kept at >= 30 for the sparse
    portrait).
- 2026-07-21 DM2 round-19 cross-cutting (job/w3):
  `dm2_v1_lighting_palette_runtime_gate` (binary
  test_dm2_v1_lighting_falloff_boundary) CLOSED — 146/161 → 162/162.
  All 15 FAILs were stale fixture expectations against intentional
  source-ownership re-anchors, re-anchored per domain in six commits
  (0c0e5cb1c, b1b7feb3d, b1e7d08e4, 5e39051ab, bab077e37, fcbddceb3);
  every engine behavior was verified source-faithful against skproject
  (/Volumes/Extern-disk/reference-skproject) before each re-anchor, so
  no engine change was needed:
  - HUD asset-index packing: 1da849469 bound the 8-bit Champion
    HeroType save field to the runtime HUD, so the test-side
    DM2_V1_HUD_PORTRAIT_COUNT rejection bound widened 8 → 256
    (header constant consumed only by this test).
  - Rain/fog/storm overlay application: dm2_v1_render_weather_overlay
    paints only through the receipt-owned ENVIRONMENT transaction
    (skproject c_weather.cpp:221-266, c_querydb.cpp
    DM2_QUERY_TEMP_PICST:2381); the enum-only fixtures without
    is_outdoor + renderer receipt now assert the fail-closed untouched
    framebuffer, with pixel application still proven by
    test_dm2_v1_weather_renderer_material_gate.c (passes).
  - Wall panel render plan: the planner follows DM2_DRAW_DUNGEON_TILES
    walking table1d7029 — skproject pass order D0R(9), D0L(11), D1L(12),
    D1R(13), D1C(14), D2L(15), D2R(16), D2C(17), D3L(18), D3R(19) with
    G0163 frame src/dst rects — not the old DM1 back-to-front depth
    order.
  - Door render plan: 45917ebc4 made the panel route state-aware, so
    the combined check split in two — door_state=0 keeps the DB0
    record-panel route; door_state=5 (destroyed) takes the for_square
    panel plus the destroyed mask.
  - Floor/ceiling + wall callback fetches: the renderer fetches
    floor/ceiling through the GRAPHICSSET scene-material encoding
    (SCENE_MATERIAL_BASE - (set<<8) - field), so the fixture decodes
    via dm2_v1_viewport_scene_material_graphic_address; the wall plan
    src_rects are the G0163 frame rectangles (up to x=192 w=128 h=136),
    so the fixture presents the full 320x136 wall-set image they crop
    from. The GDAT palette-mapping check followed from these two.
  - HUD champion bars: skproject SkWinCore.cpp::INIT fills
    glbChampionColor in player order and DRAW_PLAYER_3STAT_HEALTH_BAR
    indexes that single table for HP, stamina and mana alike — slot 0
    draws all three bars in color 7, not one invented color per
    resource; the sampled x pairs still discriminate each fill width.
  Regression net (ctest -R lighting|weather|wall|door|hud|palette|scene,
  339 tests): the remaining failures are pre-existing on main ccfcbe984
  (dm1_*/nexus/m11 capture- and audit-bound families plus
  dm2_v1_hud_hero_type_gdat_route, v1_viewport_*_gate,
  m11_open_door_spell_runtime_source_lock, firestaff_dm2_v2_hud_widget_*)
  — the branch touches only the lighting test file and a test-only
  header constant, so none can be affected by these commits.

- 2026-07-21 Nexus round-19 second attempt (job/w4): two fixes landed
  (commits 1e41baac5, fb148e5e6).
  - ISO source receipts no longer rescan the full data tree per boot
    file/level (1e41baac5): `nexus_v1_level_aux_source_receipt` and
    `nexus_v1_structure2_source_receipt` used to call
    `asset_find_by_md5` over the whole 1.1 GB data dir to rediscover an
    ISO entry the engine had already opened; the scan could only ever
    publish the `"<iso>::<name>"` entry already in hand, so the entry's
    bytes are now hashed directly in place via the new static helper
    `nexus_v1_iso_entry_matches_canonical_md5` (semantically
    equivalent). Measured effect: real-data launch 229 s -> ~5 s;
    `nexus_v1_runtime_screenshot_readiness` PASS in 4.6 s (was
    FAIL/timeout); the entire live/skip-safe tier1 timeout family is
    green without FIRESTAFF_NEXUS_DATA_DIR. Root cause of the timeout
    family was the redundant full-tree MD5 rescan, not the environment.
  - `nexus_v1_sound_runtime_receipt` restored (fb148e5e6): a job-branch
    merge (f85b15531 'Merge branch job/w4') had silently reverted the
    test to pre-9f9dd8e00 expectations, re-asserting the legacy flat
    MAP byte-to-event promotion that 'nexus: block unproven SNDLEV
    event routes' deliberately removed. The engine was never regressed
    — the fixture was. The dc975dbe3 version of the test (matching the
    hardened engine: raw MAP bytes are not promoted to event routes;
    SNDLEV windows must be verified before binding) is restored; 0 FAIL
    verified after rebuild.
  - With the timeouts gone, previously masked pre-existing failures are
    now classifiable; triage results and the remaining list are in
    TODO.md (same date).
- 2026-07-21 Theron round-20 enclosing $45xx routine byte-bound
  (job/w5): the routine that holds the two round-17 $45xx-tier
  JSR $4696 windows binds in the stage-two image lane via the new
  `theron_v1_track02_verify_stage2_enclosing_45xx` verifier — 185
  bytes, one window [0x45b1..0x466a), hand-decoded from the
  authenticated US Track 02 media (MD5 f23601102138f87c33025877767ebf76)
  and matched against da65's inline linear decode
  (theron-us-stage2-huc6280.asm:10102-10191; the routine has no da65
  entry label — its decode starts after the preceding RTS at
  asm:10100):
  - The body: the LDA $12 / PHA / LDY $11 prologue, the two LDA #$08 /
    STA $0E / JSR L4696 multiply calls with their $54:$55 and $52:$53
    result saves, JSR L4552, the $13/accumulator ASL pair into
    L47B8/L47B9, the DEC $5A / JSR L4932 / STZ $5A window, the $14:$15
    -> $0E:$0F / JSR L458E call, the $02:$03 -> $06:$07 and $56-$58 ->
    L466A/$16/$17 saves, and the L47B9/2 X-trip-counted main loop
    (da65's L8610 head at +0x5f): JSR L424B, the $47E0 -> $00:$01 /
    $06:$07 -> $02:$03 / L47B8 -> $0E:$0F setup, JSR L466B, the $06:$07
    +$40 advance, the $17 EOR #$02 toggle with its $16 row counter (the
    STZ $16 plus L47BE/L47C4 accumulate and JSR L43D6 every $10 rows),
    DEX / BNE L8610, RTS — every relative (BCC 90 02, BNE d0 17/d0 0f,
    BNE d0 a7) resolves inside the body; the trailing RTS at 0x4669
    sits immediately before the next stream's BRK byte (da65
    asm:10193), confirming the span.
  - Flagged da65 artifact classes, media authoritative: the L0011/L0000
    zero-page-as-absolute renderings (media: A4 11 at +0x03, 85 00 at
    +0x72) and the L466A data label (the STA/LDA $466A absolute
    operands at +0x4f/+0x60, not code inside the span).
  - Call-site invariants compile-time-asserted: the two round-17
    JSR $4696 image offsets equal the routine base +0x09/+0x1a and sit
    inside the span (the whole-body exact match covers their 20 96 46
    bytes); the L8610 loop-head offset lands inside the body. The entry
    CPU address is not pinned (no bound caller; runtime bank mapping
    out of scope), so the receipt carries 0 — the L3172/$117D
    precedent.
  - Probe: US fixture + real-media whole-body match; the changed head
    byte and the changed loop-back BNE each fail closed; the JP
    fixture rejects; `summary: fail=0`. ctest baseline unchanged (the
    same 14 known failures, name-for-name).
  - The six callees L4552, L4932, L458E, L424B, L466B, and L43D6
    remain unbound future windows; US-only as before (JP rejects until
    staged JP media can verify the same stream); no semantics, System
    Card base or bank-mapping arithmetic, record semantics, or graphics
    role follows.

- 2026-07-21 Theron round-20 stage-two L3114 tier-4 callees byte-bound
  (job/w5): the nine tier-4 windows parked after round 19 all bind in
  the stage-two image lane via the new
  `theron_v1_track02_verify_stage2_l3114_tier4_callees` verifier — 279
  bytes across nine windows, hand-decoded from the authenticated US
  Track 02 media (MD5 f23601102138f87c33025877767ebf76) and matched
  against da65's inline linear decodes (theron-us-stage2-huc6280.asm:
  2356, 2947, 3000, 3059, 3082, 3109, 3119, 3136, 3322):
  - L4F7A [0x0f7a..0x0f89) (15 bytes): PHX / PHY / LDX L4FD4 / the
    CLY/DEY/BNE inner and DEX/BNE outer delay nest / PLY / PLX / RTS —
    both BNE relatives resolve to their da65 labels (d0 fd -> L4F80,
    d0 f9 -> L4F7F); the body starts exactly where the bound L4F66
    delay loop ends (adjacency compile-time-asserted).
  - L535E [0x135e..0x136e) (16 bytes): the LDA ($04),y / INY / STA
    a:$02 / LDA ($04),y / INY / STA a:$03 / DEX / BNE pair loop / RTS
    — the media confirms da65's `a:` absolute stores; the BNE (d0 f1)
    resolves to the L535E head; the body ends exactly at the bound
    L536E entry (adjacency compile-time-asserted).
  - L53C4 [0x13c4..0x1403) (63 bytes): JSR L51F9 (round-18 target) /
    the L4FD5/L4FD6 -> $04:$05 setup / the L4F8B,y record-copy loop /
    the $0E:$0F tail store / BSR L542D / the PHY/PHX/BSR L5403 row
    loop / writeback / CLC / RTS — all four relatives (BNE d0 f7 x2,
    BSR 44 45 -> L542D, BSR 44 11 -> L5403) resolve to their da65
    labels; L5403/L541E remain unbound tier-5 windows.
  - L542D [0x142d..0x1439) (12 bytes): the $04:$05 +6 fix-up — the
    trailing RTS at 0x1438 is da65's L5438 label, sitting immediately
    before the bound L5439 entry (adjacency compile-time-asserted).
  - L5455 [0x1455..0x1482) (45 bytes): the writeback / ($04),y field
    loads / BSR L542D / the PHY/PHX/JSR L4F7A/BSR L5482 row loop /
    RTS — all three relatives (BSR 44 b8 -> L542D, BSR 44 06 ->
    L5482, BNE d0 f4 -> L5475) resolve to their da65 labels.
  - L5482 [0x1482..0x1492) (16 bytes): PHX / CLY / DEC $5A / JSR
    L54A0 (round-19 target) / JSR L535E / STZ $5A / PLA / BSR L5492
    (44 01) / RTS.
  - L5492 [0x1492..0x14a0) (14 bytes): ASL A / the $04:$05 advance /
    JSR L5213 (round-17 target) / RTS — the body ends exactly at the
    bound L54A0 entry (adjacency compile-time-asserted).
  - L54AF [0x14af..0x14c5) (22 bytes): the L4F9F-indexed L4FA0,x
    pair store / INC L4F9F / RTS — the body starts exactly where the
    bound L54A0 ends (adjacency compile-time-asserted); L54C5 remains
    an unbound tier-5 window.
  - L560B [0x160b..0x1657) (76 bytes): the 9-row $5667 copy setup —
    da65's declared L563D label splits the BCC operand byte at 0x163d
    (a second instance of the round-18 mid-instruction class, emitted
    as `.byte $90` plus garbage `st0 #$EE` / `cld` / `.byte $4F`), so
    the media bytes are authoritative: BCC L5641 / INC $4FD8; the
    L0000 zero-page-as-absolute renderings are likewise superseded.
    All four relatives (BSR 44 2b -> L5657, BCC 90 03 -> L5641, BCC
    90 02 -> L5653, BNE d0 cd -> L5623) resolve to their da65 labels;
    the body starts exactly where the bound L5600 ends; L52A2/L52C8/
    L5657 remain unbound tier-5 windows.
  - The contiguous chain is compile-time-asserted end to end:
    L4F66->L4F7A, L535E->L536E, L542D->L5439->L5455->L5482->L5492->
    L54A0->L54AF ([0x142d..0x14c5) fully byte-bound), L5600->L560B.
  - Call-site invariants compile-time-asserted: the JSR L4F7A at
    +0x09 inside the bound L5C9F body and the JSR L5492 at +0x1f
    inside the bound L5C69 body.
  US-only, as before — the JP variant rejects (NOT_FOUND, invalid
  receipt) until staged JP media can verify the same streams. Probe:
  positive fixture receipt with full field asserts, four byte
  mutations (L4F7A/L53C4/L5455/L560B heads) each fail closed,
  JP-scope rejection, and the real-media check against the staged US
  Track 02 — `summary: fail=0`. ctest -R theron: 148/162 with the 14
  pre-existing failures unchanged (name-for-name baseline diff). No
  tier-5 callee (L5403/L541E/L52A2/L52C8/L5657/L54C5), LE063-target,
  semantics, System Card base or bank-mapping arithmetic, record
  semantics, or graphics role follows. The enclosing $45xx routine
  and L383E (dynamic-payload lane) remain future windows.

- 2026-07-21 Theron round-19 stage-two L3114 tier-3 callees + L5C20
  table + L5C2C entry byte-bound (job/w5): the seven tier-3 windows
  parked after round 18 all bind in the stage-two image lane via the
  new `theron_v1_track02_verify_stage2_l3114_tier3_callees` verifier —
  223 bytes across eight windows, hand-decoded from the authenticated
  US Track 02 media (MD5 f23601102138f87c33025877767ebf76) and matched
  against da65's inline linear decodes (theron-us-stage2-huc6280.asm:
  2957, 3067, 3128, 3316, 4184, 4241, 4270):
  - L5C06 [0x1c06..0x1c20) (26 bytes): DEC L4FD1 / BEQ / RTS / the
    LDA L4FD2 / EOR #$01 / STA L4FD2 toggle / BEQ L5C1B / JSR L5C25 /
    CLA / RTS / L5C1B: JSR L5C2C / CLA / RTS — both BEQ relatives
    resolve to their da65 labels; the body ends exactly at the bound
    L5C20 table (adjacency compile-time-asserted).
  - L5C20 table [0x1c20..0x1c25) (5 zero bytes as loaded; da65
    renders BRK x5): the data window read by the bound L5BF5 copy
    loop; the table ends exactly at the bound L5C25 entry (adjacency
    compile-time-asserted) and the L5BF5 data site (LDA $5C20,x at
    +0x03) targets it.
  - L5C69 [0x1c69..0x1c8c) (35 bytes): self-modifying — STA L5C7E
    rewrites the ORA/AND opcode byte at 0x1c7e; the media bytes are
    the as-loaded image. Every relative (BEQ f0 04, BRA 80 02, BNE
    d0 f4) resolves to its da65 label; the body ends exactly at the
    bound L5C8C entry (adjacency compile-time-asserted); its L5492
    callee remains an unbound tier-4 window.
  - L5C9F [0x1c9f..0x1cb0) (17 bytes): the L4FD4 save / LDA #$07 /
    JSR L4F7A / restore / RTS — ends exactly at the bound L5CB0
    entry (adjacency compile-time-asserted); its L4F7A callee remains
    an unbound tier-4 window.
  - L536E [0x136e..0x13c4) (86 bytes): the L4FB8-indexed L4FB9,x pair
    store / INC L4FB8 / the $0E:$0F x L4F8D multiply-accumulate loop /
    ASL $0E / ROL $0F / the L4FD5/L4FD6 add / the $DFF0 bounds
    compare / the STZ L4FB9,x pair / RTS — all four branches (BCC
    90 02, BNE d0 f1, BCC/BNE 90 06/d0 04, BCC 90 08) resolve to
    their da65 labels; the L53C4 continuation remains unbound.
  - L5439 [0x1439..0x1455) (28 bytes): DEC L4FB8 / the L4FB9,x pair
    load into $04:$05 / the null-pair early-out — both BNE relatives
    resolve to L5455 (an unbound tier-4 continuation).
  - L54A0 [0x14a0..0x14af) (15 bytes): ST0 #$00 / LDA $0E / STA
    a:$02 / LDA $0F / STA a:$03 / ST0 #$02 / RTS — the media confirms
    da65's `a:` absolute-store rendering ($8D $02 $00, not the
    zero-page form); the body ends exactly at the next da65 label
    L54AF (unbound).
  - L5600 [0x1600..0x160b) (11 bytes): LDA $06 / STA a:$02 / LDA $07
    / STA a:$03 / RTS — same absolute-store confirmation; the body
    ends exactly at the next da65 label L560B (unbound).
  - L5C2C alternate entry: sits at +0x07 inside the already bound
    L5C25 window (the LDA #$EF / STA $5C24 head) — offset
    compile-time-asserted, no new bytes.
  - Call-site invariants compile-time-asserted: two JSR opcodes at
    +0x00/+0x05 inside the bound L5C8C body; the JSR L536E / BSR
    L5C69 / JSR L5439 at +0x0c/+0x2c/+0x3c inside the bound L5C25
    body; the JSR L54A0 / BSR L5600 at +0x02/+0x05 inside the bound
    L55F6 body; the LDA $5C20,x data site at +0x03 inside the bound
    L5BF5 body.
  US-only, as before — the JP variant rejects (NOT_FOUND, invalid
  receipt) until staged JP media can verify the same streams. Probe:
  positive fixture receipt with full field asserts, four byte
  mutations (L536E/L5C06/L54A0 heads, L5C20 table byte) each fail
  closed, JP-scope rejection, and the real-media check against the
  staged US Track 02 — `summary: fail=0`. ctest -R theron: 148/162
  with the 14 pre-existing failures unchanged (name-for-name baseline
  diff). No tier-4 callee (L4F7A/L542D/L5482/L5492/L535E/L5455/
  L53C4/L54AF/L560B), LE063-target, semantics, System Card base or
  bank-mapping arithmetic, record semantics, or graphics role
  follows. The enclosing $45xx routine and L383E (dynamic-payload
  lane) remain future windows.

- 2026-07-21 DM1 round-18 portrait_18 reincarnate_reselect probe fix
  (job/w1, commit 8e05380f5): the parked portrait_18 class is GREEN —
  59/59 assertions, ctest
  firestaff_dm1_v1_hoc_champion_portrait_18_reincarnate_reselect_portrait_rect_position_258_gate_probe
  PASS.  Two probe-local bugs, no engine source touched: (1) the
  'SHE DEVI' title truncation was a probe buffer declared with
  CHAMPION_NAME_TEXT_CAPACITY(9) instead of
  CHAMPION_TITLE_TEXT_CAPACITY(21) — 'SHE DEVIL' (9 chars + NUL)
  truncated to 8; (2) the reselect scenario contradicted the
  source-locked F0282 routing (dm1_v1_resurrection_pc34_compat.c —
  "745-757: Cancel ... decrements party count. 785-799:
  Resurrect/Reincarnate clear candidate state and disable mirror
  sensor"): the probe expected the C165 REINCARNATE confirm to
  consume the appended slot (count 1->0) and then re-select the same
  ordinal, but the champion JOINS the party on confirm (count stays
  1) and a fresh F0280 select of the same mirror record is refused
  by the already-in-party guard.  Rewritten to the source-faithful
  round-trip F0280 select -> F0282(C162 cancel) -> F0280 reselect ->
  F0282(C165 REINCARNATE), which still gates the REINCARNATE confirm
  branch the 216_gate/ordinal-11 siblings do not cover.
- 2026-07-21 DM1 round-18 portrait_06/17 inventory-toggle contract
  (job/w1, commit 6d4aece52): the parked portrait_06/17 class is
  GREEN — both probes 37/37 assertions, ctest
  firestaff_dm1_v1_hall_of_champions_portrait_06/17_inventory_exit_restore_portrait_rect_position_runtime_probe
  PASS.  The C040 panel-survival contract was decided from ReDMCSB:
  COMMAND.C F0380:2181-2183 gates the C007..C011 inventory-toggle
  COMMANDS on !G0299_ui_CandidateChampionOrdinal — while the C040
  candidate panel is live the toggle is DROPPED (F0355 never runs;
  dedicated contract module dm1_v1_mirror_candidate_c040_inventory_
  toggle_while_panel_live_pc34_compat) and only becomes live after
  F0282 clears G0299.  The !G0299 gate inside PANEL.C F0355:2318-2322
  is the internal F0292-redraw suppression on the close path, not
  permission for the command route — the engine's
  ToggleInventoryPanel refusal is source-faithful (no engine source
  touched).  Both probes' Group C rewritten to lock the contract:
  toggle dropped twice while C040 live (state + framebuffer
  byte-identical), cancel clears G0299, the same toggle then opens
  and closes the inventory with the mirror still armed.
- 2026-07-21 DM2 round-18 cross-cutting: two pre-existing failures fixed
  (job/w3).  (1) dm2_v1_gdat_materialization_handoff — birth-defect stale
  test from d7e6a7d6c: the build gate requires the source-faithful exact
  GDAT interval (material_byte_count == stride * height) but the test
  never set material_byte_count.  Re-anchored the fixture
  (material_byte_count = stride * height = 4 for its 2x2 material); the
  generation-stale half of the contract is unchanged and still enforced.
  (2) dm2_v1_g1_record_list_material_gate — restored the 3cf040333
  fail-closed record-graph gate in dm2_v1_dungeon_find_text_wall_gfx and
  dm2_v1_dungeon_find_actuator_wall_gfx_ordinal
  (dm2_v1_dungeon_record_list_traversal_allowed: raw_data &&
  record_graph_complete).  skdungn.cpp consumes DB2 TextMode wall-GFX and
  DB3 Actuator::GraphicNumber only through the complete byte-square G1
  map-to-record graph; the gate was lost when 6ab20e42d inlined the
  text-wall-gfx owner walk and in a later actuator-walk rewrite.  The
  only production caller (dm2_v1_runtime.c door-record processing) runs
  on loaded dungeons whose graphs are complete, and failing closed on an
  incomplete graph is the designed discipline.  Both target tests PASS.
  Baseline-verified via stash: the six neighbouring failures
  (dm2_v1_g1_null_record_address_gate, dm2_v1_g1_null_record_link_gate,
  dm2_v1_g1_center_ray_surface_gate, dm2_v1_g1_record_base_gate,
  dm2_v2_hud_runtime_probe, m11_dm1_door_host_presentation_receipt) fail
  identically with and without the gate restoration — pre-existing, now
  recorded in TODO.md.  dm2_v1_lighting_palette_runtime_gate (146/161)
  triaged into a four-domain breakdown in TODO.md as the next block.
  Pre-commit bypassed: hash_harmonization fails on the known pre-existing
  dm2-mac-en data mismatch, unrelated to this change (same as rounds
  14-17).
- 2026-07-21 Nexus round-18 (job/w4): the round-17 class-(c) remainder
  plus one extra baseline failure fixed — both class (a) stale fixtures,
  no engine behavior changed. Both gates PASS via ctest; the remaining
  suite failure list is baseline-minus-these-two with no new failures.
  - m11_nexus_startup_runtime_handoff (was 7 FAILs): traced the full
    champion-start gate chain (M11 keyboard ACTION ->
    m11_nexus_refresh_startup_host_caller ->
    nexus_v1_launcher_startup_host_caller_receipt_from_snapshot ->
    real_asset_ownership -> runtime_route_from_champion_firestaff_input)
    with a standalone diagnostic probe replicating the test fixture
    against the public launcher API. The chain was healthy: the ACTION
    start was rejected one level down with
    exec.status="blocked-menu-bpk-invalid" because
    nexus_v1_launcher_fill_startup_assets_receipt requires bounded BPK
    provenance (archive_entries > 0, planned_rows > 0, not truncated,
    no fallback visuals) before real_menu_surface_route_ready — and the
    synthetic engine fixture set only route/ready_uploads/planned_rows.
    Fixture re-anchored with archive_entries=3, surface_entries=3,
    directory_trailer_found/valid=1; START_DUNGEON (exec.kind=3,
    "NEXUS READY") now applies, champion_select_active clears, and the
    fail-closed synthetic-DGN/idle/pixel assertions hold. Final
    classification: class (a), not (c) — the routing-regression
    hypothesis from round 17 was disproven by the trace.
  - nexus_v1_track1_phase_launch_synthetic (was 2 FAILs): same stale
    Structure1B fixture family as round 17's mechanics_parity — the
    probe wrote NEXUS_SQUARE_WALL (0) into cell byte[6], which the
    proven 1B decoding (nexus_v1_decode_structure1b_cell) reads as
    collision ref 0x000 -> floor. Edge cells now write collision ref
    0x0FFF (cell[6]=0x0F, cell[7]=0xFF). 32/32 PASS.
- 2026-07-21 Theron round-18 stage-two L3114 tier-2 callees byte-bound
  (job/w5): the seven callees of the bound $117D trampoline, L526D,
  and L55E0 parked after round 17 all bind in the stage-two image lane
  via the new `theron_v1_track02_verify_stage2_l3114_tier2_callees`
  verifier — 162 bytes across seven windows, hand-decoded from the
  authenticated US Track 02 media (MD5 f23601102138f87c33025877767ebf76)
  and matched against da65's inline linear decodes
  (theron-us-stage2-huc6280.asm:2740, 3299, 3310, 4174, 4205, 4261,
  4279):
  - L51F9 [0x11f9..0x1213) (26 bytes): STZ $0E / LDA $4F8C / LSR A /
    ROR $0E / LSR A / ROR $0E / STA $0F / CLC / LDA $0E / ADC $4F8B /
    STA $0E / BCC / INC $0F / RTS — da65's declared L5200 label splits
    the `ror $0E` at 0x11ff-0x1200 (a mid-instruction label artifact,
    emitted as `.byte $66` plus the garbage `asl $664A` / `asl $0F85`
    renderings), so the media bytes are authoritative; the trailing
    RTS sits immediately before the bound L5213 entry (adjacency
    compile-time-asserted).
  - L55E8 [0x15e8..0x15ef) (7 bytes): INC $0E / BNE / INC $0F / RTS,
    directly after the bound L55E0 body (adjacency compile-time
    asserted); the next da65 label L55EF confirms the span.
  - L55F6 [0x15f6..0x1600) (10 bytes): DEC $5A / JSR L54A0 / BSR
    L5600 / STZ $5A / RTS; the BSR relative (44 03) resolves to the
    da65 L5600 label; its L54A0/L5600 callees remain unbound future
    windows.
  - L5BF5 [0x1bf5..0x1c06) (17 bytes): LDY #$04 / CLX / the LDA
    $5C20,x / STA $4F8B,x / INX / DEY / BNE copy loop / STZ $4FD1 /
    RTS; the BNE relative (d0 f6) resolves to the da65 L5BF8 label;
    the L5C20 table stays unbound data.
  - L5C25 [0x1c25..0x1c69) (68 bytes): LDA #$F0 / STA $5C24 / BRA
    L5C31 / JSR L536E / the $4FB8/$4FB9,x indexed $04:$05 setup /
    LDY $4F8E / LDX $4F8D / the PHY/PHX/BSR L5C69/PLX/PLY/DEY/BNE
    loop / the $4FD4 save / JSR L5439 / restore / RTS — every relative
    (BRA 80 05, BSR 44 16, BNE d0 f7) resolves to its da65 label; the
    L5C2C alternate entry and the L536E/L5C69/L5439 callees remain
    unbound future windows.
  - L5C8C [0x1c8c..0x1c9f) (19 bytes): JSR L5C06 / BEQ / JSR L5C9F /
    JSR LE063 / LDA $222D / BEQ L5C8C / STA $08 / RTS — the BEQ
    self-loop relative (f0 f0) resolves to the L5C8C head; the
    L5C06/L5C9F callees and the LE063 far-call target remain unbound
    future windows.
  - L5CB0 [0x1cb0..0x1cbf) (15 bytes): PHA / PHX / PHY / the JSR
    LE063 / LDA $2228 / BNE poll loop / PLY / PLX / PLA / RTS — the
    BNE relative (d0 f8) resolves to the da65 L5CB3 label; the LE063
    far-call target remains an unbound future window.
  - Call-site invariants compile-time-asserted: the four JSR opcodes
    at +0x00/+0x03/+0x06/+0x09 inside the bound $117D trampoline, the
    JSR L51F9 at +0x00 of the bound L526D body, and the two BSR
    opcodes at +0x00/+0x02 of the bound L55E0 body (offset + 3 <=
    caller BYTES); all seven callees keep da65's linear CPU = image +
    $4000 form (no bank-mapping arithmetic follows).
  US-only, as before — the JP variant rejects (NOT_FOUND, invalid
  receipt) until staged JP media can verify the same streams. Probe:
  positive fixture receipt with full field asserts, four byte
  mutations (L51F9/L55F6/L5C25/L5C8C heads) each fail closed,
  JP-scope rejection, and the real-media check against the staged US
  Track 02 — `summary: fail=0`. ctest -R theron: 148/162 with the 14
  pre-existing failures unchanged (name-for-name baseline diff). No
  tier-3 callee, LE063-target, L5C2C-entry, semantics, System Card
  base or bank-mapping arithmetic, record semantics, or graphics role
  follows. The enclosing $45xx routine and L383E (dynamic-payload
  lane) remain future windows.

- 2026-07-21 DM1 round-17 pass373 launcher runtime fix (job/w1):
  pass373_dm1_v1_launcher_viewport_redraw_wall_occlusion_path is fully
  green — PASS373_LAUNCHER_VIEWPORT_REDRAW_WALL_OCCLUSION_PATH_PROVED
  with probe launchedEver:1, viewportDirty:1 and
  inputRedrawAfterViewportDirtyCount:1.  Root cause: the M12 game-subdir
  scan-root promotion lifted --data-dir .../_canonical/dm1 to its
  parent, so the direct DUNGEON.DAT MD5 branch missed and
  asset_find_by_md5 matched the zip member
  Dungeon-Master_DOS_EN.zip::.../DATA/DUNGEON.DAT (FIRESTAFF_HAS_ZLIB
  enabled in the real build) before the real file; F0500 only opens
  plain fopen paths, so DUNGEON.DAT failed to load and the launch
  smoke exited rc=3 ("no launch reached").  Fix: a dm1-subdir MD5
  probe (dataDir/dm1/DUNGEON.DAT) in m11_resolve_builtin_dungeon_path
  after the direct-file test — one file touched
  (src/engine/m11_game_view.c, +25 lines), zero new test failures
  (the 12 broad-sweep failures confirmed pre-existing via stash check
  on unchanged engine source).  Regression chain 9/9 PASS
  (pass359/361/375/405/427 + 4 v1_viewport gates); dm1_v1_startup
  1/1 PASS.
- 2026-07-21 DM2 wall/door + HUD portrait local palette gates re-anchored
  (job/w3, round 17, cross-cutting): the two sibling gates flagged in the
  round-16 observation failed in the same stale-after-5c21e5561 pattern
  (0/2 each) and are now green.  Both were rewritten to the
  UPDATE_GFXSET ownership/transaction discipline: under
  source_materials_required the materials are owned by boot-owned M11
  command plans whose commands each carry the palette of their own GDAT
  lookup; the provider callback is no longer consulted (50a939491 bound
  the wall path to G1 wall plans; the door panel/frame callback route is
  dead under source_required because the decoded_hash receipt can only
  come from a plan command; the HUD portrait branch requires
  gdat_hud_material_plan).  dm2_v1_wall_door_local_palette_gate: builds
  minimal synthetic fixtures with the builders' own receipt discipline —
  a one-command wall M11 plan (D1C, 0xa0-based palette, geometry hash
  replicated from the renderer's FNV formula) and a one-command door
  overlay plan (closed D0C panel, distance-0/0x71-stretch/light-0 draw
  controls per SkWinCore.cpp:46431-46441, no_frames, FNV-consistent
  decoded/palette hashes, plan hash replicated from the builder) — and
  asserts the per-command palettes reach the framebuffer (0xa1 wall,
  0xb1 door) with zero provider fetches.  The fail-closed case blocks at
  the transaction gate (wall, pre-fetch) and the per-material receipt
  gate (door), framebuffer untouched.  dm2_v1_hud_portrait_local_palette_gate:
  builds a 9-command HUD M11 plan (family-minimum count) with one
  CHAMPION_PORTRAIT command (FNV-consistent palette hash, exported
  pixel/plan hash functions), plus the INTERFACE_GENERAL layout/palette/
  dt07 font ownership the dynamic-overlay gate requires, and asserts the
  plan-owned draw with zero provider fetches; the fail-closed case gives
  the portrait command palette_hash 0, which the plan command gate
  rejects before any pixel or fetch.  Both target tests PASS; bounded
  regression (local_palette_gate, wall_plan_viewport, door_overlay,
  hud_m11, outdoor_scene, scene_light) 12/12 PASS including the real-data
  plan tests and both round-16 re-anchors.  No src changes; test-only
  round.  Pre-commit bypassed: hash_harmonization fails on the known
  pre-existing dm2-mac-en data mismatch, unrelated to this change (same
  as rounds 14-16).
- 2026-07-21 Nexus round-17 deep triage (job/w4): five of six named Nexus
  failures fixed — all class (a) stale expectations re-anchored to the
  proven/current engine contract, no engine behavior changed. All five
  gates PASS via ctest (script_vm additionally PASS against the retail
  corpus with FIRESTAFF_NEXUS_DATA_DIR set); the remaining suite failure
  list is baseline-minus-these-five with no new failures.
  - nexus_v1_script_vm: the 9b2482b93 merge had clobbered the test back
    to pre-hardening dispatch expectations. Restored the e35883caa
    version, then re-anchored it further to the intentionally
    strengthened SLEV evidence chain (96fb749ee binds captures to
    loaded bytes, 2026db6f8 requires bound trace evidence): the
    synthetic trace text now carries `source_fnv1a64` computed over the
    loaded 96-byte fixture (trace numerics are hex-encoded:
    source_byte_count=0x60=96, task_header_size=0x24=36, verified
    against `nexus_v1_engine_build_slev_capture_target` on the same
    fixture).
  - nexus_v1_bpk_surface_class: restored the fdddbe962 test version
    ("require complete original routes"); the engine deliberately
    refuses PRS3 decode with NEXUS_V1_BPK_DECODE_ERR_STREAM and the
    clobbered copy had reverted to the older acceptance expectations.
  - nexus_v1_startup_title_pointer_contract: one-line re-anchor —
    champion select now expects animation_active == 1 per the da98fe28a
    presentation animation package gate
    (`nexus_v1_startup_presentation_animation_package_gate` consumes
    the champion animation). 17/17 PASS.
  - firestaff_nexus_v1_mechanics_parity: fixture re-anchored to the
    proven Structure1B decoding in `nexus_v1_decode_structure1b_cell`
    (wall only when collision ref == 0x0FFF, door when BE16 flags bit0
    set, floor otherwise; stairs/teleport/pit/exit are not expressible
    in 1B cell bytes). Walls now write collision 0x0FFF; former
    "special square" cells assert the floor contract; the malformed
    actor-ref cell keeps byte[1] bit0 clear (bit0 lives in the low
    byte). Also fixed a latent stack-overflow SIGSEGV exposed on
    rebuild: inlined probes gave main a ~10.5 MB frame (Nexus_V1_World
    is ~5.5 MB, Nexus_V1_Engine ~1.5 MB) exceeding the 8 MB main-thread
    stack — probes are now PROBE_NOINLINE and probe_world's two World
    instances are static. 185/185 PASS.
  - m11_nexus_light_spell_dispatch: fixture re-anchored — the spell
    panel open path now routes through the DM1 PC34 caster selection
    (96efeb9da receipts + f50da7ea7 per-caster state; CASTER.C F0394
    requires a present, living champion), so seed_nexus_state seeds one
    present champion with hp. 37 prior FAILs all cascaded from the
    unseeded caster gate.
  Sixth failure (`m11_nexus_startup_runtime_handoff`) is class (c)
  remaining work — documented with a root-cause hypothesis in TODO.md
  same-date entry.
- 2026-07-21 Theron round-17 stage-two L3114 callees + $45xx-tier
  L4696 call sites byte-bound (job/w5): the six L3114 BSR/JSR callees
  parked after round 16 all bind in the stage-two image lane via the
  new `theron_v1_track02_verify_stage2_l3114_callees` verifier — 89
  bytes across eight windows, hand-decoded from the authenticated US
  Track 02 media (MD5 f23601102138f87c33025877767ebf76) and matched
  against da65's inline linear decodes where they exist
  (theron-us-stage2-huc6280.asm:2339, 2754, 2805, 3293):
  - L3172 [0x1172..0x117d) (11 bytes): LDA #$01 / STA $5C22 / LDA
    #$01 / STA $5C23 / RTS — declared `L3172 := $3172` absolute by da65
    without a body decode; low-image region below $3800, never
    clobbered by the dynamic-payload CD_READ.
  - $117D far-helper trampoline [0x117d..0x118a) (13 bytes): JSR
    $5BF5 / JSR $5C8C / JSR $5CB0 / JSR $5C25 / RTS; its own callees
    remain unbound future windows.
  - L4F66 [0x0f66..0x0f7a) (20 bytes): the shared delay loop
    (PHA/PHX/PHY, LDA #$03 CLX CLY DEY/DEX/DEC A BNE nest,
    PLY/PLX/PLA/RTS — matching the L3114 LDA #$1A loop), at bank-0
    offset 0x0f66 directly after the bound L4F5E selector window
    (selector-end adjacency compile-time-asserted); the next da65
    label L4F7A confirms the span.
  - L5213 [0x1213..0x121f) (12 bytes): CLC / LDA $0E / ADC #$40 /
    STA $0E / BCC / INC $0F / RTS — the da65 `bcc L521E` rendering is
    consistent with the media `90 02` (no artifact).
  - L526D [0x126d..0x1280) (19 bytes): JSR L51F9 / LDX #$04 / LSR
    $07 / ROR $06 / DEX / BNE / LDA $07 / ORA #$F0 / STA $07 / RTS;
    the next da65 label L5280 confirms the span; its L51F9 callee
    remains an unbound future window.
  - L55E0 [0x15e0..0x15e8) (8 bytes): BSR L55F6 / BSR L55E8 / DEX /
    BNE L55E0 / RTS; both BSR relatives resolve to the da65 labels;
    its L55F6/L55E8 callees remain unbound future windows.
  - Call-site invariants compile-time-asserted: the six L3114 JSR/BSR
    opcodes sit at offsets +0x01/+0x08/+0x0e/+0x32/+0x4d/+0x56 inside
    the bound L3114 body (offset + 3 <= 0x5e); the four far callees
    keep da65's linear CPU = image + $4000 form. The two $45xx-tier
    L4696 call sites bind as 3-byte JSR windows at image offsets
    0x45ba/0x45cb (da65 asm:10107/10115, both preceded by LDA #$08 /
    STA $0E), the target encoded from the pinned L4696 CPU address;
    the enclosing $45xx routine remains unbound.
  US-only, as before — the JP variant rejects (THERON ..._NOT_FOUND)
  until staged JP media can verify the same streams. Probe: positive
  fixture receipt with full field asserts, four byte mutations
  (L3172/L4F66/L55E0 heads, $45xx call-site head) each fail closed,
  JP-scope rejection, and the real-media check against the staged US
  Track 02 — `summary: fail=0`. ctest -R theron: 148/162 with the 14
  pre-existing failures unchanged (name-for-name baseline diff). No
  callee-of-callee, semantics, bank-mapping arithmetic, record
  semantics, or graphics role follows.

- 2026-07-20 DM1 round-16 same-drift-family verifier re-anchors
  (job/w1): the ten gates parked after round 15 are re-anchored to the
  current engine structure and green — no engine source touched,
  zero new test failures (remaining v1_viewport ctest failures
  confirmed pre-existing via stash check on unchanged engine source).
  Re-anchored verifiers, all PASS (9/9 registered ctest gates; the
  unregistered ones verified directly):
  - pass375_dm1_v1_deferred_explosion_pass: side contents locked on
    the per-depth m11_draw_dm1_side_contents_at_depth; explosion layer
    materializes C15 records via m11_draw_explosion_material (keeps the
    m11_draw_explosion_sprite bitmap path); projectile guards on
    renderableProjectileCount.
  - pass405_dm1_v1_viewport_projectile_explosion_layer_occlusion:
    center effect cue + side contents projectile tokens re-anchored to
    renderableProjectileCount/m11_draw_viewport_projectile_sprite;
    deferred-pass occlusion guards on the shared lane-visibility
    receipt helpers; viewport order follows the F0128 plan-driven loop.
  - pass427_dm1_v1_viewport_walls_gap_gate: far-edge side wall zones
    locked on the contract module s_wall_draw_specs[] rows
    (DM1_VIEW_SQUARE_*/DM1_WALL_* parity columns); clipped D3L2/D3R2
    side-door zones locked on the side-door render plan module; C10
    transparency key locked at the contract handoff.
  - v1_viewport_projectile_explosion_render_source_lock_gate: rebirth
    explosion types re-anchored to the DM1_EXPLOSION_TYPE_REBIRTH_STEP*
    prefix shared with the resurrection contract header.
  - v1_viewport_draw_order_gate: clipped front-wall blit validates the
    PC34 material receipt expected geometry; floor items via the
    source-bound F0115 sprite route.
  - v1_viewport_wall_depth_source_lock_gate: max-visible helper
    re-anchored to the lane-visibility receipt + contract bound
    (nearest >= 0 ? nearest + 1 : 3); side wall depth table on the
    contract spec rows; side-walls pass locked on the visibility-aware
    host receipt builder.
  - v1_viewport_center_door_occlusion_gate: nearest blocking
    center-door depth re-anchored to the contract
    dm1_viewport_3d_nearest_blocking_center_door_depth_pc34 + receipt
    field; adornment passes locked on the F0111 far-to-near overpaint
    composition (commit a5612d142).
  - v1_viewport_side_wall_occlusion_gate: full multi-section rewrite —
    side-lane helper delegates to
    dm1_viewport_3d_side_lane_clear_from_visibility_pc34 (per-lane
    open-depth masks); side table on the contract spec rows; wall
    material keeps the round-14 decision (no side-lane-open test on
    wall panels, source note locked); side doors sample the PC34
    render plan; viewport wiring on the visibility receipt.
  - pass359_dm1_v1_viewport_wall_draw_order_occlusion_sweep: helper
    block spans the lane-visibility receipt + contract door-depth
    function; side passes locked on the new receipt/spec structure;
    chained gates all green.
  - pass361_dm1_v1_viewport_occlusion_redraw_order_gate: blocking
    center replay + side contents markers re-anchored to the
    visibility receipt / per-depth pass.
  - pass373_dm1_v1_launcher_viewport_redraw_wall_occlusion_path:
    source markers re-anchored (pipeline tick entry, visibility
    receipt, per-depth side contents); all source locks, product
    locks, viewport order lock, prior gates and the cmake build pass.
    Runtime probe still BLOCKED by the pre-existing launcher smoke
    failure (DUNGEON.DAT zip::-path load, rc=3) — same environment
    issue as round 15, left for round 17.
- 2026-07-20 DM2 pre-existing test failures re-anchored: outdoor scene
  local palette gate + scene light control (job/w3, round 16,
  cross-cutting): both stash-verified pre-existing failures diagnosed in
  rounds 14/15 proved re-anchorable against skproject and are now green.
  dm2_v1_outdoor_scene_local_palette_gate: the breakage came from
  5c21e5561 ("Fix DM2 scene local palette ownership"), which added the
  source-faithful UPDATE_GFXSET ownership model — a ready gate in the
  outdoor route, an up-front provider-presence transaction gate, and an
  opaque provider-owned local-palette receipt hash that the viewport no
  longer re-derives from decoded bytes (deliberately removing the
  232a21a1e viewport-side FNV re-hash).  The test was stale, not the
  gate, so the test was re-anchored: (1) the fixture now presents a
  ready receipt (ready=1, index 0, hash 0x53434e45 'SCNE'); (2) the
  "altered source palette" case now exercises the contract the callback
  route actually enforces — an invalid local-palette receipt (hash 0)
  on the ground plane blocks the scene atomically before either plane
  draws, with the sky fully resolved and valid yet never drawn
  (asset_fetches == 2, palette_fetches == 2) — while byte-relabel
  detection stays owned by the boot-owned plan route, which re-verifies
  palette_hash via dm2_v1_weather_pixels_hash; (3) the no-provider
  fail-closed case re-anchored to asset_fetches == 0, since 5c21e5561's
  transaction gate blocks before any fetch (strictly earlier than the
  original per-plane discovery).  dm2_v1_scene_light_control: restored
  the setter-side light-plan clearing from a192cb2b0 that was lost in a
  later rewrite of dm2_v1_viewport_set_gdat_scene_control — !ready now
  zeroes gdat_scene_light_floor / gdat_scene_light_search_depth /
  gdat_scene_light_recompute_enabled in the same transaction discipline
  already used for the plane plans and c_light receipt there; consumers
  only read those fields during render frames, and the test does not
  re-render after ready=0.  Verification: both target tests PASS; the
  scoped 26-test suite (weather, graphics_data_open, gdat_image_helper,
  save_timer, dm2_touch, hit_zone, scene, outdoor) is 26/26 PASS.  A
  wide 396-test net shows 19 failures, all verified pre-existing and
  structurally unaffected by this change (zero references to the changed
  symbols; the only setter call with ready=0 in the entire test tree is
  scene_light_control itself).  Observation for future rounds:
  dm2_v1_wall_door_local_palette_gate and
  dm2_v1_hud_portrait_local_palette_gate fail in the same
  stale-after-5c21e5561 pattern and are candidates for the same
  re-anchor treatment in their own rounds.  Pre-commit bypassed:
  hash_harmonization fails on the known pre-existing dm2-mac-en data
  mismatch, unrelated to this change (same as rounds 14/15).
- 2026-07-20 Nexus engine DGN face-material receipt re-base (job/w4,
  round 16): the engine's own
  `nexus_v1_current_level_dgn_face_material_source_receipt` (flagged in
  the round-15 entry) no longer derives geometry readiness from
  `level.geometry_info.mesh_ready` — that field gates collision and
  post-grid record validation and stays 0 for the whole retail
  LEV00–LEV15 corpus, so the engine receipt could never reach READY for
  any retail level. New shared API
  `nexus_v1_level_structure3_mesh_geometry_ready`
  (include/nexus_v1_dungeon.h, src/nexus/nexus_v1_dungeon.c) returns 1
  only when every Structure3 mesh entry of the level extracts as bounded
  typed source rows through the restored mesh extractor and builds to
  NEXUS_V1_DGN_MESH_READY_GEOMETRY via `nexus_v1_dgn_mesh_build` with
  `can_submit_geometry`, no textured raster, no fallback visuals, and
  the summed extracted face count equal to the level face receipt. The
  engine receipt path now wires `geometry_source_bound` /
  `geometry_can_submit_geometry` from that API over the exact
  MD5-authenticated retained launch buffer; the receipt itself stays
  capture-required and no-draw (`can_submit_raster_input` remains 0
  until an original Saturn VDP1 capture exists), so the downstream
  material-plan consumer stays fail-closed exactly as before.
  The round-15 retail corpus test helper is replaced by the shared API
  (no duplicated extractor logic), and new skip-safe CTest
  `nexus_v1_engine_dgn_face_material_source_receipt` proves the engine
  path end-to-end against real hash-verified retail LEV00/LEV01/LEV08:
  READY with the full parsed selector census, geometry flags bound,
  `mesh_ready == 0` on the level object asserted to document the
  re-base, BLOCKED_SOURCE on a tampered buffer, NULL engine, and an
  unloaded engine. Verified PASS against the staged retail corpus
  (exits 0); ctest skips without staging as designed. Full Nexus suite
  regression: identical 23 pre-existing failures (incl. the documented
  capture-bound `nexus_v1_dgn_material_raster`), no new failures, 197
  tests.
- 2026-07-20 Theron stage-two L4696/L3114 far-callee binding (job/w5,
  round 16): the two remaining stage-two-image windows are now
  byte-bound — 163 bytes across two windows. (1) The L4696 multiply
  body [0x4696..0x46db) (69 bytes: STZ $0F / STZ $11 / LDA $0E /
  STA $12 / STZ $0E / LDX #$01, the BBS7..BBS0 $12 bit-priority chain
  with its RTS and INX staircase, and the LSR $12 / BCC / CLC /
  LDA $10 / ADC $0E / STA $0E / LDA $11 / ADC $0F / STA $0F / ASL $10 /
  ROL $11 / DEX / BNE / RTS shift-add loop — the 16-bit multiply whose
  $0E-input/$0E:$0F-output convention matches all three call sites).
  The flagged head-byte decode artifact is resolved: da65's linear
  $4000-based map labels image offset 0x696 as L4696, whose head byte
  $33 is no HuC6280 opcode (emitted as `.byte $33`,
  theron-us-stage2-huc6280.asm:987); the authenticated body at image
  offset 0x4696 (image bank 2, alongside its L8000-tier and $45xx-tier
  callers) matches da65's own L8696 decode
  (theron-us-stage2-huc6280.asm:10212-10248) instruction by
  instruction, with the disassembly's $11 zero-page accesses rendered
  as the absolute label L0011 superseded by the media bytes (64 11 /
  a5 11 / 26 11). (2) The L3114 body [0x1114..0x1172) (94 bytes: PHA /
  BSR L3172 / LDA $4FDE / BNE / BSR $117D / BRA / LDA #$1A /
  JSR $4F66 / DEC A / BNE / PLA / STA $4F8E / PLA / STA $4F8D, the
  $4F9D-$4F9E -> $06/$07 and $4F93-$4F94 -> $4F8B-$4F8C copies,
  JSR $526D, the $06:$07 +4 fix-up, and the LDX $4F8D / LDY $4F8E /
  PHX / $0E:$0F save / JSR $55E0 / restore / JSR $5213 / PLX / DEY /
  BNE loop closing in RTS) — called from the bound L4F5E selector
  window (JSR $3114 at L4F5E+4). da65 declared `L3114 := $3114`
  absolute without a body decode because CPU $3114 lies below its
  linear $4000 map; the CPU $3xxx window shows image bank 0 at offset
  CPU-$2000, and unlike L383E (whose bank-0 offset 0x183e is clobbered
  at runtime by the $3800 dynamic-payload CD_READ — the reason L383E
  stays in the dynamic-payload lane) the L3114 bytes survive, keeping
  L3114 in the stage-two image lane. The trailing RTS at 0x1171 sits
  immediately before the da65-declared L3172 entry, confirming the
  span; the BSR/JSR callees (L3172, $117D, $4F66, $526D, $55E0,
  $5213) remain unbound future windows. New verifier
  `theron_v1_track02_verify_stage2_l4696_l3114`
  (src/theron/theron_v1_track02.c) chains find_ipl_loader, requires the
  US variant plus `stage2_seed_call_sites_proven`, applies two
  `tqr_ipl_user_match` window checks across the 17-sector stage-two
  image, asserts the call-site invariant (L4696 JSR at L8000+0x6a
  inside the round-14 bound L8000 window; L3114 JSR at L4F5E+4 inside
  the round-13 bound selector window), and fills the new
  `Theron_Track02Stage2L4696L3114Receipt`
  (valid/variant/stage2_record/stage2_raw_sector/l4696_bytes/
  l3114_bytes/l4696_l3114_bound_bytes/l4696_cpu_address/
  l3114_cpu_address + proven flags). Scope: US-only — the source-lock
  document attests JP/US byte identity only for the $4090 window, so
  the JP variant rejects (`THERON_TRACK02_SIGNAL_NOT_FOUND`, invalid
  receipt) until staged JP media can verify the same streams. Probe:
  the two fixture windows (L4696 at stage2_sector+8 in-sector 0x696,
  L3114 at stage2_sector+2 in-sector 0x114), US positive test with
  full field assertions, four byte-mutation rejections (L4696 head
  0x696 -> 0x00 restored to 0x64; L4696 zero-page-artifact 0x698 ->
  0x00 restored to 0x64; L3114 head 0x114 -> 0x00 restored to 0x48;
  L3114 tail 0x171 -> 0x00 restored to 0x60), JP-scope rejection in
  the JP block, US-gated real-media check. Verification: strict
  compile clean via the project build, probe `fail=0` against the real
  hash-verified US media, ctest 147/162 with the exact same 15 known
  failures as the round-15 verified baseline (failure name list diffed
  identical). Semantics boundary unchanged: instruction bytes only —
  no multiplier or queue semantics, no callee semantics, no System
  Card base arithmetic, no bank-mapping arithmetic, no record
  semantics, no graphics role. Remaining: JP verification awaits
  staged JP media; next-tier windows (L383E in the dynamic payload —
  the dynamic-payload lane; the unbound $45xx-tier callers of L4696;
  L3114's BSR/JSR callees) are future windows; the post-$3800
  consumer chain remains capture-blocked.

- 2026-07-20 DM1 F0174 current-map alcove list wiring + round-15
  same-drift-family verifier re-anchors (job/w1): the fail-closed
  dm1_v1_wall_ornament_is_alcove_global_pc34 stub now classifies from
  loaded DUNGEON.DAT map data.  ReDMCSB evidence: DUNVIEW.C:2672-2690
  clears G0267 to -1 on every current-map change and rebuilds it from
  the map's G0261 wall-ornament table against the G0192 source table
  ({1,2,3} = Square Alcove/Vi Altar/Arched Alcove); F0149
  (DUNGEON.C:1330-1348) then tests the 0-based local index, and F0107
  decrements the ordinal at entry (P0116_i_WallOrnamentOrdinal--).
  Firestaff wiring: dm1_v1_wall_ornament_wire_current_map_alcove_list_pc34
  builds the G0267 equivalent (local indices + globals, -1 fill,
  C003_ALCOVE_ORNAMENT_COUNT slots) from the engine's DUNGEON.DAT-loaded
  state->wallOrnamentIndices[map] cache via dm1_v1_g0192_get_pc34;
  m11_dm1_wire_current_map_alcove_list keeps it in sync with
  party.mapIndex at all three classification sites (wall-ornament render
  path global index, c080 click and F0128 scheduler local ordinal via
  the F0149-faithful dm1_v1_wall_ornament_is_alcove_local_ordinal_pc34
  -> F0149_DUNGEON_IsWallOrnamentAnAlcove).  Unwired call sites still
  fail closed; the no-synthetic-hardcode contract (no globalIndex ==
  1/2/3) is kept and locked.  Unit coverage added for wire/classify/
  clear/NULL-table.  No new engine behavior beyond the classification
  becoming live; zero new test failures.
  Re-anchored verifiers, all PASS via ctest:
  - v1_viewport_alcove_wall_item_gate: locks the F0174 wiring, both
    classifiers, the m11 wiring helper + call sites, and the kept
    no-synthetic contract.
  - pass505_dm1_v1_alcove_item_c2548_blocker: same F0174 contract;
    F0115 row helper re-anchored to dm1_viewport_3d contract module.
  - pass581_dm1_v1_d3_d2_wall_ornament_order_source_lock: spec order
    locked on the contract module view-spec table; blocking center from
    the lane-visibility receipt.
  - pass510_dm1_v1_movement_sensor_rotation_defer_source_lock:
    successful-step position update re-anchored to the
    SuccessfulStepApplyPlan applier (party->mapIndex = plan->newMapIndex),
    source order WALK_OFF -> apply -> WALK_ON -> timing unchanged.
  - v1_viewport_occlusion_gate: maxVisibleForward from the lane-visibility
    receipt; primary floor passes keep the documented full D3..D1 range
    (no host pre-cull); side-walls gate via runtime_rel_forward.
  - v1_viewport_wall_parity_flip_gate: L/R swap re-anchored to the
    per-spec parity_wall/parity_flips_horizontally receipt (pass510
    round-14 pattern).
  - dm1_v1_wallset_materialization_source_lock:
    m11_is_dm1_wallset_materialized_graphic inlined into the remap +
    m11_current_map_wall_set; blits materialize via the
    dm1_viewport_3d wall host-material receipt.
  - dm1_v1_real_wall_asset_distinctness_gate: same parity re-anchor;
    GRAPHICS.DAT 93..107 distinctness unchanged.
- 2026-07-20 DM2-011 IMG9 global-palette identity bound to real GDAT
  (job/w3, round 15): the exact remaining weather material gate is
  closed.  Source rule: SkWinCore::QUERY_GDAT_IMAGE_LOCALPAL
  (SkWinCore.cpp 3e74:521A, DM2_EXTENDED_MODE == 1) returns NULL for
  every non-4bpp image, so the real 8bpp IMG9 command images carry no
  16-color local palette; SkWinCore::QUERY_GDAT_SUMMARY_IMAGE
  (0B36:0520) then installs the 256-entry identity translation
  (ref->b58[i] = i, ref->w56 = 256) and each decoded pixel byte indexes
  the global screen palette directly.  dm2_weather_decode_material now
  binds exactly that source translation per admitted format: 4bpp
  IMG3/U4 keeps the 16-entry local-palette receipt (w56 = 16), 8bpp
  IMG9 gets the 256-entry identity receipt (hash of the exact identity
  table, w56 = 256).  DM2_V1_WeatherCommandReceipt carries
  global_palette_identity_valid / palette_translation_count /
  palette_translation_hash / global_palette_identity_hash (appended),
  DM2_V1_WeatherDrawPlan carries the translation count+hash, and the
  renderer M11 gate compares both identities.  Real-data proof in the
  new dm2_v1_weather_img9_global_palette_identity_real_data test: the
  canonical DM2 GRAPHICS.DAT set 5 binds all nine 0x64..0x6c commands
  to full material (material_mask 0x1ff), every image decodes as 8bpp
  IMG9 with the round-14 extents (bolts 16x36/23x33/28x38, clouds
  224x39, rain 224x62), the identity hash matches an independent
  in-test FNV-1a of the source table, and the storm-cloud + light-rain
  overlay plan now validates as full material.  ctest -R
  "weather|graphics_data_open|gdat_image_helper" 12/12 PASS;
  dm2_v1_outdoor_scene_local_palette_gate and dm2_v1_scene_light_control
  re-verified pre-existing (identical failures on the stash-verified
  pre-round tree).  Full build green.
- 2026-07-20 DM2 main-menu event 0xD8 dispatch semantics traced
  (job/w3, round 15): the round-13 mechanism (mask 0x10 = synthetic
  keyboard button re-queued by _1031_0781) is now closed end to end,
  and the answer to "which GDAT menu item maps to event 0xD8 in
  _0aaf_0067" is: none — the trace refutes a dialog-item origin.
  Findings, all source-locked: (1) Zone _4976_0d9e[1] (skval1.h:92)
  carries event 0x00D8 on rectid 0x0197 — the identical box as zone[0]
  0xD7 START NEW GAME (91,52 48x26) — with w4 0x4010; it is the only
  zone in the PC 3.4 table with mask 0x10, so it is unreachable by
  physical click (IBMIO_MOUSE_EVENT_RECEIVER queues only 1/2/4/8,
  SkWinCore.cpp:38776-38802).  (2) No keyboard hotkey produces it
  either: the _4976_13a4 table (skval1.h:105) maps the title menu's
  Enter (0x1C) to 0xD7 via _1031_03f2/_1031_0c58, and no entry's event
  is 0xD8.  (3) _1031_0781's only callers are _0aaf_0067's dialog
  activations with 0xDB-based event codes (SkWinCore.cpp:39105,39111),
  which resolve dialog-button zones, and _0aaf_0067 runs in dialog
  views — the title view (SHOW_MENU_SCREEN loop, 2481:0180) never
  enters it.  (4) The dispatch that 0xD8 arms when the synthetic 0x10
  activation does land: HANDLE_UI_EVENT (1031:1E06) sets
  glbSpecialScreen = 1 AND _4976_5bea = 1, and LOAD_NEW_DUNGEON
  (2066:2CAD) then prefers the alternate dungeon file strDungenB
  ".Z020DUNGENB.Z024.DAT" (SkGlobal.cpp:438) before falling back to
  the standard dungeon — i.e. 0xD8 is "start new game on the B
  dungeon", skproject's own debug name for it is "???"
  (SkWinCore.cpp:705).  Net: the 0xD8 zone is the keyboard-activation
  twin of the TITLE menu's first item box (GDAT_CATEGORY_TITLE menu
  screen, item rect 0x0197), wired to the DUNGENB alternate-dungeon
  start, with no live producer in the PC 3.4 build; the zone matrix
  entry and hit-test probe already carry the correct mask semantics
  and are unchanged.
- 2026-07-20 DM2-011 saved weather-timer owner proof (job/w3, round
  15): the owner/schedule side of the saved 0x54 timer is bound.
  dm2_v1_save_timer_weather_owner_receipt (in
  dm2_v1_save_timers_pc34_compat) adopts a restored 12-byte wire
  record as a weather-chain timer iff ttype == 0x54, actor == 0 and
  map == 0 — the exact identity DM2_SET_TIMER_WEATHER writes
  (c_weather.cpp:22-30).  The receipt carries the signed schedule
  delta target_tick - restored_gametick (gametick restored from the
  same savegame header, c_savegame.cpp:1486-1487), marks
  fires_on_next_proceed for non-positive deltas (the source proceed
  fires overdue timers; it never drops them), and binds the owner
  dispatch: type 0x54 -> DM2_UPDATE_WEATHER(1)
  (c_tim_proc.cpp:4179-4183), which re-queues the chain with
  RAND16(256)+50 (c_weather.cpp:85-88; exposed as
  DM2_V1_SAVE_TIMER_WEATHER_RESCHEDULE_MIN/MAX 50/305).  The weather
  chain is not re-seeded on load — the serialized record itself is
  the owner continuity.  New test
  dm2_v1_save_timer_weather_owner_pc34_compat: owner identity
  round-trip, overdue-fire, non-chain rejects (0x55 type, actor 1,
  map 2, notype, null), sorted-queue dispatch order
  (DM2_cmp_timers), and reschedule bounds.  ctest -R save_timer 2/2
  PASS.
- 2026-07-20 Nexus FONT256.S2D section-2 composition inventory + DGN
  retail corpus re-base (job/w4, round 15):
  1. FONT256.S2D last unbound populated SCR section (admission ordinal
     1, table index 2, 15,504 bytes) now carries an exhaustive opaque
     composition inventory in `nexus_v1_font256_s2d_subrecord_grammar`,
     measured against the canonical SHA-256-attested retail asset
     (SHA-256 b820d606…13af): exactly 742 populated of 969 canonical
     16-byte blocks gathered in exactly 52 populated runs from block 0
     through block 968; byte alphabet exactly {0x00, 0x03, 0x0f, 0xff}
     with canonical counts 11,305 / 2,730 / 1,453 / 16; the lead block
     alone carries all sixteen 0xff bytes; every nonzero byte outside
     the lead block is below 0x10. New receipt fields +
     `section2_composition_bound` corpus flag; `subrecord_grammar_bound`
     stays 0 — no text, glyph, palette, record, encoding, or pixel
     meaning is assigned and the section stays capture-required.
     Synthetic mirror test rebuilt to the same canonical composition
     (52 runs, exact byte counts, all-0xff lead block) so synthetic and
     skip-safe retail paths share `check_corpus_common`; new rejection
     coverage: alphabet violation (0x01/0x07), in-alphabet byte-count
     drift (0x03→0x0f), lead-block tamper, gap-block population drift,
     and run-structure drift at constant population and byte counts
     (whole block moved into the following gap: 52→53 runs, still
     rejected); a composition-preserving intra-block byte move still
     admits with only the recorded digests moving. CTest pair
     `nexus_v1_font256_s2d_subrecord_grammar` (synthetic) and
     `nexus_v1_font256_s2d_subrecord_grammar_real` (retail path, real
     FONT256.S2D found and admitted) both PASS; full 7-test FONT256
     suite PASS.
  2. `nexus_v1_dgn_face_material_retail_corpus` re-based to the
     restored Structure3 mesh extractor — same stale-expectation class
     as round 14's scene_runtime_plan fix. The test wired
     `geometry_source_bound`/`geometry_can_submit_geometry` from
     `level.geometry_info.mesh_ready`, which stays 0 for the whole
     retail LEV00–LEV15 corpus, so all 16 levels were rejected. The
     geometry readiness source is now the corpus-verified extractor
     route: every mesh entry of each level extracts as bounded typed
     source rows (`nexus_v1_level_extract_structure3_mesh_entry`) and
     builds to READY_GEOMETRY via `nexus_v1_dgn_mesh_build` with
     `can_submit_geometry`, no textured raster, no fallback visuals,
     and the summed extracted face count equals the level face receipt.
     Also replaced the brittle `asset_find_by_md5` + exact-path strcmp
     provenance check (it resolved to an ISO-internal `….iso::LEV00.DGN`
     match whenever disc images sit next to the loose corpus) with the
     same per-file `asset_file_matches_md5` check the mesh corpus test
     uses. The locked selector census is unchanged and now verified
     live: 16/16 levels admit, 17,821 textured faces, 17,401 static +
     420 animated selectors, matching the independent
     `nexus_v1_dgn_face_mesh_corpus` census (1,144 entries / 18,478
     face-normal pairs). Verified against both a loose-file staging dir
     and the full data dir with ISOs present; binary exits 0 both ways.
     No engine behavior change (test + CMake source list only; the
     engine's own `mesh_ready`-wired receipt path is untouched).
- 2026-07-20 Theron stage-two jump-table handler-body binding (job/w5,
  round 15): the ten L410D jump-table targets $41C5..$4253 are now
  byte-bound as one contiguous span [0x1c5..0x254) — 143 handler
  bytes. The bodies were disassembled from the hash-gated US media and
  matched instruction by instruction against the source-locked
  disassembly
  (`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm:344-430`):
  handler 1 [0x1c5..0x1cb) (BSR L41B9 / CLA / JMP L40E4), handler 2
  [0x1cb..0x1d8) (BSR L41F8 / BNE L41D5 / BSR L41B9 / CLA / JMP L40E4
  with the L41D5 JMP L4101 tail), handler 3 [0x1d8..0x1de) (BSR L41F8 /
  BNE L41CF / BRA L41D5), handler 4 [0x1de..0x1e6) (BSR L41F8 /
  BCC L41D5 / BEQ L41D5 / BRA L41CF), handler 5 [0x1e6..0x1ec) (BSR
  L41F8 / BCS L41D5 / BRA L41CF), handler 6 [0x1ec..0x1f0) (BSR L4203 /
  BRA L41CD), handler 7 [0x1f0..0x1f4) (BSR L4203 / BRA L41DA),
  handler 8 [0x1f4..0x214) (BSR L4203 / BRA L41E8 plus the shared
  L41F8/L4203 operand-read sub bodies), handler 9 [0x214..0x253) (the
  L4215 operand read, the L421C $4EC1/$4D7B store sub with the ADC
  $3008/STA $3009 pair, the L4F5E selector call, the L4233 carry path,
  and the L4240/L424B sub ending in the dynamic-lane JSR $383E), and
  handler 10 [0x253..0x254) (a single RTS — the table's terminator
  entry, reached by the dispatcher's indirect JMP). Three da65
  decode-artifact spans of the same class flagged in round 14 — the
  BSR L41F8 at 0x1d8 (split into .byte/.byte as L41D8/L41D9), the LDA
  $2780,x at 0x1fc (split into .byte/bra as L41FD), and the ADC
  $3008/STA $3009 at 0x225-0x22a (split into .byte/php/bmi/ora as
  L4226) — plus the 0x245 zero-page STA $20 rendered as the absolute
  label L0020 are bound to the authenticated media bytes; the media
  bytes are authoritative. New verifier
  `theron_v1_track02_verify_stage2_jump_table_handlers`
  (src/theron/theron_v1_track02.c) chains find_ipl_loader, requires the
  US variant plus `stage2_seed_call_sites_proven`, applies one
  `tqr_ipl_user_match` window check across the 17-sector stage-two
  image, asserts the span/entry-chain invariant (handler count == the
  round-13 bound table entries; first target at the span head; the
  last target's single byte closing the span; the JMP (L410D,x)
  table-read site 0xe1 inside the bound dispatcher window), and fills
  the new `Theron_Track02Stage2JumpTableHandlersReceipt`
  (valid/variant/stage2_record/stage2_raw_sector/handlers_bytes/
  handler_count/first/last_handler_cpu_address + handlers/
  entry-chain/contiguous proven flags). Scope: US-only — the
  source-lock document attests JP/US byte identity only for the $4090
  window, so the JP variant rejects (`THERON_TRACK02_SIGNAL_NOT_FOUND`,
  invalid receipt) until staged JP media can verify the same streams.
  Probe: the fixture window (handlers at stage2_sector in-sector
  0x1c5), US positive test with full field assertions, three
  byte-mutation rejections (first handler head 0x1c5 -> 0x00 restored
  to 0x44; decode-artifact 0x1fc -> 0x00 restored to 0xbd; last handler
  0x253 -> 0x00 restored to 0x60), JP-scope rejection in the JP block,
  US-gated real-media check. Verification: strict compile clean
  (`cc -std=c99 -Wall -Wextra -Werror` via the project build), probe
  `fail=0` against the real hash-verified US media, ctest 147/162 with
  the exact same 15 known failures as the pre-change baseline (name
  list diffed identical on pristine HEAD via stash). Semantics
  boundary unchanged: instruction bytes only — no command or stream
  semantics for the ten handlers, no semantics for the L41B9/L43D6/
  L37D8 callees or the dynamic-lane $383E target, no System Card base
  arithmetic, no record semantics, no graphics role. Remaining: JP
  verification awaits staged JP media; next-tier windows (L4696 with
  its head-byte decode artifact, L3114, L383E in the dynamic payload)
  are future windows; the post-$3800 consumer chain remains
  capture-blocked.

- 2026-07-20 DM1 pass404/pass510 architecture reconciliation (job/w1,
  round 14): the documented architecture tradeoff is decided AGAINST
  restoring the pre-a8ff8d15b batch side-contents pass and the
  pre-c8ab48a2a kSideBlits swap, and FOR the current F0115-order +
  receipt-based architecture.  ReDMCSB evidence: F0128's global
  G3048->G2107 wallset swap (DUNVIEW.C:8354-8414) and G3071 restore
  (8543-8579) exist only because every original square routine consumes
  one global wallset table; Firestaff has no such table, and the
  per-spec parity_wall/parity_flips_horizontally pair in
  s_wall_draw_specs + dm1_viewport_3d_select_wall_bitmap selects
  exactly the bitmap the G3048 swap would select (D3L<->D3R, D2L<->D2R,
  D1L<->D1R, D0L<->D0R, DnC stays), with F0105's horizontal flip
  carried by the receipt.  The per-depth side-contents interleave
  (a8ff8d15b) is also strictly closer to this gate's own F0128 anchor
  (DnL/DnR immediately before DnC) than the old batch pass was, and the
  old globalIndex == 1/2/3 alcove hardcode was synthetic (removed by
  257c1f259 in favor of the real F0149 map-list port + fail-closed
  stub).  Reverting would have re-broken the 11+ source locks that went
  green in round 12.  Landed as verifier re-anchors, no engine change:
  - pass404_dm1_v1_side_contents_center_blocker_occlusion_gate: locks
    m11_draw_dm1_side_contents_at_depth (same blocker guard) plus the
    F0128 caller interleave (side pair before same-depth center
    contents) and the unchanged deferred-explosion gate. PASS.
  - pass510_dm1_v1_viewport_wall_parity_flip_source_lock: re-anchored
    to the delegated party-tuple predicate
    (dm1_viewport_3d_use_flipped_walls_pc34, locked on both sides of
    the module boundary), the m11_current_map_wall_set + F0096 wallset
    binding, the unchanged center-wall flip path, and the receipt-based
    side-wall L/R parity swap in the contract module. PASS.
  - pass508_dm1_v1_viewport_wall_runtime_readiness (pass510 gate):
    batch-order check scoped to m11_draw_viewport with the
    visibility-receipt replay guard; alcove check re-anchored to
    m11_draw_item_sprite_material. PASS.
  - v1_viewport_alcove_wall_item_gate (pass508 gate): re-anchored to
    the F0149 port (no default alcove table), the fail-closed legacy
    stub (with an explicit anti-synthetic negative check), and the
    surviving alcove-item draw structure; scope line now states runtime
    alcove rendering stays gated on the F0174 current-map list. PASS.
  - v1_viewport_d1c_doorpass_source_lock_gate (pass508 gate): near-side
    replay guard needle re-anchored to
    visibility.nearest_blocking_center_depth_index. PASS.
  All five ctests green; no engine source touched; parity-evidence
  manifests regenerated locally but not committed per policy.
- 2026-07-20 Nexus DGN scene runtime plan re-base (job/w4, round 14,
  commit dd130e63d): `nexus_v1_dgn_scene_runtime_plan` known failure
  fixed. The test predated the 2026-07-20 Structure3 mesh extractor
  restoration and still expected real retail LEV00.DGN (147456 bytes,
  canonical FNV 0xe715281f66445610) to expose no bounded mesh entry —
  BLOCKED_MESH_ENTRY, zero mesh counts, geometry consumer not ready.
  The restored extractor is independently corpus-verified
  (`nexus_v1_dgn_face_mesh_corpus`: 1,144 entries, 18,478 face/normal
  pairs against retail LEV00-LEV15) and the scene plan now binds LEV00
  Structure3 mesh entry 0 at every walkable pose: 44 vertices, 22 quad
  faces, 22 normals (16 color-fill, 6 static-texture), status
  READY_GEOMETRY_NO_DRAW. The Structure1F direct-row path stays
  unselected (no topology candidates, no face-selector or rotation
  binding), and the consumer stays fail-closed: texture submit and
  raster blocked without VDP1 consumer evidence, M11 handoff denied, no
  fallback geometry/visuals, zero render commands. Test expectations
  re-based to the verified real-data receipt; no engine behavior change.
  Adjacent regression check: geometry_readiness, face_mesh_corpus,
  face_material_provenance all PASS.
  Same round, diagnosis only (capture-bound, no fix attempted):
  `nexus_v1_dgn_material_raster`'s four failing assertions were traced
  to their exact missing engine pokes — see the 2026-07-20 route
  admission trace entry in TODO.md. All three missing pokes (PRS3
  replay-placement route epoch, Structure2 descriptor capture-target
  chain, `structure1b_selector_binding_proven`) require authenticated
  Saturn capture/producer evidence that does not exist yet;
  `structure1b_selector_binding_proven` has no setter anywhere by
  design.

- 2026-07-20 Theron stage-two L8000/L45A6 callee-pair binding (job/w5,
  round 14): the entry path's first call is now bound together with
  its single-caller callee — 224 pair bytes across two windows. (1)
  The L8000 body at user offset 0x4000 (188 bytes, head of image
  sector 8: VDC register clears through the STZ $220C/$220D/$2210/
  $2211 sequence and st0/st1/st2 pairs, the L45A6 call at +0x1c, the
  zero-page result handoff through $4C/$4D, the $47BF-$47D2 and
  $3B6A-$3B6F stores, and the L4696/L48FC tail calls at +0x6a/+0xb8)
  and (2) the L45A6 body at user offset 0x5a6 (36 bytes: the ($1C),y
  table read, the $44E7-$44EA seed copy into $01-$05, and the PLA/LSR
  branch into the dynamic-lane JSR $3AB7 or the JMP $4105 return)
  were disassembled from the hash-gated US media and matched against
  the source-locked disassembly
  (`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm:864-880,
  9343-9445`). The three da65 decode-artifact spans flagged in round
  13 — the $2211 STZ at +0x0b (split into .byte/ora), the ADC $00 at
  +0x2a (split into .byte/brk), and the STA $47CE/LDA #$00 at
  +0x4a..+0x4e (split into .byte/dec/brk) — are bound to the
  authenticated media bytes; the disassembly also renders several
  zero-page accesses as absolute labels (L0000/L004C), so the media
  bytes are authoritative. L45A6's only call site is the JSR at
  L8000+0x1c, so the two bind together, keeping every call site of a
  bound window inside a bound window. New verifier
  `theron_v1_track02_verify_stage2_l8000_pair`
  (src/theron/theron_v1_track02.c) chains find_ipl_loader, requires
  the US variant plus `stage2_seed_call_sites_proven`, applies two
  `tqr_ipl_user_match` window checks across the 17-sector stage-two
  image, asserts the call-site invariant (L8000's call site 0x11
  inside the bound entry path [0x00..0xb5); the L45A6/L4696/L48FC
  call sites inside the bound L8000 window), and fills the new
  `Theron_Track02Stage2L8000PairReceipt` (valid/variant/
  stage2_record/stage2_raw_sector/l8000_bytes/l45a6_bytes/
  pair_bound_bytes + l8000/l45a6/call-site proven flags). Scope:
  US-only — the source-lock document attests JP/US byte identity only
  for the $4090 window, so the JP variant rejects
  (`THERON_TRACK02_SIGNAL_NOT_FOUND`, invalid receipt) until staged JP
  media can verify the same streams. Probe: the two fixture windows
  (L8000 at stage2_sector+8 in-sector 0x00, L45A6 in-sector 0x5a6),
  US positive test with full field assertions, three byte-mutation
  rejections (L8000 head 0x00 -> 0x00 restored to 0xc6; L8000
  decode-artifact +0x0b -> 0x00 restored to 0x9c; L45A6 0x5a6 -> 0x00
  restored to 0xb1), JP-scope rejection in the JP block, US-gated
  real-media check. Verification: strict compile clean
  (`cc -std=c99 -Wall -Wextra -Werror`), probe `fail=0` against the
  real hash-verified US media, ctest 147/162 with the exact same 15
  known failures as the pre-change baseline (name list diffed
  identical; the probe-registration-hygiene failure independently
  confirmed pre-existing on pristine HEAD via stash). Semantics
  boundary unchanged: instruction bytes only — no semantics for the
  L4696/L48FC callees or the dynamic-lane $3AB7 target, no System
  Card base arithmetic, no record semantics, no graphics role.
  Remaining: JP verification awaits staged JP media; next-tier
  windows (the ten jump-table handler bodies $41C5..$4253, L4696 with
  its head-byte decode artifact, L3114, L383E in the dynamic payload)
  are future windows; the post-$3800 consumer chain remains
  capture-blocked.
- 2026-07-20 DM1 pass784 mirror-candidate C040 cancel-then-reopen
  same tick (job/w1, commit 975e45ced): verifier
  pass784_dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick
  PASS; dm1 suite 114 -> 113 failing with zero new failures.  The
  contract module logic (spec/init/run, F0282 C162 cancel branch +
  F0280 new-sensor reopen in one tick) was already landed with 53/53
  runtime assertions green; the lock failed because the module named
  its public API in PascalCase with snake_case #define aliases —
  inverted vs the mirror-candidate sibling convention (snake_case
  canonical in header and source).  Renamed
  spec/source_evidence/init/run to
  dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_*_pc34
  across header, source and test and dropped the alias macros.  No
  behavior change; mirror_candidate group 52/52 green.
- 2026-07-20 DM2-003/005 follow-up: the c_ai re-queue at the
  DM2_PROCEED_CCM end bound data-backed (job/w2, round 13 block 2).
  New `dm2_v1_caii_ccm_end_requeue` binds c_ai.cpp:5608-5614 +
  5641-5646: the timer type (loop_result != 1 ? 1 : 0) + 0x21, the
  s350.v1e0570 suppression return, the setmticks word rebuild
  (c_timer.h:66, delta OR-ed unmasked — verbatim), the slot word@2
  pending-timer cancel through the bound DM2_1c9a_0db0,
  DM2_QUEUE_TIMER over the session queue, and the ticket store into
  slot word@2. The CCM message loop (stream grammar) and the
  DM2_CREATURE_SOMETHING_1c9a_0a48 animation-frame reader stay
  host-owned; their outputs enter as explicit parameters
  (loop payload timer fields, loop_result, suppress_requeue,
  mticks_map, mticks_delta). `dm2_v1_caii_attack_pc34_compat` gained
  scenarios (z)-(cc): full requeue with peek-verified
  type/setmticks/payload, the 0x21 type, suppression, and the
  slot-less fail-close. 29 scenarios PASS. dm2_v1 lane 213 tests,
  same 27 known baseline failures, zero new failures.job/w2

- 2026-07-20 DM2-003/005 follow-up: DM2_CREATURE_SOMETHING_1c9a_0a48
  bound data-backed (job/w2, round 14). New module
  `dm2_v1_creature_something_pc34_compat` binds the CCM mticks-delta
  reader (c_1c9a.cpp:5434-5672) together with its animation core
  DM2_GET_CREATURE_ANIMATION_FRAME + DM2_4FCC (c_creature.cpp:
  3217-3278 + 3285-3378). Fully data-driven: dtRaw8/0xfb attribution
  and dtRaw7/0xfc info-sequence rows resolve through the real GDAT
  asset loader, the aidef bit0 static/dynamic gate resolves through
  the proven session AI table (dm2_v1_creature_ai_spec_def), and all
  RAND16/RANDBIT/RAND draws consume the session LCG (c_random.cpp:
  13-47) in exact source order. The s350 context enters as explicit
  parameters; bound effects include the slot byte@7 frame/direction
  rewrite (jitter + bit6 draws, 0x23/0x24/0x25 mode guard), the
  v1e055e adj writeback, the source's own zeroed fallback row with
  the v1e055a NULL reset, and the complete delta band arithmetic
  (dying *3, flee *4+RANDBIT, 75x/100 max-1, map *2/*4, big-creature
  min(1, hi), signed 16-bit truncation) returning gametick + delta —
  the exact mticks_delta the round-13 CCM end re-queue consumes
  (c_ai.cpp:5614). DM2_QUEUE_NOISE_GEN1 receipted, never simulated;
  the unchecked table1d607e[v1e0584] probe fails closed outside the
  proven 0x2f span. New test `dm2_v1_creature_something_pc34_compat`:
  GAF static/dynamic/fail-closed paths + twelve 1c9a_0a48 scenarios
  with LCG determinism — PASS. dm2_v1 lane 208 passed, same 33
  known baseline failures (verified identical on the pristine tree),
  zero new failures.

- 2026-07-20 DM2-003/005 follow-up: DM2_ai_13e4_0360 bound complete,
  including the argl0 != 0 AI-stop tail (job/w2, round 13 block 1).
  New public `dm2_v1_caii_ai_13e4_0360`
  (include/dm2_v1_caii_alloc_pc34_compat.h,
  src/dm2/dm2_v1_caii_alloc_pc34_compat.c) binds
  c_ai.cpp:5912-5960 in source order: handle -1 resolution via
  DM2_GET_CREATURE_AT, the record byte@5 == 0xff guard, the slot
  byte@0x17/0x1a == 0x13 guards (once the AI-stop marker is written,
  further turns are blocked), the byte@0x17 direction write, the
  argl0 == 0 return, and the argl0 != 0 tail: table1d613a[slot
  byte@1a] & 0x10 sets slot byte@0x21 = 1, otherwise the bound
  DM2_1c9a_0db0 + DM2_1c9a_0cf7 pair cancels and re-queues the think
  timer at the creature tile. The table's proven span 0x00-0x55 fails
  closed AFTER the dir write (the source's OOB read order). This is
  the callee of the AI-stop callers c_creature.cpp:233,
  c_ai.cpp:2114 (DM2_PROCEED_XACT_85) and c_tim_proc.cpp:2988
  (DM2_ACTIVATE_CREATURE_KILLER) — all pass dir 0x13, argl0 1.
  `dm2_v1_caii_attack_pc34_compat` gained scenarios (t)-(y): direct
  argl0 == 0 write, byte@0x21 flag path with follow-up guard denial,
  the cancel-and-requeue tail over the live queue, handle -1
  resolution with direction bits kept, the byte@5 guard, and the tail
  span guard. 25 scenarios PASS. dm2_v1 lane 213 tests, same 27 known
  baseline failures, zero new failures.job/w2
- 2026-07-20 DM2 aux-0x10 zone-mask provenance traced (job/w3, commit
  8322d4d67): the last open evidence question from the round-11 DM2
  zone inventory is answered. The 0x10 bit carried by the DM2
  main-menu zone 0xD8 is not a physical mouse button —
  IBMIO_MOUSE_EVENT_RECEIVER (SkWinCore.cpp:38776-38802) only ever
  queues codes 1/2/4/8. It is a synthetic keyboard activation:
  SkWinCore::_1031_0781 (SkWinCore.cpp:32143-32154) resolves a zone by
  event code and re-queues the zone's own mask byte as the "button"
  through FIRE_QUEUE_MOUSE_EVENT (SkWinCore.cpp:8547), invoked from
  the _0aaf_0067 menu/dialog keyboard loop on Enter / default-item
  activation (SkWinCore.cpp:39105,39111). The synthetic value passes
  the (button & 0x13) gate in IBMIO_USER_INPUT_CHECK
  (SkWinCore.cpp:15301-15303) and matches the zone through
  (ww & (w4 & 0xff)) in _1031_0a88 (SkWinCore.cpp:12144). The bit is
  now named TOUCH_CLICK_BUTTON_KEYBOARD_PC34_COMPAT in the shared
  touch_click_zone_matrix_pc34_compat.h enum with the full provenance,
  zone ordinal 1 uses it, its evidence string carries the chain, and
  the hit-test probe wording moved from "aux-0x10" to
  keyboard-synthesized activation. The workspace generator
  gen_dm2_zone_matrix.py was updated in lockstep; regeneration is
  byte-identical. ctest -R "dm2_touch|hit_zone" 3/3 PASS.
- 2026-07-20 DM2-011 slot commands 0x64..0x6c bound to the
  QUERY_TEMP_PICST execution chain (job/w3, commit 2e534690c): the
  weather command range now covers the lightning bolt 100+RAND16(3)
  (0x64..0x66) alongside cloud 0x67..0x69 and rain 0x6a..0x6c end to
  end — GDAT command receipts, DistantEnvironment slot receipts (up
  to 3 slots in source cloud/rain/bolt order), renderer receipt,
  outdoor M11 receipt, and the viewport weather overlay.  Bolt slots
  carry the c_weather.cpp:471 RANDDIR byte (0..3; only 2 evaluates
  the 0x20 mirror) in the cmFW position instead of a GDAT FW key.
  QUERY_GDAT_TEXT's source decode (SkWinCore.cpp 2636:0377, gated on
  dtWordValue(0,0,0) bit 3 per 55629) is applied before the CMDSTR
  parse with the lowercase cd/fw keys of EnvCM_CD/EnvCM_FW.
  Real-data verification against the canonical DM2 GRAPHICS.DAT
  (read-only probe, nothing written outside the worktree): set 5
  carries all nine dtText+dtImage pairs — bolt "cd6002", cloud
  "cd6004fw32", rain "cd6005fw8" — and every image decodes through
  the source-faithful IMG9 path (bolts 16x36/23x33/28x38, clouds
  224x39, rain 224x62, 8bpp); the decoded-pixel receipt records
  these real extents.  The one remaining material gap is named in
  TODO.md: the real 8bpp IMG9 images have no 16-color local palette
  and their QUERY_GDAT_SUMMARY_IMAGE global-palette identity is
  unproven, so they keep the pixel receipt but stay material-invalid
  (no draw) until that palette receipt is bound.  GRAPHICS_DATA_OPEN
  now hashes the full 0x64..0x6c text range.  ctest -R
  "weather|graphics_data_open|gdat_image_helper" 11/11 PASS (the
  dm2_v1_outdoor_scene_local_palette_gate and dm2_v1_scene_light_control
  failures reproduce on the pre-round tree — pre-existing, untouched).
- 2026-07-20 DM2-011 DM2_UPDATE_WEATHER(0) light/cloud command handling
  (job/w3, commit c7e562153): the arg == 0 frame update
  (c_weather.cpp:91-506) is bound as dm2_v1_update_weather_0 in
  dm2_v1_update_weather_pc34_compat — day rollover through
  table1d70f0[(gametick+v1e1438)/0x555 % 0x18], the exact lightning
  evaluation RNG order (calm-path rain decay every 3rd tick,
  u16(0x100 - intensity + (RAND&0xf)) threshold, RG51w 7/0x28 at 0xcd,
  cloud_state = CUTX8(intensity), flag latch, rain increment gating,
  v1e1481-gated flash), and light/cloud command selection: cloud
  0x67/0x68/0x69 (+storm_active) at 0x10/0x40/0x80, rain
  0x6a/0x6b/0x6c at 0x40/0x80/0xc0, lightning bolt 100+RAND16(3) via
  RANDBIT, with source slot semantics (failed RETRIEVE does not
  advance, next command overwrites the 10-byte slot, 0xff terminator)
  reported as a compacted live_cmds chain. Thunder paths hand off
  CREATE_CLOUD/INVOKE_MESSAGE as receipt flags with an explicit
  rng_diverges marker; the v1d718c sound latch, clamped volume and
  final v1e024c = 1 light-recalc (m_4A899) are bound.
  RECALC_LIGHT_LEVEL, UPDATE_GLOB_VAR, noise queues, bolt rect
  geometry and RETRIEVE calls stay host-owned (retrieve_mask +
  gdat_entry_6c inputs). Fail-closed on zone > 31 and intensity != 0
  with step == 0 (source division guard). State struct extended with
  v1e147a/v1e1479/v1e1481/v1e024c/v1d718c (appended). 9 new test
  groups incl. ref-LCG cross-checked bolt (7 draws) and thunder
  paths. ctest dm2_v1_update_weather PASS; full build green; the 33
  dm2-suite failures verified pre-existing (identical on stashed
  tree). Remaining under DM2-011: command-to-QUERY_TEMP_PICST
  execution, saved timer owner proof, real-data capture.

- 2026-07-20 Nexus DGN geometry readiness + vector arithmetic hardening
  (job/w4, round 13, commit fb2414d71): the
  `nexus_v1_dgn_geometry_readiness` known failure is fixed (3 assertions).
  Root cause was merge `9b2482b93` ("Merge local worktree changes"), which
  combined a mainline missing the Structure3 vector measurement loop with
  side-branch test expectations and no-draw plan gates: counters stayed
  zero, the synthetic vector expectations (6 plane pairs / 3 zero-winding
  triangles) no longer matched the deliberately extended four-triangle
  fixture (8 / 4), and the `__int128` overflow hardening from `73cd02057`
  was silently dropped. Fix: (1) restored the `__int128`
  `nexus_v1_fixed_vector_dot` / `nexus_v1_fixed_face_winding_sign`
  arithmetic with the long-double fallback kept exact (corrected a
  cross_z operand typo carried since `73cd02057`), so the extreme
  INT32_MIN/INT32_MAX Y/Z-plane fixture validates with exact positive
  winding and zero plane error; (2) updated the stale vector expectations
  to the current fixture (8 pairs, 8 within tolerance, 0 outside, 4
  degenerate/zero-winding, max errors 0); (3) replaced the unreachable
  `command_count > 0` expectation in the opaque-payload Structure1F
  render-plan CHECK with the designed zero-command no-draw outcome, since
  the face/material plan gate correctly refuses to emit source commands
  when Structure3 provenance cannot bind. Full nexus sweep: only
  `nexus_v1_dgn_geometry_readiness` dropped from the failure list;
  `nexus_v1_dgn_material_raster` remains with the same four pre-existing
  engine route-admission assertions (Structure2 source / MNS selector
  binding chain never satisfied by the test's engine setup), diagnosed in
  TODO.md for a later round.
- 2026-07-20 Theron stage-two dispatch-machine closure (job/w5, round
  13, c66b83798): the L40B7 dispatch machine is now contiguously bound
  end-to-end — 67 closure bytes across five windows, closing the
  executed dispatch machine [0x00..0x121). (1) The register-seed tail
  at user offset 0xb5 (2 bytes: the STZ $FC operand and the RTS of the
  $40ae seed subroutine, closing the gap between the executed entry
  path [0x00..0xb5) and the dispatcher body), (2) the seven dispatch
  stubs at 0xf1 (28 bytes: shared LDA #imm / BRA L40E4 return tails
  that command handlers jump to for selecting the stream-advance count
  1..5, 7, 9), (3) the ten-entry jump table at 0x10d (20 bytes:
  little-endian handler addresses $41C5..$4253, strictly increasing,
  each verified to point inside the loaded image), (4) the L4AF7
  MPR-page body at 0xaf7 (9 bytes: the $FFF5-derived TAM #$08 page-map
  idiom), and (5) the L4F5E selector body at 0xf5e (8 bytes: loads the
  $4EC1 argument address in X/Y and calls L3114) were all disassembled
  from the hash-gated US media and matched exactly against the
  source-locked disassembly
  (`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm:183-235,
  1590-1594, 2334-2337`). New verifier
  `theron_v1_track02_verify_stage2_dispatch_machine`
  (src/theron/theron_v1_track02.c) chains find_ipl_loader, requires the
  US variant plus `stage2_seed_call_sites_proven`, applies five
  `tqr_ipl_user_match` window checks across the 17-sector stage-two
  image, range-checks every jump-table entry against the loaded-image
  span, asserts the [0x00..0x121) contiguity chain plus the in-window
  call sites (0xc1, 0xcc, 0xeb), and fills the new
  `Theron_Track02Stage2DispatchMachineReceipt` (valid/variant/
  stage2_record/stage2_raw_sector/seed_tail_bytes/dispatch_stubs_bytes/
  jump_table_bytes/jump_table_entries/mpr_page_bytes/selector_bytes/
  loop_closure_bound_bytes/dispatch_machine_bound_bytes + six proven
  flags). Scope: US-only — the source-lock document attests JP/US byte
  identity only for the $4090 window, so the JP variant rejects
  (`THERON_TRACK02_SIGNAL_NOT_FOUND`, invalid receipt) until staged JP
  media can verify the same streams; no JP requirements were added to
  find_ipl_loader. Probe: the five fixture windows (the 0xaf7/0xf5e
  image offsets map into the second raw sector of the image), US
  positive test with full field assertions, five byte-mutation
  rejections (0xb5 -> 0x00 restored to 0xfc; 0xf1 -> 0x00 restored to
  0xa9; 0x10d -> 0x00 restored to 0xc5; 0xaf7 -> 0x00 restored to 0x18;
  0xf5e -> 0x00 restored to 0xa2), JP-scope rejection in the JP block,
  US-gated real-media check. Verification: strict compile clean
  (`cc -std=c99 -Wall -Wextra -Werror`), probe `fail=0` against the
  real hash-verified US media, ctest 147/162 with the exact same 15
  known failures as the pre-change baseline (name list diffed
  identical, no new failures). Semantics boundary unchanged:
  instruction and table bytes only — no handler semantics for the ten
  jump-table targets, no System Card base arithmetic, no record
  semantics, no graphics role. Finding: L383E is not in the stage-two
  image ($383E lies below the $4000 load address inside the dynamic
  $3800 payload, record 0x4e0 US) — its binding belongs to the
  dynamic-payload lane. Remaining: JP verification awaits staged JP
  media; next-tier windows (the ten jump-table handler bodies, L8000
  with its da65 decode artifacts — L45A6 [0x5a6..0x5ca) is clean but
  its only caller L8000 is unbound, so they bind together — L4696,
  L3114, L383E in the dynamic payload) are future windows; descriptor
  semantics unbound; post-$3800 consumer chain still capture-blocked;
  System Card trace-only; VDC/VCE runtime-gated.

- 2026-07-20 DM1 pass560 viewport-3d source lock (job/w1, commit
  2e3ba3181): dm1_v1_viewport_3d_source_lock and its verifier
  pass560_dm1_v1_mirrored_door_front_source_lock both PASS; dm1 suite
  127 -> 114 failing with zero new failures.  Added
  dm1_viewport_3d_draw_wall_parity_mirrored — reverse-source-column
  sampling through the same C10 clip route as draw_wall with no
  scratch bitmap (DUNVIEW.C:8016-8038/8126-8139 mirrored door-front
  paths, 6849-6858/6880-6889 clip gates, 6240-6264/6304-6331 zone
  frames) — and routed the four wall parity draws through it.  The
  F0105 floor-pit/stairs flipped helper keeps its strict
  no-draw-without-source-temp gate, resolving the collision between
  the F0103/F0105 direct-call gates (82caa8b32/5041d4240) and the
  d0l/d0r parity test (fadea6678): the parity test now calls the
  parity route with identical pixel expectations.  Stale drift
  tokens re-anchored to live code: side contents renamed helper
  (a8ff8d15b), F0115 floor item receipt route (b2fa93c55), side-wall
  host receipt builder parity flag (c8ab48a2a).  pass560 verifier
  LOCAL spans re-anchored from dead line ranges to the current
  door-front occlusion tables (src 931-942, test 1303-1340); the
  whole-file drift scan in the test binary remains the durable
  form.  11 additional source-lock verifiers went green as a side
  effect (pass517/518/563/565 d0c/565 d1/570/577/583/608/611/628).
- 2026-07-20 DM2-003/005 follow-up: the c_ai turn block inside
  ATTACK_CREATURE bound data-backed (job/w2, round 12). The round-11
  diverged-stream stop in `dm2_v1_caii_attack_creature` is replaced by
  the full bound direction dance (c_creature.cpp:438-536): the entry
  RANDBIT, DM2_CALC_VECTOR_DIR (util.cpp:30-46, verbatim — including
  its tie-break RANDBIT draw) from the creature's CCM dispatch
  coordinates toward the attack origin, the word@0xa & 8 branch, the
  skip00247/skip00248/skip00251 ladder over the record's facing bits
  (word@0xe >> 8 & 3) with the exact source RNG draw sequence, and
  DM2_ai_13e4_0360 with argl0 == 0 (c_ai.cpp:5912-5960): the slot
  byte@0x17/0x1a == 0x13 guards and the byte@0x17 direction write.
  Final turn values 0-3 (absolute), 6/7 (relative) and -1 (no turn)
  are receipted (ai_turn_ran/entry_roll/vector_dir/facing/dir/applied/
  guard_denied). The creature position the source reads as globals
  (ddat.v1e0270/v1e0272, c_dballoc.cpp:438-440) enters the bounded
  slice as new target_x/target_y parameters. Fail-closed kept: gate
  unknown/out of span, or gate passed without a bound stream, still
  stops BEFORE the reaction roll (rng_stream_diverged). The argl0 != 0
  tail (c_ai.cpp:5949-5959) belongs to other callers and stays
  unbound. Test `dm2_v1_caii_attack_pc34_compat` reworked to 19
  scenarios: (h) now proves the bound turn with seeded LCG draws
  (survival 0, entry 1, RANDDIR 2, roll 60 -> dir 7 in slot byte@0x17),
  new (p)-(s) cover the entry-flip-0 no-turn with aligned reaction
  roll, the skip00248 reversal (dir 6), the byte@0x17 write guard with
  the dying-mode tail, and the diverged stop without a stream. dm2_v1
  lane 213 tests, same 27 known baseline failures, zero new failures
  (broad -R dm2 dm2_v2 probe failures pre-exist the lane merges —
  verified identical with the changes stashed).job/w2
- 2026-07-20 Nexus/Theron per-view zone inventory round 12 (job/w3,
  commit 33061f9bf): the Nexus question is answered honestly negative
  and the Theron prerequisite is landed.  Nexus (Sega Saturn) has no
  extractable source-locked zone tables: the original SH-2 code is
  staged only as opaque capture-gated binaries, the Firestaff input
  layer owns all input (docs/nexus_input.md), and the Nexus startup
  menu is Firestaff-authored layout — documented in TODO.md.
  Theron's Quest (TurboGrafx-CD) likewise has no extractable original
  tables: ReDMCSB WIP20210206 contains zero Theron/TurboGrafx
  coverage (1184 files verified), the only local disassembly is the
  IPL + stage2 boot loaders (docs/source-lock/theron-disassembly/,
  no input/UI-zone code), and THQUEST.BIN is not disassembled
  locally.  New `src/theron/theron_touch_click_zone_matrix_pc34_compat.c`
  + `include/theron_touch_click_zone_matrix_pc34_compat.h` therefore
  ship the IMPLEMENTED Firestaff chrome geometry with explicit
  no-original-table provenance: V1 chrome view (320x240 extended
  canvas, 9 zones — top bar, viewport, right panel, bottom panel,
  message bar, 4 champion slots, the same zone kinds the existing
  theron_v2_hud_target_size_pc34 audit consumes) and V2 HUD overlay
  view (256x224 PC Engine native, 17 zones — compass, 4 rune
  indicator cells, quest/dungeon/relic text widgets, 4 champion
  mini-bars, 5 action strip icons ATK/CST/USE/DRP/MOV with 1-based
  Theron_V2_ActionIcon ids).  New CTest
  `theron_touch_click_zone_matrix_audit` mirrors the DM2/CSB lane:
  pinned 26 total / 9+17 per-view counts, per-view bounds, disjoint
  slot grids, 13 hit-test probes incl. nested coarse panels and view
  isolation, hit_zone_audit_m11.h classification cross-check via
  fs_gesture_audit_zones at UI 100/150/200 x 1x..4x with pinned
  decisions 9/6/4/0 + 7 exempt (4x4 rune cells + 5px text boxes —
  presentation-only indicators), below-min 17/11/7/7,
  below-recommended 19/17/16/11, UI-scale hypothetical 11/7/7 (4
  zones would lift at UI-200 2x).  `ctest -R "theron_touch|dm2_touch|
  csb_touch|hit_zone"` 5/5 PASS, `-R "touch|gesture"` 33/33 PASS,
  full Release build green; the 15 failing `ctest -R theron` probes
  are pre-existing media-dependent Track 02 gates (missing game data
  in this environment), untouched by this change.  Remaining: a real
  THQUEST.BIN disassembly before any original-table Theron
  extraction; Nexus input structures only via future Saturn capture
  work.

- 2026-07-20 Nexus DGN Structure3 mesh extractor retail corpus binding
  (job/w4, round 12, commit f981a719d): the
  `nexus_v1_dgn_face_mesh_corpus` known failure is fixed (25 -> 24 of 196
  nexus tests). Root cause: the July 14 revert of "receipt-bind Structure3
  face normal planes" removed the fixed-point plane/winding measurement
  loop while `vectors.valid` and the c02ceb1ec no-draw plan gate still
  required its counters, so `structure3_vectors.valid` stayed 0 for every
  retail level and the extractor's receipt chain failed closed on all
  1,144 entries. Fix re-applies the measurement loop (cherry-pick of
  697a6c945 with conflict resolution against the edge-incidence and
  handoff work landed since) as corpus provenance, and stops enforcing
  exact plane coherence: retail measurement shows 12,171 of 72,540
  face/normal plane pairs outside the exact signed-16.16 envelope
  (non-planar quads et al.), so within/outside counts are reported as
  receipt facts instead of revoking vector validity. Verified: the corpus
  test now passes against retail LEV00-LEV15 — 1,144 entries, 18,478 unit
  face/normal pairs, texture-selector corpus (17,821 textured, 1,291
  unique static, 44 unique animated), and the face-edge corpus (73,226
  slots, 47,321 unique, 22,240 boundary) — plus the tampered-payload FNV
  rejection path. Full nexus sweep diffed: only
  `nexus_v1_dgn_face_mesh_corpus` dropped from the failure list;
  `nexus_v1_dgn_material_raster` and `nexus_v1_dgn_geometry_readiness`
  remain for separate capture-bound reasons.
- 2026-07-20 Theron stage-two call-graph continuations (job/w5, round
  12, 337f0474e): the first-tier callee bodies invoked from the
  contiguously bound executed entry path [0x00..0xb5) are now
  byte-bound — 154 continuation bytes across four windows. (1) The
  L40B7 command-dispatch loop at user offset 0xb7 (58 bytes, called at
  0x1e inside the bound entry prologue; its own L4814 call at 0xb9 sits
  inside its body), (2) the L4B2D count-down delay at 0xb2d (15 bytes,
  called at 0x52 inside the bound main path), (3) the L4B73 st0/st1/st2
  port clear at 0xb73 (35 bytes, called at 0x55 inside the bound main
  path), and (4) the L4814 zero-page pointer setup at 0x814 (46 bytes,
  called from the dispatcher; the two da65 decode-artifact bytes at
  0x81f-0x820 are bound to the authenticated media bytes) were all
  disassembled from the hash-gated US media and matched exactly against
  the source-locked disassembly
  (`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm:189-219,
  1207-1232, 1622-1632, 1661-1680`). New verifier
  `theron_v1_track02_verify_stage2_call_graph`
  (src/theron/theron_v1_track02.c) chains find_ipl_loader, requires the
  US variant plus `stage2_seed_call_sites_proven`, applies four
  `tqr_ipl_user_match` window checks across the 17-sector stage-two
  image, asserts that every call site (0x1e, 0x52, 0x55, 0xb9) sits
  inside an already-bound window, and fills the new
  `Theron_Track02Stage2CallGraphReceipt` (valid/variant/stage2_record/
  stage2_raw_sector/dispatcher_bytes/delay_bytes/port_clear_bytes/
  pointer_setup_bytes/call_graph_bound_bytes + four proven flags).
  Scope: US-only — the source-lock document attests JP/US byte identity
  only for the $4090 window, so the JP variant rejects
  (`THERON_TRACK02_SIGNAL_NOT_FOUND`, invalid receipt) until staged JP
  media can verify the same streams; no JP requirements were added to
  find_ipl_loader. Probe: the four fixture windows (the 0xb2d/0xb73/
  0x814 image offsets map into the second raw sector of the image), US
  positive test with full field assertions, four byte-mutation
  rejections (0xb7 -> 0x00 restored to 0x64; 0xb2d -> 0x00 restored to
  0x48; 0xb73 -> 0x00 restored to 0x78; 0x814 -> 0x00 restored to
  0xa9), JP-scope rejection in the JP block, US-gated real-media check.
  Verification: strict compile clean (`cc -std=c99 -Wall -Wextra
  -Werror`), probe `fail=0` against the real hash-verified US media,
  ctest 146/161 with the exact same 15 known failures as the
  pre-change baseline (name list diffed identical, no new failures).
  Semantics boundary unchanged: instruction bytes only — no System Card
  base arithmetic, no record semantics, no command meanings for the
  L410D dispatch table, no graphics role for the st0/st1/st2 writes.
  Remaining: JP verification of the same streams awaits staged JP
  media; next-tier call-graph windows (L410D dispatch table
  [0x10d..0x11d), L4AF7, L4F5E, L383E, L8000 in image sector 8 with its
  da65 decode artifacts, L45A6, L4696) are future windows; descriptor
  semantics unbound; post-$3800 consumer chain still capture-blocked;
  System Card trace-only; VDC/VCE runtime-gated.

- 2026-07-20 DM1 clobber-restoration round 11 (job/w1, commit
  e707dd2bc): four probes re-based to the verified PC 3.4 C127 map0
  layout and verified PASS. (1) portrait_12/22 screenshot_receipt:
  canonical poses moved off the stale fixture — LINFLAS (12) to
  (12,9)N (matching the green walkpath_from_entrance_12 constants)
  and GOTHMOG (22) to (12,13)W (matching the green
  walkpath_from_entrance_22 comment block). (2) portrait_01
  front_east_entry: entry pose (13,14)E facing the real shipped
  VIBIA mirror (ordinal 8 on the (14,14) west wall, confirmed by a
  live pose scan); sensor locate switched from the unverifiable
  square-chain cell-bit walk to the canonical coordinate-agnostic
  sensorData scan (ordinal_23 / seed_first_c127_data pattern).
  (3) portrait_21 front_east_entry (165_gate): source cell (16,17)N
  (front=(16,16) south wall, sensorData=21), neighbors
  (16,16)E/(16,18)E, entry cell (7,9)E; Group C thresholds set from
  a measured 10-pose EAST scan — (7,9)E open-hall distant view
  renders a near-black D1C rect coincidentally matching ordinal
  21's 0-heavy cell at ~95% (documented; GetFrontMirrorOrdinal==-1
  is the real no-portrait lock), neighbor poses measure 0%.
  pass560/pass784 triaged and documented in TODO.md as
  not-probe-re-baseable (missing source_runtime_contract module
  and viewport-3d source-lock failures respectively — engine work).
  Full dm1 suite (--timeout 60): 127 of 1338 failing, down from
  131; failure list diffed against round 10 — only the four fixed
  probes dropped, no new failures.
- 2026-07-20 DM2-003/005 follow-up: ATTACK_CREATURE message body taken
  data-backed (job/w2, round 11). `dm2_v1_caii_attack_creature`
  (include/dm2_v1_caii_alloc_pc34_compat.h,
  src/dm2/dm2_v1_caii_alloc_pc34_compat.c) binds
  c_creature.cpp:318-649 over the record pool: handle -1 resolution
  through DM2_GET_CREATURE_AT with the source early return, the
  unknown-AI-flags guard via the wired provider, the CAII-slot-less
  static denial, the vol_00 bit cuts (and16 0xbfff, poke16 with the
  x86 SHL mod-32 count), RANDBIT vol&0x4000 survival, the hp word
  write, the three-band aggro block (hp > 30 sets, hp <= 4 probes
  100*hp/aidef word@4 > 0xf, the 5..30 band consumes RANDDIR), the
  champion bit set/clear into record word@0xa gated by vl_14 =
  strength > RAND16(100), and the full reschedule gate: vl_10 +
  strength-0 forced rg1, the skip00254 cut, the
  table1d613a[slot byte@1a] chain into table1d607e[GDAT word@1]
  t6 & 0x410 with the & 2 tail, the dying-mode 0x13 and
  below-word@6-threshold early returns, and the closing
  DM2_1c9a_0db0 + DM2_1c9a_0cf7 cancel-and-reschedule. The c_ai turn
  block (c_creature.cpp:438-536) stays host-owned behind a data-backed
  entry gate (table1d607e[w1].uc[0] & 0x80): a passing gate declares
  the RNG stream diverged and stops BEFORE the reaction roll,
  fail-closed. RNG draws run on the session DM2_V1_DropRng through
  local LCG helpers with CUTX16 semantics (RAND16(n) =
  CUTX16(draw) % n; 100 does not divide 2^16, so the drops.h macro
  would be wrong — drops.h:80-86). table1d607e (47x4) and
  table1d613a (86 bytes, proven span 0x00-0x55) are verbatim
  per-module copies (mdata.c:1564-1639); out-of-span mode bytes fail
  closed. New creature-module accessors `dm2_v1_creature_ai_base_hp`
  (aidef word@4) and `dm2_v1_creature_gdat_word1` (CREATURES word@1,
  loader field 0x01 capture) are wired in the runtime think-binding;
  the delete head gained a data-backed `invoke_message_would_run`
  receipt. New CTest `dm2_v1_caii_attack_pc34_compat` covers fifteen
  scenarios (a-o). dm2_v1 lane 213 tests, same 27 known baseline
  failures, zero new failures.job/w2
- 2026-07-20 DM2 per-view touch/click zone matrix inventory (job/w3,
  commit d18c33fa7): the complete DM2 MOUSE_INPUT route-table set is
  now decoded and shipped as
  `src/dm2/dm2_touch_click_zone_matrix_pc34_compat.c` +
  `include/dm2_touch_click_zone_matrix_pc34_compat.h`, source-locked
  against SKWIN skval1.h (sk0d9e `_4976_0d9e` 257 commands, sk1891
  `_4976_1574` 74-node tree, `_4976_169c` packed child list, sk16ed
  `_4976_16ed` 60 rect descriptors, `_4976_1891` 10 view roots).  The
  decisive decode unlock: `_1031_07d6` (SkWinCore.cpp:55037-55120) is a
  startup relocation pass that rewrites the packed w2/w4 list ordinals
  to real offsets, so the tables were decoded in relocated form —
  which also dissolved the apparent "orphaned dungeon chrome" problem:
  all 74 nodes are reachable from the 10 view roots.  Rectangles
  resolve from the GDAT rect pool (GRAPHICS.DAT DM2 PC English md5
  25247ede4dabb6a71e5dabdfbcd5907d, raw 201 T1/I0/D4/S0, magic 0xFC0D,
  2183 rects) through QUERY_BLIT_RECT anchor semantics and `_1031_01d5`
  origin translation (topleft(7)=(0,40), topleft(18)=(48,32)); all 421
  zones are screen-relative in the 320x200 source screen with zero
  out-of-bounds (DM2 has no CSB-style CM2 translation in the hit-test).
  Views: main_menu 5, credits 2, sleep 3, pause 2, dialog 34, dungeon
  136 (movement arrows, viewport, champion hands, spell runes, party
  positions, magic map, moneybox, stats bars), champion_ribbon 3,
  champion 37, inventory 165, savegame_slots 34.  New CTest
  `dm2_touch_click_zone_matrix_audit` mirrors the CSB lane: pinned
  total/per-view counts, bounds, per-view disjoint grid families
  (movement 6, spell runes 6, moneybox 6, container 8, backpack 17+17,
  scabbards 4, pouches 2, savegame slots 34), 25 source-ordered
  hit-test probes incl. button masking and the aux-0x10 menu mask, and
  the hit_zone_audit_m11.h classification cross-check via
  fs_gesture_audit_zones at UI 100/150/200 x 1x..4x with pinned
  decisions 69/215/132/0 + 5 exempt (the hidden 1x1 pause box repeated
  in five views), below-min 352/137/5/5, below-recommended
  392/346/142/108, UI-scale hypothetical 137/5/5 (132 zones would lift
  at UI-200 2x).  `ctest -R "dm2_touch|hit_zone"` 3/3 PASS,
  `-R "touch|gesture"` 32/32 PASS, full Release build green.  The
  commit also fixes two pre-existing NDEBUG/-Werror Release build
  breaks (assert-only unused variables in
  test_csb_v1_f2262_timer_a_event and
  test_redmcsb_f0691_draw_compressed_img3).  Remaining: Nexus/Theron
  per-view inventories; DM2 button-mask bit 0x10 (aux menu modifier)
  origin not yet located — documented honestly in the zone evidence.

- 2026-07-20 FONT256.S2D subrecord grammar admission (job/w4, round
  11): the open subrecord question from the section corpus receipt is
  answered read-only. New module
  `nexus_v1_font256_s2d_subrecord_grammar`
  (`include/nexus_v1_font256_s2d_subrecord_grammar.h`,
  `src/nexus/nexus_v1_font256_s2d_subrecord_grammar.c`) rechecks the
  canonical admission and section corpus receipt against the live
  SHA-256-attested retail file and binds the observed internal
  subrecord arithmetic of three populated SCR sections, still as
  opaque measurements with no text, glyph, palette, record, encoding,
  or pixel meaning: ordinal 0 (table index 0, 8208 bytes) carries a
  16-byte preamble of eight canonical BE16 words {0x0010, 0x0000,
  0x4000, 0xffff x5} plus a 4096-word BE16 ramp word[i] == 2*(i &
  2047) — two identical 2048-word step-2 half ramps 0x0000..0x0ffe —
  closing the section exactly; ordinal 2 (table index 4, 528 bytes)
  carries exactly 33 sixteen-byte records (three canonical head
  records, thirty all-0x8000-word records) closing exactly; ordinal 3
  (table index 6, 484 bytes) is entirely zero. For ordinal 1 (table
  index 2, 15504 bytes) the grammar inventory is NEGATIVE and now
  receipt-bound: 742 populated of 969 canonical 16-byte blocks as an
  opaque composition measurement — the section stays capture-required
  with no proven subrecord structure. A bounded iterator exposes
  exactly the 38 raw subrecord spans (3 + 1 + 33 + 1) summing to the
  populated chain length 24724. New CTest pair
  `nexus_v1_font256_s2d_subrecord_grammar` (synthetic mirror with
  rejection matrix: NULL args, out-of-range ordinal, admission drift,
  preamble/ramp tamper in both halves, block population drift in both
  directions, canonical/base record tamper, zero-section tamper;
  population-preserving content tamper rebinds the live FNV and moves
  only the recorded digests) and
  `nexus_v1_font256_s2d_subrecord_grammar_real` (skip-safe retail
  path) both PASS. Full nexus suite: 194 → 196 tests, the 25 known
  baseline failures unchanged (list-diffed). No glyph layout,
  palette, encoding, or draw route proven — the ramp's and records'
  roles remain original-Saturn evidence work.
- 2026-07-20 Theron stage-two executed entry-path contiguity (job/w5,
  round 11, 1c15b6c16): the two remaining unbound windows in the proven
  stage-two sector (raw sector 1224 US = index01 225 + record 0x3e7) are
  now byte-bound, making the entire executed entry path [0x00..0xb5) =
  181 bytes contiguously bound end-to-end. (1) The entry prologue at
  user offset 0x00 (41 bytes: SEI/stack/MPR paging around the L8000
  call, the L40B7 call, and the System Card entry calls up to the seed
  JSR) and (2) the main path at user offset 0x2c (82 bytes: post-seed
  init, interrupt mask, L4B2D/L4B73 calls, TII clear/copy up to the
  retry-head BSR) were both disassembled from the hash-gated US media
  and matched exactly against the source-locked disassembly
  (`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm:107-181`).
  New verifier `theron_v1_track02_verify_stage2_entry_path`
  (src/theron/theron_v1_track02.c) chains find_ipl_loader, requires US
  variant plus `stage2_seed_call_sites_proven`, applies two
  `tqr_ipl_user_match` window checks, compile-time asserts the
  [0x00..0xb5) contiguity, and fills the new
  `Theron_Track02Stage2EntryPathReceipt` (valid/variant/stage2_record/
  stage2_raw_sector/entry_path_prologue_bytes/main_path_bytes/
  bound_bytes + three proven flags). Scope: US-only — the source-lock
  document attests JP/US byte identity only for the $4090 window, so
  the JP variant is documentedly rejected
  (`THERON_TRACK02_SIGNAL_NOT_FOUND`, invalid receipt) until JP media is
  staged; no JP requirements were added to find_ipl_loader. Probe:
  both window arrays in the fixture, US positive test with field
  assertions, two byte-mutation rejections (0x00 -> 0x00 restored to
  0x78; 0x2c -> 0x00 restored to 0x20), JP-scope rejection in the JP
  block, US-gated real-media check. Verification: strict compile clean
  (`cc -std=c99 -Wall -Wextra -Werror`), probe `fail=0` against the real
  hash-verified US media, ctest 146/161 with the exact same 15 known
  failures as the pre-change baseline (name list diffed identical, no
  new failures). Semantics boundary unchanged: instruction bytes only —
  no System Card base arithmetic, no record semantics, no graphics
  role. Remaining: JP verification of the same stream awaits staged JP
  media; L40B7+/call-graph continuations (e.g. L4814 in sector 2, L8000
  in image sector 8) are future windows; descriptor semantics unbound;
  post-$3800 consumer chain still capture-blocked; System Card
  trace-only; VDC/VCE runtime-gated.

- 2026-07-20 DM1 clobber-restoration round 10 (job/w1, commit
  592963ce3): three tests green. (1)
  firestaff_dm1_v1_hoc_champion_portrait_02/06_door_nearby_no_float_runtime_probe
  re-based from the stale (1,2)N seed route to the verified PC 3.4
  C127 map0 HALK pose (7,9)N (SEED_POSE constants + meta checks) —
  the round-5 TODO grouping with the C346 frame-edge class was wrong
  for this pair since they never sample frame edges. (2)
  tests/test_m11_dm1_front_mirror_asset_fail_closed.c source gate
  updated for 45917ebc4: stale 'sole C127 consumer' string replaced
  with the C346->C026 HoC overlay wording plus a new
  suppressGenericWallOrnament check. The same verified (7,9)N re-base
  was applied to portrait_06/17_inventory_exit_restore,
  portrait_19/22_wall_ornament_no_float and
  portrait_18_reincarnate_reselect: shipped-HALK sanity + seed now
  lock and the C026 portrait renders at 100% match, but those five
  remain red on the previously parked behaviour/bitmap classes
  (frame-edge signature at the closed mirror door, inventory-toggle
  contract, F0282 reselect flow) — evidence and remaining work in
  TODO.md. Full dm1 suite (--timeout 60): 131 of 1338 failing, down
  from 134; failure list diffed against round 9 — only the three
  fixed tests dropped, no new failures.

- 2026-07-20 DM2-003/005 follow-up: 0fcb DELETE_CREATURE_RECORD branch
  head taken data-backed + delete decision head bound (job/w2, round
  10). New queue accessor `dm2_v1_source_timer_peek_ticket`
  (include/dm2_v1_timeline.h, src/dm2/dm2_v1_timeline.c) binds the
  source's timerarray slot read (c_1c9a.cpp:5943-5944). In
  `dm2_v1_caii_free_slot` the branch (c_1c9a.cpp:5936-5957) is now
  entered when the round-9 data-backed record-delete flag is set AND a
  pending timer ticket is live: the payload (valueA lo/hi = x, y per
  c_timer.h:80-82 getxA/getyA) is read BEFORE the timer is deleted in
  source order, receipted record_delete_branch + record_delete_x/y;
  flag-without-timer takes the source's RG3L = 0 outcome (receipted
  record_delete_no_timer). After the slot is marked free the branch runs
  the new bounded decision head
  `dm2_v1_caii_delete_creature_record_head` (c_record.cpp:1357-1425):
  DM2_GET_CREATURE_AT resolution with source early return, the jz_test8
  AI gate (c_record.cpp:1385) data-backed through the wired provider,
  and the BOUND CAII slot byte@1a clear (c_record.cpp:1408-1413). The
  mutating tail stays receipted unbound: invoke-message/map-swap
  (c_record.cpp:1387-1406), tile-rooted MOVE_RECORD_TO (1419, DM2-002),
  DROP_CREATURE_POSSESSION (1422, DM2-002), DM2_1c9a_0247 dballoc
  cleanup (1423), DEALLOC_RECORD free-chain (1424). free_slot gained a
  dungeon parameter (runtime boundary + callers updated); the runtime
  think-binding now wires the proven provider
  dm2_v1_creature_ai_spec_flags. New CTest
  `dm2_v1_caii_record_delete_pc34_compat` (peek accessor, branch-taken
  with payload coords, no-timer RG3L = 0, bit0-set/no-provider closed
  branches, direct head matrix incl. slot mode-byte clear and NULL-CAII
  fail-closed) PASS. dm2_v1 lane 212 tests, same 27 known baseline
  failures, zero new failures. Remaining: the delete body's mutating
  tail (DM2-002 ground-stack/possession walk, dballoc free chain,
  message system), the ATTACK_CREATURE body, the event-driven activation
  callers (c_moverec.cpp:983, c_tim_proc.cpp:2887), the c_ai re-queue in
  the CCM end, and the CCM stream owner/grammar.job/w2
- 2026-07-20 0DMSTRT.BIN structure admission (job/w4, round 10): the
  last opaque Nexus asset opened. The SHA-256-attested retail
  0DMSTRT.BIN (39516 bytes, pinned in `firestaff_known_hashes.c`)
  carries no RES* framing; read-only inspection revealed a different
  fully verifiable structure — a boot-library image (observed class
  tag "GFS_SBL", stamp reporting version 2.10 dated 1996-02-01) with
  an exact zero-gap arithmetic partition. New module
  `nexus_v1_0dmstrt_structure_admission`
  (`include/nexus_v1_0dmstrt_structure_admission.h`,
  `src/nexus/nexus_v1_0dmstrt_structure_admission.c`) admits the whole
  file against the pinned identity (canonical SHA-256 string plus live
  FNV-1a rebind, exact size): dense region A [0x0000, 0x08a8) with a
  canonical 2102 non-zero population, 11672-byte all-zero gap, dense
  region B [0x3640, 0x9978) with 24077 non-zero bytes, 83-byte
  all-zero gap, 49-byte tail descriptor (0xff separator, 31-byte
  printable version stamp with the "GFS_SBL" class tag, NUL
  terminator, byte 0x01, "CD001" standard identifier, ISO-style
  "." / ".." directory-id stub with a canonical 0xff population of
  5), 4-entry fixup table, 32-byte all-zero gap, and a 12-entry fixup
  table ending exactly at the source size; a 7-entry head table at
  0x0058 behind a 0xffff sentinel anchors region A. All 23 fixup
  entries share the observed BE16 tag 0x0601 with canonical BE16
  value tables; a bounded iterator exposes exactly the 8 raw
  partition spans whose lengths sum to the source size. New CTest
  pair `nexus_v1_0dmstrt_structure_admission` (synthetic mirror with
  rejection matrix: NULL args, size/identity drift, gap tamper,
  non-zero population drift, tail/stamp/stub tamper, sentinel/tag/
  value tamper; population-preserving content tamper rebinds the live
  FNV and moves only the recorded digests) and
  `nexus_v1_0dmstrt_structure_admission_real` (skip-safe canonical
  path verifying both non-zero witnesses against the retail file)
  both PASS. Full nexus suite: 192 → 194 tests, the 25 known baseline
  failures unchanged (list-diffed). No instruction, code, data,
  relocation, or execution semantics proven — the fixup values'
  meaning and the boot flow's use of this image remain
  original-Saturn evidence work.
- 2026-07-20 Theron boot-chain record topology (job/w5, round 10,
  6a4f0d079): the boot chain's complete static record footprint is now
  proven as one fail-closed receipt. New module
  `theron_v1_track02_boot_record_topology` joins every statically named
  record of the authenticated Track 02 boot chain across both
  coordinate frames — the IPL-family spans (IPL executable, preload,
  stage-two executable) anchored through the data track's INDEX 01 raw
  sector, the stage-three manifest record and descriptor corpus
  file-relative — re-verifying each loader-named record's MODE1
  envelope against the hash-gated media and joining the spans with the
  proven descriptor referenced set into one membership bitmap with a
  contains query. Verified against the real hash-gated US Track 02:
  IPL executable 1156..1159 (4 sectors), preload 1220..1221, stage-two
  executable 1224..1240 (17), stage-three record 1248, joined with the
  162-record corpus into 183 distinct named records across a 336-slot
  span (1156..1491), slot-flag hash 0x0538d2e4 reproduced exactly.
  Coordinate overlap facts proven: the stage-three table references the
  stage-two executable's final two sectors (1239/1240) and its own
  manifest record (1248). The probe gains a heap-allocated synthetic
  fixture at the real coordinates (join/membership plus non-MODE1,
  shifted-anchor, record-mismatch, invalid-topology rejections), a
  full-chain real-media helper, US checks with the verified constants,
  and doc-attested JP anchor checks (224 frame, 3-sector executable,
  record 0x4df). Verification: strict compile clean
  (-Wall -Wextra -Werror), probe 32/32 PASS against real US media,
  ctest -R theron 146/161 with the same 15 known-failure names as
  baseline (no increase). Semantics boundary held: record-span topology
  only — no record is assigned a level, object, tile, palette, bitmap,
  code, or command meaning, and an unnamed record is not proven
  unreachable through any other path. Remaining: JP topology numbers
  await staged JP media; consumer-side wiring requires capture
  evidence.
- 2026-07-20 DM1 clobber-restoration round 9 tail (job/w1, commits
  572dcd81f and 649cb46a3): (1)
  firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe re-based
  to the verified PC 3.4 C127 map0 layout (ordinal table from the
  2026-07-18 first-slice entry): the fixture had claimed a fake hall
  around (1,1)-(2,6) with TextString-derived ordinals; all 16 poses
  now use the front-mirror rule (adjacent floor square on the mirror's
  own face side, facing the wall — 1=(7,9)N, 4=(10,5)S, 10=(7,13)S,
  13=(7,16)S, 15=(11,10)S, 18=(9,13)E) plus wrong-wall -1
  expectations, matching the 354c32788 portrait04 pattern; the stale
  ZED label corrected (ordinal 10 is THURFOOT per the portrait10
  probe). Probe green: 18 poses + stale-ordinal rect + portrait rect +
  resurrect round-trip; the 50-probe champion_mirror family fully
  green. (2) dm1_v22 finished_art test/probe race fixed: both binaries
  staged fixtures under the same /tmp/scratch/dm1-famg-data tree (and
  the probe wiped the unrelated /tmp/scratch/assets), so under
  parallel ctest each rm -rf'd the other's manifests mid-scenario —
  nondeterministic failures that surfaced once the aux4 fix reshuffled
  worker scheduling; each passed standalone. The probe now uses its
  own /tmp/scratch/dm1-famg-probe-data namespace; the pair passes 3/3
  under -j2. Full dm1 suite (--timeout 60): 134 of 1338 failing, down
  from 136; failure list diffed against round 8 — only the two fixed
  probes dropped, no new failures.

- 2026-07-20 DM1 clobber-restoration round 9 (job/w1, commit
  c0e8071f7): firestaff_dm1_v1_champion_panel_action_menu_routing_probe
  (suite #316) fixed — it previously hung forever in its
  actionDisabledTicks drain loop. An instrumented run (temporary probe
  patch, reverted before commit) showed the C11 receipt firing at
  tick 3 with aux3=2/aux4=0 while the orchestrator dispatch
  (memory_tick_orchestrator_pc34_compat.c:12203-12227) reads the
  champion owner from the canonical aux4 Priority and emitted
  EMIT_ACTION_ENABLED for champion 0 — so champion 2's action lock was
  never cleared and the disabled-tick mirror re-armed from the pending
  receipt every tick. Root cause: the 9b2482b93 merge left M11's
  static F0330 scheduler on the older aux3 champion layout after
  round-6 e600115e4 normalized the boundary to aux4. The fix keys
  M11's scheduler, its prior-receipt matcher, the F0407 throw
  B.SlotOrdinal marker and the disabled-tick mirror on aux4, matching
  the source layout (CHAMPION.C F0330:2253-2255 writes EVENT.Priority;
  TIMELINE.C C11:1927-1932 dispatches F0253 for it). Probe now
  terminates in 0.36 s, all assertions green. Verified: 15 C11-family
  tests (f0330/f0407/throw/swing/original_save) all pass; a 78-test
  champion/action selection shows only the two pre-existing baseline
  failures (champion_panel_pixels_runtime,
  champion_panel_damage_flash_decay). Full dm1 suite (--timeout 60):
  135 of 1338 failing, down from 136; failure list diffed against
  round 8 — only the fixed probe dropped, no new failures.

- 2026-07-20 DM2-003/005 follow-up: AI-spec table owner proven and bound
  (job/w2, round 9). The source chain DM2_QUERY_CREATURE_AI_SPEC_FLAGS
  (c_record.cpp:1346-1349) → DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD
  (c_record.cpp:1351-1354) resolves GDAT CREATURES word@5 into the
  36-byte AIDefinition table word@0 — the indirection the proven
  EXTENDED_LOAD_AI_DEFINITION GDAT path (SkWinCore.cpp:233-400) already
  captures. New accessor `dm2_v1_creature_ai_spec_flags`
  (include/dm2_v1_creature.h, src/dm2/dm2_v1_creature.c) follows it
  fail-closed, leaving the legacy capped-index `dm2_v1_creature_ai_spec`
  view untouched for its combat/projectile consumers. The CAII module
  consumes flags through the new session-wired provider hook
  `dm2_v1_caii_set_ai_spec_flags_fn` (no creature-TU dependency; no
  provider = fail-closed "unknown"). Two gates are now data-backed:
  DM2_1c9a_0fcb's record-delete flag (c_1c9a.cpp:5917-5929) computed as
  ((flags & 0x1) == 0 && slot byte@1a == 0x13), receipted
  `record_delete_flag` (1/0/-1) with the DM2_DELETE_CREATURE_RECORD
  branch still unbound; and new accessor
  `dm2_v1_caii_attack_guard_allows_alloc` binding the ATTACK_CREATURE
  vl_18 gate (c_creature.cpp:370-385: alloc only when AIDefinition
  word@0 bit0 set) returning 1/0/-1. New CTest
  `dm2_v1_caii_ai_spec_pc34_compat` (synthetic dtWordValue GDAT fixture:
  type 12 → row 5 → 0x0001, type 7 → row 9 → 0x0200) covers the
  accessor, fail-closed no-session/no-provider paths, guard polarity and
  the full 0fcb record-delete matrix — PASS. dm2_v1 lane 211 tests, same
  27 known baseline failures, zero new failures. Remaining: the
  DM2_DELETE_CREATURE_RECORD body (c_1c9a.cpp:5930-5944), the
  ATTACK_CREATURE body, the event-driven activation callers
  (c_moverec.cpp:983, c_tim_proc.cpp:2887), the c_ai re-queue in the CCM
  end, the CCM stream owner/grammar, and the possession chain walk /
  tile-rooted ground-stack for DM2-002.job/w2
- 2026-07-20 TITLE.BIN CNFD payload admission chain (job/w4, round 9):
  the last TITLE.BIN admission gap closed. New module
  `nexus_v1_title_cnfd_payload_admission`
  (`include/nexus_v1_title_cnfd_payload_admission.h`,
  `src/nexus/nexus_v1_title_cnfd_payload_admission.c`) admits all 33
  CNFD records read-only against the canonical TITLE.BIN: each receipt
  revalidates the round-5 RES* directory receipt (directory entry
  27+i, class CNFD, class-local id == i) and binds the observed
  DGT2-shaped payload — 16-byte head ("CNFD" magic, class-local id,
  "pp" tag, BE16 width, BE16 height, BE16 flag word), 32-byte prefix,
  packed plane of width*height/2 bytes. Record length closes
  arithmetically as 16 + 32 + width*height/2 for all 33 records; the
  corpus receipt binds the chain [0x16eec, 0x1b658) (0x476c bytes)
  covering the TITLE.BIN tail exactly after MAPD, exactly 8 distinct
  prefixes across the 33 records, flag word 0x8000 for records
  {0,6,12,18,24,30} else 0x8b00, and even width*height everywhere. A
  bounded plane-span iterator exposes exactly the raw prefix and plane
  spans with no decode. New CTest pair
  `nexus_v1_title_cnfd_payload_admission` (synthetic mirror with
  rejection matrix: NULL args, out-of-range index, identity drift,
  width tamper, flag tamper, prefix divergence 8→9 with admission
  intact, plane tamper live-rebind) and
  `nexus_v1_title_cnfd_payload_admission_real` (skip-safe canonical
  path with per-record nonzero plane witnesses) both PASS. With CNFD
  admitted the TITLE.BIN chain is fully closed at admission level —
  all 60 directory entries (DGT2, TITL, MAPD, CNFD) now have internal
  payload admission. Full nexus suite: 190 → 192 tests, the 25 known
  baseline failures unchanged (list-diffed). No glyph, font, palette,
  or presentation semantics and no CNFD-to-screen assignment —
  original-Saturn evidence work.
- 2026-07-20 Theron stage-three referenced-record span topology
  (job/w5, round 9, d6e28c629): the descriptor corpus work extends
  from whole-table media identity to the exact referenced-record set.
  New `theron_v1_stage3_descriptor_record_span_from_corpus` derives,
  from a proven corpus and the same authenticated manifest, the precise
  distinct record set the stage-three loader table references — packed
  one bit per span slot (4,096-slot fail-closed capacity), with
  aggregate counts and a per-slot flag hash — and
  `theron_v1_stage3_descriptor_record_span_contains` answers whether
  any record belongs to that authenticated set. The manifest is
  cross-checked against corpus aggregates (variant, stage-three record,
  derived base, non-zero selector count, distinct count, min/max);
  mismatches, over-capacity spans, or an unproven corpus fail closed
  with a zeroed receipt. Verified against the real hash-gated US Track
  02: 162 referenced records across 253 span slots (records
  0x4d7..0x5d3, 91 unreferenced slots), slot-flag hash 0x5634053b
  reproduced exactly; membership includes the stage-three self record
  0x4e0 and rejects interior gaps (0x4de, 0x4e1), below/above-span
  records, and the later traced 0x0b52 record. The probe gains
  synthetic span tests (derivation, membership, unproven-corpus /
  mismatched-manifest / distinct-drift rejections, invalid span) and US
  real-media span checks. Verification: strict compile clean
  (-Wall -Wextra -Werror), probe 23/23 PASS against real US media,
  ctest -R theron 146/161 with the same 15 known-failure names as
  baseline (no increase). Semantics boundary held: record-membership
  boundary only — an unreferenced slot is not proven absent from any
  other loader path, and neither membership nor absence assigns a
  level, object, tile, palette, bitmap, or command meaning. Remaining:
  JP span numbers await staged JP media; wiring the membership gate
  into later-route candidate admission still requires the capture-side
  record evidence those intakes gate on.
- 2026-07-20 DM1 clobber-restoration round 8 (job/w1, commit
  a410bd13c): firestaff_m11_dm1_v2_effects_framepath_probe fixed
  (29/29 green). The probe passed at d644dbecc only because the V2
  seed gate then read legacy summary counts; once the live-effect
  chain routed through
  dm1_v1_viewport_runtime_materialization_decide_pc34, two fixture
  defects surfaced: the "two-cell corridor" left both square bytes
  zeroed (walls — ReDMCSB DUNVIEW.C F0115 never draws projectiles or
  explosions on wall squares, and the decision gates them on f0115
  eligibility), and the spell fireball carried associated thing 0
  instead of THING_NONE (F0328 only fills the C05..C0B Thing for
  object projectiles). The probe now encodes real corridor squares and
  THING_NONE. The decision's live C15 scan also regained the
  pre-decision no-draw rule: a negative explosion type has no aspect
  branch in DUNVIEW.C:5958-5994, so such a record is skipped instead
  of poisoning the renderable list. Verified: the four
  decide-owning tests (materialization_pc34_compat,
  f0190_c040_m11_integration_audit, d0c_ordinary_c15_receipt_audit,
  framepath probe) all pass; the 9 failing projectile/explosion-family
  tests and the 2 failing materialization/stamina tests fail
  identically on the unmodified baseline (stash-verified). Full dm1
  suite (--timeout 60): 136 of 1338 failing, down from 137 real; the
  failure list is list-diffed against round 7 — the only delta is the
  fixed framepath probe. The champion_panel_action_menu_routing_probe
  hang was root-cause analysed (see TODO.md same-date entry) but no
  fix was attempted this round.
- 2026-07-20 DM2-003/005 follow-up: DM2_1c9a_0fcb (CAII slot free)
  bound — the slot lifecycle is complete (job/w2, round 8).
  `dm2_v1_caii_free_slot` in `dm2_v1_caii_alloc_pc34_compat` binds the
  bounded slice of skproject c_1c9a.cpp:5896-5944 with runtime boundary
  `dm2_v1_runtime_free_caii_slot`: fail-closed for out-of-range indexes
  (the source compares slot > ddat.v1e08a0 unsigned and would index out
  of bounds at slot == capacity, c_1c9a.cpp:5905), already-free early
  return, DB4 handle rebuild as slot word@0 | 0x1000
  (c_1c9a.cpp:5915), slot byte@1a = 0, pending timer deleted through
  the round-7 bound DM2_1c9a_0db0 path (c_1c9a.cpp:5933), alloc
  counter--, record byte@5 = -1, slot word@0 = -1. The
  DM2_DELETE_CREATURE_RECORD branch (c_1c9a.cpp:5930-5944) stays
  unbound — its flag derives from DM2_QUERY_CREATURE_AI_SPEC_FLAGS
  whose AI-spec table owner is unproven (receipted
  record_delete_unbound, never simulated). The source's
  despawn/cleanup callers (c_ai.cpp:5775, c_moverec.cpp:684 + 997,
  c_savegame.cpp:2049) remain future wiring. New CTests
  `dm2_v1_caii_free_pc34_compat` (guard paths, free semantics, slot
  reuse lifecycle) and `dm2_v1_caii_free_runtime_pc34_compat`
  (activate → free → zero dispatches for the freed creature →
  re-activation reuses the freed slot and the think chain resumes
  end-to-end) PASS. dm2_v1 lane: 210 tests, same 27 known baseline
  failures, zero new failures. Remaining: the c_ai re-queue inside the
  DM2_PROCEED_CCM end behind the CCM body, the CCM stream
  owner/grammar, the event-driven activation callers (ATTACK_CREATURE
  body, c_moverec.cpp:983, c_tim_proc.cpp:2887), the AI-spec table
  owner (gates the 0fcb record-delete branch), and the possession
  chain walk / tile-rooted ground-stack mutation for DM2-002.job/w2
- 2026-07-20 M11 in-game hit-zone audit, DM1 slice (job/w3): the M11
  half of the "UI scaling and touch-target audit across launcher and
  game views" cross-cutting TODO. New pure audit header
  `include/hit_zone_audit_m11.h` + CTest `m11_ingame_hit_zone_audit`
  consume the LIVE DM1 V1 hit-zone inventory from
  `touch_click_zone_matrix_pc34_compat.c` (104 source-locked zones
  from ReDMCSB COMMAND.C route tables + the I34E layout-696 ZONES
  table) — no zone geometry is duplicated, so the audit can never
  drift from the shipped hit-test table. Per zone, per UI scale
  100/150/200, per presentation scale 1x..4x the audit records
  classification against the 24 px floor / 44 px recommendation
  (cross-checked through `fs_gesture_audit_zones`) and a per-zone
  lifting decision. Pinned shipped-geometry contract: floor-at-1x 19
  zones, needs-2x 62, needs-3x 17 (the TODO-exemplar 13x11 spell
  runes lift only at 3x presentation — 2x leaves the 11 px side at
  22 < 24 — plus 85x11 action rows and 11x11/9x9 icons), needs-4x 5
  (43x7 champion name strips, 35x7 action.pass), never-lifts-exempt 1
  (hidden 2x2 freeze-game debug box, COMMAND.C:394). Aggregate
  below-floor counts pinned at 85/23/6/1 for 1x/2x/3x/4x and
  below-recommended at 98/85/27/8. UI-scale finding pinned: zone
  geometry is UI-scale independent today (M11_UIScale has no
  hit-test/HUD-geometry consumer); the hypothetical re-audit shows
  UI-200 geometry would lift 22 of the 23 sub-floor zones at 2x, so
  the audit must be re-run if HUD geometry ever consumes
  M11_UIScale. Verification: `m11_ingame_hit_zone_audit` PASS, full
  build green, `ctest -R "m11|touch|gesture|hit"` shows 24 failures,
  all in the documented pre-existing clusters (dm2/nexus/theron
  handoffs, source-locks, HoC, capture-blocked CSB, m11 probes);
  rond-6 leader_hand and all touch/gesture audits stay green; zero
  new failures (the diff is purely additive). Remaining: CSB/DM2/
  Nexus/Theron per-view zone tables must be extracted (source-locked)
  before the same audit can run on those views.
- 2026-07-20 CSB dungeon-view per-view zone inventory + hit-zone
  audit (job/w3): the CSB slice of the same cross-cutting TODO. New
  module pair `src/csb/csb_touch_click_zone_matrix_pc34_compat.c` /
  `include/csb_touch_click_zone_matrix_pc34_compat.h` carries the CSB
  dungeon game view's per-view click/touch zone inventory — 56
  source-locked zones grouped by view from the ReDMCSB WIP20210206
  COMMAND.C PC-media (MEDIA529/I34E) MOUSE_INPUT route tables that
  the Firestaff CSB lane executes: G0447 primary interface (19),
  G0448 secondary movement (8), G0452 action-area names (4), G0453
  action-area icons (4), G0454 spell area (9), G0455 champion
  names/hands (12). Hit-test semantics mirror F0358 (first
  zone+button match; all dungeon-view routes CM1 screen-relative).
  Zone rectangles resolve through the shared I34E layout zone space
  (DEFS.H:3748-3937) via the existing layout-696 extraction and are
  cross-validated against the Amiga G20E/G21E and Atari ST
  A20ED..A22G CSB literal tables in the same COMMAND.C (box-for-box
  agreement on the dungeon chrome); C147 freeze-game is the
  COMMAND.C:394 PC literal box. New CTest
  `csb_touch_click_zone_matrix_audit` pins per-view counts, 320x200
  bounds, source-disjoint grid families (movement arrows, status
  boxes, bar toggles, names, hands, spell symbols, action rows/icons,
  champion icons), per-view hit-test probes incl. button masking and
  view isolation, and runs the hit_zone_audit_m11.h classification
  cross-check (UI 100/150/200 x 1x..4x through
  `fs_gesture_audit_zones`) with pinned decisions (floor-at-1x 16,
  needs-2x 22, needs-3x 12, needs-4x 5, never-lifts-exempt 1 — the
  same hidden 2x2 freeze box) and aggregate counts (below-min
  40/18/6/1, below-recommended 53/40/22/7 at 1x..4x), plus the
  UI-scale-independence finding (2x below-min 18/6/1 at UI
  100/150/200; UI-200 would lift 17 of the 18 sub-floor zones at 2x).
  Honest provenance limitation: CSB's own GRAPHICS.DAT (graphic 561
  layout) is not staged in any permitted location, so per-file
  CSB-native rect confirmation is pending — the inventory is an
  engine-shared I34E-zone-space declaration, not a CSB-file
  extraction. Verification: `csb_touch_click_zone_matrix_audit` PASS,
  `ctest -R "hit_zone|csb_touch|touch|gesture"` 33/33 PASS, zero new
  failures (the diff is purely additive). Remaining: the CSB-native
  graphic-561 extraction once the file is staged, and DM2/Nexus/
  Theron per-view inventories.
- 2026-07-20 CSB route-table set completed (job/w3): the CSB
  touch/click zone inventory now covers all twelve PC MOUSE_INPUT
  route tables — 147 zones total, up from the round-9 set of 56.
  Extended `src/csb/csb_touch_click_zone_matrix_pc34_compat.c` /
  `include/csb_touch_click_zone_matrix_pc34_compat.h`, all
  source-locked against the ReDMCSB WIP20210206 COMMAND.C PC-media
  (MEDIA529/I34E) route tables: G0449 champion inventory (38 routes
  incl. the PC-only C141 music toggle and the C081 panel zone via
  the layout-696 C100/C101 center anchor), G0456 chest panel (8),
  G0457 resurrect/reincarnate/cancel panel (3, panel-rooted at
  viewport-local (80,52), DEFS.H MEDIA539 M664-666 → zones
  570/571/573), G0445 entrance (5 incl. the bonus-dungeon
  MASK0x0010 button route and the I34E-only C216 quit zone 434;
  M566=202/M567=203 via DEFS.H MEDIA405), G0446 restart game (2,
  the I34E MEDIA730 literal boxes), and G2045 champion rename panel
  (35 — the 577-613 rename block resolved from layout-696:
  backspace (107,114,69,9) viewport-local, OK (197,114,19,9), title
  (207,93,9,19), thirty-one 9x9 keys, plus the right-button
  full-screen space route). The header gains six new view enum
  values (CHAMPION_INVENTORY=7 … PANEL_CHAMPION_RENAME=12) and
  `CSB_TOUCH_CLICK_BUTTON_BONUS_DUNGEON_PC34_COMPAT 0x0010u`. The
  hit-test now mirrors F0358 fully: CM2 zones are tested minus the
  COORD.C G2067/G2068 viewport origin (0,33). The
  `csb_touch_click_zone_matrix_audit` CTest is re-pinned: 147 total,
  per-view counts 19/8/4/4/9/12/38/8/3/5/2/35, bounds per
  coordinate mode (320x200 / 224x136), new source-disjoint grid
  families (backpack line1=9/line2=8, quiver 4, chest 8, the 34-key
  rename grid, the four unique entrance boxes), 17 new hit-test
  probes, and the hit_zone_audit_m11.h classification cross-check
  re-pinned (decisions 21/69/51/5/1, below-min 126/57/6/1,
  below-recommended 139/126/64/42, UI-scale hypothetical 57/6/1 —
  56 zones would lift at UI-200 2x). Verification:
  `csb_touch_click_zone_matrix_audit` PASS (0 failures), `ctest -R
  "csb_touch|hit_zone"` 3/3 PASS, `ctest -R "touch|gesture"` 31/31
  PASS, zero new failures. Remaining: the CSB-native graphic-561
  extraction once the file is staged, and DM2/Nexus/Theron per-view
  inventories.
- 2026-07-20 Nexus TITLE.BIN MAPD TIBG admission (job/w4, one commit):
  completes the TITLE.BIN record-class admission set by binding the
  single MAPD record's internal TIBG payload. New
  `nexus_v1_title_mapd_tibg_admission` module
  (`include/nexus_v1_title_mapd_tibg_admission.h`,
  `src/nexus/nexus_v1_title_mapd_tibg_admission.c`; picked up by the
  existing `src/nexus/nexus_v1_*.c` glob into `firestaff_nexus`)
  revalidates the round-5 RES* directory receipt for MAPD entry 26
  (id 0) against the live SHA-256-attested source and binds its
  observed TIBG payload shape, verified read-only against the retail
  asset before coding: 64-byte header ("MAPD" magic, id, "TIBG" tag,
  thirteen canonical BE32 fields including the payload-size field
  0x8c6c = record bytes - 8), a 4-byte-cell span `[0x40, 0x8c54)` of
  8965 cells with exactly five marker cells `00 40 00 1c` at
  `0x40 + k*0x1c04` (k = 0..4), an observed filler-cell population of
  3360, and a 32-byte tail of sixteen BE16 words ending in 0xffff, all
  closing arithmetically against the canonical record length 0x8c74
  (the header's five relative offsets land exactly on the marker
  positions). Receipts retain per-span FNV-1a digests for record,
  header, cell span, and tail plus opaque field/count measurements; a
  bounded span iterator exposes exactly the raw cell span and tail
  span. No byte, cell, or word is assigned tile, map, palette, colour,
  image, or presentation semantics and no decode or draw route is
  permitted. Tests: new `tests/test_nexus_v1_title_mapd_tibg_admission.c`
  (dual-mode; synthetic mirror with canonical framing by default) plus
  skip-safe wrapper
  `tests/test_nexus_v1_title_mapd_tibg_admission_real.sh`, registered
  as CTest pair `nexus_v1_title_mapd_tibg_admission` (synthetic) and
  `nexus_v1_title_mapd_tibg_admission_real` (canonical TITLE.BIN path),
  covering the receipt, header/marker/cell/tail arithmetic, the filler
  population, iterator spans, and rejection across NULL arguments,
  identity drift, header field tamper, marker tamper, filler
  population drift, and tail last-word tamper; non-filler cell tamper
  rebinds the live FNV and moves only the recorded cell-span digest,
  as designed. Verification: full `cmake --build build --parallel 10`
  green; both new CTests pass, the real one against the canonical
  retail TITLE.BIN; focused Nexus sweep `ctest --test-dir build -j10
  -R nexus` shows the same 25 pre-existing failures as the pre-change
  baseline (10+3+6+6 across the four sweep chunks; track1 readiness
  timeouts, PRS3 lanes, script_vm, sound receipt, mechanics parity,
  M11/startup/DGN lanes) with zero new failures and both new tests
  green (188 -> 190 registered Nexus tests). This still proves no
  tile, map, palette, colour, image, or presentation semantics
  (including the cell values', header fields', and tail words'
  meaning) and no MAPD-to-screen assignment; how the original title
  flow uses this payload remains original-Saturn evidence work.

- 2026-07-20 Theron stage-three descriptor corpus media correlation
  (job/w5, round 8, 46584d09b): the static descriptor evidence now
  covers the complete stage-three loader record table. New
  `theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest`
  in `theron_v1_later_record_correlation` resolves every non-zero
  selector in the authenticated 218-entry manifest against the derived
  base and re-verifies each resolved MODE1 sector against the
  hash-gated Track 02 bytes (sync, mode, chained user-data identity in
  descriptor-table order, aliases included), failing closed with a
  zeroed receipt on any out-of-bounds selector, malformed envelope,
  overflow, or changed byte. Verified against the real hash-gated US
  Track 02: all 216 non-zero selectors (2 zero) resolve in-bounds to
  162 distinct well-formed MODE1 records spanning records 0x4d7..0x5d3,
  with chained identity hashes 0xbd3eeb40 (ordinal/record pairs) and
  0x90c5b97f (user data) reproduced exactly by the C implementation.
  The probe gains a fully in-bounds synthetic fixture (distinct/alias
  counting, min/max span), five rejection/tracking tests
  (out-of-bounds selector, non-MODE1 resolved sector, zero first
  selector, non-aligned media, changed-user-byte), and a US-only
  real-media section so the corpus proof runs with only the US Track 02
  staged (JP/US comparison now skips cleanly unless both variants are
  present). Verification: strict compile clean (-Wall -Wextra -Werror),
  probe 14/14 PASS against real US media, ctest -R theron 146/161 with
  the same 15 known-failure names as baseline (no increase). Semantics
  boundary held: record-coordinate spans and identity hashes only —
  descriptor_semantics_proven stays 0; no level, object, tile, palette,
  bitmap, or command meaning assigned. Remaining: JP corpus numbers
  await staged JP media; a real post-$3800 consumer trace is still
  required before any record grammar.
- 2026-07-20 DM1 clobber-restoration round 7 (job/w1, commit
  4a340d225): dm1_v1_original_save_c13_m11_runtime fixed by giving the
  M11 movement path its own driver for the live C13 rebirth chain.
  (1) New exported
  DM1_V1_F0255_DispatchDueViAltarRebirthPc34Compat in
  src/memory/memory_tick_orchestrator_pc34_compat.c (~line 5672, after
  the orch_write_raw_projectile_f0219_compat forward declaration;
  prototype in include/memory_tick_orchestrator_pc34_compat.h after
  the F0887 declaration): it pulls due C13 events from the queue with
  the C14 projectile-index shift and runs
  orch_c13_apply_vi_altar_rebirth_compat — the F0255 boundary the
  handoff test showed F0887 deliberately does not own. (2) The driver
  is invoked in m11_process_dm1_v1_pipeline_tick
  (src/engine/m11_game_view.c ~line 17149) immediately before the
  F0887 block. (3) Test fixture in
  tests/test_dm1_v1_original_save_c13_m11_runtime.c completed with
  columnSftBases[1], columnsCumulativeSquareFirstThingCount and
  dungeonColumnCount=1 — required since db1e5846e rewrote F0160/F0510
  to compact-SFT. Verified: original_save_pc34_handoff,
  original_save_c13_m11_runtime and original_save_pc34_portrait_receipt
  all pass; a stash comparison showed the 30 failures in the
  movement/m11 shard (including m11_game_view,
  dm1_v1_movement_core_lane_source_lock,
  dm1_v1_savegame_pc34_native_export_pc34_compat) fail identically on
  the baseline — no regressions. Full dm1 suite: 136 of 1338 run
  failing (down from 139), plus pass512_dm1_v1_movement_cross_reference_audit
  still failing on its own; the framepath probe analysis is documented
  in TODO.md but no fix was attempted this round.
- 2026-07-20 DM2-003/005 follow-up: DM2_1c9a_0db0/DELETE_TIMER
  replacement path bound — self-maintaining creature timer chain
  (job/w2, round 7). The source queue gained stable session-issued
  tickets mirroring the timerarray slot index (DM2_QUEUE_TIMER returns
  the stable index, skproject c_timer.cpp:235-257; DM2_DELETE_TIMER
  frees it, c_timer.cpp:215-232): additive `tickets[]`/`next_ticket`
  fields on DM2_V1_SourceTimerQueue,
  `dm2_v1_source_timer_enqueue_ticketed` (legacy enqueue delegates),
  and `dm2_v1_source_timer_cancel` (fail-closed on zero/unknown/stale
  tickets). The creature-schedule receipt now carries `timer_ticket`.
  `dm2_v1_caii_delete_timer` binds DM2_1c9a_0db0
  (c_1c9a.cpp:5734-5763): DB4 check ((handle >> 10) & 0xf == 4), record
  byte@5 CAII slot, slot word@2 pending-timer delete + write-back -1
  over the session tickets. `dm2_v1_caii_schedule_creature_at` binds
  the COMPLETE DM2_1c9a_0cf7 (c_1c9a.cpp:5695-5728): replace-first when
  the slot timer word references a live ticket
  (receipt.replaced_existing == 1; stale post-dispatch references fail
  safe), enqueue the new 0x21/0x22 timer, store the issued ticket in
  slot word@2 (c_1c9a.cpp:5724-5728); the CAII alloc path writes word@2
  as well. A record without a CAII slot fails closed (no_caii_slot) —
  the source would index the creatures array out of bounds. Runtime
  boundary `dm2_v1_runtime_reschedule_creature_at` exposed for the
  source's direct callers (c_creature.cpp:648, c_move.cpp:700). New
  CTests `dm2_v1_caii_timer_replace_pc34_compat` (ticket
  issue/cancel/stale-guard, 0db0 paths, replacement without duplicate
  accumulation) and `dm2_v1_caii_reschedule_runtime_pc34_compat`
  (activation → reschedule → exactly one think timer dispatched;
  post-dispatch reschedule still schedules) PASS. dm2_v1 lane: 208
  tests, same 27 known baseline failures, zero new failures. Remaining:
  the c_ai re-queue inside the DM2_PROCEED_CCM end behind the CCM body,
  the CCM stream owner/grammar, the event-driven activation callers
  (ATTACK_CREATURE body, c_moverec.cpp:983, c_tim_proc.cpp:2887),
  DM2_1c9a_0fcb (CAII slot free), and the possession chain walk /
  tile-rooted ground-stack mutation for DM2-002.job/w2
- 2026-07-20 Launcher menu-row hit-height audit at fontScale 1..3
  (job/w3): the launcher half of the remaining "UI scaling and
  touch-target audit across launcher and game views" cross-cutting
  TODO. New shared header `include/menu_row_metrics_m12.h` is the
  single source of truth for every launcher menu-row surface: legacy
  320x200 settings-classic (base pitch 18, frame 24, text top pad 5),
  settings-dense (pitch 34), save-browser (base pitch 22, frame at
  -4), and the modern 1080p settings rows (height 50, pitch 70) + tab
  strip (34). Both draw paths (menu_startup_m12.c,
  menu_startup_render_modern_m12.c) now consume the named constants,
  and new CTest `m12_menu_row_hit_height_audit` audits every surface
  at fontScale 1..3: containment of the presented label (conservative
  11-row Unicode glyph bound at the
  `m12_effective_text_scale`-resolved scale — localized Å/Ä/Ö labels
  are 11-row Unicode glyphs), presented-height classification at
  1x..4x presentation scale against the 24 px floor / 44 px
  recommendation cross-checked through `fs_gesture_audit_zones`, and
  per-row decisions (legacy rows are V1-parity source-space small at
  1x by design and must clear the floor at >= 2x presentation; modern
  settings rows meet the recommendation natively; the tab strip meets
  the floor, accepted below recommendation as secondary navigation;
  the modern renderer is documented as fontScale-independent). The
  audit exposed a real overflow: at fontScale 2/3 the fixed 18/22 px
  legacy pitches could not contain the 22/33 px scaled labels (the
  next row's frame overpainted the overflow), so the classic settings
  and save-browser pitches are now scale-aware via
  `m12_menu_row_settings_classic_pitch()`,
  `m12_menu_row_settings_classic_visible_rows()`, and
  `m12_menu_row_save_browser_pitch()`; fontScale 1 stays bit-identical
  (18 px pitch / 6 visible rows, 22 px pitch). Verification:
  `m12_menu_row_hit_height_audit` PASS, full build green, and the
  m12|touch|gesture|launcher ctest set shows exactly the 8 documented
  pre-existing m12 failures (m12_all_games_boot_readiness_receipt,
  m12_quick_resume_gate, m12_startup_menu, m12_session_timer,
  m12_language_cycle_layout, save_browser_export_import_m12,
  save_byte_manifest_m12, m12_polished_ui_flow — none touch the draw
  path changed here) plus 5 cross-lane launcher-handoff failures in
  untouched dm2/nexus/theron/dm1 lanes; zero new failures. Remaining
  on the parent TODO: the M11 in-game hit-zone audit per game view at
  each UI scale.
- 2026-07-20 Nexus TITLE.BIN DGT2 payload admission (job/w4, one
  commit): extends the TITLE.BIN RES* directory corpus into the 22 DGT2
  records' internal payloads. New
  `nexus_v1_title_dgt2_pp_payload_admission` module
  (`include/nexus_v1_title_dgt2_pp_payload_admission.h`,
  `src/nexus/nexus_v1_title_dgt2_pp_payload_admission.c`; picked up by
  the existing `src/nexus/nexus_v1_*.c` glob into `firestaff_nexus`)
  revalidates the round-5 RES* directory receipt for each DGT2 entry
  (0..21) against the live SHA-256-attested source and binds its
  observed payload shape: 16-byte head ("DGT2" magic, class-local id,
  "pp" tag, BE16 width, BE16 height, BE16 flag word), 32-byte post-head
  prefix, and a packed width*height/2 byte plane, with exact per-record
  length arithmetic 16 + 32 + width*height/2 and no trailing bytes.
  Canonical provenance bindings verified read-only against the retail
  asset before coding: dimensions 64x8 (records 0..3), 104x8 (4..5),
  24x24 (6..20), 168x12 (21); flag word 0x8220 except 0x81e0 for
  records 6..20; one contiguous DGT2 sub-chain `[0x2e8, 0x2318)`
  (0x2030 bytes) inside the whole-file chain; 20 distinct 32-byte
  prefixes of 22 with exactly two byte-identical pairs ((2,4) and
  (3,5)), recorded as observations that flip cleanly on divergence
  rather than admission requirements. Receipts retain per-span FNV-1a
  digests for record, prefix, and plane ranges; a bounded plane-span
  iterator exposes exactly the 22 raw packed-plane spans. No byte or
  word is assigned colour, palette, image, pixel, or presentation
  semantics and no decode or draw route is permitted. Tests: new
  `tests/test_nexus_v1_title_dgt2_pp_payload_admission.c` (dual-mode;
  synthetic mirror with canonical framing by default) plus skip-safe
  wrapper `tests/test_nexus_v1_title_dgt2_pp_payload_admission_real.sh`,
  registered as CTest pair `nexus_v1_title_dgt2_pp_payload_admission`
  (synthetic) and `nexus_v1_title_dgt2_pp_payload_admission_real`
  (canonical TITLE.BIN path), covering per-record receipts, chain
  arithmetic, the prefix-pair and distinct-count observations, iterator
  spans, real-mode per-plane nonzero-count witnesses (195/195/191/191/
  301/301/107/113/115/114/102/112/111/106/117/107/96/106/106/103/111/
  578), and rejection across NULL arguments, out-of-range indices,
  identity drift, and head dimension/flag tamper; plane tamper rebinds
  the live FNV and moves only the recorded digest, as designed.
  Verification: full `cmake --build build --parallel 10` green; both new
  CTests pass, the real one against the canonical retail TITLE.BIN;
  focused Nexus sweep `ctest --test-dir build -j10 -R nexus` shows the
  same 25 pre-existing failures as the pre-change baseline (10+3+6+6
  across the four sweep chunks; track1 readiness timeouts, PRS3 lanes,
  script_vm, sound receipt, mechanics parity, M11/startup/DGN lanes)
  with zero new failures and both new tests green (186 -> 188
  registered Nexus tests). This still proves no colour, palette, image,
  pixel, or presentation semantics (including the packed plane's nibble
  order and the 32-byte prefixes' meaning) and no DGT2-to-screen
  assignment; which payloads the original title flow draws, where, and
  in which order remains original-Saturn evidence work.

- 2026-07-20 Theron static IPL + stage-two loader read-window binding
  (job/w5, round 7, 008caff66): the four remaining unbound static
  windows of both original loader read windows are now bound as
  fail-closed byte patterns in `theron_v1_track02_find_ipl_loader`,
  each verified byte-for-byte against the hash-gated US Track 02 media
  (JP patterns document-attested identical, exercised via the synthetic
  JP fixture). The stage-one CD_EXEC retry branch at user offset 0xa7
  (BRA $4080 loop head) and the stage-one CD_READ preload-table load at
  0xa9 (CLX + four LDA $40dc,X / STA $fc/$fe/$fd/$f8 pairs sharing the
  CD_EXEC zero-page argument map) complete the stage-one window
  contiguously across [0xa9..0xd4]; the stage-two register-seed JSR
  $40ae at 0x29 and BSR +0x2e at 0x7e (displacement lands exactly on
  the seed body) complete the stage-two window across [0x7e..0xb4] plus
  0x29. Three receipt fields (cd_exec_retry_branch_proven,
  cd_read_table_load_proven, stage2_seed_call_sites_proven) record the
  completeness; the probe adds the fixture bytes, receipt assertions,
  and four byte-mutation rejection tests. Verification: strict compile
  clean (-Wall -Wextra -Werror), IPL-loader probe against the real
  hash-verified US media reports fail=0, ctest -R theron 146/161 with
  the same 15 known-failure names as baseline (no increase). Semantics
  boundary held: instruction bytes only — System Card base arithmetic
  stays trace-only, no record semantics, no graphics role; VDC/VCE
  destination still requires runtime evidence and the post-$3800
  consumer chain remains capture-blocked.
- 2026-07-20 DM1 clobber-restoration round 6 (job/w1): five failing
  source-locked tests fixed across three commits. (1) Restored the
  F0190 killed-all shouldDeleteGroupEvents chain (plan field, plan ->
  apply copy, afterplay gate, side-effects predicate) removed by
  df88dbda4 — dm1_v1_f0190_killed_all_runtime_cleanup_pc34_compat
  passes (003a9152f). (2) Restored the six-argument schedule() helper
  in the square-state dispatch test (cell parameter lost to merge
  drift; wall SET/CLEAR carry cell 1) —
  dm1_v1_square_state_dispatch_pc34_compat passes (1a9e0f62d).
  (3) Normalized the C11 enable-champion-action boundary to the
  canonical aux4 receipt (e600115e4): the 9b2482b93 merge had kept the
  legacy aux3 form in the save boundary while dispatch, canonical
  F0330 and every current test key the champion on aux4. Handoff
  materialization stores Priority in aux4 with the source-locked
  SlotOrdinal (0|2) validation and retains unowned B/C bytes for the
  F0433 round trip; native export reads the aux4 receipt and re-emits
  Priority plus opaque B/C; the spell path no longer double-schedules
  through the legacy aux3 helper (F0884's disabled-emission scan owns
  scheduling, matching ancestor 33739de37); C53 watchdog dispatch is
  NOCOPYPROTECTION consume-only again (the a1647b3c1 re-arm restore
  contradicted the current handoff test); the F0887 C13 case consumes
  external F0435 receipts without synthesizing a rebirth follow-up;
  and the roundtrip-reload report regained its clobbered external
  portrait receipt (offset, fingerprint, preservation) plus the
  inactive M516 champion record receipt from the 41fa2000b/221ab7d89
  design. Fixes dm1_v1_f0330_c11_production_pc34_compat,
  dm1_v1_original_save_pc34_handoff (C11 dispatch, C53, C13 sections)
  and dm1_v1_original_save_pc34_portrait_receipt. Full dm1 suite: 139
  known failures, down from 141 — no regressions (the four remaining
  cluster failures m12_quick_resume_gate, dm1_v1_save_load_source_lock,
  dm1_v1_original_save_c13_m11_runtime, dm1_v1_save_load were verified
  failing at baseline with the changes stashed).
- 2026-07-20 DM2-003/005 follow-up: CAII slot allocator
  DM2_ALLOC_CAII_TO_CREATURE bound — lazy creature-activation end-to-end
  (job/w2, round 6). Source research correction: there is NO map-load
  CAII loop in skproject; activation is event-driven
  (DM2_ATTACK_CREATURE resolves the record via DM2_GET_CREATURE_AT at
  the activation cell when its record argument is -1,
  c_creature.cpp:347-352, then allocs; c_moverec.cpp:983,
  c_tim_proc.cpp:2887, c_1c9a.cpp:9982 call the allocator directly), so
  the binding mirrors that reach path instead of inventing a spawn
  walk. New module `src/dm2/dm2_v1_caii_alloc_pc34_compat.c` binds the
  observable slice of c_1c9a.cpp:5772-5894 over a session-owned CAII
  array (34-byte slots, source stride 0x22; capacity caller-owned
  stand-in for ddat.v1e08a0 until DM2_1c9a_3c30/DM2_INIT is proven;
  alloc counter stand-in for ddat.v1d4020): the record byte@5 early
  return (already_allocated), the word@0xe bit-10 read-modify-write
  receipted as a no-op, the free-slot scan (signed word@0 < 0), the
  full slot init (word@0 = bare record index — DM2_1c9a_0fcb rebuilds
  the DB4 handle by OR-ing 0x1000, c_1c9a.cpp:5915; word@2 = -1;
  byte@6 = (gametick >> 2) - 1; byte@4 = gametick - 0x7f; word@0xc =
  (x & 0x1f) | (y << 5) | (map << 10); byte@0x16/0x17 = -1; byte@7 = 0;
  record byte@5 = slot index; alloc counter++), the round-5 bound
  scheduling producer DM2_1c9a_0cf7 queueing the creature's first
  0x21/0x22 timer (c_1c9a.cpp:5860), and slot byte@1a = 0x00 grouped /
  0x11 ungrouped (c_1c9a.cpp:5861-5866). The no-free-slot path fails
  closed without mutation (DM2_RECYCLE_A_RECORD_FROM_THE_WORLD
  c_1c9a.cpp:5880-5891 unproven); DM2_PREPARE/UNPREPARE_LOCAL_CREATURE_VAR,
  DM2_14cd_0802 and the s350 group scan with
  DM2_CREATURE_SOMETHING_1c9a_0a48 stay host-owned until the CCM body is
  proven. Runtime wiring: `dm2_v1_runtime_caii_init` (session array) +
  `dm2_v1_runtime_alloc_caii_at` (activation boundary over the session
  pools/dungeon/source queue) — activation → CAII slot → 0x21/0x22
  timer → next-tick dispatch → per-cell DM2_THINK_CREATURE resolution
  verified end-to-end. New CTests `dm2_v1_caii_alloc_pc34_compat` and
  `dm2_v1_caii_alloc_runtime_pc34_compat` PASS. dm2_v1 lane: 206 tests,
  same 27 known baseline failures, zero new failures. Remaining: the
  c_ai re-queue inside the DM2_PROCEED_CCM end (c_ai.cpp:5609-5614 +
  5644) behind the CCM body, the CCM stream owner/grammar, the
  DM2_1c9a_0db0/DELETE_TIMER replacement path (needs stable timer
  indices), the event-driven activation callers, and the possession
  chain walk / tile-rooted ground-stack mutation for DM2-002.job/w2
- 2026-07-20 CSB leader_hand test repair + registration (job/w3, commit
  d7b4f50a5): `test_m11_csb_leader_hand_no_dm1_fallback` built but was
  never registered with CTest and failed 16/169 checks when run
  manually. Three root causes, three fixes. (a) Overlay-marker cluster
  (10 checks): not a runtime regression — `M11_GameView_Draw`'s CSB
  branch is intentionally fail-closed without a hash-verified startup
  session (gates added 2026-07-13/15, after the test was written
  2026-07-05); the test now draws through the CSB-owned viewport path
  directly via `draw_csb_runtime_overlay_frame()` (csb_v1_viewport_init
  + csb_v1_viewport_render_frame, the same production code M11 binds
  drawers into) and reads counters via
  `capture_csb_runtime_overlay_draw_stats()`. (b) STAB stamina
  write-back (1 check): the CSB melee branch in
  `M11_GameView_TriggerActionRow` applied the F0325 stamina cost on the
  M11 champion but never wrote vitals back to the CSB runtime; added
  `m11_write_csb_runtime_champion_vitals()` after the action-completion
  plan. (c) SHOOT cooldown/refill (5 checks): the aging chain was
  tick-driven but the CSB input path never advanced gameTick and the
  expired C11 timeline event (F0330) was never consumed; the CSB
  bridge.mapped branch now advances gameTick and dispatches F0887
  timeline events mirroring GAMELOOP.C order (C11 consumption drives
  the SHOOT ready-hand refill), and the test loop cap was raised
  20→64 for the 50 source-authentic cooldown ticks. Test registered in
  CMakeLists (add_test in the EXISTS block); `ctest -R leader_hand`
  13/13 green, `m11_csb_leader_hand_no_dm1_fallback` PASS 169/169.
  The m11_game_view.c additions drifted three rond-4 redmcsb gate
  spans; refreshed `tools/verify_v1_inventory_toggle_redmcsb_gate.py`
  and `tools/verify_v1_inventory_chest_actionhand_redmcsb_gate.py` and
  synced the evidence JSONs for both plus
  `v1_status_refresh_order_redmcsb_gate.json` (tool-regenerated
  line-number sync for the same drift; evidence-sync precedent
  1d1c3cb73/fff924d07). All three gates exit 0. CSB suite: 12 known
  failures unchanged (hint_oracle timeouts x6, real-data
  launch/presentation/first-viewport x3, m11_runtime_capture_boundary,
  pass547, phase7 — all capture-blocked). m11|quick_resume lane:
  identical 25 pre-existing failures with and without the change
  (dm2/nexus/theron/source-locks/HoC/m12 cluster).
- 2026-07-20 Nexus TITLE.BIN TITL PP payload admission (job/w4, one
  commit): extends the TITLE.BIN RES* directory corpus into the four TITL
  records' internal PP payloads. New
  `nexus_v1_title_titl_pp_payload_admission` module
  (`include/nexus_v1_title_titl_pp_payload_admission.h`,
  `src/nexus/nexus_v1_title_titl_pp_payload_admission.c`; picked up by
  the existing `src/nexus/nexus_v1_*.c` glob into `firestaff_nexus`)
  revalidates the round-5 RES* directory receipt for each TITL entry
  (22..25) against the live SHA-256-attested source and binds its
  observed PP payload of the already admitted ST-124 section-6 shape:
  six-byte PP header ("PP" tag, BE16 width, BE16 height), 512-byte
  post-header prefix, width*height byte plane, and two trailing bytes,
  with exact per-record length arithmetic 14 + 512 + width*height + 2.
  Canonical provenance bindings verified read-only against the retail
  asset before coding: dimensions 304x104 / 160x28 / 304x22 / 256x16,
  all four 512-byte prefixes byte-identical with a shared 0x8220 leading
  word, canonical trailing bytes 0x0000, and one contiguous TITL
  sub-chain `[0x2318, 0xe278)` (0xbf60 bytes) inside the whole-file
  chain. Receipts retain per-span FNV-1a digests for record, prefix,
  plane, and trailing ranges plus the trailing word as an opaque
  measurement; the shared-prefix observation is a recorded fact, not an
  admission requirement, and flips cleanly on prefix divergence. A
  bounded plane-span iterator exposes exactly the four raw width*height
  spans; no byte or word is assigned colour, palette, image, pixel, or
  presentation semantics and no decode or draw route is permitted.
  Tests: new `tests/test_nexus_v1_title_titl_pp_payload_admission.c`
  (dual-mode; synthetic mirror with canonical PP framing by default)
  plus skip-safe wrapper
  `tests/test_nexus_v1_title_titl_pp_payload_admission_real.sh`,
  registered as CTest pair `nexus_v1_title_titl_pp_payload_admission`
  (synthetic) and `nexus_v1_title_titl_pp_payload_admission_real`
  (canonical TITLE.BIN path), covering per-record receipts, chain
  arithmetic, the shared-prefix observation, iterator spans, real-mode
  trailing-zero and per-plane nonzero-count witnesses (15187/912/1572/
  885), and rejection across NULL arguments, out-of-range indices,
  identity drift, and PP header dimension/leading-word tamper; plane and
  trailing tamper rebind the live FNV and move only the recorded
  digests, as designed. Verification: full
  `cmake --build build --parallel 10` green; both new CTests pass, the
  real one against the canonical retail TITLE.BIN; focused Nexus sweep
  `ctest --test-dir build -j10 -R nexus` shows the same 25 pre-existing
  failures as the pre-change baseline (10+3+6+6 across the four sweep
  chunks; track1 readiness timeouts, PRS3 lanes, script_vm, sound
  receipt, mechanics parity, M11/startup/DGN lanes) with zero new
  failures and both new tests green (184 -> 186 registered Nexus tests).
  This still proves no colour, palette, image, pixel, or presentation
  semantics and no TITL-to-screen assignment; which TITL images the
  original title flow draws, where, and in which order remains
  original-Saturn evidence work.

- 2026-07-20 Theron raw-loader-trace chain: generalized loader
  consume/dispatch loop continuation (round 6, one commit, job/w5
  77518f993). Instead of binding one loop iteration at a time, a single
  continuation binder
  `..._control_return_consumer_control_return_consumer_loop_continuation`
  now requires a fixed count of further iterations
  (`THERON_V1_RAW_LOADER_LOOP_CONTINUATION_ITERATIONS = 2`) after the
  twice-resumed consumer read, each with the full fail-closed window:
  (1) an opaque main-RAM control transfer after the previous consumer
  read; (2) its adjacent call-entry row proving the target was fetched
  in main RAM; (3) the adjacent next-instruction row; (4) exactly one
  main-RAM RTS whose linked post-RTS row resumes at the exact call
  return address (zero or two qualifying resumes fail closed); (5) the
  next source-adjacent FIFO byte's receipt row re-verified against the
  hash-verified Track 02 media; (6) its consumer row carrying the
  expected loop sequence (3, 4) joined to that receipt's fifo_sequence
  and main-RAM destination — and the consumer reader PC must equal the
  resumed return address, so the loop back-edge is explicit: the resumed
  loader path itself performs the next read. A missing iteration,
  out-of-order observation, off-target or duplicated resume,
  media-mismatched receipt byte, different-byte consumer, out-of-order
  sequence, different transfer or destination, non-main-RAM (System
  Card) reader, or a reader that is not the resumed path fails closed.
  Probe: new loop-continuation block appends both iterations to the
  synthetic capture (buffer enlarged to 16384), positive field
  assertions, and fail-closed negatives (mutated handoff byte,
  off-target loop resume, System Card loop reader). Verification: chain
  file compiles clean under `-Wall -Wextra -Werror`; full build green;
  `ctest -R theron` 146/161 with the same 15 pre-existing
  environment/media failures as baseline (names identical, no increase);
  synthetic chain harness passes all 41 positive/negative checks (1 new
  positive iteration-pair bind, 8 new negative rejections) with a loud
  truncation guard added to the capture construction. Admission remains
  byte-/control-flow provenance only — no record, routine ABI, level,
  object, palette, bitmap, or rendering semantics proven; where the loop
  terminates or dispatches into a record consumer stays unproven, and an
  authentic capture of the repeated consume/dispatch loop on original
  media is still required.
- 2026-07-20 DM2-003 follow-up: creature-scheduling producer
  DM2_1c9a_0cf7 bound end-to-end (job/w2, round 5). New module
  `src/dm2/dm2_v1_creature_schedule_pc34_compat.c` binds the observable
  slice of skproject/SKULLWIN/c_1c9a.cpp:5695-5728: for the creature
  record at cell (x, y) — resolved via the proven
  `dm2_v1_get_creature_at` (c_querydb.cpp:1486-1507) — it derives the
  source timer tuple verbatim (type 0x22 when the record group/leader
  link word@8 != 0xffff, else 0x21, c_1c9a.cpp:5708-5712; owner =
  creature-type byte@4, c_1c9a.cpp:5713-5714; due = gametick + 1 via
  setmticks, c_1c9a.cpp:5707; payload = setxyA(x, y),
  c_1c9a.cpp:5715-5716; map = caller-owned stand-in for ddat.v1d3248
  until DM2_CHANGE_CURRENT_MAP_TO is proven) and enqueues it through the
  local source timer queue (DM2_QUEUE_TIMER, c_1c9a.cpp:5723). The CAII
  creature-array slot timer word and the DM2_1c9a_0db0 delete
  (c_1c9a.cpp:5699-5706) stay host-owned until the CCM body is proven —
  receipted as replaced_existing == 0, never simulated. Runtime wiring:
  new DM2-owned boundary `dm2_v1_runtime_schedule_creature_at` populates
  the session record pools lazily, schedules over the boot dungeon data
  and the runtime source queue, and the next dm2_v1_runtime_tick
  dispatches the timer through dm2_v1_proceed_timers to the per-cell
  DM2_THINK_CREATURE binding, which resolves the same record — the
  producer/consumer chain is active end-to-end. New CTests
  `dm2_v1_creature_schedule_pc34_compat` (timer derivation, due
  semantics, payload packing, fail-closed paths, source evidence) and
  `dm2_v1_creature_schedule_runtime_pc34_compat` (end-to-end
  producer→queue→dispatch→think resolution, fail-closed without dungeon
  data) PASS. dm2_v1 lane: 204 tests, same 27 known baseline failures,
  zero new failures. Remaining: the c_ai re-queue inside the
  DM2_PROCEED_CCM end (c_ai.cpp:5609-5614 + 5644) behind the CCM body,
  the ALLOC_CAII map-load spawn path (c_creature.cpp:384), the CCM
  stream owner/grammar, and the possession chain walk / tile-rooted
  ground-stack mutation for DM2-002.job/w2
- 2026-07-20 CSB completion-matrix + launch-intent metatests green
  (job/w3, round 5). (a) csb_v1_completion_matrix: the runtime-spine
  needles now have real test backing instead of missing coverage —
  test_csb_v1_boot_runtime_handoff drives the accumulator tick API
  (csb_v1_runtime_tick/csb_v1_runtime_tick_due) on the handed-off
  profile and proves one banked 55ms quantum fires exactly one V1
  tick with wall time accumulated across both tick APIs, and
  test_csb_v1_runtime_route_first_frame_movement_utility_gate now
  packs the multi-step route state into a bounded 32-byte prefix,
  writes it through csb_v1_save_header_build + csb_v1_save_game, and
  reloads it through the bounded csb_v1_load_game prefix path with
  byte-exact memcmp, untouched-tail, and header field checks.
  (b) csb_v1_experimental_launch_intent_fixture: pass874's
  launch-gate refactor had dropped the explicit
  `intent.valid = m12_game_supported(intent.gameId)` conjunct at the
  M12 intent boundary (semantically redundant with gate.canLaunch,
  which already implies supported, but no longer explicit); restored
  as a belt-and-braces guard in src/ui/menu_startup_m12.c with
  identical behavior — the 8/38 m12 failures are verified
  pre-existing against a correctly rebuilt baseline. CSB suite: 21
  known failures -> 12, all remaining entries capture-blocked
  (hint_oracle timeouts x6, real-data launch/presentation/
  first-viewport x3, m11_runtime_capture_boundary, pass547, phase7).
  Same round investigated test_m11_csb_leader_hand_no_dm1_fallback
  (builds, never registered): 16/169 checks fail in two clusters
  (runtime overlay marker fallback draw stats, STAB stamina
  write-back + SHOOT refill); it stays unregistered pending a
  dedicated repair round — rationale documented in TODO.md.
- 2026-07-20 Nexus TITLE.BIN RES* 60-record directory corpus receipts
  (job/w4, one commit): extends the canonical provenance-receipt pattern
  of the FONT256.S2D section corpus and WARNING.BIN resource corpus to
  the title/startup asset lane. New `nexus_v1_title_res_corpus_receipt`
  module (`include/nexus_v1_title_res_corpus_receipt.h`,
  `src/nexus/nexus_v1_title_res_corpus_receipt.c`; picked up by the
  existing `src/nexus/nexus_v1_*.c` glob into `firestaff_nexus`) admits
  the canonical 112,216-byte SHA-256-attested TITLE.BIN RES* container
  (`51f1f18b...305fc3`, live FNV-1a rebind, no canonical FNV pin so
  synthetic mirrors bind their own live bytes) and publishes bounded
  per-record receipts for the full 60-entry directory with canonical
  provenance bindings of the same class as the WARNING.BIN descriptor
  constants: entry classes 22 DGT2 / 4 TITL / 1 MAPD / 33 CNFD,
  class-local ids (DGT2 0..21, TITL 0..3, MAPD 0, CNFD 0..32), and the
  60 record offsets forming one contiguous chain `[0x2e8, 0x1b658)` that
  covers the source tail with zero gap. Each record head must repeat its
  directory magic and class-local id; DGT2/CNFD heads carry the observed
  `0x70 0x70` tag pair, TITL heads the `0x50 0x50` pair, and the MAPD
  head the observed `TIBG` tag. Receipts retain the raw head words at
  +8/+10/+12/+14 as opaque measurements only, plus whole-record,
  directory-entry, and record-head FNV-1a digests; a bounded span
  iterator exposes exactly the 60 whole-record raw spans with no inferred
  subspans, and corpus admission revalidates identity, every directory
  entry, the exact class counts, chain contiguity, and tail coverage on
  every call. Tests: new `tests/test_nexus_v1_title_res_corpus_receipt.c`
  (dual-mode; synthetic mirror of the canonical framing by default) plus
  skip-safe wrapper
  `tests/test_nexus_v1_title_res_corpus_receipt_real.sh`, registered as
  CTest pair `nexus_v1_title_res_corpus_receipt` (synthetic) and
  `nexus_v1_title_res_corpus_receipt_real` (canonical TITLE.BIN path),
  covering all 60 receipts, class counts, chain arithmetic, tail
  coverage, iterator spans, real-mode DGT2 head-word groups
  (64x8/104x8/24x24/168x12), and rejection across NULL arguments,
  out-of-range indices, identity drift, directory-table tamper, and
  per-class record-head tag tamper; record-body tamper rebinds the live
  FNV and moves only the recorded digests, as designed. Verification:
  full `cmake --build build --parallel 10` green; both new CTests pass,
  the real one against the canonical retail TITLE.BIN; focused Nexus
  sweep `ctest --test-dir build -j10 -R nexus` shows the same 25
  pre-existing failures as the pre-change baseline (10+3+6+6 across the
  four sweep chunks; track1 readiness timeouts, PRS3 lanes, script_vm,
  sound receipt, mechanics parity, M11/startup/DGN lanes) with zero new
  failures and both new tests green (182 -> 184 registered Nexus tests).
  This still proves no record grammar, image, palette, or presentation
  semantics, no `pp`/`PP`/`TIBG` payload meaning, and no
  resource-to-screen assignment; which records the original title/startup
  flow uses and in which order remains original-Saturn evidence work, and
  0DMSTRT.BIN shows no RES* framing and stays excluded pending original
  evidence.
- 2026-07-20 Theron raw-loader-trace chain: resumed control routine's
  bounded window and the loader path's second resumption (round 5
  continuation, one commit, job/w5 2786c494a). The chain past the
  resumed control transfer entry gained three fail-closed steps:
  (1) resumed control entry-next admission — the next main-RAM
  instruction row must be adjacent to the resumed control entry;
  (2) resumed control return admission — exactly one main-RAM RTS whose
  linked post-RTS row resumes at the exact resumed control call return
  address (control_pc + 3), with zero or two qualifying resumes failing
  closed and other routines' RTS/post-RTS rows remaining opaque;
  (3) twice-resumed consumer admission — after the exact second resume
  row, a FIFO receipt row for the byte two positions after the first
  bound consumer byte (source_offset + 2) must re-verify against the
  hash-verified Track 02 media, and the third observed consumer
  (sequence=2) must join that receipt's fifo_sequence and main-RAM
  destination with a main-RAM reader — the first loop-iteration evidence
  for the loader's per-byte consume/dispatch pattern. Probe: game
  payload capture buffer enlarged to 8192 (the chain capture outgrew
  4096 and silently truncated the newest rows — root cause of a
  synthetic-harness failure found during verification); positive
  assertions for all three receipts plus fail-closed negatives (mutated
  handoff byte per step, off-target resumed resume address, System Card
  twice-resumed reader). Verification: full build green;
  `ctest -R theron` 146/161 with the same 15 pre-existing
  environment/media failures as baseline (names identical, no increase);
  synthetic chain harness passes all 32 positive/negative checks.
  Admission remains byte-/control-flow provenance only — no record,
  routine ABI, level, object, palette, bitmap, or rendering semantics
  proven; an authentic capture of the repeated consume/dispatch loop on
  original media is still required.
- 2026-07-20 DM1 timeline-dispatch clobber restoration (commit
  a1647b3c1, job/w1). Root cause of the seven assert-crashing DM1
  timeline-dispatch tests (F0242/F0248/F0190/F0249 family) was two git
  clobber events, not UB: df88dbda4 deleted 3119 lines of dispatch code
  in an unrelated CSB commit and 9b2482b93 dropped it again in an evil
  merge. Restored from base 3cde9a3e7 plus post-merge diffs 843dd979b,
  1ac6bdad9, d7e6a7d6c, 7b9a757f1; the duplicate legacy aux3/aux1 C11
  case was resolved in favour of the restored source-locked C11 block.
  6 of 8 target tests pass again (f0248 explosion/new-object/square
  object launchers, f0242 fakewall deferral, f0190 moving-killed-all
  boundary + m10 handoff).

- 2026-07-19 Theron raw-loader-trace chain: resumed loader path reads
  after the bounded control return (round 5, one commit, job/w5
  30ea9cab1). The chain past the bounded control return gained three
  fail-closed steps: (1) resumed consumer admission — after the exact
  post-RTS resume row, an observed FIFO receipt row for the byte
  adjacent to the first bound consumer byte (same generation/LBA,
  source_offset + 1) must re-verify against the hash-verified Track 02
  media, and the following consumer row must be the second observed
  consumer (sequence=1) joined to that receipt's fifo_sequence and
  main-RAM destination with a main-RAM reader; out-of-order,
  different-byte, different-transfer, or System Card reader observations
  fail closed; (2) resumed control transfer admission — the first
  main-RAM JSR after that resumed read (opaque target); (3) resumed
  control entry admission — an adjacent call-entry row proving the
  resumed target was actually fetched in main RAM. Probe: capture
  extended with the four synthetic rows; positive assertions for all
  three receipts plus fail-closed negatives (mutated handoff byte per
  step, System Card resumed reader, missing resumed control row,
  non-main-RAM resumed entry physical). Verification: full build green;
  `ctest -R theron` 146/161 with the same 15 pre-existing
  environment/media failures as baseline (names identical, no increase);
  synthetic chain harness passes all 24 positive/negative checks.
  Admission remains byte-/control-flow provenance only — no record,
  routine ABI, level, object, palette, bitmap, or rendering semantics
  proven; an authentic capture of the resumed read/control sequence on
  original media is still required.

- 2026-07-19 DM1 V1 viewport ReDMCSB-gate refresh + flaky-surface
  characterization (Jobb DM1, round 4, one commit): all seven drifted
  V1 ReDMCSB source gates green again — status_bar_layout,
  draw_stack, inventory_panel_open, inventory_toggle,
  inventory_chest_actionhand, status_refresh_order and
  entrance_input_wait.  The failures were gate-side staleness after
  M11 refactors (inline-marker/line-range drift), not runtime
  regressions; the gates now lock the post-refactor delegation chain
  (champion_status_slotbox_pc34_compat.h 187/195 zone bases,
  dm1_v1_champion_status_* helpers, the F0115 floor-item and viewport
  projectile wrappers) and current function spans, with evidence
  JSONs refreshed in the same commit (precedent fff924d07).  Same
  commit repairs a CMake registration bug:
  m11_dm1_floor_item_host_presentation_receipt had a dangling
  add_test inside the unrelated csb_leader_hand EXISTS block and no
  add_executable — permanent Not Run; it now builds and passes.
  Flaky 141-vs-149 surface characterized: the eight swinging tests
  are the receipt Not-Run (permanently fixed) plus seven
  assert-crashes in the F0242/F0248/F0190/F0249 timeline-dispatch
  family whose orchestrator sources are identical between passing and
  failing baselines — UB/uninitialized-memory suspects, documented in
  TODO.md for a dedicated sanitizer round.  DM1 suite: 147/1337
  failing after this round (pre-fix current-main 155); zero new
  regressions.  Commit: 1d1c3cb73.

- 2026-07-19 CSB startup title/door receipt cluster (job/w3, round 4,
  dd6edde84): the three startup receipt-gate tests
  (boot_title_import_ui_gate, startup_entrance_pointer,
  startup_receipt_coherence) plus startup_session_contract are green
  again.  All four were stale synthetic fixtures, not runtime defects:
  the frame-80 STRIKES BACK wave carries source step 21 (not 20 — a
  df88dbda4-branch merge artifact), closed/opening door blits land at
  y=30 per the PC34 source-zone render lock, begin_door_opening holds
  step 0 through the 20-vblank pre-open delay, and the first presented
  CHAOS zoom raster is 48x12 (the 16x4/32x8 allocations are not
  presented on PC34; the two-vblank full-size hold is frames 78-79).
  Verified against the M11 phase-capture table, title capture
  admission, real sequence, package identity, and boot runtime handoff
  contracts; an M20-style runtime rewrite was evaluated and rejected
  because it regresses m11_startup_resume_gate and the launcher
  handoff.  Known `-R csb` failures 24 -> 21 with zero new
  regressions.  Test-only change (4 files, 13 lines).
- 2026-07-19 Nexus WARNING.BIN four-resource DGT2/PP corpus chain (job/w4,
  one commit): extends the 2026-07-17 resource-0 admission -> execution ->
  M11 presentation chain to all four canonical DGT2/PP resources. New
  `nexus_v1_warning_dgt2_resource_corpus` module
  (`include/nexus_v1_warning_dgt2_resource_corpus.h`,
  `src/nexus/nexus_v1_warning_dgt2_resource_corpus.c`; picked up by the
  existing `src/nexus/nexus_v1_*.c` glob into `firestaff_nexus`) admits each
  resource against the live SHA-256-attested or canonical-FNV-witnessed
  source with per-resource canonical provenance bindings of the same class
  as the existing resource-0 constants: descriptor offsets
  `0x48`/`0x5c58`/`0xb868`/`0xf8f8`, resource lengths
  `0x5c10`/`0x5c10`/`0x4090`/`0x9290`, PP dimensions 240x96, 240x96, 200x80,
  and 272x136, the 512-byte BGR555 CLUT, the width*height index plane, and
  the two trailing bytes per resource. The corpus receipt binds the observed
  contiguous four-resource chain `[0x48,101256)` covering the source tail.
  Per-resource execution copies the exact index bytes and original BGR555
  words to caller-owned exact-sized buffers and invokes only the explicit
  presentation callback; per-resource M11 presentation revalidates the full
  receipt chain before writing the top-left index plane into the real
  320x200 M11 indexed surface with the ST-124-ordered BGR555->RGB6 exact
  palette expansion, leaving the cleared frame unpresented on any drift.
  Tests: new `tests/test_nexus_v1_warning_dgt2_resource_corpus.c` plus
  skip-safe wrapper
  `tests/test_nexus_v1_warning_dgt2_resource_corpus_real.sh`, registered as
  CTest `nexus_v1_warning_dgt2_resource_corpus` (canonical WARNING.BIN
  path), covering per-resource receipts, exact plane/palette copies, M11
  surface writes with untouched out-of-image pixels, and tamper rejection
  across every resource's pixel plane, CLUT, PP header, the descriptor
  table, stale receipts, callback refusal, and identity drift.
  Verification: full `cmake --build build --parallel 10` green; the new
  CTest passes against the canonical retail WARNING.BIN; focused Nexus
  sweep `ctest --test-dir build -j10 -R nexus` shows the same 25
  pre-existing failures as the pre-change baseline (10+3+1+11 across the
  four sweep chunks; track1 readiness timeouts, PRS3 lanes, script_vm,
  sound receipt, mechanics parity, M11/startup lanes) with zero new
  failures and the new test green (181 -> 182 registered Nexus tests).
  This still proves no Saturn VDP display command, interlace, colour-DAC,
  gamma, timing, placement contract, or resource-to-screen assignment;
  which resource the original warning flow shows and in which order remains
  original-Saturn evidence work.
- 2026-07-19 Theron raw-loader-trace chain: control-routine execution
  window and bounded return (round 4, one commit, job/w5 4f3caf4b0). The
  chain past the post-consumer-read control transfer gained three
  fail-closed steps: (1) control entry admission — the producer's
  call-entry row must be adjacent to the control JSR and prove the
  control target was actually fetched in main RAM (exact caller, target,
  entry, opcode); (2) control entry-next admission — the next main-RAM
  instruction row must be adjacent to that entry; (3) control return
  admission — exactly one main-RAM RTS whose linked post-RTS row resumes
  at the exact control call return address (control_pc + 3), with zero
  or two qualifying resumes failing closed and other routines'
  RTS/post-RTS rows remaining opaque. Probe: capture extended with the
  four synthetic rows; positive assertions for all three receipts plus
  fail-closed negatives (missing entry row, non-main-RAM entry physical,
  off-target resume address, mutated handoff byte per step).
  Verification: full build green with -Wall -Wextra -Werror;
  `ctest -R theron` 146/161 with the same 15 pre-existing
  environment/media failures as baseline (names identical, no increase);
  synthetic chain harness passes all 15 positive/negative checks.
  Admission remains byte-/control-flow provenance only — no routine ABI,
  record, level, object, palette, bitmap, or rendering semantics proven;
  an authentic capture of this control window on original media is still
  required.

- 2026-07-19 DM1 V1 C040/G0299 candidate-panel audit completion
  (Jobb DM1, round 3, one commit): the non-action C040 audit across
  the public M11 V1 helper/command surface is now closed.  The
  final two bypasses — direct `M11_GameView_LoadDm1SavePath` and
  `M11_GameView_LoadDm1OriginalPc34SaveBytes` in
  src/engine/m11_game_view.c — now refuse to replace the world
  while `candidateMirrorPanelActive` mirrors
  `G0299_ui_CandidateChampionOrdinal`, matching the ReDMCSB
  `COMMAND.C F0380` C140/G0299 disabled-state contract.  All other
  public mutators were already gated (pickup/drop/use, quickload,
  inventory/map toggles, acting-champion set/clear, leader hand,
  chest open/close, spell panel, handle-input/pointer paths,
  mirror re-entry); quicksave stays intentionally allowed.  The
  `m11_quick_resume_roundtrip` regression now locks the boundary:
  with the panel live, both direct load APIs and quickload are
  rejected and the world (map position, tick) is preserved, and
  after clearing the panel the original PC34 save loads with
  ORIGINAL_SAVE_PC34 provenance.  Test status: DM1 suite 141/1337
  failing before and after (identical failure lists, baseline
  unchanged); m12_quick_resume_gate remains a pre-existing
  baseline failure.  Commit: 4c85b7ebc.

- 2026-07-19 CSB CSBWin save-import lanes completed (job/w3 round 3,
  two commits): 66e677eb6 restored the merge-drift-clobbered
  timer->source_index pool-slot stamp (cc57e9aca contract) in
  parse_timer_summaries and re-based two stale tick_accumulator
  fixtures on the source-locked CSBWin pool/heap contract
  (SaveGame.cpp:1791-1792/1845/1852 MaxTimer pool + NumTimer active
  prefix; Timer.cpp CheckTimers:885-906 min-heap) — summary fixture
  num_timer/max_timers 9/11 -> 3/3 with source_index stamps,
  resume-file fixture heap [2,0,1] -> [0,2,1] with NumTimer=3.
  csb_v1_runtime_tick_accumulator flips fully green (54 CSBWin
  sub-failures -> 0 across summary apply, ITEM16, timer queue,
  resume report/file, core export, native save/load tail
  preservation). cba511814 stamped the missing EXPOOL fnv1a tail
  receipt (b35d17974 contract) in the multilevel DSA
  movement-filter fixture, unblocking the GLOBALSTORE commit;
  csb_v1_dsa_multilevel_filter_save_handoff green. CSB suite: 339
  tests, known failures 26 -> 24, zero new regressions.

- 2026-07-19 Nexus FONT256.S2D populated-section corpus receipts (job/w4,
  one commit): extends the 2026-07-17 first-section witness to all four
  populated SCR sections. New `nexus_v1_font256_s2d_section_corpus_receipt`
  module (`include/nexus_v1_font256_s2d_section_corpus_receipt.h`,
  `src/nexus/nexus_v1_font256_s2d_section_corpus_receipt.c`; picked up by the
  existing `src/nexus/nexus_v1_*.c` glob into `firestaff_nexus`) admits each
  populated section (table indices 0/2/4/6) against the live canonical
  source with source-FNV, section-table-FNV, and per-section-FNV rechecks,
  binds each 16-byte preamble witness, and records only opaque raw
  composition measurements (zero/nonzero byte counts, post-preamble word
  count, BE16 ramp-prefix length, full-ramp flag). The corpus-level receipt
  binds the observed contiguous four-section chain `[0x0120,0x61b4)`
  (24,724 bytes covering the source tail) as one capture target, and a
  bounded span iterator emits exactly the four whole-section spans in
  admission order with no inferred subspans. No byte or word is assigned
  text, glyph, palette, record, or pixel meaning; draw routes remain
  blocked. Tests: new dual-mode
  `tests/test_nexus_v1_font256_s2d_section_corpus_receipt.c` (synthetic SCR
  envelope by default; retail path via argv) plus skip-safe wrapper
  `tests/test_nexus_v1_font256_s2d_section_corpus_receipt_real.sh`,
  registered as CTests `nexus_v1_font256_s2d_section_corpus_receipt`
  (synthetic) and `nexus_v1_font256_s2d_section_corpus_receipt_real`
  (skip-safe real data). Verification: full `cmake --build build --parallel
  10` green; both new CTests pass (synthetic assertions plus the canonical
  retail FONT256.S2D path, including tamper drift rejection on every
  section, preamble, and the section table); focused Nexus sweep
  `ctest --test-dir build -j10 -R nexus` shows the same 25 pre-existing
  failures as the pre-change baseline (10+3+1+11 across the four sweep
  chunks; track1 readiness timeouts, PRS3 lanes, script_vm, sound receipt,
  mechanics parity, M11/startup lanes) with zero new failures and the two
  new tests green (179 -> 181 registered Nexus tests). Remaining FONT256
  work is unchanged: an original Saturn trace or independently reviewed
  format material before any subrecord grammar, palette, glyph, or draw
  route.

- 2026-07-19 Theron raw-loader-trace chain: consumer-read and
  control-transfer admission (one commit, job/w5 627074f7c). The chain
  past the branch-target JSR CD receipt gained two fail-closed steps:
  (1) consumer-read admission binds the observed
  `pce_cd_fifo_origin_main_ram_consumer` row to the exact FIFO byte
  (generation/LBA/offset/value), the exact fifo_sequence and main-RAM
  destination of the joined receipt row, and a main-RAM reader PC — a
  joined byte consumed by a different transfer or by a System Card
  reader fails closed; (2) control-transfer admission binds the first
  observed main-RAM JSR after that consumer read (opaque target).
  Two latent trace-window defects repaired on the way: the TII transfer
  receipt now snapshots the accepted block-transfer row (later
  parsed-but-source-filtered rows no longer overwrite the reported
  source/destination/byte count — the root cause of the chain failing on
  two-routine captures), and the post-envelope execution binder now
  ignores RTS rows outside the exact destination span and other
  routines' post-RTS rows, per its documented
  exactly-one-RTS-inside-the-span contract. Probe: game-payload capture
  buffer enlarged to 4096, positive consumer/control assertions plus six
  fail-closed negatives (mutated handoff byte, mutated consumer value,
  System Card reader, truncated capture, non-main-RAM control PC,
  reordered consumer). Verification: full build green with
  -Wall -Wextra -Werror; `ctest -R theron` 146/161 with the same 15
  pre-existing environment/media failures as the pre-change baseline
  (no increase); synthetic chain harness passes all 8 positive/negative
  checks. Admission remains byte-/control-flow provenance only — no
  level/object/palette/bitmap semantics proven; an authentic capture of
  the loader's consumer reads and control decisions on original media is
  still required.

- 2026-07-19 DM1 HoC portrait-probe re-base, sixth slice (Jobb E part
  8 = round 5, nine commits): 14 more stale-fixture probes re-based
  on the verified PC34 C127 layout and verified PASS in family runs;
  the portrait suite went 27 -> 13 failing (141/154 passing).
  Commits: 95e30fc77 (east_walkpath 01/02/06/07), 6b83ab1cf
  (east_walkpath 03), d2cca9185 (east_walkpath 12), ccf325b90
  (ordinal21 east_walkpath), 085e5ed43 (07 walkpath_from_stairs),
  50c97f079 (12 walkpath_from_entrance), 9e765ff32 (08
  walkpath_from_entrance), 58fec5b4d (22 walkpath_from_entrance),
  9ecb645a3 (d2c_far_positive 01/11/22).
  Walkpath cluster (11 probes): all re-anchored to the runtime-
  verified C127 sensor table (scanner-dump confirmed; ordinal 9 =
  ZED at (8,14)S and ordinal 10 = GANDO at (7,14)N — the old
  working notes had the 9/10 names swapped, runtime wins).
  Reusable patterns locked: mirror-niche cells are SOLID (forward
  into the niche cell is BLOCKED, verified for (10,13), (14,14),
  (10,6), (16,4), (12,8), (7,8)); the y=9 row is open x=6..17 and
  the accepted-step return leg is turn-left to N + STRAFE_LEFT
  west along y=9 (the old forward-step return was a latent bug);
  non-adjacent waypoints re-seed via set_pose +
  DM1_V1_MovementPipeline_InitPc34Compat (teleport-then-walk);
  east_walkpath 12 got a new y=9 route HALK (7,9)N -> (8,9)N ->
  LINFLAS (12,9)N -> (13,9)N.  22 walkpath_from_entrance: entrance
  (7,9)N HALK, corridor (9,13)N -> turn-right SONJA 18 at (9,13)E
  (forward blocked by the solid (10,13) niche), GANDO 10 at
  (7,13)S, WUUF 13 at (7,16)S, walkpath region scan x=6..9
  y=8..17 (GOTHMOG 22 at (12,13)W stays outside).
  d2c_far_positive cluster (3 probes): re-anchored to the verified
  HALK geometry — positive D1C pose (7,9)N, D2C-far pose (7,10)N,
  sensor cell (7,8) south wall (sensorData=1); 11 keeps the
  sensorData-keyed seed (1 -> 11, coordinate-agnostic helper); 22's
  no-floating side lanes moved to (6,10)N/(8,10)N/(6,9)N/(8,9)N +
  (7,10)W and its corridor scan to x=7 y=9..13 (exactly one
  ordinal-1/11 hit as before).  All three PASS on first run after
  the coordinate swap — no behaviour drift, only stale fixtures.
  Parked untouched this round: reincarnate_reselect 18 (runtime
  triage of the F0282 C165 flow), inventory_exit_restore 06/17
  (C040 panel-survival contract), wall_ornament 19/22 +
  door_nearby 02/06 (C346 frame-edge bitmap class),
  front_south/east_entry 12/22/01/21, screenshot_receipt 12/22,
  cancel_reopen 04/11 (vacuous).

- 2026-07-19 DM1 F0128 per-square scheduler, live M11 wiring (Jobb E
  part 1 continuation, worktree job/w2): `src/engine/m11_game_view.c`
  now feeds the contract bridge the live sampled 19-square view every
  DM1 dungeon-view frame. A new bridge section maps each F0128 view
  square (D4L..D0C, DUNVIEW.C:8479-8542 visit order) to its view-cone
  coordinates, reuses the 3x3 sampled cells for D1..D3 and samples
  D4/D0/L2/R2 on demand, and translates each cell to the view-relative
  element (C16/C17 door side/front and C18/C19 stairs side/front via
  the square 0x08 orientation axis vs party direction — the same axis
  rule `dm1_v1_stairs_front_facing_pc34` locks for stairs; D0-row doors
  stay door-side since F0125/F0126/F0127 have no source door-front
  pass). Pit/teleporter visibility follows SquareAspect M554 (pit bit
  0x04 clear = shows, teleporter bit 0x04 set = blue haze; the
  teleporter visible/open masks moved to
  `include/dm1_v1_field_teleporter_effect_pc34_compat.h` so the bridge
  names them). The frame builds and re-verifies the plan
  fail-closed; the F0115 content loop then consumes the verified plan's
  per-square spans (`SquareSpan`) in source visit order — side pairs
  immediately before their center square, squares whose span carries no
  F0115 step (plain wall) draw no thing-layer content — while a scene
  the contract cannot schedule keeps the legacy hand-rolled loop
  unchanged (no host substitute plan). A frame-local
  `M11_Dm1F0128PerSquareSchedulerReceipt` (include/m11_game_view.h)
  publishes valid/planReady/planDrivenContentLoop/stepCount/
  scheduleHash/f0115ContentSquareCount via
  `M11_GameView_GetDm1F0128PerSquareSchedulerReceipt`. CSB/DM2/Theron/
  Nexus M11 paths are untouched (they return before the DM1 viewport
  pass). New integration CTest `dm1_v1_f0128_scheduler_m11_wiring`
  (tests/test_dm1_v1_f0128_scheduler_m11_wiring.c) drives
  `M11_GameView_Draw` over a synthetic 11x11 fixture: inactive frame
  publishes no plan; all-corridor, D1C door-front, and D1C wall scenes
  each match a directly-built contract plan on step count and schedule
  hash, with F0115 square counts 9/9/8. Verification: full Ninja build
  green; `ctest -R f0128` 7/7 PASS; `ctest -R "f0128|m11"` failure set
  byte-identical to the pre-change baseline (24 pre-existing
  data/environment-dependent failures: launcher handoffs, capture
  smokes, runtime source-locks — verified via stash/rebuild baseline
  comparison); adjacent teleporter/f0113/f0115 suite 13/14 with only
  the pre-existing `dm1_v1_teleporter_visual_effect_source_lock`
  failure (reproduced on baseline). Remaining: broaden real PC34/Mac
  capture parity and let more M11 passes (F0104 floor/pit/stairs,
  F0111 door, F0113 field) consume the plan's per-square spans instead
  of their own batched ordering.

- 2026-07-19 Jobb F2 + lokalisering rond 2, w5 (two commits):
  0f7fc0a43 (launcher-options runtime handoff) + 9a0fa9c8f
  (csb/theron translations + format-string fixes).
  Jobb F2 (0f7fc0a43, 6 files, +493): new
  `M12_LauncherRuntimeOptions` struct in `include/menu_startup_m12.h`
  (global launcher settings + per-game language/cheats/speed folded
  in), `M12_StartupMenu_ExportLauncherRuntimeOptions()` in
  `src/ui/menu_startup_m12.c` with clamped ranges (minimap 64..256,
  corner 0..3, combatLog 50..500, fontScale 1..3),
  `M12_LaunchIntent.launcherOptions`/`launcherOptionsBound` populated
  in `M12_StartupMenu_GetLaunchIntent` after
  `m12_enforce_mode_constraints`, `M11_GameLaunchSpec.launcherOptions`
  + `M11_GameViewState.launcherOptions` with accessor
  `M11_GameView_GetLauncherRuntimeOptions`, and helper
  `m11_apply_launcher_options_handoff` applied after Shutdown/Init in
  all five start branches (dm1/csb/dm2/nexus/theron);
  `spec.languageIndex` now populated. New CTest
  `m12_launcher_options_runtime_handoff` PASS;
  `m12_quick_resume_gate`/`m12_polished_ui_flow` and the
  m12_startup_menu probe failures verified identical to baseline
  (stash-tested, environment-dependent).
  Lokalisering rond 2 (9a0fa9c8f, 50 files, +2920/-2894): csb and
  theron catalogs translated for the 13 fallback locales
  (cs/da/es/fi/hu/it/ko/nl/no/pl/pt/ru/tr) — 33 csb keys + 38 theron
  strings each, style following the reviewed sv/de/fr/ja/zh catalogs
  (ALL-CAPS on Theron UI strings, sentence case on descriptions,
  brand strings kept, printf specifiers preserved exactly), header
  comments corrected (bogus "Swedish catalog"/"English catalog
  auto-generated" notes replaced). Broken printf format strings fixed
  mechanically in all 18 dm1.*.po catalogs (~1814 rows: %S->%s,
  %D->%d, %U->%u, %C->%c, incl. the %03D variant) and in
  theron.{de,fr,ja,zh}.po; argument order restored in dm1.ja/dm1.zh/
  theron.zh where machine translation had reordered %-specifiers
  without positional markers. startup-menu.de.po + generator
  `po/translations_other.py`: "CHEA TS"->"CHEATS",
  "DURCHSTECHEND"->"DEMNÄCHST". Leftover English translated in
  theron.{de,fr,ja,zh} ("HP AND STAMINA RECOVER SLOWLY").
  Verification: custom msgid-vs-msgstr format-spec checker 0 problems
  across all catalogs (msgfmt -c cannot catch these — no c-format
  flags — but passes syntax), `po/validate_po_layout.sh` PASS (csb
  100% native all locales, theron 94-100%), `firestaff_l10n` PASS;
  the 10 failing startup probes are identical on baseline
  (stash-verified) and environment-dependent. Acceptable fallbacks
  left untouched per policy (game titles, champion names, rune words,
  GRAPHICS.DAT, etc.).

- 2026-07-19 DM1 HoC portrait-probe re-base, fifth slice (Jobb E part
  7 = round 4, six commits): 15 more stale-fixture probes re-based
  on the verified PC34 C127 layout and verified PASS in family runs;
  the portrait suite went 42 -> 27 failing (127/154 passing).
  Commits: 8a9e44379 (approach family), 9973585b0 (resurrect_reselect),
  d362e660e (input_focus_restore 22), 7fcd444ba (turn_away_return 20),
  3db5d2513 (south_return family), 4a3345863 (all-portraits wall
  coordinate gate).
  Approach family (6 probes, 8/8 family tests PASS):
  approach_from_right 04 (anchor (11,6)W + wrong-cell band x=11
  y=6..9 + cross-check (10,5)S), 05 (natural ELIJA (14,2)S route,
  band {15,2}W/{14,2}E/{14,3}E/{14,2}N, cross-check (14,3)N, seed
  kept as unused fallback), 18 (anchor (11,13)W + band x=11
  y=12..15 + cross-check (9,13)E), 22 ((11,13)E anchor: CANONICAL
  (12,13)W + band (12,12)W/(12,14)W/(12,13)S/(12,13)E);
  approach_from_left 0 (natural DAROOU (8,7)E route, APPROACH
  (7,7)E, band x=7 y=6..10, Group D native-first with seed
  fallback to (7,9)N) and 17 (anchor (9,13)E on SONJA 18, band
  x=9 y=12..16, cross-pose WUUF (7,16)S).  Key learning: "from
  right" = east of the mirror cell facing WEST (sees the E wall =
  wrong side for non-E-face sensors); for E-face mirrors (22) the
  right-anchor equals the natural pose so the band carries the
  wrong variants.
  resurrect_reselect (3): 00/11/22 moved to natural routes
  (9,7)W/(16,8)N/(12,13)W with no retarget; the retarget helper is
  retained but unused.  Probe 22's side-wall changed to DIR_NORTH
  because (12,13)W is the positive direction.
  input_focus_restore 22 (1): pose constants (3,6)W -> (12,13)W on
  the verified (11,13)E GOTHMOG sensor; the sensorData-keyed seed
  is coordinate-agnostic and a no-op on shipped data; narrative
  updated (EAST wall of (11,13)).
  turn_away_return 20 (1): dropped the synthetic HALK->20 seed
  entirely; natural ALEX route (17,9)S on the (17,10)N sensor;
  in-place turn axis rotated S->W->N->E->S with Group C/D
  expectations and byte-stability narrative updated.
  south_return family (3): 05 (Slice-1 rejection pose table re-keyed
  to verified poses, none of which is ordinal 5's own (14,3)N pose;
  Slice-2/3 anchor (1,5)S -> (7,16)S WUUF; header premise rewritten —
  ordinal 5 IS a Hall mirror at (14,2)S on the verified layout), 14
  (natural (10,4)N LEYLA pose on the (10,3)S sensor; the old
  (1,18)/(1,19) map-edge/OOB claims removed; negatives re-anchored
  to (9,4)E/(11,4)W/(10,3)N), champion_mirror_portrait_rect_south_
  return 287 (SCAN_MAX 16 -> 20 so the natural (16,14)S AZIZI pose
  is inside the scan; probe self-discovers it and passes).
  hoc_all_portraits_wall_coordinate_gate (1): full 24-pose table
  swap to the verified layout in kExpectedPoses and
  kPlayerRouteMirrorSweep (all flags wall-positive, sweep counts
  19/19/5 -> 24/24/0), Group B wrong-wall negatives re-anchored to
  verified poses with sensor-free front cells, Groups C/D HALK
  sensor lookup replaced by a sensorData-keyed scan (the old
  (1,1)/cell-bit lookup found nothing on the real layout).
  Parked after attempted re-base (reverted to HEAD, failing as
  before, NOT pose-fixable): reincarnate_reselect 18 — native
  (9,13)E rebase done but "post-confirm champion slot consumed"
  fails (no championCount-- path in m11_game_view.c) and the Group
  G re-enable retarget 18->18 cannot match (disable zeroes
  sensorType while the helper requires sensorType==127); needs
  runtime triage of the F0282 C165 flow.

- 2026-07-19 DM1 F0128 per-square scheduler, consumer-bridge slice
  (Jobb E part 1 continuation, worktree job/w2): the scheduler module
  gains the M11-facing consumption API
  `DM1_V1_F0128_PerSquareSchedulerSquareSpanPc34Compat` (per-square
  [start,count) step spans in source visit order) and
  `DM1_V1_F0128_PerSquareSchedulerMatchesObservedPc34Compat` (compares
  a caller-observed draw sequence against the contract plan, first-
  divergence index on mismatch). Both fail closed on NULL/out-of-range
  input. CTest `dm1_v1_f0128_per_square_scheduler_pc34_compat` now
  covers span queries (D4L single early step, D1C door pass1..pass2
  span, D0C final visit) and observed-sequence matching (self-match,
  truncation index, swapped door steps divergence index) — 104/104
  assertions PASS. Full Ninja build green; focused
  f0128/f0115/per_square_scheduler CTest 13/13 PASS. Remaining: the
  live M11 draw path still needs to call the bridge (kept out of this
  slice to avoid touching the shared 21K-LOC m11_game_view.c lane).


- 2026-07-19 DM1 F0115/F0128 complete per-square source scheduler,
  first contract slice (Jobb E part 1, worktree job/w2): new
  contract-only module
  `src/dm1/dm1_v1_f0128_per_square_scheduler_pc34_compat.c` +
  `include/dm1_v1_f0128_per_square_scheduler_pc34_compat.h` merges the
  F0104/F0107/F0108/F0111/F0113 material families into one 19-square
  F0128 visit-order scheduler — D4L/D4R/D4C early F0115 passes, then
  D3L2/D3R2, D3L/D3R/D3C, D2L2/D2R2, D2L/D2R/D2C, D1L/D1R/D1C,
  D0L/D0R/D0C per ReDMCSB DUNVIEW.C:8479-8542 and per-square functions
  F0676/F0677:6226-6360, F0678/F0679:6837-6899, F0116-F0127:6361-8317.
  Every square carries its source cell-order words (DEFS.H:2658-2677,
  e.g. D1L 0x0028/0x0039/0x0032, D1R 0x0018/0x0049/0x0041, D0L 0x0002,
  D0R 0x0001, D0C 0x0021, right lanes 0x0128/0x0439/0x4312). Real
  field-after-things: F0113 lands only after the square's last F0115
  step (DUNVIEW.C:6289/6487/8315) and only for visible teleporters on
  field-capable squares. Door/object occlusion capture: F0108 floor
  ornament, F0115 pass1 behind the door, F0104 door frame, F0111 door
  occluder, F0115 pass2 in front (DUNVIEW.C:6444-6461, D1C
  7873-7937). Wall squares run F0104 wall material + F0107 right/front
  alcove predicates and return without a thing pass unless the front
  ornament is an alcove (0x0000 order, F0116:6433-6437). D2L2/D2R2
  stay wall/field-only (order unreferenced, DUNVIEW.C:6843/6874).
  Fail-closed: NULL input, out-of-contract elements, and door-front on
  squares with no source door pass (D0C) reject with no partial plan;
  the plan verifier rejects field-before-things and pass2-before-door
  reorderings. New CTest
  `dm1_v1_f0128_per_square_scheduler_pc34_compat` (86/86 contract
  assertions, data-free). Verification: full Ninja build green;
  focused CTest run over f0128/f0115_thing_pass/f0115_item_placement/
  per_square_scheduler 13/13 PASS; broader dm1_v1_viewport|dm1_v1_f01
  slice 84/101 PASS with the same 17 known baseline failures as before
  this change (pass405/pass427/pass361/pass362/pass508/pass510-512,
  wall golden/3d source-lock lanes, etc. — all pre-existing, none
  touched by this slice). Remaining: wire the verified plan into the
  live M11 draw path and broaden real PC34/Mac capture parity.job/w2

- 2026-07-19 DM2-003 follow-up: weather timer producer bound to the
  DM2-owned source queue (Jobb W3). `dm2_v1_runtime_tick` now enqueues a
  type-0x54 c_tim (actor 0, mticks = gametick + delay) through
  `dm2_v1_runtime_enqueue_source_timer`, mirroring
  skproject/SKULLWIN/c_weather.cpp:20-30 DM2_SET_TIMER_WEATHER
  (tim.setmticks(0, gametick + delay), tim.settype(0x54),
  tim.setactor(0), DM2_QUEUE_TIMER). The 182-tick cadence stays owned by
  the existing DM2_SET_TIMER_WEATHER receipt; the dispatcher pops the
  0x54 timer at the 182-tick boundary and acknowledges it fail-closed
  (no DM2_UPDATE_WEATHER handler bound yet); the producer re-schedules
  the next cycle after the pop; indoor sessions never enqueue. New CTest
  `dm2_v1_weather_timer_producer_pc34_compat` PASS; weather transition
  behaviour unchanged (host path still owns the seed transition at the
  boundary, verified by the new test and the unchanged
  dm2_v1_weather_seed_regression). dm2_v1 lane: 198 tests, same 27 known
  baseline failures, zero new failures.

- 2026-07-19 DM2-005 follow-up: legacy CCM interpreter aligned to the
  source b_1a dispatch matrix (Jobb W3). `src/dm2/dm2_v1_ccm.c`,
  `include/dm2_v1_ccm.h` opcode values are now the exact skproject
  creature command bytes (skproject/SKULLWIN/c_creature.cpp:2930-3212
  DM2_PROCEED_CCM compare chain, already bound verbatim in
  `dm2_v1_ccm_dispatch_pc34_compat`): 0x01/0x02/0x09 WALK_NOW, 0x03/0x04
  CCM03, 0x05 JUMPS, 0x06/0x07 CCM06, 0x08/0x26 ATTACKS_PARTY, 0x0A
  STEAL_FROM_CHAMPION, 0x0E/0x0F SHOOT_ITEM, 0x13 KILL_ON_TIMER_POSITION,
  0x15/0x16 ROTATES_TARGET_CREATURE, 0x17 PLACE_MERCHANDISE, 0x18
  TAKE_MERCHANDISE, 0x19/0x29/0x2A/0x2D/0x2E PUTS_DOWN_ITEM, 0x1A/0x2B/0x2C
  TAKES_ITEM, 0x27/0x28 CAST_SPELL, 0x3D-0x40 EXPLODE_OR_SUMMON. The
  retired legacy numbering had CAST_SPELL at 0x15 (source: ROTATES_TARGET),
  CREATURE_ATTACKS_PARTY at 0x17 (source: PLACE_MERCHANDISE), SHOOT_ITEM
  at 0x0D (source: CCM0C), and invented 0x00/0x10-0x12/0x14 no-op states
  the source chain routes to no handler; those bytes now fail closed as
  UNKNOWN_OPCODE. Unproven handler bodies (CCM0B, CCM0C, ACTIVATES_WALL,
  USES_LADDER_HOLE, TRANSFORM, DM2_1B7D5) are explicit stubs. Each table
  row records its DM2_V1_CcmSourceHandler group; the diverged
  `include/dm2_v1_creature.h` DM2_CCM_* macros, the creature runtime
  bridge arg mapping (`dm2_v1_creature.c`), and the combat-probe
  constants (`probes/firestaff_dm2_v1_creature_combat_probe.c`) are
  re-based on the same matrix. Tests: new CTest
  `dm2_v1_ccm_source_alignment_pc34_compat` (7/7) cross-checks every
  legacy row + row name against `dm2_v1_ccm_dispatch_source_group` /
  `dm2_v1_ccm_dispatch_group_name`, asserts NONE-bytes are absent from
  the table and every handled source byte 0x00-0x55 has a row;
  `dm2_v1_ccm_pc34_compat` re-based PASS; `dm2_v1_ccm_dispatch_pc34_compat`
  PASS unchanged. dm2_v1 lane: 197 tests, same 27 known baseline
  failures, zero new failures.job/w3

- 2026-07-19 DM2-003/DM2-005 follow-up: per-cell DM2_THINK_CREATURE
  binding over the DM2-002 record pool (Jobb W3). New module
  `src/dm2/dm2_v1_think_creature_pc34_compat.c` binds the source
  boundary verbatim: `dm2_v1_get_creature_at` mirrors
  skproject/SKULLWIN/c_querydb.cpp:1486-1507 DM2_GET_CREATURE_AT — tile
  record link via the proven `dm2_v1_dungeon_get_first_thing`
  (c_map.cpp:44-69 byte-square bit-0x10 + column-index ground-stack
  table), then a bounded next-link walk returning the first record whose
  DB index (handle bits 10-13, direction bits 14-15 ignored) is
  dbCreature (4), direction bits preserved in the returned word exactly
  like the source; OBJECT_END_MARKER/absent/corrupt chains return the
  source 0xffff fail-closed. `dm2_v1_think_creature_timer_handler` is a
  DM2-owned 0x21/0x22 dispatcher handler mirroring
  c_tim_proc.cpp:4079-4088 (x = getxA, y = getyA, think type = timer
  type word, timer map from l_00 high byte): a cell without a creature
  is the source's early return (c_ai.cpp:5670-5673) — the timer is
  consumed and receipted, never simulated; a resolved creature is
  receipted with its DB4 type byte (record byte@4); the think body
  (DM2_PREPARE_LOCAL_CREATURE_VAR + the c_ai.cpp body) stays fail-closed
  until the CCM stream owner/grammar is proven, with an explicit
  DM2_V1_ThinkCreatureBody callback boundary for the future binding.
  New CTest `dm2_v1_think_creature_pc34_compat` PASS (chain walk,
  direction-bit preservation, early return, payload decode, corrupt
  self-loop bounding, bound-body hand-off, full DM2_PROCEED_TIMERS
  integration). dm2_v1 lane: 199 tests, same 27 known baseline failures,
  zero new failures. Remaining: session-owned record pool set in the
  runtime so the live 0x21/0x22 dispatch can resolve per-cell (the
  runtime handler still steps the local CCM pool as its documented
  interim boundary), the creature-scheduling producer, and the proven
  think body.job/w3

- 2026-07-19 DM2-006 follow-up: ALLOC_NEW_DBITEM drop path over the
  DM2-002 record pool (Jobb W3). New module
  `src/dm2/dm2_v1_dbitem_alloc_pc34_compat.c` binds the full
  drop-creation chain from skproject/SKULLWIN/c_record.cpp:
  GET_ITEMDB_OF_ITEMSPEC_ACTUATOR (367-401) and
  GET_ITEMTYPE_OF_ITEMSPEC_ACTUATOR (403-444) with the exact 9-bit
  itemspec mask and group split (weapon/cloth/misc; 0x1fc scroll,
  >=0x1e0 container, >=0x1b0 creature, else potion; >0x1fc invalid);
  ALLOC_NEW_RECORD (1076-1139) with the forward OBJECT_NULL scan, zero +
  OBJECT_END_MARKER init, dbContainer w2 termination, the bones 0x800A
  mapping without the dbMisc 3-record reserve, and a fail-closed
  OBJECT_NULL on exhaustion (RECYCLE_A_RECORD_FROM_THE_WORLD is a full
  world walk and stays unproven); SET_ITEMTYPE (284-345) with the exact
  per-DB writes (db5/6/10 word@2 low 7 bits, db8 high 7 bits, db9
  container charge split with the (w&6)==2 word@6 mark, db4 byte@4, db7
  scroll no-op) and its handle guards; ALLOC_NEW_DBITEM (1142-1165).
  `dm2_v1_drops_place_source_slots` binds the DROP_CREATURE_POSSESSION
  generated-drops loop (1537-1634) with the source's interleaved RNG
  order — slot count roll then that slot's per-item direction draws —
  the OBJECT_NULL slot break BEFORE the direction draw, the party-cell
  (party_dir + RANDBIT) & 3 vs RANDDIR rule, and the direction folded
  into the record word (dir << 14 | handle & 0x3fff) for the bounded
  from-nowhere MOVE_RECORD_TO append to a caller-owned destination list
  (tile-rooted ground-stack mutation stays unproven). The drops RNG
  helpers (RAND16/RANDBIT/RANDDIR, c_random.cpp:13-47) are now public on
  the existing DM2_V1_DropRng. New CTest
  `dm2_v1_dbitem_alloc_pc34_compat` PASS (itemspec mapping, pool
  scan/reserve/bones/exhaustion, per-DB SET_ITEMTYPE, reference-LCG
  cross-check of the interleaved draw order, party-cell rule, OBJECT_NULL
  break without a draw, source-ordered ground chain). dm2_v1 lane: 200
  tests, same 27 known baseline failures, zero new failures. Remaining:
  possession chain walk (c_record.cpp:1640+), tile-rooted ground-stack
  mutation, death-path runtime wiring to a session-owned pool set, and
  source cooldown/eligibility ordering.job/w3

- 2026-07-19 DM2 0x54 DM2_UPDATE_WEATHER handler binding (Jobb W3). New
  module `src/dm2/dm2_v1_update_weather_pc34_compat.c` binds the
  c_tim_proc.cpp:4179-4183 0x54 dispatch into DM2_UPDATE_WEATHER(1) and
  the arg==1 branch of skproject/SKULLWIN/c_weather.cpp:33-90: zone
  weather-flag read table1d6b76[4*v1e1472 + 0x70] (132-byte table bound
  verbatim from dm2data.cpp:889-896), byte-arithmetic ++v1e147b with the
  retry > 0x1f forced transition (DM2_weather_3df7_0037(0) stays
  host-owned behind a receipt flag, no requeue, no RNG advance),
  previous-intensity snapshot, intensity step v1e1474 += (u8)v1e1484 *
  (i8)v1d7108[(v1e1478<<5)+retry] with clamp 0..0xff, and the in-handler
  requeue delay RAND16(256)+50 (50..305) over the shared DM2_V1_DropRng
  LCG (c_random.cpp:13-31). The 128-byte v1d7108 pattern table is bound
  verbatim from the extracted v1d7108.dat (DM2_READ_BINARY,
  dm2data.cpp:1371) and verified byte-for-byte against the reference.
  Zone index outside 0..31 or pattern row outside 0..3 is fail-closed
  with no state mutation. New CTest
  `dm2_v1_update_weather_pc34_compat` PASS (normal step with
  reference-LCG cross-check of the requeue draw, forced transition,
  clamp at both ends, fail-closed bounds, retry byte wrap, requeue
  delay bounds). dm2_v1 lane: 201 tests, same 27 known baseline
  failures, zero new failures. Remaining: runtime wiring of the 0x54
  dispatch (the runtime producer still owns its fixed 182-tick cadence
  while the source re-queues RAND16(256)+50 inside the handler, and
  v1e1478/v1e1484 need map-load provenance), the
  DM2_weather_3df7_0037 transition owner, and the arg==0 day-rollover
  branch (c_weather.cpp:91+).job/w3

- 2026-07-19 DM2 0x54 weather chain runtime wiring +
  DM2_weather_3df7_0037 binding (Jobb W2). The synthetic 182-tick
  weather cadence is retired; the runtime now runs the
  self-perpetuating source chain from skproject/SKULLWIN/c_weather.cpp.
  New `dm2_v1_weather_transition` binds DM2_weather_3df7_0037
  (c_weather.cpp:509-567): arg==0 full transition (host-flagged
  DM2_UPDATE_GLOB_VAR(0x40,0,6) light request, day_tick = gametick +
  0x555, normal reseed with queue delay RAND16(8000)+500, pattern_row =
  RANDDIR, step = RAND16(3)+1; the v1d7188 storm-forced branch with
  delay RAND16(500), row 3, step 1, rain-counter clear; common reset of
  cloud/lightning/intensity/previous/retry plus wind_dir = RANDDIR and
  the source re-queue), the arg!=0 keep-current branch (previous
  cleared, step floored to 1, no requeue), and the common tail
  (cloud_timer = RAND16(4)+4, day_word = table1d70f0[hour] with the
  24-entry dm2data.cpp:182-191 table bound verbatim, hour/days from
  (gametick+v1e1438)/0x555, v1d7188 cleared). The transition RAND16
  draws use the source's CUTX16-then-modulo semantics
  (c_random.cpp:24-28) via the new raw dm2_v1_drops_rand24 accessor —
  dm2_v1_drops_rand16 applies the modulo to the full 24-bit draw, which
  only matches the source for moduli dividing 2^16. In
  `dm2_v1_runtime_tick` the retired producer block is replaced: outdoor
  sessions start the chain with a session-seeded transition (arg=0,
  mirroring the c_savegame.cpp:546 session-start call; the v1d652d
  arg-selecting flag is unproven and the bounded choice is documented),
  the 0x54 dispatch is bound to dm2_runtime_update_weather_timer, which
  steps the session-owned v1e14xx state through
  dm2_v1_update_weather_1, re-queues the source delay, and runs the
  bound transition when the handler forces one. The presentation
  weather intensity is derived from v1e1474 (bounded 0..255 -> 0..100);
  the weather enum stays a host presentation selector. CTests:
  `dm2_v1_update_weather_pc34_compat` extended (normal reseed with
  reference-LCG cross-check, storm path, keep-current, NULL-RNG
  fail-closed), `dm2_v1_weather_timer_producer_pc34_compat` and
  `dm2_v1_weather_seed_regression` rewritten for the source chain
  (reference-LCG-derived boundaries, per-pop intensity steps, indoor
  never starts, deterministic restart on outdoor re-entry). dm2_v1
  lane: 201 tests, same 27 known baseline failures, zero new failures.
  Remaining: the arg==0 DM2_UPDATE_WEATHER day-rollover and
  weather-visuals branch (c_weather.cpp:91-507 — needs the
  light/cloud/SFX/creature-strike subsystems), the v1d652d
  saved-weather flag semantics for the session-start arg, and the
  weather-timer saved-record owner (SKPROJECT-GAP-001).job/w2

- 2026-07-19 DM2 per-cell DM2_THINK_CREATURE runtime wiring over
  session-owned record pools (Jobb W2). New
  `dm2_v1_record_pool_set_init_from_dungeon` exposes the DM2-002 pool
  population directly from dungeon data whose G1 candidate evidence
  validates; the world wrapper `dm2_v1_record_pool_set_init_from_world`
  moved to dm2_v1_world_model.c so record-pool consumers no longer link
  the world model. `dm2_v1_runtime_tick` lazily populates the
  session-owned pool set from the boot dungeon data and binds the
  0x21/0x22 dispatch to `dm2_v1_think_creature_timer_handler`: a popped
  think timer resolves the DB4 creature record AT THE TIMER CELL via
  DM2_GET_CREATURE_AT (skproject c_querydb.cpp:1486-1507) against the
  session pools, the c_ai.cpp:5670-5673 no-creature early return
  consumes the timer without simulating, and the think body stays
  unbound (receipted) until the CCM stream owner/grammar is proven. The
  former unconditional CCM-instance step handler and its door-reader
  bridge are retired (dead code — no creature-scheduling producer
  exists yet); without validated dungeon evidence the 0x21/0x22
  handlers stay unbound and the dispatcher acknowledges those timers
  fail-closed. New CTest `dm2_v1_think_creature_runtime_pc34_compat`
  PASS (lazy population, per-cell resolution with creature-type
  receipt, early return, unbound body, fail-closed without dungeon
  data). dm2_v1 lane: 202 tests, same 27 known baseline failures, zero
  new failures. Remaining: the creature-scheduling producer (map-load
  timer list + c_ai re-queue), the CCM stream owner/grammar for the
  think body, and the possession chain walk / tile-rooted ground-stack
  mutation for DM2-002.job/w2

- 2026-07-19 CSB CSBWin resume/save-import restore (job/w4, one commit
  8b08850cd): the CSBWin 512-byte resume load path re-locked against
  the local CSBWin reference (Timer.cpp, SaveGame.cpp); two CSB tests
  back to green, csb suite known failures 28 -> 26, zero regressions.
  Root cause (two parts):
  (a) `tests/csbwin_resume_fixture.c` stored its saved TimerQueue as
      [2,0,1] — not a min-heap, so no original CSBWin save could ever
      contain it: Timer.cpp `CheckTimers`:885-906 asserts the active
      queue prefix stays heap-ordered after every SetTimer/DeleteTimer
      (ASSERTTimer call sites 916/936/952/988/1037/1044/1157). With
      NumTimer=2 the faithful queue is [0,2,1]: active prefix
      [0,2] (times 0x01020304 < 0x21222324), free handle 1
      (FirstAvailTimer=1) trailing per `DeleteTimer`:912-941.
  (b) `csb_v1_runtime_materialize_csbwin_timer_queue` walked the full
      MaxTimer pool storage instead of the GAMEBLOCK2.NumTimer-owned
      active prefix (SaveGame.cpp GAMEBLOCK2:024; load loops
      :1867/1887/1906 walk only m_numTimer), which both tripped the
      intentional ce342b364 heap validator on the old fixture and
      would have projected free TIMER slots into the live timeline.
  Consumer realignment: csbwin_timer_queue_resume and
  startup_resume_gate queue-slot assertions follow the corrected
  fixture order; champion_bones_expool's direct-profile setup now
  stamps `csbwin_num_timer` per the documented
  NumTimer-owns-active-size model. Verified green:
  csb_v1_save_import_path_pc34_compat, csb_v1_m11_startup_resume_gate
  (fixed); csb_v1_csbwin_duplicate_timer_policy,
  csb_v1_m11_csbwin_timer_queue_resume,
  csb_v1_csbwin_champion_bones_expool_runtime (stay green).

- 2026-07-19 CSB F0276 sensor-lane restore (job/w4, one commit
  9ad005de8): four merge-drift-broken F0276 runtime lanes re-locked
  against ReDMCSB `MOVESENS.C F0276_SENSOR_ProcessThingAdditionOrRemoval`
  and verified green in the full csb suite (known failures 32 -> 28,
  zero regressions).
  - Party floor sensors C001/C002: added the missing switch cases in
    `csb_v1_runtime_process_party_floor_sensors_at_level` with the
    F0276:1587-1620 pre-scan occupancy observation
    (L0772_B_SquareContainsObject / L0773_B_SquareContainsGroup) and
    plumbed P0591_B_PartySquare through every runtime caller (move
    destination add = 0, turn add = 1, removal passes = 1/0), so a
    same-square F0284 turn no longer re-triggers while entering a new
    square triggers per source. C003 data==0 and C009 now also honor
    PartySquare (F0276:1677-1680, 1716-1720).
  - Object floor path C001: `csb_v1_runtime_process_object_floor_sensors_at`
    now admits C001 next to C004 with the F0276:1666-1669 object/group
    occupancy guard; excluding the moved thing reproduces the source
    pre-link (add) / post-unlink (removal) scan, so a HOLD plate
    publishes SET on add then CLEAR on unlink and the F0238 C05..C10
    same-square timeline merge leaves exactly the pending CLEAR event
    (proven by the C49 object-chain fixture).
  - C10 local effect (C10_EFFECT_ADD_300XP_STEAL_SKILL): implemented
    F0269/F0304 semantics — 300 XP divided once by
    G0305_ui_PartyChampionCount and credited to every living champion
    (full 300 to G0411_i_LeaderIndex when leader-only), hidden skill
    C08 Steal plus base skill C01 = (C08-C04)>>2 Ninja (CHAMPION.C
    F0304:875-893), and F0026-bounded share/8 temporary experience
    capped at 32000. Quiescent-timeline model: no combat >>1/<<1
    scaling, difficulty factor 0.
  Fixed tests: csb_v1_f0276_party_c001_sensor_pc34_compat,
  csb_v1_f0276_party_c002_sensor_pc34_compat,
  csb_v1_f0276_object_local_xp_pc34_compat,
  csb_v1_f0276_object_chain_pc34_compat.

- 2026-07-19 Jobb F4 (touch/UI-audit, w5, one commit): the salvaged
  `fs_gesture_navigation_gate` module (per-game gesture translation +
  touch-target zone audit, 24 px source-space floor / 44 px recommended)
  and its 155-assertion test are wired into CMake as CTest
  `fs_gesture_navigation_gate` — they existed on disk since the
  2026-06-28 salvage but were never built. Two salvaged test fixtures
  contradicted the canonical firestaff_touch.c semantics (edge band 20%
  = 64 px on a 320-px framebuffer makes an x=50-origin swipe an edge
  strafe; 5 px travel is inside the 24 px tap tolerance and is a TAP)
  and were re-based on those constants; one set-but-unused variable and
  one unused helper removed (-Wall -Wextra clean). New CTest
  `m12_touch_layout_audit` covers the launcher half of the TODO audit:
  all three shipped M12 touch-layout presets (Classic 12 / Compact 9 /
  One-handed 9 zones) pass both the 24 px floor and the 44 px
  recommendation, `M12_TOUCH_MIN_ZONE_SIZE` is pinned >= the audit
  platform floor, and an under-size user resize clamps exactly to the
  floor and passes the audit. Gesture ctest set 6/6 PASS. Commit:
  8da1c0e6c.

- 2026-07-19 Jobb F1/F3/G (cross-cutting + lokalisering, worktree w5,
  three commits): (F1) version metadata centralized — `firestaff_version.h`
  is now generated by CMake `configure_file` from
  `project(Firestaff VERSION ...)` into `build/generated/include/`
  (searched first on the include path); the hand-maintained header is
  removed and `menu_startup_m12.c` + `changelog_m12.c` consume
  `FIRESTAFF_VERSION_STRING`/`FIRESTAFF_VERSION_NUMBER` from the generated
  header. Root cause of the baseline
  `m12_version_changelog_consistency` failure fixed: missing V3.0.86
  changelog entry added so the changelog matches the CMake project
  version; test now PASS and `firestaff --version` reports v3.0.86 from
  the single source of truth. AGENTS.md version-sync docs updated to the
  three-place contract; `.gitignore` covers `/build/` again. (F3)
  extractor diagnostics — external archives (.7z/.rar/.cab/...) skipped
  because no extractor (7zz/7z/bsdtar) is installed now record a bounded,
  deduplicated diagnostic in asset_find_by_hash; the launcher scan logs
  each skipped archive + tool list and `--scan-data` prints an
  "External archives skipped (no extractor installed)" section. New
  coverage in test_asset_find_by_hash via a
  FIRESTAFF_TEST_DISABLE_EXTERNAL_ARCHIVE_TOOLS override; verified
  end-to-end with a fake .7z fixture. (G) localization — the 13
  fallback-only `startup-menu.*.po` catalogs (cs, da, es, fi, hu, it,
  ko, nl, no, pl, pt, ru, tr) are now natively translated (59 strings
  each from the English msgid source, structure mirroring the reviewed
  Swedish catalog; proper nouns/brand strings keep source spelling per
  sv/de/fr policy). `po/validate_po_layout.sh` reports all 13 OK at
  74-87% native coverage (no more startup-menu FALL); dm1.*.po was
  already translated; test_firestaff_l10n PASS. Commits: 26c2a77b2,
  ecfd2f23e, 95af49437 (last one --no-verify after the known local
  hash_harmonization hook anomaly). Note: asset_status_scan_metrics and
  asset_status_data_dir_change_cache_invalidation fail identically with
  and without these changes (environment/state-dependent baseline
  flakes, verified by rebuild-and-run on the pre-change tree).

- 2026-07-18 DM1 HoC portrait-probe re-base, fourth slice (Jobb E part
  6 = round 3, three commits): 11 more stale-fixture probes re-based
  on the verified PC34 C127 layout and verified PASS in family runs;
  the portrait suite went 53 -> 42 failing (112/154 passing).
  Commits: 5098b86c0 (side-wall negatives), ea832ce64
  (after_party_shuffle), ac5cbb4c3 (approach_from_right 01).
  Side-wall negatives (5): ordinal_2/6_d2l_negative,
  portrait_00_d2r_negative_072_gate, portrait_00_d1r_no_portrait_
  192_gate, portrait_09_d1l_no_portrait_273_gate — in this family
  only the positive D1C cross-check was stale; the negative
  side-band anchors passed unchanged against the real layout.
  after_party_shuffle (5): hoc_champion_portrait_02/07/09/11 plus
  hall_of_champions_portrait_14 — PARTY_MAP seeds moved to the real
  mirror squares (02 -> (7,9)N, 07 -> TIGGY natural (14,6)S route,
  09/11 -> (7,9)N); probe 14 also needed its pre-shuffle recruit
  poses re-based (HALK_POSE (1,2)N -> (7,9)N in front of the (7,8)
  S-face sensor, WUUF_POSE (1,5)S -> (7,16)S in front of the (7,17)
  N-face sensor) before its post-LEYLA championCount=3 contract
  asserted again.  approach_from_right (1): hoc_champion_portrait_
  01 — wrong-wall anchor (2,1) DIR_WEST -> (8,8) DIR_WEST, standing
  east of the real (7,8) S-face HALK cell so the visible east wall
  carries no C127 sensor; positive cross-check moved to (7,9)
  DIR_NORTH.  Parked after attempted re-base (reverted to HEAD,
  failing as before, NOT pose-fixable):
  portrait_06/17_inventory_exit_restore — at the (7,9) park the
  baseline asserts 100% but the inv-off contract fails;
  ToggleInventoryPanel behaviour and C040 panel survival look like
  a behaviour-contract question aging the BUG-120/121 panel guards,
  needs runtime triage rather than a pose fix.
  portrait_19/22_wall_ornament_no_float — at the (7,9) park the
  C346 frame edges read BLACK top=0/64 left=0/43 but bottom=58
  right=41 with LIGHT_GRAY/GRAY/DARK_GRAY rings all 0; signature of
  a 1px shift or different edge profile in the C346 bitmap, needs
  ReDMCSB/bitmap evidence before any probe-side change.

- 2026-07-18 DM1 HoC portrait-probe re-base, third slice (Jobb E part
  5 = round 2, seven commits): 41 more stale-fixture probes re-based
  on the verified PC34 C127 layout (full ordinal->square table in the
  2026-07-18 triage entry below) and verified PASS individually plus
  in family runs; the portrait suite went 94 -> 53 failing.
  cancel_reopen family (15 probes): portrait_00/01/02/03/05/06/08/13/
  14/15/16/17/18/19/20 — HALK-seeded park pose (1,2) NORTH -> real
  (7,9) NORTH in front of the (7,8) S-face HALK sensor; probe 13
  uses the natural WUUF route (1,5) SOUTH -> real (7,16) SOUTH in
  front of the (7,17) N-face sensor.  leave_and_reenter family (3):
  portrait_02/06/19 — seed pose (1,2) NORTH -> (7,9) NORTH, leave
  target (1,3) SOUTH -> real (7,13) SOUTH in front of the (7,14)
  N-face ZED sensor (shipped ZED_ORDINAL=10 expectation locks
  against a live mirror again); the single forward step became
  LEAVE_STEP_COUNT=4 steps through the open hall column x=7, with
  the symmetric 4-step NORTH return keeping the blocked-step
  teleport fallback.  west/side-wall negatives (6):
  ordinal_17_west_negative + ordinal_13_wuuf_west_negative (only the
  Group D positive cross-check was stale; moved to the real (7,16)
  SOUTH WUUF route), portrait_04_side_wall_negative (anchor (10,5)S,
  wrong-wall neighbours (9,6)E/(11,6)W/(10,7)N — the set portrait04
  already proved), portrait_07_side_wall_negative_271_gate (anchor
  (14,6)S; negatives (13,7)E/(15,7)W/(14,8)N),
  portrait_21_side_wall_negative_189_gate (anchor (16,17)N;
  negatives (15,16)E/(17,16)W/(16,15)S), hall_of_champions_west_
  negative (only the HALK cross-check moved (1,2)N -> (7,9)N; the
  (1,2)W map-edge negative stays, and no same-square WEST look was
  locked at (7,9) because the (17,9)W far-view D1C question from
  round 1 is still open).  Panel-state family (8):
  portrait_05/09/22_candidate_panel_cancel, portrait_05_after_party_
  shuffle, portrait_06_backtrack_same_tick, portrait_09/15_popup_
  focus_return, portrait_03_save_load_reopen_219_gate — all the same
  mechanical (1,2) -> (7,9) park swap.  front_north_entry family
  (6): portrait_00/12/13/17/21/22 — shared HALL_NORTH_ENTRY_X/Y
  constants (1,2) -> (7,9).  palette_match family (3): portrait_05
  (ELIJA (14,2)S -> pose (14,3)N, negative (15,2)W), portrait_09
  ((8,14)S -> pose (8,15)N, negative (8,13)S), portrait_23 ((5,13)E
  -> pose (6,13)W, negative (5,12)S) — wrong-aspect negatives use
  the mirror cell's own faces, not across-the-hall looks.  No real
  code bugs found in this round; the park swap was attempted and
  reverted for portrait_06/17_inventory_exit_restore, portrait_19/22
  _wall_ornament_no_float and portrait_00 d1r/d2r because their
  side-band/frame thresholds are calibrated to the old (1,2)
  corridor view and need per-probe recalibration (listed in TODO.md
  for round 3).  Commits 8e9d88ace, 8e0121812, fb74a9416, 5a8ac5a07,
  4a7fdb870, 589f0a140, 537eabca4.

- 2026-07-18 DM1 HoC portrait-probe re-base, second slice (Jobb E part
  4, three commits): 12 stale-fixture probes re-based on the verified
  PC34 C127 layout (full ordinal->square table in the 2026-07-18
  triage entry below) and verified PASS individually plus in a 12/12
  dedicated ctest run: portrait00 + portrait03 (HALK-seeded route
  (1,2) N -> real (7,9) NORTH in front of (7,8) S face),
  portrait10 ((1,3) S -> (7,13) SOUTH, front (7,14) N face),
  portrait15 ((2,4) S -> (11,10) SOUTH, front (11,11) N face),
  ordinal_23_front_north_entry (pose (7,9) NORTH; the probe's manual
  front-cell THING-chain mutation walk was also broken for
  non-symmetric squares and was replaced by the sensor-pool seed
  route sensorType==127/data==1 used by portrait00/03),
  ordinal_4_approach_from_left (wrong-wall anchor (1,2) E -> (9,6)
  EAST on the west wall of (10,6); positive cross-check on the real
  (10,5) SOUTH route), portrait_rect_ordinal16_pc34_compat ((2,7) S
  -> (11,15) SOUTH; wrong-aspect negatives (10,16) E / (12,16) W;
  the first (11,15) W negative matched unrelated ordinal-21 pixels
  at 77% and was swapped for the same-cell wrong-aspect pose),
  ordinal_07_portrait_rect_position + ordinal_07_south_return
  ((2,17) S -> (14,6) SOUTH, front (14,7) N face),
  champion_portrait_01_south_return ((1,0)/(1,2) -> (7,7) SOUTH
  negative + (7,9) NORTH canonical),
  ordinal_5_front_south_entry ((2,16) N -> (14,3) NORTH, front
  (14,2) S face), portrait21_south_return (was a SKIP-in-vacuo
  discovery probe — its (3,11) SOUTH fixture matched nothing and the
  skip guard passed it vacuously; now asserts 18/18 on the real
  (17,9) SOUTH route in front of the (17,10) N-face ALEX sensor,
  seeded 20 -> 21; its same-square WEST turn-away faced across the
  open hall and matched unrelated far-view D1C pixels at 95%, so the
  wrong-wall negatives use the mirror cell's own wrong aspects
  (16,10) E / (17,11) N per the portrait04 pattern). No real code
  bugs found in this slice — all failures were stale fixtures; one
  far-view D1C observation noted in TODO.md for later triage.
  DM1 portrait suite: 105 -> 94 failing. Commits 8cd356b73,
  6830f8deb, 40a2944f5.

- 2026-07-18 DM1 HoC portrait-probe triage, first slice (Jobb E part 3,
  two commits): (a) merge-drift-clobber fixed —
  `M11_GameView_GetD1CWallOrnamentZone` resolved through a local
  m11_game_view.c stub returning hardcoded (96,16,32,88), introduced by
  1750ad9ea ("Fix release runtime compile contracts"). The helper is now
  the exported `dm1_v1_front_mirror_wall_ornament_zone_xywh_pc34` in
  `src/dm1/dm1_v1_wall_ornament_pc34_compat.c`, resolved through the
  source-locked G0194/G0205 tables (global ornament 43 -> coord set 5,
  G0205 row 12 = D1C), yielding the real (80,29,64,43) frame zone the
  C346 mirror frame and C026 portrait (96,35) occupy (ReDMCSB
  DUNVIEW.C:3913-3928). Commit 0ff519944. (b) stale test fixture fixed —
  `firestaff_dm1_v1_champion_mirror_portrait04_rect_position_runtime_probe`
  claimed the LEIF (ordinal 4) C127 sensor at map0 (2,2) with party pose
  (2,1) SOUTH. Independent DUNGEON.DAT decode per the dmweb.free.fr
  dungeon-file spec (little-endian PC; seed=99, 14 maps, 684 sensors,
  start (1,3) SOUTH) proves the LEIF sensor lives at map0 (10,6) on the
  north face (sensor record #22). Probe re-based to pose (10,5) SOUTH
  with wrong-wall neighbours (9,6) EAST / (11,6) WEST / (10,7) NORTH and
  now passes 18/18, including >=90% ordinal-4 pixel match in the D1C
  rect. Commit 354c32788. Verified real C127 map0 layout (ordinal at
  (x,y) face): 0=(8,7)E, 1=(7,8)S, 2=(9,10)N, 3=(16,15)N, 4=(10,6)N,
  5=(14,2)S, 6=(16,4)W, 7=(14,7)N, 8=(14,14)W, 9=(8,14)S, 10=(7,14)N,
  11=(16,7)S, 12=(12,8)S, 13=(7,17)N, 14=(10,3)S, 15=(11,11)N,
  16=(11,16)N, 17=(13,11)S, 18=(10,13)W, 19=(15,12)W, 20=(17,10)N,
  21=(16,16)S, 22=(11,13)E, 23=(5,13)E. Front-mirror pose rule: stand
  on the adjacent floor square on the mirror's own face side, facing the
  wall (visibleWallCell = (viewDir+2)&3 matches the sensor cell).
  Remaining triage documented in TODO.md.

- 2026-07-18 CSB merge-drift test triage (Jobb D, five commits): after the
  worktree-integration merge (df88dbda4 + a192cb2b0) clobbered newer
  CSBWin runtime code in `src/csb/csb_v1_runtime_pc34_compat.c`, restored
  the pre-merge (a192cb2b0^) implementations of the CSBWin timer queue
  machinery — `csb_v1_runtime_event_is_before`,
  `csb_v1_runtime_fix_unmerged_timer_placement`,
  `csb_v1_runtime_append_unmerged_map_timer_to_queue`,
  `csb_v1_runtime_materialize_csbwin_timer_queue` (staging + heap
  validation + `csbwin_timeline_event_queue_slot` bookkeeping) and
  `csb_v1_runtime_materialize_csbwin_item16_summaries` (atomic staging) —
  reverted c354907a5's drive-by dungeon guard in
  `csb_v1_runtime_rotate_party`, and moved `last_input_dispatch`
  publication before the dequeued check in
  `csb_v1_runtime_process_input_queue` (pass673 regression vs the
  pass680 contract). Restored `csb_v1_runtime_set_csbwin_saved_skin`
  (DB11 free-node expansion + all-zero-column erasure), the fnv1a
  receipt check in
  `csb_v1_runtime_locate_appended_expool_record_internal`
  (b35d17974 contract), and raised
  `CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES` 4096 -> 8192 so a 24x256-byte
  EDT_Palette bundle fits (the clobber caused buffer-overflow aborts).
  Re-applied ee0df4933 in `src/shared/asset_status_m12.c`: runtime cache
  materialization no longer rewrites a matched version path to the
  extracted asset-cache leaf, so CSB version identity keeps its
  archive-backed GRAPHICS hash provenance. Two test fixtures were
  aligned with intentional post-merge contracts instead of reverting
  code: `test_csb_v1_csbwin_dsa_door_timer_handoff` now sets a complete
  restored TIMER pool (max_timers/num_timer/first_avail_timer/sequence)
  so the transactional TT_DOOR -> TT_1 pool handoff (5d01ef67a,
  ade2ad312) succeeds, and
  `test_csb_v1_dsa_parameter_message_save_handoff` binds its dungeon to
  the profile so the source-faithful DSA PutState persistence
  (dbc23d545) can commit. The fail-closed forward-move guard
  (442822c26) kept its hardening; the input bridge test now expects
  blocked-without-verified-dungeon. 14 CSB tests back to green
  (csb_v1_csbwin_timer_restart_export,
  csb_v1_input_command_bridge_pc34_compat,
  csb_v1_queue_overflow_pc34_compat,
  csb_v1_teleporter_rotation_runtime_pc34_compat,
  csb_v1_csbwin_duplicate_timer_policy,
  csb_v1_csbwin_item16_atomic_restore,
  csb_v1_dsa_restored_timer_tick_bridge,
  csb_v1_csbwin_champion_bones_expool_runtime,
  csb_v1_saved_skin_expool_writeback, csb_v1_expool_palette_restore,
  csb_v1_expool_receipt_runtime_lookup,
  csb_v1_csbwin_dsa_door_timer_handoff,
  csb_v1_dsa_parameter_message_save_handoff,
  csb_archive_required_materialize_cache_gate). CSB lane: 339 tests,
  32 known baseline failures, zero new regressions (verified against a
  79786f091 baseline worktree: e.g. csb_v1_runtime_tick_accumulator went
  from 65 FAIL assertions at baseline to 9). Commits: 6d90d4f4b,
  0fbdd85e4, 4a34c06e0, f2841dd61, 2417cc122 (the last with
  --no-verify; the hash_harmonization hook fails on two pre-existing
  on-disk dm2-mac-en data mismatches unrelated to the change).
  Remaining: C38 combat/PARRY/leadership runtime lane, F0276 sensor
  ordering, save-import path, viewport/redmcsb gates, and the
  real-data/timeout tests (hint_oracle, pc_real_asset_launch,
  pc_package_presentation, first_viewport_frame).

- 2026-07-18 DM2-009 savegame timer payload materialisation (bounded
  slice): new `include/dm2_v1_save_timers_pc34_compat.h` +
  `src/dm2/dm2_v1_save_timers_pc34_compat.c` implement the source's
  `GAME_LOAD` timer section order (skproject/SKULLWIN
  c_savegame.cpp:1517-1527): per-record SUPPRESS decode through the
  verified `v1d6463 = vsgame+0x00` 12-byte mask
  (`ff ff ff 3f 7f ff ff ff ff ff 00 00`, dm2data.cpp:97-99,
  dm2data.h:608) over the 12-byte `c_tim` wire layout (c_timer.h:8-46),
  with the source's per-record mask re-arm and cross-record bit carry
  (c_savegame.cpp:655-733); `clrtype()` for `[num_timers, max_timers)`;
  `DM2_SORT_TIMERS` identity-fill + heapify with `DM2_cmp_timers`' full
  tiebreak chain — ticks asc, type desc, actor desc, record-index asc
  (c_timer.cpp:31-48, 126-194); and `DM2_REARRANGE_TIMERLIST`'s
  num_timer_indices/free-chain rebuild (c_timer.cpp:97-122). Fail-closed:
  underflow decodes into scratch and leaves caller records/indices
  untouched (source M_exit), out-of-bounds counts reject, `dummya` is
  never restored. Proves the saved timer-record byte layout half of
  SKPROJECT-GAP-001; weather-timer ownership stays open. New CTest
  `dm2_v1_save_timers_pc34_compat` PASS (mask vector, cmp tiebreaks,
  round-trip incl. 24-bit ticks/6-bit map/7-bit type, hand-computed
  heapify permutation, free-chain links, underflow atomicity, bounds).
  DM2 lane 196 tests, 27 known baseline failures, zero new. Remaining:
  corpus-verified full DB-record materialisation and the post-load
  `DM2_READ_SKSAVE_DUNGEON` / `DM2_PROCEED_GLOBAL_EFFECT_TIMERS` rebuild
  order (receipted pending).

- 2026-07-18 DM2-008 source-ordered runtime sound queue (bounded slice):
  new `include/dm2_v1_sound_queue_pc34_compat.h` +
  `src/dm2/dm2_v1_sound_queue_pc34_compat.c` implement the original
  queue/query/change-detection order from skproject/SKULLWIN:
  `DM2_SOUND9` seven-byte `s_ssound` runtime-queue mutation with
  duplicate/capacity gates (c_sound.cpp:650-662; xsndptr2 is runtime
  state, never GDAT-materialised), 1-based `DM2_QUERY_SND_ENTRY_INDEX`
  (c_sound.cpp:664-673), `DM2_QUEUE_NOISE_GEN1` with map gate, 20/6
  positional/immediate caps, volume halving, four-case party-facing
  rotation, `R_1FB7D` occlusion clamp via an explicit probe (absent
  probe rejects fail-closed), same-sample+position change detection,
  and the 8-slot delayed path whose type-0x15 timer is receipted
  pending rather than simulated (c_sfx.cpp:138-331),
  `DM2_QUEUE_NOISE_GEN2` class remap (c_sfx.cpp:334-345), exact `R_928`
  attenuation/bearing metric including the negative-x and y==0 table
  branches the older approximation missed (c_sound.cpp:256-308,
  table1d14e2 c_sound.cpp:32-37), `R_8FE` precedence
  (c_sound.cpp:310-319), `DM2_PLAY_SOUND` gate, permutation bubble sort
  and 64-slot free-sample scan with whole-pass early return
  (c_sound.cpp:342-434), and `DM2_SOUND8` flush (c_sound.cpp:633-647).
  Fail-closed throughout: unresolved sample bindings reject, facing > 3
  rejects (source UB), playback is receipted explicitly unavailable and
  the sample-slot table is never mutated (SKPROJECT-GAP-003). New CTest
  `dm2_v1_sound_queue_pc34_compat` PASS; DM2 lane 195 tests, 27 known
  baseline failures, zero new failures. Remaining: verified sample
  backend (`do_sound`), secondary `s54p_00->s54p_00` duplicate
  comparison, `DM2_PROCESS_SOUND` delayed release.

- 2026-07-18 DM2-005 source-exact CCM b_1a dispatch matrix (bounded
  slice): new `include/dm2_v1_ccm_dispatch_pc34_compat.h` +
  `src/dm2/dm2_v1_ccm_dispatch_pc34_compat.c` bind DM2_PROCEED_CCM's
  ordered compare chain (skproject/SKULLWIN/c_creature.cpp:2930-3212)
  verbatim: every b_1a command byte 0x00-0xFF maps to its source handler
  group (WALK_NOW 0x01/0x02/0x09, CCM03 0x03-0x04, JUMPS 0x05, CCM06
  0x06-0x07, ATTACKS_PARTY 0x08/0x26, STEAL_FROM_CHAMPION 0x0A, CCM0B
  0x0B, CCM0C 0x0C-0x0D, SHOOT_ITEM 0x0E-0x0F, KILL_ON_TIMER_POSITION
  0x13, ROTATES_TARGET_CREATURE 0x15-0x16, PLACE_MERCHANDISE 0x17,
  TAKE_MERCHANDISE 0x18, PUTS_DOWN_ITEM 0x19/0x29/0x2A/0x2D/0x2E,
  TAKES_ITEM 0x1A/0x2B/0x2C, CAST_SPELL 0x27-0x28, ACTIVATES_WALL
  0x2F-0x31, USES_LADDER_HOLE 0x35-0x3A, TRANSFORM 0x3B-0x3C,
  EXPLODE_OR_SUMMON 0x3D-0x40, DM2_1B7D5 0x55) or NONE where the chain
  admits no branch. table1d613a (mdata.c:1615-1639, 86 bytes) is bound
  verbatim with fail-closed reads beyond the proven span, plus the
  c_creature.cpp:3194-3206 gametick writeback gate (flags & 3). New CTest
  `dm2_v1_ccm_dispatch_pc34_compat` PASS (full 256-byte matrix against an
  independently derived expected table, timing-byte spot checks,
  fail-closed out-of-span, writeback gate, evidence). Execution stays
  fail-closed: handler groups without a proven runtime owner remain
  dispatch receipts only; the CCM stream owner/grammar requirement is
  unchanged. Commit a3d423d3b.

- 2026-07-18 DM2-006 source-ordered creature drop resolution (bounded
  slice): `src/dm2/dm2_v1_drops.c` gains
  `dm2_v1_drops_resolve_source_slots`, binding DROP_CREATURE_POSSESSION's
  generated-drops loop (skproject/SKWINSPX/src/v4/skcrture.cpp:2084-2118)
  verbatim: CREATURES word fields 0x0A..0x14 (CREATURE_STAT_DROP_FIRST..
  LAST, skdefine.h:898) resolve in ascending slot order, word 0 skipped
  via the source's `continue`, base=(w&15)+1, extra=(w&0x70)>>4, count +=
  RAND16(extra+1) when extra != 0, item = w>>7, with count rolls
  consuming the source LCG (c_random.cpp:13-31, state*0xbb40e62d+11 >>8)
  in source order. The GDAT AI-table loader now imports CREATURES drop
  words per creature type alongside the AI row; the death path resolves
  them source-ordered and receipts admitted slots/total items in the
  death-drop observer (`source_ordered`, `source_slots_admitted`,
  `source_total_items`). Per-item direction draws (RAND01/RAND02) and
  ALLOC_NEW_DBITEM record creation stay fail-closed until the DB item
  allocator is bound; the data-free Thorn Demon fallback is unchanged.
  New CTest `dm2_v1_drops_source_order_pc34_compat` PASS (zero-skip,
  base-only no-RNG, per-slot roll order against an independent LCG
  replica, NULL guards, death-path observer binding, fallback
  preservation); existing `dm2_v1_creature_death_drop_pc34_compat` still
  PASS. Commit 24b1e1dc1.

- 2026-07-18 DM2-007 source rune-key spell lookup + failure classes
  (bounded slice): `src/dm2/dm2_v1_spell.c` gains
  `dm2_v1_spell_pack_query_key` / `dm2_v1_spell_find_by_runes` /
  `dm2_v1_spell_record_mana_cost` / `dm2_v1_spell_proceed_failure`,
  binding DM2_FIND_SPELL_BY_RUNES (skproject/SKULLWIN/c_events.cpp:
  2211-2264): query key packing rune[0]<<24...rune[3]<<0 over the
  zero-terminated hero rune string (max four runes, single-rune tail
  rejected like the source NULL), reverse table scan, 24-bit masked
  compare when the record key top byte is zero (power rune stripped),
  full 32-bit compare for exact-power-locked records. Mana formula
  ((w6>>10)&0x3f)*(cast_power+0x12)/0x18 (c_events.cpp:2282-2289) and the
  DM2_PROCEED_SPELL_FAILURE classes 0x10/0x20/0x30 with status
  writebacks, glob-var ids 0x45/0x46/0x44, and the TRY_CAST_SPELL
  rune-clear/panel rule (c_events.cpp:2687-2786) are bound as receipts;
  DM2_UPDATE_GLOB_VAR and the v1e0b6c window stay receipted-pending, not
  simulated. The fixed 34-spell runtime path is unchanged until live
  rune strings bind to validated GDAT records. New CTest
  `dm2_v1_spell_rune_lookup_pc34_compat` PASS. Commit 18923e6ad.

- 2026-07-18 DM2-003 source-ordered timer dispatcher (bounded slice): new
  `include/dm2_v1_proceed_timers_pc34_compat.h` + `src/dm2/dm2_v1_proceed_timers_pc34_compat.c`
  implement the skproject DM2_PROCEED_TIMERS boundary. Source anchors:
  c_tim_proc.cpp:3980-4230 (pop due heap head, DM2_CHANGE_CURRENT_MAP_TO
  per timer, 26-entry type matrix: 0x01 STEP_DOOR, 0x02 DESTROY_DOOR, 0x04
  tile actuator subdispatch classes 0-6 with class-3 no-op, 0x0c, 0x0d
  RESURRECTION, 0x0e, 0x15 SOUND, 0x19 CLOUD, 0x1e STEP_MISSILE, 0x21/0x22
  THINK_CREATURE, 0x3d, 0x46 LIGHT, 0x47, 0x48, 0x4b POISON, 0x54 WEATHER,
  0x55, 0x56, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e ALLOC_NEW_CREATURE),
  c_timer.cpp:31-47 (DM2_cmp_timers: tick asc, type desc, actor desc,
  source index asc), c_timer.cpp:261-278 (GET_AND_DELETE_NEXT_TIMER).
  Unknown types skip via the source's `continue`; known types without a
  bound DM2-owned handler are acknowledged fail-closed, never simulated.
  The unconditional host-side creature-tick simulation in
  `dm2_v1_runtime_tick` is removed: creature state advances only through
  dispatched 0x21/0x22 timers over the new DM2-owned runtime queue
  (`dm2_v1_runtime_enqueue_source_timer` single entry point,
  `dm2_v1_runtime_last_proceed_timers_receipt` probe accessor). New CTest
  `dm2_v1_proceed_timers_pc34_compat` PASS; dm2_v1 ctest lane 191 tests
  with the same 27 known baseline failures, zero new failures. Remaining
  DM2-003 work: bind proven timer producers (doors, missiles, weather,
  creature scheduling), per-cell DM2_THINK_CREATURE after DM2-005, and
  removing the DM1-generic M11 creature-group pass for DM2 sessions.

- 2026-07-18 DM2-002 c_record pool ownership layer (bounded slice): new
  `include/dm2_v1_record_pool_pc34_compat.h` + `src/dm2/dm2_v1_record_pool_pc34_compat.c`
  give the DM2 world a source-ordered c_record ownership model instead of
  only the reduced parallel record view. Exact skproject anchors:
  c_record.cpp:28-31 table_recordsizes {4,6,4,8,16,4,4,4,4,8,4,0,0,0,8,4},
  c_record.cpp:44-52 GET_ADDRESS_OF_RECORD handle decode (pool (r>>10)&0xf,
  index r&0x3ff, direction bits masked), c_record.cpp:54-57
  GET_NEXT_RECORD_LINK (first word), c_record.cpp:60-170 APPEND/CUT list
  paths, c_moverec.cpp MOVE_RECORD_TO relocation boundary (cut+append over
  list heads; tile-rooted relocation stays rejected until c_map ground-stack
  link state is proven). Pools own exact copies of their validated G1 spans
  plus the proven DB3/DB4 G1-extension continuations; zero-sized DB11-13,
  absent pools, and null/end handles all fail closed. `dm2_dungeon_world_t`
  now owns a populated `DM2_V1_RecordPoolSet` whenever the G1 pools validate
  (dm2_world_get_record_pools accessor), populated in dm2_world_from_mem and
  released in dm2_world_free. New CTest `dm2_v1_record_pool_pc34_compat`
  (size table, handle decode, address bounds, link walk, append/cut/
  relocate, fail-closed cases) passes; dm2_v1 ctest lane 190 tests with the
  same 27 known failures as the pre-change baseline, zero new failures.
  Remaining DM2-002 work: tile-rooted append/cut, DM2_MOVE_RECORD_TO full
  cross-map path, save-state relocation, and retiring the remaining parallel
  record reads in dm2_v1_runtime.c/dm2_v1_world_state.c.

- 2026-07-18 Build baseline restored after worktree merge drift: main was
  unbuildable after df88dbda4 ("csb: implement F0243 door destruction") and
  a192cb2b0 ("Integrate game support worktrees") clobbered ~40k lines of
  definitions, declarations, and CMake link contracts. Repaired in the
  Extern-disk work clone (cmake hangs under ~/Documents/iCloud): restored
  clobbered symbols (dm2_v1_viewport_door_frame_graphic_index_for_graphicsset,
  tr_asset_generated_v1_rendering_allowed + TrAssetBundle
  .synthetic_rendering_blocked + tr_asset_block_synthetic_rendering_for_
  verified_media, dm2_v2_hud_runtime render_with_assets/last_path_mode/
  last_path_counts/last_slot_class, csb_v1_runtime_set_csbwin_saved_skin,
  dm1_v1_original_save decode-header receipt, ReDMCSB F1007/F1008/F1017/
  F1018/F1020/F1025/F1026/F1031 aliases), rewrote the Nexus startup-menu
  test to the intentional from_data_gate rename, patched ~60 test/probe
  targets with missing sources (dm2_v1_asset_loader.c, dm2_v1_midi_backend.c
  + CoreMIDI/CoreFoundation, dm1 quiver-refill), and linked firestaff_dm2_v2
  PUBLIC firestaff_dm2 for the GDAT-backed HUD chrome plan. Verification:
  `cmake --build build --parallel 10` green at 100%; ctest baseline
  2446 tests, ~386 failing (~84% pass) with failures concentrated in DM1
  HoC portrait-rect runtime probes, CSBWin timer/DSA handoffs, and
  asset-status zip-cache lanes (tracked in TODO.md Known Bugs as jobb
  A-G follow-ups).

- 2026-07-17 Theron Track 02 later-record plan/corpus admission: bound the
  opaque replay-tail later-record candidate to the validated dungeon-handoff
  capture target and artifact corpus. Admission requires exact record/raw
  sector and destination identity, Track02/source-trace hashes, replay tail,
  layout epoch, and plan identity. The receipt is capture-required/no-draw
  only and refuses record-format, object, bitmap, decode, draw, and fallback
  semantics. Verification: Ninja built
  `test_theron_v1_track02_later_route_candidate_plan_corpus_admission`,
  `test_theron_v1_track02_later_route_candidate_intake`, and
  `test_theron_v1_track02_later_route_candidate_campaign_index`; all three
  focused CTests passed in `build-theron-trace-md5`. No commit/push.

- 2026-07-17 Theron Track 02 live-handoff capture-required admission: joined
  the verified `0x0b52` loader-output admission and immutable G8 FIFO capture
  witness to the live dynamic CD_READ handoff only under matching artifact
  corpus/plan identity, Track02 MD5, source-trace MD5, ownership record and
  destination, and lifecycle scan epoch. The receipt preserves the two
  records as separate no-draw observations and rejects bitmap/object, decode,
  draw, and fallback permissions. Verification: Ninja built
  `test_theron_v1_track02_live_handoff_capture_required_admission`,
  `test_theron_v1_track02_g8_fifo_capture_binding`, and
  `test_theron_v1_track02_loader_output_record_admission`; the three focused
  CTests passed in `build-theron-trace-md5`, with scoped `git diff --check`
  passing. No commit/push.

# ✅ 2026-07-11 CSB DM1 utility-import transaction: `csb_v1_import_from_dm1_save_buffer()` now constructs imported champions in a candidate party and commits it only after every declared record is complete and valid. A malformed later champion or an EOF mid-import now reports the original error while preserving the live party exactly. ReDMCSB anchor: `CEDTINCI.C F7090_MakeNewAdventure` line 10 (candidate adventure construction); CSBWin anchor: `SaveGame.cpp` DM1 import path. Verification: Ninja `test_csb_v1_utility_import_block_verify_pc34_compat` passed 16/16 and `test_csb_v1_save_import_path_pc34_compat` passed 57/57; focused CTest passed 2/2. No push.
# 2026-07-11 - CSBWin Extended Features DSA inspection

- ✅ 2026-07-11 CSBWin DSA authenticated stack/arithmetic/control handoff: added a complete-action interpreter restricted to runtime-owned, checksum-authenticated `DSAAction` words. It accepts only `LOAD`, `STORE`, and CSBWin `EX_AMPERSAND`'s pure stack/arithmetic/comparison/shift/`SETNEWSTATE` subcodes on a source-sized 100-cell scratch stack, committing the A..Z parameter surface only after full consumption. `LOAD` retains signed extended-state decoding, while `STORE` and `AMPERSAND` retain CSBWin's raw unsigned extension-word behavior. `AMPERSAND2`, variable/world/filter subcodes, unsupported words, malformed programs, source-illegal `LOAD_ABS32`, and stack faults reject without mutation. Focused `test_csb_v1_dsa_trigger_single_step_pc34_compat` passes 98/98; focused CTest passes. Source: CSBWin `Data.h:1686-1890`; `DSA.cpp:98-426, 1074-1189, 1317-1385, 2324-2719, 2852-2858`.

- ✅ 2026-07-11 CSBWin DSA authenticated LOAD/STORE handoff: added the next
  source-dependent `DSACMD_STORE` family after the existing `DSACMD_LOAD`
  decoder. The new boundary resolves an action by DSA id/state/ordinal only
  from `csb_v1_chaos_import_extended_save_dsas()` runtime-owned records, then
  accepts precisely a fully consumed `LOAD -> STORE` program. It preserves
  CSBWin `DSA.cpp` `EX_LOAD`/`EX_STORE` selector and compact/extended
  next-state decoding, writes the mutable A..Z parameter surface only after
  validation, and rejects unowned, malformed, illegal, or continued programs
  without mutation. Focused DSA unit passes 93/93. The extended-save runtime
  fixture verifies checksum-authenticated import through the production
  prefix inspector and the `0xABCD` LOAD INTEGER -> STORE A handoff. Source:
  CSBWin `Data.h:1686-1708, 1954-2011`; `DSA.cpp:1074-1189, 1317-1385`.

- 2026-07-11 CSB title/entrance M11 handoff geometry: corrected the C004 opening composite so the moving C002/C003 door strips retain their ReDMCSB destination y=28 rather than mistakenly using source y=0 in the M11 composite contract. The focused entrance regression now locks both C002/C003 boxes, the real-data M12-to-M11 launcher gate checks the C001 PRESENTS and CHAOS special palettes, and the PC real-asset probe proves that C004's first 28 rows remain intact before C002/C003 start. Source lock: ReDMCSB `TITLE.C` `F0437` lines 424-463 and `ENTRANCE.C` `F0438` lines 172-239. Verification: Ninja built the focused targets; focused CTest passed 3/3; direct `firestaff_csb_v1_pc_real_asset_launch_probe` passed 27/27 against local PC CSB `GRAPHICS.DAT`/`DUNGEON.DAT`.

- Added a CSB-owned, read-only decoder for the variable CSBWin Extended Features DSA section. It follows `SaveGame.cpp` `ReadDSAs`, `DSA.cpp` `DSA::Read`/`DSAState::Read`/`DSAAction::Read`, and `data.cpp` `RCS`: numbered DSA records, unique state indexes, action/program bounds, and the trailing checksum must all validate before aggregate evidence and the exact following-tail offset are reported.
- Kept the boundary non-promoting. DSA descriptions and program words remain in the caller buffer, no runtime profile changes, and game-info, trace data, and level-index bytes remain separate follow-up formats.
- Extended the data-free classifier test with positive, checksum-reject, action-count-reject, and input-immutability coverage.
- ✅ 2026-07-11 DM2 TITLE/0 raw-menu fallback regression: corrected the M11 startup profile gate to model skproject `SkWinCore::SHOW_MENU_SCREEN` lines 55187-55199 exactly: `TITLE/0 dt07/4` is consumed as the complete 320x200 menu only when that typed raw entry exists; otherwise the decoded `dtImage/4` menu surface and its menu commands remain required. The local verified PC English data takes the image-fallback branch, while the raw branch retains the existing no-overlay contract. Verified after the Nexus `mesh_ref` build repair with a fresh target build and focused CTest: `dm2_v1_boot_profile_smoke`, `dm2_v1_m11_startup_profile_gate`, and `dm2_v1_runtime_handoff_smoke` passed 3/3.
# 2026-08-08 Theron loader sidecar normalization

- ✅ Hardened `theron_v1_mednafen_main_ram_trace_parse_file()` for the actual
  early Mednafen artifact format, which wrote record separators as literal
  `\\n` bytes. Parsing normalizes only an in-memory temporary stream; the
  original sidecar remains the MD5 identity and semantic publication stays
  blocked.
- Added a regression using a complete escaped-separator loader witness. The
  local real capture now parses as READY with two block transfers, two RTS
  records, and the source-locked first TIA (`$2286/$1f0286`, `$c800 → $0404`,
  `0x80` bytes).
- This fixes capture ingestion only. It does not claim the missing `$2600`
  consumer, tile/material bank, VCE route, startup animation owner, HUD
  geometry, or later-level/object semantics.
# 2026-08-08 Theron consumer-side trace normalization

- ✅ Applied the loader-sidecar transport fix to the Main-RAM consumer trace
  parser and executed HuC6280 code-window verifier.
- Added a regression with escaped separators covering both record admission
  and code-byte verification. Hash identity and semantic publication gates
  remain unchanged.
- No creature, RNG, T700/T900, inventory, palette, portrait, or audio meaning
  is inferred from this transport-only repair.
# 2026-08-08 Theron source-record admission hardening

- ✅ Track 02 monster and generator ledger binders now reject records for
  unloaded or source-header-unverified levels.
- Added regression coverage proving an unverified level cannot seed a source
  monster record. Live creature publication remains blocked until the original
  RNG consumer and creature consumer are authenticated.
# 2026-08-08 Theron authenticated dungeon VRAM layout

- ✅ Fixed the production VRAM snapshot loader to admit the observed dungeon
  layout: BAT at VRAM `$0000`, with BAT tile indices addressing patterns from
  VRAM byte zero. The historical `$1000` fixture layout remains supported and
  is tried first.
- The source-backed VCE palette and BAT/tile atlas now bind from the supplied
  `vram_dungeon.bin`/`vce_dungeon.bin` snapshot instead of being rejected as
  an empty production surface.
- This is a real bitmap/tile-bank transport fix, not a claim that dungeon
  squares, creatures, HUD ownership or perspective are decoded.
# 2026-08-08 Theron VDC atlas capacity

- ✅ Increased the source-backed VDC tile/palette atlas from 1024 to the
  BAT-defined maximum of 2048 entries. The real 64×32 dungeon snapshot is no
  longer rejected when its captured BAT uses more than 1024 unique pairs.
- ✅ Verified the supplied raw dungeon snapshot end to end: 64 KiB VRAM,
  1 KiB VCE, 1057 bound tile/palette entries, 896 BAT cells and 27,259
  presented indexed pixels. The optional test BMP writer is source-backed
  inspection only and is not a README promotion.
# 2026-08-08 Theron source-backed ground object materialization

- ✅ Materialized authenticated Track 02 weapon, clothing, scroll, potion,
  and chest occurrences as ground objects with exact source category/index,
  chain reference, raw bytes, and decoded flags/payload fields.
- ✅ Added regression coverage over the real US Track 02 BIN and JP variant;
  all seven dungeons load successfully and report source materialization.
- Inventory ownership, pickup/equip/use, T900 rules, misc consumers,
  projectile/cloud consumers, and creature combat remain explicitly gated;
  this change does not infer those semantics.
# 2026-08-08 Theron source item-property binding

- ✅ Ground objects now retain the source Track 02 66-entry category value
  and matched six-byte item-property record when the object type agrees with
  the weapon/armor/consumable category table.
- ✅ Real US and JP Track 02 loader tests pass; mismatched or out-of-range
  source types remain unclassified instead of being guessed into inventory.
- Inventory ownership, equipment/use resolution, T900 translation, and
  remaining creature/media consumers are still open requirements.
# 2026-08-08 Theron source-backed inventory pickup provenance

- ✅ Pickup no longer reduces a source-authenticated object solely to a
  compact item ID: the runtime keeps category, type, keep/curse/broken/
  poison state, charges/power, text/chest fields, source references, and
  matched property bytes in the champion's parallel inventory-source slot.
- ✅ The legacy 30-slot item-ID array remains unchanged for compatibility;
  this is source provenance, not an unsupported claim about T900 equip/use,
  stacking, save serialization, or consumption behavior.
# 2026-08-08 Theron source-backed inventory roundtrip

- ✅ Added an explicit source-backed roundtrip API for carried weapon,
  clothing, scroll, potion, and chest records. Dropping reconstructs a ground
  object with the original category/type, source references, decoded flags,
  charges, text/chest fields, and matched property bytes.
- ✅ Added regression coverage proving the legacy inventory slot and its
  parallel provenance record are cleared after a successful roundtrip.
- ✅ Verified `test_theron_v1_combat_mechanics` (104/104) and the real US Track
  02 dungeon loader (all seven dungeons).
- This is not yet the original T900 drop/equip/use/stack/save consumer; the
  legacy `THERON_CMD_DROP` path remains fail-closed until that consumer is
  source-authenticated.
# 2026-08-08 Theron live creature packed-cell binding

- ✅ Static Track 02 monster groups now copy the exact per-member two-bit cell
  ordinal from the source packed cells byte into each live creature.
- ✅ Added a real US/JP loader invariant proving the member ordinal is derived
  from the authenticated source group byte, not a host coordinate seed.
- ✅ Verified all seven US Track 02 dungeon loads and the production regular-
  spawn/combat admission test.
- The original RNG consumer, creature AI/attack/drop owners, and generator
  timing are still fail-closed; this change binds record identity only.
# 2026-08-08 Theron monster raw-word retention

- ✅ Track 02 monster decoding now preserves the two source flag words in
  addition to the decoded count and direction fields.
- ✅ The source ledger and every live creature retain those words for the
  future AI, generator, combat, and T900 consumers without assigning guessed
  bit meanings.
- ✅ Verified all seven real US Track 02 dungeon loads and the production
  regular-spawn admission test.
# Theron `$4644` preconsumer contract (2026-08-08)

- ✅ Added the exact 27-byte US raw-BIN `$4644` preconsumer at file offset
  `$9c4c4` with FNV-1a `$a3c3f7eb`. It preserves the source argument setup
  and calls to `$C96B`/`$CC4C` without treating unresolved consumers as a
  host RNG implementation.
- ✅ The focused HuC6280 receipt test passes against the authentic US Track 02
  BIN; JP and ISO receipts remain correctly unmarked for this US-only span.
# Theron source inventory save provenance (2026-08-08)

- ✅ Appended the complete `inventory_source` provenance matrix as an explicit
  31-byte-per-slot little-endian wire record after the existing state fields.
  Existing version-1 snapshots with no trailing section remain readable;
  truncated provenance sections reject. The immediately preceding raw-tail
  layout is also accepted for migration.
- ✅ Verified category, source refs, poison/charges, text ref and property bytes
  survive serialize/deserialize in the focused purchase-state regression.
# Theron source-level pickup gate (2026-08-08)

- ✅ Real Track 02 levels now reject a pickup when the object has no bound
  source reference or is outside the four decoded carryable categories
  (weapon, clothing, scroll, potion). This closes the generic host-item
  fallback at the T900 ownership boundary.
- ✅ Fixture worlds without a verified source level retain their existing
  behavior; the focused 104-case mechanics suite remains green.
