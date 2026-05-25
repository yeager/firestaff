# GAP C01–C24: Hall of Champions Champion Stat Stubs

## Status
**GAP — Champion stat initialization is flat/default; no G0243-matched class derivation**

## Hall of Champions Context

The Hall of Champions (champion creation/recruitment screen) is implemented in:
- `src/dm1/dm1_v1_entrance_champion_select_pc34_compat.c` — screen flow
- `src/dm1/dm1_v1_champion_stats_pc34_compat.c` — stat access layer

Champion stat generation is handled by `m11_stats_init()` and `m11_stats_add_champion()`.

## Gap: Flat Default Stat Initialization

`m11_stats_add_champion()` initializes all newly created champions with identical flat defaults regardless of class, name, or any other attribute:

```c
stats[DM1_STAT_HEALTH]     = 100;
stats[DM1_STAT_STAMINA]   = 100;
stats[DM1_STAT_MANA]      = 50;
stats[DM1_STAT_STRENGTH]  = 30;
stats[DM1_STAT_DEXTERITY] = 30;
stats[DM1_STAT_WISDOM]    = 30;
stats[DM1_STAT_VITALITY]  = 30;
stats[DM1_STAT_ANTIFIRE]  = 10;
stats[DM1_STAT_ANTIMAGIC] = 10;
stats[DM1_STAT_LUCK]      = 10;
```

This is a generic stub. No class-specific stat bonuses are applied (e.g., Fighter STR+, Wizard WIS+, Priest VIT+).

## G0243 Not Used for Champion Generation

`g_graphicInfoCreature[]` (from `G0243_as_Graphic559_CreatureInfo`, source: `dm1_v1_creature_render_pc34_compat.c:47–72`) defines per-creature graphic info, aspect bits, and animation state. This data is used for dungeon creature rendering but has no mapping to champion stat generation.

There is no `G0243_match` function or equivalent that cross-links dungeon creature stats to champion starting stats.

## 5 Flag Bugs in Creature Aspect Bits

The `aspectBits` field (stores `MASK0x0020_LEVITATION`, `MASK0x0040_FLIP_BITMAP`, `MASK0x0080_IS_ATTACKING`) is set per creature in `g_graphicInfoCreature[]`. The following inconsistencies are documented:

| Creature | Type Index | aspectBits (hex) | Bug |
|---|---|---|---|
| **C12 Black Flame** | 11 | `0x00` | Both `LEVITATION` (0x20) and `NON_MATERIAL` tag are missing. Black Flame is an ethereal fire creature; should have `NON_MATERIAL`. Value `0x00` has neither flag. |
| **C20 Materializer** | 19 | `0xA9` | Missing `LEVITATION` flag (0x20). Materializer is a phasing/etheric creature; `0xA9` has no levitation bit. |
| **C14 Vexirk** | 14 | `0x30` | `LEVITATION` bit (0x20) missing. Vexirk is a floating/hovcering creature. |
| **C15 Magenta Worm** | 15 | `0x78` | `LEVITATION` bit (0x20) present but partially masked/conflicting with worm ground movement. |
| **C21 Oitu** | 21 | `0xA9` | Same `0xA9` tag as Materializer — likely a copy-paste error from adjacent entry. Oitu should have distinct aspect bits. |

C24 Red Dragon uses `0xCB` (identical to C23 Lord Chaos and C26 Grey Lord) — Red Dragon should have distinct flight/hover bits separate from humanoid lords.

## Stats Not G0243-Matched

Champion stat generation is entirely disconnected from G0243 CreatureInfo data:
- No Fighter/Ninja/Priest/Wizard class bonuses applied at creation
- No race or name-based adjustments
- No difficulty scaling
- All champions arrive at identical 30/30/30 physical stats

## Impact
- Normal play: 30/30/30 base stats are playable for starting champions
- Hardcore/modding: class identity (Fighter strong, Wizard wise, etc.) is purely cosmetic
- Flag bugs: spell effects targeting `NON_MATERIAL` or `LEVITATING` creatures may not apply correctly to affected creature types

## Required Fix (Non-Blocking)
1. Add class-specific stat bonuses to `m11_stats_add_champion()` keyed by selected class
2. Audit `aspectBits` for C12, C14, C15, C20, C21, C24 against DOS runtime capture
3. Add champion class enum `M11_CHAMPION_CLASS_*` and wire into stat generation
