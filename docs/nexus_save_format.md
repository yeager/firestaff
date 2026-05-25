# Nexus V1 Save File Format

## Status: NOT DEFINED

There is **no save file format** defined or implemented in the Nexus V1 engine.
No binary serialization, no magic bytes, no header structures.

## What State Structures Exist

If a save system were implemented, it would serialize these structs:

### `Nexus_V1_GameState`
```
current_level    : int
party_x          : int
party_y          : int
party_dir        : int
game_started     : int
data_dir         : const char*
```

### `Nexus_V1_ChampionPool`
```
champions[]      : array[NEXUS_MAX_CHAMPIONS=24] of Nexus_V1_Champion
champion_count   : int
party[]          : array[NEXUS_MAX_PARTY=4] of champion indices
party_count      : int
leader_index     : int
```

### `Nexus_V1_Champion`
```
name_ascii[32]   : char
name_jp[64]     : char
primary_class    : Nexus_ChampionClass enum
health/max_health: int
stamina/max_stamina : int
mana/max_mana    : int
strength/dexterity/wisdom/vitality : int
anti_magic/anti_fire : int
fighter/ninja/priest/wizard_level : int
food/water       : int
alive            : int
portrait_index   : int
inventory[30]    : uint8_t item indices
```

### `Nexus_V1_Level`
```
width            : uint16_t
height           : uint16_t
squares[y][x]    : uint16_t[32][32] (5-bit wall type per cell)
has_3d_geometry  : int
geometry_offset  : int
geometry_size    : int
```

## Original Game Format Context

The original Saturn game used **memory card blocks** (8 KB per save) with a
proprietary header tracking: party data, champion state, dungeon progress,
and inventory. No file-level save image is reverse-engineered in this codebase.

## Conclusion

No save format is defined. A future implementation would need to design a
binary format covering game state, champion state, inventory, and dungeon
progress for all 16 levels.