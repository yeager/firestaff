# CSB V1 - Graphics/UI Changes Audit

## Source Paths
- CSB: /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Viewport.cpp, Graphics.cpp
- CSB: /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Bitmaps.cpp, UI.h, UI.cpp
- DM1: ReDMCSB DUNVIEW.C, GRAPH21.C, BugsAndChanges.htm

## Vertical Blank Handler Fix (CHANGE7_01_FIX)
- File: BASE.C
- DM1 bug (BUG0_03): VBL interrupts could be ignored under heavy load,
  causing top of dungeon view to use wrong color palette
- CSB: VBL handler changed so no interrupts can be ignored and palette
  switching always starts on time
- **Only significant graphics bug fix in CSB**

## Wall Drawing Optimization (CHANGE7_15_OPTIMIZATION)
- File: DUNVIEW.C
- New function to draw walls in center of dungeon view without
  unnecessary transparency support
- Performance optimization, no visual change

## Code Conversion to Assembly (CHANGE7_16_OPTIMIZATION)
- Files: CHAMDRAW.C, CHAMPION.C, DUNGEON.C, REVIVE.C, TIMELINE.C
- Some source code converted from C to assembly for performance/size
- No visual change, internal optimization

## Maximum Loadable Graphics Update (CHANGE7_13_OPTIMIZATION)
- Files: DEFS.H, DUNVIEW.C
- Maximum number of loadable graphics updated
- Capacity change, no visual difference

## Mouse Pointer Handling (CHANGE7_14_FIX)
- Files: CHEST.C, DUNVIEW.C, LOADSAVE.C, MOVESENS.C, STARTUP1.C
- Part of BUG0_00 fix for useless code removal

## Champion Bars Click (CHANGE7_28_IMPROVEMENT)
- Files: COMMAND.C, DEFS.H
- Left click on champion bars opens inventory
- UI interaction change, not a graphics asset change

## Color Palette Handling

### BUG0_04 (DUNGEON.C - NOT FIXED in CSB)
- DM1: Lord Chaos front/side/attack bitmaps use color 9 but no replacement specified
- On Lord Chaos map, Demon-defined colors are used (appropriate but unspecified)
- **This bug persists in CSB** - not listed in CSB-specific fixes
- Zytaz bitmaps also noted with color 9/10 issues

## CSB Engine Version Display (CHANGE7_36, CHANGE8_13)
- DIALOG.C: Engine version 2.0/2.1 printed in top right corner of dialog boxes
- New UI element vs DM1

## Copy Protection Graphics (CHANGE7_04, CHANGE7_10)
- Files: DUNVIEW.C, CHAMPION.C, COPYPRO6.C, DEFS.H, NEWMAP.C, PANEL.C, TIMELINE.C
- Additional copy protection checks for fuzzy bits detection
- No gameplay graphics changes

## Graphics Asset Count
- CSB Statistics.cpp: NUM_MONSTER_TYPE = 27 (same as DM1)
- No new creature graphic sets added beyond the Grey Lord (0x1a)
- No new dungeon tile graphics
- No new UI element bitmap additions

## Conclusion
CSB graphics/UI changes vs DM1 are MINIMAL:
1. VBL interrupt fix (visual quality under load)
2. Wall drawing optimization (performance)
3. Engine version displayed in dialog boxes (new UI element)
4. Left-click champion bar interaction (UI interaction)
5. BUG0_04 (Lord Chaos color palette) NOT fixed in CSB
6. Some code-to-assembly conversion for rendering functions
No new graphic assets, no new sprite sheets, no new animation types.
