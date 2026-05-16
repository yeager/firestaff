# Pass604 — DM2 V1 (Dungeon Master II: Skullkeep) Support

Status: PASS604_DM2_V1_INITIAL_SUPPORT

## Modules

### dm2_v1_dungeon_loader
- SKULL.ASM dungeon loading routines
- Enhanced DM1 format: outdoor levels, buildings, weather zones
- 30 max levels, 64x64 max map size

### dm2_v1_outdoor_renderer
- SKULL.ASM outdoor viewport, sky gradient, building draw
- Day/night cycle, weather-dependent sky color
- DM2 signature feature

### dm2_v1_combat
- SKULL.ASM combat calculation, ranged attack path
- Extended: crossbow, guns, bombs, range penalty

### dm2_v1_companion
- SKULL.ASM NPC companion AI, loyalty, trading
- Up to 4 companions with own AI/inventory

### dm2_v1_tech_magic
- SKULL.ASM tech/magic hybrid item system
- Tech items: guns, bombs, mechanical devices
- Magic items: traditional + new spells
- Hybrid: combined tech+magic requirements

### dm2_v1_save_load
- SKULL.ASM save/load format

## Source Reference
- SKULL.ASM: 522,128 lines (complete DM2 DOS disassembly)
