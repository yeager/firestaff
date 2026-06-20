# DM1 V1 Bonus/Discount Items — Source Lock

**Audit date:** 2026-05-25
**Status:** COMPLETE

## Finding: PC 3.4 DM1 has no bonus content; Amiga 2.2 has a kid-dungeon sidecar

Original Dungeon Master PC 3.4 does **not** have any bonus content, special
editions, pre-order bonuses, or discount variants with extra/different items.
That runtime claim remains true for DM1 V1 PC34 parity work.

The earlier blanket statement that no DM1 archive contains `DungeonB.dat` was
too broad. The local DM1 Amiga 2.2 English archive now has an extracted
`DUNGEONB.DAT` kid-dungeon sidecar:

- Path: `~/.firestaff/data/dm1-extras/amiga-2.2-en/DUNGEONB.DAT`
- Size: 4,806 bytes
- SHA256: `9bac133b4d8d6ca88abad70ff4a3a6436f264e3ae3a7503e0b40a8a6b4007730`
- MD5: `d42915cf346494efa0ed78cfbbb4c2b5`

This is tracked as Amiga 2.2 provenance only. It is not promoted as a DM1 PC34
runtime file and it is not the 2,098-byte CSB dungeon hash that Greatstone's
`dm_amiga_22_en/dungeon.dat.kid` cross-link currently maps to in the older
inventory report.

### Source Evidence

1. **ReDMCSB FILENAME.C**:
   - The shared bonus dungeon file reference is `DUNGEONB.DAT`.
   - For DM1 PC34 parity this is not a runtime-required file.
   - For DM1 Amiga 2.2 provenance, a separate extracted 4,806-byte
     `DUNGEONB.DAT` sidecar is now hash-locked.

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

DM1 V1 PC34 has **no bonus content, no special editions, and no discounted
variants** with extra items. The only PC34 content variants are language
(EN/FR/DE) and platform ports. The DM1 Amiga 2.2 English kid-dungeon sidecar is
tracked separately as platform provenance and data-coverage evidence.

The shared "bonus" file path in ReDMCSB (`DUNGEONB.DAT`) must therefore stay
out of DM1 PC34 launch requirements, but it should not be used to deny the
separate Amiga 2.2 sidecar that now exists locally and in the hash registry.
