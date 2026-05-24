# DM1 V1 Overworld Map — Source Locked

## Finding: DM1 V1 Has No Overworld/Field Map

DM1 V1 (Dungeon Master, PC/Amiga/Atari ST, all versions) does not have an
overworld, world map, or field map. The dungeon is a self-contained unit.
There is no separate map screen or mode for traveling between dungeons on
a world surface.

Source evidence:
- ReDMCSB include/dm1_v1_field_teleporter_effect_pc34_compat.h:5:
  Note: DM1 has no overworld/field map — teleporters are in-dungeon
- ReDMCSB NEWMAP.C (83 lines): only contains F0003_MAIN_ProcessNewPartyMap_CPSE,
  which handles switching between dungeon maps — not a world map.
- No file in ReDMCSB source references a world/overworld map data structure.

## Entrance Map

The only world-level entity is the Entrance Map (index 255):

- DEFS.H:1118 — CM1_MAP_INDEX_NONE = -1
- DEFS.H:1119 — C255_MAP_INDEX_ENTRANCE = 255
- ENTRANCE.C:68 — G0309_i_PartyMapIndex = C255_MAP_INDEX_ENTRANCE
- DRAWVIEW.C:829 — Guard: if (G0309_i_PartyMapIndex != C255_MAP_INDEX_ENTRANCE)
- DUNVIEW.C:8606 — Guard: G0309_i_PartyMapIndex != C255_MAP_INDEX_ENTRANCE

The entrance is shown on the title screen when starting a new game (or loading
a saved game from the title). It displays the dungeon facade/door art with
ambient animation. When the party enters, G0309_i_PartyMapIndex transitions
from 255 to the first dungeon map (index 0 typically, via F0174_DUNGEON_SetCurrentMapAndPartyMap).

## Map Index System

All dungeon maps are stored in G0277_ps_DungeonMaps[] (array of MAP structs),
with count G0278_ps_DungeonHeader->MapCount. Each map has:
- Level — dungeon depth (-1 to -14, negative = below surface)
- OffsetMapX, OffsetMapY — position in world grid (see overworld_coords.md)
- Width, Height — map dimensions in squares

There is no unified world-space tile array. Each map is a discrete unit.

## Conclusion

DM1 V1 does not support navigating between dungeons via an overworld map.
Inter-dungeon travel is handled by:
1. Stairs within a dungeon complex (level transitions, same world position)
2. Teleporters (in-dungeon, potentially cross-complex)
3. Returning to the entrance (exit/re-enter flow)
