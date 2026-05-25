# Nexus V1 — Difficulty Modes

## Sources
- `docs/nexus_overview.md`
- `docs/nexus_features.md`
- `docs/nexus_menus.md`
- `src/nexus/nexus_v1_engine.c`, `nexus_v1_game.c`
- `docs/NEXUS_PLAN.md`

---

## 1. No Difficulty Settings

Dungeon Master Nexus **has no selectable difficulty modes** (Easy/Normal/Hard, etc.). This is consistent with the entire DM series:

| Game | Difficulty Options? |
|------|---------------------|
| DM1 | No |
| CSB | No |
| DM2 | No |
| **Nexus** | **No** |

The disc image T-9111G V1.003 is a single binary with no difficulty variant files.

---

## 2. Where Difficulty Comes From

Without selectable modes, difficulty in Nexus arises from:

### A. Champion Composition
- Party of 4 vs. solo run — difficulty scales dramatically with party size
- Class mix: Fighters provide durability, Wizards/Ninjas provide offense
- Anti-Magic and Anti-Fire resistances (start at 5 in Nexus vs. 0 in DM1)

### B. Dungeon Design — 16 Levels vs. DM1's 10
Nexus has 16 dungeon levels (LEV00–LEV15) vs. DM1's 10 levels. The extra 6 levels represent a deeper dungeon with corresponding difficulty scaling:
- LEV00: Entry level (relatively easy)
- LEV01–LEV07: Standard dungeon progression
- LEV08–LEV12: Mid-to-deep dungeon, harder encounters
- LEV13–LEV15: Deepest levels, highest creature HP/damage

### C. Creature Stats
From `nexus_v1_creatures.c`, creature HP and damage values in Nexus are tuned to the 3D rendering era — no direct comparison to DM1 sprite frame damage values is available in documentation, but the game is known to be challenging at depth.

### D. Viewport Distance Change
DM1 had 2-square visibility. Nexus has **4-square visibility**. This is a mechanical difficulty change:
- More warning time in corridors = easier navigation
- But deeper levels compensate with harder creatures and tighter spaces

### E. Food and Water
Same 1500-unit food/water pool as DM1. No changes documented to consumption rate.

---

## 3. No Variant Difficulty Files

The extracted Nexus ISO (137 files) contains no variant files for difficulty:
- No separate LEV*.DGN files for "hard mode"
- No separate champion stat tables
- No difficulty toggle in the engine (`nexus_v1_engine.c` has no difficulty field in `Nexus_V1_GameState`)

Evidence from `Nexus_V1_GameState` struct (from source):
```c
struct Nexus_V1_GameState {
    int  current_level;
    int  party_x, party_y;
    int  party_dir;
    bool game_started;
    char data_dir[256];
};
```
No difficulty field exists.

---

## 4. Comparison with DM Series

| Difficulty Aspect | DM1 | CSB | DM2 | Nexus |
|------------------|-----|-----|-----|-------|
| Selectable difficulty | No | No | No | **No** |
| Party size (max) | 4 | 4 | 4 + companions | **4** |
| Levels (dungeon) | 10 | 10 | 10 + outdoor | **16** |
| Visibility (squares) | 2 | 2 | 2 | **4** |
| Food/Water pool | 1500 | 1500 | 1500 | **1500** |
| Creature difficulty scaling | Depth | Depth | Depth | **Depth + 16 levels** |

---

## 5. Difficulty Conclusion

Nexus offers **one game mode**: Japanese-language first-person dungeon crawl through 16 levels with a party of up to 4 champions. Difficulty is determined entirely by player choices (party composition, inventory, skill) and natural dungeon depth progression — not by a settings menu toggle.
