# DM1 V1 — Champion AI (Auto-Play)

**Source-locked to:** ReDMCSB WIP20210206 CHAMPION.C, GROUP.C, DEFS.H, MOVESENS.C
**Companion:** src/dm1/dm1_v1_champion_stats_pc34_compat.c

---

## 1. No Automatic Champion AI

**DM1 V1 has no autonomous champion AI.** Champions are purely player-controlled
via the command queue system (INPUT.C → command queue → GAMELOOP execution).

There is no equivalent of:
- Automatic attack decisions
- Automatic spell casting
- Automatic movement pathfinding for champions
- Champion "think" or "decide" functions

The party moves only when the player issues commands.

---

## 2. Command Queue System (Input Pipeline)

Champion actions flow through a **queued command system**:

```
User input (mouse/keyboard)
  → INPUT.C: click events, keyboard events
  → Command queue (per-champion pending action slots)
  → GAMELOOP.C: each game tick, process queue
  → CHAMPION.C: execute commands (attack, cast spell, move, use item)
```

Relevant Firestaff files:
- dm1_v1_input_poll_pc34_compat.c — input polling
- dm1_v1_input_command_queue_pc34_compat.c — command queue
- dm1_v1_champion_stats_pc34_compat.c — champion stat management

---

## 3. Click Routing — Champion Selection

Click routing is the mechanism by which user clicks on a champion portrait
or panel select a specific champion for the next command:

- F0200_CHAMPION_ClickChampion (CEDT020.C): champion portrait click
- F0201_CHAMPION_ClickPortrait (CEDT020.C): panel portrait click
- F0202_CHAMPION_ClickActionBar (CEDT020.C): action bar / slot click

Champion selection is stored in G0301_i_SelectedChampionIndex (DEFS.H).

When the player clicks on a dungeon square (move command):
- MOVESENS.C click routing → determine target
- Command queued for selected champion
- Execution deferred until next game tick

---

## 4. What IS Automatic (Passive Systems)

### 4a. Food/Water Consumption (Automatic Drain)

Champions consume food and water over time (per-tick drain):
- dm1_v1_champion_stats_pc34_compat.c:m11_stats_tick(): fixed 1 per tick
- When food = 0: health decreases 1 per tick
- This is automatic, not AI — it's a passive resource system

### 4b. Stamina Regeneration

- m11_stats_tick(): stamina tick counter, +1 stamina every 3rd tick
- Only when stamina < max; no champion decision involved

### 4c. Poison Damage

- If poisoned: health decreases by poisonAmount each tick
- Automatic, no champion action required

### 4d. Death/Resurrection

- Health ≤ 0 → champion marked dead (alive=0)
- Resurrect at altar with HP restoration
- No AI pathfinding to nearest altar

### 4e. Champion Stats (No AI)

- Fighter/Ninja/Priest/Wizard skill advancement
- XP awarded by sensors (rotation/XP sensors)
- No automatic stat allocation

---

## 5. Auto-Fight / Auto-Attack

There is **no auto-fight mode** in DM1 V1. All combat requires player input.

The closest equivalent is the **Giggler steal mechanic** which, after
stealing, causes the Giggler to flee automatically (C5 BEHAVIOR_FLEE).
This is creature AI, not champion AI.

---

## 6. No Champion Pathfinding

Champions do not navigate the dungeon autonomously. When a champion dies:
- They are marked dead (alive=0, stats cleared)
- They cannot move or act
- The player must resurrect them at a RESURRECT sensor (altar)

There is no "return to town" or "walk to nearest safe point" behavior.

---

## 7. Key Source Citations

| Function | File:Line | Role |
|---|---|---|
| CHAMPION.C F0306 (stamina-adjusted value) | CHAMPION.C:1078 | Stamina effect on actions |
| INPUT.C / CEDT020.C champion click routing | CEDT020.C | Champion selection |
| G0301_i_SelectedChampionIndex | DEFS.H | Selected champion state |
| G0305_ui_PartyChampionCount | DEFS.H | Living champion count |
| F0276_PROCESS_FLOOR sensors | MOVESENS.C | Floor sensor trigger (party) |
| m11_stats_tick() | dm1_v1_champion_stats_pc34_compat.c | Passive food/water/poison drain |
| m11_stats_stamina_adjusted_value_pc34() | dm1_v1_champion_stats_pc34_compat.c | Stamina penalty calc |

---

## 8. Summary Table

| System | Automated? | Notes |
|---|---|---|
| Champion movement | No | Player click-to-move only |
| Champion attack | No | Player click-to-attack only |
| Champion spell casting | No | Player selects spell + target |
| Stat regeneration | Yes | Stamina regen every 3rd tick |
| Resource drain | Yes | Food/water -1 per tick |
| Poison damage | Yes | -poisonAmount per tick |
| Champion selection | No | Player click on portrait |
| Auto-fight | No | Not present in DM1 V1 |

**Conclusion: DM1 V1 has no champion AI. Champions are 100% player-driven.**
The only "automatic" systems are passive resource drains and stat mechanics.