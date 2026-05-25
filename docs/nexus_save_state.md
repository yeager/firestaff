# Nexus V1 Saved State — What Is Tracked

## Status: ENGINE EXISTS, SAVE NOT IMPLEMENTED

The Nexus V1 engine defines all the state structures that **would** be saved,
but no save/load mechanism exists to persist them.

## State Structures Defined

### Game State (`Nexus_V1_GameState`)
- `current_level` — dungeon level index (0–15)
- `party_x`, `party_y` — party grid position
- `party_dir` — facing direction (0=North, matching DM1)
- `game_started` — boolean flag
- `data_dir` — path to game data

### Champion State (`Nexus_V1_Champion`)
Per champion, 24 exist in roster:
- Identity: `name_ascii`, `name_jp`, `primary_class`, `portrait_index`
- Vital stats: `health/max_health`, `stamina/max_stamina`, `mana/max_mana`
- Attributes: `strength`, `dexterity`, `wisdom`, `vitality`
- Resistances: `anti_magic`, `anti_fire`
- Class levels: `fighter_level`, `ninja_level`, `priest_level`, `wizard_level`
- Survival: `alive`
- Provisions: `food`, `water` (starting at 1500 each)
- Inventory: `inventory[30]` — array of item indices

### Champion Pool (`Nexus_V1_ChampionPool`)
- `champions[24]` — full roster
- `party[4]` — active party champion indices
- `party_count`, `leader_index`

### Dungeon Level (`Nexus_V1_Level`)
- `width`, `height` — grid dimensions (typically 32×32)
- `squares[y][x]` — 5-bit wall type per cell (16×16 possible tile types)
- `has_3d_geometry` — flag
- `geometry_offset`, `geometry_size` — raw 3D geometry data

## What Would Be Saved

A complete save would need to persist:
1. **Game progress**: current level, party position/direction
2. **Champions**: all 24 champions (alive status, HP/stamina/mana, class levels,
   food/water, inventory)
3. **Party composition**: which 4 champions are active, leader
4. **Dungeon state**: current level grid (potentially with dynamic changes)
5. **3D geometry**: raw geometry data for loaded levels

## What Is NOT Tracked

- Quest progress or events (no event system yet)
- World map state (Nexus is dungeon-only currently)
- Any per-champion skill/spell progression beyond class levels
- Chest/door states (no interaction system fully implemented)

## Notes

The 8 champion characters have Japanese names (UTF-8) and are mirrored from
DM1's system (same stats, classes, mechanics). The 30-slot champion inventory
matches DM1's item capacity.