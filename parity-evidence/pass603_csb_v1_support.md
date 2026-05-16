# Pass603 — CSB V1 Support

Status: PASS603_CSB_V1_INITIAL_SUPPORT

## Scope

Initial CSB (Chaos Strikes Back) V1 support modules source-locked to
CSBWin (BeipDev) and ReDMCSB shared code.

## Modules

### csb_v1_dungeon_loader
- CSBWin/CSBCode.cpp:318-480 DBank::Initialize
- CSBWin/CSBCode.cpp:6800-6950 LoadDungeon
- ReDMCSB DUNGEON.C F0148-F0170 shared format
- CSB extensions: DSA thing type 15, level headers

### csb_v1_chaos_magic (DSA system)
- CSBWin/Chaos.cpp: InitializeE, _CALL0-_CALL9
- CSBWin/DSA.cpp: full bytecode interpreter (5806 lines)
- DSA opcodes: SET/CLEAR/TOGGLE/TEST/JUMP/CALL/RETURN/DELAY/SOUND/SPAWN/MOVE/DAMAGE/TELEPORT/MESSAGE/END
- 256 global flags, per-script stack

### csb_v1_viewport
- CSBWin/Viewport.cpp: 7290 lines (CSB rendering)
- CSBWin/Graphics.cpp: 3186 lines (asset cache)
- CSB-specific: custom backgrounds, extended wall sets, prison door intro

### csb_v1_monster
- CSBWin/Monster.cpp: 4819 lines (creature AI)
- CSBWin/Attack.cpp: 2431 lines (attack resolution)
- Shared: ReDMCSB GROUP.C F0190-F0230

### csb_v1_character
- CSBWin/Character.cpp: 5528 lines
- CSBWin/SaveGame.cpp: 2953 lines (DM1 import)
- CSB-specific: DM1 champion import, no selection hall

### csb_v1_save_load
- CSBWin/SaveGame.cpp: 2953 lines

## Non-Claims
- No pixel parity with original CSB Amiga/Atari.
- DSA interpreter is structural, not bytecode-verified.
