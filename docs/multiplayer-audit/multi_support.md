# DM1 V1 Multiplayer Support — Source Lock Audit

## Source Evidence

**ReDMCSB:** `Toolchains/Common/Source/*.c` — 50 C/H source files, zero multiplayer references
**Firestaff:** `src/engine/firestaff_multiplayer.c`, `include/firestaff_multiplayer.h`

## Finding: DM1 V1 Has No Multiplayer Support

Dungeon Master (DM1) V1 was designed and shipped as a **single-player experience**.
No multiplayer code, data structures, or documentation exist in ReDMCSB for DM1.

### ReDMCSB Evidence
- `Toolchains/Common/Source/` — 50 C/H source files covering: rendering (BLIT\*.C), dungeon (DUNGEON\*.C), creatures (CREATURE\*.C), magic (CASTER.C), AI (CEDT\*.C), save/load (SAVEHEAD.C, LOADSAVE.C), party (PARTY.C), graphics (ANIM\*.C), inventory (AMMO.C), doors (DOOR\*.C), sensors (SENSOR\*.C)
- Grep across all 50 files for `multiplayer|two.player|link|serial|modem` — **zero matches**
- Documentation: Engine.htm, BugsAndChanges.htm, Readme.htm, Toolchains.htm — none reference networking, serial links, or multi-player modes for DM1
- ReDMCSB.xlsx (media/files manifest) lists only single-player executables

### Historical Context
DM1 (1987, Atari ST) predates widespread consumer networking. The game was distributed on floppy disk with no built-in serial/null-modem capability. Players ran the game solo, controlling a party of up to 4 champions through the dungeon.

### Firestaff Multiplayer Header
`include/firestaff_multiplayer.h` references "DM2-style LAN play" as the architecture target — confirming the multiplayer framework was designed for **DM2** and retrofitted into the Firestaff engine, not derived from DM1 V1 source.

## Conclusion

| Item | Status |
|------|--------|
| Original DM1 multiplayer (serial cable) | **NOT SUPPORTED** |
| Original DM1 multiplayer (null-modem) | **NOT SUPPORTED** |
| Original DM1 multiplayer (modem) | **NOT SUPPORTED** |
| ReDMCSB source evidence | **ZERO multiplayer code** |
| Source lock path | `Toolchains/Common/Source/` — confirmed single-player |

