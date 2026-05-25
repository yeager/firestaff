# DM2 V1 — Autosave System

## Verdict: DM2 Has NO Autosave

DM2 does not implement an automatic/periodic save mechanism. There is no equivalent of a "quicksave" or periodic auto-save in the source.

## Evidence

- No `autosave`, `auto_save`, `AutoSave`, or similar identifier anywhere in the codebase.
- Save is purely manual: triggered only via the **Disk** icon button in the right-panel UI (`UI_EVENTCODE_DISK_OP`).
- When the Disk button is pressed, `GAME_SAVE_MENU()` is invoked. The function begins with the comment `_2066:0D09` (original flat-address notation).
- `GAME_SAVE_MENU()` calls `_0aaf_02f8_DIALOG_BOX(0x0b, bp13)` for an empty slot dialog or `(0x12, bp13)` for an existing save overwrite confirmation.
- There is no ticker-based autosave trigger in the game loop (`MAIN_LOOP`), timer system, or `PROCESS_ACTUATOR_TICK_GENERATOR`.

## DM2 Save Trigger Path

```
User clicks Disk icon (UI_EVENTCODE_DISK_OP)
  -> GAME_SAVE_MENU()
     -> Dialog box: empty slot selection (0x0b) or overwrite confirm (0x12)
        -> USER TYPES NAME via _0aaf_02f8_DIALOG_BOX(0x0d, 0x00)
           -> _2066_33e7() — slot picker (scans 10 slots for first 0xDEAD/0xBEEF entry)
              -> GAME_SAVE() — full serialized save to SKSave.dat
```

## Contrast with DM1

DM1 also had no autosave. DM1 saves were triggered manually, per-champion via `CHAMP.DAT` writes and per-dungeon-level via `DUNGEON.DAT`. Neither game implements background auto-save.

## What DM2 DOES Have as a Safety Net

1. **Backup file**: Before writing `SKSave.dat`, the existing file is renamed to `SKSave.bak`. If the write fails midway, `.bak` still holds the previous save.
2. **Resume path**: The "RESUME GAME" main-menu option (`glbSpecialScreen = 0`) bypasses the slot-selection dialog and directly loads `SKSave.dat` (or `.bak` fallback). This is effectively a single "last session" slot.
3. **Load fallback chain**: `GAME_LOAD()` tries `SKSave.dat` first, then `SKSave.bak`, then `Dungeon.ftl` (fresh new game).

## Bottom Line

Neither DM1 nor DM2 has autosave. DM2 instead provides a manually-triggered multi-slot save dialog and a safety backup mechanism. The absence of autosave means a crash loses all progress since the last manual save.