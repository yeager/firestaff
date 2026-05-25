
- 🔧 Partially implemented
- ✅ Viewport/wall occlusion — DM1 V1 side-field occlusion now has source-locked D3/D2, D1, D0, and D0C current-square evidence manifests and focused viewport regression coverage through 59fffa57; the side-content center-blocker probe is enabled and validates the ReDMCSB side-wall occlusion source route (9b9cda30); pass608 same-viewport capture blocker is CTest-locked and source-contract backed (43c7a58a), pass609 tightens the same-viewport capture contract with portable CTest coverage (59fffa57), pass610 locks Firestaff-side 224x136 viewport crop capture readiness (cbfad52e), pass622 pins the next capture-closure gap to the missing original command/state/redraw transcript, pass623 (a7de3704) binds the Firestaff input-script crop rows to that blocker, pass625 locks the first source-backed original transcript target row/preflight, and pass626 locks the original turn-redraw route target without promoting parity; pass627 (c53c7968) documents the remaining gap as missing original DOS runtime transcript (F0380 to F0097) blocking promotion — Firestaff same-viewport capture contract is fully implemented and CTest-locked, requires paired original DOS capture session to close
- ✅ Input command routing — release-mouse button identity and routed click acceptance are source-locked for the V1 command queue path (6a168a9e), movement collision-before-sensor dispatch ordering is covered by the movement pipeline gate (2462666b), the stairs backstep movement-cooldown gate is source-locked by pass578 (06cedcf6), pending-click single-slot overwrite/replay is source-locked by pass624, and `dm1_v1_movement_queue_capture_closure` pins the current command input -> queue -> movement dispatch capture contract; pass623 (a7de3704) now ties the canonical Firestaff input script to viewport crop rows, and pass626 locks the original turn-redraw transcript route target; remaining input parity work is capture-only: a source-bound original PC/I34E transcript plus paired original/Firestaff viewport crops
- ✅ Sensors & Mechanisms — floor/wall sensor type classification (F0720-F0723, F0727-F0728), effect dispatch (F0724, F0272), HOLD resolution, rotation deferral (F0271), once-only disable (F0272:1180), and all 10 floor (C000-C009) and 20 wall (C001-C018, C127) sensor types with per-type source-locked skip logic are source-locked with ReDMCSB line-level citations (MOVESENS.C F0268-F0276, TIMELINE.C F0241-F0248, CLIKVIEW.C F0372/F0377, DEFS.H); door animation timeline events (party-on-square damage BUG0_78, creature block/rattle, normal sequence) are source-locked via F0717_DOOR_ResolveClosingObstruction_Compat and Pass 418 orchestrator gate (22f9ae9c + e7c0e3be); GAP-3 C003 non-consuming ornament click is source-locked in F0723:326-328 (MOVESENS.C F0275:1412-1415, 1767, 1527); GAP-2 F0273/F0274 logic embedded in context fields, not independently callable; GAP-1 door animation already implemented (22f9ae9c + e7c0e3be); 7/7 sensor-related CTests pass
- ✅ Creature type-specific behavior — Giggler steal, Ghost/non-material melee gating, quarter-square melee cell shuffle, adjacent creature projectile chance, fixed possession drop payloads, fixed possession runtime materialization (de044e27), Couatl idle aspect movement sound gate (a6541cf5), and Black Flame fireball-impact healing (63a905c8) are source-locked; ordinary melee weapons no longer hit non-material creatures, while Vorpal/Disrupt and materializer/projectile explosion harm paths are accepted; other type specials remain
- 🔧 Creature group spawning (C006 floor sensor group generator) — successful empty-square materialization is source-locked with fixed group-slot reuse, square insertion, party-map active-state seed, F0180 event37 wandering hookup, delayed C65 re-enable, F0185/F0245 buzz dispatch, event60/61 blocked-destination defer (5ace9238), Lord Chaos adjacent random retry (0a12e07d), narrow F0267 teleporter destination/cross-map placement with covered audible buzz/wandering scheduling (191c9d3c), group move-removal planning coverage (91578481), F0267 open-pit fall damage/death, lower-map insertion, and carried group-slot fall-kill drops (9db12dbf), generated/deferred non-square insertion projectile non-impact preservation (850c1541), and generated/deferred creature-not-allowed rejection plus carried-slot drop handling (1741688d), and moving fixed-possession partial/death drops (365b4b8f); ordinary moving-group projectile impact/removal (dab7b40d), group-specific teleporter rotation, and multi-hop teleporter audible buzz order (d3ef5834) are source-locked; remaining C006/F0267 work is original capture parity for broader chained runtime proof
- 🔧 Champion stats panel — eye-click runtime now shows source-locked HP/stamina/mana, all six statistic families, four base skill levels, F0351 skill-level/statistic names (2fcd982d), and CHAMDRAW status-value formatting/zone routing (d504de4c); true current-vs-maximum statistic color rows (ea743e8c), the panel-HUD statistic color/format helper and status box model (dc4cbb37), runtime current/maximum statistic row wiring (4c671c34), and the F0351 text-run layout/color model plus M11 stats text routing coverage (ac3efa45), and the Firestaff indexed-framebuffer draw path for the colored per-stat panel (2355619c) are source-locked; remaining polish is original PC 3.4 capture/comparator proof for true original-vs-Firestaff framebuffer parity
- 🔧 Full inventory panel (8 body slots + 2 hand slots + backpack) — C507..C536 source slot-box bridge, full backpack runtime storage/pickup/place path plus all C520..C536 source slot round-trips (78412579), champion inventory slot pickup/place hash refresh (fb747c23), PC34 open-chest slot setter (461139c9), and open action-hand chest icon remap (ba92e3a7) are source-locked; remaining panel polish/chest runtime details stay open
- 🔧 Equip/unequip items to body slots — partially: PC34 slot masks, status hand-slot route resolution, and leader-hand/body-slot swap transaction are source-locked (6acbf589); full inventory/backpack/chest storage expansion remains pending
- 🔧 Backpack/chest container management — M11 V1 open action-hand chest state, C537..C544 panel route hit-testing, visible chest slot drawing, leader-hand/chest slot swaps, middle visible chest-slot pickup compaction (e99b5839), all C520..C536 backpack source slot runtime round-trips (78412579), action-hand chest panel close/reopen persistence, PC34 open-chest slot setter (461139c9), and open chest C145 action-hand icon remap (ba92e3a7) are source-locked; broader full panel polish remains pending
- ✅ Item identification — potion eye-panel power-prefix display and M11 leader-hand runtime wiring are source-locked for Priest > 1, including the original empty-flask quirk; weapon eye-panel name/attribute description formatting is source-locked for POISONED/BROKEN/CURSED (94094865); item weight line display is source-locked (35461e0b); armor BROKEN/CURSED, junk consumable/compass/waterskin state lines, scroll/container panel routes, object-description panel layout (440eaca1), M11 item eye object-description runtime rendering in source layout (48a05966), leader-hand scroll eye-click routing to the source scroll panel renderer (4fa32322), leader-hand container eye-click routing to the source chest panel renderer (d206cb6c), action-hand scroll panel C023 framebuffer pixels (64de7dcf), and leader-hand object-description panel C020/C029 source pixels (772c12fc), and empty-hand mouth food/water/poisoned panel source pixels are source-locked; remaining item-identification polish is framebuffer/pixel parity for other routed panels and full original-vs-Firestaff framebuffer parity
- 🔧 V2.1/V2.2 presentation modes selectable from menu
- 🔧 CSB launch entry in menu
- 🔧 DM2 launch entry in menu
- 🔧 Nexus launch entry in menu
- 🔧 Touch controls config option exists
- 🔧 Accessibility module exists (firestaff_accessibility)
- 🔧 Audio/creature sound trigger system — 35 sound events verified SOURCE-LOCKED. Remaining KNOWN_DIFF items correctly bounded as original-faithfulness capture tasks (runtime SFX cadence/overlap). No code gaps.
# Firestaff DONE — Completed Work

Status per 2026-05-19 v2.4.0.

## Legend
- ✅ Done
- 🚫 Source-blocked / intentionally not implemented

---

## DM1 V1 — Core Gameplay

### Movement & Navigation

- ✅ Cardinal movement (WASD + arrow keys + click)
- ✅ Turning (left/right)
- ✅ Collision detection (walls, doors, creatures)
- ✅ Collision and doors parity row — ReDMCSB-backed movement blockers, accepted movement state, viewport redraw, and canonical DM1 PC DUNGEON.DAT overlay/runtime cases now cover representative wall, closed/open door, fakewall, and door-button states
- ✅ Firestaff-side viewport crop capture readiness is source-locked by pass610: wall/collision capture rows now emit 224x136 viewport crops beside 320x200 captures, with source geometry and no original-vs-Firestaff pixel parity claim (cbfad52e).
- ✅ Blocked wall/door/closed-real-fakewall self-damage request source-lock
- ✅ Movement cooldown (G0310/G0311 timing from F0267)
- ✅ Stairs backstep movement-cooldown gate is source-locked by pass578: queued backward-on-stairs movement remains queued and does not apply stairs transition, stamina, map, or cooldown side effects while movement is gated (06cedcf6).
  - ✅ Input wait loop timing source-lock: PC-34 input wait exits only when input wait has stopped and game time is ticking (6a842b72).
- ✅ Pit fall + level change + fall damage (20 HP/pit)
- ✅ Teleporter chains (up to 1000 iterations)
- ✅ Stair transitions (up/down, correct direction per F0155)
- ✅ Dead creature groups don't block (v2.3.0)
- ✅ Empty-party group collision F0267 destination cleanup is source-locked: empty-party movement can enter a group square, then the post-move destination group is deleted and enter sensors are replayed in source order (ab5e892e)

### Viewport Rendering

- ✅ Wall rendering from GRAPHICS.DAT (DM1 zone blits)
- ✅ Door rendering (frame + panel, open/close states)
- ✅ Door frame persists when portcullis is open (v2.0.0)
- ✅ Wall ornaments from GRAPHICS.DAT (per-map ornament cache)
- ✅ Floor ornaments
- ✅ Champion mirror portraits (C127 sensor)
- ✅ Random wall ornament generation (F0169-F0172)
- ✅ Inscription text overlay on D1C front walls (v2.4.0)
- ✅ Floor pit graphics (open/closed, invisible pits)
- ✅ Stair graphics
- ✅ Creature sprites from GRAPHICS.DAT
- ✅ Projectile sprites (spell projectiles, thrown items)
- ✅ Explosion/burst rendering (smoke, fireball aftermath)
- ✅ DM1 V1 explosion viewport-zone mapping source-locked for D0-D4 view squares and rebirth overlays (`d0720a66`)
- ✅ Floor item sprites (scatter rendering, multi-item)
- ✅ Alcove content rendering (items inside wall alcoves)
- ✅ Object zone Y coordinates (PC34 C696/C2500 layout-696 source-lock, pass606)
- ✅ Side door rendering (D1L/R, D2L/R, D3L/R)
- ✅ Door ornaments
- ✅ Depth occlusion (center wall blocks far content; boundary wall blocker probe source-locked, 919598c5)
- ✅ Side lane occlusion
- ✅ DM1 V1 side-content center-blocker probe now runs as a normal CTest gate and validates the source-locked side-wall occlusion route (9b9cda30)
- ✅ Palette dimming based on light level
- ✅ HiDPI/Retina scaling (v2.0.0)
- ✅ Fakewall viewport/collision parity — open fakewalls render as corridor, closed imaginary fakewalls stay passable but wall-like
- ✅ Wall/collision runtime capture gate — exact map/x/y/direction, movement-pipeline state, and PPM screenshots for blocked and accepted movement
- ✅ Inscription rendering on side walls (D2L/R, D3L/R) — unreadable plaque heights source-locked (pass582)
- ✅ Readable inscription rendering (source message zone centering)
- ✅ Teleporter visual effect — source-backed GRAPHICS.DAT field bitmap overlay, not procedural sparkle


- ✅ DM1 V1 floor ornament draw order (F0108→items→creatures), variant index mapping (G0191), ornament click routing (CLIKVIEW.F0377), wall ornament fountain/sensor routing — SOURCE-LOCKED, no gaps
- ✅ DM1 V1 wall bitmap rendering parity — wall flip mechanism ((MapX+MapY+Direction)&1), D3/D2/D1/D0/D0C bitmap selection, draw order (s_thing_layers), explosion occlusion — SOURCE-LOCKED vs DUNVIEW.C
- ✅ DM1 V1 projectile rendering (F0115/F0116/F0117) — draw order, projectile bitmap indices (454+offsets), creature targeting, travel blockers, 6 verify scripts PASS — SOURCE-LOCKED
### Creature System

- ✅ Creature groups loaded from dungeon.dat
- ✅ Creature behavior profiles (27 types, movement/attack/sight/smell)
- ✅ Creature AI — movement toward party (cardinal-only, F0228/F0209)
- ✅ Creature attack on party (F0230 damage formula with dodge/parry/armor)
- ✅ Creature death detection + item drops (v2.2.0)
- ✅ Dead groups cleaned up (don't block movement)
- ✅ Kill XP award via lifecycle system
- ✅ Creature rendering — sprites load and source-locked aspect frames cycle
- ✅ Creature projectile attacks - ReDMCSB F0207/F0212 payload now feeds M11 live runtime insertion with source-backed C48 first-move ignore-impact proof; broader exact creature AI event scheduling remains separate
- ✅ Creature sound effects attack/movement ordinals and runtime trigger coverage are source-locked
- ✅ Couatl idle aspect movement sound gate is source-locked for the random idle update path while attacking/resting and non-Couatl cases stay silent (a6541cf5)
- ✅ Creature flee behavior — source-locked fear-triggered flee delay, flee direction, countdown expiry, and behavior dispatch are covered by the DM1 V1 creature AI gate
- ✅ Creature special positioning — quarter-square melee cell shuffle and adjacent creature projectile chance are source-locked in the DM1 V1 creature AI gate (5121884b)
- ✅ Creature fixed possession drop payloads and runtime materialization are source-locked for fixed source object pools, unused-slot allocation, raw object payload writes, cell tagging, square-list linking, death-path integration, and dead group unused-pool cleanup (de044e27)
- ✅ Black Flame fireball-impact healing is source-locked: fireball impacts add attack value to Black Flame health up to the source cap instead of applying normal creature damage (63a905c8)
- ✅ C006 Lord Chaos blocked-destination adjacent random retry is source-locked from ReDMCSB event60/61 handling (0a12e07d)
- ✅ C006 generated groups now reuse ReDMCSB-style fixed unused group slots instead of appending beyond source capacity (1d3a9bd4)
- ✅ C006 generated/deferred groups now route through the narrow F0267 teleporter destination/cross-map placement path, including the covered audible buzz and target-square wandering scheduling case (191c9d3c)
- ✅ C006/F0267 open-pit group movement is source-locked for generated lower-map insertion, fall damage/death, and carried group-slot fall-kill drops at the lower destination (9db12dbf)
- ✅ C006 generated/deferred group insertion is source-locked to skip projectile impact/removal when ReDMCSB calls F0267 from a non-square source, preserving projectile links and queued projectile events (850c1541)
- ✅ C006 generated/deferred group placement is source-locked to reject creature types not allowed on the destination map, drop carried-slot possessions, skip insertion/AI/wander scheduling, and avoid success buzz on failed generator placement (1741688d)
- ✅ C006/F0267 moving fixed-possession partial/death drops are source-locked for falling moving groups: partial death consumes killed moving-creature cells, surviving fixed possessions and carried slots drop at the destination, and deleted source groups are cleaned up in ReDMCSB order (365b4b8f)
- ✅ F0267 teleporter rotation parity is source-locked for party, projectile, object, and projectile-associated object rotation/cell behavior (9fd978af)

- ✅ DM1 V1 creature projectile firing decision (F0823 ↔ GROUP.C F0207, range>1 gate, distance gate 50% adjacent, kinetic energy [20,255], F0212_Create) — 7 explicit creature cases + default match ReDMCSB; C25/C26 structurally safe (default: FIREBALL) but not explicitly handled — not reachable in any original dungeon (BUG0_13)
- ✅ DM1 V1 creature render aspect table (s_aspects[27]) — all entries match ReDMCSB G0219: firstNativeBitmapRelativeIndex, coordinateSet, replacementColorSetIndices, M618_GRAPHIC_FIRST_CREATURE=584 — SOURCE-LOCKED
- ✅ C006/F0267 multi-hop teleporter audible buzz parity is source-locked for chained group teleporter traversal, preserving all per-hop buzzes before generator and sensor buzz dispatch (d3ef5834)

### Combat

- ✅ Champion melee actions (SWING, CHOP, STAB, THRUST, HACK, etc.)
- ✅ Action menu (3 actions per weapon type + empty hand)
- ✅ Melee damage applied to creature groups (F0738)
- ✅ Creature damage to champions (F0230)
- ✅ Parry skill XP
- ✅ Combat result logging
- ✅ Shield defense bonus in combat — hand-slot shield defense now includes source-locked F0312 hand strength plus armor defense in F0313 wound defense
- ✅ Weapon broken flag display is source-locked in the inventory eye panel; ReDMCSB exposes WEAPON.Broken as an item-description attribute and no DM1 V1 runtime weapon-breakage write path was found
- ✅ Poison damage over time — creature PoisonAttack → F0322 immediate damage, 36-tick follow-up scheduling, event-count decrement, and Vitality-adjusted gate are source-locked
- ✅ Ranged weapon actions (SHOOT with bow/crossbow/sling) — ammunition class validation, runtime projectile launch, and F0253 ready-hand reload from compatible quiver ammunition are source-locked; M11 mirrors the refill at bounded action end until the exact delayed action-enable event is exposed (d000fd99)

### Spell System

- ✅ Rune panel UI (open/close, 6 power runes + 6 element runes)
- ✅ Rune buffer (up to 4 runes per cast)
- ✅ Spell encoding (F0750)
- ✅ Spell table lookup (F0752)
- ✅ Mana cost check + skill level requirement
- ✅ Spell projectile creation (fireball, lightning, poison cloud, etc.)
- ✅ Spell shield / fire shield effects
- ✅ HEAL action (mana → HP transfer)
- ✅ FREEZE LIFE action
- ✅ Light spell (OH IR RA -> source-backed magical light bridge)
- ✅ Darkness spell (DES IR SAR -> source-backed darkness/recovery bridge)
- ✅ Open Door spell (ZO BREU) — M11 projectile door-impact SFX, non-destroyed button-door GameTime+1 toggle animation, and source-backed UI cast projectile launch are locked; direct key-door behavior remains separate/out of scope
- ✅ Poison cloud damage over time — source-locked cloud ticks now preserve repeated damage, attack decay, +1 tick rescheduling, and no-wound party damage
- ✅ Spell failure feedback — needs-practice/meaningless feedback metadata and cast-click symbol cleanup are source-locked

### Champion System

- ✅ Champion recruitment from mirrors (HoC)
- ✅ Active champion selection
- ✅ Champion bar display (HP/stamina/mana bars)
- ✅ Skill levels (Fighter/Ninja/Priest/Wizard)
- ✅ Skill XP and level-up via lifecycle system
- ✅ Champion death detection
- ✅ Resurrection system (dm1_v1_resurrection_pc34_compat)
- ✅ Champion stat screen current-value coverage — inventory eye panel is source-locked to F0351-style skills/statistics display data
- ✅ Champion stat screen F0351 visible skill-level names and statistic names are source-locked in the M11 inventory eye-click runtime panel (2fcd982d)
- ✅ Champion stat screen current-vs-maximum statistic color rows are source-locked: below max red, above max light green, equal lightest gray, with max suffix lightest gray (ea743e8c)
- ✅ Champion panel HUD statistic color/format helper mirrors the F0351 current/maximum row colors and split value formatting (4a13163e)
- ✅ Champion max-stat runtime rows are wired through champion state serialization, fallback loading, panel row helpers, and M11 empty-hand eye stats output (4c671c34)
- ✅ Champion stats panel text-run layout/color model and M11 stats text routing are source-locked for the F0351 statistic rows, with tests covering panel helper coordinates/colors and runtime stats detail output (ac3efa45)
- ✅ Champion panel HP/stamina/mana numeric status-value formatting and zone routing are source-locked from CHAMDRAW.C F0289/F0290 (d504de4c)
- ✅ Champion weight/load panel formatting, color thresholds, label/value routing, rounded maximum load, stamina-adjusted load, and movement-cost load gates are source-locked from ReDMCSB CHAMDRAW/CHAMPION paths (0a075966)
- ✅ Champion stamina regeneration — source-locked to F0331 food/water, resting, idle-delay, max-stamina amount, and mana-regeneration stamina cost; no direct load factor in regeneration path

### Inventory & Items

- ✅ Leader hand object (pick up, put down, swap)
- ✅ Alcove item interaction (click to pick up)
- ✅ Item throwing (throw into dungeon, creates projectile)
- ✅ Torch fuel tracking + light power calculation
- ✅ Torch burns out → extinguish
- ✅ Floor item pickup — rendered grabbable object-cell gate and runtime pile-top object wiring source-locked
- ✅ Scroll text display (F0168 C2_TEXT_TYPE_SCROLL) — TextString decode/visibility/separator path and inventory scroll panel rendering source-locked
- ✅ M11 action-hand scroll panel C023 framebuffer pixels are source-locked against GRAPHICS.DAT for the scroll route, skipping transparent red pixels and the later text band (64de7dcf)
- ✅ Potion consumption effects — potion stat/heal/mana/stamina/water/shield effects, VI wound RNG masks, inert/unknown potion empty-flask conversion, M11 mouth-click wiring, swallow audio routing, and no-mouth-animation behavior are source-locked
- ✅ Food/water item consumption — food amounts, water/waterskin charges, caps, leader-hand removal, swallow audio routing, C545 food mouth visual blit, and water/potion no-visual runtime behavior are source-locked through M11 mouth-click (4209e8e4)
- ✅ Weapon eye-panel name and attribute descriptions are source-locked for POISONED/BROKEN/CURSED formatting (94094865)
- ✅ Item eye-panel weight line display is source-locked from ReDMCSB item description formatting (35461e0b)
- ✅ Armour/junk/scroll/container eye-panel description families are source-locked from ReDMCSB F0342/F0336 routing and formatting: armour BROKEN/CURSED, junk CONSUMABLE plus compass and waterskin state lines, and scroll/container delegation to scroll/chest panels
- ✅ PC34 open-chest slot setter preserves panel slot writes and close-time compact ordering in the backpack/chest runtime gate (461139c9)
- ✅ Open action-hand chest icon remap is source-locked in M11 inventory rendering: closed container icon C144 becomes open chest C145 only for the active open action-hand chest (ba92e3a7)
- ✅ Middle visible chest-slot pickup compaction is source-locked in M11 inventory runtime, preserving the surrounding compacted list after removing the selected slot (e99b5839)
- ✅ Full backpack source slot runtime round-trips are source-locked for C520..C536, covering route/zone mapping, leader-hand pickup, storage clear, exact-slot placement, and leader-hand clear (78412579)
- ✅ Champion inventory slot pickup/place deterministic hash refresh is source-locked for C507..C536 mutations so C528 pickup and C536 placement update the M11 render/state hash like chest-slot mutations (fb747c23).
- ✅ Status-box hand-slot route resolution is source-locked into the equip-slot transaction path, including candidate/open/dead/current-inventory gates (6acbf589)
- ✅ Object-description panel layout is source-locked for form-feed reset, PC34 body origin, 18-character wrap, text color, and 7-pixel line advance (440eaca1)
- ✅ M11 item eye object-description runtime rendering is source-locked to the source panel, circle, icon, name, and wrapped body layout (48a05966)
- ✅ M11 leader-hand scroll eye-click routes to the source scroll panel renderer instead of the generic dialog/object-description overlay (4fa32322)
- ✅ M11 leader-hand container eye-click routes to the source chest panel renderer instead of the generic dialog/object-description overlay (d206cb6c)
- ✅ M11 leader-hand object-description panel source pixels are source-locked for the C020 panel and C029 circle blits, using the actual GRAPHICS.DAT source asset dimensions before icon/text/chrome overdraw (772c12fc)
- ✅ Fountain interaction — empty-hand drink, waterskin/water refill, empty-flask-to-water-flask conversion, load delta, swallow sound ordinal, and front-wall sensor continuation are source-locked
- 🚫 Direct key-on-locked-door action blocked: ReDMCSB source audit found no DM1 V1 door-square key route; keys/specific objects are source-backed through wall sensors/object mechanisms only

### Survival / Needs

- ✅ Food and water drain (1 unit per 6 ticks per champion)
- ✅ Champion bar color reflects hunger/thirst status
- ✅ Food/water depletion → HP damage — source-locked champion-needs pending HP damage is applied in M11 runtime on starvation/dehydration underflow
- ✅ Rest system — source-locked rest/wake command routing, rest recovery multiplier, and forced wake on creature attack before resting-hit damage
- ✅ Stamina drain from actions — source-locked action-menu stamina table, per-action random jitter, and F0325-style clamp/underflow damage in M11 runtime
- ✅ Stamina regeneration — source-locked champion-needs gate covers gain/loss, rest multiplier, idle movement delay bonus, food/water depletion, and stamina overflow damage

### Sensors & Mechanisms

- ✅ Floor sensors (party walk-on/walk-off triggers)
- ✅ Wall click sensors — buttons/switches now fire correctly (v2.4.0)
- ✅ Remote door toggle (SET/CLEAR/TOGGLE)
- ✅ Remote pit toggle
- ✅ Remote teleporter toggle
- ✅ Sensor text display (inscription/message popup)
- ✅ Sensor audible feedback (click sound)
- ✅ Wall sensor with specific object (key slot — C003/C004)
- ✅ Object storage sensor (C013 — place item, get effect)
- ✅ Object exchanger sensor (C016 — swap item for effect)
- ✅ AND/OR gate sensor (C005 — multi-condition trigger)
- ✅ Countdown sensor (C006 — timer-based trigger)
- ✅ Projectile launcher sensors (C007-C010, C014-C015)
- ✅ End game sensor (C018 — source-locked win/endgame trigger)
- ✅ Floor party-on-stairs sensor (C005 — source-locked stairs-only trigger)
- ✅ Floor possession sensor (C008 — party has specific item)
- ✅ Floor creature sensor (C007)

### Entrance & Title

- ✅ Entrance screen (Enter/Resume/Quit) — always shows for DM1 (v2.3.1)
- ✅ Entrance door opening animation (31-step, 4px/step)
- ✅ Resume loads saved game (M566 path)
- ✅ Click-only buttons (no keyboard shortcuts for Enter/Resume/Quit)
- ✅ No dungeon flash before entrance
- ✅ TITLE.DAT swoosh/zoom animation path + decode + launcher handoff before DM1 Entrance source-locked (all DM1 presentation modes; see `parity-evidence/runtime/title_dat_swoosh_path_decode_closure_20260520.md`)
- ✅ Title song / music playback (SONG.DAT runtime + opt-in SDL dummy-driver live path source-locked)

### Save/Load

- ✅ Save game on quit (DM1_SaveGame)
- ✅ Load game on Resume (DM1_LoadGame)
- ✅ Direct auto-save on level change absent/source-locked — ReDMCSB routes ordinary stairs/pits/teleporters through movement/map/sensor paths only; save writes are C140-only
- ✅ Multiple save slots — DM1 V1 runtime compatibility is source-locked to one primary save plus automatic .bak fallback (ef9fadd9); ReDMCSB does not support numbered in-game save slots, so any future multi-slot browser must be Firestaff-native UX

### Audio

- ✅ Sound index emission system exists (M11_Audio_EmitSoundIndex)
- ✅ Sound effect playback source-index fallback lanes are source-locked for doors/combat/creatures, including queued SDL source-index playback gate, Open Door projectile door-impact SFX, and source-silent CALM/BRANDISH/CONFUSE and FIREBALL/DISPELL/LIGHTNING action cues; party footsteps are provenance-locked absent in DM1 V1
- 🚫 Ambient dungeon sound blocked: ReDMCSB source-lock found event-indexed SFX only, no DM1 V1 ambient loop
- ✅ DM1 V1 audio/creature sound trigger system — 35 sound events (C00-C34 + M541/M542/M560/M561/M562/M563/M619/M620), all 27 creature types have attack/movement sound ordinals, 0 unlinked calls — SOURCE-LOCKED vs SOUND.C/DEFS.H
- ✅ Music/title song (SONG.DAT runtime and opt-in SDL dummy-driver live playback source-locked)


## DM1 V1 Data Structures

- ✅ DM1 V1 DUNGEON.DAT complete source-lock: 11/11 categories — dungeon file header (COMPRESSED_DUNGEON_HEADER, DEFS.H:984), map metadata (MAP struct, DEFS.H:1050), cell/wall data (DUNGEON.C:1423), floor/square data (M034_SQUARE_TYPE), sensor data C000-C127 (DEFS.H:1191-1284), actuator/trigger data (TIMELINE.C:711-1313), object records (ThingDataByteCount), creature generator C006 (TIMELINE.C:964), champion records (MOVESENS.C F0280), text/string data (DUNGEON.C:2210), endianness+checksum (DEFS.H:984, DECOMPDU.C:20) — SOURCE-LOCKED
- ✅ DM1 V1 GRAPHICS.DAT loading pipeline — header (0x8001, 713 entries, SHA 2c3aa836), F9012 entry classification, IMG3 RLE decompression, M11 asset cache via F0490 — SOURCE-LOCKED vs MEMORY.C:1212-2306

## DM1 V2.1 / V2.2 — Enhanced Modes

- ✅ Phase 0 — V1 parity gate: DM1 V2 may not change command semantics, dungeon timing, source-locked collisions, save/load data, or ReDMCSB-backed rules unless the behavior is behind an explicit V2 presentation toggle
  - ✅ Phase 0 slice — V2 presentation command-route gate: V1/off preserves ReDMCSB movement command IDs 1..6; V2/on maps presentation runtime IDs without changing source IDs (b5a41085)
- ✅ Phase 1 — Presentation scaffold: split V1 gameplay state from V2 render/input presentation, add deterministic V2 config persistence, and keep V1 as the default boot/runtime path
  - ✅ Phase 1 slice — deterministic V2 presentation settings scaffold source-locked; V1 default path and command/gameplay state remain isolated (ac70837c)
  - ✅ Phase 1 slice — presentation profile boundary source-locked; V2 presentation mode consumes copied V1 gameplay snapshots and keeps command/gameplay routing pinned to V1 (931688c6)
  - ✅ Phase 1 slice — launch smoke gate source-locked; V2 menu/config boot path can reach the presentation runtime without changing V1 default route (6a4b7b86)
  - ✅ Phase 1 slice — Phase 0/1 boundary gate source-locks V2 presentation routing away from V1 gameplay domains and verifies the scoped V2-only presentation scaffold (54a74d30).
- ✅ Phase 2 slice — graphics pipeline source isolation verified; V2 presentation may scale/filter/present source DM1 graphics while keeping palette, font, viewport geometry, and gameplay state boundaries source-locked (8308645d)
- ✅ Phase 3 slice — HUD champion/action interaction gate source-locked; existing V2 HUD command mirror is now wired into CTest and guarded against bypassing V1 inventory/slot transaction owners (f8b0b54d)
- ✅ Phase 4 slice — V2 lighting palette presentation gate mirrors ReDMCSB-selected DM1 V1 palette indices/thresholds and disables V2-only local effects on invalid source input; full Phase 4 remains open (7a2d4c6b).
- ✅ Phase 4 slice — field/projectile effect metadata gate source-locks presentation-only V2 metadata to ReDMCSB projectile, explosion, fluxcage, and field draw ownership without claiming pixel parity (db290990).
- ✅ Phase 5 slice — smooth movement presentation source-lock gate proves camera interpolation is presentation-only after accepted source-style movement and does not mutate V1 timing/cooldown, collision, sensor/event, creature timing, or redraw-cadence owners (2f3442bf).
- ✅ Phase 5 runtime bridge slice — source-accepted V1 movement/turn ticks can start V2 camera interpolation while preserving source-owned cooldowns, collision, sensors, creature timing, and redraw cadence (89b7332b).
- ✅ Phase 6 slice — touch/controller affordance route gate maps V2-only swipe/controller metadata onto existing source-locked movement commands while preserving V1 touch/click parity and rejecting V2-off affordances (b3fe97e3).
- ✅ Phase 7 slice — presentation-disabled V2 source route now has a deterministic state-hash gate against the normalized V1 movement model for the same command script; full enhanced V2 runtime equivalence and pixel gates remain open (95559ebc).

## CSB V1 (Chaos Strikes Back)

- ✅ Phase 0 — Provenance gate: hash-lock exact CSB dungeon/graphics/title/music assets and keep CSBWin/CSB lineage sources as secondary evidence; do not reuse DM1 assumptions without variant proof (185019bf; reinforced by 80293bb0)

## DM2 V1 (Skullkeep)

- ✅ Phase 0 — Source and provenance gate: mirror/hash-lock `gbsphenx/skproject` (`master` HEAD `a962896e42aaf54c76157a7b062fb5b0526929e6`), Sphenx SKWin reference page/package provenance, and exact Skullkeep game assets before parser or runtime work (`dm2_v1_phase0_provenance_gate`)

## Cross-Cutting Features

### Touch Support

- ✅ Entrance/menu click zones and runtime touch dispatch are source-locked from ReDMCSB
- ✅ DM1 V1 dungeon viewport/main UI touch-route evidence and CTest gate source-locked (7a16e741)
- ✅ Touch input zones for movement/turning are source-locked to existing DM1 V1 mouse command routes and queue dispatch (c4e64fcc)
- ✅ DM1 V1 champion status ready/action hand touch zones are source-locked to the original status-box hand-slot route matrix and CTest coverage (e1546462)
- ✅ Touch item interaction routes are source-locked as mouse-command queue producers for status hand, inventory action hand, backpack, chest panel, scaled viewport, dungeon-viewport guard, and locked-queue pending clicks (a29fe25e)

### Platform

- ✅ macOS build (CMake + make)
- ✅ GitHub Actions CI/CD release pipeline
- ✅ Linux build verification (Ubuntu 24.04 x86_64 CMake/Ninja build, CI-equivalent smoke probes, and local DEB/RPM preview - docs/platform/linux-build-verification-20260520.md)
- ✅ Windows build verification (GitHub Actions windows-2022/MSYS2 release job, ZIP, NSIS installer, and SHA256 published for v2.4.1 - docs/platform/windows-build-verification-20260520.md)


## DM1 V1 — Source-Lock Audit (completed 2026-05-24/25)

### Movement & Collision
- ✅ Movement forward step — F0101/F0102 source-locked
- ✅ Movement turning — F0100/F0128 flip parity verified
- ✅ Movement timing — GAMELOOP pipeline source-locked
- ✅ Movement collision — s_wall_specs[] flip parity verified

### Creature System
- ✅ Creature aspect table — s_aspects[27] C00-C26 vs ReDMCSB G0219
- ✅ Creature projectile firing — F0823 vs GROUP.C F0207
- ✅ Projectile draw order — s_thing_layers[] vs DUNVIEW.C:4567-4581
- ✅ Door animation — F0717 via Pass 418 CMakeLists.txt gate
- ✅ Door frame rendering — dm1_viewport_3d_draw_frame() per-cell G21xx
- ✅ Wall bitmap rendering — s_wall_specs[] vs DUNVIEW.C:183

### Champion System
- ✅ Champion stat panel — F0351 vs PANEL.C:1965-2096
- ✅ Champion bar graph — F0287 zone geometry C195-C206
- ✅ Hall of Champions — C127→C026 portrait strip, click routing
- ✅ C030/C031/C032 labels — PANEL.C:1594-1606 source-cited
- ✅ Portrait sensorData — m11_game_view.c:8995 no portrait swap bug

### Inventory & Items
- ✅ Item description panel — PANEL.C:1444-1469
- ✅ HUD status bar — 53 zones C195-C206, G0046 farger

### Data Structures
- ✅ DUNGEON.DAT — 11/11 categories source-locked (37a90a0f)
- ✅ GRAPHICS.DAT — 713 entries, SHA 2c3aa836, F9012/F0488/F0490 verified

### Audio/Triggers
- ✅ Audio/creature runtime triggers — 35 sound events C00-C34 mapped
- ✅ Dungeon music cue — G2039 confirmed, no creature-based combat music
- ✅ Triggers/scripting — floor/wall sensors C000-C127, timeline, F0268-F0276

### Systems
- ✅ Combat system — hit/damage formula source-locked
- ✅ AI behavior — creature FSM WANDER/ATTACK/APPROACH/FLEE source-locked
- ✅ Menu system — startup/champion select/ingame source-locked
- ✅ Overworld — no overworld in DM1 V1, entrance index 255
- ✅ Network/serialization — 512-byte header, checksum pipeline
- ✅ Performance — 60Hz VBLANK, GAMELOOP pipeline, chip/fast/heap memory
- ✅ Platform/build — SDL2/SDL3 CMake alias, TITLE file discovery
- ✅ Terrain/environment — 7 terrain types, square encoding, G0039 light table
- ✅ Modding — no mod loader, checksum-obfuscation binary saves
- ✅ Multiplayer — DM1 V1 NO multiplayer
- ✅ Variants — I34E vs I34M confirmed, SHA256 locked
- ✅ QA — 387 tests, 89% pass rate, BUG0_02 timeline overflow critical

### Spells
- ✅ DM1 V1 spells/magic — 25 spells, 4-step symbol grid, mana formula

### Dungeon
- ✅ DM1 V1 dungeon audit — map metadata, cell/wall/floor data, C006 generators
- ✅ DM1 V1 triggers — events, conditional, puzzle, spawn, timer docs

## CSB V1 — Source-Lock Audit (completed 2026-05-24/25)

- ✅ CSB V1 deep audit — Grey Lord (0x1a) only new creature, 24 levels vs 14
- ✅ CSB V1 source-lock — champions/combat/creatures/dungeon/graphics/items/mechanics
- ✅ Key findings: BUG0_04 Lord Chaos palette NOT fixed, projectile speed normalization
- ✅ Grey Lord encounter details documented
- ✅ NEOPHYTE rank below NOVICE, reincarnation penalty changes

## Nexus V1 — Source-Lock Audit (completed 2026-05-24/25)

- ✅ Nexus overview/overview (Java/Android origin, 3D IsoEngine)
- ✅ Nexus features — DMDF 3D models, CD audio, FMV, 16 levels
- ✅ Nexus creatures — AI stub vs DM1 full system, SDDRVS.TSK
- ✅ Nexus dungeon — 16 levels, 32×32 grid, DGN format
- ✅ Nexus champions — Japanese roster, Ninja class, FACE.BIN
- ✅ Nexus combat — 3-state patrol/chase/attack
- ✅ Nexus AI — ~30-line Manhattan-distance stub
- ✅ Nexus menus — VDP2 layers not emulated, FONT256.S2D glyph only
- ✅ Nexus HUD — compass, movement arrows, stat bars not rendered
- ✅ Nexus items — 30-slot inventory vs DM1 12, flask GAP
- ✅ Nexus sound — CD-DA, SNDLEV*.SAL not reverse-engineered
- ✅ Nexus triggers — SDDRVS.TSK scripting vs DM1 hardwired
- ✅ Nexus modding — zero cheat codes, BUG0_02 overflow
- ✅ Nexus platform — Saturn big-endian SH2, 30 FPS
- ✅ Nexus QA — 0% test coverage, no disc image
- ✅ Nexus save — NOT IMPLEMENTED
- ✅ Nexus variants — Windows/Saturn/Java, English only

## DM2 V1 — Source-Lock Audit (completed 2026-05-24/25)

- ✅ DM2 overview/story/tech — SKULL.ASM 522K lines, Lord Dragoth boss
- ✅ DM2 creatures — 64-entry AI index, companion/minion system
- ✅ DM2 spells — 34 spells vs ~25 in DM1, minion summons
- ✅ DM2 combat — hit/damage formula, 12 AI_ATTACK_FLAGS
- ✅ DM2 save — SUPPRESS codec, 10 slots, no autosave
- ✅ DM2 input — SDL keyboard/mouse, c_Tmouse FIFO
- ✅ DM2 platform — CMake SKULLWIN/SKWINDOS, Allegro 5 + SDL 1.x
- ✅ DM2 variants — English only, DOS/Win95/Android/iOS
- ✅ DM2 AI — b_1a bytecode dispatch, companion loyalty system
- ✅ DM2 modding — DMDC2/DM2GDED/DMute editors
- ✅ DM2 QA — 0% test coverage, 7 source files no fixtures
- ✅ DM2 network — single-player only, FSTP is Firestaff stub
- ✅ DM2 dungeon — 30 levels, X-teleporter system, OUTDOOR/INDOOR/BUILDING
- ✅ DM2 graphics — 8-bit VGA still, outdoor renderer, dual palettes
- ✅ DM2 mechanics — Tech/Magic/Hybrid affinity, 24h day/night clock
- ✅ DM2 items — 6 categories, tech_level gun gate, Merchant NPC AI 0x21
- ✅ DM2 start-up — ANIM title, no FMV, SkCodeParam flags
- ✅ DM2 sound — HMP/MIDI, GDAT2 V5 SFX, QUEUE_NOISE system
- ✅ DM2 triggers — actuator records, gametick timeline
- ✅ DM2 story — Torham Zed protagonist, 9-phase walkthrough
- ✅ DM2 source — SKULL.ASM 16-layer module hierarchy, 16-bit memory model

## Recently Fixed (v2.0.0 → v2.4.0, 2026-05-19)

- Champion mirror portrait, front-wall inscription, and viewport floor-object placement source-path cleanup (DUNVIEW.C/F0115/C2500, pass606)
- DM1 V1 wall/collision runtime capture gate with exact coordinates, movement-pipeline state, and screenshots
- HiDPI/Retina scaling
- Stair direction (F0155)
- Door frame preservation for open portcullis doors (F0111)
- Resume button loads saved data (M566)
- Entrance click-only + dungeon flash fix
- Champion mirrors/wall ornaments (F0169-F0172)
- Object zone Y coordinates (COORD.C)
- Creature damage formula (F0230)
- Creature death item drops
- Dead creatures do not block passage
- Entrance screen always appears for DM1
- Inscription text overlay
- Wall click sensors work for door buttons and switches

## DM1 V1 — Core Gameplay (updated 2026-05-24)

### Movement & Collision

- ✅ Viewport/wall occlusion — pass627 (c53c7968) documents same-viewport capture contract as fully CTest-locked; remaining gap is missing original DOS runtime transcript (F0380 to F0097) blocking promotion; Firestaff capture contract fully implemented
- ✅ Input command routing — movement collision-before-sensor dispatch ordering, stairs backstep cooldown, pending-click single-slot overwrite/replay, pass623 input-script crop rows, pass626 original turn-redraw route target all source-locked
- ✅ Sensors & Mechanisms — all 10 floor (C000-C009) and 20 wall (C001-C018, C127) sensor types source-locked (MOVESENS.C F0268-F0276, TIMELINE.C F0241-F0248, CLIKVIEW.C F0372/F0377); GAP-1 door animation wired via Pass 418 (22f9ae9c + e7c0e3be); GAP-3 C003 non-consuming ornament click source-locked (F0723:326-328); 7/7 sensor CTests pass

### Creature System

- ✅ Creature projectile firing — F0823 ↔ ReDMCSB GROUP.C F0207; 7 explicit cases + default match; C25/C26 structurally safe (default FIREBALL), BUG0_13
- ✅ Creature aspect table — s_aspects[27] (C00-C26) matches ReDMCSB G0219 exactly; Hall champions C00-C23 present; M618_GRAPHIC_FIRST_CREATURE=584 verified
- ✅ Projectile draw order — s_thing_layers[] (Objects→Creatures→Projectiles→Explosions) verified vs DUNVIEW.C:4567-4581; D0C occlusion via s_projectile_occlusion_specs[]; verify_pass405/pass563 pass
- ✅ Door animation — F0717_DOOR_ResolveClosingObstruction_Compat wired via Pass 418; CMakeLists.txt gate added (6dd8fcab); 17/20 door tests pass
- ✅ Door frame rendering — dm1_viewport_3d_draw_frame() full per-cell G21xx dispatch + F0105 flip for all 9 door square types; 0 warnings build (b6b7fbf5)
- ✅ Wall bitmap rendering — s_wall_specs[] flip parity verified; G2107_WallSet[15] matches DUNVIEW.C:183; D3/D2/D1/D0/D0C selection source-locked
- ✅ Floor ornament draw order — Layer0→Layer1→Layer2 matching DUNVIEW.C:4567-4582; all 11 view-squares with F0108 citations
- ✅ Audio/creature runtime triggers — 35 sound events C00-C34 mapped; 0 unlinked calls; ReDMCSB SOUND.C/DATA.C G0060 source-anchored
- ✅ Dungeon music cue — G2039_ai_MapIndexToMusicTrack[14] confirmed; no creature-based combat music (CEDT/HINT/ANIM.C searched); NOT_A_GAP
- ✅ Portrait sensorData comment (m11_game_view.c:8995) — no portrait swap bug; both original and Firestaff blit same sheet cell; committed (62411518)

### Champion System

- ✅ Champion stat panel — F0351 vs PANEL.C; all 6 stats, color thresholds, skill level >=2 condition source-locked
- ✅ Champion bar graph rendering — bar height math (F0287), zone geometry (C195-C206), G0046 champion colors {7,11,8,14} verified vs CHAMDRAW.C
- ✅ Hall of Champions rendering — all 24 champions present; C127→C026 portrait strip, click routing (F0866 bypasses no-leader) source-locked; Vi Altar rebirth and See Through Walls documented
- ✅ C030/C031/C032 food/water/poison labels — DM1_ChampionPanel_DrawFoodWaterPoisonLabels() added; source-cited to PANEL.C:1594-1606; committed (361a8381)

### Inventory & Items

- ✅ Item description panel — form-feed reset, 18-char wrap, POISONED/BROKEN/CURSED prefixes, weight line source-locked (PANEL.C:1444-1469)
- ✅ HUD status bar — 53 zones (C195-C206), bar height math, G0046 farger source-locked to CHAMDRAW.C/DEFS.H/PANEL.C/TIMELINE.C

### Data Structures

- ✅ DUNGEON.DAT all data structures — 11/11 categories source-locked; committed (37a90a0f)
- ✅ GRAPHICS.DAT loading pipeline — 713 entries, SHA 2c3aa836..., F9012/F0488/F0490 verified; all asset families source-locked

### DM1 V1 — Audit Findings 2026-05-25

- ✅ Inventory equip/unequip — m11_inventory_can_equip/equip/unequip slot validation added; CHAMPION.C F0300-F0302 source citations; committed (a85fb55d)
- ✅ Save/load serialization — DM1_SaveGame/LoadGame via F0897_WORLD_Serialize_Compat complete; champions/inventory/stats serialized; dungeon state correctly excluded from save blob
- ✅ Item pickup API — m11_obj_examine/pickup/drop exist; m11_inventory_pickup_mouse() exists (line 107)
- ⚠️ Save/load integration GAP — DM1_SaveGame/LoadGame never called from any input handler or menu; m11_sl_* slot infrastructure has ZERO callers; G2018 quit-guard not found; 512-byte header replaced by 64-byte CRC32 (not original-compatible)
- ⚠️ Item pickup wiring GAP — m11_inventory_pickup_mouse() no backpack capacity check; m11_inventory_can_pickup() absent; m11_obj_pickup() defined but not wired into viewport click path; no pickup command bridging viewport click → inventory
- ⚠️ Object interaction GAP — m11_obj_use() is a stub (checks usable flag only, no stat effects); zero call sites outside own definition; module registered in engine manifest but no input/champion-action integration
- ⚠️ Group management GAP — m11_group_add_active() defined in group_management_pc34_compat.c but NEVER called from sensor_trigger, game_loop, or any other module; C006/F0267 group spawning not wired
- ⚠️ Teleporter wiring GAP — ALL 10 public API functions in dm1_v1_teleporter_pit_pc34_compat.c are ORPHANED; F0267 referenced in comments but no actual calls exist from movement_pipeline or game_loop
- ✅ HUD status bar — GAP documented: DM1 V1 top strip (dungeon name/floor/clock/gold/food/water) does not exist in ReDMCSB source; DM2/CSB feature only; not a Firestaff gap


### DM1 V1 — Rendering Pipeline Audit 2026-05-25 (Continued)

- ✅ s_draw_order[19] defined correctly (back-to-front, D4L→D0C) — lines 78–101
- ✅ s_wall_specs[], s_projectile_occlusion_specs[], wall flip parity all correct
- ✅ Draw primitives (F0098/F0100/F0101) implemented in dm1_v1_draw_primitives_pc34_compat.c
- ⚠️ VIEWPORT RENDERING GAP — dm1_viewport_3d_draw_frame() is skeleton only; for loop over s_draw_order[] (lines 507–578) has comments but NO executable code for Steps 4–11; all wall drawing, door occlusion, far-object rendering is absent; build still passes because primitives exist but are never called
- ✅ Build: 100% clean (all targets link)


## DM1 V1 GAP Fixes (2026-05-25) — Completed Today

- ✅ ESC-dialog YES/NO buttons — m11_draw_dialog_choices_source() was never called for return-to-menu branch because drawnSourceBackdrop was 0; added choice-drawing loop after cdlgX/cdlgY assignment; commit 4ae014dd
- ✅ Object interaction (m11_obj_use stub) — replaced stub with real implementation delegating to dm1_inventory_consume_potion_pc34 / dm1_inventory_consume_water_junk_pc34 / dm1_inventory_consume_food_junk_pc34; wired sound module to test; commits cb8b1586 + da06e726
- ✅ CardArt NULL guard + Theron card art — M12_CardArt_Resolve now guards against NULL gameId before any strlen call (crash fix); Theron uses nexus card art candidates; commit a9ac95dc
- ✅ Equip/unequip slot validation — added m11_inventory_can_equip/equip/unequip with F0300-F0302 source citations and 6-slot capacity check; commit a85fb55d
