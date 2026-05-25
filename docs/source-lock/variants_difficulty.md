# DM1 V1 Difficulty Variants — Source Lock

**Audit date:** 2026-05-25
**Status:** COMPLETE

## Finding: No Difficulty Settings in Original DM1

Original Dungeon Master (1987) does **not** have configurable difficulty levels, difficulty modes, or separate easy/hard variants. This was confirmed through:

### Source Evidence

1. **ReDMCSB FILENAME.C**: No difficulty-specific file paths or dungeon variants
2. **ReDMCSB DUNGEON.C**: No difficulty branching logic
3. **ReDMCSB GAMEPLAY.C / COMBAT.C**: No difficulty multiplier flags
4. **Dungeon file inspection**: Only one DUNGEON.DAT per language variant; no separate "easy" or "hard" dungeon files

### Comparison with Later Games

Chaos Strikes Back (CSB) introduced a separate "DungeonB.dat" (EXPANSION_SET / bonus dungeon) and there are bugfix changes that vary by version, but the original DM1 release did not include an easy mode or hard mode.

### Conclusion

DM1 V1 has **no difficulty variants**. The dungeon layout and game balance were fixed for all players. The only variants are:
- Language (EN/FR/DE via DUNGEONF.DAT / DUNGEONG.DAT)
- Regional (US English vs EU Multilingual)
- Platform (DOS, Amiga, Atari ST, FM-Towns, Mac, PC-98, etc.)

There are no "dungeon_easy.dat" or "dungeon_hard.dat" files in any archive.