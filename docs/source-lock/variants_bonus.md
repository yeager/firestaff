# DM1 V1 Bonus/Discount Items — Source Lock

**Audit date:** 2026-05-25
**Status:** COMPLETE

## Finding: No Bonus Content or Special Editions for DM1

Original Dungeon Master (1987) does **not** have any bonus content, special editions, pre-order bonuses, or discount variants with extra/different items. This was confirmed through:

### Source Evidence

1. **ReDMCSB FILENAME.C**: 
   - The bonus dungeon file referenced is `DUNGEONB.DAT` but this is for **Chaos Strikes Back** (CSB), not DM1
   - No `DungeonB.dat` exists in the DM1 archives

2. **ReDMCSB FILENAME.C** (line from I34E section):
   ```
   char* G1149_pc_BonusDungeonFileName = "A:DUNGEONB.DAT";
   ```
   This is the CSB expansion set file, not DM1 bonus content.

3. **Archive inspection**: The DM1 PC 3.4 archive contains only the standard game files:
   - DM.EXE (LZEXE compressed)
   - DATA/DUNGEON.DAT
   - DATA/GRAPHICS.DAT
   - DATA/SONG.DAT
   - SELECTOR, IBMIO, EGA, VGA, FIRES, ANIM, SWOOSH, TANDY, INSTALL.EXE, STATS.EXE

4. **Spanish GRAPHICS.DAT**: The 8.6 MB `Spanish GRAPHICS.DAT` is a localization resource (not a bonus item), likely from a fan or third-party Spanish translation project.

### DM1 vs CSB Bonus Content

For Chaos Strikes Back, there IS a bonus dungeon file mechanism:
- `DUNGEONB.DAT` = bonus dungeon (from the "Dungeon B" expansion)
- This is the CSB expansion set, not an add-on to DM1

### Conclusion

DM1 V1 has **no bonus content, no special editions, and no discounted variants** with extra items. The only content variants are language (EN/FR/DE) and platform (DOS, Amiga, ST, etc.).

The "bonus" file path in ReDMCSB (DUNGEONB.DAT) belongs to the Chaos Strikes Back expansion set, not to Dungeon Master itself.