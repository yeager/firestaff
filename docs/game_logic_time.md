# DM1 V1 — Time & Turn System

## Source
ReDMCSB: `GAMELOOP.C:124–160` (F0256_MAIN_Tick_CPSDF), `TIMELINE.C` (F0233–F0261),
`CHAMPION.C:2295–2370` (F0297_CHAMPION_ProcessTick)

---

## 1. The Tick — Fundamental Time Unit

`G0313_ul_GameTime` is a **32-bit unsigned** clock incremented by 1 every game tick.

```
G0313_ul_GameTime++  // per iteration of main game loop
```

~10 ticks/second (exact rate hardware-dependent). One tick is the basic unit for:
- Stamina consumption
- Mana regeneration checks
- Temporary XP decay
- Event timeline expiration

**CRITICAL BUG** (`GAMELOOP.C:124`): `G0313_ul_GameTime` is 32-bit but event times in
the timeline are stored in **24 bits** (`Map_Time`). Max safe value: 2^24-1 = 16,777,215
(~850–1000 hours). Beyond this, watchdog events expire immediately → SYSTEM ERROR 60 hang.

---

## 2. Per-Tick Activities

**Source: GAMELOOP.C:124–160, CHAMPION.C:2295–2370**

Each tick:
- `G0313_ul_GameTime++`
- Every **512 ticks**: decay party conditions
- Every **64 ticks**: update random encounter seeds
- Every **64 ticks** (16 if resting): stamina/hunger/water consumption
- Every **63 ticks**: champion skill temporary XP decay + stamina recovery
- Every **tick**: event queue drain — process all events where `eventTime <= G0313_ul_GameTime`

---

## 3. Event Timeline — Binary Min-Heap

**Source: TIMELINE.C:330–420** (F0236_TIMELINE_FixPlacement)

The timeline stores scheduled events in a **binary min-heap** (array `G0371_pui_Timeline[]`,
count `G0372_ui_EventCount`). Earliest event is always at index 0.

**Event structure** (`DM1_Event_V1`):
- `map_time`: packed (mapIndex << 24) | gameTime  — 32-bit, 8-bit map + 24-bit time
- `type` / `priority`: event classification (Type_Priority for tiebreaking)
- `location`: MapX, MapY (16-bit each)
- `data`: event-specific payload (ticks, slot, etc.)

**Key event types:**
| Type | Name | Purpose |
|------|------|---------|
| C01 | EVENT_DOOR_ANIMATION | Door open/close animation |
| C05 | EVENT_CORRIDOR | Dungeon behavior triggers (pits, creature spawns) |
| C10 | EVENT_DOOR | Auto-close door after opening |
| C11 | EVENT_ENABLE_CHAMPION_ACTION | Re-enable action after attack |
| C12 | EVENT_HIDE_DAMAGE_RECEIVED | Remove floating damage number |
| C13 | EVENT_VI_ALTAR_REBIRTH | Champion resurrection |
| C53 | EVENT_WATCHDOG | Detect hung game |

**Event ordering** (`F0234_TIMELINE_IsEventABeforeEventB`):
1. Earlier time wins
2. If simultaneous: higher `Type_Priority` wins
3. If still tied: lower memory address (= lower index) wins

**Sensor decode formula** (`M046_TICKS`, `TIMELINE.C:987–995`):
```
ticks = raw_sensor_ticks
if (ticks > 127) ticks = (ticks - 126) << 6
eventTime = G0313_ul_GameTime + ticks
```

---

## 4. F0325 — Stamina Decrement

`F0325_CHAMPION_DecrementStamina(championIndex, amount)` — `CHAMPION.C:2025–2120`

Called throughout: combat attacks, throwing, mana regen, movement (load-based cost).
**NOTE**: F0325 is stamina decrement, NOT a time formula. The per-tick champion
processing is in `F0297_CHAMPION_ProcessTick` (`CHAMPION.C:2295–2370`).

---

## 5. Dungeon Time vs Real Time

Time only advances during the main game loop. Pausing (menus, dialogs) freezes time.

**Resting** (`G0300_B_PartyIsResting`):
- Doubles stamina recovery rate
- Halves time criteria for mana regeneration (mana ticks more frequently)
- Set when party clicks "Wait"

---

## 6. Scent Tracking System

**Source: CHAMPION.C:2295–2370**

Each champion has `Scents[]` tracking detected creature positions.
- `ScentCount`: active entries
- `Scents[]`: (MapX, MapY, MapIndex, ScentValue) tuples
- `ScentStrengths[]`: decay counters (1 per tick)

Party generates a scent at its current position. Creatures with SMELL attribute
use these to pursue the party.

---

## 7. Firestaff Implementation

**File:** `src/dm1/dm1_v1_event_timer_pc34_compat.c`
- Full binary min-heap: `dm1v1_event_queue_*` functions
- `dm1v1_event_add(scheduleTime, mapIndex, eventType, ...)` — schedule
- `dm1v1_event_process_tick(currentGameTime)` — drain expired events
- `DM1_EventQueue_V1` — 64-entry event queue

**File:** `src/dm1/dm1_v1_game_loop_pc34_compat.c`
- `dm1v1_tick()` — per-tick update (stamina, mana, XP decay, event drain)

**File:** `src/dm1/dm1_v1_game_loop_integration_pc34_compat.c`
- Bridge between engine and DM1 V1 tick system

**Parity status:** FULL for event scheduling and tick processing.
Partial: scent tracking not yet implemented.
