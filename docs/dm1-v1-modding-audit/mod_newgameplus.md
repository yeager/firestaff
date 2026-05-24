# DM1 V1 New Game Plus / Replay Features — Source Audit

## ReDMCSB Source Evidence

### No New Game Plus System
ReDMCSB Common/Source contains no new-game-plus, challenge mode, or NG+ save-inheritance system.
- No NG+ flag in GAME struct (DEFS.H:4449–4483)
- No additional difficulty modifiers beyond DungeonHeader.Difficulty field
- No post-credits or post-victory continuation

### DungeonHeader.Difficulty (DEFS.H:1065–1068, MAP Bitfield C)
Per-map difficulty is a 4-bit value (0–15) affecting creature type selection.
This is a dungeon-authoring field, not a player-selectable replay modifier.

### No Persistent Meta-Progression
- No meta-gold, no unlock tracker, no champion legacy system
- Starting party is always fresh (G7111_Games[C0_GAME_SOURCE].GlobalData reset on new game)
- No unlockables from prior playthroughs

### Restart / New Game Flow
- F7052_SaveGame / F7051_LoadGame handle save/load
- Starting new game clears GAME struct via F0008_MAIN_ClearBytes (CEDTINCD.C:75)
- New game is a clean state — no NG+ inheritance

### One Save Slot Per Game
- G7111_Games array has 2 entries (CEDT001.C:28: GAME* G7111_Games /* Pointer to array of 2 structs */)
- Index 0 = DM campaign, Index 1 = CSB campaign
- Only one active campaign save at a time per game

## Firestaff Implementation
Firestaff preserves the single-slot approach. No NG+ system implemented.
DungeonHeader.Difficulty field is read but no separate difficulty mode menu.

## References
- ReDMCSB/DEFS.H:1065–1068 — MAP Bitfield C (Difficulty field)
- ReDMCSB/CEDT001.C:28 — GAME array declaration
- ReDMCSB/CEDTINCD.C:75 — GAME struct clear on new game
- ReDMCSB/DEFS.H:4449–4483 — GAME struct definition

STATUS: DOCUMENTED — No new game plus or replay feature exists in DM1 V1 source or Firestaff.
