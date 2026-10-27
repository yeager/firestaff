# Nexus V1 Overworld Audit

## 1. No Overworld in DM1 — Nexus Has One

DM1 (Dungeon Master) is entirely dungeon-based. The party descends through
8 levels (D0-D7) with no external map. Nexus for Saturn is the first game
in the series to include an overworld map connecting the dungeon to the
outside world.

## 2. Overworld in Nexus Saturn

The overworld is NOT stored in the LEV*.DGN files (those are dungeon levels).
The overworld is believed to be part of a separate map data structure or
level file accessible via the Saturn CD game data.

Key evidence from firestaff source:
- Nexus_V1_Engine.game has fields: party_x, party_y, party_dir (dungeon position)
- There is no explicit overworld state in nexus_v1_game.h currently
- This suggests overworld implementation is TODO / future work in firestaff

## 3. Comparison: Overworld Presence

| Game        | Has Overworld? | Location of Map Data |
|-------------|---------------|---------------------|
| DM1         | NO            | N/A (pure dungeon)  |
| Chaos Strikes Back | NO       | N/A (pure dungeon)  |
| DM2 / Skullkeep | YES      | Separate world map  |
| DM Nexus    | YES (Saturn)  | Separate from DGN dungeon files |

## 4. Overworld vs Dungeon Architecture in Nexus

Nexus splits the game world into two disjoint data spaces:

DUNGEON: 16 levels stored in LEV00-LEV15.DGN (DMWeb 2048-byte block container; Structure1B is 64x64 cells)
OVERWORLD: Separate map data, not loaded as a DGN file

The party transitions from overworld to dungeon via a special entrance square
(likely a teleporter or stairs type), similar to how DM1 transitions between
levels via stairs squares.

## 5. Firestaff Implementation Status

Current firestaff Nexus V1 engine focuses on dungeon rendering (viewport,
3D rasterizer). Overworld is not yet implemented. Evidence:

- nexus_v1_game.h: no overworld state fields
- nexus_v1_dungeon.c: only handles DGN file loading
- No overworld map file referenced in data directory listing

## 6. Design Implications

The overworld map in Nexus serves the same function as in DM2:
- Provides spatial context beyond the dungeon
- Allows navigation between multiple dungeon entrances
- Supports outdoor quests / exploration

Unlike DM2 which stored world map in a separate archive, Nexus Saturn
likely stores overworld data in an undocumented structure on the CD,
possibly within DM.BIN or as a separate asset file not yet identified
in the firestaff codebase.
