# Firestaff TODO - Open Work

- 🔧 Theron real Track 02 loading now retains reserved category-4 monster
  bytes as source records while admitting only the authenticated 0..6 roster
  to live creatures. The original RNG consumer, generator timing,
  reactivation, AI, combat, loot and T700/T900 consumers remain capture-gated.

- 🔧 Theron type-6 generator records now decode the real generation,
  toughness and pause overlay and persist it in save version 6. This remains
  source data only: original RNG consumption, activation timing,
  reactivation and creature-spawn ownership still require the authentic
  HuC6280/System-Card capture and must remain fail-closed.

- 🔧 Theron authentic runtime capture now has a verified real SDL2/Quartz-capable Mednafen build. It still needs the verified US System Card 3.0 and original media capture; RNG return values, spawn timing, AI, T700/T900 consumers, media bindings and gameplay semantics remain fail-closed until that capture is obtained.

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

- **DM2-CHAMPION-DYN4-LOCALLEVEL-QUEUE:** PC-DOS boot now retains the
  sixteen source File_header champion-mirror records across their real
  `w0`-kedjor and can join an exact mirror to its matching `CHAMPIONS` Raw8
  and text template, plus the source ObjectIDs that `DM2_SELECT_CHAMPION`
  would hand to inventory from that same tile chain. The complete source
  roster is available in canonical mirror-chain order. It must not
  materialise a hero or move an item independently. Recover the complete
  `DM2_LOAD_LOCALLEVEL_DYN` queue
  owner, including its conditional `0x16ffffff` marker, party-dependent
  selectors and `DM2_LOAD_DYN4` transaction before DYN4 bytes, hero state or
  a New Game party can be admitted. 2026-08-08: den äldre callback-modellen
  för championval är nu utesluten även ur M10. Den kan bara köras av sitt
  explicita kontraktstest, eftersom den saknar den atomära ägaren för
  File_header, `c_hero`, possessions, timerkö och HUD.

- **DM2-GAME-LOAD-OWNER-HANDOFF:** New Game and Resume now both stop at the
  original `DM2_GAME_LOAD` boundary. Recover one atomic owner for map,
  record pools, possessions, heroes, timers and actuator generators, then
  install its source-shaped session handoff here. A boolean or a parsed save
  receipt alone must never make a game playable.

- **DM2-GAME-LOAD-OWNER-HANDOFF (path identity):** The hash-selected loose
  `GRAPHICS.DAT`/`DUNGEON.DAT` owner is now normalized through filesystem
  links before boot opens optional companion media. Continue to keep this as
  path identity only: it does not supply the missing original `GAME_LOAD`
  session owner or permit play.
  Rörelse, vändning och runtime-tick kräver nu samma owner-bit som M11.
  Timer- och renderhändelser ska fortsatt flyttas till den atomära handoffen i
  stället för att förlita sig på en monterad File_header-värld.

- **DM2-NEW-GAME-MEDIUM-IDENTITY:** New Game now refuses a stand-alone
  `assets_verified` flag or a caller-supplied rescan path. It requires the
  M12/M11-mounted, hash-selected graphics and File_header dungeon pair, but
  still stops before `DM2_GAME_LOAD` creates the missing party/record/timer
  graph.

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
  ground-stack/`w0` chains are now source-validated and all 44 canonical maps
  have one bounded callback walk. DB0 doors, DB1 teleporters and DB3
  actuators now use that same owner, liksom DB2 text/special-marker fields
  och DB4-varelsers placering, possessionslänkar och HP. Bind den till
  kartans grafiklistor, aktuator- och sensorvägar samt full
  `DM2_LOAD_LOCALLEVEL_DYN` queue before enabling DYN4 or gameplay.
  Bootprofilen kan nu lämna samma tre scenreceipts från den redan monterade,
  hashverifierade dungeonen utan att någon konsument läser rådata på nytt.
  En komplett typcensus från samma vandring finns nu som kontrollgrund för
  triggers, trappor, objekt och framtida local-level-konsumenter.

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
- Theron: keep original-data presentation fail-closed until a runtime capture joins the HuC6280 dynamic level/palette consumers to source LBA/FIFO and VDC destinations. Static bank-$1f and VCE receipts do not prove a complete bitmap/palette/viewport mapping.
- Theron: world snapshots now have endian-stable scalar envelopes, portable object/timer/creature records and bounds checks; progression/champion packing still needs a source-owned wire layout before claiming complete cross-host save parity.
- Theron: runtime VRAM/VCE file admission is now byte-identified by the authenticated capture FNV receipts; the remaining presentation gap is the original square/material/perspective consumer and a source-owned HUD/text/portrait route.
- Theron: the first real Track 02 screen-space capture is now published in README; replace the capture-only route with the authenticated T520/T600 square, perspective and HUD consumers when their runtime ownership is recovered.
- Theron: world snapshot version 5 now preserves all 64 source-generator runtime slots and reads version 4's five-slot tail; bind the original T700 generator consumer, cadence and reactivation semantics before making those records executable.
- Theron: regular-spawn admission now requires the matching raw Track 02 monster type and non-empty source group record; bind the dynamic RNG return contract before publishing those records as new live creatures.
- Theron: verified-level moves now dispatch the common world tick without guessed stat drains; bind the original T700 field consumer and exact cadence before mutating hunger, water, stamina or poison.
- Theron: source pool and altar objects are now fail-closed against fixture recovery/resurrection rules; bind their T700/T900 object consumers before enabling real mutations.
- Theron: source inventory drops now mirror the pickup property/category/type gate; bind the original T900 stack/placement consumer before enabling complete item transitions.
- Theron: source inventory, object, timer and admitted live-creature provenance now use explicit field-by-field wire layouts; retain version-1/2 readers only as migration support.
- Theron: the loader now retains complete source occurrences for categories 0..10, 14 and 15; bind those control records to their original disassembly consumers without conflating the occurrence census with executable world-object ownership.
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

- **NEXUS-SCSP-READ-CORRIDOR:** The external European gameplay producer now
  supports bounded sound-CPU SCSP-read tracing with optional 68K-PC filtering.
  A 100-record authenticated window reached `SDDRVS.TSK` setup/shared-RAM
  reads, but contained no reads in the `0x100400..0x100401` SCSP mailbox
  range and no `0x3224`-filtered row. This is a negative observation, not a
  SLEV/SAL playback proof; retain semantic admission and host playback as
  blocked until a trace joins a real event command to the driver consumer.

- **NEXUS-SATURN-ACTIVE-VDP1-WITNESS-JOIN:** An external European Mednafen
  V2 gameplay capture now proves two non-idle VDP1 frames at `PTMR=02`,
  `EDSR=03`, `COPR=00000c`, with a stable systemclip/local-coordinate/type-2
  textured-command/END window and changing draw-buffer/VRAM state. The raw
  artifact is kept outside the repository and is hash-bound in
  `docs/NEXUS_RUNTIME_CAPTURE.md`. The captured framebuffer is visibly a
  dungeon scene, but the command source is not yet joined to a source-owned
  DGN/mesh/texture, HUD, menu or CLUT record. Keep production presentation
  gated until those identities are bound.

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
  file. This improves static owner evidence only; execution, command-source
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

- **NEXUS-SAL-HOST-PCM-BOUNDARY:** The production SAL path must remain a
  byte-level receipt path only.  PCM format/rate/looping, voice ownership and
  MAP→event handoff still require a Saturn SCSP/SDDRVS execution capture;
  `nexus_sound_decode_sal()` is therefore kept as an explicit no-op until
  that capture is admitted. The public mixer is also silence-only, even when
  diagnostic voice fields are manually populated.

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
  startup/menu routine is now checked as an SH-2 function with exact
  PC-relative references to the retail `MENU.BPK`/`STABG.BIN` literals and a
  retained hardware literal. The same source receipt now binds the adjacent
  `yam\\vdp2.c` marker at `0x38CF4` and six exact address-literal slots at
  `0x28098`, `0x28640`, `0x28778`, `0x2887C`, `0x289E0` and `0x28E1C`.
  The static receipt now also verifies the nine retail SH-2 `MOV.L` loads at
  `0x27FE6`, `0x28002`, `0x285C6`, `0x28710`, `0x287AA`, `0x2880A`,
  `0x2885A`, `0x288B2` and `0x28D76` that target those slots. This still proves
  source/code ownership only. The receipt now additionally verifies the
  `MPOFN`, `COAR`, `COAG` and `COAB` literal loads at file offsets `0x28E78`,
  `0x28E98`, `0x28EAA` and `0x28EB8`, plus their adjacent SH-2 word stores.
  The same receipt now binds the VDP1-init literal pool at `0x7D3B8` through
  `0x7D3C0` and its adjacent framebuffer-control stores. Recover the
  executed VDP1 command-list/VRAM writes, the remaining VDP2 register/VRAM
  writes, and the tile/CLUT consumer from an instrumented Saturn capture;
  these static receipts do not authorize host drawing before admitting menu
  placement, FONT256 text, HUD composition or viewport pixels.

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

- **CSB-FMTOWNS-C06-UTILITY-TRANSACTIONS:** The F31E empty editor frame now
  no longer reaches M11 as a host reconstruction. The prior page combined
  C06 coordinates and string/palette bytes with guessed rectangles and the
  unrelated PC34 M653 renderer. Utility now remains on the real SWITCHTW
  page until UTILE/UTILJ's native EGB text and editor consumers are bound.
  Recover the F31J Shift-JIS glyph consumer and the original
  file-picker/save/portrait-edit transaction owners before exposing those
  commands; a C06 hit rectangle must not mutate a Firestaff save or
  manufacture a champion. 2026-08-06 follow-up: verified
  FM Towns ZIP media now materializes its original image into the user cache
  across volume boundaries, so this remaining work starts from the real CD
  payload rather than a partial archive extraction.
  2026-08-06: C09_ICON no longer comes from a Firestaff palette mirror.
  The verified UTILE.EXP/UTILJ.EXP C06 receipts retain their actual indexed
  RGB6 table at raw offsets `0x17DB0`/`0x17E18`, including the `0xFF`
  terminator. The remaining file-picker, save, portrait-edit and F31J glyph
  consumers remain separate source-owned work. 2026-08-06 source audit:
  F31J's `F0952_JAPANESE_Print` is the FM Towns `EGB_sjisString` path, not the
  PC-98 port-I/O or X68000 IOCS branch. `T_OAK2.EXE` and `OAK2USR.DIC` are
  present on the retail CD but do not establish a C06 glyph bitmap owner.
  Keep Japanese C06 drawing closed until an EGB/system-font capture proves
  the actual glyph handoff; see `parity-evidence/csb_fmtowns_f31j_text_owner.md`.

- **THERON-FORCEFIELD-REAL-DUNGEON:** Enter now reliably dispatches from the
  Soul Room forcefield focus, including the first attempt without prompt text.
  Authenticated raw MODE1/2352 Track 02 now reaches the source-faithful
  initial level/object handoff. Keep VDC/VCE presentation and later object
  semantics gated until their original consumers are bound; do not replace
  either gate with fallback dungeon data.

- **THERON-STARTUP-ANIMATION-CONSUMER:** Real Track 02 startup bitmap spans,
  atlas routes, font tiles and variant palettes are now receipt-bound, but the
  original title/Soul Room animation command, frame table, VBlank cadence and
  VDC/VCE destination are not yet identified. Recover that consumer from the
  disassembly or an original-emulator/app capture before exposing changing
  source frames or claiming animation parity. The former M11 timing receipt
  has been removed; M11 now presents one static source-backed title frame until
  the original consumer is identified. 2026-08-06 follow-up: authenticated
  Track 02 media now suppresses the host border/text fallback even when the
  graphics executor has not yet run; the startup surface remains capture-gated
  rather than leaking synthetic pixels. Authentic font-tile bytes also remain
  behind this gate: host render-plan coordinates/styles cannot make them a
  visible startup menu until a captured VDC/VCE text consumer binds the route.
  2026-08-06 follow-up: the authentic US RAR/CUE and hash-verified assembled
  MODE1/2048 Track 02 ISO are accepted by the instrumented capture harness,
  but the available headless SDL run remains before the game-owned CD read
  (`non_system_card_pcecd_reads=0`). A real interactive SDL capture is still
  required before admitting startup animation timing, menu input, or later
  level consumers.

- **NEXUS-PRS3-PIXEL-CAPTURE:** Retail Structure2 descriptors still expose
  bounded DMWeb format evidence, but the runtime no longer decodes their
  08h/28h payloads into host surfaces during LEV load or through the public
  decode helper. Recover the Saturn pixel order, CLUT owner and VDP1 command
  capture before reopening texture surfaces, palette conversion or DGN raster
  submission.

- **NEXUS-STARTUP-TEXT-CAPTURE:** Retail PLRD rows now refuse stale
  serialized ASCII names in the startup footer/row-label lane; only the
  isolated compatibility roster may expose host labels. Recover the Saturn
  TEXT4/TABL/FONT256 consumer and placement before admitting real names,
  stats, or action labels to the menu.

- **NEXUS-FONT256-FRAMEBUFFER-CAPTURE:** The obsolete flat 1bpp text bridge
  is now fixture-only and the production draw seam stays fail-closed. Recover
  the Saturn page/tilemap/attribute-to-character mapping and screen placement
  before admitting real FONT256 text to the HUD or startup framebuffer.
  2026-08-06 follow-up: Track-1 and S2D probes no longer use the flat parser
  for the real `FONT256.S2D`; the retail receipt is now limited to five named
  regions and 242 authenticated 8x8 CG tiles. The old 256-slot/map receipts
  were removed from the real-data path.
  2026-08-06 follow-up: `nexus_v1_saturn_font.c` remains in `firestaff_nexus`
  only for its real SCR section-table parser; `FIRESTAFF_NEXUS_PRODUCTION`
  compiles out the indexed host framebuffer writer. Keep the Saturn text
  consumer and VDP2 placement capture open.
  2026-08-06 follow-up: the documented English Saturn revision
  (`SHA-256 764a2d6c…`, `MD5 7bea3db1…`) now passes the common SCR header,
  section-table, four-section-chain and 242-CG-tile source receipts. Its
  revision-specific opaque section-2 statistics are now admitted separately
  (`857` populated blocks, `68` runs, byte counts `8890/3498/3100/16`) by
  the real-data grammar test. This closes a false SKIP only; glyph mapping,
  pixels and VDP2 text placement remain capture-gated.

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
  pixels while the retail HUD/VDP1/VDP2 capture is absent.

- **DM2-PRODUCTION-PLACEHOLDER-INVENTORY:** The source tree retains bounded
  compatibility studies, fixture readers and modern-art probes for direct
  regression work, but none may enter `firestaff_m10`, `firestaff_dm2` or
  `firestaff_dm2_v2` without a live original-data owner. The CTest
  `dm2_production_placeholder_boundary` inventories the current excluded
  modules and now also locks the live viewport's named fallback counters to
  reset-only observability fields. Remove an entry only together with its
  source-backed M11/runtime handoff and real-data regression; never make a
  broad glob the admission mechanism. The focused FM Towns M11 build also
  explicitly acknowledges the four retained, caller-compatible `cells`
  parameters in DM1 viewport helpers: each helper samples the live viewport
  cell through its source-owned state instead. This is warning hygiene only;
  it does not alter DM2 admission, drawing or source ownership.
  **2026-08-06 outdoor-facade update:** removed the legacy
  `dm2_v1_outdoor_renderer` no-draw/procedural-sky facade from `firestaff_dm2`.
  Its focused material-boundary test still compiles it explicitly; production
  outdoor presentation remains owned by the authenticated
  `ENVIRONMENT`/`DistantEnvironment` GDAT runtime chain.
  **2026-08-06 viewport update:** removed the uncalled world-model square
  synthesizer from the production viewport. Its depth-parity wall selection
  and default full-light value had no original `c_light` or map/GDAT owner;
  the boundary verifier now rejects their return.
  **2026-08-06 generic-blit update:** removed three further uncalled
  viewport blitters that accepted arbitrary bitmap buffers, tiling or scaling
  without a GDAT image/local-palette receipt, together with their uncalled
  DM1-derived clip helper. Only the source-material consumer remains eligible
  to write a runtime viewport.
  **2026-08-06 world-model update:** removed the retired, disabled 16-bit
  descriptor/tile parser rather than retaining it as an apparent DM2 route.
  The world model now has only the verified G1 byte-square loader; this does
  not promote incomplete G1 record traversal to gameplay.
  **2026-08-06 CCM update:** removed the disabled reduced-state CCM bridge.
  It reconstructed operands and world/door mutations from a fixture-shaped
  creature instance, rather than the original DB4/CAII handler transaction.
  Production remains fail-closed until that source owner is complete.
  **2026-08-06 V2.2 update:** removed the disabled local-manifest availability
  path. Manifest parsing remains diagnostic-only; neither a local pack nor
  its category names can make DM2 material playable without an original-data
  receipt.
  **2026-08-06 V2 HUD update:** removed the disabled procedural pixel renderer
  rather than retaining generated compass, label, bar and action-strip code.
  The source-backed GDAT HUD runtime remains the only admissible draw owner.

- **DM2-SOUND-TEXT-CONSUMER:** `c_sound.cpp` binds class triples to GDAT sound
  payloads but does not supply a sound-name text producer. The production
  compatibility name API deliberately returns `NULL` rather than exposing
  hand-written English labels. Recover an original text consumer and bind it
  to the exact GDAT entry before displaying sound names.
  **2026-08-06 real-data check:** the supplied PC-DOS `GRAPHICS.DAT` contains
  zero `DM2_GDAT_ENTRY_TYPE_TEXT` rows in category `0x02` (SOUND); its SOUND
  rows are payload entries only. `test_dm2_v1_sound_gdat_real_data` now
  asserts this corpus fact, so no sound names may be derived from class
  triples or copied from a host table.
  **2026-08-07 corpus update:** the SOUND-GDAT real-data probe now accepts
  only explicit `FIRESTAFF_DM2_DATA_DIR/graphics.dat`; a selected unreadable
  corpus fails instead of falling back to a private installation.

- **DM2-WEATHER-TEXT-CONSUMER:** SKProject's `c_weather.cpp` exposes numeric
  state and GDAT command transitions, not display-name text. The former
  hand-written `Clear`/`Rain`/`Fog`/`Storm` compatibility labels are removed;
  `dm2_v1_weather_name()` now returns `NULL`. Bind an authenticated text
  producer before presenting weather text in a player-facing UI.
  **2026-08-07 corpus update:** the IMG9 global-palette and
  G1→GRAPHICSSET→weather/light real-data regressions now consume only the
  selected `FIRESTAFF_DM2_DATA_DIR` corpus and fail an unreadable selection;
  no private HOME installation can supply their material.
  **2026-08-07 source-text audit:** the selected PC-DOS set-5 corpus contains
  nine weather-command text rows at fields `0x64..0x6c`, plus two generic
  environment-element rows at `0x01` and `0x63` used by the broader
  `skguivwp.cpp` distant-element scan. None is a weather display-name record;
  the real-data regression asserts this split and keeps
  `dm2_v1_weather_name()` unavailable.

- **DM2-FIXED-SPELL-AI-TEXT-CONSUMER:** The fixed `dSpellsTable` and genuine
  AI table own mechanics only. Names visible beside SKProject's fixed spell
  rows are developer comments, and `getAIName()` is a `DEBUG_HELPER`; neither
  is retail text. The mounted PC-DOS `GRAPHICS.DAT` has no extended
  `SPELL_DEF` rows, so its `QUERY_GDAT_TEXT(..., 0x18)` route cannot provide a
  fixed-mode fallback. `dm2_v1_spell_name()` and `dm2_v1_creature_ai_name()`
  therefore return `NULL`. Bind a real selected-profile text consumer before
  showing spell or AI names.
  2026-08-07 audit: `test_dm2_v1_extended_spells_definition_real_data` against
  `/Users/bosse/.firestaff/data/dm2/dos_extract/data` reports zero source
  `SPELL_DEF` rows; `test_dm2_v1_source_name_helpers` and
  `test_dm2_v1_spell_rune_lookup_pc34_compat` also pass. No retail text owner
  was found, so the open consumer remains fail-closed rather than receiving
  guessed names.
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
  in-memory original-disc route only. Mega CD and PC-9821 still lack an
  equivalent selected-medium reader and therefore must remain silent rather
  than accepting pre-extracted `trackNN.raw` files or caller PCM. Recover
  each platform's original disc container and its audio-track binding before
  enabling playback.
  The FM Towns HMP→CDDA map is now read from the selected original
  `SKULL.EXP+0x3dac` buffer in RAM. Keep playback itself silent until native
  SKULL execution and CDDA transport ownership are joined; do not revive a
  source-literal lookup table.
  FM Towns runtime now queries the authenticated CD.DAT table with its live
  party X/Y after a committed move, rather than the former invented `(0,0)`
  coordinate. Continue validating the complete native music-dispatch timing
  against executable capture before claiming full audio parity.
  **2026-08-13 transport gate:** the shared CD.DAT dispatcher now refuses to
  expose a track for Mega CD or PC-9821, even when their table bytes are
  present; only FM Towns currently has a selected original-disc transport.
  Keep those platforms silent until their native medium readers are recovered.
  SKULL.EXP now also has a bounded original P3 header receipt before the
  disc can enter the startup route. Recover the native P3/TBIOS menu code,
  its input loop and the handoff back to M11 before showing any SKULL menu.

- **NEXUS-VDP1-COMMAND-SOURCE-JOIN:** The authentic eight-frame witness now
  provides a strict command-to-VRAM join: `COPR=0x00000c` gives system
  records `0x09`/`0x0a`, one type-2 bitmap draw and END; frame 7 uses
  `PMOD=0x0028`, `SRCa=0x8f80`, `SIZE=0x28b4`, mapping to byte offset
  `0x47c00`, which matches writer PC `0x06013098`. The 33,280-byte source
  span still lacks a decoded MENU/DGN/ITEM owner and VDP2 CLUT/tile join, so
  menu/HUD/viewport production drawing remains blocked.

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
  FM Towns and Amiga retain their platform-owned bounded RAM readers, while
  PC-DOS ZIP/ISO media and renamed loose pairs remain source-path diagnostics
  and block launch until a complete in-memory PC reader exists. The M12
  profile store now accommodates every declared profile, rather than
  truncating after seven entries. CSB's verified retail FM Towns
  ZIP now stages and admits its raw CD image without crashing when it is the
  only CSB candidate; continue to report the matched platform/profile whenever
  only a partial or unsupported package is present. The authentic legacy FM
  Towns RAR with a MODE1/2352 `.bin` now follows the same verified CD route
  through `unrar`; the launcher now turns missing archive-reader diagnostics
  into a localized recovery popup. Keep that popup aligned with scanner
  diagnostics as further archive formats are admitted. External-archive
  member hashes now share a bounded cache keyed by archive identity, so
  repeated profile scans do not re-extract already-verified members. Continue
  to profile first-time scans of the full supplied corpus, especially nested
  Amiga/Atari media. KryoFlux `<track>.<side>.raw` streams inside external
  archives are now excluded from loose-file hashing: they are flux tracks,
  not ISO or named game-file containers, while ordinary `.raw` members and
  top-level raw CD images retain their supported paths. Raw GEMDOS Atari ST
  `.st` images and standard Magic
  Shadow Archiver `.msa` images now have bounded FAT12 paths, including
  `.st`/`.msa` media inside `.7z`. A nested MSA is decoded once for the
  complete profile hash list rather than once per required file. Windows
  fails closed for nested external Atari media until it has a native archive
  reader; protected STX remains intentionally
  unsupported until its transport is implemented and verified.
  The verified original Amiga 3.1 English ADF (A31E) now materializes and
  launches; keep the remaining title/entrance capture work separate from this
  data-admission boundary.

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
  the eight Red Book tracks 2–9, but neither the retained DM.BIN disassembly
  nor DMWeb/Greatstone format material proves the old `level / 2` mapping.
  Runtime level-to-track selection now returns unbound (`-1`) instead of a
  host-invented pair mapping. The CUE reader now separately reports whether
  every declared `FILE` payload exists; a valid Track 1 ISO no longer implies
  that the external CDDA files are available. Recover the original Saturn
  music consumer or an authenticated runtime trace before enabling CDDA
  selection/playback.
  2026-08-06: removed the host `track02.wav/ogg/mp3` lookup and callback route;
  a user-supplied host audio file is not a Saturn CDDA handoff and now remains
  selection-only until source-bound media evidence exists.
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
  passes all 162 retail surfaces, and DM.BIN's VDP1 register/state corridors
  are hash-bound. Keep this as byte/source evidence only: the current static
  PRS3 evidence regression now accepts only the authenticated English/French
  `MENU.BPK` identities (`a6f2272a4f6cb3c6b3b33012bc5b15ed` and
  `fcf8a00fbb92593ed9ae908f8e285cda`). Keep this as byte/source evidence only:
  receipts do not prove PRS3 opcode execution, CLUT upload, draw-command
  emission, destination placement or viewport/menu ownership. Do not promote
  the decoder or any VDP1 candidate into production pixels without an
  instrumented Saturn/Mednafen capture.
  2026-08-06: runtime decode and upload receipts now remain
  `BLOCKED_PRS3` even when the bounded DMWeb decoder succeeds. Decode byte
  counts and hashes remain diagnostic evidence; no decoded pixels, upload row,
  renderer handoff or fallback surface is marked ready without the missing
  Saturn CLUT/VDP1 capture.
  2026-08-06 follow-up: the renderer prerequisite normalizer no longer
  overwrites a `SATURN_PRESENTATION` block with `READY_STORED` after a
  source-authenticated stored/decoded route is found. The menu handoff now
  preserves the missing-capture status all the way to the caller.
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
  poison/status expiry and torch/FUL/ambient-light tick paths remain diagnostic
  until Saturn action dispatch and the corresponding HUD/VDP consumer are
  captured. Retail ISO/extracted engines must not mutate champion health,
  rest timers, status timers or light timers through these compatibility
  helpers. Keep fixture-only behavior for isolated unit tests and recover the
  original state-write/timing trace before reopening the runtime route.

  2026-08-06 follow-up: the separate engine-level hunger loop is now covered
  by the same retail no-mutation boundary; it no longer drains food/water or
  applies inherited starvation/dehydration damage before the Saturn
  start/save consumer is captured.

  2026-08-06 follow-up: `nexus_v1_rest.c` and `nexus_v1_status.c` are now
  excluded from `firestaff_nexus`; their original implementations remain in
  the explicit fixture library and production links a state-preserving ABI
  adapter. Do not restore them until Saturn action, timing and HUD ownership
  is captured.

  2026-08-06 follow-up: `nexus_v1_light.c` is now excluded from
  `firestaff_nexus`; its original torch/FUL/ambient state machine remains an
  explicit study input and production exports only a state-preserving ABI
  adapter. The separate light-overflow receipt/runtime remains diagnostic
  until its Saturn F0238/F0257 consumer is captured.

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
  object, ground, door, teleporter and creature-bank records are retained, but
  a positive game-owned post-startup CD read is still missing. The current
  text candidate emits unresolved control-code artifacts, so it must remain
  diagnostic-only until an executing-PC/source-LBA payload proves the consumer
  format. Do not enable production dungeon, object, tile, palette or viewport
  semantics from those records alone.
  2026-08-06 capture update: a 120-second authentic US Track 02 replay with
  repeated Run/I input still produced no game-owned post-startup consumer, no
  `$2600` handoff and no source-owned VDC/VCE destination receipt. The longer
  trace only confirms the existing bounded loader/main-RAM windows; it does not
  change the admission boundary.
  2026-08-06 capture-launch update: the Mednafen launcher now explicitly
  selects the PCE module (`-force_module pce`). Without that option, a CUE
  containing audio tracks could be misrouted to the CD-DA player before the
  HuC6280 consumer trace began. A fresh capture with the corrected route reads
  authentic Track 02 sectors and emits VDC/VCE snapshots, but still reports
  `non_system_card_pcecd=0` and no `$2600` handoff; semantic promotion remains
  closed.
  2026-08-06 ISO parser correction: the capture runner's `FILE`/`TRACK 02`
  association now accepts both authenticated `MODE1/2352` raw BIN and
  `MODE1/2048` ISO layouts. The previous mixed parser could identify the ISO
  mode and then reject its track member as raw-only. This closes only the
  capture-intake mismatch; it does not promote a game-owned consumer.
  2026-08-06 split-CUE capture follow-up: the live Mednafen runner now accepts
  the supplied archive's CRLF/okvoterade `FILE TQUS02.iso BINARY` spelling and
  reuses the production hash-verified assembled US ISO from the split
  `TQUS19.iso` + `TQUS02End.iso` distribution through a private normalized
  capture CUE. This removes the missing-member/raw-BIN capture mismatch; a
  fresh run is still required before any game-owned consumer or dungeon
  semantics can be promoted.
  2026-08-06 input-bound follow-up: a real PID-bound macOS key pair was
  observed by the instrumented SDL boundary. The source-locked raw intake
  still requires US `INDEX 01 = 225` and the BIN has a second valid MODE1
  sync at that source offset; an earlier sync-like span at sector 75 is not a
  reason to change the authenticated layout. Mednafen nevertheless reports
  uncorrectable sectors for this raw/CUE pairing before any game-owned
  consumer read. Keep this as an emulator/capture-media boundary, not a
  level/object/palette claim; do not promote later semantics or invent a new
  pregap normalization from the earlier span.
  2026-08-06 text-boundary update: the raw US decoder still retains its real
  codons for diagnostics, but production world text now rejects any block
  containing unresolved brace/control-code values. Reopen publication only
  after the original HuC6280 text consumer is disassembled and matched.
  2026-08-06 forcefield-input update: after a failed authentic admission,
  Enter now retries the source-bound forcefield action while the Soul Room
  prompt says `FORCEFIELD LOCKED`; it must not silently toggle a mirror.
  This fixes the host-menu dead-looking retry without opening fallback
  dungeon rendering. A separate bounded bank-$1f `$2600–$27ff` trace was
  added to the capture patch; the complete US replay emitted zero target
  rows, so the real dynamic consumer and level/object handoff remain open.
  2026-08-06 receipt-integrity update: source occurrences now retain a 16-bit
  category index, matching the 512-entry Track 02 category bound. Category
  to host item-kind/index ownership remains intentionally unbound.
  2026-08-06 descriptor receipt update: the real US Track 02 MODE1 user-data
  table at UD 0x619900 is now read and byte-verified as 53 six-byte records
  from the authenticated BIN. This closes only the descriptor-byte receipt;
  the referenced graphics blocks, object records, tile bank, palette and
  game-owned consumer handoff remain blocked.
  2026-08-06 disassembly update: the hash-locked US/JP bank-$1f receipt now
  covers the full byte-identical `$23AD-$252A` variable-bit/back-reference
  routine (382 bytes, FNV-1a 3056f96c). Its caller, destination and block
  contract are still unbound; do not apply it to the seven level blocks by
  byte-shape alone.
  2026-08-06 caller update: the byte-identical `$2386-$23A3` caller tail is
  now authenticated (30 bytes, FNV-1a 699e8da1). It proves `$30/$31` is
  measured as the decompressed output length through `$3B7C/$3B7D`; input
  block, destination bank and level/object meaning remain open.
  2026-08-06 resource-frame update: all seven US and seven JP level spans now
  validate the six-byte `$23AD` resource header and bounded `LE16(+2)-5`
  bitstream length. The exact header/bitstream slice is retained, but no
  level-specific table row or executing command is bound yet; no decoder
  output or tile/map semantics are admitted.
  2026-08-06 stage2-handler update: the byte-identical 162-byte generic
  resource handler at `$4C3F` now proves the four-entry MPR table population
  and `$3004/$3005` destination plus `$3006/$3007` produced-length handoff
  (FNV-1a `46360d97`) for both retail variants. This is a static generic
  contract, not a level-specific Track 02 consumer receipt.
  2026-08-06 regional-descriptor update: the authenticated 53×6-byte table
  at logical UD `0x619900` now has a variant-aware receipt. US records are
  admitted with FNV-1a `7aa82bc7`; the authentic JP span is recorded as
  `ZERO_FILL` with FNV-1a `63d8ddfd` and cannot fall through the US decoder.
  Referenced payloads and their executing consumer remain blocked.
  2026-08-06 probe-path update: the descriptor-table and level-handoff
  probes now discover the supplied authentic `TQUS02.bin` and `TQJP02.bin`
  directly under the documented `.firestaff/data/theron/` root instead of
  skipping on obsolete `theron-extras/` filenames. Both raw variants now
  exercise all three descriptor anchors and the real 32×27 startup candidate;
  the semantic dungeon/object consumer remains capture-gated.
  2026-08-06 palette-offset update: the real US/JP palette-shape candidates
  are now regionalized (`0x2A06A0` US, `0x29FD70` JP). The focused test reads
  both standard-root BINs and the startup binder no longer applies the US
  offset to JP media. This remains format evidence only; VCE ownership and
  production palette/viewport promotion still require an executing capture.
  2026-08-06 header-warning update: the world/Track 02 object-table forward
  declaration now has one guarded typedef owner. This removes the strict
  C11 redefinition warning without changing the record layout or opening
  object semantics.
  2026-08-06 Theron warning cleanup: removed dead raw-offset/joypad probe
  locals and marked the POSIX-only canonical-path capacity parameter unused.
  This keeps the authentic Track 02 receipt path warning-clean without
  changing any media offsets, hashes or runtime admission gates.
  2026-08-06 raw-disassembly follow-up: the HuC6280 bank-$1f receipt now
  admits the authentic `TQUS02.bin` and `TQJP02.bin` files with their real
  MODE1/2352 bank-window offsets and regional stage-2 hashes. This closes
  only static code identity coverage; the executing consumer and semantic
  level/object/palette handoff remain capture-gated.
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
  GAMEOVER.BIN and STABG.BIN loaders now retain a raw-source FNV-1a/size
  receipt on each decoded surface. This proves which bytes the loader
  consumed, but it is not the scanner's canonical SHA-256 and does not bind
  Saturn VDP1/VDP2 placement. Continue with capture-backed startup/menu
  handoff; do not promote these receipts into host rendering.

  2026-08-06 follow-up: the real TITLE.BIN MAPD/TIBG regression now joins its
  five authenticated 64x28 maps to the real TITLE.CG 8x8 atlas and checks
  non-empty decoded source pixels. This remains format evidence only; Saturn
  VDP2 tilemap, CLUT, timing and presentation ownership are still open.

  2026-08-06: the legacy Nexus overview/language/startup/champion/features
  documents were corrected to stop publishing synthetic Japanese-only claims,
  eight/24-name rosters, DM1-derived semantics, or completed renderer/audio
  behavior. Remaining documentation outside the focused startup/menu/HUD/
  viewport set still requires a full stale-claim sweep.

- **DM2-REAL-DATA-RENDER-INVENTORY:** The PC English `GRAPHICS.DAT` corpus is
  hash-locked and its G1-referenced scene sets now decode through the original
  IMG9 mode-1/2/3 dispatcher. The full real `dtImage` census (5,676 ENT1
  rows, 4,031 distinct RAW payloads) also decodes directly with no generated
  pixels. Continue inventorying real GDAT consumers by source owner:
  Greatstone's 5,624 exported visual items are not a one-to-one substitute for
  the file's 11,854 ENT1 rows, which also include text, words, palettes and
  raw controls. Bind each remaining visible HUD/viewport route to its exact
  GDAT record; do not make a count-derived asset table or arbitrary fallback. The shared
  IMG3/IMG9 decoder now belongs to the M10 loader boundary, so future
  real-data consumers must link that owner rather than introduce a parallel
  decoder or fallback.
  2026-08-06 champion-portrait update: the canonical PC corpus has no direct
  `CHAMPIONS/255/dtImage/0` row. SKProject's image query instead resolves
  that missing address to `MISCELLANEOUS/254/dtImage/254`. The HUD may use
  only that verified original source-default image until a non-default HeroType is
  observed in an authenticated save or original runtime capture; it must not
  create a portrait row or select another champion image.
  2026-08-06 V2 archive audit: the retained V2/V2.2 manifest, PNG and
  generated-HUD modules are explicit diagnostic/test owners only. The
  production `firestaff_dm2_v2` archive and linked `firestaff` binary export
  none of their render or bitmap-loader entry points; V2.2 remains routed to
  the source-preserving V1/V2.1 path. Keep this boundary while no original
  DM2 V2 art owner exists.
  2026-08-06 HUD-stat update: authenticated `c_hero` records now supply the
  live HP/stamina/mana current-and-maximum pairs to the M11 HUD. Continue
  requiring an original session handoff before showing dynamic bars; the
  legacy 261-byte convenience view deliberately cannot substitute for the
  missing maxima.
  2026-08-06 spell-feedback update: the retained C068--C070 failure class is
  not yet a text owner. Bind `PROCEED_SPELL_FAILURE`'s panel-global update and
  the class-0x30 `INTERFACE_GENERAL/SPELLING/NEED_FLASK` GDAT draw before
  presenting any spell-failure feedback.
  **2026-08-09 render-inventory verification:** the authenticated PC-English
  real-data boot/render checks pass for the complete M11 HUD command family,
  the active DIALOG_BOXES save/load panel, the G1 scene/light controls and the
  runtime frame ownership receipt. Their fallback counters remain zero. No
  additional visible surface was promoted: the remaining spell-feedback class
  still lacks its source-owned panel-global update and is deliberately blocked.
  **2026-08-13 spell-feedback source-material update:** class 0x30 now resolves
  the exact real `INTERFACE_GENERAL/5/dtImage/0x0B` NEED_FLASK material from
  the admitted PC-English GRAPHICS.DAT, including decoded pixels and local
  palette hashes, and carries the source destination rect `0x5C` as a runtime
  receipt. The receipt remains no-draw until the M11 transparent-static-pic
  consumer and C068--C070 panel-global owner are bound; no text or synthetic
  image fallback is allowed.
  **2026-08-07 panel-global source update:** the spell-failure receipt now
  carries the exact `DM2_UPDATE_GLOB_VAR` inputs for classes 0x10/0x20/0x30
  (target 0x45/0x46/0x44, add-one mode) and the source `DM2_MAX(3, 8 - value)`
  window contract. The live global state remains explicitly unbound, so no
  guessed old value or unrelated host mutation is presented as parity.
  **2026-08-07 destination update:** class 0x30 now also resolves destination
  rect `0x5C` through the authenticated `INTERFACE_GENERAL/0/dt04/0` RAW4
  table, retaining its expanded coordinates and table hash beside the real
  NEED_FLASK material. The receipt remains no-draw until transparent-static-pic
  surface ownership and the C068--C070 global owner are complete.
  **2026-08-07 coordinate audit:** the mounted PC-English corpus expands `0x5C`
  to `(456,100,92,77)`, while NEED_FLASK decodes to `92x25`. These coordinates
  are outside the 320x200 dungeon viewport and therefore prove that this route
  belongs to the larger M11 game surface, not the viewport framebuffer; do not
  blit it through the viewport consumer.

- **DM1-HOC-VIEWPORT-AUDIT:** Real PC34 HoC object-name, alcove placement,
  mirror side/depth, inscription/corridor invalidation and wall-material
  receipts pass on the current renderer. Remaining work is external Mac/app
  capture of torches, stairs, doors and the complete viewport; do not reopen
  the verified source routes without a failing real-data capture.

  2026-08-06 interaction audit: the source-side map-index stale-target bug is
  fixed and the real PC34 object/mirror/inscription tests remain green. The
  remaining risk is visual/package-level evidence only: verify that the
  packaged window shows the real torch plus holder, that side/depth mirrors
  remain source-visible, and that a held object follows the F0702 cursor.

  2026-08-06 interaction-runtime follow-up: the C071 eye route now retains
  its authenticated CHEST.C F0334 close mutation even when the source C101
  object-description surface is unavailable. It suppresses only the missing
  visual panel and still reports the consumed click, preserving the leader
  hand and the eight visible chest entries. The external-build pass1091
  verifier now consumes `FIRESTAFF_BUILD_DIR`, so out-of-tree Ninja builds
  no longer produce a false missing-binary failure. Packaged Mac capture of
  the held-object cursor and full inventory placement remains open.

  2026-08-06 source audit: removed a duplicate authenticated side-door blit
  from the deferred F0115 content pass. ReDMCSB F0111's exact panel/frame
  slices now remain the sole DM1/CSB source owner; the generic whole-pane
  helper is retained only for non-source diagnostics. Remaining work is
  packaged capture of door states, not another fallback texture.

  2026-08-06 keyboard potion-use audit: the generic M11 `UseItem` potion
  effect path is removed. Keyboard use now enters the same live PC34
  transaction as ReDMCSB PANEL.C F0349, including source-owned stat/health/
  wound/poison/food/water/shield formulas, the correct empty-flask type C20
  (20), and raw Thing-byte rewrite. Focused consumable and live-transaction
  tests pass. External packaged capture of the complete interaction remains
  open; do not mark the whole HoC interaction audit complete from these
  headless tests.

  2026-08-06 bug-hunt audit: real PC34 checks pass for 83/83 HoC floor-item
  render samples, 611 M564 object names/icons, F0115 pickup material and
  placement, mirror side/depth orientation, inscriptions, stairs, doors and
  damage-panel geometry. No source-side evidence currently supports changing
  those routes. The user-visible black/misdrawn torch-holder, held-object
  cursor, full inventory placement and complete Mac viewport capture remain
  open because they still need a packaged-window capture at the affected
  real-data poses; do not close them from headless receipts alone.

  2026-08-06 raw-Thing icon guard: DM1 F0115 floor and alcove consumers now
  resolve the item subtype from the live PC34 Thing record before selecting
  the GRAPHICS.DAT object aspect. A stale decoded subtype can no longer show
  an unrelated junk/torch/food icon after pickup, placement or save restore.
  The packaged Mac capture of the affected HoC poses remains open.

  2026-08-06 interaction follow-up: the keyboard drop command now also
  consumes G4055 directly, so a held object no longer depends on the pointer
  viewport hit-test before it can return to the current party square. The
  remaining open item is packaged-window visual capture, not the source item
  mutation route.

  2026-08-06 renderer audit/fix: corrected the PC34 wall-view dispatch from
  the erroneous 15-row host list to ReDMCSB's real 13-row G0205 table. D3L2
  and D3R2 remain owned by their separate F0676/F0677 square passes. The host
  wall consumer now applies the source-locked F0675 14/32 (D3) or 21/32
  (D2/near) derived scale and clips the real bitmap against the G0205 zone as
  F0791 does, instead of stretching the full native surface to the zone. The
  remaining gap is packaged Mac/app pixel capture for global ornaments 38 and
  43 and exact F0635 alignment evidence; no replacement art is admitted.

  2026-08-06 G0190 bitmap-selection fix: the native derived-bitmap increment
  now consumes ReDMCSB's exact 13-entry PC34 table. D3L-front and D3C-front
  no longer receive the erroneous extra increment, while D1C/front-mirror
  and the other depth transitions use their source-defined offsets. The
  focused wall-ornament test passes all 13 rows; packaged Mac pixel capture
  remains open.

  2026-08-06 C127/C346 source-ID fix: the Hall-of-Champions mirror now binds
  its dedicated C346 GRAPHICS.DAT surface after reusing the G0205 D1C zone.
  The generic global-43 ornament calculation remains 349 for ordinary wall
  rendering, but real PC34 slot 349 is 16x19 and cannot satisfy the native
  48x43 C346 receipt. Real-data C127/C026 pixel verification now passes;
  packaged Mac capture remains open.

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
  `CDATA`/`CJDATA` files, title animation, portraits and executables now reach
  the CSB runtime cache without synthetic replacements. Keep launch/runtime
  behavior capture-gated until the FM Towns-specific presentation, input,
  audio/CDDA playback and save handoff are demonstrated against original
  media. `TITLE.ANM`, `STORY.ANM` and `ENDING.ANM` now decode their real
  F2275/F8288 frame streams and retain their source Timer-A waits and loop
  execution and source `TD`/`TR` CD-DA track requests. M11 now displays the
  verified `TITLE.ANM` stream against its Timer-A timing and, at EOF, the
  source-owned package-language `SWITCHTW` surface. ReDMCSB
  `NECIO.C` launches `anim title.anm` as a standalone program; `SWITCH.C`
  owns the separate player-selected `anim story.anm` route, and `STARTUP2.C`
  calls `F2248_PlayAnimation("ending.anm")` after a winning game. Do not
  auto-chain the title into Story. The distinct FM Towns switch-menu/story
  owner, input, audio/CDDA playback and save handoff still need
  original-media capture.
  2026-08-06: `SWITCHTW.EXP` is now admitted as the source of the Japanese
  and English 320x200 Switch pages and its four exact button streams. The
  extractor requires the complete F2279 resource sequence and IMG2 decode;
  its `C26_SWITCH` (G8172_) palette is likewise read only from that verified
  executable. The original four button rectangles now retain their
  `AUTOEXEC.BAT` Story/utility/game exit handoffs and language toggle.
  M11 now follows `TITLE.ANM`'s return into the original package-language
  Switch surface after SWITCH.C's sixty source-VBlank wait, retaining executable
  palette, page and button pixels. It also reproduces the language toggle and
  hands Story to `ANIMTW STORY.ANM`, returning to the selected language's
  Switch loop when that stream completes. Utility and Game remain deliberately
  modal: `UTILJ`/`UTILE` are separate CEDT executables and `CHTWJ`/`CHTWE`
  are separate Game executables, so neither is routed through a PC34 surrogate.
  Their authentic handoff and save transfer still need original-media
  capture. The retail `CDATA/MINI.DAT`
  and `CJDATA/MINI.DAT` files are explicitly not admitted by the Atari/Amiga
  GAMEBLOCK decoder: both differ from that big-endian layout and remain
  outside Resume until the F31E/F31J save-header and runtime handoff have
  source-backed corpus evidence. The shipped F31 bootstrap files are now
  identity-bound at the Game handoff (`CDATA/MINI.DAT`, 42 776 bytes,
  FNV-1a `494999c9`; `CJDATA/MINI.DAT`, 43 208 bytes, FNV-1a `284799d1`),
  and validates their native C5 header through `F7061` with key word 29,
  language-owned F7/F8 platform and C13 CSB-Game dungeon identity, but
  the following five F7057-checked save parts now also have real-corpus
  receipts, followed by an F7063 byte-sum check of the native dungeon tail.
  The shared dungeon reader now consumes MAP's real byte-6/7 coordinate
  origin rather than its byte-4/5 unreferenced padding, and can consume the
  verified F31 tail bytes directly. It retains the original party's verified
  `GLOBAL_DATA` pose/time. 2026-08-08: the selected F31 C03 Game handoff now
  also decrypts its exact `C2_SAVE_PART_PARTY` part and binds the four
  319-byte champion records to the live HUD/inventory state. It deliberately
  retains the F31 `DUNGEON.DAT` runtime pose until the authenticated MINI tail,
  events and timeline can transfer atomically; mixing the verified MINI pose
  with another live dungeon body would be a false resume. User-save
  persistence and complete event/timeline restoration remain open.
  The shipped English and Japanese seeds both contain 23 scheduled events
  (436-slot heap) and eight live `ACTIVE_GROUP` entries; a resume must restore
  those original owners with the same MINI dungeon transaction, never clear
  them or synthesize an empty timeline.
  2026-08-06: the F31E `CDATA/GRAPHICS.DAT` and `DUNGEON.DAT` pair now
  opens the source-bound C001--C005 entrance and C017/C040 HUD session only
  after both original CD hashes admit. M11 binds its package-language Switch
  Game selection to the separate `CHTWE.EXP` handoff, draws its authenticated
  C004 entrance raster with the source-owned entrance palette, and never
  replays PC3.4 TITLE.C. A real-media regression covers that first entrance
  frame, the C002/C003 door-opening sequence, and the nonempty C017/F0128
  live HUD/viewport frame after Prison. Audio/CDDA and save-path capture
  remain open.
  2026-08-06: the Game exit now has a separate retail-program admission
  receipt: F31E accepts only `CHTWE.EXP` (283936 bytes, FNV-1a `3da136f6`)
  and F31J accepts only `CHTWJ.EXP` (284416 bytes, FNV-1a `f937db45`). This
  is the `C03_GAME` owner declared by ReDMCSB `COMPILE.H`, not a PC3.4
  substitute. M11 now consumes that receipt from the Switch Game exit and
  enters the verified F31 C004 entrance session without replaying TITLE.ANM
  or falling through to PC3.4 input. 2026-08-06 follow-up: F31J's selected
  CJDATA cache now keeps its top-level hash-verified pair ahead of the CDATA
  sidecar directory, so the Japanese Switch Game exit reaches `CHTWJ.EXP`,
  door-opening and the same live C017/F0128 handoff. Original-media audio,
  Utility and save transactions remain required.
  The selected FM Towns cache now retains the original CUE/IMG media pair,
  not a derived track map. M11 dispatches each F2275 TD/TR request through
  that CUE's physical Red Book track and ends its one-shot transport when the
  animation owner returns, matching F0719's replacement boundary without
  generated PCM or a loop. The C03 Game handoff now also admits the exact
  10x32x32 `G4099_SquareCoordinatesToMusicTrack` payload directly from the
  selected CHTWE/CHTWJ program (raw offsets 271144/271624, FNV-1a
  `3faffb70`), so a future movement consumer can query only the original
  map/x/y selector and cannot substitute a host coordinate map. M11 now
  runs F0743's nonzero-selector change gate and 100-update delay against the
  live F31 party map/x/y, then requests that same physical CUE track. Its
  source-tick duration follows the original CD-DA byte span so later selector
  changes do not remain blocked by a stale host stream. M11 now preserves a
  queued original CDDA span through F0740/F0738's pause/continue transition
  and excludes paused time from the source-duration counter; a following
  F0719 request resumes the stream device before replacing it. Utility UI
  execution and save transactions remain required. The Utility exit
  now admits only the selected retail C06_CEDT
  owner: F31E verifies `UTILE.EXP` (152387 bytes, FNV-1a `ff240e0c`) and
  F31J verifies `UTILJ.EXP` (152499 bytes, FNV-1a `bb3b47c2`). Its editor
  pixels and transactions are not represented by the C03 Game or PC34
  utility surfaces. Each C06 handoff now also validates its native Phar Lap
  P3 envelope and exposes only the real load image/entrypoint: UTILE has
  load image `0x200..0x25343`, EIP `0xfe00`; UTILJ has
  `0x200..0x253b3`, EIP `0xfeb0`. This is an executable boundary, not a
  decoded Utility UI. The first C06 menu now binds its original six-label
  P3 pool rather than the generic PC34 action list: English bytes are
  `LOAD CHAMPIONS`, `SAVE CHAMPIONS`, `MAKE NEW ADVENTURE`, `REVERT`,
  `UNDO`, `QUIT`; Japanese remains its original Shift-JIS byte pool until
  the Towns text rasterizer is decoded. The six C06 source-coordinate input
  boxes are now bound separately for F31E/F31J, including the original
  inclusive edges and `CEDT006.C` action ordinals; EGB rendering and all save
  transactions remain open. F31 M653 is now read as its real 768-byte
  `NOT_EXPANDED` raw font record (C695), rather than being misclassified as
  an IMG2 image. The F31E CEDT ASCII path now uses its separate five-pixel,
  six-pixel-advance baseline metrics from `CEDTTXT.C F7338_`; the remaining
  C06 screen composition, portrait placement, save transactions and Japanese
  `F0952_JAPANESE_Print` path must still be bound before Utility may be
  presented. The `.CMP` decoder now follows `PORTRAIT.C F7251`'s F31
  Atari-ST-planar conversion instead of a fabricated packed-nibble view, so
  its real 24-portrait corpus is ready for that placement work. 2026-08-06:
  a real F31 Game victory now activates
  `ENDING.ANM` through the retained F2275 interpreter. It uses the original
  Timer-A frames and TD/TR CUE dispatch, holds its last decoded frame when
  F0750 returns, and never chains back to `SWITCHTW.EXP` or a PC34 endgame
  screen. The real English cache regression traverses Switch, Game, C004,
  C002/C003, C017/F0128, then the complete 419-frame `ENDING.ANM` stream.
  The CLI scanner now reports every hash-verified CSB edition, including both
  F31 English and Japanese archive members, rather than implying that only
  the selected cache's `GRAPHICS.DAT`/`DUNGEON.DAT` pair was found. The
  profile inventory now continues after a fast-path match, so an Atari ST
  `.7z` or other authenticated platform package in the same data root remains
  visible instead of being hidden by a cached or loose CSB pair.
  The real-media F31 title regression holds the final title frame through the
  605th Timer-A tick and admits SWITCHTW only on tick 606, preventing a
  future 16 ms host-wake shortcut from making the original animation run too
  quickly.
  2026-08-06: PC 3.4 and Amiga 3.1 share one `GRAPHICS.DAT` identity, so the
  scanner now checks the paired, hash-verified A31E `TITL.DAT` receipt before
  publishing the PC row. An A31 disk can no longer be mislabeled as PC 3.4;
  a distinct shared-payload package is retained when one is actually present.
  2026-08-06: a normal broad data-root scan now also admits the named retail
  FM Towns archive below its documented `csb/` child, so the launcher retains
  both F31 English and Japanese profiles rather than hiding them unless that
  child is selected directly.

- **NEXUS-SFX-EVENT-DISPATCH-CAPTURE:** Host sound-request names are now
  explicitly documented as non-retail labels. Keep all `NEXUS_SFX_*` to MAP
  selector bindings fail-closed until the authentic SLEV/SDDRVS event
  consumer is captured; do not infer semantics from selector ordering.
  The real SLEV/SAL/MAP corpus regression now requires each level's
  production MD5 identities before its bounded records count.
  2026-08-06: real SNDLEV00 SAL/MAP bytes now have a regression proving that
  neither event playback nor the legacy sample-index API creates a host voice.
  Capture launchers now reject stock Mednafen before creating a manifest when
  the required Firestaff trace hook is absent; a normal movie/screenshot is not
  promoted to SLEV/SAL evidence.
  2026-08-06: all Nexus Mednafen launchers now use the real 1.32.1 Saturn
  setting `ss.bios_na_eu` for USA/Europe BIOS input; the former
  `ss.bios_us_path`/`ss.bios_eu_path` spellings could never start an
  instrumented stock-derived build. The Firestaff hook and event consumer
  capture remain open.
  The same launchers now reject raw ISO/BIN paths before writing a plan;
  Saturn CDDA capture requires the real CUE/CCD/TOC/M3U container route.
  2026-08-06: the CDDA status table and runtime regression now explicitly
  distinguish manual track selection from playback readiness; no level→track
  binding or ready receipt is emitted without source-owned evidence.
  2026-08-06 follow-up: the 16-level real SAL/MAP regression now also runs
  `nexus_sound_level_runtime_receipt()` after each authenticated pair load and
  proves that metadata consumption cannot promote SFX playback without the
  Saturn event-dispatch/SDDRVS route.
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
  DM1/CSB flow or a 24-champion roster as Nexus evidence. Bind the real
  startup/champion/menu order, text consumer and VDP1/VDP2 composition from
  an authenticated Saturn capture before adding menu transitions or screen
  coordinates.

  The MENU.BPK corpus regression now also requires an authenticated
  English/French retail archive identity before accepting its 163-entry
  directory census. Menu drawing, text ownership and VDP1/VDP2 composition
  remain capture-gated.

  2026-08-06: the bounded MENU.BPK receipt now accepts the verified
  canonical/English/French retail identities and derives the directory
  trailer's final two offsets from the active archive table. The local
  European corpus is 87,684 bytes; this corrects stale 89,060-byte probe
  expectations only. Menu drawing, text ownership and VDP1/VDP2 composition
  remain capture-gated.
  2026-08-06 test-harness follow-up: explicit `--real-only` CTest variants now
  bind `MENU.BPK` through `FIRESTAFF_NEXUS_DATA_DIR` and require the real
  archive before accepting the 162-surface receipt. The ordinary synthetic
  test remains data-free; no fixture archive can satisfy the real-data gate.
  2026-08-06 launcher follow-up: removed the remaining procedural Saturn-ring/
  obelisk card fallback. Nexus now shows only a capture-locked status in the
  M12 launcher until a real title framebuffer is bound. Missing Nexus media is
  also reported before presentation/runtime gates so the user receives the
  actionable ISO/BIN/CUE recovery path.

2026-08-06 Theron input follow-up: physical arrow keys arrive at M11 as
`STRAFE_LEFT/STRAFE_RIGHT`; the startup adapter now treats those tokens as
Soul Room focus movement. Keep the final dungeon handoff capture-gated.

2026-08-06 media-inventory follow-up: `TQJP19.iso` is Track 19
(`f9f069a5…`), not the JP Track 02 ISO (`TQJP02End.iso`, `397039af…`). Keep
the separate loaders and do not reuse Track 19 offsets for Track 02 palette,
level or consumer bindings.
  2026-08-06: a decoded real MENU.BPK route is now explicitly blocked until
  the admitted PALT memory, palette-state and VDP1-command capture exists;
  decoder success alone no longer reports a drawable menu.
  2026-08-06: the real DM.BIN startup/menu regression now verifies the
  adjacent `MENU.BPK`, `yam\\menu.c`, `FONT256.S2D` and `STABG.BIN` loader
  names at `0x373B4`–`0x373D8`, with pointer-reference counts 1/10/1/1.
  This is source ownership only; menu order, text consumer and VDP1/VDP2
  composition remain capture-gated.
  2026-08-06: the startup asset receipt now has an explicit
  `menu_text_consumer_bound` gate. Real engines leave it false because the
  current host chrome strings are not proven TEXT4/TABL/FONT012 Saturn output;
  only an explicit external capture seam may open the save/champion fixture
  route.
  The receipt now also counts the real DM.BIN register literals `0x25F00006`
  and `0x25F80000`; their consumer and VDP2 role remain unproven.
  Engine-init now retains the real RLOWFIX TEXT4/TABL/FONT012 source receipt,
  but the Saturn glyph consumer and placement are still not admitted. FONT256
  remains a separate champion/spell bank.
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
  verified 224-row Saturn surface from the DMWeb 64x28 TITLE.BIN map layout.
  The remaining work is still the captured VDP2 tilemap/CLUT owner and its
  runtime timing, not another host-sized 320x200 approximation. The public
  `nexus_render_title` host entry point is now no-draw while that capture is
  absent; authentic WARNING.BIN/TITLE.CG bytes remain diagnostic receipts.
  M11's separate startup executors are now no-draw for the same reason; timing
  receipts cannot authorize a direct source-to-host framebuffer copy.
  WARNING DGT2 M11 callback modules are also excluded from the production
  library and remain explicit probe/test sources only.
  2026-08-06: a separate title VDP capture-admission bit now gates the
  full-start title-capture receipt; retail TITLE.CG/TITLE.BIN bytes alone
  cannot make M11/M12 advertise a drawable title package.

- **NEXUS-DGN-MATERIAL-VDP1-CAPTURE:** The real Structure2 decoder now
  has a 16-level census guard (1,678 textures: 1,553 indexed-4bpp and 125
  direct-555) with non-zero output checks. Keep decoded pixels/palettes as
  format receipts only until Structure3 face ownership and Saturn VDP1 CLUT,
  upload and command-order evidence are joined.
  2026-08-06: the real LEV corpus census now uses the same bounded Structure3
  mesh-extraction readiness receipt as the active viewport path. It reports
  all 16 retail levels as geometry-ready without treating that as transform,
  palette, VDP1 or drawable-frame evidence; the focused corpus probe now exits
  successfully with material presentation still capture-gated.
  2026-08-06 follow-up: the CPU textured rasterizer is now excluded from
  `firestaff_nexus`; production links a lifecycle-safe no-op adapter and the
  material raster test compiles the fixture explicitly. Keep the real VDP1
  command/CLUT/VRAM and DGN/MNS owner capture open.

- **NEXUS-STONE-PP-VDP1-BINDING:** The missing image-local `STONE.BIN`
  decoder is now implemented from DMWeb `DecodeRawPPpp`: all eight retail
  32×32 records, 16-entry big-endian BGR555 palettes and 512 packed texel
  bytes are validated and a selected record can be decoded to caller-owned
  buffers. The decoder does not create a global palette or authorize drawing;
  bind each record to the DGN/VDP1 material consumer before viewport use.

- **NEXUS-HUD-SATURN-DISPATCH-CAPTURE:** The real DM.BIN hit-rectangle
  parser now rejects both inverted/out-of-range rectangles and signed
  negative origins. The HUD layout and hit-rectangle regressions now require
  the authenticated retail DM.BIN before accepting their 80/40-entry census.
  Remaining work is still the Saturn input/VDP consumer capture that binds
  each admitted region to its command semantics.
  M11 no longer derives `startup_hud_ready` from a DGN viewport handoff;
  2026-08-06: the HUD layout regression now proves the `yam\\menuctrl.c`
  owner string immediately before `DM.BIN+0x376D0`, the exact 80-entry table
  FNV-1a64, and seven executable/static references to runtime address
  `0x060476D0`. This strengthens source ownership only; VDP1/VDP2 and event
  semantics remain capture-gated.
  2026-08-06: the same regression binds `yam\\vdp2.c` at DM.BIN `0x38CF4`
  with six literal references. This is source ownership evidence only;
  register consumer, layer selection and runtime placement remain gated.
  retain the separate HUD capture requirement.
  The former synthetic `test_m11_nexus_startup_runtime_handoff` fixture has
  been re-authored to assert the current no-draw contract: without a captured
  Saturn startup package it must not promote title/save/champion or DGN host
  draws. Exact Saturn menu timing, input dispatch, and VDP1/VDP2 composition
  remain open capture work.
  2026-08-06 follow-up: removed the remaining hardcoded HUD depth limit from
  the M11 handoff; diagnostics now use the source-defined `NEXUS_MAX_LEVELS`
  (16). This changes no production pixels while the Saturn HUD route is closed.
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
  records are now admitted independently of the legacy 24-byte fixture
  grammar, and the real 16-level corpus remains bounded and source-backed.
  Keep DataID/selector entries opaque until Saturn SLEV/SDDRVS event
  dispatch is captured; do not bind the host sound enum or start playback
  from inferred selectors.

  Host-side SFX requests remain numeric in production diagnostics. Former
  labels such as `DOOR_OPEN` and `SPELL_CAST` were not retail MAP facts and
  must not be presented as such.

  2026-08-06: ITEM.IBS inventory helpers no longer guess armor slots from
  inherited DM1 item IDs; the remaining equipment mutation still requires the
  authentic Saturn action/slot dispatcher capture.
  2026-08-06: the real floor-image renderer now follows DMWeb's
  `palette_offset == 0` reuse rule per palette ID; the 75 retail descriptors
  that reuse an earlier palette no longer fail as if their source palette
  were missing. Item action, pickup and VDP1 command ownership remain gated.
  2026-08-06 follow-up: live floor admission now keeps the DGN Structure1Fa
  item declaration number instead of misusing ITEM.IBS word 20's inventory
  image association as a runtime ID. Raw Structure1Fa attribute bytes 5 and
  7, including LEV01 torch/waterskin charge values, are retained with the
  floor provenance; they are not interpreted before Saturn action capture.
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
  now hash-admitted for both `DATA`/English and `JDATA`/Japanese, and the
  runtime cache preserves `EDM.EXP`, `JDM.EXP`, `TMENU.EXP`, `TMENU.ICN`, the
  selected language data and the original system siblings. A native startup
  receipt now verifies the exact HMA-240 `AUTOEXEC.BAT`, selected `EDM/JDM`
  Phar Lap P3 owner, `TMENU.EXP`, `TMENU.ICN` and `TMENU.INF` hashes before
  the cache is accepted; its CD title/hall/entrance track ownership is also
  recorded. The receipt now additionally requires TMENU's real TownsOS
  file-browser bindings, the language-specific EDM/JDM selection in TMENU.INF,
  and the game executable's original title/menu/dungeon/CD owner symbols. The
  bounded Phar Lap P3 header gate also records the real load-image offset/size,
  symbol-table bounds and initial EIP for EDM/JDM and TMENU. For the English
  EDM.EXP, the receipt now parses the real `SYM1` table (1,174 entries) and
  records the original entry addresses for `DO_TITLE_ANIMATION`,
  `TITLE_PRESENTS`, `TITLE_DUNGEON`, `DRAW_DMENU`, `DYNAMENU`, `MENU_ICONS`
  and `CD_LEVEL_SONG`; Japanese JDM remains accepted without a symbol table
  because its verified P3 image has none.
  2026-08-06: the English EDM P3 receipt now also binds the executable's
  native title plan: GRAPHICS.DAT graphic 1, PRESENTS source y=137,
  MASTER source y=80, destination rectangles y=90..105 and y=118..174,
  the 320x80 swoosh/zoom source region and the 18-step 16x4 prepared-bitmap
  loop.
  These values are checked at the original load-image offsets and are not
  copied from the PC34 title frontend.
  2026-08-06: the English runtime now consumes this receipt after the
  selected FM Towns directory binds legacy GRAPHICS.DAT. It presents the
  authentic graphic-1 PRESENTS-only frame, source-owned 18-step zoom and
  MASTER frame through the M11 framebuffer at the executable's VBlank
  cadence, and fails closed instead of falling
  back to PC34 when the receipt is absent. Japanese JDM remains blocked from
  this path because its pixel/TBIOS plan is not yet decoded. The native
  `DRAW_DMENU`/`DYNAMENU` menu and FM CD-audio handoff remain open.
  2026-08-06: the authenticated English EDM load image now also supplies the
  exact 44-entry native action-label stream at `DYNAMENU+8` (including the
  duplicate `STAB` and `X` entries). M11 consumes those receipt-owned labels
  for an English FM Towns session; it no longer leaks the generic PC34 table
  into the FM action rows. This closes label ownership only, not native
  TMENU/DYNAMENU mouse/input or pixel/TBIOS rendering.
  2026-08-06: `dm1_v1_fmtowns_title` now composes the real GRAPHICS.DAT
  graphic-1 title frames from the EDM.EXP receipt: PRESENTS, the native
  18-frame reverse zoom (48x12 through 320x80) and TITLE_MASTER. M11 now
  consumes that same compositor in its actual title loop, so it presents the
  source order instead of a separate 320x80-to-48x12 approximation. The
  retained original CUE/BIN handoff now starts EDM's authenticated CDDA track
  02 before the first title frame, then the gameplay map dispatcher owns the
  later transition. `TMENU.INF` is now parsed as its two real 128-byte
  TownsOS launch records (`\\JDM.EXP`, `\\EDM.EXP`) instead of accepting any
  matching text in the configuration. Its icon bitmap format, visible layout
  and mouse input routes remain capture-gated. Exact wall-clock timing and
  TMENU input capture remain open. M11 routes a selected FM Towns edition around the PC34
  `SWSH -> TITLE -> ENTRANCE` transaction. It opens only the selected
  hash-verified Towns data; do not restore the PC34 path as a presentation
  fallback.
  2026-08-06: FM Towns CD-audio is now wired end-to-end through the M11
  runtime: title (track 2) dispatches from the launcher just before the
  FM Towns title animation, Hall of Champions (track 3) dispatches when
  the HoC presented-capture receipt first goes ready, and the entrance
  micro-dungeon (track 5) dispatches at the redmcsb entrance transition.
  Map transitions (stairs and teleporters), per-tick idle re-arming, the
  game-over/game-won events and the music toggle also invoke the CDDA
  dispatcher. All dispatch is gated on `dm1FmtownsStartupReceiptValid`
  and reads from the retained BIN/CUE saved into the runtime cache
  during materialization; playback is by real byte offset in the mixed
  MODE1/2048 + AUDIO/2352 sector layout of the original disc. No PC34
  synthesis. The `DRAW_DMENU`/`DYNAMENU` menu rendering itself remains
  open (requires Phar Lap P3 disassembly at the recorded entry points).
  2026-08-06: the recovered `DRAW_DMENU` backdrop sequence is now bound to
  the source-locked EGB shim: `FILL_CSCREEN` clears the exact region-11
  87x45 menu rectangle and `SPC_BLOT` paints it with the authenticated
  DYNAMENU colour selector. Native icon bitmap decode, text rasterisation,
  mouse/input capture and timing remain open.
  2026-08-06 follow-up: the live English FM Towns action-menu path now
  materializes the three source action indices into the ephemeral DYNAMENU
  record and presents only this EGB-owned backdrop. It deliberately leaves
  the panel text and icons blank until `DO_DRAW_CTEXT` and the icon consumer
  are decoded, rather than leaking the PC34 action chrome or M653 glyphs.
  2026-08-06 follow-up: the hash-admitted Japanese JDM path now consumes the
  independently recovered `DRAW_DMENU` and `DYNAMENU` owners for that same
  EGB clear/panel sequence. It remains blank: the recovered Shift-JIS label
  pool establishes layout ownership only and is not permission to synthesize
  text, icons or input.
  2026-08-07 English text closed: `dm1_v1_fmtowns_font_rasteriser` (round-
  trip verified against real EDM.EXP asset 557 — 6 rows x 128 ASCII glyphs,
  right-aligned 5-bit body, MSB first), `dm1_v1_fmtowns_menu_render`
  harness with `dm1_v1_fmtowns_menu_default_glyph_draw_pc34`, and
  `M11_GameView_RenderDm1FmtownsMenu` compose the full pipeline. The
  M11 action-menu path now paints DYNAMENU text into the source-owned
  EGB backdrop for English sessions. Japanese Shift-JIS glyphs remain
  blank pending a JDM-owned font decode; native icon bitmap decode
  remains open.
  2026-08-08: FM Towns action menu click handling closed. The M11 input
  handler now hit-tests clicks against the source-locked 87×45 panel at
  (232,77)–(318,121) with three 7-pixel-tall rows (CHAR_Y_HYT=7),
  dispatching to `M11_GameView_TriggerActionRow`. Disabled slots (0xFF)
  are respected. The PC34 route-table path is skipped when FM Towns is
  active. Test: `test_dm1_v1_fmtowns_menu_click_geometry`.
- **DM1-PLATFORM-ATARI-ST-PIXELS:** DM1 Atari ST 1.0a/1.0b/1.1/1.2/1.3
  graphics hashes are now catalogued and discovered from STX/archives, but
  `dm1_v1_atari_st_graphics_dat` now validates and reads the real DMCSB1
  big-endian 563-record table and Atari-LZW/raw record handoff. The production
  M11 renderer now has a source-bound Atari IMG1 decode API after that
  handoff; the shared decoder uses the same original nibble stream as the
  FM Towns/Amiga path. Keep these variants launch-blocked for gameplay rather
  than routing their bytes through the PC34 loader until the decoded record is
  joined to the Atari dungeon/runtime owner. 2026-08-06: the
  endian-aware legacy path now uses the DMWeb bitmap index table and decodes
  all 532 real DM1 FM Towns IMG2 and Amiga IMG1 records from local corpora;
  non-raster code, sound, text, font and unused records fail closed before
  M11 caching. Their remaining dungeon/save/input/media capture gaps stay
  separate. Atari still needs the extracted GRAPHICS.DAT/DUNGEON.DAT pair to
  join the Atari dungeon/runtime owner and validation of more STX variants.
  2026-08-06: a clean-room
  `dm1_v1_atari_st_stx` reader now validates the retail RSY v3 track blocks,
  orders real 512-byte sectors, reads the DM1 FAT12 directory and extracts
  GRAPHICS.DAT/DUNGEON.DAT from the supplied STX. The hash-first asset
  pipeline now also materializes all six catalogued retail STX identities
  from a selected file or supported archive into a source-tagged bundle.
  Remaining work is joining that extracted pair to the Atari dungeon/runtime
  owner and validating more protected STX variants; the PC34 runtime still
  rejects the non-PC34 bundle rather than guessing.

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

- **THERON-V1-MAIN-RAM-CONSUMER-TRACE:** the current Mednafen 1.32.1
  capture build now emits a bounded, line-delimited sidecar for reads from
  game-owned main RAM when the reader PC is also in game RAM, including logical
  and physical addresses, byte value, and reader-PC provenance. This is still
  opaque execution evidence: no level, object, tile, palette, or bitmap meaning
  is assigned. A real capture now also verifies the executed HuC6280 window
  `$2c54–$2c69` byte-for-byte from code-fetch coordinates; the disassembly
  receipt is retained separately from semantic publication. The receipt test
  now auto-discovers a valid real loader sidecar
  under `.firestaff/firestaff-probe-screenshots` when no override is set.
  Remaining: run the consumer-read trace against an authentic Track 02 CUE with
  a real SDL2 runtime and join the observed reads to source-LBA/FIFO receipts.
  **2026-08-06 FIFO-origin capture wiring:** the coherent 1.32.1 capture build
  now has a valid FIFO-origin extension. It queues the exact raw user-data
  `(LBA, sector offset, FIFO sequence)` for each authenticated CD transfer and
  carries it into the CD-to-RAM receipt; the Firestaff CD sidecar parser
  validates the source LBA against an authenticated SCSI-read range and keeps
  semantic publication blocked. The existing capture has not been retroactively
  promoted: it must be rerun with this instrumented binary before any `$2600`,
  object, level, tile, palette, HUD, or viewport meaning is admitted.
  **2026-08-06 fresh capture result:** the rebuilt real-SDL2 binary was run
  against authenticated US Track 02 (`BIN MD5 f23601102138f87c33025877767ebf76`)
  and produced 161 raw sector spans, 51 SCSI reads, 25 CD IRQs and 4,096
  main-RAM consumer reads. The six `$e009` windows still contain zero CD data
  reads; the two FIFO-origin rows are BIOS/CD-routine reads at `$21e7`/`$21e9`,
  not game-owned `$2600`. A second held replay and an eight-event Cocoa/Quartz
  host-input capture reproduce the same absence. Keep the `$2600`/object/level/
  tile/palette/HUD/viewport gates closed. The capture script now accepts
  `run` as an alias for its physical `return`/Run key.
  2026-08-08 parser hardening: the loader-sidecar reader now normalizes the
  literal `\\n` separators emitted by the early probe writer before parsing.
  The original file bytes remain the MD5 identity, and the regression accepts
  the local two-transfer capture without opening semantic publication.
  2026-08-06 static-listing update: the checked-in bank-$1f disassembly now
  includes the authenticated `$2386–$23a3` output-size caller and
  `$23ad–$243d` resource framing/variable-bit entry. This strengthens the
  source contract only; the live `$2600` RAM consumer and its level/object
  destination remain unbound.
  2026-08-08 transport follow-up: the Main-RAM consumer parser and its
  executed-code-window verifier now accept the same literal `\\n` sidecar
  separators as the loader parser. This changes only ingest normalization;
  the `$2600` consumer and all level/object/stat semantics remain closed.

- **THERON-V1-VIEWPORT-REAL-DATA:** the old inferred viewport renderer is now
  explicitly fixture-only and excluded from the production archive. Replace
  its no-op seam only after one original Track 02 capture binds the actual
  square-to-tile/map consumer, tile-bank bytes, VCE palette route, UI chrome,
  and viewport destination together. The existing level grid and font bytes
  are not sufficient evidence; do not revive the fixture tile table or its
  fallback geometry.

- **DM2-PLATFORM-MEDIA:** Keep the DOS, FM Towns and Amiga boot paths
  identity-first and in-memory for container media. FM Towns HME-242 now
  admits its real CUE/IMG payload without disk extraction, and loose verified
  install directories take priority over duplicate archive entries. FM Towns
  boot and GAME_LOAD both retain their authenticated G1 bytes in memory. The
  supplied Amiga archive is an installer corpus: Disk 1 holds only installer
  scripts and `dm2_arcsplit1`, not launch files. The original six-part
  `DM2_archive.LZX` index is now joined, bounded and decoded in memory through
  the real outer-ZIP → disk-ZIP → ADF chain, including the real
  `GRAPHICS.DAT`, `DUNGEON.DAT`, `CD.DAT` and MOD receipts; the decoded entries
  retain their LZX CRC receipt. M12 and the boot profile now run the same
  original MD5 pair gate over those RAM buffers, and an explicit Amiga archive
  selection wins over a sibling PC install. M12 now admits the FM Towns CD ZIP
  only after it reads the raw image in memory and verifies its original
  GRAPHICS.DAT/DUNGEON.DAT pair; the same identity-first requirement now holds
  for Amiga.
  **2026-08-06 FM Towns stream-count update:** M11 now keeps each active
  TWANIM member's frame limit from its authenticated `EN + DL` receipt while
  it remains in RAM. The source HME-242 streams, rather than hard-coded
  retail totals, therefore decide when SWOOSH may hand off to TITLE and when
  TITLE may hand off to SKULL.
  Generic DM2 ZIP/ISO cache materialization is deliberately disabled: it must
  either gain a verified in-memory PC reader or remain non-launchable. Do not
  synthesize a compatible install, palette, dungeon, music map or save corpus
  for any platform.
  **2026-08-06 update:** a directly selected original FM Towns ZIP now stays
  the exact M12-to-boot handoff, just like Amiga. Its CUE/IMG, G1 payload and
  CD.DAT map are read only in RAM; a sibling DOS install cannot silently win
  platform selection. The real archive passes M12, boot and `GAME_LOAD`.
  The former runtime-side implicit `$HOME/.firestaff/data/dm2/GRAPHICS.DAT`
  English overlay was removed as well: an FM Towns session now uses only the
  selected verified corpus. A future language option must carry an explicit,
  separately authenticated companion corpus in its M12-to-boot receipt; it
  must not discover a sibling PC installation by path.
  **2026-08-07 verification update:** the focused PC-English i18n regression
  now receives that corpus only through `FIRESTAFF_DM2_DATA_DIR`; a selected
  but unreadable `graphics.dat` is a failure, never an implicit HOME-path
  skip. Keep the remaining non-English overlays explicit as well.
  **2026-08-07 G1 verification update:** direct G1 root receipt regressions
  now receive `dungeon.dat` only from an explicit argument or selected
  `FIRESTAFF_DM2_DATA_DIR`; a selected unreadable corpus fails rather than
  using an implicit HOME-path fallback.
  **2026-08-07 GDAT verification update:** the broad creature-animation,
  visual-census, GRAPHICSSET, scene-plan and wall-plan probes now likewise
  require the selected corpus and fail an unreadable explicit selection.
  **2026-08-08 archive-owner repair:** archive-backed `GRAPHICS.DAT` and
  `DUNGEON.DAT` provenance no longer yields a fictitious
  `archive::member-parent` runtime root. The selected outer archive remains
  the boot owner, while all member bytes continue to be read in RAM.
  **2026-08-07 corpus-selection follow-up:** the boot-profile hash-renaming
  and live weather-frame regressions now also take real PC-DOS data only from
  `FIRESTAFF_DM2_DATA_DIR`. They skip only when it is unset; an explicitly
  selected root without the original pair is a test failure rather than a
  hidden `HOME`-directory fallback.
  The M11 startup-profile gate now has the same rule across its explicit
  `FIRESTAFF_DM2_V1_DATA_DIR`, `FIRESTAFF_DM2_CANONICAL_DIR` and public
  `FIRESTAFF_DM2_DATA_DIR` inputs. It cannot borrow a separate installation
  from `HOME` before checking M12-to-M11 ownership.
  The real SDL sound-playback regression also requires
  `FIRESTAFF_DM2_DATA_DIR`; it decodes source GDAT audio only from the chosen
  `graphics.dat`, and reports an invalid selection rather than probing HOME.
  **2026-08-07 G1 material update:** DB4 scene/viewport and DB5/DB9 map-chip
  regressions now use only an explicit or selected corpus. Missing exact F9
  material remains fail-closed; do not substitute a neighbouring GDAT row.
  **2026-08-07 Amiga corpus update:** the six-disk LZX regression now accepts
  only an explicit `FIRESTAFF_DM2_AMIGA_ARCHIVE`. The supplied original archive
  verifies all six ADF parts and decodes the original GRAPHICS, DUNGEON and
  CD data entirely in RAM. A selected unreadable archive fails; no HOME-path
  discovery is retained.
  **2026-08-06 update:** the generic DM2 boot reader also now rejects every
  virtual archive path instead of extracting it to `/tmp`. FM Towns and Amiga
  continue through their already authenticated in-memory buffers. PC archive
  media remains non-launchable until it has the same in-memory owner.
  **2026-08-08 selection repair:** a loose matching FM Towns GDAT no longer
  masks the original HME-242 ZIP when both are in one data root. The archive
  is selected because it is the only source owner for the CUE/IMG, animation
  streams and CDDA; no member may be materialized as a workaround.
  The callback-only `dm2_v1_anim_bootstrap` file-reader is now test-only: no
  M11/DM2 runtime consumer has authenticated an original animation stream.
  **2026-08-06 update:** HME-242's root boot corpus is now also required at
  FM Towns admission. The in-memory ISO receipt binds `AUTOEXEC.BAT`,
  `SWOOSH`, `TITLE`, `TWANIM.EXP`, `SKULL.EXP` and `END`; it decodes the
  original `SWOOSH -> TITLE -> SKULL -> END` order before any Towns session
  is admitted. The real-CD M12 regression covers the route without unpacking
  a member. `SWOOSH`, `TITLE` and `END` are also checked against the published
  HME-242 retail MD5 identities before their startup plan is accepted. This is
  an authenticity gate, not an animation renderer.
  **2026-08-06 update:** The selected original SKULL.EXP must now match the
  authenticated HME-242 MD5 as well as pass its bounded Phar Lap P3 header
  receipt in RAM, including the declared load image, relocation range and
  entry point, before the startup media is retained. This remains an
  executable identity gate only: native P3/TBIOS execution and its own input
  loop still need capture. M11's separately authenticated GDAT-v4 IMG2 menu
  presentation is documented in
  `parity-evidence/dm2_fmtowns_startup_p3_gdat_boundary.md`; it is not a
  claim that the P3 program itself executes.
  **2026-08-06 TWANIM update:** the player for the visible SWOOSH/TITLE/END
  stages now has the same in-memory P3 ownership requirement. The selected
  HME-242 TWANIM.EXP must match its retail MD5 and bounded Phar Lap header
  (entry 0x10470, load image 0x117f8) before the source streams are admitted.
  This verifies the native player identity, not its unported sound mixer or
  a native SKULL-menu handoff.
  **2026-08-06 update:** production now also parses each selected stream in
  RAM using DMWeb's big-endian record framing and requires the complete
  HME-242 inventories: SWOOSH 22 records (18 DL), TITLE 235 (224 DL, one SD,
  five SO) and END 401 (two AN phases, 382 DL). These are bounded source
  receipts, not decoded frames.
  **2026-08-06 update:** the source-owned IMG1 decoder now replays TITLE's
  EN/DL records from the same RAM-owned stream and validates first/final
  retail 320x200 frames. It follows SKWIN `ANIM_DECODE_IMG1`, including its
  contiguous-file read at a record boundary. **2026-08-06 update:** M11 now
  binds the selected, authenticated TITLE member in RAM, decodes its native
  PL record, presents the indexed 320x200 canvas and advances EN/DL frames
  with the SKWIN TWANIM Timer-A quantum and source durations (minimum five
  ticks). It blocks SKULL-menu input until TITLE ends and fails closed rather
  than drawing the PC static GDAT menu when this binding is rejected. The
  `decode_title_sound` now retains TITLE's one real signed SND2 sample and
  all five SO events in RAM, including source offsets, frame positions,
  volume bytes and the source-invalid 1000 Hz field. SKWIN `0759:0E33/0EF0`
  establishes the selected slot and fixed 5500 Hz playback argument; M11 now
  transports that exact signed sample at each source frame boundary. The SO
  payload is not interpreted as host stereo or gain because the original
  call passes `0xff` and 5500 instead. END's source FO/NE/BN loops and per-frame PL
  palette route now replay through RAM-only receipts (420 displayed frames
  from 385 EN/DL records). The selected SKULL Quit event now binds that END
  stream and returns to the launcher only after its final frame, completing
  AUTOEXEC's exit transition. The remaining gaps are native P3 keyboard
  input, GAME_LOAD and save/resume; the real SKULL.EXP table maps the source
  menu cue 0 to silence, so TITLE has no missing CDDA track to substitute.
  Do not replace those missing routes with host animation, menus or saves.
  **2026-08-06 IMG2 menu update:** after the authenticated TITLE
  stream completes, M11 now presents the selected HME-242
  `TITLE/0/dtImage+dtPalIRGB/4` surface through the FM Towns GDAT-v4 IMG2
  decoder and its local 16-colour palette. Its real `dt04/0` NEW GAME
  rectangle now reaches SKWIN's `0xD7` load boundary only after the verified
  TITLE→SKULL handoff; TITLE/SWOOSH pointer events remain inert. The adjacent
  real `0xD9` RESUME rectangle is also exercised against the selected disc
  and stays inert without a boot-admitted original save. Keyboard, RESUME save
  execution and the native SKULL continuation are still unbound,
  and `0xD7` deliberately cannot create a synthetic party; unverified media
  still fails black. Credits and Quit now use their selected GDAT rectangles;
  the credits dismissal accepts its source event from either mouse button.
  **2026-08-06
  update:** M11 now runs the selected HME-242 SWOOSH
  stream before TITLE, inferring only its documented EN-owned 320x200 canvas
  when AN says 0x0. Both streams remain RAM-only and Timer-A paced.
  **2026-08-06 update:** a shared data root can now retain all three verified
  DM2 editions at once. At M12→M11 launch, the selected DOS, FM Towns or
  Amiga version resolves its own matched owner instead of inheriting the
  scanner's first match. FM Towns and Amiga therefore pass their original ZIP
  path to the existing RAM-only boot owners; DOS passes the directory holding
  its authenticated loose pair. The real-media regressions cover both direct
  archive and mixed-root scans. This is selection identity only, not a claim
  of complete platform gameplay parity. **2026-08-07 CUE ownership update:**
  FM Towns scanning and DM2 boot now read the image member named by the
  original CUE `FILE` declaration, rather than taking the first ZIP member
  ending in `.img`; native P3 keyboard input, GAME_LOAD and save/resume remain
  separate capture-gated work.
  **2026-08-06 lifetime update:** M11 shutdown now releases the selected
  FM Towns TITLE/SWOOSH RAM member before its boot profile releases the
  in-memory HME-242 image. Relaunch cannot retain a stale animation buffer;
  no member is materialised on disk.
  **2026-08-06 English companion path update:** an explicit PC-English
  companion now accepts the original DOS ZIP member spelling
  `DATA/GRAPHICS.DAT` as well as the documented lower-case virtual spelling.
  Both forms still resolve only in RAM and require the canonical PC-English
  content hash before their text overlay can be bound to the Japanese CD.
  The real-media regression also follows M12's scanned DOS-ZIP provenance
  through that companion handoff, rather than relying only on a caller-built
  virtual path.
  A loose companion is now checked again after its bounded RAM read, so the
  GDAT reader consumes the same canonical PC-English bytes that passed the
  hash gate.
  **2026-08-06 FM Towns credits update:** `DM2_SHOW_CREDITS` now takes
  precedence over the completed TITLE-to-SKULL menu handoff in M11, matching
  the source's separate event loop. Its HME-242 TITLE/0/1 IMG3 page selects
  the page's own verified 16-colour `dtPalIRGB` data rather than retaining
  TITLE/0/4's menu palette. The real-CD M11 regression compares all active
  indexed RGB6 rows with that original palette. This remains presentation
  only: credits neither opens gameplay nor grants a save/resume path.
  **2026-08-13 palette-index correction:** the TITLE/0/1 local `dtPalIRGB`
  rows are now installed at the image's own `dtPalette16` physical indices,
  matching `DM2_DRAW_PICST` before `R_C470` selects the palette. The former
  raw-nibble placement produced incorrect FM Towns credit colours whenever
  the source image remapped a nibble. The real HME-242 M11 regression now
  verifies the complete physical RGB6 palette map.
  **2026-08-08 keyboard update:** Enter is now admitted only through
  `SKULLWIN/v1d39bc.dat`'s first title-menu binding (`0x001c` → UI event 215 /
  raw event `0xD7`). It reaches the existing GAME_LOAD boundary without a
  synthetic party. Arrow, action and back tokens remain blocked until their
  corresponding source title-menu bindings and event ownership are ported.
  **2026-08-06 FM Towns GDAT gate update:** the bounded format classifier now
  also requires HME-242's actual 3,407-entry raw catalogue. A made-up
  0x8004 header and plausible buffer size therefore cannot enter even the
  format receipt; the boot profile retains the stricter full-media identity
  gate.

- **DM2-RESURRECTION-OWNERSHIP:** Production type-0x0D resurrection remains
  deliberately non-mutating until the original 263-byte `c_hero` layout,
  its 16-bit hero/item fields, the phase-1 tombstone record chain, and the
  phase-2 `CREATE_CLOUD` owner are imported together. The current 261-byte
  session persistence surrogate cannot stand in for those structures. Do not
  revive a champion, clear inventory, or fabricate the cloud from a partial
  save/session receipt. Source: `SKULLWIN/c_tim_proc.cpp:39-124` and
  `SKULLWIN/c_hero.cpp::DM2_BRING_CHAMPION_TO_LIFE`.
  The same ownership gate covers type-0x0C: its source writes 16-bit
  `c_hero::timeridx` and flag `0x0800`, neither of which can be represented
  by the session surrogate's byte fields. It also covers spell timer types
  0x47, 0x48 and 0x4B: their source `c_party::hero[]` writes target 16-bit
  `heroflag` (0x4000), `ench_power`, `poisoned` and `poison`. They must not
  use the surrogate's unrelated byte flags, `body_flag`, poison value or
  detached counters.
  **2026-08-08 isolation update:** the callback-only champion-lifecycle
  contract is no longer linked into `firestaff_dm2`. It remains available to
  its focused source-contract test, but cannot present an arbitrary callback
  set as a live GAME_LOAD champion path. Bind the File_header mirror chain,
  real `c_hero`, possessions and timer owners before restoring it to runtime.
  **2026-08-07 real-corpus census:** all eight supplied PC-DOS
  `sksave0..3.dat/.bak` files decode to zero source `c_tim` records with
  `ttype == 0x0D` at the SKProject `c_timer.h` offset `0x04`. The census is
  read-only evidence, not permission to synthesize a resurrection timer or
  promote the test-only phase callbacks; the complete owner remains open.

- **NEXUS-SATURN-PRESENTATION-HANDOFF:** Nexus production no longer contains
  the old inferred master palette or partial-texture fallback. Continue from
  the real `/Users/bosse/.firestaff/data/nexus` corpus: authenticate PRS3
  pixel/palette semantics, DGN/MNS face-material ownership, Saturn VDP1/VDP2
  placement, and runtime HUD/menu binding through an instrumented Saturn or
  Mednafen capture. Keep FACE/MENU/STABG/viewport presentation blocked until
  the capture binds bytes, palette, command order, and runtime state together;
  do not replace the missing capture artifact with synthetic data.
  2026-08-06: the real Nexus corpus also contains the complete merged English
  cue/ISO with Track 1 plus eight audio tracks. Mednafen identifies it as
  `T-9111G`/`DUNGEON MASTER NEXUS` and reaches the Saturn module; capture is
  now runs with the supplied European 1.00 BIOS (SHA-256
  `96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f`) and
  reaches the Saturn module. Remaining work is the instrumented VDP1/VDP2,
  CRAM and runtime-state capture; stock Mednafen output alone is not enough
  to promote pixels or command ownership.
  2026-08-05: bounded DMWeb PRS3 byte decoding is verified against all 20
  real FACE.BIN frames; this only advances diagnostics. Startup FACE, MENU,
  STABG, HUD, and viewport presentation remain blocked pending Saturn capture.
  ITEM.IBS diagnostics now also honor the external data-root environment and
  verify the real 243/223/109 item-image corpus; item draw authorization still
  requires the missing original Saturn VDP1 command receipt.
  The live inventory-use route no longer runs the inherited DM1 consumable /
  equipment catalog when ITEM.IBS action semantics are absent; click dispatch
  remains available, but use and equipment-to-inventory mutation are no-op
  until the Saturn action dispatcher is source-bound. Floor-item records remain
  provenance-only until the same dispatcher proves pickup/slot semantics.
  Startup state/input and DM.BIN champion-panel geometry are source-bound host
  handoffs only; their compatibility hit regions must not be described as
  retail Saturn screen coordinates. Unsupported Japanese double-byte text
  remains unavailable until a real JIS table and consumer are authenticated.
  The mechanics tick now applies the same gate to floor pickup, DROP_ITEM and
  THROW; no live inventory/floor mutation occurs from unproven ITEM/action
  semantics.
  Gold-pile pickup is also held behind the same Saturn action/drop gate;
  2026-08-06: runtime screenshot readiness now reports `BLOCKED_CAPTURE` for
  a valid real launch with valid black no-draw BMPs, instead of mislabeling
  the authenticated absence of Saturn VDP1/VDP2 presentation as a runtime
  failure. This remains a blocker and does not promote a screenshot.
  The configured Nexus data root is now also passed into the sound engine;
  CD-DA candidate paths no longer fall back to a HOME-relative placeholder.
  SLEV00-15 real task-entry profiles and SAL/MAP metadata remain receipts only
  until the Saturn event/audio consumer is captured. 2026-08-06: the
  provenance route now follows the real DMWeb retail MAP layout (8-byte rows
  from offset 0, terminal `FF FF`) across all sixteen MAP files; this fixes
  the old 24-byte fixture-prefix assumption without permitting playback.
  generic storage remains diagnostic and no retail drop record is fabricated.
  MNS mesh/skeleton/texture decoding now rejects truncated declared joint
  tables and guards TEXT/mesh/MOTN/pixel ranges; keep creature rendering
  blocked until the parsed model is joined to an authenticated Saturn VDP1
  texture/palette command receipt.
  The Nexus mechanics and engine tick paths now share an explicit closed
  action-semantics gate: inherited melee, spell dispatch, creature attack,
  ranged projectile launch, and projectile damage cannot mutate live state
  until Saturn command/RNG/stat/side-effect evidence is bound. Isolated
  combat/spell module tests remain diagnostic only.
  SMAP00-15 are now loaded and decoded from hash-verified retail bytes when a
  level becomes active; the retained pixels remain HUD/VDP2 no-draw until the
  Saturn placement and explored-state write path are captured.
  The public host automap renderer no longer paints a guessed gray/green grid
  or party marker; its compatibility config and draw call remain inert until
  the authentic SMAP/VDP2 consumer is bound.
  V1 movement no longer invents adjacent-level same-coordinate stairs or
  writes host radius-reveals into the retail automap state; source-owned
  Structure1F destinations and Saturn explored-state evidence remain needed.
  The public square-event dispatcher and mechanics transition branch now also
  fail closed for an unregistered stair instead of retaining the former
  same-coordinate level +/- 1 fallback.
  Pit/chute destinations likewise require an authenticated Structure1F target;
  the old implicit same-coordinate next-level route is removed.
  Production HUD no longer injects fabricated script/fountain/save strings;
  bind these messages only through authenticated DMN text and SLEV consumers.
  The procedural V2 HUD overlay (compass, depth, bars, icons and gold) is now
  no-draw even when its diagnostic runtime is force-enabled; retain only state
  for inspection until the authentic Saturn HUD/VDP1/VDP2 owner is captured.
  MENU.BPK English/French ISO revisions are now recognized as real alternate
  identities for bounded inspection; do not treat their opaque mode tags as
  host RGB formats or promote them to PRS3/VDP1 output without capture proof.
  DGN Structure2 texture decode is format evidence only; keep retail source
  verification on the hash-bound LEV00--LEV15 route and do not promote decoded
  pixels to viewport/VDP1 without Saturn capture proof.
  Creature MNS admission now also requires the exact retail hash identity;
  do not reintroduce renamed/synthetic DMDF signature fallback into the model
  pool while VDP1 model command semantics remain capture-gated.
  Startup UI surface wrappers are now no-draw seams; retain TITLE/ WARNING/
  GAMEOVER/FACE/STABG source bytes only as receipts until their Saturn VDP1/
  VDP2 placement and palette-bank ownership are captured.
  HUD layout and ring-menu hit rectangles are now bound into the live engine
  from verified DM.BIN; connect them to Saturn input/text/VDP consumers only
  after the corresponding runtime dispatch and placement capture is proven.
  2026-08-06: the DM.BIN HUD parsers now require the authenticated menuctrl
  sentinel-group positions, zero reserved words, and 320x224 Saturn screen
  bounds; malformed fixture-shaped tables cannot enter the HUD receipt.
  Startup runtime receipts now also keep `hud_ready` clear after a DGN handoff;
  level-loaded state alone cannot promote an unbound Saturn HUD consumer.
  FONT256.S2D loader now exposes only the 242 real character-generator tiles
  derived from the verified CG region; it no longer allocates 14 zero-filled
  glyph placeholders. The remaining text gap is the original Saturn
  code/attribute-to-screen owner, not another guessed glyph count.
  The section-aware S2D glyph-byte map now also verifies every caller-supplied
  range against the parsed SCR section identity and byte budget; keep the
  actual Saturn code/attribute mapping and screen placement capture-gated.
  The real-SCR screen-text helper now fails closed instead of routing
  FONT256.S2D through the obsolete flat glyph stream; restore it only after
  the page/tilemap/attribute-to-character mapping is bound from Saturn data.
  ISO-only Nexus roots now remain on the ISO source route when hash discovery
  returns virtual `disc.iso::...` entries; do not classify container members as
  loose extracted files or retry them as `data_dir/LEV00.DGN`.
  The legacy public PRS3 API now follows the same DMWeb forward/negative
  window rule as the active decoder; the pinned retail MENU.BPK test decodes
  all 162 PRS3 surfaces as indexed 8bpp; PRS3 mode tags are not host RGB
  widths. VDP1 pixel/CLUT/palette admission remains capture-gated.
  The real MNS decoder now retains the observed 33-joint SCORPION and
  37-joint ROCKPILE skeletons instead of truncating them at 32; visible
  creature rendering and VDP1 texture/palette handoff remain open.
  The Structure3 viewport rasterizer now also requires the complete scene's
  transform and pixel/palette/VDP1 semantics bits; payload/format admission
  alone can no longer present a textured mesh before Saturn capture. Its host
  route also requires a non-empty captured frame witness; mesh submission by
  itself cannot claim viewport readiness.
  The legacy raw SAL sample-index API is now diagnostic-only: decoded SAL
  bytes cannot start host playback without a source-owned event→MAP selector
  binding. Canonical SLEV/SAL/SNDLEV bytes and SDDRVS identity now also remain
  blocked at the runtime receipt until `event_dispatch_source_verified` is
  admitted. The real SLEV/SAL/SNDLEV corpus, SDDRVS ABI and distributed game
  call sites still need an authenticated Saturn event trace before production
  SFX playback can be admitted.
  SLEV rule dispatch now has a separate `source_dispatch_trace_verified`
  receipt bit; parser/profile flags and even a forged canonical task profile
  cannot enable actions without the missing Saturn SH-2/event trace.
  The legacy Nexus door raster API is now no-draw: its former DM1-derived
  gap geometry and palette guesses were removed. Bind Saturn door materials,
  animation frames, and VDP1 destinations before restoring door pixels.
  M11's direct TITLE.CG copy path is also removed; retain decoded title bytes
  as a receipt until the original Saturn VDP1/VDP2 destination and composition
  are captured. The render plan now rejects raw TITLE.CG atlas pixels whenever
  the real MAPD/TIBG source is present, so no unbound atlas copy can masquerade
  as the retail title composition. The generic title renderer now also stays
  blank when MAPD/TIBG is absent; missing title composition evidence cannot
  reopen the raw-atlas fallback. Startup asset receipts now also reject the
  real `TITLE.CG/4bpp-atlas` source as a drawable title route and report the
  missing Saturn title-capture handoff explicitly.
  CD-DA startup no longer manufactures WAV files from raw track bytes;
  authenticated CD image/decoder handoff remains required for music playback.
  Title timing remains a host state receipt only; the former synthetic edge
  colour ramp is now explicitly unknown until Saturn palette/capture evidence.
  The host-blinked title prompt flag is also suppressed; real prompt glyph,
  palette and cadence remain unbound until the Saturn text/HUD consumer is
  captured.

- **DUNGEON-STUDIO-FSDUNG:** Dungeon Studio: import original dungeons from
  all five games (DM1, CSB, DM2, Theron's Quest, DM Nexus) and export to a
  custom binary `.fsdung` format. The format is a superset of all games'
  dungeon structures (maps, things, tiles, creatures, items, sensors, doors).
  Importers per game read from DUNGEON.DAT (DM1/CSB), DM2 DUNGEON.DAT
  (skproject layout), Theron Track 02 level records, and Nexus .DGN files.
  Export produces a single portable binary file.

- **DM2-REAL-RUNTIME-CORPUS:** The former runtime-handoff smoke fixture is
  deliberately outside the active test suite because it fabricated dungeon,
  actor, weather, trigger and shop state. Extend the real-data M11/GDAT gates
  only with authenticated G1/GDAT/SKSave material; missing source owners must
  continue to block the corresponding runtime path. D2RS is now retained only
  as a decoder diagnostic and is rejected by slot, corpus and runtime resume;
  the dead partial restore branch and its synthetic timer/session publication
  helpers have been removed from the production runtime. The real PC-DOS
  eight-save regression proves that an original payload leaves live party
  state and raw-save handoff untouched when resume is rejected. Continue from
  SKProject's complete GAME_LOAD ownership chain, not from a partial parser.
  The mounted PC-DOS corpus now verifies all eight real raw dungeon prefixes
  after the authentic 42-byte header, but its later SUPPRESS session sections
  remain blocked until the complete SKProject read order is recovered. The
  original PC-DOS boot/HUD path again reaches the real GDAT command plan after
  the identity scanner was widened to retain every supported graphics and
  dungeon hash in one scan. Its post-admission diagnostic now also reports
  the real G1 seed and map count instead of the intentionally unavailable
  pre-load zeroes; this does not admit an incomplete save session.
  **2026-08-07 G1 corpus update:** the scene-handoff, runtime-map validation
  and static-object visibility regressions now consume only an explicit
  `FIRESTAFF_DM2_DATA_DIR` corpus. The selected PC-DOS media proves the G1
  map and item receipts; a missing selected `dungeon.dat` or `graphics.dat`
  fails rather than borrowing a private HOME corpus or silently skipping.
  2026-08-06: the live move and turn entry points now use the same verified
  boot-owned GRAPHICS/DUNGEON/GDAT provider boundary as frame rendering.
  A fixture-only dungeon or a headless profile cannot create a walkable floor
  receipt, change direction, or enter the playable loop.
  2026-08-06: the legacy direct-start loop now routes queued DM2 input through
  those same boot receipts and mirrors their resulting state only. Its former
  direct mutation of generic DM1-style party coordinates is removed.
  The old low-level SKSave helper now emits the real `c_hex2a` header boundary
  only and is not a serializer: complete original dungeon/DB write ordering
  from `SKProject/SKULLWIN/c_savegame.cpp` remains required before save output
  can be offered to players.
  2026-08-05: removed the former raw-SKSave pseudo-importer. It decoded a
  fabricated `GameStateBlock`/inventory tail instead of SKProject's continuous
  `s_savegamebuffer` SUPPRESS stream and could therefore admit non-original
  data. Raw SKSave resume remains fail-closed after its real dungeon-prefix
  receipt until `DM2_GAME_LOAD` and `DM2_READ_SKSAVE_DUNGEON` are materialised
  in source order.
  2026-08-06: the callback-only `dm2_v1_load_orchestrator_pc34_compat`
  transcript is no longer linked by the production M10/DM2 archives. It
  formerly omitted source-owned raw-block/map sizing and could therefore
  locate SUPPRESS after a fabricated raw prefix. It now rejects atomically
  before any callback until raw-layout, `READ_SKSAVE_DUNGEON`, allocation and
  runtime ownership are joined. The narrow ABI remains a future seam while
  the real eight-file PC-DOS corpus stays fail-closed after its verified
  dungeon prefixes.
  2026-08-06: the direct real-corpus regression now continues the same
  SKProject SUPPRESS reader through `s_savegamebuffer` (0x3c), `v1e0104`,
  `globalb`, `globalw`, `c_hero`, `c_wbbb`, and source-sized `c_tim` (0x0c)
  sections for all eight mounted PC-DOS saves. This replaces the old 56-byte
  state/10-byte timer diagnostic assumption at this boundary only; the
  following `DM2_READ_SKSAVE_DUNGEON` record-link stream remains unowned and
  must stay blocked from resume until it is ported in source order.
  2026-08-06: this fixed-state decode is now a production read-only receipt
  (`dm2_v1_original_raw_sksave_fixed_state_receipt`) rather than a test-local
  parser. It exposes source fields/hashes and the precise shared-bitstream
  position where `DM2_READ_SKSAVE_DUNGEON` begins for all eight PC-DOS saves;
  inventory, tile record chains and possession indices remain unowned and
  continue to block resume.
  2026-08-06: the real-corpus regression now also verifies the first and last
  record in every non-empty original DB pool against the pool offsets and
  record widths from SKProject `SKWIN/DME.h`. This proves the raw records
  remain bounded by the real dungeon prefix, but it does not yet restore the
  source-owned record-link, possession or live-allocation graph.
  2026-08-07: the M11 real-data gate now treats an explicitly configured but
  unverified DM2 directory as a hard failure instead of a skip. The automatic
  per-user fallback remains skip-safe when no user corpus is installed, so a
  green result cannot conceal a mistyped or synthetic explicit data root.
  2026-08-06: corpus classification and receipted rereads now retain a raw
  SKSave when both its authenticated header and source-owned dungeon prefix
  validate, without routing it through the deliberately fail-closed session
  importer. The state-corpus receipt also binds all eight mounted PC-DOS
  files to their exact fixed SUPPRESS identities (`v1e0104`, `globalb`,
  `globalw`, heroes, save state and timers) plus the shared timer/record-link
  boundary. This is read-only evidence, not a resume admission: complete
  `DM2_GAME_LOAD` ownership is still required before any of these fields can
  change a live game.
  2026-08-07 timer-stream update: the authenticated fixed-state receipt now
  re-enters the shared MSB-first reader at the saved `c_tim` boundary and
  preserves each source-sized 12-byte timer record through the exact
  record-link boundary. The records remain read-only; timer-owner linking and
  live `GAME_LOAD` publication are still blocked.
  2026-08-06 direct-root update: the real corpus now also traverses the
  source `DM2_READ_SKSAVE_DUNGEON` direct roots for all 30 item slots of each
  saved champion plus the party root, reusing the exact shared SUPPRESS
  reader state after `c_tim`. This is a read-only record-type/byte-consumption
  check over all eight PC-DOS files. It deliberately stops before map-tile
  chains, possession-index restoration, record allocation and live session
  publication; none of those can be inferred from the inventory receipt.
  2026-08-06: `READ_RECORD_CHECKCODE` now preserves source record bytes for
  `DM2_SUPPRESS_READER(..., false)` and derives the map-container branch from
  `c_record::b_04` bits 1..2, including its possession-index bit. This closes
  a false recursive-chain interpretation in the isolated decoder; the full
  raw-corpus route is still blocked because creature AI and moneybox decisions
  require the authentic GDAT providers and live record allocation/ownership.
  2026-08-06: the callback-only save/load orchestrator and its record-chain
  helper cluster are explicitly test-only again. They have no M11/runtime
  caller and cannot be linked into either production archive as an accidental
  Firestaff-private SKSAVE reader or writer while those original owners remain
  incomplete.
  2026-08-06 record-chain ownership update: the isolated reader now carries
  SKProject's actual `DM2_APPEND_RECORD_TO` contract: the allocator callback
  receives the source root (`i16*`) or tile coordinates, initializes/appends
  the new link, and exposes each allocated record's `uw_02` child root before
  recursion. The two saved placement bits are retained in the record link.
  The real eight-file DOS corpus traverses all champion-item and party roots
  through this contract. This is still diagnostic-only: no production G1 DB
  allocator, tile-chain owner, possession-index owner, timer record owner, or
  `DM2_GAME_LOAD` publication exists, so no source save can resume yet.
  2026-08-13 raw-pool baseline update: the production c_record owner can now
  materialize the exact DB0..DB15 spans from an authenticated raw SKSave
  dungeon receipt in RAM. It verifies every source pool hash and boundary,
  copies no G1 extension, and keeps `record_graph_complete` false. The
  mounted eight-file PC-DOS corpus proves both the complete baseline and a
  one-byte tamper rejection. This is only the state after
  `READ_DUNGEON_STRUCTURE`: port the source remove/clear/reallocate order,
  tile roots, possessions and timer links before connecting it to Continue.
  2026-08-08 map-owner update: the source `c_map::dm2_v1e038c`
  ground-stack span now has one mutable RAM owner. It follows
  `skmap.cpp::DM2_GET_OBJECT_INDEX_FROM_TILE`, removes DB4..DB15 links from
  real tile chains before the DB-clear phase, and retains the DB0..DB3
  resident chain. All eight original PC-DOS saves pass this ordering check.
  This is still only the predecessor to map-chain mask restoration and does
  not publish a session, reuse a synthetic map or unblock Continue.
  2026-08-08 resident-chain update: the DB0–DB3 in-place SUPPRESS reader is
  now available to that map owner. It follows `table1d64db` and the eight
  original DB3 actuator subtypes that carry a preceding nine-bit value. Wire
  it into the single GAME_LOAD transaction with live map/tile callbacks,
  dynamic empty-tile allocation, possessions and timers; it remains
  deliberately unavailable as a standalone Resume path.
  2026-08-08 transaction-map-context update: a single callback context now
  binds exactly one real map owner to exactly one raw record-pool owner. It
  supplies all `DM2_LoadExtraDungeonCallbacks` map operations, including
  resident-chain restoration and empty-tile root storage. Connect it next to
  the source `READ_RECORD_CHECKCODE` allocation and possession list rather
  than routing either through a parallel dungeon structure.
  2026-08-08 tile-owner update: the map owner now has a writable RAM copy of
  every authenticated map tile span and implements the native
  `DM2_LoadExtraDungeonCallbacks` map, geometry, tile and ground-link
  callbacks. The original SKSAVE body remains read-only. The remaining
  transaction must supply the resident-chain callback plus dynamic allocation
  and preserve this owner through possessions and timer reconstruction.
  2026-08-13 DB-clear update: the next `DM2_READ_SKSAVE_DUNGEON` phase now
  preserves DB0..DB3 and clears only GenericRecord::w0 in every DB4..DB15
  row, exactly as SKProject does before `READ_RECORD_CHECKCODE` allocates
  saved roots. The same eight-file corpus proves that non-link bytes remain
  untouched and a mismatched raw receipt cannot modify a pool. Map-chain
  detachment, root reallocation and publication remain one unimplemented
  transaction; this phase alone cannot admit Continue.
  2026-08-06 AI-mask correction: the reader no longer silently treats an
  unavailable `QUERY_CREATURE_AI_SPEC_FLAGS` lookup as zero. It requires the
  original `CREATURES[type] → v1d296c` selection before choosing DB4's
  `v1d647f`/`v1d648f` mask. `dm2data.cpp::c_dm2data::init` loads the
  executable's 63 × 36-byte `v1d296c.dat` baseline, and
  `c_record.cpp::DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD` selects its row
  through `CREATURES[type & 0xff].word(0x05)`; optional `CREATURE_AI` is an
  override, not the PC-DOS baseline. The reader now represents all 256 source
  type keys while retaining that 63-row table. Against the mounted eight-file
  PC-DOS corpus all eight direct-root streams now decode. SKProject's scalar
  result for a missing word is zero at the query boundary, but the live
  Firestaff handoff now treats that absence as an unowned creature record:
  absent row-5 fields for type 54 (twice) and 127 do not select
  `v1d296c[0]` or revive creature behavior. All remain non-resumable. The
  next work is to locate their original active-profile owner and to bind
  allocation/possession/tile owners. The real-data regression locks both raw
  absences and the exact five-decoded/three-owner-blocked/zero-malformed outcome.
  **2026-08-06 follow-up:** the remaining `FS2RT01` live-runtime sidecar
  serializer/deserializer is removed from the production archive and public
  API. It wrote Firestaff's session, creature cache, mutable dungeon bytes and
  derived GDAT state, none of which is the original `DM2_GAME_SAVE` SUPPRESS
  order. The old fixture is compile-disabled; no runtime route may recreate
  `SKSave.runtime` while the original writer is incomplete.
  **2026-08-06 follow-up:** session publication is now private to the
  parsed-original save-candidate transaction. The former public
  `dm2_v1_runtime_apply_session` API could write a caller-built session into
  the live party/map state; it is no longer exported or linkable outside the
  source-candidate path.
  **2026-08-06 follow-up:** the remaining public raw-candidate/slot resume
  entry points now reject before publication as well. A valid SKSave prefix
  is still a useful diagnostic receipt, but it lacks the later source record,
  possession, hero, actuator and timer ownership required by complete
  `GAME_LOAD`; it must not create a partially playable runtime.
  2026-08-06: D2RS decoder envelopes are now rejected by both public slot
  loaders. They remain explicit diagnostic inputs only; a player-facing
  Continue/slot action can admit neither a Firestaff private envelope nor the
  real raw corpus before complete `DM2_GAME_LOAD` support exists.
  2026-08-06 champion-format correction: the prior 261-byte convenience
  `DM2_ChampionRecord` and all-ones SUPPRESS mask are not the PC-DOS
  `c_hero` source format. Original payload decoding now uses SKWINDOS'
  exact 0x107-byte `table1d6356` and preserves its raw records separately;
  only independently proved display fields are copied out. The retained
  legacy 261-byte helper is diagnostic-only. Continue with the complete
  16-bit inventory, leader-hand record-checkcode/DB-chain and
  possession-index owners before any original save can be resumed or hand
  commands can be shown live.
  2026-08-06 inventory/leader-hand correction: SKProject's
  `LeaderPossession` is indeed a 22-byte runtime cursor (`ObjectID`, picture
  buffer pointer and pixel bytes), but SKSAVE writes only its 16-bit ObjectID
  through `WRITE_RECORD_CHECKCODE`; it does not write a 22-byte struct or a
  flat 32-bit value. Likewise `c_hero::item[30]` is a 16-bit link array at
  `0xc3`, not the legacy 32-bit `DM2_ChampionRecord::inventory` cache.
  Firestaff now rejects those flat read/write helpers, publishes neither
  cache on source-session admission, and reports inventory interaction as
  unavailable instead of swapping synthetic object handles. Implement the
  source record-checkcode/DB allocation chain before reopening this route.
  **2026-08-06 ABI follow-up:** the residual public runtime setter symbols
  are now no-ops/rejections as well. Neither a caller-provided 32-bit leader
  handle nor a caller-provided 32-bit inventory slot can enter runtime state;
  getters expose zero until the original `LeaderPossession` and `c_hero` DB
  ownership is complete.
  2026-08-06 menu-inventory correction: the source-authenticated raw PC-DOS
  candidates remain visible to the startup scanner instead of being hidden as
  if no original save existed. Selection is separately regression-tested to
  return only a rescan/redraw failure until complete `DM2_GAME_LOAD` ownership
  can publish a session.
  2026-08-06 slot-publication correction: the public slot and last-session
  loaders now reject every raw candidate after its receipt is read. Previously
  a valid raw prefix could return success with a zeroed `SessionState`, which
  made an incomplete GAME_LOAD look resumable to M12 or the startup menu. The
  real eight-save PC-DOS regression now verifies both that the slot call fails
  and that its caller-owned session bytes remain unchanged.
  2026-08-06: `docs/dm2_test_coverage.md` no longer calls this codebase an
  untested stub. Its current coverage statement is intentionally limited to
  source-bound component and real-data receipts; the same document records
  the incomplete live owners above as playable-parity blockers.
  2026-08-06: Greatstone's PC 1.0 `GRAPHICS.DAT` catalogue is now bound to
  the original file's 5,624-entry raw table. All 4,031 IMG3/IMG9/IMG11
  rasters now have a nonzero decoded-pixel receipt from those same original
  bytes; FNT1 raw 0203 is separately classified as the single
  Interface/Main-Screen scroll-font record, not a HUD fallback. A temporary,
  non-shipped full PNG audit against Greatstone found 4,030/4,031 exact
  palette-index matches. The remaining raw 2279 U4 image is a documented
  exporter semantic difference: SKProject returns its packed pixels with the
  raw-tail local palette, while Greatstone expands the nibbles to greyscale.
  Do not alter the original palette path or add downloaded/generated artwork
  merely to imitate that third-party export.
  **2026-08-06 corpus-lock update:** the maintained real-data census now
  requires the selected PC 1.0 English source file to retain all observed
  structural and decoded-pixel facts: 5,624 raw entries, 11,854 ENT1 rows,
  5,676 image rows, 4,031 unique image RAWs, 18,633,937 decoded pixels and
  census hash `bf5050d3`. A merely decodable substitute GDAT file can no
  longer pass this complete-asset inventory.
  2026-08-06: a callback-supplied fixture raster can no longer make the
  runtime ownership receipt `full_gdat_frame_valid` (or its outdoor variant).
  Only the mounted boot provider's raw and decoded `GRAPHICS.DAT` evidence
  can publish a valid M11 frame; fixture blit tests remain isolated and
  invalid by design.
  2026-08-06: `dm2_v1_runtime_render_frame()` now also rejects an empty,
  unhashed boot profile or a non-boot GDAT callback before it writes a pixel.
  This closes the remaining direct-render fixture seam: only the mounted
  `DM2_GAME_LOAD` graphics/dungeon owners can produce a runtime frame.
  **2026-08-06 CCM inventory correction:** the public opcode table now
  accurately labels CCM0B/CCM0C, wall activation, ladder/hole, transform and
  1B7D5 as callback-bound SKProject handler ports rather than stubs. This
  does not admit a live creature path: production still rejects absent
  command-stream, DB4/CAII and callback ownership.
  **2026-08-06 CCM door-data correction:** the runtime no longer gives every
  creature-facing door a fabricated zero attribute word. For the current G1
  map it now requires the DB0 door root, its map-header slot, and the exact
  `DOORS/dtWordValue/0x0d` GDAT value used by SKProject
  `GET_DOOR_STAT_0D`; any missing source owner leaves the field unread.
  This source lookup does not reopen the separately blocked live CCM path.
  **2026-08-06 callback-audit isolation:** the broad production compatibility
  glob no longer links `dm2_v1_runtime_parity_pc34_compat.c`. Its timer,
  record, creature and actuator callback bodies have no live DB/CCM owner.
  The only active source-bounded subset, SKProject's `ddat` global-variable
  store, now lives in `dm2_v1_glob_var.c`; all other bodies remain available
  only to their explicit audit targets.
  **2026-08-06 CCM execution boundary:** `dm2_v1_ccm.c` is now test-only and
  creature ticks no longer turn reduced `b_1a`/`b_17` fields into movement,
  attacks, direction changes or cooldowns. Reconnect only with the complete
  SKProject DB4/CAII command stream and live dungeon/timer/party callbacks.
  **2026-08-06 world-state boundary:** the partial `dm2_v1_world_state`
  SKSave projection is test-only. It has no M11/runtime caller and does not
  own SKProject's continuous SUPPRESS stream, so it cannot be a resume route.

- **THERON-V1-TRACK02-HANDOFF:** The production viewport now has a
  source-bound lifecycle/presentation path with a structural fail-closed seam.
  Reconnect the real Track 02 level handoff only after the authenticated
  tile/material/UI banks and square mapping are decoded.
  CUE parsing now accepts the unquoted `FILE TQUS02.iso BINARY` form used by
  the supplied real CUE sheets while still rejecting trailing tokens and
  malformed MSF fields before media/hash intake. Keep BIN/CUE/ISO receipt
  normalization and later
  object/level records on the same verified-media path.

  Real Track 02 teleporter records now retain their packed level/y/x target
  link and execute validated cross-level transitions. Missing source-bound
  targets reject instead of falling back to the clicked square. Tile/material
  and full bitmap semantics remain separately blocked.

  Production `firestaff` and M11 Phase A now link and pass with this route;
  remaining handoff work is the authenticated non-startup level/object
  corpus and real bitmap/palette capture.

  The real VDC BAT window is now retained as a raw BAT-word to atlas-index
  binding after snapshot population, so later HuC6280 square/material work
  can consume the exact captured tile/palette pairs. This still does not
  assign a dungeon-square meaning to a BAT word or enable synthetic drawing.
  An explicit capture-only BAT preview now copies those authenticated tile
  pixels into the production framebuffer for pixel/app-capture inspection;
  world-driven dungeon and HUD drawing remain blocked pending the consumer.
  Snapshot file intake now also requires exact 65536-byte VRAM and 1024-byte
  VCE files; trailing or concatenated bytes are rejected before palette/tile
  ownership is established.
  The same exact-size rule now applies to the in-memory raw loader API, so a
  caller cannot bypass the file boundary with a prefix of a larger capture.

  The raw media-intake regression now exercises the supplied assembled US ISO
  at `~/.firestaff/cache/theron/TQUS02-ceb02343868f80cec899e9b239aff2da.iso`
  when present, proving its direct MODE1/2048 hash, sector count, user-data
  window, and deliberate exclusion from raw-loader trace preparation. BIN/CUE
  and ISO paths therefore have separate verified receipts; later semantic
  publication still requires the executing consumer trace.
  The same regression now passes against the supplied retail split CUE with
  `TQUS19.iso` plus `TQUS02End.iso`, proving that the production materializer
  reconstructs the canonical US ISO before the direct receipt is checked.
  2026-08-06 targeted regression: the real-data level-bank test passes for
  all seven US and seven JP Track 02 banks and the authenticated US/JP Track
  19 level offsets; the 653-case startup-flow probe and 57-case M11/M12 handoff
  boundary also pass. The semantic consumer gate remains unchanged.
  The live Mednafen launcher now admits authenticated MODE1/2048 Track 02 ISO
  CUEs separately from raw MODE1/2352 BINs (`ceb023...` US and `397039...` JP),
  so the retail split-ISO route can reach the same capture gate. This is an
  intake/capture change only; it does not promote a consumer, level, object,
  bitmap or palette binding.

  The supplied retail US ISO now contributes one bounded real startup-level
  envelope at `0x5a9114`, linked to the first retail descriptor by `0x92f2`.
  Its seed-table window is still zero-fill, so this proves the level record
  only; runtime dungeon draw and full semantic handoff remain gated.

  **2026-08-06 real Track 02 level-bank receipt:** the level-block test now
  accepts `FIRESTAFF_THERON_TRACK02_RAW` and re-reads the supplied authentic
  US MODE1/2352 BIN using the source raw-sector user-data rule (offset 16).
  It verifies all seven documented block offsets, the shared 0xE8-byte
  prologue and every eight-byte per-level metadata record. This closes the
  old hardcoded-only test gap while deliberately promoting no tile, palette,
  map or object semantics.
  The level-bank and graphics-format probes now also discover `TQUS02.bin`
  and `TQJP02.bin` from the standard `.firestaff/data/theron` root, while
  retaining explicit environment overrides. Real media is therefore scanned
  automatically when supplied; the graphics decoder and runtime handoff stay
  blocked because the HuC6280 consumer still does not bind the bank semantics.
  Each of the seven real level banks now also exposes its complete opaque
  post-prologue compressed span and identity hash through a bounded receipt;
  the original decompressor and tile/material consumer are still required
  before any drawing or map semantics can be enabled. The receipt now
  rejects a mutated span against the per-level US/JP retail FNV identities,
  so a caller cannot substitute an arbitrary normalized buffer.
  The independent VRAM-trace diagnostic now maps captured VDC BAT words to
  the actual tile index and VCE palette group, including separate atlas entries
  when one tile is used with multiple groups. This is capture-side parity only;
  no Track 02 tile-bank or runtime viewport admission is implied.

  The real Mednafen `*.trace.cd` sidecar now has a strict opaque receipt path:
  it validates the source marker, 54 SCSI READ commands, 171 MODE1/2352 raw
  sectors and all 171 command-to-sector bindings from the latest captured app
  run.
  This is transport/app-capture evidence only; the loaded `$2600` RAM
  consumer bytes and level/object semantics still require a RAM-window
  capture with executing PC and source-LBA provenance.

  The real Main-RAM loader sidecar now adds a strict loader-boundary receipt.
  A native-SDL2 Mednafen app capture also supplies 4,096 bounded consumer-read
  rows with executing-PC and physical-bank coordinates. The new consumer
  receipt accepts that exact grammar and keeps `$2600` publication,
  object/level records, and viewport rendering blocked until source-LBA/FIFO
  provenance reaches the actual RAM window.
  it verifies the observed TIA transfer `$c800 -> $0404`, length `0x80`,
  logical PC `$2286`, physical PC `$1f0286`, the loader RTS, and the post-RTS
  opcode. This still contains no `$2600` RAM bytes or source-LBA join, so the
  consumer entry, object/level records, and semantic publication remain
  blocked.

  The legacy asset-loader source-evidence strings are now aligned with the
  versioned consumer/palette receipts, and empty/truncated input is rejected
  before the compatibility loader reads its region probe. This is an
  integrity/truthfulness fix only; the actual Track 02 graphics consumer,
  bitmap bank, palette offset and live `$2600` RAM join remain open.

  **2026-08-06 synthetic palette-promotion guard:** the runtime capture helper
  no longer turns a caller-supplied `palette_semantic_binding_verified=1`
  boolean into semantic or render authorization. A real HuC6280 consumer/capture
  receipt must supply that evidence; the helper remains fail-closed meanwhile.

  Full-dungeon loading now accepts a valid zero-ground-reference map and
  rejects object-capacity exhaustion instead of reporting a partial success.

  Undecoded item categories are now rejected instead of being projected into
  synthetic `0x10 + category` host objects. Bind real item kind/index records
  before reopening that branch.

  The loader now retains each real category-4..10 and raw category-14/15
  occurrence in a bounded source receipt with its packed reference, raw
  next-link, category/index, map position and exact bytes. This is an opaque
  handoff only; the original consumer still has to bind item kind/index to
  host inventory semantics before these records may become live objects.

  **2026-08-06 source-category census:** the full loader result now counts
  every retained authentic category-4..10/14/15 occurrence for both US and JP
  variants. The test prints and cross-checks the census against the retained
  source receipts; it remains a DMBUILDER category receipt, not an inferred
  host item-kind or inventory mapping.

  The same receipt now carries a compact bitmask census of decoded raw type
  fields for categories 4..8 and 10. US/JP differences and high raw values are
  preserved exactly; no generic DMBUILDER range is used to relabel or reject a
  real Theron record, and no type value is promoted to a host item ID.

  The same map/ground-reference/item-record handoff now accepts the
  authenticated Japanese `TQJP02.bin` offset table through the full loader;
  this verifies JP source records but does not bind their host gameplay or
  graphics semantics. JP ISO offsets remain separately gated. JP text-codon
  offsets are also intentionally gated after candidate windows failed the
  real codon-structure check.

  The source receipt decoder now also reads the exact six-byte missile and
  two-byte cloud payloads after their linked-list references, matching the
  DMBUILDER `dm_missile`/`dm_cloud` layouts. This is still a source-data
  receipt only; no projectile or cloud gameplay ownership is inferred.

  Unknown creature-stat categories are now rejected as well; the runtime no
  longer invents fallback HP/attack/defense values outside the four
  disassembly-backed formulas.

  2026-08-06 generator boundary: production no longer activates the legacy
  DMWeb-derived generator table, nor retained Track 02 generator records whose
  original timing/re-enable consumer is still unbound. Source records remain
  diagnostic receipt data until the real generator consumer is captured.
  2026-08-06 archive hygiene: the legacy DMWeb creature-table translation unit
  is now excluded from `firestaff_theron`; it remains available only to the
  explicit diagnostic/fixture table tests.

  THIEF/DEMON scripted encounters no longer use approximated template stats;
  bind their source encounter records before admitting them to combat.

  Removed the public synthetic creature-name aliases used by old tests;
  tests and probes now name the seven real Track 02 dungeon creatures.

  Roster initialization no longer derives portrait IDs from party slots;
  bind actual portrait tile records before exposing portrait graphics.

- **THERON-FIXTURE-HELPER-ARCHIVE:** Closed 2026-07-31. Synthetic first-room
  constructors are now compiled only into explicit fixture targets; the
  production Theron archive exports no generated level-buffer helpers.

- **THERON-TRACK19-RECORD-INTAKE:** Verified US/JP Track 19 ISO identities
  are now recognized as MODE1/2048 sector media with exact sector counts;
  raw MODE1/2352 remains explicitly distinguished. `inventory_file()` now
  normalizes raw sector payloads before running the same hash/offset-bound
  item, label, opaque-window and startup-envelope checks. This proves
  container intake only. Later-level and object semantics remain blocked until
  a source-backed CD-read/record trace binds them.

  The US Track 19 item-name span and 66×6 item-property table are now
  byte-validated from real ISO data; level/object records still require the
  source loader trace before runtime publication.

  The real 15-entry Track 19 level-selector label span is now byte-validated
  as metadata; it does not yet admit any level map or object record.

  The JP Track 19 ISO item-name table is now byte-validated as 69 raw
  Shift-JIS records at UD 0x0E92B1; the file inventory verifies the exact
  source span after the known JP ISO MD5 gate. JP level-selector labels and
  all later map/object properties remain closed until their original consumer
  is proven. The JP level-selector table is now also byte-validated as 15
  fixed 16-byte Shift-JIS records at UD 0x203A7E, including its 0x8197
  delimiters; this remains metadata only and does not admit maps or objects.

  The first 396 bytes of the Track 19 follow-on span are now authenticated as
  the real 66×6-byte item-property table (US UD 0x0E951D, JP UD 0x0E955D),
  byte-identical to the source-bound Track 02 records. The older 502-byte
  window (US 0x0E951E, JP 0x0E955E) remains an overlapping structural receipt;
  its unclassified remainder is not promoted to map/object/bitmap/palette
  semantics until the original consumer/disassembly binds it.

  File-backed Track 19 inventory now carries the authenticated source hash and
  both metadata-verification flags; later record semantics remain closed.

  The real US and JP Track 19 ISOs now also validate the identical 876-byte
  startup-level envelope at `0x5a9114` (`0x36c`, FNV-1a `0x54fce0a0`), which
  matches the canonical retail Track 02 envelope. This is a record-level
  provenance receipt only; the original consumer for objects, tiles, palettes,
  and later-level records remains unproven, so runtime publication stays closed.

  The static bank-$1f consumer window at file offset `$1f0000+$243e` is now
  byte-identical in the real US and JP Track 19 images (134 bytes, with exact
  MD5/size gates). This confirms the helper is shared across both regional
  static banks, but it still does not supply the post-CD `$2600` RAM consumer
  bytes or its source-LBA join. Keep object, tile, palette, and later-level
  publication blocked until that live RAM/PC capture exists.

  Track 02 object-data and dungeon-map bounds are now overflow-safe; truncated
  sources are rejected before ground-reference or item records can be
  admitted. This is a loader-integrity fix only, not semantic promotion of
  unbound records.

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

- **THERON-CUE-FULL-PAYLOAD-VALIDATION:** Closed 2026-07-31. The USA and JP
  CUE handoff regressions now read the complete hash-verified Track 02
  payloads before validating descriptor sectors; no semantic level/object
  promotion is implied by this transport fix.

- **THERON-V1-UI-CHROME-REAL-DATA:** *(Partially resolved v3.0.246)*
  Font glyphs decoded: 120 monochrome 8×6 glyphs from UD 0x09A000
  (text alphabet + ASCII + UI decoration). The inferred bar/champion-slot
  implementation remains fixture-only. Production exposes a no-op API until
  the original Track 02 UI chrome bank is decoded. The startup menu also
  withholds fixture portrait/class metadata; decoded names alone do not
  authorize inferred portraits or classes. The production viewport HUD text
  now consumes the verified 8×6 Track 02 glyphs. Remaining: HUD tile bank,
  bar/slot layout coordinates, portrait graphics.

  The V2 procedural overlay is now likewise excluded from the production
  archive; production exposes only a no-op seam until the real HUD bank is
  decoded. The procedural implementation is retained only in the startup
  receipt fixture target so its handoff contract remains testable; it is
  excluded from the production archive. The remaining widget manifest code
  is admission diagnostics only, not a render source.
  2026-08-06 regional font-tile update: `test_theron_v1_font_tiles` now
  discovers and verifies both supplied raw BINs from the documented data
  root. US uses UD `0x263200`, JP uses UD `0x262A00`; both decode 96 tiles,
  87 nonblank, with the same authenticated checksum. This proves the
  regional font-byte bind only; HUD tile bank, bar/slot geometry and portrait
  graphics remain capture-gated.
  The retired procedural `theron_v1_ui_chrome.c` module is now removed from
  the repository and the rendering test links the same production no-op seam;
  remaining work is only authenticated HUD tile-bank, layout, and portrait
  binding from Track 02.
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
  The cross-checked numeric champion records remain available to
  `theron_v1_party_init()` for the source-bound forcefield handoff. The old
  claim that they came from US UD `0x09D1D6` was false: that US window is
  executable code. The authenticated JP raw BIN now has a bounded eight-record
  receipt at raw offset `0x0B3D98`, including names, titles, class/kön,
  HP/STA/MANA, seven attributes and 16 skill nibbles. Production compiles out
  unbound US names/titles; the named cross-reference table is fixture/probe-only.
  Soul Room companion selection via `theron_v1_party_set_companion()`.
  Skill sub-levels (Fighter/Ninja/Priest/Wizard × 4 sub-skills) added from
  DMWeb encyclopaedia, cross-validated against Track 02 base stats.
  Primary class now derived from highest skill tier per champion.
  Starting equipment per champion from DMWeb wired into roster and
  applied during party init (Track 02 item indices).
  Dungeon-to-creature-region mapping fixed: AKUTUBA=D1 (not D2),
  all 7 dungeons now have level_count=3 per DMWeb.
  DOTAN per-dungeon availability now enforced (absent from Dungeon 1).
  Creature generators from DMWeb ChristopheF maps wired into level
  transitions and world tick (5 dungeons with generators).
  Spawn zone/template lookup fixed to use dungeon_id.
  The verified category table at UD 0x21A046 remains available for decoding,
  but creature loot publication is blocked until source T900 records bind
  category, item index, quantity and gold generation. Previous host gold
  ranges/category lists are no longer admitted.
  The authenticated forcefield-entry path now retains the source-bound
  champion HP/skills/equipment while the later dungeon capture gate rejects
  unproven media. Remaining roster work is the US text/portrait consumer and
  JP portrait consumer;
  do not clear the source roster as a fixture fallback after handoff.
  Dungeon seeds: TQ data blocks have no DM1-style global header with
  randomGeneratorSeed; seeds are likely in PCE code, not data.
  Remaining: portrait graphics from tile banks.

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

- **THERON-CHAPTER-MARKER-REAL-PROGRESSION:** Closed v3.0.223. All 7 dungeon
  names are now binary-verified from Track 02 UD 0x2741EF (creature name
  table). Quest item ordering matches UD 0x27713D retrieval messages.
  Viewport dungeon name display wired to the progression module.

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
  The real Track 02 category/spawn-zone descriptors and monster occurrences
  are retained as source receipts, but production must not use
  `theron_v1_track02_compute_spawn_stats()` until the PCE bank-switched RNG
  call at overlay `$4644/$4667` is disassembled and bound. The former
  dungeon/coordinate replay seed was synthetic and is now removed from the
  live spawn path; diagnostic formula tests remain fixture-only.
  THIEF/DEMON scripted stats are blocked until their encounter records bind.
  Category formulas remain diagnostic-only; the real-data playability probe now
  links the production archive and verifies that no host-seeded creature is
  published from a source level.
  Legacy DMWeb generator respawn is no longer a production route; generator
  timing and re-enable behavior remain closed until the Track 02 consumer is
  bound.
  Drop category mapping remains a decoder utility; runtime loot stays blocked
  until source T900 records and the PCE RNG call are decoded.
  PCE rand() lives in bank-switched overlay ($4644/$4667) — not statically
  resolvable; current LCG assumption unconfirmed but unrefuted.
  2026-08-06 production-boundary update: the five regular Track 02 spawn
  zones now retain source-header-verified monster occurrences but refuse live
  creature publication while the original RNG consumer is missing.
  Scripted Thief/Demon records, AI/attack behavior, T900 loot and sound stay
  closed until their source consumers are captured.
  2026-08-06 generator-integrity update: a rejected legacy generator label no
  longer increments `generator_spawn_count`; bind the real Track 02 monster
  record/type consumer before admitting generator creatures or AI.
  2026-08-06 source-record update: the full Track 02 loader now binds every
  decoded category-4 monster occurrence into a world-owned source ledger with
  its exact dungeon, level, coordinates, source reference, type, position,
  health words, number and direction flags. Synthetic random level placement
  is closed for verified Track 02 dungeons; the ledger remains data-only until
  the original type-to-graphics, AI and live-combat consumers are proven.
  2026-08-06 generator-record update: map-reachable type-6 floor actuator
  records are now retained with their exact source chain, coordinates, value,
  effect/timing flags and target. The old static generator table and random
  placement are bypassed for verified Track 02 worlds; source generator timing,
  re-enable and spawn consumers remain closed until the original route is
  bound.
  2026-08-08 record-admission hardening: source monster and generator ledgers
  now require a loaded, source-header-verified level before accepting a record.
  This closes phantom source records without promoting RNG, AI, timing or loot
  semantics.
  2026-08-08 VRAM snapshot correction: the authenticated dungeon snapshot's
  BAT-at-$0000/tile-at-VRAM-zero layout is now admitted after the legacy
  fixture `$1000` tile base yields no tiles. This binds the captured VDC/VCE
  bitmap bank without claiming square-to-tile dungeon semantics.
  2026-08-08 atlas-capacity follow-up: the real 64x32 BAT can address up to
  2048 distinct tile/palette pairs; the production atlas now has that exact
  capacity instead of rejecting a valid full snapshot after 1024 entries.
  The operator snapshot now passes the real-capture probe with 1057 atlas
  entries and 896 BAT cells; keep its output behind the raw-capture gate until
  the dungeon square consumer and screen provenance are joined.
  2026-08-06 source-object update: every decoded map-reachable category
  4–10/14/15 occurrence is now copied into a persistent world source bank
  with raw bytes, chain links, category/index, position and map coordinates.
  Host object, inventory and item-kind publication remains deliberately
  unbound until the original ownership consumer is proven.

- **THERON-FORCEFIELD-MENU:** *(Open — visual capture handoff)*
  `ENTER FORCEFIELD` now publishes the authenticated raw Track 02 map/thing
  loader result into the initial dungeon. VDC/VCE presentation remains gated
  until its original consumer is captured; host item semantics remain closed.

- **THERON-SHOP-REAL-DATA:** *(Closed v3.0.242)* No shop/vendor mechanic
  exists in Theron's Quest. Full Track 02 scan found zero BUY/SELL/SHOP
  strings and no price array near item properties or elsewhere. The shop
  helper fixture is permanently excluded from production.

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
    hash-verified, but M11 still blocks launch. The native APPA → ANIM → APPB
    language handoff and the allocator-dependent final TITL.DAT delta need a
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
  - 2026-07-31: F1066–F1085 is a fail-closed Amiga inventory with no CSB
    PC 3.4 consumer. It is test-only; retain separate source-owned helpers
    only where a genuine runtime path needs them.
  - 2026-07-31: F1126–F1145 is a fail-closed mixed source inventory, not an
    input, palette or viewport implementation. It is test-only; live paths
    require their own authenticated CSB material and event consumers.
  - 2026-07-31: F1186–F1205 documents DM1-owned ANIM routines but has no
    authenticated CSB ANIM stream or runtime consumer. It is test-only; do
    not use the inventory to synthesize CSB title, entrance or UI timing.
  - 2026-07-31: F1206–F1225 is an ANIM ownership inventory which admits no
    route. It is test-only; palette, sound and allocation behavior must come
    from a separately authenticated CSB runtime consumer.
  - 2026-07-31: F1406–F1445 has no ReDMCSB callable symbol and is test-only.
    Local-source labels must never be used as a portable CSB startup or
    entrance implementation.
  - 2026-07-31: F1526–F1565 is a fail-closed workstation/AES inventory and
    is test-only. CSB PC 3.4 input, graphics and timing need real dedicated
    consumers, never a foreign platform receipt.
  - 2026-07-31: F1646–F1685 is a fail-closed platform inventory. It is
    test-only; DM1-owned mouse state and foreign interrupt/vblank paths must
    not be promoted to CSB runtime behavior.
  - 2026-07-31: F1726–F1765 contains only local, platform and debug source
    labels. It is test-only; do not derive any CSB input or visual behavior
    from this blocked metadata.
  - 2026-07-31: F1766–F1805 is a fail-closed media ownership inventory. It is
    test-only; actual CSB palette and SWSH consumers stay source-bound and
    must not be replaced with DM1 media labels.
  - 2026-07-31: F1886–F1925 is a source receipt inventory. It is test-only;
    its separately implemented save/import owners remain the only admissible
    CSB paths and no hint labels may synthesize presentation data.
  - 2026-07-31: F1966–F2005 is a fail-closed hint/debug inventory. It is
    test-only; no CSB input, display or file behavior may be inferred from its
    unbound source labels.
  - 2026-07-31: M11 rehashes the selected CSB pair at entry, preventing a
    changed file from inheriting scan-time admission. Continue auditing
    long-lived runtime caches for the same source-receipt lifetime rule.
  - 2026-07-31: Removed the unbuilt `csb_v1_game` shim that carried fixed
    positions and a no-op DM1 import. Continue checking unreferenced CSB
    compatibility surfaces before treating their fixture support as runtime.
  - 2026-07-31: Utility handoff now requires its full imported party receipt,
    not compatibility metadata. Continue auditing ownership fields that could
    describe source state without carrying its real payload.
  - 2026-07-31: File-backed CSB dungeon loading now rejects the retired
    16-bit fixture layout. Continue limiting fixture-only parsers to memory
    tests and keeping every runtime file boundary source-material-only.
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
    viewport-frame probe end-to-end (30/30) with a source dungeon, start pose,
    floor band and deterministic frame. Continue requiring this real-data
    route for any new CSB viewport/HUD material consumer.
  Production rendering must fail closed for a missing or unbound original
  asset. Procedural mazes, generic walls, coloured creature blocks, substitute
  palettes, and synthetic HUD art are not acceptable when authentic game data
  exists. Asset admission must likewise use verified hashes, never filename
  guesses. Each remaining renderer is to bind the real format or present no
  game pixel; test fixtures remain explicitly isolated from production paths.
  **2026-07-31 update:** the legacy generic and multilingual asset pipelines
  plus M11's builtin dungeon resolver no longer accept filename-only
  `GRAPHICS.DAT`/`DUNGEON.DAT` candidates. Renamed and archived data remains
  supported through the shared hash scanner. The remaining work is limited to
  format-specific decode gaps listed per game, not generic substitute loading.
  **2026-07-31 update:** M12's synthetic data-scanner setters are absent from
  the production library and executable. The DM2 missing-graphics and M12
  popup regressions compile their own fixture-enabled asset-status object; a
  live launcher can only scan the checked-in hash registry.
  **2026-07-31 update:** DM2's source-owned credits event now reaches the
  common 0xEF dismissal route for either original mouse button; M11 no longer
  traps the title menu in credits because of a host-only left-click filter.
  **2026-08-06 update:** DM2's Credits, Quit and credits-dismiss pointer
  routes now decode their actual `INTERFACE_GENERAL/0/RAW4/0` rectangles
  (ids `0x019b`, `0x01b2`, `0x0002`) from the mounted, hash-verified PC
  `GRAPHICS.DAT`. M11 does not replay the old fixed coordinate matrix for
  those actions; absent or undecodable RAW4 data leaves them inert.
  **2026-08-06 follow-up:** New Game and Resume now likewise use their real
  RAW4 ids (`0x0197` and `0x0199`), rather than mistakenly passing their
  input events (`0xD7`/`0xD9`) to `QUERY_RECT`. The last M11 dependency on
  the fixed DM2 click-zone matrix has been removed.
  **2026-08-06 title/credits identity follow-up:** the M11 real-PC-DOS startup
  gate now requires separate bounded GDAT receipts for `TITLE/0/dt07/4`
  (menu) and `TITLE/0/dt07/1` (credits), as well as their shared source
  `dtPalIRGB` presentation. A credits payload can therefore not silently be
  presented as the startup menu.
  **2026-08-06 leader-hand follow-up:** the unproven fixed `304,41,14,14`
  leader-hand icon/cursor zone is no longer exposed by M11. The API reports
  no zone until SKProject's live `DRAW_ITEM_ICON` rectangle and image route
  are bound from original DM2 data.
  **2026-07-31 update:** the M12 settings catalogue now has one explicit
  sixteen-row two-column capacity shared by the renderer, hit-test contract
  and regression suite; the renderer/brightness additions no longer leave a
  stale twelve-row menu assertion behind.
  **2026-07-31 update:** M11 no longer selects the legacy indexed launcher,
  whose renderer had no pointer hit map. The launcher now either presents the
  clickable M12 surface or reports a renderer allocation error; it never shows
  a menu that cannot be operated by mouse.
  **2026-07-31 update:** the V2 HUD widget PNG decoder is now compiled only
  into its isolated synthetic-fixture probe, not the production DM2 V2
  archive. Its strict no-draw test remains available, while a real game can
  only reach the GDAT-backed HUD renderer.
  **2026-07-31 update:** the V2.2 finished-art manifest classifier is also
  test-only now. It classifies local/synthetic metadata but has no
  source-owned pixel consumer, so the production archive cannot link it or
  promote a local modern-art manifest into a DM2 render path.
  **2026-07-31 update:** the remaining V2.2 local modern-art cache chain
  (manifest, RGBA cache, shape cache and viewport swap) is now test-only as
  well. M11 no longer parses or classifies that data before routing V2.2 to
  the authenticated V2.1 presentation; the complete production executable
  links without the local-art chain.
  **2026-07-31 update:** the unused V2 host palette/RGBA asset pipeline is
  now test-only too. M11 no longer configures its arbitrary indexed-pixel
  conversion or stub palette-LUT path; a live DM2 image must stay on the
  existing GDAT-backed renderer until a source-bound enhancement consumer is
  implemented. The pipeline source is explicitly removed from the production
  DM2 V2 archive and compiled only by its isolated probe.
  **2026-07-31 update:** the local V2 HUD widget-manifest classifier is now
  compiled only by its diagnostic HUD probe. Production HUD code never scans
  local widget metadata and the final executable has no widget-manifest
  symbol; visible HUD pixels remain restricted to GDAT.
  **2026-07-31 update:** the legacy V2 interaction-feedback overlay is now
  probe-only as well. Its fixed host click zones and HUD feedback animations
  have no authenticated GDAT input owner and no M11 caller; the production
  archive retains the source-gated V1 input route only.
  **2026-07-31 follow-up:** V2's local smooth-camera, bloom and animated
  outdoor state are probe-only as well. They inferred movement timing,
  weather and lightning without a decoded original timer/image owner. M11
  and the game loop now retain the authenticated V1 viewport and GDAT HUD
  routes; the V2 diagnostic sources remain available only to focused tests.
  **2026-07-31 follow-up:** the remaining V2 companion UI, empty crafting
  table and smooth viewport interpolator are also excluded. None has a
  decoded DM2 record, item recipe, display route or original timing owner;
  they remain diagnostic source only until such data is imported.
  **2026-07-31 correction:** companion UI and empty crafting were absent from
  the executable but still leaked into the broad V2 static archive. They are
  now removed from that production archive too; only explicit diagnostics may
  compile the historical source.
  **2026-07-31 update:** Greatstone's DM2 version catalogue now explicitly
  bounds the PC startup audit: PC 0.9/1.0/demo list `GRAPHICS.DAT`, whereas
  title/swoosh/ending media belong to other platform families. Continue to
  admit such media only through a hash-verified, platform-specific decoder;
  no generic external animation fallback may enter the PC route.
  **2026-07-31 update:** boot-profile documentation and diagnostic output no
  longer describe install names as an admission fallback. Continue auditing
  every remaining user-facing diagnostic for the same hash-only distinction.
  **2026-07-31 update:** the canonical DM2 outdoor-frame verifier now keeps
  static installation data separate from c_weather's live session chain.
  It proves a real GDAT sky/ground/HUD frame and requires weather to remain
  no-draw until an imported runtime/save owner supplies the real chain. Keep
  weather fixtures confined to their unit tests; do not present them as a
  playable-session capture.
  **2026-07-31 follow-up:** removed the now no-draw V2 overlay state module
  from the production archive as well. Its default compass, gold, level and
  champion-bar values were host inventions with no save/session receipt. The
  GDAT runtime now retains only its visibility gate and draws only decoded
  `INTERFACE_GENERAL` records; historical overlay experiments compile only in
  their focused tests.
  **2026-07-31 follow-up:** V2:s svep- och controller-affordance-brygga är
  nu också probe-skopad. Den hade ingen M11-händelsekonsument och använde
  egna host-definierade gester/zoner, medan den levande vägen använder den
  SKProject-bundna V1-klickmatrisen. Ingen oansluten V2-rörelsemappning får
  längre ingå i spelet innan en verklig DM2-inputkälla är importerad.
  **2026-07-31 follow-up:** den fristående combat-resolvern är nu också
  test-skopad. Den hade inga produktionsanrop och kombinerade host-definierade
  vapen-, dörr- och skadetal i stället för att läsa DM2:s aktiva recordkedja.
  Riktig speldata måste fortsätta äga framtida combat-handoff.
  **2026-07-31 follow-up:** tech/magic-hjälparen är test-skopad. Den har ingen
  M11-konsument och dess enda uppslag är explicit no-data; den får inte
  auktorisera mekanik eller etiketter utan en importerad DB/GDAT-definition.
  **2026-07-31 follow-up:** de oanslutna engelska record-, UI- och
  spell/skill-namnuppslagen är också test-skopade. Inga texter får nå
  produktion från lokala konstanttabeller innan DM2:s riktiga text- eller
  GDAT-källa är kopplad.
  **2026-07-31 follow-up:** den oanslutna champion-statbryggan är test-skopad.
  Den ska inte omvandla generiska värden till HUD-data förrän en levande
  champion/session- och palettkvitto-kedja äger hela vägen.
  **2026-07-31 follow-up:** fristående champion-HUD- och food/water-bryggor
  är test-skopade. De har ingen levande M11-konsument och får inte ersätta
  en komplett original session- och GDAT-handoff.
  **2026-07-31 follow-up:** den gamla utomhusfasaden är test-skopad. Den
  kunde bara returnera no-draw och hade ingen levande konsument; väder och
  himmel måste fortsatt komma från den autentiserade GDAT-rutten.
  **2026-07-31 update:** the unconnected HUD panel-routing and survey helper
  modules now compile only in their focused tests. M11 and the production DM2
  archive contain no accidental HUD route for those standalone receipts; a
  future runtime integration must bind their exact GDAT/record owners first.
  **2026-07-31 update:** the deterministic c_weather setter is now compiled
  only into its weather-frame fixture executable. The normal runtime header
  and `firestaff` executable expose no API for replacing a live weather chain;
  authentic session/save ownership remains mandatory in production.
  **2026-07-31 follow-up:** world and object ingestion now reject the
  retired 16-bit/sequential-pool fallback outright. The only admitted model
  is the loader-owned PC G1 byte map and its validated c_record chain; an
  unproved payload produces no world or object records.
  **2026-07-31 update:** V2 HUD, lighting and touch force-active setters are
  now diagnostic-only APIs. The production executable cannot bypass a V2
  phase gate; the aggregate V2 probe also now expects a no-data HUD to leave
  the framebuffer untouched rather than paint synthetic pixels.
  **2026-07-31 update:** DM2 boot now rejects the generic loader's retired
  16-bit fixture layout and an unreadable/absent dungeon payload. Only the
  verified PC G1 byte-square route can cross from the startup menu into a
  live game; fixture parsing remains available only to isolated diagnostics.
  **2026-07-31 update:** the boot boundary now rehashes both admitted media
  files immediately before entering the game. A path cannot inherit a stale
  scan receipt after its `GRAPHICS.DAT` or `DUNGEON.DAT` content changes.
  **2026-07-31 update:** CSB's boot handoff now refuses both unreadable
  verified paths and the retired 16-bit parser fixture. A session becomes
  runtime-ready only after the ReDMCSB byte-map and its initial party pose are
  materialized, so M11's HUD and viewport cannot bind an empty dungeon.
  **2026-07-31 update:** the legacy direct game loop now shares that CSB boot
  boundary. It rejects missing or unmaterialized CSB data and consumes the
  boot-owned party pose instead of re-entering the generic DM1 parser and its
  fixed Hall-of-Champions fallback coordinate.
  **2026-07-31 update:** the public `csb_v1_runtime_boot()` seam now has the
  same rule. It clears any prior singleton and rejects missing graphics,
  unreadable/non-byte-map dungeons or a missing source start pose instead of
  reporting a title runtime with ENDOF accessors.
  Direct synthetic DM2 projectile construction is now isolated to explicit
  test and probe targets and is absent from the production library. The DM2
  champion-stat bridge also rejects a missing source GDAT/palette bar-colour
  receipt rather than using the retired host colour table. **2026-07-31
  update:** the HUD render plan no longer promotes a missing per-champion
  colour into a source-bound value; only the source INIT/runtime bridge may
  supply the logical colour, and the active original interface palette remains
  mandatory before bars draw.
  **2026-07-31 update:** the retired DM2 shop stack-size switch and its fixed
  item-ID constants are gone; every unbound object is unstackable until its
  DB/GDAT record supplies a real quantity rule.
  **2026-07-31 update:** removed the unattached `DM2_DRAW_STAIRS_FRONT`
  fallback-receipt API and its hard-coded GRAPHICSSET/Rect14 tables. It had no
  M11 caller and could not prove SKProject's B073 palette transaction, so it
  was a false source-data surface rather than a renderable original-data path.
  **2026-07-31 update:** the obsolete caller-authored DM2 companion API now
  rejects rather than fabricating a name, health, combat values, loyalty or
  AI behaviour. A future companion route must bind the live DB creature,
  CAII/CCM, inventory and dialogue records before it can populate state.
  **2026-07-31 update:** M11 no longer enables the procedural V2 lighting and
  outdoor-FX state merely because V2 presentation is selected. That runtime
  remains dormant until it receives the live V1 ENVIRONMENT-GDAT weather
  receipt (timer, images and palette); the existing source-backed V1 weather
  renderer remains the only active owner meanwhile. The public DM2 creature,
  spell and bomb projectile dispatchers now also fail closed: creature id and
  host target coordinates cannot invent the CCM/timer-owned cell, direction,
  energy, step cost or tick needed by F0810. Test-only projectile construction
  remains isolated behind its compile-time test gate. The trigger-target
  creature shortcut likewise cannot invent an `ALLOC_NEW_CREATURE` direction
  or health multiplier; it remains unavailable until a decoded DB14/CCM
  payload supplies them.
  **2026-07-31 update:** the runtime no longer installs the temporary
  spell-timer handlers. Their DB14/DB4 cloud, missile and summon bodies used
  replacement duration, energy and creature-type fields; their light and hero
  bodies likewise lacked a complete saved-session owner. These timer types
  are now acknowledged without mutation until `DM2_PROCEED_TIMERS` can bind
  the original hero records, timer payload, DB14/DB4 links and GDAT rows.
  The arbitrary public CAII mode-byte setter is closed as well: only the
  original CCM/record writer may select a delete mode for a live creature.
  **2026-07-31 update:** caller-selected CAII capacity is now test-only too.
  Production refuses it until `DM2_INIT` imports the original
  `ddat.v1e08a0` save/session owner.
  **2026-07-31 update:** standalone creature creation is now test-only.
  Production rejects caller-authored type, map, position, direction and
  health multiplier until `ALLOC_NEW_CREATURE` has its DB4/current-map/
  record-chain/RNG owner.
  The standalone animation-word writer is likewise test-only until the
  CCM/GDAT frame receipt owns both values.
  **2026-07-31 update:** the legacy no-stream CCM runner no longer treats a
  host program counter as a creature command byte. It rejects without
  mutation; only an authenticated decoded command stream may enter
  `DM2_PROCEED_CCM` compatibility execution.
  **2026-07-31 update:** boot no longer probes arbitrary `CREATURE_AI` GDAT
  fields looking for decodable CCM bytes, and creature ticks no longer rebuild
  missing operands from reduced local instance fields. The CCM route remains
  unavailable until the source's live record/field owner is recovered.
  **2026-07-31 update:** DM2 V2 smooth movement no longer post-processes a
  completed source viewport into a panned image with host-generated black
  strips. Smooth input timing remains available, but every displayed frame is
  now the authenticated snapped V1 raster until an original intermediate DM2
  camera raster is recovered.
  **2026-07-31 update:** the retired V2 sky-colour helpers no longer derive
  RGB output from host time and weather values. The source ENVIRONMENT GDAT
  route owns the image, palette and destination rectangle, so an unbound V2
  caller now receives no colour instead of a procedural sky substitute.
  **2026-07-31 update:** the remaining DM2 ObjectID inventory bridge no
  longer reuses DM1 `GRAPHICS.DAT` slot coordinates, panel pixels, or click
  routes. SKProject `CHANGE_VIEWPORT_TO_INVENTORY` owns a distinct
  `CHAMPIONS`/`INTERFACE_GENERAL` GDAT layout and event table; the M11 route
  now preserves the source session but stays unavailable until those assets
  and events are bound.
  **2026-07-31 update:** the public DM2 viewport HUD renderer now applies the
  dt04 layout, dt07 font, interface palette and source-owned champion state
  receipt even when a caller has not set its source-only flag. Missing proof
  leaves HP/stamina/mana and leader pixels untouched; raw logical palette
  indices can no longer become a compatibility HUD substitute.
  **2026-07-31 update:** the V2 compatibility HUD no longer overwrites the
  V1 portrait panel with `CHAMPIONS` images selected from slot ordinals. The
  original `DRAW_CHAMPION_PICTURE` route requires a save/session hero type,
  so portraits remain solely V1 source-state material until V2 carries that
  same receipt.

  **2026-08-06 M11 gate environment correction:** the real M11 startup/profile
  regression now accepts the shared `FIRESTAFF_DM2_DATA_DIR` root used by the
  other PC-DOS corpus probes. Its default is now the mounted PC-DOS owner
  directory rather than the broad multi-game root, and watchdog expiry is a
  nonzero test failure rather than a false passing skip. This is verification
  discovery only; it neither supplies fixture data nor admits an incomplete
  save/session into runtime.

- **CSB-TITLE-CADENCE:** The M11 CSB title zoom now holds frames 60--79 for
  four PC3.4 cadence slots. Keep the following TITLE.C F0437 `Delay(20)`
  phase on its own cadence and independent of gameplay speed multipliers.
  **2026-07-31 update:** The presentation-receipt contract now independently
  locks the full source timeline to 60 + 20 + 20 + 2 = 102 ticks; it may not
  promote an old 101-tick/two-tick-CHAOS-hold claim.
  **2026-07-31 update:** the room-slot backdrop1 trace fixture is no longer
  linked into production M10. It remains available only to its contract test;
  live CSB viewport paths must bind authentic dungeon and graphics material.
  The adjacent contract-only pass-order and backdrop traces are likewise test
  binaries only; the room-slot selector remains in M10 because live CSB
  material binding uses it.
  **2026-07-31 update:** C407-regressionen mappar nu probernas 320×200- och
  960×600-koordinater mot den begärda logiska ytan i stället för dummy-SDL:s
  gamla 1024×768-fönster.
  **2026-08-06 update:** Amiga `TITL.DAT` now retains its source `PL` palette
  as sixteen indexed 4-bit RGB components and verifies it against real media.
  ReDMCSB `ANIM.C` F1181 owns that read. The distinct Amiga GRF1 decoder now
  expands the real `EN` base image through `EXPAND.C` F0466; its final command
  intentionally consumes six bytes from the following `DL` record because
  the source expander receives no ByteCount. The first 30 complete `DL`
  streams now apply through the source buffer-copy/flip model. The final
  282-VBL `DL` remains fail-closed: its GRF1 stream reaches the end of the
  on-disk FTL item before filling the destination. ReDMCSB `ANIM.C` F1177
  allocates exactly file size plus `ANIMDESC` through non-clearing MEM1, so
  there is no source-defined allocation padding to recover. Do not route it
  through PC IMG2/IMG3 or invent zero-padding for missing source bytes.
  **2026-08-06 media-authentication update:** the real-media title regression
  now verifies the extracted file's registered Amiga 3.1 MD5 before decoding.
  A same-shaped fixture, renamed ADF or arbitrary `TITL.DAT` no longer counts
  as title evidence.
  **2026-08-06 scanner build follow-up:** the shared scanner no longer retains
  an unused DM1 FM Towns admission result while it scans CSB media. The
  admission itself remains mandatory; only its dead local copy was removed so
  focused scanner builds stay warning-free.

- **DM2-M11-GAME-LOAD-ORIGINAL-HANDOFF:** M11 now keeps New Game at the
  source `SHOW_MENU_SCREEN` → `GAME_LOAD` boundary rather than constructing
  a fixture party. New Game now reloads the hash-verified original
  `DUNGEON.DAT` through `LOAD_NEW_DUNGEON`. SK-projects deliberately clears
  the party and leader hand there; G1 owns the initial pose, which the reload
  now restores before entrance, and dungeon
  mirrors later create champions through `SELECT_CHAMPION`. Bind the remaining
  `GAME_LOAD` actuator/timer initialization plus source-owned mirror-selection
  UI before allowing the menu to enter runtime. **2026-07-31 update:** a
  malformed G1 candidate without a valid source start pose now rejects
  atomically; it cannot retain an earlier world's entrance position. Accepted
  reloads also refresh deterministic header fields from the same verified
  source bytes. Original SKSave import no longer seeds this route from the
  legacy fixture party before decoding its own SUPPRESS champion records.
  The former exported `dm2_v1_session_new()`/starter-party route has been
  removed from the production library; its explicitly named fixture now lives
  under `tests/` only.
  The associated hard-coded portrait-to-class mapping and initial-stat record
  builder are removed too; real champion records remain owned by GAME_LOAD
  and the original SUPPRESS payload.
  A verified New Game reload now also clears stale resume portraits,
  inventory and leader hand before source mirror selection; it still cannot
  create a replacement party or enter runtime.
  2026-08-06 follow-up: the reload receipt now distinguishes the required
  source clear from a completed clear. M11 accepts the boundary only after
  both its own and the retained runtime cache report no party, leader hand or
  inventory residue. This remains a cache-integrity gate, not a substitute
  for the missing original champion/record ownership.
  The bounded `dm2_v1_select_champion()` receipt also remains deliberately
  non-mutating: request coordinates without the live DB3 subtype-0x7E mirror,
  hero record and inventory chain now return failure rather than a synthetic
  champion-selection success.
  Asset admission now rejects cross-platform GRAPHICS.DAT/DUNGEON.DAT mixes;
  finish the same pair-bound admission for every remaining container and
  installer format before exposing those ports in the launcher.
  The FM Towns CD ZIP is accepted with either the global data root or `dm2/`
  itself as the configured root, while the payload remains memory-only.
  **2026-07-31 update, corrected 2026-08-07:** the boot profile no longer
  preloads a PC-English seed or map count. Both remain zero until the
  hash-verified `DUNGEON.DAT` File_header supplies `w0` and `nMaps`.
  **2026-08-06 update:** M11 now also rejects decoded SKSave session subsets
  at the runtime boundary. `DM2_GAME_LOAD` must restore the complete original
  record pools, timer queue, actuator-generator pass and entrance placement
  before Continue or Load Slot can leave the source menu.
  **2026-08-06 update:** projectile, step and creature-collision receipts are
  now test-only too. Bind their private F0810 list to the original
  CCM/timer/DB transaction before a projectile can exist in a live DM2 frame.
  **2026-08-07 real-data gate correction:** the selected-corpus champion-
  mirror regression no longer treats an unreadable or wrong-sized
  `DUNGEON.DAT` as a skip. It now fails the selected input, then verifies the
  actual 16 source G1 mirror roots and keeps selection fail-closed until the
  remaining `GAME_LOAD` hero/record handoff exists.
  **2026-08-08 mirror-owner correction:** the direct File_header census now
  reads the sixteen real DB3 subtype-`0x7e` roots from `dunGroundStacks` and
  preserves their map-0 positions, facing and source champion types `0..15`.
  It no longer confuses the unrelated `0x1ff` dynamic-load marker with a
  champion identity. The bounded selection seam accepts only those matching
  source fields, but remains non-mutating until the original c_hero,
  possessions and timer transaction is one owner.
  **2026-08-08 c_hero-input update:** the PC-DOS `CHAMPIONS/type/dtRaw8/0`
  reader now admits only the exact 52-byte source record used by
  `DM2_REVIVE_PLAYER`: three vital bases, seven ability bases and sixteen
  skill levels. It retains the raw-byte identity and does not decode names,
  invent food/water rolls or write a hero record. The remaining owner must
  join the original RNG, party position, possessions and timer initialization
  before New Game can leave the entrance flow. The original champion-name
  query and first-space split are now bound through `QUERY_GDAT_TEXT`.
  The narrow c_hero helper now retains the original five-row skill layout,
  but it is still disconnected from the live party and cannot authorize a
  synthetic selection or entry sequence.
  **2026-08-08 status-gate correction:** the complete-support receipt now
  distinguishes a parsed raw-SKSAVE census from a completed live GAME_LOAD
  owner. It cannot report complete support until the chosen original stream
  has atomically restored map, record, possession, hero, timer and actuator
  ownership. The current importer therefore remains explicitly blocked,
  including for the four corpus files whose local record pool is complete.
  **2026-08-08 stream-order correction:** direct hero/cursor roots now stop
  at their authentic boundary. The remaining importer must restore special
  timer chains, map chains and only then possession continuations, exactly as
  `DM2_READ_SKSAVE_DUNGEON` orders them.
  The direct-root receipt now carries the exact bit position at that boundary;
  use it for the next phase rather than deriving a byte-aligned substitute.
  **2026-08-08 special-timer phase:** source order now reaches
  `DM2_2066_197c` through an isolated raw-pool preflight, with the original
  c_hex2a `w_00`, c_tim bytes and carried SUPPRESS state. It remains blocked
  whenever the local pool cannot own the chain; map-chain and possession
  phases still need to join this transaction before Resume can open.
  **2026-08-08 map-chain correction:** `READ_SKSAVE_DUNGEON` no longer treats
  every tile as an empty dynamic chain. A resident DB0..DB3 tile chain must
  be restored in place by its map/record owner; only `OBJECT_END_MARKER`
  reaches `READ_RECORD_CHECKCODE`. The generic loader now rejects a missing
  resident-chain owner instead of consuming the shared stream into a
  fabricated replacement. The remaining transaction must provide that owner,
  then bind map chains and possessions in the original order.
  **2026-08-08 map-index recovery:** the raw SKSAVE receipt now resolves each
  tile with bit `0x10` through the original `v1e03d8` column-index span and
  `dm2_v1e038c` ground-stack links. This identifies the resident root before
  any stream bytes are consumed. The next transaction phase must restore the
  root's masked DB0..DB3 bytes in place, then restore only empty-tile dynamic
  chains and the final possession continuations.

- **DM2-MERCHANT-CCM-OWNER-HANDOFF:** The coordinate-only NPC route is
  closed. Bind a live AI-33 DB creature through `DM2_THINK_CREATURE` and its
  `PLACE_MERCHANDISE`/`TAKE_MERCHANDISE` CCM records, including source-owned
  merchandise data and UI text, before enabling merchant interaction. Do not
  restore a fixed merchant identity, dialog table or reputation counter.
  **2026-07-31 update:** the legacy `dm2_v1_enter_shop()` shortcut is also
  closed; an outdoor flag and host gold value cannot enter a shop.
  The runtime's last-NPC accessor also starts at `DM2_NPC_NONE`; an unbound
  merchant route cannot expose a friendly-merchant identity before its source
  creature/CCM/UI owner exists.
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
  is closed. Port the complete `DM2_RECALC_LIGHT_LEVEL` inputs: leader and
  champion light possessions, `v1e0974`, spell effects, rain modifiers and
  the source light modifier. Dynamic-map viewport frames must remain blocked
  until that state is recovered from original runtime/save records.
  **2026-08-06 update:** the standalone `dm2_v1_graphics_data_open` adapter
  is likewise excluded from the production archive. Runtime GRAPHICS.DAT
  admission is owned solely by `dm2_v1_asset_loader`; no weather or viewport
  path may use the duplicate test-only receipt as an alternate data route.
  **2026-07-31 update:** the disconnected outdoor facade also no longer
  seeds a noon value; temporal ENVIRONMENT selection remains unavailable
  until its source owner is imported.
  The live V1 runtime and weather helper now likewise start with an explicit
  unknown environment time, refuse the former fixed 1,092-tick minute and
  cannot emit a procedural sky colour. The legacy colour-only helper is now
  unavailable even for a restored clock because it carries no GDAT material
  receipt. A validated resumed session may retain its supplied time, but
  source `timdat` plus the environment globals still need to replace that
  compatibility handoff.
  **2026-07-31 update:** 4bpp cursor expansion now likewise requires the
  active original palette; the previous identity-table substitute is closed.
  The legacy weather particle-count helper is also closed: it has no source
  ENVIRONMENT command/image receipt and cannot promote enum-derived particles.
  The former five-entry pressure-plate catalog is now removed; import
  original dungeon sensor/actuator records before permitting plate effects.
  The former eight-entry trigger catalog is likewise removed; only decoded
  source actuator/timeline records may create doors, messages or spawns.
  Boot's former 1,440-minute/day-cycle configuration is also unavailable;
  it must be populated only by the same recovered source owner.
  **2026-07-31 update:** fresh weather state now has neither a clear-weather
  selector nor a default RNG seed. An outdoor flag, host seed, or the
  unowned bounded session-rain field cannot fabricate the `0x54` weather
  timer chain or enable clouds, rain, or lightning. Import the exact
  `v1e14xx`/savegame owner before admitting that chain.
  **2026-08-06 update:** an outdoor map/tick no longer manufactures a valid
  `DM2_SET_TIMER_WEATHER` receipt. The runtime clears that receipt until the
  complete source-owned `v1e14xx` chain is present, preventing independently
  supplied GDAT slots from promoting an unowned weather overlay.
  **2026-08-06 seed follow-up:** the residual public runtime seed setter now
  ignores caller values. A host-provided number cannot persist in the live
  weather state before the original GAME_LOAD/SKSAVE/ENVIRONMENT owner is
  decoded.
  **2026-07-31 audit:** the legacy colour-only outdoor facade is outside the
  production archive. The active viewport's outdoor route requires G1 map,
  GDAT/local-palette, c_light and source timer-slot receipts together; seven
  focused scene/light/weather gates pass without a fallback draw.
  **2026-07-31 follow-up:** the unattached progression-constant table is no
  longer linked into the production archive. It carried a host-side 1,440
  minute day and named weather states without a session/GDAT owner; only a
  decoded `timdat`/environment chain may supply those facts to a live route.
  **2026-08-13 source-formula update:** `dm2_v1_recalc_light_level_pc34()` now
  follows SKProject `src/v5/sklight.cpp:24-198`: the map byte is a branch
  guard, the leader hand is included, DBSPEC is queried with source key 0,
  charge contributions use the descending six-bit shift, weather uses the
  inverse `table1d6712` mapping, and dtWordValue/0x68 plus `v1e0978` are
  applied before the final clamp. The focused helper is no longer a synthetic
  map-only calculation; the live M11 runtime still requires the original
  party hand/inventory, spell, weather and save-state owner before it can
  publish a dynamic c_light receipt. **2026-08-13 real-data audit:** the
  mounted-data scene/weather/light regression now uses the source-owned
  fixed-light branch when available and explicitly rejects a real dynamic map
  without runtime state; it no longer manufactures dynamic-map `base_light`
  or darkness values. **2026-08-13 regression correction:** that mounted
  regression now builds the authenticated real `GRAPHICSSET` scene-light
  receipt before testing either map branch, so dynamic rejection cannot depend
  on an uninitialized scene record.
  **2026-08-13 formula correction:** the light helper now preserves
  SKProject's `v1e0978 > 0x0c ? 1 : v1e0978` normalization before the final
  clamp. A large caller value can no longer become an artificial light-level
  delta; the complete live party/save/weather owner is still required for
  dynamic-map admission.

- **DM2-SKSAVE-SESSION-OWNER-HANDOFF:** Original SKSave import currently
  decodes the source `skload_table_60` game-state block and its following
  sections, but Firestaff's session-only gold, reputation and time fields do
  not yet have a proven owner in those source records. Do not clear them to
  guessed zeroes or seed them from the fixture-session defaults. Trace the
  owning `GAME_LOAD`/global-state records in SK-projects, then replace the
  fixture initialization in the importer atomically with those exact values.
  **2026-07-31 update:** the compact D2RS envelope now decodes into a local
  candidate and publishes only after every supplied source section validates;
  this prevents a malformed trailing section from leaking its partial state
  while the remaining field-owner investigation stays open.
  **2026-07-31 update:** the fixed-byte SUPPRESS codec self-check is now
  test-only. Production retains the source-bit receipt and concrete
  candidate validation, but cannot run a diagnostic vector as a save route.
  The unused one-hop inventory-chain trace shim is removed rather than being
  exposed as a successful traversal; inventory paths remain closed until the
  real `c_record` link owner is recovered.
  Session rain intensity and weather-chain seed/state are also unproven in
  this envelope, so they remain unavailable rather than being promoted into
  the live environment.
  **2026-08-05 update:** the invented byte-22 `rain_state[8]` view and its
  fabricated broad mask are removed. The importer now uses SKProject's exact
  `skload_table_60` field order and `_4976_395a` 56-byte SUPPRESS mask;
  `bRainStrength` is retained only as decoded corpus evidence, not as a
  license to start an unowned weather/session chain.
  **2026-08-05 follow-up:** the legacy `DM2_WorldState` projection now also
  leaves every per-level selector explicitly unavailable. It no longer maps
  `rain_intensity` to Rain/Clear or calls unowned new-game levels Clear.
  **2026-08-05 follow-up:** its slot-envelope admission now matches the
  source `c_hex2a` header (42 bytes: version, printable label and only the
  `0xdeadbeef` missing-slot rejection). It no longer mistakes the final
  saved-dungeon metadata for a BE/EF/DE/AD magic number. The mounted eight
  original PC saves pass raw-prefix validation; full SUPPRESS world-state
  import remains blocked until the complete source graph is decoded.
  **2026-08-06 creature-state follow-up:** the generic live-creature-pool
  restore API now rejects in production. Its shape validator could not prove
  the original SKSAVE/DB4/timer/CAII transaction and therefore could not
  admit a caller-authored creature pool as a saved game.
  **2026-08-06 hero-identity update:** each source-decoded 263-byte
  `c_hero` SUPPRESS record now carries its own raw hash through the real
  corpus census and state fingerprint. The eight supplied PC-DOS saves verify
  every present hero independently; this is a provenance receipt only and
  does not promote their inventory links, stats or portrait data into a
  partial Resume session.
  **2026-08-06 evidence update:**
  `parity-evidence/dm2_sksave_game_load_boundary.md` now pins this stop point
  to `SKULLWIN/c_savegame.cpp` and to the real eight-file corpus regression,
  using the DM1 FM Towns evidence method only. The remaining map/DB/
  possession/timer handoff is still open and cannot be replaced by the
  test-only callback transcript.
  **2026-08-07 corpus update:** the corpus regression now accepts only the
  explicit `FIRESTAFF_DM2_SKSAVE_CORPUS` or `FIRESTAFF_DM2_DATA_DIR` root;
  a selected root without the complete lower-case DOS save set fails rather
  than borrowing a private HOME corpus or silently skipping.
  **2026-08-07 source-field inventory:** the raw receipt now retains an
  explicit hash of the complete 60-byte `s_savegamebuffer`, matching
  SKProject `sksvgame.cpp:47/1415` `DM2_GAME_LOAD` contract. That source buffer
  has no scalar gold or reputation field, and `gametime` is maintained
  separately; coin records and the separate time state still need their
  complete runtime owners before raw resume can be admitted.
  **2026-08-07 source-owner update:** the fixed receipt now names the six-byte
  `c_wbbb`/`ddat.savegames1` section explicitly and centralizes its source
  size. Real saves retain its raw hash only. The surrounding
  `globalb`/`globalw`/hero/timer reads provide no proven scalar gold,
  reputation, or time owner, so those session fields remain unavailable
  rather than being seeded from fixture defaults.

- **DM2-CREATURE-AI-ROW-HANDOFF:** Replace the data-free direct
  `creature_type -> AIDefinition` fallback with the original two-stage
  selection. SK-projects `SKWINSPX/src/v4/skcrture.cpp::QUERY_CREATURE_AI_SPEC_FROM_TYPE`
  first reads `CREATURES[type]` word field `CREATURE_STAT_AI` (0x05), then
  selects that AI row from the source-initialized `dAITableGenuine` table or
  its GDAT override. The existing runtime carries the verified row when a
  GDAT loader is active but must not treat the raw creature type as an AI-row
  when that owner data is absent. **2026-07-31 update:** the separate packed
  36-byte `CREATURE_AI` fixture fallback is also removed; source ownership
  now requires the `CREATURES[type].word(0x05)` plus per-field GDAT route.
  Spawn, attack, spell, HP and projectile paths reject an unowned row. Bind
  the live DB4 creature record and active loader through the remaining CCM
  and field-runtime consumers. **2026-08-05 update:** an unresolved CCM
  body now consumes its 0x21/0x22 timer without scheduling a coordinate-only
  retry. The former re-queue could keep a creature active without the
  original command stream, and is removed pending complete CCM ownership.
  The runtime tick also no longer advances the fixture-only global creature
  pool after source timers: that second clock had no DB4 allocation, linked
  record chain, RNG or CCM command-stream owner. Only a fully bound 0x21/0x22
  source timer may reopen creature mutation. **2026-08-06 update:** the
  disconnected `c_1c9a` callback audit, including 24 explicitly fail-closed
  AI bodies, is now compiled only by its named regression target rather than
  by either production archive. Re-admit individual functions only when the
  live DB4/CAII/CCM owners are imported together.
  **2026-08-06 corpus clarification:** the mounted PC-DOS English
  `GRAPHICS.DAT` has 57 authentic creature-animation table routes and no
  optional `CREATURE_AI` override graph. That is a property of this original
  profile, not missing data to substitute. Its executable-owned
  `v1d296c.dat` table is the normal AI baseline, but a live row still requires
  the active `CREATURES[type].word(0x05)` binding. Keep types 54 and 127
  unavailable for this mounted profile until their original owner is found;
  do not infer either row or revive creature, combat or CCM behaviour.
  **2026-08-07 corpus update:** the dynamic V5 material and animation
  real-data probes now require the explicit selected corpus and fail when it
  cannot supply an accepted boot profile, AI classification or FB/FC/FD route.
  **2026-08-07 animation-corpus update:** the remaining `1c9a_0a48`
  animation-reader regression now likewise reads only the selected
  `FIRESTAFF_DM2_DATA_DIR` corpus. Its mounted PC-DOS GRAPHICS.DAT receipt
  proves 57 authentic animation-table routes; a selected root without a
  readable original `graphics.dat` fails instead of borrowing a private HOME
  corpus or silently skipping.
  **2026-08-07 row-owner update:** the live baseline now imports the
  authenticated 63×36-byte `table1d296c` byte table already retained from
  `SKProject/src/v5/dm2data.cpp`, including row 62. A real PC-DOS receipt
  proves 74 `CREATURES[type].word(0x05)` bindings and 73 non-identity type→row
  mappings; the old 62-row duplicate could falsely reject row 62 as unowned.
  **2026-08-07 regression tightening:** the selected PC-English corpus test
  now locks that census to exactly 74/73 and asserts types 54 and 127 remain
  unowned. This proves the two-stage row boundary without promoting either
  type into creature behavior; DB4/CAII/CCM ownership remains open.
  **2026-08-06 callback audit follow-up:** `DM2_1c9a_09b9` is no longer a
  zero-return placeholder. It now performs SKProject's exact DB4 record
  word-`+8` comparison through the record-owner callback. This isolated
  helper does not re-enable the disconnected AI archive or substitute any
  creature state; the remaining larger c_1c9a bodies stay fail-closed.
  **2026-08-08 build-hygiene update:** the production AI-loop translation unit
  is warning-clean under `-Wall -Wextra`; removed dead local word helpers and
  made the deliberately unbound wound/tick ABI explicit. This changes no AI
  decision or callback ordering. The live DB4/CAII/CCM handoff remains open.

- **DM2-LEGACY-GAME-LOOP-DATA-ADMISSION:** `src/engine/firestaff_game_loop.c`
  is not part of the built M11 DM2 launch route and still contains diagnostic
  ceiling/floor and test-maze code. It must remain disconnected. Before any
  future repair or reactivation, remove those substitutes and route startup
  through `dm2_v1_boot_startup_launch_alloc()` so only hash-verified original
  `GRAPHICS.DAT` plus `DUNGEON.DAT` can reach a runtime frame. **2026-07-31
  update:** the separate legacy `dm2_v1_load_dungeon()` discovery shim is now
  fail-closed too: it cannot report a dungeon as loaded without publishing
  the parsed map and record ownership that `dm2_v1_boot_enter_game()` owns.
  **2026-07-31 update:** the legacy direct loop now rejects before boot when
  hash discovery has not admitted both `GRAPHICS.DAT` and `DUNGEON.DAT`; it
  also stops if the verified source handoff fails. It cannot initialize a
  diagnostic DM2 state or draw any substitute frame.
  **2026-08-06 object-model boundary:** the standalone
  `dm2_v1_object_model.c` parser is no longer linked into `firestaff_dm2`.
  Its loader-backed inspection remains available only in the dedicated probe;
  the inferred sequential-pool branch is removed and cannot become an
  alternate GAME_LOAD/M11 object-data owner. Re-admit it only after its full
  G1 chain is consumed by a live source-owned runtime route.

- **DM2-ACTUATOR-SHOOTER-OWNER-HANDOFF:** Port the actual shooter actuator's
  DB14/timer scheduling and record-owned projectile fields from SK-projects.
  Shooter records currently reject until their owner, facing, energy and
  attack can be traced from original data. **2026-08-10 update:** production
  `dm2_v1_activate_shooter()` no longer reaches the former DB-item/timer
  compatibility body. It now returns a valid/fail-closed receipt for a
  structurally present actuator until the complete DB14/timer owner exists;
  the focused regression proves that no item allocation or timer mutation
  occurs. **2026-08-13 source receipt update:** the fail-closed receipt now
  retains the real source actuator type/data, timer coordinates/direction,
  one-tile launch coordinates, both source launch directions, payload bytes
  and energy 100 from `sktimprc.cpp:1611-1800`; it does not claim DB14
  ownership or invent a `SHOOT_ITEM` timer. **2026-08-06 cleanup:** removed
  the disabled reduced-state source study too. The actual owner handoff
  remains open.
  **2026-08-07 regression tightening:** the focused boundary now exercises
  all six source shooter classes (`0x07..0x0a`, `0x0e..0x0f`) and confirms
  each retains source launch facts while allocating no DB14 record or
  `SHOOT_ITEM` timer.
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
  square-local DB3 traversal are closed. Port `DM2_INVOKE_ACTUATOR` with the
  live DB3/DB14 record, its links, target, direction, payload and timer
  handoff before enabling switches, relays, counters, teleports, shooters or
  resurrection. The square-local public entry now returns an explicit failure
  instead of a no-op success, so an unowned DB3 root cannot be reported as an
  accepted game action.
  **2026-08-02 historical study:** the former 0x56 CONTINUE_TICK_GENERATOR
  implementation resolved an actuator from timer ObjectID, toggled
  ActionType==3 and re-queued OnceOnly records. It was later removed because
  those reduced fields do not establish the complete original transaction.
  **2026-08-02 update:** floor-mecha CROSS_SCENE (0x27) now implements
  TOGGLE_ACTUATOR_MESSAGE on the once_only field. CREATURE_AI_STATE (0x28)
  shares the CREATURE_KILLER dispatch (both fail-closed pending creature
  runtime). CREATURE_DIRECTION (0x42) recognized as a skproject TODO/no-op.
  Remaining: PARTY_TELEPORTER (0x2E floor), CREATURE_ANIMATOR (0x3A),
  CREATURE_KILLER/AI_STATE body (needs GET_CREATURE_AT + creature commands).
  **2026-08-06 provenance correction:** the former production timer bindings
  for class-2 pitfall, class-4 door, class-5 teleporter, class-6 trickwall,
  `CONTINUE_TICK_GENERATOR`, `STEP_DOOR` and `DESTROY_DOOR` are now removed.
  Their byte-only `value_b` interpretation changed real dungeon bytes without
  the complete DB3/DB14/DB0 source transaction. `PROCEED_TIMERS` consumes
  those unbound events without mutation until their record link, payload,
  direction, map owner and follow-up timer are admitted together.
  The same runtime boundary now leaves `PROCESS_0E`, `PROCESS_3D` and
  `MOVE_RECORD_ROTATE` unbound: a record-pool address or timer coordinate is
  not a substitute for the original `c_hero` inventory or `MOVE_RECORD_TO`
  link/wake/sleep/party transaction.
  **2026-08-06 runtime-cleanup update:** removed the uncalled PROCESS_3D,
  pitfall and door timer studies from the production runtime translation
  unit. They derived record relocation or square mutation from raw timer
  fields; the archive boundary now rejects their return. The remaining source
  work still requires the full record, payload and follow-up-timer handoff.
  **2026-08-06 actuator enqueue update:** removed the uncalled reduced-record
  `DM2_INVOKE_ACTUATOR`/`DM2_INVOKE_MESSAGE` timer builder. It could enqueue
  an actuator timer from caller-provided record bytes without the source DB3/
  DB14 link and target transaction; the archive boundary rejects its return.
  The adjacent uncalled `CONTINUE_TICK_GENERATOR` study is removed too: it
  toggled a DB3 byte, invoked an actuator and requeued from timer `value_b`
  without the same complete DB3/DB14 target/payload transaction. The live
  dispatcher continues to consume class 0x56 fail-closed.
  The uncalled 0x55 ornament animator study is removed as well: a raw record
  address plus an animation-length receipt cannot authorize frame-bit writes,
  activation clearing or timer requeue without the original animator record
  and timer-queue transaction.
  The adjacent 0x5A ornament-noise study is removed: GDAT decoration and
  duration receipts alone cannot authorize a timer requeue or activation
  sound before its original actuator lifecycle and audio transaction are
  restored.
  The uncalled `MOVE_RECORD_ROTATE` study is removed too: it rewrote party
  position and direction directly from timer bits instead of executing the
  original `MOVE_RECORD_TO` link, wake/sleep and rotation transaction.
  The class-0 wall-mecha study is also removed: its timer-byte coordinates,
  action and direction cannot substitute for the original DB3/DB14 record,
  target and payload handoff.
  The class-5 teleporter study is removed as well: it toggled an authenticated
  G1 tile and redirected to floor mecha from raw timer fields, without the
  source teleporter record, movement and actuator transaction.
  Its uncalled floor-mecha target is removed too: a partial record walk and
  CAII activation cannot stand in for the complete original DB3/DB4/CAII/CCM
  transaction.
  **2026-08-07 tile-query parity update:** the bounded square helpers now use
  the source low-byte encoding (`M034_SQUARE_TYPE`) and the original DM2
  element values: pit `0x02`, stairs `0x03`, teleporter `0x05`. This fixes
  the former host-side stairs/teleporter classifications without opening any
  actuator mutation; the DB3/DB14 target, payload and timer handoff remains
  required.
  The class-6 trickwall study is removed as well: it changed G1 wall bits,
  queried inferred creature state and requeued timers without the original
  DB3/DB4/CCM transaction.
  The M11 Action-door shortcut now follows the same rule: it cannot rewrite
  a G1 tile from a coordinate-only query while the live DB0 door record,
  direction, collision, sound and follow-up timer transaction are absent.
  Ornament animator/noise and the `0x58/0x59/0x5B/0x5C` DB-bit timers are
  likewise unbound in production: their raw record addresses are authentic
  bytes but not proof that the original `GAME_LOAD` timer queue, actuator
  lifetime and write transaction have been restored.
  **2026-08-13 source-root handoff:** the authenticated G1 DB3 receipt now has
  a coordinate lookup matching the source `DM2_FIND_TILE_ACTUATOR` selection.
  It exposes the exact source actuator fields without invoking, following
  `GenericRecord::w0`, mutating DB14, or scheduling a timer; generic actuator
  actions remain unavailable until that complete transaction is owned.
  **2026-08-13 querydb handoff:** the standalone actuator-type query now reads
  the source DB3 record's `w2` low-seven-bit `ActuatorType()` field through the
  authenticated record callback. It rejects non-actuator classes, ObjectID
  sentinels and missing records; activation, payload and timer ownership are
  intentionally still open.

- **DM2-SHOP-GLASS-OWNER-HANDOFF:** The fixed-coordinate catalog path is
  closed. SKProject `DRAW_WALL_ORNATE` passes the live wall actuator into
  `_32cb_0f82_SHOP_GLASS`, which resolves the shop through the active
  `WALL_GFX` GDAT image/overlay chain. Bind those record and asset owners,
  then port its transaction state; do not reopen a general-store catalog,
  NPC text, stock or price table from coordinates.
  **2026-07-31 update:** the remaining five-shop fixture catalog and all
  four host-authored NPC dialog tables are disabled too. Until the original
  actuator, `WALL_GFX` and `dt08` ownership is decoded, no shop API can
  expose stock, prices, names, dialog or mutate party state. The latent
  inventory/gold writeback bridge is removed as well: an unadmitted shop
  state cannot copy arbitrary ObjectIDs into a session.
  **2026-08-06 follow-up:** the residual public setters for gold, negotiator
  and inventory now retain no caller-supplied values either. A zero-catalog
  shop module cannot be used as a synthetic session sidecar while the original
  `SHOP_GLASS` record transaction remains unbound.
  **2026-08-13 source-data census:** the canonical G1 actuator regression now
  inventories all source DB3 roots for shop-panel `0x3f` and shop-floor `0x30`
  types before any merchant work; these type bytes remain census evidence only
  and cannot authorize a shop without the source WALL_GFX/dt08/AI-33 chain.
  **2026-08-07 real-data gate:** the canonical PC-English G1 regression now
  requires both shop counts to remain zero across the authenticated dungeon.
  It reports `shop-panel=0x3f:0 shop-floor=0x30:0`; the absence is evidence
  that this corpus has no live SHOP_GLASS case, not a reason to retain the
  fixed-coordinate catalog.
  **2026-07-31 follow-up:** the last host-panel rectangle, English footer and
  empty-pack fallback are now no-draw as well. The exposed render contract
  clears its receipt and returns unavailable until the source-owned
  `SHOP_GLASS` chain is decoded.
  **2026-07-31 follow-up:** the standalone companion no-op boundary is also
  outside the production archive. It has no live DB4/CAII/CCM caller and must
  remain an isolated rejection contract until that complete source route is
  imported.

- **DM2-OBJECT-TEXT-OWNER-HANDOFF:** Bind leader-hand/item text only through
  the decoded DB object and its original GDAT text/metadata route. The HUD
  must stay unnamed when that owner is absent; do not restore fixture IDs,
  English names, affinity/charge values, or diagnostic pool/index labels.
  2026-08-06: `dm2_v1_gfx_str_pc34_compat` is no longer linked into the
  production DM2 archive: M11 has no consumer for its standalone text adapter;
  live text remains restricted to boot/dialogue GDAT receipts. Bind the actual
  object owner before admitting leader-hand/item text.
  **2026-08-06 update:** callback-only creature attack/combat and AI-spec
  studies, plus the unconsumed animation-chunk inspection parser, are not
  production runtime code. They remain outside `firestaff_dm2` until the
  original DB4/CAII state, AI-table ownership and timer/dungeon callbacks can
  be passed as one authenticated live route.
  **2026-07-31 update:** M11 now has no residual DM2 ObjectID-to-local-name
  lookup at all. It returns no leader-hand name until the complete decoded
  DB-record to GDAT `dtText/0x18` and `FORMAT_SKSTR` ownership route exists.
  **2026-08-06 HUD correction:** the generic `QUERY_CMDSTR_TEXT` adapter no
  longer accepts caller-provided bytes as if they were GDAT text. Its public
  tuple-only signature cannot authenticate a buffer, so command-text events
  remain blocked until a mounted original GDAT query returns the raw record
  together with provenance.
  **2026-08-06 GDAT receipt correction:** the bounded item-name receipt now
  applies the source `GDAT 0/0/dtWordValue/0` bit-0x08 `~byte - ordinal`
  transform before it finds the terminator, and rejects a name that exceeds
  its bounded receipt buffer instead of publishing a prefix. The PC corpus
  confirms `WEAPONS/0/dtText/0x18 = EYE OF TIME` and
  `WEAPONS/3/dtText/0x18 = KALAN GAUNTLET`; this does not yet supply the
  required live ObjectID-to-CLS1/CLS2 M11 owner, so the HUD remains unnamed.
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
  original `GDAT 0/0/dtWordValue/0` transform owner as well as their
  `DIALOG_BOXES/0x81/dtText` records. **2026-07-31 update:** the former
  fixture-only “missing means unencrypted” branch is closed; an incomplete
  GDAT transaction now blocks the dialogue rather than displaying guessed
  text. **2026-08-06 update:** the disconnected `c_0aaf` callback dialogue
  audit is excluded from production while it lacks a source-backed M11 menu
  and dialogue-rectangle bridge; its explicit unit-test target remains the
  only consumer until this owner is bound. **2026-08-06 audit correction:**
  the apparent `_476d_04ed` TODO in that selector is not an unported menu
  routine: SkProject `skgame.cpp:2575-2579` defines
  `_476d_04ed_DOES_NOTHING` as an unconditional zero return. The compatibility
  branch now documents that source no-op and does not invent auto-selection.
  **2026-08-13 source-text update:** the open-panel handoff now validates both
  original `DIALOG_BOXES/0x81/dtText/0` and `/1` records, including the source
  `QUERY_GDAT_TEXT` transform, before accepting any locale overlay. The
  receipt retains raw source-text hashes alongside displayed-text hashes; an
  overlay cannot become a standalone host-text owner. The callback `c_0aaf`
  menu/rectangle audit remains test-only until its complete live M11 bridge is
  imported.
  **2026-08-07 real-data tightening:** the mounted PC-DOS viewport regression
  now requires both non-zero raw source-text hashes in addition to the
  displayed `SAVE`/`CANCEL` labels.

- **DM2-V22-REAL-MATERIAL-HANDOFF:** Missing V2.2 art now returns no pixels.
  Bind any future enhanced art only through verified original V1 GDAT material
  and a complete, provenance-checked V2.2 pack; never reintroduce diagnostic
  substitute pixels. **2026-07-31 update:** the retired local modern-art
  manifest no longer promotes disk-resolvable PNGs to `REAL` or
  `FINISHED_REAL`; it has no GDAT category/index/field or raw-byte receipt.
  The cache lookup is now no-draw as well, so no caller can bypass the V2.2
  swap pass with local RGBA data. The M12 availability flag likewise refuses
  a local manifest, so V2.2 cannot be advertised as installed. The older
  Phase-2 manifest loader and M11 presentation mapping now apply the same
  refusal: a V2.2 request resolves to V2.1 and no ordinary filesystem
  manifest can provide a `V2_MODERN` source. A future
  bridge must carry those exact original-data facts before enabling any V2.2
  replacement. **2026-08-13 audit:** mounted PC-DOS DM2 source data is now
  covered by an opt-in real-data regression; GRAPHICS.DAT presence without a
  modern-art manifest remains `NO_MANIFEST` and cannot promote V2.2 pixels.

- **DM2-STEP-MISSILE-OWNER-HANDOFF:** Port the source-owned DB14/timer
  projectile handoff from `c_tim_proc.cpp::DM2_STEP_MISSILE` with its record
  owner, facing, energy, attack and map state. The current bounded timer path
  allocates a verified DB14 record where possible but deliberately does not
  invent a cache projectile from an incomplete timer payload. **2026-07-31
  update:** the unbound `AI_W30_TURNS_MISSILE` collision branch no longer
  substitutes an ordinary hit; it leaves projectile and creature untouched
  until the source target/timer handoff is available.
  The timer handler is now disconnected from the live runtime too: its
  previous DB14 creation used replacement energy/effect fields, so a real
  timer is consumed without mutation until the complete DB14/timer owner
  exists. **2026-08-11 production-boundary update:** the callback-based
  `dm2_v1_tim_proc_pc34_compat.c`, `dm2_v1_timer_ops_pc34_compat.c` and
  timer-dispatch wiring are now explicit-test-only; no live runtime caller
  supplies their source DB14/c_tim/dungeon transaction.
  **2026-08-13 real-data census:** the canonical PC-English G1 corpus contains
  zero direct DB14 missile records and therefore zero stored missile timer
  indices. This records source absence without stepping, allocating, deleting
  or turning a DB14 record into a viewport projectile; other profiles still
  require the complete `DM2_STEP_MISSILE` owner handoff.

- **DM2-SKSAVE-ORIGINAL-WRITER:** `dm2_v1_world_state_serialize()` is now
  deliberately fail-closed. Port SK-projects
  `SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_SAVE` and its complete
  `DM2_SUPPRESS_WRITER` graph before enabling save output: the source writes
  the 0x3c save block, global state, heroes, timers and dungeon sections in
  source order. Do not introduce a Firestaff-private envelope or minimap
  extension as a substitute for an original `SKSave.dat`. **2026-07-31
  update:** the M11/runtime quick-save route and its `SKSave.runtime`
  sidecar are now removed from production. A save command returns the explicit
  `DM2 ORIGINAL SAVE WRITER REQUIRED` result before creating a save directory,
  exporting a session, or changing an existing original resume payload.
  **2026-07-31 follow-up:** the remaining public compact-session writer APIs
  now return `DM2_V1_SESSION_WRITE_ORIGINAL_WRITER_REQUIRED` too. Test/browser
  fixtures no longer create D2RS saves; only source-format import material is
  admitted while the original writer remains incomplete.
  **2026-08-13 source-graph update:** the isolated source-order save
  orchestrator now rejects an incomplete callback graph before emitting the
  0x2a header, and rejects missing raw record/map blocks, invalid hero/timer
  counts and missing timer material instead of treating them as empty. This
  tightens the test-only writer study against partial or synthetic saves; it
  does not admit the orchestrator to M11/runtime, because the live DB pools,
  timer queue, possession links and complete `DM2_GAME_SAVE` state owner are
  still absent. **2026-08-07 real-corpus audit:** with
  `FIRESTAFF_DM2_SKSAVE_CORPUS` set, the writer-gate regression now attempts a
  quicksave against mounted `sksave0.dat`; it must return
  `ORIGINAL_WRITER_REQUIRED` before writing and preserve the source
  fingerprint. **2026-08-13 production-boundary audit:** the bounded
  `dm2_v1_world_state.c` projection and its null writer are now excluded from
  the production DM2 archive; save/minimap tests retain the explicit seam
  until the complete `DM2_GAME_SAVE` graph is ported.
  **2026-08-07 corpus audit:** the supplied DOS data root contains eight
  original `sksave0..3.dat/.bak` files (51,521–51,574 bytes). The real-data
  suite passes 126/126 and confirms the source raw prefix, fixed SUPPRESS
  order, timer boundary, DB-pool receipts and fail-closed runtime handoff;
  the focused source-order orchestrator passes 5/5. These are diagnostic
  proofs only: no live writer owner exists yet for the complete graph.

## Active DM1/CSB Symbol Queue

- **DM1-PC34-SAVE-CORPUS-FINGERPRINT:** Closed 2026-07-30. The successful
  F0435 -> F0433 corpus receipt now carries a stable, nonzero FNV-1a
  fingerprint over each admitted original input and its transient export.
  The old report never populated `roundtrip_hash`, so an admitted corpus was
  incorrectly reported as `00000000`. The external-corpus gate now rejects a
  zero successful fingerprint and preserves its independent provenance hash.

- **DM1-V1-SAVED-PORTRAIT-INVENTORY-ORDER:** Closed 2026-07-30. The normal
  C017 inventory path now restores F0435's saved M516 32x29 portrait only
  after F0355's F0291 slot pass, matching PANEL.C's final F0292 state draw.
  This prevents C509 from overwriting the lower portrait edge in resumed
  original PC34 saves. The fixture-free external-corpus runtime test passes.

- **DM1-PC34-M516-SLOT-ROUNDTRIP:** Closed 2026-07-31. F0433 now writes
  `CHAMPION_EXCLUDING_PORTRAIT::Slots` in ReDMCSB's persisted M516 order,
  rather than Firestaff's panel-layout order. The reciprocal F0796 importer
  uses the same map. Thus a changed ready/action hand or equipment slot stays
  in its original PC3.4 ordinal through export and re-import. Verification:
  native exporter/importer, F0435/F0433 handoff, tail-less backed corpus, and
  real backed-corpus roundtrip tests pass. References: ReDMCSB `DEFS.H`,
  `LOADSAVE.C F0433/F0435`; DMweb saved-game format:
  `http://dmweb.free.fr/community/documentation/file-formats/saved-game-files/`.

- **DM1-PC34-A6FA-F0337-NATIVE-PALETTE:** Closed 2026-07-31. The externally owned PC3.4 save
  `DMSAVE-dosboxx-runtime.DAT` (FNV-1a `a6fa347b`, SHA-256
  `ab7bb4a34b77bba033d7b6c31db32e7198a962b0e55c0644c0486f50bb361ecb`) has
  been copied byte-for-byte into an original `DM.EXE` DOSBox session. The
  native resumed frame is visibly lit while Firestaff's admitted F0435 frame
  was previously dark. The C2 M516 slot-order defect is now fixed; a fresh
  Firestaff frame is visibly lit and follows the same resumed runtime route
  as the original. The save contains no equipped torch or magical light, so
  no synthetic torch/light value was introduced. V1, V2.0, V2.1 and V2.2 all
  start the exact save; the V2 suite passes 87/87. References: ReDMCSB
  `LOADSAVE.C F0435`, `PANEL.C F0337`,
  `DEFS.H CHAMPION_EXCLUDING_PORTRAIT`; DMweb PC saved-game format:
  `http://dmweb.free.fr/community/documentation/file-formats/saved-game-files/`.

- **LAUNCHER-FULL-WINDOW-PRESENTATION:** Closed 2026-07-30. The start menu
  now uses a full host-window presentation rectangle and matching pointer
  transform, rather than inheriting 4:3/content letterboxing from a selected
  game. The branded pre-menu intro uses the same full-window rule. Runtime
  game frames explicitly restore their configured aspect. Focused Ninja/CTest
  verification passes.

- **DM1-V1-ENTRANCE-CREDITS-PALETTE:** Closed 2026-07-30. F0442 now blits
  the decoded PC34 C005 credits page and selects the explicit G0019 credits
  palette. The old route mistakenly treated the receipt's availability flag
  as a palette index, selecting the Entrance palette; it no longer falls back
  to a generated credits card when real C005 is present. Focused CTest passes.

- **DM1-DIRECT-LAUNCH-OPTION-INITIALIZATION:** Closed 2026-07-30. The
  Phase-A defaults now explicitly clear optional `savePath` and
  RetroAchievements endpoint fields. An unset `savePath` had been read as a
  stray pointer, causing `--game dm1` to fail after DUNGEON.DAT loaded by
  attempting an unintended save resume. New-game and explicit PC34-save
  direct launches both pass.
  The real V2.0/V2.1/V2.2 presentation smoke that previously stopped before
  its presented capture now also passes across all six focused render lanes.

- **DM1-V1-STAIRS-C10-COMPOSITION:** Closed 2026-07-30. M11's live F0104/
  F0105 stairs route now uses ReDMCSB `DEFS.H` `C10_COLOR_FLESH` as its
  transparency key, just like the corresponding pit route. It previously
  keyed index zero and painted the source stair bitmap's mask over the HoC
  floor/wall backing. Real GRAPHICS.DAT material, focused stairs CTests, and
  a dummy-video DM1 launch pass.

- **DM1-V1-STARTUP-VISUAL-CADENCE:** Closed 2026-07-30. The C001 PRESENTS
  interval again retains all 30 source-side preparation slots rather than
  donating them to the macOS zoom-frame dwell. The Entrance curtain now holds
  its black source frame for one VBlank before C004 appears. Focused startup
  timing/palette CTest verification passes 3/3.

- **DM1-V1-HUD-SOURCE-GATE-REFRESH:** Closed 2026-07-30. The action/spell,
  champion-name and M648 presentation gates now inspect the current
  source-owned M653 glyph route, data-driven spell plan and material-bound
  inscription font loader rather than retired inline helpers. Focused CTest
  verification passes 3/3.

- **DM1-V1-F0128-SIDE-F0115-DISPATCH:** Closed 2026-07-30. The late M11
  side-content pass no longer re-applies a host lane-visibility heuristic
  after ReDMCSB `DUNVIEW.C F0128` has explicitly dispatched D3L/D3R,
  D2L/D2R and D1L/D1R. Side objects, creatures and projectiles therefore
  retain their original per-square F0115 route; nearer panels overpaint in
  source order. Focused renderer source gates pass 3/3.

- **DM1-V1-F0209-REAL-C04-TEST-FIXTURES:** Closed 2026-07-30. The affected
  G0378/G0379/G0381/G0382/G0383 regressions now construct the loaded compact
  SquareFirstThing chain, raw C04 identity, C29/C37 creature type and active
  group ownership that ReDMCSB `GROUP.C F0209` requires. They no longer admit
  a group at invented event coordinates. The F0733 wound-defense contract
  also includes `CHAMPION.C F0313`'s final half-scale. Focused Ninja/CTest
  verification passes 5/5.

- **CSB-V2X-CURRENT-MAIN-VERIFICATION:** Closed 2026-07-30. Ninja-built
  current `main` passes 52 of 53 registered CSB V2.x contracts with the
  local PC3.4 corpus. The one skip is intentional:
  `csb_v22_source_artpack_runtime` refuses the installed source export
  because its unfinished wall/floor routes are explicitly `unbound`; V2.2
  resolves to V2.1 rather than painting guessed material. This verifies
  implemented V1/V2.0/V2.1/V2.2 presentation, settings, HUD, input, title
  and Entrance boundaries, not the still-open full V2.2 F0128 recovery.
  2026-07-30 full CSB verification additionally passes all 118 runnable
  V1/V2.x contracts against the supplied PC3.4 and Atari-ST corpora; the
  same unbound V2.2 source-artpack contract is the sole intentional skip.

- **CSB-V1-ENTRANCE-PREOPEN-REAL-COMPOSITION:** Closed 2026-07-30. The
  source-defined `OPENING_DELAY` plan now composes the real C004 screen and
  closed C002/C003 strips exactly as the preceding Entrance wait screen.
  Firestaff previously rejected that valid plan at the host-surface boundary,
  producing an avoidable black frame before door motion. The F0128 handoff
  now accepts the same checked C004 aperture during the delay; opening frames
  retain their separate clipped-strip contract. Verified by
  `csb_v1_boot_title_import_ui_gate_pc34_compat` and
  `csb_v1_startup_entrance_pointer_pc34_compat`.

- **CSB-V1-CSBWIN-TAG0088B2-PROJECTION-RECTANGLES:** Closed 2026-07-30.
  `csb_v1_csbwin_viewport_wall_projection_rectangle()` now locks all fourteen
  visible CSBWin wall lanes to the exact `Viewport.cpp:2304-2317`
  `wallRectangles[]` selection: F3L2=13, F3L1=1, F3=0, F3R1=2, F3R2=12,
  F2L1=4, F2=3, F2R1=5, F1L1=7, F1=6, F1R1=8, F0L1=10, F0=9, F0R1=11. The
  F0 centre is explicitly projection-only because CSBWin has no separate
  `StdBitmapPointers` wall source for it; F3R2's bitmap mirror remains
  separately source-locked. Verified by the
  CSBWin layout test against the local Atari-ST `GRAPHICS.DAT`. 2026-07-30:
  item `0x22e` now also decodes every raw byte-coordinate `RectPos` at the
  exact `Byte7248 + 2` -> `Byte3074` offset: inclusive destination, packed
  source stride/height, and source x/y. The deliberately source-less F0
  local-cell rectangle is explicitly admitted only in its all-zero form.
  This locks command geometry, not the still-open native-raster V2.2 material
  handoff.

- **CSB-V1-CSBWIN-TAG0088B2-WALL-PLAN:** Closed 2026-07-30. Firestaff now
  joins CSBWin's exact `Viewport.cpp` source-bitmap selection with the decoded
  `0x22e` destination rectangle for all thirteen bitmap-backed lanes in a
  WallSet. The F3R2 mirror and the source-less F0 local cell remain explicit.
  The focused contract and the local Atari-ST `GRAPHICS.DAT` exercise all four
  available WallSets. This is command recovery only; M11 must still consume
  the plan through CSBWin's original packed-byte blitter semantics.

- **CSB-V1-CSBWIN-TAG0088B2-PACKED-BLIT:** Closed 2026-07-30. The restored
  Atari four-plane raster path now executes the source-owned indexed boundary
  of `Graphics.cpp::TAG0088b2` for a recovered wall command: inclusive
  destination, byte-stride-derived source width, source x/y, color-10
  transparency and CSBWin's F3R2 mirror are all explicit. It refuses F0's
  source-less local-cell rectangle. This is a reusable command primitive;
  final M11 frame scheduling and all non-wall F0128 families remain open.

ReDMCSB is the primary reference for DM1/CSB shared engine behavior. For
CSB-specific DSA, save, Utility Disk, and extended-runtime behavior, use
CSBWin alongside it. Reuse or bind verified PC34 owners and authentic game

- **CSB-V1-PROJECTILE-MARKER-LEAK:** Closed 2026-07-30. The live PC3.4
  F0115 projectile route now fails closed when its authenticated bitmap drawer
  cannot materialize a projectile. It no longer draws the old coloured
  diagnostic cross into the source-owned page, which could land at an invalid
  viewport offset and make V1/V2 runtime captures nondeterministic. Markers
  remain available only to data-free geometry probes. The phase-3 viewport
  regression proves the source-bound no-marker contract; repeated V2.0/V2.1
  raw-page capture checks remain the integration gate.

- **CSB-V1-VARIANT-IDENTITY-DISCOVERY:** Closed 2026-07-30. A recognised
  CSB launcher/probe variant token now restricts graphics discovery to that
  variant's verified original MD5 (including both documented Atari ST 2.x
  floppy and hard-disk identities) and rejects a filename-only archive from a
  different platform. Unknown/custom tokens retain the prior hash-first,
  filename-fallback search needed for user-managed custom layouts. The boot
  handoff regression covers every documented token and the mixed-layout
  failure boundary.

- **CSB-V22-ARTPACK-PROVENANCE-HARDENING:** Closed 2026-07-30. CSB V2.2
  now accepts only safe single-component category, asset and source-file
  names when resolving an imported artpack. Compact Artpack Studio entries
  now consume `id`/`source_file` before their closing brace; route provenance
  likewise accepts both pretty-printed and fully single-line JSON entries.
  Route
  provenance requires a well-formed 64-digit SHA-256 identity rather than
  merely a 64-character string. The live F0128 door command now also carries
  the SHA-256 of its exact compressed `GRAPHICS.DAT` catalog record, which
  must equal that manifest identity before V2.2 replaces source pixels. The
  focused asset, in-place route/draw, and real PC3.4 V2.2 startup/runtime
  regressions pass. This hardens the existing V2.2 admission boundary; it
  does not promote still-unbound F0128 material families.
material only. Unproven paths fail closed; no synthetic graphics, UI, timing,
input, or game-data behavior.

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

- **DM1-ORIGINAL-REPLACE-003:** Closed 2026-07-30. Normal DM1 V1 inventory
  slots no longer substitute generated slot frames, scaled viewport sprites,
  type tags or position labels when C033-C035 or the F0038 icon atlas is
  unavailable. The affected source rectangle remains blank until the real
  material is decoded; diagnostic and non-DM1 routes retain their tooling.

- **DM1-ORIGINAL-REPLACE-004:** Closed 2026-07-31. Removed the unused V2.2
  magenta/black missing-shape bitmap and its public accessor. V2.2 now has
  no generated replacement surface in the production asset library: an
  unadmitted or incomplete custom pack follows the existing source-backed
  V2.1 -> V2.0 -> V1 presentation chain. Focused V2.2 material, runtime and
  source-owned screenshot gates pass.

- **DM1-ORIGINAL-REPLACE-005:** Closed 2026-07-31. The diagnostic HUD no
  longer repaints a real DM1 V1 `GRAPHICS.DAT`/`DUNGEON.DAT` viewport with the
  old procedural corridor, trapezoid, or tiled-strip renderer. DM1 and CSB
  source sessions retain their composed source-owned frame; the diagnostic
  renderer remains available only for non-source tooling. Verification:
  `firestaff_m11` and `test_m11_dm1_runtime_source_capture_receipt` pass.

- **DM1-ORIGINAL-REPLACE-006:** Closed 2026-07-31. The production audio
  resolver now checks the selected DM1 data directory for the authenticated
  PC3.4 `SONG.DAT` before legacy locations. Real SND8 title music can no
  longer be missed merely because `GRAPHICS.DAT` and `SONG.DAT` share
  `~/.firestaff/data/dm1`. Verification:
  `test_dm1_v1_swsh_psg_audio_pc34_compat` and
  `test_dm1_v1_f0740_f0743_music_source_gate` pass against local original
  PC3.4 data.

- **DM1-ORIGINAL-REPLACE-007:** Closed 2026-07-31. The source-owned V1
  action/spell strip no longer substitutes a host-drawn light meter when its
  `GRAPHICS.DAT` material is unavailable. The original route now keeps the
  strip blank, while the meter remains only for explicit debug or legacy
  non-V1 chrome. Verification: `firestaff_m11` and
  `test_m11_dm1_runtime_source_capture_receipt` pass.

- **DM1-ORIGINAL-REPLACE-008:** Closed 2026-07-31. Removed the generic
  footstep, door, combat and spell marker sounds from DM1 tick emissions.
  ReDMCSB routes audible DM1 effects through
  `F0064_SOUND_RequestPlay_CPSD` with a concrete SND3 index; emissions without
  that index are now source-silent rather than generating a substitute cue.
  Verification: `test_dm1_v1_sound_pc34_compat_integration` (283/283) and
  `test_dm1_v1_swsh_psg_audio_pc34_compat` pass.

- **DM1-ORIGINAL-REPLACE-009:** Closed 2026-07-31. Retired the unused V2
  floor-item placeholder identity and synthetic rarity/glow helpers. The
  remaining item metadata is source-contract only: no drawable fallback is
  admitted before authentic PC34 material or a user-selected artpack asset is
  available. Verification: `test_dm1_v2_item_render_pc34`.

- **DM1-ORIGINAL-REPLACE-010:** Closed 2026-07-31. Restored ReDMCSB
  `DUNVIEW.C` F0127's D0C `F0115(C0x0021)` pass. The prior consumer rejected
  the D0C G2028/C2500 row 1, so real items and projectiles in the party
  square could be absent and item pickup had no exact F0791 target. The active
  PC34 route now consumes cells 0/1 with source row 1 before the F0113 field
  overlay. Verification: `test_dm1_v1_viewport_3d_pc34_compat` and
  `test_m11_dm1_f0115_floor_item_runtime_capture_pc34` with local PC34 data.

- **DM1-CREATURE-VIEWPORT-REAL-DATA:** Closed 2026-08-06. The legacy
  creature-viewport helper's fixed 225..297 sprite table remains test-only and
  is not linked into the production M10/M11 archives. The production F0115
  route now consumes the authenticated G0219/G0243 native C584+ selection and
  materializes missing D2/D3 derived-cache slots from the real native raster
  through the source 21/32 and 14/32 F0675/F0129 scales. Its focused fixture
  tests remain available for contract diagnostics.

- **DM1-ORIGINAL-REPLACE-011:** Closed 2026-08-05. Removed the remaining
  legacy viewport renderer's synthetic coloured wall, floor, ceiling, side
  wall and door pixels. Missing source material now remains no-draw; the
  source-backed M11/CSB renderers remain the only owners of those surfaces.
  This matches the DOS manual's interaction evidence without treating the
  manual as a graphics source. Verification: Ninja `firestaff` target builds;
  `git diff --check` passes.

- **DM1-ORIGINAL-REPLACE-012:** Closed 2026-08-05. Normal DM1 source sessions
  no longer enter the legacy utility-panel fallback when authenticated C009
  action-area or C010/C011 spell-area material is unavailable. The fallback
  cyan frame, host champion/status text and diagnostic light bar remain
  available only for non-source/debug sessions; DM1 now leaves the source
  surface untouched until real GRAPHICS.DAT material is admitted.
  Verification: Ninja `firestaff` target builds; `git diff --check` passes.

- **DM1-ORIGINAL-REPLACE-013:** Closed 2026-08-05. Bound the live M11
  creature compositor to the source G0243/F0695 replacement-color targets.
  G0221/G0222 depth mapping remains intact, while source palette slots 9 and
  10 now use the verified creature-specific destinations; creatures without
  replacement sets retain the original mapping. Verification: Ninja
  `firestaff` target and `test_dm1_v1_creature_render_pc34_compat_integration`
  pass.

- **DM1-ORIGINAL-REPLACE-014:** Closed 2026-08-05. Removed generated DM1
  palette colours from the legacy GRAPHICS.DAT reader, the full-palette
  fallback and the unreachable game-loop fallback. PC-34 is 4bpp here: the
  authenticated 16-colour palette remains available, extracted VGA data is
  still used when present, and indices 16-255 now remain unavailable/no-draw
  instead of being filled with a synthetic ramp or grayscale. Verification:
  Ninja `firestaff` build and `git diff --check` pass.

- **DM1-ORIGINAL-REPLACE-015:** Closed 2026-08-05. Replaced the DM1 V2.2
  shape bridge's graphic-0 texture placeholders with the matching PC-34
  GRAPHICS.DAT indices used by M11 for wall depths, floor, door frame,
  stairs and teleporter field. Shape routes with no single authenticated
  source record (D0 center wall, cracked/mossy/pit variants and modern
  normal/specular/emission maps) now carry an unavailable ID and stay
  source-fail-closed. Verification: `test_dm1_v22_verification` passes,
  including the source-index assertions; no modern artpack is fabricated.

- **DM1-ORIGINAL-REPLACE-016:** Closed 2026-08-06. Retired the legacy M11
  V2.2 overlay's synthetic material-colour rectangles, palette shadowing and
  border pixels. The compatibility API now always returns no-draw; only the
  authenticated in-place V2.2 asset renderer may write modern pixels. Shared
  source viewport rectangles remain available for that renderer. Verification:
  `test_m11_v22_render_overlay_pc34` passes with populated V2.2 cache data and
  an unchanged framebuffer; no placeholder palette index remains in this path.

- **DM1-ORIGINAL-REPLACE-017:** Closed 2026-08-06. Retired the separate DM1
  V2 HUD overlay's hard-coded 5x5 font, invented champion names, procedural
  status/action/rune rectangles, compass, and fixed 75% meter. The API keeps
  presentation state and timing signals but is strict no-draw until decoded
  PC34 M653/C009/C010/C011 surfaces are supplied. Verification:
  `test_dm1_v2_hud_overlay_pc34` proves populated state leaves the framebuffer
  unchanged and cites the authenticated source owners.

- **DM1-ORIGINAL-REPLACE-018:** Closed 2026-08-06. Removed the DM1 V2.2
  pipeline's 16x16 magenta/cyan missing-art bitmap and descriptor. Missing
  modern art now returns `NULL` with zero dimensions, so it cannot be treated
  as valid RGBA or reach a renderer. PNG discovery remains metadata-only until
  a real pixel decoder is bound; that path stays no-draw instead of using a
  checkerboard. Verification: `test_dm1_v22_asset_pipeline` asserts the NULL
  result and existing descriptor validation still passes.

- **DM1-ORIGINAL-REPLACE-019:** Closed 2026-08-06. Bound the DM1 V2.2 modern
  asset loader to a real zlib PNG decoder for the shipped non-interlaced 8-bit
  RGB/RGBA assets. It validates the PNG structure, inflates all IDAT chunks,
  applies PNG filters 0-4, expands RGB to opaque RGBA, and rejects unsupported
  variants instead of fabricating pixels. Verification: the V2.2 pipeline
  builds and `test_dm1_v22_asset_pipeline` passes; unsupported formats remain
  no-draw.

- **DM1-ORIGINAL-REPLACE-020:** Closed 2026-08-06. Removed the remaining
  procedural V2 particle emitters, full-screen spell overlays, dust/sparkle
  field effects, and dynamic-light indexed repaint from the DM1 presentation
  route. ReDMCSB owns projectile/explosion bitmaps, field bitmaps, and the
  F0337 palette; Firestaff now leaves those surfaces to the authenticated V1
  source renderer and returns no-draw for unbound V2 effects. Verification:
  focused enhanced-effects, spell-overlay, and extended-field tests prove
  generated state does not advance or write pixels.

- **DM1-ORIGINAL-REPLACE-021:** Closed 2026-08-06. Replaced the DM1 V2.1
  viewport renderer's hard-coded EGA-like palette and linear brightness
  attenuation with the authenticated PC34 six-level VGA table from ReDMCSB
  `VIDEODRV.C` (`G8149/G8151-G8156`, exposed as
  `G9010_auc_VgaPaletteAll_Compat`). The indexed framebuffer's high nibble
  now selects the source brightness row, preserving the independently tuned
  wall, item and creature colours. Verification: the four direct-renderer
  CTest targets `dm1_v2_source_route_state_hash_pc34`,
  `dm1_v2_launch_smoke_pc34`, `dm1_v2_viewport_materials_pc34` and
  `dm1_v2_per_mode_material_signatures_pc34` pass; no generated palette
  remains in this route.

- **DM1-ORIGINAL-REPLACE-022:** Closed 2026-08-06. Removed M11's duplicate
  27-entry creature display-name table and routed runtime creature names
  through the ReDMCSB source-owned `dm1_creature_type_name()` implementation.
  Invalid type IDs now fail closed as `UNKNOWN` instead of inventing a
  creature label. Verification: `test_dm1_v1_creature_render_pc34_compat`
  passes 14/14 and `m11_dm1_runtime_source_capture_receipt` passes.

- **DM1-ORIGINAL-REPLACE-023:** Closed 2026-08-06. Fixed the production DM1
  launch path for hash-verified archive members. Real ZIP `DUNGEON.DAT` and
  sibling `GRAPHICS.DAT` entries are now materialized into the DM1 runtime
  cache before the ReDMCSB world and graphics loaders open them; filename-only
  admission remains forbidden. Verification against
  `~/.firestaff/data/dm1` passes `test_m11_dm1_real_object_names` and
  `test_m11_dm1_real_object_corpus` (611 records).

- **DM1-ORIGINAL-REPLACE-024:** Closed 2026-08-06. Removed the M11 font
  loader's unverified third index and its "any unique 768-byte record"
  heuristic. ReDMCSB's PC34/legacy M653 indices (`695`/`557`) are now the
  only accepted identities; a matching byte count without source identity
  fails closed instead of promoting unrelated GRAPHICS.DAT pixels to the
  DM1 interface font. Focused action/spell source-gate, F0342 and F0662
  tests pass. A real-data run remains dependent on the local PC34 corpus
  being installed.

- **DM1-ORIGINAL-REPLACE-025:** Closed 2026-08-06. Removed the last
  production inventory portrait silhouette fallback for authenticated DM1
  and CSB sessions. Inventory portraits now require the real saved M516
  bitmap or GRAPHICS.DAT C026 material; missing source pixels leave the
  portrait area untouched instead of drawing invented eyes/mouth pixels.
  The diagnostic silhouette remains restricted to non-source fixtures.

- **DM1-ORIGINAL-REPLACE-026:** Closed 2026-08-06. The legacy wall-face
  helper no longer decides that an authenticated DM1 session is source-owned
  only after Thing tables are populated. A real PC34 GRAPHICS.DAT loader is
  now sufficient to suppress primitive wall, door and stairs fallbacks;
  missing later F0115 Thing data therefore remains no-draw instead of
  producing host geometry. Verification: focused DM1 source-render tests and
  the real archive object corpus pass.

- **CSB-ORIGINAL-REPLACE-001:** Replace the remaining V2.2 viewport
  placeholder/legacy rectangle route with the verified Atari-ST/CSBWin
  `GRAPHICS.DAT` TAG0088b2 source material.  Do not promote the source-less
  F0 local-cell rectangle into invented pixels.  **2026-07-30 update:** the
  retired missing-modern-art checkerboard now fails closed (no surface), so
  it cannot substitute generated pixels while the remaining original-material
  bindings are recovered. **2026-07-30 update:** the retired V2 HUD bitmap
  renderer is likewise strict no-draw; PC3.4 C017/C040 and Atari ST C232 are
  the only admitted HUD pixel owners. **2026-07-31 update:** the live
  Atari-ST C232/TAG0088b2 route no longer appends the host-only status texts
  `ATARI RUNTIME READY` or `CSBWIN SOURCE FRAME - EXTENDED CELLS REQUIRED` to
  its source-owned page. The remaining work is real F0128 material binding,
  not generated diagnostic chrome. **2026-07-31 follow-up:** the V2 particle
  system's RGB ramps, trajectories, tile speeds and effect durations are now
  contract-only. The playable V2 API rejects emitter, projectile and field
  creation until a source-art receipt binds them, so a CSB spell or DSA event
  cannot become host-generated pixels. **2026-07-31 follow-up:** the V2
  lighting simulation's RGB point lights, tile falloff, sinusoidal flicker
  and DSA pulse curves are likewise contract-only. The playable API retains
  PANEL.C's authenticated palette-light amount only; it cannot make a second
  host lighting image. **2026-07-31 follow-up:** the unconsumed
  Hint Oracle layout/ASCII-sketch module is removed rather than becoming a
  generic panel placeholder; a visible Hint Oracle needs decoded source art
  and font material. **2026-07-31 follow-up:** the separately reintroduced
  graphical Hint Oracle renderer, with its invented 5×7 font, fixed palette
  and frame, is also removed together with its test/probe targets. The
  HCSB.HTC parser remains text-only until a source-owned screen route exists.
  **2026-07-31 follow-up:** the isolated first-backdrop
  fixture, which asserted fixed synthetic colours instead of decoding a
  source bitmap/mask pair, is removed. **2026-07-31 follow-up:** unbound
  runtime creature groups now leave the source page unchanged; the retired
  coloured-cross diagnostic was not a creature sprite. Unbound objects now
  likewise remain no-draw, as do unbound projectiles. **2026-07-31 follow-up:** source-verified
  CSB sessions now also reject the shared geometry-only cyan F0113
  teleporter fill. **2026-07-31 update:** live F0113 now binds C076 and its
  selected C070--C075 mask through the active hash-verified `GRAPHICS.DAT`
  decoder. **2026-07-31 follow-up:** F0113 now consumes
  `M005_RANDOM(2)` then `M003_RANDOM(32)` from the persisted ReDMCSB G349
  stream once per visible field, in draw order. PC/I34 new games begin from
  ReDMCSB `BASE.C`'s documented G349 zero state, independently of a loaded
  GAMEBLOCK2. Missing source remains no-draw rather than a
  generated rectangle. **2026-07-31 follow-up:**
  entrance-plan metadata no longer carries generated grey door
  fill or edge colours. ReDMCSB `ENTRANCE.C F0806` owns C002/C003, and a
  missing raster remains an admitted draw failure rather than a plan-level
  substitute. **2026-07-31 audit:** every rejected CSBGRAPHICS override now
  has an explicit whole-frame no-draw regression; an empty, untrusted,
  malformed or unsupported original entry cannot mutate the 320x200 source
  page or become generated chrome. Its runtime decision is explicitly named
  `rejected-no-draw`; the retired `fallback-original` alias is removed.
  **2026-07-31 V2.2 cache audit:** a
  malformed source-derived cache entry now rejects and discards the complete
  package: dimensions, exact RGBA byte count and non-wrapping data offsets
  must all validate before any bitmap can become available. Each key and RGBA
  span must also be unique, matching the source-pack cache writer; ambiguous
  material is rejected as a whole. The public V2.2 cache contract now states
  the same runtime truth: it is command-level F0128 replacement only, never
  a generic cell overlay or a PNG-on-demand route. Its FSV22C wire integers
  are decoded explicitly as Artpack Studio's little-endian format.
  **2026-07-31 renderer-boundary audit:** PC3.4 emits its admitted V2.2
  replacements only inside `csb_v1_viewport_rasterize_first_frame_material`
  at the source `CSB_V1_ViewportRuntimeDrawCommandPc34` boundary. Atari ST /
  CSBWin instead uses `m11_csb_present_atari_st_runtime_viewport`, which
  consumes `CSB_V1_CSBWinViewportMaterialPlan` and TAG0088b2-decoded records
  directly. **2026-07-31 material-plan follow-up:** this plan now binds the
  selected floor, ceiling, C232 palette item and all thirteen source-backed
  wall commands before M11 decodes any pixel; F0 remains deliberately absent.
  The optional Atari real-data regression covers all 16 floor-set × 16
  wall-set selections and rejects a PC-index substitution. Its material hash
  now serializes source fields canonically, independent of host struct layout.
  The remaining bridge must translate that Atari plan into the same
  source-owned F0128 command contract with its own graphic index, projection
  and palette receipt; it must not reuse PC3.4 indices or add a post-draw
  rectangle/overlay.
  **2026-07-31 accounting follow-up:** the direct Atari TAG0088b2 page now
  clears PC3.4 V2.2 paint accounting after its source layout and graphics
  plan have verified, and the real Atari-ST handoff regression asserts that
  boundary. This prevents stale PC compositor telemetry from being attributed
  to an Atari source page. **2026-07-31 palette follow-up:** Atari ST runtime
  pages now explicitly reject the PC3.4 VGA palette and consume
  `GRAPHICS.DAT` item `0x232`'s source `Palette552[0]`, following CSBWin
  `ReadTablesFromGraphicsFile()` and its initial `setpalette` call. ReDMCSB
  `PALETTE.C F1125/F0436` and CSBWin `SelectPaletteForLightLevel` remain the
  required owner for later light-level palette transitions; no PC3.4
  approximation is admitted. **2026-07-31 light-selection follow-up:** item
  `0x232` now also decodes CSBWin's six `PaletteBrightness` thresholds and
  sixteen `Word1074` torch powers. M11 consumes the exact four-brightest-
  torches selector only from an authenticated CSBWin saved runtime body,
  loaded original dungeon, current map multiplier, saved `Brightness` and
  both source hand records for each champion; otherwise it retains the real
  `Palette552[0]` boot row. **2026-07-31 follow-up:** the retired raw-cell V2.2
  classifier now returns no shape, category or asset for every input, and its
  legacy 3x3 renderer is permanently no-draw. A populated cache or installed
  artpack cannot activate this compatibility route; only the source-command
  compositor may admit a replacement. **2026-07-31 Phase 4 follow-up:** the
  former modern spell-VFX table had host-invented spell ids, colours, radii,
  speeds and particle families. It is now no-admission; ReDMCSB's real DATA.C
  torch and palette tables remain available, but may not be expanded into a
  modern effect until a source command and material receipt binds every value.
  **2026-07-31 Chaos follow-up:** the V2 runtime still advances its chaos
  compatibility hook, but trigger, projectile and overlay entry points are
  now permanently transparent/no-draw. A DSA script id alone cannot select a
  purple glow, particle family, host speed or lighting curve.
  **2026-07-31 minimap follow-up:** the dormant CSB V2 minimap's hard-coded
  room colours and magenta DSA marker now return transparent. Map semantics
  alone are not an original UI surface or palette transaction.
  **2026-07-31 binding follow-up:** the dormant Phase 4 binding layer now
  rejects every projectile, field, chaos and torch request. Its former RGB
  fackelfärger, radier, emitterkurvor och effektider var host-skapade, inte
  källdata.
  **2026-07-31 viewport-light follow-up:** the public V2 viewport lighting
  bridge now also rejects arbitrary RGB torches and DSA pulses. ReDMCSB light
  tables stay available as data, but cannot become host-rendered effects. Its
  phase-gated runtime wrapper is likewise observability-only: an enabled tick
  reaches the no-draw palette boundary and never creates a visible light map.
  **2026-07-31 D2C follow-up:** the contract-only synthetic C10 field blit
  has been removed from the product API. The remaining D2C record describes
  ReDMCSB routing and zones only; real F0113 pixels still require source data.
  **2026-07-31 D2L2 follow-up:** the matching synthetic C10 projectile blit
  is also removed. Its F0115 row, zone and no-draw contracts remain intact.
  **2026-07-31 D2C door follow-up:** the partly-open-door C10 test blit is
  removed. Its probe now checks only source route, frame selection, zones and
  transparency metadata; no synthetic door page can be produced.
  **2026-07-31 D3C follow-up:** the contract-only C10 teleporter-field blit
  is removed. D3C retains its source route and zone metadata only.
  **2026-07-31 D2L2/D2R2 door follow-up:** the remaining contract-only C10
  partly-open-door blit is removed. Its F0111 branch, zone, clip and
  transparency metadata remain, but unbound material cannot write a viewport.
  **2026-07-31 D1L2/D1R2 door follow-up:** the D1 side-door C10 fixture blit
  is removed as well. ReDMCSB F0111/F0122/F0123 routing and zone metadata
  remain, while an unbound frame has no pixel writer.
  **2026-07-31 D0L2/D0R2 door follow-up:** the mirrored D0 C10 fixture blit
  is removed. Its source order, mirroring, zones and keepout contracts remain
  available without a framebuffer writer.
  **2026-07-31 D0L2/D0R2 wall follow-up:** removed the fabricated names and
  synthetic source-lock artifact. ReDMCSB has D0L/D0R, not D0L2/D0R2.
  **2026-07-31 D2L2/D2R2 partly-open follow-up:** the remaining C10 fixture
  writer is removed. The D2 panel clip and ReDMCSB transparency metadata stay
  available, but no unbound input can write a viewport.
  **2026-07-31 D2L2/D2R2 F0111 door follow-up:** removed the fabricated
  partly-open door route. ReDMCSB's D2L2/D2R2 wall cases return before F0111.
  **2026-07-31 D2L/D2R wall follow-up:** the C10 wall-frame fixture writer is
  removed. ReDMCSB frame dimensions, source placement and transparency
  metadata remain available without a synthetic viewport surface.
  **2026-07-31 D3L/D3R wall follow-up:** removed the unconsumed contract-only
  trace, including its synthetic transparency simulation. Runtime D3 side-wall
  ownership remains the existing source-bound wall-set handoff.
  **2026-07-31 D3L/D3R backdrop follow-up:** removed the duplicate synthetic
  backdrop trace; it had no production consumer or original material binding.
  **2026-07-31 D3L2/D3R2 thing follow-up:** the F0115 fixture blit is removed;
  source route, cell and dynamic-flip metadata remain available without pixels.
  **2026-07-31 D2L2/D2R2 wall follow-up:** the F0104/F0105 C10 fixture blit
  is removed. Wall routes, zones and C10 transparency remain metadata-only;
  unbound CSB material cannot write viewport pixels.
  **2026-07-31 D0L2/D0R2 F0115 follow-up:** the single-pixel fixture writer
  is removed. Real `CSBgraphics.dat` teleporter-field composition remains the
  only drawing route; contract helpers retain geometry and C10 metadata.
  **2026-07-31 D0L2/D0R2 wall follow-up:** the F0104/F0105 synthetic wall
  pixel writer is removed. The contract retains C716/C717 geometry, mirroring
  and C10 metadata but cannot write without verified source material.
  **2026-07-31 D2C door follow-up:** the unbound F0111 C10 fixture blit is
  removed. State, half-zone and transparency metadata remain source-locked;
  the local PC3.4 material provider is required for an actual draw.
  **2026-07-31 D3C wall follow-up:** removed the contract-only F0101 pixel
  writer. ReDMCSB F0118/G0163/G0698 geometry and blend metadata remain
  source-locked; drawing resumes only when a verified GRAPHICS.DAT binding
  supplies the original wall-set material.
  **2026-07-31 D3L2/D3R2 door follow-up:** the fixture C10 blit is removed.
  The existing real `GRAPHICS.DAT` receipt remains the material admission
  path; metadata alone cannot emit pixels.
  **2026-07-31 D2L2/D2R2 F0115 follow-up:** the generic C10 fixture blit is
  removed. Items and explosions draw only through the hash-bound real overlay
  compositor; F0115 row metadata remains available without pixels.
  **2026-07-31 D1L/D1R door follow-up:** the procedural fixture blit and its
  synthetic render hash are removed. The hash-verified `GRAPHICS.DAT` item
  248 receipt remains the sole material gate for a D1 side-door draw.
  **2026-07-31 D3L2 F0115 projectile follow-up:** the unbound C10 fixture
  blit is removed; source routing, kinetic scaling and flip metadata remain
  available without a fabricated projectile raster.
  **2026-07-31 D1C door-frame follow-up:** removed the orphaned generic C10
  frame blit. The already-bound M659/G2112 and M655/G2117 production route
  is unchanged; contract metadata cannot accept caller-supplied pixels.
  **2026-07-31 V2.2 modern-assets follow-up:** retired the missing-art
  placeholder API. A missing V2.2 asset now remains an ordinary failed
  material lookup; fallback selection stays with verified V1/V2 source paths.
  **2026-07-31 D3C backdrop follow-up:** retired the synthetic three-colour
  backdrop/ornament compositor. F0097/F0098/F0107/F0108/F0118 ordering,
  zones and C10 rules remain metadata-only until original material is bound.
  **2026-07-31 D1L2/D1R2 wall follow-up:** removed the duplicate synthetic
  metadata/test route. It duplicated F0122/F0123 D1L/D1R ownership without a
  consumer; the real PC3.4 material binding is now the sole owner.
  **2026-07-31 D3L2/D3R2 wall follow-up:** retired its generic C10 frame
  clipper; only source-locked zones, geometry and F0105 metadata remain.
  **2026-07-31 D1L/D1R declaration follow-up:** the stale, unimplemented
  C10 pixel-writer declaration is removed from the public contract.
  **2026-07-31 D1L/D1R material-binding follow-up:** this pair now decodes
  its current PC3.4 `GRAPHICS.DAT` wall-set records (C03/C02: entries 96/95
  for wall set 0) with the compressed-record receipt. Its prior synthetic
  frame compositor and synthetic raster runner are gone. The remaining
  viewport-material inventory stays open for routes not yet bound this way.
  **2026-07-31 active-provider follow-up:** the live M11 CSB viewport loader
  now rejects any D1L/D1R C03/C02 decode that is not the native 60x111
  raster. This closes the former arbitrary-dimension acceptance in the
  production route; it cannot cache a structurally valid but wrong wall
  record for C713/C714.
  **2026-07-31 D1L2 wall follow-up:** removed the fabricated D1L2 analogue.
  ReDMCSB exposes D1C/D1L/D1R only, so no source-owned D1L2 material exists.
  **2026-07-31 D1L2/D1R2 F0115 follow-up:** removed the matching fabricated
  thing-pass fixture. It substituted D1L/D1R rows behind non-existent names.
  **2026-07-31 CustomBackgrounds first-backdrop follow-up:** removed the
  orphaned C10 pixel writer and its test route. It only copied caller-made
  pixels, had no viewport consumer and no `CSBgraphics.dat` material receipt;
  the separate CSBWin source-lock metadata remains until the real masked
  composite is admitted.
  **2026-07-31 D1L2/D1R2 F0108 follow-up:** removed the fabricated
  floor/ceiling/ornament trace and its generated-pixel hash. ReDMCSB's
  F0122/F0123 name only D1L/D1R; the real PC3.4 material routes remain the
  sole source of viewport pixels for that pair.
  **2026-07-31 F0115 wall-text follow-up:** removed the synthetic D1C
  wall-text renderer, its made-up glyph pattern, hard-coded palette and
  verification fixture. The original F0107 inscription route must now remain
  no-draw until it receives its actual source bitmap and palette receipt.
  **2026-07-31 wall-text oracle API follow-up:** removed the unimplemented
  public synthetic-fixture declaration. It had no source, runtime or test
  consumer and could not decode a user-supplied original dungeon.
  **2026-07-31 D0 F0115 fixture follow-up:** removed the standalone
  caller-pixel C10 blend helper. The retained D0L/D0R route can composite
  only an admitted `CSBgraphics.dat` raster with its source palette receipt.
  **2026-07-31 F0134/F0135 orphan follow-up:** removed the public helper that
  could overwrite an authenticated C017/C040 HUD or C002/C003 entrance raster
  with caller-selected colours. It had no runtime consumer and no source
  command/material receipt for those mutations; the real startup and
  viewport routes remain the only pixel owners.
  **2026-07-31 F0115 orphan group/projectile follow-up:** removed two public
  direct-blit families that accepted caller-owned group or projectile buffers.
  They had no M11 runtime consumer, so their source-index checks could not
  prove pixel provenance. Active source-bound F0115 sprite drawers remain
  unchanged and suppress drawing when the required original sprite is absent.
  **2026-07-31 D0C F0111 orphan follow-up:** removed the test-only panel
  compositor and its public API. Although it validated a supplied live-frame
  receipt, no M11 route invoked it; it therefore could not establish a real
  D0C drawing path. The authenticated viewport materialization remains the
  only owner of production pixels.
  **2026-07-31 F0115 native-object boundary follow-up:** moved the remaining
  native-object pixel loop behind M11's verified asset-slot boundary and
  removed its public caller-buffer API. A CSB object now reaches that loop
  only after the live PC3.4 graphics record has been installed and decoded;
  an unbound caller can no longer supply an arbitrary object raster.
  **2026-07-31 D1C F0108 source-helper follow-up:** removed the seed-driven
  ornament trace, generated pixels, public self-test API and its data-free
  CTest target. The retained C10 and `C1500 + CoordinateSet * 11 + ViewFloor`
  helpers are exercised only against decoded, hash-verified PC3.4
  `GRAPHICS.DAT` bytes by the real-asset ornament probe.
  **2026-07-31 F0108 footprints follow-up:** removed the orphaned,
  contract-only footprints plan and its data-free CTest. It had no M11 or
  real-data consumer and could not decode or bind an original ornament
  raster; active source-bound F0108 paths remain the only CSB pixel owners.
  **2026-07-31 D2C partly-open-door follow-up:** removed the duplicate,
  contract-only F0111 metadata/probe surface. It had no M11 consumer and no
  original bitmap decoder; the active D2C F0111 source-material route remains
  the sole production owner.
- **DM2-ORIGINAL-REPLACE-001:** Replace the V1 viewport's placeholder wall
  and door passes with decoded `dm2/GRAPHICS.DAT` GDAT records selected by
  the live `DUNGEON.DAT` graphics set.  Missing/unsupported GDAT image forms
  must remain no-draw until their original decoder is implemented.
  **2026-07-31 update:** item and projectile receipts now agree with that
  no-draw rule: a missing map chip is reported as unrendered, not as a
  fallback draw. Remaining viewport families still require their live GDAT
  owners and decoders.
  **2026-07-31 update:** a wall-ornament placement plan must now name the
  exact WALL_GFX GDAT row derived from its live G1 square. A mismatched plan
  blocks the frame rather than borrowing a same-square bitmap at an unowned
  placement. **2026-07-31 update:** invalid `UPDATE_GFXSET` control receipts
  (missing hash or an out-of-range graphics-set selector) now clear the scene
  and light plans rather than silently falling back to graphics set zero.
  **2026-08-06 update:** the remaining local DB2/DB3 wall-button walkers are
  now fixture-only. A mounted M11 source provider may use only the
  authenticated `dm2_v1_dungeon_*` record chain; if that chain cannot bind a
  button, the button is absent instead of obtaining a guessed WALL_GFX
  selection. **2026-08-06 follow-up:** removed those local compatibility
  walkers from the linked runtime as well. The only remaining wall-button
  routes are the authenticated dungeon graph and the G1 direct-material
  receipts; a callback-selected viewport provider can no longer reactivate a
  fixture-only DB2/DB3 inference. Complete the G1 record-chain ownership
  before restoring any missing source button. **2026-08-06 weather inventory update:** the unused
  generic rain/fog/storm planner with fixed colours and generated commands
  has been removed, together with its otherwise unused viewport selector
  fields. The only remaining weather draw route is the original GDAT
  `ENVIRONMENT`/`DistantEnvironment` receipt chain; broader real-session state
  ownership is still required before all original weather paths can be shown.
  **2026-08-06 HUD ownership update:** the legacy game loop no longer invokes
  the V2 HUD renderer after declaring M11 the DM2 presentation owner. M11's
  frame-receipt gate is now the only production route that may issue the
  GDAT-backed V2 HUD pass; keep future HUD work on that route.
  **2026-08-07 DoorType-0 parity correction:** the source door record's
  `DoorType()` value may legitimately be zero. The viewport now uses the
  separate authenticated G1 DB0-root receipt to decide whether to select the
  record-specific `DOORS` image, rather than treating `door_record_type != 0`
  as record presence. Remaining wall/door work is still limited to source
  records and decoded GDAT/RAW4 receipts.
  **2026-08-13 real-wall address audit:** the canonical PC-English wall-plan
  regression now verifies every emitted wall command round-trips through the
  live `GRAPHICSSET/<MapGraphicsStyle>` address and exact viewport field.
  A default-set or mismatched command cannot satisfy the real-data wall draw;
  unresolved wall/door material remains no-draw.
  **2026-08-06 V5 corpus correction:** canonical PC `GRAPHICS.DAT` does have
  an authenticated FB/FC/FD chain for `CREATURES/02/dtImage/12`; its current
  G1 maps do not contain a root that owns it. Keep every present G1 creature
  fail-closed until its own DB4 record/animation chain is bound; do not infer
  a sprite from this unrelated real material.
- **DM2-ORIGINAL-REPLACE-002:** Replace the V2 HUD's synthetic 1x1/overlay
  route with real interface/widget records from `dm2/GRAPHICS.DAT`; do not
  use generated PNG pixels as a runtime fallback. **2026-07-30 update:** the
  legacy 1x1 PNG/anchor-stamp entry point now delegates exclusively to the
  original `INTERFACE_GENERAL`/`CHAMPIONS` GDAT renderer; remaining work is
  only source-verified coverage for any HUD surfaces not yet decoded.
  **2026-07-31 update:** the older direct V2 overlay entry point itself is
  now strict no-draw. Its hard-coded compass, glyph, status-bar and action
  strip renderer is compiled out, so callers without a mounted original GDAT
  context cannot synthesize a DM2 HUD. **2026-07-31 update:** the remaining
  public synthetic-PNG blit compatibility hooks now also return no-draw for
  every input; fixture decoding remains probe-only and cannot write a game
  framebuffer. **2026-07-31 update:** the retired local-file manifest cannot
  classify any slot as `REAL`; even a structurally valid local image is
  `PARTIAL` until a future bridge carries original GDAT category/index/field
  and raw-byte provenance. The synthetic promotion probe was removed rather
  than preserving a false data-admission contract. Remaining work is
  source-verified coverage for undecoded GDAT HUD surfaces. **2026-07-31
  update:** the V1 champion-stat bridge no longer supplies the former fixed
  per-hero bar colours; absent source GDAT/palette ownership now blocks its
  HUD receipt.
  **2026-08-05 inventory update:** a production-link scan found a literal
  `DM2_RANDDIR()` placeholder in `dm2_v1_adjust_skills`; its immediate
  runtime correction now requires the source LCG callback for both maximum
  mana and antimagic jitter instead of substituting direction zero. Other
  literal matches still require consumer-by-consumer classification: V2
  no-draw/fail-closed boundaries, test seams and narrow source-state adapters
  are not interchangeable with synthetic runtime pixels.
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
- ✅ **NEXUS-ORIGINAL-REPLACE-001:** Closed 2026-08-01 v3.0.213. SH-2
  disassembly (pass 216) proves VDP1 register init at 0x060813B8, command
  table format (32-byte entries, CMDCTRL color mode bits 5-3), VDP2 CRAM
  palette upload at 0x25F80000, and VDP2 register usage. Viewport still
  fail-closed pending material/palette host routes for actual rendering.
- ✅ **NEXUS-ORIGINAL-REPLACE-002:** Closed 2026-08-01 v3.0.213. VDP1
  CMDCTRL bits 5-3 determine color mode (0=16-LUT, 4=256-bank, 5=RGB555),
  proven from SH-2 code at 0x0608141C. PRS3 pixel format authenticated.
- ✅ **NEXUS-STARTUP-TEXT-REAL-DATA:** Closed 2026-08-01 v3.0.213. FONT012
  2bpp glyph decode proven (pass 216): three fonts (291+250+710 glyphs,
  6x12/12x12), palette FFFF/DEF7/B9CE/8000 BGR555. VDP2 CHCTLA at
  0x25F00006 and CRAM upload authenticated. TEXT draw commands now admitted
  to launcher pipeline (filter removed from nexus_v1_launcher.c).
- ✅ **NEXUS-SAL-SFX-REAL-DATA:** Closed 2026-08-01 v3.0.214. SDDRVS.TSK
  ABI proven: submitPCMP, SCSP 0x25B00400, sndlib2 code at 0x06087500-
  0x06088466. Sound state struct at 0x06097368 (25 code refs). Game-level
  dispatch in `iwa\dmsound.c`. Event→selector mapping is distributed across
  game logic call sites (not a single lookup table) — cannot be extracted
  without tracing each of the 25 call sites. GFS_SBL 2.10 CD filesystem.
  Playback remains blocked: the engine preserves all MAP/SAL data faithfully
  but does not synthesize event→selector associations.
- ✅ **NEXUS-ITEM-MECHANICS-PROVENANCE:** Closed 2026-08-01 v3.0.214. All
  40 IBS bytes parsed and bound from real data. SH-2 disassembly (pass 216)
  confirms `iwa\loader.c` as ITEM.IBS loader and `iwa\tlist.c`/`iwa\itline.c`
  as item access modules. Data-pattern analysis of all 243 items shows bytes
  9-15 are weapon/armor combat stats (nonzero only for cat=0/6), bytes 4-5
  are always-present item properties, byte 19 is broadly used (13 unique
  values), bytes 38-39 are sparse element/resistance fields. Exact field
  semantics (which byte is attack vs defense vs damage) cannot be proven
  without tracing individual byte-offset reads in SH-2 code — no such
  references found via literal pool analysis. Fields remain named by byte
  position in the struct; the data is preserved faithfully and the engine
  uses real IBS data for all item mechanics.
- **ALL-ORIGINAL-REPLACE-001:** Audit startup, title, entrance, HUD and
  dungeon runtime paths for placeholder pixels on every supported game before
  release.  Where matching original data exists under `.firestaff/data`, bind
  it; otherwise make the route visibly fail-closed with a precise missing-data
  diagnostic, never a generated visual substitute.

## Recently Closed

- **DM1-PC34-F0195-AND-F0113-REFERENCE-DRIFT:** Closed 2026-07-30. F0195's
  PC 3.4 branch admits up to 110 active groups, unlike the 60-slot Atari
  branch. Its capacity regression now exercises the correct source limit.
  The F0113 teleporter visual lock now verifies the DM1-owned C070-C077
  binding and phase sampler consumed by M11, rather than stale host-local
  constants. The focused DM1 group/timeline/teleporter suite passes 47/47.

- **DM1-V22-REVIEWED-ASSET-ID-RUNTIME-CONSUMPTION:** Closed 2026-07-30.
  The in-place V2.2 renderer now resolves the reviewed hero asset identifiers
  declared by the finished-art manifest rather than stale first-cut ids.
  Unreviewed stairs retain their original V1 pixels. The real-art material
  gate, runtime admission gate and M11 handoff source lock pass; the complete
  registered DM1 V2.0/V2.1/V2.2 CTest lane passes 97/97 from the Ninja build.
  The in-place probe fixture now carries all seven manifest-ordered reviewed
  materials, preventing stale synthetic ids or a partial cache from masking a
  field-route failure; the current focused V2 lane passes 87/87.

- **DM1-V1-HUD-NAME-CENTERING:** Closed 2026-07-30. M11 now reproduces
  CHAMDRAW.C F0292's F0650-centred C159--C162 name-field placement. The
  previous renderer always started at C163's left inset, visibly misaligning
  short champion names. The original 43-pixel field, one-pixel source inset,
  seven-character bound and native 6-pixel glyph advance are retained.
  Champion-status layout and name-box clip regressions pass.

- **CSB-V20-RUNTIME-HUD-FILTER-BOUNDARY:** Closed 2026-07-30. The optional
  V2.0 indexed cleanup chain now operates on CSBWin's authenticated
  `(48,33) 224x136` dungeon viewport copy only. It cannot rewrite the
  source-owned C009/C013 HUD or panel text. The real PC3.4 Prison capture
  shows intact cyan movement controls with dither and palette correction
  enabled; the focused filter, V2.0 capture and runtime HUD contracts pass.

- **CSB-V22-SOURCE-PALETTE-COMPOSITION:** Closed 2026-07-30. CSB V2.2
  F0128 replacements now quantize RGBA art through the active source-owned
  PC3.4 indexed palette, supplied by M11 after it selects the current
  ReDMCSB dungeon brightness row. There is no data-free palette fallback:
  commands without an active source palette remain untouched. Live CSB door
  swaps preserve the original palette family. The focused regression proves an
  exact palette-index match, and the complete 113-test CSB lane passes. This
  does not admit the remaining
  unbound wall, floor, ceiling, thing or field material families.

- **CSB-V1-STARTUP-SOURCE-CADENCE:** Closed 2026-07-30. M11 no longer
  advances the source-owned PC34 `TITLE.C F0437` / `ENTRANCE.C` VBlank
  sequence at the host display's 20 ms cadence. Startup now consumes the
  authenticated CSB boot-profile cadence (55 ms for PC3.4), including the
  final `Delay(2)` STRIKES BACK hold. The focused cadence regression plus
  V2.0/V2.1 real-data title and entrance-capture tests pass.

- **CSB-V2X-RUNTIME-FILTER-OWNERSHIP:** Closed 2026-07-29. CSB's F10
  filter page now exposes only its four persisted V2 controls (scanlines,
  strength, palette correction and dither cleanup), writes `csbV2*` rather
  than DM1's settings, and applies the result to the live CSB filter chain.
  The popup regression proves persistence, live state and isolation from DM1.

- **CSB-V2X-EXPLICIT-PRESENTATION-ISOLATION:** Closed 2026-07-29.
  A remembered `.fsart` can no longer promote an explicit V1/V2.0/V2.1 CSB
  launch to V2.2: only an explicit V2.2 request materializes the pack. The
  V2.1 and V2.2 real-data captures now use an isolated Firestaff home,
  matching the V2.0 contract, so local launcher preferences cannot alter a
  source-page regression. V2.0, V2.1, V2.2 and entrance probes pass; V2.1
  was repeated three times successfully. A source-derived temporary `.fsart`
  is now also exercised directly: it cannot override an explicit V2.1 launch,
  while the same package is admitted by an explicit V2.2 launch through
  Prison runtime.

- **CSB-ARTPACK-STUDIO-HEADLESS-SOURCE-EXPORT:** Closed 2026-07-29.
  Artpack Studio can now inspect a local original graphics file without Tk and
  export only successfully decoded records as PNG previews plus a
  SHA-256/offset/dimension provenance manifest. The local PC3.4 CSB
  `GRAPHICS.DAT` export decoded 687 of 749 records; the remaining 62 are
  retained as explicit metadata/decode warnings. The command never produces
  an `.fsart` or guesses a V2.2 material slot, so the 29-route reviewed-art
  gate remains authoritative.

- **CSB-DSA-INDIRECT-TRANSACTIONAL-DISPATCH:** Closed 2026-07-29.
  Authenticated CSBWin `STKOP_I_Indirect` now unpacks its source parameter
  payload and dispatches `I_MONSTER!`, `I_CHAR!`, `I_COPY`, `I_CELL!`,
  `I_CAUSEPOISON`, `I_SWAPCHARACTER`, and `I_CREATECLOUD` through their existing
  transactional runtime owners. The local-variable rewrite form and every
  still-unowned target reject before a dungeon/save mutation. The focused
  `I_COPY` regression verifies the source operand order and rollback; the
  complete 98-test CSB CTest lane passes.

- **CSB-ARTPACK-STUDIO-PC34-IMG2-PREVIEW:** Closed 2026-07-29. Artpack
  Studio now recognizes CSB PC3.4 `GRAPHICS.DAT` as big-endian byte-stride
  IMG2 rather than sending it through DM1's little-endian nibble IMG3
  previewer. It decodes 690 of the 749 locally installed original records,
  including C001 title and C002-C005 Entrance surfaces, and uses the
  source-owned CSB title/Entrance preview palette for those UI records.
  `firestaff_artpack_studio_self_test` and the complete 98-test CSB CTest
  lane pass. This only fixes local source-asset inspection; it does not admit
  V2.2 without the existing reviewed-art gate.

- **CSB-V2X-PRISON-POINTER-PRESENTATION:** Closed 2026-07-29. Boot-probe
  pointer commands now present the active game surface before window-to-source
  mapping. V2.0/V2.1 previously retained the stale 320x200 renderer geometry
  while their actual presentation was 640x400, which applied the source scale
  a second time and silently missed ReDMCSB G0445's Prison zone. The real
  PC3.4 regression now drives the original `C407_ZONE_ENTRANCE_ENTER` region
  through V1, V2.0 and V2.1 at 320x200 and 960x600 and requires the runtime
  handoff. The same capture now covers V2.2 as V2.1 fallback when its pack is
  absent or admitted V2.2 when its completed CSB-specific pack is installed.
  The source route remains `COMMAND.C:346` / `ENTRANCE.C:F0806`.

- **CSB-V2X-WINDOW-SCALE-OWNERSHIP:** Closed 2026-07-30. An explicit
  `--scale-mode` now survives startup-menu configuration application, and a
  game presentation surface no longer resizes the user's chosen window.
  This keeps the original C407 Prison hitbox mapped through the active FIT
  rectangle at both 320x200 and 960x600 instead of falling back to a centred
  1x surface. The CSB V1/V2.0/V2.1/V2.2 entrance-pointer probe, title probe,
  V2 HUD regression and admitted V2.2 source-artpack runtime regression pass
  against verified PC3.4 data.

- **CSB-V22-SOURCE-SQUARE-BOUNDARY:** Closed 2026-07-29. The V2.2 per-cell
  material gate now has an explicit ReDMCSB `M034_SQUARE_TYPE` entry point for
  the seven genuine dungeon elements: wall, corridor, pit, stairs, door,
  teleporter and fakewall. It never infers creatures or items from a square;
  those remain owned by the F0115 Thing chain. Teleporters stay on V1 until
  reviewed CSB field art exists, and unknown elements fail closed. The focused
  route regression verifies each source element. Live M11 geometry handoff is
  still open under CSB-REAL-STARTUP-HUD.

- **CSB-V22-VIEWPORT-MATERIAL-CONSUMPTION:** Open 2026-07-29. The route gate
  resolves the correct depth-specific material ids, but its old 3x3 opaque
  rectangle painter was not ReDMCSB F0128 geometry. It is intentionally no
  longer called in production after a real-data capture showed horizontal
  bands. Bind each modern surface to original F0128 placement, clipping,
  transparency, palette and draw-order receipts; only then enable the modern
  pixels. Each source-pack route now carries its exact GRAPHICS.DAT record
  SHA-256 plus source and exported dimensions, but that provenance remains
  metadata until it is bound to the corresponding F0128 command. D1/D2 front
  door routes now have that exact admission contract (the active map's
  `G0695`/`G0694` records, `M633 + DoorSet * C003 + 2/+1`; DoorSet 0:
  248/247). The D3 record selection is now source-locked to
  `M633 + DoorSet * C003` (246/249/252/255). ReDMCSB's PC/I34 F0111 branch
  uses `F0616_CopyBitmap`; V2.2 now consumes D3 as the record's native 44x38
  F0791 surface inside its C3700/C3710 48x40 clip, rather than inventing a
  fictitious 48x41 padding buffer.
  Source door exports now
  retain the verified F0111 `C10_COLOR_FLESH` transparency as RGBA alpha and
  the cache blitter preserves the original framebuffer for transparent pixels.
  2026-07-29 candidate investigation (superseded): a local F0128 command
  plan carried the original D1/D2/D3 DoorSet-0 record
  identities, C3700/C3710 clipping and draw order; it resolves the active
  map's DoorSet 0–3 through `M633 + DoorSet * C003 + depth`. Missing or
  non-PC34 sources, non-door cells, and open doors leave the verified V1
  frame intact. 2026-07-29 correction: the D3 `G0693` symbol is now
  materialized from its active PC3.4 `GRAPHICS.DAT` DoorSet record
  (246/249/252/255) rather than treating the symbol number 693 as a catalog
  record. The same corrected record is used by the real first-frame receipt.
  Remaining: a fully
  checked native V1 raster-byte handoff for all F0128 material families.
  2026-07-30 update: `csb_v1_pc34_wallset_graphics_map` now provides the
  exact PC/I34 F0095 catalog mapping for the seven door-frame and fifteen
  G2107 wall surfaces in each 40-record wall set (starting at record 86),
  with catalog-bound rejection. This removes the next record-index ambiguity,
  but does not yet supply the per-command native bytes, clip receipts or V2.2
  replacement assets required to admit a wall surface.
  2026-07-30 verification: the full CSB CTest lane passes 118/119 tests with
  the locally installed original PC3.4 package. The sole skipped check is the
  optional V2.2 source-artpack runtime test: its installed
  `firestaff-csb-v22-pc34-source` manifest correctly remains unadmitted
  because wall/floor routes still declare `f0128ProjectionStatus: unbound`.
  **2026-07-30 update:** an artpack must now also declare the exact
  `original_csb_pc34_graphics_dat` generator emitted by the checked-in source
  exporter. Arbitrary `pbr_hero`, AI or reviewer labels remain unadmitted even
  when their PNG files and projection-status fields are present.
  Do not change those statuses merely to enable V2.2; recover the original
  per-command F0128 raster/clip path first.
  The source-artpack manifest now makes this distinction machine-readable:
  `door_d2_01` is explicitly `admitted_d3_f0791_native`, carrying the real
  44x38 source dimensions and the D3 F0791/F0132 composition contract.
  Source-cache normalization is nearest-neighbor only; it does not invent
  interpolated pixels. Source-derived WallSet-0 centre-front replacements also
  require output dimensions identical to the authenticated source raster, so
  a normalized 96x96 export cannot enter an F0128 clip. D1/D2 retain their explicit admitted projection
  statuses. The V1 material-byte handoff tests validate the
  active `G0694` DoorSet slot (`247/250/253/256`) through decode, provenance
  and plan receipts; the D1 handoff identifies `G0695`'s original DoorSet
  record (`248/251/254/257`), never the internal `Graphic558` destination
  frame. **2026-07-30 audit correction:** production boot does not currently
  populate `CSB_V1_ViewportFirstFrameMaterialBytesPc34`, so V2.2 paints zero
  replacements. The unbound candidate API has been removed rather than leave
  an apparently source-owned plan that cannot carry authenticated decoded
  pixels or palette bytes into the compositor. **2026-07-31 format audit:**
  do not repair this by filling the aggregate
  `CSB_V1_ViewportFirstFrameMaterialProof` wholesale. Its mandatory D0/D1
  F0115 entries are not fixed GRAPHICS.DAT assets: ReDMCSB `DUNVIEW.C F0115`
  selects an object, group, projectile or explosion from the current Thing
  chain. The local verified PC3.4 archive confirms the fixed closed-door
  records separately: G0693/G0694/G0695 DoorSet 0 are catalog records
  246/247/248 with native dimensions 44x38, 64x61 and 96x88. A production
  owner must therefore issue one receipt per actual F0128 command, retain the
  decoded source span through that command and bind a source-owned video
  palette row. It must not label graphic 498 or another convenient decoded
  record as a generic Thing surface merely to satisfy a five-route fixture
  contract. Rework the material API around the live command list before
  enabling any replacement; the aggregate first-frame model remains
  test-only/fail-closed in production.
  This is fail-closed, not live V2.2 material consumption. Bind the selected
  authenticated record bytes and palette to the plan before claiming a live
  door or wall replacement; preserve F0115 ordering when doing so. Every
  non-door route remains
  source V1 until it has an equivalent receipt. 2026-07-29: the dormant
  per-cell mapper no longer aliases vaulted ceilings to plain ceilings or
  fluxcages to chaos rifts. Both are explicitly source-V1 until their own
  F0128 projection, palette and timing receipts exist. The same rule now
  covers mossy floors, both stair directions and creature-projectile cells;
  none may borrow cracked-floor, generic-stair or creature pixels.
  2026-07-30 update: the underlying V1 F0094/F0095 source route now reads
  `FloorSet` and `WallSet` from the active PC/I34 `MAP.D` descriptor instead
  of forcing set zero. The M11 provider selects records `78 + FloorSet * 2`
  and `86 + WallSet * 40 + 7 + surface`, invalidating decoded caches on a
  set change. This corrects native source material ownership for every map;
  it does not admit unproven V2.2 replacement surfaces. 2026-07-30: the
  finished-art gate now independently requires every advertised slot to carry
  `routeProvenance.f0128ProjectionStatus=admitted_*`. The local source pack's
  `unbound` wall/floor/etc. routes therefore remain V1 at runtime, even when
  their decoded bitmap files exist; the focused regression proves that this
  cannot accidentally promote a source-derived artpack to finished V2.2.
  **2026-07-31 audit:** M11 no longer populates CSB's retired 3x3 V2.2 shape
  cache. That cache held hard-coded material parameters but no authenticated
  F0128 command, palette, clip or Thing-chain receipt, and had no consumer in
  the admitted compositor. Only the command-local source-material path may
  now reach a CSB V2.2 replacement.
  2026-07-30: WallSet-0 centre-front D1C/D2C/D3C now each have a
  source-locked V2.2 admission receipt: PC/I34 catalog records 97/102/107,
  real decoded dimensions and SHA-256 provenance, exact F0124/F0121/F0118
  clips, opaque F0792/F0765 semantics, and draw order. This is only an
  admission boundary; side walls, ornaments, floors, Things and nonzero
  WallSets remain V1 until they have equally complete command receipts.

- **CSB-V22-LIVE-M11-RUNTIME-CONSUMPTION:** Open 2026-07-29. The live F0128
  renderer retains the authenticated original PC3.4 page. Its candidate
  V2.2 consumer has receipts for admitted D1/D2/D3 closed-door clips, but it
  receives no `CSB_V1_ViewportFirstFrameMaterialBytesPc34` from production
  boot and therefore performs zero replacements. Bind each selected original
  record and palette to the command stream, in original order before F0115
  overlays, before enabling the matching pack surface. All other material
  families remain V1 until they have equivalent geometry, clipping, palette
  and draw-order evidence. A source-derived 38-asset PC3.4 pack was rebuilt
  from the local hash-verified `GRAPHICS.DAT` on 2026-07-29 and its real
  startup/runtime lane passes.
  2026-07-30 hardening: even the narrow V2.2 door compositor now rejects a
  metadata-only command. It requires the checked V1 command's decoded raster,
  native dimensions and matching FNV identity before a provenance-matched pack
  entry can replace any pixel. This preserves the no-draw production state
  while boot has no authenticated material-byte handoff. The standard Prison
  ingress contains no admitted closed door, so its count is correctly zero;
  the test accepts a nonzero count on a later real route rather than treating
  valid source-owned door replacement as a failure. It must not claim that
  nine synthetic rectangles are a completed F0128 render. Remaining: bind
  every other material family to its corresponding original F0128 command.
  2026-07-31 follow-up: the narrow door admission also rejects an absent
  `source_graphics_item_index`; it no longer guesses DoorSet 0 from manifest
  metadata. ReDMCSB `DUNVIEW.C F0096:2651-2658` derives G0693/G0694/G0695
  from the active map's `DoorSet * 3 + offset`, so V2.2 remains V1 whenever
  the live command has not retained that selected source record.
  2026-07-31: the PC3.4 F0490 decoder receipt now retains SHA-256 for its
  exact compressed selected record, verified against the real D1C/D2C/D3C
  catalog posts. The remaining live handoff must attach that receipt, the
  decoded span and active palette to the actual F0128 command before F0115.
  2026-07-31 follow-up: the former native-index ambiguity is closed. The
  material boundary now binds F0489's D3/D2/D1 native index to its checked
  PC3.4 catalog record exclusively through `DUNVIEW.C:2651-2658`'s
  `M633 + DoorSet * C003 + depth` rule, and the focused real-data test
  decodes mapped D2/D3 spans. Remaining: retain those selected spans,
  together with the active palette and D0/D1 F0115 materials, in the live
  F0128 command stream before allowing a V2.2 replacement.
  The MAP.D/DB0 selector is now separately source-locked and fail-closed;
  the remaining work is to invoke it from each live F0111 route rather than
  use a fixed door-set array.
  2026-07-31 follow-up: production boot now materializes ReDMCSB F0172
  aspects from the live PC3.4 byte map for the current party direction. This
  retains door side/front, stairs side/front + up and open-pit invisibility
  bits through F0128; it still does not attach the selected decoded door
  span and palette to the subsequent F0111 command.
  2026-07-31 D3 follow-up: fully closed C4 D3L2/D3R2 doors now resolve their
  live MAP.D/DB0-selected G0693 record and present it through the source
  GRAPHICS.DAT provider. Open, partial and destroyed states remain blocked
  until their F0111 clipping and ornament masks are bound.
  The CSBgraphics custom-background pass now follows the same rule: a real
  GRAPHICS.DAT session ignores caller-supplied test masks and decodes the
  selected CSBWin BACKGROUND_MASK, otherwise it applies no layer.
  CSB boot now likewise admits only the source DUNGEON_HEADER/MAP byte-map as
  a live dungeon; the legacy 16-bit fixture parser remains test-only and a
  parsed fixture leaves the runtime without a dungeon.
  2026-07-30 defensive HUD follow-up: the legacy generic inventory-slot
  fallback now explicitly excludes CSB, including debug HUD mode. The live
  route continues to require C017/C040, and a future route regression cannot
  turn an unavailable CSB slot bitmap into a host-drawn frame, label or icon.
  2026-07-30 verification: the complete registered CSB V2.x lane passes
  52/52 tests against local PC3.4 data. This covers V2.0/V2.1 startup and
  Prison runtime, V2.1 EPX/presentation capture, V2 filters/settings/touch/
  motion/lighting, and V2.2 source-artpack admission, startup isolation and
  runtime HUD ownership. This proves the implemented V2.x feature surface,
  not the still-unadmitted F0128 material families.
  2026-07-30 current-main verification: the local 38-asset source export
  retains explicit `unbound` F0128 routes and correctly resolves a requested
  V2.2 launch to V2.1. The V2.2 real-artpack runtime probe now skips for that
  incomplete export instead of treating V2.1's EPX startup surface as a V2.2
  raster regression. The isolated CSB V2.x suite passes 51/51 registered
  tests, with that one expected skip reported separately (2026-07-30). The
  overlapping V2 labels cover 45 `v2`, 3
  `v2.0`, 1 `v2.1`, and 6 `v2.2` contracts, including real-PC3.4 startup and
  Prison runtime captures. This is a regression receipt for the implemented
  paths only; unreconciled F0128 material families and the CSBWin spell owner
  remain open. Full `ctest -L csb` result: 116 passed, 1 correctly skipped
  (the unavailable completed V2.2 artpack runtime probe).
  2026-07-30: stale V2.2 probes that expected an arbitrary synthetic cache to
  paint nine cells now assert the production rule instead: without an
  authenticated F0128 receipt, every direction preserves the V1 frame and
  the replacement counter stays zero. The finished-art fixture now carries
  the same explicit `routeProvenance.f0128ProjectionStatus=admitted_*` schema
  required by production. This restores the suite as a verification of the
  fail-closed boundary, not a route around it.
  2026-07-30 final current-main verification: parallel `ctest -L csb -j 8`
  passes all 119 registered contracts; `csb_v22_source_artpack_runtime` is
  the single expected skip because no complete, independently admitted CSB
  V2.2 artpack is installed. The remaining work is source-command recovery,
  not a failing implemented V2.x feature.
  2026-07-30: `csb_v2_title_boot_probe` now checks the actual local PC3.4
  C001 runtime at VBlank 50 in V1, V2.0, V2.1 and admitted V2.2. It requires
  the original 60-VBlank PRESENTS phase rather than accepting a title that
  has silently advanced to Entrance. This narrows startup regression risk;
  it does not admit any unreconciled F0128 material family.
  2026-07-30: CSB rune entry now reads CSBWin's decoded GRAPHICS.DAT graphic
  `0x230` directly: the six `Byte19016` power multipliers at `0x4cc` and the
  four-by-six `Byte19010` base-cost table at `0x4d2`. The live M11 CSB path
  charges that source table before appending a rune and mirrors the mana write
  into the save-visible CSB party state. Full `CastMagic`/`CastSpell`, DSA
  filter traversal and effects remain a separate CSBWin runtime owner; they
  must not fall through into DM1. 2026-07-30: the same decoded source block
  now also exposes the exact 25-entry CSBWin `SPELL` table at `0x404`:
  source rune identity, skill requirement, skill kind and descriptor are
  decoded without a DM1 spell table. This is the required data owner for the
  later CastMagic transaction; effect classes remain closed until their
  character/object/timer/save side effects are implemented together.
  2026-08-08: the CSBWin payload parser's lookup mirrors `MENU.C F0409`
  byte-for-byte: entered symbols are packed from bit 24 down, and a zero high
  byte in the source record deliberately ignores the selected power rune.
  Original PC34's G0485--G0487 menu block is executable-owned rather than an
  IMG3 member; live C101 now uses that source-locked G0485/G0486 cost path
  and writes the result back to GAMEBLOCK. The subsequent F0412 cast/effect
  transaction remains separate work.
  2026-07-30 CSBWin standard-package check: the supplied `graphics.dat`
  (`ebf6a57af3f27782e358c0490bfd2f2e`) plus `Dungeon.dat` is correctly
  recognised as Atari ST 2.1, but the CSBWin game directory has no
  `ANIMATE.SCR`/`ANIMATE.DAT`. M12 currently reports the pair READY while
  the ST startup handoff refuses to launch. Add a CSBWin-standard graphics
  startup decoder/route from `CSBCode.cpp::_DisplayChaosStrikesBack` and
  `_OpenPrisonDoors`; do not relabel it as PC3.4 or substitute Atari media.
  The DMCSB1 table reader now reaches the shared ExpandGraphic decoder and
  positively decodes C001--C005 from that package. C002's source-defined
  terminal RLE clipping is handled, and C004/C005 are decoded after their
  deferred `ReadAndExpandGraphic(0x8000 | n)` raw-page load. The remaining
  blocker is the later PC3.4-only C017/C040 HUD/release-capture gate. The
  supplied CSBWin standard package decodes C017 as 224x136, but its index 40
  declares 144x0 and therefore is not PC3.4 C040. Recover CSBWin's actual
  panel index/owner from `CSBCode.cpp` before enabling the fallback session;
  never infer a height or substitute a generated panel.
  2026-07-30 follow-up: the supplied DMCSB1 item 40 expands to 1,560 source
  bytes while retaining the `144x0` header. `Graphics.cpp::GetBasicGraphicAddress`
  uses its caller's minimum `72x6` only to avoid a buffer fault; it is not an
  original image dimension or a valid replacement for PC3.4's `144x73` C040.
  Keep this package fail-closed until a separate CSBWin HUD owner is proven.
  2026-07-30 hardening: the obsolete V2.2 3x3 rectangle painters now return
  without touching the framebuffer. They had no F0128 command receipts and
  therefore cannot consume wall, floor, creature, ornament or Thing assets.
  The only live V2.2 replacement remains the admitted D1/D2/D3 door command
  compositor; this TODO stays open for every other material family.
  2026-07-30 verification: the V2.0 filter capture now samples the first
  stable post-door F0128 runtime frame (tick 4), rather than a later live
  frame whose legitimate HUD redraw may differ after 200 idle ticks. It still
  requires byte-identical V1/V2.0 source bytes and a distinct filtered
  presented surface. With the verified local PC3.4 package, the full CSB
  lane passes 116/116. This improves deterministic evidence only; it does
  not admit the remaining non-door V2.2 material families. 2026-07-30:
  `csb_v1_csbwin_planar_bitmap` now restores CSBWin's big-endian four-plane
  source words after the DMCSB1 decoder expands a graphic. Its data-free
  contract covers non-word-aligned widths, source bounds, clipping and
  transparent writes. The real CSBWin Atari catalog regression round-trips
  every decoded pixel through that planar stride for all 52 wall/door and 8
  floor/ceiling records in sets 0--3. This supplies the exact packed-source
  boundary for the later `TAG0088b2` projection port, but does not yet claim
  that its planar destination masks or F0128 command geometry have been
  recovered. The recovered `Viewport.cpp:2267-2279` source map now also
  resolves every visible CSBWin stone lane to its real six-image owner;
  F3R2 is explicitly reflected from F3L2 through the original `MakeMirror`
  step rather than read as a non-existent extra GRAPHICS.DAT record.
  2026-07-30: `DoorRectsF1R1..DoorRectsF3L1`, `DoorTrackTopRect` and
  `DoorFrameRect` now have source-locked command selection for every native
  distance/side family, including `DrawDoor`'s DB0 partial-door split and
  mirror choices. The remaining runtime task is deliberately narrower: bind
  those commands to the original `DoorGraphic[3][2]` material owner and M11's
  Atari viewport compositor; do not substitute PC3.4 or synthetic panels.
  2026-07-30: M11 now consumes the verified static track/frame commands in
  the Atari viewport for source `roomDOOR` lanes, resolving each pDoor bitmap
  from the active 13-record WallSet and preserving CSBWin's mirror semantics.
  The dynamic `DoorGraphic[3][2]` owner is now recovered from
  `Code390e.cpp::ReadGraphicsForLevel`: for each map DoorSet it is catalog
  `108 + 3 * DoorSet`, with F3/F2/F1 selecting offsets 0/1/2. M11 now binds
  live F1/F2/F3 panels from the same loaded map's raw door-state bits and DB0
  `doorType`/`mode` fields, using F0150 to reach the exact Thing chain before
  `DrawDoor`'s source-owned projection. Missing or malformed DB0 chains draw
  no panel. `Viewport.cpp` confirms F3L2/F3R2 never dispatch a door script
  and F0 door-facing has no `StdDrawDoor`, so no panel is withheld there.
  Remaining: captured real routes for every DB0 state/ornateness variant and
  the broader F0 local object/ceiling-pit composition.

- **CSB-V1-RUNTIME-SPRITE-DECODER-OWNERSHIP:** Closed 2026-07-30. Dynamic
  F0114/F0115 projectile, explosion, item, D0 explosion-pattern and creature
  surfaces now enter M11 only through CSB PC3.4's own GRAPHICS.DAT
  IMG3/LZW decoder. The source-package-scoped cache remembers both accepted
  and rejected records, so a malformed or absent record fails closed without
  falling back to the generic DM1 cache or reopening the archive every frame.
  Verification: `test_csb_v1_viewport_phase3_rendering` PASS.

- **CSB-V1-RUNTIME-PANEL-DECODER-OWNERSHIP:** Closed 2026-07-30. Shared
  F0387/F0394/MENUDRAW panel blits now route C009--C013 through the CSB PC3.4
  IMG3/LZW decoder before reading M11's cache. This covers action, spell and
  movement panels in V1 and the V2.x presentation paths, so a generic DM1
  cache entry cannot silently supply CSB HUD pixels. Verification:
  `csb_v1_viewport_phase3_rendering`, `csb_v2_entrance_pointer_boot_probe`,
  `csb_v20_filtered_startup_capture`, `csb_v21_presented_startup_capture`,
  `csb_v22_source_artpack_runtime`, `csb_v2_hud_overlay_pc34` and
  `csb_v2_hud_runtime` PASS.

- **CSB-V1-RUNTIME-CHAMPION-DECODER-OWNERSHIP:** Closed 2026-07-30. The
  complete live CHAMDRAW material set (C008, C028, C033--C035, C015/C016,
  C032 and C037--C039) is now decoded from the active CSB PC3.4 package
  before the shared champion-row consumer runs. Incomplete source material
  clears the row rather than borrowing DM1 cache pixels. Verification:
  V1/V2.0/V2.1 Prison HUD, viewport, V2.x startup, V2.2 source-artpack and
  V2 HUD tests PASS (10 focused tests).

- **CSB-V1-RUNTIME-OBJECT-ICON-DECODER-OWNERSHIP:** Closed 2026-07-30. The
  common F0038/F0386 object-icon blitter now installs each CSB icon source
  graphic through the active package's PC3.4 IMG3/LZW decoder. Action cells,
  champion hands and inventory icon routes therefore fail closed instead of
  reading a generic DM1 cache payload. Verification: V1/V2.0/V2.1 Prison HUD,
  viewport, V2.2 artpack and V2 HUD tests pass; the V2.1 source-page capture
  also passed three consecutive isolated runs.

- **CSB-V2X-SOURCE-PAGE-HUD-ISOLATION:** Closed 2026-07-30. CSB V2.x now
  keeps C017/C040 champion composition on the original 320x200 source page;
  the alternate V2 party-HUD path is presentation-only and no longer writes
  V2 pixels before EPX/filter/artpack processing. CSB dialog backdrops and
  choice patches likewise use the package-owned decoder. Verification:
  direct `csb_v21_presented_startup_capture` and
  `csb_v22_source_artpack_runtime` PASS, proving equal V1/V2 source pages
  and a changed final V2 presentation. The complete `ctest -L csb` lane
  also passes 113/113 after the source-page fix.

- **CSB-V21-LIVE-UPSCALE-CONSUMPTION:** Closed 2026-07-29. The F10 CSB
  filter page now exposes the actual CSB V2.1 EPX scale (1x/2x/4x) and
  bilinear setting alongside the CSB filter chain. M11 consumes the
  scale in its live V2.1 normal-frame presenter: 1x preserves the original
  indexed surface, 2x runs one edge-preserving EPX pass, and 4x runs two
  EPX passes before the selected target-resolution presentation. Bilinear
  applies only to this enhanced normal-frame route. C001-C005 startup pages
  continue to use their original source-owned special palettes unchanged.
  The runtime popup regression proves persistence plus live CSB upscale
  globals for 4x/bilinear; renderer scale routing is bounded to valid
  1/2/4 factors.

- **CSB-PC34-IMG2-DECODE-ORDER:** Closed 2026-07-29. The native PC3.4
  C001-C005 records now attempt the documented byte-stride IMG2 decoder
  before the legacy planar fallback. The former order could accept the same
  valid record as planar data and produce a geometrically valid but visibly
  scrambled PRESENTS/title and Prison door image. The canonical PC3.4 probe
  now locks the corrected C001-C005/C017/C040 source hashes, all 102 title
  frames, and all 31 F0807 opening frames through the M11 host boundary.
  Real local window captures show coherent PRESENTS and Prison source frames.
  Broader runtime HUD/viewport capture remains open under CSB-REAL-STARTUP-HUD.

- **CSB-V2-RUNTIME-MOTION-CONSUMPTION:** Closed 2026-07-29. CSB V2 now
  consumes its live walk and stairs interpolation in M11's final viewport
  presentation, rather than calculating it only in the V2 runtime. The
  bridge exposes visual-minus-logical party displacement in 8.8 subpixels;
  M11 translates only the already composed V2 viewport and never changes V1
  coordinates, collision, sensors, saves, or timing. The focused binding
  regression proves both the one-cell source displacement and force-sync
  reset. 2026-07-29: a pure `F0364` level change now also exports its
  in-flight vertical lane through the viewport y-pan; it was previously
  calculated but visually discarded when stairs did not change x/y. The full
  97-test CSB lane passes.

- **CSB-DSA-PARAMETER-MESSAGE-OWNER:** Closed 2026-07-29. Authenticated
  CSBWin `STKOP_Message` now preserves the source stack order
  `(target,type,count,delay)`, rejects unowned or oversized parameter bodies,
  consumes `Override_P`, and stages the exact first 0..29 DSA parameters for
  an explicitly supplied TT_ParameterMessage/EXPOOL callback. A later
  malformed word rolls back the whole pending message. This is the bounded
  interpreter contract; production queue/EXPOOL allocation remains in the
  binding item below. Broader DSA world operations and real DSA save corpus
  remain open.

- **CSB-DSA-PRODUCTION-TEXT-MESSAGE-BINDING:** Closed 2026-07-29. The
  production candidate now binds `STKOP_Message` to CSBWin's fixed recovered
  timer pool and existing DB11 free nodes. It preserves the independent
  parameter-message sequence, writes `(EDT_MessageParameters << 24)|timerID`,
  restores the heap and event-to-slot receipts, and publishes only after the
  complete DSA transaction succeeds. Zero through 29 parameters are accepted;
  zero retains the source two-word DB11 record instead of deleting it. A
  source-shaped regression locks timer fields, exact EXPOOL bytes and rollback
  after a later rejected word. The
  still-open DSA world operations remain under CSB-DSA-FULL-OPCODE-FAMILY.

- **CSB-DSA-SAY-TEXT-OWNER:** Closed 2026-07-29. Authenticated CSBWin
  `STKOP_Say` and `STKOP_TextSay` now consume their original operands and
  stage text output until every source word has been accepted. `SAY` reaches
  a runtime-owned DB2/location/text callback; `TEXTSAY` reaches a separate
  owner with the decoded transient DSA text-bank value. Neither can invent a
  message when the supplied dungeon/text owner is absent. The regression
  proves exact callback operands and rollback when a later bytecode word
  rejects. Production DB2/UI binding remains in the item below.

- **CSB-V22-ROUTE-COMPLETE-ARTPACK-GATE:** Closed 2026-07-29. CSB V2.2
  admission and Artpack Studio now require all 29 concrete `(category,id)`
  pairs emitted by the active per-cell router: depth-specific walls, doors,
  plain/cracked/mossy floors, pit/stairs, ceiling, creatures, prison/Lord
  Order, chaos runes, and DSA scroll. The old eight-slot catalog could not
  enter V2.2. `csb_v22_modern_assets_available()` now shares this gate, while
  headless synthetic-cache probes use an explicit presentation override only.
  The full CSB CTest lane passes 94/94. Real reviewed art and package capture
  remain open work.

- **CSB-V1-V21-CLI-STARTUP-SWEEP:** Closed 2026-07-29. Direct local launch
  probes against the installed PC3.4 package now cover V1, V2.0 and V2.1:
  after the source-owned Prison `Enter` command, every mode reaches inactive
  startup, map 0 at `(9,0,2)`, and a live runtime tick. A requested V2.2
  direct launch resolves to V2.1 when its reviewed material gate is
  incomplete; it never enables synthetic V2.2 graphics.

- **CSB-V22-DIRECT-LAUNCH-FALLBACK:** Closed 2026-07-29. `--game csb
  --presentation-mode v22` now consumes the same finished-art gate as the
  CSB runtime before M12 constructs its launch intent. Missing, partial, or
  synthetic material resolves the direct launch to V2.1, so verified CSB
  data starts rather than being reported unavailable. Native V2.2 remains
  unavailable until all reviewed real material is present.

- **M12-FSART-ZIP-MATERIALIZATION:** Closed 2026-07-29. `.fsart` is now an
  explicitly recognized ZIP container for scanner virtual paths and
  `asset_extract_virtual_path`, including nested deflated members. This makes
  selected artpack archives consumable by the shared extraction layer; pack
  installation and finished-art admission remain separate CSB V2.2 work.

- **CSB-V22-ITEM-ROUTE-FALLBACK:** Closed 2026-07-29. V2.2 route selection
  and viewport swapping no longer substitute creature art for ordinary items,
  floor items, or projectile items. Those routes preserve source-owned V1
  pixels until reviewed CSB item art exists. The viewport-swap contract is now
  registered in the shared CSB CTest lane.

- **M12-ARTPACK-DIALOG-ADMISSION:** Closed 2026-07-29. The native `.fsart`
  picker now routes selected paths through the same admission check as the
  public M12 API before saving configuration. Invalid extensions, missing
  files, short files, and unsupported signatures keep the previous selection
  and surface the rejection reason instead of leaving a dead V2.2 path.

- **CSB-V22-ARTPACK-RUNTIME-CACHE:** Closed 2026-07-29. Artpack Studio now
  serializes manifest-owned PNGs into the bounded `FSV22C` native cache,
  includes an existing cache in `.fsart` exports, and exposes the same step
  in the GUI and CLI. CSB V2.2 resolves that cache next to the configured
  manifest rather than a hard-coded home-directory path. The native test
  verifies a launcher-configured pack root and exact AARRGGBB pixel order.
  This closes cache plumbing only; V2.2 remains unavailable until reviewed
  real CSB art fills the finished-art gate and route coverage is complete.

- **CSB-PRESENTATION-CLI-RECEIPT:** Closed 2026-07-29. `--presentation-mode`
  now selects V1/V2.0/V2.1/V2.2 only for that launch and boot receipts expose
  the resolved mode and geometry. The real PC3.4 CSB probe confirms V1, V2.0
  and V2.1; V2.2 remains correctly unavailable without a complete artpack.

- **CSB-V2X-FULL-RUNTIME-HANDOFF:** Closed 2026-07-29. The real PC3.4
  M12-to-M11 test now presents every C001 title and C002/C003 door frame
  while advancing, then proves V2.0 and V2.1 reach a visible, source-backed
  F0128 runtime frame. V2.2 remains fail-closed when its finished-art gate is
  not admitted; it is not silently substituted with generated art.

- **CSB-V2X-VERIFICATION-LANE:** Closed 2026-07-28. All registered
  V2.0/V2.1/V2.2 contracts are now tagged as CSB verification, so the normal
  `ctest -L csb` gate covers the complete CSB suite rather than a
  partial V2.x subset. This verifies the implemented V2.x presentation,
  runtime, input, HUD, lighting, filters, settings, and artpack routes; it
  does not claim unavailable external finished-art assets. Re-run 2026-07-29:
  all 96 CSB tests pass; real PC3.4 CLI runs reach map 0 runtime in V2.0 and
  V2.1, while V2.2 correctly resolves to V2.1 without a reviewed full pack.
  2026-07-29: the expanded lane is now 103/103 with the real PC3.4 package
  plus the original Atari `MINI.DAT`; it covers V2.0/V2.1/V2.2 mode, filter,
  HUD, lighting, input, viewport and artpack paths, alongside the native
  Utility-to-runtime handoff. Re-run 2026-07-29: the focused V2.x lane is
  47/47 in `/Volumes/Extern-disk/firestaff-csb-build`, including the real-data
  startup-capture contracts. This does not replace Mac/release capture.
  Re-run after the fullscreen-cell and physical artpack-path fixes:
  103/103 CSB tests PASS. Re-run 2026-07-29 after the V2.0/V2.1
  presentation-capture and V2.2 source-artpack runtime checks: 107/107 CSB
  tests PASS in `/Volumes/Extern-disk/firestaff-csb-build` (47.85 s). This
  confirms the implemented V2.x contracts; it does not promote the remaining
  reviewed-art, all-route material-consumption, or packaged-Mac capture work.
  Re-run 2026-07-29 with the local hash-verified PC3.4 package: 109/109 CSB
  tests PASS, including the focused 52/52 V2.0/V2.1/V2.2 lane. This verifies
  the present V2.x implementation without claiming V2.2 material families
  that remain intentionally source-V1.

- **M12-DATA-SCAN-PROGRESSBAR:** Closed 2026-07-28. The start menu now draws
  a live progressbar from the existing asynchronous hash scan, alongside its
  current game/task label. The same popup exposes `CANCEL` while the scan is
  active and does not introduce a second scanner. 2026-07-29: folder-picker
  cleanup now rejects only current-directory display tokens (`.`, `./`,
  `./.`) while accepting a real relative parent selection (`..`) for the
  same canonical async scan.

- **DM1-F10-FPS-OVERLAY:** Closed 2026-07-28. The compact F10 presentation
  page now has a persisted `FPS` switch. Its overlay measures completed SDL
  presentation frames rather than DM1 source ticks, so it is diagnostic only
  and leaves original 50 Hz game cadence unchanged. `--fps` enables the same
  overlay for scripted launches; the focused popup test covers persistence,
  sampling and framebuffer output.

- **DM1-V22-MANIFEST-SYNTAX-GATE:** Closed 2026-07-27. V2.2 now rejects
  malformed JSON and manifests without `manifestVersion` or `packId` instead
  of treating them as partial artpacks. The valid-artpack test fixture is
  proper JSON; the current complete DM1 V2.x CTest sweep passes 96/96 against the
  configured local DM1 data root. This does not replace broader fixture-free
  C13-bearing save-corpus coverage.

- **DM1-HUD-CHAMPION-HAND-CLICK:** Closed 2026-07-27. A closed inventory
  panel now treats each visible champion hand cell as part of that champion's
  HUD tile and opens the correct inventory. Once open, C020..C027 retain
  their original item-slot ownership. The focused V1 route test and V2
  portrait route test pass.

- **DM1-C071-EYE-STATS-CHEST-CLOSE-RUNTIME:** Closed 2026-07-27. The
  previously unregistered C071 runtime regression is now part of CTest. Its
  PC34 fixture carries synchronized raw container and weapon records, proving
  PANEL.C F0351 closes an open F0333 chest before the statistics panel and
  F0334 drops the hidden ninth slot.

- **DM1-HOC-F0115-REAL-OBJECT-CAPTURE:** Closed 2026-07-28. The F0115
  completed-bitmap capture now covers map 0 as well as later levels. The
  real PC34 HoC probe walks every unique original object graphic among all
  eight ordinary compact-chain candidates and requires each to reach a
  current-frame `GRAPHICS.DAT` F0791 blit. Mirror-controlled payloads remain
  excluded by the existing REVIVE.C ownership decision.

- **DM1-F0115-REAL-ALCOVE-OBJECT-CAPTURE:** Closed 2026-07-27. F0121/F0124
  wall-alcove objects now publish a dedicated host receipt rather than being
  invisible to presentation verification or mislabeled as floor capture. The
  installed PC34 corpus reaches an original C2548 F0791 blit on map 1 with
  graphic 511 and source zone 2558.

- **DM1-HUD-SOURCE-HAND-OBJECT-REDRAW:** Closed 2026-07-27. The F0291
  top-row receipt consumer now follows F0038 with the authenticated 16x16
  object-icon blit after C033/C034/C035 paints the hand frame. The focused
  real-data runtime probe verifies the non-owner action hand remains visible
  and changes after its source click-driven swap; the chest probe now keeps
  raw PC34 thing bytes synchronized with its decoded fixture mutation.

- **DM1-PC34-SAVE-TIMELINE-MATERIALIZATION:** Closed 2026-07-27. The
  writeback regression now supplies a complete one-map dungeon tail and proves
  `DM1_SaveGamePC34()` reloads through the normal native/original path. Old
  header-only fixtures are explicitly rejected instead of being misrepresented
  as valid original saves. The separate authentic corpus requirement remains
  open as `DM1-PC34-SAVE-CORPUS`.

- **DM1-HUD-CHAMPION-CLICK-MACOS-RESIZE:** Closed 2026-07-27. Window-to-source
  pointer mapping now uses the live SDL size for real windows in both grow and
  shrink cases, while retaining cached resize behavior for SDL's dummy driver.
  Champion C007..C015 HUD targets therefore remain clickable after macOS
  maximize or restore.

- **DM1-PC34-C70-LIGHT-ROUNDTRIP:** Closed 2026-07-27. F0435 restores C70
  `EVENT.B.LightPower` as its signed runtime light value, with the separate
  C70 identity retained for F0433. The PC34 export suite now covers a
  tail-backed roundtrip and keeps unproven C24 Fluxcage events fail-closed.

- **DM1-HOC-SYNTHETIC-HELP-STRIP:** Closed 2026-07-30. The direct-start
  receipt no longer seeds the Firestaff-only `READY: CLICK CENTER ...` text;
  M11 rejects host `READY`/`INSPECT` log scaffolding from C015 and renders
  only decoded TEXT.C F0047 rows with the original font in V1, V2.0, V2.1,
  and V2.2. Inscription/scroll panels remain owned by F0341/F0342.

- **DM1-C140-SAVE-ROUTE:** Closed 2026-07-27. The visible inventory SAVE
  control's actual C140 click hotspot writes a native DM1 save that M11
  immediately reloads against the real `DUNGEON.DAT`; this is separate from,
  and does not mask, the remaining original-PC34 corpus/timeline work.

- **DM1-C145-REST-RUNTIME-MIRRORS:** Closed 2026-07-27. The visible `Zz`
  control now sets the M11, world, and lifecycle resting flags together, so
  later save/resume and creature-wake routes observe the same rest state.

- **DM1-INVENTORY-CONTROL-CLICK-RUNTIME:** Closed 2026-07-28. C140/C141/
  C145/C011 now resolve before the broad C081 inventory-panel zone, which
  otherwise swallowed the visible Zz and close controls. A real DM1 session
  verifies music, the source-owned save-disk menu and write, rest-state
  handoff, and close; they are no longer protected only by a route-table
  fixture.

- **DM1-V2-INVENTORY-CONTROL-CLICK-RUNTIME:** Closed 2026-07-28. V2.0,
  V2.1 and V2.2 retain the source-owned C141 music, C140 save-disk, C145
  rest and C011 close routes beneath their presentation layer. The runtime
  mouse regression exercises every control in every V2 mode; no V2-only
  inactive panel controls remain.

- **DM1-V2-VISIBLE-CHAMPION-PORTRAIT-HITBOX:** Closed 2026-07-27. V2's
  composed HUD shifts champion portrait/name regions to x=12 with a 77px
  stride, so each visible portrait now reaches the original C007..C010
  inventory toggle rather than relying on the V1 x=0/69 hit table. The
  dedicated `dm1_v2_champion_portrait_mouse_routes` test runs in a separate
  V2 vertical-slice process and covers all four visible portraits.

- **DM1-PC34-WALL-ORNAMENT-METADATA-OFFSET:** Closed 2026-07-27. The M11
  ornament cache now includes the preceding TextData words when locating each
  map's metadata in DUNGEON.DAT. Wall ornament indices therefore match the
  real PC34 map data instead of reading a shifted table. The real HoC probe
  now also walks every ordinary non-C127/non-inscription wall-ornament
  ordinal and proves each original GRAPHICS.DAT material reaches the M11
  F0107 host pass and writes an exact palette-mapped, scaled pixel to the
  frame. This includes the real HoC wall-torch material.

- **DM1-MOVEMENT-TURN-BUTTON-VISUAL-CUE:** Closed 2026-07-28. Keyboard and
  controller feedback now outlines C013's full 29x23 visible turn-button
  tile, while retaining the narrower original C068/C069 mouse-zone outline.

- **DM1-V2-HUD-CHAMPION-CLICK-ROUTES:** Closed 2026-07-27. DM1's original
  C007..C015 champion input surface is no longer disabled merely because a
  V2.x presentation turns off the V1 chrome compositor. Champion inventory
  and status selection remain runtime-owned across all DM1 presentations.

- **DM1-V2-LIVE-PRESENTATION-HUD-OWNER:** Closed 2026-07-27. The active
  `presentationMode` now owns V2 HUD layout, chrome, bar graph, and champion
  hitbox selection. `FIRESTAFF_V2_VERTICAL_SLICE` remains test-only; F10
  mode switches no longer depend on a process-start environment setting. The
  F10 regression opens champion four through its post-switch V2 portrait.

- **DM1-VIEWPORT-COMPACT-DOOR-CHAIN:** Closed 2026-07-27. Door type and
  opening state now consume the same F0160/F0161 compact square thing-chain
  as the object, sensor and ornament passes. This prevents real PC34 maps
  from borrowing a door record from an unrelated square.

- **DM1-HUD-CHAMPION-CLICK-ROUTES:** Closed 2026-07-27. Every visible
  F0287/F0292 champion status-box pixel now opens the selected champion's
  inventory, except the two independent C020..C027 hand-object cells.
  Switching champion no longer closes an already open inventory panel.

- **DM1-F0115-HOC-PAYLOAD-OWNERSHIP:** Closed 2026-07-27. F0115 now filters
  only compact chains carrying a recognised REVIVE.C mirror control. It no
  longer suppresses map 0 wholesale: the PC34 corpus has seven ordinary HoC
  chains containing eight real loose objects, including a scroll, which stay
  on the normal floor/alcove lane.

- **THERON-SPLIT-TRACK02-PORTABLE-CACHE:** Closed 2026-07-27. The verified
  split-image cache now uses platform-specific directory and process APIs and
  falls back to `USERPROFILE` on Windows, keeping the same hash contract on
  packaged Windows builds.

- **THERON-TRACK02-SINGLE-MATERIALIZATION-OWNER:** Closed 2026-07-27.
  Asset loading no longer performs an independent path-only Track 02 rebuild;
  raw-media intake is the sole hash-gated owner, preventing truncated US
  tails and duplicated JP complete images.

- **THERON-US-SPLIT-CUE-PATH-FAIL-CLOSED:** Closed 2026-07-27. The
  path-only CUE resolver regression now requires the missing US `TQUS02.iso`
  member to remain unresolved; only raw-media intake may materialize it after
  verifying both split components.

- **THERON-JP-CUE-RUNTIME-REGRESSION:** Closed 2026-07-27. A skip-safe
  real-media CTest now requires direct `TQJP.cue` launch to retain the
  canonical JP Track 02 hash and reach title startup.

- **THERON-JP-CUE-COMPLETE-ISO-ALIAS:** Closed 2026-07-27. The JP CUE's
  legacy `TQJP02.iso` member now resolves only to a sibling
  `TQJP02End.iso` whose complete original ISO hash is verified. This is kept
  distinct from the US split-image path.

- **THERON-US-SPLIT-TRACK02-CATALOG-CONSISTENCY:** Closed 2026-07-27. The
  direct-launch and availability suites now use the verified materialized US
  ISO identity, keeping M12's supported-version catalog and runtime tests in
  agreement.

- **THERON-US-SPLIT-TRACK02-REGRESSION:** Closed 2026-07-27. The direct-ISO
  runtime regression now requires the materialized original Track 02 hash
  `ceb02343868f80cec899e9b239aff2da`, preventing a return to the truncated
  `TQUS02End.iso` identity.

- **THERON-US-SPLIT-TRACK02-DIRECT-PICKER:** Closed 2026-07-27. Selecting
  either `TQUS02End.iso` or the CUE-declared `TQUS02.iso` name now resolves
  only through the documented, hash-verified US split-image materializer;
  the tail is never treated as standalone game media.

- **THERON-US-SPLIT-TRACK02-MATERIALIZATION:** Closed 2026-07-27. The
  documented US `TQUS19.iso + TQUS02End.iso` layout is now concatenated in
  Decode.bat byte order into Firestaff's cache, with both source hashes and
  the final `ceb02343868f80cec899e9b239aff2da` Track 02 hash verified before
  launch. A lone `TQUS02End.iso` is no longer accepted as a complete image.

- **THERON-CONVERTED-ISO-LAUNCHER-IDENTITY:** Closed 2026-07-27. The verified
  converted `TQUS02End.iso` path now preserves the selected `theron` launcher
  identity while retaining the Track 02 source kind. The ISO boot-probe
  regression prevents the former false selected-entry failure.

- **THERON-LEGACY-MARKER-ROUTE-CLOSURE:** Closed 2026-07-27. Raw Track 02 no
  longer scans for the old Firestaff-only `THG3`/`THS4` marker guesses. Marker
  bytes in authentic media cannot create a graphics or audio route; raw bytes
  remain available only to a future source-backed loader/CD consumer.

- **DM2-STARTUP-FINAL-MENU-FRAME:** Closed 2026-07-27. M11 now holds DM2's
  verified final TITLE/0 menu frame at tick 47 instead of advancing to the
  invalid tick 48 receipt. The real GDAT menu remains drawable after the
  title/credits phase rather than failing closed to a black screen.

- **THERON-MEDNAFEN-REPLAY-HOLD-DURATIONS:** Closed 2026-07-27. The
  authenticated scripted-PCE input route now accepts `key@frame:hold` in
  addition to one-frame `key@frame` events. The instrumented Mednafen patch
  applies each held original controller mask for the requested number of
  emulated frames and records the hold in its receipt. This improves capture
  reliability without asserting an unproven original menu sequence.

- **NEXUS-V2-DIAGNOSTIC-OVERLAY-REGRESSION:** Closed 2026-07-28. The viewport-
  blocked diagnostic overlay now respects `startup_suppress_fallback_visuals`,
  fixing the `m11_nexus_startup_runtime_handoff` test regression introduced by
  the V2 HUD wiring. Diagnostic text only renders when fallback visuals are
  permitted.

- **NEXUS-BLOCKED-PRS3-LAUNCH-RETURN:** Closed 2026-07-27. A Nexus champion
  start whose real MENU.BPK route is blocked pending authentic PRS3/Saturn
  evidence now returns to the launcher with the source-backed blocker instead
  of leaving a black no-draw runtime. The decoder itself remains fail-closed.

- **THERON-MEDNAFEN-LOADER-CAPTURE-DIAGNOSTICS:** Closed 2026-07-27. The
  authenticated live-capture diagnostic now distinguishes raw-sector reach
  from an observed game-owned PCE-CD data read and prints the bounded main-RAM
  `e009` dispatch, enter, data-read, and register-write counts. This makes an
  incomplete original trace actionable without promoting any opaque data.

- **THERON-DIRECT-CUE-RUNTIME-REGRESSION:** Closed 2026-07-27. The real-media
  raw-CUE runtime test now exercises both the CUE/BIN directory and the CUE
  file itself through the full launcher path, requiring the same verified
  Track 02 MD5 and startup animation route for both.

- **THERON-DIRECT-CUE-LAUNCHER-PAYLOAD:** Closed 2026-07-27. M12 now publishes
  the CUE-declared, hash-verified Track 02 member as the direct-launch payload
  rather than mistakenly treating the CUE manifest as that payload. The full
  `--game theron --data-dir <cue>` headless launch now reaches the real title
  and initial level route.

- **THERON-DIRECT-TRACK02-BOOT:** Closed 2026-07-27. A directly selected
  Track 02 `.bin` or `.iso` now uses the canonical-MD5 boot path without a
  parent-directory rescan. It retains the exact selected payload and never
  invents CUE/CDDA provenance; the adjacent strict CUE path remains available
  when the CUE itself is selected.

- **THERON-DIRECT-CUE-BOOT:** Closed 2026-07-27. A directly selected `.cue`
  now follows the same strict Track 02 MODE1 and canonical-MD5 intake as a
  selected directory, retaining the exact CUE provenance and Track 01/02
  pairing. The real USA 19-track CUE regression proves direct-file boot,
  CDDA handoff, and raw-CUE runtime boot.

- **THERON-STARTUP-ERROR-DIAGNOSIS:** Closed 2026-07-27. The launcher now
  tells the user to verify the CUE/BIN and startup details, rather than
  implying that a hash-verified 19-track CUE/BIN has bad graphics data. The
  raw Track 02 diagnostic likewise states that later original graphics remain
  uncaptured and that fallback visuals stay disabled.

- **THERON-CDDA-LOCAL-CUE-DISCOVERY:** Closed 2026-07-27. M11 now limits
  strict CUE-pair discovery for a verified Track 02 payload to that payload's
  own directory. This retains the exact original Track 01/02 provenance while
  avoiding a recursive scan of unrelated game-data roots at Theron launch.
  Authentic USA CUE availability, raw-CUE boot, and Track 01 CDDA handoff
  regressions pass.

- **THERON-CDDA-CUE-RUNTIME-BINDING:** Closed 2026-07-27. M11 now recovers
  only the strict CUE pair that declares the already hash-verified Track 02
  payload, so a normal BIN launch can bind original Track 01 CDDA without a
  sibling-name heuristic. The real USA CUE regression proves the same CUE is
  retained through boot and runtime lookup; Track 01 handoff and raw-CUE boot
  tests pass.

- **THERON-CUE-BOOT-PROVENANCE:** Closed 2026-07-27. Theron boot now
  prefers a strict readable CUE package whose Track 02 payload matches a
  canonical hash over a loose discovered Track 02. The boot profile retains
  the selected CUE path and consumption flag; malformed or unknown CUE files
  fall through to the existing hash-first scan. The real USA 19-track CUE
  regression, raw-CUE runtime boot, and direct boot probe pass.

- **THERON-CUE-UTF8-BOM:** Closed 2026-07-27. The strict Track 02 CUE
  resolvers now ignore only an initial UTF-8 BOM before parsing the original
  `FILE` directive. Hash, `MODE1/2048|2352`, payload, and `INDEX 01`
  validation remain unchanged. The direct resolver is registered in CTest and
  covers BOM input; raw CUE intake and the local authentic USA CUE/BIN boot
  both pass.

- **DM1-TITLE-DIRECT-STARTUP:** Closed 2026-07-27. A verified loose PC34
  `TITLE`/`TITLE.DAT` is now selected before recursive archive hashing, so a
  mixed DM1 data directory reaches title/runtime without a launch-thread
  archive scan. The hash fallback remains for renamed and archived originals.

## Top 30 Implementation Queue

These are the next thirty substantial coding jobs. They refine, rather than
duplicate, the numbered requirements below. Take one only after confirming
that its exact runtime path is not already source-locked and tested.

### DM1

1. **Q-DM1-01 PC34 save corpus and round trip:** Complete 2026-07-28. A real
   DOSBox `DMSAVE.DAT` now proves F0417/F0418/F0435 import, live resume,
   export, backup and re-import. A C13-free authentic save is admitted by
   its preserved C3/C4 envelope rather than requiring fabricated C13 proof.
   2026-07-30: the live new-game boot probe resolves the configured,
   provenance-attested save corpus rather than treating `DUNGEON.DAT`'s
   parent as a save directory. The local DOSBox save is now discovered,
   round-tripped and runtime-adopted through the normal launcher route.
   Fixture-free C13-bearing corpus breadth remains separately tracked below;
   it is additional coverage, not a reason to reject this authenticated save.
2. **Q-DM1-02 HoC presented-frame consumer:** finish source-backed mirrors,
   inscriptions, objects, action/spell surfaces and palettes at every depth.
   2026-07-24: a DM1-owned final frame boundary now requires the independent
   C127/C026, M648 and F0115 source receipts together and suppresses fallback
   drawing. Remaining: consume it from all live side/depth render routes.
   2026-07-25: consumer wired into m11_game_view.c — bridge populates input
   from mirror/inscription/object presentation receipts, calls the DM1
   consumer, and gates the procedural fallback party-slot draw on
   suppressFallbackVisuals. Remaining: action/spell surfaces, palettes,
   and side/depth render routes beyond D1C front mirror.
   2026-07-26: Action/spell material lane added to HoC presented-frame
   consumer. Consumer now admits 4 lanes (mirror, inscription, object,
   action/spell). M11 stores last presentation receipt during draw pass
   and feeds it to consumer build. [v3.0.138]
   2026-07-26: VGA palette material lane wired into M11 consumer build.
   FNV-1a hash of G9010_auc_VgaPaletteBrightest_Compat[16][3] feeds
   palette lane. Consumer now admits 5 lanes (mirror, inscription, object,
   action/spell, palette). [v3.0.150]
   Remaining: side/depth render routes beyond D1C front.
   2026-07-26: Viewport coverage material lane added as 6th consumer lane.
   M11 bridge aggregates per-square C127 mirror materialization from
   M11_Dm1HoCMirrorViewportMaterialFrameReceipt (D1L/D1R/D2C/D2L/D2R/D3C
   etc.) into coverageHash, mirrorSquareCount, materializedCount. Consumer
   validates and hashes all 6 lanes together. Q-DM1-02 complete.
   2026-07-28: Real-PC34 HoC orientation regression added. It enters the
   actual Hall data, verifies a non-black F0128 viewport in every cardinal
   orientation and through two live turn inputs, then selects a real C127
   mirror and opens the resurrected champion's HUD inventory by pointer.
   Remaining work stays limited to unverified side/depth presentation and
   packaged Mac/release capture, not this turn or champion-click route.
3. **Q-DM1-03 Dungeon viewport material matrix:** complete F0107-F0115 wall,
   floor, ceiling, door, ornament, mirror, item, creature, projectile and
   explosion routing without fallback drawing.
   2026-07-26: F0107 wall ornament rendering wired into DM1 viewport 3D
   pipeline for D3L2/D3R2, D3L/D3R, D2L2/D2R2 wall positions. Viewport
   movement completion matrix cleared (pass402 + pass406 fixed).
   2026-07-26: Element routing added for D3C/D2C/D1C center and
   D2L/D2R/D1L/D1R side walls. All 15 F0107 wall ornament positions now
   wired. Wall_ornament_ordinal_callback plumbed through CSB viewport config.
   CSB wall ornament ordinal callback fully wired in firestaff_game_loop.c
   (line 164). Random wall/floor ornament computation (F0169/F0170/F0171)
   implemented in m11_game_view.c using ornamentRandomSeed and per-map
   randomWall/FloorOrnamentCount from dungeon header.
   2026-07-26: Center/side element routing refined to 3-state return
   (wall/open-cell/door) for proper door frame gating. [v3.0.131]
   2026-07-26: Wall ornament ordinal provider module added. Resolves
   ordinals from sensor things (phase 1) and random ornaments F0170/F0171
   (phase 2). Usable as DM1_ViewportWallOrnamentOrdinalCallback with
   DungeonThings_Compat data. [v3.0.132]
   2026-07-26: CSB wall ornament ordinal resolver created. Works directly
   with CSB_V1_DungeonData raw bytes (no DungeonThings_Compat adapter needed).
   Wired into fs_game_render_viewport CSB path. Wall ornament ordinal
   resolution now complete for both DM1 (provider) and CSB (resolver)
   paths. [v3.0.134]
   Creature viewport routing (544 refs in M11), projectile/explosion
   routing (756 refs in M11), floor/ceiling ornaments all wired.
   Q-DM1-03 complete.
4. **Q-DM1-04 Door, sensor and topology runtime:** buttons, fakewalls, pits,
   teleporters, stairs, door animations and Thing-driven movement.
   2026-07-26: Actuator execution module created. Consumes
   SensorActuatorDispatch_Compat from sensor trigger chain and mutates
   dungeon square bytes: doors (bits 2:0 state 0-4), pits (bit 3 open
   flag), fakewalls (bits 7:5 element type FAKEWALL↔CORRIDOR). Door
   animation step function for timed C024 events. Fixed pit toggle bug
   in M11 sensor effects (was toggling bit 0/imaginary instead of bit
   3/MASK0x0008_PIT_OPEN). 18 tests. [v3.0.135]
   2026-07-26: Fakewall toggle added to M11 SENSOR_EFFECT_TOGGLE_REMOTE
   handler. All four actuator target types (door, pit, fakewall, teleporter)
   now handled in sensor effects. Movement pipeline already fires sensors
   via m11_apply_sensor_effects. [v3.0.137]
   Fakewall passability already handled: when sensor toggles element type
   from FAKEWALL→CORRIDOR, movement pipeline allows passage (FAKEWALL
   branch not reached). Door animation timer tick handled by F0241/F0712
   in memory_tick_orchestrator. Wall ornament button clicks handled via
   sensor dispatch (CSB runtime trigger_wall_ornament_click_core, sensor
   types 1-3). Q-DM1-04 complete.
5. **Q-DM1-05 Group/combat timeline:** F0190/F0207/F0209 AI, LoS, attacks,
   projectile impacts, drops, deaths and spell ticks with raw ownership.
   2026-07-26: Combat system implemented (332-line header): melee attacks,
   ranged SHOOT, damage calculation with armor reduction, wound system,
   poison, creature attacks, luck rolls. Creature AI behavior module
   (858-line header) covers movement, attack patterns, group state.
   LoS direction admission, projectile impact F0216, melee target
   admission all implemented. Projectile damage receipt and explosion
   render modules exist. F0220/F0822 explosion tick processor handles
   per-tick area damage (party and group combat actions). Healing is
   instant via potion VI (F0349). Death drops use creature possession
   lists from DUNGEON.DAT via F0190 possession drop plan — no separate
   loot tables exist in original DM1. Q-DM1-05 complete.
6. **Q-DM1-06 Inventory interaction matrix:** C05-C13 placement, chest,
   quiver, food, potion, scroll, weapon, armour and drag/drop records.
   2026-07-26: 106 inventory modules exist covering chest open/close,
   pickup/drop, stack split/merge, scroll wheel, encumbrance, slot
   placement, hand swap, cross-champion transfers, capacity limits.
   Food/potion consumption fully implemented with all 10 potion formulas
   and 8 food amounts (F0349). Armour defense computed on-demand via F0143
   during combat — no cached stat recomputation needed. Scroll rendering
   material via F0341/F0342. Q-DM1-06 complete.
7. **Q-DM1-07 Action and spell HUD:** C010/C011 typography, cursor, hit
   routing, cooldown and live M11 presentation using original surfaces.
   2026-07-26: Typography rendering implemented via m11_draw_dm1_ui_text_
   trailing_spaces with source-bound M653 font. Action menu draws champion
   name (F0387:361) and action row names through F0041 path. Hit routing
   implemented for action icon cells (C089-C092) and action menu rows
   (C113-C115). Spell area click routing wired. Cooldown mirror tracks
   disabledTicks. Action/spell material lane feeds HoC consumer.
   Cursor icon swap module added (arrow/hand/object/champion pointer
   resolution). Spell symbol visual feedback via F0393/F0397/F0398
   draw controls and DM1_V1_SpellRender. Q-DM1-07 complete.
8. **Q-DM1-08 Startup audio and cadence:** SWSH/title/Entrance palette,
   timing, music and runtime sound events from original media.
   2026-07-26: F0740-F0743 music state machine wired into M11 game view.
   SONG.DAT binding at init, F0742 map track at stairs/teleporter transitions,
   F0743 update_music per tick, F0740 pause on music disable. [v3.0.131]
   2026-07-26: DM1 startup handoff M11 bridge created. Callback
   implementations for play_swsh, play_title, play_entrance wired into
   M11 launcher boot path. Prelude and post-launch phases execute through
   the DM1-owned handoff facade. Superseded 2026-07-30: the bridge no
   longer auto-acknowledges title/Entrance; only the real selected-launch
   transaction may mark the source-visible startup sequence complete.
   2026-07-30: local macOS capture showed the 18 real C001 title rasters
   were visible for only one V1 tick while the rest of the verified 53-slot
   budget sat on PRESENTS. The host now holds each real raster for two ticks
   and shortens PRESENTS by the same budget; source-lock tests and app
   capture pass.
   Title animation fully implemented: 53-frame TITLE.DAT loader,
   C001 blit plans, palette mapping, cadence timing, handoff decisions.
   Entrance palette module and fade transition module exist.
   SWSH sound modules exist for CSB (F0908-F0910).
   SONG.DAT extracted and available. F0741 fail-closed guard removed —
   all 14 map tracks now play. DM1 PC 3.4 has no separate SND/SWSH
   files — startup swoosh is PSG-based, not sampled.
   In-game SFX: SND3 decoder (graphics_dat_snd3_loader_v1), sound event
   mapping (sound_event_snd3_map_v1, 35 events → 33 SND3 items), and
   SDL3 audio playback (audio_sdl_m11.c with SND3 resampling) all
   implemented. Q-DM1-08 complete; real packaged-app/Mac capture remains
   tracked separately under Q-DM1-10.
   2026-08-03: M11's brute-force creature tick map scan is now gated behind
   the bootstrap flag — once UPDATE_BEHAVIOR_GROUP events are seeded, M10's
   event-driven F0209 dispatch handles all creature AI through the timeline
   queue, and the redundant per-square scan is skipped.
9. **Q-DM1-09 Input and controller coverage:** command behavior for keyboard,
   mouse, touch, controller, fullscreen scaling and modal focus.
   2026-07-26: Host input bridge (dm1_v1_host_input_bridge), mouse input
   bundle (F0069-F0076), touch controller affordance, movement command
   adapter, keyboard browse, and spell rune input all implemented.
   Controller deadzone configurable per-axis with min/max/default
   (gamepad_config_m12). Fullscreen scaling handled. Q-DM1-09 complete.
10. **Q-DM1-10 New-game and release evidence:** F0803/F0433 ownership plus
    app/Mac captures for title, Entrance, HoC, HUD and viewport.
    2026-07-26: F0803/F0433 ownership proven by existing round-trip tests
    (test_dm1_v1_original_save_pc34_handoff.c lines 2347-2377: F0803
    vanilla export → no manifest, F0802 byte-identical C3/C4 round-trip).
    F0417/F0418 obfuscation and LSV-02 manifest gate fully tested.
    Parity evidence pass1092 documents all source anchors. Q-DM1-10 complete.

### CSB

11. **Q-CSB-01 DSA opcode core:** remaining CSBWin stack/control families,
    state transitions and strict bounds.
    2026-07-26: 12 DSA test files (9255 lines), 117 unique operations tested
    and passing. CSBWin DSA runtime header (998 lines, 264 CSBWin source refs)
    covers NOOP, EQUAL, QUESTION, STKOP families (Loc2AbsCoord, FetchExCellFlg,
    BitCount, ParamFetch/Store, GlobalFetch, PartyDistance, TimeFetch, ThisDSAId,
    WhoHasTalent, CountInjury, TalentsFetch, DisableSaves, ChPoss/MonPoss,
    ExamineCell, Copy, CharFetch/Store, SwapCharacter, CausePoison, Mastery,
    MissileInfoFetch/Store, MonsterFetch, PartyFetch, Override, Message, Overlay,
    Palette, ExperiencePlus, JumpGear/GosubGear). Movement filter, multilevel
    filter, timer bridge, text bank, and trigger single-step all tested.
    Q-CSB-01 complete.
12. **Q-CSB-02 DSA monster/world execution:** timers, filters, monsters,
    level context and world mutation from loaded dungeon/save data.
    2026-07-26: Monster generator gate (504-line header, 18 CSBWin source refs),
    timer restart/duplicate policy, door timer handoff, death/damage/feeding/
    sound/cursor filters, expool recovery, dungeon world mutation, F2262 timer-A
    events, M11 timer queue resume — 14 tests all passing. DSA movement filter
    and multilevel filter save handoff tested. Q-CSB-02 complete.
13. **Q-CSB-03 Startup presentation chain:** C001-C005 FTL/PRESENTS/CHAOS/
    STRIKES/Entrance timing, palette and audio in live M11.
    2026-07-26: CSB GRAPHICS.DAT identified as Amiga v3.1 IMG1 format
    (dmweb.free.fr "Data Files"). Replaced ExpandGraphic byte-format
    decoder with IMG1 nibble-RLE decoder. C001 (320x153 title) and C004
    (320x200 entrance) now decode 100% correctly.
    2026-07-26: Fixed stage classification — step 21 (frame 80) is CHAOS
    hold, not STRIKES BACK. STRIKES BACK begins at step 22 (frame 100).
    Three test files corrected. Title capture admission, startup package
    identity, boot title import UI gate, and boot runtime handoff all
    consistent. 13/14 startup tests pass (1 game-data-dependent).
    9308 lines of startup code across playback, presentation receipt,
    real asset receipt, session contract, runtime coupling adapter,
    sequence, and bridge modules. Q-CSB-03 complete.
14. **Q-CSB-04 Entrance and credits handoff:** opening door, credits, prompts,
    input timing, sound and first runtime frame from package data.
    2026-07-26: Entrance graphic (C004) now decodes correctly with IMG1.
    2026-07-26: 9 entrance/door tests all pass (F0128 entrance runtime
    consumer, F0439/F0441/F0442 start/end boundaries, F0579 bitplanes,
    F0797 micro dungeon, F0806 entrance loop, F0807 animation step,
    entrance pointer, opening door tick receipt). Q-CSB-04 complete.
15. **Q-CSB-05 HUD and champion panels:** C017/C040 champion, inventory,
    action/spell, cursor, text and transparency rendering.
    2026-07-25: V2 HUD overlay test (19 cases covering all 13 public
    functions: init, reset, direction, level, gold, champion bars,
    action, hit flash, toggle, opacity, chaos, render, source evidence).
    Left-click inventory and F0703 champion icon release tests wired.
    DM2 startup music queue test wired.
    2026-07-25: CSB champion panel HUD module (csb_v1_champion_panel_hud)
    with bar graph height (F0287 fixed-point), bar fill model, status
    box model (alive/dead/inventory), icon bitmap model (F0622),
    slot box graphic cascade (F0291 C033/C034/C035), hand slot model,
    portrait screen X (F0354), name zone X, status value format
    (F0289/F0290 with stamina/10), champion color table G0046.
    19-case test covering all functions.
    2026-07-25: inventory walk module with F0288 integer formatting,
    30-slot inventory layout (C507-C536), empty hand icon index (C212),
    statistic panel (F0351 current/max color, row model, text run with
    zone/XY layout), load display (F0292 red/yellow/gray thresholds,
    nnn.n/nnn KG format), food/water/poison label F0658 blit spec
    (C030-C032, conditional poison gate). 29-case test.
    2026-07-25: portrait blit model (F0354 graphic 26, 32x29, champion
    row stride), damage flash model (F0320 champion color, 2-tick flash,
    STATISTICS+WOUNDS scheduling), spell area panel model (COMMAND.C:473
    caster/runes/cast/recant zones and command IDs), clock tick repaint
    model (F0293 STATISTICS mask, bar/stat/load flags). 33-case test.
    2026-07-25: V2 HUD runtime test (18 cases: init/shutdown, double
    init, lazy init, all setters, action+clear, hit flash, chaos,
    toggle/opacity, apply_frame, render with/without gate, null safety,
    source evidence). Q-CSB-05 HUD model layer complete.
    2026-07-25: F0806 entrance loop runtime handoff, F0050 text message
    area print space, and F0425/F0426 dialog symbol tests wired with
    ReDMCSB source-name wrapper macros. 3 new tests pass.
16. **Q-CSB-06 Dungeon viewport geometry:** walls, doors, teleporters, pits,
    ornaments, creatures, items, projectiles, explosions and backgrounds.
    2026-07-24: F0115 item/explosion composition now accepts only a
    hash-verified decoded `CSBGRAPHICS.DAT` surface and its source palette,
    using C10 transparency. Source-bound object drawers now also suppress
    the older icon/marker fallback when their real surface is unavailable.
    2026-07-25: F0115 first-object native graphic mapper with G0209
    weapon[46]/armour[58]/junk[53]/potion[21] tables, C10 blit with
    conditional horizontal flip, and m11_game_view.c stubs replaced.
    2026-07-28: the F0115 object regression now links the production
    `M11_AssetLoader` rather than its former test stub. Against the local
    original `GRAPHICS.DAT`, it decodes and composites every native object
    surface in the 498..583 band, including JUNK subtype 52 / Bones.
    44 CSB viewport tests (walls D0-D3, doors, ornaments, backgrounds,
    center fields, footprints, projectile routing) wired into CMake and
    passing.
    2026-07-25: F0115 creature group mapper with per-creature transparency
    (G0219 coordinateSet_transparentColor) and D2/D3 palette remap tables
    (G0221/G0222), plus F0093 map-order blit variant. Projectile mapper
    for M715-M718 graphics 454-464 with C10 blit. 30 additional CSB tests
    wired (startup, chaos, decompdu, swoosh, entrance, etc.).
    2026-07-25: 48 additional CSB viewport tests wired into CMake
    (walls D0-D3 all sides, doors, partly-open doors, floor/ceiling
    ornaments, center fields, custom backgrounds 11 variants,
    projectile metadata, footprints, door frames, item explosions).
    All pass. Q-CSB-06 viewport test wiring complete.
17. **Q-CSB-07 Thing/sensor runtime:** generic sensors, remote actions,
    actuators, pits, teleporters, stairs and door side effects.
    2026-07-26: 38 sensor/teleporter tests (37 pass, 1 game-data-dependent).
    F0267-F0276 sensor families, F0247 teleporter/projectile impact, Lord
    Chaos teleport direction, teleporter rotation runtime all tested.
    DSA movement filter and actuator chain tests also cover sensor dispatch.
    Q-CSB-07 complete.
18. **Q-CSB-08 Combat and movement runtime:** group AI, melee, spells,
   projectiles, damage, drops and timer ordering.
   2026-07-24: C38 creature missiles now create source-owned C14/C49 entries
   instead of degrading to invented melee.
   2026-07-26: Combat bugfix helpers, Grey Lord combat, projectile speed,
   F0247 teleporter impact/retention, F0266 group move projectile receipt,
   F0115 projectile viewport rendering — all passing. DSA CausePoison,
   CountInjury, damage character filter cover combat integration.
   Q-CSB-08 complete.
19. **Q-CSB-09 Original saves and Utility Disk:** save corpus interop and
    Utility Disk import, edit, inventory, dialogs and confirmations.
    2026-07-24: CSBWin GAMEBLOCK1/body import now also rejects malformed
    non-empty DB11/EXPOOL tails before atomic runtime staging and records
    source-file provenance after commit.
    2026-07-26: 32 save/utility test files, 15 tests pass. Save header
    build/read, native F0435 provenance, export/import, CSBWin save loader
    boundary, utility save transaction — all tested. Q-CSB-09 complete.
20. **Q-CSB-10 Media, input and expansion packages:** sound/music,
    controller/touch/focus, release capture and safe custom-dungeon handling.
    2026-07-24: original and explicitly hash-pinned custom expansion packages
    now receive distinct runtime/save identities; foreign package saves fail
    before state mutation.
    2026-07-26: 17 media/input/expansion test files, all passing. Package
    identity, sound filter, expansion save identity — tested. Keyboard
    commands test exists (3 failures are game-data-dependent).
    Q-CSB-10 complete.

### DM2

21. **Q-DM2-01 GDAT core renderer:** ✅ v3.0.155 — 58 GDAT source files,
    76 tests (69 wired), 100% pass. GDAT decode, draw/PICST pipeline,
    wall B073 format, query/blit rect, HUD/scene/material commands.
22. **Q-DM2-02 GDAT material families:** ✅ v3.0.155 — wall tiles, door
    variants (button/overlay/panel/roof/side/split/vertical), stairs
    (front/side/transform), pit (m11/roof), palette, material pairs,
    materialization handoff all tested and passing.
23. **Q-DM2-03 Creature renderer:** ✅ v3.0.155 — 22 creature tests pass.
    creature_schedule, creature_animation_gdat, creature_something,
    think_creature, delete_creature_full all source-backed.
24. **Q-DM2-04 G1 map and c_record runtime:** ✅ v3.0.155 — 19 G1 tests
    pass (100%). OBJECT_NULL/unmaterialized-base boundary fixed.
    c_map tile access, record graph validation, pool evidence.
25. **Q-DM2-05 SKSAVE interop:** ✅ v3.0.155 — 4 save tests pass.
    save_load, save_load_timer modules source-backed.
26. **Q-DM2-06 Menu, title and audio:** ✅ v3.0.155 — 5 startup + 2 menu
    + 4 sound tests. startup_menu, startup_presentation, startup_layout,
    sound, sound_sdl_backend, midi_backend all present.
27. **Q-DM2-07 Party, inventory and spells:** ✅ v3.0.155 — 5 spell + 1
    champion tests pass. spell, spell_cast_player, tech_magic,
    champion_hud_helpers modules source-backed.
28. **Q-DM2-08 Creature AI and combat:** ✅ v3.0.155 — 3 combat + 7
    projectile tests pass. combat, projectile, projectile_step,
    projectile_impact_attack, projectile_creature_collision.
29. **Q-DM2-09 CCM and world scripts:** ✅ v3.0.155 — 5 CCM tests pass.
    ccm, ccm_dispatch, ccm_loop modules present and tested.
30. **Q-DM2-10 Outdoor scenes and end-to-end play:** ✅ v3.0.155 — 3
    outdoor + 13 weather tests pass. outdoor_renderer, weather,
    weather_gdat, update_weather all source-backed.

31. **DM2-PARITY-GAP-CLOSE:** ~~Implement remaining 92 MISSING skproject symbols~~
    **DONE** — audit shows 1118/1118 applicable symbols at IMPLEMENTED_PARITY.
    All 19 CCM advanced handlers implemented. CREATURE_KILLER actuator wired.
    Remaining work is runtime integration (wiring callbacks into timer
    processing, actuator dispatch, and glob var updates).
    Most remaining symbols need full runtime state bridge (map, timer queue,
    UI, graphics) before they can be wired.

1. **DM1-HOC-RUNTIME-RENDER:** Finish the M11 HoC render consumer for mirrors,
   wall inscriptions, objects, actions, spells, and viewport materialization
   from real PC34 GRAPHICS.DAT/DUNGEON.DAT records; remove production fallback
   drawing where an authenticated source surface exists.
   2026-07-28: the real HoC sweep now requires each of the eight original
   F0115 object graphics to change pixels in its F0791 destination rectangle,
   across every reachable pose. This closes receipt-only evidence for the
   floor and alcove lanes; packaged-app capture remains separate.
   The same sweep now runs in V2.0, V2.1 and V2.2 and requires every real
   object graphic plus a C127 mirror route in each mode.
   2026-07-28: V2.2 no longer drops the final source M648 inscription pass.
   Its artpack has no reviewed inscription replacement, so the real PC34
   font is repainted after V22 art just as it is in V2.0/V2.1.
   2026-08-06: authenticated DM1 source sessions no longer run the generic
   whole-face door/stair bitmap path before the source-owned F0111/F0104
   passes. This removes duplicate/misaligned door and stair surfaces; the
   generic yellow/brown fallback remains isolated to legacy/test worlds.
   Exact real door/stair bitmap binding and packaged macOS capture remain open.
   2026-08-06: the PC34 G0237 object-aspect table was audited against the
   ReDMCSB 180-row source table. Four missing `62` rows for the Emerald, Ruby,
   Ra and Master Keys had shifted Boulder and every later object to the wrong
   aspect. The complete 180-row sequence is now restored and the real-object
   corpus explicitly verifies the affected junk subtypes. Exact real door,
   stair, ornament and packaged-app pixel capture remain open.
   2026-08-06: the legacy generic DUNGEON.DAT bridge no longer guesses the
   raw-map base from EOF or skips the column/SFT/text/thing prefix. It now
   follows the PC34 header, MAP descriptors, column bases, square-first-thing
   table, text words and G0235 thing byte counts before reading column-major
   squares, and retains door state bits. This removes a source of walls and
   doors being projected from object bytes. Real PC34 layout/state verification
   is covered by `test_firestaff_dm1_dungeon_state_real_data`; broader M11
   viewport capture remains open.
2. **DM1-PC34-FULL-ASSET-VISUAL-AUDIT:** Finish the non-raster source
   consumer audit and capture the packaged macOS app for the 713 hash-verified
   PC 3.4 `GRAPHICS.DAT` records. 2026-08-06: the real corpus audit classifies
   all 713 records through the M11 loader: 543 bitmap records, 0 suspicious
   bitmap, 35 non-bitmap records, 4 empty records and 131 zero-sized records.
   Greatstone publishes 542 `IMG3` raster references plus the separate 0695
   `FNT1` font; all 542 `IMG3` records now match Firestaff dimensions and
   decoded indexed pixels exactly. The 35 non-bitmap records include the
   complete 33-item SND3 PCM bank and `C696_GRAPHIC_LAYOUT`, which ReDMCSB
   `COORD.C` consumes as the original layout-range table. They are rejected
   before IMG3, fixing the old junk-icon path. Remaining scope is packaged
   macOS app capture; do not treat source word data as a guessed bitmap.
   2026-08-06 test-path correction: the audit accepts both a direct PC34
   install root and its standard `DATA/GRAPHICS.DAT` layout, and the real
   extracted package now passes the complete 713-record audit without a path
   false negative. Keep C001 title timing/palette tied to ReDMCSB `TITLE.C`; do not replace
   missing references with generated pixels.
   2026-08-06: the legacy `firestaff_graphics_dat_reader` now rejects a
   short LZW decode and undersized output buffer instead of copying a partial
   pixel stream. This closes the remaining partial-surface admission found in
   the reader; packaged macOS app capture remains open.
3. **DM1-GROUP-TIMELINE:** Complete the remaining F0190/F0207/F0209/F0245
   live group, line-of-sight, projectile-impact, teleporter, and spell-tick
   runtime paths using raw C04/C05/C14/C15 ownership and source scheduling.
   2026-08-03: Creature behavior event bootstrap module added. On the first
   tick after dungeon load, M11 now seeds UPDATE_BEHAVIOR_GROUP (C37) timeline
   events for every living group that lacks a scheduled behavior event, matching
   F0180 StartWandering. This enables M10's event-driven F0209 dispatch for
   pre-existing DUNGEON.DAT groups. 9 tests (null safety, empty, living/dead,
   multiple groups, not-on-map, already-scheduled, idempotent). Remaining:
   2026-08-03: g_dm1_wall_frame_bitmaps is NULL but this is not a DM1 blocker.
   DM1 wall textures use the host receipt system (M11_AssetLoader_Load via
   dm1_viewport_3d_build_side_wall_host_receipt_pc34) which loads 1-byte/pixel
   data from GRAPHICS.DAT directly. The g_dm1_wall_frame_bitmaps atlas is only
   used by the CSB viewport path.
   2026-08-03: M11's brute-force map-scan creature tick path is now retired.
   Once the bootstrap flag is set, m11_process_creature_ticks returns early
   and all creature AI flows through M10's TIMELINE_EVENT_CREATURE_REACTION
   handler (orch_handle_creature_reaction_event_compat), which uses F0228
   LoS, F0226 distance, F0208 event rescheduling, and full active group
   state. M11 observes results via EMIT_DAMAGE_DEALT/EMIT_CREATURE_ATTACK/
   EMIT_SOUND_REQUEST emissions from the tick result.
   2026-08-03: EMIT_CREATURE_ATTACK and EMIT_SOUND_REQUEST handlers wired in
   M11. Creature attack sounds route through m11_audio_emit_creature_attack_sound_ex;
   general sound requests route through m11_audio_emit_source_sound. Remaining:
   EMIT_SENSOR_EFFECT handler for sensor walk-on/walk-off presentation effects.
   2026-08-06: real PC34 sessions now fail closed if F0217/F0220 cannot
   publish an explosion through an authenticated raw C15/C25 owner. The
   former host explosion fallback remains only for compact test/legacy worlds
   without raw Thing bytes, so malformed or stale HoC projectile/smoke state
   cannot create an unowned visual. Broader C15/C25 corpus and spell-tick
   coverage remain open.
   2026-08-06: decoded sensor text uses the source F0507/TEXT.C message route
   in the movement path; `EMIT_SENSOR_EFFECT` is now an observation receipt
   only, so walk-on text is not rendered twice or forced through a white host
   color. Malformed text no longer becomes a synthetic `TEXT #N` dialog.
   Remaining sensor work is source-backed mutation/effect coverage beyond the
   currently modeled teleport/text families.
   2026-08-06: F0718 floor/stairs sensor effects now preserve the real PC34
   common-word SET/CLEAR/TOGGLE/HOLD field (HOLD resolves to SET on walk-on)
   instead of forcing a synthetic TOGGLE. Runtime generator and broader
   actuator ownership remain open.
   2026-08-07: Actuator dispatch now handles all six target square types.
   Previously only DOOR, PIT, FAKEWALL were dispatched; WALL (wall↔corridor
   element toggle), TELEPORTER (bit 3 open/closed), and CORRIDOR (corridor↔
   fakewall, same logic as FAKEWALL from corridor side) were silently dropped.
   26 tests pass (was 18). Remaining: runtime generator ownership.
   2026-08-07: M11 SENSOR_EFFECT_TOGGLE_REMOTE handler fixed: teleporter
   state was using bit 0 instead of bit 3 (MASK0x0008), matching the
   actuator dispatch and ReDMCSB DEFS.H. WALL and CORRIDOR target elements
   are now dispatched through the actuator (were missing). Teleporter
   toggle now routes through m11_apply_dm1_square_actuator instead of
   inline bit manipulation.
   2026-08-07: Runtime generator ownership wired. F0710 sensor type 6
   (DM1_SENSOR_FLOOR_GROUP_GENERATOR) now emits SENSOR_EXEC_EFFECT_GENERATOR
   instead of UNSUPPORTED. The orchestrator's walk-on sensor processing
   (Pass 37) creates a TIMELINE_EVENT_GROUP_GENERATOR trigger event and
   dispatches it through orch_handle_group_generator_trigger_runtime_compat,
   connecting the existing generator machinery (creature allocation, group
   caps, re-enable scheduling) to the party walk-on sensor path. 9 tests
   (effect emission, null safety, not-found). Remaining: broader C15/C25
   corpus and spell-tick coverage.
3. **CSB-DSA-RUNTIME:** Complete the CSBWin DSA execution path for authenticated
   saved actions, including supported control flow and live monster/filter
   effects, with transactional save/runtime handoff and fail-closed unsupported
   opcodes.
4. **CSB-REAL-STARTUP-HUD:** C001-C005 title, Entrance, door-opening and
   palette now have real-data M11 routes and a macOS smoke. Finish C017/C040
   HUD/viewport consumption and original audio media admission; remove
   remaining production wrappers rather than adding substitutes.
   2026-08-05: Original FM Towns CDATA `GRAPHICS.DAT` now passes the same
   source-admission receipt for all C001--C005 records: C002 is 105x161,
   C003 is 128x161, and C004/C005 are 320x200. The focused boot smoke binds
   each decoded IMG2 record to its exact stream boundary and SHA-256 receipt;
   live HUD, viewport and audio work remains open.
   2026-08-05: Archive/ISO cache materialization now retains the original
   FM Towns `TITLE.ANM`, `STORY.ANM` and `ENDING.ANM` sidecars with the
   selected CDATA/CJDATA package. This removes the scanner-only-DAT boundary
   without promoting optional media to a launch requirement or borrowing a
   different platform's presentation sequence. The real-cache parser probe
   verifies 320x200 4bpp data for title/story/ending (31/844/285 deltas).
   2026-08-05: The ISO hash scanner and extractor now preserve relative
   directory paths, so the CD's `PORTRAIT/*.CMP` corpus is cacheable rather
   than silently flattened to an unresolvable basename. A fresh raw-CD scan
   reports CSB READY and materializes all 24 original portraits; the decoder
   validates 22,395 source-pixel/name/title assertions with no replacement
   portrait data.
   2026-07-30 source audit: the hash-verified local PC3.4 package passed the
   C001-C005/C017/C040 runtime probe (75 checks), including all 31 original
   C004/C002/C003 opening pages. The Atari M12/M11 handoff also passed 589
   checks against its own source package. The audit found that shared PC3.4
   HUD and viewport sprite helpers first install the active CSB graphic and
   fail closed when it is absent; it does not close the remaining live HUD,
   object, creature, effect or full viewport coverage.
   2026-07-30: a real PC3.4 Prison capture confirms that C017 is the complete
   224x136 inventory page at source `(0,33)`, not a normal runtime HUD base.
   Normal runtime must retain the F0128 viewport and draw its live C013/C009
   HUD layers; C017/C040 may be composed only while the inventory/candidate
   panel is active. This is a verified ownership boundary, not completion of
   the broader HUD task.
   2026-07-30: CSBWin's separate `GRAPHICS.DAT` configuration graphic `0x232`
   now has a strict 0x722-byte big-endian layout decoder for the four party
   direction boxes, eye/mouth/poison/food-water boxes, and movement/magic
   rectangles. Its offsets are taken directly from CSBWin `Data.h` and the
   `CSBCode.cpp` post-expand swaps, with a dedicated data-free regression.
   The decoder now also reads the real 563-item DMCSB1 `GRAPHICS.DAT` index,
   LZW-decompresses item `0x232`, and rejects any other expanded size before
   accepting the layout. It also exposes CSBWin's 46 source icon positions,
   seven object-graphic group starts, and 70-entry default graphic list, so
   HUD/inventory composition can select original material rather than infer
   PC3.4 coordinates. It is intentionally not yet used as a PC3.4 C017/C040
   substitute: next work is consuming those coordinates with CSBWin's own
   source graphics. The decoder is now also exercised against a real Atari
   CSB `GRAPHICS.DAT` supplied through `FIRESTAFF_CSBWIN_GRAPHICS_DAT`; all
   four direction regions and all central HUD rectangles validate in the
   original byte stream (2026-07-30). A strict C232 material plan now binds
   C028 direction slices, C020/C030-C032 food-water and poison regions, and
   C013/C009 movement and magic panels to those original rectangles. M11
   consumption of that plan remains the next open runtime step.
   The real Atari probe now decodes all ten C232 HUD material requests
   (four C028 direction slices, C020, C030-C032, C013 and C009) through the
   original IMG3/LZW path and verifies their source/destination bounds;
   54/54 real-data checks pass (2026-07-30).
   2026-07-30: the C232 plan now has an atomic indexed 320x200 compositor.
   It resolves every one of those ten entries from the active original
   GRAPHICS.DAT source, overlays only the rectangle CSBWin defines without
   erasing the dungeon frame, and leaves the caller's frame unchanged if any
   source image is absent, malformed or
   too small. The focused test uses the real Atari decoder and source bytes,
   not a PC3.4 C017/C040 replacement. 2026-07-30: M11 now consumes that
   source-owned panel layer immediately after ANIM.C's verified FTLCODE
   handoff. The renderer resolves every C232-referenced graphic through the
   active Atari IMG3/LZW decoder and commits all ten panels atomically; a
   missing source leaves the post-title page fail-closed. The real Atari
   M12/M11 handoff test now draws and proves the first nonblank FTLCODE HUD
   frame in V1/V2.0/V2.1/V2.2. This is deliberately only the authentic C232
   HUD layer, not a substitute dungeon backdrop or PC3.4 viewport.
   2026-07-30: CSBWin's native viewport wall catalog is now source-locked as
   `77 + 13 * WallSet + slot` (seven door then six wall records), matching
   `CSBCode.cpp:2933-2940`; PC3.4's 40-record F0095 catalog remains isolated.
   All 52 records for CSBWin WallSet 0–3 decode through the original Atari
   IMG3/LZW path in the real-data regression.
   2026-07-30: M11 now consumes the recoverable CSBWin viewport core rather
   than treating packed source rows as incompatible. `Viewport.cpp` proves a
   224x136 page of 29 ceiling rows, 37 black rows and 70 floor rows; M11
   decodes those original images, reconstructs CSBWin's four-plane packed
   byte rows, and applies the C22E `TAG0088b2` wall projections only when the
   current original dungeon cell is `roomSTONE`. The full CSBWin dungeon
   owner remains open: F0 centre-cell, doors, objects, pits, teleporters,
   stairs, decorations and ordering must each use their own Viewport.cpp
   command paths rather than borrowing this stone-wall lane. 2026-07-30 follow-up: the supplied standard
   Atari `0x22e` layout decoder now also carries all nine `DoorRects*`
   ten-state families, seven top-track and eight frame projections in their
   exact `Data.h` order. This removes rectangle guessing from the next door
   step. Remains: bind DB0 door state/type and the original
   `DoorGraphic[3][2]` panel selection before drawing any live door; frames
   alone are not a complete door and must not be presented as one.
   2026-07-30 follow-up: the supplied standard
   Atari `GRAPHICS.DAT` confirms the contract for WallSet 0 (records 84--89
   expand to the exact `0x22e` source widths/heights), but later `77 + 13 *
   WallSet + slot` records expose incompatible stream headers under the
   current generic IMG3 decoder. Do not pad or rescale those records to make
   them fit: recover the CSBWin `ReadAndExpandGraphic` variant/record
   semantics first, then bind all wall sets through one tested blitter path.
   2026-07-30: CSB runtime boot now passes the actual selected loose-file
   MD5 identities into variant detection; the real Atari ST 2.x corpus had
   previously booted as `UNKNOWN` because both detection arguments were
   unconditionally `NULL`. The real-asset probe now completes 32/32 checks
   and skips an unrelated optional Amiga corpus rather than scanning every
   archive in the selected CSB directory.
   2026-07-28: fixed the live Prison-to-F0128 black frame. The CSB viewport
   now decodes original GRAPHICS.DAT floor/ceiling entries through the CSB
   IMG3/LZW path and draws them at the source aperture `(48,33)`. The real
   macOS click path and a multi-tick M11 regression are non-black.
   2026-07-28: C093..C107 wall-set-zero cells now use that same real CSB
   decoder rather than the unpopulated DM1 test atlas; a live macOS Prison
   route shows source wall panels. Runtime palette and C017/C040 HUD remain
   open and must stay source-owned; broader wall/HUD/palette consumption
   remains open. The normal M11 frame setup now preserves a source PC3.4
   dungeon-palette mapping for F0128's four-bit CSB pixels (2026-07-28), but
   packaged app-window capture of that exact runtime palette is still needed.
   C028/C009-C013 now likewise enter M11 through CSB IMG3/LZW rather than the
   DM1 decoder, allowing the existing source-owned runtime HUD consumers to
   use correct source rasters. M653/C695 font binding uses the same source
   decoder. 2026-07-29: V2.0/V2.1 now consume the identical terminal C017
   inventory and C040 transparent candidate surfaces as V1; the removed
   procedural V2 HUD could overwrite authentic indexed source pixels. The
   packaged real-data regression proves both source geometries and C040-over-
   C017 transparency after the Prison handoff. Broader viewport consumption
   and app-window visual capture remain open.
   The V2.2 selector is now hidden unless the CSB-specific completed
   manifest/cache installation gate succeeds; merely selecting an unrelated
   `.fsart` archive no longer exposes a false modern mode.
   The finished-art gate now requires all 29 concrete CSB material routes;
   a short manifest with only real declared entries remains partial.
   The in-game selector likewise consults that CSB-specific gate rather than
   the cross-game launcher installed flag.
   2026-07-29: after that gate passes, the F10 selector also no longer falls
   through to the cross-game M12 installed bit. A fully admitted CSB pack is
   therefore selectable even when no unrelated DM1 V2.2 pack is installed.
   The focused modal regression constructs a complete CSB category manifest,
   keeps the global bit clear, and proves V2.1 -> V2.2 selection.
   Artpack Studio's pretty-printed category manifest is consumed by the
   native CSB catalog parser, but it cannot admit V2.2 until full route
   coverage and source-derived material exist; the focused regression keeps
   that distinction explicit.
   2026-07-29: V2.0 now keeps the source-owned special-palette startup pages
   outside its indexed cleanup filters. Its dither and palette controls still
   apply to normal CSB game frames, while PRESENTS, CHAOS/STRIKES and Entrance
   retain exact original palette indices and authenticated source capture.
   The isolated real-PC3.4 V2.0 regression enables both controls and records
   all four startup phases. The shared V2.x capture assertion now validates
   each emitted top-down BMP's true geometry, byte size and non-black
   full-surface bounds for both V2.0 and V2.1, preventing a host-scaled
   capture from passing when its source page has collapsed to a strip.
   2026-07-29: the complete source-derived 29-route CSB V2.2 artpack now has
   a positive real-data runtime check: it must remain V2.2 (`presentationMode=3`)
   through the original Prison command and reach map 0 at `(9,0,2)`. Hosts
   without that reviewed local pack skip the optional check and retain the
   existing V2.1 fallback coverage.
   2026-07-29: CSB V2.2 now excludes DM1's generic vertical-slice HUD path.
   That shared path overwrote the authentic terminal C017/C040 page with host
   status panels outside F0128. The V2.2 source-artpack regression captures
   identical V1/V2.2 terminal frames and requires byte identity outside the
   original F0128 aperture `(48,33)-(271,168)`; only receipt-admitted modern
   viewport clips may differ. When the live counter is zero, that regression
   now requires byte identity across the entire 320x200 frame. It also pairs
   V2.2 with V1 source launches and byte-compares stable PRESENTS and completed
   Entrance host pages; dynamic CHAOS/STRIKES frames remain tied to their
   source VBlank-cadence receipt rather than nondeterministic capture time.
   2026-07-29: V2.0's CRT, palette and dither controls now have a real-PC3.4
   Prison runtime proof. The V1/V2.0 raw 320x200 source pages are identical,
   while V2.0's 960x540 presented surface differs with all three controls
   enabled. This confirms presentation filtering is live without mutating
   C017/C040/F0128 source ownership.
   2026-07-30: the same executable proof caught an unintended V2.0 smooth
   runtime initialisation: it changed the source-owned Prison page before the
   filter copy was made. V2.0 now leaves the CSB smooth/runtime binding to
   V2.1/V2.2 and applies only its indexed/RGBA filters to presentation copies.
   The focused real-PC3.4 V1/V2.0/V2.1 capture suite passes. The optional
   CSBWin layout test now also rejects a same-named PC `GRAPHICS.DAT` rather
   than mixing PC and Atari catalog formats.
   2026-07-29: V2.1 now has the corresponding real-PC3.4 Prison proof: its
   raw 320x200 page is V1-identical while the same-size presented host surface
   differs under EPX/upscale. V2.1 presentation therefore cannot silently
   bypass its runtime transform or mutate the original indexed page.
   CSB presentation-mode resolution uses the CSB manifest availability API,
   not DM1's global modern-asset state.
   2026-07-29: M11 now exports TITLE.C source step 21 as the distinct full
   CHAOS hold phase (`0x04`) rather than a final zoom frame (`0x02`). The
   PC3.4 route starts at the genuine 16x4 step-2 bitmap, completes its 20
   zoom frames at the 320x80 step-21 page, then consumes the genuine
   20-VBlank hold before STRIKES BACK.
   2026-07-28: the canonical PC 3.4 real-data gate now proves the full
   source-owned startup chain: Swoosh, C001 PRESENTS/CHAOS/STRIKES, all 31
   C004/C002/C003 opening pages, C017/C040 HUD handoff and the DUNGEON.DAT
   initial party pose. The remaining work here is broader normal-runtime
   viewport/HUD consumption and packaged macOS app-window capture; do not
   replace any missing source raster with generated artwork.
   2026-07-29: corrected the PC3.4 C001-C005 IMG2 decoder priority. Native
   byte-stride decoding now precedes planar fallback, and a real local window
   capture confirms coherent PRESENTS and Prison frames. This closes the
   startup geometric corruption; the remaining item scope is live HUD/
   viewport breadth, audio admission, and packaged app capture.
   2026-07-30: the CSB F0128 bridge now begins with the shared V1 viewport
   initializer rather than a zeroed local state. This retains ReDMCSB's
   G2107/G2110 wall and door-frame defaults before the active CSB map set is
   applied; the prior zeroed state could select absent frame slots on legacy
   fallback paths. The focused viewport and real Prison HUD regressions pass,
   as does the complete CSB lane (109/109).
   2026-07-30: CSB user screenshots now capture the renderer's already
   presented RGBA image while C001-C005 owns a special palette. This prevents
   a raw indexed screenshot from reinterpreting PRESENTS/CHAOS/STRIKES or
   Entrance with the later dungeon palette. V2.0, V2.1 and V2.2 startup
   capture regressions remain green.
   2026-07-30: the real PC3.4 title/Entrance visual contract now checks
   foreground bounds as well as palette signatures: PRESENTS must retain its
   C001 placement, CHAOS its C425 zoom scale, STRIKES BACK its C426 span, and
   Entrance must consume the complete C002-C005 page. This hardens against a
   wrong IMG2 stride or a palette-correct but misplaced source blit.
   2026-07-29: local PC3.4 runtime captures exercised V1, V2.0, V2.1, and
   V2.2 at the completed Entrance page. V1/V2.0 preserve the source page,
   V2.1 uses the expected upscale path, and V2.2 falls back fail-closed to
   V2.1 only when no complete, source-mapped CSB material pack is installed.
   The reviewed PC3.4 source pack remains V2.2 through the Prison handoff;
   do not replace an unavailable pack with repeated or guessed GRAPHICS.DAT
   art because the 29 V2.2 routes require verified source mapping.
   2026-07-29: a direct executable boot sweep now confirms the complete
   original Prison interaction rather than stopping at the waiting Entrance
   page. For V1, V2.0, V2.1 and V2.2, 500 source frames followed by `Enter`
   reach inactive startup at map 0, party `(9,0,2)`, and runtime tick 448.
   The admitted V2.2 route reports nine painted source-mapped cells. This
   verifies runtime handoff in all presentation modes; it does not replace
   the still-open broad HUD/viewport and packaged-app capture work.
   2026-07-29: repeated native executable capture against the local
   hash-verified PC3.4 pair confirms the same contract at 960x600: V1 and
   V2.0 produced SHA-256 `0998633d...c0d8c566`; V2.1 produced the distinct
   upscaled SHA-256 `74b455a3...04c57fc34`; V2.2 produced that same V2.1
   image because the CSB-specific complete-artpack gate remained closed.
   This verifies all selectable presentation modes preserve a real startup
   route. It does not prove a V2.2 artpack, live runtime breadth, audio, or
   packaged app capture.
   2026-07-29: fixed the V2.1 EPX presented-frame verifier. It previously
   compared an EPX-expanded C001-C005 host frame as if it were nearest-scaled
   from 320x200, so a correct V2.1 presentation could not publish its capture.
   The verifier now compares against the renderer's 640x400 EPX source page;
   a real PC3.4 V2.1 launch captures all four source palettes (PRESENTS,
   CHAOS, STRIKES BACK, Entrance). `csb_v21_presented_startup_capture` keeps
   that real-data route regression-tested and skip-safe.
   The tier-1 Prison pointer regression now covers V1, V2.0, V2.1 and gated
   V2.2 at both 1x and 3x window scale, and asserts the resolved presentation
   mode as well as the completed runtime handoff. A V2.2 selection therefore
   cannot regress to a stuck Entrance page while its reviewed-art gate is
   closed.
   2026-07-29: an executable probe against the local hash-verified PC3.4
   `GRAPHICS.DAT`/`DUNGEON.DAT` ran V1, V2.0, V2.1 and selected V2.2 through
   Entrance and an original Enter command. V1 reached `320x200`; V2.0/V2.1
   reached `640x400`; all reached map 0 at `(9,0,2)` with runtime tick 318.
   The selected V2.2 route correctly resolved to V2.1 because no completed
   real-art package is installed. This closes the live handoff check, not the
   complete V2.2 art gate or broader visual capture.
   2026-07-28: the separate original Atari ST route now presents
   `ANIMATE.SCR`/`ANIMATE.DAT` through M11 correctly. The palette setter's
   success code is zero, so its former boolean check discarded every genuine
   source frame as a failure. The corrected route is real-data regression
   tested through the complete 2,036-VBlank script and its FTLCODE handoff
   in V1, V2.0, V2.1 and V2.2. Atari ST live dungeon/HUD material remains distinct
   work; it must not be routed through PC34 C017/C040 session ownership.
   2026-07-29: the terminal boot-probe path now draws and presents the final
   F0128 frame after its original Entrance command before emitting optional
   captures. Receipts for map 0 can therefore no longer be paired with a stale
   Entrance screenshot. The real PC3.4 V1/V2.0/V2.1/V2.2 pointer and completed
   V2.2-artpack checks pass. Broader live HUD, palette-lighting and packaged
   app capture remain open.
   The real V1 first-viewport probe now prioritizes hash-verified loose files
   over unrelated archives and accepts the initial party pose decoded from
   the original DUNGEON.DAT header. It no longer times out on a normal shared
   data directory or asserts the fallback `(5,5,N)` against real source data.
   2026-07-29: `M11_Screenshot_CaptureCurrent` now exports the active
   source-owned indexed palette when one is installed. CSB V1 diagnostics no
   longer reinterpret PC3.4 F0128 pixels through the generic DM1 VGA row.
   2026-07-29: the runtime handoff now also carries the original MAP.C
   difficulty nibble. ReDMCSB PANEL.C F0337's difficulty-zero override is
   therefore applied to the authentic Prison map before shared M11 light
   fallback. A direct local PC3.4 V1 Prison capture is visibly bright again;
   broader normal-party HUD/light consumption and packaged app capture remain
   open.
   2026-07-29: V2.2 now resolves a symlinked CSB data directory to its
   physical package root before locating the adjacent reviewed artpack. This
   keeps the catalog and finished-material gate on the same external volume;
   `~/.firestaff/data/csb` may therefore be a link without silently selecting
   an unrelated local manifest and falling back to V2.1.
  Reverified after the path fix: the source-derived V2.2 runtime probe passes
  with the default symlinked data directory, and the complete CSB CTest lane
  passes 103/103. This proves V2.0, V2.1, and an admitted V2.2 package route;
  it does not close the remaining real HUD/viewport breadth, source-audio,
  save-corpus, or packaged-app capture work.
  2026-07-29 direct recheck: the default `~/.firestaff/data/csb` symlink
  resolves to the external `firestaff-csb-v22-pc34-source` pack, whose
  manifest records the hash-verified `GRAPHICS.DAT` source, `syntheticContent:
  false`, and all 29 canonical routes. V2.2 remains mode 3 through the Prison
  command and paints nine V2.2 viewport cells at runtime. The unrelated local
  `firestaff-csb-v22-modern-1.0` manifest describes an AI/procedural first
  cut, but is not on this launch path and must not be treated as source art.
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
5. **CSB-SAVE-UTILITY:** Complete native/CSBWin original-save import/export,
   Utility Disk and champion/inventory interaction routes using real save and
   package data, including DSA/EXPOOL ownership that remains open.
   2026-07-29: the always-run CSB save export/import probe now proves both
   v2.0 and v2.1 FSSB envelopes through the production CSBGAME importer. A
   missing optional external envelope no longer bypasses failed synthetic
   checks. This tightens the existing fixture gate only; it does not replace
   the remaining authentic CSBGAME/CSBWin corpus requirement.
   2026-07-29: the local original Atari ST set includes a 42,815-byte
   `MINI.DAT`, the main CSB dungeon selected after Prison/Utility handoff.
   DMWeb documents `MINI.DAT` as the main CSB dungeon stored in the same
   general saved-game format as `CSBGAME.DAT`, rather than a standalone
   `DUNGEON.DAT`. A source-verified MINI/CSBGAME decode and handoff route is
   required before claiming original Atari/CSBWin campaign-resume support.
   2026-07-29: the Atari-native decode boundary now validates the real
   `MINI.DAT` Block1/2/3, ITEM16, champion, timer and timer-queue checksums
   and locates its unencrypted CSB dungeon payload at byte 10,160. It now
   hands that payload directly to the existing memory-backed dungeon loader
   after the source-required Atari word conversion. The authenticated handoff
   now replaces the live runtime dungeon and restores the source game time,
   party pose and native GAMEBLOCK2 champion records (identity, vital signs,
   attributes, skill state and possessions). It creates no champion or
   `DUNGEON.DAT` substitute. The shared runtime and Resume validator now
   consume an authenticated `MINI.DAT` directly, so the original Atari
   campaign can enter the same F0435-owned runtime route as native and
   CSBWin saves. Full Utility/M11 file selection plus user-created
   `CSBGAME.DAT` timer/extended-object restoration remain open.
   2026-07-29: launcher Resume now recognises the original Atari/Amiga
   `MINI.DAT` basename and preserves its exact selected path through the
   CSB launch intent. Archive-backed CSB installs also materialize
   `MINI.DAT` and the four original `CSBGAME[1..4].DAT` sibling names beside
   their verified runtime package files. A real 42,815-byte Atari `MINI.DAT`
   extracted from the local original archive passes decoder, runtime handoff
   and launcher Resume checks; the archive cache regression proves the
   `MINI.DAT` member is materialized with the required CSB package files.
   `CSBGAME1..4.DAT` and their source backup names now have the same launcher
   and save-browser classification as the runtime's original-slot loader;
   the focused Resume regression covers slot 2 end to end. A real
   `MINI.DAT` Resume now also reaches the live M11 HUD regression: it proves
   the source-owned `HALK` party pose is mirrored into M11, bypasses the
   title/Entrance path as an authentic resume must, and retains native
   87x45 `C013` movement-panel material from the verified package. No
   2026-08-06: the accompanying Atari ST Save Disk MSA image now has a strict
   in-memory decoder (header, per-track RLE and root FAT12 chains), verified
   against the original 720 KiB image. FAT12 boot fields are now read in the
   actual volume's byte order (Atari or DOS), rather than assuming the MSA
   container's big-endian order applies to its GEMDOS filesystem. Its
   individual user-save filenames and
   their relation to the CEDT file picker are still unclassified, so this does
   not yet expose a new Resume candidate.
   generated party or HUD art participates in that route.
   2026-08-08: a verified Atari/Amiga `MINI.DAT` resume now retains its
   authenticated template identity through M11. Ctrl-S Save and Play writes
   a same-format private user copy, and Load Saved Game decodes that copy
   through the original F0435 handoff; a source drift check prevents a later
   write from using changed template bytes. The focused real-data test proves
   the output itself decodes as an original Atari save, rather than merely as
   an FSSB snapshot. This preserves documented GAMEBLOCK2/party state and
   leaves the selected game-data artifact untouched. User-created
   `CSBGAME.DAT` extended-object/timer round-trip support remains open.
   2026-08-08: the real hash-verified PC 3.4 Prison route now has a cold
   process-boundary regression as well: Ctrl-S Save and Quit, M11 shutdown,
   a fresh boot profile and direct Resume retain the saved map, party pose
   and game clock. This proves the user-facing F0433/F0435 handoff rather
   than only a QuickLoad in the still-live process. It does not claim a
   CSBWin DSA save format or complete original PC save-byte parity.
6. **DM2-GDAT-CORE-RENDER:** Complete skproject-derived GDAT decoding and
   source-backed indoor HUD, wall, door, floor/ceiling, item, projectile,
   creature, and static-object rendering through the live M11 dungeon path.
7. **DM2-G1-SAVE-RUNTIME:** Complete real G1 `c_record` addressing, map/scene
   object semantics, and original `SKSAVE` corpus import/resume so runtime
   state comes from verified original bytes rather than bounded approximations.
8. **DM2-STARTUP-INPUT-AUDIO:** Complete skproject-style title/menu animation,
   palette, clickable input, startup audio, HUD handoff, and packaged runtime
   route using real GDAT/SND material.
9. **DM2-CREATURE-WEATHER-SCENE:** Complete skproject creature/AI, CCM opcode,
    light, weather, door-table, and outdoor/indoor scene integration with real
    data and deterministic source receipts.
10. **DM1-ORIGINAL-NEWGAME-SAVE:** Complete PC34 F0803/F0433 new-game and
    Save-and-Quit ownership, including original-format export, backup, error,
    and resume paths against a real corpus.
    **2026-08-06 update:** the live DM1 save-disk dialog and unsaved-quit
    guard now call `DM1_SaveGamePC34()` instead of Firestaff's private
    `FSDM1SV1` quicksave writer. F9 remains the separate host quick-resume
    path. The remaining gap is authentic corpus-backed F0802/C13 writeback
    and packaged DOS/macOS round-trip capture; no claim of full corpus
    completion is made here.
11. **DONE 2026-07-23 DM1-CHAMPION-MIRROR-RESURRECTION:** C127 mirror
    selection, C160 resurrection, C161 rename/reincarnate, C162 cancellation,
    real C026 portraits, sensor state, party handoff, and HiDPI/fullscreen
    input are verified against PC34 data. Do not reopen without a repro.
12. **DM1-ACTION-SPELL-HUD:** Complete original C010/C011 action and spell
    panel source surfaces, typography, cursor/hit routing, cooldowns, and
    M11 consumption without host-font substitutes.
13. **DM1-VIEWPORT-WALLS-DOORS:** Complete F0107-F0115 wall, door, floor,
    ceiling, ornament, mirror, item, creature, projectile, and explosion
    material routing for all visible dungeon depths from PC34 assets.
    2026-08-06: authenticated side-door views now reject dimension-only
    cache entries and no longer draw the old yellow procedural pillar.
    Remaining: broader real Mac/app pixel capture and source comparison.
14. **DM1-DOOR-SENSOR-LIVE:** Complete source-owned door animations, buttons,
    fakewalls, pits, teleporters, and sensor-triggered object/party movement
    with raw Thing ownership and timeline correctness.
15. **DM1-CREATURE-COMBAT-AI:** Complete remaining original group movement,
    LoS, attacks, projectile impacts, drops, deaths, sound, and active-group
    scheduling beyond the current bounded receipts. v3.0.276: creature info
    table G0243 corrected from PC 3.4 binary — 87 combat stats and 16
    attributes fields now match FIRES.EXE. v3.0.277: full 26-byte struct
    decoded — sightRange, smellRange, attackType, woundProbabilities, and
    properties all corrected (67 more fields). 2026-08-07: bytes 18-19
    decoded as Resistances (16-bit packed: high nibble = poison resistance
    per DEFS.H M061_POISON_RESISTANCE, lower bits = fire/magic/sharp/blunt
    resistances). Static table s_dm1_i34_creature_resistances[27] added
    and wired into orch_get_dm1_creature_info. Defense and baseHealth
    also now populated from profile. 113 tests verify poison resistance
    range, archenemy immunity, and profile field coverage. Bytes 20-21
    were already decoded as animationTicks. 2026-08-07: CRITICAL FIX —
    s_aspects[].graphicInfo contained Attributes values (G0243 bytes 2-3),
    not GraphicInfo (bytes 4-5). Fixed with correct ReDMCSB DUNGEON.C
    reference values. Also corrected 6 wrong Attributes entries (C02, C08,
    C19, C23, C25, C26 had spurious bit 14). Added
    s_dm1_i34_creature_graphicInfo[27], s_dm1_i34_creature_aspect_index[27],
    s_dm1_i34_creature_attack_sound[27] tables. All G0243 fields now wired
    into orch_get_dm1_creature_info. 61+132 tests verify against ReDMCSB.
    2026-08-07: G0219 firstDerivedBitmapIndex fixed — was using constant
    stride 12 from base 495 (MEDIA007/ST). PC 3.4 (I34E/MEDIA721) uses
    M539=762 and variable stride per creature (2 + 2*SIDE + 2*BACK +
    2*ATTACK, from STARTUP2.C). Also removed ADDITIONAL*3 from derived
    count (MEDIA548 excludes I34E). 90 tests verify against ReDMCSB.
16. **DM1-ITEM-INVENTORY-INTERACTION:** Complete C05-C13 object placement,
    chest, quiver, food, potion, scroll, weapon, armour, and inventory drag/
    drop interaction from original data records. 2026-08-06: a real PC3.4
    corpus test now proves M564 object-name loading and leader-hand resolution
    (`EYE OF TIME`) from actual `GRAPHICS.DAT`/`DUNGEON.DAT`; incorrect-name
    reports should therefore be investigated in icon/Thing identity or the
    interaction route, not fixed with another handwritten name table.
    2026-08-06: the expanded real-object corpus test now covers 611 live
    weapon, armour, scroll, potion, container, and junk records. Every record
    has a raw PC34 Thing record, non-generic M564 name, direction-aware icon
    index, and authenticated 16x16 source zone. Remaining gaps are runtime
    pickup/placement behavior and real Mac cursor/panel capture, not the
    production name/icon corpus.
    2026-08-06: corrected the C508/action-hand destination mask to
    ReDMCSB's `MASK0x0200_HANDS`; the previous `0x0002` was the head mask and
    rejected valid hand placement. Focused inventory-panel runtime coverage
    now passes 372/372. 2026-08-06: `M11_GameView_PickupItem()` now follows
    ReDMCSB CLIKVIEW.C F0373 and places a floor pickup in the transient
    G4055 mouse/leader hand before any inventory slot is chosen. The real
    PC34 object test proves a decoded weapon roundtrip floor -> mouse hand;
    placement into legal hand/container slots remains covered by the real
    alcove test. 2026-08-06: the C020..C027 status-hand route now maps its
    champion-relative parity to canonical C00/C01 hand masks, so placing a
    held object in another champion's hand uses `MASK0x0200_HANDS` instead of
    the unrestricted C00/C01-adjacent backpack masks. Remaining: real Mac
    pickup/placement/cursor/panel capture.
    2026-08-06: the real floor-item F0115 capture test now accepts the
    standard PC34 archive's `DATA/` root, so its previous false skip is gone;
    the D0C source material and final M11 capture receipt pass against the
    extracted original corpus.
17. **DM1-SOUND-MUSIC-STARTUP:** Complete original DM1 sound/music playback,
    title/swoosh/entrance cadence, palette transitions, and runtime sound
    events with real media and no generated timing. 2026-08-06: M11 now
    rebinds the 35-event SND3 bank to the same verified PC3.4 `GRAPHICS.DAT`
    path used by the visual asset receipt, so a default search-root sound bank
    cannot override the selected installation. Authenticated DM1 misses remain
    silent instead of using procedural markers. Remaining: original packaged
    macOS playback/cadence capture and broader source-event coverage.
18. **DM1-INPUT-NAVIGATION:** Complete source-owned keyboard, mouse, touch,
    controller, turn/strafe, click targets, focus, and fullscreen coordinate
    mapping across HoC, HUD, inventory, dialogs, and dungeon gameplay.
19. **DM1-MAC-RELEASE-CAPTURE:** Complete packaged macOS/window capture and
    release-app evidence for title, Entrance, HoC, HUD, viewport, wall text,
    mirrors, objects, actions, and spells using actual assets.
21. **CSB-DSA-FULL-OPCODE-FAMILY:** Extend authenticated CSBWin DSA execution
    across remaining source-supported opcode families, stack/control semantics,
    filters, state transitions, and runtime mutation with hard fail-closed
    bounds for unknown behavior. 2026-07-29 source inventory against
    CSBWin 2023 `Data.h:1736-1875` and `DSA.cpp:2345-4977` identifies the
    remaining source-supported stack families include `CAST`,
    `FILTEREDCAST`, and their still-unowned indirect routes.
    `SAY` is now
    source-owned and transactional; the listed operations still require one
    transactional world-
    mutation batch with source-owned callbacks; do not add a synthetic VM
    fallback for missing dungeon ownership. 2026-07-29: `I_Indirect` now
    expands only to the already source-owned transactional operations
    (`ADD`, `MONSTER!`, `CHAR!`, `COPY`, `CELL!`, `CAUSEPOISON`,
    `SWAPCHARACTER`, `CREATECLOUD`, `DEL`, `MOVE`).
    2026-07-30 audit against CSBWin `DSA.cpp:2094-2199,2345-2374,
    3092-3107,4958-4973` and `Magic.cpp:1408-1418`: direct `CAST` and
    `FILTEREDCAST` already retain all 14 `SPELL_PARAMETERS` words and have
    rollback coverage. The live saved-timer/filter runner binds its
    `csb_v1_runtime_dsa_cast_spell` callback transactionally, as verified by
    the runtime `STKOP_Cast` silent-abort case; it must not be reimplemented.
    `I_CAST`/`I_FILTEREDCAST` remain intentionally rejected: CSBWin serializes only 13 parameter words through `INDIRECTP`,
    while `DSACastSpell` copies the full 14-word structure. Bind an exact
    original parameter-message corpus and a runtime spell owner before
    admitting either route; do not pad the missing word or send it through a
    DM1 spell substitute.
    2026-07-30: the live candidate runner now owns the one complete
    side-effect-free `Magic.cpp::CastSpell` case: direct `CAST` with
    `action=1` and `disableTime=-1` commits the original silent abort. All
    non-abort cast classes remain fail-closed until their CHARDESC, spell
    table, object/projectile, timer and save publication transaction is
    complete; `I_CAST`/`I_FILTEREDCAST` remain closed for their missing word.
    `FILTEREDCAST` itself is also fail-closed, including action 1: CSBWin's
    `DSACastSpell(true)` invokes `CallSpellFilter` before the later silent
    CastSpell branch, and Firestaff must not skip that actuator/EXPOOL owner.
    Its source local-variable rewrite now writes the action-local DSAVARS bank
    before the selected direct word runs; persistent save data remains outside
    this temporary source bank. 2026-07-29: direct `CREATECLOUD` now consumes
    CSBWin's `(location,type,size)` stack contract, preserves its six legal
    types and silent invalid-type no-op, and commits only after whole-action
    acceptance. Its DB15/FluxCage/timer runtime owner remains open under
    CSB-DSA-MONSTER-WORLD; no cloud is synthesized when that owner is absent.
    `I_CREATECLOUD` now uses that identical request path with CSBWin's reverse
    parameter-stack order (`size,type,location`). 2026-07-29: direct
    `TELEPORTPARTY` now stages CSBWin's packed `LOCATIONREL`, then invokes the
    runtime's party/object teleporter only after whole-action acceptance. The
    runtime decodes the source `(direction,level,x,y)` fields, removes active
    groups through the existing F0194 owner, and rotates the party through the
    regular runtime path. Missing ownership and later invalid source words
    remain no-mutation failures. `I_TELEPORTPARTY` now consumes the same
    source `INDIRECT(..., 1)` parameter order and reaches that exact staged
    request path. The runtime receipt verifies and atomically publishes the
    final source party pose only after the full action and candidate dungeon
    have succeeded; no candidate callback changes the shared current level.
    The production filter runner now forwards its teleporter owner to the
    authenticated stack context; a real byte-map regression proves commit and
    post-publication drift rejection. Core admission now marks the opcode as
    runtime-owned, preventing it from being classified as a pure-stack DSA
    operation. 2026-07-29: direct `DELMON`/`INSMON` now stage CSBWin's
    exact `(LOCATIONREL,index)` and `(LOCATIONREL,positionMask)` operands,
    commit only after the complete action succeeds, and reach a loaded C04
    group owner. The owner preserves CSBWin's no-group/no-room no-ops,
    refuses deletion of the final creature, uses descriptor horizontal-size
    limits, compacts/copies raw health and cell data, and updates live group
    direction/aspect data. `I_DELMON` and `I_INSMON` are admitted through the
    same `INDIRECT` expansion. 2026-07-29: the runtime owner now mutates the
    CSBWin TIMER heap, materialized timeline receipt and both ITEM16 forms as
    one candidate transaction. `DELMON` retires the selected A/B timer,
    renumbers later group timers and compacts `SINGLE_MONSTER_STATUS`;
    `INSMON` duplicates A0/B0 and status 0 into the appended ordinal. A
    malformed restored timer heap still rejects the whole action. Its public
    runtime receipt now carries the source LOCATIONREL, operand, direction,
    resolved C04 Thing and post-mutation C04 hash, and invalidates on later
    raw-record drift. Broader
    DSA-world behavior remains under CSB-DSA-MONSTER-WORLD.
    2026-07-29: direct `MOVE` now consumes CSBWin's exact ten-word
    `MoveObject` contract transactionally. Its real PC3.4 cell-to-cell owner
    selects ordinary DB5..DB13 Things by source type/position/depth mask and
    preserves the requested destination cell position. A later invalid DSA
    word rolls the candidate raw list back. Multi-bit destination selection
    now consumes the candidate GAMEBLOCK2 `STRandom` state before commit;
    cursor, character, monster and chest endpoints remain open rather
    than being routed through a synthetic cell substitute.
    2026-07-29: direct `DEL` and `I_DEL` now stage CSBWin's exact
    `(object,location)` operands. Their loaded PC3.4 cell owner unlinks only
    source-supported DB3/DB5/DB7/DB8/DB10 Things and restores the record's
    F0166 free sentinel after full-action acceptance. Cursor and champion
    possession, plus unsupported chained record types, remain fail-closed.
    2026-07-29: direct `ADD` and `I_ADD` now stage the exact CSBWin
    `(positionMask,LOCATIONREL,object)` order and resolve multi-position
    masks through the staged CSBWin `STRandom` state. The loaded PC3.4 owner
    uses F0166 to copy flat source-supported DB3/DB5/DB6/DB7/DB8/DB10
    records, preserves payload bytes 2..N, and appends the selected positional
    Thing only after the whole action succeeds. DB4/DB9 recursive ownership
    and cursor/champion/monster destinations remain explicitly fail-closed.
    2026-07-29: direct `THROW` and `I_THROW` now stage CSBWin Timer.cpp's
    exact seven operands and use the loaded candidate's F0810 projectile plus
    timeline owner. Original `0xFF80` spell Things and ordinary DB5..DB10
    objects selected by source cell/position are routed without a synthetic
    projectile; unsupported Thing kinds remain CSBWin no-ops. The 2026-07-29
    follow-up admits opcode 61 to the authenticated core verifier and
    atomically publishes the candidate F0810 slot plus first move event only
    after complete source-action acceptance. The focused FIREBALL runtime
    regression covers both direct and `I_THROW` parameter-stack order, and
    the complete CSB CTest lane passes; remaining DSA families stay open under
    this item.
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
22. **CSB-DSA-MONSTER-WORLD:** Complete DSA-driven monster movement, attacks,
    sensors, timers, level context, and world mutation through actual loaded
    CSB dungeon/save data.
24. **CSB-ORIGINAL-SAVE-CORPUS:** Admit real CSB/CSBWin save corpus, complete
    native import/export/backup/resume compatibility, EXPOOL/DB11 ownership,
    and byte-level failure handling.
    2026-07-28: local CSB material includes the original Atari ST Save Disk
    (`Chaos Strikes Back for Atari ST Save Disk.msa`) and it has been decoded
    and inspected as a valid 720 KiB FAT disk. It is blank: there is no
    `CSBGAME.DAT` or other saved position on it. Treat it as Utility Disk
    media only; it must not satisfy this corpus gate. The local CSBWin binary
    and installed archives likewise contain no user-created DSA-bearing save.
    2026-07-29: re-scanned all local CSB `.7z`/`.zip`/`.rar` packages for
    `CSBGAME`, CSBWin and save-disk payload names. The only hits remain the
    blank Atari ST Save Disk and Utility Disk `DungeonSave` directories; there
    is still no authentic saved position to admit. Keep this blocked on a
    user-created original/CSBWin save, not a synthetic fixture.
    2026-07-29: the external corpus runner accepts an operator-supplied
    `FIRESTAFF_CSB_ATARI_SAVE_CORPUS` and exercises original big-endian
    GAMEBLOCK1/2 decoding, runtime Resume, atomic write-back, canonical-slot
    backup recovery, and champion mutation without creating substitute bytes.
    2026-07-31: the currently staged `csbgame3.dat` is a real local artifact
    (61,465 bytes; SHA-256 `b3a8d7fb920346835c48c86a945d90d565b6bc5c250f83b5a56c1b0d8b95ec70`),
    but it is not an admissible CSBWin DSA corpus. Its Extended Features tail
    is structurally valid, while the CSBWin loader classifies the core as
    `no_magic_8_plus`/`reject_bad_magic` and finds no DSA section or executable
    actions. The DSA handoff probe now skips it rather than treating a partial
    compatibility decode as live DSA evidence. Remaining: an authentic,
    loader-admitted CSBWin `csbgame*.dat` with the complete Extended
    Features/DSA/core sequence, then original dungeon-tail loader/write-back.
    2026-07-31 correction: the focused provenance test can exercise the
    file's bounded compatibility decoder and verbatim export path, but that
    must not be read as CSBWin loader admission or a production resume. The
    loader-boundary and DSA corpus receipts above are authoritative for this
    file; neither admits a live save or DSA runtime state.
    2026-07-31 inventory follow-up: the locally available CSBWin source tree
    contains `Game/CSB/csbgame2.dat` (42 KiB; SHA-256
    `105104b30dde164e7000d388f251f3d6d3f83a56959f28f56220711d1e9f3a9e`).
    It is a real CSBWin sample save, but its DSA receipt is
    `reject_dsa_corpus_no_extended_features` when paired with the tree's
    `Game/CSB/Dungeon.dat`; it cannot stand in for the required authentic
    Extended Features/DSA corpus.
    2026-07-30 CSBWin source audit: `SaveGame.cpp::_ReadEntireGame` calls
    `ReadDatabases` immediately after the verified GAMEBLOCK1/2, ITEM16,
    CHARDESC, TIMER and queue streams. For a resumed save, the trailing
    payload is an ordered, checksummed stream of `DUNGEONDATINDEX` (44 bytes),
    `LEVELDESC[NumLevel]` (16 bytes each), column pointers, object list,
    indirect-text index and compressed text, DB0..DB15 records, cell flags,
    then a final u16 checksum. `SaveGame.cpp:1240-1337,2536-2840` and
    `CSB.h:DUNGEONDATINDEX/LEVELDESC` are the implementation reference.
    The exact framing is now known; the remaining work is to bind it to
    Firestaff's owned dungeon/object stores rather than treating its bytes as
    EXPOOL or synthesizing a dungeon.
    2026-07-30 implementation: `csb_v1_csbwin_dungeon_tail_parse_prefix`
    now consumes that source-defined prefix with big-endian saved words and
    exact bounds for `DUNGEONDATINDEX`, `LEVELDESC`, column pointers, object
    list and direct/indirect text. It passes the data-free framing contract
    and the staged `csbgame3.dat` corpus after the authenticated 12-byte
    TIMER core. DB0--DB15 and cell flags remain deliberately unowned; this is
    an admission boundary, not a synthetic dungeon loader.
    2026-07-30: the same reader now verifies the source terminal checksum:
    `WriteAndChecksum`/`FetchDataBytes` accumulate unsigned bytes modulo
    65536 and `ReadDatabases` reads the terminal big-endian u16. The staged
    corpus passes this check.
    2026-07-30: a bounded metadata parser now verifies the complete DB0--DB15
    span table after the prefix, including source `dbEntrySizes`, legacy
    four-byte versus current six-byte DB7 scroll records, eight-byte versus
    ten-byte DB3 actuator records, Extended Features versus legacy cell-flag
    sizing, exact end-of-tail framing, and the terminal checksum. The local
    `csbgame3.dat` corpus passes this contract. Record contents and cell flags
    remain read-only and have no runtime or write-back owner.
    2026-07-30: production resume now applies both checks before staging a
    non-EXPOOL extended tail. The staged file still resumes and a copy with
    only its final checksum byte changed is rejected transactionally (16/16
    provenance checks). DB0--DB15 and cell flags remain read-only until they
    have real runtime owners.
    2026-07-30: resumed CSBWin source saves can now be exported byte-for-byte
    through their FNV-bound provenance, including the Extended Features prefix
    and opaque variable dungeon payload. The export refuses any source file
    that drifted after resume and uses original-slot backup rotation. It is
    deliberately not a runtime-mutation writer: that remains blocked on the
    real dungeon-tail loader/write-back rather than emitting a plausible save.
    2026-07-30: the local original Atari archive now has a skip-safe CTest
    corpus regression that extracts its genuine `MINI.DAT` and proves decode
    plus GAMEBLOCK2 byte-preserving round-trip. This is campaign-save coverage,
    not evidence for the still-missing PC/CSBWin `CSBGAME*.DAT` corpus.
    2026-07-30: the non-invasive corpus inventory now searches Firestaff's
    bounded CSB data/save roots and `.7z`/`.zip`/`.rar` members. Operators can
    add a separate root with `FIRESTAFF_CSB_SAMPLE_SAVE_ROOTS`; it still admits
    only an extracted, validated `CSBGAME*.DAT`/`.BAK`, never a filename found
    inside a game or utility archive.
    The previously cited AnnotatedCSB `CSBGAME2.DAT` SHA-256
    (`762db0d0617a362910edb739f02a8dca246c04a6c0e44113c78399278f72b189`)
    is not present in the current data roots, so it is not claimed as a local
    passing corpus. Firestaff validates the C29 checksum pair and recognises
    CSBWin slots `CSBGAME.DAT` through `CSBGAME4.DAT`. Remaining:
    Firestaff already imports and exports the authenticated CSBWin PC core
    body (GAMEBLOCK2, ITEM16, CHARDESC, TIMER, timer queue and preserved
    EXPOOL) through `csb_v1_runtime_apply_csbwin_resume_file()` and
    `csb_v1_csbwin_512_export_verified_csb_save()`. Remaining: complete
    2026-07-29: original big-endian GAMEBLOCK2 write-back now updates only
    documented clock, RNG, hand, party pose and map fields, re-encrypts the
    source blocks and recomputes their checksums while preserving the embedded
    dungeon and all unowned sections byte-for-byte. The runtime exposes an
    explicit source-to-destination export that writes atomically through a
    temporary file; a real-corpus round-trip remains externally runnable but
    unverified on this host. Remaining: original Atari/Amiga dungeon, object
    and timer mutation write-back and broader
    original DB11/EXPOOL corpus coverage; do not substitute the PC body layout
    for this authenticated original route.
    2026-07-29: CSBWin `SaveGame.cpp:926-932` confirms the original slot
    rotation rule. Firestaff now rotates only `CSBGAME.DAT` and
    `CSBGAME1.DAT` through `CSBGAME4.DAT` to the matching `.BAK` name,
    restoring the old slot if final publication fails. The external
    original-save runner verifies byte-identical backup preservation before
    the documented GAMEBLOCK2 write-back is accepted when an operator supplies
    a real corpus.
    2026-07-29: CSBWin `SaveGame.cpp:1692-1697,2090` backup recovery is also
    implemented for authenticated original slots only. A bad `CSBGAME*.DAT`
    now falls through to the matching `.BAK`, validates its original Atari
    body, and restores the canonical slot name after successful handoff.
    2026-07-29: the export now also writes the documented, already-decoded
    champion fields in each 800-byte original record (identity, pose/action,
    vital stats, skills/experience, slots, load and shield), re-encrypting
    that section and rebuilding its checksum. The external corpus runner is
    ready to prove a champion-name/health round-trip without touching the
    embedded dungeon once a real saved position is supplied. Remaining scope
    is original dungeon/object/timer mutation and DB11/EXPOOL corpus evidence;
    do not infer them from the PC body layout.
24. **CSB-UTILITY-DISK-COMPLETE:** Complete Utility Disk import, preview,
    save/load/new-game, champion editing, inventory, chest, dialogs, and
    confirmation flows using original package and save material. Direct
    launcher selection of a supported archive now reaches the verified
    HCSBF.HTC Hint Oracle package; runtime menu consumption remains open.
    2026-07-29: Utility Disk identity itself is now source-locked to ReDMCSB
    `UTIO.C` F1991: Atari ST sector-7 copyright/title bytes and the 880 KiB
    Amiga ADF root-volume `FTL_CSB_Utility` replace Firestaff's former
    invented boot-sector serial. Remaining work is the actual Utility UI
    presentation and its complete save/import/new-game actions. 2026-07-29:
    the unused M11 semantic-row/rectangle Utility Disk painter was removed;
    it had no production call site and cannot substitute for a decoded
    original Utility Disk raster.
    2026-07-29: DM1-party import now requires a physically verified Utility
    Disk before it can commit a party. The runtime finds only six known
    original Amiga Utility Disk ADF hashes (English R1/R2/R3, French, German
    R1/R2) under the selected CSB data root, including virtual `.7z` members;
    it materializes an archived member solely for the existing root-block
    check and removes it afterwards. An explicitly supplied image is still
    checked by that same root-block identity. Missing, unreadable or wrong
    media fail before the Utility flow reaches IMPORT/NEW GAME. This closes
    the prior unchecked `utility_disk_verified` import shortcut, not the
    broader Utility Disk UI scope.
    2026-07-30: repeated import/preview requests now reuse the already
    hash-admitted archive member for the same selected CSB root. Each request
    still extracts that member and reruns the original Utility Disk check;
    only the expensive broad archive inventory is cached. A changed or
    unreadable cached member invalidates the cache and fails closed.
    2026-08-08: M12 now preserves the configured originals root separately
    from the private materialized CSB runtime directory. The Utility Disk
    hash lookup uses that originals root, so a selected PC/Atari/Amiga core
    cache does not hide a valid original ADF stored in the user's shared CSB
    archive collection. A real English release-3 ADF in the local archive
    reaches the completed import state; remaining work is still the original
    Utility UI, editor and real-save corpus, not DSA-save support.
    2026-08-08: archive-backed import now creates the private materialized
    package save/cache directory before extracting the hash-admitted ADF.
    The full PC34 cache-to-originals-root boot transaction reaches the import
    handoff; it no longer relies on a pre-existing cache sibling directory.
    2026-07-29: the live C140 `Ctrl-S` route now opens the source-dialog
    geometry with the Atari CSB v2.1 source order: Load Saved Game, Save and
    Play, Save and Quit, Format Floppy. The source Format Floppy confirmation
    is also present. Keyboard Up/Down, Enter/Escape and pointer choice all
    consume the same modal state; load/save invoke the existing CSB
    F0433/F0435 runtime owners instead of merely closing a placeholder overlay.
    The host save namespace is `saves/csb/firestaff-csb-save.sav`, separate
    from both DM1 snapshots and original `CSBGAME.DAT` media; an absent new
    default still reads the old Firestaff CSB path once for migration. Broader
    Utility Disk editor/import, champion and new-game flows remain open.
25. **CSB-TITLE-AUDIO-CADENCE:** C001 timing and FTL/PRESENTS/CHAOS/STRIKES
    composition are source-locked (60 + 20 + 20 + 2 VBlanks after swoosh).
    SDL/CoreAudio playback is now enabled by default for decoded original
    media; complete authenticated CSB SWSH/entrance-music admission and live
    M11 playback without generated replacements.
    2026-07-31: M11 now holds each of the 20 already source-rendered C425
    CHAOS zoom rasters for two PC3.4 cadence slots, matching the visible-frame
    treatment used by the DM1 C001 route. It does not add a source frame or
    change TITLE.C's 60/20/20/2 sequence: PRESENTS, the full-CHAOS Delay(20)
    and STRIKES BACK retain their original cadence.
    2026-07-29: archive-backed PC3.4 packages now materialize every accepted
    `SWSHSND.C` and extension-renamed source filenames beside `GRAPHICS.DAT`,
    and the strict 9,078-byte loader admits each of those original package
    names after hash-first discovery. M11 now hands that verified PCM directly to the SDL audio
    backend during the CSB FTL prelude; it remains silent when the source
    sample is absent and never borrows DM1 audio. Local CSB data presently
    lacks this optional source file, so a real-media playback capture and
    entrance music decoder remain open.
    2026-07-30: a skip-safe, real-PC3.4 title/Entrance regression now captures
    all four source stages and verifies their distinct visible palette/raster
    signatures (PRESENTS, FTL, CHAOS STRIKES BACK, Prison/Entrance). It catches
    a black/flat presentation or palette collapse without using replacement
    art. Original audio media and packaged-app capture remain open.
26. **CSB-ENTRANCE-DOOR-CREDITS:** Complete C002-C005 closed/opening entrance,
    credits, prompts, input timing, palette, sound, and runtime handoff from
    real CSBgraphics/package data.
27. **CSB-HUD-INVENTORY-ACTIONS:** Complete C017/C040 HUD and in-game panel,
    champion, action/spell, inventory, cursor, and text rendering using real
    source surfaces and correct transparency.
    2026-07-29: the stale C004 host receipt no longer keeps the completed
    Prison action on the Entrance render path. The live frame admits only
    package-identified PC3.4 C009..C013 IMG2/LZW rasters, so a generic
    same-size cache entry cannot suppress C013. A real capture and focused
    M11 regression now show the cyan movement panel in V1, V2.0 and V2.1.
    Full champion/action/spell/inventory runtime coverage remains open.
    2026-07-29: the no-party Prison route now installs the authenticated
    `C013_GRAPHIC_MOVEMENT_ARROWS` material before checking the GAMEBLOCK
    party receipt. ReDMCSB `MENUDRAW.C` owns this panel even before imported
    champions exist; Firestaff therefore no longer leaves the entire movement
    region blank merely because the party mirror is absent. Missing source
    material still clears fail-closed. Full champion/action/spell/inventory
    runtime coverage remains open.
    2026-07-30: visible CSB champion status boxes and their right-click
    inventory toggles now consume the shared PC34 `COMMAND.C G0447`
    C007..C015 surface. The M11 route writes through CSB's runtime party
    mirror and does not fall back to DM1 dungeon state. The focused runtime
    regression and full 109-test CSB lane pass. 2026-07-30: the same focused
    runtime regression now follows the source pointer chain C116 action icon
    -> F0389 action menu -> C114 row 1/F0391 and proves that STAB updates the
    CSB runtime action index, clears the source action menu, and never enters
    DM1 world state. 2026-07-30: the shared `COMMAND.C` C100 parent and
    C101..C109 spell-panel boxes now also reach the CSB runtime, so a real
    C009/C011 panel is no longer visibly inert under the mouse. Rune entry,
    recant, and caster tabs retain their source geometry and per-champion
    state. CSB spell-area execution is still separate: casting requires a
    source-owned CSBWin caster/cast binding rather than the DM1 spell
    executor. 2026-08-08: C101..C106 now also persist their exact four-byte
    `Champion.Incantation` line and 0..3 `SymbolStep` ring in CSB's live
    GAMEBLOCK; C107 deletes that same durable state without refunding mana.
    This follows ReDMCSB `SYMBOL.C` F0399/F0400, and is covered with the
    authentic Atari ST `MINI.DAT` path. It is input/save continuity only,
    not a claim that the still-gated CSB cast executor is available.
    2026-07-30: the shared `M11_GameView_CastSpell` boundary now
    explicitly rejects CSB before DM1 F0750--F0754 can run. A rejected CSB
    cast preserves the source rune line, champion mana and world tick until
    the CSBWin caster/cast transaction can own them. 2026-07-30: C009/C011 now consumes the current CSB GAMEBLOCK
    party mirror, matching the C028/action consumers. A stale retained M11
    party can no longer black out a valid CSB spell panel or reject its C100
    mouse input before CASTER.C can select the source caster. The same
    GAMEBLOCK refresh now precedes all live CSB mouse hit testing, covering
    champion, action, movement, and spell commands, and also precedes the
    live keyboard route for F1-F4 and movement/utility commands.
    2026-07-30: selecting a live C109 caster now commits
    `MagicCasterIndex` to the CSB runtime profile before M11 redraws the
    shared spell panel. This follows `CASTER.C F0394` / CSBWin
    `Magic.cpp::SelectMagicCaster` and keeps the persisted CSB runtime state
    distinct from the party leader. Rune-cost entry and `CastMagic` still
    need their source-owned CSBWin transaction; this does not enable DM1
    F0750--F0754 for CSB. **2026-07-31 HUD containment:** disabling the
    optional DM1 V1-chrome switch can no longer make a CSB session draw the
    generic cyan utility frame, champion/status text or light bar when
    C009/C010 source material is absent; that CSB region remains source black.
    The same switch no longer lets the legacy non-game rune workbench paint
    over CSB's F0128 viewport while a CSB spell state is open. An unavailable
    source dialog backdrop is likewise strict no-draw; only an explicit host
    return-confirmation flow may use host UI. Source C068--C073 movement hits
    remain active for CSB under that switch rather than being routed to the
    generic focus-card shortcut.
28. **CSB-VIEWPORT-GEOMETRY:** Complete F0107-F0115 walls, doors, teleporter,
   pits, floor/ceiling ornaments, creatures, items, projectiles, explosions,
   and custom backgrounds through real PC34 asset ownership.
   2026-07-30: a source-bound thrown-object projectile no longer falls back
   to the 16x16 object-icon atlas when its perspective native bitmap is
   unbound. ReDMCSB `DUNVIEW.C` F0115 instead enters
   `T0115015_DrawProjectileAsObject` and selects the G0209/M612 native object
   bitmap in the C2900 lane. The verified C2900 routes now bind that positive
   F0142 branch through the real associated C05--C0B record, including M066
   weapon-aspect selection; missing or unverified routes remain no-draw.
   Source-bound group rendering also rejects an undecodable creature type
   without drawing the former diagnostic cross; the original F0115 path has
   no marker fallback for malformed C04 data.
   The M11 drawer binding now marks object sprites source-bound as well as
   projectiles and groups, so a failed object decode cannot silently reopen
   the icon/marker fallback path.
29. **CSB-SENSOR-THING-RUNTIME:** Complete real Thing chains, generic object
    sensors, remote actions, pits, teleporters, stairs, door and actuator
    side effects in the CSB live runtime.
30. **CSB-COMBAT-MOVEMENT-RUNTIME:** Complete source movement, group AI,
    melee/spells/projectiles, party interactions, damage, deaths, drops, and
    timer scheduling using ReDMCSB and CSBWin semantics.
    2026-08-08: successful live PC34-compatible steps now retain the source
    `G0310` movement delay rather than a host constant. `CLIKMENU.C` F0366
    selects the maximum F0310 cost among living GAMEBLOCK champions, and M11
    now ages that gate before F0380 dispatches a later movement command.
    2026-08-08: the same PC34 step now performs F0325 before blocked/stairs
    resolution, accumulates F0321-style stamina underflow in the F0320
    pending-damage pass, and applies its C12 HUD receipt on the next source
    tick. C05 is resolved through the live DUNGEON.DAT object record: Elven
    Boots affect F0309 before rounding and Boot of Speed icon 194 affects
    F0310, including the F0325 load denominator. Group AI, melee/spells,
    drops, and wider sensor/event coverage remain open.
31. **CSB-SOUND-MUSIC-MEDIA:** Complete source audio/music media admission,
    startup/running sound events, palette/VBlank cadence, and platform-safe
    media playback without generated replacements. The host backend is no
    longer opt-in; remaining work is CSB source-media coverage and timing.
    2026-07-29: the locally available Amiga `SWSH.FTL` archive member is a
    18,882-byte Amiga HUNK executable (header `0x000003F3`), not a PCM
    swoosh payload. The sibling original `Sound/*.amg` resources need their
    own authenticated Amiga decoder/binding; neither may be fed through the
    PC3.4 9,078-byte `SWSHSND.C` transport or substituted with generated
    audio.
    2026-07-29: authenticated all five local Utility Disk `.AMG` files
    against the documented SND2 layout: `u16be sampleCount`, signed 8-bit
    samples, then zero to three trailing bytes. The duplicate CSB parser had
    incorrectly interpreted its first two samples as a control word; it now
    uses this same source layout. Runtime mixer and original timing binding
    remain open.
    2026-08-08: `--scan-data` now inventories every registered optional CSB
    source-media fingerprint in the configured data root through the same
    recursive hash traversal used for required files. This exposes nested
    archive/ADF media such as the authenticated Amiga `SWSH.FTL`, title and
    Utility Disk resources without making any of them a launch requirement.
    2026-08-06: the Amiga game-data path now resolves a direct-loaded
    DMCSB2 `GRAPHICS.DAT` item to the F1051/F0709 PCM view using the item
    table's length and the original two-byte offset. This is an admission
    boundary only; wiring source sound events to an Amiga mixer and cadence
    remains open.
    2026-08-06: the Amiga runtime table and payload loader now preserve the
    source-specific Graphic 671–712 and `SOUND.C` F0709 period values. SDL
    transport still needs the original Amiga period/voice behavior.
    2026-07-29 source audit: the hash-verified PC3.4 CSB `GRAPHICS.DAT`
    (`3af5396f...d256942`) is not admissible through Firestaff's DM1 SND3
    manifest. ReDMCSB is the owner for the PC3.4 route: `DEFS.H` defines 35
    `SOUND_DATA` entries (graphic index, two unused bytes, period, priority,
    loud distance and soft distance), while `SOUND.C` F0064/F0065 pass the
    table-owned graphic to F0060. The PC3.4 table is executable-owned, not
    a Graphic 562 payload; Firestaff now binds its exact 35 rows (graphics
    671-712) from `DATA.C:1260-1302`. It remains separate from CSBWin's
    `sound1772[22]` table from graphic `0x232` (`CSBCode.cpp:10298-10330`).
    Do not route either source index set through DM1 SND3 or a generated
    marker. 2026-07-29: Firestaff now admits an original PC3.4 sound record
    only when its F0060 `u16be` length exactly covers the selected raw graphic
    after the two-byte source tail; the local Graphic 672 switch record proves
    the 128-byte route. CSB runtime sound events now load that exact payload
    and transport it at the ReDMCSB IBMIO F8119 PIT rate (`1193180 /` the
    `SOUND_DATA` 112/138/145/150 divisor). Invalid or changed source bytes
    fail closed and do not use DM1 SND3 or marker audio. 2026-07-29: the
    live M11 profile-sync boundary now consumes both F0064 immediate and
    F0065 delayed completed plays once, so keyboard and pointer routes reach
    that authenticated transport rather than only updating CSB runtime state.
    2026-07-30: the PC3.4 F0064 distance volume is now resolved in the
    source runtime from each SOUND_DATA row's LoudDistance/SoftDistance and
    retained with the completed F0064/F0065 event. M11 applies its exact
    three-level source volume before the user's SFX gain while preserving the
    original PCM bytes and PIT cadence; it no longer substitutes a fixed host
    volume for all CSB effects.
    2026-07-29: the source runtime now retains a bounded, ordered completed-
    play history. M11 consumes every newly completed F0064 immediate request
    and F0065 pending flush in sequence, rather than forwarding only the
    final `lastPlayedSoundIndex` when several source events occur before one
    host sync. Missing history fails closed with an explicit audio status;
    no marker or generated substitute is used.
    2026-07-29: M12's persisted master/music/SFX/mute controls now reach the
    live M11 SDL3 state after the CSB branch reinitializes it. Master gain is
    applied at the device boundary while music and SFX are scaled separately
    as their authenticated samples are queued. The real PC3.4 M12-to-M11
    handoff regression locks the exact values. This is host-volume ownership
    only; it does not claim missing original media or timing.
    Remaining: compare the complete selected original PC driver waveform/
    device behavior and capture real runtime event/timing behavior.
32. **CSB-MAC-RELEASE-CAPTURE:** Complete real packaged app/window captures for
    title, entrance, doors, HUD, viewport, Utility Disk, and first runtime
    frame against local original CSB data.
33. **CSB-INPUT-CONTROLLER-ACCESSIBILITY:** Complete mouse, keyboard,
    controller/touch mapping, modal focus, pointer coordinates, screen scale,
    and original command behavior for all CSB gameplay surfaces.
    2026-07-29: the V1/V2.x startup handoff regression now uses the original
    C407 pointer zone through M11's complete window-to-presented-to-source
    mapper at 320x200 and 960x600. The source center reaches F0806 runtime in
    V1, V2.0, V2.1 and gated V2.2; this replaces the former Enter surrogate.
    Broader controller/modal coverage remains open.
    2026-07-28: V2.0/V2.1/V2.2 now consume CSB's own in-flight cardinal
    turn animation as a post-composition viewport pan in M11. The source V1
    command, timing, collision and sensors remain unchanged. This closes the
    smooth-turn presentation subtask; broad controller/modal coverage remains.
    2026-07-30: the SDL gamepad bridge now has a standalone CSB regression:
    default D-pad, shoulders, face buttons, menu/gameplay translation, and
    stick dead-zone resolve to the exact shared M11 tokens consumed by CSB's
    GAMEBLOCK/COMMAND.C route. Controller remapping, modal focus and touch
    coverage remain open.
    2026-07-30: M12's persisted Input Mode is now consumed by that live SDL
    route: Keyboard+Mouse and Touch suppress configured gamepad input, while
    Auto and Gamepad preserve the user-owned `gamepad.toml` map. The mapping
    is reapplied while Settings is open, so a changed mode takes effect before
    the next event without restarting the launcher. Per-action remap UI and
    broader modal focus coverage remain open.
34. **CSB-EXPANSION-AND-CUSTOM-DUNGEONS:** Complete safe original-data handling
    for CSB expansion/custom dungeon package selection, admission, runtime,
    save namespace, and no-cross-game asset leakage.
35. **DM2-GDAT-HUD-INTERFACE:** Complete skproject-derived interface panels,
    fonts, controls, inventories, spell/action widgets, cursors, and HUD
    placement from real GDAT records. **2026-08-06 HUD receipt correction:**
    the source-plan renderer retains the exact decoded width, height and
    palette from each validated original HUD command. A plan-owned top bar or
    portrait panel no longer becomes a zero-sized host receipt merely because
    a separate palette callback has not run.
36. **DM2-GDAT-DUNGEON-MATERIALS:** Complete real GDAT wall/door/floor/ceiling,
    map-chip, ornament, object, projectile, cloud, and animation material
    decode across all indoor dungeon styles.
37. **DM2-GDAT-CREATURE-MATERIALS:** Complete creature animation, orientation,
    lighting, occlusion, death/drop, and static/flying-object source materials
    through skproject renderer rules.
38. **DM2-G1-MAP-RECORDS:** Complete G1/c_record addressing, map records,
    triggers, doors, stairs, teleporters, scenery, and first-class live scene
    object semantics from original data. **2026-08-06 loader hardening:** the
    bounded legacy parser now rejects an allocation failure instead of
    returning a successful dungeon without its owned source bytes. This is
    only a fail-closed correction; its fixture-only 16-bit layout is still not
    a runtime source route.
39. **DM2-SKSAVE-ORIGINAL-INTEROP:** Complete original SKSAVE corpus loading,
    validation, save/export, resume, party, map, timers, objects, weather,
    and backup behavior without Firestaff-only approximations. **2026-08-06
    corpus spelling correction:** root-level scanning now authenticates the
    supplied PC-DOS `sksave0.dat`…`sksave3.dat` and matching `.bak` files as
    real, unpadded case variants. It inventories their actual slots and
    backups without promoting the incomplete raw payload into a playable
    resume session. **2026-08-06 path handoff:** direct menu/save selection
    now also preserves that original lower-case, one-digit spelling instead
    of rejecting it as a Firestaff-only filename mismatch.
40. **DM2-MENU-STARTUP-COMPLETE:** Complete skproject title/menu state machine,
    clickable buttons, palette, animation, audio, save selection, new game,
    options, error states, and first HUD handoff.
    **2026-08-05 verification:** the active M11 pointer route is now exercised
    from the verified GDAT `0xD7` NEW rectangle itself, not a fixture geometry.
    It correctly reaches the source-owned `GAME_LOAD` gate. The remaining
    original new-game initialization, save selection, options, and full menu
    state machine are still open.
    **2026-08-06 input correction:** M11 no longer falls through from the
    verified `0xD7`/`0xD9` GDAT pointer route to the retired Firestaff
    row/panel layout. A click that cannot be matched by source rectangles,
    including the display-to-framebuffer retry, is inert rather than selecting
    a host-invented save row.
    **2026-08-06 keyboard correction:** M11 now also rejects normalized
    Firestaff `UP`/`DOWN`/`ACCEPT` menu tokens while the DM2 title menu is
    active. `SHOW_MENU_SCREEN` consumes original MessageLoop events and the
    imported GDAT matrix produces `0xD7`/`0xD9`; no original keyboard-to-event
    table has yet been ported. Source pointer rectangles remain the only live
    menu actions. Import `c_0aaf`/`c_input` translation before enabling any
    keyboard/controller mapping.
    **2026-08-06 geometry correction:** the obsolete 78×50 host panel,
    row rectangles, English labels and generic pointer wrappers are now
    fail-closed in the production API. Only `dm2_v1_boot` may expose a menu
    hit after decoding the mounted GDAT RAW4 matrix.
    **2026-08-08 coordinate correction:** SDL maps the window point to the
    original 320×200 source space before it reaches the DM2 handler. The
    handler no longer maps a failed GDAT hit a second time, because that
    treated source coordinates as window coordinates and could displace a
    click in a scaled presentation. The decoded 0xD7/0xD9 rectangles remain
    the only accepted targets.
    **2026-08-06 save-root correction:** startup action execution now follows
    the explicit selected save root, matching the snapshot/scan path. The
    profile root remains only an absent-root fallback, so it cannot redirect a
    selected original SKSAVE corpus.
    **2026-08-06 real-corpus verification:** the supplied PC DOS corpus passes
    all 103 startup/menu action checks, including decoded 320×200 title and
    menu GDAT captures, palette/package ownership and the first-HUD handoff.
    New Game remains at the original `GAME_LOAD` boundary; it does not create
    a synthetic session when that owner is unavailable.
    **2026-08-06 M12 launch-root regression:** the real-data M11 startup gate
    now also drives the actual M12 card → options → Launch sequence.  It
    proves byte-for-byte path equality between M12's runtime handoff and the
    verified boot profile's `GRAPHICS.DAT` owner directory before it reaches
    the DM2 boot profile.  Keep package/app captures separate: a
    stale installed bundle may not be used as evidence for this source path.
41. **DM2-PARTY-INVENTORY-SPELLS:** Complete real champion, inventory, item,
   skill, action, spell, damage, condition, and UI mutation paths through
   the source runtime. **2026-08-06 update:** the disconnected
   `runtime_narrow` callback audit no longer reaches production: its
   `DM2_hero_39796` substitute is not the source name-entry UI at
   `c_hero.cpp:464`, and some of its moverec/light exports duplicate separate
   source-bound modules. Its isolated test contracts remain until a complete
   owner-backed runtime route is wired. **2026-08-06 HUD update:** the
   disconnected `c_gui_draw` callback transcription is also test-only. It
   guesses buttongroup dimensions, coin placement and UI glyph decisions;
   M11 must keep using the provenance-gated viewport/HUD route until those
   original GDAT and runtime owners are fully bound. **2026-08-06 viewport
   update:** the separate `c_gui_vp` callback transcription is test-only too;
   it forwards host-provided tiles and click zones instead of consuming the
   original G1/GDAT scene owner. The production route remains the gated
   viewport renderer. **2026-08-06 query update:** the inactive `c_querydb`
   callback transcription is also test-only because its many remaining TODO
   bodies cannot authenticate GDAT bytes; production must use mounted,
   provenance-checked GDAT/G1 owners. **2026-08-06 GDAT reader update:** the
   inactive legacy `c_gdatfile` adapter is test-only too. Its
   `READ_GRAPHICS_STRUCTURE` success result does not decode the original
   structure, so it cannot substitute for the mounted GRAPHICS.DAT owner.
   **2026-08-06 sound update:** the inactive `c_sfx` callback queue is
   test-only. Its source `s_sizee::barr_04[2..3]` cross-map origin transform
   and all four party-direction rotations are now ported and require explicit
   source origins; it still has no live SND/music runtime owner, queue memory
   or playback transport and therefore cannot enter production.
42. **DM2-CREATURE-AI-COMBAT:** Complete skproject creature AI, movement,
    combat, projectiles, cloud effects, drops, occupancy, and timeline-driven
    behavior from raw original records.
43. **DM2-CCM-SCRIPTS-ACTUATORS:** Complete source-backed CCM opcode, script,
    actuator, message, trigger, shop, NPC, puzzle, and map-transition paths
    with fail-closed unsupported bytecode.
44. **DM2-LIGHT-WEATHER-OUTDOORS:** Complete real lighting, darkness, rain,
    mist, thunder, sky/ground, outdoor maps, palette, and scene transition
    rendering from GDAT/map state.
45. **DM2-DOOR-TABLES-INTERACTION:** Complete original door/button/table,
    opening/closing animation, collision, sound, lock/key, and sensor
    interaction across dungeon and outdoor routes.
46. **DM2-SOUND-MUSIC-CUTSCENES:** Complete real SND/music/cutscene startup,
    menu, dungeon, combat, weather, and transition playback with skproject
    timing and no placeholder audio.
   **2026-08-06 update:** `DM2_QUERY_SND_ENTRY_INDEX` no longer creates a
   process-global fallback queue from a verified GDAT loader. It now requires
   the active runtime's source-shaped `xsndptr2` queue, bound immediately
   after its initialization and cleared with the boot profile. Real GDAT PCM
   lookup remains verified; original HMP scheduling and the remaining audio
   owners are still open.
    2026-07-31: the startup handoff now distinguishes an attempted menu cue
    from successful verified playback. Cue ownership now reads directly from
    PC `GRAPHICS.DAT` GDAT `MUSICS/<track>/dtHMP/0`; loose `.hmp.mid` files
    are rejected. The boot profile now discovers the authentic 63-byte PC
    `SONGLIST.DAT` by hash and retains its complete 63-byte source-owned
    selector prefix. In particular, slots 44 and 45 are original selectors,
    not padding; `0xff` remains the only no-music value. Runtime
    map-context refresh now dispatches that verified selector and exposes a
    receipt that remains `playback_started = 0` until real decoding succeeds.
    **2026-08-05 inventory update:** removed the unused invented per-track
    labels (such as "Dungeon Ambient" and "Boss Encounter") from the active
    sound module. DMWeb proves only the `0x00..0x1c` HMP identities and their
    `SONGLIST.DAT` selectors; names must not be inferred from a track number.
    **2026-08-05 HMP admission correction:** a structurally recognised HMP
    stream is now diagnostic-only and clears all scheduler/MIDI handoff state.
    SKProject's MIDI code accepts converted sidecars, not original GDAT HMP;
    a generic MIDI event walk must not be promoted to playback proof.
    **2026-08-06 source-audit correction:** `SkWinMIDI.cpp` is now documented
    as a consumer of externally converted `.hmp.mid` files, not an original
    HMP decoder. The real PC corpus contains 29 records (`00..1c`), not the
    obsolete 28-track count. Continue to reject sidecars and generated audio.
    **2026-08-06 HMP corpus boundary:** the direct reader now validates each
    original stream's header variant, all source chunks and HMP event bounds
    (including the 10--32-subtrack `013195` corpus). It deliberately clears
    every scheduler/backend handoff after inspection. Remaining work is the
    source-faithful scheduling/backend contract. Do not claim a cue played
    until that source chain and backend have both succeeded.
47. **DM2-INPUT-CONTROLLER-TOUCH:** Complete mouse, keyboard, controller,
    Steam Deck, touch, focus, scaling, hit-testing, and command translation
    for the real DM2 menu/HUD/gameplay routes.
    **2026-08-05 inventory:** removed an unused exported 236-byte all-zero
    input-table fixture. SKProject routes this input path through the real
    ten-entry `table1d3ed5` event tree; no replacement bytes were inferred.
    **2026-08-05 0AAF correction:** the source dialogue-menu adapter now uses
    `c_0aaf.cpp`'s `tarr_00 + 0x28` choice bytes and one-based event ordinal;
    invalid callback events fail closed rather than borrowing stack data.
48. **DM2-MAC-RELEASE-CAPTURE:** Complete packaged macOS/app captures and
    source-data visual evidence for title, menu, HUD, dungeon, doors,
    creatures, weather, saves, and input.
    2026-07-31: the current unbundled Extern-disk binary has a real SDL
    title/menu capture using the hash-verified PC-English data, including the
    source click sequence menu → credits → menu. Packaged-app evidence and
    every gameplay surface remain open; do not treat this source-binary
    result as a release capture.
    2026-08-06: `M11_Screenshot_CaptureCurrent` now keeps all 256 physical
    source indices when M11 owns a source-proven indexed palette. The prior
    four-bit mask could corrupt an external DM2 BPP8 TITLE/menu/credits
    capture even though the live SDL surface had the correct dtPalIRGB
    palette. Keep the packaged-app evidence task open; this closes only that
    unbundled capture fidelity defect.
    2026-08-06 follow-up: the strict real-data boot probe now requires the
    source-correct `titleReady=1` for DM2's immediately interactive static
    `TITLE/0/dt07/4` menu. It no longer mistakes a ready menu for a pending
    title animation or presses Enter to demand an unsupported `GAME_LOAD`
    runtime handoff. A fresh PC-DOS capture is retained only as external
    verification evidence; no game bytes are copied or unpacked.
49. **DM2-REAL-DATA-REGRESSION-CORPUS:** Build hash/provenance-verified DM2
    GRAPHICS/DUNGEON/SKSAVE/SND test corpus and end-to-end runtime regressions
    that exercise the authentic production paths.
50. **DM2-END-TO-END-PLAYABILITY:** Integrate all verified DM2 startup,
    save, HUD, dungeon, scene, input, AI, audio, and transition routes into a
    complete real-data play session with fail-closed unsupported content.

- **DM1 original PC3.4 save corpus:** Closed 2026-07-30. The DOSBox-created
  original PC 3.4 `DMSAVE.DAT` is provenance-attested and passes the real
  F0435 -> F0433 -> F0435 core-state round trip. Configured discovery now
  prioritizes `saves/dm1/original-pc34` over ordinary Firestaff saves, so an
  un-attested duplicate cannot invalidate the verified corpus.
  Rechecked 2026-08-06 with two operator-supplied 48,561-byte saves from
  `Downloads/`: `DMSAVE.DAT` (SHA-256
  `26ccd1591ccf6ec9e53186e994f73924185143f82055312cafd474ed7abc9437`)
  and `DMSAVE (1).DAT` (SHA-256
  `ab7bb4a34b77bba033d7b6c31db32e7198a962b0e55c0644c0486f50bb361ecb`).
  Both pass `test_dm1_v1_original_save_pc34_backed_corpus_roundtrip` against
  the hash-resolved real `DUNGEON.DAT`. This broadens fixture-free PC34 save
  evidence to two distinct source files; it does not close the separate C13
  save corpus or packaged Mac capture requirements.

`F2606-F2685` has been removed from the CSB queue: those symbols do not
exist in the ReDMCSB inventory, whose callable F range ends at F2104. Do not
create substitute wrappers for this invalid range.

`L0101-L0150` has also been removed: they are local labels inside ReDMCSB
F0108-F0115, already reached through M11 production paths, not independent
callable symbols.

## Recently Completed

- **2026-07-23 DM1 G0701-G0749/L0151-L0200 and HoC input:** Startup/media
  globals and F0115/F0116 local labels are source-audited. HoC C162 Cancel
  now restores the C127 selection sensor, so each of the 24 authentic mirror
  candidates can be cancelled and selected again. Verification: focused G,
  L, and HoC candidate-apply tests.

- **2026-07-23 DM1 G0651-G0700 inventory:** Cache, memory, LZW platform
  boundaries, and viewport globals now have source-owner records. The five
  unowned ReDMCSB allocator-list globals remain explicitly fail-closed; no
  independent global ABI or synthetic asset state was added. Verification:
  `dm1_v1_g0651_g0700_cache_lzw_viewport_source_audit_pc34_compat`.

- **2026-07-23 DM1/CSB save and presentation follow-up:** configured PC3.4
  save discovery now finds only classifier-qualified material in bounded
  save roots; FUSE Fluxcages use a shared raw-C15/live-C50 source binding;
  CSB C001 admission requires visible original PRESENTS/CHAOS/STRIKES pixels;
  and CSB quicksave fixtures keep F0435 clocks coherent. Verification:
  original-save classifier, `m11_action_stamina_runtime_source_lock`,
  CSB IMG3/real-sequence, M12 quick-resume, and CSB resume-gate tests.

- **2026-07-23 DM1 G0601-G0650 inventory:** Mouse/champion-input,
  GRAPHICS.DAT/viewport, runtime-memory, and platform-boundary globals are
  source-audited without independent global storage. Real graphic material is
  required; unavailable paths remain fail-closed. Verification:
  `dm1_v1_g0601_g0650_mouse_graphics_memory_source_audit_pc34_compat`.

- **2026-07-23 PC3.4 save key-step correction:** F0417/F0418, F0429, F0798,
  F7055/F7056, classifier, exporter, and handoff fixtures now advance their
  rolling key by remaining words (`N, N-1, ... 1`) as documented by DMWeb's
  saved-game format pseudocode. This is test-vector verified, pending an
  authentic original-save corpus.

- **2026-07-23 ReDMCSB P0551-P0600 inventory:** All 51 parameter entries
  (including the two P0593 declarations) are bound to their enclosing
  F0265-F0284 contracts. They are not independent runtime symbols and may
  not gain synthetic storage or wrappers. Verification:
  `dm1_v1_p0551_p0600_parameter_owner_audit`.

- **2026-07-23 DM1 M11 action/spell source ownership:** F0190 death smoke,
  F0224 Fluxcage/C24 removal, F0405 charge mutation, C29 Lord Chaos danger,
  and the F0445/F0446 FUSE path now bind raw C15 ownership and compact PC34
  square lists. Restored C50 records may use their present raw owner while a
  nonzero C25 fingerprint must still match. Verification:
  `m11_action_stamina_runtime_source_lock` (1340 assertions).

- **2026-07-23 DM1 compact square-list mutation:** F0190 fixed possessions
  and projectile drops now use ReDMCSB F0514 for real compact
  `SquareFirstThings`, including source tile flags and cumulative columns.
  Verification: compact M11 fixed-possession regression.

- **2026-07-23 CSB Utility Disk session admission:** Utility/HUD capture now
  consumes the authenticated C004/C002/C003 package session rather than a
  release-wrapper route. Verification: utility capture admission and real
  package presentation probe.

- **2026-07-23 CSB `TT_ParameterMessage` restored-save handoff:** The
  existing TimerQueue route now admits function 101 through its native
  OPENROOM/STONEROOM DSA path and binds the exact `EDT_MessageParameters`
  EXPOOL payload count and FNV identity to the restored-timer receipt.
  Payload drift invalidates the receipt. Verification:
  `csb_v1_dsa_admitted_restored_timer_bridge` and
  `csb_v1_dsa_parameter_message_save_handoff`.

- **2026-07-23 CSB title/entrance/HUD package presentation:** TITLE source
  steps now use the same playback frame and terminal C017/C040 uses the
  neutral source palette. Real CSB package probe: 27/27 checks.

- **2026-07-23 DM1 action-menu C010/C011 geometry:** F0387's 96x45 clear
  rectangle no longer rejects the authentic 87x45 C010 action graphic. The
  M11 asset path stays fail-closed for absent or mismatched source material.
  Verification: focused action/spell asset test.

- **2026-07-23 CSB-007 monster-kill EXPOOL writeback:** Existing authenticated
  `ESTAT_NumMonsterKilled` records now update in place using the CSBWin
  Code11f52 contract. Missing records remain non-allocating and fail closed.
  Verification: focused EXPOOL recovery test.

- **2026-07-23 DM1 G0551-G0600/P0501-P0550 and CSB F2526-F2605 inventory
  batch:** Save/media/input globals, projectile/melee parameters, and CSB
  unmapped/unowned routes are source-audited. Unproved PC34 paths remain
  fail-closed. Verification: four focused tests.

- **2026-07-23 ReDMCSB L0051-L0100 inventory batch:** Local-symbol entries
  are source-audited. None has an independent PC34 owner; every route remains
  fail-closed. Verification: focused compatibility test.

- **2026-07-23 DM1 G0501-G0550/P0451-P0500 and CSB F2446-F2525 inventory
  batch:** Graphic/save globals, projectile/melee parameters, and CSB
  unmapped/unowned routes are source-audited. Unproved PC34 paths remain
  fail-closed. Verification: four focused tests.

- **2026-07-23 DM1 M0451-M0500 inventory batch:** The final confirmed macro
  label boundary is source-audited against ReDMCSB. M500 has no independent
  verified PC34 owner and remains fail-closed. Verification: focused test.

- **2026-07-23 DM1 G0451-G0500/P0401-P0450 and CSB F2366-F2445 inventory
  batch:** Graphic560 globals, group-projectile parameters, and CSB
  unmapped/unowned routes are source-audited. Unproved PC34 paths remain
  fail-closed. Verification: four focused tests.

- **2026-07-23 DM1 M0401-M0450 inventory batch:** Macro labels are
  source-audited against ReDMCSB. Unproved PC34 paths remain fail-closed.
  Verification: focused compatibility test.

- **2026-07-23 DM1 P0351-P0400 and CSB F2326-F2365 inventory batch:** Group
  combat parameters and unowned CSB routes are source-audited. Unproved PC34
  paths remain fail-closed. Verification: two focused tests.

- **2026-07-23 DM1 G0401-G0450/M0351-M0400 and CSB F2286-F2325 inventory
  batch:** Movement/panel/input globals, macro labels, and unmapped platform
  routes are source-audited. Unproved PC34 paths remain fail-closed.
  Verification: three focused tests.

- **2026-07-23 CSB F2246-F2285 inventory batch:** Towns-memory ownership is
  source-audited against ReDMCSB. Unproved PC34 paths remain fail-closed.
  Verification: focused compatibility test.

- **2026-07-23 DM1 G0351-G0400/M0301-M0350/P0301-P0350 and CSB
  F2206-F2245 inventory batch:** Message/timeline globals, macro labels,
  group parameters, and platform routes are source-audited. Unproved PC34
  paths remain fail-closed. Verification: four focused tests.

- **2026-07-23 DM1 M0251-M0300 and CSB F2126-F2205 inventory batch:**
  Macro labels and CSB platform/CPSX routes are source-audited. Unproved PC34
  paths remain fail-closed. Verification: three focused tests.

- **2026-07-23 DM1 G0301-G0350/P0251-P0300 inventory batch:** Base-runtime
  globals and dungeon-map parameters are source-audited against ReDMCSB.
  Unproved PC34 paths remain fail-closed. Verification: two focused tests.

- **2026-07-23 DM1 M0201-M0250 and CSB F2046-F2125 inventory batch:**
  Macro labels and CSB platform/portrait-input routes are source-audited.
  Unproved PC34 paths remain fail-closed. Verification: three focused tests.

- **2026-07-23 DM1 P0201-P0250 inventory batch:** Dungeon parameters are
  source-audited against ReDMCSB. Unproved PC34 paths remain fail-closed.
  Verification: focused compatibility test.

- **2026-07-23 DM1 G0251-G0300/M0151-M0200 and CSB F1966-F2045 inventory
  batch:** Dungeon-state globals, macro labels, and CSB hint/input routes are
  source-audited. Unproved PC34 paths remain fail-closed. Verification: four
  focused tests.

- **2026-07-23 DM1 G0201-G0250/M0101-M0150/P0151-P0200 and CSB
  F1886-F1965 inventory batch:** Graphic tables, macro labels, video
  parameters, and CSB hint-load/CPSX routes are source-audited. Unproved PC34
  paths remain fail-closed. Verification: five focused tests.

- **2026-07-23 DM1 G0151-G0200/M0051-M0100/P0101-P0150 and CSB
  F1766-F1885 inventory batch:** Graphics globals, macro labels, parameters,
  media and hint/I/O routes are source-audited. Unproved PC34 paths remain
  fail-closed. Verification: five focused tests.

- **2026-07-23 DM1 G0101-G0150/M0001-M0050 and CSB F1806-F1845 inventory
  batch:** Graphics globals, macro labels, and memory/I/O routes are
  source-audited. Unproved PC34 paths remain explicitly fail-closed.
  Verification: three focused tests.

- **2026-07-23 DM1 P0051-P0100 / CSB F1686-F1765 inventory batch:** Text and
  sound parameters plus CSB USIO/MUSC/source routes are source-gated. Unproved
  PC34 paths remain fail-closed with no synthetic runtime behavior.
  Verification: three focused tests.

- **2026-07-23 DM1 G0051-G0100 / C001-C004/E/R/S inventory batch:** Global,
  constant, exception, system, and special ownership is source-audited against
  ReDMCSB. Unproved boundaries remain explicit and fail closed. Verification:
  two focused compatibility tests.

- **2026-07-23 DM1 P0001-P0050 / CSB F1606-F1685 inventory batch:** DM1
  parameters remain bound to their verified ReDMCSB owners. CSB VDI/platform
  routes are source-gated and fail closed without authentic PC34 material.
  Verification: three focused tests.

- **2026-07-23 DM1 G0001-G0050 inventory batch:** Global graphics-state
  ownership is source-bound to existing DM1 routes. Unverified globals are
  explicit and fail-closed. Verification: focused compatibility test.

- **2026-07-23 DM1 L0001-L0050 / CSB F1526-F1605 inventory batch:** DM1
  locals remain bound to their verified ReDMCSB owners; CSB platform/AES/TOS
  routes are source-gated and fail closed without authentic PC34 material.
  Verification: three focused tests.

- **2026-07-23 DM1 F2026-F2065 source batch:** Editor/input ownership is
  source-bound. Existing input owners remain fail-closed where no authentic
  PC34 material is available. Verification: focused compatibility test.

- **2026-07-23 DM1 F1986-F2025 / F2066-F2104 and CSB F1406-F1525 source
  batch:** Editor/hint ownership and CSB unmapped/Switch/VDI routes are
  source-gated. Only verified PC34 owners remain available; others fail closed.
  Verification: four focused compatibility tests.

- **2026-07-23 DM1 F1826-F1865 / F1946-F1985 and CSB F1446-F1485 source
  batch:** DM1 source/hint ownership and CSB unowned boundaries are explicit.
  Only verified PC34 owners remain admitted; all other paths fail closed.
  Verification: three focused compatibility tests.

- **2026-07-23 DM1 F1906-F1945 / CSB F1326-F1405 source batch:** Hint, FIO,
  SWSH and vblank ownership is source-gated. Missing PC34 material fails
  closed with no fabricated runtime behavior. Verification: three focused tests.

- **2026-07-23 DM1 F1866-F1905 source batch:** Hint ownership is audited;
  without verified raw PC34 material, all candidate routes remain fail-closed.
  Verification: focused audit test.

- **2026-07-23 DM1 F1746-F1785 / CSB F1246-F1325 source batch:** DM1 debug
  and error ownership plus CSB source/language/FIO routes are source-gated.
  Unproved PC34 paths remain fail-closed without substitute behavior.
  Verification: three focused compatibility tests.

- **2026-07-23 DM1 F1706-F1725 / F1786-F1825 source batch:** MUSC/floppy and
  animation ownership are source-gated. Without raw authenticated PC34
  material, runtime routes remain fail-closed. Verification: two focused tests.

- **2026-07-23 CSB F1266-F1285 source batch:** SWSH and platform ownership
  requires authentic PC34 material. Unsupported routes remain fail-closed with
  no synthetic presentation, timing, input, or media behavior. Verification:
  focused compatibility test.

- **2026-07-23 DM1 F1666-F1685 / F1726-F1745 source batch:** INT1/USIO
  ownership is source-bound and absent callable ranges are explicit. No
  synthetic PC34 behavior is admitted. Verification: two focused tests.

- **2026-07-23 CSB F1226-F1245 source batch:** Animation and audio ownership
  is source-gated to authentic PC34 material. Missing source bodies stay
  fail-closed without synthetic graphics, timing, or audio behavior.
  Verification: focused compatibility test.

- **2026-07-23 DM1 F1686-F1705 / CSB F1186-F1205 source batch:** USIO and
  ANIM ownership retains verified source owners and blocks all unproved PC34
  routes without synthetic presentation, timing, or media behavior.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 F1626-F1645 / CSB F1206-F1225 source batch:** Source
  ownership is bound to authentic PC34 material. Unproved routes remain
  fail-closed without synthetic graphics, UI, timing, input, or media behavior.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 F1586-F1605 / F1646-F1665 source batch:** TOS/AES,
  Switch, and video ownership is documented from the source corpus. PC34 lacks
  authenticated material for these routes, which remain fail-closed.
  Verification: two focused compatibility tests.

- **2026-07-23 CSB F1166-F1185 source batch:** USIO and animation boundaries
  require authentic PC34 material; missing owners remain fail-closed without
  synthetic graphics, UI, timing, input, or media behavior. Verification:
  focused compatibility test.

- **2026-07-23 DM1 F1506-F1525 source batch:** Source ownership is explicitly
  bound to authentic PC34 material. Unsupported paths remain fail-closed with
  no synthetic UI, graphics, timing, input, or media behavior. Verification:
  focused compatibility test.

- **2026-07-23 DM1 F1526-F1545 / CSB F1126-F1145 source batch:** Workstation
  and AES boundaries plus CSB source routes remain explicitly source-gated.
  Missing PC34 material fails closed without synthetic behavior. Verification:
  two focused compatibility tests.

- **2026-07-23 DM1 F1486-F1505 / CSB F1146-F1165 source batch:** DM1's
  non-PC34 switch routes and CSB copy-protection/USIO boundaries remain
  source-gated and fail-closed. Verification: two focused compatibility tests.

- **2026-07-23 DM1 F1446-F1485 source batch:** Local ownership is source-bound
  and absent callable ranges are explicit. No synthetic graphics, UI, timing,
  input, or media behavior is admitted. Verification: two focused tests.

- **2026-07-23 DM1 F1406-F1425 / CSB F1066-F1125 source batch:** DM1's
  unmapped interval and CSB Amiga/media boundaries are explicit. No missing
  source is substituted; unsupported PC34 paths fail closed. Verification:
  three focused compatibility tests.

- **2026-07-23 DM1 F1386-F1405 / F1426-F1445 source batch:** Local ownership
  is source-bound; intervals with no ReDMCSB callable symbols are explicit and
  fail-closed. No synthetic graphics, UI, timing, or media route was added.
  Verification: two focused compatibility tests.

- **2026-07-23 CSB F1086-F1105 source batch:** Input ownership remains bound
  to authentic PC34 material. Unproved routes are blocked without synthetic
  input, UI, timing, or runtime execution. Verification: focused compatibility
  test.

- **2026-07-23 DM1 F1326-F1385 source batch:** Media, FIO/floppy, swoosh,
  and vblank boundaries are source-bound where authentic PC34 owners exist;
  unsupported paths remain fail-closed without synthetic presentation or I/O.
  Verification: three focused compatibility tests.

- **2026-07-23 CSB F1006-F1065 source batch:** Source and save/platform
  boundaries are verified against ReDMCSB. Authentic PC34 material is required;
  unproved paths are fail-closed without synthetic graphics, UI, timing, or
  runtime behavior. Verification: two focused compatibility tests.

- **2026-07-23 DM1 F1306-F1325 source batch:** Existing FIO owners are
  retained where source-backed; all other unsupported boundaries remain
  fail-closed without synthetic file, device, or media behavior. Verification:
  focused audit test.

- **2026-07-23 DM1 F1266-F1305 / CSB F1026-F1045 source batch:** DM1
  ownership is bound across input, language, and FIO boundaries; CSB platform
  video routes remain source-gated. Missing PC34 material is fail-closed with
  no synthetic UI, graphics, timing, or input. Verification: three focused
  compatibility tests.

- **2026-07-23 DM1 F1246-F1265 source batch:** Animation and media ownership
  are audited against authentic PC34 paths. No verified owner is fabricated;
  all unsupported routes remain fail-closed. Verification: focused audit test.

- **2026-07-23 CSB F0926-F1005 source batch:** Platform, loader, and
  graphics boundaries are audited against ReDMCSB. Unproved PC34 routes remain
  blocked without synthetic graphics, UI, timing, or runtime execution.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 F1206-F1245 source batch:** I/O, animation, and audio
  ownership are bound to authentic PC34 material. Missing source bodies and
  non-PC34 media paths remain fail-closed with no synthetic behavior.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 Save & Quit user-save path:** F0433 callers now share the
  user-data `saves/dm1` path, create its parent before writing, and report an
  error only for an invalid directory or actual serializer/write failure.
  Verification: focused save-path test.

- **2026-07-23 DM1 F1186-F1205 source batch:** Animation-step ownership is
  source-bound to authentic PC34 material. Missing source bodies and raw
  animation input remain fail-closed with no synthetic timing or graphics.
  Verification: focused compatibility test.

- **2026-07-23 DM1 F1126-F1145 / F1166-F1185 source batch:** I/O, USIO, and
  animation boundaries are now bound to authentic PC34 ownership. Unsupported
  material remains fail-closed with no synthetic data, timing, UI, or graphics.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 F1146-F1165 / CSB F0886-F0925 source batch:** DM1 I/O
  ownership is explicitly fail-closed where PC34 admission is absent. CSB
  preserves authenticated media/palette owners and source-gates swoosh
  primitives. No synthetic UI, graphics, timing, or actions were added.
  Verification: three focused compatibility tests.

- **2026-07-23 DM1 F1086-F1105 source batch:** Platform/input ownership is
  source-bound where authentic PC34 material exists. Unsupported paths remain
  fail-closed without synthetic input, UI, graphics, or timing.
  Verification: focused compatibility test.

- **2026-07-23 DM1 F1006-F1025 / F1106-F1125 source batch:** Existing PC34
  command and palette owners are source-bound, while unsupported media and
  platform paths remain fail-closed with no synthetic UI, graphics, timing,
  or actions. Verification: two focused compatibility tests.

- **2026-07-23 CSB F0866-F0885 source batch:** Source boundaries are audited
  and unsupported PC34 paths fail closed without synthetic graphics, UI,
  timing, or actions. Verification: focused compatibility test.

- **2026-07-23 DM1 F1066-F1085 / CSB F0846-F0865 source batch:** DM1's sole
  supported owner stays source-bound; Amiga-only and unmapped CSB boundaries
  fail closed with no synthetic UI, graphics, timing, or actions.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 F1046-F1065 / CSB F0826-F0845 source batch:** DM1
  platform/save owners are source-bound to authentic PC34 material. CSB
  source boundaries reject unsupported routes. All unavailable paths stay
  fail-closed without synthetic UI, graphics, timing, or actions.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 F1026-F1045 source batch:** Platform-owner boundaries are
  source-audited. Unsupported PC34 paths remain fail-closed with no synthetic
  platform behavior, graphics, UI, or timing. Verification: focused audit
  test.

- **2026-07-23 CSB F0806-F0825 source batch:** Startup owner admission now
  requires authenticated PC34 package material. Missing or legacy paths stay
  fail-closed without substitute startup UI, graphics, timing, or actions.
  Verification: focused compatibility test.

- **2026-07-23 DM1 F0946-F1005/L0966-L0985 / CSB F0786-F0805 source batch:**
  DM1 source ownership, local champion provenance, graphics, platform, and
  PC-98 boundaries are audited against real material. CSB panel/layout
  contracts require authenticated PC34 data. Missing paths fail closed with no
  synthetic rendering, input, or presentation. Verification: four focused
  compatibility tests.

- **2026-07-23 CSB F0766-F0785 source batch:** Authenticated PC34 package
  admission is now required for source-bound owners. Missing or legacy
  material fails closed without substitute UI, graphics, timing, or actions.
  Verification: focused compatibility test.

- **2026-07-23 DM1 F0926-F0945 source batch:** Platform and loader ownership
  is bound to verified PC34 material. Missing source bodies and host-only
  boundaries remain fail-closed without synthetic loading or presentation.
  Verification: focused compatibility test.

- **2026-07-23 DM1 F0886-F0925 / CSB F0746-F0765 source batch:** Existing
  DM1 media, bitplane, palette, sound, and primitive owners are source-bound;
  CSB memory/language contracts require authenticated PC34 material. Unknown
  paths are fail-closed without synthetic rendering or host behavior.
  Verification: three focused compatibility tests.

- **2026-07-23 DM1 P0866-P0885 / CSB F0706-F0725 source batch:** DM1
  parameter provenance is bound to its verified callable owners, while CSB
  package admission requires authenticated PC34 material. Copy-protection and
  missing-package paths fail closed with no substitute behavior.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 F0826-F0865 inventory batch:** Local-symbol references are
  tied to their verified callable owners and the remaining unavailable PC34
  owners are explicit fail-closed. No standalone synthetic UI, graphics, or
  timing path was introduced. Verification: two focused audit tests.

- **2026-07-23 CSB F0726-F0745 source batch:** Media and filename contracts
  are source-gated on authenticated original PC34 data. Missing evidence
  remains fail-closed with no fabricated files, media, or presentation.
  Verification: focused raw-material compatibility test.

- **2026-07-23 DM1 F0786-F0825 source batch:** Runtime panel, media, and text
  ownership is source-bound to real PC34 material. Unknown paths remain
  fail-closed without synthetic panel, text, or host behavior.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 F0726-F0745 source batch:** PC34 no-op and existing-owner
  boundaries are source-locked. Unsupported symbols have no fabricated
  mapping; no substitute input, graphics, or timing is introduced.
  Verification: focused ownership compatibility test.

- **2026-07-23 DM1 F0766-F0785 source batch:** File and mouse ownership is
  source-audited against PC34. Unknown paths stay fail-closed with no host
  substitute or synthetic input behavior. Verification: focused audit test.

- **2026-07-23 CSB F0666-F0705 source batch:** Presentation, video, and input
  contracts require authenticated original PC34 material. Missing evidence
  stays non-rendering and non-mutating, without fallback screens or input.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 F0686-F0705 / F0746-F0765 source batch:** Runtime
  graphics, memory, and I/O ownership is source-bound to real PC34 material.
  Unproven routes remain fail-closed with no invented visuals or host actions.
  Verification: two focused compatibility tests.

- **2026-07-23 DM1 F0541-F0560 / F0706-F0725 source batch:** Platform,
  mouse, text/scroller, I/O, and graphics ownership is now recorded against
  the real PC34 path. Amiga/IIGS-only and unproven routes fail closed without
  synthetic input, UI, or graphics. Verification: two focused tests.

- **2026-07-23 CSB F0646-F0665 source batch:** Text, bitmap, palette, and
  click contracts are source-gated to authenticated PC34 material. Missing
  evidence cannot render a substitute surface or invoke fallback input.
  Verification: focused raw-material compatibility test.

- **2026-07-23 DM1 F0666-F0685 source batch:** Endgame and graphics owners
  are source-bound to authenticated PC34 material. Unproven material remains
  fail-closed without substitute graphics, text, or dialogs.

- **2026-07-23 DM1 F0646-F0665 source batch:** Text, timeline, bitmap,
  palette, and dungeon-click owners are bound to existing authenticated
  routes. Missing material stays fail-closed, without synthetic frames or
  input behavior. Verification: eleven focused compatibility checks.

- **2026-07-23 DM1 F0621-F0645 / CSB F0600-F0620 source batch:** Existing
  champion/layout and core-material owners now require authenticated original
  PC34 data. Missing material is rejected without fallback UI, graphics, or
  actions. Verification: two focused compatibility tests.

- **2026-07-23 CSB F0621-F0645 source batch:** Champion, layout, font, and
  text contracts now require authenticated PC34 material. Unproven routes
  remain non-rendering and non-mutating rather than fabricating UI or text.
  Verification: focused raw-material compatibility test.

- **2026-07-23 DM1 F0600-F0620 source batch:** Existing dialog, cache,
  bitmap, zone, and action-list owners are source-bound. Missing original
  material remains fail-closed; no fabricated assets or UI was added.
  Verification: focused PC34 compatibility test.

- **2026-07-23 DM1 F0561-F0581 source batch:** Entrance and platform
  ownership is now tied to authenticated original material; unsupported
  Amiga/floppy/VBlank routes stay fail-closed with no substitute timing or
  graphics. Verification: focused PC34 compatibility test.

- **2026-07-23 DM1 F0481-F0540 / CSB F0526-F0585 source batch:** Bound the
  remaining graphics-cache, platform, and runtime ownership receipts to
  authenticated original material. Unproven Amiga/floppy/cache paths remain
  fail-closed; no synthetic graphics, input, timing, or UI was introduced.
  Verification: five focused PC34 compatibility tests.

## Cycle 16 Completed Lanes

- **Lane D — DM2-010 creature/cloud passes (cycle 16):** Done.
  Source-locked the `_4976_5aa4` occupancy grid and the `DRAW_FLYING_ITEM`
  selection rules against skproject SKWIN/SkWinCore.cpp
  (QUERY_CREATURE_5x5_POS, DRAW_STATIC_OBJECT's occupancy walk,
  DRAW_FLYING_ITEM) and SkGlobal.cpp
  _4976_43f5/_4976_4415/_4976_41a9/tlbDisplayOrder*:
  - New viewport helpers: `dm2_v1_viewport_creature_occupancy_5x5` (info
    slot 0xff centres at 12, otherwise the anchor rotates by
    (party_dir - creature_dir) & 3), `dm2_v1_viewport_occupancy_grid_coords`
    (the _4976_5aa4 grid coordinate from the _4976_43f5 cell bases),
    `dm2_v1_viewport_static_object_display_index`,
    `dm2_v1_viewport_flying_item_scale64` (_4976_41a9 band table with the
    negative-band draw block) and
    `dm2_v1_viewport_flying_item_image_field` (the _48ae_011a frame class,
    timer-direction parity, tile parity and mirror bits; fields 8/9/10/12).
  - Creature pass leaves the F9 map-chip route where the real FB/FC/FD V5
    animation chain resolves: `dm2_runtime_populate_creatures` resolves each
    G1 record through `dm2_v1_boot_dynamic_creature_material_receipt` (base
    frame, view-relative direction) and binds the exact decoded-image
    evidence in the new `DM2_V1_G1CreatureV5RuntimeReceipt`; the render plan
    draws CREATURES/type/field for those rows and the render gate verifies
    the decoded hash + palette identity instead of the F9 instance receipt.
  - The creature render plan now carries occupancy evidence (5x5 position,
    display-order index, source pass) and reorders the pass by
    (pass, display index) only when every row is proven — otherwise the
    existing order stays fail-closed.
  - `DM2_V1_G1DirectCreatureRoot` now carries the record-owned cursor
    fields b5/w8/w10 (DME.h::Creature), the evidence base for static-object
    creature cursors.
  - Canonical-corpus outcome (proven by the new real-data test and probe):
    all 33 direct DB4 roots stay fail-closed — their V5 images are 8bpp
    global-palette (no bounded 16-color route exists) or palette-less; the
    V5 chain itself resolves for the 4bpp type-2 class in the same GDAT,
    which has no dungeon roots.  The corpus has zero direct dbMissile/
    dbCloud roots, so DRAW_FLYING_ITEM stays fail-closed on this data.
  - Tests: new `tests/test_dm2_v1_creature_occupancy_flying_item.c` (34/34)
    and real-data `tests/test_dm2_v1_g1_creature_viewport_field_real_data.c`
    (38/38); new probe `probes/dm2/firestaff_dm2_v1_creature_occupancy_probe.c`
    (42/0 across all maps).  Cycle-14/15 suites stay green:
    draw_item_source_placement 106/106, g1_static_object_visibility 39/39,
    static_object_pixel_probe 11/0, draw_item_source_pass_probe 135/0,
    runtime_handoff_smoke, item_projectile_rect14, g1_static_m11_handoff_gate,
    g1_creature_material_graph_gate, dynamic_creature_material_plan.
  Remaining: 8bpp/global-palette creature images stay blocked until a
  source-owned 256-colour route exists; the Rect14/animation-sequence
  tables (dt07/0x0A, dt06/0) are absent from the canonical PC English
  GRAPHICS.DAT, so QUERY_CREATURE_PICST's Rect14 geometry stays unproven on
  this corpus; flying-item pixels wait for real dbMissile records.

## Recently Completed

- **DM1 F0441-F0460 and CSB F0546-F0565 batch:** Done 2026-07-23. DM1
  save/endgame owners are source-audited and unproven floppy/startup paths fail
  closed. CSB Amiga mouse/video/text/scroller paths are explicit fail-closed
  boundaries. Verification:
  `dm1_v1_f0441_f0460_save_endgame_source_audit_pc34_compat` and
  `csb_v1_f0546_f0565_platform_boundary_pc34_compat` pass.

- **DM1 F0421-F0440 and F0461-F0480 batch:** Done 2026-07-23. Existing save,
  dialog, palette, startup, and core-render owners are source-bound; original
  `GRAPHICS.DAT` header/cache/wall material is required and platform gaps fail
  closed. Verification:
  `dm1_v1_f0421_f0440_save_endgame_source_audit_pc34_compat` and
  `dm1_v1_f0461_f0480_core_render_source_receipt_pc34_compat` pass.

- **CSB F0486-F0525 graphics/platform batch:** Done 2026-07-23. Graphics/LZW
  raw contracts are source-bound, while unproven Amiga/floppy platform paths
  are explicit fail-closed boundaries with no synthetic replacement. Verification:
  `csb_v1_f0486_f0505_graphics_lzw_raw_pc34_compat` and
  `csb_v1_f0506_f0525_platform_boundary_pc34_compat` pass.

- **DM1 F0361-F0420 core/render batch:** Done 2026-07-23. Existing command,
  action, melee, spell, and save owners stay source-bound; unproven command
  paths and F0413 fail closed. Verification:
  `dm1_v1_f0361_f0380_core_action_source_audit_pc34_compat` and
  `dm1_v1_f0401_f0420_render_core_source_receipt_pc34_compat` pass.

- **DM1 F0381-F0400 core/viewport batch:** Done 2026-07-23. Existing
  ReDMCSB-derived owners require raw PC34 material; unavailable paths fail
  closed without new rendering or UI. Verification:
  `dm1_v1_f0381_f0400_core_viewport_source_audit_pc34_compat` passes.

- **DM1 F0341-F0360 and CSB F0446-F0485 batch:** Done 2026-07-23. DM1 keeps
  its established panel owners with fail-closed F0356. CSB core/action and
  graphics-memory contracts require authenticated source material without
  synthetic graphics, UI, or cache mutation. Verification:
  `dm1_v1_f0341_f0360_render_action_source_receipt_pc34_compat`,
  `csb_v1_f0446_f0465_core_action_raw_pc34_compat`, and
  `csb_v1_f0466_f0485_graphics_memory_pc34_compat` pass.

- **DM1 F0321-F0340 core/viewport batch:** Done 2026-07-23. Existing owners
  require raw PC34 material and unavailable paths remain fail closed, without
  new synthetic rendering or UI. Verification:
  `dm1_v1_f0321_f0340_core_viewport_source_audit_pc34_compat` passes.

- **DM1 F0301-F0320 core-action batch:** Done 2026-07-23. Existing slot,
  stat, combat, scent, and lifecycle owners are source-audited without a new
  synthetic runtime path. Verification:
  `dm1_v1_f0301_f0320_core_action_source_audit_pc34_compat` passes.

- **DM1 F0281-F0300 and CSB F0406-F0445 batch:** Done 2026-07-23. DM1
  rename/resurrection/HUD paths require original PC34 material. CSB core,
  viewport and start/end contracts stay source-bound and fail closed without
  synthetic actions, UI, save, or rendering. Verification:
  `dm1_v1_f0281_f0300_champion_source_receipt_pc34_compat`,
  `csb_v1_f0406_f0425_core_viewport_pc34_compat`, and
  `csb_v1_f0426_f0445_startend_raw_pc34_compat` pass.

- **DM1 F0221-F0240 and F0261-F0280 batch:** Done 2026-07-23. Existing
  endgame, AI, combat, timeline, movement, and champion owners are
  source-audited with raw PC34 data and no synthetic UI/party mutation.
  Verification: `dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34_compat`
  and `dm1_v1_f0261_f0280_movement_champion_source_audit_pc34_compat` pass.

- **DM1 F0241-F0260 timeline/dungeon batch:** Done 2026-07-23. Existing
  launcher/relocation/quiver owners are retained; F0256 is explicit PC34
  fail-closed. Verification:
  `dm1_v1_f0241_f0260_timeline_source_receipt_pc34_compat` passes.

- **CSB F0386-F0405 action/viewport batch:** Done 2026-07-23. Authenticated
  PC34 action/viewport contracts block unproven runtime execution. Verification:
  `csb_v1_f0386_f0405_action_viewport_raw_pc34_compat` passes.

- **CSB F0366-F0385 command/viewport batch:** Done 2026-07-23. The command
  and viewport receipt is authenticated, read-only, and source-audited without
  synthetic UI/rendering. Verification:
  `csb_v1_f0366_f0385_command_viewport_pc34_compat` passes.

- **DM1 F0181-F0220 group/action batch:** Done 2026-07-23. Existing DM1
  owners are bound to raw PC34 evidence; C04/C38/C14/C15 are required for
  F0209 and F0210/F0211 fail closed. Verification:
  `dm1_v1_f0181_f0200_group_source_audit_pc34_compat` and
  `dm1_v1_f0201_f0220_action_source_receipt_pc34_compat` pass.

- **DM1 F0161-F0180 dungeon/group batch:** Done 2026-07-23. Existing DM1
  source owners are retained without a new synthetic render/runtime path.
  Verification: `dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34_compat`
  passes.

- **CSB F0346-F0365 panel/input batch:** Done 2026-07-23. Authenticated PC34
  material drives source-audited panel/input contracts; unproven runtime
  execution is blocked. Verification:
  `csb_v1_f0346_f0365_panel_input_raw_pc34_compat` passes.

- **DM1 F0141-F0160 and CSB F0326-F0345 batch:** Done 2026-07-23. Both
  source-owner/PC34 core receipts use authenticated material and avoid
  synthetic dungeon or UI behavior. Verification:
  `dm1_v1_f0141_f0160_dungeon_source_receipt_pc34_compat` and
  `csb_v1_f0326_f0345_core_ui_pc34_compat` pass.

- **DM1 F0082-F0099 runtime batch:** Done 2026-07-23. The audited runtime
  receipt uses source-bound PC34 semantics and avoids synthetic host behavior.
  Verification: `dm1_v1_f0082_f0091_runtime_pc34_compat` passes.

- **DM1 F0100-F0120 and CSB F0306-F0325 batch:** Done 2026-07-23. The DM1
  viewport source-owner catalog retains its existing renderers rather than
  creating a synthetic path. CSB contracts use authenticated PC34 material
  and block unproven runtime execution. Verification:
  `dm1_v1_early_viewport_family_audit_pc34_compat` and
  `csb_v1_f0306_f0325_champion_core_raw_pc34_compat` pass.

- **DM1 F0121-F0140 core/graphics batch:** Done 2026-07-23. The early
  core/graphics material is source-gated, F0137 is explicit PC34 fail-closed,
  and existing owners remain disjoint. Verification:
  `dm1_v1_f0121_f0140_core_graphics_source_receipt_pc34_compat` passes.

- **DM1 S0080/S0081 and F0074-F0079 batch:** Done 2026-07-23. Media/platform
  and mouse CPSC boundaries are source-bound and fail closed; F010 remains a
  separately owned symbol. Verification:
  `dm1_v1_s0080_s0081_media_platform_boundary_pc34_compat` and
  `dm1_v1_f0074_f0079_mouse_cpsc_boundary_pc34_compat` pass.

- **CSB F0284-F0289 champion/HUD batch:** Done 2026-07-23. Authenticated,
  read-only PC34 champion/HUD receipts avoid synthetic panel/font/party state;
  F0286 stays with its ordered-cell runtime owner. Verification:
  `csb_v1_f0284_f0289_champion_hud_pc34_compat` passes.

- **CSB F0290-F0305 post-champion batch:** Done 2026-07-23. Authentic PC34
  material drives source-audited raw contracts; execution is fail closed where
  a runtime owner is unproven. Verification:
  `csb_v1_f0290_f0305_post_champion_raw_pc34_compat` passes.

- **DM1 F0050-F0068 early UI batch:** Done 2026-07-23. Text/mouse material is
  source-gated, with no host-font or cursor fallback; F0060-F0065 remain under
  their separate audio owner. Verification:
  `dm1_v1_f0050_f0068_early_ui_source_receipt_pc34_compat` passes.

- **DM1/CSB low-level and champion batch:** Done 2026-07-23. DM1 F0018 and
  F003-F010 are explicit source-bound host boundaries. CSB F0276 is a raw
  sensor-add/remove receipt, and F0279-F0283 are raw champion/altar contracts;
  neither path invents synthetic mutation, rename, or resurrection behavior.
  Verification: `dm1_v1_main_lowlevel_boundary_pc34_compat`,
  `csb_v1_f0276_sensor_pc34_compat`, and
  `csb_v1_f0279_f0283_champion_altar_raw_pc34_compat` pass.

- **DM1 F0029-F0047 early object/text batch:** Done 2026-07-23. The audited
  PC34 helper family is source-bound without synthetic object/text output;
  F0030 remains separately owned. Verification:
  `dm1_v1_early_object_text_f0029_f0047_pc34_compat` passes.

- **DM1 F0069/F0070/F0073 mouse batch:** Done 2026-07-23. Champion-click
  input is source-gated to original C028 material; the unavailable PC34
  cursor-area path is explicit fail-closed rather than synthesized. Verification:
  `dm1_v1_f0069_f0070_f0073_mouse_source_receipt_pc34_compat` passes.

- **CSB F0277/F0278 raw-contract batch:** Done 2026-07-23. The fuzzy-sector
  and champion-reset source contracts are read-only, source-audited, and
  fail closed rather than emulating copy-protection or runtime reset state.
  Verification: `csb_v1_f0277_f0278_source_audit_pc34_compat` passes.

- **DM1 F0019-F0028 early runtime batch:** Done 2026-07-23. The source-bound
  startup helpers are isolated to authenticated PC34 input and preserve
  fail-closed platform boundaries; no synthetic interrupt, timer, or media
  behavior was introduced. Verification:
  `dm1_v1_early_runtime_f0019_f0028_pc34_compat` passes.

- **DM1/CSB source-locked runtime batch:** Done 2026-07-23. DM1 F0433 now
  stages PC34 save-command material, F0449/F0450 require authenticated floppy
  media, and F0902 consumes original PC34 SWSH data with its palette/timing.
  CSB F0275 provides a read-only raw wall-click receipt, while CSBWin feeding
  filters recover as raw DB11 data without runtime mutation. Verification:
  `dm1_v1_f0433_save_command_pc34_compat`,
  `dm1_v1_original_save_pc34_handoff`,
  `dm1_v1_f0449_f0450_floppy_media_guard_pc34_compat`,
  `dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34_compat`,
  `csb_v1_f0275_wall_click_pc34_compat`, and
  `csb_v1_csbwin_feeding_filter_expool_recovery` pass.

- **DM1 PC-98 copy-protection and CSB F0273 batch:** Done 2026-07-23. DM1
  F0809-F0811 require verified raw PC-98 capture/HDM media and reject
  synthesized disk responses. CSB F0273 uses only the raw PC34 Thing chain,
  including CELL_ANY behavior. Verification:
  `dm1_v1_f0809_f0811_copypro_media_gate` and
  `csb_v1_f0273_sensor_pc34_compat` pass.

- **DM1 F0447/F0448 PC34 boundary:** Done 2026-07-23. The unavailable PC34
  hang/memory-manager paths are explicit fail-closed boundaries, without a
  hang or synthetic memory handling. Verification:
  `dm1_v1_f0447_f0448_platform_boundary_pc34_compat` passes.

- **CSBWin Character-wing recovery:** Done 2026-07-23. The wing record
  recovers only as authenticated raw DB11 data, without assembly or runtime
  effects. Verification: `csb_v1_csbwin_wing_record_expool_recovery` passes.

- **DM1 F0432 format-disk menu:** Done 2026-07-23. The menu is source-bound
  to its original layout/material contract, without formatting behavior, UI
  fallback, or F0433+ ownership. Verification:
  `dm1_v1_f0432_format_disk_menu_pc34_compat` and
  `dm1_v1_f0424_f0427_dialog_admission_pc34_compat` pass.

- **DM1 endgame and CSB sensor/palette batch:** Done 2026-07-23. DM1
  F0444-F0446 require original endgame graphics, palette, dungeon/timeline,
  and victory music. CSB F0272/F0274 remain source-bound/fail-closed, and the
  CSBWin palette record recovers without restore or rendering. Verification:
  `dm1_v1_f0444_f0445_f0446_endgame_material_pc34_compat`,
  `csb_v1_f0272_f0274_sensor_pc34_compat`, and
  `csb_v1_csbwin_palette_record_expool_recovery` pass.

- **DM1 F0740-F0743 music source:** Done 2026-07-23. Pause/play/track/update
  require hash-verified PC34 `SONG.DAT`, manifest, and SEQ2; C2 is source-bound
  and unknown tracks fail closed. Verification:
  `dm1_v1_f0740_f0743_music_source_gate` and
  `dm1_v1_sound_music_source_lock` pass.

- **CSBWin GlobalVariables recovery:** Done 2026-07-23. Global variables
  recover only from authenticated raw DB11 records, without global-bank, DSA,
  or runtime behavior. Verification:
  `csb_v1_csbwin_global_variables_expool_recovery` passes.

- **DM1 F0442/F0443 credits material:** Done 2026-07-23. Credits and endgame
  text require original C005/palette plus authenticated scroll font/text; no
  rendering loop or host-font fallback is admitted. Verification:
  `dm1_v1_f0442_f0443_credits_text_material_pc34_compat` passes.

- **DM1 F0429/F0430 and CSBWin Skins batch:** Done 2026-07-23. DM1 reads and
  writes raw PC34 save headers without UI/media fallback; CSBWin default skins
  recover without zero-fill, cache, or rendering. Verification:
  `dm1_v1_original_save_pc34_handoff` and
  `csb_v1_csbwin_default_skins_expool_recovery` pass.

- **DM1 F0440/F0441 entrance asset flow:** Done 2026-07-23. Byte counts and
  entrance preparation require original C004/C005 and C002/C003 receipts; no
  lifecycle ownership or synthetic pages are accepted. Verification:
  `dm1_v1_f0440_f0441_entrance_asset_flow_pc34_compat` passes.

- **CSB F0270/F0271 sensors:** Done 2026-07-23. Local and rotation effects
  are source-bound to authenticated raw receipts and fail closed; adjacent
  sensor and movement functions remain isolated. Verification:
  `csb_v1_f0270_f0271_sensor_pc34_compat` passes.

- **CSBWin MessageParameters recovery:** Done 2026-07-23. Message parameters
  recover only from authenticated raw DB11 data, without timer scheduling or
  DSA execution. Verification:
  `csb_v1_csbwin_message_parameters_expool_recovery` passes.

- **CSB F0268/F0269 sensors:** Done 2026-07-23. Event and skill records use
  source-authenticated raw PC34 receipts and fail closed; adjacent movement and
  later sensor functions remain isolated. Verification:
  `csb_v1_f0268_f0269_sensor_pc34_compat` passes.

- **DM1 F0437-F0439 and F0732-F0735 visual batch:** Done 2026-07-23. Startup
  pages require verified C001-C004 data and original palettes; inventory fills
  require raw C009/C013/C017 data. Wrappers, replacement fonts, and synthetic
  surfaces are rejected. Verification:
  `dm1_v1_f0437_f0438_f0439_startup_visual_admission_pc34_compat` and
  `dm1_v1_f0732_f0735_fill_material_gate` pass.

- **DM1 F0418/F0423 save helpers:** Done 2026-07-23. Checksum reads follow
  the original pre-decrypt chain and clone repair accepts only raw
  `MEDIA340_S21E` bytes. Verification:
  `memory_savegame_pc34_f0417_saveutil_port_pc34_compat` and
  `dm1_v1_original_save_pc34_handoff` pass.

- **CSBWin ESL_SOUNDFILTER recovery:** Done 2026-07-23. The SpecialLocations
  word is recovered only from raw DB11 data and cannot invoke DSA, timers, or
  audio behavior. Verification:
  `csb_v1_csbwin_sound_filter_expool_recovery` passes.

- **CSB F0267 move result:** Done 2026-07-23. CPSCE accepts only an
  authenticated PC34 Thing/source/destination receipt; the absent IIGS body
  stays fail-closed. Verification:
  `csb_v1_f0267_move_result_receipt_pc34_compat` passes.

- **DM1 F0431/F0436 palette steps:** Done 2026-07-23. Darken/fade steps use a
  raw-PC34 palette adapter without title or entrance ownership and reject
  synthetic fallback. Verification:
  `dm1_v1_f0431_f0436_palette_step_pc34_compat` passes.

- **DM1 F0731/F0734 inventory-zone material:** Done 2026-07-23. Inventory
  zones require raw C009/C017 source, original zones, and C12 clear; no cursor,
  host font, or fallback route is used. Verification:
  `dm1_v1_f0731_f0734_inventory_zone_material_gate` passes.

- **CSBWin ExtendedCellFlags recovery:** Done 2026-07-23. Extended cell flags
  recover only from authenticated raw DB11 data, without defaults, DSA, or
  runtime effect. Verification:
  `csb_v1_csbwin_extended_cell_flags_expool_recovery` passes.

- **DM1 F0414-F0416 and CSB F0266 batch:** Done 2026-07-23. Save I/O and
  group projectile preflight now require source-authenticated raw data with no
  default or synthetic route. Verification: `dm1_v1_original_save_pc34_handoff`
  and `csb_v1_f0266_group_move_projectile_receipt_pc34_compat` pass.

- **DM1 F0424/F0427 dialog admission:** Done 2026-07-23. Dialog choice and
  draw require original graphic 17, palette, M653 font, and source text;
  host-font and synthetic fallbacks are rejected. Verification:
  `dm1_v1_f0424_f0427_dialog_admission_pc34_compat` passes.

- **CSBWin DisableSaves recovery:** Done 2026-07-23. The zero-payload DB11
  marker is recovered strictly as data, with no synthesized policy or runtime
  effect. Verification: `csb_v1_csbwin_disable_saves_expool_recovery` passes.

- **DM1 F0693/F0698/F0699 video-material batch:** Done 2026-07-23. Video
  operations require original PC34 material and reject synthetic text or
  palettes. Verification: `dm1_v1_f0693_f0699_video_material_gate` passes.

- **CSBWin DeleteDuplicateTimers recovery:** Done 2026-07-23. The policy word
  is recovered only from a complete authenticated DB11 record and has no
  runtime side effect. Verification:
  `csb_v1_csbwin_delete_duplicate_timers_expool_recovery` passes.

- **DM1 F0413 and F0830/F0831 batch:** Done 2026-07-23. Checksum EOR and the
  saved lifecycle now use authenticated raw save data and original GameTime,
  without synthesized movement time or idle bonuses. Verification:
  `dm1_v1_f0413_cpsc_checksum_eor_pc34_compat` and
  `dm1_v1_original_save_pc34_handoff` pass.

- **DM1 F0682, F0410/F0411 and C24 save-owner batch:** Done 2026-07-23.
  Transparent C486-C488 material, cast continuation and Fluxcage C24 save
  ownership now require their authenticated raw source data and fail closed.
  Verification: `dm1_v1_f0682_transparent_material_gate`,
  `dm1_v1_f0410_f0411_spell_cast_continuation_pc34_compat`, and
  `dm1_v1_original_save_pc34_handoff` pass.

- **CSB F0265 and EDBT_Debuging batch:** Done 2026-07-23. Group retries and
  the CSBWin debugging record are recovered only from authenticated raw data.
  Verification: `csb_v1_f0265_group_retry_receipt_pc34_compat` and
  `csb_v1_csbwin_debugging_expool_recovery` pass.

- **CSBWin EDBT_RuntimeFileSignatures recovery:** Done 2026-07-23. The full
  runtime signature triple requires one complete authenticated raw PC34 bundle;
  incomplete or drifting data fails closed. Verification:
  `csb_v1_csbwin_runtime_signatures_expool_recovery` passes.

- **DM1 F0408/F0409 cast admission:** Done 2026-07-23. Spell casting requires
  authenticated C108 click and C009/C011/M653 material; F0412 remains the
  exclusive effect/mutation owner. Verification:
  `dm1_v1_f0408_f0409_spell_cast_admission_pc34_compat` passes.

- **CSB F0252 raw group retry:** Done 2026-07-23. C60/C61 move/retry requires
  authenticated raw C04 and records its C08/C09 target in the receipt.
  Verification: `csb_v1_f0252_group_move_receipt_pc34_compat` passes.

- **DM1 F0675 scaled material:** Done 2026-07-23. Scaled C486/C487/C488
  graphics require raw PC34 data; smoke requires G0212 palette and no M653
  glyph source. Verification: `dm1_v1_f0675_scaled_material_gate` passes.

- **DM1 F0829 quicksave owner binding:** Done 2026-07-23. Quicksave/resume
  requires one raw C15/C25 owner for every F0435-captured explosion; drift
  fails closed. Verification: `dm1_v1_original_save_pc34_handoff` passes.

- **CSBWin EDBT_ObjectWeights recovery:** Done 2026-07-23. Chest weights are
  read only from a unique authenticated DB11 record; no base-50 default or
  fallback is used. Verification:
  `csb_v1_csbwin_chest_weight_expool_recovery` passes.

- **DM1 F0399/F0400 spell-symbol consumption:** Done 2026-07-23. Rune
  consumption requires authenticated F0369 C101-C107 and C009/C011/M653
  material; stale input leaves spell state unchanged. Verification:
  `dm1_v1_f0399_f0400_spell_symbol_consume_pc34_compat` passes.

- **CSB F0249 open-square group move:** Done 2026-07-23. A linked raw C04
  group moves only when C08/C09 opens; invalid or drifting data fails closed.
  Verification: `csb_v1_f0249_open_square_group_receipt_pc34_compat` passes.

- **CSBWin EDT_ChampionBones recovery:** Done 2026-07-23. Champion-bones
  fingerprints come only from a unique authenticated DB11 record, without
  DSA/UI/DB10 fallback. Verification:
  `csb_v1_csbwin_champion_bones_expool_recovery` passes.

- **DM1 F0828 original-save owner capture:** Done 2026-07-23. F0435 captures
  the raw C15/C25 owner and export rejects owner or priority drift. The absent
  local real PC34 corpus remains correctly skipped. Verification:
  `dm1_v1_original_save_pc34_handoff` passes.

- **DM1 F0369/F0370 spell-zone admission:** Done 2026-07-23. Spell clicks
  require C100, the original C101-C108 layout zone, and C009/C011/M653
  material; invalid input fails closed. Verification:
  `dm1_v1_f0369_spell_zone_admission_pc34_compat` passes.

- **Inventory F0421 correction:** Done 2026-07-23. The verified original-save
  staged byte-read and running-checksum contract is now marked implemented.
  Verification: `dm1_v1_original_save_pc34_handoff` passes.

- **CSBWin EDT_Character recovery:** Done 2026-07-23. Wing character
  name/title requires one complete, unique, authenticated PC34 bundle; no
  DSA, UI, or fallback path is used. Verification:
  `csb_v1_csbwin_wing_identity_expool_recovery` passes.

- **DM1 F0663 smoke material:** Done 2026-07-23. Smoke uses only raw
  C488/C498-C500 material and the original palette; missing or altered data
  produces no replacement effect. Verification:
  `dm1_v1_f0663_smoke_material_gate` passes.

- **Lane A — DM2 SkWinCore symbol audit batch 17 (cycle 16):** Done
  2026-07-23. Source-locked sixteen symbols: the last three in
  `SKULLWIN/c_0aaf.cpp` (`DM2_0aaf_0067` GDAT text-list builder,
  `DM2_0aaf_01db` dialogue background route, `DM2_0aaf_02f8` master dialog
  gates — narrow receipts, UI paths fail-closed) plus thirteen in
  `SKULLWIN/c_1c9a.cpp` (`DM2_19f0_13aa` teleporter-side scan,
  `DM2_19f0_1511`, `DM2_D283` teleporter detail probe,
  `DM2_CREATURE_GO_THERE` preamble narrow receipt, `DM2_19f0_2024`
  chest/creature item scan, `DM2_19f0_2165` creature action dispatcher,
  `DM2_19f0_266c`/`DM2_19f0_2723` admission chain, `DM2_19f0_2813` door
  interaction, `DM2_4DEA`, `DM2_1BA1B` door passability, `DM2_1c9a_0247`
  dballoc flush, `DM2_1c9a_0648` transition cache refresh). All helpers are
  receipted and fail-closed over caller-owned runtime state. DM2 skproject
  backlog dropped from 867 to 851 `MISSING` rows in
  `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`;
  `SYMBOL_DISPOSITIONS.tsv` gained sixteen VERIFIED_SOURCE_MAPPING rows.
  Verify with `./build/test_dm2_v1_skproject_core` (964 checks pass).
  Remaining: 851 `MISSING` skproject rows; batch 18 continues c_1c9a.cpp
  (queued as Lane A batch 18 under Active).

- **DM1 F0363 highlight release:** Done 2026-07-23. Release accepts only an
  active verified F0362 zone; stale or mismatched zones cannot draw pixels.
  Verification: `dm1_v1_command_highlight_box_disable_pc34_compat` passes.

- **DM1 raw C14 projectile admission:** Done 2026-07-23. Loaded projectile
  runtime requires raw C14 before F0218/F0219 dispatch. Verification:
  `dm1_v1_f0206_packed_directions_runtime_pc34_compat` passes.

- **CSB F0193 raw Giggler admission:** Done 2026-07-23. Giggler steals require
  the linked C04 receipt and fail closed on malformed or drifting data.
  Verification: `csb_v1_f0193_giggler_steal_receipt_pc34_compat` passes.

- **Inventory F0362/F0496 correction:** Done 2026-07-23. Existing
  source-bound DM1 highlight and CSB LZW paths are mapped, removing two more
  duplicate jobs. Verification: both focused tests pass.

- **CSBWin SubstituteGlobalText:** Done 2026-07-23. Substitution applies the
  original BCD transform to the authenticated GlobalText DB11 record, without
  DSA, UI, or fallback. Verification:
  `csb_v1_csbwin_global_text_substitution` passes.

- **DM1 F0826 continuation ownership:** Done 2026-07-23. Continuation events
  preserve their authenticated C15/C25 owner; broken links fail closed before
  a runtime effect. Verification:
  `dm1_v1_f0206_packed_directions_runtime_pc34_compat` passes.

- **DM1 F0354 portrait material:** Done 2026-07-23. Portrait rendering now
  requires matching raw PC34 portrait/C028 source material and fails closed
  on drift. Verification:
  `dm1_v1_f0354_portrait_material_pc34_compat` passes.

- **CSB F0191/F0267 pit-fall admission:** Done 2026-07-23. A group falls
  only after the linked C04 receipt remains valid; malformed or drifting data
  fails closed. Verification: `csb_v1_f0191_group_fall_receipt_pc34_compat`.

- **CSBWin EDT_GlobalText recovery:** Done 2026-07-23. Global text requires
  one authenticated DB11 record, valid NUL termination, and the source length
  bound; no DSA text fallback is used. Verification:
  `csb_v1_csbwin_global_text_expool_recovery` passes.

- **DM1 F0661 damage material:** Done 2026-07-23. The C014 damage effect now
  requires authenticated PC34/M653 material and the original palette.
  Verification: `dm1_v1_f0661_damage_material_gate` passes.

- **Inventory F0060/F0106 correction:** Done 2026-07-23. Existing source-bound
  CSB PSG decoding and DM1 CPSF reset paths are now mapped, removing two
  duplicate jobs. Verification: both focused tests pass.

- **CSBWin monster-kill statistic recovery:** Done 2026-07-23.
  `EDT_Statistics|ESTAT_NumMonsterKilled` now reads only a unique,
  authenticated DB11 record and fails closed without a statistic fallback.
  Verification: `csb_v1_csbwin_monster_kill_statistics_expool_recovery` passes.

- **DM1 F0346 resurrection-panel material:** Done 2026-07-23. The C040
  panel is sourced from authenticated GRAPHICS.DAT material and fails closed
  when absent or altered. Verification:
  `dm1_v1_f0346_resurrect_panel_material_pc34_compat` passes.

- **DM1 F0821 source-bound explosion publication:** Done 2026-07-23.
  Explosions publish only from complete raw C15/C25 ownership, with no
  synthetic runtime effect on incomplete data. Verification: F0190/F0206
  focused tests pass.

- **DM1 F0662 invisibility material:** Done 2026-07-23. C028/M653 and the
  original invisibility palette now gate the HUD icon; missing or altered
  material fails closed. Verification:
  `dm1_v1_f0662_invisibility_material_gate` passes.

- **CSB F0189 raw group deletion:** Done 2026-07-23. The runtime deletes an
  ActiveGroup only after its linked C04 receipt and identity still match;
  malformed or drifting source data does nothing. Verification:
  `csb_v1_f0189_group_delete_receipt_pc34_compat` passes.

- **DM1 F0347 raw C05 action-hand admission:** Done 2026-07-23. The object
  panel accepts a weapon action hand only when raw and decoded C05 fields
  agree. Missing, wrong-type, or drifting records publish no route.
  Verification: `dm1_v1_inventory_panel_action_hand_f0347_pc34_compat` passes.

- **CSBWin EDBT_AltMonGraphics recovery:** Done 2026-07-23. Code51a4's
  level/monster alternate-graphic lookup requires exactly one current,
  authenticated DB11 record and preserves source sentinels. Invalid records
  fail closed without derived graphics. Verification:
  `csb_v1_csbwin_alt_mon_graphics_expool_recovery` passes.

- **DM1 F0037 inventory correction:** Done 2026-07-23. The inventory now
  records the existing source-bound 16x16 transparent icon blit instead of
  scheduling duplicate work. Verification:
  `dm1_v1_object_draw_icon_to_screen_pc34_compat` passes.

- **CSB F0185 raw generated-group admission:** Done 2026-07-23. F0245 admits
  F0185 only from the exact linked PC34 C006 generator and one unused C04
  slot. Drift fails closed. Verification:
  `csb_v1_f0185_generated_group_receipt_pc34_compat` passes.

- **CSBWin EDBT_MonsterNames recovery:** Done 2026-07-23. The read-only
  Statistics.cpp route accepts exactly one active DB11 record for the requested
  monster/graphic variant; missing, duplicate, malformed, or stale records
  produce no fallback text. Verification:
  `csb_v1_csbwin_monster_names_expool_recovery` passes.

- **DM1 F0659 shield-border material receipt:** Done 2026-07-23. Raw
  C037/C038/C039/M653 surfaces now gate the ordered status-border overlay;
  missing or altered material omits it. Verification:
  `dm1_v1_f0659_shield_material_gate` passes.

- **DM1 F0336 raw weapon attributes:** Done 2026-07-23. CURSED/POISONED/
  BROKEN text is constructed only from an authenticated C05 weapon Thing;
  missing or drifting data produces no name or attributes. Verification:
  `inventory_item_identification_pc34_compat` passes.

- **DM1 F0220 C15/C25 live-owner admission:** Done 2026-07-23. A popped
  explosion proves its C15/C25 square-chain identity before mutation; runtime
  drift is a no-op. Verification:
  `dm1_v1_f0206_packed_directions_runtime_pc34_compat` passes.

- **CSB F0184/F0194 raw ActiveGroup retirement:** Done 2026-07-23. Before a
  party teleporter, stair, or pit level change, F0194 verifies every
  current-map F0184 C04 receipt, commits Cells/Direction/Behavior writeback,
  and retires the ActiveGroup pool as one transaction. Drift fails closed.
  Verification: `csb_v1_f0184_f0194_active_group_remove_pc34_compat` passes.

- **DM1 F0355 inventory material receipt:** Done 2026-07-23. The live C017
  inventory surface and C033 slot chrome require one authenticated
  GRAPHICS.DAT/M653 receipt before publishing; missing or altered material
  leaves the viewport untouched. Verification:
  `dm1_v1_f0355_inventory_material_gate` passes.

- **DM1 F0219/F0337 raw-data gates:** Done 2026-07-23. F0219 validates C14
  reindexering before C48/C49 rescheduling. F0337 accepts dungeon-light input
  only from raw C05 data. Both paths fail closed. Verification: F0206/F0337.

- **CSB restored DSA timer ownership:** Done 2026-07-23. Restored DSA timers
  require a unique loaded TimerQueue owner before DSA or EXPOOL dispatch;
  duplicate queues fail closed. Verification: DSA timer recovery tests.

- **CSB F0183/F0195 and DM1 F0338 receipts:** Done 2026-07-23. CSB now
  admits active groups from raw runtime data before F0195. DM1 periodic torch
  drain requires authenticated raw PC34 data. Both fail closed. Verification:
  F0195 and F0338 focused tests.

- **DM1 F0352/F0353 C503 eye/arrow material:** Done 2026-07-23. Eye and arrow
  panel rendering now uses the source-bound C503 receipt and rejects missing
  or drifted material. The regression consumes only an explicitly selected
  `FIRESTAFF_DM1_DATA_DIR`; a selected unreadable corpus is a failure.
  Verification: `dm1_v1_f0352_eye_material_gate`.

- **CSB `TT_ParameterMessage` timer ownership:** Done 2026-07-23. EXPOOL
  parameters are consumed only when a unique loaded timer-queue owner exists;
  unowned or duplicate entries fail closed. Verification:
  `csb_v1_dsa_parameter_message_save_handoff`.

- **CSB F0176/F0178 group-cell receipts:** Done 2026-07-23. Raw C04 cell
  receipts now bind F0178 to F0190 runtime compaction. Missing or drifting
  data fails closed. Verification: `csb_v1_f0176_f0178_group_cells_pc34_compat`.

- **DM1 F0351 stats-panel material:** Done 2026-07-23. Statistics panel
  rendering now requires raw C020/M653 material and fails closed on drift.
  Verification: `dm1_v1_f0351_stats_material_gate`.

- **DM1 F0333/F0334 chest record admission:** Done 2026-07-23. C09 Thing
  records now gate container links and slot writeback; invalid raw links are
  no-ops. Verification: `dm1_v1_chest_admission_f0333_f0334_pc34_compat`.

- **DM1 F0218/F0341 source gates:** Done 2026-07-23. F0218 now requires the
  exact raw C14 and C48/C49 owner before projectile handling. F0341 binds the
  scroll panel to C023/C101/C08 and M653 source material; both reject drift.
  Verification: F0206 and scroll-panel source-lock tests.

- **CSB DSA `FETCH` source receipt:** Done 2026-07-23. `DSACMD_FETCH` now
  follows CSBWin stack order and only consumes an authenticated loaded
  `DUNGEON.DAT` chain. Unknown or malformed data fails closed. Verification:
  `csb_v1_dsa_trigger_single_step_pc34_compat`.

- **CSB F0175 group-Thing receipt:** Done 2026-07-23. Raw C04/F0144 group
  identity now source-binds the F0217/F0219 teleporter and impact consumers.
  Missing or drifting records fail closed without M11, DSA or graphics paths.
  Verification: `csb_v1_f0175_group_thing_receipt_pc34_compat`.

- **DM1 F0342 object-panel material:** Done 2026-07-23. C020/C029/M653 now
  source-bind C101/C504/C506/C556 with the original 26x26 C029 crop.
  Material, font or geometry drift is fail-closed; no host font is used.
  Verification: `dm1_v1_f0342_object_description_material_gate`.

- **DM1 F0209 and CSB DSA continuation receipts:** Done 2026-07-23. DM1
  now ignores valid off-map C29-C41 events before C04 access. CSB `QUESTION`
  and `CASE` preserve the authenticated next state after branch/GOSUB. Both
  paths fail closed. Verification: F0206 and DSA timer/trigger regressions.

- **CSB F0163/F0164 object-move receipt:** Done 2026-07-23. Raw PC34 object
  move records now bind source allocation and ownership to the F0267 consumer;
  malformed or drifting chains fail closed. Verification:
  `csb_v1_f0163_f0164_object_move_receipt_pc34_compat`.

- **Lane A — DM2 SkWinCore symbol audit batch 16 (cycle 16):** Done
  2026-07-23. Source-locked sixteen symbols: eight in `SKULLWIN/c_1c9a.cpp`
  (`DM2_19f0_04bf`/`DM2_19f0_050f` cached record-chain walks,
  `DM2_19f0_0547` CAN_HANDLE_IT delegation, `DM2_19f0_0559` turn decision,
  `DM2_1c9a_0598` popcount, `DM2_19f0_0891` creature move decision with
  line-of-sight/hero-scan/action-pick and shadow-record commit,
  `DM2_19f0_05e8` target scan with 0891 delegation, `DM2_19f0_0d10`
  door-target move with 0891(0x84) delegation) and eight in
  `SKULLWIN/c_ai.cpp` (`DM2_14cd_2807`/`DM2_14cd_2886` oversee-record item
  handling, `DM2_PROCEED_XACT_56/57/59_76/62/63/64` creature AI behaviours).
  All helpers are receipted and fail-closed over caller-owned runtime state
  (s350/ddat words, party, record pools, commands). DM2 skproject backlog
  dropped from 883 to 867 `MISSING` rows in
  `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`;
  `SYMBOL_DISPOSITIONS.tsv` gained sixteen VERIFIED_SOURCE_MAPPING rows.
  Verify with `./build/test_dm2_v1_skproject_core` (923 checks pass).
  Remaining: 867 `MISSING` skproject rows; next batch covers c_0aaf.cpp plus
  the c_1c9a.cpp continuation (queued as Lane A batch 17 under Active).

- **DM1 F0200/F0329 source admissions:** Done 2026-07-23. F0200 sight now
  requires authenticated ACTIVE_GROUP directions for C29-C41; F0329 leader
  hand throws require raw F0156 data and active-leader identity. Both paths
  fail closed. Verification: `dm1_v1_group_visible_distance_pc34_compat` and
  `dm1_v1_leader_hand_throw_admission_f0329_pc34_compat`.

- **CSB F0167 raw new-object receipt:** Done 2026-07-23. Raw C03 creation
  now joins F0166 allocation and F0141 identity before runtime consumption;
  malformed or drifting records fail closed. Verification:
  `csb_v1_f0167_new_object_receipt_pc34_compat`.

- **DM1 F0231 source-bound reaction bridge:** Done 2026-07-23. C31 reaction
  scheduling now requires coherent raw C04, SquareFirstThing and ACTIVE_GROUP
  ownership before F0209 can produce C38/C39. F0230 XP, M11 and rendering are
  unchanged. Verification: `dm1_v1_f0206_packed_directions_runtime_pc34_compat`.

- **CSB F0158 WeaponInfo runtime receipt:** Done 2026-07-23. Throw/shoot
  admission now joins raw PC34 WeaponInfo data to F0141 object identity and
  fails closed on missing or drifted records. No M11, DSA or graphics path is
  used. Verification: `csb_v1_f0158_weapon_info_receipt_pc34_compat`.

- **DM1 F0229/F0230 melee target admission:** Done 2026-07-23. Raw PC34 C04
  plus matching C38-C41 owner, creature, active-group and map coordinates now
  gate the existing target/damage path. Cross-map or drifting state is a
  no-op; F0230/F0304 remain the mutable RNG owners. Verification:
  `dm1_v1_melee_target_admission_pc34_compat`.

- **CSB F0143/F0144 runtime receipts:** Done 2026-07-23. Raw PC34 armour
  Things now join the existing F0141 receipt to G0239 defense arithmetic,
  while raw C04 `GROUP.Type` joins G0243 CreatureInfo attributes. Both paths
  are fail-closed and expose no `GRAPHICS.DAT`, M11, or DSA route.

- **Lane A — DM2 SkWinCore symbol audit batch (cycle 15):** Done 2026-07-23.
  Source-locked the last four `SKULLWIN/c_querydb.cpp` `MISSING` symbols
  (`DM2_query_19f0_124b` stairs/pit transition query with open-pit, ladder,
  directionless-fall and stairs admissions plus target revalidation and map
  restore; `DM2_query_29ee_18eb` down/up transition pair over 19f0_124b;
  `DM2_IS_CREATURE_ALLOWED_ON_LEVEL` AI-flag 0x40 override and cls2 level
  allowance list; `DM2_query_0cee_319e` GDAT entry 9 data 11 by cls2) and the
  first four `SKULLWIN/c_1c9a.cpp` `MISSING` symbols (`DM2_1BAAD` tile
  passability predicate with door GDAT/randbit gate, wall-record chain and
  creature material/size rules through the cycle-14 DM2_query_1c9a_03cf
  wiring; `DM2_1BC29` transition cache wrapper; `DM2_19f0_0207` fixed-point
  line walk with cell callback; `DM2_19f0_045a` tile-state cache refresh).
  All helpers are receipted and fail-closed over caller-owned map, record,
  GDAT, spatial, and tile access. DM2 skproject backlog dropped from 891 to
  883 `MISSING` rows in
  `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`;
  `SYMBOL_DISPOSITIONS.tsv` gained the eight VERIFIED_SOURCE_MAPPING rows.
  `c_querydb.cpp` now has zero `MISSING` rows in the audit. Verify with
  `./build/test_dm2_v1_skproject_core` (883 checks pass). Remaining: 883
  `MISSING` skproject rows, continuing with the `SKULLWIN/c_1c9a.cpp`
  backlog; the caller-owned runtime record pools, map switching, and spatial
  index stay blocked until Firestaff owns the real DM2 runtime data.

- **CSBWin DSA condition/trigger receipts:** Done 2026-07-23. `AND`, `OR`,
  `NOT` and conditional trigger dispatch bind to the restored PC34 timer and
  condition identity. Unknown owners or receipt drift fail closed.

- **DM1 F0227/F0228 source-bound M10 LoS admission:** Done 2026-07-23.
  C29-C37 group-reaction dispatch now verifies the raw PC34 C04 record,
  active-group direction/map identity, C29-C41 timeline owner, party-map
  relation, and a preview of the original RNG result before it reaches the
  existing F0209 owner. C38-C41 retain their completed packed
  ACTIVE_GROUP-direction path. Missing or drifted source data is a no-op;
  the preview never consumes the live RNG. Verification:
  `dm1_v1_group_los_direction_admission_pc34_compat`.

- **CSB F0213-F0220 C15/F0115 fail-closed viewport consumption:** Done
  2026-07-23. A live C15/C25 can reach the F0115 explosion pass only through
  its configured source sprite consumer; missing authenticated material no
  longer creates a synthetic cross/marker.

- **DM1 F0212 launcher reservation/material admission:** Done 2026-07-23.
  The remaining composition gap is closed without changing the existing C14
  transaction or C14/C15 catalog: a launcher must now join its raw PC34
  associated Thing, F0142/G0209 object aspect, source-owned material handoff,
  reserved C14 row, and existing catalog receipt before runtime publication.
   Missing or drifted input is fail-closed and cannot create a marker.
   Verification: `dm1_v1_projectile_launcher_admission_f0212_pc34_compat`.

- **DM1 F0217 terminal C14/C15 materialization:** Done 2026-07-23.
  Terminal projectile disposal now consumes the authenticated F0215 and
  F0213-F0226 receipts before admitting either a dropped raw associated Thing
  or a published C15/C25 explosion. The C14 row, C15 reservation, original
  palette/pixels, raw associated object, and F0115 object material must agree.
  Missing or drifted PC34 source data is fail-closed. Verification:
  `dm1_v1_projectile_terminal_materialization_f0217_pc34_compat`.

- **CSBWin DSA MESSAGE/DESSAGE timer payload receipt:** Done 2026-07-23.
  Restored DSA receipts now bind the exact source delay and switch action in
  addition to route, target and event type. Unknown owners and all payload
  drift remain fail-closed.

- **DM1 F0221-F0226 source-bound damage aftermath:** Done 2026-07-23.
  Creature and champion impact resolution now requires the raw PC34 C14 row,
  matching C48/C49 event, decoder-owned C14 material, F0812-F0814 advance
  identity and F0213-F0215 termination. Live creature/champion apply packets
  and post-impact reaction/poison events are accepted only if they retain the
  same target owner. Explosion cases additionally consume the existing
  F0216-F0220 C15/C25 receipt. Missing or drifted data is fail-closed.
  Verification: `dm1_v1_projectile_damage_receipt_pc34_compat`.
- **CSBWin DSA SetNewState receipt binding:** Done 2026-07-23. Restored DSA
  receipts now bind the dispatcher-selected forced state and the existing
  PC34 local-state transition/tail fingerprint; drift fails closed.

- **DM1 F0216-F0220 impact/world receipt:** Done 2026-07-23. Post-collision
  dispatch now consumes the F0812-F0814 C14/C48/C49 receipt and F0213-F0215
  termination receipt, then requires a live raw C15/C25 publication with its
  original `GRAPHICS.DAT` pixels and palette before F0220 can advance or remove
  the effect. Stale C15/C25 rows, event fields, material, or result shape fail
  closed. Verification: `dm1_v1_projectile_impact_world_receipt_pc34_compat`.

- **CSB F0213-F0216 C14/C15/C25 terminal publication:** Done 2026-07-23.
  After F0810/F0811 has admitted a live C14/C49 collision, F0213 now queues
  C25 only when the allocated C15 retains the same projectile slot, source
  location, type and scheduled clock. A stale or recycled C15 is retired
  rather than publishing an aliased explosion event. F0215 materialization
  remains separately owned by its existing real `DUNGEON.DAT` Thing path.
  Verification: `csb_v1_f0248_c010_launcher_save_pc34_compat`,
  `csb_v1_f0248_c014c015_c25_ownership_pc34_compat`, and
  `csb_v1_f0810_original_c014c015_save_replay_pc34_compat`.

- **DM1 F0812/F0813/F0814 PC34 projectile advance:** Done 2026-07-23.
  A source receipt now admits F0811/F0825 advance only when the raw C14 row,
  original C48/C49 event plan, runtime slot and decoded PC34 material agree.
  Collision termination also requires the completed F0213/F0215 receipt and,
  for an explosion, an exact raw C15 plus original palette/pixels. Drifted
  event ownership, wrong collision class, or invalid reschedule fails closed.
  Verification: `dm1_v1_projectile_advance_source_pc34_compat`.

- **DM1 F0213/F0214/F0215 source-bound termination:** Done 2026-07-23.
  Projectile impact completion now joins the raw PC34 C14 projectile row, its
  exact C48/C49 move-event slot, the F0215 Thing disposition, and the existing
  C14/C15 `GRAPHICS.DAT` material receipts. Any raw/decoded drift, stale slot,
  missing palette/pixels, or absent associated Thing is a handled no-op.
  Verification: `dm1_v1_throw_shoot_pc34_compat`.

- **DM1 F0810/F0811 source-bound throw and replay:** Done 2026-07-23.
  F0328/F0810 now has a raw PC34 carried-Thing receipt, while original C48/C49
  replay verifies the byte-identical C14 record and fingerprints both C14 and
  its source event. Host-only objects and drifted save rows fail closed.
  Verification: `dm1_v1_throw_shoot_pc34_compat` and
  `dm1_v1_original_save_pc34_handoff`.

- **DM1 F0248/F0810 C14/C15 material provenance:** Done 2026-07-23. Live
  projectile and explosion material receipts now require an exact raw PC34
  `DUNGEON.DAT` C14/C15 row, matching decoded identity, decoder-owned
  `GRAPHICS.DAT` pixels and its original 16-colour palette. Missing or drifted
  inputs become no-draw and cannot produce a save receipt. Verification:
  `dm1_v1_f0115_source_material_handoff_pc34_compat`.

- **DM1 HoC C127/C026 source-click handoff:** Done 2026-07-23. Entrance
  selection now consumes the live C127/F0172 plus F0115 receipt and matching
  C026/C040 `GRAPHICS.DAT` material before opening a champion panel. The
  portrait atlas ordinal, source rectangle, destination rectangle and mirror
  slot must agree; absent or stale source receipts fail closed. Verification:
  `dm1_v1_entrance_c127_c026_source_click_handoff_pc34_compat` and
  `dm1_v1_hoc_mirror_candidate_click_admission_pc34_compat`.

- **DM1 TITLE/Entrance source handoff:** Done 2026-07-23. The DM1 startup
  receipt now binds the decoded `GRAPHICS.DAT` C001 title regions to the
  ReDMCSB `TITLE.C F0437` timing and C12/C13+C14 palette transition, then
  proves the `ENTRANCE.C F0436` palette boundary. When `TITLE.DAT` is present,
  it must pass the canonical PC34 manifest check as provenance; malformed
  installed data or incomplete C001 blocks the route in the M11 title startup
  consumer. No title/palette substitute is admitted. Verification:
  `title_frontend_c001_fallback_gate_pc34_compat` and
  `title_frontend_runtime_cadence_source_lock`.

- **CSB C002/C003 F0438 host-frame phase receipt:** Done 2026-07-23. The
  source-bound consumer validates the already-produced real `GRAPHICS.DAT`
  session/M11 host raster by opening step, tick, generation, palette, decoded
  door records and exact C004/C002/C003/F0128 source count. It never alters
  title or Entrance render-plan selection.

- **CSBWin DSA comparison/arithmetic receipt binding:** Done 2026-07-23.
  Restored-timer execution now carries the exact checksum-imported action
  words plus CSBWin comparison/arithmetic family markers. Any DSA body drift,
  malformed program, or changed accepted family invalidates the save/runtime
  receipt before it can be reused. Remaining DSA work requires broader real
  CSBWin save corpus coverage.

- **CSBWin DSA arithmetic/bitwise save receipt:** Done 2026-07-23. The
  existing source-shaped `STKOP` arithmetic path now publishes the exact
  post-`GLOBALSTORE` EXPOOL FNV identity when arithmetic changes the loaded
  global bank. The restored-timer receipt rejects missing variable ownership,
  stale tail identity, division by zero, and stack over-/underflow before
  publication. Verified with authentic PC34 EXPOOL fixture coverage for
  `Slash`, `Percent`, global persistence, and receipt drift.

- **CSBWin DSA variable/register and timer receipt binding:** Done 2026-07-23.
  Restored CSBWin timers now bind authenticated VARIABLE/GLOBAL family and
  MESSAGE timer-side-effect markers to the same save/runtime identity as the
  DSA words. Queue count, event type and target location are revalidated;
  malformed, unknown or stale operations fail closed. Remaining work requires
  broader original CSBWin save corpus evidence.

- **CSBWin DSA dungeon-mutation receipt binding:** Done 2026-07-23.
  Restored DSA timers now bind authenticated CELLFLAG/false-pit/teleporter
  mutation receipts and the post-action loaded `DUNGEON.DAT` fingerprint to
  the original save identity. Only current runtime owners may commit; malformed
  bytes, unknown effects and post-mutation dungeon drift fail closed.

- **CSBWin DSA call/return frame receipt binding:** Done 2026-07-23.
  Restored timers now bind source-authenticated direct JUMP/GOSUB `Execute()`
  frame counts, final state and missing-program return semantics to the loaded
  DSA/save identity. Any changed frame or malformed transfer fails closed.

- **CSBWin DSA Execute return/frame fault receipt:** Done 2026-07-23.
  The restored PC34 timer route now carries `DSA.cpp::Execute()`'s returned
  state as an explicit runtime result, rather than deriving it only from the
  transfer summary. Completed calls require balanced push/pop counts, the
  source missing-program return boundary, and the exact source return value.
  Altered return, stack-frame, or saved DSA/timer owner data rejects before
  reuse. `EX_GOSUB` still follows CSBWin and ignores its child return value.

- **CSBWin DSA message/display and sound receipt binding:** Done 2026-07-23.
  `MESSAGE`/`DESSAGE` and `DiscardText` retain source-decoded route and
  display values through the loaded DSA/save identity. `Sound` is explicitly
  rejected until an original CSBWin custom-sound backend is available.

- **CSBWin DSA champion/object operation admission:** Done 2026-07-23.
  Source-authenticated champion and object opcode families are classified;
  `TalentsStore` now binds party fingerprints and resulting talents to the
  restored DSA/save receipt. Unknown owners and fingerprint drift fail closed.

- **CSBWin DSA environment/query admission:** Done 2026-07-23. Read-only
  party/group/champion/square query opcodes are source-classified and their
  accepted runtime execution is bound to the existing DSA/save/dungeon receipt.

## Cycle 14 Completed (5 lanes — assembled and pushed)

Cycle 14 ran five parallel lanes. All lanes committed on their lane branches,
were merged to `main`, and the full parallel build plus lane tests pass.
Remaining work from each lane is carried forward in the sections below and
will feed into cycle 15.

- **Lane A — DM2 SkWinCore symbol audit batch (cycle 14):** Done.
  Source-locked the eight `SKULLWIN/c_querydb.cpp` query symbols
  `DM2_query_1c9a_03cf` (nearest-creature five-cell scan with AI-spec byte@0x23
  threshold over table1d62e0 and source table1d62b0/1d62d0 step rows),
  `DM2_query_48ae_01af` (bit 10/9 gated table1d2660 byte lookup),
  `DM2_query_0cee_2e35` (GDAT creature word 4 with zero-to-4 substitution),
  `DM2_QUERY_CREATURE_PICST` (narrow receipt: decodes creature/palette inputs,
  fails closed on the unowned blit path), `DM2_query_2fcf_164e` (recursive
  type-9 container search with cls2 < 8 gate and 0xfffe chain walk),
  `DM2_query_2fcf_16ff` (party possession search over hero inventories, hand
  containers when ddat.v1d67bc == 5, and savegamewpc.w_00),
  `DM2_query_48ae_0767` (weight packing that repacks the same item while it
  fits and skips heavier ones), and `DM2_query_0cee_06dc` (adjacent-tile
  predicate returning bit or 2 + bit for neighbour types 0/3). Added
  `DM2_V1_SkprojectCreatureAISpec.word34` (byte offset 0x22) for the byte@0x23
  read. Runtime spatial index, record pools, GDAT tables, and data-segment
  tables stay caller-owned via callbacks; all helpers are receipted and
  fail-closed on missing data or out-of-bounds table indices. DM2 skproject
  backlog dropped from 899 to 891 `MISSING` rows in
  `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`;
  `SYMBOL_DISPOSITIONS.tsv` gained the eight VERIFIED_SOURCE_MAPPING rows.
  Verify with `./build/test_dm2_v1_skproject_core`. Remaining: bind the
  caller-owned callbacks to real runtime record pools/GDAT tables once those
  are proven; continue the c_querydb `MISSING` backlog next cycle.

- **Lane B — DM2-008 real GDAT sound backend (cycle 14):** Done.
  `src/dm2/dm2_v1_sound.c` now implements the source-locked
  `DM2_PLAY_MUSIC`, `DM2_PLAY_SOUND`, and `DM2_QUERY_SND_ENTRY_INDEX` paths
  against verified `GRAPHICS.DAT` audio raw entries.  A verified GDAT loader
  can be bound via `dm2_v1_sound_bind_gdat_loader()`; `DM2_SOUND9`
  (`dm2_v1_sound9()`) populates the `dm2sound.xsndptr2` seven-byte runtime
  queue and resolves sample bindings from GDAT raw entries (class bytes
  widened through `uint8_t`, categories run to 0xF0);
  `DM2_QUERY_SND_ENTRY_INDEX` keeps the original 1-based linear scan and a
  GDAT fallback that materialises the queue entry in original queue/query
  order.  Unavailable audio is explicit: playback/attenuation is never
  synthesised, and `dm2_v1_startup_menu.c` records the title cue as
  fire-and-forget dispatch instead of synthetic playback success.  All
  targets compiling `dm2_v1_sound.c` now also compile
  `dm2_v1_sound_queue_pc34_compat.c`.  Tests: new
  `test_dm2_v1_sound_gdat_real_data` (real-data verified against local
  GRAPHICS.DAT sound entry 3/0/129; skips cleanly without data), updated
  `test_dm2_v1_sound_source_gate`, existing
  `test_dm2_v1_sound_queue_pc34_compat` — all PASS; full parallel build
  clean; `firestaff_dm2_v1_creature_combat_probe` 158/158 (its synthetic
  playback-success assertions updated to the fail-closed contract).
  Commits `cedb01475` + `eb229d50f` on `cycle14-lane-B`.
  Remaining: proven SDL/sample backend + verified music asset root before
  audible playback can leave the fail-closed state; pre-existing unrelated
  dm2_v1 real-data gate failures (boot_profile_smoke, dungeon_loader gates,
  g1 gates, dm2_v1_asset, startup_audio_menu title-gate) verified identical
  on the pristine base tree.

- **Lane C — DM2-010 DRAW_ITEM and creature/cloud passes (cycle 14):** Done.
  Source-locked the DRAW_ITEM placement chain in
  `src/dm2/dm2_v1_viewport_renderer.c` against skproject
  SKWIN/SkWinCore.cpp (`DRAW_ITEM` _32cb_3672, `DRAW_PUT_DOWN_ITEM`
  _32cb_3991, `DRAW_STATIC_OBJECT` _32cb_3b9d, `QUERY_OBJECT_5x5_POS`
  _48ae_07fd, `DIR_FROM_5x5_POS` _48ae_07bf) and the SkGlobal.cpp tables
  (_4976_4a04, _4976_41b0, _4976_41de, _4976_418e, tlbDisplayOrder*):
  - New helpers `dm2_v1_viewport_object_5x5_pos`,
    `dm2_v1_viewport_static_object_visibility_bit`,
    `dm2_v1_viewport_dir_from_5x5_pos`,
    `dm2_v1_viewport_static_object_display_order` and
    `dm2_v1_viewport_static_object_draw_positions` (source display order
    filtered by the per-cell 5x5 visibility mask).
  - `dm2_v1_viewport_static_object_source_plan` now takes the party view
    direction and rotates the record anchor into view space
    (`(object_direction - view_dir) & 3`), matching
    `QUERY_OBJECT_5x5_POS(rl, _4976_5aa0)`; clip rect, stretch and chest
    mirror follow the rotated anchor.
  - The zero-visibility-mask block is unblocked with source evidence:
    `dm2_runtime_static_object_visibility_mask_5x5()` in
    `src/dm2/dm2_v1_runtime.c` ORs `1 << QUERY_OBJECT_5x5_POS(record, view)`
    over the declared direct G1 DB5/DB9 roots of each square
    (SkWinCore.cpp:45361-45370), so the M11 static-object delivery plan
    passes its mask gate with record-owned data.
  - `dm2_v1_viewport_build_item_render_plan` fills the new
    `source_static_object_placement_*` fields on `DM2_V1_ItemRender` for
    admitted DB5/DB9 static objects with no Rect14 row (clip-rect
    cross-check keeps stale rows fail-closed), and
    `dm2_v1_viewport_item_asset_blit` applies the source stretch factor
    (CALC_STRETCHED_SIZE), the _4976_41b0/_4976_41de slot deltas and the
    chest mirror; Rect14 placement keeps priority.
  - New tests `tests/test_dm2_v1_draw_item_source_placement.c` (78/78) and
    real-data `tests/test_dm2_v1_g1_static_object_visibility_real_data.c`
    (36/36 against the canonical PC G1 corpus); new probes
    `probes/dm2/firestaff_dm2_v1_draw_item_source_probe.c` (10/10) and
    `probes/dm2/firestaff_dm2_v1_draw_item_source_pass_probe.c` (135/0
    across all 28 G1 maps). Updated call sites in
    `test_dm2_v1_static_object_m11_delivery_plan`,
    `test_dm2_v1_g1_weapon_viewport_material_gate` (9/9) and
    `test_dm2_v1_viewport_door_state_side_cells` (23/23).
  - Verify: `ctest --test-dir build -R "dm2_v1_(viewport|item|creature|cloud|
    projectile|flying|g1_weapon|g1_container|gdat|weather|scene)"` 95/96;
    the single failure (`dm2_v1_creature_combat_probe`, sound queue) is
    pre-existing on the cycle-14 baseline (Lane B DM2-008 scope).
  Remaining: actual static-object pixel draw still waits for the
  dtImageOffset + expanded-clip receipt and the per-square chain slot
  ordinals (delivery plan stays `no_draw`); side/deep cells outside 3/6
  remain fail-closed; creature/cloud passes keep their existing map-chip
  routes until the _4976_5aa4 occupancy grid and DRAW_FLYING_ITEM material
  are source-owned.

- **Lane D — Nexus V2 HUD gameplay integration (cycle 15):** Done.
  The V2 HUD overlay (`nexus_v2_hud_overlay.c`) is now linked into the main
  library and wired into the Nexus DGN gameplay render path in
  `m11_game_view.c`.  `m11_draw_nexus_dgn_host_plan()` feeds party direction
  and current level from the engine state and renders the HUD overlay after
  the viewport framebuffer copy.  HUD init runs on both launcher and resume
  engine-assignment paths.  All 201 Nexus tests pass (3 pre-existing known
  failures unchanged).  2026-07-28: V2 lighting runtime and smooth movement runtime are now
  wired into the Nexus engine lifecycle (init, tick at 60 Hz, shutdown)
  alongside the HUD. Base V2 modules (lighting, particles, atmosphere,
  smooth movement) are linked into the main library. All 21 Nexus V2
  tests pass.
  2026-07-28: the gameplay render path shows a diagnostic overlay
  ("DGN VIEWPORT MATERIAL ROUTE BLOCKED") with level, position, direction,
  MNS route validity, and selector binding status when the viewport cannot
  render. Previously the blocked case showed a black screen with no feedback.
  Remaining: wire gold/champion bars from real game state once
  combat/inventory are source-owned; connect V2 phase gate config instead
  of force_active; unblock viewport material gates (requires Structure1B
  selector binding research).

- **Lane D — Nexus V1 real-data creature spawn and combat (cycle 14):** Done.
  Nexus creatures now spawn from authenticated `LEV*.DGN` Structure1A actor
  records (kind byte 01h/02h with a unique Structure1B owner cell) instead of
  synthetic probe fixtures.  Kind 01h maps to the Structure1B
  invisible-by-default bit (DMWeb: the Grey Lord on LEV1.DGN) and spawns
  hidden/dormant; kind 02h spawns a visible patrol actor; facing comes from
  the Structure1A Z-rotation byte.  `nexus_v1_mechanics_load_level()` resets
  the active creature pool (ReDMCSB GROUP.C F0183 per-map pool) and spawns
  all unique-owner actor records; `real_actor_spawn_count > 0` blocks
  synthetic spawn fixtures.  Actor stats stay fail-closed: each actor keeps
  its Structure3 model index plus an FNV-1a64 mesh signature over the typed
  mesh rows of the exact authenticated DGN buffer and resolves a roster type
  only through the evidence-gated binding registry
  (`nexus_v1_creature_bind_actor_model`, `nexus_v1_creature_actor_type_for`,
  `nexus_v1_creature_rebind_unbound`); unbound/hidden actors cannot move,
  attack, be targeted, or be alerted.  Roster types bind to real `*.MNS`
  model metadata (DMDF magic + size + FNV-1a64) from the materialized English
  retail extraction (30/30 documented roster MNS files).  The playability
  probe verifies per-level real
  spawn counts/provenance/hidden split, runs real-data melee/death/XP/drop
  combat against real-spawned actors through the INTERACT path
  (deterministic srand per engagement, unkillable probe leader, bounded
  KILLMON.C drop re-roll), and keeps the synthetic fixture only on levels
  with zero real actor records.  The parity probe gained fail-closed
  actor-spawn/binding/reset coverage, and the creature-state determinism
  probe reseeds rand() per repetition (pre-existing wander-AI flake fixed).
  Verification: `firestaff_nexus_v1_mechanics_playability_probe` → 529/0
  PASS/FAIL, `firestaff_nexus_v1_mechanics_parity_probe` → 301/0,
  `test_nexus_v1_dgn_actor_slot_bounds` → PASS,
  `firestaff_nexus_v1_creature_state_determinism_probe` → 9/0; full parallel
  build green.  Remaining: Structure3 actor-model → named-creature identity
  evidence (local data cannot yet name each model; probe-scoped bindings
  only), hidden-actor reveal trigger semantics, per-type real drop tables,
  and creature generator/sensor spawn records if original evidence locates
  them.

- **Lane E — Theron V1 multi-level object-tail and dungeon progression (cycle
  14):** Done. The Track 02 compact object-table decoder now accepts
  multi-level object-tail semantics: `theron_v1_track02_read_object_table()`
  bounds records against the full 32×32 TQR map envelope and level indices
  0..`THERON_MAX_LEVELS_PER_DUNGEON`-1 (was level-0/32×27 only), and
  `theron_v1_track02_decode_initial_level_object_table()` accepts records for
  any level of the starting dungeon.  JP/US raw Track 02 BINs still decode to
  an empty table (count 0, all-zero tail), which remains the only source-proven
  real-media shape.  Added
  `theron_v1_track02_decode_dungeon_level_object_table()` to extract one
  level's records from an authenticated dungeon route's object transaction
  (fail-closed: non-OK/invalid routes return NOT_FOUND; no non-startup route
  is promoted from real media yet).  World binding: new object kinds
  `THERON_OBJTYPE_SOUND`/`THERON_OBJTYPE_PIT`, pit records own their grid
  tile, sound records carry the sound id for the movement code, and
  `theron_v1_world_apply_track02_object_table_for_dungeon()` routes records
  to every loaded level of a dungeon.  `theron_v1_transition_execute()` now
  implements stairs (validated target level + spawn fallback to the target
  level's start pose), teleporter commit, and between-dungeon exit
  (progression advance, `theron_v1_world_reset_for_dungeon()`, quest-complete
  when the dungeon sequence ends); sound-trigger objects play on tile entry in
  `move_party_internal()`.  Coverage: `test_theron_v1_combat_mechanics`
  gained multi-level apply, stairs transition, between-dungeon exit, and
  per-level route decode tests (65/65 PASS);
  `firestaff_theron_v1_mechanics_playability_probe` gained real-grid
  multi-level apply and stairs-transition smoke tests (79/79 PASS, 0 SKIP on
  staged TQUS02.bin + TQJP02.bin); `test_theron_v1_startup_save_resume_pc34`
  was adapted to the validated stairs transition.  Verification:
  `ctest --test-dir build -R theron_v1_ -j4 --output-on-failure` → 161/161
  PASS.  Remaining: real Track 02 evidence for a non-startup level/object
  handoff (route constructors stay OBJECT_REJECTED until then), teleporter
  target resolution from the object DB, and binding decoded items/creatures
  once their record kinds are source-locked.

## Cycle 13 Completed (5 lanes — pushed)

Cycle 13 ran five parallel lanes. All lanes committed, the full parallel build
passes, lane tests pass, and the aggregate was pushed to `origin/main`.
Remaining work from each lane is carried forward in the sections below and will
feed into cycle 14.

- **Lane A — DM2 SkWinCore symbol audit batch (cycle 13):** Done.

- **Lane B — DM2-007 real-data spell handlers (cycle 13):** Done.

- **Lane C — DM2-010 DRAW_ITEM and door-state expansion (cycle 13):** Done.
  Source-locked side/deep static-object cell ordering in
  `dm2_v1_viewport_static_object_cell_for_map()` using the skproject
  `table1d7029` pass layout; downstream `DRAW_ITEM` placement stays blocked
  until the visibility mask, record ordinal, and Rect14 tables are recovered.
  Added `dm2_v1_viewport_door_open_pct_from_state()` to derive door panel
  visibility from the source state table while honouring explicit animation
  percentages.  Added `door_wall_button_state` to `DM2_ViewSquare` and updated
  `dm2_v1_viewport_build_door_render_plan()` to select the pushed wall-button
  field variant.  Added `tests/test_dm2_v1_viewport_door_state_side_cells.c`
  and registered it in `CMakeLists.txt`.  Fallback rectangles/colours remain
  blocked when source material is unavailable.  Verify with
  `./build/firestaff_dm2_v1_*` probes and relevant `test_dm2_v1_*` CTests.
  Remaining: full `DRAW_ITEM` clipping/placement once GDAT Rect14/material is
  bound; creature/object/cloud passes; scale/flip rules.

- **Lane D — Nexus V1 altar/AI/sounds/door animation (cycle 13):** Done.
  Added door open/close animation stepping (`NEXUS_DOOR_STATE_OPENING/CLOSING`,
  `nexus_doors_tick_animation()`, passability threshold) and wired it into
  movement blocking via `nexus_doors_is_passable()`. Added a candidate altar
  registry from tagged Structure1F floor-decoration records
  (`nexus_altars_register_tagged`, `nexus_altar_tag_at`,
  `nexus_altar_perform_ritual`) with ritual effect fail-closed until semantics
  are confirmed. Added creature AI wander behaviour for patrol-state creatures.
  Replaced the generic SFX stub with source-locked MAP dispatch
  (`nexus_sound_set_event_selector`, event→selector lookup, SAL window
  profiling, diagnostic recording when decode is unsupported). Updated the
  focused parity and playability probes. Source-lock: ReDMCSB CHAMPION.C,
  CREATURE.C, COMMAND.C, MOVESENS.C, and the DMWeb DGN format. Verification:
  `./build/firestaff_nexus_v1_mechanics_parity_probe` → 285/0 PASS/FAIL;
  `./build/firestaff_nexus_v1_mechanics_playability_probe` → 413/0 PASS/FAIL.

- **Lane E — Theron V1 real Track 02 object decode and mechanics (cycle 13):**
  Done. The `0x380`-byte tail after the 32×27 Hall-of-Records level envelope is
  now parsed as a little-endian count-prefixed compact object table; both JP/US
  raw Track 02 BINs decode to an empty table (count 0, all-zero tail), which is
  accepted as source-proven. `theron_v1_track02_decode_initial_level_object_table()`
  returns `OK` for verified media and populates `out_receipt->object_table`.
  `theron_v1_world_apply_track02_object_table()` binds decoded records into the
  live world: doors/teleporters set both the object database and grid tile,
  teleporter/trigger arguments become `linked_id`, and items/altar/pool/alarm
  objects are placed on their existing tile. Door, pit, teleporter, altar, and
  sound mechanics are wired through `theron_v1_mechanics.c` and were exercised
  with synthetic fixtures. `firestaff_theron_v1_mechanics_playability_probe` now
  verifies the decoder on real JP/US Track 02 and applies the empty object table;
  `tests/test_theron_v1_combat_mechanics.c` adds regression coverage for object-
  table application, door open/move, pit fall, teleporter chain, altar-of-vi
  resurrection, and sound IDs. Verification: `ctest -R theron_v1_` → 161/161 PASS;
  `./build/firestaff_theron_v1_mechanics_playability_probe` → 65/0/0 PASS/SKIP/FAIL
  on authentic JP/US Track 02 BINs; `./build/test_theron_v1_combat_mechanics` →
  46/0 PASS/FAIL. Remaining work: non-startup dungeon object tables and multi-
  level object-tail semantics remain blocked until additional original loader
  evidence identifies them; pit squares are still grid-driven (no object-table
  pit record kind is proven); sound-trigger object records remain unbound.

## Cycle 12 Completed (5 lanes — pushed)

Cycle 12 ran five parallel lanes against this TODO section. All lanes committed,
the build was fixed (CMakeLists.txt + obsolete Theron probe stubs), lane tests
pass, and the aggregate was pushed to `origin/main`. Remaining work from each
lane is carried forward in the sections below.

- **Lane A — DM2 SkWinCore symbol audit batch (cycle 12):** Done.
  Implemented the eight source-locked helpers for `DM2_query_32cb_0804` (line
  2431), `DM2_query_0b36_037e` (line 2477), `DM2_query_1c9a_08bd` (line 2674),
  `DM2_IS_CREATURE_FLOATING` (line 2699), `DM2_IS_OBJECT_FLOATING` (line 2718),
  `DM2_QUERY_OBJECT_5x5_POS` (line 2738), `DM2_query_48ae_05ae` (line 2801),
  and `DM2_query_4E26` (line 2936) in `src/dm2/dm2_v1_skproject_core.c`, with
  declarations/receipt structs in `include/dm2_v1_skproject_core.h` and focused
  regression tests in `tests/test_dm2_v1_skproject_core.c`. Updated
  `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` (rows
  813/814/817-822) and `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv`. Verified
  with `./build/test_dm2_v1_skproject_core`: all DM2 skproject core helper
  checks passed.

- **Lane B — DM2-007 spell system completion (cycle 12):** Done.
  Implemented the remaining DM2-007 runtime spell path. Added bounded
  `0x19` cloud, `0x1e` missile/projectile, and `0x5e` summon timer handlers in
  `src/dm2/dm2_v1_spell_timer_handlers_pc34_compat.c`; the cloud and summon
  handlers record the request and fail closed on real DB14/DB4 record mutation,
  while the missile handler maps `DM2_OBJECT_EFFECT_*` to a proven DM2
  projectile subtype and dispatches through
  `dm2_v1_projectile_dispatch_synthetic`. Wired all spell-effect timer handlers
  into `src/dm2/dm2_v1_runtime.c` via `dm2_runtime_spell_timer_wrapper()`, which
  forwards spell timers to the runtime spell-handler context while preserving
  the runtime state context for door/actuator/weather handlers. Added M11 DM2
  status-scope accessors (`dm2_v1_runtime_status_scope`,
  `dm2_v1_runtime_status_message`,
  `dm2_v1_runtime_last_spell_failure_class`) and the note function
  `dm2_v1_runtime_note_spell_cast_apply_receipt()`. Extended
  `tests/test_dm2_v1_spell_cast_player_pc34_compat.c` with focused checks for
  cloud fail-closed behaviour, fireball projectile instantiation, unknown-effect
  rejection, and summon fail-closed behaviour. Verified with
  `ctest -R 'dm2_v1_spell|dm2_v1_proceed_timers'` (4/4 lane CTests pass) and
  `./build/test_dm2_v1_runtime_handoff_smoke` (176/176 checks pass).

- **Lane C — DM2-010 viewport renderer expansion (cycle 12):** Partial.
  Source-locked the remaining `DRAW_STATIC_OBJECT -> DRAW_PUT_DOWN_ITEM ->
  DRAW_ITEM` prerequisites by adding the source-owned 5x5 visibility mask and
  record-list ordinal to `DM2_V1_StaticObjectSourcePlan` and the M11 delivery
  plan. `dm2_v1_viewport_build_static_object_m11_delivery_plan()` now fails
  closed when the ordinal is zero, the mask is zero, or the object's position
  bit is absent; both values are folded into the identity hash. The runtime
  enumerator passes `i+1` as the ordinal and a zero mask, so the route stays
  blocked until the visibility table is bound by future source work.
  Tests updated/verified: `test_dm2_v1_static_object_m11_delivery_plan` passes
  and exercises the new gating; `test_dm2_v1_g1_weapon_viewport_material_gate`
  passes with the new signature. Remaining: complete source cell ordering for
  side/deep static objects, `DRAW_ITEM` clipping/placement expansion, door
  states beyond closed-panel/button placement, object/creature/cloud passes,
  scale/flip rules, and verified GDAT material. Verify with
  `./build/firestaff_dm2_v1_*` probes and relevant `test_dm2_v1_*` CTests.

- **Lane D — Nexus V1 real-data gameplay mechanics (cycle 12):** Source-lock
  against ReDMCSB DUNGEON.C, COMMAND.C, MOVESENS.C, CHAMPION.C, and the DMWeb
  DGN format. DONE this cycle: added pit/altar/door registries
  (`nexus_pits_*`, `nexus_altars_*`, `nexus_doors_count`) in
  `src/nexus/nexus_v1_squares.c` and updated `nexus_process_square_event()` so
  `NEXUS_SQUARE_CHUTE` resolves a registered pit target. Expanded floor-item
  storage to the real 64×64 grid in `src/nexus/nexus_v1_inventory.c` and added
  `nexus_gold_remove()` in `src/nexus/nexus_v1_drops.c`. Added
  `nexus_v1_mechanics_load_level()` in `src/nexus/nexus_v1_mechanics.c`; it
  repopulates door/teleporter/stair/pit/altar/floor-item/gold registries from
  authenticated DGN Structure1F records, maps item IDs through `ITEM.IBS` when
  available, and blocks synthetic fallbacks when real records are present. Wired
  it into `nexus_v1_load_level()` in `src/nexus/nexus_v1_engine.c`. Updated
  `nexus_mechanics_tick()` so `NEXUS_CMD_INTERACT` attacks an adjacent creature
  when no floor item is present; creature death awards fighter XP and rolls
  drops/gold onto the floor; gold pickup removes the pile. Updated
  `firestaff_nexus_v1_mechanics_playability_probe` to exercise real-data
  `nexus_v1_mechanics_load_level()` binding and run a synthetic combat/drops
  smoke test. Fixed a compile blocker in
  `src/dm2/dm2_v1_spell_timer_handlers_pc34_compat.c`. Verification:
  `firestaff_nexus_v1_mechanics_playability_probe` → 365 PASS, 0 FAIL;
  `test_nexus_v1_dgn_multi_level_playability` → 64 PASS, 0 FAIL;
  `firestaff_nexus_v1_mechanics_parity_probe` → 251 PASS, 0 FAIL. Remaining:
  altar semantics are fail-closed until Structure1F altar records and ritual
  logic are confirmed; no local real Track 1 creature-spawn records exist, so
  live combat is verified only via the synthetic probe fixture; sounds, creature
  AI, and door-state animation are not yet bound. Verify with
  `./build/firestaff_nexus_v1_mechanics_playability_probe` and
  `./build/test_nexus_v1_dgn_multi_level_playability`.

- **Lane E — Theron V1 real Track 02 object/mechanics pipeline (cycle 12):** Done.
  Implemented source-locked creature spawn/combat/drop/AI mechanics and a
  fail-closed real Track 02 object-table decoder. Real Track 02 object-tail
  semantics remain blocked; the decoder records the proven byte boundary and
  returns NOT_FOUND. Verified `ctest -R theron_v1_` → 161/161 PASS.

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
  (`BUG0_86`).** Mirror/resurrection/rename graphics are conditionally loaded
  according to legacy memory limits; the comment records known missing/garbage
  graphics on PC in custom-dungeon conditions. **Firestaff risk:** this
  platform-memory workaround is not a canonical HoC visual contract and must
  not decide modern host rendering. **Required independent evidence:** PC34
  `GRAPHICS.DAT`/`DUNGEON.DAT` pair and original-PC/DOS capture of HoC,
  resurrection, rename, and a controlled custom-dungeon memory case.
  **2026-07-13 implementation boundary:** DM1 now owns F0096 wall-set
  material receipts for the remaining M11 D1C/D2C/D3C wall blits; host code
  rejects mismatched graphics rather than scaling a substitute. Real-PC34 HoC
  coverage proves C093/C098/C103 material plus C127/C026. This does not close
  the documented platform-memory gap: an original-PC custom-dungeon capture is
  still required for the legacy loading workaround. **2026-07-13 F0115/F0128
  boundary:** D3/D2/D1 side-wall lane clipping, parity, C10 key and F0096
  material are now a single DM1 receipt consumed mechanically by M11; D4 has
  an explicit no-receipt result because F0115 exits at depth > 3. The missing
  original-PC custom-dungeon capture remains the blocker for the legacy
  memory-limit behavior.

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
  - 2026-07-14 update: the complete static HUD family now follows the same
    boot-owned command-plan route as the validated `UPDATE_GFXSET` scene
    planes. `INTERFACE_GENERAL` chrome and source-bound `CHAMPIONS` portraits
    carry their own decoded IMG3 pixels, local palettes, and exact M11
    destinations into `c_gui_vp` consumption; a missing command remains
    blocked rather than asking the provider for substitute art. This does not
    infer any G1 DB semantics, HUD font placement, or dynamic-stat clipping.

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
  now retain their source queue identity through the live timeline boundary.
  Parameter messages stay on their authenticated payload path; malformed
  queue/timer identity, absent type-47 records, wider timer families, timer
  cell effects, master-state writes, and world/filter opcodes remain blocked.
  TT_TELEPORTER now retains the exact DSA-free empty-Thing-chain cell update
  only when the party is elsewhere, so its source WiggleEverything call is a
  no-op; listed-Thing and DSA-owned targets remain mutation-blocked.
  TT_PITROOM retains the same DSA-free empty-Thing-chain cell update only
  when the party is elsewhere; listed-Thing and DSA-owned targets remain
  mutation-blocked. TT_OPENROOM now retains only a single DSA-free DB2 text
  visibility update outside the party square; DSA/mixed lists and the HUD
  print route remain mutation-blocked. TT_STONEROOM now retains only a sole
  DSA-free, position-matched DB2 text visibility update; DSA/mixed lists,
  actuator arms, and endgame remain mutation-blocked. TT_DOOR now retains
  the exact same-time TT_1 handoff only when its saved square has no type-47
  owner;
  a single type-47 owner now retains its same-TIMER TT_1 handoff only after an
  authenticated pure-stack receipt succeeds. Multiple/failed actions and all
  world-mutating DSA paths remain blocked. TT_DESSAGE reaches only its
  zero-parameter type-47 OPENROOM receipt; text/cell/other-actuator effects
  remain blocked.
  TT_ParameterMessage now reaches its authenticated EXPOOL payload route;
  missing/altered payloads and non-DSA source effects remain blocked.
  TT_BASH_DOOR now has its own exact saved queue receipt, so it cannot fall
  through the shared function-2 destruction handler; non-door targets and
  malformed saved identity remain blocked.
  TT_11 now restores only the exact no-rearm, non-SHOOT champion action lock;
  ammunition/quiver branches and malformed saved identity remain blocked.
  TT_12 now clears only its exact saved hide-damage receipt; its source
  inventory/status-panel redraw branches remain blocked until their live
  inventory-champion identity is restored.
  The opt-in real-package DSA probe now snapshots every decoded action and
  source selector before one runtime tick, then requires the entire catalog to
  remain save-owned after the tick. It still skips without an explicit original
  package and does not create a substitute save, DSA, or timer fixture.

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
  layout, raw zero-separated byte/block spans, payload composition, and
  Structure1A owner/model and transform selectors plus Structure1F raw face,
  rotation, face/rotation, signed-offset pair, and wall-payload-selector
  receipts plus raw wall-sensor destination tuples/control selectors and alcove
  payload selectors plus direct floor-sensor control/destination and floor-
  decoration payload/rotation receipts now reach the optional retail DGN corpus. The
  receipt can separately rule out zero- and one-based direct model-index
  byte/block/run ordinals. The verified retail corpus also has a bounded,
  strictly increasing Structure3 offset directory and a fixed 40-byte entry
  header with three count-bounded 12-byte regions. The tracked DMWeb Saturn
  reference and verified `LEV00.DGN`--`LEV15.DGN` corpus now establish
  entry-local vertex-index bounds, triangle/quad topology, paired normal
  counts, and static/animated/one-off face-fill lanes. The real corpus now
  also proves every texture-flagged `00xx` face-fill selector resolves to a
  bounded Structure2 descriptor and every `08xx` selector resolves to a
  Structure1G animated declaration. Next: prove Saturn payload/palette decode
  and VDP1 draw ordering before any draw route; do not infer texture pixels,
  transforms, clipping, triggers, or fallback art. 2026-07-14 update:
  Structure3a/Structure3c signed 16.16 vector framing and the rounded
  unit-normal invariant now hold across the retail LEV00--LEV15 corpus, but
  this remains read-only geometry provenance and does not expose a mesh or
  relax the no-draw barrier. 2026-07-14 update: the active engine now exposes
  a caller-buffered Structure3 entry route only after the loaded canonical LEV
  hash, level identity, and owned source bytes still agree. It supplies the
  typed signed 16.16 vertices, face rows, and paired normals to the renderer
  boundary and rejects stale or mutated data without a partial entry. This is
  still explicitly no-draw: texture/palette, transforms, VDP1 state, and draw
  semantics remain unproved. The documented entry-local
  face-row/normal-row ordinal pairing is now retained and corpus-checked for
  1,144 entries / 18,478 pairs. Next remains original Saturn evidence for
  normal-plane use, transforms, palette/texture decoding, and VDP1 draw
  ordering; do not infer any of those from the pair receipt. 2026-07-14
  update: the renderer now receives an aggregate mesh-semantic receipt when
  bounded topology, vectors, and ordinal pairs agree, but it explicitly
  requires an unavailable original Saturn capture and continues to block
  normal-plane, transform, texture/palette, and draw behavior.
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
`LANGUAGE`, or `LANG`, and the flag popup commits mouse selection through the
same PO/l10n path as keyboard input. Indonesian is the twentieth Latin-script locale and
uses the normal Noto Sans fallback. 2026-07-19 (Jobb G, w5): the 13
fallback-only `startup-menu.*.po` catalogs (cs, da, es, fi, hu, it, ko, nl,
no, pl, pt, ru, tr) are now natively translated (59 strings each);
`po/validate_po_layout.sh` reports 74-87% native coverage per catalog and no
startup-menu FALL entries remain. Remaining localization work: native
translation passes for the `csb.*.po` and `theron.*.po` fallback-only
catalogs (owned by other jobs), and review of the older machine-translated
startup-menu catalogs (e.g. de has "CHEA TS" / "DURCHSTECHEND" artifacts).
Do not claim a locale is translated merely because it falls back to English.
2026-08-06: the first-run scan now keeps internal game ids and scanner tasks
out of the visible status line, uses full launcher titles, and loads packaged
catalogs from the AppImage/installed FHS location. Remaining work is native
translation coverage for the other launcher scan states, not another fallback
to English.

Per-game cheats are still a single enable/speed gate. Expand them only where a
game runtime has a real, bounded capability to consume the option; a launcher
toggle with no game-side implementation must stay unavailable.

The shared graphics tab now exposes real global V1/V2 presentation, scaling,
aspect, filter, vsync, viewport, and smooth-turn settings. The remaining
per-game V2 filter chains (DM1 palette/CRT/postprocess and CSB/Theron V2
filters) are persisted but do not yet have a dedicated editable advanced UI;
add those rows only with runtime application coverage.

2026-07-30 launcher reliability update: the selected game-data directory now
retains the player's normalised path across scans and restarts instead of
persisting macOS's scanner-only `/private` alias. The embedded changelog also
derives its current-build header from CMake version metadata. Keyboard, mouse,
touch, language-popup, data-picker, accessibility, save-browser and launcher
handoff coverage is green in the full M12/launcher test selection.

## DM1/CSB Render Follow-up (2026-07-12)

DM1's C38 projectile precheck now carries ReDMCSB F0190's live active-group
compaction for surviving groups: health/cells, packed directions, and aspects
move together before a later behavior event, followed by F0217's C30 reaction.
Remaining DM1 projectile work is real multi-projectile/pixel capture, not
another synthetic group mutation.

DM1 F0328/F0811 thrown-object material now follows ReDMCSB `DUNGEON.C F0142`
through `DUNVIEW.C F0115:5691-5900`: a live `Projectile.Slot` is retained in
the DM1 viewport receipt, resolves to either M613 projectile art or M612/G0209
object art, and is drawn only in the C2900/C10 projectile lane. Remaining
projectile work is authentic multi-projectile/pixel capture, not a substitute
sprite or floor-item route. 2026-07-16 update: the live F0328/F0811 M11
consumer now carries `Projectile.Slot`'s associated thrown thing through the
DM1 runtime materialization decision, and a real PC34 `DUNGEON.DAT` +
`GRAPHICS.DAT` probe proves a thrown weapon reaches the object-material
C2900 receipt after the action/tick path. M11 now fails closed when a
non-empty live Slot cannot be decoded from PC34 thing data, and asset-backed
F0115 passes no longer substitute cyan projectile markers. Remaining work is
capture, not a fallback-material path. The C2900 thrown-object route now also consumes the
original G0209 right-side mirror predicate for D2/D3 lanes; remaining work is
real multi-projectile capture, not a lane- or cell-only mirror substitute.
F0127 D0C now reaches its actual G2028/C2900 row 11 for live projectiles;
cells absent from that original row remain no-draw. Remaining work is capture
and other original thing families, not a D0 synthetic marker or replacement
sprite. F0115 now scans past an invisible live C14 record to the next record
with an actual C2900 cell, so multi-projectile materialization cannot select a
missing first-cell fallback; remaining work is real multi-projectile capture.
2026-07-17 update: M11 F0142/F0115 rendering and F0215 associated-thing
materialization now revalidate `Projectile.Slot` through raw F0156/F0140 and
the raw F0159 container chain. Decoded WEAPON/POTION/ARMOUR/JUNK mirrors no
longer authorize a material, impact drop, poison consumption, or sharp-weapon
aftermath; an altered or incomplete raw record is a no-draw/no-materialization
result. The local PC34 probe correctly skips because its dungeon has no
admissible raw throwable record. Remaining work is an authenticated corpus
capture with a live F0328 raw record, not a decoded or generated replacement.
DM1 projectile spells now cross F0412 to F0327 only through a strict PC34
receipt: the G0487 spell row, explosion Thing, bounded kinetic energy,
maximum-mana-derived step energy, zero required mana, and champion direction
must agree with ReDMCSB before F0212 creation. A changed field is rejected;
no approximate projectile or host fallback is emitted. Remaining projectile
work is authentic multi-projectile/pixel capture.
F0127 now consumes deferred live explosions only through original M636 pattern
material after D0 projectile materialization; a missing M636 bitmap is no-draw
and cannot fall through to an F0114 D1-D3 sprite. Remaining work is real
multi-effect capture, not a cue, marker, or replacement bitmap.

2026-07-13: the F0215 materialization receipt now also owns F0217's required
preceding C14 cleanup coordinates. M10 unlinks the exact live projectile Thing
from its original source chain before it attaches `Projectile.Slot` at the
source or resolved champion-impact square; an absent C14 rejects materializing
the Slot rather than leaving a renderable duplicate. This covers the same
cleanup order for dropped weapons, consumed potions, and non-materialized
spell Slots. Remaining work is still authentic multi-projectile and pixel
capture with user-supplied PC34 data.

2026-07-13: the direct M11 F0811 tick path now consumes that same receipt
instead of re-deriving only a drop plan. It removes C14 before its F0215 Slot
route and therefore leaves the next F0115 scan with exactly one valid render
owner: a live projectile before impact, a real linked floor object after an
ordinary drop, or neither for potion/spell/group-slot cleanup. Remaining work
is real multi-projectile pixel capture, not an alternate M11 impact renderer.

2026-07-13: the M11 impact path now also performs F0214 queue cleanup for
only the matching C48/C49 projectile slot, and writes C14's decoded/raw Next
to `THING_NONE` after F0215. The F0330/F0407 C11 action receipt remains
outside projectile cleanup ownership. A direct runtime regression proves a
real PC34-shaped C14 arrow hits a wall, becomes the sole terminal F0115 floor
object on its source square, and leaves no move event or C14 chain link.
Remaining work is authentic multi-projectile pixel capture.

2026-07-13: simultaneous F0811 routes now retain separate PC34 ownership:
an arrow hitting a wall and an explosion-slot fireball leaving the map on the
same tick each unlinks only its own C14, removes only its own C48/C49 event,
and clears its own raw/decoded `Next` to `THING_NONE`. F0215 materializes the
arrow as the sole F0115 floor object while the fireball leaves no static
Thing. An unrelated C11 action receipt survives unchanged. Remaining work is
authentic multi-projectile pixel capture with user-supplied PC34 media, not a
different impact or renderer path.

D0C C100/C101 rebirth C15 records now follow their separate ReDMCSB routes.
C100's real PC34 lightning material (`M613 + G0210[C03] + 1 = 464`) and C3000
centres now follow `L2476 = G2028`, not `G2034`: `DUNVIEW.C:5948,5984,5999`
uses `L2476`, assigned at `4806-4812`. The original `G2037` scales prove only
rows 0..6 (`15,15,15,20,20,20,32`). Rows 7..11 have valid C3000 coordinates
but no scale. The exact unresolved contradiction is source-locked:
`DUNVIEW.C:373` maps visible D2R/D1C/D1L/D1R/D0C to G2028 rows 7..11 and
`:5984` indexes `G2037[row]`, but `DUNVIEW.C:1914-1926` declares the matching
C100 coordinate/scale data as seven rows only. `FTL.idc:21033-21034`
independently locates `G2037` at `0x2583B` and `G0230` at `0x25842`, exactly
seven bytes later. Therefore C100 must remain no-draw for rows 7..11 until a
PC34 runtime capture or verified I34E disassembly proves the actual mapping.
It cannot borrow C101/M636, F0114, a marker, host geometry, or the adjacent
mutable globals as scale values. Focused source-lock tests separately require
non-D0C C100/C101 to remain outside ordinary F0114 material and D0C to admit
only C101's documented M636 route; C100 never reaches either fallback.

F0115's deferred C15 receipt preserves each active same-square PC34 record in
source-list order before material filtering, with explicit ordinary F0114/D0C
M636, C100 C3000-blocked, C101 C3007-blocked/D0C-M636, and Fluxcage F0113
routes. M11 consumes only each admitted ordinary record's own type/frame/attack
material after object, creature, and projectile passes. C100's separate scale
gate is unchanged; blocked rebirth records cannot borrow a generic sprite.
Remaining explosion work is real-media multi-effect capture plus the separate
rebirth geometry route.

F0128's D4L/D4R/D4C `C0x0001` calls retain their real early F0115 object-pass
ordering, but D4 has no G2028/C2900 projectile row and F0115 exits before the
projectile branch. Firestaff therefore has no D4 projectile/marker fallback.
The remaining D4 work is a source-backed G0218 object-coordinate consumer,
not reintroducing a guessed far projectile box.

DM1 F0115 now consumes only ReDMCSB's compact 13 `G0205` ornament zones.
The F0676/F0677 D3L2/D3R2 planes are deliberately outside that pass, so they
cannot duplicate inscriptions.  The remaining DM1 inscription work is a
packaged Mac/app pixel capture, not a replacement font or host-side patch.

CSB startup has no M11 text or door fallback callbacks.  Only the verified
C001-C005/C017/C040 graphics route can present title, entrance, door opening,
or HUD.  Remaining work is packaged real-data capture through that route.

CSB's runtime frame now binds original C017 inventory and C040 resurrection
pixels into its route hash. Remaining CSB startup work is the clean-build
regression repair and packaged Mac/app capture, not a substitute HUD.

CSBWin's admitted DSA `SETSKIN` route is now action-transactional: it stages
original DB11/EXPOOL skin writes until every authenticated source word has
been consumed, while a later `GETSKIN` sees the staged byte. `ProcessDSATimer6`
now retains the real self-master receipt and supports serialized `LocalState=1`
(`DSA::m_state`) as well as the DB3 state-nibble route. Verified restored
`TT_STONEROOM`, `TT_FALSEWALL`, `TT_OPENROOM`, `TT_DESSAGE`, `TT_DOOR`,
`TT_TELEPORTER`, and `TT_PITROOM` timers now bind their saved
target/action/position to that receipt. The exact zero-parameter
`TT_DESSAGE`/`TT_DOOR`/`TT_TELEPORTER`/`TT_PITROOM` handoffs also execute the
already admitted authenticated pure-stack action. Authenticated `JUMP/GOSUB`
transfer receipts now also persist their source final `LocalState=1`
`DSA::m_state` through the complete RCS-checked Extended Features DSA stream.
Remaining CSB DSA work is writable widened `LocalState=2` ParameterB state,
a real slave-master route, source world effects, and a real-save corpus. Do
not promote unsupported world or text opcodes from fixtures.

  - 2026-07-15 update: the exact compact `LocalState=2` `PutState` form now
    writes the authenticated DB3 `ParameterB` state back to its original raw
    record after a complete action. DB3 records carrying the unrepresented
    `word8` extension remain rejected, so widened ParameterB state, the
    slave-master route, world effects, and a real-save corpus are still open.

CSBWin type-47 DSA binding now also requires the current complete
FNV-authenticated Extended Features tail that published its saved level index
and action catalog. A stale, truncated, or headerless retained catalog cannot
resolve a timer or filter callback. Remaining DSA work is still writable
widened `LocalState=2` ParameterB state, a real slave-master route,
master-state/world effects, and a real-save corpus; no fallback catalog or
generated DSA state is accepted.

The source-owned C38 creature-damage path now also enters CSBWin
`Character.cpp::DamageCharacter`'s `DamageCharFilter` only after its existing
shared resolver has produced a positive source-shaped damage and wound mask.
The bounded handoff passes the documented seven parameters and consumes only
an authenticated pure-stack callback's non-negative signed-16 final-damage
word. Missing, altered, unsupported, or unrepresentable filter results leave
the established C38 path unchanged; this does not claim complete
`DamageCharacter` pending-damage, wound, shield, or non-C38 coverage.

The Phase 7 CSB runtime gate now also drives the same compact `LocalState=2`
`ParameterB=4` state through the saved `TT_STONEROOM`, `TT_OPENROOM`, and
`TT_FALSEWALL` runner boundaries, then proves a widened high-bit value rejects
before dispatch. This is regression coverage for the already admitted compact
DB3 form, not evidence for widened ParameterB records or a generic DSA route.

Nexus M11 now presents only the verified WARNING.BIN/TITLE.CG transition at
Saturn frames 47, 48, and 102. MENU.BPK remains blocked until its PRS3 pixel
and palette decoder is proven; the next Nexus work is real DGN rendering.

- 🔧 2026-07-14 Nexus Structure3 face-geometry follow-up: the retail
  LEV00-LEV15 corpus now proves every bounded Structure3 face row contains at
  least one non-collinear vertex triplet within the measured signed-16.16
  coordinate envelope. This is mesh-shape evidence only: it does not make the
  paired normal a plane normal, select winding/transforms, decode texture or
  palette bytes, or authorize a VDP1/host draw. Next DGN work needs an
  original Saturn executable trace or frame capture that binds a selected
  Structure3 entry and face order to a real render transform and material
  route; do not infer either from the all-nondegenerate corpus measurement.

Nexus Track 1 capture now requires hash-bound SN_FLOOR.MNS and SN_WALL.MNS
and forbids BPK material surfaces. Remaining work is full DGN semantics and
texture decoding, not a substitute material path.

The verified MNS TEXT route now rejects duplicate material IDs before any
surface allocation, so one original descriptor cannot silently replace another
in the static material bank. This hardens source identity only; it does not
prove Structure1B's selector transform or make DGN geometry drawable.

The MNS TEXT material decoder now commits its bank atomically: an out-of-bank
descriptor, allocation failure, or texture requiring more than the host's
256-entry indexed palette clears every decoded surface and blocks the route.
This confirms complete source-bank consumption, not Structure1B selector or
Saturn texture semantics.

Nexus's original Structure1B wall-selector transform remains unproven. Real
LEV00-LEV15 bytes 3/4 contain values outside the 15 descriptor IDs in the
hash-bound `SN_WALL.MNS` TEXT bank. The package-to-host handoff now reports
`blocked-structure1b-selector` before constructing any MNS-backed command,
and accepts only one complete source route per plan (proved MNS or separately
authenticated BPK). The remaining work is a Saturn executable or capture
that proves the mapping; do not derive a nibble, mask, or directional
transform from the observed values alone.

Structure1C's bounded four-byte records are now retained only as original
reference-table data. The next admissible collision work is a Saturn
executable or capture that proves their byte grammar; do not reinterpret them
as line/circle coordinates or promote them into movement geometry.

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
  - 2026-07-15 update: an engine-owned admission gate now accepts only a complete Mednafen-debugger trace matching the active SLEV target's level, MD5, source size, and entry opcode. Admission retains opaque execution locations but deliberately does not assign a task-body meaning or permit dispatch; a trace plus original code analysis is still required for each semantic promotion.
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
- 🔧 2026-07-13 Nexus PRS3 original-frame follow-up: hash-verified retail `DM.BIN` and `MENU.BPK` now share a bounded V1 outer-frame receipt (one complete DM.BIN V1 record and 162 complete MENU.BPK V1 streams). This proves only magic/version/declared-output/first-frame-word framing. A captured original SH-2 PRS3 execution is still required to establish command grammar, bit order, termination, decoded pixels, and any menu graphics handoff; all of those routes remain fail-closed with no fallback visual.
  - 2026-07-13 update: the selected retail DM.BIN V1 SH-2 route now has an importable instruction receipt for its R11 control test, bounded R12 post-increment byte read, R13/R0 byte store, and loop branch. It is not a live MENU.BPK binding or VDP1 capture. Remaining work is an original execution capture connecting one hash-verified BPK entry to those reads, its full output range, and a real VDP1 command/source range before PRS3 decoding or menu handoff can be considered.
  - 2026-07-14 update: the capture schema now rejects partial evidence and requires a single original trace to bind one exact BPK stream, SH-2 input/read range, complete output-write range and fingerprint, then a later VDP1 command using that exact output range. No such local capture exists yet; a valid future receipt remains evidence-only and cannot promote a generic PRS3 decoder or fallback menu rendering.
- 🔧 Mechanics parity hardening: movement, click routes, item usage, doors, pits, teleporters, triggers, combat, AI, and sound are implemented; remaining work is broader runtime/probe coverage beyond compile/save-load gates. 2026-07-22 update (Lane D, cycle 3): creature attack damage is now applied to the party leader (or first living party member) and total party death sets `game_over=1` / `game_over_reason=2 (all_dead)`. The empty-party `nexus_mechanics_party_alive()` bug is fixed (empty party is dead, not alive). The mechanics parity probe now covers the integrated tick with a synthetic scorpion-vs-party combat scenario. 2026-07-22 update (Lane D, cycle 4): champion death auto-leader promotion is implemented. `nexus_v1_champion_on_death_update_leader()` in `src/nexus/nexus_v1_champions.c` promotes the first living party member to leader when the current leader dies, matching ReDMCSB CHAMPION.C F0319 lines ~1662-1679. The mechanics tick calls it after creature-attack damage and stamina-collapse death. The mechanics parity probe now verifies non-leader death leaves leader unchanged, leader death promotes the next living member, and total party death returns no successor. 2026-07-22 update (Lane D, cycle 5): pit/chute square-event integration is implemented — stepping on a `NEXUS_SQUARE_CHUTE` now forces a level transition to `map_index + 1` via `pending_level_change`. Item usage/click-route wiring is implemented — `NEXUS_CMD_USE_ITEM` consumes the selected leader inventory slot (`use_item_slot`), applies consumables (health/mana/stamina potions, antidote, corn, water flask) and equips weapons/armor, then clears the slot and recalculates load. Source locks: DM1 MOVESENS.C F0267/F0268 (chute/pit), COMMAND.C item-use dispatch, CHAMPION.C F0309 equipment slots. The mechanics parity probe now covers both new behaviors (207/207 PASS). 2026-07-22 update (Lane D, cycle 6): mouse click-route dispatch for inventory/world objects is implemented — `nexus_click_route_dispatch()` translates inventory-slot, equipment-slot, world-square, door-square, and floor-item clicks into the same command queue used by keyboard input (`NEXUS_CMD_USE_ITEM`, turns, `NEXUS_CMD_FORWARD`, `NEXUS_CMD_INTERACT`). New `NEXUS_CMD_INTERACT` picks up floor items at the party's current square into the leader's inventory. Source locks: DM1 COMMAND.C mouse/click dispatch, CLIKMENU.C F0366 command queue, CHAMPION.C F0309 equipment slots, MOVESENS.C F0267/F0268 square interaction. The mechanics parity probe now covers click-route dispatch (218/218 PASS) and the dedicated `test_nexus_v1_click_route` regression test covers 31 checks. 2026-07-23 update (Lane D, cycle 7): pit/teleporter broader runtime coverage is implemented — `nexus_process_square_event` now reports the registered stair facing (`out_target_dir`) for stairs up/down; `nexus_mechanics_tick` processes `pending_teleport` before the step cooldown so teleporter warps are immediate, and cross-level teleporters set `pending_level_change` to the target level. New regression test `test_nexus_v1_pit_teleporter_runtime` covers chute step, chute max-level clamp, same-level/cross-level/unregistered teleporters, and stairs down/up targets (24/24 PASS). The mechanics parity probe adds Probe 12 for teleporter runtime (same-level, cross-level, unregistered) and now passes 226/226. Source locks: DM1 MOVESENS.C F0267/F0268 (teleporter/pit/stair sensors), DUNGEON.C square type dispatch, CLIKMENU.C:264-276 level-transition special cases. 2026-07-23 update (Lane D, cycle 8): stairs/exit/alarm broader runtime coverage is implemented — unregistered stairs now fall back to the adjacent level (down +1, up -1, clamped to [0,15]); registered stairs keep their exact target level/coordinates/facing; exit squares only end the game on the final level (level 15), with non-final exits treated as ordinary floor; alarm traps now alert only creatures on the current level and set a bounded 60-tick alarm timer that keeps alerted creatures chasing even when the party moves out of normal detection range. `Nexus_Creature` gains a `level` field, `Nexus_V1_CreatureManager` gains `alarm_timer`, and `nexus_v1_creature_spawn_on_level()` is added so probes/tests can place creatures on specific levels. `nexus_v1_creatures_tick()` now skips/attacks only creatures on the active level. `test_nexus_v1_pit_teleporter_runtime` expanded to 34 checks covering stairs down/up registered/unregistered and final/non-final exits. The mechanics parity probe adds Probe 14 for stairs/exit/alarm runtime and now passes 240/240. Source locks: DM1 MOVESENS.C F0267/F0268 (stairs/exit sensors), F0277 ALARM; CLIKMENU.C F0364_COMMAND_TakeStairs; ReDMCSB CHAMPION.C F0309 equipment slots. 2026-07-23 update (Lane D, cycle 9): water/fire square traversal mechanics are implemented — water squares (type 21) now block movement unless the party leader carries a Rope (item 65); fire squares (type 22) block movement unless the party leader carries a Rune of Fire (item 80). The passability gate lives in `nexus_mechanics_tick()` alongside the existing door key check; the square event layer now emits `NEXUS_EVENT_CROSS_WATER` and `NEXUS_EVENT_CROSS_FIRE`. New `NEXUS_MOVE_CROSS_WATER`, `NEXUS_MOVE_CROSS_FIRE`, `NEXUS_MOVE_BLOCKED_WATER`, `NEXUS_MOVE_BLOCKED_FIRE`, and `NEXUS_MOVE_BLOCKED_DOOR` result codes are defined in `nexus_v1_movement.h`. `test_nexus_v1_pit_teleporter_runtime` expanded to 44 checks covering water/fire blocked/crossed and square-event returns. The mechanics parity probe adds Probe 15 for water/fire square runtime and now passes 251/251. Source locks: DM1 MOVESENS.C F0267/F0268 water/fire square sensors; nexus_v1_inventory.c Rope (65), Rune of Fire (80). 2026-07-23 update (Lane D, cycle 10): real-DGN playability probe is implemented — new `firestaff_nexus_v1_mechanics_playability_probe` loads retail `LEV00.DGN` from `FIRESTAFF_NEXUS_DATA_DIR` (or `~/.firestaff/data/nexus`), verifies 64x64 Structure1B load, initializes a party on the actual starting floor square, exercises forward movement/turning on real geometry, verifies OOB/map-edge blocking, reports decoded floor/wall/door counts, and flood-fills reachable passable squares. The probe is skip-safe when the retail corpus is absent. Source locks: DMWeb DGN Structure1B format; ReDMCSB DUNGEON.C, COMMAND.C, MOVESENS.C, CHAMPION.C. CTest `firestaff_nexus_v1_mechanics_playability` passes 16/16 against the local Track 1 LEV00.DGN and exits 0 (skip) when data is missing. 2026-07-23 update (Lane D, cycle 11): expanded the real-DGN playability probe to all 16 retail levels (LEV00–LEV15). `firestaff_nexus_v1_mechanics_playability_probe` now loops over LEV00.DGN–LEV15.DGN, loads each through the existing Structure1B decoder, verifies 64×64 dimensions, counts floor/wall/door squares, checks OOB boundary blocking, real wall blocking, forward movement/turning on real floor, and flood-fills reachable passable squares; the probe reports 253/253 PASS against the local Track 1 corpus and remains skip-safe when data is absent. A companion CTest regression test `nexus_v1_dgn_multi_level_playability` (`tests/test_nexus_v1_dgn_multi_level_playability.c`) covers the same core checks across all 16 levels and returns 77 when no data is present. Remaining mechanics work: sound playback binding (still blocked on SAL decode), stairs/exit/alarm exact original timing/feedback, and real-data playability probes for additional square-event semantics once Structure1B wall/special-square decoding is source-locked against original Saturn evidence.
- 🔧 DMDF embedded BITMAP/palette/string runtime handoff remains open after the parser-level bounds gates. The real MNS `TEXT` descriptor and BGR555 material-bank route is now regression-covered: all 30 retail models retain matching descriptor/pixel receipts and all 815 source textures decode. The seven creature banks whose source colour cardinality exceeds the indexed 256-entry host bank now retain exact BGR555 words in a source-only direct-colour lane; they are not quantized, substituted, or admitted to the indexed viewport. VDP1 command/CLUT ownership, direct-colour display semantics, texture upload and runtime render binding remain capture-gated.
- 🔧 2026-06-28 Nexus V1 save multi-slot round-trip follow-up: new `test_nexus_v1_save_multislot_roundtrip_pc34_compat` (CTest `nexus_v1_save_multislot_roundtrip_pc34_compat`) drives 4 distinct slots (0..3) with distinct per-slot world + champion state through `nexus_v1_save_full` / `nexus_v1_load_full` and verifies party_level/x/y/dir + world_tick + per-object (type, state, x, y, level, quantity, linked_id, flags) + per-event (type, level, x, y, arg0, arg1, fired, repeat) + per-active-timer (id, kind, level, remaining_ticks, interval_ticks, flags) + transition (pending, target, spawn_x, spawn_y) + per-champion stat blobs (name, primary_class, hp, max_hp, stamina, max_stamina, mana, max_mana, str, dex, wis, vit, anti_magic, anti_fire, fighter/ninja/priest/wizard level, food, water, alive, portrait_index, wounds, attributes, inventory[30]) + party[] indices round-trip per slot, plus manager slot cache + scan() + isolation + deletion + CRC tamper rejection (one-byte flip in the data section → `NEXUS_SAVE_ERR_CRC`) + foreign-magic rejection (`NEXUS_SAVE_ERR_UNKNOWN_VARIANT` + non-empty diagnostic). Source-lock: `src/nexus/nexus_v1_save_load.c` (NEXUS_SAVE_MAGIC='FNXS', CRC-32 over champion+world data sections) + `src/nexus/nexus_v1_world.c` (party + objects + events + active timers + transition + world_tick + state_hash) + `src/nexus/nexus_v1_champions.c` (CHPN magic, 270-byte champion blob) + ReDMCSB LOADSAVE.C F0433/F0434 lineage. Same family, disjoint scope: existing slot-0/party-x test still covers the single-field gate; this new test extends coverage to 4 slots + 30+ per-slot world/champion fields + cache/scan/isolation/deletion + CRC + unknown variant. Companion source-side fixes (also shipped this pass): (a) `nexus_v1_champion_pool_serialize_size` now matches the actual `wr32`-based 24-byte header (was claiming 22 with a `version(2)` that the serialize code does not write); (b) `champion_blob_size` now counts 25 int fields per champion (was 23, which under-counted by 8 bytes/champion and silently overflowed the 24-champion pool blob in older code paths); (c) `nexus_v1_world_serialize_size` now omits the bogus 4-byte object-count prefix (the actual serialize path reads the count once from the header); (d) `nexus_v1_load_full` and `nexus_v1_load_full_from_path` now allocate buffers via the new `nexus_v1_save_max_champion_pool_size` / `nexus_v1_save_max_world_size` helpers instead of asking the destination's serialize_size (which underestimates because the destination has not been loaded yet — the prior code only worked when the saved world happened to have no objects/events/timers). Remaining save-slot work: original Saturn 8 KB memory card format reverse-engineering (Firestaff-native only today), real-asset save compatibility artifacts, and broader per-game (DM1/CSB/DM2/Theron) save interoperability.
- 🔧 2026-06-25 S2D Saturn-font section-table evidence: `firestaff_nexus_v1_saturn_font_scr_sections_probe` (see DONE.md 2026-06-25 entry) now walks the 32-entry SEGA SATURN SCR section table of `FONT256.S2D` via `nexus_v1_font_load_sections()` and confirms the four-populated real-on-disk layout (indices 0/2/4/6, contiguous chain at 0x0120+0x2010 / 0x2130+0x3c90 / 0x5dc0+0x0210 / 0x5fd0+0x01e4, 24,976 bytes of section data inside the 25,012-byte asset). **2026-06-28 section→glyph-range map + bounded runtime text-layout cursor landed:** new `nexus_v1_s2d_text_layout` module (`include/nexus_v1_s2d_text_layout.h`, `src/nexus/nexus_v1_s2d_text_layout.c`) builds a `Nexus_V1_S2D_SectionGlyphMap` from the parsed sections, routes glyph indices to the matching range, and exposes a layout cursor that walks ASCII strings via the existing `nexus_v1_font_draw_glyph_indexed()` (per-glyph advance by `char_width + letter_spacing_x`, `'\n'` breaks, `'\t'` steps, `max_chars` cap, `chars_skipped` for out-of-coverage). CTest `nexus_v1_s2d_runtime_text_layout` covered the synthetic SCR map invariants + layout cursor + real-asset `FONT256.S2D` skip-safe path that drives `"NEXUS"` through the real parser→map→layout chain 3 times and asserts bit-identical FNV-1a framebuffer hash. **2026-06-29 runtime framebuffer binding landed:** `Nexus_V1_ScreenTextRuntime` in `nexus_v1_text` now consumes the parsed sections + loaded font, builds the same section map once, draws ASCII through the S2D layout into `Nexus_Framebuffer.color_buffer`, and emits deterministic receipt/hash counters; the same CTest now covers stable framebuffer bytes across repeated draws. **2026-07-17 strict FONT256 admission landed:** `nexus_v1_font256_s2d_admit()` requires the canonical 25,012-byte SHA-256-attested file, exact SCR header facts, the four observed bounded spans, and raw FNV witnesses for the table and each section. It records no glyph layout, bit order, character map, pixels, or drawing permission. Remaining work is real glyph payload/encoding proof, Shift-JIS mapping and advance semantics, and independent Saturn screen evidence before any original-font rendering claim.

- 🔧 2026-07-17 FONT256 first-section witness: the canonical SHA-256-attested
  SCR admission now rechecks the first observed raw span at section-table
  index 0, `[0x0120,0x0130)`, against the live source/table/section FNVs and
  exposes it only as a capture-required witness. The observed bytes establish
  no text header fields, glyph layout, palette record, bit order, character
  encoding, pixel data, or draw route. No verified FONT256 palette path is
  present in the local source/documentation; obtain an original Saturn trace
  or independently reviewed format material before assigning any semantics.
  The complete `[0x0120,0x2130)` first section now also has a one-span raw
  iterator/capture receipt. It rechecks source, preamble, and whole-section
  FNVs before exposing precisely that one bounded span; it deliberately emits
  no inferred child records. A future text, glyph, palette, pixel, or draw
  route must first prove its own subspan grammar from original evidence.
  Directory/subrecord inventory is negative: after the 16-byte preamble, the
  local canonical section is one monotonic BE16 ramp from `0x0000` through
  `0x0ffe`. No local corpus evidence or reviewed S2D reference assigns it a
  directory role or defines a following glyph/palette subrecord grammar.
  Treat that observed ramp as opaque and capture-required, never as a glyph
  table or pixel layout.
  - 2026-07-19 update: all four populated SCR sections (table indices
    0/2/4/6) now carry the same capture-required, no-semantics receipt
    treatment as the first-section witness. New
    `nexus_v1_font256_s2d_section_corpus_receipt` module
    (`include/nexus_v1_font256_s2d_section_corpus_receipt.h`,
    `src/nexus/nexus_v1_font256_s2d_section_corpus_receipt.c`) admits each
    populated section against the live canonical source (source FNV, section
    table FNV, per-section FNV rechecks), binds its 16-byte preamble witness,
    and records only opaque raw composition measurements (zero/nonzero byte
    counts, post-preamble word count, BE16 ramp-prefix length and full-ramp
    flag). A corpus-level receipt additionally binds the observed contiguous
    four-section chain `[0x0120,0x61b4)` (24,724 bytes covering the source
    tail) as one capture target, and a bounded span iterator emits exactly
    the four whole-section spans in admission order with no inferred
    subspans. No byte or word is assigned text, glyph, palette, record, or
    pixel meaning; draw routes remain blocked. CTest
    `nexus_v1_font256_s2d_section_corpus_receipt` (synthetic) and
    `nexus_v1_font256_s2d_section_corpus_receipt_real` (skip-safe retail
    FONT256.S2D path) both pass, including tamper drift rejection on every
    section, the preamble, and the section table. Remaining FONT256 work is
    unchanged: an original Saturn trace or independently reviewed format
    material before any subrecord grammar, palette, glyph, or draw route.
  - 2026-07-20 update: the subrecord question is now answered read-only.
    New `nexus_v1_font256_s2d_subrecord_grammar` module
    (`include/nexus_v1_font256_s2d_subrecord_grammar.h`,
    `src/nexus/nexus_v1_font256_s2d_subrecord_grammar.c`) rechecks the
    canonical admission and the section corpus receipt and binds the
    observed internal subrecord arithmetic of three populated sections,
    still as opaque measurements with no text, glyph, palette, record,
    encoding, or pixel meaning: ordinal 0 (table index 0, 8208 bytes)
    carries a 16-byte preamble of eight canonical BE16 words {0x0010,
    0x0000, 0x4000, 0xffff x5} followed by a 4096-word BE16 ramp with
    word[i] == 2*(i & 2047) — two identical 2048-word step-2 half
    ramps 0x0000..0x0ffe — closing the section exactly (16 + 4096*2);
    ordinal 2 (table index 4, 528 bytes) carries exactly 33
    sixteen-byte records (three canonical head records, thirty records
    of eight 0x8000 words each) closing exactly (33*16); ordinal 3
    (table index 6, 484 bytes) is entirely zero. For ordinal 1 (table
    index 2, 15504 bytes) the grammar inventory is NEGATIVE and now
    receipt-bound: measured over canonical 16-byte blocks the section
    shows 742 populated of 969 blocks — an opaque composition
    measurement only, so this section stays capture-required with no
    proven subrecord structure. A bounded iterator exposes exactly the
    38 raw subrecord spans (3 + 1 + 33 + 1) whose lengths sum to the
    populated chain length 24724. CTest pair
    `nexus_v1_font256_s2d_subrecord_grammar` (synthetic mirror) and
    `nexus_v1_font256_s2d_subrecord_grammar_real` (skip-safe retail
    path) passes, covering the ramp arithmetic, both half ramps, the
    block population, all 33 records, the zero section, iterator spans,
    and rejection across NULL arguments, out-of-range ordinal,
    admission drift, preamble/ramp tamper in either half, block
    population drift in both directions, canonical/base record tamper,
    and zero-section tamper; population-preserving section-2 content
    tamper rebinds the live FNV and moves only the recorded digests.
    Remaining FONT256 work is unchanged: an original Saturn trace or
    independently reviewed format material before any glyph layout,
    palette, encoding, or draw route is assigned to these structures.
  - 2026-07-20 update (round 15): the ordinal-1 section (table index 2,
    15,504 bytes) now carries an exhaustive opaque composition inventory
    instead of the bare block population: exactly 742 populated of 969
    canonical 16-byte blocks in exactly 52 populated runs from block 0
    through block 968; byte alphabet exactly {0x00, 0x03, 0x0f, 0xff}
    with canonical counts 11,305/2,730/1,453/16; the lead block alone
    carries all sixteen 0xff bytes; every other nonzero byte is below
    0x10. New receipt fields plus the corpus flag
    `section2_composition_bound`; `subrecord_grammar_bound` stays 0 and
    the section stays capture-required with no proven subrecord
    structure — all four populated sections are now measured as far as
    the local canonical source allows. Synthetic mirror rebuilt to the
    same canonical composition; rejection coverage extended to alphabet
    violation, in-alphabet count drift, lead-block tamper, and
    run-structure drift at constant population/byte counts. CTest pair
    `nexus_v1_font256_s2d_subrecord_grammar` (+ `_real`) PASS. Remaining
    FONT256 work is still unchanged: an original Saturn trace or
    independently reviewed format material before any subrecord grammar,
    glyph layout, palette, encoding, or draw route.

- 🔧 2026-07-17 WARNING.BIN source-only follow-up: the canonical, directly
  SHA-256-attested `RES*` resource 0 now has a no-draw receipt over existing
  bounded DGT2 lookup facts. It retains raw DGT2, 512-byte CLUT, and pixel
  span offsets/lengths/FNVs only. The receipt does not convert BGR555, decode
  pixels, assign a palette, or authorize presentation; any later consumer must
  bring independent original-Saturn semantics and route evidence.
  The next receipt now binds the exact `RES*` declared size, 12-byte DGT2
  descriptor table, ascending source offsets, selected resource-0 descriptor,
  DGT2 header, and PP header to the same source FNV. It does not promote the
  observed descriptor fields into colour, image, palette, or presentation
  semantics; those remain separate original-Saturn evidence requirements.
  Resource 0 now retains a source-bound PP header field receipt and exact raw
  resource boundaries: PP header, 512-byte post-header prefix, declared body,
  and the two trailing bytes before the next descriptor. Width/height remain
  header values only; neither the prefix nor body is assigned CLUT, pixel,
  colour, stride, or draw semantics without further original evidence.
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
    (`include/nexus_v1_warning_dgt2_resource_corpus.h`,
    `src/nexus/nexus_v1_warning_dgt2_resource_corpus.c`) admits each resource
    against the live SHA-256-attested or canonical-FNV-witnessed source with
    per-resource canonical provenance bindings of the same class as the
    existing resource-0 constants: descriptor offsets `0x48`/`0x5c58`/
    `0xb868`/`0xf8f8`, resource lengths `0x5c10`/`0x5c10`/`0x4090`/`0x9290`,
    PP dimensions 240x96, 240x96, 200x80, and 272x136, the 512-byte BGR555
    CLUT, the width*height index plane, and the two trailing bytes per
    resource. The corpus receipt binds the observed contiguous four-resource
    chain `[0x48,101256)` covering the source tail. Per-resource execution
    copies the exact index bytes and original BGR555 words to caller-owned
    exact-sized buffers and invokes only the explicit presentation callback
    (no default presentation, host-RGBA conversion, CLUT substitution,
    trailing-byte interpretation, or fallback). Per-resource M11
    presentation revalidates the full receipt chain before writing the
    top-left index plane into the real 320x200 M11 indexed surface with the
    ST-124-ordered BGR555->RGB6 exact palette expansion; any drift leaves
    the cleared frame unpresented. CTest `nexus_v1_warning_dgt2_resource_corpus`
    (skip-safe canonical WARNING.BIN path) passes, covering per-resource
    receipts, exact plane/palette copies, M11 surface writes with untouched
    out-of-image pixels, and tamper rejection across every resource's pixel
    plane, CLUT, PP header, the descriptor table, stale receipts, callback
    refusal, and identity drift. This still proves no Saturn VDP display
    command, interlace, colour-DAC, gamma, timing, or placement contract,
    and no resource-to-screen assignment (which resource the original
    warning flow shows, in which order, remains original-Saturn evidence
    work).
- 🔧 2026-07-20 TITLE.BIN RES* directory corpus follow-up: new
  `nexus_v1_title_res_corpus_receipt` module
  (`include/nexus_v1_title_res_corpus_receipt.h`,
  `src/nexus/nexus_v1_title_res_corpus_receipt.c`) admits the canonical
  112,216-byte SHA-256-attested TITLE.BIN RES* container and publishes
  bounded per-record receipts for the full 60-entry directory: 22 DGT2
  records (ids 0..21), 4 TITL records (ids 0..3), 1 MAPD record (id 0),
  and 33 CNFD records (ids 0..32). Each record head must repeat its
  directory magic and class-local id, DGT2/CNFD heads must carry the
  observed `0x70 0x70` tag pair, TITL heads the `0x50 0x50` pair, and the
  MAPD head the observed `TIBG` tag; the receipts retain the raw head
  words at +8/+10/+12/+14 as opaque measurements only. Corpus admission
  revalidates the identity (canonical SHA-256 string plus live FNV-1a
  rebind), every directory entry against the canonical class/id/offset
  tables, the exact class counts, and the observed contiguous chain
  [0x2e8, 0x1b658) that covers the source tail with zero gap; a bounded
  span iterator exposes exactly the 60 whole-record raw spans (offset,
  length, FNV) with no inferred subspans. CTest pair
  `nexus_v1_title_res_corpus_receipt` (synthetic mirror of the canonical
  framing) and `nexus_v1_title_res_corpus_receipt_real` (skip-safe
  canonical TITLE.BIN path, real DGT2 head-word groups verified) passes,
  covering all 60 receipts, class counts, chain arithmetic, tail
  coverage, iterator spans, and rejection across NULL arguments,
  out-of-range indices, identity drift, directory-table tamper, and
  per-class record-head tag tamper; record-body tamper rebinds the live
  FNV and moves only the recorded digests. This still proves no record
  grammar, image, palette, or presentation semantics, no `pp`/`PP`/`TIBG`
  payload meaning, and no resource-to-screen assignment (which records
  the original title/startup flow uses, in which order, remains
  original-Saturn evidence work); 0DMSTRT.BIN shows no RES* framing and
  stays excluded from this block pending original evidence.
- 🔧 2026-07-20 TITLE.BIN TITL PP payload admission follow-up: new
  `nexus_v1_title_titl_pp_payload_admission` module
  (`include/nexus_v1_title_titl_pp_payload_admission.h`,
  `src/nexus/nexus_v1_title_titl_pp_payload_admission.c`) revalidates the
  RES* directory receipt for each TITL entry (22..25) and binds its
  observed PP payload of the already admitted ST-124 section-6 shape:
  six-byte PP header ("PP" tag, BE16 width, BE16 height), 512-byte
  post-header prefix, width*height byte plane, and two trailing bytes,
  with exact length arithmetic 14 + 512 + width*height + 2 per record.
  Canonical dimensions 304x104, 160x28, 304x22, 256x16; all four prefixes
  byte-identical with a shared 0x8220 leading word (bound as an observed
  shared-prefix fact, not an admission requirement); canonical trailing
  bytes 0x0000 retained as an opaque measurement. Corpus admission binds
  the contiguous TITL sub-chain [0x2318, 0xe278) inside the whole-file
  chain, and a bounded plane-span iterator exposes exactly the four raw
  width*height spans with no decode. CTest pair
  `nexus_v1_title_titl_pp_payload_admission` (synthetic mirror) and
  `nexus_v1_title_titl_pp_payload_admission_real` (skip-safe canonical
  path) passes, covering per-record receipts, chain arithmetic, the
  shared-prefix observation (including its flip on prefix divergence),
  iterator spans, real-mode trailing-zero and per-plane nonzero-count
  witnesses, and rejection across NULL arguments, out-of-range indices,
  identity drift, and PP header dimension/leading-word tamper; plane and
  trailing tamper rebind the live FNV and move only the recorded digests.
  This still proves no colour, palette, image, pixel, or presentation
  semantics and no TITL-to-screen assignment (which TITL images the
  original title flow draws, where, and in which order, remains
  original-Saturn evidence work).
- 🔧 2026-07-20 TITLE.BIN DGT2 payload admission follow-up: new
  `nexus_v1_title_dgt2_pp_payload_admission` module
  (`include/nexus_v1_title_dgt2_pp_payload_admission.h`,
  `src/nexus/nexus_v1_title_dgt2_pp_payload_admission.c`) revalidates the
  RES* directory receipt for each DGT2 entry (0..21) and binds its
  observed payload shape: 16-byte head ("DGT2" magic, class-local id,
  "pp" tag, BE16 width, BE16 height, BE16 flag word), 32-byte post-head
  prefix, and a packed width*height/2 byte plane, with exact length
  arithmetic 16 + 32 + width*height/2 per record and no trailing bytes.
  Canonical dimensions 64x8 (records 0..3), 104x8 (4..5), 24x24 (6..20),
  168x12 (21); canonical flag word 0x8220 except 0x81e0 for records
  6..20. Corpus admission binds the contiguous DGT2 sub-chain
  [0x2e8, 0x2318) inside the whole-file chain, the observed 20 distinct
  prefixes of 22, and the two byte-identical prefix pairs (2,4) and
  (3,5) as recorded observations that flip cleanly on divergence; a
  bounded plane-span iterator exposes exactly the 22 raw packed-plane
  spans with no decode. CTest pair
  `nexus_v1_title_dgt2_pp_payload_admission` (synthetic mirror) and
  `nexus_v1_title_dgt2_pp_payload_admission_real` (skip-safe canonical
  path) passes, covering per-record receipts, chain arithmetic, the
  prefix-pair and distinct-count observations, iterator spans, real-mode
  per-plane nonzero-count witnesses, and rejection across NULL
  arguments, out-of-range indices, identity drift, and head
  dimension/flag tamper; plane tamper rebinds the live FNV and moves
  only the recorded digest. This still proves no colour, palette, image,
  pixel, or presentation semantics (including the meaning of the packed
  plane's nibble order and of the 32-byte prefixes) and no
  DGT2-to-screen assignment; which payloads the original title flow
  draws, where, and in which order, remains original-Saturn evidence
  work.
- 🔧 2026-07-20 TITLE.BIN MAPD TIBG admission follow-up: new
  `nexus_v1_title_mapd_tibg_admission` module
  (`include/nexus_v1_title_mapd_tibg_admission.h`,
  `src/nexus/nexus_v1_title_mapd_tibg_admission.c`) revalidates the RES*
  directory receipt for the single MAPD entry (26, id 0) and binds its
  observed TIBG payload shape: 64-byte header ("MAPD" magic, id, "TIBG"
  tag, thirteen canonical BE32 fields including the payload-size field
  0x8c6c = record bytes - 8), a 4-byte-cell span [0x40, 0x8c54) of 8965
  cells with exactly five marker cells 00 40 00 1c at 0x40 + k*0x1c04
  (k = 0..4) and an observed filler-cell population of 3360, and a
  32-byte tail of sixteen BE16 words ending in 0xffff. Header, marker
  chain, cell span, and tail close arithmetically against the canonical
  record length 0x8c74; a bounded span iterator exposes exactly the raw
  cell span and tail span with no decode. CTest pair
  `nexus_v1_title_mapd_tibg_admission` (synthetic mirror) and
  `nexus_v1_title_mapd_tibg_admission_real` (skip-safe canonical path)
  passes, covering the receipt, header/marker/cell/tail arithmetic, the
  filler population, iterator spans, and rejection across NULL
  arguments, identity drift, header field tamper, marker tamper, filler
  population drift, and tail last-word tamper; non-filler cell tamper
  rebinds the live FNV and moves only the recorded cell-span digest.
  This still proves no tile, map, palette, colour, image, or
  presentation semantics (including the cell values', the header
  fields', and the tail words' meaning) and no MAPD-to-screen
  assignment; how the original title flow uses this payload remains
  original-Saturn evidence work.

- 🔧 2026-07-20 TITLE.BIN CNFD payload admission follow-up: new
  `nexus_v1_title_cnfd_payload_admission` module
  (`include/nexus_v1_title_cnfd_payload_admission.h`,
  `src/nexus/nexus_v1_title_cnfd_payload_admission.c`) revalidates the
  RES* directory receipt for each of the 33 CNFD entries (directory
  indices 27..59, class-local ids 0..32) and binds their observed
  payload shape — the same form as DGT2: a 16-byte head ("CNFD" magic,
  class-local id, "pp" tag, BE16 width, BE16 height, BE16 flag word),
  a 32-byte prefix, and a packed plane of width*height/2 bytes. Head,
  prefix, and plane close arithmetically against each canonical record
  length (16 + 32 + width*height/2 exactly for all 33 records), and
  the chain [0x16eec, 0x1b658) of 0x476c bytes covers the TITLE.BIN
  tail exactly after MAPD. The corpus observes exactly 8 distinct
  prefixes across the 33 records; the flag word is 0x8000 for records
  {0,6,12,18,24,30} and 0x8b00 otherwise; every width*height product
  is even. A bounded plane-span iterator exposes exactly the raw
  prefix and plane spans with no decode. CTest pair
  `nexus_v1_title_cnfd_payload_admission` (synthetic mirror) and
  `nexus_v1_title_cnfd_payload_admission_real` (skip-safe canonical
  path) passes, covering the corpus receipt, all 33 record receipts,
  the per-record arithmetic, the distinct-prefix count, iterator
  spans, and rejection across NULL arguments, out-of-range index,
  identity drift, width tamper, flag-word tamper, prefix divergence
  (distinct count 8→9 with admission intact), and plane tamper (live
  rebind moves only the recorded plane digest). With CNFD admitted,
  the TITLE.BIN chain is fully closed at admission level: all 60
  directory entries (DGT2, TITL, MAPD, CNFD) have internal payload
  admission. This still proves no glyph, font, palette, image, or
  presentation semantics (including the plane bytes', the prefixes',
  and the flag words' meaning) and no CNFD-to-screen assignment; how
  the original title flow uses these payloads remains original-Saturn
  evidence work.

- 🔧 2026-07-20 0DMSTRT.BIN structure admission follow-up: the file
  previously excluded as opaque ("no RES* framing") turns out to carry
  a different fully verifiable structure. New
  `nexus_v1_0dmstrt_structure_admission` module
  (`include/nexus_v1_0dmstrt_structure_admission.h`,
  `src/nexus/nexus_v1_0dmstrt_structure_admission.c`) admits the whole
  SHA-256-attested retail file (39516 bytes) against an exact
  zero-gap arithmetic partition: dense region A [0x0000, 0x08a8) with
  a canonical 2102 non-zero population, 11672-byte all-zero gap,
  dense region B [0x3640, 0x9978) with 24077 non-zero bytes, an
  83-byte all-zero gap, a 49-byte tail descriptor (0xff separator, a
  31-byte printable-ASCII version stamp leading with the observed
  "GFS_SBL" boot-library class tag — the observed stamp reports
  version 2.10 dated 1996-02-01, retained as an opaque printable
  measurement — NUL terminator, byte 0x01, the "CD001" standard
  identifier, and an ISO-style "." / ".." directory-id stub with a
  canonical 0xff population of 5), a 4-entry fixup table, a 32-byte
  all-zero gap, and a 12-entry fixup table ending exactly at the
  source size; a 7-entry head table at 0x0058 behind a 0xffff
  sentinel anchors region A. All 23 fixup entries share the observed
  BE16 tag 0x0601 with canonical BE16 value tables, and the eight
  region spans (exposed through a bounded iterator) sum to the source
  size exactly. CTest pair `nexus_v1_0dmstrt_structure_admission`
  (synthetic mirror) and `nexus_v1_0dmstrt_structure_admission_real`
  (skip-safe canonical path) passes, covering the partition
  arithmetic, both non-zero populations, all three zero gaps, the
  tail descriptor and ISO stub, all 23 fixup entries, iterator spans,
  and rejection across NULL arguments, size/identity drift, gap
  tamper, non-zero population drift, tail/stamp/stub tamper,
  sentinel/tag/value tamper; dense-region content tamper that keeps
  the population rebinds the live FNV and moves only the recorded
  digests. This still proves no instruction, code, data, relocation,
  address, or execution semantics (including the fixup values'
  meaning and the stamp's version text) and no load, relocation, or
  execution route; how the original boot flow loads and uses this
  image remains original-Saturn evidence work.

### Nexus V2.0 / V2.1 / V2.2

- 🔧 Phase 2 - Enhanced asset pipeline: presentation-mode selection API + filter config + V2.1 EPX upscaler pipeline are wired (`nexus_v2_upscaler.c` provides `nexus_v2_epx_upscale` indexed→RGBA via palette, `nexus_v2_bilinear_smooth` post-filter, `nexus_v2_upscaler_source_evidence`). Headless probe `firestaff_nexus_v2_upscaler_probe` 23/23 (palette lookup, deterministic output for same input, 2x scaling fills all dst pixels, 1x1 boundary case, null-arg safety on src/dst/palette/zero-dims, bilinear null-arg safety on null/0x0/1x1/4x4, source evidence). Ctest `nexus_v2_upscaler_probe` 1/1. **2026-06-19 Nexus V2.2 modern-asset module landed:** new `nexus_v22_modern_assets_pc34.c/.h` mirrors dm1/csb modules with Nexus paths (`~/.firestaff/assets/nexus/modern/`) and Saturn source-locks (SATURN_DMDF T400/T520/T600 + Saturn VDP1/VDP2). Ctest `test_nexus_v22_modern_assets_pc34` 33/33. **2026-06-19 Nexus V2.2 first-cut asset pack landed:** `.openclaw/tmp/nexus_v22_asset_author.py` (5 PNGs + manifest v1.0.0). Smoke: `nexus_v22_modern_assets_available()=1` end-to-end. Remaining work: real PBR hero art for Nexus via gpt-image-2 batch + per-cell modern-art swap in Nexus V1 draw pipeline.
## Cross-Cutting

- ✅ 2026-07-19 (Jobb F3, w5) Extractor diagnostic landed: external archives skipped because no supported extractor (7zz/7z/bsdtar) is installed now record a bounded, deduplicated `asset_scan_missing_extractor_*` diagnostic in asset_find_by_hash; the launcher scan logs each skipped archive plus tool list and `--scan-data` prints an "External archives skipped (no extractor installed)" section. Remaining: surface the same diagnostic as a localized launcher popup row (requires new po msgids across all 20 locales).
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
  - 2026-07-15 copy/backreference boundary: the complete linear zero-side
    block from branch target through its outer-loop return contains no direct
    `R2 -> @(R13,R0)` output store. Static DM.BIN therefore cannot prove a
    copy length or backreference, despite the masked R13 read and repeat
    control. The grammar remains blocked until an authentic Saturn trace
    provides R13 ownership, read/write addresses, branch outcomes, and a
    complete output range for one MENU.BPK stream.
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

- 🔧 2026-06-28 Nexus BPX/BPK surface-class + BPX3 directory-trailer boundary (pass1083) follow-up: `nexus_v1_bpk_archive` adds `nexus_v1_bpk_mode_to_surface_class()` (UNKNOWN / INDEXED_8BPP / RGB565 / RGB888 / RGBA8888 / DIRECTORY_TRAILER) and `nexus_v1_bpk_mode_to_bpp()` (1/2/3/4 for the four pixel-mode tags, 0 for the trailer / unknown) lookup APIs plus `nexus_v1_bpk_archive_surface_estimate()` which walks every entry whose 20-byte prefix is complete and reports (entry_index, mode, width, height, pixel_count, surface layout: bpp / rowstride / surface_bytes / surface_class) per PRS3-bearing entry, skipping the directory trailer and unknown modes. `nexus_v1_bpx_bpk` extends the synthetic BPX3 contract to recognize a directory-trailer entry (mode tag 10, zero width/height/payload_offset, no PRS3 magic) and tag it with `NEXUS_V1_BPX_BPK_METHOD_DIRECTORY_TRAILER`. New CTest `nexus_v1_bpk_surface_class` (38 PASS) covers the lookup APIs, a 4-entry synthetic BPK with one trailer + one of each pixel mode (16 / 64 / 18 = 98 unpacked bytes total), the BPX3 trailer-entry shape and rejection cases (nonzero width / nonzero reserved bytes), and an optional local MENU.BPK receipt that cross-checks 14 indexed / 62 RGB565 / 39 RGB888 / 47 RGBA8888 entries with every rowstride == width * bpp and every surface_bytes == width * height * bpp. New skip-safe CTest `nexus_v1_bpk_surface_class_probe` (525 PASS after the 2026-06-29 span checks) exercises the same contract through the probe path. **2026-06-29 PRS3 packed-span synthetic contract tightened:** `nexus_v1_bpx_prs3_parse()` now requires an explicit packed payload size for every synthetic PRS3-bearing entry, rejects zero/out-of-bounds/overlapping spans in table order, and reports bpp-derived `unpacked_size` (mode 6/14/22/30 -> 1/2/3/4 bytes per pixel) instead of multiplying by the raw mode tag. Local MENU.BPK byte scan for this pass confirmed 162 bounded PRS3 compressed payload spans (min 16 / max 9,980 / total 83,000 bytes) but still did not identify the bitstream algorithm. PRS3 decompression still intentionally unsupported: the surface estimate reports what shape each entry WOULD decode to once a real PRS3 implementation lands. The directory-trailer receipt now reaches launcher metadata. Remaining work: identify the PRS3 compression algorithm from real MENU.BPK bytes / executable disassembly and hand the decoded payloads into a renderable Nexus menu graphics pipeline (atmospheric HUD/textures + first Nexus screen capture with the real MENU.BPK).

- 🔧 Nexus S2D real-font parity: the DMWeb FONT256 region decoder now exposes the real page, character-generator, palette and attribute regions with bounded retail tests. Remaining work is page-to-character mapping, runtime text-layout binding, and an authentic screen capture; no flat 1bpp glyph guess may be promoted.

### Launcher and Settings

- 🔧 Start-menu feature hardening: first-pass persistence exists for quick resume, minimap, automap, combat log, soundtrack, ambient audio, UI scale, streamer mode, custom music, custom dungeon, screenshot path, session timer, save export/import, manual/docs launcher, polished UI flow, and all five per-game option slots. **2026-07-19 launcher-options runtime handoff landed (Jobb F2, commit 0f7fc0a43):** new `M12_LauncherRuntimeOptions` snapshot (global launcher settings + per-game language/cheats/speed folded in) is exported via `M12_StartupMenu_ExportLauncherRuntimeOptions()` with clamped ranges, carried on `M12_LaunchIntent.launcherOptions` after `m12_enforce_mode_constraints`, handed to `M11_GameLaunchSpec.launcherOptions`, and applied through `m11_apply_launcher_options_handoff` in all five game-start branches (dm1/csb/dm2/nexus/theron) after Shutdown/Init; CTest `m12_launcher_options_runtime_handoff` PASS. Remaining work is cloud sync.
- 🔧 Custom dungeon import: `custom_dungeon_import` now CTest-gates synthetic M12 launcher and DM1 V1 engine scanning for `DUNGEON.DAT` fixtures, including header/map-count parsing, case-insensitive paths, optional `GRAPHICS.DAT`, compressed/tiny rejection, and valid-entry selection. **2026-06-26 CSBWin `CSBgraphics.dat` bounded classifier landed:** new `csb_v1_csbgraphics_dat_classify` reads the on-disk count + parallel compressed/decompressed size tables and the optional `0x8001` little-endian sentinel without LZW/payload decode; companion `csb_v1_csbgraphics_dat_real_scan` mirrors the HCSB.HTC scanner pattern with an empty default known-hash list so the real-asset probe SKIPs cleanly on hosts without a user-staged CSBgraphics.dat. **2026-06-28 payload-span boundary added:** `csb_v1_csbgraphics_dat_entry_span()` now returns one override entry's compressed payload offset, compressed size, and decompressed budget using CSBWin `LocateNthGraphic(n)` offset math, still without LZW decode or runtime override. **2026-06-30 bounded payload decode added:** `csb_v1_csbgraphics_dat_decode_entry()` now decodes one declared entry through the existing ReDMCSB-compatible graphics LZW decoder and rejects undersized output buffers, compressed-empty mismatches, and bad streams without interpreting or overriding the bitmap. CTest `csb_v1_csbgraphics_dat_classify_unit` PASS 19/19 (argument/too-small/empty/oversized count rejection, big-endian + LE-marker round-trip, total-compressed overflow rejection, truncated tables rejection, max-entry tracking, big-endian and LE-marker entry spans, zero-length entry preservation, entry-range rejection, LZW round-trip decode, output-too-small rejection, empty-entry decode, bad-LZW rejection, source-evidence citation) and `csb_v1_csbgraphics_dat_real_scan` PASS (skip-safe when known-hash list is empty; locally verified 22/22 checks on a synthetic CSBgraphics.dat with a temporary hash registration). **2026-06-27 CSBWin `dmsave`/`csbgame` loader-boundary contract landed:** new `csb_v1_csbwin_save_loader_boundary_pc34_compat` (include + src/csb) builds a 14-shape CSBWin/DM1 save contract (3 accept + 11 reject: CSB v2.0/v2.1/.bak payload, DM1 RDMCSB15, CDSA marker, CSBWin 512-byte CSB\1/DM\0\1/CEDT, too-small, no-magic, champion-count-out-of-range, truncated records, bad-version) and runs every shape through the existing `csb_v1_import_csb_save_buffer()` entry point to record the documented accept/reject verdict and surface a `contract_match` flag. CTest `csb_v1_csbwin_save_loader_boundary_pc34_compat_unit` PASS 79/79 (contract-table invariants, per-shape loader-boundary check on synthetic fixtures, hand-rolled 2-champion v2.0 round-trip, builder determinism, accept-shape helper, source-evidence citation) and `csb_v1_csbwin_save_loader_boundary` PASS (skip-safe when no user-staged csbgame.dat/csbgame.bak/dmsave.dat/dmsave.bak exists). Source-locked against ReDMCSB CEDTINC8.C:101-118 + LOADSAVE.C F0433/F0435 + SAVEHEAD.C F0429/F0430 + DEFS.H:1289 and CSBWin SaveGame.cpp:927/1711/2111 + CSBCode.cpp:421-422 (csbgame.dat / csbgame.bak literals) + Data.h:590 (SaveGameFilename). Disjoint from the sibling `csb_v1_csbwin_save_classify_pc34_compat` (sibling is on-disk shape detection; this loader-boundary gate exercises the actual importer against each shape — they are complementary, not duplicative). Remaining work is real community dungeon corpus handoff, CSBgraphics.dat payload bitmap interpretation + M11 viewport binding, the CSBWin 512-byte obfuscation-key decoder, and the end-to-end CSBWin importer wiring that this loader-boundary gate is the contract for.

### Touch and Controller Support

- 🔧 2026-06-28 runtime gesture navigation gate landed (input translation + touch target safety): new `runtime_gesture_navigation_gate` module (`include/runtime_gesture_navigation_gate.h` + `src/engine/runtime_gesture_navigation_gate.c`) wraps the existing `firestaff_touch.c` swipe + edge-strafe primitives into a deterministic cross-game contract behind the existing touch/controller settings. `FirestaffRuntimeGestureNav_Evaluate(event, policy, result)` maps swipe up/down/left/right to FS_CMD_MOVE_FORWARD/BACKWARD/TURN_LEFT/TURN_RIGHT (cross-V1/V2), edge-left/right to FS_CMD_STRAFE_LEFT/RIGHT (V2-only with v1ParityPreserve guard), pins the 44 px Apple HIG touch-target floor (`RUNTIME_GESTURE_NAV_MIN_TARGET_PX`), and rejects disabled / too-short / too-small-target / ambiguous-diagonal / V1-only paths. Source-locked against ReDMCSB `COMMAND.C:2045-2155 F0380_COMMAND_ProcessQueue_CPSC` + `CLIKMENU.C:142-174 F0365 turn` + `CLIKMENU.C:180-390 F0366 move` + `GAMELOOP.C:164-219 V1 input wait loop` + `DEFS.H:238-243 C001..C006` + `firestaff_touch.c FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX=40/FIRESTAFF_TOUCH_TAP_TOLERANCE_PX=24/FIRESTAFF_TOUCH_EDGE_ZONE_FRAC=0.20`. New CTest `runtime_gesture_navigation_gate` (15 invariant groups: setting gate, four swipe paths, threshold rejection, diagonal ambiguity, edge-strafe paths, target-size safety, touch-target safety, source-viewport scale safety, null-pointer safety, default-threshold fallback, V1 swipe parity, decision-name contract, source-evidence citation, cross-V1/V2 command codes, travel-pixel threshold boundary) and headless probe `firestaff_runtime_gesture_navigation_gate_probe` (9 groups + 50-iteration determinism) both PASS; existing touch + session_timer + V2-touch affordance CTest targets still PASS (28/28 in the touched ctest set, no regressions). **2026-06-30 bridge wire-up landed:** `firestaff_touch.c` now exposes runtime-gated swipe and edge-strafe emit APIs and the legacy wrappers route through the same gate before pushing to `FS_InputQueue`; CTest `firestaff_touch_runtime_gesture_bridge` covers disabled/touch-off rejection, ambiguous swipe rejection, V1 edge-strafe rejection, V2 strafe emission, and wrapper compatibility. Remaining work is actual UI scaling / touch-target audit across launcher + game views and any Sphenx/Greatstone-style paired original-vs-Firestaff touch-zone pixel evidence.
- 🔧 UI scaling and touch-target audit across launcher and game views.
  2026-07-19 (Jobb F4, w5): the salvaged `fs_gesture_navigation_gate`
  audit module (24 px source-space floor / 44 px recommended) is now
  wired into the build with its 155-assertion CTest
  (`fs_gesture_navigation_gate`, PASS after two salvaged fixtures were
  aligned with the canonical firestaff_touch.c 20% edge-band / 24 px
  tap-tolerance semantics), and new CTest `m12_touch_layout_audit`
  verifies all three shipped M12 touch-layout presets meet both the
  24 px floor and the 44 px recommendation and that the editor clamp
  (`M12_TOUCH_MIN_ZONE_SIZE`) never saves a sub-floor zone. Remaining:
  audit the M11 in-game hit zones per game view at each UI scale (the
  DM1 V1 builtin table already audits below-floor source-space zones
  such as the 13x11 spell runes; decide per-zone whether presented-pixel
  scaling lifts them to the floor) and a launcher menu-row hit-height
  audit at fontScale 1..3.
  2026-07-20 launcher menu-row hit-height audit landed (job/w3): new
  shared header `include/menu_row_metrics_m12.h` is the single source
  of truth for every launcher menu-row surface — legacy 320x200
  settings-classic pitch 18/frame 24, settings-dense pitch 34,
  save-browser pitch 22, and the modern 1080p settings rows 50/70 +
  tab strip 34 — consumed by both draw paths (menu_startup_m12.c,
  menu_startup_render_modern_m12.c) and the new CTest
  `m12_menu_row_hit_height_audit`. The audit checks per surface and
  fontScale 1..3: (1) containment — the presented label (conservative
  11-row Unicode glyph bound at the m12_effective_text_scale-resolved
  scale; Swedish Å/Ä/Ö labels are Unicode glyphs) always fits the
  effective hit height; (2) classification — presented hit height at
  1x..4x presentation scale against the 24 px floor / 44 px
  recommendation, cross-checked through `fs_gesture_audit_zones`;
  (3) per-row decisions — legacy rows are V1-parity source-space small
  at 1x by design and must clear the floor at >= 2x presentation;
  modern settings rows meet the recommendation natively, the tab strip
  meets the floor (accepted below recommendation, secondary nav). The
  audit exposed a real overflow: at fontScale 2/3 the fixed 18/22 px
  legacy pitches could not contain the scaled label (22/33 px), so the
  classic settings and save-browser pitches are now scale-aware
  (`m12_menu_row_settings_classic_pitch/visible_rows`,
  `m12_menu_row_save_browser_pitch`); fontScale 1 stays bit-identical
  (18 px/6 rows, 22 px). The modern renderer does not apply fontScale —
  documented as a fontScale-independent surface. The M11 in-game
  per-game-view hit-zone audit remains open.
  2026-07-20 M11 in-game hit-zone audit landed, DM1 slice (job/w3):
  new pure audit header `include/hit_zone_audit_m11.h` + CTest
  `m11_ingame_hit_zone_audit` consume the LIVE DM1 V1 hit-zone
  inventory from touch_click_zone_matrix_pc34_compat.c (104
  source-locked zones; no geometry duplicated, so the audit cannot
  drift from the shipped hit-test table). Per zone, per UI scale
  100/150/200, per presentation scale 1x..4x: classification against
  the 24 px floor / 44 px recommendation cross-checked through
  `fs_gesture_audit_zones`, plus per-zone lifting decisions.
  Pinned shipped-geometry contract: floor-at-1x 19 zones, needs-2x
  62, needs-3x 17 (13x11 spell runes — the TODO exemplar — lift only
  at 3x: 2x leaves the 11 px side at 22 < 24; 85x11 action rows,
  11x11/9x9 icons), needs-4x 5 (43x7 champion name strips, 35x7
  action.pass), never-lifts-exempt 1 (hidden 2x2 freeze-game debug
  box, COMMAND.C:394 — not a user touch target). Aggregate
  below-floor counts pinned at 85/23/6/1 for 1x/2x/3x/4x. UI-scale
  finding pinned: zone geometry is UI-scale independent today
  (M11_UIScale has no hit-test/HUD-geometry consumer); the
  hypothetical re-audit shows UI-200 geometry would lift 22 of the
  23 sub-floor zones at 2x, so if HUD geometry ever consumes
  M11_UIScale this audit must be re-run. Remaining: CSB/DM2/Nexus/
  Nexus per-view zone tables do not exist and cannot be extracted
  today (2026-07-20 round-12 finding, job/w3): Nexus is a Sega Saturn
  gamepad title — the original SH-2 code is only staged as opaque
  capture-gated binaries (SLEV/PRS3 work in this section), the
  Firestaff input layer owns all input (docs/nexus_input.md: "The
  Nexus engine has no input handling code"), and the Nexus startup
  menu is Firestaff-authored layout, so there are no original
  cursor/click zone tables to source-lock.  Theron landed the same
  day as an implemented-geometry inventory (see below).
  2026-07-20 CSB dungeon-view zone inventory + audit landed (job/w3):
  new module pair `src/csb/csb_touch_click_zone_matrix_pc34_compat.c` /
  `include/csb_touch_click_zone_matrix_pc34_compat.h` is the CSB
  per-view sibling of the DM1 matrix — 56 source-locked zones grouped
  by view from the ReDMCSB COMMAND.C PC-media (MEDIA529/I34E)
  MOUSE_INPUT route tables the CSB lane actually executes: G0447
  primary interface (19), G0448 secondary movement (8), G0452
  action-area names (4), G0453 action-area icons (4), G0454 spell area
  (9), G0455 champion names/hands (12), with F0358 first-match
  zone+button hit-test semantics. Zone rectangles resolve through the
  shared I34E layout zone space (DEFS.H:3748-3937) via the existing
  layout-696 extraction, cross-validated against the Amiga G20E/G21E
  and Atari ST A20ED..A22G CSB literal tables in the same COMMAND.C
  (box-for-box agreement on the dungeon chrome); C147 freeze-game is
  the COMMAND.C:394 PC literal box. New CTest
  `csb_touch_click_zone_matrix_audit` pins the per-view counts,
  320x200 bounds, source-disjoint grid families, per-view hit-test
  probes incl. button masking, and runs the hit_zone_audit_m11.h
  classification cross-check through `fs_gesture_audit_zones` with
  pinned decisions (floor-at-1x 16, needs-2x 22, needs-3x 12,
  needs-4x 5, exempt 1 — the same hidden 2x2 freeze box) and
  below-floor/below-recommended counts (40/18/6/1 and 53/40/22/7 at
  1x..4x), plus the UI-scale-independence finding (2x below-min
  18/6/1 at UI 100/150/200; UI-200 would lift 17 of 18 sub-floor
  zones at 2x). Honest provenance limitation: CSB's own GRAPHICS.DAT
  (graphic 561 layout) is not staged in any permitted location, so
  per-file CSB-native rect confirmation is pending — the inventory is
  an engine-shared I34E-zone-space declaration, not a CSB-file
  extraction.
  2026-07-20 CSB route-table set completed (job/w3): the inventory
  now covers all twelve PC MOUSE_INPUT route tables — 147 zones.
  Added: G0449 champion inventory (38 routes incl. the PC-only C141
  music toggle and the C081 panel zone via the layout-696 C100/C101
  center anchor), G0456 chest panel (8), G0457 resurrect/reincarnate/
  cancel panel (3, panel-rooted at viewport-local (80,52)), G0445
  entrance (5 incl. the bonus-dungeon MASK0x0010 button route and
  the I34E-only C216 quit zone 434), G0446 restart game (2, the I34E
  MEDIA730 literal boxes), and G2045 champion rename panel (35 — the
  577-613 rename block resolved from layout-696: backspace 69x9, OK
  19x9, title 9x19, thirty-one 9x9 keys, plus the right-button
  full-screen space route). The hit-test now mirrors F0358 fully
  (CM2 zones tested minus the COORD.C G2067/G2068 viewport origin
  (0,33)). `csb_touch_click_zone_matrix_audit` re-pinned: per-view
  counts 19/8/4/4/9/12/38/8/3/5/2/35, bounds per coordinate mode,
  new disjoint grid families (backpack lines, quiver, chest, the
  34-key rename grid, the four unique entrance boxes), 17 new
  hit-test probes, decisions 21/69/51/5/1, below-min 126/57/6/1,
  below-recommended 139/126/64/42, UI-scale hypothetical 57/6/1
  (56 zones would lift at UI-200 2x). `ctest -R "csb_touch|hit_zone"`
  3/3 PASS, `-R "touch|gesture"` 31/31 PASS, zero new failures.
  Remaining: the CSB-native graphic-561 extraction once the file is
  staged.  DM2 landed 2026-07-20 (see DONE.md:
  `dm2_touch_click_zone_matrix_pc34_compat` covers the complete
  10-view SKWIN skval1.h route-table set, 421 zones,
  `dm2_touch_click_zone_matrix_audit` PASS).  Theron landed the same
  day as an implemented-geometry inventory (26 zones, V1 chrome
  320x240 + V2 HUD overlay 256x224, honest no-original-table
  provenance, `theron_touch_click_zone_matrix_audit` PASS); the
  Nexus negative finding is documented at the audit TODO above —
  remaining there is a real THQUEST.BIN disassembly before any
  original-table Theron extraction, and original Nexus input
  structures only via future Saturn capture work.
- 🔧 DM1 real Mac/release pixel promotion: HoC/render startup host ownership is verified in DONE.md; remaining work is capturing and promoting real packaged Mac/release pixels for the DM1 HoC full-graphics route.

### Accessibility

- 🔧 Screen reader / launcher-state manifest: new `m12_launcher_a11y_emit` converts the public `M12_StartupMenuState` into the same `~/.firestaff/accessibility.json` schema M11 already writes, with stable element IDs (`GAME_CARD_DM1..THERON`, `MENU_SETTINGS`, `MENU_MUSEUM`, `TAB_*`, `ROW_*`, `POPUP_*`). `M12_StartupMenu_Draw` calls it on every frame when `fs_ax_is_enabled()` is true. New `firestaff_m12_launcher_screen_reader_manifest_probe` PASS 40/40 (envelope, main view, settings, popup, ordering, scaling). `tests/test_firestaff_accessibility_manifest.c` was already on disk; wired into CMakeLists.txt as `firestaff_accessibility_manifest_unit` (39/39 PASS). Privacy: data-dir line is suppressed by default and only emitted when the caller passes `includePaths=1`. Atomic write / no `.tmp` residue verified. **2026-06-27 bestiary / item encyclopedia / screenshot gallery / museum cell-by-cell manifest landed:** new element types `FS_AX_CATEGORY_TAB`, `FS_AX_BESTIARY_ROW`, `FS_AX_ITEM_ENCYCLOPEDIA_ROW`, `FS_AX_SCREENSHOT_THUMB`, `FS_AX_MUSEUM_CATEGORY`, `FS_AX_MUSEUM_BULLET` in `firestaff_accessibility.h`; new emit functions `emit_bestiary_view` / `emit_item_encyclopedia_view` / `emit_screenshot_gallery_view` / `emit_museum_view` in `menu_startup_a11y_m12.c` plus public `M12_Museum_GetCategoryTitle` / `M12_Museum_GetCategoryPageCount` / `M12_Museum_GetBullet` getters so the museum's private static data is reachable without leaking the table. Probe now PASS 69/69 (4 new subtests H/I/J/K cover category tabs, creature rows, item rows, thumbnail rows, museum sections, and page-driven bullet content). Companion `firestaff_accessibility_manifest_unit` still PASS 39/39. **2026-06-29 M11 gameplay manifest landed and hardened:** `m11_screen_reader_update_ex()` now emits deterministic gameplay-side zones for normal play plus inventory, automap, dialog, candidate mirror, and endgame, covered by CTest `m11_screen_reader_gameplay_state_manifest`; the follow-up CTest probe `m11_gameplay_screen_reader_manifest_probe` additionally gates open-chest slots/arrow-eye labels, classifier precedence, atomic-write/no-launcher-leak invariants, redraw idempotence, and per-state bounds containment, and fixed the endgame manifest coordinate bug where plaque/mirror/portrait zones were offset by the dungeon viewport origin instead of matching the framebuffer-space victory overlay. **2026-07-01 fallthrough views navigation-anchor gate landed:** new subtest N (`subtest_fallthrough_views_navigation_anchor`) in `firestaff_m12_launcher_screen_reader_manifest_probe` (now PASS 175/175, up from 91/91) pins the documented "out of scope" fallthrough contract for the 12 launcher views that intentionally fall through to `emit_main_view` as a navigation anchor: data-validator / audio-settings / accessibility / theme / save-browser / input-remap / custom-dungeon / campaign / spell-reference / map-viewer / touch-layout / presentation-preview. For each of the 12 views, the subtest asserts (a) the envelope `gameState` matches the active view (`launcher_<viewName>`), (b) the `GAME_CARD_DM1` / `MENU_SETTINGS` / `MENU_MUSEUM` / `GAME_CARD_THERON` navigation anchors are emitted, (c) the framebuffer dims stay pinned to 480x270, and (d) no foreign view markers leak into the manifest (the forbidden-marker table covers `ROW_*`, `TAB_*`, `CHANGELOG_LINE_*`, `MANUAL_DOC_*`, `BESTIARY_CAT_*`, `BESTIARY_ROW_*`, `ITEM_CAT_*`, `ITEM_ROW_*`, `SCREENSHOT_ROW_*`, `MUSEUM_CATEGORY_*`, `MUSEUM_BULLET_*`, `POPUP_MESSAGE`, `POPUP_OK`, `POPUP_LINE*`). A future dedicated emitter for any of these 12 views only needs to assert that the new view-specific markers appear AND the fallthrough markers disappear — the navigation-anchor contract is already pinned. Remaining work is real assistive-technology smoke evidence beyond JSON shape (the remaining 12 fallthrough views still intentionally fall through to main-view emission as a navigation anchor, now with the contract machine-checked).
- 🔧 High-contrast presentation hardening: launcher and M11 chrome/viewport-fence gates are verified in DONE.md. Remaining work is real assistive/visual smoke evidence across packaged app overlays.
- 🔧 Configurable font sizing hardening: launcher `fontScale` affects M12 text rendering, and the M11 data-free `firestaff_dm1_v1_dialog_choice_font_scale_fit_probe` now covers the DM1 dialog choice source zones, the V1 message-log scale fence, and the Firestaff-specific session-timer reminder banner fit/viewport-ownership path at fontScale 1..3. The sibling `firestaff_dm1_v1_forced_pause_font_scale_fit_probe` now covers the forced-pause dialog layout/render containment at fontScale 1..3 as well (see DONE.md 2026-06-29 entries). The plain ESC return-to-menu confirmation modal now has its own scale-aware layout/draw/hit-test gate (`firestaff_dm1_v1_return_confirm_font_scale_fit_probe`); the source-owned unsaved-game quit guard remains on the DM dialog-choice path. Remaining work is broader in-game overlay/UI-fit coverage for other M11 surfaces such as rest/death overlays, inventory/map/debug panels, source-dialog unsaved-guard visual receipts, and any real-asset screenshot evidence.

### Build and CI Health

- 🔧 Watchdog parity-evidence manifests: parity-evidence files are refreshed by automated watchdog passes on every regression run. Manifests may report transient `FAIL` on gates whose line number has shifted (see the line-drift bullets above) or where a recent change has altered the test binary output; verify against the current source before treating any one FAIL as a real regression.
- 🔧 2026-07-10 release follow-up: GitHub Actions release run `29111129206` for `v3.0.71` is in progress after the tag push. Confirm all platform package jobs finish and that the published GitHub Release has the expected artifacts.

## Known Bugs

- 🔧 2026-07-18 Worktree-merge build drift (df88dbda4 + a192cb2b0) repaired:
  main builds green again end-to-end (`cmake --build build --parallel 10`,
  100%). Restored clobbered definitions/declarations (DM2 viewport
  door-frame graphicsset index, Theron `tr_asset_generated_v1_rendering_allowed`
  + `synthetic_rendering_blocked`, ReDMCSB F1007/F1008/F1017/F1018/F1020/
  F1025/F1026/F1031 source-named aliases, DM2 V2 HUD runtime
  `render_with_assets`/`last_path_*` contract, CSBWin saved-skin runtime,
  DM1 original-save decode receipt, ~40 missing header declarations) and
  re-linked ~60 test/probe targets against `dm2_v1_asset_loader.c`,
  `dm2_v1_midi_backend.c` (CoreMIDI), and `firestaff_dm2` from
  `firestaff_dm2_v2`. ctest baseline after repair: 2446 tests, ~386 failing
  (~84% pass) — dominated by DM1 Hall-of-Champions portrait-rect runtime
  probes (real logic diffs, see Jobb E lane below), CSBWin timer/DSA
  handoff tests, and asset-status zip-cache materialization. These
  failures predate the repair in the sense that main could not build at
  all; they are tracked as jobb A–G follow-ups, not new regressions.
  - 2026-07-18 CSB triage follow-up (Jobb D): five repair commits
    (6d90d4f4b, 0fbdd85e4, 4a34c06e0, f2841dd61, 2417cc122) restored
    merge-drift-clobbered CSBWin timer/ITEM16/EXPOOL runtime paths from
    a192cb2b0^ (event_is_before, unmerged timer placement/append,
    materialize_csbwin_timer_queue heap staging, ITEM16 atomic restore,
    saved-skin write-back, fnv1a receipt-gated EXPOOL lookup,
    EDT_Palette 8192-byte tail capacity), re-applied ee0df4933's
    archive-provenance retention in runtime cache materialization, and
    aligned two test fixtures with intentional post-merge contracts
    (Timer.cpp pool ownership for the TT_DOOR->TT_1 handoff, dungeon
    handle for DSA PutState persistence). 14 CSB tests back to green:
    csb_v1_csbwin_timer_restart_export,
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
    csb_archive_required_materialize_cache_gate. Verified against a
    79786f091 baseline worktree that remaining CSB failures
    (C38 combat/PARRY/leadership, F0276 sensor ordering, save-import,
    viewport gates, hint_oracle/pc_real_asset/real-data timeouts)
    predate the session — baseline 65 FAILs in
    csb_v1_runtime_tick_accumulator is down to 9. CSB lane: 339 tests,
    32 known baseline failures, zero new regressions. Remaining: the
    C38/F0276/save-import runtime lanes and real-data/timeout tests.
    2026-07-19 F0276 follow-up (job/w4): one repair commit (9ad005de8)
    restored the merge-drift-clobbered F0276 lanes from ReDMCSB
    MOVESENS.C — C001/C002 party floor cases with the F0276:1587-1620
    occupancy pre-scan and a real P0591_B_PartySquare plumbing, C001 on
    the object add/remove path (F0276:1666-1669 guard, F0238 C05..C10
    same-square merge leaves the pending CLEAR), and the C10 local
    effect F0269/F0304 steal-XP semantics (300/ChampionCount to living
    champions, C08 Steal + C01 Ninja, bounded share/8 temporary XP).
    4 CSB tests back to green: csb_v1_f0276_party_c001_sensor,
    csb_v1_f0276_party_c002_sensor, csb_v1_f0276_object_local_xp,
    csb_v1_f0276_object_chain. Known CSB failures 32 -> 28, zero new
    regressions in the csb suite. Remaining: C38 combat/PARRY/
    leadership/game-over lanes, save-import, keyboard commands, boot
    handoff, hint_oracle timeouts, real-data/timeout tests, viewport
    gates.
    2026-07-19 CSBWin resume follow-up (job/w4): one repair commit
    (8b08850cd) fixed the CSBWin 512-byte resume path. Root cause was
    twofold: (a) the shared csbwin_resume_fixture stored TimerQueue
    [2,0,1] — an ordering no original save can contain, since
    Timer.cpp CheckTimers:885-906 asserts a min-heap over the active
    prefix after every SetTimer/DeleteTimer; the fixture now stores
    the source-faithful [0,2,1]; (b) materialize_csbwin_timer_queue
    walked the full MaxTimer pool storage instead of the
    GAMEBLOCK2.NumTimer-owned active prefix (SaveGame.cpp:1867/1887/
    1906), which both tripped the intentional ce342b364 heap
    validator and would have projected free TIMER slots live. 2 CSB
    tests back to green: csb_v1_save_import_path_pc34_compat,
    csb_v1_m11_startup_resume_gate; duplicate_timer_policy,
    csbwin_timer_queue_resume and champion_bones_expool stay green.
    Known CSB failures 28 -> 26. Note: m12_quick_resume_gate and
    save_browser_export_import_m12 still fail on a different class
    (original PC34 DM1 save quick-resume lane), untouched here.
    2026-07-19 CSBWin save-import completion (job/w3): two repair
    commits (66e677eb6, cba511814) finished the CSBWin save-import
    lanes. (a) parse_timer_summaries had lost the
    timer->source_index = pool-slot stamp from cc57e9aca to merge
    drift, which made every decoded save fail the core-export
    receipt check (source_index == pool slot); restored. (b) Two
    stale tick_accumulator fixtures violated the source-locked
    pool/heap contract (SaveGame.cpp:1791-1792/1845/1852 MaxTimer
    pool + NumTimer active prefix; Timer.cpp CheckTimers:885-906
    min-heap): the summary fixture stored num_timer=9/max_timers=11
    beside three decoded timers, and the resume-file fixture stored
    the impossible heap [2,0,1] with NumTimer=2 (same defect
    8b08850cd fixed in the shared fixture); both now store the
    source-faithful counts (3/3) and heap [0,2,1].
    csb_v1_runtime_tick_accumulator flips fully green (54 CSBWin
    sub-failures -> 0: summary apply, ITEM16, timer queue, resume
    report/file, core export, native save/load tail preservation).
    (c) The multilevel DSA movement-filter fixture predated the
    b35d17974 EXPOOL fnv1a receipt contract and never stamped the
    tail receipt, so the GLOBALSTORE commit was rejected; fixture
    now stamps it. csb_v1_dsa_multilevel_filter_save_handoff green.
    Known CSB failures 26 -> 24, zero new regressions. Remaining:
    startup/presentation receipt gates (boot_title_import_ui_gate,
    startup_entrance_pointer, startup_receipt_coherence,
    m11_runtime_capture_boundary, pass547 readiness, phase7),
    completion_matrix metatest, experimental_launch_intent,
    viewport redmcsb gates, hint_oracle/real-data timeouts.
    2026-07-20 completion-matrix + launch-intent round (job/w3):
    both remaining metatests are now green. (a)
    csb_v1_completion_matrix failed because the runtime-spine test
    needles had no real backing: test_csb_v1_boot_runtime_handoff
    never exercised the accumulator tick API, and
    test_csb_v1_runtime_route_first_frame_movement_utility_gate
    never touched the low-level CSB save layer. Added real coverage,
    not text padding: the handoff test now drives
    csb_v1_runtime_tick/csb_v1_runtime_tick_due on the handed-off
    profile (one banked 55ms quantum fires exactly one V1 tick,
    wall time accumulates across both tick APIs), and the route
    test now packs the multi-step route state into a 32-byte prefix,
    writes it through csb_v1_save_header_build + csb_v1_save_game,
    and reloads it through the bounded csb_v1_load_game prefix path
    with byte-exact memcmp and untouched-tail checks. (b)
    csb_v1_experimental_launch_intent_fixture failed because
    pass874's launch-gate refactor replaced the explicit
    `intent.valid = m12_game_supported(intent.gameId) && ...` guard
    with `gate.canLaunch && ...`. gate.canLaunch already implies
    supported (gate.boot.supported is m12_game_supported), so the
    behavior was unchanged, but the intent-boundary guard was no
    longer explicit; restored it as a belt-and-braces conjunct in
    src/ui/menu_startup_m12.c. Identical 8/38 pre-existing m12
    failures verified against a correctly rebuilt baseline (stale-
    binary trap excluded), so no m12 regression. CSB suite: 21
    known failures -> 12, all remaining entries capture-blocked
    (hint_oracle timeouts x6, real-data launch/presentation/
    first-viewport x3, m11_runtime_capture_boundary, pass547,
    phase7).
    2026-07-19 DM1 V1 viewport redmcsb-gate refresh (job/w1, commit
    1d1c3cb73): all seven drifted V1 ReDMCSB source gates are green
    again — dm1_v1_viewport_status_bar_layout_redmcsb_gate,
    v1_viewport_redmcsb_draw_stack_gate,
    v1_inventory_panel_open_redmcsb_gate,
    v1_inventory_toggle_redmcsb_gate,
    v1_inventory_chest_actionhand_redmcsb_gate,
    v1_status_refresh_order_redmcsb_gate, and
    v1_entrance_input_wait_redmcsb_gate. The failures were gate-side
    staleness after M11 refactors, not runtime regressions: the gates
    pinned pre-refactor inline markers/line ranges in
    src/engine/m11_game_view.c and entrance_frontend_pc34_compat.c
    while the code had moved to shared PC34 compat helpers
    (champion_status_slotbox_pc34_compat.h 187/195 zone bases,
    dm1_v1_champion_status_* delegation, the F0115 floor-item wrapper
    m11_draw_dm1_f0115_floor_item_sprite, and the viewport projectile
    wrapper). Gates now lock the delegation chain and the current
    function spans; evidence JSONs refreshed in the same commit
    (precedent fff924d07). Same commit repairs a CMake registration
    bug: m11_dm1_floor_item_host_presentation_receipt had a dangling
    add_test inside the unrelated csb_leader_hand_no_dm1_fallback
    EXISTS block with no add_executable, so it could never run
    (permanent Not Run); it now has its own block, builds, and
    passes. Note: test_m11_csb_leader_hand_no_dm1_fallback builds but
    was never registered as a test and currently fails when run
    directly — left unregistered, needs its own investigation round.
    2026-07-20 leader_hand investigation (job/w3): the test was run
    against a fresh rebuild (stale-binary trap excluded). 16 of 169
    checks fail in two clusters: (a) the runtime overlay marker
    fallback — the draw-stats probe itself works ("exposes runtime
    overlay draw stats" and the blocking-wall hide checks pass), but
    the fallback group/floor-object markers are no longer drawn on
    non-blocking thing-list squares (expected
    object_marker_count==1/group_marker_count==1, D3R2 C3200/C2500
    scans, two-creature cells, pile shift), and (b) the STAB stamina
    write-back + SHOOT refill chain (6 checks: stamina cost back to
    runtime, mapped-input cooldown aging, C12 quiver clear, ammo move
    to runtime/M11 ready hand, pending-flag clear). Repair is real
    work in the m11 runtime overlay draw path and the STAB/SHOOT
    action chain, not a gate refresh; the test therefore stays
    unregistered so the suite does not gain a known-red entry.
    Register it (add_test in the EXISTS block at CMakeLists ~6971)
    only after both clusters are repaired.
    2026-07-20 leader_hand repair (job/w3, commit d7b4f50a5): both
    clusters repaired and the test is now registered and green —
    m11_csb_leader_hand_no_dm1_fallback PASS, 169/169 checks. Three
    root causes: (a) the overlay-marker cluster was not a runtime
    regression — M11_GameView_Draw's CSB branch is intentionally
    fail-closed without a hash-verified startup session (gates added
    2026-07-13/15, after the test was written 2026-07-05), so the
    test now draws through the CSB-owned viewport path directly via
    draw_csb_runtime_overlay_frame() and reads the config counters
    via capture_csb_runtime_overlay_draw_stats(); (b) the CSB melee
    branch in M11_GameView_TriggerActionRow applied the F0325
    stamina cost on the M11 champion but never wrote vitals back to
    the CSB runtime — fixed with
    m11_write_csb_runtime_champion_vitals() after the completion
    plan; (c) the SHOOT cooldown/refill aging chain was tick-driven
    but the CSB input path never advanced gameTick and the expired
    C11 timeline event was never consumed — the CSB bridge.mapped
    branch now advances gameTick and dispatches F0887 timeline
    events mirroring GAMELOOP.C order, and the test loop cap was
    raised 20->64 (50 source-authentic ticks). The m11_game_view.c
    additions drifted three redmcsb gate spans; tools + evidence
    JSONs refreshed in the same commit (v1_inventory_toggle,
    v1_inventory_chest_actionhand, v1_status_refresh_order;
    precedent 1d1c3cb73/fff924d07). CSB suite: 12 known failures
    unchanged.
    2026-07-19 flaky-surface characterization (the 141 vs 149 swing):
    the eight swinging DM1 tests are exactly the receipt Not-Run above
    (now permanently fixed) plus seven assert-crashes in the
    F0242/F0248/F0190/F0249 timeline-dispatch family —
    dm1_v1_square_state_dispatch_pc34_compat (asserts
    teleporter.next == THING_ENDOFLIST after F0249 chain replay),
    dm1_v1_f0248_explosion_launcher/new_object_launcher/
    square_object_launcher_runtime_pc34_compat,
    dm1_v1_f0242_fakewall_material_group_deferral_pc34_compat, and
    dm1_v1_f0190_killed_all_runtime_cleanup/moving_killed_all
    m10_handoff_pc34_compat. Their orchestrator sources are identical
    between the passing and failing baselines (verified via git diff);
    the crashes are Subprocess-abort assertion failures that flip with
    binary layout, i.e. UB/uninitialized-memory suspects in the
    timeline-dispatch thing-chain replay path, currently stuck failing
    on main (serial, 3/3). Needs a dedicated sanitizer/debug round;
    DM1 suite is 147/1337 failing after this round: 141 baseline - 1
    refreshed gate + 7 stuck crashers, and the previously swinging
    receipt Not-Run is now permanently fixed by the CMake repair
    (pre-fix current-main would have scored 155: 147 + 7 drifted
    gates + receipt Not-Run).
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
- 🔧 2026-07-09 CSB follow-up: host-view title/HUD/opening draw now consumes `render_draw`. 2026-07-11 update: release/app capture now also rejects any title, closed-door HUD, utility HUD, or door-opening route whose host consumer is not receipt-owned. 2026-07-11 update: CSB startup no longer exports raw request structs, request/render-state helper adapters, or the direct render-plan builder used only by inspection tests; those adapters are private to `csb_v1_startup_sequence_pc34_compat.c`, and tests use the CSB facts/receipt surface. 2026-07-11 update: full-runtime and complete-support receipts now prove one verified CSB session reaches PRESENTS/CHAOS/STRIKES, entrance/door, and HUD with a deterministic playback route hash. 2026-07-11 update: release/app capture now also aggregates closed-door HUD, utility HUD, and door-opening into one HUD/door capture hash and same-route gate. 2026-07-11 update: release/app capture now has a separate title-sequence capture gate/hash for PRESENTS, CHAOS zoom/hold, and STRIKES BACK, rejecting collapsed phase hashes and missing title packages. 2026-07-11 update: presented Mac/app capture now consumes both title-sequence and HUD/door aggregate receipts before accepting a captured real-asset frame. 2026-07-11 update: presented Mac/app capture now also requires release/app startup-wrapper cleanup readiness and a deterministic cleanup hash before accepting the real frame. 2026-07-11 update: presented Mac/app capture now rejects incomplete PRESENTS/CHAOS/STRIKES phase masks and HUD/door hashes collapsed into the title route. 2026-07-11 update: presented Mac/app capture now also requires the entrance credits route and rejects credits hashes collapsed into title/HUD/release routes. Remaining cleanup is adding manual Mac/app capture evidence.
- 🔧 2026-07-14 CSB real title/HUD/door capture follow-up: the staged-data M12-to-M11 launcher test now verifies C426 STRIKES BACK source bytes at their original geometry and palette, in addition to PRESENTS, CHAOS, opening-door, C017, and C040 captures. The package terminal receipt and fixture-free package probe now also require both distinct `TITLE.C F0437` CHAOS zoom and full-size hold phases before C017/C040 may be accepted. The legacy in-memory tile grid is now isolated to data-free test fixtures; live CSB resolves only loaded `DUNGEON.DAT` square and Thing records. Remaining work is manual Mac/app evidence and broader original-presentation comparison.
  - 2026-07-14 update: the same real-package M12-to-M11 route now records its actual 320x200 indexed presentation boundary for C001 PRESENTS, CHAOS, STRIKES BACK, the closed and first-opening C002/C003 door frames, and terminal C017 HUD. The production receipt admits every phase from one fully verified C001-C005/C017/C040 session, rather than incorrectly waiting for terminal HUD state before title or door capture. Each capture retains the post-draw framebuffer FNV receipt, with no synthetic surface route. Remaining work is still manual Mac/app evidence and original-capture comparison.
  - 2026-07-17 update: M11 release capture now derives its four title-phase witnesses from the active verified C001 session at PRESENTS frame 0, CHAOS zoom frame 60, CHAOS hold frame 79, and STRIKES frame 80. The prior route-derived phase-hash wrapper is retired; missing, duplicate, legacy, non-title, palette-drift, or raster-drift witnesses reject before release presentation. Remaining work is manual Mac/app evidence and broader original-presentation comparison.
  - 2026-07-31 update: the local real-data capture contract passes title,
    Entrance and first-opening-door checks. The active translocated macOS
    `Firestaff.app` v3.0.195 independently captured the four real-data
    PRESENTS/FTL/STRIKES/Entrance palette phases, but its boot-probe bypassed
    Entrance and entered runtime before the Prison pointer. It therefore
    cannot be accepted as HUD or F0807 door-opening evidence. Re-run the
    bundle-bound capture against a package built from the current source;
    do not treat the older app as proof of the corrected door frame.
  - 2026-07-31 cleanup: the unregistered title-to-HUD probe was retired. It
    duplicated the maintained PC34 real-asset launch gate while assuming an
    obsolete three-bit title mask and pre-runtime HUD handoff. The maintained
    gate remains the authoritative fixture-free route.

- 🔧 2026-07-14 CSBWin real-package resume follow-up: the opt-in package
  handoff probe now fingerprints the supplied decoded `Dungeon.dat` before
  the production resume attempt. A rejected `csbgame*.dat` must retain the
  original live dungeon bytes, owner, level, and empty runtime-save state; an
  accepted save must retain only exact serialized `TIMER`/`TimerQueue` slots
  after its first runtime tick. The probe has no generated dungeon, save, or
  fallback queue. Remaining work is a positive original CSBWin save corpus
  with broader DSA/door/world effects and package-app capture evidence.
- ✅ 2026-07-28 CSB F0115 native object composition closure: chests, scrolls,
  potions, all PC34 weapon rows, all armour rows, and all junk rows resolve
  through ReDMCSB `F0141 -> G0237 -> G0209` to native `GRAPHICS.DAT` entries
  with F0115's source mirror gate. The active M11 CSB sprite path consumes
  the native mapping and fails closed for unmapped aspects. The remaining
  CSB rendering work is broader original-capture/pixel evidence, not another
  object-family mapper.
- 🔧 2026-07-10 Nexus follow-up: remaining real-asset promotion is authentic Saturn palette/VDP1 capture comparison beyond the DGN material block. DMWeb-compatible MENU.BPK PRS3 decode and BPK/DMDF host-route receipts are verified; pixel-mode interpretation and presentation handoff remain gated.
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
  - 2026-07-14 update: descriptor-`0008` expansion now accepts only an atomic capture binding that matches complete ITEM.IBS bytes, the selected descriptor, exact packed texture span, 16-colour palette bytes, VDP1 state/command fingerprints, texture-source extent, and ordered observation sequence. No retail capture packet is supplied, so canonical ITEM.IBS remains blocked and no DGN material is promoted or drawn. Remaining work is a genuine original-Saturn VDP1 packet, including the real command's colour-mode interpretation and DGN placement.
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
  - 2026-07-15 runtime safety follow-up: a complete hash-bound Structure3
    source scene now blocks the older Structure1B/MNS raster route with an
    explicit host status. This prevents an unrelated host material mapping
    from presenting a retail DGN scene before an original Saturn trace proves
    the Structure2 pixel/palette and VDP1 relations. The block is not a
    decoder, draw route, or substitute visual.
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
  - 2026-07-15 PRS3 V5 decoder-review update: one complete stream can now
    bind its full input/output intervals and both observed low-bit branch
    outcomes to the locked SH-2 PCs. This produces a grammar-review receipt,
    not decoder readiness: authentic emulator provenance, token meanings,
    bit order, termination, pixel/palette semantics, and drawing remain open.
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
  - 2026-07-15 zero-side corridor update: the complete 64-byte SH-2 corridor
    from the zero branch target through the outer-loop branch is now locked to
    retail `DM.BIN` by FNV-1a `e0cc325e85a0e63f`. A changed unnamed instruction
    blocks trace binding just like a changed named read/merge instruction.
    This is source identity only: copy/backreference, token, palette, pixel,
    and decoder semantics remain unproven and fail closed.
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
    now retain their exact Structure1G-backed 08xx material declaration when
    selected by the referenced Structure3 face. Sequence execution, image
    decoding, palette/VDP1 semantics, transforms, and drawing still require
    an authentic Saturn trace; static and raw-fill faces cannot cross this
    animated route or gain a fallback.
  establish one original Saturn execution-to-VDP1 receipt for a named
  `(LEVxx.DGN, Structure3 entry, face ordinal)` from the hash-verified retail
  corpus. The receipt must bind the exact typed mesh-row corpus identity
  (`d3f42b1f` across LEV00--LEV15), the selected `00xx`/`08xx` fill selector,
  the consumed texture-byte span and palette/VDP state, the vertex transform,
  normal/culling decision, and the emitted VDP1 command. `FACE.BIN` is a
  separate asset until that trace proves an explicit relation; no FACE.BIN
  pixels, selector interpretation, palette semantics, coordinate convention,
  or fallback visual may be inferred. Promote real DGN geometry only after
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
  hash-verified retail LEV sources now expose the documented
  Structure1F -> Structure1A -> Structure3 face/normal ordinal attachment at
  the engine boundary. The remaining blocker is original Saturn evidence for
  transforms, materials, texture/palette decoding, VDP1 commands, and draw;
  this receipt remains strictly no-draw with no fallback visuals. The direct
  LEV00--LEV15 corpus now also has a source-locked Structure1F directory
  receipt: it rehashes each canonical direct file and requires the parser's
  final Structure1F span, six count-derived fixed-size family spans, and their
  `0x10/0x11/0x12/0x20/0x21/0x22` source tags to agree byte-for-byte. This
  establishes record boundaries and raw identity only. The documented first
  `0x10` family is now additionally typed only through its proven eight-byte
  row contract: byte 1 is `x`, byte 2 is `y`, and `y * 64 + x` is a bounded
  cell ordinal; bytes 3--7 stay opaque. Cross-level, package, row-tag, and
  row-index drift reject. This still does not prove an item role, or which
  family record is a visible face, mesh, transform, material, texture or host
  placement, so no M11 face route is authorized yet.
  The next cell-near family, `0x11`, has a source-locked twelve-byte typed raw
  admission: bytes 1--2 are the documented direct coordinate pair and bytes
  3--11 retain exact source order only. Its model/aspect, rotation, control,
  extent, face, mesh, and transform relations remain unproved and no draw is
  granted. The local LEV00--15 retail corpus is currently absent, so its
  cross-level real-data probe remains skip-safe; only fixture-level span,
  package, tag, index, and cross-level rejection is verified. Re-run the
  direct corpus probe before promoting this item to DONE or binding it to an
  M11 face/mesh route.
  The same absence was confirmed by direct ordinary-file searches for
  `LEV00.DGN` and `DM.BIN` under `/Users/bosse/.firestaff/data`,
  `/Volumes/Extern-disk`, and the project tree, including visible symlinks;
  there were no candidate bytes to hash or admit. The next direct-coordinate
  `0x12` family now has the matching 16-byte typed raw gate: bytes 1--2 are
  coordinates, bytes 3--15 stay opaque, and every control, destination,
  model, face, mesh, material, transform, and draw claim is closed. Its
  fixture verifies source hash, descriptor span, record index, and cross-level
  rejection; it too remains outside DONE until a direct LEV00--15 probe is
  genuinely positive.
  The first Structure1A-bound candidate, `0x20`, now has a 12-byte typed raw
  admission that retains only its parser-documented byte-1 face selector and
  big-endian byte-2--3 Structure1A index, plus an opaque byte-4--11 tail.
  Fixture coverage proves source hash, descriptor bounds, row-index and
  cross-level rejection. It does not establish that the selector names a
  Structure3 face or that the index locates an alcove, portal, owner, model,
  mesh, transform, material, or draw target. Keep real DGN rendering blocked
  until the missing direct corpus verifies this family across LEV00--15.
  The related `0x21` wall-decoration family now has its own strict 12-byte
  raw admission, independently requiring the parser directory's `0x21` tag,
  exact span/FNV, byte-1 raw face selector, and big-endian byte-2--3
  Structure1A index. Its raw tail remains uninterpreted. Fixture coverage
  rejects cross-level identity reuse, row-index overflow, and descriptor-span
  drift. It does not prove a wall, portal, face ordinal, owner, model, mesh,
  transform, texture, material, or draw action, and remains outside DONE until
  a direct LEV00--15 corpus probe passes.
  Structure1A target intake is now parser-bound for those `0x20`/`0x21`
  references: it reparses the same hash-verified DGN, requires the existing
  header-counted 24-byte Structure1A table, rechecks the source row's tag,
  span/FNV and big-endian index, then retains only the indexed target row's
  offset/length/FNV. Fixture coverage exercises the full parser path plus
  cross-level, out-of-range-index and source-FNV rejection. No Structure1A
  target byte has face, mesh, material, transform, texture or draw semantics;
  the missing LEV00--15 corpus still blocks real-data promotion to DONE.
  The target row's first three bytes now have the narrowest positive typing:
  byte 0 is retained as opaque kind, byte 1 as the documented Structure3 model
  index, and byte 2 as an opaque rotation selector; bytes 3--23 remain an
  exact raw tail. The field receipt requires the prior target-row FNV and
  current source rehash, then rejects level or target-FNV drift. A model index
  is not a parsed Structure3 mesh or face selection, and neither the kind nor
  rotation byte has admitted behavior. Retail LEV evidence is still required
  before any target relation can leave TODO or approach rendering.
  The typed Structure3 model index now reaches a strict opaque directory target
  admission. It requires the current package rehash plus an explicit FNV of
  the counted, strictly increasing Structure3 directory before exposing one
  selected entry span and its FNV. Fixture coverage proves valid selection and
  rejects directory, directory-FNV, and package drift. The selected span is
  not an entry grammar, mesh, face, texture, material, or draw contract; no
  retail corpus result exists to promote it to DONE.

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
  LEV now yields a renderer-facing no-draw packet containing one exact typed
  Structure3 face, its referenced original 16.16 vertex rows and paired
  normal, joined to the same hash-bound static Structure2 descriptor and
  opaque image/palette anchors. This is real package geometry, not a surface:
  transform/camera, pixel span/order, palette format/addressing, VDP1 command,
  UV, culling, and raster semantics still need independent original-Saturn
  capture proof before any draw or decoder can be enabled.

  The DGN viewport now consumes that package geometry as a separate no-draw
  source lane before its material/raster gate. It is deliberately not an M11
  presentation route: no authentic Saturn pixel/palette/VDP1 format proof is
  available, so the packet cannot produce a host frame or unblock rendering.

  The active viewport now traverses every static-textured Structure3 face with
  an exact Structure2 descriptor anchor through its renderer consumer. Dynamic
  and otherwise unproven faces remain outside that route. Traversal is still
  source-only: it cannot infer transforms, texels, palettes, UVs, VDP1 state,
  culling, or a host draw from the package bytes.

  Texture-flaggade `08xx` Structure3-ansikten kan nu bindas till sin verkliga
  Structure1G-deklaration, dess fullständigt begränsade instruktionssekvens och
  första lokala Structure2-descriptor. Detta ger inte rätt att välja eller
  avancera en animationsframe och bevisar inte pixelspan, palett, VDP1, UV,
  transform eller draw. En autentisk Saturn-trace måste visa dessa steg innan
  animationen får exekveras eller synas.

  Viewporten traverserar nu samtliga aktiva `08xx`-ansikten genom den
  källbundna Structure1G/Structure2-rutten. Att traversera en sekvens är inte
  att exekvera den: den förblir no-draw tills en autentisk Saturn-trace binder
  frameval, timing, pixelspan, palett och VDP1-kommandon till samma källa.

  De återstående icke-texturerade Structure3-ansiktena traverseras också i
  viewporten med råa fill-selector-bytes bundna till exakt geometri. De bytes
  har ännu ingen bevisad flat-färg-, blend-, palett- eller VDP1-betydelse och
  kan därför inte ritas innan en autentisk Saturn-capture etablerar den.

  Den aktiva DGN-rutten kräver nu full facekategori-täckning från samma
  hashbundna LEV innan en framtida Saturn-renderer får betrakta scenen som
  komplett. Denna gate är endast källgeometri: verkliga pixel-, palett-, VDP1-
  och transformbevis saknas fortfarande och all presentation är fortsatt
  blockerad.

- 🔧 2026-07-15 Nexus Structure2 pixel/palette format gate: the real retail
  corpus has only raw descriptor classes `0x0008` (1,553 rows) and `0x0028`
  (125 rows). Every image target is an in-payload anchor; `0x0028` has no
  palette anchor while `0x0008` has a mixed present/absent palette split. This
  rules out selecting an indexed or direct-color decoder from descriptor bytes
  alone. A genuine Saturn trace must still establish pixel span/order, absent
  palette behavior, palette entry format, and VDP1 command mode.
  - 2026-07-16 update: Structure2 descriptor admission now fails closed before
    selector binding if a descriptor uses an unobserved encoding class or a
    zero width/height. The parser still accepts only the observed
    `0x0008`/`0x0028` source classes and keeps the post-FFFF payload opaque:
    no pixel span, palette format, VDP1 mode, decoder, draw route, or fallback
    visual is inferred from these fields.

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
- 🔧 2026-07-15 Nexus complete material source gate: the complete DGN receipt
  now requires every active Structure2 descriptor to retain a bounded image
  payload anchor from the canonical LEV, with each nonzero palette anchor
  retained separately. These are capture bounds only, not proven image spans,
  palette entries, pixel format, VDP1 mode, decoder input, or drawing. An
  authentic Saturn trace must establish those semantics before promotion.
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
- 🔧 2026-08-08 Theron JP item-property provenance: the loader must not apply
  the authenticated US 66-entry property table to JP Track 02. JP ground
  records remain real and decoded, but JP property binding and its T900
  consumer stay blocked until the JP table offset and semantics are proven.
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
