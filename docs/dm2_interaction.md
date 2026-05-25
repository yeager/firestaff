# DM2 V1 — Interaction: How to Interact with the World in DM2 vs DM1

**Audit date:** 2026-05-25
**Sources:** SKULL.ASM, skproject SKWIN/SkWinCore.cpp, docs/dm2_triggers.md, docs/dm2_actuators.md, docs/dm2_items.md, docs/dm2_special_squares.md, docs/dm2_input.md, docs/dm2_champ_changes.md

---

## 1. DM1 Interaction Model (Reference)

DM1 used a simple, hardwired interaction model:

- **Wall switches**: face a wall tile with a switch graphic, press a key to activate
- **Pressure plates / floor switches**: step on a floor tile to depress it and trigger an event
- **Pit traps**: floor tiles that open on step or timer
- **Doors**: interact to open/close; some locked, some not
- **Items**: pick up by walking over them; use from inventory
- **Spells**: cast from spellbook via hotkey

Each trigger type was bound to a specific tile type. The trigger logic was compiled into the game binary. Cross-map triggers were not possible.

---

## 2. DM2 Interaction Model — Actuator System

DM2 replaced the hardwired approach with a **data-driven actuator architecture**. This is the primary interaction mechanism.

### What is an Actuator?

An actuator is a **separate data record** stored in the dungeon data file (category dbActuator, record type 3):
- Attached to a **floor or wall cell**, independent of the base tile type
- Has **8 bytes of data** (DB_SIZE_ACTUATOR) or 4 bytes (DB_SIZE_SIMPLE_ACTUATOR)
- Has a **type** (from ACTUATOR_TYPE_* enum) and **per-type data fields**
- Can trigger **another actuator** on the same or different map (cross-map wiring)
- Can be wired in **series/relay chains** via relay actuator types

### Actuator Type Taxonomy (key families)

| Type Hex | Name | Category |
|----------|------|----------|
| 0x01 | DM1_WALL_SWITCH | DM1 retro |
| 0x03 | ITEM_WATCHER | Floor - fires when item enters/exits |
| 0x04 | DM1_ITEM_EATER | DM1 retro |
| 0x05 | DM1_BITFIELDS_TRIGGER | DM1 retro |
| 0x06 | DM1_COUNTER | DM1 retro |
| 0x08 | MISSILE_SHOOTER | Projectile - fires on trigger |
| 0x09 | WEAPON_SHOOTER | Projectile - fires weapon |
| 0x0E | ITEM_SHOOTER | Projectile - fires item |
| 0x15 | CHARGED_ITEM_WATCHER | Floor - fires when charge consumed |
| 0x16 | CROSS_MAP | Teleport/Wiring - cross-map trigger |
| 0x17 | 2_STATE_WALL_SWITCH | Wall - toggle switch |
| 0x18 | WALL_SWITCH | Wall - standard wall switch |
| 0x1A | KEY_HOLE | Wall - requires key |
| 0x1D | COUNTER | Logic - increment/decrement counter |
| 0x1E | TICK_GENERATOR | Timing - fires at intervals |
| 0x20 | RELAY_1 | Wiring - forwards trigger |
| 0x21 | ARRIVAL_DEPARTURE | Wiring - fires on enter/exit |
| 0x22 | FLYING_ITEM_CATCHER | Floor - catches projectile |
| 0x23 | FLYING_ITEM_TELEPORTER | Floor - teleports projectile |
| 0x26 | SWITCH_SIGN_FOR_CREATURE | Wall - creature-activated switch |
| 0x2A | ALCOVE_ITEM_TRAP | Trap - damages items |
| 0x2C | ORNATE_ANIMATOR | Visual - animates wall ornate |
| 0x2E | CREATURE_GENERATOR | Spawn - spawns creature |
| 0x31 | WORK_TIMER | Timing - delayed fire |
| 0x3B | PLACED_ITEM_TELEPORTER | Teleport - teleports placed items |
| 0x3C | ITEM_GENERATOR | Spawn - spawns item |
| 0x3D | RELAY_2 | Wiring - forwards trigger |
| 0x3F | SHOP_PANEL | Shop - opens shop interface |
| 0x40 | ITEM_RECYCLER | Utility - removes items |
| 0x46 | PUSH_BUTTON_WALL_SWITCH | Wall - push button |
| 0x47 | ITEM_CAPTURE | Floor - captures item |
| 0x7E | RESURECTOR | Champion - resurrection timer |
| 0x7F | CHAMPION_MIRROR | DM1 retro |

Source: docs/dm2_triggers.md, docs/dm2_actuators.md

---

## 3. Trigger Invocation Path

1. Player/creature activates a **sensor** (steps on floor, uses wall switch, etc.)
2. DM2_INVOKE_ACTUATOR is called with the actuator pointer, a flag value, and a delay
3. The function computes a **future game-tick** when the actuator fires (based on delay parameter)
4. The actuator is placed in a **pending queue**
5. DM2_PROCESS_ACTUATOR_TICK_GENERATOR() runs each game tick, dequeues expired actuators
6. For each expired actuator: reads type-specific data, executes the effect (fire projectile, toggle door, spawn creature, etc.)

---

## 4. Wall Switch Interaction

Wall switches (ACTUATOR_TYPE_WALL_SWITCH = 0x18, ACTUATOR_TYPE_2_STATE_WALL_SWITCH = 0x17, ACTUATOR_TYPE_PUSH_BUTTON_WALL_SWITCH = 0x46):

- Player must **face the wall** and press a key (or click)
- Switch toggles state (on/off for 2-state, momentary for push button)
- Can be linked to doors, trapdoors, projectile shooters, or any other actuator
- Key hole (ACTUATOR_TYPE_KEY_HOLE = 0x1A) requires having the right key in inventory

Interaction path:
1. Player faces wall with switch
2. Input system routes to c_input / c_keybd
3. Actuator invocation: DM2_INVOKE_ACTUATOR(switch_actuator, flag, delay)
4. Pending queue updated
5. On tick: actuator effect fires

---

## 5. Floor Interaction

Floor actuators (ITEM_WATCHER, CHARGED_ITEM_WATCHER, FLYING_ITEM_CATCHER, FLYING_ITEM_TELEPORTER, ITEM_CAPTURE, ARRIVAL_DEPARTURE):

- Triggered when party/creature steps on the cell
- Some detect specific items (ITEM_WATCHER - fires when specific item enters/exits)
- Some detect all items (ITEM_CAPTURE - removes item from floor)
- FLYING_ITEM_* handle projectiles in flight

---

## 6. Teleporter Interaction

Teleporter squares (GDAT_CATEGORY_TELEPORTERS = 0x18) are a dedicated category:

- X teleporter (SDFSM_CMD_X_TELEPORTER = 4) - cross-scene teleport, activated by stepping on the floor square
- Anchor teleporter (SDFSM_CMD_X_ANCHOR = 5) - anchored teleporter in Sun Clan village

Activation: party steps on teleporter square, DM2_TELEPORT_TO_POSITION is called.

---

## 7. Ladder/Stairs Interaction

Ladders (ACTUATOR_TYPE_LADDER = 0x11, ACTUATOR_TYPE_SIMPLE_LADDER = 0x1C):

- Standard ladder: 0x11 - moves party vertically between levels
- ACTUATOR_TYPE_SIMPLE_LADDER (0x1C): noted as beta-only in skproject
- GDAT_WALL_ORNATE__IS_LADDER_UP field: 1 = ladder going up, absent = ladder going down

Activation: party faces ladder and presses interact key.

---

## 8. Shop Interaction

ACTUATOR_TYPE_SHOP_PANEL (0x3F):
- Triggers the shop interface when activated (approaching a shop-keeper NPC or panel)
- Opens a shop UI where champions can buy/sell items
- Related to the companion system - shop NPCs are part of DM2's world populated with NPCs

---

## 9. Item Interaction

Item pickup: party walks over item on floor, item is added to party inventory.

Item use from inventory:
- Potions: consumed to apply effect
- Scrolls: cast associated spell
- Weapons: equipped to champion slot
- Keys: used with KEY_HOLE actuators

GDAT_CATEGORY_WEAPONS (0x10) - extended weapon data with projectile flags.

---

## 10. Companion Interaction

DM2 introduces companion NPCs:
- Companions can be recruited (interaction with NPC in world)
- Companion loyalty (0-100) affects behavior
- Companion mode switching: 0=follow, 1=guard, 2=aggressive
- Champions can die and be resurrected (RESURRECTOR actuator 0x7E)

Player interacts with companions primarily through the UI (give orders, view stats) rather than through world interaction.

---

## 11. Input System

Input routing (from dm2_input.md and skproject):
- CSkWin (SkWin.h): xMiceInput[MAXMICEIN], xKeybInput[MAXKEYBIN] - ring buffers
- DequeueKinput() / allocMinput() - allocate from ring buffers
- c_input: high-level input processing
- c_keybd: keyboard scancode to game-key mapping
- c_mcursor: mouse cursor display
- c_tmouse: tracked mouse state
- Double-step movement flag (enableDoubleStepMove)

Player actions: keyboard (WASD replacement for N/S/E/W + facing), mouse (click on viewport to move/interact).

---

## 12. Cross-Map Wiring

ACTUATOR_TYPE_CROSS_MAP (0x16) allows one actuator to trigger another map's actuator. This enables global event chains across level transitions.

Example: stepping on a floor plate in one room opens a door in a different room.

---

## 13. Comparison: Interaction

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Trigger abstraction | Hardwired tile types | Generic actuator records |
| Trigger placement | Bound to specific tile types | Any floor or wall cell |
| Number of trigger types | ~5 (switch, plate, trap, door, item) | 40+ actuator types |
| Cross-map triggers | Not possible | Via CROSS_MAP actuator (0x16) |
| Delay/timer support | Very limited | TICK_GENERATOR (0x1E), WORK_TIMER (0x31) |
| Event chaining | Single-step | Multi-step relay chains (RELAY_1/2) |
| Data location | Compiled in binary | Dungeon data file |
| Shop interaction | None | SHOP_PANEL (0x3F) |
| Teleporter | Generic floor | Dedicated GDAT 0x18 category |
| Companion interaction | N/A | UI-based, mode switching |
| Key-locked doors | Simple key check | KEY_HOLE actuator (0x1A) |

---

## STATUS: AUDIT COMPLETE