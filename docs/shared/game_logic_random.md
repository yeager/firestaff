# DM1 V1 — Random Encounters & Event Triggers

## Source
ReDMCSB: `TIMELINE.C` (F0233–F0261, F0209_GROUP_ProcessEvents29to41),
`GROUP.C` (F0185–F0225), `MOVESENS.C`, `DUNGEON.C`

---

## 1. Creature Spawning — No Random Dice Rolls

DM1 does **not** use random-encounter dice. Creatures are **placed in dungeon data**
(`DUNGEON.DAT`) at specific map squares. Events control when they appear.

### Dungeon Loading
- Groups (up to 4 creatures in quarter-square cells) are defined per square in dungeon data
- Loaded at map load time by `F0124_DUNGEON_LoadFloor`
- Some groups are hidden behind sensors; sensors reveal them when triggered

### Sensor-Based Activation
**Source: TIMELINE.C:987–1020, MOVESENS.C**

Sensors (invisible dungeon squares, `C4_CELL_SENSOR`) trigger events:
```
ticks = M046_TICKS(sensor)
if (ticks > 127) ticks = (ticks - 126) << 6  // decompress
schedule: type=C05_EVENT_CORRIDOR, time=gameTime + ticks
```

---

## 2. Behavior Event Processing

**Source: TIMELINE.C:1850–1920, GROUP.C F0209_GROUP_ProcessEvents29to41**

When C05_EVENT_CORRIDOR fires:
1. Get square first thing
2. If GROUP: dispatch behavior based on `Ticks` field (behavior type):
   - `T0209044_SetBehavior6_Attack` — creature attacks party
   - `T0209085_SingleSquareMove` — creature moves one square
   - `T0209094_FleeFromTarget` — creature flees
   - etc.
3. If not GROUP: execute square effect (pit open/close, door, etc.)

---

## 3. Creature AI Behaviors

**Source: GROUP.C F0190–F0225, F0209_GROUP_ProcessEvents29to41**

| Behavior | Description |
|----------|-------------|
| 0 IDLE | Stand still |
| 1 WANDER | Random movement |
| 2 SEEK | Move toward last detected party position |
| 3 FLEE | Move away from party |
| 4 ATTACK | Melee attack |
| 5 SPECIAL | Special (Vexirk ranged, Giggler steal) |
| 6 BLOCK | Block party movement |
| 7 WALL | Wall state |

### Scent Detection — SMELL attribute
**Source: GROUP.C F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal**
- Each champion emits scent at position
- Scent decays 1/tick (`CHAMPION.C:2295–2370`)
- Creature picks direction of strongest detected scent

### Sight Detection — SIGHT attribute
**Source: GROUP.C F0200_GROUP_GetDistanceToVisibleParty, F0202_GROUP_IsMovementPossible**
- Raycasts through dungeon squares
- Wall/FakeWall blocks line-of-sight
- Returns Manhattan distance to nearest visible party square

---

## 4. Dungeon Square Types

**Source: DEFS.H, DUNGEON.C**

| Cell Type | Description |
|-----------|-------------|
| C0 CELL_EMPTY | Floor |
| C1 CELL_WALL | Solid wall |
| C2 CELL_DOOR | Door (open/closed/animating/destroyed) |
| C3 CELL_PILLAR | Blocks movement |
| C4 CELL_SENSOR | Invisible trigger |
| C5 CELL_PIT | Open/close states |
| C6 CELL_FAKEWALL | Imaginary wall |
| C7 CELL_TELEPORTER | Warp square |

### Door States: CLOSED / OPEN / OPENING / CLOSING / DESTROYED

---

## 5. Time-Based Events

| Event | Source | Purpose |
|-------|--------|---------|
| C05 CORRIDOR | TIMELINE.C:1893 | Dungeon behaviors, creature activation |
| C10 DOOR | TIMELINE.C:1881 | Auto-close door |
| C13 VI_ALTAR_REBIRTH | TIMELINE.C:1995 | Champion resurrection |
| C11 ENABLE_CHAMPION_ACTION | TIMELINE.C:1927 | Re-enable action after attack |
| C12 HIDE_DAMAGE_RECEIVED | TIMELINE.C:1933 | Floating damage removal |
| C53 WATCHDOG | TIMELINE.C:1714 | Detect hung game (300 ticks ahead) |

---

## 6. Notable Bugs

**BUG0_18** (TIMELINE.C:514): Timeline full → projectiles freeze, explosions
persist forever, champion actions disabled.

**BUG0_59** (ENDGAME.C:841): Fluxcage removal from Lord Chaos square fails →
infinite loop during endgame sequence.

**BUG0_61** (STARTUP2.C:1168): Watchdog time not reset on load → SYSTEM ERROR 60
if save game time > watchdog time.

---

## 7. Firestaff Implementation

**File:** `src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c`
- `DM1GroupBehaviorContext_Compat` — creature vision/scent/size/context
- `DM1BehaviorResult_Compat` — chosen movement or attack
- `F0810_GROUP_ProcessEvents29to41` — event dispatcher
- `F0818_GetDistanceToVisibleParty` — sight raycasting
- `F0819_GetSmelledPartyDirection` — scent tracking

**File:** `src/dm1/dm1_v1_sensor_trigger_pc34_compat.c`
- Sensor decode: `M046_TICKS` tick formula
- Event type C05/CORRIDOR behavior dispatch

**File:** `src/dm1/dm1_v1_dungeon_square_structs_pc34_compat.c`
- Square type enums (EMPTY, WALL, DOOR, PIT, etc.)

**Parity status:** FULL for creature AI behaviors and sensor decode formula.
Partial: event dispatch not fully wired to all behavior types.
