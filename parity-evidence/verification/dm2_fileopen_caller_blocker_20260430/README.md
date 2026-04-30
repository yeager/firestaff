# DM2 startup file-open higher-caller blocker (2026-04-30)

Scope: one narrow evidence gate for the DM2 generic DOS file-open callsite. No runtime, gameplay, render, or asset parity is claimed.

## ReDMCSB source boundary audit first

ReDMCSB is used only for DM/CSB boundary comparison, not as DM2 C source:

- `FILENAME.C:4-10` (`global filename definitions`) names the DM/CSB PC data members: `DATA\DUNGEON.DAT` and `DATA\GRAPHICS.DAT`.
- `MEMORY.C:1212-1285` (`F0477_MEMORY_OpenGraphicsDat_CPSDF`) shows the DM/CSB graphics open boundary, including `F0770_FILE_Open(G2130_GraphicsDatFileName)` and the graphics-open error stop.
- `LOADSAVE.C:2333-2377` (dungeon-load open block in `F0436_LOADSAVE_LoadGameFile`) shows the DM/CSB dungeon open boundary, including `F0770_FILE_Open(G1059_pc_DungeonFileName)`.

## DM2 disassembly evidence only

`SKULL.ASM` is treated as disassembly evidence only, explicitly not C source.

Verified DM2 anchors:

- Generic DOS open inner routine: `SKULL.ASM:6154-6200`; decoded bytes include `mov si,[bp+0Eh]`, leading-space trim, `mov dx,si`, `mov ah,3Dh`, `int 21h`.
- Wrapper to inner routine: `SKULL.ASM:6139-6153` pushes filename/flags args and calls the inner routine.
- Direct callers found in canonical `SKULL.EXE`: image offsets `0x1439` and `0x1464`, corresponding to `SKULL.ASM:4761-4783` and `SKULL.ASM:4813-4826`. Both prepare `lea ax,[bp+0Ah]` and push a path buffer before calling the wrapper.
- Startup member strings: `SKULL.ASM:461403-461472` contains `.Z020DUNGEON.DAT`, `.Z020DUNGEON.FTL`, `.Z020GRAPHICS.DAT`.

Canonical binary anchors:

- `SKULL.EXE` sha256 `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35`.
- `SKULL.EXE+0x1a3a`: `B4 3D CD 21`.
- `SKULL.EXE+0x70c65`: `DUNGEON.DAT`.
- `SKULL.EXE+0x70c98`: `GRAPHICS.DAT`.

## Finding

Blocked precisely: the generic DOS open routine and canonical member strings are verified, but this db-only `SKULL.ASM` dump does not expose a statically verifiable higher-level caller/string-xref path from `DUNGEON.DAT` or `GRAPHICS.DAT` to the selected generic open routine. The only direct wrapper callers verified by byte scan use a `BP+0Ah` path buffer, not a direct member-string pointer.

Next step: use a labeled DM2 disassembly, decompilation/xref pass, or dynamic trace over the startup member loader to bridge the string-table reference to the `BP+0Ah` path buffer before making a caller-path claim.

## Validation

```sh
python3 -m py_compile tools/verify_dm2_fileopen_caller_blocker.py
python3 tools/verify_dm2_fileopen_caller_blocker.py
mkdir -p parity-evidence/verification/dm2_fileopen_caller_blocker_20260430 && python3 tools/verify_dm2_fileopen_caller_blocker.py > parity-evidence/verification/dm2_fileopen_caller_blocker_20260430/evidence.json
git diff --check
```
