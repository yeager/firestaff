# Firestaff TODO — DM1 V1 Parity & Beyond

Status per 2026-05-19 v2.4.0.

## Legend
- ✅ Done
- 🔧 Partially implemented
- ❌ Not started
- 🐛 Known bug

---

## DM1 V1 — Core Gameplay

### Movement & Navigation
- ✅ Cardinal movement (WASD + arrow keys + click)
- ✅ Turning (left/right)
- ✅ Collision detection (walls, doors, creatures)
- ✅ Movement cooldown (G0310/G0311 timing from F0267)
- ✅ Pit fall + level change + fall damage (20 HP/pit)
- ✅ Teleporter chains (up to 1000 iterations)
- ✅ Stair transitions (up/down, correct direction per F0155)
- ✅ Dead creature groups don't block (v2.3.0)

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
- ✅ Floor item sprites (scatter rendering, multi-item)
- ✅ Alcove content rendering (items inside wall alcoves)
- ✅ Object zone Y coordinates (COORD.C source-lock, v2.0.0)
- ✅ Side door rendering (D1L/R, D2L/R, D3L/R)
- ✅ Door ornaments
- ✅ Depth occlusion (center wall blocks far content)
- ✅ Side lane occlusion
- ✅ Palette dimming based on light level
- ✅ HiDPI/Retina scaling (v2.0.0)
- ✅ Fakewall viewport/collision parity — open fakewalls render as corridor, closed imaginary fakewalls stay passable but wall-like
- 🐛 Remaining intermittent wall/collision reports need exact coordinate/screenshot/runtime capture
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
- 🔧 Creature type-specific behavior — Giggler steal source-locked; Ghost non-material and other type specials remain
- ❌ Creature group spawning (C006 floor sensor group generator)
- ❌ Creature projectile attacks (Vexirk spells, Dragon fire)
- ✅ Creature sound effects attack/movement ordinals and runtime trigger coverage are source-locked
- ❌ Creature flee behavior (low HP retreat)

### Combat
- ✅ Champion melee actions (SWING, CHOP, STAB, THRUST, HACK, etc.)
- ✅ Action menu (3 actions per weapon type + empty hand)
- ✅ Melee damage applied to creature groups (F0738)
- ✅ Creature damage to champions (F0230)
- ✅ Parry skill XP
- ✅ Combat result logging
- 🔧 Ranged weapon actions (SHOOT with bow/crossbow) — bow/crossbow parameter path source-locked; full runtime projectile wiring remains pending
- ❌ Shield defense bonus in combat
- ❌ Weapon breakage
- ❌ Poison damage over time (creature PoisonAttack → F0322)

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
- 🔧 Open Door spell (ZO BREU) — projectile door-impact SFX source-locked; full door behavior remains pending
- ❌ Poison cloud damage over time
- ❌ Spell failure feedback (fizzle animation)

### Champion System
- ✅ Champion recruitment from mirrors (HoC)
- ✅ Active champion selection
- ✅ Champion bar display (HP/stamina/mana bars)
- ✅ Skill levels (Fighter/Ninja/Priest/Wizard)
- ✅ Skill XP and level-up via lifecycle system
- ✅ Champion death detection
- ✅ Resurrection system (dm1_v1_resurrection_pc34_compat)
- 🔧 Champion stats panel — exists but not fully interactive
- ❌ Champion stat screen (full F0340 display with all attributes)
- 🔧 Champion weight/load system (encumbrance) — F0306/F0309/F0310 max-load and movement-cost core source-locked
- ❌ Champion stamina regeneration rate tied to load

### Inventory & Items
- ✅ Leader hand object (pick up, put down, swap)
- ✅ Alcove item interaction (click to pick up)
- ✅ Item throwing (throw into dungeon, creates projectile)
- ✅ Torch fuel tracking + light power calculation
- ✅ Torch burns out → extinguish
- ✅ Floor item pickup — rendered grabbable object-cell gate and runtime pile-top object wiring source-locked
- ❌ Full inventory panel (8 body slots + 2 hand slots + backpack)
- 🔧 Equip/unequip items to body slots — partially: PC34 slot masks and leader-hand/body-slot swap transaction are source-locked; full inventory/backpack/chest storage expansion remains pending
- 🔧 Backpack/chest container management — PC34 backpack/chest slot namespace, chest open/close visible-slot compaction, and leader-hand/chest slot swap helpers source-locked; runtime panel wiring remains pending
- ❌ Item identification (scroll reading, potion identification)
- ✅ Scroll text display (F0168 C2_TEXT_TYPE_SCROLL) — TextString decode/visibility/separator path and inventory scroll panel rendering source-locked
- ❌ Potion consumption effects
- ❌ Food/water item consumption (click to eat/drink)
- ❌ Fountain interaction (fill flask with water)
- ❌ Key usage (use key on locked door)

### Survival / Needs
- ✅ Food and water drain (1 unit per 6 ticks per champion)
- ✅ Champion bar color reflects hunger/thirst status
- ❌ Food/water depletion → HP damage
- ❌ Rest system (sleep to recover, wake on creature approach)
- ❌ Stamina drain from actions
- ❌ Stamina regeneration

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
- ✅ TITLE.DAT swoosh/zoom animation path + decode source-locked (see `parity-evidence/runtime/title_dat_swoosh_path_decode_closure_20260520.md`)
- ✅ Title song / music playback (SONG.DAT runtime + opt-in SDL dummy-driver live path source-locked)

### Save/Load
- ✅ Save game on quit (DM1_SaveGame)
- ✅ Load game on Resume (DM1_LoadGame)
- ❌ Auto-save on level change
- ❌ Multiple save slots

### Audio
- ✅ Sound index emission system exists (M11_Audio_EmitSoundIndex)
- ✅ Sound effect playback source-index fallback lanes are source-locked for doors/combat/creatures, including Open Door projectile door-impact SFX; party footsteps are provenance-locked absent in DM1 V1
- 🚫 Ambient dungeon sound blocked: ReDMCSB source-lock found event-indexed SFX only, no DM1 V1 ambient loop
- ✅ Music/title song (SONG.DAT runtime and opt-in SDL dummy-driver live playback source-locked)

---

## DM1 V2 — Enhanced Mode

- 🔧 V2 presentation mode selectable from menu
- ❌ Phase 0 — V1 parity gate: DM1 V2 may not change command semantics, dungeon timing, source-locked collisions, save/load data, or ReDMCSB-backed rules unless the behavior is behind an explicit V2 presentation toggle
- ❌ Phase 1 — Presentation scaffold: split V1 gameplay state from V2 render/input presentation, add deterministic V2 config persistence, and keep V1 as the default boot/runtime path
- ❌ Phase 2 — Graphics pipeline: source-asset-preserving upscale path for walls, creatures, objects, projectiles, fonts, palette/light levels, and title/entrance surfaces
- ❌ Phase 3 — Modern UI overlay: optional HUD/inventory/champion/rune/action panels that mirror V1 commands without bypassing source-locked click routes or inventory transactions
- ❌ Phase 4 — Lighting and visual effects: enhanced lighting/shadows, palette interpolation, field/teleporter/projectile effects, and HiDPI-safe composition with deterministic fallback
- ❌ Phase 5 — Smooth movement presentation: interpolation/camera easing between source-locked V1 movement ticks without changing cooldowns, collision, sensors, creature timing, or redraw cadence
- ❌ Phase 6 — Touch/controller ergonomics: V2-only gesture/controller affordances mapped onto existing command routes, with V1 touch/click parity preserved
- ❌ Phase 7 — V2 verification suite: side-by-side V1/V2 deterministic probes proving identical gameplay state hashes for the same input script plus screenshot/pixel gates for V2 presentation

---

## CSB (Chaos Strikes Back)

- 🔧 CSB launch entry in menu
- ❌ Phase 0 — Provenance gate: hash-lock exact CSB dungeon/graphics/title/music assets and keep CSBWin/CSB lineage sources as secondary evidence; do not reuse DM1 assumptions without variant proof
- ❌ Phase 1 — Boot/profile split: separate CSB runtime profile from DM1, including asset discovery, menu launch, save namespace, deterministic config, and variant-specific diagnostics
- ❌ Phase 2 — Dungeon data model: source-lock CSB dungeon.dat parsing differences, map metadata, object records, wall formats, champion transfer/import state, and start-position semantics
- ❌ Phase 3 — Rendering parity: CSB wall/door/floor/ornament/creature/item/projectile rendering, including back-wall ornaments and four-sided wall decoration rules
- ❌ Phase 4 — Mechanics parity: CSB-specific sensors, actuators, teleporters, pits, doors, pressure plates, end conditions, and dungeon logic that diverges from DM1
- ❌ Phase 5 — Creature/combat parity: CSB creature roster, AI differences, attacks/projectiles, drops, sounds, and combat constants
- ❌ Phase 6 — Utility/import flow: CSB utility disk behavior, champion import path, reincarnation/resurrection differences, and saved-party interoperability
- ❌ Phase 7 — Verification suite: canonical CSB asset manifests, parser probes, deterministic input scripts, viewport/pixel gates, save/load round trips, and source-evidence manifests

---

## DM2 (Skullkeep)

- 🔧 DM2 launch entry in menu
- ❌ Phase 0 — Source and provenance gate: mirror/hash-lock `gbsphenx/skproject` (`master` HEAD `a962896e42aaf54c76157a7b062fb5b0526929e6` at planning time), Sphenx SKWin reference page/package provenance, and exact Skullkeep game assets before parser or runtime work
- ❌ Phase 1 — Runtime profile split: separate DM2/Skullkeep boot profile from DM1/CSB, including menu launch, asset roots, save namespace, platform/version diagnostics, and deterministic config
- ❌ Phase 2 — Data formats: source-lock DM2 dungeon and graphics formats from SKWin/SKWINSPX/DMDC2 references, including GDAT categories, dungeon records, text, item records, actuators, doors, pits, teleports, ornate data, and variant/platform differences
- ❌ Phase 3 — Core world model: implement DM2 map loading, party placement, map transitions, outdoor/interior state, timers, object database, and deterministic world-state hashing
- ❌ Phase 4 — Rendering pipeline: source-lock Skullkeep wall/floor/door/ornament/item/creature/projectile/cloud rendering, palette/light handling, UI surfaces, title/intro assets, and GDAT-backed animation frames
- ❌ Phase 5 — Movement and interaction: port DM2 movement, click routing, item pickup/placement, containers, shop/trader interactions, doors, ladders, pits, teleports, buttons, generators, and mine/cart-specific routes
- ❌ Phase 6 — Creature, combat, spells, and environmental systems: source-lock DM2 creature AI, attacks/projectiles, champion actions, spells/clouds, weather/ambient timers, sounds, drops, and progression constants
- ❌ Phase 7 — Save/import compatibility: support DM2 save/load, PC savegame interoperability where source-backed, champion state persistence, object/container state, and cross-version diagnostics
- ❌ Phase 8 — Verification suite: canonical DM2 asset manifests, parser probes, GDAT/dungeon record fixtures, deterministic input scripts, pixel/viewport gates, save/load round trips, and source-evidence manifests tied to SKWin/skproject references

---

## DM Nexus

- 🔧 Nexus launch entry in menu
- ❌ Nexus-specific content and mechanics

---

## Cross-Cutting Features

### Touch Support
- 🔧 Touch controls config option exists
- ✅ Entrance/menu click zones and runtime touch dispatch are source-locked from ReDMCSB
- ❌ Touch input zones for movement/turning
- ❌ Touch-based item interaction
- ❌ Gesture navigation (swipe to turn, tap to move)
- ❌ UI scaling for touch targets

### Accessibility
- 🔧 Accessibility module exists (firestaff_accessibility)
- ❌ Screen reader integration
- ❌ High-contrast mode
- ❌ Configurable font size

### Platform
- ✅ macOS build (CMake + make)
- ✅ GitHub Actions CI/CD release pipeline
- ❌ Linux build verification
- ❌ Windows build
- ❌ iOS/Android build
- ❌ Web/WASM build

---

## Known Bugs (need repro)

1. 🐛 Remaining intermittent wall/collision reports need exact coordinate/screenshot/runtime capture
2. 🐛 Some floor objects may still look slightly mispositioned despite the zone Y fix

---

## Recently Fixed (v2.0.0 → v2.4.0, 2026-05-19)

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
