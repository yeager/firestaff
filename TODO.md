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
- 🐛 Väggar saknas ibland — behöver exakt plats/screenshot för repro
- 🐛 Kollision blockerar ibland fel — behöver exakt plats för repro
- ❌ Inscription rendering on side walls (D2L/R, D3L/R) — only D1C front done
- ✅ Readable inscription rendering (source message zone centering)
- ❌ Teleporter visual effect (shimmer/sparkle)

### Creature System
- ✅ Creature groups loaded from dungeon.dat
- ✅ Creature behavior profiles (27 types, movement/attack/sight/smell)
- ✅ Creature AI — movement toward party (cardinal-only, F0228/F0209)
- ✅ Creature attack on party (F0230 damage formula with dodge/parry/armor)
- ✅ Creature death detection + item drops (v2.2.0)
- ✅ Dead groups cleaned up (don't block movement)
- ✅ Kill XP award via lifecycle system
- ✅ Creature rendering — sprites load and source-locked aspect frames cycle
- ❌ Creature type-specific behavior (Giggler steal, Ghost non-material, etc.)
- ❌ Creature group spawning (C006 floor sensor group generator)
- ❌ Creature projectile attacks (Vexirk spells, Dragon fire)
- ❌ Creature sound effects (attack sounds, movement sounds)
- ❌ Creature flee behavior (low HP retreat)

### Combat
- ✅ Champion melee actions (SWING, CHOP, STAB, THRUST, HACK, etc.)
- ✅ Action menu (3 actions per weapon type + empty hand)
- ✅ Melee damage applied to creature groups (F0738)
- ✅ Creature damage to champions (F0230)
- ✅ Parry skill XP
- ✅ Combat result logging
- ❌ Ranged weapon actions (SHOOT with bow/crossbow)
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
- ❌ Light spell (ZO → increase magical light)
- ❌ Darkness spell
- ❌ Open Door spell (ZO BREU)
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
- ❌ Champion weight/load system (encumbrance)
- ❌ Champion stamina regeneration rate tied to load

### Inventory & Items
- ✅ Leader hand object (pick up, put down, swap)
- ✅ Alcove item interaction (click to pick up)
- ✅ Item throwing (throw into dungeon, creates projectile)
- ✅ Torch fuel tracking + light power calculation
- ✅ Torch burns out → extinguish
- 🔧 Floor item pickup — partially (alcoves yes, floor scatter limited)
- ❌ Full inventory panel (8 body slots + 2 hand slots + backpack)
- ❌ Equip/unequip items to body slots
- ❌ Backpack/chest container management
- ❌ Item identification (scroll reading, potion identification)
- ❌ Scroll text display (F0168 C2_TEXT_TYPE_SCROLL)
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
- ❌ Projectile launcher sensors (C007-C010, C014-C015)
- ❌ End game sensor (C018)
- ❌ Floor party-on-stairs sensor (C005)
- ✅ Floor possession sensor (C008 — party has specific item)
- ❌ Floor creature sensor (C007)

### Entrance & Title
- ✅ Entrance screen (Enter/Resume/Quit) — always shows for DM1 (v2.3.1)
- ✅ Entrance door opening animation (31-step, 4px/step)
- ✅ Resume loads saved game (M566 path)
- ✅ Click-only buttons (no keyboard shortcuts for Enter/Resume/Quit)
- ✅ No dungeon flash before entrance
- 🐛 Swoosh animation not playing — TITLE.DAT path/decode issue
- ❌ Title song / music playback

### Save/Load
- ✅ Save game on quit (DM1_SaveGame)
- ✅ Load game on Resume (DM1_LoadGame)
- ❌ Auto-save on level change
- ❌ Multiple save slots

### Audio
- 🔧 Sound index emission system exists (M11_Audio_EmitSoundIndex)
- ❌ Sound effect playback (footsteps, doors, combat, creatures)
- ❌ Ambient dungeon sound
- ❌ Music/title song

---

## DM1 V2 — Enhanced Mode

- 🔧 V2 presentation mode selectable from menu
- ❌ Upscaled graphics pipeline
- ❌ Modern UI overlay
- ❌ Enhanced lighting/shadows
- ❌ Smooth movement interpolation

---

## CSB (Chaos Strikes Back)

- 🔧 CSB launch entry in menu
- ❌ CSB-specific dungeon.dat parsing differences
- ❌ CSB-specific creature types
- ❌ CSB back-wall ornaments (4-sided walls)
- ❌ CSB utility/import champion system
- ❌ CSB-specific sensors and mechanisms

---

## DM2 (Skullkeep)

- 🔧 DM2 launch entry in menu
- ❌ DM2 dungeon.dat format
- ❌ DM2 graphics.dat format
- ❌ DM2 creature types and AI
- ❌ DM2-specific mechanics (shops, minecart, etc.)

---

## DM Nexus

- 🔧 Nexus launch entry in menu
- ❌ Nexus-specific content and mechanics

---

## Cross-Cutting Features

### Touch Support
- 🔧 Touch controls config option exists
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

1. 🐛 Väggar saknas ibland i viewport — behöver exakt position och screenshot
2. 🐛 Kollision blockerar ibland där det borde vara fritt — behöver exakt position
3. 🐛 Swoosh-animation visas inte — TITLE.DAT path/decode
4. 🐛 Flytande objekt kan fortfarande se lite off ut trots zone Y fix

---

## Recently Fixed (v2.0.0 → v2.4.0, 2026-05-19)

- HiDPI/Retina skalning
- Trapp-riktning (F0155)
- Dörrfodret vid öppna gallerdörrar (F0111)
- Resume-knapp laddar sparad data (M566)
- Entrance click-only + dungeon flash fix
- Champion mirrors/wall ornaments (F0169-F0172)
- Object zone Y-koordinater (COORD.C)
- Creature damage formula (F0230)
- Creature death item drops
- Dead creatures blockerar inte passage
- Entrance-skärmen visas alltid för DM1
- Inscription text overlay
- Wall click sensors fungerar (dörrknappar/switchar)
