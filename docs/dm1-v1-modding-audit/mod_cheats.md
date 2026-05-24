# DM1 V1 Cheat Codes / Debug Features — Source Audit

## ReDMCSB Source Evidence

### No Debug/Cheat Menu in Source
ReDMCSB Common/Source contains no debug console, cheat code parser, or developer menu.
- No keyboard handler for cheat sequences (e.g., no IDDQD, SPECK, MUGGER, etc.)
- No debug overlay for coordinates, FPS, entity counts, or event timelines
- No developer-only options in the main menu

### No Wizard/God Mode
- No F7088_SetWizardMode or equivalent in any CEDT*.C file
- No immortality flag in CHAMPION struct (DEFS.H:659–705)
- No ignore-damage flag in CHAMPION struct

### No Teleport/Warp Command
- No debug teleport function in the disassembly
- No developer-only position-set function
- Movement is entirely game-logic driven

### No Test Dungeon / Demo Mode
- No DEMO, DEMO2, or shareware trigger in the source
- No hidden startup mode triggered by command-line arguments
- G3792_CAN_T_FIND_NEW_ADVENTURE is the only adventure-not-found string (CEDTINCD.C)

### What Debug Infrastructure Exists
The event system (EVENT struct, G_EVENT_MAXCOUNT) provides logging hooks:
- Events are logged to Timeline (CEDTINC8.C save/load of G.TimeLine)
- F7058_WriteSavePartWithChecksum validates each save section
- Checksum mismatch causes error exit with code 23004

The MEDIA671_GPROF conditional in COMPILE.H suggests the original Atari ST build had
profiling, but no runtime cheat/debug layer.

## Conclusion
DM1 V1 has **no cheat codes, no debug menu, and no developer console**. The original
games (FTL Productions, 1987) shipped without any cheat infrastructure in the code.
The only debug mechanism is the checksummed save system and error-exit longjmp.

## References
- ReDMCSB/LOADSAVE.C:2337–2373 — Data file path search (no debug args)
- ReDMCSB/DEFS.H:659–705 — CHAMPION struct (no cheat flags)
- ReDMCSB/CEDTINC8.C — Checksum validation during save/load
- ReDMCSB/COMPILE.H — MEDIA671_GPROF conditional (profiling only)

STATUS: DOCUMENTED — No cheat codes, debug menus, or developer features exist in DM1 V1 source or Firestaff.
