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
- ✅ Blocked wall/door/closed-real-fakewall self-damage request source-lock
- ✅ Movement cooldown (G0310/G0311 timing from F0267)
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
- ✅ Palette dimming based on light level
- ✅ HiDPI/Retina scaling (v2.0.0)
- ✅ Fakewall viewport/collision parity — open fakewalls render as corridor, closed imaginary fakewalls stay passable but wall-like
- ✅ Wall/collision runtime capture gate — exact map/x/y/direction, movement-pipeline state, and PPM screenshots for blocked and accepted movement
- ✅ Inscription rendering on side walls (D2L/R, D3L/R) — unreadable plaque heights source-locked (pass582)
- ✅ Readable inscription rendering (source message zone centering)
- ✅ Teleporter visual effect — source-backed GRAPHICS.DAT field bitmap overlay, not procedural sparkle

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
- ✅ Creature flee behavior — source-locked fear-triggered flee delay, flee direction, countdown expiry, and behavior dispatch are covered by the DM1 V1 creature AI gate
- ✅ Creature special positioning — quarter-square melee cell shuffle and adjacent creature projectile chance are source-locked in the DM1 V1 creature AI gate (5121884b)
- ✅ C006 Lord Chaos blocked-destination adjacent random retry is source-locked from ReDMCSB event60/61 handling (0a12e07d)
- ✅ C006 generated groups now reuse ReDMCSB-style fixed unused group slots instead of appending beyond source capacity (1d3a9bd4)
- ✅ C006 generated/deferred groups now route through the narrow F0267 teleporter destination/cross-map placement path, including the covered audible buzz and target-square wandering scheduling case (191c9d3c)

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
- ✅ Champion panel HP/stamina/mana numeric status-value formatting and zone routing are source-locked from CHAMDRAW.C F0289/F0290 (d504de4c)
- ✅ Champion stamina regeneration — source-locked to F0331 food/water, resting, idle-delay, max-stamina amount, and mana-regeneration stamina cost; no direct load factor in regeneration path

### Inventory & Items

- ✅ Leader hand object (pick up, put down, swap)
- ✅ Alcove item interaction (click to pick up)
- ✅ Item throwing (throw into dungeon, creates projectile)
- ✅ Torch fuel tracking + light power calculation
- ✅ Torch burns out → extinguish
- ✅ Floor item pickup — rendered grabbable object-cell gate and runtime pile-top object wiring source-locked
- ✅ Scroll text display (F0168 C2_TEXT_TYPE_SCROLL) — TextString decode/visibility/separator path and inventory scroll panel rendering source-locked
- ✅ Potion consumption effects — potion stat/heal/mana/stamina/water/shield effects, VI wound RNG masks, inert/unknown potion empty-flask conversion, M11 mouth-click wiring, swallow audio routing, and no-mouth-animation behavior are source-locked
- ✅ Food/water item consumption — food amounts, water/waterskin charges, caps, leader-hand removal, swallow audio routing, C545 food mouth visual blit, and water/potion no-visual runtime behavior are source-locked through M11 mouth-click (4209e8e4)
- ✅ Weapon eye-panel name and attribute descriptions are source-locked for POISONED/BROKEN/CURSED formatting (94094865)
- ✅ Item eye-panel weight line display is source-locked from ReDMCSB item description formatting (35461e0b)
- ✅ Armour/junk/scroll/container eye-panel description families are source-locked from ReDMCSB F0342/F0336 routing and formatting: armour BROKEN/CURSED, junk CONSUMABLE plus compass and waterskin state lines, and scroll/container delegation to scroll/chest panels
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
- ✅ Music/title song (SONG.DAT runtime and opt-in SDL dummy-driver live playback source-locked)

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
- ✅ Touch item interaction routes are source-locked as mouse-command queue producers for status hand, inventory action hand, backpack, chest panel, scaled viewport, dungeon-viewport guard, and locked-queue pending clicks (a29fe25e)

### Platform

- ✅ macOS build (CMake + make)
- ✅ GitHub Actions CI/CD release pipeline
- ✅ Linux build verification (Ubuntu 24.04 x86_64 CMake/Ninja build, CI-equivalent smoke probes, and local DEB/RPM preview - docs/platform/linux-build-verification-20260520.md)
- ✅ Windows build verification (GitHub Actions windows-2022/MSYS2 release job, ZIP, NSIS installer, and SHA256 published for v2.4.1 - docs/platform/windows-build-verification-20260520.md)

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
