# DM1 V1 Version Differences (3.4 vs 3.5 vs 3.6) — Source Lock

**Audit date:** 2026-05-25
**Status:** INCOMPLETE — requires additional research

## Finding: PC Version 3.4 Only (Available Archive)

The original DM1 PC archive available on N2 is specifically the **Dungeon Master 3.4** release (DOS English). No 3.5 or 3.6 PC release archives were found in the original-games directory.

The file `Game,Dungeon_Master,DOS,Software.7z` contains the PC 3.4 release.

## ReDMCSB Version Identifiers

ReDMCSB's `COMPILE.H` provides version metadata via EXEID values:

| EXEID | Designation | Platform |
|-------|-------------|----------|
| 82 | `I34M\ANIM` | PC 3.4 Multilingual |
| 83 | `I34M\DM.EXE` | PC 3.4 Multilingual |
| 84 | `I34M\FIRES` | PC 3.4 Multilingual |
| 85 | `I34M\INSTALL.EXE` | PC 3.4 Multilingual |
| 86 | `I34M\SELECTOR` | PC 3.4 Multilingual |
| 71 | `I34E\ANIM` | PC 3.4 English |
| 72 | `I34E\DM.EXE` | PC 3.4 English |
| ... | ... | ... |

All EXEIDs in the 70s–80s range correspond to **version 3.4** only (I34 = "IBM 3.4").

## No 3.5 or 3.6 in ReDMCSB for PC

The ReDMCSB source base appears to include:
- **DM1** = versions 3.1, 3.3, 3.4, 3.5, 3.6 (various platforms)
- **DM2/CSB** = versions 2.x

For the IBM PC specifically, ReDMCSB only targets versions **3.4** (`I34E`, `I34M`). The `A35E` / `A35M` designations are **Amiga** versions (Amiga 3.5 English/Multilingual), NOT PC 3.5.

## Version Progression Evidence

From ReDMCSB `COMPILE.H`, the version designation pattern:
- `A31E` / `A31M` = Amiga 3.1
- `A33M` = Amiga 3.3 (multilingual)
- `A35E` / `A35M` = Amiga 3.5 (English/Multilingual)
- `A36M` = Amiga 3.6 (multilingual)
- `I34E` / `I34M` = IBM PC 3.4 (English/Multilingual)

There is no IBM PC 3.5 or 3.6 in the ReDMCSB source tree. The PC version 3.4 appears to have been the final/different version for that platform.

## What Changed Between DM1 Versions?

Based on the ReDMCSB Bugs and Changes documentation, version changes include:

- **Bug fixes** (CHANGE1_xx through CHANGE7_xx)
- **Optimization changes** (code converted from C to assembly)
- **Ghost champion movement behavior** changed between early and late versions
- **Saved game file naming** for different language variants (BUGX_XX comments)

## Conclusion

- DM1 PC version is **3.4 only** in available archives
- ReDMCSB has **no 3.5 or 3.6 for IBM PC** — only 3.4 (I34E, I34M)
- The version numbers 3.5 and 3.6 exist in the ReDMCSB taxonomy but are **Amiga platform** versions (A35E/A35M/A36M)
- No version-difference dungeon data found between versions for PC, as only one PC version archive is available