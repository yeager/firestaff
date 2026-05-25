# DM2 V1 — Checkpoint/Restart System

## Verdict: DM2 Has NO Checkpoint/Restart System

DM2 does not implement a checkpoint-restart mechanism. There is no concept of "checkpoints" in the source—no markers, no auto-restart from a mid-level position, no " Continue from here" feature beyond full save slot restores.

## What Exists Instead

### 1. Full Slot Restore
When a save slot is loaded (`GAME_LOAD()` -> `SELECT_LOAD_GAME()`), the game restores **everything**:
- Dungeon layout (all maps, all object records)
- All champion states (HP, mana, equipment, inventory)
- All timer states
- Player position, facing direction, current map
- All global variables and spell effects

This is all-or-nothing. There is no partial restore or "rewind to checkpoint" capability.

### 2. Dungeon.ftl as the "Last Resort" Fallback
In `GAME_LOAD()`, if both `SKSave.dat` and `SKSave.bak` fail to open, the game tries to load `Dungeon.ftl`:
```cpp
glbDataFileHandle = FILE_OPEN(FORMAT_SKSTR(ptrDungeon_ftl, NULL));
if (glbDataFileHandle >= 0) {
    bp04 = 1;
    glbSpecialScreen = 1;
    goto _2db4;  // restart fresh from .ftl
}
```
If `Dungeon.ftl` fails, it shows an error and exits. `Dungeon.ftl` is the compiled dungeon definition file—not a checkpoint.

### 3. The Resume Path (Main Menu "Resume")
"Resume Game" on the main menu (`UI_EVENTCODE_RESUME_GAME`) bypasses the slot-selection dialog entirely and loads the last-saved state directly from `SKSave.dat` (or `.bak`). This is effectively a "return to last save state" but still requires a prior manual save.

### 4. Death/Defeat = Full Restart
If `glbPlayerDefeated` is set (party wipe), the game shows a "defeat" state and the player must resume from a save slot or start a new game. There is no mid-dungeon checkpoint revive.

### 5. No Mid-Level Save Markers
Unlike modern games with explicit checkpoint markers, DM2 has:
- No trigger-based checkpoint registration
- No named restart positions
- No "restart at entrance of current level" shortcut
- No re-spawn at crypt/door after death (beyond the cryocell/revive champion system which is spell-based, not save-based)

## DM1 Comparison

DM1 also had no checkpoint system. DM1 saved champion state to `CHAMP.DAT` and dungeon state to `DUNGEON.DAT` per level. To "checkpoint," players had to manually leave the current dungeon level and save at the overworld. DM2's save system is more centralized (single file per slot) but still requires explicit manual saves.

## Summary Table

| Feature | DM2 Present? |
|---------|-------------|
| Named save slots (10) | Yes |
| Manual save (Disk button) | Yes |
| Backup `.bak` on save | Yes |
| Load from named slot | Yes |
| Resume last save (no dialog) | Yes |
| Automatic/periodic save | No |
| Checkpoint markers | No |
| Mid-level restart point | No |
| Death = auto-reload checkpoint | No |
| Dungeon.ftl fallback (fresh start) | Yes |